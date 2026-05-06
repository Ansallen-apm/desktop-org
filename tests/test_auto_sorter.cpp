#include <gtest/gtest.h>
#include "auto_sorter.h"
#include "zone_manager.h"
#include "icon_manager.h"

class AutoSorterTest : public ::testing::Test {
protected:
    ZoneManager zm;
    IconManager im;
    AutoSorter* sorter;

    void SetUp() override {
        sorter = new AutoSorter(zm, im);
    }

    void TearDown() override {
        delete sorter;
    }
};

TEST_F(AutoSorterTest, GetExtension_Normal) {
    EXPECT_EQ(sorter->GetExtension("document.pdf"), ".pdf");
    EXPECT_EQ(sorter->GetExtension("archive.tar.gz"), ".gz");
    EXPECT_EQ(sorter->GetExtension(".hiddenfile"), ".hiddenfile");
}

TEST_F(AutoSorterTest, GetExtension_CaseInsensitive) {
    // Note: GetExtension currently just extracts the string. 
    // Case-insensitivity normalization happens in MapExtensionToZone,
    // but the extracted extension should match exactly what is in the string.
    // However, if MapExtensionToZone uses lower case, GetExtension should also return lowercase.
    // Let's verify its behavior. Wait, GetExtension uses ::tolower.
    EXPECT_EQ(sorter->GetExtension("image.JPG"), ".jpg");
    EXPECT_EQ(sorter->GetExtension("Video.Mp4"), ".mp4");
}

TEST_F(AutoSorterTest, GetExtension_NoExt) {
    EXPECT_EQ(sorter->GetExtension("folder"), "");
    EXPECT_EQ(sorter->GetExtension("file_without_extension"), "");
}

TEST_F(AutoSorterTest, MapExtensionToZone) {
    // Initially unmapped
    EXPECT_EQ(sorter->GetMappedZone(".pdf"), -1);

    // Map to zone 0
    sorter->MapExtensionToZone(".pdf", 0);
    EXPECT_EQ(sorter->GetMappedZone(".pdf"), 0);

    // Map uppercase, should normalize to lowercase
    sorter->MapExtensionToZone(".DOCX", 1);
    EXPECT_EQ(sorter->GetMappedZone(".docx"), 1);

    // Re-map overrides existing
    sorter->MapExtensionToZone(".pdf", 2);
    EXPECT_EQ(sorter->GetMappedZone(".pdf"), 2);

    // Clear rules
    sorter->ClearRules();
    EXPECT_EQ(sorter->GetMappedZone(".pdf"), -1);
    EXPECT_EQ(sorter->GetMappedZone(".docx"), -1);
}
