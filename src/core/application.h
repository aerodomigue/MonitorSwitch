#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <string>
#include <functional>
#include "../services/display/display_service.h"
#include "../services/usb/usb_service.h"
#include "../services/storage/storage_service.h"
#include "../services/autostart/autostart_service.h"

/**
 * Main application controller that coordinates all services
 */
class Application {
public:
    /**
     * Get all currently connected USB devices (for UI)
     */
    std::vector<UsbDevice> getConnectedUsbDevices() const;
public:
    Application();
    ~Application();

    /**
     * Initialize the application and all services
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * Load application configuration from storage
     * Call this after initialize() and UI setup is complete
     */
    void startConfiguration();

    /**
     * Set up logging callback for UI integration
     * @param logCallback function to call for logging messages
     */
    void setLogCallback(std::function<void(const std::string&)> logCallback);

    /**
     * Run the main application loop
     * @return exit code
     */
    int run();

    /**
     * Shutdown the application gracefully
     */
    void shutdown();

    /**
     * Set the selected device for monitoring
     * @param deviceId device ID to monitor
     */
    void setSelectedDevice(const std::string& deviceId);

    /**
     * Get the currently selected device
     * @return device ID or empty string if none selected
     */
    std::string getSelectedDevice() const;

    /**
     * Enable or disable autostart
     * @param enable true to enable, false to disable
     * @return true if successful, false otherwise
     */
    bool setAutostart(bool enable);

    /**
     * Check if autostart is enabled
     * @return true if enabled, false otherwise
     */
    bool isAutostartEnabled() const;

    /**
     * Test screen control by turning display off for 1 second then back on
     * @param onComplete callback to execute when test completes
     */
    void testScreenControl(std::function<void(bool)> onComplete = nullptr);

    // Legacy methods for compatibility
    void start() { initialize(); }
    void controlScreen();
    void managePeripheralIDs();

private:
    void onDeviceConnected(const UsbDevice& device);
    void onDeviceDisconnected(const UsbDevice& device);
    void handleSelectedDeviceDisconnected();
    void handleSelectedDeviceReconnected();
    void loadConfiguration();
    void saveConfiguration();
    
    // Helper method to log messages to UI
    void logToUI(const std::string& message);
    
    std::unique_ptr<DisplayService> m_displayService;
    std::unique_ptr<UsbService> m_usbService;
    std::unique_ptr<StorageService> m_storageService;
    std::unique_ptr<AutostartService> m_autostartService;
    
    AppConfig m_config;
    bool m_isRunning;
    bool m_isSelectedDeviceConnected;
    std::string m_selectedDeviceId;
    std::function<void(const std::string&)> m_uiLogCallback;
};

#endif // APPLICATION_H