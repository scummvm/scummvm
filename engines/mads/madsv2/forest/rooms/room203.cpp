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

#include "mads/madsv2/core/digi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/rooms/section2.h"
#include "mads/madsv2/forest/rooms/room203.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];                /* 0x00 — sprite series handles  */
	int16 sequence[10];              /* 0x14 — sequence handles       */
	int16 animation[10];             /* 0x28 — animation handles      */
	AnimationInfo animation_info[10];/* 0x3C */
	int16 _8c;                       /* 0x8C */
	int16 _8e;                       /* 0x8E */
	int16 _90;                       /* 0x90 */
	int16 _92;                       /* 0x92 */
	int16 _94;                       /* 0x94 */
	int16 _96;                       /* 0x96 */
	int16 _98;                       /* 0x98 */
	int16 _9a;                       /* 0x9A */
	int16 _9c;                       /* 0x9C */
	int16 _9e;                       /* 0x9E — animation handle */
	int16 _a0;                       /* 0xA0 — animation handle */
	int16 _a2;                       /* 0xA2 — talk animation handle */
	int16 _a4;                       /* 0xA4 */
	int16 _a6;                       /* 0xA6 */
	int16 _a8;                       /* 0xA8 */
	int16 _paa;                      /* 0xAA */
	int16 _ac;                       /* 0xAC */
	int16 _ae;                       /* 0xAE */
	int16 _b0;                       /* 0xB0 */
	int16 _b2;                       /* 0xB2 */
	int16 _b4;                       /* 0xB4 */
};

static Scratch scratch;

#define local  (&scratch)
#define ss     local->sprite
#define seq    local->sequence
#define aa     local->animation
#define aainfo scratch.animation_info


static void room_203_init1() {
	global[g131] = 0;
	global[g141] = 0;

	if (global[g046] != 0) {
		object_set_quality(5, 0, -1);
		object_set_quality(8, 0, -1);
		scratch._a0 = kernel_run_animation_disp('r', 1, 0);
		kernel_position_anim(scratch._a0, 132, 133, 88, 6);
		scratch._9e = kernel_run_animation_disp('e', 3, 0);
		kernel_position_anim(scratch._9e, 105, 137, 90, 5);
		player.x = 91;
		player.y = 124;
		player.facing = 3;
	} else {
		object_set_quality(5, 0, 0);
		object_set_quality(8, 0, 0);
		room_203_flag = 0;
		scratch._a0 = kernel_run_animation_disp('r', 9, 0);
		kernel_position_anim(scratch._a0, 110, 153, 98, 2);
		scratch._9e = kernel_run_animation_disp('e', 9, 0);
		kernel_position_anim(scratch._9e, 86, 148, 96, 3);
		player.x = 70;
		player.y = 130;
		player.facing = 9;
	}

	if (previous_room == 201) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 101);
		aainfo[0]._val1 = -1;
		scratch._9a = 11;
		scratch._92 = 60;
		if (flags[8] == 1) {
			aa[4] = kernel_run_animation(kernel_name('w', 1), 105);
			aainfo[4]._val1 = -1;
			aainfo[4]._val2 = 0;
			scratch._9c = 60;
		}
		return;
	} else if (previous_room == 204) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 101);
		aainfo[0]._val1 = -1;
		scratch._9a = 11;
		scratch._92 = 99;
		return;
	} else if (previous_room == 210) {
		aa[0] = kernel_run_animation(kernel_name('y', 3), 101);
		aainfo[0]._val1 = -1;
		scratch._9a = 11;
		scratch._92 = 74;
		global_digi_play(11);
		scratch._a6 = 5;
		return;
	}

	if (previous_room == 199)
		restore_player();

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9e, 2);
	kernel_reset_animation(scratch._a0, 2);
	kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_NOW, 0);
	kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_NOW, 0);
	global[g133] = 0;
	global[g143] = 0;

	if (global[g046] == 0) {
		global_digi_play(15);
		scratch._a6 = 2;
		aa[4] = kernel_run_animation(kernel_name('w', 4), 105);
		aainfo[4]._val1 = -1;
		aainfo[4]._val2 = 0;
		scratch._9c = 63;
		player.walker_visible = -1;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = -1;
		kernel_flip_hotspot(168, -1);
		kernel_flip_hotspot(170, -1);
		kernel_flip_hotspot(169, -1);

		if (global[g049] != 0) {
			aa[3] = kernel_run_animation(kernel_name('a', 1), 104);
			scratch._98 = 30;
			aainfo[3]._val1 = -1;
			aainfo[3]._val2 = 25;
			kernel_reset_animation(aa[3], 25);
			global[g049] = -1;
			player.walker_visible = 0;
			player.commands_allowed = 0;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
		}

		kernel_flip_hotspot(119, global[g047] == 0 ? 1 : 0);
		kernel_flip_hotspot(27, global[g047]);
	} else {
		global_digi_play(11);
		scratch._a6 = 5;
	}
}

static void room_203_init() {
	scratch._b4 = 0;
	scratch._ac = 0;
	scratch._ae = 0;
	scratch._a8 = 0;

	if (previous_room != KERNEL_RESTORING_GAME && previous_room != 199 && global[g050] == 0)
		global[g046] = 0;

	scratch._a6 = 5;
	global[player_score] = -1;
	global[g009] = -1;
	kernel_flip_hotspot(65, false);
	kernel_flip_hotspot(66, false);
	kernel_flip_hotspot(168, false);
	kernel_flip_hotspot(170, false);
	kernel_flip_hotspot(169, false);

	ss[0] = kernel_load_series(kernel_name('p', 5), 0);
	ss[1] = kernel_load_series(kernel_name('p', 6), 0);

	if (global[g047] != 0) {
		kernel_flip_hotspot(119, false);
		kernel_flip_hotspot(27, -1);
		seq[1] = kernel_seq_stamp(ss[1], false, KERNEL_FIRST);
		kernel_seq_depth(seq[1], 15);
		kernel_seq_loc(seq[1], 128, 74);
		kernel_seq_scale(seq[1], 100);
	} else {
		kernel_flip_hotspot(119, -1);
		kernel_flip_hotspot(27, false);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 15);
		kernel_seq_loc(seq[0], 129, 79);
		kernel_seq_scale(seq[0], 100);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
		}

		for (int i = 0; i < 10; i++) {
			aainfo[i]._val1 = 0;
			aainfo[i]._val2 = 1;
			aainfo[i]._val3 = 0;
			aainfo[i]._val4 = 0;
		}

		if (previous_room != 199 && flags[8] != 3)
			flags[8]++;
	}

	room_203_init1();
}

static void room_203_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (local->_a4) {
		case 11:
			kernel_timing_trigger(40, 106);
			aainfo[0]._val2 = 9;
			kernel_reset_animation(aa[0], 9);
			break;

		case 12:
			kernel_timing_trigger(40, 106);
			aainfo[0]._val2 = 33;
			kernel_reset_animation(aa[0], 33);
			break;

		case 13:
			aainfo[0]._val2 = 28;
			kernel_reset_animation(aa[0], 28);
			break;

		case 14:
		case 15:
			aainfo[1]._val2 = 11;
			kernel_reset_animation(aa[1], 11);
			break;

		case 16:
			aainfo[0]._val2 = 12;
			kernel_reset_animation(aa[0], 12);
			break;

		case 17:
			aainfo[0]._val2 = 57;
			kernel_reset_animation(aa[0], 57);
			break;

		case 21:
			kernel_timing_trigger(40, 106);
			aainfo[0]._val2 = 17;
			kernel_reset_animation(aa[0], 17);
			break;

		case 22:
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._val1 = 0;

			aa[4] = kernel_run_animation(kernel_name('w', 3), 105);
			aainfo[4]._val1 = -1;
			aainfo[4]._val2 = 0;
			local->_9c = 62;
			aainfo[0]._val2 = 41;
			kernel_reset_animation(aa[0], 41);
			break;

		case 23:
			kernel_timing_trigger(40, 106);
			aainfo[0]._val2 = 23;
			kernel_reset_animation(aa[0], 23);
			break;

		case 24:
			kernel_timing_trigger(40, 106);
			aainfo[2]._val2 = 6;
			kernel_reset_animation(aa[2], 6);
			break;

		case 25:
			aainfo[2]._val2 = 20;
			kernel_reset_animation(aa[2], 20);
			break;

		case 26:
			aainfo[2]._val2 = 10;
			kernel_reset_animation(aa[2], 10);
			break;

		case 30:
			dont_frag_the_palette();
			kernel_abort_animation(local->_a2);
			player.walker_visible = true;
			local->_b2 = 0;
			global[g154] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g150] = -1;
			local->_90 = global[g046] ? 16 : 15;

		case 31:
			kernel_timing_trigger(40, 106);
			aainfo[0]._val2 = 25;
			kernel_reset_animation(aa[0], 25);
			break;

		case 32:
			aainfo[0]._val2 = 5;
			kernel_reset_animation(aa[0], 5);
			break;

		case 33:
			aainfo[3]._val2 = 3;
			kernel_reset_animation(aa[3], 3);
			break;

		case 34:
		case 35:
			aainfo[3]._val2 = 39;
			kernel_reset_animation(aa[3], 39);
			break;

		case 36:
			kernel_timing_trigger(40, 106);
			aainfo[3]._val2 = 60;
			kernel_reset_animation(aa[3], 60);
			break;

		case 38:
		case 39:
			aainfo[3]._val2 = 63;
			kernel_reset_animation(aa[3], 63);
			break;

		case 51:
			kernel_timing_trigger(40, 106);
			aainfo[0]._val2 = 24;
			kernel_reset_animation(aa[0], 24);
			break;

		case 52:
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = true;
			local->_a4 = 0;
			break;

		default:
			break;
		}
		break;

	case 24:
		switch (local->_8c) {
		case 50:
			aa[1] = kernel_run_animation(kernel_name('Z', 1), 103);
			aainfo[1]._val1 = -1;
			aainfo[1]._val2 = 0;
			local->_96 = 50;
			player.commands_allowed = false;
			player.walker_visible = false;
			kernel_reset_animation(local->_9e, 0);

			global[g131] = 0;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, local->_9e);
			kernel_synch(KERNEL_ANIM, 0, KERNEL_PLAYER, aa[1]);
			global[g133] = 1;
			break;

		case 51:
			aa[1] = kernel_run_animation(kernel_name('e', 4), 103);
			aainfo[1]._val1 = -1;
			aainfo[1]._val2 = 0;
			local->_96 = 52;
			player.commands_allowed = false;
			player.walker_visible = false;
			kernel_reset_animation(local->_9e, 0);

			global[g131] = 0;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, local->_9e);
			kernel_synch(KERNEL_ANIM, 0, KERNEL_PLAYER, aa[1]);
			global[g133] = 1;
			break;

		}
		break;

	case 25:
		player.commands_allowed = false;

		switch (local->_8e) {
		case 41:
			aa[2] = kernel_run_animation(kernel_name('R', 3), 102);
			aainfo[2]._val1 = -1;
			aainfo[2]._val2 = 0;
			local->_94 = 41;
			local->_a8 = 0;
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_a0);
			global[g143] = 1;
			break;

		case 42:
			aa[2] = kernel_run_animation(kernel_name('R', 1), 102);
			aainfo[2]._val1 = -1;
			aainfo[2]._val2 = 0;
			local->_94 = 42;
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_a0);
			global[g143] = 1;
			break;

		case 43:
			aa[2] = kernel_run_animation(kernel_name('R', 2), 102);
			aainfo[2]._val1 = -1;
			aainfo[2]._val2 = 0;
			local->_94 = 43;
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_a0);
			global[g143] = 1;
			break;

		default:
			break;
		}
		break;

	case 26:
		kernel_reset_animation(local->_9e, 0);
		kernel_synch(KERNEL_ANIM, aa[local->_9e], KERNEL_NOW, 0);
		global[g133] = 1;
		global[g131] = 0;

		if (local->_90 != 55) {
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[local->_a0], KERNEL_NOW, 0);
			global[g143] = 1;
			global[g141] = 0;
		}

		player.walker_visible = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = false;

		switch (local->_90) {
		case 14:
			aa[0] = kernel_run_animation(kernel_name('x', 4), 101);
			aainfo[0]._val1 = -1;
			aainfo[0]._val2 = 0;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			local->_9a = 14;
			break;

		case 15:
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._val1 = 0;
			aa[0] = kernel_run_animation(kernel_name('z', 2), 101);
			aainfo[0]._val1 = -1;
			aainfo[0]._val2 = 0;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			local->_9a = 15;
			break;

		case 16:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			aa[0] = kernel_run_animation(kernel_name('m', 1), 101);
			aainfo[0]._val1 = -1;
			aainfo[0]._val2 = 0;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			local->_9a = 16;
			kernel_seq_delete(seq[0]);
			break;

		case 55:
			global[g143] = 0;
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, aa[local->_a0], KERNEL_NOW, 0);
			aa[1] = kernel_run_animation(kernel_name('e', 1), 103);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			aainfo[1]._val1 = -1;
			aainfo[1]._val2 = 0;
			local->_a8 = 0;
			break;

		default:
			break;
		}
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[0]._val1 = 0;

		switch (local->_9a) {
		case 11:
			if (global[g046]) {
				global[g131] = -1;
				global[g141] = -1;
				kernel_reset_animation(local->_9e, 1);
				kernel_reset_animation(local->_a0, 1);
				player.walker_visible = true;
				global[g133] = 0;
				global[g143] = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
				kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
				player.commands_allowed = true;
			} else {
				if (local->_a6 != 1) {
					global_digi_play(10);
					local->_a6 = 1;
				}

				aa[0] = kernel_run_animation(kernel_name('x', 2), 101);
				aainfo[0]._val1 = -1;
				aainfo[0]._val2 = 0;
				local->_9a = 12;

				if (flags[8] == 1) {
					aa[4] = kernel_run_animation(kernel_name('w', 2), 105);
					aainfo[4]._val1 = -1;
					aainfo[4]._val2 = 0;
					local->_9c = 61;
				} else {
					aa[4] = kernel_run_animation(kernel_name('w', 3), 105);
					aainfo[4]._val1 = -1;
					aainfo[4]._val2 = 0;
					local->_9c = 62;
					aainfo[0]._val2 = 41;
					kernel_reset_animation(aa[0], 41);
				}
			}
			break;

		case 14:
			dont_frag_the_palette();
			kernel_abort_animation(local->_9e);
			dont_frag_the_palette();
			kernel_abort_animation(local->_a0);
			local->_a0 = kernel_run_animation_disp('r', 1, 0);
			kernel_position_anim(local->_a0, 132, 133, 88, 6);
			local->_9e = kernel_run_animation_disp('e', 3, 0);
			kernel_position_anim(local->_9e, 105, 137, 90, 5);
			close_journal(3);
			display_interface();
			global[g046] = -1;

			if (local->_ac)
				global[g050] = -1;

			kernel_flip_hotspot(168, false);
			kernel_flip_hotspot(170, false);
			kernel_flip_hotspot(169, false);

			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(local->_9e, 1);
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g133] = 0;
			global[g143] = 0;

			player.walker_visible = false;
			player_demand_facing(3);
			player_demand_location(124, 91);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = true;
			player.walker_visible = true;
			room_203_flag = true;
			break;

		case 15:
			new_room = 201;
			break;

		case 16:
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(local->_9e, 1);
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g133] = 0;
			global[g143] = 0;

			kernel_flip_hotspot(27, true);
			seq[1] = kernel_seq_stamp(ss[1], true, -1);
			kernel_seq_depth(seq[1], 15);
			kernel_seq_loc(seq[1], 128, 74);
			kernel_seq_scale(seq[1], 100);
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._val1 = 0;
		aainfo[2]._val2 = 0;

		switch (local->_8e) {
		case 41:
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._val1 = 0;
			aa[4] = kernel_run_animation(kernel_name('w', 5), 105);
			aainfo[4]._val1 = -1;
			aainfo[4]._val2 = 0;
			local->_9c = 64;
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g143] = 0;
			display_interface();
			break;

		case 42:
			global[g141] = -1;
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_timing_trigger(1, 26);
			local->_90 = 14;
			break;

		case 43:
			global[g141] = -1;
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g143] = 0;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		break;

	case 103:
		dont_frag_the_palette();
		kernel_abort_animation(aa[1]);
		aainfo[1]._val1 = 0;
		aainfo[1]._val2 = 0;

		switch (local->_96) {
		case 50:
			global[g040] = -1;
			new_room = 210;
			break;

		case 52:
			aa[1] = kernel_run_animation(kernel_name('e', 1), 103);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			aainfo[1]._val1 = -1;
			aainfo[1]._val2 = 0;
			local->_96 = 53;
			local->_a8 = 0;
			break;

		case 53:
			aa[1] = kernel_run_animation(kernel_name('e', 2), 103);
			aainfo[1]._val1 = -1;
			aainfo[1]._val2 = 0;
			aainfo[1]._val4 = 0;
			aainfo[4]._val2 = 17;
			kernel_reset_animation(aa[4], 0);
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
			local->_96 = local->_ac ? 55 : 54;
			break;

		case 56:
			if (local->_ac) {
				global[g131] = -1;
				kernel_reset_animation(local->_9e, 1);
				kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
				global[g133] = 0;
			} else {
				aa[1] = kernel_run_animation(kernel_name('e', 4), 103);
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
				aainfo[1]._val1 = -1;
				aainfo[1]._val2 = 0;
				local->_96 = 57;
			}
			break;

		case 57:
			global[g131] = -1;
			kernel_reset_animation(local->_9e, 1);
			kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_timing_trigger(1, 26);
			local->_90 = 14;
			break;

		default:
			local->_96 = 0;
			break;
		}
		break;

	case 104:
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._val1 = 0;

		if (local->_98 == 33) {
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g150] = -1;
			local->_90 = 14;
			local->_98 = 0;
			local->_94 = 0;
			local->_96 = 0;
		}
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(aa[4]);
		aainfo[4]._val1 = 0;

		if (local->_9c == 64) {
			if (local->_94 == 41 && local->_ac == 0) {
				global[g145] = -1;
				local->_8e = 42;
			}

			if (local->_96 == 54) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[5]);
				aainfo[5]._val1 = 0;
				aa[1] = kernel_run_animation(kernel_name('e', 3), 103);
				aainfo[1]._val1 = -1;
				aainfo[1]._val2 = 0;
				local->_96 = 56;
			}

			object_set_quality(5, -1, -1);
			object_set_quality(8, -1, -1);

			if (local->_ac) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[5]);
				aainfo[5]._val1 = 0;
				aa[1] = kernel_run_animation(kernel_name('e', 3), 103);
				aainfo[1]._val1 = -1;
				aainfo[1]._val2 = 0;
				local->_96 = 56;
			}
			break;
		}
		break;

	case 106:
		switch (local->_a4) {
		case 23:
			digi_play_build(203, 'W', 1, 1);
			local->_a4 = 51;
			aainfo[4]._val2 = 1;
			kernel_reset_animation(aa[4], 1);
			break;

		case 24:
			aainfo[2]._val2 = 10;
			kernel_reset_animation(aa[2], 10);
			break;

		case 31:
			aainfo[0]._val2 = 27;
			kernel_reset_animation(aa[0], 27);
			break;

		case 36:
			aainfo[3]._val2 = 57;
			kernel_reset_animation(aa[3], 57);
			digi_play_build(203, 'b', 8, 1);
			local->_a4 = 38;
			break;

		case 34:
			aainfo[0]._val2 = 11;
			kernel_reset_animation(aa[0], 11);
			break;

		case 35:
			aainfo[0]._val2 = 35;
			kernel_reset_animation(aa[0], 35);
			break;

		case 44:
			aainfo[0]._val2 = 19;
			kernel_reset_animation(aa[0], 19);
			break;

		default:
			break;
		}
		break;

	case 107:
		player.commands_allowed = false;
		break;

	default:
		break;
	}
}

static void room_203_pre_parser() {
	if (global[g046] == 0) {
		player.need_to_walk = 0;
		return;
	}
	if (player_parse(13, 21, 0))
		player.walk_off_edge_to_room = 201;
	if (player_parse(13, 24, 0))
		player.walk_off_edge_to_room = 204;
}

static void room_203_parser() {
	if (player_parse(13, 27, 0)) {
		if (global[g046] != 0) {
			global[g135] = -1;
			scratch._8c = 50;
		} else {
			player.commands_allowed = 0;
			global[g150] = -1;
			scratch._90 = 15;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(13, 21, 0)) {
		if (global[g046] == 0) {
			player.commands_allowed = 0;
			global[g150] = -1;
			scratch._90 = 15;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(13, 24, 0) && global[g046] == 0) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._90 = 15;
	}

	if (global[walker_converse_state] != 0) {
		if (global[g046] == 0) {
			player.commands_allowed = 0;
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 107);
			scratch._a8 = 0;
			if (global[g049] != 0) {
				inter_spin_object(8);
				aainfo[3]._val2 = 28;
				kernel_reset_animation(aa[3], 28);
				global[g049] = 0;
			} else {
				global[g135] = -1;
				scratch._8c = 51;
			}
		} else {
			player.commands_allowed = 0;
			digi_play_build_ii('c', 1, 1);
			scratch._a4 = 52;
		}
		player.command_ready = false;
		return;
	}

	if (global[player_selected_object] >= 0) {
		if (global[player_selected_object] == 5) {
			if (global[g046] != 0) {
				// TODO: display_interface()
			} else {
				player.commands_allowed = 0;
				kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
				kernel_timing_trigger(1, 107);
				if (global[g049] != 0) {
					aainfo[3]._val2 = 41;
					kernel_reset_animation(aa[3], 41);
					global[g049] = 0;
					scratch._ae = -1;
				} else {
					global[g145] = -1;
					scratch._8e = 41;
					scratch._a8 = 0;
				}
			}
		} else if (global[player_selected_object] == 8) {
			if (global[g046] != 0) {
				// TODO: display_interface()
			} else {
				player.commands_allowed = 0;
				kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
				kernel_timing_trigger(1, 107);
				if (global[g049] != 0) {
					scratch._ac = 0;
					aainfo[3]._val2 = 28;
					kernel_reset_animation(aa[3], 28);
					global[g049] = 0;
				} else {
					kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
					aa[3] = kernel_run_animation(kernel_name('a', 2), 104);
					aainfo[3]._val1 = -1;
					aainfo[3]._val2 = 0;
					scratch._98 = 31;
					player.walker_visible = 0;
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				}
			}
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 119, 0)) {
		player.commands_allowed = 0;
		if (global[g046] == 0) {
			global[g150] = -1;
			scratch._90 = 15;
		} else {
			global[g154] = 2;
			player.walker_visible = 0;
			scratch._a2 = kernel_run_animation_talk('b', 7, 0);
			kernel_position_anim(scratch._a2, player.x, player.y, player.scale, player.depth);
			scratch._b4 = -1;
			kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_PLAYER, 0);
			digi_play_build_ii('b', 1, 1);
			scratch._a4 = 30;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 168, 0)) {
		if (player_has(5) && player_has(8)) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			aa[3] = kernel_run_animation(kernel_name('a', 1), 104);
			scratch._98 = 30;
			aainfo[3]._val1 = -1;
			global[g049] = -1;
			aainfo[3]._val2 = 0;
			player.walker_visible = 0;
			player.commands_allowed = 0;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
		} else if (player_has(8)) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			aa[3] = kernel_run_animation(kernel_name('a', 2), 104);
			aainfo[3]._val1 = -1;
			aainfo[3]._val2 = 0;
			scratch._98 = 31;
			player.walker_visible = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = 0;
		} else {
			player.commands_allowed = 0;
			global[g150] = -1;
			scratch._90 = 15;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 169, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 51;
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 170, 0)) {
		player.commands_allowed = 0;
		global[g145] = -1;
		scratch._8e = 43;
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 65, 0)) {
		player.commands_allowed = 0;
		aainfo[3]._val2 = 28;
		kernel_reset_animation(aa[3], 28);
		global[g049] = 0;
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 66, 0)) {
		player.commands_allowed = 0;
		aainfo[3]._val2 = 33;
		kernel_reset_animation(aa[3], 33);
		global[g049] = 0;
		player.command_ready = false;
		return;
	}

	if (global[g049] == 0)
		return;

	if (player_parse(13, 106, 0)) {
		player.commands_allowed = 0;
		aainfo[3]._val2 = 41;
		kernel_reset_animation(aa[3], 41);
		global[g049] = 0;
		player.command_ready = false;
	}
}

void room_203_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
	s.syncAsSint16LE(scratch._8e);
	s.syncAsSint16LE(scratch._90);
	s.syncAsSint16LE(scratch._92);
	s.syncAsSint16LE(scratch._94);
	s.syncAsSint16LE(scratch._96);
	s.syncAsSint16LE(scratch._98);
	s.syncAsSint16LE(scratch._9a);
	s.syncAsSint16LE(scratch._9c);
	s.syncAsSint16LE(scratch._9e);
	s.syncAsSint16LE(scratch._a0);
	s.syncAsSint16LE(scratch._a2);
	s.syncAsSint16LE(scratch._a4);
	s.syncAsSint16LE(scratch._a6);
	s.syncAsSint16LE(scratch._a8);
	s.syncAsSint16LE(scratch._paa);
	s.syncAsSint16LE(scratch._ac);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._b0);
	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
}

void room_203_preload() {
	room_init_code_pointer       = room_203_init;
	room_pre_parser_code_pointer = room_203_pre_parser;
	room_parser_code_pointer     = room_203_parser;
	room_daemon_code_pointer     = room_203_daemon;
	section_2_walker();
	section_2_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
