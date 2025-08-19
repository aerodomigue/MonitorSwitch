cmake_minimum_required(VERSION 3.16)

project(MonitorSwitch VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Find platform-specific packages
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
elseif(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(UDEV REQUIRED libudev)
    find_package(X11 REQUIRED)
endif()

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Sanity check: ensure critical header exists (helps debug CI path issues)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/application.h")
    message(FATAL_ERROR "Missing header: ${CMAKE_CURRENT_SOURCE_DIR}/src/core/application.h")
else()
    message(STATUS "Found header: ${CMAKE_CURRENT_SOURCE_DIR}/src/core/application.h")
endif()

# Collect common source files
file(GLOB_RECURSE COMMON_SOURCES
    src/main.cpp
    src/ui/*.cpp
    src/ui/components/*.cpp
    src/core/*.cpp
    src/models/*.cpp
)

# Platform-specific source files
set(PLATFORM_SOURCES "")
if(WIN32)
    file(GLOB_RECURSE WIN_SOURCES
        src/services/display/display_service.cpp
        src/services/usb/usb_service.cpp
        src/services/storage/storage_service.cpp
        src/services/autostart/autostart_service.cpp
    )
    set(PLATFORM_SOURCES ${WIN_SOURCES})
elseif(APPLE)
    file(GLOB_RECURSE MAC_SOURCES
        src/services/display/display_service_mac.cpp
        src/services/usb/usb_service_mac.cpp
        src/services/storage/storage_service_unix.cpp
        src/services/autostart/autostart_service_mac.cpp
    )
    set(PLATFORM_SOURCES ${MAC_SOURCES})
elseif(UNIX)
    file(GLOB_RECURSE LINUX_SOURCES
        src/services/display/display_service_linux.cpp
        src/services/usb/usb_service_linux.cpp
        src/services/storage/storage_service_unix.cpp
        src/services/autostart/autostart_service_linux.cpp
    )
    set(PLATFORM_SOURCES ${LINUX_SOURCES})
endif()

# Collect header files
file(GLOB_RECURSE HEADERS
    include/*.h
    src/*.h
    src/ui/*.h
    src/ui/components/*.h
    src/core/*.h
    src/models/*.h
    src/services/display/*.h
    src/services/usb/*.h
    src/services/storage/*.h
    src/services/autostart/*.h
)

# Create executable
if(WIN32)
    # Add Windows resource file for icon and version info
    # WIN32 flag creates a Windows GUI application (no console window)
    add_executable(MonitorSwitch WIN32 ${COMMON_SOURCES} ${PLATFORM_SOURCES} MonitorSwitch.rc)
elseif(APPLE)
    # Create macOS app bundle
    add_executable(MonitorSwitch MACOSX_BUNDLE ${COMMON_SOURCES} ${PLATFORM_SOURCES})
else()
    add_executable(MonitorSwitch ${COMMON_SOURCES} ${PLATFORM_SOURCES})
endif()

# Define platform macros for compatibility with existing code
if(WIN32)
    target_compile_definitions(MonitorSwitch PRIVATE 
        PLATFORM_WINDOWS
        # Windows version targeting for compatibility
        WINVER=0x0601           # Windows 7
        _WIN32_WINNT=0x0601     # Windows 7 and later (includes Windows 11)
        NTDDI_VERSION=0x06010000 # Windows 7 SP1
    )
elseif(APPLE)
    target_compile_definitions(MonitorSwitch PRIVATE PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(MonitorSwitch PRIVATE PLATFORM_LINUX)
endif()

# Set include directories for the target (put ours before system paths)
target_include_directories(MonitorSwitch BEFORE PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Debug: print include dirs at configure time
get_target_property(MS_INCS MonitorSwitch INCLUDE_DIRECTORIES)
message(STATUS "MonitorSwitch include dirs: ${MS_INCS}")

# Link Qt libraries
target_link_libraries(MonitorSwitch 
    Qt6::Core 
    Qt6::Widgets
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(MonitorSwitch 
        setupapi
        user32
        shell32
        ole32
        advapi32
    )
elseif(APPLE)
    target_link_libraries(MonitorSwitch 
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )
elseif(UNIX)
    target_link_libraries(MonitorSwitch 
        ${UDEV_LIBRARIES}
        ${X11_LIBRARIES}
        ${X11_Xext_LIB}
    )
    target_include_directories(MonitorSwitch PRIVATE ${UDEV_INCLUDE_DIRS})
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(MonitorSwitch PRIVATE /W4)
else()
    target_compile_options(MonitorSwitch PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation
if(APPLE)
    set_target_properties(MonitorSwitch PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_ICON_FILE MonitorSwitch.icns
    )
    
    # Add the .icns file as a resource if it exists
    if(EXISTS ${CMAKE_SOURCE_DIR}/icons/MonitorSwitch.icns)
        target_sources(MonitorSwitch PRIVATE ${CMAKE_SOURCE_DIR}/icons/MonitorSwitch.icns)
        set_source_files_properties(${CMAKE_SOURCE_DIR}/icons/MonitorSwitch.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif()
    
    # Also add the PNG for runtime access
    target_sources(MonitorSwitch PRIVATE ${CMAKE_SOURCE_DIR}/icons/MonitorSwitch.png)
    set_source_files_properties(${CMAKE_SOURCE_DIR}/icons/MonitorSwitch.png PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    
    install(TARGETS MonitorSwitch BUNDLE DESTINATION .)
elseif(UNIX AND NOT APPLE)
    install(TARGETS MonitorSwitch RUNTIME DESTINATION bin)
    install(FILES MonitorSwitch.desktop DESTINATION share/applications)
    install(FILES icons/MonitorSwitch.png DESTINATION share/icons/hicolor/64x64/apps)
elseif(WIN32)
    # Windows installation
    install(TARGETS MonitorSwitch RUNTIME DESTINATION .)
    install(DIRECTORY icons/ DESTINATION icons FILES_MATCHING PATTERN "*.ico" PATTERN "*.png" PATTERN "*.svg")
    install(FILES README.md DESTINATION . OPTIONAL)
else()
    install(TARGETS MonitorSwitch RUNTIME DESTINATION bin)
endif()