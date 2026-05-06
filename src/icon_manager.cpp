#include "icon_manager.h"

IconManager::IconManager() : m_hListView(NULL), m_explorerPid(0) {}
IconManager::~IconManager() {}

HWND IconManager::FindDesktopListView() {
    HWND hShellView = NULL;
    HWND hWorkerW   = NULL;
    HWND hProgman   = FindWindow("Progman", NULL);
    HWND hDesktop   = GetDesktopWindow();

    if (hProgman) {
        hShellView = FindWindowEx(hProgman, NULL, "SHELLDLL_DefView", NULL);
        if (!hShellView) {
            do {
                hWorkerW = FindWindowEx(hDesktop, hWorkerW, "WorkerW", NULL);
                if (!hWorkerW) break;
                hShellView = FindWindowEx(hWorkerW, NULL, "SHELLDLL_DefView", NULL);
            } while (!hShellView);
        }
    }
    if (hShellView)
        return FindWindowEx(hShellView, NULL, "SysListView32", "FolderView");
    return NULL;
}

bool IconManager::Initialize() {
    m_hListView = FindDesktopListView();
    if (m_hListView)
        GetWindowThreadProcessId(m_hListView, &m_explorerPid);
    return m_hListView != NULL;
}

int IconManager::GetIconCount() const {
    if (!m_hListView) return 0;
    return (int)SendMessage(m_hListView, LVM_GETITEMCOUNT, 0, 0);
}

bool IconManager::PinIcon(int index, int x, int y) {
    if (!m_hListView) return false;
    int count = GetIconCount();
    if (index < 0 || index >= count) return false;
    return SendMessage(m_hListView, LVM_SETITEMPOSITION,
        (WPARAM)index, MAKELPARAM(x, y)) == TRUE;
}

std::string IconManager::GetIconName(int index) const {
    if (!m_hListView || m_explorerPid == 0) return "";
    if (index < 0 || index >= GetIconCount()) return "";

    HANDLE hProc = OpenProcess(
        PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        FALSE, m_explorerPid);
    if (!hProc) return "";

    const int BUF = MAX_PATH;
    void* pMem = VirtualAllocEx(hProc, NULL, sizeof(LVITEMA) + BUF,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pMem) { CloseHandle(hProc); return ""; }

    char*   pRemText = (char*)pMem + sizeof(LVITEMA);
    LVITEMA lvi = {};
    lvi.mask = LVIF_TEXT; lvi.iItem = index;
    lvi.pszText = pRemText; lvi.cchTextMax = BUF;
    WriteProcessMemory(hProc, pMem, &lvi, sizeof(LVITEMA), NULL);
    SendMessage(m_hListView, LVM_GETITEMA, (WPARAM)index, (LPARAM)pMem);

    char local[MAX_PATH] = {};
    ReadProcessMemory(hProc, pRemText, local, BUF - 1, NULL);
    VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);
    CloseHandle(hProc);
    return std::string(local);
}

POINT IconManager::GetIconPosition(int index) const {
    POINT pt = {-1, -1};
    if (!m_hListView || m_explorerPid == 0) return pt;
    if (index < 0 || index >= GetIconCount()) return pt;

    HANDLE hProc = OpenProcess(
        PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        FALSE, m_explorerPid);
    if (!hProc) return pt;

    POINT* pMem = (POINT*)VirtualAllocEx(hProc, NULL, sizeof(POINT),
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pMem) { CloseHandle(hProc); return pt; }

    if (SendMessage(m_hListView, LVM_GETITEMPOSITION, (WPARAM)index, (LPARAM)pMem)) {
        ReadProcessMemory(hProc, pMem, &pt, sizeof(POINT), NULL);
    }

    VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);
    CloseHandle(hProc);
    return pt;
}

// P3: Check if the desktop ListView has LVS_AUTOARRANGE style
bool IconManager::IsAutoArrangeEnabled() const {
    if (!m_hListView) return false;
    LONG_PTR style = GetWindowLongPtr(m_hListView, GWL_STYLE);
    return (style & LVS_AUTOARRANGE) != 0;
}

// P3: Remove LVS_AUTOARRANGE so PinIcon positions are respected
bool IconManager::DisableAutoArrange() {
    if (!m_hListView) return false;
    LONG_PTR style = GetWindowLongPtr(m_hListView, GWL_STYLE);
    if (!(style & LVS_AUTOARRANGE)) return true; // already off
    style &= ~(LONG_PTR)LVS_AUTOARRANGE;
    SetWindowLongPtr(m_hListView, GWL_STYLE, style);
    // Verify
    return !(GetWindowLongPtr(m_hListView, GWL_STYLE) & LVS_AUTOARRANGE);
}
