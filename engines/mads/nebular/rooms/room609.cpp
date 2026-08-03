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
#include "mads/nebular/rooms/section6.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _videoDoorMode;
};

static Scratch local;


static void room_609_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('h', 0), 0);
	g_sprite_ids[4] = kernel_load_series("*RXCD_9", 0);
	g_sprite_ids[5] = kernel_load_series("*RXMRC_9", 0);

	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
	kernel_seq_depth(g_sequence_ids[1], 5);
	g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
	kernel_seq_depth(g_sequence_ids[2], 9);

	if (!player.been_here_before)
		global[kBeenInVideoStore] = false;

	if (previous_room == 611) {
		player.x = 264;
		player.y = 69;
		player.facing = FACING_SOUTHWEST;
	} else if (previous_room == 610) {
		player.x = 23;
		player.y = 90;
		player.facing = FACING_EAST;
		kernel_timing_trigger(60, 60);
		player.commands_allowed = false;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 86;
		player.y = 136;
		player.facing = FACING_NORTHEAST;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
		kernel_seq_depth(g_sequence_ids[1], 5);
		kernel_run_animation(kernel_name('R', 1), 70);
	}

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_DOOR_KEY);
		if (game.difficulty != DIFFICULTY_EASY)
			inter_give_to_player(OBJ_PENLIGHT);
	}

	section_6_music();
	kernel.quotes = quote_load(773, 774, 775, 776, 777, 0);
}

static void room_609_daemon() {
	switch (kernel.trigger) {
	case 60:
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 9);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		kernel_flip_hotspot(words_video_store_door, false);
		player_walk(101, 100, FACING_EAST);
		kernel_timing_trigger(180, 62);
		break;

	case 62:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_flip_hotspot(words_video_store_door, true);
		kernel_seq_depth(g_sequence_ids[2], 9);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 63);
		break;

	case 63:
		if (!global[kHasTalkedToHermit] && (game.difficulty != DIFFICULTY_HARD)) {
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 26, 0, 0, 2);
			kernel_seq_depth(g_sequence_ids[3], 7);
			kernel_seq_loc(g_sequence_ids[3], 287, 73);
			kernel_seq_scale(g_sequence_ids[3], 47);
		}
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 9);
		player.commands_allowed = true;
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 70:
		player.walker_visible = true;
		player.clock = kernel_anim[0].next_clock - player.frame_delay;
		kernel_timing_trigger(6, 71);
		break;

	case 71:
		if (!global[kHasTalkedToHermit]) {
			g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 26, 0, 0, 2);
			kernel_seq_depth(g_sequence_ids[3], 7);
			kernel_seq_loc(g_sequence_ids[3], 287, 73);
			kernel_seq_scale(g_sequence_ids[3], 47);
		}
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[1], 5);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
		kernel_seq_depth(g_sequence_ids[1], 5);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void enterStore() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		if (local._videoDoorMode == 2)
			kernel_timing_trigger(1, 4);
		else {
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x305), 0, 0, 0x1110, 120, 0, 34);
			kernel_timing_trigger(120, 1);
		}
		break;

	case 1:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x306), 0, 0, 0x1110, 120, 0, 34);
		kernel_timing_trigger(60, 2);
		break;

	case 2:
		player.walker_visible = false;
		g_sequence_ids[5] = kernel_seq_pingpong(g_sprite_ids[5], true, 11, 0, 0, 2);
		kernel_seq_range(g_sequence_ids[5], 1, 2);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 3);
		break;

	case 3:
		kernel_seq_timeout(g_sequence_ids[5], -1);
		player.walker_visible = true;
		inter_move_object(OBJ_DOOR_KEY, 1);
		kernel_timing_trigger(15, 4);
		break;

	case 4:
		player.walker_visible = false;
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], true, 1);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_timing_trigger(15, 5);
		break;

	case 5:
		kernel_seq_delete(g_sequence_ids[5]);
		player.clock = kernel.clock - player.frame_delay;
		player.walker_visible = true;
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 9);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 6);
		break;

	case 6:
		kernel_flip_hotspot(words_video_store_door, false);
		if (local._videoDoorMode == 1) {
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x307), 0, 0, 0x1110, 180, 0, 34);
		}
		player_walk(23, 90, FACING_WEST);
		kernel_timing_trigger(180, 7);
		break;

	case 7:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 9);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 8);
		break;

	case 8:
		kernel_flip_hotspot(words_video_store_door, true);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 9);
		global[kBeenInVideoStore] = true;
		player.commands_allowed = true;
		new_room = 610;
		break;

	default:
		break;
	}
}

static void room_609_pre_parser() {
	if (player_said_3(unlock, door_key, video_store_door))
		player_walk(78, 99, FACING_NORTHWEST);
}

static void room_609_parser() {
	if (player_said_2(walk_towards, alley))
		new_room = 611;
	else if (player_said_2(walk_through, video_store_door)) {
		if (!global[kBeenInVideoStore]) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x308), 0, 0, 0x1110, 120, 0, 34);
				kernel_timing_trigger(120, 1);
				break;

			case 1:
				player.walker_visible = false;
				g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], true, 1);
				kernel_seq_player(g_sequence_ids[5], false);
				kernel_timing_trigger(30, 2);
				break;

			case 2:
				kernel_seq_delete(g_sequence_ids[5]);
				player.clock = kernel.clock - player.frame_delay;
				player.walker_visible = true;
				kernel_timing_trigger(60, 3);
				break;

			case 3:
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x309), 0, 0, 0x1110, 120, 0, 34);
				kernel_timing_trigger(120, 4);
				break;

			case 4:
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		} else {
			local._videoDoorMode = 2;
			enterStore();
		}
	} else if (player_said_3(unlock, door_key, video_store_door)) {
		local._videoDoorMode = 1;
		enterStore();
	} else if (player_said_2(get_inside, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[1], 5);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[1];
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
			kernel_seq_depth(g_sequence_ids[1], 5);
			kernel_seq_timeout(syncIdx, g_sequence_ids[1]);
			kernel_timing_trigger(6, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 10, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = g_sequence_ids[4];
			g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_timeout(syncIdx, g_sequence_ids[4]);
			new_room = 504;
		}
		break;

		default:
			break;
		}
	} else if (player.look_around)
		text_show(60910);
	else if (player_said_2(look, street))
		text_show(60911);
	else if (player_said_2(look, spot_a_pot))
		text_show(60912);
	else if (player_said_2(look, video_store))
		text_show(60913);
	else if (player_said_2(look, billboard))
		text_show(60914);
	else if (player_said_2(look, statue))
		text_show(60915);
	else if (player_said_2(look, car))
		text_show(60916);
	else if (player_said_2(look, newsstand))
		text_show(60917);
	else if (player_said_2(look, video_store_door)) {
		if (!global[kBeenInVideoStore])
			text_show(60918);
		else
			text_show(60919);
	} else if (player_said_2(walk_down, street))
		text_show(60730);
	else
		return;

	player.command_ready = false;
}

void room_609_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._videoDoorMode);
}

void room_609_preload() {
	room_init_code_pointer = room_609_init;
	room_daemon_code_pointer = room_609_daemon;
	room_pre_parser_code_pointer = room_609_pre_parser;
	room_parser_code_pointer = room_609_parser;

	section_6_walker();
	section_6_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
