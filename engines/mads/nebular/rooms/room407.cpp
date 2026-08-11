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
#include "mads/nebular/rooms/section4.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _fromNorth;
	int16 _dest_x;
	int16 _dest_y;
};

static Scratch local;


static void room_407_init() {
	if (previous_room != KERNEL_RESTORING_GAME)
		local._fromNorth = false;

	if (previous_room == 318) {
		player.x = 172;
		player.y = 92;
		player.facing = FACING_SOUTH;
		local._fromNorth = true;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 172;
		player.y = 132;
		player.facing = FACING_NORTH;
	}

	kernel.quotes = quote_load(592, 0);
	section_4_music();
}

static void room_407_daemon() {
	if (kernel.trigger == 70) {
		new_room = 318;
		kernel.force_restart = true;
	}

	if (kernel.trigger == 80) {
		player.clock = kernel.clock - player.frame_delay;
		player.commands_allowed = true;
		player.walker_visible = true;
		local._fromNorth = false;
		player_walk(173, 104, FACING_SOUTH);
	}
}

static void room_407_pre_parser() {
	if (player_said_1(take))
		player.need_to_walk = false;

	if (player_said_2(look, door))
		player.need_to_walk = true;

	if (player_said_2(walk_down, corridor_to_north)) {
		player_walk(172, 91, FACING_NORTH);
		local._fromNorth = false;
	}

	if (player.need_to_walk && local._fromNorth) {
		if (global[kSexOfRex] == REX_MALE) {
			local._dest_x = 171;
			local._dest_y = 95;
		} else {
			local._dest_x = 173;
			local._dest_y = 96;
		}
		player_walk(local._dest_x, local._dest_y, FACING_SOUTH);
	}
}

static void room_407_parser() {
	if (player.x == local._dest_x && player.y == local._dest_y && local._fromNorth) {
		if (global[kSexOfRex] == REX_MALE) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			player.commands_allowed = false;
			player.walker_visible = false;
			g_engine->_soundManager->command(21, 0);
			kernel_run_animation(kernel_name('s', 1), 70);
			global[kHasBeenScanned] = true;
			kernel_message_add(quote_string(kernel.quotes, 592), 0, 0, 0x1110, 60, 0, 34);
			g_engine->_soundManager->command(22, 0);
		}

		if (global[kSexOfRex] == REX_FEMALE) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			player.commands_allowed = false;
			player.walker_visible = false;
			g_engine->_soundManager->command(21, 0);
			kernel_run_animation(kernel_name('s', 2), 80);
			g_engine->_soundManager->command(23, 0);
			global[kHasBeenScanned] = true;
		}
	}

	if (player_said_2(walk_down, corridor_to_south) && !local._fromNorth)
		new_room = 406;
	else if (player_said_2(walk_down, corridor_to_north))
		new_room = 318;
	else if (player_said_2(look, scanner)) {
		if (global[kHasBeenScanned])
			text_show(40711);
		else
			text_show(40710);
	} else if (player_said_2(look, door))
		text_show(40712);
	else if (player_said_2(look, corridor_to_south))
		text_show(40713);
	else if (player_said_2(look, corridor_to_north))
		text_show(40714);
	else if (player.look_around)
		text_show(40715);
	else
		return;

	player.command_ready = false;
}

void room_407_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._fromNorth);
	s.syncAsSint16LE(local._dest_x);
	s.syncAsSint16LE(local._dest_y);
}

void room_407_preload() {
	room_init_code_pointer = room_407_init;
	room_pre_parser_code_pointer = room_407_pre_parser;
	room_parser_code_pointer = room_407_parser;
	room_daemon_code_pointer = room_407_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
