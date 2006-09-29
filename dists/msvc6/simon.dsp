# Microsoft Developer Studio Project File - Name="agos" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=agos - Win32 mad mp3 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "agos.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "agos.mak" CFG="agos - Win32 mad mp3 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "agos - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "agos - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "agos - Win32 mad mp3 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "agos - Win32 mad mp3 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "agos - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "agos___Release"
# PROP Intermediate_Dir "agos___Release"
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

!ELSEIF  "$(CFG)" == "agos - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "agos___Win32_Debug"
# PROP BASE Intermediate_Dir "agos___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "agos___Debug"
# PROP Intermediate_Dir "agos___Debug"
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

!ELSEIF  "$(CFG)" == "agos - Win32 mad mp3 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "agos___Win32_MP3_Enabled_Debug"
# PROP BASE Intermediate_Dir "agos___Win32_MP3_Enabled_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "agos___mad_mp3_Debug"
# PROP Intermediate_Dir "agos___mad_mp3_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "BYPASS_COPY_PROT" /D "USE_ADLIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GX /ZI /Od /I "." /I "common" /I "..\..\\" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /D "BYPASS_COPY_PROT" /D "USE_MAD" /D "MSVC6_COMPAT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "agos - Win32 mad mp3 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "agos___Win32_MP3_Enabled_Release"
# PROP BASE Intermediate_Dir "agos___Win32_MP3_Enabled_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "agos___mad_mp3_Release"
# PROP Intermediate_Dir "agos___mad_mp3_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "." /I "common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "USE_ADLIB" /YX /FD /c
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

# Name "agos - Win32 Release"
# Name "agos - Win32 Debug"
# Name "agos - Win32 mad mp3 Debug"
# Name "agos - Win32 mad mp3 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\agos\charset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\agos\debugger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\debugger.h
# End Source File
# Begin Source File

SOURCE=..\..\agos\intern.h
# End Source File
# Begin Source File

SOURCE=..\..\agos\items.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\midi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\midi.h
# End Source File
# Begin Source File

SOURCE=..\..\agos\midiparser_s1d.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\res.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\agos.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\agos.h
# End Source File
# Begin Source File

SOURCE=..\..\agos\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\agos\verb.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\vga.cpp
# End Source File
# Begin Source File

SOURCE=..\..\agos\vga.h
# End Source File
# End Group
# End Target
# End Project
