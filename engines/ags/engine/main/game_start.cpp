/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//
// Game initialization
//

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/screen.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/game_start.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/script/script.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void start_game_init_editor_debugging() {
	Debug::Printf(kDbgMsg_Info, "Try connect to the external debugger");
	if (!init_editor_debugging())
		return;

	// Debugger expects strict multitasking
	_GP(usetup).multitasking = true;
	_GP(usetup).override_multitasking = -1;
	SetMultitasking(1);

	auto waitUntil = AGS_Clock::now() + std::chrono::milliseconds(500);
	while (waitUntil > AGS_Clock::now()) {
		// pick up any breakpoints in game_start
		check_for_messages_from_debugger();
	}

	ccSetDebugHook(scriptDebugHook);
}

static void start_game_load_savegame_on_startup(int loadSave) {
	if (loadSave != -1) {
		current_fade_out_effect();
		try_restore_save(loadSave);
	}
}

void start_game() {
	set_room_placeholder();
	set_cursor_mode(MODE_WALK);
	_GP(mouse).SetPosition(Point(160, 100));
	newmusic(0);

	set_our_eip(-42);

	// skip ticks to account for initialisation or a restored game.
	skipMissedTicks();

	RunScriptFunctionInModules("game_start");

	set_our_eip(-43);

	// Only auto-set first restart point in < 3.6.1 games,
	// since 3.6.1+ users are suggested to set one manually in script.
	if (_G(loaded_game_file_version) < kGameVersion_361_10)
		SetRestartPoint();

	set_our_eip(-3);

	if (_G(displayed_room) < 0) {
		current_fade_out_effect();
		load_new_room(_G(playerchar)->room, _G(playerchar));
	}

	first_room_initialization();
}

void initialize_start_and_play_game(int override_start_room, int loadSave) {
	//try { // BEGIN try for ALI3DEXception

	set_cursor_mode(MODE_WALK);

	if (override_start_room)
		_G(playerchar)->room = override_start_room;

	Debug::Printf(kDbgMsg_Info, "Engine initialization complete");
	Debug::Printf(kDbgMsg_Info, "Starting game");

	if (_G(editor_debugging_enabled))
		start_game_init_editor_debugging();

	start_game_load_savegame_on_startup(loadSave);

	// only start if not restored a save
	if (_G(displayed_room) < 0)
		start_game();

	RunGameUntilAborted();

	/*} catch (Ali3DException gfxException) {
		quit(gfxException.Message.GetCStr());
	}*/
}

} // namespace AGS3
