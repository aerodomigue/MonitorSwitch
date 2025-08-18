#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

/**
 * Configuration data structure
 */
struct AppConfig {
    bool startOnBoot;
    std::string selectedDeviceId;
    int screenOffDelay;
    std::vector<std::string> knownDevices;
    
    AppConfig() : startOnBoot(true), screenOffDelay(10) {}
};

/**
 * Service responsible for data persistence and configuration management
 */
class StorageService {
public:
    StorageService();
    ~StorageService();

    /**
     * Set a logging callback function for UI logging
     * @param logCallback function to call for logging messages
     */
    void setLogCallback(std::function<void(const std::string&)> logCallback);

    /**
     * Initialize the storage service and create necessary directories
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * Load configuration from storage
     * @return loaded configuration or default values if not found
     */
    AppConfig loadConfig();

    /**
     * Save configuration to storage
     * @param config configuration to save
     * @return true if successful, false otherwise
     */
    bool saveConfig(const AppConfig& config);

    /**
     * Get the application data directory path
     * @return full path to app data directory
     */
    std::string getAppDataPath();

    /**
     * Save device list to storage
     * @param devices list of device IDs to save
     * @return true if successful, false otherwise
     */
    bool saveDeviceList(const std::vector<std::string>& devices);

    /**
     * Load device list from storage
     * @return list of saved device IDs
     */
    std::vector<std::string> loadDeviceList();

    /**
     * Add a device to the known devices list
     * @param deviceId device ID to add
     * @return true if successful, false otherwise
     */
    bool addKnownDevice(const std::string& deviceId);

    /**
     * Remove a device from the known devices list
     * @param deviceId device ID to remove
     * @return true if successful, false otherwise
     */
    bool removeKnownDevice(const std::string& deviceId);

    /**
     * Check if application data directory exists, create if not
     * @return true if directory exists or was created successfully
     */
    bool ensureAppDataDirectory();

private:
    std::string getLocalAppDataPath();
    std::string getPlatformAppDataPath();  // Cross-platform method
    std::string getConfigFilePath();
    std::string getDeviceListFilePath();
    bool createDirectoryRecursive(const std::string& path);
    bool fileExists(const std::string& filePath);
    
    // Helper method to log both to console and UI
    void log(const std::string& message);
    
    std::string m_appDataPath;
    std::function<void(const std::string&)> m_logCallback;
    static const std::string CONFIG_FILENAME;
    static const std::string DEVICE_LIST_FILENAME;
};

#endif // STORAGE_SERVICE_H
