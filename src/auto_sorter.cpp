#include "auto_sorter.h"
#include <algorithm>

// Grid layout constants for icon positioning within a zone
static const int ICON_GRID_WIDTH  = 80;  // Horizontal spacing between icons
static const int ICON_GRID_HEIGHT = 80;  // Vertical spacing between icons
static const int ICON_PADDING     = 20;  // Padding from zone left edge
static const int TITLE_HEIGHT     = 40;  // Reserve space for zone title at top

AutoSorter::AutoSorter(ZoneManager& zm, IconManager& im) : m_zm(zm), m_im(im) {}

AutoSorter::~AutoSorter() {}

void AutoSorter::MapExtensionToZone(const std::string& extension, int zoneIndex) {
    std::string ext = extension;
    // Normalize to lowercase for case-insensitive matching
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    m_extensionRules[ext] = zoneIndex;
}

void AutoSorter::ClearRules() {
    m_extensionRules.clear();
}

int AutoSorter::GetMappedZone(const std::string& extension) const {
    auto it = m_extensionRules.find(extension);
    return (it != m_extensionRules.end()) ? it->second : -1;
}

std::string AutoSorter::GetExtension(const std::string& filename) const {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    std::string ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

// Fix #10: Full implementation using VirtualAllocEx IPC via IconManager::GetIconName
bool AutoSorter::RunSort() {
    int iconCount = m_im.GetIconCount();
    if (iconCount == 0) return false;

    int zoneCount = m_zm.GetZoneCount();
    // Track how many icons have been placed in each zone (for grid layout)
    std::vector<int> zoneItemCounters(zoneCount, 0);

    for (int i = 0; i < iconCount; ++i) {
        // Read the icon label via cross-process memory read from Explorer.exe
        std::string filename = m_im.GetIconName(i);
        if (filename.empty()) continue;

        std::string ext = GetExtension(filename);
        if (ext.empty()) continue;

        auto it = m_extensionRules.find(ext);
        if (it == m_extensionRules.end()) continue;

        int targetZoneIndex = it->second;
        if (targetZoneIndex < 0 || targetZoneIndex >= zoneCount) continue;

        RECT zRect = m_zm.GetZoneRect(targetZoneIndex);
        int zoneWidth = zRect.right - zRect.left;
        if (zoneWidth <= 0) continue;

        // Calculate how many icon columns fit within the zone
        int cols = (zoneWidth - ICON_PADDING) / ICON_GRID_WIDTH;
        if (cols < 1) cols = 1;

        int itemsPlaced = zoneItemCounters[targetZoneIndex];
        int col = itemsPlaced % cols;
        int row = itemsPlaced / cols;

        int x = zRect.left + ICON_PADDING + col * ICON_GRID_WIDTH;
        int y = zRect.top + TITLE_HEIGHT + row * ICON_GRID_HEIGHT;

        m_im.PinIcon(i, x, y);
        zoneItemCounters[targetZoneIndex]++;
    }

    return true;
}

void AutoSorter::SyncManualDrags() {
    static std::map<std::string, POINT> lastPositions;
    int iconCount = m_im.GetIconCount();
    bool ruleChanged = false;

    for (int i = 0; i < iconCount; ++i) {
        POINT pt = m_im.GetIconPosition(i);
        if (pt.x == -1 && pt.y == -1) continue;

        std::string filename = m_im.GetIconName(i);
        if (filename.empty()) continue;

        auto it = lastPositions.find(filename);
        if (it != lastPositions.end()) {
            if (it->second.x != pt.x || it->second.y != pt.y) {
                // Icon moved manually!
                int zoneIdx = m_zm.HitTest(pt);
                if (zoneIdx != -1) {
                    std::string ext = GetExtension(filename);
                    if (!ext.empty()) {
                        auto ruleIt = m_extensionRules.find(ext);
                        if (ruleIt == m_extensionRules.end() || ruleIt->second != zoneIdx) {
                            m_extensionRules[ext] = zoneIdx;
                            ruleChanged = true;
                            
                            // Update ZoneManager's extension string
                            std::string currentExts = m_zm.GetZoneExtensions(zoneIdx);
                            if (currentExts.find(ext) == std::string::npos) {
                                if (!currentExts.empty()) currentExts += ", ";
                                currentExts += ext;
                                m_zm.SetZoneExtensions(zoneIdx, currentExts.c_str());
                            }
                        }
                    }
                }
            }
        }
        lastPositions[filename] = pt;
    }

    if (ruleChanged) {
        RunSort();
    }
}
