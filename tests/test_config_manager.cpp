#include <gtest/gtest.h>
#include <windows.h>
#include "config_manager.h"

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a local temp config file instead of the real APPDATA one
        ConfigManager::SetOverrideConfigPath(".\\test_config.ini");
        DeleteFileA(".\\test_config.ini");
    }

    void TearDown() override {
        DeleteFileA(".\\test_config.ini");
        ConfigManager::SetOverrideConfigPath("");
    }
};

TEST_F(ConfigManagerTest, SaveAndLoadZones) {
    std::vector<ZoneConfig> original;
    
    ZoneConfig z1 = {};
    z1.rect = {10, 20, 100, 200};
    z1.color = RGB(12, 34, 56);
    strncpy_s(z1.name, "Test Zone 1", _TRUNCATE);
    strncpy_s(z1.extensions, ".pdf, .docx", _TRUNCATE);
    original.push_back(z1);

    ZoneConfig z2 = {};
    z2.rect = {500, -100, 800, 50}; // Negative coordinates check
    z2.color = RGB(255, 255, 255);
    strncpy_s(z2.name, "Special@#$Chars", _TRUNCATE);
    strncpy_s(z2.extensions, ".exe,.zip,.7z", _TRUNCATE);
    original.push_back(z2);

    // Save
    ConfigManager::SaveZones(original);

    // Load
    std::vector<ZoneConfig> loaded = ConfigManager::LoadZones();
    
    ASSERT_EQ(loaded.size(), 2);
    
    EXPECT_EQ(loaded[0].rect.left, 10);
    EXPECT_EQ(loaded[0].rect.top, 20);
    EXPECT_EQ(loaded[0].rect.right, 100);
    EXPECT_EQ(loaded[0].rect.bottom, 200);
    EXPECT_EQ(loaded[0].color, RGB(12, 34, 56));
    EXPECT_STREQ(loaded[0].name, "Test Zone 1");
    EXPECT_STREQ(loaded[0].extensions, ".pdf, .docx");

    EXPECT_EQ(loaded[1].rect.left, 500);
    EXPECT_EQ(loaded[1].rect.top, -100);
    EXPECT_EQ(loaded[1].rect.right, 800);
    EXPECT_EQ(loaded[1].rect.bottom, 50);
    EXPECT_EQ(loaded[1].color, RGB(255, 255, 255));
    EXPECT_STREQ(loaded[1].name, "Special@#$Chars");
    EXPECT_STREQ(loaded[1].extensions, ".exe,.zip,.7z");
}

TEST_F(ConfigManagerTest, EmptyConfig) {
    // config.ini is deleted in SetUp
    std::vector<ZoneConfig> loaded = ConfigManager::LoadZones();
    
    // Should safely return empty vector
    EXPECT_TRUE(loaded.empty());
}
