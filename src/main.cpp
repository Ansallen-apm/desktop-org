#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include "desktop_hooker.h"
#include "zone_manager.h"
#include "icon_manager.h"
#include "auto_sorter.h"
#include "config_manager.h"
#include "dialog_helper.h"

// ─── Globals ─────────────────────────────────────────────────────────────────
ZoneManager g_zoneManager;
IconManager g_iconManager;
AutoSorter* g_pSorter = nullptr;

// ─── System Tray ─────────────────────────────────────────────────────────────
static const UINT WM_TRAYICON      = WM_USER + 1;
static const UINT IDI_TRAY         = 1;
static UINT       WM_TASKBARCREATED = 0;

static void AddTrayIcon(HWND hwnd) {
    NOTIFYICONDATAA nid = {};
    nid.cbSize          = sizeof(nid);
    nid.hWnd            = hwnd;
    nid.uID             = IDI_TRAY;
    nid.uFlags          = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon           = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(nid.szTip, "DesktopOrg - 桌面整理工具 (Ctrl+Shift+D)");
    Shell_NotifyIconA(NIM_ADD, &nid);
}
static void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATAA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd   = hwnd;
    nid.uID    = IDI_TRAY;
    Shell_NotifyIconA(NIM_DELETE, &nid);
}

// ─── Save helper ─────────────────────────────────────────────────────────────
static void SaveLayout(HWND hwnd) {
    int count = g_zoneManager.GetZoneCount();
    std::vector<ZoneConfig> cfgs;
    for (int i = 0; i < count; ++i) {
        ZoneConfig z = {};
        z.rect  = g_zoneManager.GetZoneRect(i);
        z.color = g_zoneManager.GetZoneColor(i);
        strncpy_s(z.name, sizeof(z.name), g_zoneManager.GetZoneName(i), _TRUNCATE);
        cfgs.push_back(z);
    }
    ConfigManager::SaveZones(cfgs);
    MessageBoxA(hwnd, "Layout saved successfully.", "DesktopOrg", MB_OK);
}

// ─── Drag State ──────────────────────────────────────────────────────────────
enum class DragMode { None, CreateZone, MoveZone, ResizeZone };
struct DragState {
    DragMode mode      = DragMode::None;
    POINT    startPt   = {};
    POINT    currentPt = {};
    int      zoneIdx   = -1;
    RECT     origRect  = {};
    int      edgeMask  = 0;  // 1=L,2=T,4=R,8=B
};
static DragState g_drag;

static RECT CalcDragRect() {
    int dx = g_drag.currentPt.x - g_drag.startPt.x;
    int dy = g_drag.currentPt.y - g_drag.startPt.y;
    if (g_drag.mode == DragMode::CreateZone) {
        RECT r;
        r.left   = min(g_drag.startPt.x, g_drag.currentPt.x);
        r.top    = min(g_drag.startPt.y, g_drag.currentPt.y);
        r.right  = max(g_drag.startPt.x, g_drag.currentPt.x);
        r.bottom = max(g_drag.startPt.y, g_drag.currentPt.y);
        return r;
    }
    RECT r = g_drag.origRect;
    if (g_drag.mode == DragMode::MoveZone) {
        r.left += dx; r.right  += dx;
        r.top  += dy; r.bottom += dy;
    } else { // ResizeZone
        if (g_drag.edgeMask & 1) r.left   += dx;
        if (g_drag.edgeMask & 2) r.top    += dy;
        if (g_drag.edgeMask & 4) r.right  += dx;
        if (g_drag.edgeMask & 8) r.bottom += dy;
    }
    // Clamp so rect is never inverted
    if (r.right  < r.left + 60) r.right  = r.left + 60;
    if (r.bottom < r.top  + 60) r.bottom = r.top  + 60;
    return r;
}

// Draw drag preview (dashed frame)
static void DrawDragPreview(HWND hwnd) {
    if (g_drag.mode == DragMode::None) return;
    RECT r = CalcDragRect();
    HDC hdc = GetDC(hwnd);
    HPEN pen = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
    HPEN old = (HPEN)SelectObject(hdc, pen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
    SelectObject(hdc, old);
    DeleteObject(pen);
    ReleaseDC(hwnd, hdc);
}

// ─── Window Procedure ─────────────────────────────────────────────────────────
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool isVisible = true;

    // Re-add tray icon after Explorer restarts
    if (uMsg == WM_TASKBARCREATED) { AddTrayIcon(hwnd); return 0; }

    switch (uMsg) {

    case WM_DESTROY:
        RemoveTrayIcon(hwnd);
        UnregisterHotKey(hwnd, 1);
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        RemoveTrayIcon(hwnd);
        DestroyWindow(hwnd);
        return 0;

    // ── Tray icon messages ──────────────────────────────────────────────────
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt; GetCursorPos(&pt);
            HMENU hm = CreatePopupMenu();
            AppendMenuA(hm, MF_STRING, 2001, isVisible ? "Hide Zones" : "Show Zones");
            AppendMenuA(hm, MF_STRING, 2002, "Sort Desktop Now");
            AppendMenuA(hm, MF_STRING, 2003, "Save Layout");
            AppendMenuA(hm, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hm, MF_STRING, 2004, "Exit");
            SetForegroundWindow(hwnd);
            int cmd = TrackPopupMenu(hm, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hm);
            if (cmd == 2001) { isVisible = !isVisible; ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE); }
            else if (cmd == 2002) { if (g_pSorter) g_pSorter->RunSort(); InvalidateRect(hwnd, NULL, TRUE); }
            else if (cmd == 2003) { SaveLayout(hwnd); }
            else if (cmd == 2004) { PostMessage(hwnd, WM_CLOSE, 0, 0); }
        } else if (lParam == WM_LBUTTONDBLCLK) {
            isVisible = !isVisible;
            ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE);
        }
        return 0;

    // ── Global hotkey ───────────────────────────────────────────────────────
    case WM_HOTKEY:
        if (wParam == 1) { isVisible = !isVisible; ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE); }
        return 0;

    // ── Smart pass-through: only capture events on zones ────────────────────
    case WM_NCHITTEST: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (g_drag.mode != DragMode::None) return HTCLIENT; // Capture during drag
        int em = 0;
        if (g_zoneManager.HitTestEdge(pt, em) != -1) return HTCLIENT;
        if (g_zoneManager.HitTest(pt) != -1)         return HTCLIENT;
        return HTTRANSPARENT; // pass through outside zones
    }

    // ── Mouse cursor feedback ────────────────────────────────────────────────
    case WM_SETCURSOR: {
        POINT pt; GetCursorPos(&pt);
        int em = 0, idx = g_zoneManager.HitTestEdge(pt, em);
        if (idx != -1 && em) {
            bool h = (em & 5) != 0, v = (em & 10) != 0;
            if      (h && v)  SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
            else if (h)       SetCursor(LoadCursor(NULL, IDC_SIZEWE));
            else              SetCursor(LoadCursor(NULL, IDC_SIZENS));
        } else if (idx != -1) {
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        } else {
            SetCursor(LoadCursor(NULL, IDC_CROSS));
        }
        return TRUE;
    }

    // ── Zone drag: start ─────────────────────────────────────────────────────
    case WM_LBUTTONDOWN: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        g_drag.startPt = g_drag.currentPt = pt;
        int em = 0;
        int idx = g_zoneManager.HitTestEdge(pt, em);
        if (idx != -1 && em) {
            g_drag.mode    = DragMode::ResizeZone;
            g_drag.zoneIdx = idx;
            g_drag.edgeMask = em;
            g_drag.origRect = g_zoneManager.GetZoneRect(idx);
        } else if (idx != -1) {
            g_drag.mode    = DragMode::MoveZone;
            g_drag.zoneIdx = idx;
            g_drag.origRect = g_zoneManager.GetZoneRect(idx);
        } else {
            g_drag.mode = DragMode::CreateZone;
        }
        SetCapture(hwnd);
        return 0;
    }

    case WM_MOUSEMOVE: {
        if (g_drag.mode == DragMode::None) return 0;
        g_drag.currentPt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        DrawDragPreview(hwnd);
        return 0;
    }

    // ── Zone drag: commit ────────────────────────────────────────────────────
    case WM_LBUTTONUP: {
        if (g_drag.mode == DragMode::None) return 0;
        ReleaseCapture();
        g_drag.currentPt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        RECT final = CalcDragRect();
        int minSize = 60;
        if ((final.right - final.left) >= minSize &&
            (final.bottom - final.top) >= minSize)
        {
            if (g_drag.mode == DragMode::CreateZone) {
                char name[64] = "New Zone";
                ShowInputDialog(hwnd, "Zone Name:", name, sizeof(name));
                g_zoneManager.AddZone(final, name, RGB(80, 120, 160));
            } else {
                g_zoneManager.SetZoneRect(g_drag.zoneIdx, final);
            }
        }
        g_drag.mode = DragMode::None;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_LBUTTONDBLCLK: {
        // Double-click on empty desktop: toggle visibility
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (g_zoneManager.HitTest(pt) == -1) {
            isVisible = !isVisible;
            ShowWindow(hwnd, isVisible ? SW_SHOW : SW_HIDE);
        }
        return 0;
    }

    // ── Right-click on zone: customize ──────────────────────────────────────
    case WM_RBUTTONUP: {
        POINT pt; GetCursorPos(&pt);
        int idx = g_zoneManager.HitTest(pt);
        if (idx != -1) {
            HMENU hm = CreatePopupMenu();
            AppendMenuA(hm, MF_STRING, 1001, "Change Color");
            AppendMenuA(hm, MF_STRING, 1002, "Rename Zone");
            AppendMenuA(hm, MF_STRING, 1003, "Delete Zone");
            int cmd = TrackPopupMenu(hm, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hm);
            if (cmd == 1001) {
                COLORREF c = g_zoneManager.GetZoneColor(idx);
                if (ShowColorDialog(hwnd, c)) {
                    g_zoneManager.SetZoneColor(idx, c);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (cmd == 1002) {
                char name[64];
                strncpy_s(name, g_zoneManager.GetZoneName(idx), _TRUNCATE);
                if (ShowInputDialog(hwnd, "New zone name:", name, sizeof(name))) {
                    g_zoneManager.SetZoneName(idx, name);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (cmd == 1003) {
                // Rebuild without this zone
                std::vector<ZoneConfig> cfgs;
                int n = g_zoneManager.GetZoneCount();
                for (int i = 0; i < n; ++i) {
                    if (i == idx) continue;
                    ZoneConfig z = {};
                    z.rect  = g_zoneManager.GetZoneRect(i);
                    z.color = g_zoneManager.GetZoneColor(i);
                    strncpy_s(z.name, g_zoneManager.GetZoneName(i), _TRUNCATE);
                    cfgs.push_back(z);
                }
                // Rebuild ZoneManager
                g_zoneManager = ZoneManager();
                for (auto& z : cfgs) g_zoneManager.AddZone(z.rect, z.name, z.color);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        return 0;
    }

    // ── Paint ────────────────────────────────────────────────────────────────
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
        g_zoneManager.DrawZones(hdc, &ps.rcPaint);
        EndPaint(hwnd, &ps);
        return 0;
    }
    } // switch

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// ─── WinMain ─────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");

    const char CLASS_NAME[] = "DesktopOrgOverlayClass";
    WNDCLASSA wc    = {};
    wc.lpfnWndProc  = WindowProc;
    wc.hInstance    = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style        = CS_DBLCLKS;

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Window class registration failed.\nThe app may already be running.",
            "DesktopOrg Error", MB_ICONERROR);
        return -1;
    }

    DesktopHooker hooker;
    HWND workerW = NULL;
    if (!hooker.Initialize()) {
        MessageBoxA(NULL, "Could not hook the desktop layer.\nOverlay may not appear correctly.",
            "DesktopOrg Warning", MB_ICONWARNING);
    } else {
        workerW = hooker.GetDesktopWorkerWindow();
    }

    // Load saved zones, or create defaults
    auto saved = ConfigManager::LoadZones();
    if (!saved.empty()) {
        for (auto& z : saved) g_zoneManager.AddZone(z.rect, z.name, z.color);
    } else {
        RECT r1 = {100, 100, 400, 300};
        g_zoneManager.AddZone(r1, "Work Files", RGB(50, 100, 150));
        RECT r2 = {500, 100, 800, 300};
        g_zoneManager.AddZone(r2, "Personal",   RGB(150, 100, 50));
    }

    // Setup icon manager and auto-sorter
    g_pSorter = nullptr;
    if (g_iconManager.Initialize()) {
        // P3: Warn if auto-arrange is on
        if (g_iconManager.IsAutoArrangeEnabled()) {
            if (MessageBoxA(NULL,
                "Desktop 'Auto Arrange Icons' is ON.\n"
                "Icon pinning will not work until it is disabled.\n\n"
                "Click YES to disable it automatically.",
                "DesktopOrg", MB_YESNO | MB_ICONWARNING) == IDYES)
            {
                g_iconManager.DisableAutoArrange();
            }
        }

        static AutoSorter sorter(g_zoneManager, g_iconManager);
        sorter.MapExtensionToZone(".pdf",  0);
        sorter.MapExtensionToZone(".docx", 0);
        sorter.MapExtensionToZone(".xlsx", 0);
        sorter.MapExtensionToZone(".jpg",  1);
        sorter.MapExtensionToZone(".png",  1);
        sorter.MapExtensionToZone(".mp4",  1);
        sorter.RunSort();
        g_pSorter = &sorter;
    }

    // Create overlay window (no WS_EX_TRANSPARENT; handled via WM_NCHITTEST)
    HWND hwnd = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        CLASS_NAME, "DesktopOrg Overlay",
        WS_POPUP | WS_VISIBLE,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        workerW, NULL, hInstance, NULL
    );
    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create overlay window.", "DesktopOrg Error", MB_ICONERROR);
        return -1;
    }

    // Black = transparent via colorkey
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 0x44)) {
        MessageBoxA(hwnd, "Ctrl+Shift+D hotkey unavailable.\nAnother application may be using it.",
            "DesktopOrg", MB_ICONWARNING);
    }

    AddTrayIcon(hwnd);
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
