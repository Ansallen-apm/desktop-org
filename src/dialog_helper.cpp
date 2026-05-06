#include "dialog_helper.h"
#include <commdlg.h>
#include <string.h>

// ── Color Picker ──────────────────────────────────────────────────────────────
bool ShowColorDialog(HWND hParent, COLORREF& colorInOut) {
    static COLORREF customColors[16] = {};
    CHOOSECOLORA cc = {};
    cc.lStructSize  = sizeof(cc);
    cc.hwndOwner    = hParent;
    cc.lpCustColors = customColors;
    cc.rgbResult    = colorInOut;
    cc.Flags        = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColorA(&cc)) {
        colorInOut = cc.rgbResult;
        return true;
    }
    return false;
}

// ── Input Dialog (no .rc resource file needed) ────────────────────────────────
static char   s_inputBuf[128] = {};
static bool   s_inputAccepted = false;
static HWND   s_inputEdit     = NULL;
static HWND   s_inputParent   = NULL;

static LRESULT CALLBACK InputDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCTA* cs = (CREATESTRUCTA*)lp;
            CreateWindowExA(0, "STATIC", (const char*)cs->lpCreateParams,
                WS_CHILD | WS_VISIBLE, 10, 10, 280, 20, hwnd, NULL, NULL, NULL);
            s_inputEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", s_inputBuf,
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                10, 36, 280, 24, hwnd, (HMENU)101, NULL, NULL);
            SendMessageA(s_inputEdit, EM_SETSEL, 0, -1); // select all text
            CreateWindowA("BUTTON", "OK",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                10, 72, 80, 26, hwnd, (HMENU)IDOK, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel",
                WS_CHILD | WS_VISIBLE,
                100, 72, 80, 26, hwnd, (HMENU)IDCANCEL, NULL, NULL);
            SetFocus(s_inputEdit);
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wp) == IDOK) {
                GetWindowTextA(s_inputEdit, s_inputBuf, sizeof(s_inputBuf));
                s_inputAccepted = true;
            } else if (LOWORD(wp) == IDCANCEL) {
                s_inputAccepted = false;
            } else {
                return DefWindowProcA(hwnd, msg, wp, lp);
            }
            EnableWindow(s_inputParent, TRUE);
            DestroyWindow(hwnd);
            SetForegroundWindow(s_inputParent);
            return 0;
        case WM_CLOSE:
            s_inputAccepted = false;
            EnableWindow(s_inputParent, TRUE);
            DestroyWindow(hwnd);
            SetForegroundWindow(s_inputParent);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

bool ShowInputDialog(HWND hParent, const char* prompt, char* buffer, int bufSize) {
    // Register once
    static bool registered = false;
    if (!registered) {
        WNDCLASSA wc  = {};
        wc.lpfnWndProc   = InputDlgProc;
        wc.hInstance     = GetModuleHandleA(NULL);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = "DesktopOrgInputDlg";
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        RegisterClassA(&wc);
        registered = true;
    }

    strncpy_s(s_inputBuf, sizeof(s_inputBuf), buffer, _TRUNCATE);
    s_inputAccepted = false;
    s_inputParent   = hParent;

    HWND hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        "DesktopOrgInputDlg",
        "DesktopOrg",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        0, 0, 310, 140,
        hParent, NULL, GetModuleHandleA(NULL),
        (LPVOID)prompt
    );

    // Centre over parent
    RECT pr, dr;
    GetWindowRect(hParent, &pr);
    GetWindowRect(hwnd, &dr);
    int w = dr.right - dr.left, h = dr.bottom - dr.top;
    int cx = (pr.left + pr.right) / 2;
    int cy = (pr.top  + pr.bottom) / 2;
    SetWindowPos(hwnd, NULL, cx - w / 2, cy - h / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    EnableWindow(hParent, FALSE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Local message loop (modal)
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    if (s_inputAccepted) {
        strncpy_s(buffer, bufSize, s_inputBuf, _TRUNCATE);
    }
    return s_inputAccepted;
}
