# Microsoft Developer Studio Project File - Name="XTool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=XTool - Win32 DebugMultithreaded
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "XTool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "XTool.mak" CFG="XTool - Win32 DebugMultithreaded"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "XTool - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "XTool - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "XTool - Win32 ReleaseMultithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE "XTool - Win32 DebugMultithreaded" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "XTool - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Libs"
# PROP Intermediate_Dir "\Garbage\XTool\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /Yu"xglobal.h" /FD /c
# ADD BASE RSC /l 0x419
# ADD RSC /l 0x419
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"XTool.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "XTool - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Libs"
# PROP Intermediate_Dir "\Garbage\XTool\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"xglobal.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x419
# ADD RSC /l 0x419
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"XTool.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Libs\XToolDBG.lib"

!ELSEIF  "$(CFG)" == "XTool - Win32 ReleaseMultithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "XTool___Win32_ReleaseMultithreaded"
# PROP BASE Intermediate_Dir "XTool___Win32_ReleaseMultithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Libs"
# PROP Intermediate_Dir "\Garbage\XTool\ReleaseMT"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /Yu"xglobal.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /Yu"xglobal.h" /FD /c
# ADD BASE RSC /l 0x419
# ADD RSC /l 0x419
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"XTool.bsc"
# ADD BSC32 /nologo /o"XTool.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Libs\XToolMT.lib"

!ELSEIF  "$(CFG)" == "XTool - Win32 DebugMultithreaded"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "XTool___Win32_DebugMultithreaded"
# PROP BASE Intermediate_Dir "XTool___Win32_DebugMultithreaded"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Libs"
# PROP Intermediate_Dir "\Garbage\XTool\DebugMT"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"xglobal.h" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"xglobal.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x419
# ADD RSC /l 0x419
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"XTool.bsc"
# ADD BSC32 /nologo /o"XTool.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Libs\XToolDBG.lib"
# ADD LIB32 /nologo /out:".\Libs\XToolDBGMT.lib"

!ENDIF 

# Begin Target

# Name "XTool - Win32 Release"
# Name "XTool - Win32 Debug"
# Name "XTool - Win32 ReleaseMultithreaded"
# Name "XTool - Win32 DebugMultithreaded"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\XTCORE.CPP
# ADD CPP /Yc"xglobal.h"
# End Source File
# End Group
# Begin Group "XBuffer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XBuffer\XBCNVIN.CPP
# End Source File
# Begin Source File

SOURCE=.\XBuffer\XBCNVOUT.CPP
# End Source File
# Begin Source File

SOURCE=.\XBuffer\XBCORE.CPP
# End Source File
# Begin Source File

SOURCE=.\XBuffer\XBSEARCH.CPP
# End Source File
# End Group
# Begin Group "XConsole"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XConsole\XCCORE.CPP
# End Source File
# Begin Source File

SOURCE=.\XConsole\XCOUTPUT.CPP
# End Source File
# End Group
# Begin Group "XErrHand"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XERRHAND\BSUFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\XERRHAND\CrashHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\XERRHAND\DiagAssert.cpp
# End Source File
# Begin Source File

SOURCE=.\XERRHAND\GetLoadedModules.cpp
# End Source File
# Begin Source File

SOURCE=.\XERRHAND\IsNT.cpp
# End Source File
# Begin Source File

SOURCE=.\XERRHAND\NT4ProcessInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\XERRHAND\TLHELPProcessInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\XErrHand\XERRHAND.CPP
# End Source File
# End Group
# Begin Group "XKey"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XKey\XKEY.CPP
# End Source File
# Begin Source File

SOURCE=.\XKey\XNAMES.CPP
# End Source File
# End Group
# Begin Group "XMsgBuffer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XMsgBuffer\XMSGBUF.CPP
# End Source File
# End Group
# Begin Group "XRecorder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XRecorder\XREC.CPP
# End Source File
# End Group
# Begin Group "XResource"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XResource\zip_headers.h
# End Source File
# Begin Source File

SOURCE=.\XResource\zip_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\zip_resource.h
# End Source File
# End Group
# Begin Group "XStream"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XStream\XSCNVIN.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSCNVOUT.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSCORE.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSENLV.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSGETLIN.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSINPUT.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSOUTPUT.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSRDWR.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSSERV.CPP
# End Source File
# Begin Source File

SOURCE=.\XStream\XSTIME.CPP
# End Source File
# End Group
# Begin Group "XUtil"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XUtil\CPUID.CPP
# End Source File
# Begin Source File

SOURCE=.\XUtil\XClock.cpp
# End Source File
# Begin Source File

SOURCE=.\XUtil\XUTIL.CPP
# End Source File
# End Group
# Begin Group "XZip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XZip\INZIP.CPP
# End Source File
# Begin Source File

SOURCE=.\XZip\OUTZIP.CPP
# End Source File
# Begin Source File

SOURCE=.\XZip\TREEZIP.CPP
# End Source File
# End Group
# Begin Group "XMath"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\XMath\XMath.cpp
# End Source File
# Begin Source File

SOURCE=.\XMath\xmath.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\XToolLOG.txt
# End Source File
# End Target
# End Project
