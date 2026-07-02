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

#include "engines/util.h"
#include "mads/madsv2/console.h"
#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mps_archive.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/forest.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/inventory.h"
#include "mads/madsv2/forest/main.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/rooms/section2.h"
#include "mads/madsv2/forest/rooms/section3.h"
#include "mads/madsv2/forest/rooms/section4.h"
#include "mads/madsv2/forest/rooms/section5.h"
#include "mads/madsv2/forest/rooms/section9.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/core/mps_installer.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

ForestEngine *g_engine;

ForestEngine::ForestEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		MADSV2Engine(syst, gameDesc) {
	g_engine = this;
	init_extra();
	init_inventory();
}

ForestEngine::~ForestEngine() {
	g_engine = nullptr;
}

Common::Error ForestEngine::run() {
	initGraphics(320, 200);
	_screen = new Graphics::Screen();
	scr_live.data = (byte *)_screen->getPixels();

	// Create a debugger console
	setDebugger(new Console());

	// Set up to read mpslabs installer archive if needed
	if (_gameDescription->desc.flags & GF_INSTALLER) {
		MpsArchive *arch = MpsArchive::open();
		if (arch)
			SearchMan.add("mpslabs", arch);
	}

	_midiPlayer.open();

	// Run the game
	Forest::forest_main();

	return Common::kNoError;
}

void ForestEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_midiPlayer.syncSoundSettings();
}

void ForestEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	_midiPlayer.pause(pause);
}

void ForestEngine::global_init_code() {
	Common::fill(global, global + GLOBAL_LIST_SIZE, 0);
	Common::fill(flags, flags + 40, 0);

	flags[0] = flags[1] = flags[2] = flags[3] = -4;
	flags[4] = flags[5] = 4;
	flags[34] = flags[35] = flags[36] = 4;

	global[g009] = -1;
	global[player_score] = -1;
	global[g022] = 0;

	// Randomly select the destinations for the passageways in the underground quick transport area
	int16 table[5] = { 106, 203, 302, 305, 307 };
	int16 *const global_ptrs[5] = {
		&global[tunnel_1_room], &global[tunnel_2_room], &global[tunnel_3_room],
		&global[tunnel_4_room], &global[tunnel_5_room]
	};

	for (int16 *dest : global_ptrs) {
		int pick;
		do {
			int index = imath_random(0, 4);
			pick = table[index];
			table[index] = 0;
		} while (pick == 0);
		*dest = pick;
	}

	global[player_selected_object] = -1;
	global[g017] = -1;
	global[intro] = 0;
	global[outro] = 0;
	global[g066] = 0;
	global[walker_converse_now] = 0;

	player.facing = FACING_NORTH;
	player.turn_to_facing = FACING_NORTH;
}

void ForestEngine::section_music(int section_num) {
	switch (section_num) {
	case 1: Rooms::section_1_music(); break;
	case 2: Rooms::section_2_music(); break;
	case 3: Rooms::section_3_music(); break;
	case 4: Rooms::section_4_music(); break;
	case 5: Rooms::section_5_music(); break;
	case 9: Rooms::section_9_music(); break;
	}
}

void ForestEngine::global_section_constructor() {
	Forest::global_section_constructor();
}

bool ForestEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return game.going && !win_status && !kernel.activate_menu && player.commands_allowed &&
		inter_input_mode == INTER_LIMITED_SENTENCES && section_id != 9;
}

void ForestEngine::syncRoom(Common::Serializer &s) {
	Forest::sync_room(s);
}

void ForestEngine::global_daemon_code() {
	Forest::global_daemon_code();
}

void ForestEngine::global_pre_parser_code() {
	if (player_said_1(look) || player_said_1(throw)) {
		player.need_to_walk = false;
	}
}

void ForestEngine::global_parser_code() {
	// No implementation
}

void ForestEngine::global_error_code() {
	Forest::global_error_code();
}

void ForestEngine::global_room_init() {
	Forest::global_room_init();
}

void ForestEngine::global_sound_driver() {
	Common::strcpy_s(kernel.sound_driver, "/");
	env_catint(kernel.sound_driver, new_section, 1);
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
