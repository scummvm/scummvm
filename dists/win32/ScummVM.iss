#define AppName "ScummVM"
#define FileVersion=GetFileVersion('scummvm.exe')
#define ProductVersion=GetStringFileInfo('scummvm.exe', 'ProductVersion')
#define Copyright=GetStringFileInfo('scummvm.exe', 'LegalCopyright')

[Setup]
AppCopyright={#Copyright}
AppName={#AppName}
AppVersion={#ProductVersion}
VersionInfoVersion={#FileVersion}
AppVerName={#AppName} {#ProductVersion}
AppPublisher=The ScummVM Team
AppPublisherURL=https://www.scummvm.org/
AppSupportURL=https://www.scummvm.org/
AppUpdatesURL=https://www.scummvm.org/
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=true
AlwaysUsePersonalGroup=false
EnableDirDoesntExistWarning=false
Compression=lzma2/ultra
OutputDir=C:\{#AppName}
OutputBaseFilename=scummvm-win32
DisableStartupPrompt=true
AppendDefaultDirName=false
SolidCompression=true
DirExistsWarning=no
SetupIconFile=graphics\scummvm-install.ico
UninstallDisplayIcon={app}\scummvm.exe
WizardImageFile=graphics\left.bmp
WizardSmallImageFile=graphics\scummvm-install.bmp
ShowLanguageDialog=yes
LanguageDetectionMethod=uilanguage
PrivilegesRequired=none
DisableWelcomePage=False


; This privilege escalation code comes from
; https://stackoverflow.com/a/35435534/252087

#define InnoSetupReg \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\" + AppName + "_is1"
#define InnoSetupAppPathReg "Inno Setup: App Path"

[Code]
function IsWinVista: Boolean;
begin
  Result := (GetWindowsVersion >= $06000000);
end;

function IsElevated: Boolean;
begin
  Result := IsAdminLoggedOn or IsPowerUserLoggedOn;
end;

function HaveWriteAccessToApp: Boolean;
var
  FileName: string;
begin
  FileName := AddBackslash(WizardDirValue) + 'writetest.tmp';
  Result := SaveStringToFile(FileName, 'test', False);
  if Result then
  begin
    Log(Format(
      'Have write access to the last installation path [%s]', [WizardDirValue]));
    DeleteFile(FileName);
  end
    else
  begin
    Log(Format('Does not have write access to the last installation path [%s]', [
      WizardDirValue]));
  end;
end;

procedure ExitProcess(uExitCode: UINT);
  external 'ExitProcess@kernel32.dll stdcall';
function ShellExecute(hwnd: HWND; lpOperation: string; lpFile: string;
  lpParameters: string; lpDirectory: string; nShowCmd: Integer): THandle;
  external 'ShellExecuteW@shell32.dll stdcall';

function Elevate: Boolean;
var
  I: Integer;
  RetVal: Integer;
  Params: string;
  S: string;
begin
  { Collect current instance parameters }
  for I := 1 to ParamCount do
  begin
    S := ParamStr(I);
    { Unique log file name for the elevated instance }
    if CompareText(Copy(S, 1, 5), '/LOG=') = 0 then
    begin
      S := S + '-elevated';
    end;
    { Do not pass our /SL5 switch }
    if CompareText(Copy(S, 1, 5), '/SL5=') <> 0 then
    begin
      Params := Params + AddQuotes(S) + ' ';
    end;
  end;

  { ... and add selected language }
  Params := Params + '/LANG=' + ActiveLanguage;

  Log(Format('Elevating setup with parameters [%s]', [Params]));
  RetVal := ShellExecute(0, 'runas', ExpandConstant('{srcexe}'), Params, '', SW_SHOW);
  Log(Format('Running elevated setup returned [%d]', [RetVal]));
  Result := (RetVal > 32);
  { if elevated executing of this setup succeeded, then... }
  if Result then
  begin
    Log('Elevation succeeded');
    { exit this non-elevated setup instance }
    ExitProcess(0);
  end
    else
  begin
    Log(Format('Elevation failed [%s]', [SysErrorMessage(RetVal)]));
  end;
end;

procedure InitializeWizard;
var
  S: string;
  Upgrade: Boolean;
begin
  Upgrade :=
    RegQueryStringValue(HKLM, '{#InnoSetupReg}', '{#InnoSetupAppPathReg}', S) or
    RegQueryStringValue(HKCU, '{#InnoSetupReg}', '{#InnoSetupAppPathReg}', S);

  { elevate }

  if not IsWinVista then
  begin
    Log(Format('This version of Windows [%x] does not support elevation', [
      GetWindowsVersion]));
  end
    else
  if IsElevated then
  begin
    Log('Running elevated');
  end
    else
  begin
    Log('Running non-elevated');
    if Upgrade then
    begin
      if not HaveWriteAccessToApp then
      begin
        Elevate;
      end;
    end
      else
    begin
      if not Elevate then
      begin
        WizardForm.DirEdit.Text := ExpandConstant('{localappdata}\{#AppName}');
        Log(Format('Falling back to local application user folder [%s]', [
          WizardForm.DirEdit.Text]));
      end;
    end;
  end;
end;


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
Name: "{userdesktop}\ScummVM"; Filename: "{app}\scummvm.exe"; Tasks: desktopicon

; Start menu icons
Name: {group}\ScummVM; Filename: {app}\scummvm.exe; Parameters: "--no-console"; WorkingDir: {app}; Comment: ScummVM; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\ScummVM (with console); Filename: {app}\scummvm.exe; WorkingDir: {app}; Comment: ScummVM; Flags: createonlyiffileexists; IconIndex: 0; Languages: not german
Name: {group}\ScummVM (mit Konsolenfenster); Filename: {app}\scummvm.exe; WorkingDir: {app}; Comment: ScummVM; Flags: createonlyiffileexists; IconIndex:0; Languages: german
Name: {group}\Saved Games\Saved Games; Filename: {userappdata}\ScummVM\Saved Games; WorkingDir: {userappdata}\ScummVM\Saved Games; Comment: Saved Games; IconIndex: 0; MinVersion: 0, 1; Languages: not german
Name: {group}\Spielstände\Spielstände; Filename: {userappdata}\ScummVM\Saved Games; WorkingDir: {userappdata}\ScummVM\Saved Games; Comment: Spielstände; IconIndex: 0; MinVersion: 0, 1; Languages: german

[Run]
Filename: {app}\ScummVM.exe; Flags: nowait skipifdoesntexist postinstall skipifsilent

; Creates a symbolic link for standard save games area, under Windows Vista and higher
; Filename: {cmd}; Parameters: "/c ""mklink /d ""%userprofile%\Saved Games\ScummVM"" ""%appdata%\ScummVM\Saved games"" "" "; MinVersion: 0, 6.1

[UninstallDelete]
Type: files; Name: {app}\ISTool.url

[Dirs]
Name: "{userappdata}\ScummVM"; MinVersion: 0, 1

; Create the Saved Games folder and prevent it from being deleted during an uninstall process
Name: "{userappdata}\ScummVM\Saved Games"; MinVersion: 0, 1; Flags: uninsneveruninstall

[Files]
; AUTHORS and LICENSING
Source: AUTHORS.txt; DestDir: {app}
Source: COPYING.txt; DestDir: {app}
Source: COPYRIGHT.txt; DestDir: {app}
Source: LICENSES\*; DestDir: {app}\LICENSES;

; NEWS
Source: NEWS.html; DestDir: {app}; Flags: ignoreversion; Languages: not german
Source: NEUES.html; DestDir: {app}; Flags: ignoreversion; Languages: german

; QUICKSTART
Source: doc/de/Schnellstart.txt; DestDir: {app}; Flags: ignoreversion; Languages: german
Source: doc/es/InicioRapido.txt; DestDir: {app}; Flags: ignoreversion; Languages: spanish
Source: doc/fr/DemarrageRapide.txt; DestDir: {app}; Flags: ignoreversion; Languages: french
Source: doc/it/GuidaRapida.txt; DestDir: {app}; Flags: ignoreversion; Languages: italian
Source: doc/no-nb/HurtigStart.txt; DestDir: {app}; Flags: ignoreversion; Languages: norwegian
Source: doc/se/Snabbstart.txt; DestDir: {app}; Flags: ignoreversion; Languages: swedish

; README and DOCUMENTATION
Source: README.html; DestDir: {app}; Flags: ignoreversion; Languages: not (czech or german or swedish)
Source: doc/cz/PrectiMe.txt; DestDir: {app}; Flags: ignoreversion; Languages: czech
Source: doc/de/LIESMICH.txt; DestDir: {app}; Flags: ignoreversion; Languages: german
Source: doc/se/LasMig.txt; DestDir: {app}; Flags: ignoreversion; Languages: swedish
Source: *Manual*.pdf; DestDir: {app}; Flags: ignoreversion isreadme

; ScummVM executables and libraries
; for 32 bit aka. i686:
Source: scummvm-i686.exe; DestDir: {app}; DestName: scummvm.exe; Flags: sign ignoreversion; Check: not Is64BitInstallMode
Source: SDL2-i686.dll; DestDir: {app}; DestName: SDL2.dll; Flags: replacesameversion; Check: not Is64BitInstallMode
Source: SDL2_net-i686.dll; DestDir: {app}; DestName: SDL2_net.dll; Flags: replacesameversion; Check: not Is64BitInstallMode
Source: WinSparkle-i686.dll; DestDir: {app}; DestName: WinSparkle.dll; Flags: replacesameversion; Check: not Is64BitInstallMode
Source: discord-rpc-i686.dll; DestDir: {app}; DestName: discord-rpc.dll; Flags: replacesameversion; Check: not Is64BitInstallMode

; for 64 bit aka. x86_64
Source: scummvm-x86_64.exe; DestDir: {app}; DestName: scummvm.exe; Flags: sign ignoreversion; Check: Is64BitInstallMode
Source: SDL2-x86_64.dll; DestDir: {app}; DestName: SDL2.dll; Flags: replacesameversion; Check: Is64BitInstallMode
Source: SDL2_net-x86_64.dll; DestDir: {app}; DestName: SDL2_net.dll; Flags: replacesameversion; Check: Is64BitInstallMode
Source: WinSparkle-x86_64.dll; DestDir: {app}; DestName: WinSparkle.dll; Flags: replacesameversion; Check: Is64BitInstallMode
Source: discord-rpc-x86_64.dll; DestDir: {app}; DestName: discord-rpc.dll; Flags: replacesameversion; Check: Is64BitInstallMode
