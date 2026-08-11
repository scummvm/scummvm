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

static void room_354_init() {
	global[kAfterHavoc] = true;
	global[kTeleporterRoom + 1] = 351;

	if (previous_room == 361) {
		player.x = 231;
		player.y = 110;
	}
	else if (previous_room == 401) {
		player.x = 106;
		player.y = 152;
		player.facing = FACING_NORTH;
	} else if (previous_room == 316) {
		player.x = 71;
		player.y = 107;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 167;
		player.y = 57;
	}

	section_3_music();
}

static void room_354_pre_parser() {
	if (player_said_2(walk_down, corridor_to_south))
		player.walk_off_edge_to_room = 401;
}

static void room_354_parser() {
	if (player.look_around)
		text_show(35414);
	else if (player_said_2(walk_down, corridor_to_north)) {
		player_start_walking(208, 0, FACING_NORTHEAST);
		player.walk_off_edge_to_room = 353;
	} else if (player_said_2(walk_down, corridor_to_east))
		new_room = 361;
	else if (player_said_2(walk_down, corridor_to_west))
		new_room = 316;
	else if (player_said_2(walk_down, corridor_to_south))
		new_room = 401;
	else if (player_said_2(look, controls))
		text_show(35410);
	else if (player_said_2(look, signal))
		text_show(35411);
	else if (player_said_2(look, catwalk))
		text_show(35412);
	else if (player_said_2(look, air_duct))
		text_show(35413);
	else if (player_said_2(look, corridor_to_north))
		text_show(35415);
	else if (player_said_2(look, corridor_to_south))
		text_show(35416);
	else if (player_said_2(look, corridor_to_east))
		text_show(35417);
	else if (player_said_2(look, corridor_to_west))
		text_show(35418);
	else if (player_said_2(look, debris))
		text_show(35419);
	else if (player_said_2(look, guard))
		text_show(35420);
	else
		return;

	player.command_ready = false;
}

void room_354_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_354_preload() {
	room_init_code_pointer = room_354_init;
	room_pre_parser_code_pointer = room_354_pre_parser;
	room_parser_code_pointer = room_354_parser;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
