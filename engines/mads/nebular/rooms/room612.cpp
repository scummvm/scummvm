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
#include "mads/nebular/rooms/section6.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _actionMode;
	int16 _cycleIndex;
};

static Scratch local;


static void handleWinchMovement() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 1, 5);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_SPRITE, 5, 1);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		// CHECKME: Is the "else" block useful as action is always equal to 1 at this point?
		// Or is it a missing bit of code we could fix?
		if (local._actionMode == 1) {
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 17, 0, 0, 7);
			g_engine->_soundManager->command(19, 0);
			inter_move_object(OBJ_PADLOCK_KEY, 1);
			global[kBoatRaised] = false;
		} else {
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 17, 0, 0, 9);
			g_engine->_soundManager->command(18, 0);
		}
		kernel_seq_depth(g_sequence_ids[2], 1);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 3);
		break;

	case 2:
		kernel_seq_timeout(g_sequence_ids[4], -1);
		player.walker_visible = true;
		break;

	case 3:
	{
		int syncIdx = g_sequence_ids[2];
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, local._cycleIndex);
		kernel_seq_depth(g_sequence_ids[2], 1);
		kernel_seq_timeout(syncIdx, g_sequence_ids[2]);
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x2F4), 0, 0, 0x1110, 120, 0, 34);
		player.commands_allowed = true;

		text_show(61217);
	}
	break;

	default:
		break;
	}
}

static void room_612_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('c', -1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('p', -1), 0);
	g_sprite_ids[3] = kernel_load_series("*RXCD_3", 0);
	g_sprite_ids[4] = kernel_load_series("*RXRC_6", 0);

	if ((global[kLineStatus] == 2) || (global[kLineStatus] == 3)) {
		g_sprite_ids[5] = kernel_load_series(kernel_name('f', -1), 0);
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, -1);
		kernel_seq_depth(g_sequence_ids[5], 1);
		int idx = kernel_add_dynamic(words_fishing_line, words_walkto, 0, g_sequence_ids[5], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 34, 117, FACING_SOUTHEAST);
	}

	if (global[kBoatRaised])
		local._cycleIndex = -2;
	else
		local._cycleIndex = -1;

	g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, local._cycleIndex);
	kernel_seq_depth(g_sequence_ids[2], 1);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 280;
		player.y = 75;
		player.facing = FACING_SOUTHWEST;
		player.walker_visible = false;
		player.commands_allowed = false;
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
		kernel_seq_depth(g_sequence_ids[1], 3);
		kernel_run_animation(kernel_name('R', 1), 70);
	}

	section_6_music();

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_PADLOCK_KEY);

	kernel.quotes = quote_load(757, 756, 0);
}

static void room_612_daemon() {
	switch (kernel.trigger) {
	case 70:
		player.walker_visible = true;
		player.clock = kernel_anim[0].next_clock - player.frame_delay;
		kernel_timing_trigger(6, 71);
		break;

	case 71:
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[1], 2);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
		kernel_seq_depth(g_sequence_ids[1], 2);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_612_parser() {
	if (player_said_2(get_inside, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[1], 2);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[1];
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
			kernel_seq_depth(g_sequence_ids[1], 3);
			kernel_seq_timeout(syncIdx, g_sequence_ids[1]);
			kernel_timing_trigger(6, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], true, 10, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], true, -2);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
			new_room = 504;
		}
		break;

		default:
			break;
		}
	} else if (player_said_3(unlock, padlock_key, control_box)) {
		local._cycleIndex = -2;
		local._actionMode = 1;
		handleWinchMovement();
	} else if (player.look_around || player_said_2(look, expressway))
		text_show(61210);
	else if (player_said_2(look, rope) || player_said_2(look, armature)) {
		if (global[kBoatRaised])
			text_show(61211);
		else
			text_show(61212);
	} else if (player_said_2(take, rope))
		text_show(61213);
	else if (player_said_2(look, control_box)) {
		if (global[kBoatRaised])
			text_show(61214);
		else
			text_show(61216);
	} else if (player_said_2(open, control_box))
		text_show(61215);
	else if (player_said_2(look, buildings))
		text_show(61218);
	else if (player_said_2(look, dome))
		text_show(61219);
	else if (player_said_2(look, statue))
		text_show(61220);
	else if (player_said_2(look, maintenance_building))
		text_show(61221);
	else if (player_said_2(open, maintenance_building))
		text_show(61222);
	else if (player_said_2(look, wall))
		text_show(61223);
	else if (player_said_2(look, support))
		text_show(61224);
	else if (player_said_2(walk_down, expressway_to_east) || player_said_2(walk_down, expressway_to_west))
		text_show(61225);
	else
		return;

	player.command_ready = false;
}

void room_612_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._actionMode);
	s.syncAsSint16LE(local._cycleIndex);
}

void room_612_preload() {
	room_init_code_pointer = room_612_init;
	room_daemon_code_pointer = room_612_daemon;
	room_parser_code_pointer = room_612_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_fishing_line);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
