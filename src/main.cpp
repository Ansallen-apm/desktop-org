#include <windows.h>
#include "desktop_hooker.h"


// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Draw a semi-transparent black background for testing the overlay
            HBRUSH brush = CreateSolidBrush(RGB(50, 50, 50));
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);
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
