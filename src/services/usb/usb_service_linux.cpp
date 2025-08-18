#include "usb_service.h"
#include <iostream>
#include <thread>
#include <algorithm>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_LINUX
#include <libudev.h>
#include <dirent.h>
#endif

UsbService::UsbService() 
    : m_hiddenWindow(nullptr), m_deviceNotification(nullptr), m_isMonitoring(false) {
}

UsbService::~UsbService() {
    shutdown();
}

bool UsbService::initialize() {
#ifdef PLATFORM_LINUX
    return true; // Initialization successful
#else
    std::cout << "USB service not implemented for this platform" << std::endl;
    return false;
#endif
}

void UsbService::shutdown() {
    stopMonitoring();
}

std::vector<UsbDevice> UsbService::getConnectedDevices() {
#ifdef PLATFORM_LINUX
    return enumerateUsbDevices();
#else
    return std::vector<UsbDevice>();
#endif
}

bool UsbService::isDeviceConnected(const std::string& deviceId) {
    auto devices = getConnectedDevices();
    return std::any_of(devices.begin(), devices.end(),
        [&deviceId](const UsbDevice& device) {
            return device.deviceId == deviceId;
        });
}

void UsbService::setOnDeviceConnected(DeviceCallback callback) {
    m_onDeviceConnected = callback;
}

void UsbService::setOnDeviceDisconnected(DeviceCallback callback) {
    m_onDeviceDisconnected = callback;
}

bool UsbService::startMonitoring() {
    if (m_isMonitoring) {
        return false;
    }
    
#ifdef PLATFORM_LINUX
    m_isMonitoring = true;
    m_cachedDevices = getConnectedDevices();
    
    // Start a polling thread for device changes
    std::thread([this]() {
        while (m_isMonitoring) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (m_isMonitoring) {
                checkForDeviceChanges();
            }
        }
    }).detach();
    
    return true;
#else
    return false;
#endif
}

void UsbService::stopMonitoring() {
    m_isMonitoring = false;
}

void UsbService::handleDeviceChange(int wParam, long lParam) {
    // This method is Windows-specific, kept for compatibility
    checkForDeviceChanges();
}

void UsbService::checkForDeviceChanges() {
    auto currentDevices = getConnectedDevices();
    
    // Check for newly connected devices
    for (const auto& device : currentDevices) {
        auto it = std::find_if(m_cachedDevices.begin(), m_cachedDevices.end(),
            [&device](const UsbDevice& cached) {
                return cached.deviceId == device.deviceId;
            });
        
        if (it == m_cachedDevices.end() && m_onDeviceConnected) {
            m_onDeviceConnected(device);
        }
    }
    
    // Check for disconnected devices
    for (const auto& cached : m_cachedDevices) {
        auto it = std::find_if(currentDevices.begin(), currentDevices.end(),
            [&cached](const UsbDevice& device) {
                return device.deviceId == cached.deviceId;
            });
        
        if (it == currentDevices.end() && m_onDeviceDisconnected) {
            UsbDevice disconnected = cached;
            disconnected.isConnected = false;
            m_onDeviceDisconnected(disconnected);
        }
    }
    
    m_cachedDevices = currentDevices;
}

std::vector<UsbDevice> UsbService::enumerateUsbDevices() {
    std::vector<UsbDevice> devices;
    
#ifdef PLATFORM_LINUX
    // Method 1: Use udev library (preferred)
    struct udev* udev = udev_new();
    if (udev) {
        struct udev_enumerate* enumerate = udev_enumerate_new(udev);
        udev_enumerate_add_match_subsystem(enumerate, "usb");
        udev_enumerate_add_match_property(enumerate, "DEVTYPE", "usb_device");
        udev_enumerate_scan_devices(enumerate);
        
        struct udev_list_entry* dev_list_entry;
        udev_list_entry_foreach(dev_list_entry, udev_enumerate_get_list_entry(enumerate)) {
            const char* path = udev_list_entry_get_name(dev_list_entry);
            struct udev_device* dev = udev_device_new_from_syspath(udev, path);
            
            if (dev) {
                const char* vid = udev_device_get_sysattr_value(dev, "idVendor");
                const char* pid = udev_device_get_sysattr_value(dev, "idProduct");
                const char* product = udev_device_get_sysattr_value(dev, "product");
                const char* manufacturer = udev_device_get_sysattr_value(dev, "manufacturer");
                
                if (vid && pid) {
                    std::string deviceId = "USB_VID_" + std::string(vid) + "&PID_" + std::string(pid);
                    
                    std::string friendlyName;
                    if (manufacturer && product) {
                        friendlyName = std::string(manufacturer) + " " + std::string(product);
                    } else if (product) {
                        friendlyName = std::string(product);
                    } else {
                        friendlyName = "Unknown USB Device";
                    }
                    
                    devices.emplace_back(deviceId, friendlyName, std::string(vid), std::string(pid));
                }
                
                udev_device_unref(dev);
            }
        }
        
        udev_enumerate_unref(enumerate);
        udev_unref(udev);
    } else {
        // Method 2: Fallback to parsing /sys/bus/usb/devices
        DIR* dir = opendir("/sys/bus/usb/devices");
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_name[0] == '.') continue;
                
                std::string devicePath = "/sys/bus/usb/devices/" + std::string(entry->d_name);
                
                // Read vendor ID
                std::ifstream vidFile(devicePath + "/idVendor");
                std::string vid;
                if (vidFile.is_open() && std::getline(vidFile, vid)) {
                    vidFile.close();
                    
                    // Read product ID
                    std::ifstream pidFile(devicePath + "/idProduct");
                    std::string pid;
                    if (pidFile.is_open() && std::getline(pidFile, pid)) {
                        pidFile.close();
                        
                        std::string deviceId = "USB_VID_" + vid + "&PID_" + pid;
                        
                        // Try to read product name
                        std::ifstream productFile(devicePath + "/product");
                        std::string product;
                        std::string friendlyName = "Unknown USB Device";
                        if (productFile.is_open() && std::getline(productFile, product)) {
                            friendlyName = product;
                        }
                        
                        devices.emplace_back(deviceId, friendlyName, vid, pid);
                    }
                }
            }
            closedir(dir);
        }
    }
#endif
    
    return devices;
}

UsbDevice UsbService::getDeviceInfo(const std::string& devicePath) {
    // Simplified implementation
    return UsbDevice();
}
