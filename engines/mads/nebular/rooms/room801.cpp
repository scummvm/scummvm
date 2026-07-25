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
#include "mads/nebular/rooms/section8.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _walkThroughDoor;
};

static Scratch local;


static void room_801_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 3), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('a', -1), 0);

	if (previous_room != 802) {
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 5);
		kernel_seq_depth(g_sequence_ids[2], 13);
	}

	if ((global[kCameFromCut]) && (global[kCutX] != 0)) {
		player.x = global[kCutX];
		player.y = global[kCutY];
		player.facing = global[kCutFacing];
		global[kCutX] = 0;
		global[kCameFromCut] = false;
		global[kReturnFromCut] = false;
		global[kBeamIsUp] = false;
		global[kForceBeamDown] = false;
		global[kDontRepeat] = false;
	} else if (previous_room == 808) {
		player.x = 148;
		player.y = 110;
		player.facing = FACING_NORTH;
	} else if (previous_room == 802) {
		player.x = 307;
		player.y = 111;
		player_walk(270, 118, FACING_WEST);
		player.walker_visible = true;
	} else if ((previous_room != KERNEL_RESTORING_GAME) && !global[kTeleporterCommand]) {
		player.x = 8;
		player.y = 117;
		player_walk(41, 115, FACING_EAST);
		player.walker_visible = true;
	}

	global[kBetweenRooms] = false;

	if (global[kTeleporterCommand]) {
		player.commands_allowed = false;
		switch (global[kTeleporterCommand]) {
		case 1:
			player.x = 8;
			player.y = 117;
			global[kTeleporterUnderstood] = true;
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[1], 1, 13);
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 75);
			g_engine->_soundManager->command(30, 0);
			break;

		case 2:
			player.x = 8;
			player.y = 117;
			global[kTeleporterUnderstood] = true;
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[1], 1, 13);
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 80);
			g_engine->_soundManager->command(30, 0);
			break;

		case 3:
		case 4:
			player.x = 8;
			player.y = 117;
			player_walk(41, 115, FACING_EAST);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		global[kTeleporterCommand] = 0;
	}

	local._walkThroughDoor = false;
	if (previous_room == 802) {
		player.commands_allowed = false;
		local._walkThroughDoor = true;
	}

	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 11, 0, 0, 0);
	kernel_seq_range(g_sequence_ids[3], -1, -2);
	kernel_seq_depth(g_sequence_ids[3], 14);

	g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 9, 0, 0, 0);
	kernel_seq_range(g_sequence_ids[4], -1, -2);
	kernel_seq_depth(g_sequence_ids[4], 14);

	section_8_music();
}

static void room_801_daemon() {
	if (kernel.trigger == 75) {
		if (global[kSexOfRex] == REX_FEMALE) {
			g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 8, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[5], 1, 8);
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 140);
		} else {
			player.commands_allowed = true;
			player.walker_visible = true;
			player.x = 8;
			player.y = 117;
			player_walk(41, 115, FACING_EAST);
		}
	}

	if (kernel.trigger == 140) {
		g_engine->_soundManager->command(27, 0);
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 8);
		kernel_timing_trigger(100, 141);
	}

	if (kernel.trigger == 141) {
		kernel.force_restart = true;
		new_room = previous_room;
		global[kTeleporterCommand] = 0;
	}

	if (kernel.trigger == 80) {
		global[kTeleporterCommand] = 1;
		new_room = global[kTeleporterDestination];
		kernel.force_restart = true;
	}

	if (local._walkThroughDoor && (Common::Point(player.x, player.y) == Common::Point(270, 118))) {
		player.commands_allowed = false;
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 4, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], 1, 5);
		kernel_seq_depth(g_sequence_ids[2], 10);
		local._walkThroughDoor = false;
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 120);
	}

	if (kernel.trigger == 120) {
		g_engine->_soundManager->command(12, 0);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 5);
		kernel_seq_depth(g_sequence_ids[2], 10);
		player.commands_allowed = true;
	}

	if (kernel.trigger == 90) {
		player_walk(307, 111, FACING_EAST);
		kernel_timing_trigger(80, 130);
	}

	if (kernel.trigger == 130) {
		g_engine->_soundManager->command(12, 0);
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 4, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], 1, 5);
		kernel_seq_depth(g_sequence_ids[2], 10);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 110);
	}

	if (kernel.trigger == 110) {
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 5);
		kernel_seq_depth(g_sequence_ids[2], 10);
		new_room = 802;
	}
}

static void room_801_pre_parser() {
	if (player_said_2(look, control_panel)) {
		player_walk(148, 110, FACING_NORTH);
		player.need_to_walk = true;
		player.ready_to_walk = true;
	}

	if (player_said_2(walk_inside, teleporter) && global[kBeamIsUp]) {
		global[kCutX] = player.x;
		global[kCutY] = player.y;
		global[kCutFacing] = player.facing;
		global[kForceBeamDown] = true;
		global[kDontRepeat] = true;
		new_room = 803;
	}
}

static void room_801_parser() {
	if (player_said_2(look, control_panel))
		new_room = 808;
	else if (player_said_2(walk_inside, teleporter)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		new_room = 807;
	} else if (player_said_2(walk_through, door) && (Common::Point(player.x, player.y) == Common::Point(270, 118))) {
		player.commands_allowed = false;
		player.facing = FACING_EAST;
		player_select_series();
		global[kBetweenRooms] = true;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 4, 0, 0, 1);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 90);
		kernel_seq_range(g_sequence_ids[2], 1, 5);
		kernel_seq_depth(g_sequence_ids[2], 13);
		g_engine->_soundManager->command(11, 0);
	} else if (player_said_2(look, ceiling))
		text_show(80110);
	else if (player_said_2(look, monitor))
		text_show(80111);
	else if (player_said_2(look, teleporter))
		text_show(80112);
	else if (player_said_2(look, equipment) || player.look_around)
		text_show(80113);
	else if (player_said_2(look, speaker))
		text_show(80114);
	else if (player_said_2(look, eye_chart))
		text_show(80115);
	else if (player_said_2(look, wall))
		text_show(80116);
	else if (player_said_2(look, door))
		text_show(80117);
	else
		return;

	player.command_ready = false;
}

void room_801_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._walkThroughDoor);
}

void room_801_preload() {
	room_init_code_pointer = room_801_init;
	room_daemon_code_pointer = room_801_daemon;
	room_pre_parser_code_pointer = room_801_pre_parser;
	room_parser_code_pointer = room_801_parser;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
