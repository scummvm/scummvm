# Microsoft Developer Studio Project File - Name="bs1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bs1 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bs1.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bs1.mak" CFG="bs1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bs1 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bs1 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bs1 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bs1___Release"
# PROP Intermediate_Dir "bs1___Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bs1 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bs1___Debug"
# PROP Intermediate_Dir "bs1___Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "bs1 - Win32 Release"
# Name "bs1 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\sword1\animation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\animation.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\collision.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\control.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\control.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\eventman.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\eventman.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\logic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\logic.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\memman.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\memman.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\menu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\menu.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\mouse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\mouse.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\music.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\music.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\object.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\objectman.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\objectman.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\resman.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\resman.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\router.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\router.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\screen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\screen.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\staticres.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\sword1.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\sword1.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\sworddefs.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\swordres.h
# End Source File
# Begin Source File

SOURCE=..\..\sword1\text.cpp
# End Source File
# Begin Source File

SOURCE=..\..\sword1\text.h
# End Source File
# End Group
# End Target
# End Project
