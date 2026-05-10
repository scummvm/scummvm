; Get the version information from the ScummVM executable.
; Since we always bundle the 32 bit and 64 bit version of the same
; version in one installer, we just use the 32 bit version here
; for simplicity.
#define AppName "ScummVM"
#define FileVersion=GetFileVersion('..\..\win32dist-mingw\scummvm-i686.exe')
#define ProductVersion=GetStringFileInfo('..\..\win32dist-mingw\scummvm-i686.exe', 'ProductVersion')
#define Copyright=GetStringFileInfo('..\..\win32dist-mingw\scummvm-i686.exe', 'LegalCopyright')

[Setup]
AppCopyright={#Copyright}
AppName={#AppName}
AppVersion={#ProductVersion}
VersionInfoVersion={#FileVersion}
AppVerName={#AppName} {#ProductVersion}
VersionInfoCompany=The ScummVM Team
AppPublisher=The ScummVM Team
AppPublisherURL=https://www.scummvm.org/
AppSupportURL=https://www.scummvm.org/
AppUpdatesURL=https://www.scummvm.org/
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#AppName}
EnableDirDoesntExistWarning=false
OutputDir=C:\{#AppName}
OutputBaseFilename=scummvm-win32
DisableStartupPrompt=true
AppendDefaultDirName=false
DirExistsWarning=no
DisableDirPage=no
SetupIconFile=graphics\scummvm-install.ico
UninstallDisplayIcon={app}\scummvm.exe
WizardImageFile=graphics\left.bmp
WizardSmallImageFile=graphics\scummvm-install.bmp
ShowLanguageDialog=yes
LanguageDetectionMethod=uilanguage
DisableWelcomePage=False
ArchitecturesInstallIn64BitMode=x64
AlwaysShowGroupOnReadyPage=True
AlwaysShowDirOnReadyPage=True

SolidCompression=true
Compression=lzma2/ultra64
CompressionThreads=2
LZMANumBlockThreads=4
LZMADictionarySize=524288
LZMAUseSeparateProcess=yes

[Languages]
Name: english; MessagesFile: compiler:Default.isl
Name: brazilian; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: catalan; MessagesFile: compiler:Languages\Catalan.isl
Name: czech; MessagesFile: compiler:Languages\Czech.isl
Name: danish; MessagesFile: compiler:Languages\Danish.isl
Name: french; MessagesFile: compiler:Languages\French.isl
Name: german; MessagesFile: compiler:Languages\German.isl
Name: hungarian; MessagesFile: compiler:Languages\Hungarian.isl
Name: italian; MessagesFile: compiler:Languages\Italian.isl
Name: norwegian; MessagesFile: compiler:Languages\Norwegian.isl
Name: polish; MessagesFile: compiler:Languages\Polish.isl
Name: russian; MessagesFile: compiler:Languages\Russian.isl
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl
Name: swedish; MessagesFile: compiler:Languages\Swedish.isl
Name: basque; MessagesFile: compiler:Languages\Basque.isl
Name: belarusian; MessagesFile: compiler:Languages\Belarusian.isl
Name: dutch; MessagesFile: compiler:Languages\Dutch.isl
Name: finnish; MessagesFile: compiler:Languages\Finnish.isl
Name: galician; MessagesFile: compiler:Languages\Galician.isl
Name: greek; MessagesFile: compiler:Languages\Greek.isl
Name: portuguese; MessagesFile: compiler:Languages\Portuguese.isl
Name: ukrainian; MessagesFile: compiler:Languages\Ukrainian.isl
Name: hebrew; MessagesFile: compiler:Languages\Hebrew.isl

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
; Optional desktop icon
Name: "{userdesktop}\ScummVM"; Filename: "{app}\scummvm.exe"; Parameters: "--no-console"; Tasks: desktopicon

; Start menu icons
Name: {group}\ScummVM; Filename: {app}\scummvm.exe; Parameters: "--no-console"; WorkingDir: {app}; Comment: ScummVM; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\ScummVM (with console); Filename: {app}\scummvm.exe; WorkingDir: {app}; Comment: ScummVM; Flags: createonlyiffileexists; IconIndex: 0; Languages: not german
Name: {group}\ScummVM (mit Konsolenfenster); Filename: {app}\scummvm.exe; WorkingDir: {app}; Comment: ScummVM; Flags: createonlyiffileexists; IconIndex:0; Languages: german
Name: {group}\Saved Games\Saved Games; Filename: {userappdata}\ScummVM\Saved Games; WorkingDir: {userappdata}\ScummVM\Saved Games; Comment: Saved Games; IconIndex: 0; MinVersion: 0, 1; Languages: not german
Name: {group}\Spielstände\Spielstände; Filename: {userappdata}\ScummVM\Saved Games; WorkingDir: {userappdata}\ScummVM\Saved Games; Comment: Spielstände; IconIndex: 0; MinVersion: 0, 1; Languages: german

[Run]
Filename: {app}\ScummVM.exe; Parameters: "--no-console"; Flags: nowait skipifdoesntexist postinstall skipifsilent

; Creates a symbolic link for standard save games area, under Windows Vista and higher
; Filename: {cmd}; Parameters: "/c ""mklink /d ""%userprofile%\Saved Games\ScummVM"" ""%appdata%\ScummVM\Saved games"" "" "; MinVersion: 0, 6.1

[InstallDelete]
Type: files; Name: {app}\SDL2.dll
Type: files; Name: {app}\SDL2_net.dll
Type: files; Name: {app}\discord-rpc.dll
Type: files; Name: {app}\LICENSES\*
Type: files; Name: {app}\*.html
Type: files; Name: {app}\*.txt
Type: files; Name: {app}\*.pdf
Type: dirifempty; Name: {app}\LICENSES

[UninstallDelete]
Type: files; Name: {app}\ISTool.url

[Dirs]
Name: "{userappdata}\ScummVM"; MinVersion: 0, 1

; Create the Saved Games folder and prevent it from being deleted during an uninstall process
Name: "{userappdata}\ScummVM\Saved Games"; MinVersion: 0, 1; Flags: uninsneveruninstall

[Files]
; NEWS, README, AUTHORS, LICENSES...
Source: ..\..\win32dist-mingw\*.txt; DestDir: {app}

; QUICKSTART
Source: ..\..\win32dist-mingw\doc\de\Schnellstart.txt; DestDir: {app}; Flags: ignoreversion; Languages: german
Source: ..\..\win32dist-mingw\doc\es\InicioRapido.txt; DestDir: {app}; Flags: ignoreversion; Languages: spanish
Source: ..\..\win32dist-mingw\doc\fr\DemarrageRapide.txt; DestDir: {app}; Flags: ignoreversion; Languages: french
Source: ..\..\win32dist-mingw\doc\it\GuidaRapida.txt; DestDir: {app}; Flags: ignoreversion; Languages: italian
Source: ..\..\win32dist-mingw\doc\no-nb\HurtigStart.txt; DestDir: {app}; Flags: ignoreversion; Languages: norwegian
Source: ..\..\win32dist-mingw\doc\sv\Snabbstart.txt; DestDir: {app}; Flags: ignoreversion; Languages: swedish

; README and DOCUMENTATION
Source: ..\..\win32dist-mingw\doc\cz\PrectiMe.txt; DestDir: {app}; Flags: ignoreversion; Languages: czech
Source: ..\..\win32dist-mingw\doc\de\LIESMICH.txt; DestDir: {app}; Flags: ignoreversion; Languages: german
Source: ..\..\win32dist-mingw\doc\sv\LasMig.txt; DestDir: {app}; Flags: ignoreversion; Languages: swedish

; ScummVM executables and libraries
; for 32 bit aka. i686:
Source: ..\..\win32dist-mingw\scummvm-i686.exe; DestDir: {app}; DestName: scummvm.exe; Flags: sign ignoreversion; Check: not Is64BitInstallMode
Source: ..\..\win32dist-mingw\WinSparkle-i686.dll; DestDir: {app}; DestName: WinSparkle.dll; Flags: replacesameversion; Check: not Is64BitInstallMode

; for 64 bit aka. x86_64
Source: ..\..\win32dist-mingw\scummvm-x86_64.exe; DestDir: {app}; DestName: scummvm.exe; Flags: sign ignoreversion; Check: Is64BitInstallMode
Source: ..\..\win32dist-mingw\WinSparkle-x86_64.dll; DestDir: {app}; DestName: WinSparkle.dll; Flags: replacesameversion; Check: Is64BitInstallMode
