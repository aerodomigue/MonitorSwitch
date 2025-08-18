#include <gtest/gtest.h>
#include "core/application.h"

class ApplicationTest : public ::testing::Test {
protected:
    Application* app;

    void SetUp() override {
        app = new Application();
    }

    void TearDown() override {
        delete app;
    }
};

TEST_F(ApplicationTest, TestInitialization) {
    EXPECT_TRUE(app->initialize());
}

TEST_F(ApplicationTest, TestStart) {
    app->initialize();
    EXPECT_NO_THROW(app->start());
}

TEST_F(ApplicationTest, TestPeripheralManagement) {
    app->initialize();
    app->start();
    EXPECT_NO_THROW(app->connectPeripheral("PeripheralID"));
    EXPECT_TRUE(app->isPeripheralConnected("PeripheralID"));
}

TEST_F(ApplicationTest, TestShutdown) {
    app->initialize();
    app->start();
    EXPECT_NO_THROW(app->shutdown());
}