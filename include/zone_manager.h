#ifndef ZONE_MANAGER_H
#define ZONE_MANAGER_H

#include <windows.h>
#include <vector>

struct Zone {
    RECT     rect;
    COLORREF color;
    char     name[64];
};

class ZoneManager {
public:
    ZoneManager();
    ~ZoneManager();

    void AddZone(const RECT& rect, const char* name, COLORREF color = RGB(100, 150, 255));
    void DrawZones(HDC hdc, const RECT* pClipRect = nullptr);

    // --- Mutation ---
    void SetZoneColor(int index, COLORREF color);
    void SetZoneName(int index, const char* name);
    void SetZoneRect(int index, const RECT& rect);

    // --- Query ---
    int     HitTest(POINT pt) const;
    int     HitTestEdge(POINT pt, int& outEdgeMask) const; // edges: 1=L,2=T,4=R,8=B
    RECT    GetZoneRect(int index) const;
    COLORREF GetZoneColor(int index) const;
    const char* GetZoneName(int index) const;
    int     GetZoneCount() const { return (int)m_zones.size(); }

private:
    std::vector<Zone>   m_zones;
    std::vector<HBRUSH> m_brushCache;
    HFONT               m_hFont;

    void RebuildBrush(int index);
};

#endif // ZONE_MANAGER_H
