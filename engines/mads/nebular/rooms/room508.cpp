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

struct Scratch {
	int16 _chosenObject;
};

static Scratch local;


static void room_508_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('a', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('m', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('h', 0), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('l', 2), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('t', 0), 0);
	g_sprite_ids[6] = kernel_load_series("*RXMRC_9", 0);
	g_sprite_ids[7] = kernel_load_series(kernel_name('l', 3), 0);

	if (!player.been_here_before) {
		global[kLaserOn] = false;
		local._chosenObject = 0;
	}

	if (!global[kLaserOn]) {
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
		kernel_seq_depth(g_sequence_ids[3], 8);
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, -2);
		int idx = kernel_add_dynamic(words_spinach_patch_doll, words_walkto, 0, g_sequence_ids[5], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 57, 116, FACING_NORTHEAST);
		kernel_flip_hotspot(words_hole, false);
		kernel_flip_hotspot(words_laser_beam, false);
	} else {
		kernel_load_variant(1);
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
		kernel_seq_depth(g_sequence_ids[3], 8);
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
		kernel_seq_depth(g_sequence_ids[4], 11);
		int idx = kernel_add_dynamic(words_laser_beam, words_walkto, 0, g_sequence_ids[4], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 57, 116, FACING_NORTHEAST);
		g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 15, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[2], 6, 8);
		kernel_seq_depth(g_sequence_ids[2], 6);
		if (global[kLaserHoleIsThere]) {
			g_sequence_ids[7] = kernel_seq_stamp(g_sprite_ids[7], false, -2);
			kernel_flip_hotspot(words_hole, true);
			kernel_flip_hotspot(words_laser_beam, true);
		}
		g_engine->_soundManager->command(21, 0);
	}
	g_engine->_soundManager->command(20, 0);

	if (previous_room == 515) {
		player.x = 57;
		player.y = 116;
		player.facing = FACING_NORTHEAST;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 289;
		player.y = 139;
		player.facing = FACING_WEST;
	}

	section_5_music();
	kernel.quotes = quote_load(627, 0);

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_COMPACT_CASE);
		inter_give_to_player(OBJ_REARVIEW_MIRROR);
	}
}

static void room_508_pre_parser() {
	if (player_said_2(walk, outside))
		player.walk_off_edge_to_room = 506;
}

static void handlePedestral() {
	if (!global[kLaserOn])
		text_show(50835);

	if (global[kLaserHoleIsThere])
		text_show(50836);

	if (global[kLaserOn] && !global[kLaserHoleIsThere]) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[6] = kernel_seq_pingpong(g_sprite_ids[6], false, 9, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[6], 1, 4);
			kernel_seq_player(g_sequence_ids[6], false);
			kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			if (local._chosenObject == 2)
				inter_take_from_player(OBJ_COMPACT_CASE, 1);
			else
				inter_take_from_player(OBJ_REARVIEW_MIRROR, 1);

			g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 6, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			g_sequence_ids[7] = kernel_seq_stamp(g_sprite_ids[7], false, -2);
			kernel_flip_hotspot(words_hole, true);
			kernel_flip_hotspot(words_laser_beam, true);
			break;

		case 3:
			kernel_seq_timeout(g_sequence_ids[6], -1);
			player.walker_visible = true;
			kernel_timing_trigger(120, 4);
			break;

		case 4:
			text_show(50834);
			global[kLaserHoleIsThere] = true;
			new_room = 515;
			break;

		default:
			break;
		}
	}
}

static void room_508_parser() {
	if (player_said_2(pull, lever)) {
		if (!global[kLaserOn]) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x273), 0, 0, 0x1110, 120, 2, 34);
				break;

			case 2:
				player.walker_visible = false;
				kernel_seq_delete(g_sequence_ids[3]);
				g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 10, 0, 0, 1);
				kernel_seq_depth(g_sequence_ids[1], 7);
				kernel_seq_timeout(-1, g_sequence_ids[1]);
				kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;

			case 3:
				g_engine->_soundManager->command(19, 0);
				g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 15, 0, 0, 1);
				kernel_seq_depth(g_sequence_ids[2], 6);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 4);
				g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
				kernel_seq_depth(g_sequence_ids[3], 8);
				kernel_seq_timeout(g_sequence_ids[1], -1);
				player.walker_visible = true;
				kernel_timing_trigger(15, 5);
				break;

			case 4:
				g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 15, 0, 0, 0);
				kernel_seq_range(g_sequence_ids[2], 6, 8);
				kernel_seq_depth(g_sequence_ids[2], 6);
				break;

			case 5:
				kernel_seq_delete(g_sequence_ids[5]);
				kernel_run_animation(kernel_name('B', 1), 6);
				break;

			case 6:
			{
				g_engine->_soundManager->command(22, 0);
				g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
				kernel_seq_depth(g_sequence_ids[4], 11);
				int idx = kernel_add_dynamic(words_laser_beam, words_walkto, 0, g_sequence_ids[4], 0, 0, 0, 0);
				kernel_dynamic_walk(idx, 57, 116, FACING_NORTHEAST);
				kernel_message_purge();
				kernel_load_variant(1);
				kernel_timing_trigger(30, 7);
			}
			break;

			case 7:
				global[kLaserOn] = true;
				text_show(50833);
				player.commands_allowed = true;
				break;

			default:
				break;
			}
		} else {
			text_show(50837);
		}
	} else if (player_said_3(reflect, rearview_mirror, laser_beam) || player_said_3(put, rearview_mirror, pedestal) || player_said_3(put, rearview_mirror, laser_beam)) {
		local._chosenObject = 1;
		handlePedestral();
	} else if (player_said_3(put, compact_case, pedestal) || player_said_3(put, compact_case, laser_beam) || player_said_3(reflect, compact_case, laser_beam)) {
		local._chosenObject = 2;
		handlePedestral();
	} else if (player.look_around)
		text_show(50822);
	else if (player_said_2(look, target_area))
		text_show(50810);
	else if (player_said_2(look, spinach_patch_doll))
		text_show(50811);
	else if (player_said_2(take, spinach_patch_doll))
		text_show(50812);
	else if (player_said_2(look, sand_bags))
		text_show(50816);
	else if (player_said_2(take, sand_bags))
		text_show(50817);
	else if (player_said_2(look, control_station))
		text_show(50818);
	else if (player_said_2(look, monitor)) {
		if (global[kLaserOn])
			text_show(50820);
		else
			text_show(50819);
	} else if (player_said_2(look, laser_cannon)) {
		if (global[kLaserOn])
			text_show(50822);
		else
			text_show(50821);
	} else if (player_said_2(take, laser_cannon))
		text_show(50823);
	else if (player_said_2(look, lever)) {
		if (global[kLaserOn])
			text_show(50825);
		else
			text_show(50824);
	} else if (player_said_2(push, lever))
		text_show(50826);
	else if (player_said_2(look, laser_beam)) {
		if (global[kLaserHoleIsThere])
			text_show(50828);
		else
			text_show(50827);
	} else if (player_said_2(take, laser_beam))
		text_show(50829);
	else if (player_said_2(look, ceiling)) {
		if (global[kLaserHoleIsThere])
			text_show(50831);
		else
			text_show(50830);
	} else if (player_said_2(look, wall))
		text_show(50832);
	else if (player_said_2(look, pedestal)) {
		if (!global[kLaserOn])
			text_show(50813);
		else if (!global[kLaserHoleIsThere])
			text_show(50814);
		else
			text_show(50815);
	} else
		return;

	player.command_ready = false;
}

void room_508_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._chosenObject);
}

void room_508_preload() {
	room_init_code_pointer = room_508_init;
	room_pre_parser_code_pointer = room_508_pre_parser;
	room_parser_code_pointer = room_508_parser;

	section_5_walker();
	section_5_interface();
	vocab_make_active(words_spinach_patch_doll);
	vocab_make_active(words_walkto);
	vocab_make_active(words_laser_beam);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
