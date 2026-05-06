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

    // Fix #9: Pass clip rect to avoid redrawing invisible zones
    void DrawZones(HDC hdc, const RECT* pClipRect = nullptr);

    // Hit test to find which zone a point is in
    int HitTest(POINT pt) const;

    // Fix #10: Required by AutoSorter to calculate icon grid positions
    RECT GetZoneRect(int index) const;

    int GetZoneCount() const { return (int)m_zones.size(); }

private:
    std::vector<Zone> m_zones;
    // Fix #8: Cache GDI brushes per zone to avoid creating/deleting every frame
    std::vector<HBRUSH> m_brushCache;
    HFONT m_hFont;
};

#endif // ZONE_MANAGER_H
