#include "storage_service.h"
#include "config.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <sstream>

#ifdef PLATFORM_MACOS
    #include <pwd.h>
    #include <unistd.h>
#elif defined(PLATFORM_LINUX)
    #include <pwd.h>
    #include <unistd.h>
#endif

const std::string StorageService::CONFIG_FILENAME = "config.ini";
const std::string StorageService::DEVICE_LIST_FILENAME = "devices.txt";

StorageService::StorageService() {
}

StorageService::~StorageService() {
}

void StorageService::setLogCallback(std::function<void(const std::string&)> logCallback) {
    m_logCallback = logCallback;
}

void StorageService::log(const std::string& message) {
    // Always log to console
    std::cout << "[CONFIG] " << message << std::endl;
    
    // Also log to UI if callback is set
    if (m_logCallback) {
        m_logCallback(message);
    }
}

bool StorageService::initialize() {
    log("Initializing storage service...");
    
    m_appDataPath = getAppDataPath();
    log("App data path: " + m_appDataPath);
    
    bool success = ensureAppDataDirectory();
    if (success) {
        log("Storage service initialized successfully");
    } else {
        log("Failed to initialize storage service");
    }
    
    return success;
}

AppConfig StorageService::loadConfig() {
    log("Loading configuration file...");
    AppConfig config;
    
    std::string configPath = getConfigFilePath();
    log("Loading configuration from: " + configPath);
    
    if (!fileExists(configPath)) {
        log("Configuration file does not exist, using default values");
        return config;
    }
    
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            log("Failed to open configuration file for reading");
            return config;
        }
        
        log("Successfully opened configuration file");
        
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) {
            lineNumber++;
            if (line.empty() || line[0] == '#') continue;
            
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                log("Line " + std::to_string(lineNumber) + ": " + key + " = " + value);
                
                if (key == "startOnBoot") {
                    config.startOnBoot = (value == "true" || value == "1");
                    log("Set startOnBoot to: " + std::string(config.startOnBoot ? "true" : "false"));
                } else if (key == "selectedDeviceId") {
                    config.selectedDeviceId = value;
                    log("Set selectedDeviceId to: " + config.selectedDeviceId);
                } else if (key == "screenOffDelay") {
                    config.screenOffDelay = std::stoi(value);
                    log("Set screenOffDelay to: " + std::to_string(config.screenOffDelay) + " seconds");
                }
            }
        }
        
        log("Loading device list...");
        config.knownDevices = loadDeviceList();
        log("Loaded " + std::to_string(config.knownDevices.size()) + " known devices");
        
    } catch (const std::exception& e) {
        log("Error loading config: " + std::string(e.what()));
    }
    
    log("Configuration loading completed");
    return config;
}

bool StorageService::saveConfig(const AppConfig& config) {
    log("Saving configuration file...");
    std::string configPath = getConfigFilePath();
    log("Saving configuration to: " + configPath);
    
    try {
        std::ofstream file(configPath);
        if (!file.is_open()) {
            log("Failed to open configuration file for writing");
            return false;
        }
        
        log("Writing configuration values...");
        
        file << "# MonitorSwitch Configuration File\n";
        file << "# Generated automatically - do not edit manually\n\n";
        
        file << "startOnBoot=" << (config.startOnBoot ? "true" : "false") << "\n";
        log("Written startOnBoot: " + std::string(config.startOnBoot ? "true" : "false"));
        
        file << "selectedDeviceId=" << config.selectedDeviceId << "\n";
        log("Written selectedDeviceId: " + config.selectedDeviceId);
        
        file << "screenOffDelay=" << config.screenOffDelay << "\n";
        log("Written screenOffDelay: " + std::to_string(config.screenOffDelay));
        
        file.close();
        
        log("Saving device list...");
        bool deviceListSaved = saveDeviceList(config.knownDevices);
        
        if (deviceListSaved) {
            log("Configuration saved successfully");
        } else {
            log("Failed to save device list");
        }
        
        return deviceListSaved;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

std::string StorageService::getAppDataPath() {
    if (!m_appDataPath.empty()) {
        return m_appDataPath;
    }
    
    std::string appDataDir = getPlatformAppDataPath();
    if (appDataDir.empty()) {
        return "";
    }
    
    m_appDataPath = appDataDir + "/" + DATA_SAVE_PATH;
    return m_appDataPath;
}

bool StorageService::saveDeviceList(const std::vector<std::string>& devices) {
    try {
        std::ofstream file(getDeviceListFilePath());
        if (!file.is_open()) {
            return false;
        }
        
        file << "# Known USB Devices\n";
        for (const auto& device : devices) {
            file << device << "\n";
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving device list: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> StorageService::loadDeviceList() {
    std::vector<std::string> devices;
    
    std::string deviceListPath = getDeviceListFilePath();
    log("Loading device list from: " + deviceListPath);
    
    if (!fileExists(deviceListPath)) {
        log("Device list file does not exist");
        return devices;
    }
    
    try {
        std::ifstream file(deviceListPath);
        if (!file.is_open()) {
            log("Failed to open device list file for reading");
            return devices;
        }
        
        log("Successfully opened device list file");
        
        std::string line;
        int deviceCount = 0;
        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '#') {
                devices.push_back(line);
                deviceCount++;
                log("Device " + std::to_string(deviceCount) + ": " + line);
            }
        }
        
        log("Total devices loaded: " + std::to_string(devices.size()));
        
    } catch (const std::exception& e) {
        log("Error loading device list: " + std::string(e.what()));
    }
    
    return devices;
}

bool StorageService::addKnownDevice(const std::string& deviceId) {
    auto devices = loadDeviceList();
    
    if (std::find(devices.begin(), devices.end(), deviceId) != devices.end()) {
        return true; // Already exists
    }
    
    devices.push_back(deviceId);
    return saveDeviceList(devices);
}

bool StorageService::removeKnownDevice(const std::string& deviceId) {
    auto devices = loadDeviceList();
    
    auto it = std::find(devices.begin(), devices.end(), deviceId);
    if (it != devices.end()) {
        devices.erase(it);
        return saveDeviceList(devices);
    }
    
    return true;
}

bool StorageService::ensureAppDataDirectory() {
    std::string appDataPath = getAppDataPath();
    if (appDataPath.empty()) {
        return false;
    }
    
    return createDirectoryRecursive(appDataPath);
}

std::string StorageService::getPlatformAppDataPath() {
#ifdef PLATFORM_MACOS
    // macOS: ~/Library/Application Support/
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        }
    }
    if (home) {
        return std::string(home) + "/Library/Application Support";
    }
    
#elif defined(PLATFORM_LINUX)
    // Linux: ~/.config/
    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if (xdgConfig) {
        return std::string(xdgConfig);
    }
    
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            home = pw->pw_dir;
        }
    }
    if (home) {
        return std::string(home) + "/.config";
    }
    
#else
    // Fallback for other Unix systems
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/.config";
    }
#endif
    
    return "";
}

std::string StorageService::getConfigFilePath() {
    return getAppDataPath() + "/" + CONFIG_FILENAME;
}

std::string StorageService::getDeviceListFilePath() {
    return getAppDataPath() + "/" + DEVICE_LIST_FILENAME;
}

bool StorageService::createDirectoryRecursive(const std::string& path) {
    try {
        std::filesystem::create_directories(path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }
}

bool StorageService::fileExists(const std::string& filePath) {
    return std::filesystem::exists(filePath);
}
