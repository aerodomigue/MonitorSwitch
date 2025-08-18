@echo off
echo MonitorSwitch Windows Build and Installer Creation
echo ==================================================

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    pause
    exit /b 1
)

echo 1. Building the application...
REM Create build directory
if not exist "build\" mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
if %ERRORLEVEL% NEQ 0 (
    echo Failed to configure project
    cd ..
    pause
    exit /b 1
)

REM Build the project
echo Building...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo Failed to build project
    cd ..
    pause
    exit /b 1
)

cd ..

echo 2. Deploying Qt libraries...
REM Create deployment directory
if exist "deploy\" rmdir /s /q deploy
mkdir deploy

REM Copy executable
copy build\Release\MonitorSwitch.exe deploy\
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy executable
    pause
    exit /b 1
)

REM Deploy Qt libraries
echo Running windeployqt...
windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw deploy\MonitorSwitch.exe
if %ERRORLEVEL% NEQ 0 (
    echo Failed to deploy Qt libraries
    pause
    exit /b 1
)

REM Copy icons
mkdir deploy\icons
copy icons\MonitorSwitch.ico deploy\icons\
copy icons\MonitorSwitch.png deploy\icons\
copy icons\MonitorSwitch_icon.svg deploy\icons\

echo 3. Creating installer...
call create_installer.bat

echo.
echo Build and installer creation complete!
echo - Portable version: deploy\
echo - Installer: installer_output\MonitorSwitch-Setup.exe
