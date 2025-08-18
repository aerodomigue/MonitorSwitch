; MonitorSwitch Installer Script for Inno Setup
; This creates a Windows installer that doesn't require administrator privileges

[Setup]
AppName=MonitorSwitch
AppVersion=1.0.0
AppPublisher=MonitorSwitch Team
AppPublisherURL=https://github.com/aerodomigue/MonitorSwitch
AppSupportURL=https://github.com/aerodomigue/MonitorSwitch/issues
AppUpdatesURL=https://github.com/aerodomigue/MonitorSwitch/releases
DefaultDirName={localappdata}\MonitorSwitch
DefaultGroupName=MonitorSwitch
AllowNoIcons=yes
LicenseFile=LICENSE
InfoBeforeFile=README.md
OutputDir=installer_output
OutputBaseFilename=MonitorSwitch-Setup
SetupIconFile=icons\MonitorSwitch.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode
Name: "autostart"; Description: "Start MonitorSwitch automatically with Windows"; GroupDescription: "Startup Options"

[Files]
; Main executable
Source: "deploy\MonitorSwitch.exe"; DestDir: "{app}"; Flags: ignoreversion

; Qt libraries (deployed by windeployqt)
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "MonitorSwitch.exe"

; Icons
Source: "icons\*"; DestDir: "{app}\icons"; Flags: ignoreversion recursesubdirs createallsubdirs

; Documentation
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion isreadme

[Icons]
Name: "{group}\MonitorSwitch"; Filename: "{app}\MonitorSwitch.exe"; IconFilename: "{app}\icons\MonitorSwitch.ico"
Name: "{group}\{cm:UninstallProgram,MonitorSwitch}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\MonitorSwitch"; Filename: "{app}\MonitorSwitch.exe"; IconFilename: "{app}\icons\MonitorSwitch.ico"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\MonitorSwitch"; Filename: "{app}\MonitorSwitch.exe"; IconFilename: "{app}\icons\MonitorSwitch.ico"; Tasks: quicklaunchicon

[Registry]
; Add to Windows startup if requested
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "MonitorSwitch"; ValueData: """{app}\MonitorSwitch.exe"""; Tasks: autostart

[Run]
Filename: "{app}\MonitorSwitch.exe"; Description: "{cm:LaunchProgram,MonitorSwitch}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
function IsUpgrade: Boolean;
begin
  Result := RegKeyExists(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppId")}_is1');
end;

function InitializeSetup: Boolean;
begin
  Result := True;
  
  // Check if upgrade and ask user
  if IsUpgrade then
  begin
    if MsgBox('A previous version of MonitorSwitch is installed. Do you want to upgrade?', mbConfirmation, MB_YESNO) = IDNO then
      Result := False;
  end;
end;
