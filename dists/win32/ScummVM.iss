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
Name: br; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: ca; MessagesFile: compiler:Languages\Catalan.isl
Name: cz; MessagesFile: compiler:Languages\Czech.isl
Name: da; MessagesFile: compiler:Languages\Danish.isl
Name: fr; MessagesFile: compiler:Languages\French.isl
Name: de; MessagesFile: compiler:Languages\German.isl
Name: hu; MessagesFile: compiler:Languages\Hungarian.isl
Name: it; MessagesFile: compiler:Languages\Italian.isl
Name: nb; MessagesFile: compiler:Languages\Norwegian.isl
Name: pl; MessagesFile: compiler:Languages\Polish.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl

[Icons]
Name: {group}\{cm:UninstallProgram, ScummVM}; Filename: {uninstallexe}
Name: {group}\ScummVM; Filename: {app}\scummvm.exe; WorkingDir: {app}; Comment: scummvm; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\ScummVM (noconsole); Filename: {app}\scummvm.exe; Parameters: "--no-console"; WorkingDir: {app}; Comment: scummvm; Flags: createonlyiffileexists; IconIndex: 0
Name: {group}\Authors; Filename: {app}\AUTHORS.txt; WorkingDir: {app}; Comment: AUTHORS; Flags: createonlyiffileexists
Name: {group}\Copying; Filename: {app}\COPYING.txt; WorkingDir: {app}; Comment: COPYING; Flags: createonlyiffileexists
Name: {group}\Copying.LGPL; Filename: {app}\COPYING.LGPL.txt; WorkingDir: {app}; Comment: COPYING.LGPL; Flags: createonlyiffileexists
Name: {group}\Copyright; Filename: {app}\COPYRIGHT.txt; WorkingDir: {app}; Comment: COPYRIGHT; Flags: createonlyiffileexists
;NEWS
Name: {group}\News; Filename: {app}\NEWS.txt; WorkingDir: {app}; Comment: NEWS; Flags: createonlyiffileexists; Languages: not de
Name: {group}\Neues; Filename: {app}\Neues.txt; WorkingDir: {app}; Comment: Neues; Flags: createonlyiffileexists; Languages: de
;QUICKSTART
Name: {group}\QuickStart; Filename: {app}\QUICKSTART.txt; WorkingDir: {app}; Comment: QUICKSTART; Flags: createonlyiffileexists; Languages: not (de or es or fr or it or nb)
Name: {group}\Schnellstart; Filename: {app}\Schnellstart.txt; WorkingDir: {app}; Comment: Schnellstart; Flags: createonlyiffileexists; Languages: de
Name: {group}\InicioRapido; Filename: {app}\InicioRapido.txt; WorkingDir: {app}; Comment: InicioRapido; Flags: createonlyiffileexists; Languages: es
Name: {group}\DemarrageRapide; Filename: {app}\DemarrageRapide.txt; WorkingDir: {app}; Comment: DemarrageRapide; Flags: createonlyiffileexists; Languages: fr
Name: {group}\GuidaRapida; Filename: {app}\GuidaRapida.txt; WorkingDir: {app}; Comment: GuidaRapida; Flags: createonlyiffileexists; Languages: it
Name: {group}\HurtigStart; Filename: {app}\HurtigStart.txt; WorkingDir: {app}; Comment: HurtigStart; Flags: createonlyiffileexists; Languages: nb
;README
Name: {group}\Readme; Filename: {app}\README.txt; WorkingDir: {app}; Comment: README; Flags: createonlyiffileexists; Languages: not (cz or de)
Name: {group}\PrectiMe; Filename: {app}\PrectiMe.txt; WorkingDir: {app}; Comment: PrectiMe; Flags: createonlyiffileexists; Languages: cz
Name: {group}\Liesmich; Filename: {app}\Liesmich.txt; WorkingDir: {app}; Comment: Liesmich; Flags: createonlyiffileexists; Languages: de

[Run]
Filename: {app}\ScummVM.exe; Flags: nowait skipifdoesntexist postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\ISTool.url

[Files]
Source: AUTHORS.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYING.LGPL.txt; DestDir: {app}; Flags: ignoreversion
Source: COPYRIGHT.txt; DestDir: {app}; Flags: ignoreversion
;NEWS
Source: NEWS.txt; DestDir: {app}; Flags: ignoreversion; Languages: not de
Source: doc/de/Neues.txt; DestDir: {app}; Flags: ignoreversion; Languages: de
;QUICKSTART
Source: doc/QUICKSTART.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: not (de or es or fr or it or nb)
Source: doc/de/Schnellstart.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: de
Source: doc/es/InicioRapido.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: es
Source: doc/fr/DemarrageRapide.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: fr
Source: doc/it/GuidaRapida.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: it
Source: doc/no-nb/HurtigStart.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: nb
;README
Source: README.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: not (cz or de)
Source: doc/cz/PrectiMe.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: cz
Source: doc/de/Liesmich.txt; DestDir: {app}; Flags: ignoreversion isreadme; Languages: de
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
