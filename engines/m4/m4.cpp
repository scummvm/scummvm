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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "m4/m4.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/conv_io.h"
#include "m4/gui/hotkeys.h"
#include "m4/platform/sound.h"
#include "m4/detection.h"
#include "m4/console.h"
#include "m4/core/param.h"

namespace M4 {

M4Engine *g_engine;

M4Engine::M4Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("M4") {
	g_engine = this;
}

M4Engine::~M4Engine() {
	delete _screen;
}

uint32 M4Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String M4Engine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error M4Engine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 480);

	// Instantiate globals and setup
	Vars *vars = createVars();

	if (vars->init()) {
		// Run game here
		m4_inflight();
	}

	delete vars;
	return Common::kNoError;
}

void M4Engine::m4_inflight() {
	Hotkeys::add_hot_keys();

	while (_G(game).going) {
		if (_G(game).previous_room == -2) {
			midi_stop();
			kernel_load_game(_G(kernel).restore_slot);
		}
#ifdef TODO
		// Start up next section
		_G(between_rooms) = true;
		global_section_constructor();
		util_exec_function(section_preload_code_pointer);
		kernel.going = kernel_section_startup();
		util_exec_function(section_init_code_pointer);
#endif
		// TODO
	}
}

Common::Error M4Engine::syncGame(Common::Serializer &s) {
	_G(game).syncGame(s);
	_G(player).syncGame(s);
	_G(player_info).syncGame(s);
	g_vars->getGlobals()->syncGame(s);
	player_been_sync(s);
	_G(conversations).syncGame(s);
	_G(inventory)->syncGame(s);


	if (s.isLoading()) {
		// set up variables so everyone knows we've teleported
		_G(kernel).restore_game = true;
		_G(game).previous_room = KERNEL_RESTORING_GAME;

#ifdef TODO
		digi_set_overall_volume(game.digi_overall_volume_percent);
		midi_set_overall_volume(game.midi_overall_volume_percent);
#endif
	}

	return Common::kNoError;
}

} // End of namespace M4
