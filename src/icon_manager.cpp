#include "icon_manager.h"

IconManager::IconManager() : m_hListView(NULL) {}

IconManager::~IconManager() {}

HWND IconManager::FindDesktopListView() {
    HWND hShellViewWin = NULL;
    HWND hWorkerW = NULL;
    HWND hProgman = FindWindow("Progman", "Program Manager");
    HWND hDesktopWnd = GetDesktopWindow();

    // The desktop ListView is usually a child of SHELLDLL_DefView
    // which can be under Progman or WorkerW
    if (hProgman != NULL) {
        hShellViewWin = FindWindowEx(hProgman, NULL, "SHELLDLL_DefView", NULL);
        if (hShellViewWin == NULL) {
            do {
                hWorkerW = FindWindowEx(hDesktopWnd, hWorkerW, "WorkerW", NULL);
                hShellViewWin = FindWindowEx(hWorkerW, NULL, "SHELLDLL_DefView", NULL);
            } while (hShellViewWin == NULL && hWorkerW != NULL);
        }
    }

    if (hShellViewWin != NULL) {
        return FindWindowEx(hShellViewWin, NULL, "SysListView32", "FolderView");
    }
    return NULL;
}

bool IconManager::Initialize() {
    m_hListView = FindDesktopListView();
    return m_hListView != NULL;
}

int IconManager::GetIconCount() const {
    if (!m_hListView) return 0;
    return (int)SendMessage(m_hListView, LVM_GETITEMCOUNT, 0, 0);
}

bool IconManager::PinIcon(int index, int x, int y) {
    if (!m_hListView) return false;
    
    // Send LVM_SETITEMPOSITION to move the icon
    // Note: To make this stick, "Auto Arrange" must be off on the desktop
    LRESULT result = SendMessage(m_hListView, LVM_SETITEMPOSITION, index, MAKELPARAM(x, y));
    return result != 0;
}
