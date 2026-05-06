#include <gtest/gtest.h>
#include "zone_manager.h"

// Test fixture for ZoneManager
class ZoneManagerTest : public ::testing::Test {
protected:
    ZoneManager zm;

    void SetUp() override {
        // Setup a 400x300 zone at (100, 100)
        RECT r1 = {100, 100, 500, 400};
        zm.AddZone(r1, "TestZone1", RGB(255, 0, 0));
        
        // Setup another zone at (600, 100) to (800, 300)
        RECT r2 = {600, 100, 800, 300};
        zm.AddZone(r2, "TestZone2", RGB(0, 255, 0));
    }
};

TEST_F(ZoneManagerTest, AddZoneAndGetters) {
    EXPECT_EQ(zm.GetZoneCount(), 2);
    
    // Check Zone 1 attributes
    EXPECT_STREQ(zm.GetZoneName(0), "TestZone1");
    EXPECT_EQ(zm.GetZoneColor(0), RGB(255, 0, 0));
    
    // Default extensions should be empty
    EXPECT_STREQ(zm.GetZoneExtensions(0), "");
    
    // Set extensions
    zm.SetZoneExtensions(0, ".txt, .md");
    EXPECT_STREQ(zm.GetZoneExtensions(0), ".txt, .md");
}

TEST_F(ZoneManagerTest, HitTest_Inside) {
    // Point inside Zone 1
    POINT p1 = {200, 200};
    EXPECT_EQ(zm.HitTest(p1), 0);
    
    // Point inside Zone 2
    POINT p2 = {700, 200};
    EXPECT_EQ(zm.HitTest(p2), 1);
}

TEST_F(ZoneManagerTest, HitTest_Outside) {
    // Point outside all zones
    POINT p1 = {50, 50};
    EXPECT_EQ(zm.HitTest(p1), -1);
    
    // Point between the two zones
    POINT p2 = {550, 200};
    EXPECT_EQ(zm.HitTest(p2), -1);
}

TEST_F(ZoneManagerTest, HitTestEdge) {
    // Edge width is 8 pixels (defined in HitTestEdge).
    int edgeMask = 0;
    
    // Test Zone 1 Top Edge (x: 200, y: 100..107)
    POINT pTop = {200, 104};
    EXPECT_EQ(zm.HitTestEdge(pTop, edgeMask), 0); // Returns zone index 0
    EXPECT_EQ(edgeMask, 2); // 2 = Top
    
    // Test Zone 1 Left Edge (x: 100..107, y: 200)
    POINT pLeft = {104, 200};
    EXPECT_EQ(zm.HitTestEdge(pLeft, edgeMask), 0);
    EXPECT_EQ(edgeMask, 1); // 1 = Left
    
    // Test Zone 1 Top-Left Corner
    POINT pTopLeft = {104, 104};
    EXPECT_EQ(zm.HitTestEdge(pTopLeft, edgeMask), 0);
    EXPECT_EQ(edgeMask, 1 | 2); // 3 = Top-Left
    
    // Inside the zone, away from edges
    POINT pInner = {200, 200};
    EXPECT_EQ(zm.HitTestEdge(pInner, edgeMask), 0);
    EXPECT_EQ(edgeMask, 0); // 0 = No Edge
}
