

#include "application.h"
#include "utils.h"
#include "config.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "../services/usb/usb_service.h"
// Implementation for UI to get connected USB devices
std::vector<UsbDevice> Application::getConnectedUsbDevices() const {
    if (m_usbService) {
        return m_usbService->getConnectedDevices();
    }
    return {};
}

#ifdef _WIN32
#include <windows.h>
#endif

Application::Application() 
    : m_isRunning(false), m_isSelectedDeviceConnected(false) {
    
    // Initialize services
    m_displayService = std::make_unique<DisplayService>();
    m_usbService = std::make_unique<UsbService>();
    m_storageService = std::make_unique<StorageService>();
    m_autostartService = std::make_unique<AutostartService>();
}

Application::~Application() {
    shutdown();
}

void Application::setLogCallback(std::function<void(const std::string&)> logCallback) {
    // Store the callback for UI logging
    m_uiLogCallback = logCallback;
    
    // Pass the log callback to the storage service
    if (m_storageService) {
        m_storageService->setLogCallback(logCallback);
    }
    
    // Pass the log callback to the display service
    if (m_displayService) {
        m_displayService->setLogCallback(logCallback);
    }
}

void Application::logToUI(const std::string& message) {
    // Log to console
    std::cout << "[ACTIVITY] " << message << std::endl;
    
    // Log to UI if callback is available
    if (m_uiLogCallback) {
        m_uiLogCallback(message);
    }
}

bool Application::initialize() {
    std::cout << "Initializing " << APP_NAME << "..." << std::endl;
    
    // Display current platform with detailed information
#ifdef _WIN32
    std::cout << "[PLATFORM] Running on: Windows" << std::endl;
    std::cout << "[PLATFORM] Platform features: Win32 API, Registry autostart, DPMS display control" << std::endl;
#elif defined(__APPLE__)
    std::cout << "[PLATFORM] Running on: macOS" << std::endl;
    std::cout << "[PLATFORM] Platform features: IOKit, launchd autostart, CoreGraphics display control" << std::endl;
#elif defined(__linux__)
    std::cout << "[PLATFORM] Running on: Linux" << std::endl;
    std::cout << "[PLATFORM] Platform features: X11/udev, XDG autostart, DPMS display control" << std::endl;
#else
    std::cout << "[PLATFORM] Running on: Unknown Platform" << std::endl;
    std::cout << "[PLATFORM] Warning: Some features may not be available" << std::endl;
#endif
    
    // Initialize storage service first
    if (!m_storageService->initialize()) {
        std::cerr << "Failed to initialize storage service" << std::endl;
        return false;
    }
    
    // Initialize USB service
    if (!m_usbService->initialize()) {
        std::cerr << "Failed to initialize USB service" << std::endl;
        return false;
    }
    
    // Set up USB device callbacks
    m_usbService->setOnDeviceConnected(
        [this](const UsbDevice& device) { onDeviceConnected(device); }
    );
    
    m_usbService->setOnDeviceDisconnected(
        [this](const UsbDevice& device) { onDeviceDisconnected(device); }
    );
    
    // Start USB monitoring
    if (!m_usbService->startMonitoring()) {
        std::cerr << "Failed to start USB monitoring" << std::endl;
        return false;
    }
    
    std::cout << "Application initialized successfully" << std::endl;
    return true;
}

void Application::startConfiguration() {
    std::cout << "Starting application configuration..." << std::endl;
    
    // Load configuration
    loadConfiguration();
    
    // Check if selected device is currently connected
    if (!m_config.selectedDeviceId.empty()) {
        m_isSelectedDeviceConnected = m_usbService->isDeviceConnected(m_config.selectedDeviceId);
        m_selectedDeviceId = m_config.selectedDeviceId;
    }
    
    std::cout << "Application configuration completed" << std::endl;
}

int Application::run() {
    if (!initialize()) {
        return -1;
    }
    
    m_isRunning = true;
    
    std::cout << "Application running. Selected device: " 
              << (m_selectedDeviceId.empty() ? "None" : m_selectedDeviceId) << std::endl;
    
#ifdef _WIN32
    // Windows message loop
    MSG msg;
    while (m_isRunning && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#else
    // For non-Windows platforms, Qt event loop will handle this
    // This method won't be called directly on macOS/Linux
#endif
    
    return 0;
}

void Application::shutdown() {
    std::cout << "Shutting down application..." << std::endl;
    
    m_isRunning = false;
    
    // Save current configuration
    saveConfiguration();
    
    // Shutdown services
    if (m_usbService) {
        m_usbService->shutdown();
    }
    
    std::cout << "Application shutdown complete" << std::endl;
}

void Application::setSelectedDevice(const std::string& deviceId) {
    m_selectedDeviceId = deviceId;
    m_config.selectedDeviceId = deviceId;
    
    // Check if the device is currently connected
    m_isSelectedDeviceConnected = m_usbService->isDeviceConnected(deviceId);
    
    // Save the updated configuration
    saveConfiguration();
    
    std::cout << "Selected device set to: " << deviceId << std::endl;
}

std::string Application::getSelectedDevice() const {
    return m_selectedDeviceId;
}

bool Application::setAutostart(bool enable) {
    if (enable) {
#ifdef _WIN32
        // Get current executable path
        wchar_t exePath[MAX_PATH];
        GetModuleFileName(nullptr, exePath, MAX_PATH);
        
        std::wstring wExePath(exePath);
        std::string exePathStr(wExePath.begin(), wExePath.end());
#else
        // For Unix-like systems, we need to determine the executable path differently
        // This is a simplified approach - in a real application you might want to
        // use platform-specific methods to get the full executable path
        std::string exePathStr = "softKMS"; // Assumes it's in PATH
#endif
        
        if (!m_autostartService->enableAutostart(exePathStr)) {
            return false;
        }
    } else {
        if (!m_autostartService->disableAutostart()) {
            return false;
        }
    }
    
    m_config.startOnBoot = enable;
    saveConfiguration();
    
    return true;
}

bool Application::isAutostartEnabled() const {
    return m_autostartService->isAutostartEnabled();
}

void Application::testScreenControl(std::function<void(bool)> onComplete) {
    if (!m_displayService) {
        std::cerr << "[SCREEN TEST] Display service not available" << std::endl;
        if (onComplete) onComplete(false);
        return;
    }
    
    std::cout << "[SCREEN TEST] Starting screen control test..." << std::endl;
    
    // Turn off the display
    bool turnOffSuccess = m_displayService->turnOff();
    if (!turnOffSuccess) {
        std::cerr << "[SCREEN TEST] Failed to turn off display" << std::endl;
        if (onComplete) onComplete(false);
        return;
    }
    
    std::cout << "[SCREEN TEST] Display turned off, waiting 1 second..." << std::endl;
    
    // Create a separate thread to wait 1 second and then turn the display back on
    std::thread([this, onComplete]() {
        // Wait for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Turn the display back on
        bool turnOnSuccess = m_displayService->turnOn();
        if (turnOnSuccess) {
            std::cout << "[SCREEN TEST] Display turned back on - test completed successfully" << std::endl;
        } else {
            std::cerr << "[SCREEN TEST] Failed to turn display back on" << std::endl;
        }
        
        if (onComplete) onComplete(turnOnSuccess);
    }).detach(); // Detach the thread so it runs independently
}

void Application::controlScreen() {
    // Control the screen display
    if (m_displayService) {
        // This is called from the legacy interface
        // You can implement specific screen control logic here
        std::cout << "Screen control accessed via legacy interface" << std::endl;
    }
}

void Application::managePeripheralIDs() {
    // Manage peripheral IDs
    if (m_usbService) {
        auto devices = m_usbService->getConnectedDevices();
        std::cout << "Connected USB devices:" << std::endl;
        for (const auto& device : devices) {
            std::cout << "  - " << device.friendlyName << " (" << device.deviceId << ")" << std::endl;
        }
    }
}

void Application::onDeviceConnected(const UsbDevice& device) {
    std::cout << "Device connected: " << device.friendlyName << " (" << device.deviceId << ")" << std::endl;
    
    // Log all device connections to UI
    logToUI("Device connected: " + device.friendlyName + " (" + device.deviceId + ")");
    
    // If this is our selected device, handle reconnection
    if (device.deviceId == m_selectedDeviceId) {
        logToUI("Selected device reconnected: " + device.friendlyName);
        handleSelectedDeviceReconnected();
    }
}

void Application::onDeviceDisconnected(const UsbDevice& device) {
    std::cout << "Device disconnected: " << device.friendlyName << " (" << device.deviceId << ")" << std::endl;
    
    // Log all device disconnections to UI
    logToUI("Device disconnected: " + device.friendlyName + " (" + device.deviceId + ")");
    
    // If this is our selected device, handle disconnection
    if (device.deviceId == m_selectedDeviceId) {
        logToUI("Selected device disconnected: " + device.friendlyName);
        handleSelectedDeviceDisconnected();
    }
}

void Application::handleSelectedDeviceDisconnected() {
    std::cout << "Selected device disconnected - initiating screen control sequence" << std::endl;
    logToUI("Initiating screen control: turning off display in " + std::to_string(m_config.screenOffDelay) + " seconds");
    
    m_isSelectedDeviceConnected = false;
    
    // Schedule display to turn off after the configured delay
    m_displayService->scheduleDisplayOff(m_config.screenOffDelay, [this]() {
        std::cout << "Display turned off due to device disconnection" << std::endl;
        logToUI("Display automatically turned back on (timeout reached)");
        
        // Turn display back on after the off period
        m_displayService->turnOn();
        std::cout << "Display turned back on" << std::endl;
    });
}

void Application::handleSelectedDeviceReconnected() {
    std::cout << "Selected device reconnected - turning display back on" << std::endl;
    logToUI("Device reconnected: turning display back on");
    
    m_isSelectedDeviceConnected = true;
    
    // Handle device reconnection (will cancel scheduled operations and turn display on)
    m_displayService->onDeviceReconnected();
}

void Application::loadConfiguration() {
    std::cout << "[APP] Loading application configuration..." << std::endl;
    
    m_config = m_storageService->loadConfig();
    m_selectedDeviceId = m_config.selectedDeviceId;
    
    std::cout << "[APP] Configuration loaded:" << std::endl;
    std::cout << "[APP]   - Start on boot: " << (m_config.startOnBoot ? "Yes" : "No") << std::endl;
    std::cout << "[APP]   - Selected device: " << (m_config.selectedDeviceId.empty() ? "None" : m_config.selectedDeviceId) << std::endl;
    std::cout << "[APP]   - Screen off delay: " << m_config.screenOffDelay << " seconds" << std::endl;
    std::cout << "[APP]   - Known devices count: " << m_config.knownDevices.size() << std::endl;
}

void Application::saveConfiguration() {
    std::cout << "[APP] Saving application configuration..." << std::endl;
    
    if (!m_storageService->saveConfig(m_config)) {
        std::cerr << "[APP] Failed to save configuration" << std::endl;
    } else {
        std::cout << "[APP] Configuration saved successfully" << std::endl;
    }
}