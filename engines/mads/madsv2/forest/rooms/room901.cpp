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

#include "common/config-manager.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/forest/rooms/section9.h"
#include "mads/madsv2/forest/rooms/room901.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/forest.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

/* Triggers */
#define START_INTRO  100

static void room_901_init() {
	global[g009] = 0;
	global[player_score] = 0;
	player.walker_visible = false;
	viewing_at_y = 22;
	mouse_hide();

	if (ConfMan.getBool("seen_intro")) {
		new_room = 904;
	} else {
		kernel_timing_trigger(300, START_INTRO);
	}
}

static void room_901_daemon() {
	if (mouse_any_stroke || g_engine->hasPendingKey() || kernel.trigger == START_INTRO) {
		g_engine->flushKeys();
		mouse_hide();
		new_room = 903;
	}
}

static void room_901_pre_parser() {
	new_room = 903;
}

static void room_901_parser() {
	// No implementation
}

void room_901_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_901_preload() {
	room_init_code_pointer = room_901_init;
	room_pre_parser_code_pointer = room_901_pre_parser;
	room_parser_code_pointer = room_901_parser;
	room_daemon_code_pointer = room_901_daemon;

	global[g016] = true;
	mouse_hide();
	section_9_walker();
	section_9_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
