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

#include "mads/madsv2/forest/rooms/section9.h"
#include "mads/madsv2/forest/forest.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

/* Triggers */
#define TRIGGER0 100
#define TRIGGER1 101


static void room_903_init() {
	mouse_hide();
	global[g009] = false;
	global[player_score] = false;
	viewing_at_y = 22;
	kernel_timing_trigger(20, TRIGGER1);
	kernel_run_animation("*rm903i", TRIGGER0);
	player.walker_visible = false;
}

static void room_903_daemon() {
	if (kernel.trigger == TRIGGER1)
		global_midi_play(14);

	if (mouse_any_stroke || g_engine->hasPendingKey() || kernel.trigger == TRIGGER0) {
		g_engine->flushKeys();
		new_room = 904;
	}
}

static void room_903_pre_parser() {
	new_room = 904;
}

static void room_903_parser() {
}

void room_903_synchronize(Common::Serializer &s) {
	// Room has no scratch area
}

void room_903_preload() {
	room_init_code_pointer = room_903_init;
	room_pre_parser_code_pointer = room_903_pre_parser;
	room_parser_code_pointer = room_903_parser;
	room_daemon_code_pointer = room_903_daemon;

	global[g016] = true;
	section_9_walker();
	section_9_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
