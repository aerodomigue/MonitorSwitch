#include "display_service.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

#ifdef PLATFORM_LINUX
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#ifdef None
#undef None
#endif
#endif

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
#ifdef PLATFORM_LINUX
    log("Turning display on...");
    
    // Try multiple methods to wake the display
    
    // Method 1: Use xset to turn on DPMS
    if (system("xset dpms force on") == 0) {
        log("Display turned on successfully (xset)");
        return true;
    }
    
    // Method 2: Use X11 DPMS extension directly
    Display* display = XOpenDisplay(nullptr);
    if (display) {
        int dummy;
        if (DPMSQueryExtension(display, &dummy, &dummy)) {
            DPMSForceLevel(display, DPMSModeOn);
            XFlush(display);
        }
        XCloseDisplay(display);
        log("Display turned on successfully (X11 DPMS)");
        return true;
    }
    
    // Method 3: Simulate user activity
    system("xdotool mousemove_relative 1 1");
    system("xdotool mousemove_relative -- -1 -1");
    
    log("Display turned on successfully (mouse simulation)");
    return true;
#else
    log("Display turn on not implemented for this platform");
    return false;
#endif
}

bool DisplayService::turnOff() {
#ifdef PLATFORM_LINUX
    log("Turning display off...");
    
    // Method 1: Use xset to turn off display
    if (system("xset dpms force off") == 0) {
        log("Display turned off successfully (xset)");
        return true;
    }
    
    // Method 2: Use X11 DPMS extension directly
    Display* display = XOpenDisplay(nullptr);
    if (display) {
        int dummy;
        if (DPMSQueryExtension(display, &dummy, &dummy)) {
            DPMSForceLevel(display, DPMSModeOff);
            XFlush(display);
        }
        XCloseDisplay(display);
        log("Display turned off successfully (X11 DPMS)");
        return true;
    }
    
    log("Failed to turn off display");
    return false;
#else
    log("Display turn off not implemented for this platform");
    return false;
#endif
}

bool DisplayService::isDisplayOn() {
#ifdef PLATFORM_LINUX
    Display* display = XOpenDisplay(nullptr);
    if (display) {
        int dummy;
        if (DPMSQueryExtension(display, &dummy, &dummy)) {
            CARD16 state;
            BOOL onoff;
            if (DPMSInfo(display, &state, &onoff)) {
                XCloseDisplay(display);
                return onoff && (state == DPMSModeOn);
            }
        }
        XCloseDisplay(display);
    }
    
    // Fallback: assume display is on
    return true;
#else
    return true;
#endif
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
        // Platform-specific cleanup if needed
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
    return isDisplayOn();
}
