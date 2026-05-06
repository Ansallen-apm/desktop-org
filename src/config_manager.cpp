#include "config_manager.h"
#include <shlobj.h>    // SHGetFolderPathA
#include <sstream>
#include <string.h>

std::string ConfigManager::GetConfigPath() {
    char appData[MAX_PATH] = {};
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appData);
    std::string dir = std::string(appData) + "\\DesktopOrg";
    CreateDirectoryA(dir.c_str(), NULL); // Create dir if it doesn't exist
    return dir + "\\config.ini";
}

void ConfigManager::SaveZones(const std::vector<ZoneConfig>& zones) {
    std::string path = GetConfigPath();
    const char* p = path.c_str();

    char buf[32];
    sprintf_s(buf, "%d", (int)zones.size());
    WritePrivateProfileStringA("Zones", "Count", buf, p);

    for (int i = 0; i < (int)zones.size(); ++i) {
        std::string section = "Zone" + std::to_string(i);
        const char* sec = section.c_str();

        WritePrivateProfileStringA(sec, "Name",   zones[i].name, p);
        sprintf_s(buf, "%ld", zones[i].rect.left);
        WritePrivateProfileStringA(sec, "Left",   buf, p);
        sprintf_s(buf, "%ld", zones[i].rect.top);
        WritePrivateProfileStringA(sec, "Top",    buf, p);
        sprintf_s(buf, "%ld", zones[i].rect.right);
        WritePrivateProfileStringA(sec, "Right",  buf, p);
        sprintf_s(buf, "%ld", zones[i].rect.bottom);
        WritePrivateProfileStringA(sec, "Bottom", buf, p);
        sprintf_s(buf, "%lu", zones[i].color);
        WritePrivateProfileStringA(sec, "Color",  buf, p);
    }
}

std::vector<ZoneConfig> ConfigManager::LoadZones() {
    std::vector<ZoneConfig> zones;
    std::string path = GetConfigPath();
    const char* p = path.c_str();

    int count = GetPrivateProfileIntA("Zones", "Count", 0, p);
    for (int i = 0; i < count; ++i) {
        std::string section = "Zone" + std::to_string(i);
        const char* sec = section.c_str();

        ZoneConfig z = {};
        GetPrivateProfileStringA(sec, "Name", "Zone", z.name, sizeof(z.name), p);
        z.rect.left   = GetPrivateProfileIntA(sec, "Left",   0,   p);
        z.rect.top    = GetPrivateProfileIntA(sec, "Top",    0,   p);
        z.rect.right  = GetPrivateProfileIntA(sec, "Right",  300, p);
        z.rect.bottom = GetPrivateProfileIntA(sec, "Bottom", 200, p);
        z.color       = (COLORREF)GetPrivateProfileIntA(sec, "Color", RGB(100,150,255), p);
        zones.push_back(z);
    }
    return zones;
}
