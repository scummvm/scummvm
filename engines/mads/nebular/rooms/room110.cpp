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
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _crabsFl;
};

static Scratch local;


static void room_110_init() {
	g_sprite_ids[0] = kernel_load_series(kernel_name('X', 0), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('X', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('X', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('X', 3), 0);

	local._crabsFl = false;

	if (previous_room == 109) {
		player.x = 59;
		player.y = 71;
		player.facing = FACING_EAST;

		g_sequence_ids[0] = kernel_seq_stamp(g_sprite_ids[0], false, 1);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);

		local._crabsFl = true;

		int idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[0], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
		idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
		idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
		idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 194;
		player.y = 23;
		player.facing = FACING_SOUTH;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_run_animation(kernel_full_name(110, 'T', 1, "", KERNEL_AA), 70);
	}

	section_1_music();
	kernel.quotes = quote_load(89, 0);

	if (!player.been_here_before && (previous_room == 109))
		kernel_message_add(quote_string(kernel.quotes, 89), 0, 0, 0x1110, 120, 0, 34);
}

static void room_110_daemon() {
	if (kernel.trigger == 70) {
		player.walker_visible = true;
		player.commands_allowed = true;
	}
}

static void room_110_pre_parser() {
	if (player_said_2(swim_through, cave_entrance))
		player.walk_off_edge_to_room = 109;

	if (local._crabsFl) {
		local._crabsFl = false;

		kernel_seq_delete(g_sequence_ids[0]);
		kernel_seq_delete(g_sequence_ids[1]);
		kernel_seq_delete(g_sequence_ids[2]);
		kernel_seq_delete(g_sequence_ids[3]);

		g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 16, 0, 0, 1);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 16, 0, 0, 1);
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 16, 0, 0, 1);
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 16, 0, 0, 1);

		int idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[0], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
		idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
		idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
		idx = kernel_add_dynamic(words_crab, words_swim_to, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -1, 0, FACING_NONE);
	}
}

static void room_110_parser() {
	if (player_said_2(swim_through, tunnel)) {
		switch (kernel.trigger) {
		case 0:
			kernel_run_animation(kernel_full_name(110, 'T', 0, "", KERNEL_AA), 1);
			kernel_anim[0].next_clock = player.frame_delay + player.clock;
			player.commands_allowed = false;
			player.walker_visible = false;
			break;
		case 1:
			player.walker_visible = true;
			player.commands_allowed = true;
			new_room = 111;
			break;
		default:
			break;
		}
	} else if ((player.look_around) || player_said_2(look, cave))
		text_show(11001);
	else if (player_said_2(look, cave_ceiling) || player_said_2(look_at, cave_ceiling))
		text_show(11002);
	else if (player_said_2(look, rocks))
		text_show(11003);
	else if (player_said_2(take, rocks))
		text_show(11004);
	else if (player_said_2(look, tunnel))
		text_show(11005);
	else if (player_said_2(look, cave_entrance))
		text_show(11006);
	else if (player_said_2(look, fungoids))
		text_show(11007);
	else if (player_said_2(take, fungoids))
		text_show(11008);
	else
		return;

	player.command_ready = false;
}

void room_110_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._crabsFl);
}

void room_110_preload() {
	room_init_code_pointer = room_110_init;
	room_pre_parser_code_pointer = room_110_pre_parser;
	room_parser_code_pointer = room_110_parser;
	room_daemon_code_pointer = room_110_daemon;

	section_1_walker();
	section_1_interface();
	vocab_make_active(words_crab);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
