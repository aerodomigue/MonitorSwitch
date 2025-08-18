# MonitorSwitch Windows Installer

## Overview

MonitorSwitch provides two distribution options for Windows:

1. **Portable Version** - A ZIP file containing all necessary files that can be run from any location
2. **Installer** - A user-friendly installer that doesn't require administrator privileges

## Installer Features

- ✅ **No Administrator Rights Required** - Installs to user's local directory (`%LOCALAPPDATA%\MonitorSwitch`)
- ✅ **Start Menu Integration** - Creates shortcuts in the Start Menu
- ✅ **Desktop Icon** - Optional desktop shortcut
- ✅ **Auto-start Option** - Optionally start with Windows
- ✅ **Clean Uninstall** - Proper removal through Windows Programs & Features
- ✅ **Upgrade Support** - Handles updates cleanly

## Creating the Installer

### Prerequisites

1. **Inno Setup 6** - Download from [https://jrsoftware.org/isdl.php](https://jrsoftware.org/isdl.php)
   - Install to default location: `C:\Program Files (x86)\Inno Setup 6\`

2. **Visual Studio 2022** (or Build Tools) - For compiling the application

3. **Qt 6.5.0** - For development and windeployqt

### Manual Build

1. **Build the application:**
   ```batch
   build_windows_installer.bat
   ```

2. **Or step by step:**
   ```batch
   # Build
   mkdir build
   cd build
   cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release
   cd ..
   
   # Deploy Qt libraries
   mkdir deploy
   copy build\Release\MonitorSwitch.exe deploy\
   windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw deploy\MonitorSwitch.exe
   
   # Copy icons
   mkdir deploy\icons
   copy icons\* deploy\icons\
   
   # Create installer
   create_installer.bat
   ```

### Automatic Build (GitHub Actions)

The installer is automatically created when you push a tag or create a release:

1. Push a tag: `git tag v1.0.0 && git push origin v1.0.0`
2. GitHub Actions will build and create both:
   - `MonitorSwitch-Windows-Portable.zip` - Portable version
   - `MonitorSwitch-Setup.exe` - Installer

## Installation Locations

- **User Installation**: `%LOCALAPPDATA%\MonitorSwitch` (e.g., `C:\Users\YourName\AppData\Local\MonitorSwitch`)
- **Start Menu**: `Start Menu\Programs\MonitorSwitch`
- **Desktop**: Optional desktop shortcut
- **Autostart**: Optionally added to `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`

## Installer Configuration

The installer behavior is configured in `MonitorSwitch.iss`:

```ini
; Install location (no admin required)
DefaultDirName={localappdata}\MonitorSwitch

; User privileges (no admin required)  
PrivilegesRequired=lowest

; Optional features
Tasks:
- Desktop icon (unchecked by default)
- Quick launch icon (Windows 7/Vista only)
- Auto-start with Windows
```

## Distribution

The installer (`MonitorSwitch-Setup.exe`) can be distributed freely:

- ✅ No code signing required (users will see a SmartScreen warning on first run)
- ✅ Works on Windows 10 and 11
- ✅ x64 architecture only
- ✅ Self-contained (includes all Qt libraries)

## Troubleshooting

### "Windows protected your PC" SmartScreen warning

This is normal for unsigned installers. Users can click "More info" → "Run anyway".

To avoid this warning, you would need to:
1. Get a code signing certificate ($200-400/year)
2. Sign the installer with `signtool.exe`

### Installer compilation fails

1. Verify Inno Setup is installed at `C:\Program Files (x86)\Inno Setup 6\`
2. Ensure the `deploy\` directory exists with all files
3. Check that `LICENSE` and `README.md` files exist in the project root

### Application doesn't start after installation

1. Check Windows Event Viewer for error details
2. Ensure all Qt libraries were deployed correctly
3. Verify icons directory is present

## File Structure

After installation, the directory structure is:

```
%LOCALAPPDATA%\MonitorSwitch\
├── MonitorSwitch.exe           # Main application
├── *.dll                       # Qt libraries
├── platforms\                  # Qt platform plugins
├── styles\                     # Qt styles
├── icons\                      # Application icons
├── README.md                   # Documentation
└── LICENSE                     # License file
```
