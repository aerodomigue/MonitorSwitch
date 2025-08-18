#ifndef USB_SERVICE_H
#define USB_SERVICE_H

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#include "config.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

/**
 * Structure representing a USB device
 */
struct UsbDevice {
    std::string deviceId;
    std::string friendlyName;
    std::string vendorId;
    std::string productId;
    bool isConnected;
    
    UsbDevice() : isConnected(false) {}
    
    UsbDevice(const std::string& id, const std::string& name, 
              const std::string& vid, const std::string& pid)
        : deviceId(id), friendlyName(name), vendorId(vid), productId(pid), isConnected(true) {}
};

/**
 * Service responsible for USB device detection and monitoring
 */
class UsbService {
public:
    using DeviceCallback = std::function<void(const UsbDevice&)>;
    
    UsbService();
    ~UsbService();

    /**
     * Initialize the USB service and start monitoring
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * Stop monitoring and cleanup
     */
    void shutdown();

    /**
     * Get all currently connected USB devices
     * @return vector of connected USB devices
     */
    std::vector<UsbDevice> getConnectedDevices();

    /**
     * Check if a specific device is connected
     * @param deviceId the device ID to check
     * @return true if connected, false otherwise
     */
    bool isDeviceConnected(const std::string& deviceId);

    /**
     * Set callback for device connection events
     * @param callback function to call when device is connected
     */
    void setOnDeviceConnected(DeviceCallback callback);

    /**
     * Set callback for device disconnection events
     * @param callback function to call when device is disconnected
     */
    void setOnDeviceDisconnected(DeviceCallback callback);

    /**
     * Start monitoring for device changes
     * @return true if successful, false otherwise
     */
    bool startMonitoring();

    /**
     * Stop monitoring for device changes
     */
    void stopMonitoring();

private:
#ifdef PLATFORM_WINDOWS
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void handleDeviceChange(WPARAM wParam, LPARAM lParam);
#else
    void handleDeviceChange(int wParam, long lParam);  // Compatibility method
    void checkForDeviceChanges();  // Cross-platform device change detection
#endif
    
    std::vector<UsbDevice> enumerateUsbDevices();
    UsbDevice getDeviceInfo(const std::string& devicePath);
    
#ifdef PLATFORM_WINDOWS    
    HWND m_hiddenWindow;
    HDEVNOTIFY m_deviceNotification;
#else
    void* m_hiddenWindow;      // Placeholder for compatibility
    void* m_deviceNotification; // Placeholder for compatibility
#endif
    
    DeviceCallback m_onDeviceConnected;
    DeviceCallback m_onDeviceDisconnected;
    std::vector<UsbDevice> m_cachedDevices;
    bool m_isMonitoring;
};

#endif // USB_SERVICE_H
