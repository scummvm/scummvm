# Microsoft Developer Studio Project File - Name="bs2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bs2 - Win32 mad mp3 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bs2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bs2.mak" CFG="bs2 - Win32 mad mp3 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bs2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bs2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bs2 - Win32 mad mp3 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bs2 - Win32 mad mp3 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bs2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bs2___Release"
# PROP Intermediate_Dir "bs2___Release"
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

!ELSEIF  "$(CFG)" == "bs2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bs2___Win32_Debug"
# PROP BASE Intermediate_Dir "bs2___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bs2___Debug"
# PROP Intermediate_Dir "bs2___Debug"
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

!ELSEIF  "$(CFG)" == "bs2 - Win32 mad mp3 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bs2___Win32_mad mp3_Debug"
# PROP BASE Intermediate_Dir "bs2___Win32_mad mp3_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bs2___mad_mp3_Debug"
# PROP Intermediate_Dir "bs2___mad_mp3_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /vmg /GR- /GX /ZI /Od /I "." /I "common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bs2 - Win32 mad mp3 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bs2___Win32_mad mp3_Release"
# PROP BASE Intermediate_Dir "bs2___Win32_mad mp3_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bs2___mad_mp3_Release"
# PROP Intermediate_Dir "bs2___mad_mp3_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /YX /FD /c
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

# Name "bs2 - Win32 Release"
# Name "bs2 - Win32 Debug"
# Name "bs2 - Win32 mad mp3 Debug"
# Name "bs2 - Win32 mad mp3 Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bs2\driver\_console.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\_console.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\_mouse.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\d_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\d_draw.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\d_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\d_sound.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\ddutil.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\driver96.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\keyboard.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\language.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\menu.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\mouse.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\palette.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\palette.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\rdwin.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\rdwin.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\render.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\render.h
# End Source File
# Begin Source File

SOURCE=.\bs2\driver\sprite.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\bs2\anims.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\anims.h
# End Source File
# Begin Source File

SOURCE=.\bs2\build_display.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\build_display.h
# End Source File
# Begin Source File

SOURCE=.\bs2\console.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\console.h
# End Source File
# Begin Source File

SOURCE=.\bs2\controls.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\controls.h
# End Source File
# Begin Source File

SOURCE=.\bs2\credits.h
# End Source File
# Begin Source File

SOURCE=.\bs2\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\debug.h
# End Source File
# Begin Source File

SOURCE=.\bs2\defs.h
# End Source File
# Begin Source File

SOURCE=.\bs2\events.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\events.h
# End Source File
# Begin Source File

SOURCE=.\bs2\function.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\function.h
# End Source File
# Begin Source File

SOURCE=.\bs2\header.h
# End Source File
# Begin Source File

SOURCE=.\bs2\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\icons.h
# End Source File
# Begin Source File

SOURCE=.\bs2\interpreter.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\interpreter.h
# End Source File
# Begin Source File

SOURCE=.\bs2\layers.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\layers.h
# End Source File
# Begin Source File

SOURCE=.\bs2\logic.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\logic.h
# End Source File
# Begin Source File

SOURCE=.\bs2\maketext.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\maketext.h
# End Source File
# Begin Source File

SOURCE=.\bs2\mem_view.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\mem_view.h
# End Source File
# Begin Source File

SOURCE=.\bs2\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\memory.h
# End Source File
# Begin Source File

SOURCE=.\bs2\mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\mouse.h
# End Source File
# Begin Source File

SOURCE=.\bs2\object.h
# End Source File
# Begin Source File

SOURCE=.\bs2\protocol.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\protocol.h
# End Source File
# Begin Source File

SOURCE=.\bs2\resman.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\resman.h
# End Source File
# Begin Source File

SOURCE=.\bs2\router.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\router.h
# End Source File
# Begin Source File

SOURCE=.\bs2\save_rest.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\save_rest.h
# End Source File
# Begin Source File

SOURCE=.\bs2\scroll.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\scroll.h
# End Source File
# Begin Source File

SOURCE=.\bs2\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\sound.h
# End Source File
# Begin Source File

SOURCE=.\bs2\speech.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\speech.h
# End Source File
# Begin Source File

SOURCE=.\bs2\startup.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\startup.h
# End Source File
# Begin Source File

SOURCE=.\bs2\sword2.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\sword2.h
# End Source File
# Begin Source File

SOURCE=.\bs2\sync.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\sync.h
# End Source File
# Begin Source File

SOURCE=.\bs2\tony_gsdk.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\tony_gsdk.h
# End Source File
# Begin Source File

SOURCE=.\bs2\walker.cpp
# End Source File
# Begin Source File

SOURCE=.\bs2\walker.h
# End Source File
# End Group
# End Target
# End Project
