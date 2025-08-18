#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// Configuration constants and settings for the application

// Option to start the application on boot
const bool START_ON_BOOT = true;

// Application information
const std::string APP_NAME = "MonitorSwitch";
const std::string APP_VENDOR = "aerodomigue";

// Path for saving data (platform-specific resolution at runtime)
// Windows: %LOCALAPPDATA%/aerodomigue/MonitorSwitch/
// macOS: ~/Library/Application Support/aerodomigue/MonitorSwitch/
// Linux: ~/.config/aerodomigue/MonitorSwitch/
const std::string DATA_SAVE_PATH = "aerodomigue/MonitorSwitch";

// Screen control settings
const int SCREEN_OFF_DELAY_SECONDS = 10;

// Platform detection
#ifdef _WIN32
    #define PLATFORM_WINDOWS
#elif __APPLE__
    #define PLATFORM_MACOS
    #include <TargetConditionals.h>
#elif __linux__
    #define PLATFORM_LINUX
#else
    #define PLATFORM_UNKNOWN
#endif

// Other configuration options can be added here

#endif // CONFIG_H