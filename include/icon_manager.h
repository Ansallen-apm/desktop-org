#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

class IconManager {
public:
    IconManager();
    ~IconManager();

    // Find the desktop listview (SysListView32)
    bool Initialize();

    // Get number of icons on the desktop
    int GetIconCount() const;

    // Pin a specific icon index to a coordinate
    // Fix #5: Added bounds-checked version with validation
    bool PinIcon(int index, int x, int y);

    // Fix #10: Read the filename of a desktop icon via cross-process memory access
    std::string GetIconName(int index) const;

private:
    HWND m_hListView;
    DWORD m_explorerPid; // Cache the Explorer PID for IPC

    HWND FindDesktopListView();
};

#endif // ICON_MANAGER_H
