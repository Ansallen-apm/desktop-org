#ifndef DESKTOP_HOOKER_H
#define DESKTOP_HOOKER_H

#include <windows.h>

class DesktopHooker {
public:
    DesktopHooker();
    ~DesktopHooker();

    // Initialize the hook by spawning a WorkerW behind the desktop icons
    bool Initialize();

    // Get the handle to the WorkerW window where we can draw
    HWND GetDesktopWorkerWindow() const;

private:
    HWND m_workerW;
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
};

#endif // DESKTOP_HOOKER_H
