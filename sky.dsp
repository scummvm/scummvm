# Microsoft Developer Studio Project File - Name="sky" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sky - Win32 OpenGL Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sky.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sky.mak" CFG="sky - Win32 OpenGL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sky - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sky - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sky - Win32 OpenGL Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sky - Win32 OpenGL Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sky - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "sky___Release"
# PROP Intermediate_Dir "sky___Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "." /I "common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sky - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "sky___Debug"
# PROP Intermediate_Dir "sky___Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sky - Win32 OpenGL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sky___Win32_MP3_Enabled_Debug"
# PROP BASE Intermediate_Dir "sky___Win32_MP3_Enabled_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "sky___OpenGL_Debug"
# PROP Intermediate_Dir "sky___OpenGL_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /WX /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /WX /Gm /GR /GX /ZI /Od /I "." /I "common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sky - Win32 OpenGL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sky___Win32_MP3_Enabled_Release"
# PROP BASE Intermediate_Dir "sky___Win32_MP3_Enabled_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "sky___OpenGL_Release"
# PROP Intermediate_Dir "sky___OpenGL_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GR /GX /O2 /I "." /I "common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
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

# Name "sky - Win32 Release"
# Name "sky - Win32 Debug"
# Name "sky - Win32 OpenGL Debug"
# Name "sky - Win32 OpenGL Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "compacts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sky\compacts\0compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\101comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\102comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\10comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\11comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\12comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\13comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\14comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\15comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\16comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\17comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\18comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\19comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\1compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\20comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\21comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\22comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\23comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\24comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\25comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\26comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\27comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\28comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\29comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\2compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\30comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\31comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\32comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\33comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\34comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\36comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\37comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\38comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\39comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\3compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\40comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\41comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\42comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\44comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\45comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\46comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\47comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\48comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\4compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\5compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\65comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\66comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\67comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\68comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\69comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\70comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\71comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\72comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\73comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\74comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\75comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\76comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\77comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\78comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\79comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\80comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\81comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\82comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\85comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\90comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\91comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\92comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\93comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\94comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\95comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\96comp.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\9compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\linc_gen.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\lincmenu.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\objects.h
# End Source File
# Begin Source File

SOURCE=.\sky\compacts\z_compac.h
# End Source File
# End Group
# Begin Group "music"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sky\music\adlibchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\music\adlibchannel.h
# End Source File
# Begin Source File

SOURCE=.\sky\music\adlibmusic.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\music\adlibmusic.h
# End Source File
# Begin Source File

SOURCE=.\sky\music\gmchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\music\gmchannel.h
# End Source File
# Begin Source File

SOURCE=.\sky\music\gmmusic.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\music\gmmusic.h
# End Source File
# Begin Source File

SOURCE=.\sky\music\mt32music.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\music\mt32music.h
# End Source File
# Begin Source File

SOURCE=.\sky\music\musicbase.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\music\musicbase.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\sky\autoroute.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\autoroute.h
# End Source File
# Begin Source File

SOURCE=.\sky\cd_intro.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\compact.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\compact.h
# End Source File
# Begin Source File

SOURCE=.\sky\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\debug.h
# End Source File
# Begin Source File

SOURCE=.\sky\disk.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\disk.h
# End Source File
# Begin Source File

SOURCE=.\sky\grid.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\grid.h
# End Source File
# Begin Source File

SOURCE=.\sky\intro.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\logic.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\logic.h
# End Source File
# Begin Source File

SOURCE=.\sky\mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\mouse.h
# End Source File
# Begin Source File

SOURCE=.\sky\rnc_deco.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\rnc_deco.h
# End Source File
# Begin Source File

SOURCE=.\sky\screen.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\screen.h
# End Source File
# Begin Source File

SOURCE=.\sky\sky.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\sky.h
# End Source File
# Begin Source File

SOURCE=.\sky\skydefs.h
# End Source File
# Begin Source File

SOURCE=.\sky\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\sound.h
# End Source File
# Begin Source File

SOURCE=.\sky\struc.h
# End Source File
# Begin Source File

SOURCE=.\sky\text.cpp
# End Source File
# Begin Source File

SOURCE=.\sky\text.h
# End Source File
# End Group
# End Target
# End Project
