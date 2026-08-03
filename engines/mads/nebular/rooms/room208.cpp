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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _rhotundaTurnFl;
	bool _boundingFl;
	long _rhotundaTime;
};

static Scratch local;


static void updateTrap() {
	if (global[kRhotundaStatus] == 1) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 8, 24, 0, 0);
		kernel_seq_depth(g_sequence_ids[1], 5);
		int idx = kernel_add_dynamic(words_huge_legs, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 100, 146, FACING_NORTH);
		kernel_flip_hotspot(414, false);
		return;
	}

	switch (global[kLeavesStatus]) {
	case LEAVES_ON_GROUND:
	{
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
		kernel_seq_depth(g_sequence_ids[2], 15);
		int idx = kernel_add_dynamic(words_pile_of_leaves, words_walkto, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 60, 152, FACING_NORTH);
	}
	break;
	case LEAVES_ON_TRAP:
	{
		kernel_seq_depth(g_sequence_ids[3], 15);
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_flip_hotspot(words_deep_pit, false);
		int idx = kernel_add_dynamic(words_leaf_covered_pit, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 100, 146, FACING_NORTH);
		kernel_dynamic_hot[idx].prep = PREP_ON;
	}
	break;
	default:
		break;
	}
}

static void room_208_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('a', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[5] = kernel_load_series("*RXMBD_8", 0);

	updateTrap();

	local._rhotundaTurnFl = false;
	local._boundingFl = false;
	kernel_message_font = font_inter;
	kernel_message_spacing = 0;

	if (previous_room == 207) {
		player.x = 8;
		player.y = 122;
		player.facing = FACING_EAST;
	} else if (previous_room == 203) {
		player.x = 142;
		player.y = 108;
		player.facing = FACING_SOUTH;
	} else if (previous_room == 209) {
		player.x = 307;
		player.y = 123;
		player.facing = FACING_WEST;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 162;
		player.y = 149;
		player.facing = FACING_NORTH;
	}

	kernel.quotes = quote_load(129, 70, 0);

	if ((previous_room == 207) && (global[kMonkeyStatus] == MONKEY_HAS_BINOCULARS)) {
		int msgIndex = kernel_message_add(quote_string(kernel.quotes, 129), 0, 0, 0x1110, 120, 0, 34);
		kernel_message_teletype(msgIndex, 4, true);
	}

	pal_change_color(16, 0, 0, 63);
	pal_change_color(17, 0, 0, 45);

	section_2_music();
}

static void room_208_daemon() {
	if (local._boundingFl && (kernel_anim[0].anim != nullptr) &&
		(local._rhotundaTime <= kernel_anim[0].frame)) {
		local._rhotundaTime = kernel_anim[0].frame;

		if (local._rhotundaTime == 125)
			kernel_seq_delete(g_sequence_ids[4]);
	}

	if (!local._rhotundaTurnFl)
		return;

	if ((Common::Point(player.x, player.y) != Common::Point(20, 148)) || (player.facing != FACING_EAST))
		return;

	if ((kernel.trigger == 0) && local._boundingFl)
		return;

	local._boundingFl = true;

	switch (kernel.trigger) {
	case 0:
		kernel_run_animation(kernel_name('A', -1), 81);
		local._rhotundaTime = 0;
		break;
	case 81:
		kernel_seq_delete(g_sprite_ids[15]);
		global[kRhotundaStatus] = 1;
		updateTrap();
		kernel_timing_trigger(90, 82);
		break;
	case 82:
		player.commands_allowed = true;
		break;
	default:
		break;
	}
}

static void room_208_pre_parser() {

	if (player_said_1(look) && player.ready_to_walk)
		player.need_to_walk = true;

	if (player_said_2(walk_towards, grassland_to_east))
		player.walk_off_edge_to_room = 209;

	if (player_said_2(walk_towards, open_area_to_west))
		player.walk_off_edge_to_room = 207;
}

static void subAction(int mode) {
	switch (kernel.trigger) {
	case 0:
	{
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 6, 0, 0, 1);
		kernel_seq_player(g_sequence_ids[5], false);

		int endTrigger;
		if ((mode == 1) || (mode == 2))
			endTrigger = 1;
		else
			endTrigger = 2;

		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, endTrigger);
	}
	break;
	case 1:
	{
		int oldSeq = g_sequence_ids[5];
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 12, 0, 0, 3);
		kernel_seq_range(g_sequence_ids[5], 3, 4);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_timeout(oldSeq, g_sequence_ids[5]);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 2);
		g_engine->_soundManager->command(20, 0);
	}
	break;

	case 2:
	{
		switch (mode) {
		case 1:
			inter_give_to_player(OBJ_BIG_LEAVES);
			kernel_seq_delete(g_sequence_ids[2]);
			global[kLeavesStatus] = 1;
			break;

		case 2:
			inter_move_object(OBJ_BIG_LEAVES, 1);
			global[kLeavesStatus] = 2;
			updateTrap();
			break;

		case 3:
			kernel_seq_delete(g_sequence_ids[3]);
			g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
			inter_take_from_player(OBJ_TWINKIFRUIT, 1);
			g_engine->_soundManager->command(34, 0);
			break;

		case 4:
			inter_take_from_player(OBJ_BURGER, 1);
			g_engine->_soundManager->command(33, 0);
			break;

		case 5:
			inter_take_from_player(OBJ_DEAD_FISH, 1);
			g_engine->_soundManager->command(33, 0);
			break;

		default:
			break;
		}

		int oldVal = g_sequence_ids[5];
		g_sequence_ids[5] = kernel_seq_backward(g_sprite_ids[5], false, 6, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[5], 1, 3);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_timeout(oldVal, g_sequence_ids[5]);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 3);
	}
	break;

	case 3:
		player.walker_visible = true;
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_208_parser() {
	if (player_said_2(walk_towards, lowlands_to_north)) {
		if (global[kRhotundaStatus])
			new_room = 203;
		else if (kernel.trigger == 0) {
			player.commands_allowed = false;
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 70), 0, 0, 0x1110, 120, 1, 34);
			kernel_message_teletype(msgIndex, 4, true);
		} else if (kernel.trigger == 1)
			new_room = 203;
	} else if (player_said_2(walk_towards, field_to_south))
		new_room = 212;
	else if (player_said_2(take, pile_of_leaves) && (!global[kLeavesStatus] || kernel.trigger)) {
		subAction(1);
		if (player.commands_allowed)
			object_examine(OBJ_BIG_LEAVES, 0x326, 0);
	} else if (player_said_3(put, big_leaves, deep_pit) && (global[kLeavesStatus] == 1 || kernel.trigger))
		subAction(2);
	else if (player_said_3(put, twinkifruit, leaf_covered_pit)) {
		subAction(3);
		if (player.commands_allowed) {
			player.commands_allowed = false;
			local._rhotundaTurnFl = true;
			player_walk(20, 148, FACING_EAST);
		}
	} else if (player_said_3(put, burger, leaf_covered_pit)) {
		subAction(4);
		if (player.commands_allowed)
			text_show(20812);
	} else if (player_said_3(put, dead_fish, leaf_covered_pit)) {
		subAction(5);
		if (player.commands_allowed)
			text_show(20812);
	} else if (player_said_2(look, cumulous_cloud))
		text_show(20801);
	else if (player_said_2(look, open_area_to_west))
		text_show(20802);
	else if (player_said_2(look, thorny_bush))
		text_show(20803);
	else if (player_said_2(look, rocks))
		text_show(20804);
	else if (player_said_2(look, small_cactus))
		text_show(20805);
	else if (player_said_2(take, small_cactus))
		text_show(20806);
	else if (player_said_2(look, grassland_to_east))
		text_show(20807);
	else if (player_said_2(look, deep_pit))
		text_show(20808);
	else if (player_said_2(look, pile_of_leaves))
		text_show(20809);
	else if (player_said_2(look, leaf_covered_pit)) {
		if (game.difficulty == DIFFICULTY_EASY)
			text_show(20810);
		else
			text_show(20811);
	} else if (player_said_2(look, tree) || player_said_2(look, trees))
		text_show(20813);
	else if (player_said_2(take, leaf_covered_pit))
		text_show(20814);
	else if (player_said_2(look, huge_legs))
		text_show(20815);
	else if (player_said_2(take, huge_legs) || player_said_2(pull, huge_legs))
		text_show(20816);
	else if (player.look_around && (global[kRhotundaStatus] == 1))
		text_show(20819);
	else if (player.look_around && (global[kLeavesStatus] == 2))
		text_show(20818);
	else if (player.look_around)
		text_show(20817);
	else
		return;

	player.command_ready = false;
}

void room_208_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._rhotundaTurnFl);
	s.syncAsByte(local._boundingFl);
	s.syncAsSint32LE(local._rhotundaTime);
}

void room_208_preload() {
	room_init_code_pointer = room_208_init;
	room_pre_parser_code_pointer = room_208_pre_parser;
	room_parser_code_pointer = room_208_parser;
	room_daemon_code_pointer = room_208_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(words_huge_legs);
	vocab_make_active(words_leaf_covered_pit);
	vocab_make_active(words_pile_of_leaves);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
