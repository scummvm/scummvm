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
	int32 _rexDeath = -1;
	int32 _canMove = -1;
};

static Scratch  local;


static void room_413_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('a', 2), 0);
	local._rexDeath = false;

	if (previous_room == 405) {
		player.x = 142;
		player.y = 146;
		player.facing = FACING_NORTH;
		player.walker_visible = true;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		if (global[kSexOfRex] == REX_MALE) {
			kernel_run_animation(kernel_full_name(413, 'd', 1, "", KERNEL_AA), 78);
			g_engine->_soundManager->command(30, 0);
			player.walker_visible = false;
			player.commands_allowed = false;
			local._rexDeath = true;
		} else if (!global[kTeleporterCommand]) {
			player.x = 136;
			player.y = 117;
			player_walk(141, 130, FACING_SOUTH);
			player.facing = FACING_SOUTH;
			player.walker_visible = true;
		}
	}

	if ((global[kTeleporterCommand]) && (!local._rexDeath)) {
		switch (global[kTeleporterCommand]) {
		case 1:
			g_engine->_soundManager->command(30, 0);
			player.walker_visible = false;
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 7, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[1], 1, 19);
			kernel_seq_depth(g_sequence_ids[1], 8);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 76);
			break;

		case 2:
			player.walker_visible = false;
			g_engine->_soundManager->command(30, 0);
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 7, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[1], 1, 20);
			kernel_seq_depth(g_sequence_ids[1], 8);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 77);
			break;

		case 3:
		case 4:
			player.x = 136;
			player.y = 117;
			player.facing = FACING_SOUTH;
			player_walk(141, 130, FACING_SOUTH);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		global[kTeleporterCommand] = 0;
	}

	local._canMove = true;
	section_4_music();
}

static void room_413_daemon() {
	if ((kernel_anim[0].anim != nullptr) && kernel_anim[0].frame == 38)
		kernel_reset_animation(0, 37);

	if ((kernel_anim[0].anim != nullptr) && kernel_anim[0].frame == 21 && local._canMove) {
		g_engine->_soundManager->command(27, 0);
		local._canMove = false;
	}

	if (kernel.trigger == 76) {
		player.x = 136;
		player.y = 117;
		player_walk(141, 130, FACING_SOUTH);
		player.facing = FACING_SOUTH;
		player_select_series();
		player.walker_visible = true;
		player.commands_allowed = true;
	}

	if (kernel.trigger == 77) {
		global[kTeleporterCommand] = TELEPORTER_BEAM_IN;
		new_room = global[kTeleporterDestination];
		kernel.force_restart = true;
	}

	if (kernel.trigger == 78) {
		kernel.force_restart = true;
		new_room = previous_room;
		global[kTeleporterCommand] = TELEPORTER_NONE;
	}
}

static void room_413_pre_parser() {
	if (player_said_1(take) || player_said_2(put, conveyor_belt))
		player.need_to_walk = false;

	if (player_said_2(look, wooden_statue) || player_said_2(look, display)
		|| player_said_2(look, picture) || player_said_2(look, plant)) {
		player.need_to_walk = true;
	}
}

static void room_413_parser() {
	if (player_said_2(walk_inside, teleporter)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		new_room = 409;
	} else if (player_said_2(walk_into, corridor_to_south))
		new_room = 405;
	else if (player_said_2(look, wooden_statue))
		text_show(41310);
	else if (player_said_2(take, wooden_statue))
		text_show(41311);
	else if (player_said_2(look, conveyor_belt))
		text_show(41312);
	else if (player_said_2(put, conveyor_belt))
		text_show(41313);
	else if (player_said_2(look, teleporter))
		text_show(41314);
	else if (player_said_2(look, display))
		text_show(41315);
	else if (player_said_2(look, corridor_to_south))
		text_show(41316);
	else if (player_said_2(look, picture))
		text_show(41317);
	else if (player_said_2(look, plant))
		text_show(41318);
	else if (player_said_2(take, plant))
		text_show(41319);
	else if (player.look_around)
		text_show(41320);
	else
		return;

	player.command_ready = false;
}

void room_413_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._rexDeath);
	s.syncAsSint32LE(local._canMove);
}

void room_413_preload() {
	room_init_code_pointer = room_413_init;
	room_pre_parser_code_pointer = room_413_pre_parser;
	room_parser_code_pointer = room_413_parser;
	room_daemon_code_pointer = room_413_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
