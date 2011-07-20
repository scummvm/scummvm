[Setup]
AppCopyright=2011
AppName=ScummVM
AppVerName=ScummVM Git
AppPublisher=The ScummVM Team
AppPublisherURL=http://www.scummvm.org/
AppSupportURL=http://www.scummvm.org/
AppUpdatesURL=http://www.scummvm.org/
DefaultDirName={pf}\ScummVM
DefaultGroupName=ScummVM
AllowNoIcons=true
AlwaysUsePersonalGroup=false
EnableDirDoesntExistWarning=false
Compression=lzma
OutputDir=C:\ScummVM
OutputBaseFilename=scummvm-win32
DisableStartupPrompt=true
AppendDefaultDirName=false
SolidCompression=true
DirExistsWarning=no
SetupIconFile=graphics\scummvm-install.ico
WizardImageFile=graphics\left.bmp
ShowLanguageDialog=yes
LanguageDetectionMethod=uilanguage

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: eu; MessagesFile: compiler:Languages\Basque.isl
Name: br; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: ca; MessagesFile: compiler:Languages\Catalan.isl
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: da; MessagesFile: compiler:Languages\Danish.isl
Name: nl; MessagesFile: compiler:Languages\Dutch.isl
Name: fi; MessagesFile: compiler:Languages\Finnish.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: he; MessagesFile: compiler:Languages\Hebrew.isl
Name: hu; MessagesFile: compiler:Languages\Hungarian.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
Name: ja; MessagesFile: compiler:Languages\Japanese.isl
Name: nb; MessagesFile: compiler:Languages\Norwegian.isl
Name: pl; MessagesFile: compiler:Languages\Polish.isl
Name: pt; MessagesFile: compiler:Languages\Portuguese.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: sk; MessagesFile: compiler:Languages\Slovak.isl
Name: sl; MessagesFile: compiler:Languages\Slovenian.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl

[Icons]
Name: {group}\{cm:UninstallProgram, ScummVM}; Filename: {uninstallexe}
Name: {group}\ScummVM; Filename: {app}\scummvm.exe; WorkingDir: {app}; Comment: scummvm; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\ScummVM (noconsole); Filename: {app}\scummvm.exe; Parameters: "--no-console"; WorkingDir: {app}; Comment: scummvm; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\Authors; Filename: {app}\AUTHORS.txt; WorkingDir: {app}; Comment: AUTHORS; Flags: createonlyiffileexists
Name: {group}\Copying; Filename: {app}\COPYING.txt; WorkingDir: {app}; Comment: COPYING; Flags: createonlyiffileexists
Name: {group}\Copying.LGPL; Filename: {app}\COPYING.LGPL.txt; WorkingDir: {app}; Comment: COPYING.LGPL; Flags: createonlyiffileexists
Name: {group}\Copyright; Filename: {app}\COPYRIGHT.txt; WorkingDir: {app}; Comment: COPYRIGHT; Flags: createonlyiffileexists
Name: {group}\Readme; Filename: {app}\README.txt; WorkingDir: {app}; Comment: README; Flags: createonlyiffileexists
Name: {group}\News; Filename: {app}\NEWS.txt; WorkingDir: {app}; Comment: NEWS; Flags: createonlyiffileexists

[Run]
Filename: {app}\scummvm.exe; Flags: nowait skipifdoesntexist postinstall skipifsilent; Description: Launch ScummVM

[UninstallDelete]
Type: files; Name: {app}\ISTool.url

[Files]
Source: AUTHORS.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.LGPL.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYRIGHT.txt; DestDir: {app}; Flags: ignoreversion
Source: NEWS.txt; DestDir: {app}; Flags: ignoreversion
Source: README.txt; DestDir: {app}; Flags: ignoreversion isreadme
Source: README-SDL.txt; DestDir: {app}; Flags: ignoreversion
Source: scummvm.exe; DestDir: {app}; Flags: ignoreversion
Source: SDL.dll; DestDir: {app}
Source: scummclassic.zip; DestDir: {app}; Flags: ignoreversion
Source: scummmodern.zip; DestDir: {app}; Flags: ignoreversion
Source: drascula.dat; DestDir: {app}; Flags: ignoreversion
Source: hugo.dat; DestDir: {app}; Flags: ignoreversion
Source: kyra.dat; DestDir: {app}; Flags: ignoreversion
Source: lure.dat; DestDir: {app}; Flags: ignoreversion
Source: m4.dat; DestDir: {app}; Flags: ignoreversion
Source: pred.dic; DestDir: {app}; Flags: ignoreversion
Source: queen.tbl; DestDir: {app}; Flags: ignoreversion
Source: sky.cpt; DestDir: {app}; Flags: ignoreversion
Source: teenagent.dat; DestDir: {app}; Flags: ignoreversion
Source: toon.dat; DestDir: {app}; Flags: ignoreversion
Source: translations.dat; DestDir: {app}; Flags: ignoreversion
