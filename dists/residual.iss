; -- residual.iss --
; Inno Setup 5 Script for Residual.

[Setup]
AppName=Residual
AppVerName=Residual SVN
AllowNoIcons=true
Compression=lzma
DefaultDirName={pf}\Residual
DefaultGroupName=Residual
SolidCompression=yes
OutputDir=userdocs:Residual_ISS5_Output
OutputBaseFilename=residual-win32
UninstallDisplayIcon={app}\residual.exe

[Files]
Source: "AUTHORS.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "COPYING_GPL.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "COPYING_LGPL.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "NEWS.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "README-SDL.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "residual.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "residual.ini.example"; DestDir: "{app}"; Flags: ignoreversion
Source: "SDL.dll"; DestDir: "{app}"

[Icons]
Name: {group}\{cm:UninstallProgram, Residual}; Filename: {uninstallexe}
Name: {group}\residual; Filename: {app}\residual.exe; WorkingDir: {app}; Comment: residual; Flags: createonlyiffileexists
Name: {group}\Authors; Filename: {app}\AUTHORS.txt; WorkingDir: {app}; Comment: AUTHORS; Flags: createonlyiffileexists
Name: {group}\Copying_GPL; Filename: {app}\COPYING_GPL.txt; WorkingDir: {app}; Comment: COPYING_GPL; Flags: createonlyiffileexists
Name: {group}\Copying_LGPL; Filename: {app}\COPYING_LGPL.txt; WorkingDir: {app}; Comment: COPYING_LGPL; Flags: createonlyiffileexists
Name: {group}\Readme; Filename: {app}\README.txt; WorkingDir: {app}; Comment: README; Flags: createonlyiffileexists
Name: {group}\News; Filename: {app}\NEWS.txt; WorkingDir: {app}; Comment: NEWS; Flags: createonlyiffileexists

[Run]
Filename: {app}\residual.exe; Description: Launch Residual; Flags: nowait skipifdoesntexist postinstall skipifsilent
