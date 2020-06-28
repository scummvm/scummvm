/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_time
#define FORBIDDEN_SYMBOL_EXCEPTION_localtime
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir

#include "engines/icb/common/px_windows.h"
#include <math.h>

#ifdef _WIN32
#include <direct.h> // _getcwd()
#else

#endif
#include <time.h>

#include "p4.h"

#include "keyboard.h"
#include "mouse.h"
#include "debug.h"

#include "res_man.h"

#include "direct_input.h"

#include "main_menu.h"
#include "gameover.h"

#include "engines/icb/common/px_common.h"

#include "stage_view.h"
#include "game_script.h"

#include "global_switches.h"

#if _PSX
#include "pause_menu_psx.h"
#include "options_menu_psx.h"
#include "load_save_psx.h"
#endif // #if   _PSX

#if _PC
#include "movie_pc.h"
#include "options_manager_pc.h"
#endif // #if   _PC

#include "mission.h"

#include "common/keyboard.h"
#include "common/textconsole.h"

namespace ICB {

// Res_open will compute the hash value and store it
uint32 font_cluster_hash = NULL_HASH;
// Res_open will compute the hash value and store it
uint32 sys_font_hash = NULL_HASH;

int gameCycle; // holds current game cycle - ticks up one each cycle

// The PSX specific code is in p4_psx.cpp
// PC specific code is in p4_pc.cpp

_stub::_stub() {
	// setup the stub object

	Zdebug("+constructing stub+");

	// set to level 0
	stub = 0;

	mode[0] = __no_stub_mode; // engine MUST set a real mode depending upon environment

	Timer_on();
#ifdef _PC
	cycle_speed = 100;
#endif
}

_stub::~_stub() {
	// kill the stub object

	Zdebug("*destructing stub*");
}

void _stub::Reset(__stub_modes new_mode) {
	stub = 0;
	mode[stub] = new_mode;
}

void _stub::Set_current_stub_mode(__stub_modes new_mode) {
	// force current mode

	mode[stub] = new_mode;
}

void _stub::Process_stub() {
	// call the mode!

#if _PC
	// update current keys
	Poll_direct_input();

	// Check for pause key .... moved from player::UpdateInputStates so the pause
	// menu is reachable regardless of the players state (ie in conversation)
	if (mode[stub] == __mission_and_console) {
		{
			if ((Read_DI_once_keys(pause_key)) || (Read_Joystick_once(pause_button))) {
				if (!g_theOptionsManager->HasControl())
					g_theOptionsManager->StartInGameOptions();
				return;
			}
		}
	}
#endif

	// reset the time equaliser
	Reset_timer();

	switch (mode[stub]) {
	case __no_stub_mode:
		Fatal_error("no stub mode set!");
		break; // should never happen, all this

	case __toe_on_door:
		Main_menu();
		break;

	case __mission_and_console:

		Mission_and_console();

#if _PC
		Fix_time();
#endif
		Update_screen();
		break;

	case __game_script:

		gs.Process_game_script();
		break;

	case __shift_mode:

		Headup_debug_switcher();
		Update_screen();
		break;

	case __troute:

		break;

	case __set_test:

		break;

	case __font_test:

		break;

	case __stage_view:
		// REMOVED
		break;

	case __sequence:

#if _PC
		// Ask to bink to display a frame of the movie
		int32 ret;
		ret = g_theSequenceManager->drawFrame();

		// Actions on return value
		if (ret == NOMOVIE) {
			// No movie registered (illegal use of this stub mode)
			Fatal_error("Can't draw movie frame when when no movie registered!");
		} else if (ret == FINISHED) {
			// Set any sounds active again
			g_TimerOn = TRUE8;
			UnpauseSounds();

			// All done so return to whatever was being done previously
			Pop_stub_mode();
		} else if (ret == WAITING) {
			// This smooths the playback framerate
			Fix_time();
		}
#else
		Fatal_error("Sequence stub mode not used for PSX");
#endif
		Update_screen();
		break;

	case __pause_menu:

		Pause_menu();
		break;

	case __gameover_menu:

		Gameover_menu();
		break;

	case __options_menu:

#if _PC
		Fatal_error("__options_menu stub not supported on PC");
#endif

#if _PSX
		Options_menu();
#endif
		break;

	case __load_save_menu:

#if _PC
		Fatal_error("__load_save_menu stub not supported on PC");
#endif

#if _PSX
		LoadSaveMenu();
#endif

		break;

	case __credits:

#if _PC
		Credits();
#endif

#if _PSX
		Fatal_error("__credits stub not supported on PSX");
#endif
		break;

	case __scrolling_text:

#if _PC
		ScrollingText();
#endif

#if _PSX
		Fatal_error("__scrolling_text stub not supported on PSX");
#endif
		break;

	default:

		Fatal_error("unsupported stub mode");
		break;
	}
}

void _stub::Update_screen() {
	// had to be split off to stop screen updates between stub cycles - i.e. sequence to game...
#if _PC

	// Record the next frame of the video if any
	static uint32 frameNumber = 0;
	if (px.recordingVideo)
		surface_manager->RecordFrame(pxVString("c:\\temp\\icb%05d.bmp", frameNumber++));

	// Grab screen shots if required
	if (Read_DI_keys(Common::KEYCODE_LCTRL) || Read_DI_keys(Common::KEYCODE_RCTRL)) {
		if (Read_DI_keys(Common::KEYCODE_s)) {
			// Take a screen grab
			struct tm *newtime;
			time_t int32_time;
			time(&int32_time);                                      /* Get time as int32 integer. */
			newtime = localtime(&int32_time);                       /* Convert to local time. */
			if (!checkFileExists(pxVString("%sScreenShots", root))) // TODO: Had amode = 0
				error("ICB wants to create folder %sScreenShots", root);
			/*
#ifdef _WIN32
				mkdir(pxVString("%sScreenShots", root));
#else
				mkdir(pxVString("%sScreenShots", root), 0755);
#endif
			*/
			surface_manager->RecordFrame(pxVString("%sScreenShots\\%02d_%02d_%04d__%02d_%02d_%02d.bmp", root, newtime->tm_mday, 1 + newtime->tm_mon,
			                                       1900 + newtime->tm_year, newtime->tm_hour, newtime->tm_min, newtime->tm_sec));
		}
	}

	g_icb_mission->flip_time = GetMicroTimer();
	// FLIP
	surface_manager->Flip();
	g_icb_mission->flip_time = GetMicroTimer() - g_icb_mission->flip_time;

#endif

#if _PSX
	if (mode[stub] != __mission_and_console)
		Flip();
#endif
}

void _stub::Push_stub_mode(__stub_modes new_mode) {
	// push mode

	stub++;
	if (stub >= TOTAL_STUBS) {
		Message_box("Push_stub gone too far %d MAX %d", stub, TOTAL_STUBS);
		stub = TOTAL_STUBS - 1;
	}
	mode[stub] = new_mode;
}

void _stub::Pop_stub_mode() {
	// pop mode

	if (stub)
		stub--;
}

__stub_modes _stub::Return_current_stub() {
	// return the mode
	// brother jake will know if in floors mode

	return (mode[stub]);
}

void _stub::Reset_timer() {
	stub_timer_time = g_system->getMillis();
}

void _stub::Timer_off() {
	// switch the frame fixer off

	timer = FALSE8;
}

void _stub::Timer_on() {
	// switch the frame fixer on

	timer = TRUE8;
}

bool8 _stub::Return_timer_status() {
	// return on/off status

	return (timer);
}

} // End of namespace ICB
