@echo off
echo Building MonitorSwitch Windows Installer...

REM Check if Inno Setup is installed
if not exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    echo Error: Inno Setup 6 not found!
    echo Please download and install Inno Setup from: https://jrsoftware.org/isdl.php
    echo Install it to the default location: C:\Program Files (x86)\Inno Setup 6\
    pause
    exit /b 1
)

REM Check if deploy directory exists
if not exist "deploy\" (
    echo Error: deploy\ directory not found!
    echo Please run the Windows build first to create the deployment files.
    echo You can do this by running: 
    echo   mkdir build ^&^& cd build
    echo   cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
    echo   cmake --build . --config Release
    echo   Then run the deploy script
    pause
    exit /b 1
)

REM Create installer output directory
if not exist "installer_output\" mkdir installer_output

REM Run Inno Setup Compiler
echo Compiling installer...
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" MonitorSwitch.iss

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✓ Installer created successfully!
    echo Output: installer_output\MonitorSwitch-Setup.exe
    echo.
    echo The installer can be distributed without requiring administrator privileges.
    echo It will install MonitorSwitch to %%LOCALAPPDATA%%\MonitorSwitch
) else (
    echo.
    echo ✗ Failed to create installer!
    echo Check the Inno Setup output above for errors.
)

pause
