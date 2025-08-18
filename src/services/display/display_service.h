#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include "config.h"
#include <functional>

#pragma once

#include <memory>
#include "config.h"

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
#elif defined(PLATFORM_LINUX)
    // Include X11 headers after Qt headers in source file to avoid macro conflicts
#elif defined(PLATFORM_MACOS)
    #include <CoreGraphics/CoreGraphics.h>
#endif

/**
 * Service responsible for controlling the display (monitor) state
 */
class DisplayService {
public:
    DisplayService();
    ~DisplayService();

    /**
     * Set a logging callback function for UI logging
     * @param logCallback function to call for logging messages
     */
    void setLogCallback(std::function<void(const std::string&)> logCallback);

    /**
     * Turn the display on
     * @return true if successful, false otherwise
     */
    bool turnOn();

    /**
     * Turn the display off
     * @return true if successful, false otherwise
     */
    bool turnOff();

    /**
     * Get current display state
     * @return true if display is on, false if off
     */
    bool isDisplayOn();

    /**
     * Turn off display immediately and schedule it to turn back on after specified delay
     * Display will also turn back on immediately if onDeviceReconnected() is called
     * @param delaySeconds delay in seconds before automatically turning display back on
     * @param onComplete callback to execute when operation completes (delay expires)
     */
    void scheduleDisplayOff(int delaySeconds, std::function<void()> onComplete = nullptr);

    /**
     * Cancel any scheduled display operations and turn display back on
     * This should be called when the monitored device reconnects
     */
    void cancelScheduledOperations();

    /**
     * Handle device reconnection - turns display back on and cancels any pending operations
     */
    void onDeviceReconnected();

private:
    bool isDisplayActive();
    void executeDelayedOperation(int delaySeconds, std::function<void()> operation);
    
    // Helper method to log messages
    void log(const std::string& message);
    
#ifdef PLATFORM_WINDOWS
    HANDLE m_scheduledTaskHandle;
#else
    void* m_scheduledTaskHandle;  // Placeholder for compatibility
#endif
    
    bool m_isTaskScheduled;
    std::function<void(const std::string&)> m_logCallback;
};

#endif // DISPLAY_SERVICE_H
