#!/bin/bash

# Cross-platform build script for MonitorSwitch

echo "Building MonitorSwitch Application..."
echo "=============================="

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
    DEPS="qt6-base-dev qt6-tools-dev libudev-dev libx11-dev libxext-dev libgl1-mesa-dev libglu1-mesa-dev libopengl-dev libglx-dev pkg-config libxkbcommon-dev"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
    DEPS="qt6"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

echo "Detected platform: $PLATFORM"

# Check if build directory exists, create if not
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Install dependencies based on platform
echo "Checking dependencies..."
if [[ "$PLATFORM" == "Linux" ]]; then
    if command -v apt-get &> /dev/null; then
        echo "Installing dependencies with apt..."
        sudo apt-get update
        sudo apt-get install -y $DEPS cmake build-essential
    elif command -v dnf &> /dev/null; then
        echo "Installing dependencies with dnf..."
        sudo dnf install -y qt6-qtbase-devel qt6-qttools-devel libudev-devel libX11-devel libXext-devel mesa-libGL-devel mesa-libGLU-devel cmake gcc-c++
    elif command -v pacman &> /dev/null; then
        echo "Installing dependencies with pacman..."
        sudo pacman -S --needed qt6-base qt6-tools systemd-libs libx11 libxext mesa cmake gcc
    else
        echo "Unknown package manager. Please install dependencies manually:"
        echo "  Qt 6 development packages"
        echo "  libudev development headers"
        echo "  X11 development headers"
        echo "  CMake and build tools"
    fi
elif [[ "$PLATFORM" == "macOS" ]]; then
    if command -v brew &> /dev/null; then
        echo "Installing dependencies with Homebrew..."
        brew install qt6 cmake
    else
        echo "Homebrew not found. Please install Qt 6 and CMake manually."
        echo "You can install Homebrew from: https://brew.sh"
        exit 1
    fi
fi

# Configure with CMake
echo "Configuring with CMake..."
if [[ "$PLATFORM" == "macOS" ]]; then
    # Set Qt path for macOS
    QT_PATH=$(brew --prefix qt6)
    cmake -DCMAKE_PREFIX_PATH="$QT_PATH" ..
else
    cmake ..
fi

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# Build the project
echo "Building project..."
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo ""
echo "Build completed successfully!"

if [[ "$PLATFORM" == "macOS" ]]; then
    echo "Application bundle: build/MonitorSwitch.app"
    echo "To run: open build/MonitorSwitch.app"
else
    echo "Executable: build/MonitorSwitch"
    echo "To run: ./build/MonitorSwitch"
fi

echo ""
echo "To install system-wide:"
if [[ "$PLATFORM" == "Linux" ]]; then
    echo "  sudo cmake --install ."
    echo "  # Creates /usr/local/bin/MonitorSwitch"
    echo "  # Creates desktop entry and icon"
else
    echo "  sudo cmake --install ."
    echo "  # Installs to /Applications/"
fi
