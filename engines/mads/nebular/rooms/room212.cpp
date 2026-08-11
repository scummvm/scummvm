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
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_212_init() {
	if (previous_room == 208) {
		player.x = 195;
		player.y = 85;
		player.facing = FACING_SOUTH;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 67;
		player.y = 117;
		player.facing = FACING_NORTHEAST;
	}

	section_2_music();
}

static void room_212_daemon() {
	// No implementation
}

static void room_212_pre_parser() {
	if (player_said_2(walk_through, cave_entrance))
		player.walk_off_edge_to_room = 111;
}

static void room_212_parser() {
	if (player.look_around)
		text_show(21209);
	else if (player_said_1(walk_towards) && (player_said_1(field_to_north) || player_said_1(mountains)))
		new_room = 208;
	else if (player_said_2(walk_towards, cave))
		new_room = 111;
	else if (player_said_2(look, grass))
		text_show(21201);
	else if (player_said_2(look, rocks))
		text_show(21202);
	else if (player_said_2(look, cave_entrance))
		text_show(21203);
	else if (player_said_2(look, sky))
		text_show(21204);
	else if (player_said_2(look, field_to_north))
		text_show(21205);
	else if (player_said_2(look, trees))
		text_show(21206);
	else if (player_said_2(look, plants))
		text_show(21207);
	else if (player_said_2(look, mountains))
		text_show(21208);
	else
		return;

	player.command_ready = false;
}

void room_212_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_212_preload() {
	room_init_code_pointer = room_212_init;
	room_pre_parser_code_pointer = room_212_pre_parser;
	room_parser_code_pointer = room_212_parser;
	room_daemon_code_pointer = room_212_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(words_bouncing_reptile);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
