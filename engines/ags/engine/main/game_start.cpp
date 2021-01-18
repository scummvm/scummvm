/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

//
// Game initialization
//

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/characterinfo.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/screen.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/gfx/ali3dexception.h"
#include "ags/engine/main/mainheader.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/game_start.h"
#include "ags/engine/script/script.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/globals.h"
#include "ags/ags.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern int our_eip, displayed_room;
extern GameSetupStruct game;
extern GameState play;
extern std::vector<ccInstance *> moduleInst;
extern int numScriptModules;
extern CharacterInfo *playerchar;
extern int convert_16bit_bgr;

void start_game_init_editor_debugging() {
	if (editor_debugging_enabled) {
		SetMultitasking(1);
		if (init_editor_debugging()) {
			auto waitUntil = AGS_Clock::now() + std::chrono::milliseconds(500);
			while (waitUntil > AGS_Clock::now()) {
				// pick up any breakpoints in game_start
				check_for_messages_from_editor();
			}

			ccSetDebugHook(scriptDebugHook);
		}
	}
}

void start_game_load_savegame_on_startup() {
	if (_G(loadSaveGameOnStartup) != nullptr) {
		int saveGameNumber = 1000;
		const char *sgName = strstr(_G(loadSaveGameOnStartup), "agssave.");
		if (sgName != nullptr) {
			sscanf(sgName, "agssave.%03d", &saveGameNumber);
		}
		current_fade_out_effect();
		try_restore_save(_G(loadSaveGameOnStartup), saveGameNumber);
	}
}

void start_game() {
	set_cursor_mode(MODE_WALK);
	Mouse::SetPosition(Point(160, 100));
	newmusic(0);

	our_eip = -42;

	// skip ticks to account for initialisation or a restored game.
	skipMissedTicks();

	for (int kk = 0; kk < numScriptModules; kk++)
		RunTextScript(moduleInst[kk], "game_start");

	RunTextScript(gameinst, "game_start");

	our_eip = -43;

	SetRestartPoint();

	our_eip = -3;

	if (displayed_room < 0) {
		current_fade_out_effect();
		load_new_room(playerchar->room, playerchar);
		// load_new_room updates it, but it should be -1 in the first room
		playerchar->prevroom = -1;
	}

	first_room_initialization();
}

void do_start_game() {
	// only start if replay playback hasn't loaded a game
	if (displayed_room < 0)
		start_game();
}

void initialize_start_and_play_game(int override_start_room, const char *loadSaveOnStartup) {
//	try { // BEGIN try for ALI3DEXception

		set_cursor_mode(MODE_WALK);

		if (convert_16bit_bgr) {
			// Disable text as speech while displaying the warning message
			// This happens if the user's graphics card does BGR order 16-bit colour
			int oldalways = game.options[OPT_ALWAYSSPCH];
			game.options[OPT_ALWAYSSPCH] = 0;
			// PSP: This is normal. Don't show a warning.
			//Display ("WARNING: AGS has detected that you have an incompatible graphics card for this game. You may experience colour problems during the game. Try running the game with \"--15bit\" command line parameter and see if that helps.[[Click the mouse to continue.");
			game.options[OPT_ALWAYSSPCH] = oldalways;
		}

		::AGS::g_vm->setRandomNumberSeed(play.randseed);
		if (override_start_room)
			playerchar->room = override_start_room;

		Debug::Printf(kDbgMsg_Info, "Engine initialization complete");
		Debug::Printf(kDbgMsg_Info, "Starting game");

		start_game_init_editor_debugging();

		start_game_load_savegame_on_startup();

		do_start_game();

		RunGameUntilAborted();

//	} catch (Ali3DException gfxException) {
//		quit((char *)gfxException._message);
//	}
}

} // namespace AGS3
