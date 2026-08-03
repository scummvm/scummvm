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

#include "math/utils.h"
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
	bool _northFl;
	int16 _dest_x;
	int16 _dest_y;
	int32 _timer;
};

static Scratch local;


static void room_401_init() {
	if (previous_room != KERNEL_RESTORING_GAME)
		local._northFl = false;

	local._timer = 0;

	if (previous_room == 402) {
		player.x = 203;
		player.y = 115;
		player.facing = FACING_WEST;
	} else if (previous_room == 354) {
		player.x = 149;
		player.y = 90;
		player.facing = FACING_SOUTH;
		local._northFl = true;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 142;
		player.y = 131;
		player.facing = FACING_NORTH;
	}

	kernel.quotes = quote_load(468, 0);
	section_4_music();
}

static void room_401_daemon() {
	if (kernel.trigger == 70) {
		new_room = 354;
		kernel.force_restart = true;
	}

	if (kernel.trigger == 80) {
		player.clock = kernel.clock - player.frame_delay;
		player.commands_allowed = true;
		player.walker_visible = true;
		local._northFl = false;
		player_walk(149, 110, FACING_SOUTH);
	}

	if (kernel.clock >= local._timer) {
		int dist = 64 - ((Math::hypotenuse(player.x - 219, player.y - 115) * 64) / 120);

		if (dist > 64)
			dist = 64;
		else if (dist < 1)
			dist = 1;

		g_engine->_soundManager->command(12, dist);
		local._timer = kernel.clock + player.frame_delay;
	}

}

static void room_401_pre_parser() {
	if (player_said_2(walk_down, corridor_to_north)) {
		player_walk(149, 89, FACING_NORTH);
		local._northFl = false;
	}

	if (player_said_2(walk_down, corridor_to_south) && !local._northFl)
		player.walk_off_edge_to_room = 405;

	if (player_said_1(take))
		player.need_to_walk = false;

	if (player.need_to_walk && local._northFl) {
		if (global[kSexOfRex] == REX_MALE) {
			local._dest_x = 148;
			local._dest_y = 94;
		} else {
			local._dest_x = 149;
			local._dest_y = 99;
		}

		player_walk(local._dest_x, local._dest_y, FACING_SOUTH);
	}
}

static void room_401_parser() {
	if (player.x == local._dest_x && player.y && local._northFl) {
		if (global[kSexOfRex] == REX_MALE) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			player.commands_allowed = false;
			player.walker_visible = false;
			g_engine->_soundManager->command(21, 0);
			kernel_run_animation(kernel_name('s', 1), 70);
			global[kHasBeenScanned] = true;
			g_engine->_soundManager->command(22, 0);
			int idx = kernel_message_add(quote_string(kernel.quotes, 0x1D4), 153, 46, 0x1110, 60, 0, 32);
			kernel_message_teletype(idx, 4, true);
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

	if (player_said_2(walk_into, bar)) {
		if (!local._northFl)
			new_room = 402;
	} else if (player_said_2(walk_down, corridor_to_north))
		new_room = 354;
	else if (player_said_2(look, scanner)) {
		if (global[kHasBeenScanned])
			text_show(40111);
		else
			text_show(40110);
	} else if (player_said_2(look, bar))
		text_show(40112);
	else if (player_said_2(look, sign))
		text_show(40113);
	else if (player_said_2(look, corridor_to_south))
		text_show(40114);
	else if (player_said_2(look, corridor_to_north))
		text_show(40115);
	else if (player.look_around)
		text_show(40116);
	else
		return;

	player.command_ready = false;
}

void room_401_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._northFl);
	s.syncAsSint16LE(local._dest_x);
	s.syncAsSint16LE(local._dest_y);
	s.syncAsUint32LE(local._timer);
}

void room_401_preload() {
	room_init_code_pointer = room_401_init;
	room_daemon_code_pointer = room_401_daemon;
	room_pre_parser_code_pointer = room_401_pre_parser;
	room_parser_code_pointer = room_401_parser;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
