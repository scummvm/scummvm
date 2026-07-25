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
#include "mads/nebular/rooms/section5.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_551_init() {
	if (global[kSexOfRex] == REX_MALE)
		g_sprite_ids[2] = kernel_load_series(kernel_name('a', 0), 0);
	else
		g_sprite_ids[3] = kernel_load_series(kernel_name('a', 1), 0);

	if (previous_room == 501) {
		player.x = 18;
		player.y = 130;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 124;
		player.y = 119;
		player.facing = FACING_NORTH;
	}

	if (global[kTeleporterCommand]) {
		player.walker_visible = false;
		player.commands_allowed = false;

		char sepChar;
		if (global[kSexOfRex] == REX_MALE)
			sepChar = 'e';
		else
			sepChar = 'u';

		int suffixNum;
		int trigger;

		switch (global[kTeleporterCommand]) {
		case 1:
			suffixNum = 3;
			trigger = 75;
			global[kTeleporterUnderstood] = true;
			break;

		case 2:
			suffixNum = 1;
			trigger = 80;
			break;

		case 4:
			suffixNum = 2;
			trigger = 90;
			break;

		default:
			trigger = 0;
			suffixNum = 0;
		}

		global[kTeleporterCommand] = 0;

		if (suffixNum > 0)
			kernel_run_animation(kernel_name(sepChar, suffixNum), trigger);
		else {
			player.walker_visible = true;
			player.commands_allowed = true;
		}
	}

	section_5_music();
}

static void room_551_daemon() {
	switch (kernel.trigger) {
	case 75:
		player.commands_allowed = true;
		player.walker_visible = true;
		player.clock = kernel.clock - player.frame_delay;
		break;

	case 80:
		global[kTeleporterCommand] = 1;
		new_room = global[kTeleporterDestination];
		kernel.force_restart = true;
		break;

	case 90:
		if (global[kSexOfRex] == REX_MALE) {
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
			kernel_seq_depth(g_sequence_ids[2], 8);
		} else {
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
			kernel_seq_depth(g_sequence_ids[3], 8);
		}
		g_engine->_soundManager->command(28, 0);
		kernel_timing_trigger(60, 91);
		break;

	case 91:
		kernel.force_restart = true;
		break;

	default:
		break;
	}
}

static void room_551_pre_parser() {
	if (player_said_1(walk_down) && (player_said_1(street_to_west) || player_said_1(sidewalk_to_west)))
		player.walk_off_edge_to_room = 501;
}

static void room_551_parser() {
	if (player_said_2(step_into, teleporter))
		new_room = 502;
	else if ((player.look_around))
		text_show(55117);
	else if (player_said_2(look, skeleton))
		text_show(55110);
	else if (player_said_2(look, elevator_shaft))
		text_show(55111);
	else if (player_said_2(walkto, elevator_shaft))
		text_show(55112);
	else if (player_said_2(look, building))
		text_show(55113);
	else if (player_said_2(look, teleporter))
		text_show(55114);
	else if (player_said_2(look, sidewalk_to_west)) {
		if (player_has_been_in_room(505))
			text_show(55116);
		else
			text_show(55115);
	} else if (player_said_2(look, sidewalk))
		text_show(55118);
	else if (player_said_2(look, equipment_overhead))
		text_show(55119);
	else if (player_said_2(look, railing))
		text_show(55120);
	else
		return;

	player.command_ready = false;
}

void room_551_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_551_preload() {
	room_init_code_pointer = room_551_init;
	room_daemon_code_pointer = room_551_daemon;
	room_pre_parser_code_pointer = room_551_pre_parser;
	room_parser_code_pointer = room_551_parser;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
