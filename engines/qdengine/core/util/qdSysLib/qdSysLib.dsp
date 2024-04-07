# Microsoft Developer Studio Project File - Name="qdSysLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=qdSysLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qdSysLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qdSysLib.mak" CFG="qdSysLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qdSysLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "qdSysLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/qdEngine", HIKAAAAA"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qdSysLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "..\..\\" /I "..\..\qdCore" /I "..\..\qdCore\Util" /I "..\..\System" /I "..\..\System\Graphics" /I "..\..\System\Sound" /I "..\..\Parser" /I "..\..\System\Input" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__QD_SYSLIB__" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "qdSysLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "..\..\\" /I "..\..\qdCore" /I "..\..\qdCore\Util" /I "..\..\System" /I "..\..\System\Graphics" /I "..\..\System\Sound" /I "..\..\Parser" /I "..\..\System\Input" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__QD_SYSLIB__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "qdSysLib - Win32 Release"
# Name "qdSysLib - Win32 Debug"
# Begin Group "System"

# PROP Default_Filter ""
# Begin Group "Graphics"

# PROP Default_Filter ""
# Begin Group "GDI_Graphics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\..\System\Graphics\gdi_gr_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gdi_gr_dispatcher.h
# End Source File
# End Group
# Begin Group "DDraw_Graphics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\..\System\Graphics\ddraw_gr_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\ddraw_gr_dispatcher.h
# End Source File
# End Group
# Begin Group "RLE_Compress"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\..\System\Graphics\rle_compress.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\rle_compress.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_core.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_dispatcher.h
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_draw_sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_draw_sprite_rle.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_draw_sprite_rle_z.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_draw_sprite_z.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_font.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_font.h
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_screen_region.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Graphics\gr_screen_region.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\..\System\Sound\snd_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Sound\snd_dispatcher.h
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Sound\snd_sound.h
# End Source File
# End Group
# Begin Group "ErrorHandler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\..\System\app_error_handler.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\app_error_handler.h
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\..\System\Input\input_recorder.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Input\input_recorder.h
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Input\input_wndproc.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Input\input_wndproc.h
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Input\keyboard_input.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Input\keyboard_input.h
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Input\mouse_input.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\Input\mouse_input.h
# End Source File
# End Group
# Begin Group "AppLog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\..\System\app_log.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\app_log.h
# End Source File
# Begin Source File

SOURCE=.\..\..\System\app_log_file.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\app_log_file.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\..\..\System\app_core.cpp
# End Source File
# Begin Source File

SOURCE=.\..\..\System\app_core.h
# End Source File
# End Group
# Begin Group "qdCore"

# PROP Default_Filter ""
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Group "ResourceDispatcher"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\qdCore\Util\ResourceDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\Util\ResourceDispatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\Util\SynchroTimer.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\qdCore\Util\zip_container.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\Util\zip_container.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Group "Animation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\qdCore\qd_animation.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_animation.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_animation_frame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_animation_frame.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_animation_maker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_animation_maker.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_sprite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_sprite.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\qdCore\qd_resource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_resource.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_resource_container.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_resource_dispatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_sound.h
# End Source File
# End Group
# Begin Group "FileManager"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\qdCore\qd_file_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_file_manager.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_file_owner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_file_owner.h
# End Source File
# End Group
# Begin Group "Base"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\qdCore\qd_named_object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_named_object.h
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_named_object_base.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_named_object_base.h
# End Source File
# End Group
# Begin Group "Setup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\qdCore\qd_setup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\qdCore\qd_setup.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\..\qd_precomp.h
# End Source File
# End Target
# End Project
