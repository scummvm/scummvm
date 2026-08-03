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
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_215_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('e', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('a', 0), 0);

	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 7, 0, 0, 0);
	kernel_seq_loc(g_sequence_ids[1], 235, 83);
	kernel_seq_depth(g_sequence_ids[1], 15);

	if (global[kSexOfRex] == REX_MALE)
		g_sprite_ids[2] = kernel_load_series("*RXMRC_9", 0);
	else
		g_sprite_ids[2] = kernel_load_series("*ROXRC_9", 0);

	if (previous_room == 216) {
		player.x = 140;
		player.y = 119;
		player.facing = FACING_SOUTHWEST;
		player.walker_visible = false;
		player.commands_allowed = false;
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_timing_trigger(120, 70);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 204;
		player.y = 152;
		player.facing = FACING_NORTH;
	}

	kernel.quotes = quote_load(169, 170, 0);
	section_2_music();
}

static void room_215_daemon() {
	if (kernel.trigger == 70) {
		kernel_seq_delete(g_sequence_ids[3]);
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 1);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
	}

	if (kernel.trigger == 71) {
		kernel_seq_timeout(g_sequence_ids[3], -1);
		player.walker_visible = true;
		player.commands_allowed = true;
	}
}

static void room_215_parser() {
	if (player.look_around)
		text_show(21509);
	else if (player_said_2(take, twinkifruit)) {
		if (!player_has(OBJ_TWINKIFRUIT) || kernel.trigger) {
			switch (kernel.trigger) {
			case 0:
				if (global[kSexOfRex] == REX_MALE) {
					player.walker_visible = false;
					player.commands_allowed = false;
					g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 6, 0, 0, 2);
					kernel_seq_range(g_sequence_ids[2], 1, 4);
					kernel_seq_player(g_sequence_ids[2], false);
					kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_LOOP, 0, 1);
					kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
				} else {
					inter_give_to_player(OBJ_TWINKIFRUIT);
					object_examine(OBJ_TWINKIFRUIT, 0x5404, 0);
				}
				break;

			case 1:
				if (!player_has(OBJ_TWINKIFRUIT)) {
					inter_give_to_player(OBJ_TWINKIFRUIT);
					object_examine(OBJ_TWINKIFRUIT, 0x5404, 0);
				}
				break;

			case 2:
				player.walker_visible = true;
				player.commands_allowed = true;
				kernel_seq_timeout(g_sequence_ids[2], -1);
				break;

			default:
				break;
			}
		} else {
			int idx = g_engine->getRandomNumber(169, 170);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, idx), 0, 0, 0x1110, 120, 0, 34);
		}
	} else if (player_said_2(walk_outside, hut))
		new_room = 210;
	else if (player_said_2(look, bear_rug))
		text_show(21501);
	else if (player_said_2(look, bed))
		text_show(21502);
	else if (player_said_2(look, welcome_mat))
		text_show(21503);
	else if (player_said_2(look, love_altar))
		text_show(21504);
	else if (player_said_2(look, window))
		text_show(21505);
	else if (player_said_2(look, picture))
		text_show(21506);
	else if (player_said_2(look, twinkifruit) && (player.main_object_source == 4))
		text_show(21507);
	else if (player_said_2(take, bear_rug))
		text_show(21510);
	else if (player_said_2(take, love_altar))
		text_show(21511);
	else if (player_said_2(look, bag_of_twinkifruits))
		text_show(21512);
	else if (player_said_2(take, bag_of_twinkifruits))
		text_show(21513);
	else if (player_said_2(take, welcome_mat))
		text_show(21514);
	else
		return;

	player.command_ready = false;
}

void room_215_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_215_preload() {
	room_init_code_pointer = room_215_init;
	room_parser_code_pointer = room_215_parser;
	room_daemon_code_pointer = room_215_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
