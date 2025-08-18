#include "autostart_service.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef PLATFORM_LINUX
#include <pwd.h>
#include <unistd.h>
#endif

const std::string AutostartService::AUTOSTART_REGISTRY_KEY = ""; // Not used on Linux

AutostartService::AutostartService() 
    : m_applicationName(APP_NAME) {
}

AutostartService::~AutostartService() {
}

bool AutostartService::enableAutostart(const std::string& appPath) {
#ifdef PLATFORM_LINUX
    if (appPath.empty()) {
        return false;
    }
    
    // Create .desktop file for autostart
    std::string desktopFilePath = getAutostartDesktopPath();
    if (desktopFilePath.empty()) {
        return false;
    }
    
    // Create autostart directory if it doesn't exist
    std::filesystem::path dir = std::filesystem::path(desktopFilePath).parent_path();
    std::filesystem::create_directories(dir);
    
    std::ofstream desktopFile(desktopFilePath);
    if (!desktopFile.is_open()) {
        return false;
    }
    
    // Write .desktop file
    desktopFile << "[Desktop Entry]\n";
    desktopFile << "Type=Application\n";
    desktopFile << "Name=" << APP_NAME << "\n";
    desktopFile << "Comment=USB Device Monitor & Screen Controller\n";
    desktopFile << "Exec=" << appPath << "\n";
    desktopFile << "Terminal=false\n";
    desktopFile << "Hidden=false\n";
    desktopFile << "X-GNOME-Autostart-enabled=true\n";
    desktopFile << "StartupNotify=false\n";
    
    desktopFile.close();
    
    // Make the file executable
    std::filesystem::permissions(desktopFilePath, 
                                std::filesystem::perms::owner_read | 
                                std::filesystem::perms::owner_write | 
                                std::filesystem::perms::owner_exec);
    
    return true;
#else
    std::cout << "Autostart not implemented for this platform" << std::endl;
    return false;
#endif
}

bool AutostartService::disableAutostart() {
#ifdef PLATFORM_LINUX
    std::string desktopFilePath = getAutostartDesktopPath();
    if (desktopFilePath.empty()) {
        return false;
    }
    
    // Remove the .desktop file
    return std::filesystem::remove(desktopFilePath);
#else
    return false;
#endif
}

bool AutostartService::isAutostartEnabled() {
#ifdef PLATFORM_LINUX
    std::string desktopFilePath = getAutostartDesktopPath();
    return std::filesystem::exists(desktopFilePath);
#else
    return false;
#endif
}

std::string AutostartService::getAutostartRegistryKey() {
    return ""; // Not applicable on Linux
}

void AutostartService::setApplicationName(const std::string& appName) {
    m_applicationName = appName;
}

std::string AutostartService::getAutostartDesktopPath() {
#ifdef PLATFORM_LINUX
    // Get XDG config home or fallback to ~/.config
    const char* xdgConfigHome = getenv("XDG_CONFIG_HOME");
    std::string configDir;
    
    if (xdgConfigHome) {
        configDir = std::string(xdgConfigHome);
    } else {
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                home = pw->pw_dir;
            }
        }
        
        if (home) {
            configDir = std::string(home) + "/.config";
        }
    }
    
    if (!configDir.empty()) {
        return configDir + "/autostart/" + APP_NAME + ".desktop";
    }
#endif
    return "";
}

bool AutostartService::setRegistryValue(const std::string& keyPath, const std::string& valueName, const std::string& value) {
    // Not used on Linux
    return false;
}

bool AutostartService::deleteRegistryValue(const std::string& keyPath, const std::string& valueName) {
    // Not used on Linux
    return false;
}

std::string AutostartService::getRegistryValue(const std::string& keyPath, const std::string& valueName) {
    // Not used on Linux
    return "";
}
