#include "autostart_service.h"
#include "config.h"
#include <windows.h>
#include <iostream>

const std::string AutostartService::AUTOSTART_REGISTRY_KEY = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

AutostartService::AutostartService() 
    : m_applicationName(APP_NAME) {
}

AutostartService::~AutostartService() {
}

bool AutostartService::enableAutostart(const std::string& appPath) {
    if (appPath.empty()) {
        return false;
    }
    
    // Add quotes around the path to handle spaces
    std::string quotedPath = "\"" + appPath + "\"";
    
    return setRegistryValue(AUTOSTART_REGISTRY_KEY, m_applicationName, quotedPath);
}

bool AutostartService::disableAutostart() {
    return deleteRegistryValue(AUTOSTART_REGISTRY_KEY, m_applicationName);
}

bool AutostartService::isAutostartEnabled() {
    std::string value = getRegistryValue(AUTOSTART_REGISTRY_KEY, m_applicationName);
    return !value.empty();
}

std::string AutostartService::getAutostartRegistryKey() {
    return AUTOSTART_REGISTRY_KEY;
}

void AutostartService::setApplicationName(const std::string& appName) {
    m_applicationName = appName;
}

bool AutostartService::setRegistryValue(const std::string& keyPath, const std::string& valueName, const std::string& value) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_SET_VALUE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to open registry key: " << result << std::endl;
        return false;
    }
    
    result = RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ, 
                           reinterpret_cast<const BYTE*>(value.c_str()), 
                           static_cast<DWORD>(value.length() + 1));
    
    RegCloseKey(hKey);
    
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to set registry value: " << result << std::endl;
        return false;
    }
    
    return true;
}

bool AutostartService::deleteRegistryValue(const std::string& keyPath, const std::string& valueName) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_SET_VALUE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false; // Key doesn't exist or can't be opened
    }
    
    result = RegDeleteValueA(hKey, valueName.c_str());
    RegCloseKey(hKey);
    
    return (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND);
}

std::string AutostartService::getRegistryValue(const std::string& keyPath, const std::string& valueName) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return "";
    }
    
    DWORD dataSize = 0;
    result = RegQueryValueExA(hKey, valueName.c_str(), nullptr, nullptr, nullptr, &dataSize);
    
    if (result != ERROR_SUCCESS || dataSize == 0) {
        RegCloseKey(hKey);
        return "";
    }
    
    std::string value(dataSize - 1, '\0'); // -1 to exclude null terminator
    result = RegQueryValueExA(hKey, valueName.c_str(), nullptr, nullptr, 
                             reinterpret_cast<LPBYTE>(&value[0]), &dataSize);
    
    RegCloseKey(hKey);
    
    if (result != ERROR_SUCCESS) {
        return "";
    }
    
    return value;
}
