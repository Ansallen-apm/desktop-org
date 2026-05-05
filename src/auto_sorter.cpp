#include "auto_sorter.h"
#include <algorithm>

AutoSorter::AutoSorter(ZoneManager& zm, IconManager& im) : m_zm(zm), m_im(im) {}

AutoSorter::~AutoSorter() {}

void AutoSorter::MapExtensionToZone(const std::string& extension, int zoneIndex) {
    std::string ext = extension;
    // Normalize to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    m_extensionRules[ext] = zoneIndex;
}

std::string AutoSorter::GetExtension(const std::string& filename) const {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    std::string ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

bool AutoSorter::RunSort() {
    int iconCount = m_im.GetIconCount();
    if (iconCount == 0) return false;

    // In a real implementation, we would use VirtualAllocEx and ReadProcessMemory
    // to allocate memory in the Explorer.exe process, send LVM_GETITEMTEXT to 
    // the SysListView32, and read the string back to get the file names.
    // 
    // For this prototype architecture, we mock the detection:
    // We assume we know the mapping. Let's conceptually iterate:
    
    // std::vector<int> zoneCounters(10, 0); // Keep track of how many items in each zone
    
    // for (int i = 0; i < iconCount; ++i) {
    //     std::string filename = m_im.GetIconName(i); // Requires IPC
    //     std::string ext = GetExtension(filename);
    //     
    //     auto it = m_extensionRules.find(ext);
    //     if (it != m_extensionRules.end()) {
    //         int targetZoneIndex = it->second;
    //         // Calculate grid position within the target zone
    //         // RECT zRect = m_zm.GetZoneRect(targetZoneIndex);
    //         // int x = zRect.left + ...
    //         // int y = zRect.top + ...
    //         // m_im.PinIcon(i, x, y);
    //     }
    // }

    return true;
}
