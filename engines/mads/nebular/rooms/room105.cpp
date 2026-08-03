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
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _explosionFl;
};

static Scratch local;


static void room_105_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('m', 1), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('f', 4), 0);
	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 8, 0, 0, 0);

	if (global[kFishIn105]) {
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[4], 48, 144);

		int idx = kernel_add_dynamic(words_dead_fish, words_swim_to, 0, g_sequence_ids[4], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 56, 141, FACING_NORTHWEST);
	}

	if (previous_room == 104) {
		player.x = 13;
		player.y = 97;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 116;
		player.y = 147;
	}

	kernel.quotes = quote_load(74, 75, 76, 53, 52, 0);
	local._explosionFl = false;

	section_1_music();
}

static void room_105_daemon() {
	if ((Common::Point(player.x, player.y) == Common::Point(170, 87)) && (kernel.trigger || !local._explosionFl)) {
		local._explosionFl = true;
		switch (kernel.trigger) {
		case 0:
			kernel_message_purge();
			kernel_dump_all();
			player.commands_allowed = false;
			player.walker_visible = false;

			g_sprite_ids[0] = kernel_load_series(kernel_name('m', 0), 0);
			g_sprite_ids[2] = kernel_load_series(kernel_name('m', 2), 0);
			g_sprite_ids[3] = kernel_load_series(kernel_name('m', 3), 0);
			g_engine->_soundManager->command(33, 0);
			kernel_seq_init();
			kernel_new_palette();

			g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[0], 8);
			kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_EXPIRE, 0, 1);

			if (config_file.naughtiness >= NICE)
				kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_SPRITE, 8, 3);
			break;

		case 1:
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 0);
			kernel_seq_depth(g_sequence_ids[3], 8);
			kernel_seq_range(g_sequence_ids[3], -2, -2);
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 9, 0, 0, 1);
			kernel_seq_timeout(g_sequence_ids[0], g_sequence_ids[2]);
			kernel_seq_depth(g_sequence_ids[2], 8);
			kernel_seq_range(g_sequence_ids[2], 5, 7);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
		{
			int oldIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 9, 0, 0, 0);
			kernel_seq_timeout(oldIdx, g_sequence_ids[2]);
			kernel_seq_depth(g_sequence_ids[2], 8);
			kernel_timing_trigger(90, 3);
		}
		break;

		case 3:
			text_show(10507);
			kernel.force_restart = true;
			kernel_timing_trigger(90, 4);
			break;

		default:
			break;
		}
	}

	if (player.walking && (player.next_special_code > 0)) {
		player_cancel_command();
		player_start_walking(170, 87, FACING_NONE);
		player.next_special_code = 0;
	}

	if ((player.special_code > 0) && player.commands_allowed)
		player.commands_allowed = false;
}

static void room_105_pre_parser() {
	if (player_said_2(swim_towards, western_cliff_face))
		player.walk_off_edge_to_room = 104;

	if (player_said_2(swim_towards, open_area_to_south))
		player.walk_off_edge_to_room = 107;

	if (player_said_1(mine) && (player_said_1(talkto) || player_said_1(look)))
		player.need_to_walk = false;
}

static void room_105_parser() {
	if (player.look_around)
		text_show(10512);
	else if (player_said_2(take, dead_fish) && global[kFishIn105]) {
		if (player_has(OBJ_DEAD_FISH)) {
			int randVal = g_engine->getRandomNumber(74, 76);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, randVal), 0, 0, 0x1110, 120, 0, 34);
		} else {
			kernel_seq_delete(g_sequence_ids[4]);
			inter_give_to_player(OBJ_DEAD_FISH);
			global[kFishIn105] = false;
			object_examine(OBJ_DEAD_FISH, 802, 0);
		}
	} else if (player_said_2(look, western_cliff_face))
		text_show(10501);
	else if (player_said_2(look, cliff_face))
		text_show(10502);
	else if (player_said_2(look, ocean_floor))
		text_show(10503);
	else if (player_said_2(look, medical_waste))
		text_show(10504);
	else if (player_said_2(take, medical_waste))
		text_show(10505);
	else if (player_said_2(look, mine))
		text_show(10506);
	else if (player_said_2(look, dead_fish))
		text_show(10508);
	else if (player_said_2(look, surface))
		text_show(10509);
	else if (player_said_2(look, open_area_to_south))
		text_show(10510);
	else if (player_said_2(look, rocks))
		text_show(10511);
	else
		return;

	player.command_ready = false;
}

void room_105_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._explosionFl);
}

void room_105_preload() {
	room_init_code_pointer = room_105_init;
	room_pre_parser_code_pointer = room_105_pre_parser;
	room_parser_code_pointer = room_105_parser;
	room_daemon_code_pointer = room_105_daemon;

	anim_himem_preload(kernel_name('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
