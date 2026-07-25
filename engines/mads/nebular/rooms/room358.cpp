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
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_358_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_full_name(307, 'X', 0, "", KERNEL_SS), 0);
	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
	kernel_seq_loc(g_sequence_ids[1], 127, 78);
	kernel_seq_depth(g_sequence_ids[1], 15);

	if (previous_room == 357) {
		player.x = 305;
		player.y = 142;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 12;
		player.y = 141;
	}

	section_3_music();
}

static void room_358_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east))
		player.walk_off_edge_to_room = 357;

	if (player_said_2(walk_down, corridor_to_west))
		player.walk_off_edge_to_room = 359;
}

static void room_358_parser() {
	if (player.look_around)
		text_show(35815);
	else if (player_said_2(look, corridor_to_west))
		text_show(35810);
	else if (player_said_2(look, corridor_to_east))
		text_show(35811);
	else if (player_said_2(look, bed))
		text_show(35812);
	else if (player_said_2(look, sink))
		text_show(35813);
	else if (player_said_2(look, toilet))
		text_show(35814);
	else if (player_said_2(look, corridor) || player_said_2(look, wall))
		text_show(35816);
	else if (player_said_2(look, air_vent))
		text_show(35817);
	else
		return;

	player.command_ready = false;
}

void room_358_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_358_preload() {
	room_init_code_pointer = room_358_init;
	room_pre_parser_code_pointer = room_358_pre_parser;
	room_parser_code_pointer = room_358_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
