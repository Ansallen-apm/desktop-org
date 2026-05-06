#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <windows.h>
#include <commctrl.h>
#include <string>

class IconManager {
public:
    IconManager();
    ~IconManager();

    bool Initialize();

    int  GetIconCount() const;
    bool PinIcon(int index, int x, int y);
    std::string GetIconName(int index) const;

    // P3: Detect and disable "Auto Arrange" so PinIcon takes effect
    bool IsAutoArrangeEnabled() const;
    bool DisableAutoArrange();

private:
    HWND  m_hListView;
    DWORD m_explorerPid;

    HWND FindDesktopListView();
};

#endif // ICON_MANAGER_H
