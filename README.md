# DesktopOrg

DesktopOrg is a lightweight, high-performance Windows desktop icon organizer written in C++ and Win32 API. It creates a transparent overlay behind your desktop icons to draw customizable zones (fences) and automatically sorts your files based on their extensions.

## Features
- **Zone Drawing**: Draw custom zones directly on the desktop.
- **Icon Hooking**: Hooks into `SysListView32` to pin icons into zones.
- **Auto-Sort**: Automatically maps extensions (e.g. `.pdf`, `.jpg`) to specific zones.
- **Keyboard Shortcuts**: Press `Ctrl + Shift + D` or double-click the desktop to hide/show zones.
- **Visual Customization**: Right-click on a zone to change its color or title.

## Build Requirements
- CMake 3.15+
- MSVC (Visual Studio Build Tools)
- Windows SDK

## How to Build

Run the provided PowerShell build script:

```powershell
.\build.ps1
```

Or manually use CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Running
After building, execute `DesktopOrg.exe` from the `build/Release` directory.
