#include "desktop_hooker.h"

DesktopHooker::DesktopHooker() : m_workerW(NULL) {}

DesktopHooker::~DesktopHooker() {}

BOOL CALLBACK DesktopHooker::EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    HWND p = FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
    if (p != NULL) {
        // Find the next sibling of SHELLDLL_DefView which is our WorkerW
        HWND* workerW = reinterpret_cast<HWND*>(lParam);
        *workerW = FindWindowEx(NULL, hwnd, "WorkerW", NULL);
        // Fix #7: Return FALSE to stop enumeration once we've found our target.
        // Continuing after finding the result could allow later windows to overwrite
        // the correct handle with a NULL value.
        return FALSE;
    }
    return TRUE;
}

bool DesktopHooker::Initialize() {
    // Get the handle of the Program Manager (Desktop background manager)
    HWND progman = FindWindow("Progman", NULL);
    if (progman == NULL) {
        return false;
    }

    // Send a message to Progman. This undocumented message (0x052C) tells Progman
    // to spawn a WorkerW window behind the desktop icons.
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    // Enumerate windows to find the newly created WorkerW
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&m_workerW));

    return (m_workerW != NULL);
}

HWND DesktopHooker::GetDesktopWorkerWindow() const {
    return m_workerW;
}
