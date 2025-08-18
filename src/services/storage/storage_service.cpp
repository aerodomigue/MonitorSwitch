#include "storage_service.h"
#include "config.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

const std::string StorageService::CONFIG_FILENAME = "config.ini";
const std::string StorageService::DEVICE_LIST_FILENAME = "devices.txt";

StorageService::StorageService() {
}

StorageService::~StorageService() {
}

void StorageService::setLogCallback(std::function<void(const std::string&)> logCallback) {
    m_logCallback = logCallback;
}

bool StorageService::initialize() {
    m_appDataPath = getAppDataPath();
    return ensureAppDataDirectory();
}

AppConfig StorageService::loadConfig() {
    AppConfig config;
    
    std::string configPath = getConfigFilePath();
    if (!fileExists(configPath)) {
        // Return default configuration if file doesn't exist
        return config;
    }
    
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            return config;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "startOnBoot") {
                    config.startOnBoot = (value == "true" || value == "1");
                } else if (key == "startMinimized") {
                    config.startMinimized = (value == "true" || value == "1");
                } else if (key == "selectedDeviceId") {
                    config.selectedDeviceId = value;
                } else if (key == "screenOffDelay") {
                    config.screenOffDelay = std::stoi(value);
                }
            }
        }
        
        // Load known devices
        config.knownDevices = loadDeviceList();
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        // Return default config on error
    }
    
    return config;
}

bool StorageService::saveConfig(const AppConfig& config) {
    try {
        std::ofstream file(getConfigFilePath());
        if (!file.is_open()) {
            return false;
        }
        
        file << "# MonitorSwitch Configuration File\n";
        file << "# Generated automatically - do not edit manually\n\n";
        
        file << "startOnBoot=" << (config.startOnBoot ? "true" : "false") << "\n";
        file << "startMinimized=" << (config.startMinimized ? "true" : "false") << "\n";
        file << "selectedDeviceId=" << config.selectedDeviceId << "\n";
        file << "screenOffDelay=" << config.screenOffDelay << "\n";
        
        file.close();
        
        // Save device list separately
        return saveDeviceList(config.knownDevices);
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

std::string StorageService::getAppDataPath() {
    if (!m_appDataPath.empty()) {
        return m_appDataPath;
    }
    
    std::string localAppData = getLocalAppDataPath();
    if (localAppData.empty()) {
        return "";
    }
    
    m_appDataPath = localAppData + "\\" + DATA_SAVE_PATH;
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
    if (!fileExists(deviceListPath)) {
        return devices;
    }
    
    try {
        std::ifstream file(deviceListPath);
        if (!file.is_open()) {
            return devices;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '#') {
                devices.push_back(line);
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading device list: " << e.what() << std::endl;
    }
    
    return devices;
}

bool StorageService::addKnownDevice(const std::string& deviceId) {
    auto devices = loadDeviceList();
    
    // Check if device already exists
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
    
    return true; // Device wasn't in list anyway
}

bool StorageService::ensureAppDataDirectory() {
    std::string appDataPath = getAppDataPath();
    if (appDataPath.empty()) {
        return false;
    }
    
    return createDirectoryRecursive(appDataPath);
}

std::string StorageService::getLocalAppDataPath() {
    wchar_t* localAppDataPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppDataPath);
    
    if (SUCCEEDED(hr) && localAppDataPath) {
        std::wstring wPath(localAppDataPath);
        // Proper Unicode to UTF-8 conversion
        std::string path;
        int len = WideCharToMultiByte(CP_UTF8, 0, wPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (len > 0) {
            path.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, wPath.c_str(), -1, &path[0], len, nullptr, nullptr);
        }
        CoTaskMemFree(localAppDataPath);
        return path;
    }
    
    return "";
}

std::string StorageService::getConfigFilePath() {
    return getAppDataPath() + "\\" + CONFIG_FILENAME;
}

std::string StorageService::getDeviceListFilePath() {
    return getAppDataPath() + "\\" + DEVICE_LIST_FILENAME;
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

void StorageService::log(const std::string& message) {
    // Always log to console
    std::cout << "[CONFIG] " << message << std::endl;
    
    // Also log to UI if callback is set
    if (m_logCallback) {
        m_logCallback(message);
    }
}
