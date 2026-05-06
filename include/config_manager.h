#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <windows.h>
#include <string>
#include <vector>

struct ZoneConfig {
    RECT   rect;
    COLORREF color;
    char   name[64];
};

// Fix #12: Manages persistence of zone configurations using a simple INI file
// stored in %APPDATA%\DesktopOrg\config.ini
class ConfigManager {
public:
    static std::string GetConfigPath();
    static void SaveZones(const std::vector<ZoneConfig>& zones);
    static std::vector<ZoneConfig> LoadZones();
};

#endif // CONFIG_MANAGER_H
