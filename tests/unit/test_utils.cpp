#include <gtest/gtest.h>
#include "utils.h"

// Test case for utility functions
TEST(UtilsTest, SaveLoadPeripheralID) {
    // Arrange
    const std::string testID = "test_peripheral_id";
    
    // Act
    savePeripheralID(testID);
    std::string loadedID = loadPeripheralID();
    
    // Assert
    EXPECT_EQ(testID, loadedID);
}

TEST(UtilsTest, InvalidPeripheralID) {
    // Arrange
    const std::string invalidID = "invalid_id";
    
    // Act
    savePeripheralID(invalidID);
    std::string loadedID = loadPeripheralID();
    
    // Assert
    EXPECT_NE("valid_id", loadedID);
}

// Add more tests as needed for other utility functions
