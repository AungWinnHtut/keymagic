[Files]
Source: KeyMagic.exe; DestDir: {app}; Flags: uninsrestartdelete 64bit; 
Source: .\..\KeyMagicMM3.ini; DestDir: {app}; DestName: KeyMagic.ini;
Source: KeyMagicDll.dll; DestDir: {app}; Flags: uninsrestartdelete 64bit; 
Source: .\..\SetElevatedStartupTask.exe; DestDir: {app}; 
Source: .\..\Keyboards\Myanmar3.km2; DestDir: {app}\Keyboards; 
Source: .\..\mm3.ttf; DestDir: {fonts}; FontInstall: Myanmar3; Flags: uninsneveruninstall onlyifdoesntexist; 
Source: .\..\Parabaik_UTN11-3.ttf; DestDir: {fonts}; FontInstall: Parabaik; Flags: uninsneveruninstall onlyifdoesntexist;
Source: .\..\Yunghkio.ttf; DestDir: {fonts}; FontInstall: Yungkio; Flags: uninsneveruninstall onlyifdoesntexist;

[Setup]
AppCopyright=Thant Thet Khin Zaw
AppName=KeyMagic
AppVerName=1.4
DefaultDirName={pf}\KeyMagic
DefaultGroupName=KeyMagic
PrivilegesRequired=poweruser
SetupLogging=true
AppID={{8E606C1C-7BA9-44CA-AE60-D417A21C6AB6}
SolidCompression=true
Compression=lzma/Ultra64
InternalCompressLevel=Ultra64
UninstallLogMode=new
OutputBaseFilename=KeyMagic-1.4-x64-MM3+Fonts
VersionInfoVersion=1.4
VersionInfoCompany=Thant Thet Khin Zaw
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64
ShowTasksTreeLines=false

[Run]
Filename: {app}\SetElevatedStartupTask.exe; WorkingDir: {app}; Flags: RunHidden 32bit; MinVersion: 0,6.0.6000; 
Filename: {app}\KeyMagic.exe; WorkingDir: {app}; Flags: NoWait; Tasks: runkm; 

[Registry]
Root: HKCU; SubKey: Software\Microsoft\Windows\Run; ValueType: string; ValueName: KeyMagic; ValueData: {app}\KeyMagic; Flags: UninsDeleteValue; OnlyBelowVersion: 0,6.0.6000; 

[UninstallRun]
MinVersion: 0,6.0.6000; Filename: {app}\SetElevatedStartupTask.exe; Parameters: -d; WorkingDir: {app}; Flags: RunHidden 32bit;
Filename: {app}\KeyMagic.exe; Parameters: -u; WorkingDir: {app}; 

[Tasks]
Name: runkm; Description: "Run KeyMagic after installation is completed"; 

[Icons]
Name: {group}\KeyMagic; Filename: {app}\KeyMagic.exe; IconFilename: {app}\KeyMagic.exe; IconIndex: 0; 
Name: {group}\Uninstall; Filename: {uninstallexe};
Name: "{group}\{cm:UninstallProgram, KeyMagic}"; Filename: {uninstallexe};
