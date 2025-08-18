#include "usb_service.h"
#include <iostream>
#include <thread>
#include <algorithm>

#ifdef PLATFORM_MACOS
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

UsbService::UsbService() 
    : m_hiddenWindow(nullptr), m_deviceNotification(nullptr), m_isMonitoring(false) {
}

UsbService::~UsbService() {
    shutdown();
}

bool UsbService::initialize() {
#ifdef PLATFORM_MACOS
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
#ifdef PLATFORM_MACOS
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
    
#ifdef PLATFORM_MACOS
    m_isMonitoring = true;
    m_cachedDevices = getConnectedDevices();
    
    // Start a polling thread for device changes (simplified approach)
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
    
#ifdef PLATFORM_MACOS
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (!matchingDict) {
        return devices;
    }
    
    io_iterator_t iterator;
    kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iterator);
    if (kr != KERN_SUCCESS) {
        return devices;
    }
    
    io_service_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator))) {
        CFStringRef deviceName = NULL;
        CFNumberRef vendorID = NULL;
        CFNumberRef productID = NULL;
        
    // Get device location ID (if needed, otherwise remove this line)
    // IORegistryEntryGetCFProperty is not used for getting location ID; use IORegistryEntryCreateCFProperty if needed
    // Example (uncomment if you need location ID):
    // CFNumberRef locationID = (CFNumberRef)IORegistryEntryCreateCFProperty(usbDevice, CFSTR(kIOUSBDevicePropertyLocationID), kCFAllocatorDefault, 0);
        
        // Get vendor ID
        vendorID = (CFNumberRef)IORegistryEntryCreateCFProperty(usbDevice, 
                                                               CFSTR(kUSBVendorID),
                                                               kCFAllocatorDefault, 0);
        
        // Get product ID  
        productID = (CFNumberRef)IORegistryEntryCreateCFProperty(usbDevice,
                                                                CFSTR(kUSBProductID),
                                                                kCFAllocatorDefault, 0);
        
        // Get device name
        deviceName = (CFStringRef)IORegistryEntryCreateCFProperty(usbDevice,
                                                                 CFSTR("USB Product Name"),
                                                                 kCFAllocatorDefault, 0);
        
        if (vendorID && productID) {
            UInt16 vid, pid;
            CFNumberGetValue(vendorID, kCFNumberSInt16Type, &vid);
            CFNumberGetValue(productID, kCFNumberSInt16Type, &pid);
            
            char vidStr[8], pidStr[8];
            snprintf(vidStr, sizeof(vidStr), "%04X", vid);
            snprintf(pidStr, sizeof(pidStr), "%04X", pid);
            
            std::string deviceId = "USB_VID_" + std::string(vidStr) + "&PID_" + std::string(pidStr);
            
            std::string friendlyName = "Unknown USB Device";
            if (deviceName) {
                char nameBuf[256];
                if (CFStringGetCString(deviceName, nameBuf, sizeof(nameBuf), kCFStringEncodingUTF8)) {
                    friendlyName = std::string(nameBuf);
                }
            }
            
            devices.emplace_back(deviceId, friendlyName, std::string(vidStr), std::string(pidStr));
        }
        
        // Release CF objects
        if (deviceName) CFRelease(deviceName);
        if (vendorID) CFRelease(vendorID);
        if (productID) CFRelease(productID);
        
        IOObjectRelease(usbDevice);
    }
    
    IOObjectRelease(iterator);
#endif
    
    return devices;
}

UsbDevice UsbService::getDeviceInfo(const std::string& devicePath) {
    // Simplified implementation
    return UsbDevice();
}
