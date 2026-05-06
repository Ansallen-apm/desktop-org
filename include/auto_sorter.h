#ifndef AUTO_SORTER_H
#define AUTO_SORTER_H

#include <windows.h>
#include <string>
#include <map>
#include "zone_manager.h"
#include "icon_manager.h"

class AutoSorter {
public:
    AutoSorter(ZoneManager& zm, IconManager& im);
    ~AutoSorter();

    // Map a file extension (e.g., ".pdf") to a specific zone index
    void MapExtensionToZone(const std::string& extension, int zoneIndex);

    // Run the sorting algorithm: reads desktop icons and moves them to their mapped zones
    bool RunSort();

    // Check if icons have moved manually into a zone and snap them to grid
    void SyncManualDrags();

    // Clear all existing extension rules
    void ClearRules();

    // Query mapping for testing
    int GetMappedZone(const std::string& extension) const;

    // Helper to extract extension from a filename
    std::string GetExtension(const std::string& filename) const;

private:
    ZoneManager& m_zm;
    IconManager& m_im;
    std::map<std::string, int> m_extensionRules;
};

#endif // AUTO_SORTER_H
