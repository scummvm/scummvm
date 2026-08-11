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
	bool _stampedFl;
	bool _launch1Fl;
	bool _launched2Fl;
	bool _rexDivingFl;
};

static Scratch local;


static void room_111_init() {
	g_sprite_ids[0] = kernel_load_series(kernel_name('X', 0), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('X', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('X', 2), 0);

	g_sprite_ids[3] = kernel_load_series(kernel_name('B', 0), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('B', 1), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('B', 2), 0);

	g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 8, 0, 0, 0);
	kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_SPRITE, 9, 73);
	kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_SPRITE, 13, 73);

	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 5, 0, 0, 0);
	kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_SPRITE, 71, 71);

	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 0);
	g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
	g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);

	int idx = kernel_add_dynamic(words_bats, words_look_at, 0, g_sequence_ids[3], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, -2, 0, FACING_NONE);
	idx = kernel_add_dynamic(words_bats, words_look_at, 0, g_sequence_ids[4], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, -2, 0, FACING_NONE);
	idx = kernel_add_dynamic(words_bats, words_look_at, 0, g_sequence_ids[5], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, -2, 0, FACING_NONE);

	local._launch1Fl = false;
	local._launched2Fl = false;
	local._stampedFl = false;

	if ((previous_room < 201) && (previous_room != KERNEL_RESTORING_GAME)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel_run_animation(kernel_full_name(111, 'A', 0, "", KERNEL_AA), 70);
		player.x = 234;
		player.y = 116;
		player.facing = FACING_EAST;

		local._launch1Fl = true;
		local._launched2Fl = true;

		g_engine->_soundManager->command(36, 0);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 300;
		player.y = 130;
		player.facing = FACING_WEST;
	}

	local._rexDivingFl = false;

	section_1_music();
}

static void room_111_daemon() {
	if (kernel.trigger == 70) {
		player.commands_allowed = true;
		player.walker_visible = true;
		local._launch1Fl = false;
		local._launched2Fl = false;
	}

	if ((kernel.trigger == 71) && !local._stampedFl) {
		local._stampedFl = true;
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 18, 0, 0, 1);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 72);
	}

	if (kernel.trigger == 72) {
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 20);
	}

	if (!local._launch1Fl && (g_engine->getRandomNumber(1, 5000) == 1)) {
		kernel_seq_delete(g_sequence_ids[4]);
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 5, 0, 0, 1);
		local._launch1Fl = true;
		int idx = kernel_add_dynamic(words_bats, words_look_at, 0, g_sequence_ids[4], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -2, 0, FACING_NONE);
	}

	if (!local._launched2Fl && (g_engine->getRandomNumber(1, 30000) == 1)) {
		kernel_seq_delete(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 5, 0, 0, 1);
		int idx = kernel_add_dynamic(words_bats, words_look_at, 0, g_sequence_ids[5], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -2, 0, FACING_NONE);
		local._launched2Fl = true;
	}

	if (kernel.trigger == 73)
		g_engine->_soundManager->command(37, 0);

	if (local._rexDivingFl && (kernel_anim[0].frame >= 9)) {
		g_engine->_soundManager->command(36, 0);
		local._rexDivingFl = false;
	}
}

static void room_111_pre_parser() {
	if (player_said_2(walk_through, cave_entrance))
		player.walk_off_edge_to_room = 212;
}

static void room_111_parser() {
	if (player_said_2(dive_into, pool) && player_has(OBJ_REBREATHER)) {
		switch (kernel.trigger) {
		case 0:
			kernel_run_animation(kernel_full_name(111, 'A', 1, "", KERNEL_AA), 1);
			local._rexDivingFl = true;
			player.commands_allowed = false;
			player.walker_visible = false;
			break;

		case 1:
			new_room = 110;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, cave_floor))
		text_show(11101);
	else if (player_said_2(look, pool))
		text_show(11102);
	else if (player_said_2(look, cave_entrance))
		text_show(11103);
	else if (player_said_2(look, stalagmites))
		text_show(11104);
	else if (player_said_2(look, large_stalagmite))
		text_show(11105);
	else if ((player_said_1(pull) || player_said_1(take)) && (player_said_1(stalagmites) || player_said_1(large_stalagmite)))
		text_show(11106);
	else
		return;

	player.command_ready = false;
}

void room_111_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._stampedFl);
	s.syncAsByte(local._launch1Fl);
	s.syncAsByte(local._launched2Fl);
	s.syncAsByte(local._rexDivingFl);
}

void room_111_preload() {
	room_init_code_pointer = room_111_init;
	room_pre_parser_code_pointer = room_111_pre_parser;
	room_parser_code_pointer = room_111_parser;
	room_daemon_code_pointer = room_111_daemon;

	vocab_make_active(words_bats);
	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
