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
#include "mads/nebular/rooms/section7.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _rexHandingLine;
};

static Scratch local;


static void room_751_init() {
	g_sprite_ids[1] = kernel_load_series("*RM701X0", 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('a', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('f', 0), 0);
	g_sprite_ids[4] = kernel_load_series("*RM202A1", 0);

	if (!player.been_here_before)
		local._rexHandingLine = false;

	if (global[kLineStatus] == 2 || global[kLineStatus] == 3) {
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -1);
		kernel_seq_depth(g_sequence_ids[3], 7);
		int idx = kernel_add_dynamic(words_fishing_line, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 268, 140, FACING_NORTHWEST);
	}

	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
	kernel_seq_loc(g_sequence_ids[1], 48, 136);
	kernel_seq_depth(g_sequence_ids[1], 10);

	if (previous_room == 752) {
		player.x = 309;
		player.y = 138;
		player.facing = FACING_WEST;
	} else if (previous_room == 710) {
		player.x = 154;
		player.y = 129;
		player.facing = FACING_NORTH;
		player.walker_visible = false;
		player.commands_allowed = false;
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
		kernel_seq_loc(g_sequence_ids[4], 155, 129);
		kernel_timing_trigger(15, 70);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 22;
		player.y = 131;
		player.facing = FACING_EAST;
		player.commands_allowed = false;
		kernel_timing_trigger(60, 60);
	} else if (local._rexHandingLine) {
		player.walker_visible = false;
		player.x = 268;
		player.y = 140;
		player.facing = FACING_NORTHWEST;
		player.walker_visible = false;
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 7);
		kernel_seq_player(g_sequence_ids[2], false);
	} else if (global[kLineStatus] == 2) {
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -1);
		kernel_seq_depth(g_sequence_ids[3], 7);
		int idx = kernel_add_dynamic(words_fishing_line, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 268, 140, FACING_NORTHWEST);
	}

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_FISHING_LINE);
		inter_give_to_player(OBJ_BINOCULARS);
	}

	section_7_music();
	kernel.quotes = quote_load(778, 779, 780, 781, 782, 0);

	if (global[kTimebombTimer] > 0)
		global[kTimebombTimer] = 10200;
}

static void room_751_daemon() {
	switch (kernel.trigger) {
	case 70:
		kernel_seq_delete(g_sequence_ids[4]);
		g_sequence_ids[4] = kernel_seq_backward(g_sprite_ids[4], false, 6, 0, 0, 1);
		kernel_seq_loc(g_sequence_ids[4], 155, 129);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
		kernel_seq_timeout(g_sequence_ids[4], -1);
		player.walker_visible = true;
		player.commands_allowed = true;
		break;

	default:
		break;
	}

	if ((global[kTimebombTimer] >= 10800) && (global[kTimebombStatus] == 1)) {
		global[kTimebombStatus] = 3;
		global[kTimebombTimer] = 0;
		global[kCheckDaemonTimebomb] = false;
		new_room = 620;
	}

	switch (kernel.trigger) {
	case 60:
		g_engine->_soundManager->command(16, 0);
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 5, 0, 0, 1);
		kernel_seq_loc(g_sequence_ids[1], 48, 136);
		kernel_seq_depth(g_sequence_ids[1], 10);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		player_walk(61, 131, FACING_EAST);
		kernel_timing_trigger(120, 62);
		break;

	case 62:
		g_engine->_soundManager->command(17, 0);
		g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 5, 0, 0, 1);
		kernel_seq_loc(g_sequence_ids[1], 48, 136);
		kernel_seq_depth(g_sequence_ids[1], 10);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 63);
		break;

	case 63:
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
		kernel_seq_loc(g_sequence_ids[1], 48, 136);
		kernel_seq_depth(g_sequence_ids[1], 10);
		player.commands_allowed = true;
		kernel_message_purge();
		break;

	default:
		break;
	}
}

static void room_751_pre_parser() {
	if (player_said_2(look, tall_building))
		player_walk(154, 129, FACING_NORTHEAST);

	if (player_said_3(look, binoculars, tall_building))
		player_walk(154, 129, FACING_NORTH);

	if (player_said_2(walkto, east_end_of_platform))
		player.walk_off_edge_to_room = 752;

	if (!local._rexHandingLine)
		return;

	if (player_said_1(look) || player_said_1(fishing_line) || player_said_1(talkto))
		player.need_to_walk = false;

	if ((!player_said_3(put, fishing_line, hook) || !player_said_3(tie, fishing_line, hook) || !player_said_3(attach, fishing_line, hook))
		&& (player.need_to_walk)) {
		switch (kernel.trigger) {
		case 0:
			player.ready_to_walk = false;
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 11, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[2], -1, 7);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			kernel_seq_timeout(g_sequence_ids[2], -1);
			player.walker_visible = true;
			local._rexHandingLine = false;
			player.commands_allowed = true;
			player.ready_to_walk = true;
			break;

		default:
			break;
		}
	}
}

static void room_751_parser() {
	if (player_said_2(walk_along, platform))
		; // Nothing
	else if (player_said_3(look, binoculars, tall_building)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[4], 155, 129);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[4];
			g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
			kernel_seq_loc(g_sequence_ids[4], 155, 129);
			kernel_seq_timeout(syncIdx, g_sequence_ids[4]);
			kernel_timing_trigger(15, 2);
		}
		break;

		case 2:
			new_room = 710;
			break;

		default:
			break;
		}
	} else if (player_said_2(step_into, elevator)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[1]);
			g_engine->_soundManager->command(16, 0);
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 5, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[1], 48, 136);
			kernel_seq_depth(g_sequence_ids[1], 10);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x30D), 0, 0, 0x1110, 120, 0, 34);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			player_walk(22, 131, FACING_EAST);
			kernel_timing_trigger(120, 3);
			break;

		case 3:
			g_engine->_soundManager->command(17, 0);
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 5, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[1], 48, 136);
			kernel_seq_depth(g_sequence_ids[1], 10);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
			kernel_seq_loc(g_sequence_ids[1], 48, 136);
			kernel_seq_depth(g_sequence_ids[1], 10);
			kernel_timing_trigger(60, 5);
			break;

		case 5:
			player.commands_allowed = true;
			new_room = 513;
			break;

		default:
			break;
		}
	} else if (player_said_3(put, fishing_line, hook) || player_said_3(tie, fishing_line, hook) || player_said_3(attach, fishing_line, hook)) {
		if (global[kLineStatus] == 1) {
			switch (kernel.trigger) {
			case 0:
				player.walker_visible = false;
				player.commands_allowed = false;
				g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 8, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[2], -1, 6);
				kernel_seq_timeout(-1, g_sequence_ids[2]);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
			{
				int syncIdx = g_sequence_ids[2];
				g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 7);
				kernel_seq_timeout(syncIdx, g_sequence_ids[2]);
				kernel_timing_trigger(30, 2);
			}
			break;

			case 2:
				local._rexHandingLine = true;
				kernel_seq_delete(g_sequence_ids[2]);
				g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 8, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[2], 8, -2);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
			{
				kernel_seq_timeout(g_sequence_ids[2], -1);
				player.walker_visible = true;
				g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -1);
				kernel_seq_depth(g_sequence_ids[3], 7);
				int idx = kernel_add_dynamic(words_fishing_line, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
				kernel_dynamic_walk(idx, 268, 140, FACING_NORTHWEST);
				kernel_message_purge();
				inter_move_object(OBJ_FISHING_LINE, room_id);
				local._rexHandingLine = false;
				global[kLineStatus] = 2;
				player.commands_allowed = true;
				text_show(75120);
			}
			break;

			default:
				break;
			}
		}
	} else if (player.look_around || player_said_2(look, city))
		text_show(75110);
	else if (player_said_2(look, elevator))
		text_show(75112);
	else if (player_said_2(look, platform))
		text_show(75113);
	else if (player_said_2(look, cement_pylon))
		text_show(75114);
	else if ((player_said_2(look, hook) || player_said_2(look, fishing_line))
		&& (global[kLineStatus] == 2 || global[kLineStatus] == 3))
		text_show(75116);
	else if (player_said_2(look, hook))
		text_show(75115);
	else if (player_said_2(look, rock))
		text_show(75117);
	else if (player_said_2(take, rock))
		text_show(75118);
	else if (player_said_2(look, east_end_of_platform))
		text_show(75119);
	else if (player_said_2(take, fishing_line) && (global[kLineStatus] == 3 || global[kLineStatus] == 2))
		text_show(75121);
	else if (player_said_2(look, tall_building))
		text_show(75122);
	else if (player_said_3(tie, fishing_line, cement_pylon) || player_said_3(attach, fishing_line, cement_pylon))
		text_show(75123);
	else
		return;

	player.command_ready = false;
}

void room_751_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._rexHandingLine);
}

void room_751_preload() {
	room_init_code_pointer = room_751_init;
	room_daemon_code_pointer = room_751_daemon;
	room_pre_parser_code_pointer = room_751_pre_parser;
	room_parser_code_pointer = room_751_parser;

	section_7_walker();
	section_7_interface();
	vocab_make_active(words_fishing_line);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
