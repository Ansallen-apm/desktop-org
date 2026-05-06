#include <windows.h>
#include <windowsx.h>  // GET_X_LPARAM, GET_Y_LPARAM
#include "desktop_hooker.h"
#include "zone_manager.h"
#include "icon_manager.h"
#include "auto_sorter.h"
#include "config_manager.h"

// Global managers
ZoneManager g_zoneManager;
IconManager g_iconManager;

// ---------------------------------------------------------------------------
// Window Procedure
// ---------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool isVisible = true;

    switch (uMsg) {
        // Fix #1: Unregister hotkey BEFORE the window is destroyed to prevent
        //         a system-wide hotkey leak that would cause RegisterHotKey()
        //         to silently fail on the next launch.
        case WM_DESTROY:
            UnregisterHotKey(hwnd, 1);
            PostQuitMessage(0);
            return 0;

        case WM_HOTKEY:
            if (wParam == 1) {
                isVisible = !isVisible;
                ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE);
            }
            return 0;

        // Fix #4: WM_NCHITTEST-based pass-through replaces the broken WS_EX_TRANSPARENT approach.
        // - Return HTCLIENT for areas INSIDE a zone (we want mouse events).
        // - Return HTTRANSPARENT for areas OUTSIDE zones (clicks pass through to desktop).
        // This makes WM_LBUTTONDBLCLK work correctly inside zones.
        case WM_NCHITTEST: {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            int zoneIdx = g_zoneManager.HitTest(pt);
            if (zoneIdx != -1) {
                return HTCLIENT; // Inside a zone: capture mouse events
            }
            return HTTRANSPARENT; // Outside zones: pass through to desktop
        }

        case WM_LBUTTONDBLCLK:
            isVisible = !isVisible;
            ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE);
            return 0;

        case WM_RBUTTONUP: {
            POINT pt;
            GetCursorPos(&pt);

            int zoneIdx = g_zoneManager.HitTest(pt);
            if (zoneIdx != -1) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1001, "Change Color");
                AppendMenu(hMenu, MF_STRING, 1002, "Rename Zone");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, 1003, "Save Layout");

                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);

                if (cmd == 1001) {
                    MessageBox(hwnd, "Color Picker Dialog would open here.", "Customize", MB_OK);
                } else if (cmd == 1002) {
                    MessageBox(hwnd, "Rename Dialog would open here.", "Customize", MB_OK);
                } else if (cmd == 1003) {
                    // Fix #12: Save current zone layout to INI file
                    std::vector<ZoneConfig> configs;
                    int count = g_zoneManager.GetZoneCount();
                    for (int i = 0; i < count; ++i) {
                        ZoneConfig z = {};
                        RECT r = g_zoneManager.GetZoneRect(i);
                        z.rect = r;
                        z.color = RGB(100, 150, 255); // Placeholder — needs color accessor
                        strncpy_s(z.name, "Zone", _TRUNCATE);
                        configs.push_back(z);
                    }
                    ConfigManager::SaveZones(configs);
                    MessageBox(hwnd, "Layout saved.", "DesktopOrg", MB_OK);
                }
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Clear background (black = invisible with LWA_COLORKEY, or use alpha channel)
            FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // Fix #9: Pass the dirty rect so DrawZones can skip non-visible zones
            g_zoneManager.DrawZones(hdc, &ps.rcPaint);

            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// WinMain
// ---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "DesktopOrgOverlayClass";

    WNDCLASS wc      = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style         = CS_DBLCLKS; // Required for WM_LBUTTONDBLCLK to fire

    // Fix #2: Check RegisterClass return value and handle failure gracefully
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class! The application may already be running.",
                   "DesktopOrg Error", MB_ICONERROR);
        return -1;
    }

    // Fix #3: Check DesktopHooker result and bail out cleanly on failure
    DesktopHooker hooker;
    HWND workerW = NULL;
    if (!hooker.Initialize()) {
        MessageBox(NULL, "Failed to hook the desktop WorkerW layer.\n"
                         "Desktop Org may not work correctly on this system.",
                   "DesktopOrg Warning", MB_ICONWARNING);
        // workerW remains NULL — CreateWindowEx will use the desktop as parent,
        // which is acceptable degraded behavior.
    } else {
        workerW = hooker.GetDesktopWorkerWindow();
    }

    // Fix #12: Try loading zones from saved config first
    auto savedZones = ConfigManager::LoadZones();
    if (!savedZones.empty()) {
        for (auto& z : savedZones) {
            g_zoneManager.AddZone(z.rect, z.name, z.color);
        }
    } else {
        // No config found — use defaults
        RECT r1 = {100, 100, 400, 300};
        g_zoneManager.AddZone(r1, "Work Files", RGB(50, 100, 150));

        RECT r2 = {500, 100, 800, 300};
        g_zoneManager.AddZone(r2, "Personal", RGB(150, 100, 50));
    }

    // Initialize icon manager and run the auto-sorter
    if (g_iconManager.Initialize()) {
        AutoSorter sorter(g_zoneManager, g_iconManager);
        sorter.MapExtensionToZone(".pdf",  0);
        sorter.MapExtensionToZone(".docx", 0);
        sorter.MapExtensionToZone(".xlsx", 0);
        sorter.MapExtensionToZone(".jpg",  1);
        sorter.MapExtensionToZone(".png",  1);
        sorter.MapExtensionToZone(".mp4",  1);
        sorter.RunSort();
    }

    // Fix #4: Removed WS_EX_TRANSPARENT — replaced by WM_NCHITTEST pass-through logic.
    //         WS_EX_TRANSPARENT was blocking ALL mouse events, making WM_LBUTTONDBLCLK dead.
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW,   // No WS_EX_TRANSPARENT — handled via NCHITTEST
        CLASS_NAME,
        "DesktopOrg Overlay",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        workerW,  // May be NULL on fallback — CreateWindowEx handles this safely
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        MessageBox(NULL, "Failed to create the overlay window.", "DesktopOrg Error", MB_ICONERROR);
        return -1;
    }

    // Use color key for transparency: black pixels (0x000000) become invisible
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Fix #1: RegisterHotKey — will be matched by UnregisterHotKey in WM_DESTROY
    if (!RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 0x44)) {
        // Hotkey registration failed (another app may own it) — non-fatal
        MessageBox(hwnd, "Could not register Ctrl+Shift+D hotkey.\nAnother app may be using it.",
                   "DesktopOrg", MB_ICONWARNING);
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
