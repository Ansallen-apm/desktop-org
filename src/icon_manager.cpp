#include "icon_manager.h"
#include <string>

IconManager::IconManager() : m_hListView(NULL), m_explorerPid(0) {}

IconManager::~IconManager() {}

HWND IconManager::FindDesktopListView() {
    HWND hShellViewWin = NULL;
    HWND hWorkerW = NULL;
    // Fix #6: Use NULL window title for Progman — more reliable
    HWND hProgman = FindWindow("Progman", NULL);
    HWND hDesktopWnd = GetDesktopWindow();

    if (hProgman != NULL) {
        hShellViewWin = FindWindowEx(hProgman, NULL, "SHELLDLL_DefView", NULL);
        if (hShellViewWin == NULL) {
            // Fix #6: Explicitly guard against NULL hWorkerW before calling FindWindowEx
            do {
                hWorkerW = FindWindowEx(hDesktopWnd, hWorkerW, "WorkerW", NULL);
                if (hWorkerW == NULL) break; // Guard: no more WorkerW windows to check
                hShellViewWin = FindWindowEx(hWorkerW, NULL, "SHELLDLL_DefView", NULL);
            } while (hShellViewWin == NULL);
        }
    }

    if (hShellViewWin != NULL) {
        return FindWindowEx(hShellViewWin, NULL, "SysListView32", "FolderView");
    }
    return NULL;
}

bool IconManager::Initialize() {
    m_hListView = FindDesktopListView();
    if (m_hListView) {
        // Cache the PID of the Explorer process for future IPC calls
        GetWindowThreadProcessId(m_hListView, &m_explorerPid);
    }
    return m_hListView != NULL;
}

int IconManager::GetIconCount() const {
    if (!m_hListView) return 0;
    return (int)SendMessage(m_hListView, LVM_GETITEMCOUNT, 0, 0);
}

bool IconManager::PinIcon(int index, int x, int y) {
    if (!m_hListView) return false;

    // Fix #5: Validate index before sending to SysListView32
    int count = GetIconCount();
    if (index < 0 || index >= count) return false;

    // Send LVM_SETITEMPOSITION to move the icon.
    // Note: "Auto Arrange" must be disabled on the desktop for this to persist.
    return SendMessage(m_hListView, LVM_SETITEMPOSITION, (WPARAM)index, MAKELPARAM(x, y)) == TRUE;
}

// Fix #10: Implement cross-process memory access to read icon filename from Explorer.exe
std::string IconManager::GetIconName(int index) const {
    if (!m_hListView || m_explorerPid == 0) return "";

    int count = GetIconCount();
    if (index < 0 || index >= count) return "";

    // Open a handle to the Explorer process with required permissions
    HANDLE hProcess = OpenProcess(
        PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
        FALSE, m_explorerPid
    );
    if (!hProcess) return "";

    const int BUFFER_SIZE = MAX_PATH;

    // Allocate a contiguous block: LVITEM struct + text buffer in the remote process
    void* pRemoteMem = VirtualAllocEx(
        hProcess, NULL,
        sizeof(LVITEMA) + BUFFER_SIZE,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
    if (!pRemoteMem) {
        CloseHandle(hProcess);
        return "";
    }

    // The text buffer is placed immediately after the LVITEM struct
    char* pRemoteText = (char*)pRemoteMem + sizeof(LVITEMA);

    // Prepare the local LVITEM and write it to the remote process
    LVITEMA lvi = {};
    lvi.mask       = LVIF_TEXT;
    lvi.iItem      = index;
    lvi.iSubItem   = 0;
    lvi.pszText    = pRemoteText; // Points to the buffer in remote process space
    lvi.cchTextMax = BUFFER_SIZE;
    WriteProcessMemory(hProcess, pRemoteMem, &lvi, sizeof(LVITEMA), NULL);

    // Send the message — Explorer fills in pszText inside its own memory space
    SendMessage(m_hListView, LVM_GETITEMA, (WPARAM)index, (LPARAM)pRemoteMem);

    // Read the result back into our local buffer
    char localText[MAX_PATH] = {};
    ReadProcessMemory(hProcess, pRemoteText, localText, BUFFER_SIZE - 1, NULL);

    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return std::string(localText);
}
