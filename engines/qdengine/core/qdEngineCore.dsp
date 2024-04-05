# Microsoft Developer Studio Project File - Name="qdEngineCore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=qdEngineCore - Win32 Debug for QuestEditor
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qdEngineCore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qdEngineCore.mak" CFG="qdEngineCore - Win32 Debug for QuestEditor"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qdEngineCore - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "qdEngineCore - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "qdEngineCore - Win32 Debug for QuestEditor" (based on "Win32 (x86) Static Library")
!MESSAGE "qdEngineCore - Win32 Release for QuestEditor" (based on "Win32 (x86) Static Library")
!MESSAGE "qdEngineCore - Win32 Final" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/qdEngine", HIKAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qdEngineCore - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "\Garbage\qdEngine\Core\Release"
# PROP Intermediate_Dir "\Garbage\qdEngine\Core\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "System\Input" /I "Parser" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "qdEngineCore - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\Garbage\qdEngine\Core\Debug"
# PROP Intermediate_Dir "\Garbage\qdEngine\Core\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "System\Input" /I "Parser" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /Yu"qd_precomp.h" /FD /GZ /Zm150 /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "qdEngineCore - Win32 Debug for QuestEditor"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qdEngineCore___Win32_Debug_for_QuestEditor"
# PROP BASE Intermediate_Dir "qdEngineCore___Win32_Debug_for_QuestEditor"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\Garbage\qdEngine\Core\Editor_Debug"
# PROP Intermediate_Dir "\Garbage\qdEngine\Core\Editor_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "xGR" /I "Source" /I "Parser" /I "." /I "Source\Pathfinding" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "System\Input" /I "Parser" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_QUEST_EDITOR" /D "_GR_ENABLE_ZBUFFER" /Yu"qd_precomp.h" /FD /GZ /Zm150 /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "qdEngineCore - Win32 Release for QuestEditor"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qdEngineCore___Win32_Release_for_QuestEditor"
# PROP BASE Intermediate_Dir "qdEngineCore___Win32_Release_for_QuestEditor"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "\Garbage\qdEngine\Core\Editor_Release"
# PROP Intermediate_Dir "\Garbage\qdEngine\Core\Editor_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "xGR" /I "Source" /I "Parser" /I "." /I "Source\Pathfinding" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "System\Input" /I "Parser" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_QUEST_EDITOR" /D "_GR_ENABLE_ZBUFFER" /Yu"qd_precomp.h" /FD /Zm150 /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "qdEngineCore - Win32 Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qdEngineCore___Win32_Final"
# PROP BASE Intermediate_Dir "qdEngineCore___Win32_Final"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "\Garbage\qdEngine\Core\Final"
# PROP Intermediate_Dir "\Garbage\qdEngine\Core\Final"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "System\Input" /I "Parser" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "." /I "qdCore" /I "qdCore\Util" /I "System" /I "System\Graphics" /I "System\Sound" /I "System\Input" /I "Parser" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_FINAL_VERSION" /D "_FINAL_VERSION_" /FAcs /FR /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "qdEngineCore - Win32 Release"
# Name "qdEngineCore - Win32 Debug"
# Name "qdEngineCore - Win32 Debug for QuestEditor"
# Name "qdEngineCore - Win32 Release for QuestEditor"
# Name "qdEngineCore - Win32 Final"
# Begin Group "System"

# PROP Default_Filter ""
# Begin Group "Graphics"

# PROP Default_Filter ""
# Begin Group "GDI_Graphics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\Graphics\gdi_gr_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gdi_gr_dispatcher.h
# End Source File
# End Group
# Begin Group "DDraw_Graphics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\Graphics\ddraw_gr_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\ddraw_gr_dispatcher.h
# End Source File
# End Group
# Begin Group "RLE_Compress"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\Graphics\rle_compress.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\rle_compress.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\System\Graphics\gr_core.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_dispatcher.h
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_draw_sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_draw_sprite_rle.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_draw_sprite_rle_z.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_draw_sprite_z.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_font.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_font.h
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_screen_region.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Graphics\gr_screen_region.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Group "SoundDS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\Sound\ds_snd_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Sound\ds_snd_dispatcher.h
# End Source File
# Begin Source File

SOURCE=.\System\Sound\ds_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Sound\ds_sound.h
# End Source File
# End Group
# Begin Group "WAV"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\Sound\wav_file.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Sound\wav_file.h
# End Source File
# Begin Source File

SOURCE=.\System\Sound\wav_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Sound\wav_sound.h
# End Source File
# End Group
# Begin Group "MpegPlayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\plaympp_api.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\plaympp_api.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\System\Sound\snd_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Sound\snd_dispatcher.h
# End Source File
# Begin Source File

SOURCE=.\System\Sound\snd_sound.h
# End Source File
# End Group
# Begin Group "ErrorHandler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\app_error_handler.cpp
# End Source File
# Begin Source File

SOURCE=.\System\app_error_handler.h
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\Input\input_recorder.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Input\input_recorder.h
# End Source File
# Begin Source File

SOURCE=.\System\Input\input_wndproc.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Input\input_wndproc.h
# End Source File
# Begin Source File

SOURCE=.\System\Input\keyboard_input.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Input\keyboard_input.h
# End Source File
# Begin Source File

SOURCE=.\System\Input\mouse_input.cpp
# End Source File
# Begin Source File

SOURCE=.\System\Input\mouse_input.h
# End Source File
# End Group
# Begin Group "AppLog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System\app_log.cpp
# End Source File
# Begin Source File

SOURCE=.\System\app_log.h
# End Source File
# Begin Source File

SOURCE=.\System\app_log_file.cpp
# End Source File
# Begin Source File

SOURCE=.\System\app_log_file.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\System\app_core.cpp
# End Source File
# Begin Source File

SOURCE=.\System\app_core.h
# End Source File
# End Group
# Begin Group "qdCore"

# PROP Default_Filter ""
# Begin Group "GameDispatcher"

# PROP Default_Filter ""
# Begin Group "GameEnd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_game_end.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_end.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_game_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_dispatcher.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_dispatcher_base.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_dispatcher_base.h
# End Source File
# End Group
# Begin Group "GameScene"

# PROP Default_Filter ""
# Begin Group "Camera"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_camera.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_camera.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_camera_mode.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_camera_mode.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_d3dutils.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_d3dutils.h
# End Source File
# End Group
# Begin Group "GridZone"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_grid_zone.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_grid_zone.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_grid_zone_state.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_grid_zone_state.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_game_scene.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_scene.h
# End Source File
# End Group
# Begin Group "GameObjects"

# PROP Default_Filter ""
# Begin Group "StaticObject"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_game_object_static.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_object_static.h
# End Source File
# End Group
# Begin Group "AnimatedObject"

# PROP Default_Filter ""
# Begin Group "CoordsAnimation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_coords_animation.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_coords_animation.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_coords_animation_point.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_coords_animation_point.h
# End Source File
# End Group
# Begin Group "ObjectState"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_game_object_state.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_object_state.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_game_object_animated.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_object_animated.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_object_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_object_mouse.h
# End Source File
# End Group
# Begin Group "MovingObject"

# PROP Default_Filter ""
# Begin Group "Inventory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_inventory.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_inventory_cell.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_inventory_cell.h
# End Source File
# End Group
# Begin Group "ScaleInfo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_scale_info.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_scale_info.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_game_object_moving.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_object_moving.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_game_object.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_game_object.h
# End Source File
# End Group
# Begin Group "GameResources"

# PROP Default_Filter ""
# Begin Group "Sprite"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_sprite.h
# End Source File
# End Group
# Begin Group "Animation"

# PROP Default_Filter ""
# Begin Group "AnimationMaker"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_animation_maker.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_maker.h
# End Source File
# End Group
# Begin Group "AnimationSet"

# PROP Default_Filter ""
# Begin Group "AnimationSetInfo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_animation_set_info.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_set_info.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_animation_set.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_set.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_set_preview.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_set_preview.h
# End Source File
# End Group
# Begin Group "AnimationInfo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_animation_info.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_info.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_animation.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_frame.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_animation_frame.h
# End Source File
# End Group
# Begin Group "Video"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_video.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_video.h
# End Source File
# End Group
# Begin Group "Sound_"

# PROP Default_Filter ""
# Begin Group "SoundInfo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_sound_info.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_sound_info.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_sound.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_sound_handle.h
# End Source File
# End Group
# Begin Group "ScreenText"

# PROP Default_Filter ""
# Begin Group "Font No. 1"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_screen_text.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_screen_text.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_screen_text_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_screen_text_dispatcher.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_screen_text_set.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_screen_text_set.h
# End Source File
# End Group
# Begin Group "Music"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_music_track.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_music_track.h
# End Source File
# End Group
# Begin Group "qdResourceDispatcher"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_resource_container.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_resource_dispatcher.h
# End Source File
# End Group
# Begin Group "zipContainer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\zip_container.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\zip_container.h
# End Source File
# End Group
# Begin Group "TextDB"

# PROP Default_Filter ""
# Begin Group "Font No. 2"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_textdb.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_textdb.h
# End Source File
# End Group
# Begin Group "Font"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_font_info.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_font_info.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_file_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_file_manager.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_file_owner.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_file_owner.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_resource.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_resource_file.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_resource_file.h
# End Source File
# End Group
# Begin Group "GameSetup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_setup.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_setup.h
# End Source File
# End Group
# Begin Group "Base"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_named_object.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_named_object.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_named_object_base.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_named_object_base.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_named_object_indexer.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_named_object_indexer.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_named_object_reference.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_named_object_reference.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_object_list_container.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_object_map_container.h
# End Source File
# End Group
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Group "2PassScale"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\2PassScale.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\AIAStar.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\Filters.h
# End Source File
# End Group
# Begin Group "Pathfinding"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\AIAStar_API.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\AIAStar_API.h
# End Source File
# End Group
# Begin Group "FPS_Counter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\fps_counter.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\fps_counter.h
# End Source File
# End Group
# Begin Group "WinVideo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\WinVideo.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\WinVideo.h
# End Source File
# End Group
# Begin Group "ResourceDispatcher"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\Handle.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\ResourceDispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\ResourceDispatcher.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\SynchroTimer.h
# End Source File
# End Group
# Begin Group "SplashScreen"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\Util\splash_screen.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\splash_screen.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_rnd.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_rnd.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\qd_save_stream.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\qd_save_stream.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\Util\SafeMath.h
# End Source File
# End Group
# Begin Group "Parser"

# PROP Default_Filter ""
# Begin Group "XML"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Parser\xml_fwd.h
# End Source File
# Begin Source File

SOURCE=.\Parser\xml_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\Parser\xml_parser.h
# End Source File
# Begin Source File

SOURCE=.\Parser\xml_tag.h
# End Source File
# Begin Source File

SOURCE=.\Parser\xml_tag_buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Parser\xml_tag_buffer.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Parser\qdscr_convert.cpp
# End Source File
# Begin Source File

SOURCE=.\Parser\qdscr_convert.h
# End Source File
# Begin Source File

SOURCE=.\Parser\qdscr_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\Parser\qdscr_parser.h
# End Source File
# End Group
# Begin Group "GameInterface"

# PROP Default_Filter ""
# Begin Group "Contour"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_contour.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_contour.h
# End Source File
# End Group
# Begin Group "InterfaceBase"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_object_base.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_object_base.h
# End Source File
# End Group
# Begin Group "InterfaceElements"

# PROP Default_Filter ""
# Begin Group "InterfaceElementState"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_element_state.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_element_state.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_element_state_mode.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_element_state_mode.h
# End Source File
# End Group
# Begin Group "InterfaceButton"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_button.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_button.h
# End Source File
# End Group
# Begin Group "InterfaceBackground"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_background.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_background.h
# End Source File
# End Group
# Begin Group "InterfaceSlider"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_slider.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_slider.h
# End Source File
# End Group
# Begin Group "InterfaceSave"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_save.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_save.h
# End Source File
# End Group
# Begin Group "InterfaceTextWindow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_text_window.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_text_window.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_interface_element.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_element.h
# End Source File
# End Group
# Begin Group "InterfaceScreen"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_screen.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_screen.h
# End Source File
# End Group
# Begin Group "InterfaceDispatcher"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_interface_dispatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_interface_dispatcher.h
# End Source File
# End Group
# End Group
# Begin Group "GameLogic"

# PROP Default_Filter ""
# Begin Group "Conditions"

# PROP Default_Filter ""
# Begin Group "ConditionGroup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_condition_group.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_condition_group.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_condition.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_condition.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_condition_data.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_condition_data.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_condition_object_reference.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_condition_object_reference.h
# End Source File
# End Group
# Begin Group "Triggers"

# PROP Default_Filter ""
# Begin Group "TriggerProfiler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_trigger_profiler.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_trigger_profiler.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\qdCore\qd_trigger_chain.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_trigger_chain.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_trigger_element.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_trigger_element.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_trigger_element_boost.h
# End Source File
# End Group
# Begin Group "ConditionalObject"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_conditional_object.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_conditional_object.h
# End Source File
# End Group
# Begin Group "Counter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_counter.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_counter.h
# End Source File
# End Group
# End Group
# Begin Group "MiniGames"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\qdCore\qd_engine_interface.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_minigame.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_minigame.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_minigame_config.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_minigame_config.h
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_minigame_interface.cpp
# End Source File
# Begin Source File

SOURCE=.\qdCore\qd_minigame_interface.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\qd_fwd.h
# End Source File
# Begin Source File

SOURCE=.\qd_precomp.cpp
# ADD CPP /Yc"qd_precomp.h"
# End Source File
# Begin Source File

SOURCE=.\qd_precomp.h
# End Source File
# End Target
# End Project
