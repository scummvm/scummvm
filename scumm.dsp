# Microsoft Developer Studio Project File - Name="scumm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=scumm - Win32 MP3 Enabled Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scumm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scumm.mak" CFG="scumm - Win32 MP3 Enabled Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scumm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "scumm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "scumm - Win32 MP3 Enabled Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "scumm - Win32 MP3 Enabled Release" (based on "Win32 (x86) Static Library")
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
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /YX /FD /c
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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "common" /I "scumm" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scumm - Win32 MP3 Enabled Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "scumm___Win32_MP3_Enabled_Debug"
# PROP BASE Intermediate_Dir "scumm___Win32_MP3_Enabled_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "scumm___MP3_Enabled_Debug"
# PROP Intermediate_Dir "scumm___MP3_Enabled_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "common" /I "scumm" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "common" /I "scumm" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /D "COMPRESSED_SOUND_FILE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scumm - Win32 MP3 Enabled Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "scumm___Win32_MP3_Enabled_Release"
# PROP BASE Intermediate_Dir "scumm___Win32_MP3_Enabled_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "scumm___Win32_MP3_Enabled_Release"
# PROP Intermediate_Dir "scumm___Win32_MP3_Enabled_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "common" /I "scumm" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "DUMP_SCRIPTS" /D "COMPRESSED_SOUND_FILE" /YX /FD /c
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
# Name "scumm - Win32 MP3 Enabled Debug"
# Name "scumm - Win32 MP3 Enabled Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "smush"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scumm\smush\blitter.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\blitter.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\brenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\brenderer.h
# End Source File
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

SOURCE=.\scumm\smush\codec1.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec37.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec37.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec44.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec44.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec47.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\codec47.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\color.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\color.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\config.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\decoder.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\frenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\frenderer.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\imuse_channel.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\mixer.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\palette.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\palette.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\player.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\player.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\rect.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\rect.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\renderer.h
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\saud_channel.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\scumm_renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush\scumm_renderer.h
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

SOURCE=.\scumm\costume.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\costume.h
# End Source File
# Begin Source File

SOURCE=.\scumm\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\debug.h
# End Source File
# Begin Source File

SOURCE=.\scumm\debugrl.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\debugrl.h
# End Source File
# Begin Source File

SOURCE=.\scumm\gfx.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\gfx.h
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\imuse.h
# End Source File
# Begin Source File

SOURCE=.\scumm\insane.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\object.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\object.h
# End Source File
# Begin Source File

SOURCE=.\scumm\resource.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\resource.h
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

SOURCE=.\scumm\script_v1.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\script_v2.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\scumm.h
# End Source File
# Begin Source File

SOURCE=.\scumm\scummvm.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\smush.h
# End Source File
# Begin Source File

SOURCE=.\scumm\sound.h
# End Source File
# Begin Source File

SOURCE=.\scumm\string.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\sys.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\vars.cpp
# End Source File
# Begin Source File

SOURCE=.\scumm\verbs.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
