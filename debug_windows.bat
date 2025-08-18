@echo off
echo MonitorSwitch Windows Diagnostic Tool
echo =====================================
echo.

echo 1. Checking for running MonitorSwitch processes...
tasklist /FI "IMAGENAME eq MonitorSwitch.exe" 2>nul | find "MonitorSwitch.exe" >nul
if %errorlevel%==0 (
    echo WARNING: MonitorSwitch.exe processes found:
    tasklist /FI "IMAGENAME eq MonitorSwitch.exe"
    echo.
    echo To kill them, run: taskkill /F /IM MonitorSwitch.exe
    echo.
) else (
    echo No MonitorSwitch.exe processes found.
)

tasklist /FI "IMAGENAME eq MonitorSwitch_debug.exe" 2>nul | find "MonitorSwitch_debug.exe" >nul
if %errorlevel%==0 (
    echo WARNING: MonitorSwitch_debug.exe processes found:
    tasklist /FI "IMAGENAME eq MonitorSwitch_debug.exe"
    echo.
    echo To kill them, run: taskkill /F /IM MonitorSwitch_debug.exe
    echo.
) else (
    echo No MonitorSwitch_debug.exe processes found.
)

echo 2. Checking for debug log file...
set LOGFILE=%TEMP%\MonitorSwitch_debug.log
if exist "%LOGFILE%" (
    echo Debug log file found: %LOGFILE%
    echo File size: 
    dir "%LOGFILE%" | find ".log"
    echo.
    echo Last 10 lines of log:
    echo ---------------------------
    powershell "Get-Content '%LOGFILE%' | Select-Object -Last 10"
    echo ---------------------------
    echo.
    echo To view full log: notepad "%LOGFILE%"
) else (
    echo No debug log file found at %LOGFILE%
)

echo.
echo 3. System information:
echo OS: %OS%
echo Processor: %PROCESSOR_ARCHITECTURE%
echo Temp directory: %TEMP%

echo.
echo 4. Instructions:
echo - If you see running processes above, kill them first
echo - Run MonitorSwitch_debug.exe from command line to see real-time output
echo - Check the debug log file for detailed information
echo - If app won't start, try running as administrator

echo.
pause
