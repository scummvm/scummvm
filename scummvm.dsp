# Microsoft Developer Studio Project File - Name="scummvm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=scummvm - Win32 mad mp3 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scummvm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scummvm.mak" CFG="scummvm - Win32 mad mp3 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scummvm - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "scummvm - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "scummvm - Win32 mad mp3 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "scummvm - Win32 mad mp3 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scummvm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "scummvm___Release"
# PROP Intermediate_Dir "scummvm___Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /Ob2 /I "." /I "sound" /I "common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib simon___Release\simon.lib scumm___Release\scumm.lib sky___Release\sky.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "scummvm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "scummvm___Debug"
# PROP Intermediate_Dir "scummvm___Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "sound" /I "common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ALLOW_GDI" /D "BYPASS_COPY_PROT" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib simon___Debug\simon.lib scumm___Debug\scumm.lib sky___Debug\sky.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept /fixed:no
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "scummvm - Win32 mad mp3 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "scummvm___Win32_mad mp3_Debug"
# PROP BASE Intermediate_Dir "scummvm___Win32_mad mp3_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "scummvm___mad_mp3_Debug"
# PROP Intermediate_Dir "scummvm___mad_mp3_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "./sound" /I "./" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ALLOW_GDI" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /D "USE_MAD" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "sound" /I "common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ALLOW_GDI" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib libmad.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib libmad.lib wsock32.lib scumm___mad_mp3_Debug\scumm.lib simon___mad_mp3_Debug\simon.lib sky___mad_mp3_Debug\sky.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /pdbtype:sept /fixed:no
# SUBTRACT LINK32 /pdb:none /map

!ELSEIF  "$(CFG)" == "scummvm - Win32 mad mp3 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "scummvm___Win32_mad mp3_Release"
# PROP BASE Intermediate_Dir "scummvm___Win32_mad mp3_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "scummvm___mad_mp3_Release"
# PROP Intermediate_Dir "scummvm___mad_mp3_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /O2 /Ob2 /I "." /I "sound" /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_ADLIB" /D "USE_MAD" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /Ob2 /I "." /I "sound" /I "common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib libmad.lib simon___Release\simon.lib scumm___Release\scumm.lib sdl.lib /nologo /subsystem:console /machine:I386 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib sdl.lib winmm.lib wsock32.lib libmad.lib scumm___mad_mp3_Release\scumm.lib simon___mad_mp3_Release\simon.lib sky___mad_mp3_Release\sky.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "scummvm - Win32 Release"
# Name "scummvm - Win32 Debug"
# Name "scummvm - Win32 mad mp3 Debug"
# Name "scummvm - Win32 mad mp3 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\common\config-file.cpp"
# End Source File
# Begin Source File

SOURCE=".\common\config-file.h"
# End Source File
# Begin Source File

SOURCE=.\common\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\common\engine.h
# End Source File
# Begin Source File

SOURCE=.\common\file.cpp
# End Source File
# Begin Source File

SOURCE=.\common\file.h
# End Source File
# Begin Source File

SOURCE=.\common\gameDetector.cpp
# End Source File
# Begin Source File

SOURCE=.\common\gameDetector.h
# End Source File
# Begin Source File

SOURCE=.\common\list.h
# End Source File
# Begin Source File

SOURCE=.\common\main.cpp
# End Source File
# Begin Source File

SOURCE=.\common\map.h
# End Source File
# Begin Source File

SOURCE=.\common\rect.h
# End Source File
# Begin Source File

SOURCE=.\common\scaler.cpp
# End Source File
# Begin Source File

SOURCE=.\common\scaler.h
# End Source File
# Begin Source File

SOURCE=.\common\scummsys.h
# End Source File
# Begin Source File

SOURCE=.\common\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\common\str.cpp
# End Source File
# Begin Source File

SOURCE=.\common\str.h
# End Source File
# Begin Source File

SOURCE=.\common\system.h
# End Source File
# Begin Source File

SOURCE=.\common\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\common\timer.h
# End Source File
# Begin Source File

SOURCE=.\common\util.cpp
# End Source File
# Begin Source File

SOURCE=.\common\util.h
# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sound\fmopl.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\fmopl.h
# End Source File
# Begin Source File

SOURCE=.\sound\mididrv.h
# End Source File
# Begin Source File

SOURCE=.\sound\midiparser.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\midiparser.h
# End Source File
# Begin Source File

SOURCE=.\sound\midiparser_smf.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\midiparser_xmidi.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\mixer.h
# End Source File
# Begin Source File

SOURCE=.\sound\mpu401.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\mpu401.h
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gui\about.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\about.h
# End Source File
# Begin Source File

SOURCE=.\gui\browser.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\browser.h
# End Source File
# Begin Source File

SOURCE=.\gui\chooser.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\chooser.h
# End Source File
# Begin Source File

SOURCE=.\gui\console.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\console.h
# End Source File
# Begin Source File

SOURCE=.\gui\dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\dialog.h
# End Source File
# Begin Source File

SOURCE=.\gui\EditTextWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\EditTextWidget.h
# End Source File
# Begin Source File

SOURCE=.\gui\launcher.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\launcher.h
# End Source File
# Begin Source File

SOURCE=.\gui\ListWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\ListWidget.h
# End Source File
# Begin Source File

SOURCE=.\gui\message.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\message.h
# End Source File
# Begin Source File

SOURCE=.\gui\newgui.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\newgui.h
# End Source File
# Begin Source File

SOURCE=.\gui\options.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\options.h
# End Source File
# Begin Source File

SOURCE=.\gui\PopUpWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\PopUpWidget.h
# End Source File
# Begin Source File

SOURCE=.\gui\ScrollBarWidget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\ScrollBarWidget.h
# End Source File
# Begin Source File

SOURCE=.\gui\widget.cpp
# End Source File
# Begin Source File

SOURCE=.\gui\widget.h
# End Source File
# End Group
# Begin Group "backends"

# PROP Default_Filter ""
# Begin Group "sdl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\backends\sdl\sdl-common.cpp"
# End Source File
# Begin Source File

SOURCE=".\backends\sdl\sdl-common.h"
# End Source File
# Begin Source File

SOURCE=.\backends\sdl\sdl.cpp
# End Source File
# End Group
# Begin Group "fs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\backends\fs\fs.h
# End Source File
# Begin Source File

SOURCE=".\backends\fs\windows\windows-fs.cpp"
# End Source File
# End Group
# Begin Group "midi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\backends\midi\adlib.cpp
# End Source File
# Begin Source File

SOURCE=.\backends\midi\null.cpp
# End Source File
# Begin Source File

SOURCE=.\backends\midi\windows.cpp
# End Source File
# End Group
# End Group
# End Group
# Begin Source File

SOURCE=.\NEWS
# End Source File
# Begin Source File

SOURCE=.\README
# End Source File
# Begin Source File

SOURCE=.\scummvm.ico
# End Source File
# Begin Source File

SOURCE=.\scummvm.rc
# End Source File
# End Target
# End Project
