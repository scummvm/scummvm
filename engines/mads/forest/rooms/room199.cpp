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

#include "mads/forest/rooms/section1.h"
#include "mads/forest/mads/inventory.h"
#include "mads/forest/mads/words.h"
#include "mads/forest/digi.h"
#include "mads/forest/global.h"
#include "mads/forest/journal.h"
#include "mads/core/game.h"
#include "mads/core/global.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/player.h"

namespace MADS {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[16];
	int16 sequence[16];
	int16 needed_item_num;
	int16 val4;
	int16 bg_active;
	int16 bg_frame;
	int16 anim_flag;
	int16 anim_frame;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence


static void room_199_init1() {
	if (flags[15] >= 1) {
		ss[1]  = kernel_load_series("*rm199n2",  0); seq[1]  = kernel_seq_stamp(ss[1],  false, 1); kernel_seq_depth(seq[1],  5);
	}
	if (flags[16] >= 1) {
		ss[2]  = kernel_load_series("*rm199n3",  0); seq[2]  = kernel_seq_stamp(ss[2],  false, 1); kernel_seq_depth(seq[2],  5);
	}
	if (flags[17] >= 1 || flags[24] >= 1) {
		ss[3]  = kernel_load_series("*rm199n5",  0); seq[3]  = kernel_seq_stamp(ss[3],  false, 1); kernel_seq_depth(seq[3],  5);
	}
	if (flags[18] >= 1) {
		ss[4]  = kernel_load_series("*rm199n4",  0); seq[4]  = kernel_seq_stamp(ss[4],  false, 1); kernel_seq_depth(seq[4],  5);
	}
	if (flags[19] >= 1) {
		ss[5]  = kernel_load_series("*rm199n7",  0); seq[5]  = kernel_seq_stamp(ss[5],  false, 1); kernel_seq_depth(seq[5],  5);
	}
	if (flags[20] >= 1) {
		ss[6]  = kernel_load_series("*rm199n8",  0); seq[6]  = kernel_seq_stamp(ss[6],  false, 1); kernel_seq_depth(seq[6],  5);
	}
	if (flags[21] >= 1 || flags[25] >= 1) {
		ss[7]  = kernel_load_series("*rm199n6",  0); seq[7]  = kernel_seq_stamp(ss[7],  false, 1); kernel_seq_depth(seq[7],  5);
	}
	if (flags[22] >= 1) {
		ss[8]  = kernel_load_series("*rm199n9",  0); seq[8]  = kernel_seq_stamp(ss[8],  false, 1); kernel_seq_depth(seq[8],  5);
	}
	if (flags[26] >= 1 || flags[31] >= 1) {
		ss[9]  = kernel_load_series("*rm199n10", 0); seq[9]  = kernel_seq_stamp(ss[9],  false, 1); kernel_seq_depth(seq[9],  5);
	}
	if (flags[27] >= 1) {
		ss[10] = kernel_load_series("*rm199n11", 0); seq[10] = kernel_seq_stamp(ss[10], false, 1); kernel_seq_depth(seq[10], 5);
	}
	if (flags[28] >= 1) {
		ss[11] = kernel_load_series("*rm199n12", 0); seq[11] = kernel_seq_stamp(ss[11], false, 1); kernel_seq_depth(seq[11], 5);
	}
	if (flags[29] >= 1) {
		ss[12] = kernel_load_series("*rm199n13", 0); seq[12] = kernel_seq_stamp(ss[12], false, 1); kernel_seq_depth(seq[12], 5);
	}
	if (flags[30] >= 1) {
		ss[13] = kernel_load_series("*rm199n14", 0); seq[13] = kernel_seq_stamp(ss[13], false, 1); kernel_seq_depth(seq[13], 5);
	}
	if (flags[32] >= 1) {
		ss[14] = kernel_load_series("*rm199n15", 0); seq[14] = kernel_seq_stamp(ss[14], false, 1); kernel_seq_depth(seq[14], 5);
	}
	if (flags[33] >= 1) {
		ss[15] = kernel_load_series("*rm199n16", 0); seq[15] = kernel_seq_stamp(ss[15], false, 1); kernel_seq_depth(seq[15], 5);
	}
}

static void room_199_init() {
	global[play_background_sounds] = 0;
	global[g009] = -1;
	global_midi_play(7);

	local->needed_item_num      = -1;
	local->val4      = 0;
	local->anim_flag  = 0;
	local->anim_frame = -1;

	if (previous_room == 304 || previous_room == 402) {
		if (global[g070] != 0) {
			local->bg_frame = 1;
			ss[1] = kernel_load_series("*rm199z1", 0);
			kernel_flip_hotspot(words_flowers, false);
			kernel_flip_hotspot(words_fwt, false);
			int16 slot = local->bg_frame;
			seq[slot] = kernel_seq_stamp(ss[slot], false, 1);
			kernel_seq_depth(seq[slot], 1);
			local->bg_active = slot;
			player.commands_allowed = false;
			kernel_timing_trigger(300, 111);
			return;
		}
		room_199_init1();
		local->bg_frame = 0;
		ss[0] = kernel_load_series("*rm199n1", 0);
	} else if (previous_room == 503) {
		local->bg_frame = 3;
		ss[3] = kernel_load_series("*rm199v1", 0);
		kernel_flip_hotspot(words_flowers, false);
		kernel_flip_hotspot(words_fwt, false);
		int16 slot = local->bg_frame;
		seq[slot] = kernel_seq_stamp(ss[slot], false, 1);
		kernel_seq_depth(seq[slot], 5);
		local->bg_active = slot;
		player.commands_allowed = false;
		kernel_timing_trigger(2, 112);
		return;
	} else {
		room_199_init1();
		local->bg_frame = 0;
		ss[0] = kernel_load_series("*rm199n1", 0);
	}

	kernel_flip_hotspot(words_flowers, false);
	kernel_flip_hotspot(words_fwt, false);
	int16 slot = local->bg_frame;
	seq[slot] = kernel_seq_stamp(ss[slot], false, 1);
	kernel_seq_depth(seq[slot], 1);
	local->bg_active = slot;
}

static int room_199_has_materials() {
	return player_has(reeds) && player_has(elm_leaves) && player_has(cogs) &&
		player_has(feather) && player_has(stick) && player_has(lily_pad) &&
		player_has(twine) && player_has(web) && player_has(vine_weed) &&
		player_has(planks) && player_has(sticks) ? 1 : 0;
}

static void room_199_iterate_missing_items() {
	if (kernel.trigger == 7)
		kernel_timing_trigger(30, 28);
	if (kernel.trigger != 28)
		return;

	switch (local->needed_item_num) {
	case 1:
		if (!player_has(stick))
			digi_play_build(521, 'e', 6, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 2;
		break;
	case 2:
		if (!player_has(planks))
			digi_play_build(521, 'e', 10, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 3;
		break;
	case 3:
		if (!player_has(elm_leaves))
			digi_play_build(521, 'e', 3, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 4;
		break;
	case 4:
		if (!player_has(web)) digi_play_build(521, 'e', 8, 1);
		else kernel_timing_trigger(2, 28);
		local->needed_item_num = 5;
		break;
	case 5:
		if (!player_has(vine_weed))
			digi_play_build(521, 'e', 9, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 6;
		break;
	case 6:
		if (!player_has(twine)) digi_play_build(521, 'e', 20, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 7;
		break;
	case 7:
		if (!player_has(feather))
			digi_play_build(521, 'e', 5, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 8;
		break;
	case 8:
		if (!player_has(reeds))
			digi_play_build(521, 'e', 2, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 9;
		break;
	case 9:
		if (!player_has(lily_pad))
			digi_play_build(521, 'e', 7, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 10;
		break;
	case 10:
		if (!player_has(cogs))
			digi_play_build(521, 'e', 4, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 11;
		break;
	case 11:
		if (!player_has(sticks))
			digi_play_build(521, 'e', 21, 1);
		else
			kernel_timing_trigger(2, 28);
		local->needed_item_num = 12;
		break;
	case 12:
		digi_play_build(521, 'b', 1, 1);
		local->needed_item_num = 14;
		break;
	case 14:
		local->val4 = 0;
		local->needed_item_num = -1;
		local->anim_flag = 0;
		kernel_timing_trigger(2, 113);
		break;
	default:
		break;
	}
}

static void room_199_anim3() {
	switch (local->needed_item_num) {
	case 1:  digi_play_build(521, 'e', 6, 1);  break;
	case 2:  digi_play_build(521, 'e', 10, 1); break;
	case 3:  digi_play_build(521, 'e', 3, 1);  break;
	case 4:  digi_play_build(521, 'e', 8, 1);  break;
	case 5:  digi_play_build(521, 'e', 9, 1);  break;
	case 6:  digi_play_build(521, 'e', 20, 1); break;
	case 7:  digi_play_build(521, 'e', 5, 1);  break;
	case 8:  digi_play_build(521, 'e', 2, 1);  break;
	case 9:  digi_play_build(521, 'e', 7, 1);  break;
	case 10: digi_play_build(521, 'e', 4, 1);  break;
	case 11: digi_play_build(521, 'e', 21, 1); break;
	default: break;
	}
}

static void room_199_anim4() {
	if (kernel.trigger == 7) {
		kernel_timing_trigger(15, 28);
		local->needed_item_num++;
	}
	if (kernel.trigger != 28)
		return;

	switch (local->needed_item_num) {
	case 1:
		if (player_has(stick)) {
			ss[8] = kernel_load_series("*ob014i", 0);
			seq[8] = kernel_seq_stamp(ss[8], false, 1);
			kernel_seq_depth(seq[8], 1);
			kernel_seq_loc(seq[8], 213, 42);
			kernel_seq_scale(seq[8], 88);
		}
		kernel_timing_trigger(15, 115); return;
	case 2:
		if (player_has(planks)) {
			ss[13] = kernel_load_series("*ob011i", 0);
			seq[13] = kernel_seq_stamp(ss[13], false, 1);
			kernel_seq_depth(seq[13], 1);
			kernel_seq_loc(seq[13], 250, 41);
		}
		kernel_timing_trigger(15, 115); return;
	case 3:
		if (player_has(elm_leaves)) {
			ss[5] = kernel_load_series("*ob012i", 0);
			seq[5] = kernel_seq_stamp(ss[5], false, 1);
			kernel_seq_depth(seq[5], 1);
			kernel_seq_loc(seq[5], 287, 39);
			kernel_seq_scale(seq[5], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 4:
		if (player_has(web)) {
			ss[11] = kernel_load_series("*ob010i", 0);
			seq[11] = kernel_seq_stamp(ss[11], false, 1);
			kernel_seq_depth(seq[11], 1);
			kernel_seq_loc(seq[11], 213, 73);
			kernel_seq_scale(seq[11], 79);
		}
		kernel_timing_trigger(15, 115); return;
	case 5:
		if (player_has(vine_weed)) {
			ss[12] = kernel_load_series("*ob009i", 0);
			seq[12] = kernel_seq_stamp(ss[12], false, 1);
			kernel_seq_depth(seq[12], 1);
			kernel_seq_loc(seq[12], 248, 73);
			kernel_seq_scale(seq[12], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 6:
		if (player_has(twine)) {
			ss[10] = kernel_load_series("*ob008i", 0);
			seq[10] = kernel_seq_stamp(ss[10], false, 1);
			kernel_seq_depth(seq[10], 1);
			kernel_seq_loc(seq[10], 287, 72);
			kernel_seq_scale(seq[10], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 7:
		if (player_has(feather)) {
			ss[7] = kernel_load_series("*ob002i", 0);
			seq[7] = kernel_seq_stamp(ss[7], false, 1);
			kernel_seq_depth(seq[7], 1);
			kernel_seq_loc(seq[7], 212, 108);
			kernel_seq_scale(seq[7], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 8:
		if (player_has(reeds)) {
			ss[4] = kernel_load_series("*ob006i", 0);
			seq[4] = kernel_seq_stamp(ss[4], false, 1);
			kernel_seq_depth(seq[4], 1);
			kernel_seq_loc(seq[4], 249, 105);
			kernel_seq_scale(seq[4], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 9:
		if (player_has(lily_pad)) {
			ss[9] = kernel_load_series("*ob004i", 0);
			seq[9] = kernel_seq_stamp(ss[9], false, 1);
			kernel_seq_depth(seq[9], 1);
			kernel_seq_loc(seq[9], 285, 108);
			kernel_seq_scale(seq[9], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 10:
		if (player_has(cogs)) {
			ss[6] = kernel_load_series("*ob000i", 0);
			seq[6] = kernel_seq_stamp(ss[6], false, 1);
			kernel_seq_depth(seq[6], 1);
			kernel_seq_loc(seq[6], 230, 142);
			kernel_seq_scale(seq[6], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 11:
		if (player_has(sticks)) {
			ss[14] = kernel_load_series("*ob007i", 0);
			seq[14] = kernel_seq_stamp(ss[14], false, 1);
			kernel_seq_depth(seq[14], 1);
			kernel_seq_loc(seq[14], 270, 144);
			kernel_seq_scale(seq[14], 85);
		}
		kernel_timing_trigger(15, 115); return;
	case 12:
		local->val4 = 0;
		local->needed_item_num = -1;
		kernel_timing_trigger(30, 114);
		return;
	default:
		return;
	}
}

static void room_199_daemon() {
	int hasMaterials;

	switch (kernel.trigger) {
	case 7:
		if (local->anim_frame == 11) {
			local->anim_frame = -1;
			kernel_timing_trigger(30, 113);
		}
		break;

	case 111:
		global[g070] = 0;
		leave_journal();
		player.commands_allowed = true;
		break;

	case 112:
		local->needed_item_num = 0;
		local->val4 = -1;
		digi_play_build(521, 'e', 1, 1);
		break;

	case 113:
		leave_journal();
		player.commands_allowed = true;
		break;

	case 114:
		hasMaterials = room_199_has_materials();
		global[g100] = hasMaterials;
		if (hasMaterials) {
			// Got everything we need
			digi_play_build(521, 'e', 11, 1);
			local->anim_frame = 11;
		} else {
			// Start iterating through items to list what's missing
			local->needed_item_num = 1;
			local->anim_flag  = -1;
			digi_play_build(521, 'e', 12, 1);
			room_199_iterate_missing_items();
		}
		break;

	case 115:
		room_199_anim3();
		break;

	default:
		break;
	}

	if (local->val4 != 0)
		room_199_anim4();
	if (local->anim_flag != 0)
		room_199_iterate_missing_items();
}

static void room_199_parser2() {
	if (flags[33] >= 1) {
		kernel_seq_delete(seq[15]); matte_deallocate_series(ss[15], -1);
	}
	if (flags[32] >= 1) {
		kernel_seq_delete(seq[14]); matte_deallocate_series(ss[14], -1);
	}
	if (flags[30] >= 1) {
		kernel_seq_delete(seq[13]); matte_deallocate_series(ss[13], -1);
	}
	if (flags[29] >= 1) {
		kernel_seq_delete(seq[12]); matte_deallocate_series(ss[12], -1);
	}
	if (flags[28] >= 1) {
		kernel_seq_delete(seq[11]); matte_deallocate_series(ss[11], -1);
	}
	if (flags[27] >= 1) {
		kernel_seq_delete(seq[10]); matte_deallocate_series(ss[10], -1);
	}
	if (flags[26] >= 1 || flags[31] >= 1) {
		kernel_seq_delete(seq[9]);  matte_deallocate_series(ss[9],  -1);
	}
	if (flags[22] >= 1) {
		kernel_seq_delete(seq[8]);  matte_deallocate_series(ss[8],  -1);
	}
	if (flags[21] >= 1 || flags[25] >= 1) {
		kernel_seq_delete(seq[7]);  matte_deallocate_series(ss[7],  -1);
	}
	if (flags[20] >= 1) {
		kernel_seq_delete(seq[6]);  matte_deallocate_series(ss[6],  -1);
	}
	if (flags[19] >= 1) {
		kernel_seq_delete(seq[5]);  matte_deallocate_series(ss[5],  -1);
	}
	if (flags[18] >= 1) {
		kernel_seq_delete(seq[4]);  matte_deallocate_series(ss[4],  -1);
	}
	if (flags[17] >= 1 || flags[24] >= 1) {
		kernel_seq_delete(seq[3]);  matte_deallocate_series(ss[3],  -1);
	}
	if (flags[16] >= 1) {
		kernel_seq_delete(seq[2]);  matte_deallocate_series(ss[2],  -1);
	}
	if (flags[15] >= 1) {
		kernel_seq_delete(seq[1]);  matte_deallocate_series(ss[1],  -1);
	}
}

static void room_199_parser1() {
	if (local->bg_active == 2)
		kernel_flip_hotspot(words_flowers, false);
	if (local->bg_active == 3)
		kernel_flip_hotspot(words_fwt, false);

	int16 old_slot = local->bg_active;
	kernel_seq_delete(seq[old_slot]);
	matte_deallocate_series(ss[old_slot], -1);

	if (local->bg_active == 0)
		room_199_parser2();

	int16 new_slot = local->bg_frame;
	switch (new_slot) {
	case 0:
		room_199_init1();
		ss[new_slot] = kernel_load_series("*rm199n1", 0);
		break;
	case 1:
		ss[new_slot] = kernel_load_series("*rm199z1", 0);
		break;
	case 2:
		ss[new_slot] = kernel_load_series("*rm199i1", 0);
		kernel_flip_hotspot(words_flowers, true);
		break;
	case 3:
		ss[new_slot] = kernel_load_series("*rm199v1", 0);
		kernel_flip_hotspot(words_fwt, true);
		break;
	}

	seq[new_slot] = kernel_seq_stamp(ss[new_slot], false, 1);
	kernel_seq_depth(seq[new_slot], 1);
	local->bg_active = local->bg_frame;
	digi_play_build_ii('_', 1, 2);
}

static void room_199_parser() {
	if (player_parse(words_fwt, words_needle, 0)) { digi_play_build(521, 'e', 6, 2);  goto handled; }
	if (player_parse(words_fwt, words_wood, 0)) { digi_play_build(521, 'e', 10, 2); goto handled; }
	if (player_parse(words_fwt, words_leaves, 0)) { digi_play_build(521, 'e', 3, 2);  goto handled; }
	if (player_parse(words_fwt, words_web, 0)) { digi_play_build(521, 'e', 8, 2);  goto handled; }
	if (player_parse(words_fwt, words_vine_weed, 0)) { digi_play_build(521, 'e', 9, 2);  goto handled; }
	if (player_parse(words_fwt, words_twine, 0)) { digi_play_build(521, 'e', 20, 2); goto handled; }
	if (player_parse(words_fwt, words_feather, 0))  { digi_play_build(521, 'e', 5, 2);  goto handled; }
	if (player_parse(words_fwt, words_reeds, 0)) { digi_play_build(521, 'e', 2, 2);  goto handled; }
	if (player_parse(words_fwt, words_lily_pad, 0)) { digi_play_build(521, 'e', 7, 2);  goto handled; }
	if (player_parse(words_fwt, words_gears, 0)) { digi_play_build(521, 'e', 4, 2);  goto handled; }
	if (player_parse(words_fwt, words_sticks, 0)) { digi_play_build(521, 'e', 21, 2); goto handled; }

	if (player_parse(words_flowers, words_comfrey, 0))  { digi_play_build_ii('e', 9, 2);  goto handled; }
	if (player_parse(words_flowers, words_primrose, 0)) { digi_play_build_ii('e', 5, 2);  goto handled; }
	if (player_parse(words_flowers, words_snapdragon, 0)) { digi_play_build_ii('e', 3, 2);  goto handled; }
	if (player_parse(words_flowers, words_sunflower, 0)) { digi_play_build_ii('e', 6, 2);  goto handled; }
	if (player_parse(words_flowers, words_thistle, 0)) { digi_play_build_ii('e', 8, 2);  goto handled; }
	if (player_parse(words_flowers, words_eyebright, 0))  { digi_play_build_ii('e', 11, 2); goto handled; }
	if (player_parse(words_flowers, words_mint, 0)) { digi_play_build_ii('e', 1, 2);  goto handled; }
	if (player_parse(words_flowers, words_foxglove, 0)) { digi_play_build_ii('e', 4, 2);  goto handled; }
	if (player_parse(words_flowers, words_dandelion, 0))  { digi_play_build_ii('e', 7, 2);  goto handled; }
	if (player_parse(words_flowers, words_lungwort, 0)) { digi_play_build_ii('e', 12, 2); goto handled; }
	if (player_parse(words_flowers, words_chicory, 0))  { digi_play_build_ii('e', 2, 2);  goto handled; }

	if (player_parse(words_exit_journal, 0)) {
		leave_journal();
		goto handled;
	}

	if (player_parse(words_up_page, 0)) {
		if (local->bg_active != 3) {
			local->bg_active = local->bg_frame;
			local->bg_frame++;
		}
		if (local->bg_frame != local->bg_active)
			room_199_parser1();
		goto handled;
	}

	if (player_parse(words_down_page, 0)) {
		if (local->bg_active != 0) {
			local->bg_active = local->bg_frame;
			local->bg_frame--;
		}
		if (local->bg_frame != local->bg_active)
			room_199_parser1();
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_199_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.needed_item_num);
	s.syncAsSint16LE(scratch.val4);
	s.syncAsSint16LE(scratch.bg_active);
	s.syncAsSint16LE(scratch.bg_frame);
	s.syncAsSint16LE(scratch.anim_flag);
	s.syncAsSint16LE(scratch.anim_frame);
}

void room_199_preload() {
	room_init_code_pointer = room_199_init;
	room_parser_code_pointer = room_199_parser;
	room_daemon_code_pointer = room_199_daemon;

	global[g016] = -1;
	player.walker_visible = false;
	global_section_walker();
	global_section_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADS
