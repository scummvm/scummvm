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
#include "mads/core/quote.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _kargShootingFl;
	bool _loseFl;
};

static Scratch local;

static void room_104_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('h', -1), 0);
	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 14, 1, 0, 0);
	kernel_seq_depth(g_sequence_ids[1], 8);

	if (previous_room == 105) {
		player.x = 302;
		player.y = 107;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 160;
		player.y = 134;
	}

	local._loseFl = false;
	kernel.quotes = quote_load(53, 52, 0);
	local._kargShootingFl = false;

	if (g_engine->getRandomNumber(1, 3) == 1) {
		kernel_run_animation(kernel_full_name(104, 'B', -1, "", KERNEL_AA), 0);
		local._kargShootingFl = true;
	}

	section_1_music();
}

static void room_104_daemon() {
	if ((Common::Point(player.x, player.y) == Common::Point(189, 70)) && (kernel.trigger || !local._loseFl)) {
		if (player.facing == FACING_SOUTHWEST || player.facing == FACING_SOUTHEAST)
			player.facing = FACING_SOUTH;

		if (player.facing == FACING_NORTHWEST || player.facing == FACING_NORTHEAST)
			player.facing = FACING_NORTH;

		bool mirrorFl = false;
		if (player.facing == FACING_WEST) {
			player.facing = FACING_EAST;
			mirrorFl = true;
		}

		local._loseFl = true;

		switch (player.facing) {
		case FACING_EAST:
			switch (kernel.trigger) {
			case 0:
				kernel_message_purge();
				kernel_abort_animation(0);
				kernel_dump_all();
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sprite_ids[2] = kernel_load_series(kernel_name('a', 0), 0);
				kernel_new_palette();
				g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], mirrorFl, 7, 0, 0, 1);
				kernel_seq_loc(g_sequence_ids[2], 198, 143);
				kernel_seq_depth(g_sequence_ids[2], 4);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], mirrorFl, 7, 0, 0, 0);
				kernel_seq_loc(g_sequence_ids[2], 198, 143);
				kernel_seq_range(g_sequence_ids[2], -2, -2);
				kernel_timing_trigger(90, 2);
				break;

			case 2:
				text_show(10406);
				kernel.force_restart = true;
				break;

			default:
				break;
			}
			break;

		case FACING_SOUTH:
			switch (kernel.trigger) {
			case 0:
				kernel_message_purge();
				kernel_abort_animation(0);
				kernel_dump_all();
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sprite_ids[3] = kernel_load_series(kernel_name('a', 1), 0);
				kernel_new_palette();
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 1);
				kernel_seq_loc(g_sequence_ids[3], 198, 143);
				kernel_seq_depth(g_sequence_ids[3], 4);
				kernel_seq_range(g_sequence_ids[3], 1, 14);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 5, 0, 0, 1);
				kernel_seq_loc(g_sequence_ids[3], 198, 143);
				kernel_seq_depth(g_sequence_ids[3], 4);
				kernel_seq_range(g_sequence_ids[3], 15, 32);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 3, 0, 0, 0);
				kernel_seq_loc(g_sequence_ids[3], 198, 143);
				kernel_seq_range(g_sequence_ids[3], -2, -2);
				kernel_timing_trigger(90, 3);
				break;

			case 3:
				text_show(10406);
				kernel.force_restart = true;
				break;

			default:
				break;
			}
			break;

		case FACING_NORTH:
			switch (kernel.trigger) {
			case 0:
				kernel_message_purge();
				kernel_abort_animation(0);
				kernel_dump_all();
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sprite_ids[4] = kernel_load_series(kernel_name('a', 2), 0);
				kernel_new_palette();
				g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 8, 0, 0, 1);
				kernel_seq_loc(g_sequence_ids[4], 198, 143);
				kernel_seq_depth(g_sequence_ids[4], 4);
				kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 1);
				if (config_file.naughtiness >= NICE)
					kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_SPRITE, 15, 2);
				break;

			case 1:
				g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 8, 0, 0, 0);
				kernel_seq_loc(g_sequence_ids[4], 198, 143);
				kernel_seq_range(g_sequence_ids[4], -2, -2);
				kernel_timing_trigger(90, 2);
				break;

			case 2:
				text_show(10406);
				kernel.force_restart = true;
				break;

			default:
				break;
			}
			break;
		default:
			break;
		}

		if (!kernel.trigger)
			g_engine->_soundManager->command(34, 0);
	}

	if (player.walking && (player.next_special_code > 0)) {
		player_cancel_command();
		player_start_walking(189, 70, FACING_NONE);
		player.next_special_code = 0;
	}

	if ((player.special_code > 0) && player.commands_allowed)
		player.commands_allowed = false;

	if (local._kargShootingFl && (kernel_anim[0].frame >= 19)) {
		kernel_message_add(quote_string(kernel.quotes, 52), 0, 0, 0x1110, 120, 0, 34);
		local._kargShootingFl = false;
	}
}

static void room_104_pre_parser() {
	if (player_said_2(swim_towards, eastern_cliff_face))
		player.walk_off_edge_to_room = 105;

	if (player_said_2(swim_towards, open_area_to_south))
		player.walk_off_edge_to_room = 106;
}

static void room_104_parser() {
	if (player.look_around)
		text_show(10405);
	else if (player_said_2(look, curious_weed_patch))
		text_show(10404);
	else if (player_said_2(look, surface))
		text_show(10403);
	else if (player_said_2(look, cliff_face))
		text_show(10401);
	else if (player_said_2(look, ocean_floor))
		text_show(10402);
	else
		return;

	player.command_ready = false;
}

void room_104_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._kargShootingFl);
	s.syncAsByte(local._loseFl);
}

void room_104_preload() {
	room_init_code_pointer = room_104_init;
	room_pre_parser_code_pointer = room_104_pre_parser;
	room_parser_code_pointer = room_104_parser;
	room_daemon_code_pointer = room_104_daemon;

	anim_himem_preload(kernel_name('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
