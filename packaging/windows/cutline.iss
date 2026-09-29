; Cutline installer (Inno Setup 6). Build: iscc /DMyAppVersion=1.0.0 packaging\windows\cutline.iss
; Expects the portable build in dist\Cutline (see build.bat / release workflow).
#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif

[Setup]
AppId={{B7D0C3E2-5A41-4C7E-9B61-3F2A8E6D1C90}
AppName=Cutline
AppVersion={#MyAppVersion}
AppPublisher=Cutline
DefaultDirName={autopf}\Cutline
DefaultGroupName=Cutline
UninstallDisplayIcon={app}\Cutline.exe
SetupIconFile=..\..\assets\icon.ico
OutputDir=..\..\dist
OutputBaseFilename=Cutline-windows-x64-setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ChangesAssociations=yes
DisableProgramGroupPage=yes
DisableDirPage=no
UsePreviousAppDir=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "openwith"; Description: "Show Cutline in the ""Open with"" menu of video files (recommended)"; GroupDescription: "Video player:"
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked

[Files]
Source: "..\..\dist\Cutline\*"; DestDir: "{app}"; Flags: recursesubdirs ignoreversion

[Icons]
Name: "{autoprograms}\Cutline"; Filename: "{app}\Cutline.exe"
Name: "{autodesktop}\Cutline"; Filename: "{app}\Cutline.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Cutline.exe"; Description: "Start Cutline"; Flags: nowait postinstall skipifsilent

[Registry]
; Program identity (shows up in "Open with" and in Default apps)
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "Cutline"; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\DefaultIcon"; ValueType: string; ValueData: "{app}\Cutline.exe,0"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\shell\open\command"; ValueType: string; ValueData: """{app}\Cutline.exe"" ""%1"""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Cutline.Video"; ValueType: string; ValueData: "Video"; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Cutline.Video\DefaultIcon"; ValueType: string; ValueData: "{app}\Cutline.exe,0"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Cutline.Video\shell\open\command"; ValueType: string; ValueData: """{app}\Cutline.exe"" ""%1"""; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities"; ValueType: string; ValueName: "ApplicationName"; ValueData: "Cutline"; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities"; ValueType: string; ValueName: "ApplicationDescription"; ValueData: "Simple, fast video editor and player"; Tasks: openwith
Root: HKA; Subkey: "Software\RegisteredApplications"; ValueType: string; ValueName: "Cutline"; ValueData: "Software\Cutline\Capabilities"; Flags: uninsdeletevalue; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".mp4"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.mp4\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".mp4"; ValueData: "Cutline.Video"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".mov"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.mov\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".mov"; ValueData: "Cutline.Video"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".mkv"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.mkv\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".mkv"; ValueData: "Cutline.Video"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".m4v"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.m4v\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".m4v"; ValueData: "Cutline.Video"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".avi"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.avi\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".avi"; ValueData: "Cutline.Video"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".webm"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.webm\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".webm"; ValueData: "Cutline.Video"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".wmv"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.wmv\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".wmv"; ValueData: "Cutline.Video"; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\Applications\Cutline.exe\SupportedTypes"; ValueType: string; ValueName: ".flv"; ValueData: ""; Tasks: openwith
Root: HKA; Subkey: "Software\Classes\.flv\OpenWithList\Cutline.exe"; ValueType: none; Flags: uninsdeletekey; Tasks: openwith
Root: HKA; Subkey: "Software\Cutline\Capabilities\FileAssociations"; ValueType: string; ValueName: ".flv"; ValueData: "Cutline.Video"; Tasks: openwith
