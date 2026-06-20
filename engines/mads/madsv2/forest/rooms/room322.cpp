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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/rooms/room322.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];                  // 0x00
	int16 sequence[10];                // 0x14
	int16 animation[10];               // 0x28
	AnimationInfo animation_info[11];  // 0x3C
	int16 _94;
	int16 _96;
	int16 _98;
	int16 _9a;
	int16 _9c;
	int16 _9e;
	int16 _a0;
	int16 _a2;
	int16 _a4;
	int16 _a6;
	int16 _a8;
	int16 _aa;  // 0xAA
	int16 _ac;  // 0xAC
	int16 _ae;
	int16 _b0;
	int16 _b2;
	int16 _b4;  // 0xB4
	int16 _b6;  // 0xB6
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_322_init1() {
	global[g131] = 0;
	global[g141] = 0;

	if (previous_room != 308 && previous_room != 301) {
		scratch._a6 = kernel_run_animation_disp('r', 9, 0);
		kernel_position_anim(scratch._a6, 18, 130, 95, 3);
		scratch._a4 = kernel_run_animation_disp('e', 6, 0);
		kernel_position_anim(scratch._a4, 71, 81, 87, 7);
	} else {
		scratch._a4 = -1;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 106;
		player.y = 107;
		player.facing = 3;
	}

	scratch._b6 = 0;

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._a4, 2);
		kernel_reset_animation(scratch._a6, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	}

	if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._a4, 2);
		kernel_reset_animation(scratch._a6, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = -1;
		return;
	}

	if (previous_room == 210) {
		aa[0] = kernel_run_animation("*rm307y1", 100);
		aainfo[0]._frame = -1;
		scratch._9c = 85;
		return;
	}

	if (previous_room == 301) {
		aa[0] = kernel_run_animation("*rm307y2", 100);
		aainfo[0]._frame = -1;
		scratch._9c = 40;
		return;
	}

	if (previous_room == 308) {
		midi_stop();
		aa[0] = kernel_run_animation("*rm307y3", 100);
		aainfo[0]._frame = -1;
		scratch._9c = 109;
		scratch._b6 = -1;
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._a4, 2);
	kernel_reset_animation(scratch._a6, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = -1;
	player.walker_visible = -1;
}

static void room_322_anim2() {
	int16 result = -1;
	int16 seq_handle;

	int16 cur = kernel_anim[aa[2]].frame;
	if (cur == aainfo[2]._val3)
		return;

	aainfo[2]._val3 = cur;

	if (scratch._b6 == 100) {
		if (cur == 31) {
			digi_play_build(307, '_', 2, 2);
			scratch._b4 = -1;
		} else if (cur == 35) {
			digi_play_build(101, '_', 1, 2);
			scratch._b4 = -1;
		}
	} else if (scratch._b6 == 0) {
		if (cur == 54) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[2]);
			aainfo[2]._frame = 0;
			kernel_reset_animation(scratch._a6, 1);
			kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._a4, 1);
			kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_NOW, 0);
			global[g133] = 0;
			player.commands_allowed = -1;
		} else if (cur < 54) {
			if (cur == 21) {
				seq_handle = seq[1];
				kernel_seq_delete(seq_handle);
				kernel_synch(KERNEL_SERIES, seq_handle, KERNEL_ANIM, aa[2]);
				inter_move_object(14, 2);
			} else if (cur == 22) {
				seq_handle = seq[0];
				kernel_seq_delete(seq_handle);
				inter_move_object(4, 2);
				kernel_synch(KERNEL_SERIES, seq_handle, KERNEL_ANIM, aa[2]);
				kernel_flip_hotspot(113, 0);
			} else if (cur == 28) {
				digi_play_build(307, '_', 2, 2);
			} else if (cur == 31) {
				digi_play_build(101, '_', 1, 2);
			}
		}
	} else {
		if (cur == 48) {
			digi_play_build(307, '_', 2, 2);
			scratch._b4 = -1;
		} else if (cur == 53) {
			digi_play_build(101, '_', 1, 2);
			scratch._b4 = -1;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[2], result);
		aainfo[2]._val3 = result;
	}
}

static void room_322_anim3() {
	int16 cur = kernel_anim[aa[8]].frame;
	if (cur != aainfo[8]._val3) {
		aainfo[8]._val3 = cur;
		if (cur == 22 && scratch._a4 == -1)
			digi_play_build(101, '_', 2, 2);
	}

	if (global[player_hyperwalked] != -1)
		return;

	aainfo[8]._val3 = scratch._9c - 1;
	kernel_reset_animation(aa[8], aainfo[8]._val3);
}

static void room_322_anim4() {
	int16 result = -1;
	int16 var_2;

	int16 cur = kernel_anim[aa[9]].frame;
	if (cur == aainfo[9]._val3)
		return;

	aainfo[9]._val3 = cur;

	if (cur == 43) {
		var_2 = seq[1];
		kernel_seq_delete(var_2);
		kernel_synch(KERNEL_SERIES, var_2, KERNEL_ANIM, aa[9]);
	} else if (cur <= 43) {
		if (cur == 3) {
			digi_play_build(307, 'e', 5, 1);
			scratch._b4 = -1;
		} else if (cur == 20) {
			digi_play_build(307, 'r', 2, 1);
			scratch._b4 = -1;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[9], result);
		aainfo[9]._val3 = result;
	}
}

static void room_322_anim5() {
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur != aainfo[1]._val3)
		aainfo[1]._val3 = cur;

	if (global[player_hyperwalked] != -1)
		return;

	aainfo[1]._val3 = 65;
	kernel_reset_animation(aa[1], 65);
}

static void room_322_anim6() {
	int16 var_2 = -1;

	int16 cur = kernel_anim[aa[3]].frame;
	if (cur == aainfo[3]._val3)
		return;

	aainfo[3]._val3 = cur;

	if (cur == 57) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._frame = 0;
		kernel_reset_animation(scratch._a4, 1);
		kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = -1;
	} else if (cur == 29) {
		digi_play_build(305, 'e', 1, 1);
		scratch._b4 = 3;
	} else if (cur == 30 || cur == 31) {
		if (aainfo[3]._val4 == 1)
			var_2 = imath_random(29, 30);
	}

	if (var_2 >= 0) {
		kernel_reset_animation(aa[3], var_2);
		aainfo[3]._val3 = var_2;
	}
}

static void room_322_anim7() {
	int16 result = -1;

	int16 cur = kernel_anim[aa[4]].frame;
	if (cur == aainfo[4]._val3)
		return;

	aainfo[4]._val3 = cur;

	if (cur == 29) {
		digi_play_build(322, 'b', 1, 1);
		scratch._b4 = -1;
	}

	if (result >= 0) {
		kernel_reset_animation(aa[4], result);
		aainfo[4]._val3 = result;
	}
}

static void room_322_anim8() {
	int16 result = -1;

	int16 cur = kernel_anim[aainfo[0]._active].frame;
	if (cur == aainfo[10]._val3)
		return;

	aainfo[10]._val3 = cur;

	if (cur == 17) {
		digi_play_build(322, '_', 1, 2);
	} else if (cur == 27) {
		global[g009] = 0;
		global_midi_play(4);
	}

	if (result >= 0) {
		kernel_reset_animation(aainfo[0]._active, result);
		aainfo[10]._val3 = result;
	}
}

static void room_322_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur != aainfo[0]._val3)
		aainfo[0]._val3 = cur;

	if (global[player_hyperwalked] != -1)
		return;

	aainfo[0]._val3 = scratch._9c - 1;
	kernel_reset_animation(aa[0], aainfo[0]._val3);
}

static void room_322_init() {
	scratch._aa = 0;
	scratch._ac = -1;

	object_set_quality(4, 0, -1L);
	object_set_quality(14, 0, -1L);

	kernel_flip_hotspot(global[g075] ? 28 : 27, 0);

	if (object[4].location == 307) {
		ss[0] = kernel_load_series("*rm307p1", 0);
		seq[0] = kernel_seq_stamp(ss[0], false, -1);
		kernel_seq_depth(seq[0], 10);
		kernel_seq_loc(seq[0], 184, 149);
		kernel_seq_scale(seq[0], 98);
	} else {
		kernel_flip_hotspot(113, 0);
	}

	if (object[14].location == 307) {
		ss[1] = kernel_load_series("*rm307p2", 0);
		seq[1] = kernel_seq_stamp(ss[1], false, -1);
		kernel_seq_depth(seq[1], 9);
		kernel_seq_loc(seq[1], 146, 134);
		kernel_seq_scale(seq[1], 100);
	}

	ss[2] = kernel_load_series(kernel_name('u', 2), 0);
	seq[2] = kernel_seq_stamp(ss[2], false, -1);
	kernel_seq_depth(seq[2], 14);
	kernel_seq_loc(seq[2], 227, 77);
	kernel_seq_scale(seq[2], 90);

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
			scratch._b4 = 0;
		}

		for (int count = 0; count < 11; count++) {
			aainfo[count]._frame = 0;
			aainfo[count]._val3 = 1;
			aainfo[count]._val4 = 0;
			aainfo[count]._active = 0;
		}

		if (previous_room != 199 && flags[25] != 3)
			flags[25]++;
	}

	bool was_neg3 = (flags[25] == -3);
	room_322_init1();
	if (was_neg3)
		flags[25] = 0;
}

static void room_322_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state]) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = -1;
		} else if (scratch._b4 == 1) {
			global[g150] = -1;
			dont_frag_the_palette();
			kernel_abort_animation(scratch._ae);
			player.walker_visible = -1;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		} else if (scratch._b4 == 2) {
			kernel_timing_trigger(40, 114);
		} else if (scratch._b4 == 3) {
			aainfo[3]._val4 = 2;
			scratch._b4 = -1;
			scratch._aa = 0;
		} else if (scratch._b4 == 4) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._ae);
			scratch._b4 = -1;
			scratch._aa = 0;
			scratch._a8 = kernel_run_animation_point(1, 104);
			kernel_position_anim(scratch._a8, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, scratch._a8, KERNEL_NOW, 0);
		} else if (scratch._b4 == 41) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._ae);
			scratch._b4 = -1;
			scratch._aa = 0;
			scratch._a8 = kernel_run_animation_point(3, 105);
			kernel_position_anim(scratch._a8, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, scratch._a8, KERNEL_NOW, 0);
		}
		break;

	case 24:
		aa[3] = kernel_run_animation("*rm307e1", 0);
		aainfo[3]._frame = -1;
		aainfo[3]._val4 = 1;
		kernel_reset_animation(scratch._a4, 0);
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._a4);
		global[g133] = 1;
		break;

	case 26:
		if (scratch._ac == 1) {
			aa[1] = kernel_run_animation("*rm307z1", 118);
			aainfo[1]._frame = -1;
			scratch._a0 = 1;
			player.commands_allowed = 0;
			player.walker_visible = 0;
			kernel_reset_animation(scratch._a4, 0);
			kernel_reset_animation(scratch._a6, 0);
			kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_ANIM, aa[1]);
			kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_ANIM, aa[1]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._ac == 2) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._a4);
			dont_frag_the_palette();
			kernel_abort_animation(scratch._a6);
			scratch._a4 = -1;
			kernel_reset_animation(scratch._a6, 0);
			if (player_has(4)) {
				aa[8] = kernel_run_animation("*rm307r4", 111);
				aainfo[8]._frame = -1;
				scratch._9c = 32;
				kernel_synch(KERNEL_ANIM, aa[8], KERNEL_ANIM, scratch._a6);
			} else {
				aa[9] = kernel_run_animation("*rm307r3", 111);
				aainfo[9]._frame = -1;
				kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, scratch._a6);
			}
		} else if (scratch._ac == 3) {
			aa[7] = kernel_run_animation(kernel_name('z', 2), 116);
			player.commands_allowed = 0;
			player.walker_visible = 0;
			kernel_reset_animation(scratch._a4, 0);
			kernel_reset_animation(scratch._a6, 0);
			kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_ANIM, aa[7]);
			kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_ANIM, aa[7]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[7]);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._ac == 4) {
			aa[2] = kernel_run_animation("*rm307r2", 0);
			aainfo[2]._frame = -1;
			kernel_reset_animation(scratch._a6, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._a6);
			global[g143] = 1;
			kernel_reset_animation(scratch._a4, 0);
			kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_ANIM, scratch._a6);
			digi_play_build(307, 'e', 5, 1);
			scratch._b4 = -1;
		}
		break;

	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[0]._frame = 0;
		if (previous_room == 308) {
			aa[2] = kernel_run_animation("*rm307r6", 113);
			aainfo[2]._frame = -1;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
			scratch._b6 = 100;
		} else {
			aa[4] = kernel_run_animation(kernel_name('s', 1), 115);
			aainfo[4]._frame = -1;
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
		}
		break;

	case 104:
		dont_frag_the_palette();
		kernel_abort_animation(scratch._a8);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.walker_visible = -1;
		global[g135] = -1;
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(scratch._a8);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.walker_visible = -1;
		global[g150] = -1;
		scratch._ac = 4;
		break;

	case 111: {
		int16 seq_handle;
		if (player_has(14)) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[8]);
		} else {
			seq_handle = seq[0];
			kernel_seq_delete(seq_handle);
			dont_frag_the_palette();
			kernel_abort_animation(aa[9]);
			kernel_synch(KERNEL_SERIES, seq_handle, KERNEL_NOW, 0);
			aainfo[9]._frame = 0;
		}

		scratch._a4 = -2;
		aa[8] = kernel_run_animation("*rm307z3", 112);
		aainfo[8]._frame = -1;
		scratch._9c = 100;
		player.walker_visible = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);

		if (player_has(14)) {
			kernel_synch(KERNEL_ANIM, aa[8], KERNEL_NOW, 0);
		} else {
			kernel_synch(KERNEL_ANIM, aa[9], KERNEL_NOW, 0);
			inter_move_object(14, 2);
			inter_move_object(4, 2);
		}
		break;
	}

	case 112:
		dont_frag_the_palette();
		kernel_abort_animation(aa[8]);
		aainfo[8]._frame = 0;
		scratch._b4 = -1;
		kernel_seq_delete(seq[2]);
		aainfo[0]._active = kernel_run_animation(kernel_name('j', 1), 114);
		aainfo[10]._frame = -1;
		digi_play_build(322, 'u', 1, 1);
		break;

	case 113:
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._frame = 0;
		aa[4] = kernel_run_animation(kernel_name('s', 1), 115);
		aainfo[4]._frame = -1;
		kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
		break;

	case 114:
		new_room = 308;
		midi_stop();
		break;

	case 115:
		dont_frag_the_palette();
		kernel_abort_animation(aa[4]);
		aainfo[4]._frame = 0;
		if (previous_room == 210) {
			global[g141] = -1;
			global[g143] = 0;
			kernel_reset_animation(scratch._a6, 1);
			kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_NOW, 0);
			global[g131] = -1;
			global[g133] = 0;
		} else {
			scratch._a6 = kernel_run_animation_disp('r', 9, 0);
			global[g141] = -1;
			global[g143] = 0;
			kernel_position_anim(scratch._a6, 18, 130, 95, 3);
			kernel_reset_animation(scratch._a6, 1);
			kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_NOW, 0);
			scratch._a4 = kernel_run_animation_disp('e', 6, 0);
			global[g131] = -1;
			global[g133] = 0;
			kernel_position_anim(scratch._a4, 71, 81, 87, 7);
		}
		kernel_reset_animation(scratch._a4, 1);
		kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_NOW, 0);
		player.commands_allowed = -1;
		player.walker_visible = -1;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		break;

	case 116:
		dont_frag_the_palette();
		kernel_abort_animation(aa[7]);
		scratch._b4 = -1;
		kernel_seq_delete(seq[2]);
		aainfo[0]._active = kernel_run_animation(kernel_name('j', 1), 117);
		aainfo[10]._frame = -1;
		digi_play_build(322, 'u', 1, 1);
		break;

	case 117:
		new_room = 301;
		midi_stop();
		break;

	case 118:
		dont_frag_the_palette();
		kernel_abort_animation(aa[1]);
		aainfo[1]._frame = 0;
		scratch._b4 = -1;
		kernel_seq_delete(seq[2]);
		aainfo[0]._active = kernel_run_animation(kernel_name('j', 1), 119);
		aainfo[10]._frame = -1;
		digi_play_build(322, 'u', 1, 1);
		break;

	case 119:
		global[g061] = -1;
		new_room = 210;
		midi_stop();
		break;

	default:
		break;
	}

	if (scratch._a4 >= 0) {
		global_anim1(6, scratch._a4, global[g131], &global[g132]);
		global_anim2(9, scratch._a6, global[g141], &global[g142]);
	}

	if (aainfo[0]._frame)
		room_322_anim1();
	if (aainfo[2]._frame)
		room_322_anim2();
	if (aainfo[8]._frame)
		room_322_anim3();
	if (aainfo[9]._frame)
		room_322_anim4();
	if (aainfo[1]._frame)
		room_322_anim5();
	if (aainfo[3]._frame)
		room_322_anim6();
	if (aainfo[4]._frame)
		room_322_anim7();
	if (aainfo[10]._frame)
		room_322_anim8();

	if (!scratch._aa)
		return;

	kernel_random_frame(scratch._ae, &global[g151], global[g154]);
	if (scratch._b4 == 41 || scratch._b4 == 4)
		return;
	kernel_random_frame(scratch._b0, &global[g152], global[g155]);
	kernel_random_frame(scratch._b2, &global[g153], global[g156]);
}

static void room_322_pre_parser() {
	if (player_parse(34, 0))
		player_walk(21, 101, 4);

	if (player_parse(41, 0)) {
		player_walk(96, 120, 3);
		global[g017] = 0;
	}
}

static void room_322_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = 0;
		digi_play_build_ii('c', 1, 1);
		player.command_ready = 0;
		return;
	}

	if (player_parse(34, 0)) {
		global[g150] = -1;
		scratch._ac = 3;
		player.commands_allowed = 0;
		player.command_ready = 0;
		return;
	}

	if (player_parse(113, 0)) {
		global[g154] = 2;
		player.walker_visible = 0;
		player.commands_allowed = 0;
		scratch._ae = kernel_run_animation_talk('b', 2, 0);
		kernel_position_anim(scratch._ae, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._ae, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._b4 = 41;
		scratch._aa = -1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(28, 0)) {
		global[g154] = 2;
		player.walker_visible = 0;
		player.commands_allowed = 0;
		scratch._ae = kernel_run_animation_talk('b', 1, 0);
		kernel_position_anim(scratch._ae, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._ae, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._b4 = 4;
		scratch._aa = -1;
		kernel_flip_hotspot(28, 0);
		kernel_flip_hotspot(27, -1);
		global[g075] = -1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(27, 0)) {
		global[g150] = -1;
		scratch._ac = 1;
		player.commands_allowed = 0;
		player.command_ready = 0;
		return;
	}

	if (player_parse(41, 0)) {
		global[g150] = -1;
		player.commands_allowed = 0;
		scratch._ac = 2;
		player.command_ready = 0;
		return;
	}

	if (global[walker_converse_state]) {
		global[walker_converse_state] = 0;
		close_journal(3);
		player.command_ready = 0;
	}
}

void room_322_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
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
	s.syncAsSint16LE(scratch._aa);
	s.syncAsSint16LE(scratch._ac);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._b0);
	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
	s.syncAsSint16LE(scratch._b6);
}

void room_322_preload() {
	room_init_code_pointer = room_322_init;
	room_pre_parser_code_pointer = room_322_pre_parser;
	room_parser_code_pointer = room_322_parser;
	room_daemon_code_pointer = room_322_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
