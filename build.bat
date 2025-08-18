@echo off
REM Build script for MonitorSwitch application

echo Building MonitorSwitch Application...
echo ================================

REM Check if Qt is installed
echo Checking for Qt installation...
where qmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Qt not found in PATH. Installing Qt via chocolatey...
    
    REM Check if chocolatey is installed
    where choco >nul 2>&1
    if %ERRORLEVEL% neq 0 (
        echo Installing Chocolatey...
        powershell -Command "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))"
        if %ERRORLEVEL% neq 0 (
            echo ERROR: Failed to install Chocolatey
            pause
            exit /b %ERRORLEVEL%
        )
        
        REM Refresh environment variables
        call refreshenv
    )
    
    echo Installing Qt6 via Chocolatey...
    choco install qt6 -y
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Failed to install Qt6
        pause
        exit /b %ERRORLEVEL%
    )
    
    REM Refresh environment variables
    call refreshenv
)

REM Check if build directory exists, create if not
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

REM Configure with CMake
echo Configuring with CMake...

REM Try to find Qt installation path
for /f "delims=" %%i in ('where qmake 2^>nul') do set "QMAKE_PATH=%%i"
if defined QMAKE_PATH (
    for %%i in ("%QMAKE_PATH%") do set "QT_DIR=%%~dpi"
    set "QT_DIR=%QT_DIR:~0,-5%"
    echo Found Qt at: %QT_DIR%
    cmake -A x64 -DCMAKE_PREFIX_PATH="%QT_DIR%" ..
) else (
    echo Using default Qt paths...
    cmake -A x64 ..
)

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
