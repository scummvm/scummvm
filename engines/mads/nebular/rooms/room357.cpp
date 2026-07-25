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

static void room_357_init() {
	global[kAfterHavoc] = true;
	g_sprite_ids[1] = kernel_load_series(kernel_full_name(307, 'X', 0, "", KERNEL_SS), 0);
	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
	kernel_seq_loc(g_sequence_ids[1], 127, 78);
	kernel_seq_depth(g_sequence_ids[1], 15);

	if (previous_room == 318) {
		player.x = 298;
		player.y = 142;
	}
	else if (previous_room == 313) {
		player.x = 127;
		player.y = 101;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 15;
		player.y = 148;
	}

	section_3_music();
}

static void room_357_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east))
		player.walk_off_edge_to_room = 318;

	if (player_said_2(walk_down, corridor_to_west))
		player.walk_off_edge_to_room = 358;
}

static void room_357_parser() {
	if (player.look_around)
		text_show(35715);
	else if (player_said_2(look, air_vent))
		text_show(35710);
	else if (player_said_2(climb_into, air_vent))
		text_show(35711);
	else if (player_said_2(look, bed))
		text_show(35712);
	else if (player_said_2(look, sink))
		text_show(35713);
	else if (player_said_2(look, toilet))
		text_show(35714);
	else if (player_said_2(look, cell_wall))
		text_show(35716);
	else if (player_said_2(look, light))
		text_show(35717);
	else if (player_said_2(look, rip_in_floor))
		text_show(35718);
	else if (player_said_2(look, debris))
		text_show(35719);
	else if (player_said_2(take, debris))
		text_show(35720);
	else if (player_said_2(look, wall))
		text_show(35721);
	else if (player_said_2(look, corridor_to_east))
		text_show(35722);
	else if (player_said_2(look, corridor_to_west))
		text_show(35723);
	else
		return;

	player.command_ready = false;
}

void room_357_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_357_preload() {
	room_init_code_pointer = room_357_init;
	room_pre_parser_code_pointer = room_357_pre_parser;
	room_parser_code_pointer = room_357_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
