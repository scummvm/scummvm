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
#include "mads/nebular/rooms/section4.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_408_init() {
	player.x = 137;
	player.y = 150;
	player.facing = FACING_NORTH;

	g_sprite_ids[1] = kernel_load_series("*ROXRC_7", 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('m', -1), 0);

	if (object_is_here(OBJ_TARGET_MODULE)) {
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
		kernel_seq_depth(g_sequence_ids[2], 3);
		int idx = kernel_add_dynamic(words_target_module, words_walkto, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 283, 128, FACING_NORTHEAST);
	}
	section_4_music();
}

static void room_408_pre_parser() {
	if ((player_said_1(take) && !player_said_1(target_module)) || player_said_2(pull, pin) || player_said_2(open, carton))
		player.need_to_walk = false;

	if ((player_said_2(look, target_module) && object_is_here(OBJ_TARGET_MODULE)) || player_said_2(look, chest))
		player.need_to_walk = true;
}

static void room_408_parser() {
	if (player_said_2(walk_into, corridor_to_south)) {
		new_room = 405;
		g_engine->_soundManager->command(58, 0);
	} else if (player_said_2(take, target_module) && (object_is_here(OBJ_TARGET_MODULE) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			g_engine->_soundManager->command(57, 0);
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[1] = kernel_seq_pingpong(g_sprite_ids[1], true, 7, 0, 0, 2);
			kernel_seq_range(g_sequence_ids[1], 1, 2);
			kernel_seq_player(g_sequence_ids[1], false);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(g_sequence_ids[2]);
			inter_give_to_player(OBJ_TARGET_MODULE);
			object_examine(OBJ_TARGET_MODULE, 40847, 0);
			break;

		case 2:
			player.clock = player.frame_delay + kernel.clock;
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, armored_vehicle))
		text_show(40810);
	else if (player_said_2(take, armored_vehicle) || player_said_2(take, anvil) || player_said_2(take, two_ton_weight))
		text_show(40811);
	else if (player_said_2(look, missile))
		text_show(40812);
	else if (player_said_2(take, missile))
		text_show(40813);
	else if (player_said_2(look, grenade))
		text_show(40814);
	else if (player_said_2(take, grenade))
		text_show(40815);
	else if (player_said_2(take, pin) || player_said_2(pull, pin))
		text_show(40816);
	else if (player_said_2(look, blimp))
		text_show(40817);
	else if (player_said_2(take, blimp))
		text_show(40818);
	else if (player_said_2(look, ammunition))
		text_show(40819);
	else if (player_said_2(take, ammunition))
		text_show(40820);
	else if (player_said_2(look, catapult))
		text_show(40821);
	else if (player_said_2(take, catapult))
		text_show(40822);
	else if (player_said_2(look, chest)) {
		if (object_is_here(OBJ_TARGET_MODULE))
			text_show(40823);
		else
			text_show(40824);
	} else if (player_said_2(take, chest))
		text_show(40825);
	else if (player_said_2(look, suit_of_armor))
		text_show(40826);
	else if (player_said_2(take, suit_of_armor))
		text_show(40827);
	else if (player_said_2(look, escape_hatch))
		text_show(40828);
	else if (player_said_2(open, escape_hatch) || player_said_2(pull, escape_hatch))
		text_show(40829);
	else if (player_said_2(look, barrels))
		text_show(40830);
	else if (player_said_2(take, barrels))
		text_show(40831);
	else if (player_said_2(look, inflatable_raft))
		text_show(40832);
	else if (player_said_2(take, inflatable_raft))
		text_show(40833);
	else if (player_said_2(look, tomato))
		text_show(40834);
	else if (player_said_2(take, tomato))
		text_show(40835);
	else if (player_said_2(look, anvil))
		text_show(40836);
	else if (player_said_2(look, two_ton_weight))
		text_show(40837);
	else if (player_said_2(look, powder_container))
		text_show(40838);
	else if (player_said_2(look, powder_puff))
		text_show(40839);
	else if (player_said_2(take, powder_puff))
		text_show(40840);
	else if (player_said_2(look, carton))
		text_show(40841);
	else if (player_said_2(take, carton))
		text_show(40842);
	else if (player_said_2(open, carton))
		text_show(40843);
	else if (player_said_2(look, corridor_to_south))
		text_show(40844);
	else if (player.look_around)
		text_show(40845);
	else if (player_said_2(look, target_module) && object_is_here(OBJ_TARGET_MODULE))
		text_show(40846);
	else if (player_said_2(look, loading_ramp))
		text_show(40848);
	else if (player_said_2(open, chest))
		text_show(40849);
	else
		return;

	player.command_ready = false;
}

void room_408_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_408_preload() {
	room_init_code_pointer = room_408_init;
	room_pre_parser_code_pointer = room_408_pre_parser;
	room_parser_code_pointer = room_408_parser;

	section_4_walker();
	section_4_interface();
	vocab_make_active(words_target_module);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
