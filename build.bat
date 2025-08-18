@echo off
REM Build script for MonitorSwitch application

echo Building MonitorSwitch Application...
echo ================================

REM Check if build directory exists, create if not
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

REM Configure with CMake
echo Configuring with CMake...
cmake -G "Visual Studio 16 2019" -A x64 ..
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b %ERRORLEVEL%
)

REM Build the project
echo Building project...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Build completed successfully!
echo Executable location: build\Release\MonitorSwitch.exe
echo.
pause
