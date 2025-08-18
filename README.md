# MonitorSwitch

**MonitorSwitch** is a cross-platform software solution compatible with Windows, Linux, and macOS that enables intelligent monitor input switching without relying on the **DDC/CI protocol**. This makes it particularly useful for users whose monitors have poor or unreliable DDC/CI implementation.

## How it Works

The software operates by automatically disabling the video output from your PC when a specified USB peripheral is disconnected. This approach is especially effective when using USB KVM switches that don't provide monitor management capabilities.

When your selected USB device (such as a keyboard, mouse, or USB hub) is disconnected:
1. The screen turns off after a configurable delay
2. Your monitor automatically switches to the next available input
3. When the device reconnects, the screen turns back on

---

## ✅ Advantages

- **Universal Compatibility**: Works with any monitor, regardless of DDC/CI support
- **Unlimited Monitor Support**: Manage multiple monitors simultaneously, as long as each monitor is physically connected to all your PCs
- **Cross-Platform**: Native support for Windows, Linux, and macOS
- **USB KVM Integration**: Perfect companion for USB-only KVM switches
- **Customizable Delays**: Configure screen-off timing to match your workflow
- **System Tray Integration**: Runs quietly in the background with easy access
- **Auto-start Support**: Automatically launches with your system

---

## ⚠️ Limitations

- **Switch Speed**: Since the software doesn't use DDC/CI control, switching speed is limited by your monitor's input detection time
- **Physical Connections Required**: Each monitor must be physically connected to all PCs you want to switch between
- **USB Dependency**: Requires a USB device (keyboard, mouse, hub) to detect PC switching

---

## Features

### 🖥️ Device Management
- Real-time USB device detection and monitoring
- Easy device selection with search functionality
- Visual feedback for device connection status

### ⚙️ Configuration Options
- Customizable screen-off delay (1-300 seconds)
- Auto-start on system boot
- Persistent settings storage

### 📊 Activity Monitoring
- Real-time activity log
- Device connection/disconnection tracking
- Screen control operation logging

### 🎛️ System Integration
- System tray icon with context menu
- Cross-platform native notifications
- Minimal resource usage

---

## Installation

### Prerequisites
- **Qt6** (Core, Widgets)
- **CMake** 3.16 or higher
- **C++17** compatible compiler

### Platform-Specific Requirements

#### Windows
- Visual Studio 2019+ or MinGW
- Windows 10/11

#### macOS
- Xcode Command Line Tools
- macOS 10.15+
- IOKit framework
- brew

#### Linux
- GCC 9+ or Clang 10+
- X11 development libraries
- udev development libraries

### Building from Source

```bash
# Clone the repository
git clone <repository-url>
cd MonitorSwitch

# Build the project using the provided build scripts
```

#### Linux/macOS
```bash
./build.sh
```

#### Windows
```cmd
build.bat
```

#### Manual Build (Alternative)
If you prefer to build manually:

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make
```

---

## Usage

1. **Launch MonitorSwitch** - The application will start with a system tray icon
2. **Select a USB Device** - Choose a USB device that represents your KVM switch state
   - Keyboards and mice are common choices
   - USB hubs work well for multiple device setups
3. **Configure Settings** - Set your preferred screen-off delay and auto-start preferences
4. **Test the Setup** - Use the "Test Screen Control" button to verify functionality

### Typical Workflow
1. Connect your USB device to PC #1
2. MonitorSwitch detects the device and keeps the screen active
3. Switch your USB device to PC #2 via your KVM
4. MonitorSwitch detects disconnection and turns off the screen after the configured delay
5. Your monitor automatically switches to PC #2's input
6. When you switch back, MonitorSwitch detects reconnection and turns the screen back on

---

## Configuration

### Settings File Location
- **Windows**: `%APPDATA%/aerodomigue/MonitorSwitch/config.ini`
- **macOS**: `~/Library/Application Support/aerodomigue/MonitorSwitch/config.ini`
- **Linux**: `~/.config/aerodomigue/MonitorSwitch/config.ini`

### Configuration Options
```ini
startOnBoot=true
selectedDeviceId=USB_VID_1234&PID_5678
screenOffDelay=10
```

---

## Troubleshooting

### Common Issues

**Screen doesn't turn off**
- Verify the selected USB device is correctly detected
- Check that your system supports display power management
- Try running the application with administrator/root privileges

**Monitor doesn't switch inputs**
- Ensure all video cables are properly connected
- Check your monitor's input priority settings
- Some monitors require a longer delay to detect input changes

**USB device not detected**
- Refresh the device list
- Check USB device permissions (Linux/macOS)
- Verify the device is not being used exclusively by another application

### Platform-Specific Notes

#### Windows
- May require administrator privileges for display control
- Windows Display Driver Model (WDDM) compatibility required

#### macOS
- Requires accessibility permissions for display control
- May need to approve the application in System Preferences > Security & Privacy

#### Linux
- Requires X11 DPMS extension
- May need to be added to appropriate user groups for USB access

---

## Contributing

Contributions are welcome! Please read our contributing guidelines and submit pull requests for any improvements.

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on multiple platforms
5. Submit a pull request

---

## License

This project is licensed under the [MIT License](LICENSE) - see the LICENSE file for details.

---

## Acknowledgments

- Qt Framework for cross-platform GUI capabilities
- Platform-specific APIs: Win32, IOKit, X11/udev
- Community feedback and testing

---

## Support

For issues, feature requests, or questions:
- Create an issue on GitHub
- Check the troubleshooting section above
- Review existing issues for similar problems

---

**MonitorSwitch** - Smart monitor switching without DDC/CI dependency
