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

static void room_108_init() {
	if (global[kHoovicSated] == 2)
		global[kHoovicSated] = 0;

	g_sprite_ids[0] = kernel_load_series(kernel_name('X', 0), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('X', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('X', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('X', 3), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_full_name(105, 'f', 4, "", KERNEL_SS), 0);

	g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 13, 7, 0, 0);
	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 16, 9, 0, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 17, 3, 0, 0);
	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 14, 13, 0, 0);

	for (int i = 0; i <= 3; i++)
		kernel_seq_depth(g_sequence_ids[i], 0);

	if (global[kFishIn108]) {
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[4], 41, 109);
		int idx = kernel_add_dynamic(words_dead_fish, words_swim_to, 0, g_sequence_ids[4], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 41, 109, FACING_NORTHWEST);
	}

	if (previous_room == 107) {
		player.x = 138;
		player.y = 58;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 305;
		player.y = 98;
	}

	kernel.quotes = quote_load(74, 75, 76, 53, 52, 0);
	section_1_music();
}

static void room_108_daemon() {
	// No implementation
}

static void room_108_pre_parser() {
	if (player_said_2(swim_under, overhang_to_east))
		player.walk_off_edge_to_room = 109;
}

static void room_108_parser() {
	if (player.look_around)
		text_show(10812);
	else if (player_said_2(take, dead_fish) && global[kFishIn108]) {
		if (player_has(OBJ_DEAD_FISH)) {
			int randVal = g_engine->getRandomNumber(74, 76);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, randVal), 0, 0, 0x1110, 120, 0, 34);
		} else {
			kernel_seq_delete(g_sequence_ids[4]);
			inter_give_to_player(OBJ_DEAD_FISH);
			global[kFishIn108] = false;
			object_examine(OBJ_DEAD_FISH, 10808, 0);
		}
	} else if (player_said_2(swim_towards, open_area_to_north))
		new_room = 107;
	else if (player_said_2(look, cliff_face))
		text_show(10801);
	else if (player_said_2(look, ocean_floor))
		text_show(10802);
	else if (player_said_2(look, odd_rock_formation))
		text_show(10803);
	else if (player_said_2(take, odd_rock_formation))
		text_show(10804);
	else if (player_said_2(look, rocks))
		text_show(10805);
	else if (player_said_2(take, rocks))
		text_show(10806);
	else if (player_said_2(look, dead_fish))
		text_show(10807);
	else if (player_said_2(look, overhang_to_east))
		text_show(10809);
	else if (player_said_2(look, open_area_to_north))
		text_show(10810);
	else if (player_said_2(look, surface))
		text_show(10811);
	else
		return;

	player.command_ready = false;
}

void room_108_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_108_preload() {
	room_init_code_pointer = room_108_init;
	room_pre_parser_code_pointer = room_108_pre_parser;
	room_parser_code_pointer = room_108_parser;
	room_daemon_code_pointer = room_108_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
