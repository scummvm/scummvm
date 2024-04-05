# Microsoft Developer Studio Project File - Name="qdaEditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=qdaEditor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qdaEditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qdaEditor.mak" CFG="qdaEditor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qdaEditor - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "qdaEditor - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/qdEngine", HIKAAAAA"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qdaEditor - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Bin"
# PROP Intermediate_Dir "\Garbage\qdEngine\qdaEditor\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\\" /I "..\..\qdCore" /I "..\..\qdCore\Util" /I "..\..\System" /I "..\..\System\Graphics" /I "..\..\System\Sound" /I "..\..\Parser" /I "..\..\System\Input" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 XUtil.lib XMath.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib /nologo /subsystem:windows /machine:I386 /out:"../../../Bin/qda_editor.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "qdaEditor - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Bin\dbg"
# PROP Intermediate_Dir "\Garbage\qdEngine\qdaEditor\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\\" /I "..\..\qdCore" /I "..\..\qdCore\Util" /I "..\..\System" /I "..\..\System\Graphics" /I "..\..\System\Sound" /I "..\..\Parser" /I "..\..\System\Input" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 PlayMppDBG.lib libexpatd.lib XUtilDBG.lib strmbase.lib comctl32.lib dsound.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"../../../Bin/dbg/qda_editorDBG.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "qdaEditor - Win32 Release"
# Name "qdaEditor - Win32 Debug"
# Begin Group "qdaEditor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\qda_editor.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\qda_editor.h
# End Source File
# End Group
# Begin Group "Runtime"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\AVIGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\AVIGenerator.h
# End Source File
# Begin Source File

SOURCE=.\Source\qd_dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\qd_dialogs.h
# End Source File
# Begin Source File

SOURCE=.\Source\qdlg_controls.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\qdlg_controls.h
# End Source File
# Begin Source File

SOURCE=.\Source\runtime.cpp
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Resource\Icons\101.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\102.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\103.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\104.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\105.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\106.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\107.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\108.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\117.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\Icons\200.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\qda_editor.rc
# End Source File
# End Target
# End Project
