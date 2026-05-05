cmake_minimum_required(VERSION 3.15)
project(DesktopOrg VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set Windows subsystem
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/desktop_hooker.cpp
    src/zone_manager.cpp
)

# Executable
add_executable(DesktopOrg WIN32 ${SOURCES})

# Link Windows libraries
target_link_libraries(DesktopOrg PRIVATE user32 gdi32 shell32)
