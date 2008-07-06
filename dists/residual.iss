; -- residual.iss --
; Inno Setup 5 Script for Residual.

[Setup]
AppName=Residual
AppVerName=Residual SVN snapshot version
DefaultDirName={pf}\Residual
DefaultGroupName=Residual
UninstallDisplayIcon={app}\residual.exe
Compression=lzma
SolidCompression=yes
OutputDir=userdocs:Residual_ISS5_Output

[Files]
Source: "AUTHORS.txt"; DestDir: "{app}"
Source: "COPYING_GPL.txt"; DestDir: "{app}"
Source: "COPYING_LGPL.txt"; DestDir: "{app}"
Source: "NEWS.txt"; DestDir: "{app}"
Source: "README.txt"; DestDir: "{app}"
Source: "README-SDL.txt"; DestDir: "{app}"; Flags: isreadme
Source: "residual.exe"; DestDir: "{app}"
Source: "residual.ini.example"; DestDir: "{app}"
Source: "SDL.dll"; DestDir: "{app}"

[Icons]
Name: "{group}\Residual"; Filename: "{app}\residual.exe"
