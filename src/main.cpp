#include <windows.h>
#include "desktop_hooker.h"
#include "zone_manager.h"

// Global zone manager for now
ZoneManager g_zoneManager;

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
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


    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
