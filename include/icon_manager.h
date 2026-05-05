#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <windows.h>
#include <commctrl.h>
#include <vector>

class IconManager {
public:
    IconManager();
    ~IconManager();

    // Find the desktop listview (SysListView32)
    bool Initialize();

    // Get number of icons
    int GetIconCount() const;

    // Pin a specific icon index to a coordinate
    bool PinIcon(int index, int x, int y);

private:
    HWND m_hListView;

    HWND FindDesktopListView();
};

#endif // ICON_MANAGER_H
