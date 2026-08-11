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
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _devilTime;
	bool _devilRunningFl;
};

static Scratch local;


static void room_214_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('e', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('e', 1), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('t', -1), 0);
	g_sprite_ids[4] = kernel_load_series("*RXMRD_7", 0);

	local._devilTime = player.clock;
	local._devilRunningFl = false;

	if (object_is_here(OBJ_POISON_DARTS)) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[1], 103, 86);
		kernel_seq_depth(g_sequence_ids[1], 11);
	} else {
		kernel_flip_hotspot(words_poison_darts, false);
	}

	if (object_is_here(OBJ_BLOWGUN)) {
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 0, 0);
		kernel_seq_loc(g_sequence_ids[2], 90, 87);
		kernel_seq_depth(g_sequence_ids[2], 13);
	} else {
		kernel_flip_hotspot(words_blowgun, false);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 191;
		player.y = 152;
	}

	section_2_music();
}

static void room_214_daemon() {
	if ((player.clock - local._devilTime > 800) && !local._devilRunningFl) {
		local._devilRunningFl = true;
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 6, 1);
		kernel_seq_range(g_sequence_ids[3], 1, 4);
		kernel_seq_depth(g_sequence_ids[3], 2);
		kernel_add_dynamic(words_captive_creature, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
	}

	if (local._devilRunningFl) {
		switch (kernel.trigger) {
		case 71:
		{
			int oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 0, 5);
			kernel_seq_timeout(oldIdx, g_sequence_ids[3]);
			kernel_add_dynamic(words_captive_creature, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[3], 5, 8);
			kernel_seq_depth(g_sequence_ids[3], 2);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 72);
		}
		break;

		case 72:
		{
			int oldIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 0, 1);
			kernel_seq_timeout(oldIdx, g_sequence_ids[3]);
			kernel_add_dynamic(words_captive_creature, words_walkto, 0, g_sequence_ids[3], 0, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[3], 9, -2);
			kernel_seq_depth(g_sequence_ids[3], 2);
			local._devilTime = player.clock;
			local._devilRunningFl = false;
		}
		break;

		default:
			break;
		}
	}
}

static void room_214_parser() {
	if (player.look_around)
		text_show(21427);
	else if (player_said_2(walk_outside, hut))
		new_room = 207;
	else if (player_said_2(take, poison_darts) && (kernel.trigger || object_is_here(OBJ_POISON_DARTS))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], true, 6, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			g_sequence_ids[4] = kernel_seq_backward(g_sprite_ids[4], true, 6, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 2);
			kernel_seq_delete(g_sequence_ids[1]);
			inter_give_to_player(OBJ_POISON_DARTS);
			kernel_flip_hotspot(words_poison_darts, false);
			break;

		case 2:
			player.walker_visible = true;
			kernel_timing_trigger(48, 3);
			break;

		case 3:
			player.commands_allowed = true;
			object_examine(OBJ_POISON_DARTS, 0x53A5, 0);
			break;

		default:
			break;
		}
	} else if (player_said_2(take, blowgun) && (kernel.trigger || object_is_here(OBJ_BLOWGUN))) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			g_sequence_ids[4] = kernel_seq_backward(g_sprite_ids[4], false, 6, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 2);
			kernel_seq_delete(g_sequence_ids[2]);
			inter_give_to_player(OBJ_BLOWGUN);
			kernel_flip_hotspot(words_blowgun, false);
			break;

		case 2:
			player.walker_visible = true;
			kernel_timing_trigger(48, 3);
			break;

		case 3:
			player.commands_allowed = true;
			object_examine(OBJ_BLOWGUN, 0x329, 0);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, window))
		text_show(21401);
	else if (player_said_2(look, experiment_cage))
		text_show(21402);
	else if (player_said_2(look, captive_creature))
		text_show(21403);
	else if (player_said_2(look, bear_rug))
		text_show(21404);
	else if (player_said_2(look, trophy))
		text_show(21405);
	else if (player_said_2(look, large_bowl)) {
		if (config_file.naughtiness == NAUGHTY) {
			text_show(21406);
		} else {
			text_show(21407);
		}
	} else if (player_said_2(look, specimen_jars))
		text_show(21408);
	else if (player_said_2(take, large_bowl) || player_said_2(take, specimen_jars))
		text_show(21409);
	else if (player_said_2(look, shrunken_heads))
		text_show(21410);
	else if (player_said_2(take, shrunken_heads) || player_said_2(take, large_heads))
		text_show(21411);
	else if (player_said_2(look, large_heads))
		text_show(21428);
	else if (player_said_2(look, poison_darts) && (player.main_object_source == 4))
		text_show(21412);
	else if (player_said_2(open, experiment_cage))
		text_show(21414);
	else if (player_said_2(talkto, captive_creature))
		text_show(21415);
	else if (player_said_3(give, twinkifruit, captive_creature))
		text_show(21416);
	else if (player_said_3(shoot, blowgun, captive_creature) || player_said_3(hose_down, blowgun, captive_creature))
		text_show(21417);
	else if (player_said_2(look, big_heads))
		text_show(21418);
	else if (player_said_2(take, big_heads))
		text_show(21419);
	else if (player_said_2(take, bear_rug))
		text_show(21420);
	else if (player_said_2(look, floor_of_hut))
		text_show(21421);
	else if (player_said_2(look, blowgun))
		text_show(21422);
	else if (player_said_2(look, table)) {
		if (object_is_here(OBJ_POISON_DARTS) && object_is_here(OBJ_BLOWGUN)) {
			text_show(21423);
		} else if (object_is_here(OBJ_POISON_DARTS) && !object_is_here(OBJ_BLOWGUN)) {
			text_show(21424);
		} else if (!object_is_here(OBJ_POISON_DARTS) && object_is_here(OBJ_BLOWGUN)) {
			text_show(21425);
		} else {
			text_show(21426);
		}
	} else
		return;

	player.command_ready = false;
}

void room_214_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._devilTime);
	s.syncAsByte(local._devilRunningFl);
}

void room_214_preload() {
	room_init_code_pointer = room_214_init;
	room_parser_code_pointer = room_214_parser;
	room_daemon_code_pointer = room_214_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(words_captive_creature);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
