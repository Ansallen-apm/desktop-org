#include "zone_manager.h"
#include <string.h>

ZoneManager::ZoneManager() {}

ZoneManager::~ZoneManager() {}

void ZoneManager::AddZone(const RECT& rect, const char* name, COLORREF color) {
    Zone zone;
    zone.rect = rect;
    zone.color = color;
    strncpy_s(zone.name, sizeof(zone.name), name, _TRUNCATE);
    m_zones.push_back(zone);
}

void ZoneManager::DrawZones(HDC hdc) {
    for (const auto& zone : m_zones) {
        // Create brush for background
        HBRUSH brush = CreateSolidBrush(zone.color);
        
        // Fill zone rect
        FillRect(hdc, &zone.rect, brush);
        
        // Draw border
        FrameRect(hdc, &zone.rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
        
        DeleteObject(brush);

        // Draw title
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        
        RECT textRect = zone.rect;
        textRect.top += 5;
        textRect.bottom = textRect.top + 20;
        DrawTextA(hdc, zone.name, -1, &textRect, DT_CENTER | DT_SINGLELINE);
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
