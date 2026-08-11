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

static void room_802_init() {
	g_sprite_ids[2] = kernel_load_series("*RXMRC_8", 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('f', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('f', 0), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('f', 1), 0);
	g_sprite_ids[5] = kernel_load_series("*RXMBD_8", 0);
	global[kBetweenRooms] = false;

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
		global[kAntigravClock] = kernel.clock;
	} else if (previous_room == 801) {
		player.x = 15;
		player.y = 129;
		player.facing = FACING_EAST;
	} else if (previous_room == 803) {
		player.x = 303;
		player.y = 119;
		player.facing = FACING_WEST;

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 15;
		player.y = 129;
		player.facing = FACING_EAST;
	}

	player.walker_visible = true;



	if (global[kHasWatchedAntigrav] && !global[kRemoteSequenceRan]) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(200, 70);
	}

	if ((global[kRemoteOnGround]) && (!player_has(OBJ_REMOTE))) {
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
		kernel_seq_depth(g_sequence_ids[4], 8);
		int idx = kernel_add_dynamic(words_remote, words_walkto, 0, g_sequence_ids[4], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 107, 99, FACING_NORTH);
	}

	if (!player_has(OBJ_SHIELD_MODULATOR) && !global[kShieldModInstalled]) {
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		kernel_seq_depth(g_sequence_ids[1], 8);
		int idx = kernel_add_dynamic(words_shield_modulator, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 93, 97, FACING_NORTH);
	}
	section_8_music();
}

static void room_802_daemon() {
	if (kernel.trigger == 70) {
		player.commands_allowed = false;
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 8, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 1, 19);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_SPRITE, 4, 72);
	}

	if (kernel.trigger == 71) {
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
		kernel_seq_depth(g_sequence_ids[4], 8);
		int idx = kernel_add_dynamic(words_remote, words_walkto, 0, g_sequence_ids[4], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 107, 99, FACING_NORTH);

		global[kRemoteSequenceRan] = true;
		global[kRemoteOnGround] = true;
		player.commands_allowed = true;
	}

	if (kernel.trigger == 72)
		g_engine->_soundManager->command(13, 0);
}

static void room_802_pre_parser() {
	if (player_said_2(walk_towards, building_to_west))
		player.walk_off_edge_to_room = 801;

	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 803;
		global[kForceBeamDown] = false;
	}

	if (player_said_2(take, ship))
		player.need_to_walk = false;
}

static void room_802_parser() {
	if (player_said_2(take, shield_modulator) && !player_has(OBJ_SHIELD_MODULATOR)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], true, 7, 0, 0, 2);
			kernel_seq_range(g_sequence_ids[2], 1, 2);
			kernel_seq_player(g_sequence_ids[2], false);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(g_sequence_ids[1]);
			g_engine->_soundManager->command(9, 0);
			break;

		case 2:
			player.clock = kernel.clock + player.frame_delay;
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			player.commands_allowed = true;
			inter_give_to_player(OBJ_SHIELD_MODULATOR);
			object_examine(OBJ_SHIELD_MODULATOR, 80215, 0);
			break;

		default:
			break;
		}
	} else if ((player_said_2(take, remote)) && (!player_has(OBJ_REMOTE))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[5] = kernel_seq_pingpong(g_sprite_ids[5], true, 7, 0, 0, 2);
			kernel_seq_range(g_sequence_ids[5], 1, 4);
			kernel_seq_player(g_sequence_ids[5], false);
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(g_sequence_ids[4]);
			g_engine->_soundManager->command(9, 0);
			global[kRemoteOnGround] = false;
			break;

		case 2:
			player.clock = kernel.clock + player.frame_delay;
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			player.commands_allowed = true;
			inter_give_to_player(OBJ_REMOTE);
			object_examine(OBJ_REMOTE, 80223, 0);
			break;

		default:
			break;
		}
	} else if (!global[kRemoteOnGround] && (player_has(OBJ_SHIELD_MODULATOR) || global[kShieldModInstalled])
		&& (player_said_2(look, launch_pad) || player.look_around))
		text_show(80210);
	else if (!global[kRemoteOnGround] && !player_has(OBJ_SHIELD_MODULATOR) && !global[kShieldModInstalled]
		&& (player_said_2(look, launch_pad) || player.look_around))
		text_show(80211);
	else if (global[kRemoteOnGround] && !player_has(OBJ_SHIELD_MODULATOR) && !global[kShieldModInstalled]
		&& (player_said_2(look, launch_pad) || player.look_around))
		text_show(80213);
	else if (global[kRemoteOnGround] && (player_has(OBJ_SHIELD_MODULATOR) || global[kShieldModInstalled])
		&& (player_said_2(look, launch_pad) || player.look_around))
		text_show(80212);
	else if (!player_has(OBJ_SHIELD_MODULATOR) && !global[kShieldModInstalled] && player_said_2(look, shield_modulator))
		text_show(80214);
	else if (global[kRemoteOnGround] && player_said_2(look, remote))
		text_show(80216);
	else if (player_said_2(look, ship)) {
		if ((!player_has(OBJ_SHIELD_MODULATOR)) && (!global[kShieldModInstalled]))
			text_show(80218);
		else
			text_show(80217);
	} else if (player_said_2(look, bushes))
		text_show(80219);
	else if (player_said_2(look, path_to_east))
		text_show(80220);
	else if (player_said_2(look, sky))
		text_show(80221);
	else if (player_said_2(take, ship))
		text_show(80222);
	else if (player_said_2(look, tree) || player_said_2(look, trees))
		text_show(80224);
	else if (player_said_2(look, building_to_west))
		text_show(80225);
	else
		return;

	player.command_ready = false;
}

void room_802_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_802_preload() {
	room_init_code_pointer = room_802_init;
	room_daemon_code_pointer = room_802_daemon;
	room_pre_parser_code_pointer = room_802_pre_parser;
	room_parser_code_pointer = room_802_parser;

	section_8_walker();
	section_8_interface();
	vocab_make_active(words_shield_modulator);
	vocab_make_active(words_walkto);
	vocab_make_active(words_remote);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
