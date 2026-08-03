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
	bool _shootingFl;
};

static Scratch local;


static void room_107_init() {
	for (int i = 0; i < 3; i++)
		g_sprite_ids[i + 1] = kernel_load_series(kernel_name('G', i), 0);

	g_sprite_ids[4] = kernel_load_series(kernel_full_name(105, 'f', 4, "", KERNEL_SS), 0);

	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 14, 7, 0, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 17, 13, 0, 0);
	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 19, 9, 0, 0);

	for (int i = 1; i < 4; i++)
		kernel_seq_depth(g_sequence_ids[i], 0);

	if (global[kFishIn107]) {
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[4], 68, 151);
		kernel_seq_depth(g_sequence_ids[4], 1);
		int idx = kernel_add_dynamic(words_dead_fish, words_swim_to, 0, g_sequence_ids[4], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 78, 135, FACING_SOUTHWEST);
	}

	if (previous_room == 105) {
		player.x = 132;
		player.y = 47;
	}
	else if (previous_room == 106) {
		player.x = 20;
		player.y = 91;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 223;
		player.y = 151;
	}

	if (((previous_room == 105) || (previous_room == 106)) && (g_engine->getRandomNumber(1, 3) == 1)) {
		g_sprite_ids[0] = kernel_load_series(kernel_full_name(105, 'R', 1, "", KERNEL_SS), 0);
		g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], true, 4, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[0], 270, 150);
		kernel_seq_motion(g_sequence_ids[0], KERNEL_TRIGGER_SPRITE, -200, 0);
		kernel_seq_depth(g_sequence_ids[0], 2);
		kernel_add_dynamic(words_manta_ray, words_swim_to, 0, g_sequence_ids[0], 0, 0, 0, 0);
	}

	kernel.quotes = quote_load(74, 75, 76, 53, 52, 0);
	local._shootingFl = false;

	if (g_engine->getRandomNumber(1, 3) == 1) {
		kernel_run_animation(kernel_full_name(107, 'B', -1, "", KERNEL_AA), 0);
		local._shootingFl = true;
	}

	section_1_music();
}

static void room_107_daemon() {
	if (local._shootingFl && (kernel_anim[0].frame >= 19)) {
		kernel_message_add(quote_string(kernel.quotes, 52), 0, 0, 0x1110, 120, 0, 34);
		local._shootingFl = false;
	}
}

static void room_107_pre_parser() {
	if (player_said_2(swim_towards, open_area_to_west))
		player.walk_off_edge_to_room = 106;

	if (player_said_2(swim_towards, open_area_to_south)) {
		if (g_engine->isDemo()) {
			text_show(99);
			player.command_ready = false;
		} else {
			player.walk_off_edge_to_room = 108;
		}
	}
}

static void room_107_parser() {
	if (player.look_around)
		text_show(10708);
	else if (player_said_2(take, dead_fish) && global[kFishIn107]) {
		if (player_has(OBJ_DEAD_FISH)) {
			int randVal = g_engine->getRandomNumber(74, 76);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, randVal), 0, 0, 0x1110, 120, 0, 34);
		} else {
			kernel_seq_delete(g_sequence_ids[4]);
			inter_give_to_player(OBJ_DEAD_FISH);
			global[kFishIn107] = false;
			object_examine(OBJ_DEAD_FISH, 802, 0);
		}
	} else if (player_said_2(swim_towards, northern_sea_cliff))
		if (g_engine->isDemo())
			text_show(99);
		else
			new_room = 105;
	else if (player_said_2(look, northern_sea_cliff))
		text_show(10701);
	else if (player_said_2(look, dead_fish) && (player.main_object_source == STROKE_INTERFACE))
		text_show(10702);
	else if (player_said_2(look, bush_like_formation))
		text_show(10703);
	else if (player_said_2(look, rock))
		text_show(10704);
	else if (player_said_2(look, seaweed))
		text_show(10705);
	else if (player_said_2(look, open_area_to_south))
		text_show(10706);
	else if (player_said_2(look, cliff_face))
		text_show(10707);
	else if (player_said_2(look, manta_ray))
		text_show(10709);
	else if (player_said_2(take, manta_ray))
		text_show(10710);
	else
		return;

	player.command_ready = false;
}

void room_107_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._shootingFl);
}

void room_107_preload() {
	room_init_code_pointer = room_107_init;
	room_pre_parser_code_pointer = room_107_pre_parser;
	room_parser_code_pointer = room_107_parser;
	room_daemon_code_pointer = room_107_daemon;

	section_1_walker();
	section_1_interface();
	vocab_make_active(words_manta_ray);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
