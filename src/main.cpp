#include <windows.h>
#include "desktop_hooker.h"
#include "zone_manager.h"
#include "icon_manager.h"
#include "auto_sorter.h"

// Global zone manager for now
ZoneManager g_zoneManager;
IconManager g_iconManager;

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool isVisible = true;

    switch (uMsg) {
        case WM_HOTKEY:
            if (wParam == 1) { // Our hotkey ID
                isVisible = !isVisible;
                ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE);
            }
            return 0;
            
        case WM_LBUTTONDBLCLK:
            isVisible = !isVisible;
            ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE);
            return 0;

        case WM_RBUTTONUP: {
            POINT pt;
            GetCursorPos(&pt);
            
            // Check if we right-clicked a zone
            int zoneIdx = g_zoneManager.HitTest(pt);
            if (zoneIdx != -1) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1001, "Change Color");
                AppendMenu(hMenu, MF_STRING, 1002, "Rename Zone");
                
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
                
                if (cmd == 1001) {
                    // Logic to open color dialog (stubbed for now)
                    MessageBox(hwnd, "Color Picker Dialog would open here.", "Customize", MB_OK);
                } else if (cmd == 1002) {
                    // Logic to open rename dialog
                    MessageBox(hwnd, "Rename Dialog would open here.", "Customize", MB_OK);
                }
            }
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Clear background with fully transparent color
            FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH)); // Layered window treats black as transparent if colorkey used, or alpha used.
            
            // Draw all zones
            g_zoneManager.DrawZones(hdc);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[]  = "DesktopOrgOverlayClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    DesktopHooker hooker;
    if (hooker.Initialize()) {
        // Find successfully
    }

    // Initialize some test zones
    RECT r1 = {100, 100, 400, 300};
    g_zoneManager.AddZone(r1, "Work Files", RGB(50, 100, 150));
    
    RECT r2 = {500, 100, 800, 300};
    g_zoneManager.AddZone(r2, "Personal", RGB(150, 100, 50));

    if (g_iconManager.Initialize()) {
        // Arrange first 5 icons into the Work Files zone as a test
        int count = g_iconManager.GetIconCount();
        for (int i = 0; i < count && i < 5; ++i) {
            g_iconManager.PinIcon(i, r1.left + 20 + (i % 3) * 80, r1.top + 40 + (i / 3) * 80);
        }
    }

    AutoSorter sorter(g_zoneManager, g_iconManager);
    sorter.MapExtensionToZone(".pdf", 0); // Map PDFs to Work Files
    sorter.MapExtensionToZone(".jpg", 1); // Map JPGs to Personal
    sorter.RunSort();

    // Create a basic window for now to ensure compile succeeds

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, // Extended styles for overlay
        CLASS_NAME,                     // Window class
        "DesktopOrg Overlay",           // Window text
        WS_POPUP | WS_VISIBLE,          // Borderless popup
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), // Full screen
        hooker.GetDesktopWorkerWindow(), // Parent window (The desktop worker)
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Set 50% transparency for the overlay
    SetLayeredWindowAttributes(hwnd, 0, 128, LWA_ALPHA);

    // Register Ctrl + Shift + D to toggle overlay
    RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 0x44);

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
