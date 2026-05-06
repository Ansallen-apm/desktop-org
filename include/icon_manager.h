#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

struct IconData {
    int index;
    POINT pt;
    std::string name;
};

class IconManager {
public:
    IconManager();
    ~IconManager();

    bool Initialize();

    int  GetIconCount() const;
    bool PinIcon(int index, int x, int y);

    // Cross-process memory reads
    std::string GetIconName(int index) const;
    POINT GetIconPosition(int index) const;
    
    // Bulk read to prevent N+1 IPC allocations (Performance optimization)
    std::vector<IconData> GetAllIcons() const;

    // Desktop auto-arrange state
    // P3: Detect and disable "Auto Arrange" so PinIcon takes effect
    bool IsAutoArrangeEnabled() const;
    bool DisableAutoArrange();

private:
    HWND  m_hListView;
    DWORD m_explorerPid;

    HWND FindDesktopListView();
};

#endif // ICON_MANAGER_H
