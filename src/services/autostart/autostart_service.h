#ifndef AUTOSTART_SERVICE_H
#define AUTOSTART_SERVICE_H

#include <string>

/**
 * Service responsible for managing application autostart functionality
 */
class AutostartService {
public:
    AutostartService();
    ~AutostartService();

    /**
     * Enable application to start on boot
     * @param appPath full path to the application executable
     * @return true if successful, false otherwise
     */
    bool enableAutostart(const std::string& appPath);

    /**
     * Disable application autostart
     * @return true if successful, false otherwise
     */
    bool disableAutostart();

    /**
     * Check if autostart is currently enabled
     * @return true if enabled, false otherwise
     */
    bool isAutostartEnabled();

    /**
     * Get the registry key path for autostart entries
     * @return registry key path
     */
    std::string getAutostartRegistryKey();

    /**
     * Set the application name for registry entries
     * @param appName name to use in registry
     */
    void setApplicationName(const std::string& appName);

private:
    bool setRegistryValue(const std::string& keyPath, const std::string& valueName, const std::string& value);
    bool deleteRegistryValue(const std::string& keyPath, const std::string& valueName);
    std::string getRegistryValue(const std::string& keyPath, const std::string& valueName);

    std::string getAutostartDesktopPath();
    std::string getLaunchAgentPath();

    std::string m_applicationName;
    static const std::string AUTOSTART_REGISTRY_KEY;
};

#endif // AUTOSTART_SERVICE_H
