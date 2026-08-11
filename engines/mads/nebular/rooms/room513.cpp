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

static void room_513_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[3] = kernel_load_series("*RXCD_9", 0);
	g_sprite_ids[4] = kernel_load_series("*RXMRC_9", 0);

	g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
	kernel_seq_depth(g_sequence_ids[2], 2);

	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
	kernel_seq_depth(g_sequence_ids[1], 1);

	if ((previous_room == 751) || (previous_room == 701)) {
		player.x = 296;
		player.y = 147;
		player.facing = FACING_WEST;
		player.commands_allowed = false;
		kernel_timing_trigger(15, 80);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 63;
		player.y = 149;
		player.facing = FACING_NORTHEAST;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
		kernel_seq_depth(g_sequence_ids[1], 1);
		kernel_run_animation(kernel_name('R', 1), 70);
	}

	section_5_music();

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_SECURITY_CARD);

	kernel.quotes = quote_load(632, 0);
}

static void room_513_daemon() {
	switch (kernel.trigger) {
	case 80:
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 2);
		g_engine->_soundManager->command(24, 0);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 81);
		break;

	case 81:
		player_walk(265, 152, FACING_WEST);
		kernel_timing_trigger(120, 82);
		break;

	case 82:
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 7, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 2);
		g_engine->_soundManager->command(25, 0);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 83);
		break;

	case 83:
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
		kernel_seq_depth(g_sequence_ids[2], 2);
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
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[1], 1);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
		kernel_seq_depth(g_sequence_ids[1], 1);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_513_parser() {
	if (player_said_2(get_into, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[1], 1);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[1];
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
			kernel_seq_depth(g_sequence_ids[1], 1);
			kernel_seq_timeout(syncIdx, g_sequence_ids[1]);
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
	} else if (player_said_3(put, id_card, card_slot) || player_said_3(put, fake_id, card_slot)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 7, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[4], 1, 2);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			kernel_seq_timeout(g_sequence_ids[4], -1);
			player.walker_visible = true;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 7, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 2);
			g_engine->_soundManager->command(24, 0);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x278), 0, 0, 0x1110, 120, 0, 34);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			player_walk(296, 147, FACING_WEST);
			kernel_timing_trigger(120, 3);
			break;

		case 3:
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 7, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 2);
			g_engine->_soundManager->command(25, 0);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
			kernel_seq_depth(g_sequence_ids[2], 2);
			player.commands_allowed = true;
			if (global[kCityFlooded])
				new_room = 701;
			else
				new_room = 751;

			break;

		default:
			break;
		}
	} else if ((player.look_around) || player_said_2(look, street))
		text_show(51318);
	else if (player_said_2(look, elevator))
		text_show(51310);
	else if (player_said_2(look, elevator_door))
		text_show(51311);
	else if (player_said_2(look, card_slot))
		text_show(51312);
	else if (player_said_2(look, handicap_sign))
		text_show(51313);
	else if (player_said_2(look, bike_rack))
		text_show(51314);
	else if (player_said_2(look, building))
		text_show(51315);
	else if (player_said_2(look, sign))
		text_show(51316);
	else if (player_said_2(look, street_to_west) || player_said_2(walk_down, street_to_west))
		text_show(51317);
	else if (player_said_2(open, elevator_door) || player_said_2(open, elevator))
		text_show(51319);
	else if (player_said_2(look, car))
		text_show(51321);
	else if (player_said_2(look, brick_wall))
		text_show(51322);
	else if (player_said_3(put, security_card, card_slot))
		text_show(51320);
	else
		return;

	player.command_ready = false;
}

void room_513_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_513_preload() {
	room_init_code_pointer = room_513_init;
	room_daemon_code_pointer = room_513_daemon;
	room_parser_code_pointer = room_513_parser;

	section_5_walker();
	section_5_interface();
	vocab_make_active(words_elevator_door);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
