# Microsoft Developer Studio Project File - Name="saga" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=saga - Win32 mad mp3 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "saga.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "saga.mak" CFG="saga - Win32 mad mp3 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "saga - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "saga - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "saga - Win32 mad mp3 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "saga - Win32 mad mp3 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "saga - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "saga___Release"
# PROP Intermediate_Dir "saga___Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "saga - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "saga___Win32_Debug"
# PROP BASE Intermediate_Dir "saga___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "saga___Debug"
# PROP Intermediate_Dir "saga___Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "saga - Win32 mad mp3 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "saga___Win32_mad mp3_Debug"
# PROP BASE Intermediate_Dir "saga___Win32_mad mp3_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "saga___mad_mp3_Debug"
# PROP Intermediate_Dir "saga___mad_mp3_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "common" /I "saga" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /vmg /GX /ZI /Od /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /D "MSVC6_COMPAT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "saga - Win32 mad mp3 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "saga___Win32_mad mp3_Release"
# PROP BASE Intermediate_Dir "saga___Win32_mad mp3_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "saga___mad_mp3_Release"
# PROP Intermediate_Dir "saga___mad_mp3_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "common" /I "saga" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "..\..\\" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /FD /c
# SUBTRACT CPP /YX
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

# Name "saga - Win32 Release"
# Name "saga - Win32 Debug"
# Name "saga - Win32 mad mp3 Debug"
# Name "saga - Win32 mad mp3 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\saga\actionmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\actionmap.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\actionmap_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\actor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\actor.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\actor_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\actordata.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\actordata.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\animation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\animation.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\animation_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\console.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\console.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\console_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\cvar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\cvar.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\cvar_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\events.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\events.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\events_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\expr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\expr.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\font.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\font.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\font_map.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\font_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\game.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\game.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\game_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\gfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\gfx.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\gfx_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\ihnm_introproc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\ihnm_introproc.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\image.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\image.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\image_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\input.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\interface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\interface.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\interface_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\isomap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\isomap.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\isomap_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\ite_introproc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\ite_introproc.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\ite_introproc_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\math_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\music.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\music.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\objectmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\objectmap.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\objectmap_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\palanim.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\palanim.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\palanim_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\reinherit.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\render.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\render.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\render_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\resnames.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\rscfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\rscfile.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\rscfile_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\saga.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\saga.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\scene.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\scene.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\scene_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sceneproc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sceneproc.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\script.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\script.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\script_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sdata.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sdata.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sdebug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sfuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sfuncs.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sndres.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sndres.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sndres_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sprite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sprite.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sprite_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sstack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sstack.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\stack_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\sthread.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\sys_interface.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\text.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\text.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\text_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\timer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\timer.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\transitions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\xmidi_mod.h
# End Source File
# Begin Source File

SOURCE=..\..\saga\ys_dl_list.cpp
# End Source File
# Begin Source File

SOURCE=..\..\saga\yslib.h
# End Source File
# End Group
# End Target
# End Project
