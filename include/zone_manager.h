#ifndef ZONE_MANAGER_H
#define ZONE_MANAGER_H

#include <windows.h>
#include <vector>

struct Zone {
    RECT rect;
    COLORREF color;
    char name[64];
};

class ZoneManager {
public:
    ZoneManager();
    ~ZoneManager();

    // Add a new zone
    void AddZone(const RECT& rect, const char* name, COLORREF color = RGB(100, 150, 255));

    // Draw all zones to the given Device Context
    void DrawZones(HDC hdc);

    // Hit test to find which zone a point is in
    int HitTest(POINT pt) const;

private:
    std::vector<Zone> m_zones;
};

#endif // ZONE_MANAGER_H
