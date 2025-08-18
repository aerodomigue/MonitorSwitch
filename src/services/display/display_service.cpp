#include "display_service.h"
#include <thread>
#include <chrono>
#include <iostream>

DisplayService::DisplayService() 
    : m_scheduledTaskHandle(nullptr), m_isTaskScheduled(false) {
}

DisplayService::~DisplayService() {
    cancelScheduledOperations();
}

void DisplayService::setLogCallback(std::function<void(const std::string&)> callback) {
    m_logCallback = callback;
}

void DisplayService::log(const std::string& message) {
    if (m_logCallback) {
        m_logCallback(message);
    }
    std::cout << message << std::endl;
}

bool DisplayService::turnOn() {
    log("Turning display on...");
    // Send message to turn on the display
    bool success = SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1) == 0;
    if (success) {
        log("Display turned on successfully");
    } else {
        log("Failed to turn on display");
    }
    return success;
}

bool DisplayService::turnOff() {
    log("Turning display off...");
    // Send message to turn off the display
    bool success = SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2) == 0;
    if (success) {
        log("Display turned off successfully");
    } else {
        log("Failed to turn off display");
    }
    return success;
}

bool DisplayService::isDisplayOn() {
    return isDisplayActive();
}

void DisplayService::scheduleDisplayOff(int delaySeconds, std::function<void()> onComplete) {
    // Cancel any existing scheduled operations
    cancelScheduledOperations();
    
    // Turn off the display immediately
    if (!turnOff()) {
        std::cerr << "[DISPLAY] Failed to turn off display" << std::endl;
        if (onComplete) onComplete();
        return;
    }
    
    std::cout << "[DISPLAY] Display turned off, will turn back on in " << delaySeconds << " seconds or when device reconnects" << std::endl;
    
    m_isTaskScheduled = true;
    
    // Schedule turning the display back on after the delay
    std::thread([this, delaySeconds, onComplete]() {
        std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
        
        // Only turn on if the task hasn't been cancelled (e.g., by device reconnection)
        if (m_isTaskScheduled) {
            std::cout << "[DISPLAY] Delay expired, turning display back on" << std::endl;
            turnOn();
            m_isTaskScheduled = false;
            
            if (onComplete) {
                onComplete();
            }
        }
    }).detach();
}

void DisplayService::cancelScheduledOperations() {
    if (m_isTaskScheduled) {
        std::cout << "[DISPLAY] Cancelling scheduled display operations" << std::endl;
        m_isTaskScheduled = false;
    }
    
    if (m_scheduledTaskHandle) {
        CloseHandle(m_scheduledTaskHandle);
        m_scheduledTaskHandle = nullptr;
    }
}

void DisplayService::onDeviceReconnected() {
    if (m_isTaskScheduled) {
        std::cout << "[DISPLAY] Device reconnected, turning display back on" << std::endl;
        cancelScheduledOperations();
        turnOn();
    }
}

bool DisplayService::isDisplayActive() {
    // Check if the display is currently active
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    
    // A simple way to check if display is on is to see if we can get cursor position
    // This is a basic implementation - more sophisticated methods could be used
    return true; // Placeholder - would need more complex detection
}
