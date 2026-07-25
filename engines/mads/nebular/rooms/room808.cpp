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
	bool _goingTo803;
};

static Scratch local;


static void room_808_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	g_sprite_ids[4] = kernel_load_series("*REXHAND", 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('b', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('b', 1), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('b', 2), 0);

	if (global[kTopButtonPushed])
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
	else
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);

	local._goingTo803 = false;

	if (global[kCameFromCut] && global[kCutX] != 0) {
		global[kCutX] = 0;
		global[kCameFromCut] = false;
		global[kReturnFromCut] = false;
		global[kBeamIsUp] = false;
		global[kForceBeamDown] = false;
		global[kDontRepeat] = false;
	} else if ((previous_room == 803) && global[kReturnFromCut]) {
		global[kDontRepeat] = false;
		global[kBeamIsUp] = true;
		global[kAntigravClock] = kernel.clock;
		global[kAntigravTiming] = kernel.clock;
		global[kForceBeamDown] = false;
		global[kReturnFromCut] = false;
	}

	global[kBetweenRooms] = false;

	if (global[kBeamIsUp]) {
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_seq_depth(g_sequence_ids[3], 8);
	}

	section_8_music();
}

static void room_808_parser() {
	if (player_said_2(press, start_button_2)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 4, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[4], 248, 211);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 70);
			break;

		case 70:
			if (!global[kBeamIsUp] && !global[kTopButtonPushed]) {
				g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
				kernel_seq_depth(g_sequence_ids[3], 8);
				local._goingTo803 = true;
				g_engine->_soundManager->command(20, 0);
				g_engine->_soundManager->command(25, 0);
			}
			g_sequence_ids[4] = kernel_seq_backward(g_sprite_ids[4], false, 4, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[4], 248, 211);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 71);
			break;

		case 71:
			player.commands_allowed = true;
			if (local._goingTo803 && !global[kTopButtonPushed]) {
				local._goingTo803 = false;
				global[kReturnFromCut] = true;
				new_room = 803;
			}
			break;

		default:
			break;
		}
	} else if (player_said_2(press, timer_button_2)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 4, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[4], 248, 186);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 90);
			break;
		case 90:
			if (global[kTopButtonPushed]) {
				kernel_seq_delete(g_sequence_ids[1]);
				g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
				kernel_seq_depth(g_sequence_ids[2], 8);
				g_engine->_soundManager->command(20, 0);
			}
			global[kTopButtonPushed] = false;
			g_sequence_ids[4] = kernel_seq_backward(g_sprite_ids[4], false, 4, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[4], 248, 186);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 91);
			break;

		case 91:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(press, remote_button_2)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 4, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[4], 248, 163);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			if (!global[kTopButtonPushed]) {
				kernel_seq_delete(g_sequence_ids[2]);
				g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
				kernel_seq_depth(g_sequence_ids[1], 8);
				g_engine->_soundManager->command(20, 0);
			}
			global[kTopButtonPushed] = true;
			g_sequence_ids[4] = kernel_seq_backward(g_sprite_ids[4], false, 4, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[4], 248, 163);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(press, start_button_1)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 4, 0, 0, 2);
			kernel_seq_loc(g_sequence_ids[4], 168, 211);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 70);
			break;

		case 70:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(press, remote_button_1)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 4, 0, 0, 2);
			kernel_seq_loc(g_sequence_ids[4], 172, 163);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 80);
			break;

		case 80:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(press, timer_button_1)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 4, 0, 0, 2);
			kernel_seq_loc(g_sequence_ids[4], 172, 186);
			kernel_seq_depth(g_sequence_ids[4], 2);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 90);
			break;

		case 90:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(exit, panel)) {
		new_room = 801;
		global[kBetweenRooms] = true;
	} else
		return;

	player.command_ready = false;
}

void room_808_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._goingTo803);
}

void room_808_preload() {
	room_init_code_pointer = room_808_init;
	room_parser_code_pointer = room_808_parser;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
