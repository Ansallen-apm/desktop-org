cmake_minimum_required(VERSION 3.15)
project(DesktopOrg VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Windows subsystem (no console)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# Include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/desktop_hooker.cpp
    src/zone_manager.cpp
    src/icon_manager.cpp
    src/auto_sorter.cpp
    src/config_manager.cpp
    src/dialog_helper.cpp
)

# Executable
add_executable(DesktopOrg WIN32 ${SOURCES})

# Link Windows libraries
# comdlg32 = ChooseColor dialog
# shell32  = Shell_NotifyIcon + SHGetFolderPath
# comctl32 = Common controls (ListView messages)
target_link_libraries(DesktopOrg PRIVATE user32 gdi32 shell32 comctl32 comdlg32)
