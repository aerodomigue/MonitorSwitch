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

// Note: Platform detection macros (PLATFORM_WINDOWS, PLATFORM_MACOS, PLATFORM_LINUX) 
// are now defined in CMakeLists.txt to avoid redefinition warnings

// Other configuration options can be added here

#endif // CONFIG_H