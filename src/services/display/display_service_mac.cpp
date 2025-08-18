#include "display_service.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

#ifdef PLATFORM_MACOS
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach_port.h>
#endif

DisplayService::DisplayService() 
    : m_scheduledTaskHandle(nullptr), m_isTaskScheduled(false) {
}

DisplayService::~DisplayService() {
    cancelScheduledOperations();
}

void DisplayService::setLogCallback(std::function<void(const std::string&)> logCallback) {
    m_logCallback = logCallback;
}

void DisplayService::log(const std::string& message) {
    // Always log to console
    std::cout << "[DISPLAY] " << message << std::endl;
    
    // Also log to UI if callback is set
    if (m_logCallback) {
        m_logCallback(message);
    }
}

bool DisplayService::turnOn() {
#ifdef PLATFORM_MACOS
    log("Turning display on...");
    
    // Method 1: Use caffeinate to wake the display
    int result = system("caffeinate -u -t 1");
    if (result == 0) {
        log("Display turned on successfully (caffeinate)");
        return true;
    }
    
    // Method 2: Wake the display by simulating a mouse move
    CGPoint currentPos;
    CGEventRef ourEvent = CGEventCreate(NULL);
    if (ourEvent) {
        currentPos = CGEventGetLocation(ourEvent);
        CFRelease(ourEvent);
        
        // Move mouse slightly and back
        CGEventRef moveEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, 
                                                      CGPointMake(currentPos.x + 1, currentPos.y), 
                                                      kCGMouseButtonLeft);
        if (moveEvent) {
            CGEventPost(kCGHIDEventTap, moveEvent);
            CFRelease(moveEvent);
        }
        
        moveEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, currentPos, kCGMouseButtonLeft);
        if (moveEvent) {
            CGEventPost(kCGHIDEventTap, moveEvent);
            CFRelease(moveEvent);
        }
        
        log("Display turned on successfully (mouse simulation)");
        return true;
    }
    
    // Method 3: Fallback - try pressing a modifier key
    result = system("osascript -e 'tell application \"System Events\" to key code 63'");
    if (result == 0) {
        log("Display turned on successfully (key simulation)");
    } else {
        log("Failed to turn on display");
    }
    return (result == 0);
#else
    std::cout << "Display turn on not implemented for this platform" << std::endl;
    return false;
#endif
}

bool DisplayService::turnOff() {
#ifdef PLATFORM_MACOS
    log("Turning display off...");
    
    // Method 1: Use pmset to put display to sleep
    int result = system("pmset displaysleepnow");
    if (result == 0) {
        log("Display turned off successfully (pmset)");
        return true;
    }
    
    // Method 2: Use IOKit to turn off display
    io_registry_entry_t r = IORegistryEntryFromPath(kIOMainPortDefault, 
                                                   "IOService:/IOResources/IODisplayWrangler");
    if (r != MACH_PORT_NULL) {
        IORegistryEntrySetCFProperty(r, CFSTR("IORequestIdle"), kCFBooleanTrue);
        IOObjectRelease(r);
        log("Display turned off successfully (IODisplayWrangler)");
        return true;
    }
    
    // Method 3: Fallback - use AppleScript to trigger screensaver
    result = system("osascript -e 'tell application \"System Events\" to start current screen saver'");
    if (result == 0) {
        log("Display turned off successfully (screensaver)");
    } else {
        log("Failed to turn off display");
    }
    return (result == 0);
#else
    std::cout << "Display turn off not implemented for this platform" << std::endl;
    return false;
#endif
}

bool DisplayService::isDisplayOn() {
#ifdef PLATFORM_MACOS
    // Check if display is awake
    return CGDisplayIsAsleep(kCGDirectMainDisplay) == false;
#else
    return true; // Assume on if we can't check
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
