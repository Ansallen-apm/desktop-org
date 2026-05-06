#include "zone_manager.h"
#include <string.h>

static const int EDGE_HIT = 8; // pixels from edge to trigger resize cursor

ZoneManager::ZoneManager() {
    m_hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
}

ZoneManager::~ZoneManager() {
    for (HBRUSH b : m_brushCache) if (b) DeleteObject(b);
    if (m_hFont) DeleteObject(m_hFont);
}

void ZoneManager::RebuildBrush(int index) {
    if (m_brushCache[index]) DeleteObject(m_brushCache[index]);
    m_brushCache[index] = CreateSolidBrush(m_zones[index].color);
}

void ZoneManager::AddZone(const RECT& rect, const char* name, COLORREF color) {
    Zone z;
    z.rect  = rect;
    z.color = color;
    strncpy_s(z.name, sizeof(z.name), name, _TRUNCATE);
    m_zones.push_back(z);
    m_brushCache.push_back(CreateSolidBrush(color));
}

void ZoneManager::SetZoneColor(int index, COLORREF color) {
    if (index < 0 || index >= (int)m_zones.size()) return;
    m_zones[index].color = color;
    RebuildBrush(index);
}

void ZoneManager::SetZoneName(int index, const char* name) {
    if (index < 0 || index >= (int)m_zones.size()) return;
    strncpy_s(m_zones[index].name, sizeof(m_zones[index].name), name, _TRUNCATE);
}

void ZoneManager::SetZoneRect(int index, const RECT& rect) {
    if (index < 0 || index >= (int)m_zones.size()) return;
    m_zones[index].rect = rect;
}

void ZoneManager::DrawZones(HDC hdc, const RECT* pClipRect) {
    for (size_t i = 0; i < m_zones.size(); ++i) {
        const Zone& z = m_zones[i];
        if (pClipRect) {
            RECT tmp;
            if (!IntersectRect(&tmp, &z.rect, pClipRect)) continue;
        }
        FillRect(hdc, &z.rect, m_brushCache[i]);
        FrameRect(hdc, &z.rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        HFONT hOld = (HFONT)SelectObject(hdc, m_hFont);
        RECT tr = z.rect;
        tr.top += 8; tr.bottom = tr.top + 30;
        DrawTextA(hdc, z.name, -1, &tr, DT_CENTER | DT_SINGLELINE);
        SelectObject(hdc, hOld);
    }
}

int ZoneManager::HitTest(POINT pt) const {
    for (size_t i = 0; i < m_zones.size(); ++i)
        if (PtInRect(&m_zones[i].rect, pt)) return (int)i;
    return -1;
}

int ZoneManager::HitTestEdge(POINT pt, int& outEdgeMask) const {
    outEdgeMask = 0;
    for (size_t i = 0; i < m_zones.size(); ++i) {
        const RECT& r = m_zones[i].rect;
        if (!PtInRect(&r, pt)) continue;
        if (pt.x - r.left   < EDGE_HIT) outEdgeMask |= 1;
        if (pt.y - r.top    < EDGE_HIT) outEdgeMask |= 2;
        if (r.right  - pt.x < EDGE_HIT) outEdgeMask |= 4;
        if (r.bottom - pt.y < EDGE_HIT) outEdgeMask |= 8;
        return (int)i;
    }
    return -1;
}

RECT ZoneManager::GetZoneRect(int index) const {
    if (index < 0 || index >= (int)m_zones.size()) { RECT e = {}; return e; }
    return m_zones[index].rect;
}

COLORREF ZoneManager::GetZoneColor(int index) const {
    if (index < 0 || index >= (int)m_zones.size()) return RGB(100, 150, 255);
    return m_zones[index].color;
}

const char* ZoneManager::GetZoneName(int index) const {
    if (index < 0 || index >= (int)m_zones.size()) return "";
    return m_zones[index].name;
}
