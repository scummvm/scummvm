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

#include "mads/core/cycle.h"
#include "mads/core/game.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_313_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	if ((previous_room == 366) || (previous_room == 316)) {
		player.x = 30;
		player.y = 80;
		player.facing = FACING_NORTH;
	} else if ((previous_room == 311) || (previous_room == 361) || (previous_room == 391)) {
		player.x = 90;
		player.y = 70;
		player.facing = FACING_EAST;
	} else if (previous_room == 390) {
		player.x = 126;
		player.y = 70;
		player.facing = FACING_EAST;
	} else if ((previous_room == 389) || (previous_room == 399)) {
		player.x = 163;
		player.y = 70;
		player.facing = FACING_WEST;
	} else if (previous_room == 388) {
		player.x = 199;
		player.y = 70;
		player.facing = FACING_WEST;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 234;
		player.y = 70;
		player.facing = FACING_WEST;
	}

	if (global[kAfterHavoc]) {
		for (uint16 i = 0; i < cycle_list.num_cycles; i++) {
			int palIdx = cycle_list.table[i].first_list_color;
			int size = cycle_list.table[i].num_colors * 3;
			memset(&cycling_palette[palIdx], 0, size);
			memset(&master_palette[palIdx], 0, size);
		}
	}

	section_3_music();
}

static void room_313_parser() {
	if (player_said_2(crawl_to, fourth_cell))
		new_room = 387;
	else if (player_said_2(crawl_to, third_cell))
		new_room = 388;
	else if (player_said_2(crawl_to, second_cell)) {
		if (global[kAfterHavoc])
			new_room = 399;
		else
			new_room = 389;
	} else if (player_said_2(crawl_to, first_cell))
		new_room = 390;
	else if (player_said_2(crawl_to, security_station)) {
		if (global[kSexOfRex] == REX_FEMALE) {
			global[kSexOfRex] = REX_MALE;
			text_show(31301);
		}
		new_room = 391;
	} else if (player_said_2(crawl_to, equipment_room)) {
		if (global[kSexOfRex] == REX_FEMALE) {
			global[kSexOfRex] = REX_MALE;
			text_show(31301);
		}
		new_room = 366;
	} else if (!player_said_2(crawl_down, air_shaft))
		return;

	player.command_ready = false;
}

void room_313_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_313_preload() {
	room_init_code_pointer = room_313_init;
	room_pre_parser_code_pointer = room_313_parser;
	section_3_walker();
	Common::strcpy_s(player.series_name, "RM313A");
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
