#include "zone_manager.h"
#include <string.h>

ZoneManager::ZoneManager() {
    // Create a large, readable font (24pt Bold, Segoe UI)
    m_hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                          OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                          VARIABLE_PITCH, "Segoe UI");
}

ZoneManager::~ZoneManager() {
    // Fix #8: Clean up all cached GDI brushes on destruction
    for (HBRUSH brush : m_brushCache) {
        if (brush) DeleteObject(brush);
    }
    if (m_hFont) DeleteObject(m_hFont);
}

void ZoneManager::AddZone(const RECT& rect, const char* name, COLORREF color) {
    Zone zone;
    zone.rect = rect;
    zone.color = color;
    strncpy_s(zone.name, sizeof(zone.name), name, _TRUNCATE);
    m_zones.push_back(zone);

    // Fix #8: Pre-create and cache a GDI brush for this zone immediately
    m_brushCache.push_back(CreateSolidBrush(color));
}

void ZoneManager::DrawZones(HDC hdc, const RECT* pClipRect) {
    for (size_t i = 0; i < m_zones.size(); ++i) {
        const Zone& zone = m_zones[i];

        // Fix #9: Skip zones that don't intersect with the dirty/paint region
        if (pClipRect != nullptr) {
            RECT intersection;
            if (!IntersectRect(&intersection, &zone.rect, pClipRect)) {
                continue; // This zone is not in the repaint area, skip it
            }
        }

        // Fix #8: Use the pre-cached brush — zero GDI allocations per frame
        HBRUSH brush = m_brushCache[i];

        // Fill zone background
        FillRect(hdc, &zone.rect, brush);

        // Draw border (1-pixel white frame)
        FrameRect(hdc, &zone.rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // Draw zone title text
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));

        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);

        RECT textRect = zone.rect;
        textRect.top += 8;
        textRect.bottom = textRect.top + 30;
        DrawTextA(hdc, zone.name, -1, &textRect, DT_CENTER | DT_SINGLELINE);

        SelectObject(hdc, hOldFont);
    }
}

int ZoneManager::HitTest(POINT pt) const {
    for (size_t i = 0; i < m_zones.size(); ++i) {
        if (PtInRect(&m_zones[i].rect, pt)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

RECT ZoneManager::GetZoneRect(int index) const {
    if (index < 0 || index >= (int)m_zones.size()) {
        RECT empty = {};
        return empty;
    }
    return m_zones[index].rect;
}
