#include "autostart_service.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef PLATFORM_MACOS
#include <pwd.h>
#include <unistd.h>
#endif

const std::string AutostartService::AUTOSTART_REGISTRY_KEY = ""; // Not used on macOS

AutostartService::AutostartService() 
    : m_applicationName(APP_NAME) {
}

AutostartService::~AutostartService() {
}

bool AutostartService::enableAutostart(const std::string& appPath) {
#ifdef PLATFORM_MACOS
    if (appPath.empty()) {
        return false;
    }
    
    // Create LaunchAgent plist file
    std::string plistPath = getLaunchAgentPath();
    if (plistPath.empty()) {
        return false;
    }
    
    // Create LaunchAgents directory if it doesn't exist
    std::filesystem::path dir = std::filesystem::path(plistPath).parent_path();
    std::filesystem::create_directories(dir);
    
    std::ofstream plistFile(plistPath);
    if (!plistFile.is_open()) {
        return false;
    }
    
    // Write LaunchAgent plist
    plistFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    plistFile << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" ";
    plistFile << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n";
    plistFile << "<plist version=\"1.0\">\n";
    plistFile << "<dict>\n";
    plistFile << "    <key>Label</key>\n";
    plistFile << "    <string>com." << APP_VENDOR << "." << APP_NAME << "</string>\n";
    plistFile << "    <key>ProgramArguments</key>\n";
    plistFile << "    <array>\n";
    plistFile << "        <string>" << appPath << "</string>\n";
    plistFile << "    </array>\n";
    plistFile << "    <key>RunAtLoad</key>\n";
    plistFile << "    <true/>\n";
    plistFile << "    <key>KeepAlive</key>\n";
    plistFile << "    <false/>\n";
    plistFile << "</dict>\n";
    plistFile << "</plist>\n";
    
    plistFile.close();
    
    // Load the launch agent
    std::string command = "launchctl load \"" + plistPath + "\"";
    return system(command.c_str()) == 0;
#else
    std::cout << "Autostart not implemented for this platform" << std::endl;
    return false;
#endif
}

bool AutostartService::disableAutostart() {
#ifdef PLATFORM_MACOS
    std::string plistPath = getLaunchAgentPath();
    if (plistPath.empty()) {
        return false;
    }
    
    // Unload the launch agent
    std::string command = "launchctl unload \"" + plistPath + "\"";
    system(command.c_str()); // Don't check return value as it may fail if not loaded
    
    // Remove the plist file
    return std::filesystem::remove(plistPath);
#else
    return false;
#endif
}

bool AutostartService::isAutostartEnabled() {
#ifdef PLATFORM_MACOS
    std::string plistPath = getLaunchAgentPath();
    return std::filesystem::exists(plistPath);
#else
    return false;
#endif
}

std::string AutostartService::getAutostartRegistryKey() {
    return ""; // Not applicable on macOS
}

void AutostartService::setApplicationName(const std::string& appName) {
    m_applicationName = appName;
}

std::string AutostartService::getLaunchAgentPath() {
#ifdef PLATFORM_MACOS
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        }
    }
    
    if (home) {
        return std::string(home) + "/Library/LaunchAgents/com." + 
               APP_VENDOR + "." + APP_NAME + ".plist";
    }
#endif
    return "";
}

bool AutostartService::setRegistryValue(const std::string& keyPath, const std::string& valueName, const std::string& value) {
    // Not used on macOS
    return false;
}

bool AutostartService::deleteRegistryValue(const std::string& keyPath, const std::string& valueName) {
    // Not used on macOS
    return false;
}

std::string AutostartService::getRegistryValue(const std::string& keyPath, const std::string& valueName) {
    // Not used on macOS
    return "";
}
