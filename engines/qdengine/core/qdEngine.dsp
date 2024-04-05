# Microsoft Developer Studio Project File - Name="qdEngine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=qdEngine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qdEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qdEngine.mak" CFG="qdEngine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qdEngine - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "qdEngine - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "qdEngine - Win32 Final" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/qdEngine", HIKAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qdEngine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Bin"
# PROP Intermediate_Dir "\Garbage\qdEngine\Runtime\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "Parser" /I "System\Input" /I "Runtime" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 PlayMpp.lib libexpat.lib XUtil.lib MGvc32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib winmm.lib strmbase.lib ddraw.lib comctl32.lib /nologo /subsystem:windows /profile /map /debug /debugtype:both /machine:I386 /out:"..\Bin\qd_game.exe"

!ELSEIF  "$(CFG)" == "qdEngine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Bin\dbg"
# PROP Intermediate_Dir "\Garbage\qdEngine\Runtime\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "Parser" /I "System\Input" /I "Runtime" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libexpatmtd.lib XUtilDBGMT.lib PlayMppDBGMT.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib winmm.lib strmbase.lib ddraw.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcp.lib" /nodefaultlib:"libcd.lib" /out:"..\Bin\dbg\qd_gameDBG.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "qdEngine - Win32 Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qdEngine___Win32_Final"
# PROP BASE Intermediate_Dir "qdEngine___Win32_Final"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Bin"
# PROP Intermediate_Dir "\Garbage\qdEngine\Runtime\Final"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "Parser" /I "System\Input" /I "Runtime" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "Parser" /I "System\Input" /I "Runtime" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_FINAL_VERSION" /D "_FINAL_VERSION_" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 PlayMpp.lib libexpat.lib XUtil.lib XMath.lib MGvc32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib winmm.lib strmbase.lib ddraw.lib comctl32.lib /nologo /subsystem:windows /debug /debugtype:coff /machine:I386 /out:"..\Bin\qd_game.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 PlayMpp.lib libexpat.lib XUtil.lib XMath.lib MGvc32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib winmm.lib strmbase.lib ddraw.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /out:"..\Bin\qd_game_final.exe"
# SUBTRACT LINK32 /pdb:none /debug

!ENDIF 

# Begin Target

# Name "qdEngine - Win32 Release"
# Name "qdEngine - Win32 Debug"
# Name "qdEngine - Win32 Final"
# Begin Group "Runtime"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Runtime\comline_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\Runtime\comline_parser.h
# End Source File
# Begin Source File

SOURCE=.\Runtime\qd_dialog_control.cpp
# End Source File
# Begin Source File

SOURCE=.\Runtime\qd_dialog_control.h
# End Source File
# Begin Source File

SOURCE=.\Runtime\qd_dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\Runtime\qd_runtime.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Runtime\qd_dialogs.h
# End Source File
# Begin Source File

SOURCE=.\qd_game.ico
# End Source File
# Begin Source File

SOURCE=.\qd_game.ini
# End Source File
# Begin Source File

SOURCE=.\qd_game.rc
# End Source File
# Begin Source File

SOURCE=.\qd_splash.bmp
# End Source File
# Begin Source File

SOURCE=.\qd_splash_mask.bmp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Target
# End Project
