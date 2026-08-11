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

#include "mads/core/config.h"
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
	int16 _fishingLineId;
};

static Scratch local;


static void room_701_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('b', 5), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('b', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('b', 1), 0);
	g_sprite_ids[5] = kernel_load_series("*RM202A1", 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('b', 8), 0);

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_BINOCULARS);
		inter_give_to_player(OBJ_TWINKIFRUIT);
		inter_give_to_player(OBJ_BOMB);
		inter_give_to_player(OBJ_CHICKEN);
		inter_give_to_player(OBJ_BONES);

		global[kCityFlooded] = true;
		global[kLineStatus] = LINE_TIED;
		global[kBoatRaised] = false;
	}

	if (global[kBoatStatus] == BOAT_UNFLOODED) {
		if (global[kBoatRaised])
			global[kBoatStatus] = BOAT_GONE;
		else if (global[kLineStatus] == LINE_TIED)
			global[kBoatStatus] = BOAT_TIED_FLOATING;
		else if (game.difficulty == DIFFICULTY_HARD)
			global[kBoatStatus] = BOAT_ADRIFT;
		else
			global[kBoatStatus] = BOAT_TIED;
	}

	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
	kernel_seq_loc(g_sequence_ids[1], 48, 136);
	kernel_seq_depth(g_sequence_ids[1], 10);

	int boatStatus = (previous_room == 703) ? (int16)BOAT_GONE : global[kBoatStatus];

	switch (boatStatus) {
	case BOAT_TIED_FLOATING:
		g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 20, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[4], 10);
		break;
	case BOAT_ADRIFT:
		g_sequence_ids[6] = kernel_seq_pingpong(g_sprite_ids[6], false, 20, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[6], 10);
		break;
	case BOAT_TIED:
	{
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 9);
		int idx = kernel_add_dynamic(words_boat, words_climb_into, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 231, 127, FACING_NORTH);
		break;
	}
	case BOAT_GONE:
		kernel_flip_hotspot(words_boat, false);
		break;
	default:
		break;
	}

	if (global[kLineStatus] == LINE_DROPPED || global[kLineStatus] == LINE_TIED) {
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -1);
		kernel_seq_depth(g_sequence_ids[3], 8);
		int idx = kernel_add_dynamic(words_fishing_line, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 234, 129, FACING_NORTHEAST);
		local._fishingLineId = idx;
	}

	if (previous_room == 702) {
		player.x = 309;
		player.y = 138;
		player.facing = FACING_WEST;
	} else if (previous_room == 710) {
		player.x = 154;
		player.y = 129;
		player.facing = FACING_NORTH;
		player.walker_visible = false;
		player.commands_allowed = false;
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
		kernel_seq_loc(g_sequence_ids[5], 155, 129);
		kernel_timing_trigger(15, 60);
	} else if (previous_room == 703) {
		player.x = 231;
		player.y = 127;
		player.facing = FACING_SOUTH;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_run_animation(kernel_name('B', 1), 80);
		g_engine->_soundManager->command(28, 0);
	} else if (previous_room != KERNEL_RESTORING_GAME && previous_room != 620) {
		player.x = 22;
		player.y = 131;
		player.facing = FACING_EAST;
		player.commands_allowed = false;
		kernel_timing_trigger(60, 70);
	}

	kernel.quotes = quote_load(784, 783, 0);
	section_7_music();
}

static void room_701_daemon() {
	switch (kernel.trigger) {
	case 60:
		kernel_seq_delete(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_backward(g_sprite_ids[5], false, 6, 0, 0, 1);
		kernel_seq_loc(g_sequence_ids[5], 155, 129);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		kernel_seq_timeout(g_sequence_ids[5], -1);
		player.walker_visible = true;
		player.commands_allowed = true;
		break;

	case 70:
		g_engine->_soundManager->command(16, 0);
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 5, 0, 0, 1);
		kernel_seq_loc(g_sequence_ids[1], 48, 136);
		kernel_seq_depth(g_sequence_ids[1], 10);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
		player_walk(61, 131, FACING_EAST);
		kernel_timing_trigger(120, 72);
		break;

	case 72:
		g_engine->_soundManager->command(17, 0);
		g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 5, 0, 0, 1);
		kernel_seq_loc(g_sequence_ids[1], 48, 136);
		kernel_seq_depth(g_sequence_ids[1], 10);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 73);
		break;

	case 73:
		player.commands_allowed = true;
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
		kernel_seq_loc(g_sequence_ids[1], 48, 136);
		kernel_seq_depth(g_sequence_ids[1], 10);
		kernel_message_purge();
		break;

	case 80:
	{
		player.walker_visible = true;
		player.clock = kernel.clock - player.frame_delay;
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 9);
		int idx = kernel_add_dynamic(words_boat, words_climb_into, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 234, 129, FACING_NORTH);
		global[kBoatStatus] = BOAT_TIED;
		player.commands_allowed = true;
	}
	break;

	default:
		break;
	}
}

static void room_701_pre_parser() {
	if (player_said_2(walkto, east_end_of_platform))
		player.walk_off_edge_to_room = 702;

	if (player_said_2(look, building))
		player_walk(154, 129, FACING_NORTHEAST);

	if (player_said_3(look, binoculars, building))
		player_walk(154, 129, FACING_NORTH);
}

static void room_701_parser() {
	if (player_said_2(walk_along, platform)) {
	} else if (player_said_3(look, binoculars, building) && object[OBJ_VASE].location == 706) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 6, 0, 0, 1);
			kernel_seq_loc(g_sequence_ids[5], 155, 129);
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int temp = g_sequence_ids[5];
			g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, -2);
			kernel_seq_loc(g_sequence_ids[5], 155, 129);
			kernel_seq_timeout(temp, g_sequence_ids[5]);
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
			kernel_message_add(quote_string(kernel.quotes, 0x30D), 0, 0, 0x310, 120, 0, 34);
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
			global[kResurrectRoom] = 701;
			new_room = 605;
			break;

		default:
			break;
		}
	} else if ((player_said_2(pull, boat) || player_said_2(take, boat) ||
		player_said_2(pull, fishing_line) || player_said_2(take, fishing_line)) &&
		!player_has(OBJ_FISHING_LINE)) {
		if (global[kBoatStatus] == BOAT_TIED_FLOATING) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				kernel_seq_delete(g_sequence_ids[4]);
				kernel_seq_delete(g_sequence_ids[3]);
				kernel_delete_dynamic(local._fishingLineId);
				kernel_flip_hotspot(words_boat, false);
				player.walker_visible = false;
				kernel_run_animation(kernel_name('E', -1), 1);
				break;

			case 1:
			{
				player.walker_visible = true;
				player.clock = kernel_anim[0].next_clock - player.frame_delay;
				g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
				kernel_seq_depth(g_sequence_ids[2], 9);
				int idx = kernel_add_dynamic(words_boat, words_climb_into, 0, g_sequence_ids[2], 0, 0, 0, 0);
				kernel_dynamic_walk(idx, 231, 127, FACING_NORTH);
				kernel_timing_trigger(15, 2);
			}
			break;

			case 2:
				global[kBoatStatus] = BOAT_TIED;
				global[kLineStatus] = LINE_NOW_UNTIED;
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		} else if (global[kBoatStatus] == BOAT_TIED) {
			text_show(70125);
		} else if (global[kLineStatus] == LINE_DROPPED) {
			global[kLineStatus] = LINE_NOW_UNTIED;
			inter_give_to_player(OBJ_FISHING_LINE);
			g_engine->_soundManager->command(15, 0);
			kernel_seq_delete(g_sequence_ids[3]);
			object_examine(OBJ_FISHING_LINE, 70126, 0);
		} else {
			text_show(70127);
		}
	} else if (player_said_2(climb_into, boat) && global[kBoatStatus] == BOAT_TIED) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[2]);
			player.walker_visible = false;
			kernel_run_animation(kernel_name('B', 0), 1);
			break;

		case 1:
			new_room = 703;
			break;

		default:
			break;
		}
	} else if (player.look_around) {
		if (global[kBoatStatus] != BOAT_GONE) {
			if (global[kBoatStatus] == BOAT_TIED)
				text_show(70128);
			else
				text_show(70110);
		} else
			text_show(70111);
	} else if (player_said_2(look, submerged_city))
		text_show(70112);
	else if (player_said_2(look, elevator))
		text_show(70113);
	else if (player_said_2(look, platform))
		text_show(70114);
	else if (player_said_2(look, cement_pylon))
		text_show(70115);
	else if (player_said_2(look, hook)) {
		if (global[kLineStatus] == LINE_NOT_DROPPED || global[kLineStatus] == LINE_NOW_UNTIED)
			text_show(70116);
		else
			text_show(70117);
	} else if (player_said_2(look, rock))
		text_show(70118);
	else if (player_said_2(take, rock))
		text_show(70119);
	else if (player_said_2(look, east_end_of_platform))
		text_show(70120);
	else if (player_said_2(look, building))
		text_show(70121);
	else if (player_said_2(look, boat)) {
		if (global[kBoatStatus] == BOAT_ADRIFT || global[kBoatStatus] == BOAT_TIED_FLOATING)
			text_show(70122);
		else
			text_show(70123);
	} else if (player_said_3(cast, fishing_rod, boat) && player_has(OBJ_FISHING_LINE))
		text_show(70124);
	else
		return;

	player.command_ready = false;
}

void room_701_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._fishingLineId);
}

void room_701_preload() {
	room_init_code_pointer = room_701_init;
	room_daemon_code_pointer = room_701_daemon;
	room_pre_parser_code_pointer = room_701_pre_parser;
	room_parser_code_pointer = room_701_parser;

	section_7_walker();
	section_7_interface();
	vocab_make_active(words_boat);
	vocab_make_active(words_climb_into);
	vocab_make_active(words_fishing_line);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
