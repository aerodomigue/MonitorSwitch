#include "usb_service.h"
#include <setupapi.h>
#include <dbt.h>
#include <algorithm>
#include <iostream>

#pragma comment(lib, "setupapi.lib")

UsbService::UsbService() 
    : m_hiddenWindow(nullptr), m_deviceNotification(nullptr), m_isMonitoring(false) {
}

UsbService::~UsbService() {
    shutdown();
}

bool UsbService::initialize() {
    // Create a hidden window for receiving device notifications
    WNDCLASS wc = {};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"UsbServiceWindow";
    
    if (!RegisterClass(&wc)) {
        return false;
    }
    
    m_hiddenWindow = CreateWindow(
        L"UsbServiceWindow", L"USB Service",
        0, 0, 0, 0, 0,
        nullptr, nullptr, GetModuleHandle(nullptr), this
    );
    
    return m_hiddenWindow != nullptr;
}

void UsbService::shutdown() {
    stopMonitoring();
    
    if (m_hiddenWindow) {
        DestroyWindow(m_hiddenWindow);
        m_hiddenWindow = nullptr;
    }
}

std::vector<UsbDevice> UsbService::getConnectedDevices() {
    return enumerateUsbDevices();
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
    if (!m_hiddenWindow || m_isMonitoring) {
        return false;
    }
    
    // Register for device notifications
    DEV_BROADCAST_DEVICEINTERFACE notificationFilter = {};
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    
    m_deviceNotification = RegisterDeviceNotification(
        m_hiddenWindow, &notificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
    );
    
    if (!m_deviceNotification) {
        return false;
    }
    
    m_isMonitoring = true;
    m_cachedDevices = getConnectedDevices();
    
    return true;
}

void UsbService::stopMonitoring() {
    if (m_deviceNotification) {
        UnregisterDeviceNotification(m_deviceNotification);
        m_deviceNotification = nullptr;
    }
    
    m_isMonitoring = false;
}

LRESULT CALLBACK UsbService::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DEVICECHANGE) {
        UsbService* service = reinterpret_cast<UsbService*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (service) {
            service->handleDeviceChange(wParam, lParam);
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void UsbService::handleDeviceChange(WPARAM wParam, LPARAM lParam) {
    if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE) {
        auto currentDevices = getConnectedDevices();
        
        // Compare with cached devices to find changes
        for (const auto& device : currentDevices) {
            auto it = std::find_if(m_cachedDevices.begin(), m_cachedDevices.end(),
                [&device](const UsbDevice& cached) {
                    return cached.deviceId == device.deviceId;
                });
            
            if (it == m_cachedDevices.end() && m_onDeviceConnected) {
                m_onDeviceConnected(device);
            }
        }
        
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
}

std::vector<UsbDevice> UsbService::enumerateUsbDevices() {
    std::vector<UsbDevice> devices;
    
    // Get device information set for USB devices
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(
        nullptr, L"USB", nullptr,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );
    
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return devices;
    }
    
    SP_DEVINFO_DATA deviceInfoData = {};
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        wchar_t deviceId[256] = {};
        wchar_t friendlyName[256] = {};
        
        // Get device instance ID
        if (SetupDiGetDeviceInstanceId(deviceInfoSet, &deviceInfoData, deviceId, sizeof(deviceId)/sizeof(wchar_t), nullptr)) {
            // Get friendly name
            SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_FRIENDLYNAME,
                nullptr, reinterpret_cast<PBYTE>(friendlyName), sizeof(friendlyName), nullptr);
            
            // Convert to string and create device object
            std::wstring wDeviceId(deviceId);
            std::wstring wFriendlyName(friendlyName);
            
            std::string deviceIdStr(wDeviceId.begin(), wDeviceId.end());
            std::string friendlyNameStr(wFriendlyName.begin(), wFriendlyName.end());
            
            // Extract VID and PID from device ID (format: USB\VID_XXXX&PID_XXXX\...)
            std::string vid, pid;
            size_t vidPos = deviceIdStr.find("VID_");
            size_t pidPos = deviceIdStr.find("PID_");
            
            if (vidPos != std::string::npos && pidPos != std::string::npos) {
                vid = deviceIdStr.substr(vidPos + 4, 4);
                pid = deviceIdStr.substr(pidPos + 4, 4);
            }
            
            devices.emplace_back(deviceIdStr, friendlyNameStr, vid, pid);
        }
    }
    
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return devices;
}

UsbDevice UsbService::getDeviceInfo(const std::string& devicePath) {
    // Implementation for getting detailed device info from path
    // This is a simplified version
    return UsbDevice();
}
