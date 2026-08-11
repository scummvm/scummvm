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

static void room_601_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[3] = kernel_load_series("*RXCD_4", 0);

	if (global[kLaserHoleIsThere]) {
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
		kernel_add_dynamic(words_laser_beam, words_look_at, 0, g_sequence_ids[1], 0, 0, 0, 0);
	}

	g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
	kernel_seq_depth(g_sequence_ids[2], 3);

	if (previous_room == 504) {
		player.x = 73;
		player.y = 148;
		player.facing = FACING_WEST;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
		kernel_seq_depth(g_sequence_ids[2], 3);
		kernel_run_animation(kernel_name('R', 1), 70);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 229;
		player.y = 129;
		player.facing = FACING_SOUTHWEST;
	}

	section_6_music();
}

static void room_601_daemon() {
	switch (kernel.trigger) {
	case 70:
		player.walker_visible = true;
		player.clock = kernel_anim[0].next_clock - player.frame_delay;
		kernel_timing_trigger(30, 71);
		break;

	case 71:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 3);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 3);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_601_parser() {
	if (player_said_2(walk_through, entrance))
		new_room = 602;
	else if (player_said_2(get_inside, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 3);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
			kernel_seq_timeout(syncIdx, g_sequence_ids[2]);
			kernel_timing_trigger(6, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 10, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
			new_room = 504;
		}
		break;

		default:
			break;
		}
	} else if (player.look_around || player_said_2(look, street)) {
		if (!global[kLaserHoleIsThere])
			text_show(60110);
		else
			text_show(60111);
	} else if (player_said_2(look, car))
		text_show(60112);
	else if (player_said_2(look, papers))
		text_show(60113);
	else if (player_said_2(look, building))
		text_show(60114);
	else if (player_said_2(walk_down, street))
		text_show(60115);
	else if (player_said_2(look, balcony))
		text_show(60116);
	else if (player_said_2(look, entrance))
		text_show(60117);
	else if (player_said_2(look, wall))
		text_show(60118);
	else if (player_said_2(look, city))
		text_show(60119);
	else if (player_said_2(look, fountain))
		text_show(60120);
	else
		return;

	player.command_ready = false;
}

void room_601_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_601_preload() {
	room_init_code_pointer = room_601_init;
	room_daemon_code_pointer = room_601_daemon;
	room_parser_code_pointer = room_601_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_laser_beam);
	vocab_make_active(words_look_at);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
