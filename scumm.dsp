# Microsoft Developer Studio Project File - Name="scumm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=scumm - Win32 mad mp3 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scumm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scumm.mak" CFG="scumm - Win32 mad mp3 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scumm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "scumm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "scumm - Win32 mad mp3 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "scumm - Win32 mad mp3 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scumm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "scumm___Release"
# PROP Intermediate_Dir "scumm___Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "." /I "common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /YX /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scumm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "scumm___Win32_Debug"
# PROP BASE Intermediate_Dir "scumm___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "scumm___Debug"
# PROP Intermediate_Dir "scumm___Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scumm - Win32 mad mp3 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "scumm___Win32_mad mp3_Debug"
# PROP BASE Intermediate_Dir "scumm___Win32_mad mp3_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "scumm___mad_mp3_Debug"
# PROP Intermediate_Dir "scumm___mad_mp3_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "common" /I "scumm" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /vmg /GX /ZI /Od /I "." /I "common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scumm - Win32 mad mp3 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "scumm___Win32_mad mp3_Release"
# PROP BASE Intermediate_Dir "scumm___Win32_mad mp3_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "scumm___mad_mp3_Release"
# PROP Intermediate_Dir "scumm___mad_mp3_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "." /I "common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /YX /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "scumm - Win32 Release"
# Name "scumm - Win32 Debug"
# Name "scumm - Win32 mad mp3 Debug"
# Name "scumm - Win32 mad mp3 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "smush"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scumm\smush\channel.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\chunk.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\chunk.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\chunk_type.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec1.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec37.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec37.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec47.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec47.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\imuse_channel.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\saud_channel.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\smush_font.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\smush_font.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\smush_mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\smush_mixer.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\smush_player.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\smush_player.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\scumm\actor.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\actor.h
# End Source File
# Begin Source File

SOURCE=.\scumm\akos.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\akos.h
# End Source File
# Begin Source File

SOURCE=".\scumm\base-costume.cpp"
# End Source File
# Begin Source File

SOURCE=".\scumm\base-costume.h"
# End Source File
# Begin Source File

SOURCE=.\scumm\bomp.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\bomp.h
# End Source File
# Begin Source File

SOURCE=.\scumm\boxes.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\boxes.h
# End Source File
# Begin Source File

SOURCE=.\scumm\bundle.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\bundle.h
# End Source File
# Begin Source File

SOURCE=.\scumm\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\charset.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\charset.h
# End Source File
# Begin Source File

SOURCE=.\scumm\costume.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\costume.h
# End Source File
# Begin Source File

SOURCE=.\scumm\debugger.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\debugger.h
# End Source File
# Begin Source File

SOURCE=.\scumm\dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\dialogs.h
# End Source File
# Begin Source File

SOURCE=.\scumm\gfx.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\gfx.h
# End Source File
# Begin Source File

SOURCE=.\scumm\help.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\help.h
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse.h
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse_digi.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse_digi.h
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse_internal.h
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse_player.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\instrument.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\instrument.h
# End Source File
# Begin Source File

SOURCE=.\scumm\intern.h
# End Source File
# Begin Source File

SOURCE=.\scumm\midiparser_eup.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\midiparser_ro.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\music.h
# End Source File
# Begin Source File

SOURCE=.\scumm\nut_renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\nut_renderer.h
# End Source File
# Begin Source File

SOURCE=.\scumm\object.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\object.h
# End Source File
# Begin Source File

SOURCE=.\scumm\player_mod.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\player_mod.h
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v1.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v1.h
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v2.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v2.h
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v2a.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v2a.h
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v3a.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\player_v3a.h
# End Source File
# Begin Source File

SOURCE=.\scumm\resource.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\resource.h
# End Source File
# Begin Source File

SOURCE=.\scumm\resource_v2.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\resource_v3.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\resource_v4.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\saveload.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\saveload.h
# End Source File
# Begin Source File

SOURCE=.\scumm\script.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\script_v2.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\script_v5.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\script_v6.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\script_v8.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\scumm.h
# End Source File
# Begin Source File

SOURCE=.\scumm\scummvm.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\sound.h
# End Source File
# Begin Source File

SOURCE=.\scumm\string.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\usage_bits.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\usage_bits.h
# End Source File
# Begin Source File

SOURCE=.\scumm\vars.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\verbs.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\verbs.h
# End Source File
# End Group
# End Target
# End Project
