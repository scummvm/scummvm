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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section7.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_710_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	if (object[OBJ_VASE].location == 706) {
		g_sprite_ids[1] = kernel_load_series(kernel_name('g', -1), 0);
		g_sequence_ids[1] = kernel_seq_pingpong(g_sprite_ids[1], false, 6, 0, 0, 0);
	}

	player.walker_visible = false;
	kernel_timing_trigger(600, 70);

	section_7_music();
}

static void room_710_daemon() {
	if (kernel.trigger == 70) {
		if (global[kCityFlooded])
			new_room = 701;
		else
			new_room = 751;
	}
}

static void room_710_parser() {
	if (player_said_2(put_down, binoculars)) {
		player.commands_allowed = false;

		if (global[kCityFlooded])
			new_room = 701;
		else
			new_room = 751;

		player.command_ready = false;
	}
}

void room_710_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_710_preload() {
	room_init_code_pointer = room_710_init;
	room_daemon_code_pointer = room_710_daemon;
	room_parser_code_pointer = room_710_parser;

	*player.series_name = '\0';
	section_7_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
