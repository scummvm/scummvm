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

#include "mads/madsv2/forest/rooms/section3.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/text.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];            // 0x00–0x13
	int16 sequence[10];          // 0x14–0x27
	int16 animation[10];         // 0x28–0x3B
	AnimationInfo animation_info[10]; // 0x3C–0x8B
	int16 _pad8C[3];             // 0x8C–0x91
	int16 _92;                   // 0x92
	int16 _pad94;                // 0x94
	int16 _96;                   // 0x96
	int16 _pad98;                // 0x98
	int16 _9a;                   // 0x9A
	int16 _9c;                   // 0x9C
	int16 _9e;                   // 0x9E
	int16 _a0;                   // 0xA0
	int16 _a2;                   // 0xA2
	int16 _a4;                   // 0xA4
	int16 _a6;                   // 0xA6
	int16 _a8;                   // 0xA8
	int16 _aa;                   // 0xAA
	int16 _ac;                   // 0xAC
	int16 _ae;                   // 0xAE
	int16 _b0;                   // 0xB0
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_307_init1() {
	// No implementation
}

static void room_307_init2() {
	aa[6] = kernel_run_animation(kernel_name('u', 1), 0);
	kernel_translate_anim(aa[6], -75, 16, 0);
	aainfo[6]._active = -1;
	aainfo[6]._val3 = 0;

	if (player.been_here_before != 0 && previous_room != 308) {
		scratch._9c = kernel_run_animation_disp('r', 9, 0);
		kernel_reset_animation(scratch._9c, 2);
		kernel_position_anim(scratch._9c, 18, 130, 95, 3);
		scratch._9a = kernel_run_animation_disp('e', 6, 0);
		kernel_reset_animation(scratch._9a, 2);
		kernel_position_anim(scratch._9a, 71, 81, 87, 7);
		global[g131] = 0;
		global[g141] = 0;
		global[g133] = 1;
		global[g143] = 1;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 106;
		player.y = 107;
		player.facing = 3;
	}

	scratch._ac = 0;

	switch (previous_room) {
	case KERNEL_RESTORING_GAME:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	case 199:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = true;
		player.walker_visible = true;
		return;
	case 210:
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 85;
		return;
	case 301:
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 40;
		return;
	case 308:
		midi_stop();
		aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
		aainfo[0]._active = -1;
		scratch._92 = 109;
		scratch._ac = -1;
		return;
	default:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = true;
		player.walker_visible = true;
		return;
	}
}

static void room_307_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur != aainfo[0]._frame)
		aainfo[0]._frame = cur;
	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_307_anim2() {
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur != aainfo[1]._frame)
		aainfo[1]._frame = cur;
	if (global[player_hyperwalked] == -1) {
		aainfo[1]._frame = 65;
		kernel_reset_animation(aa[1], 65);
	}
}

static void room_307_anim3() {
	int16 var_4 = -1;
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur == aainfo[2]._frame)
		return;
	aainfo[2]._frame = cur;
	if (scratch._ac != 0) {
		if (cur == 31) {
			digi_play_build(307, '_', 2, 2);
			scratch._aa = -1;
		} else if (cur == 35) {
			digi_play_build(101, '_', 1, 2);
			scratch._aa = -1;
		}
	} else {
		switch (cur) {
		case 21: {
			int16 old = seq[1];
			kernel_seq_delete(seq[1]);
			kernel_synch(KERNEL_SERIES, old, KERNEL_ANIM, aa[2]);
			inter_move_object(stick, PLAYER);
			break;
		}
		case 22: {
			int16 old = seq[0];
			kernel_seq_delete(seq[0]);
			inter_move_object(lily_pad, PLAYER);
			kernel_synch(KERNEL_SERIES, old, KERNEL_ANIM, aa[2]);
			kernel_flip_hotspot(words_lily_pad, false);
			break;
		}
		case 28:
			digi_play_build(307, '_', 2, 2);
			break;
		case 31:
			digi_play_build(101, '_', 1, 2);
			break;
		case 54:
			dont_frag_the_palette();
			kernel_abort_animation(aa[2]);
			aainfo[2]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			player.commands_allowed = true;
			break;
		default:
			break;
		}
	}
	if (var_4 >= 0) {
		kernel_reset_animation(aa[2], var_4);
		aainfo[2]._frame = var_4;
	}
}

static void room_307_anim4() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur == aainfo[3]._frame)
		return;
	aainfo[3]._frame = cur;
	if (cur == 57) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
	} else if (cur < 57) {
		if (cur == 29) {
			digi_play_build(305, 'e', 1, 1);
			scratch._aa = 3;
		} else if (cur >= 30 && cur <= 31) {
			if (aainfo[3]._val3 == 1)
				result = imath_random(29, 30);
		}
	}
	if (result >= 0) {
		kernel_reset_animation(aa[3], result);
		aainfo[3]._frame = result;
	}
}

static void room_307_anim5() {
	if (scratch._aa == 0)
		return;
	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	switch (scratch._aa) {
	case 10:
		global[g155] = 0;
		// fall through
	case 30:
		kernel_timing_trigger(30, 28);
		scratch._aa++;
		break;
	case 11:
		aainfo[6]._val3 = 1;
		scratch._aa++;
		digi_play_build(307, 'U', 1, 1);
		kernel_timing_trigger(30, 109);
		kernel_timing_trigger(200, 114);
		break;
	case 12:
	case 28:
		aainfo[6]._val3 = 4;
		scratch._aa++;
		break;
	case 14:
	case 18:
	case 24:
		aainfo[6]._val3 = 0;
		kernel_timing_trigger(30, 28);
		scratch._aa++;
		break;
	case 15:
		global[g156] = 2;
		scratch._aa++;
		digi_play_build(307, 'E', 1, 1);
		break;
	case 16:
	case 22:
	case 26:
	case 32:
		global[g156] = 0;
		kernel_timing_trigger(30, 28);
		scratch._aa++;
		break;
	case 17:
		aainfo[6]._val3 = 1;
		scratch._aa++;
		digi_play_build(307, 'U', 3, 1);
		break;
	case 19:
		scratch._aa++;
		global[g156] = 2;
		digi_play_build(307, 'E', 2, 1);
		if (config_file.forest1 == 0)
			digi_play_build(307, '_', 1, 2);
		break;
	case 20:
		aainfo[6]._val3 = 4;
		global[g156] = 0;
		scratch._aa++;
		break;
	case 23:
		aainfo[6]._val3 = 1;
		scratch._aa++;
		digi_play_build(307, 'u', 5, 1);
		break;
	case 25:
		global[g156] = 2;
		scratch._aa++;
		digi_play_build(307, 'E', 3, 1);
		break;
	case 27:
		aainfo[6]._val3 = 1;
		scratch._aa++;
		digi_play_build(307, 'U', 7, 1);
		kernel_timing_trigger(200, 114);
		break;
	case 31:
		global[g156] = 2;
		scratch._aa++;
		digi_play_build(307, 'e', 4, 1);
		break;
	case 33:
		aainfo[6]._val3 = 1;
		scratch._aa++;
		digi_play_build(307, 'u', 9, 1);
		kernel_timing_trigger(300, 114);
		break;
	case 34:
		aainfo[6]._val3 = 3;
		dont_frag_the_palette();
		kernel_abort_animation(scratch._a4);
		dont_frag_the_palette();
		kernel_abort_animation(scratch._a8);
		dont_frag_the_palette();
		kernel_abort_animation(scratch._a6);
		aainfo[4]._active = 0;
		scratch._a0 = 0;
		scratch._aa = -1;
		player.walker_visible = true;
		global[g017] = -1;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		scratch._9c = kernel_run_animation_disp('r', 9, 0);
		kernel_position_anim(scratch._9c, 18, 130, 95, 3);
		global[g141] = -1;
		kernel_reset_animation(scratch._9c, 2);
		global[g143] = 0;
		scratch._ae = kernel_run_animation_write(0);
		kernel_reset_animation(scratch._ae, 1);
		kernel_position_anim(scratch._ae, 71, 81, 87, 7);
		global[walker_converse_now] = 1;
		global[g007] = 6;
		kernel_synch(KERNEL_ANIM, scratch._ae, KERNEL_NOW, 0);
		break;
	default:
		break;
	}
}

static void room_307_anim7() {
	int16 result = -1;
	int16 var_2;
	int16 cur = kernel_anim[aa[6]].frame;
	if (cur == aainfo[6]._frame)
		return;
	aainfo[6]._frame = cur;

	bool dispatch = (cur == 1) || (cur >= 3 && cur <= 12) || (cur == 15);
	if (!dispatch) {
		if (result >= 0) {
			kernel_reset_animation(aa[6], result);
			aainfo[6]._frame = result;
		}
		return;
	}

	switch (aainfo[6]._val3) {
	case 0:
	case 3:
	case 4:
		if (cur >= 8 && cur <= 12) {
			aainfo[6]._val4++;
			if (imath_random(1, 6) < aainfo[6]._val4) {
				digi_play_build(307, '_', 4, 2);
				result = 0;
				aainfo[6]._val4 = 0;
				if (scratch._b0 != 0) {
					player.commands_allowed = true;
					scratch._b0 = 0;
				}
				if (aainfo[6]._val3 == 4) {
					aainfo[6]._val3 = 0;
					scratch._aa++;
					kernel_timing_trigger(1, 28);
				}
			} else {
				do {
					result = imath_random(7, 11);
				} while (aainfo[6]._frame == result + 1);
			}
		} else {
			aainfo[6]._val4++;
			if (imath_random(10, 20) < aainfo[6]._val4) {
				aainfo[6]._val4 = 0;
				var_2 = imath_random(2, 4);
			} else {
				var_2 = 3;
			}
			if (aainfo[6]._val3 == 3) {
				var_2 = 1;
				aainfo[6]._val3 = 0;
				aainfo[6]._val4 = 0;
			}
			if (aainfo[6]._val3 == 4) {
				var_2 = 1;
				aainfo[6]._val4 = 0;
			}
			switch (var_2) {
			case 1:
				result = imath_random(7, 11);
				digi_play_build(307, '_', 1, 2);
				break;
			case 2:
				result = 1;
				break;
			default:
				result = 0;
				break;
			}
		}
		break;
	case 1:
		result = imath_random(3, 6);
		break;
	case 2:
		result = 14;
		break;
	default:
		break;
	}

	if (result >= 0) {
		kernel_reset_animation(aa[6], result);
		aainfo[6]._frame = result;
	}
}

static void room_307_anim9() {
	int16 cur = kernel_anim[aa[8]].frame;
	if (cur != aainfo[8]._frame) {
		aainfo[8]._frame = cur;
		if (cur == 22 && scratch._9a == -1)
			digi_play_build(101, '_', 2, 2);
	}
	if (global[player_hyperwalked] == -1) {
		aainfo[8]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[8], aainfo[8]._frame);
	}
}

static void room_307_anim10() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[9]].frame;
	if (cur == aainfo[9]._frame)
		return;
	aainfo[9]._frame = cur;
	switch (cur) {
	case 3:
		digi_play_build(307, 'e', 5, 1);
		scratch._aa = -1;
		break;
	case 20:
		digi_play_build(307, 'r', 2, 1);
		scratch._aa = -1;
		break;
	case 43: {
		int16 old = seq[1];
		kernel_seq_delete(seq[1]);
		kernel_synch(KERNEL_SERIES, old, KERNEL_ANIM, aa[9]);
		break;
	}
	default:
		break;
	}
	if (result >= 0) {
		kernel_reset_animation(aa[9], result);
		aainfo[9]._frame = result;
	}
}

static void room_307_init() {
	object_set_quality(lily_pad, 0, -1);
	object_set_quality(stick, 0, -1);

	scratch._9a = -1;
	scratch._9c = -1;
	scratch._ae = -1;
	global[perform_displacements] = 120;
	stop_speech_on_run_animation = false;
	scratch._b0 = 0;
	scratch._a0 = 0;
	global[g067] = -1;
	scratch._a2 = -1;

	kernel_flip_hotspot(global[g075] != 0 ? 28 : 27, 0);

	if (object_is_here(lily_pad)) {
		ss[0] = kernel_load_series(kernel_name('p', 1), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, -1);
		kernel_seq_depth(seq[0], 10);
		kernel_seq_loc(seq[0], 184, 149);
		kernel_seq_scale(seq[0], 98);
	} else {
		kernel_flip_hotspot(words_lily_pad, false);
	}

	if (object_is_here(stick)) {
		ss[1] = kernel_load_series(kernel_name('p', 2), 0);
		seq[1] = kernel_seq_stamp(ss[1], false, -1);
		kernel_seq_depth(seq[1], 9);
		kernel_seq_loc(seq[1], 146, 134);
		kernel_seq_scale(seq[1], 100);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
			scratch._aa = 0;
		}

		for (int16 i = 0; i < 10; i++) {
			aainfo[i]._active = 0;
			aainfo[i]._frame = 1;
			aainfo[i]._val3 = 0;
			aainfo[i]._val4 = 0;
		}

		if (previous_room != 199) {
			if (flags[21] != 3)
				flags[21]++;
		}
	}

	switch (flags[21]) {
	case -3:
		room_307_init1();
		flags[21] = 1;
		room_307_init2();
		break;
	default:
		room_307_init2();
		break;
	}
}

static void room_307_daemon() {
	switch (kernel.trigger) {
	case 7: {
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
		} else if (scratch._aa == 3) {
			aainfo[3]._val3 = 2;
			scratch._aa = -1;
			scratch._a0 = 0;
		} else if (scratch._aa == 4) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._a4);
			scratch._aa = -1;
			scratch._a0 = 0;
			scratch._9e = kernel_run_animation_point(1, 'h');
			kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_NOW, 0);
		} else if (scratch._aa == 40) {
			aainfo[6]._val3 = 0;
			scratch._aa = -1;
			scratch._a0 = 0;
			player.commands_allowed = true;
		} else if (scratch._aa == 41) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._a4);
			scratch._aa = -1;
			scratch._a0 = 0;
			scratch._9e = kernel_run_animation_point(3, 'i');
			kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_NOW, 0);
		}
		break;
	}

	case 24:
		aa[3] = kernel_run_animation(kernel_name('E', 1), 0);
		aainfo[3]._active = -1;
		aainfo[3]._val3 = 1;
		kernel_reset_animation(scratch._9a, 0);
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9a);
		global[g133] = 1;
		break;

	case 26:
		if (scratch._a2 == 1) {
			aa[1] = kernel_run_animation(kernel_name('Z', 1), 102);
			aainfo[1]._active = -1;
			scratch._96 = 1;
			player.commands_allowed = false;
			player.walker_visible = false;
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[1]);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[1]);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._a2 == 2) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._9a);
			dont_frag_the_palette();
			kernel_abort_animation(scratch._9c);
			scratch._9a = -1;
			scratch._9c = -1;
			if (player_has(lily_pad)) {
				aa[8] = kernel_run_animation(kernel_name('R', 4), 111);
				aainfo[8]._active = -1;
				scratch._92 = 32;
				kernel_synch(KERNEL_ANIM, aa[8], KERNEL_ANIM, scratch._9c);
			} else {
				aa[9] = kernel_run_animation(kernel_name('R', 3), 111);
				aainfo[9]._active = -1;
				kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, scratch._9c);
			}
		} else if (scratch._a2 == 3) {
			aa[2] = kernel_run_animation(kernel_name('R', 2), 0);
			aainfo[2]._active = -1;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, scratch._9c);
			digi_play_build(307, 'e', 5, 1);
			scratch._aa = -1;
		}
		break;

	case 27:
		dont_frag_the_palette();
		kernel_abort_animation(scratch._ae);
		scratch._ae = -1;
		scratch._9a = kernel_run_animation_disp('e', 6, 0);
		kernel_position_anim(scratch._9a, 71, 81, 87, 7);
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		player.walker_visible = true;
		aainfo[0]._active = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		if (flags[21] == 1) {
			global[g155] = 2;
			aainfo[4]._active = -1;
			scratch._a0 = -1;
			global[g017] = 0;
			global[g154] = 0;
			global[g156] = 0;
			scratch._a6 = kernel_run_animation_talk('r', 9, 0);
			kernel_position_anim(scratch._a6, 18, 130, 95, 3);
			scratch._a8 = kernel_run_animation_talk('e', 6, 0);
			kernel_position_anim(scratch._a8, 71, 81, 87, 7);

			// WORKAROUND: In the original when first entering room _9a & _9c == -1, which caused a
			// wobbly in kernel_synch. Hence the conditional check to set master_type to PLAYER
			kernel_synch(KERNEL_ANIM, scratch._a8, (scratch._9a == -1) ? KERNEL_PLAYER : KERNEL_ANIM, scratch._9a);
			kernel_synch(KERNEL_ANIM, scratch._a6, (scratch._9c == -1) ? KERNEL_PLAYER : KERNEL_ANIM, scratch._9c);

			kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_PLAYER, 0);
			digi_play_build(307, 'r', 1, 1);
			scratch._aa = 10;
		} else if (previous_room == 308) {
			aa[2] = kernel_run_animation(kernel_name('R', 6), 113);
			aainfo[2]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
		} else {
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g131] = -1;
			global[g141] = -1;
			global[g133] = 0;
			global[g143] = 0;
			player.commands_allowed = false;
			digi_play_build(307, 'u', 11, 1);
			scratch._aa = 40;
			aainfo[6]._val3 = 1;
			kernel_timing_trigger(60, 110);
		}
		break;

	case 102:
		global[tunnel_exit_5_opened] = -1;
		new_room = 210;
		break;

	case 104:
		dont_frag_the_palette();
		kernel_abort_animation(scratch._9e);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g135] = -1;
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(scratch._9e);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g150] = -1;
		scratch._a2 = 3;
		break;

	case 109:
		player.walker_visible = false;
		scratch._a4 = kernel_run_animation_talk('b', 9, 0);
		kernel_position_anim(scratch._a4, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_PLAYER, 0);
		break;

	case 110:
		player_demand_facing(9);
		break;

	case 111: {
		if (player_has(stick)) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[8]);
		} else {
			int16 old = seq[0];
			kernel_seq_delete(seq[0]);
			dont_frag_the_palette();
			kernel_abort_animation(aa[9]);
			kernel_synch(KERNEL_SERIES, old, KERNEL_NOW, 0);
			aainfo[9]._active = 0;
		}
		scratch._9a = -2;
		aa[8] = kernel_run_animation(kernel_name('Z', 3), 112);
		aainfo[8]._active = -1;
		scratch._92 = 100;
		player.walker_visible = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		if (player_has(stick)) {
			kernel_synch(KERNEL_ANIM, aa[8], KERNEL_PLAYER, 0);
		} else {
			kernel_synch(KERNEL_ANIM, aa[9], KERNEL_NOW, 0);
			inter_move_object(stick, PLAYER);
			inter_move_object(lily_pad, PLAYER);
		}
		break;
	}

	case 112:
		new_room = 308;
		break;

	case 113:
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._active = 0;
		scratch._9c = kernel_run_animation_disp('r', 9, 0);
		kernel_reset_animation(scratch._9c, 2);
		kernel_position_anim(scratch._9c, 18, 130, 95, 3);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		scratch._9a = kernel_run_animation_disp('e', 6, 0);
		kernel_reset_animation(scratch._9a, 2);
		kernel_position_anim(scratch._9a, 71, 81, 87, 7);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g131] = -1;
		global[g141] = -1;
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = false;
		scratch._ac = 0;
		digi_play_build(307, 'u', 11, 1);
		scratch._aa = 40;
		aainfo[6]._val3 = 1;
		kernel_timing_trigger(60, 110);
		break;

	case 114:
		digi_play_build(307, '_', 1, 2);
		break;

	}

	if (scratch._9a >= 0)
		global_anim1(6, scratch._9a, global[g131], &global[g132]);
	if (scratch._9c >= 0)
		global_anim2(9, scratch._9c, global[g141], &global[g142]);
	if (scratch._ae >= 0)
		global_anim3(scratch._ae, &global[g008]);

	if (aainfo[0]._active != 0)  room_307_anim1();
	if (aainfo[2]._active != 0)  room_307_anim3();
	if (aainfo[8]._active != 0)  room_307_anim9();
	if (aainfo[9]._active != 0)  room_307_anim10();
	if (aainfo[1]._active != 0)  room_307_anim2();
	if (aainfo[3]._active != 0)  room_307_anim4();
	if (aainfo[6]._active != 0)  room_307_anim7();
	if (aainfo[4]._active != 0)  room_307_anim5();

	if (scratch._a0 != 0) {
		kernel_random_frame(scratch._a4, &global[g151], global[g154]);
		if (scratch._aa != 41 && scratch._aa != 4) {
			kernel_random_frame(scratch._a6, &global[g152], global[g155]);
			kernel_random_frame(scratch._a8, &global[g153], global[g156]);
		}
	}
}

static void room_307_pre_parser() {
	if (player_parse(words_walk_to, words_room_301, 0))
		player.walk_off_edge_to_room = 301;

	if (player_parse(words_frog, 0)) {
		switch (kernel.trigger) {
		case 0:
			scratch._b0 = -1;
			player.commands_allowed = false;
			digi_play_build(307, '_', 1, 2);
			kernel_timing_trigger(120, 114);
			break;
		case 114:
			aainfo[6]._val3 = 3;
			break;
		}
	}

	if (player_parse(words_room_308, 0)) {
		player_walk(96, 120, 3);
		global[g017] = 0;
	}
}

static void room_307_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (player_parse(words_lily_pad, 0)) {
		global[g154] = 2;
		player.walker_visible = false;
		player.commands_allowed = false;
		scratch._a4 = kernel_run_animation_talk('b', 2, 0);
		kernel_position_anim(scratch._a4, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._aa = 41;
		scratch._a0 = -1;
		goto handled;
	}

	if (player_parse(words_room_210h, 0)) {
		global[g154] = 2;
		player.walker_visible = false;
		player.commands_allowed = false;
		scratch._a4 = kernel_run_animation_talk('b', 1, 0);
		kernel_position_anim(scratch._a4, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._aa = 4;
		scratch._a0 = -1;
		kernel_flip_hotspot(words_room_210h, false);
		kernel_flip_hotspot(words_room_210, true);
		global[g075] = -1;
		goto handled;
	}

	if (player_parse(words_room_210, 0)) {
		global[g150] = -1;
		scratch._a2 = 1;
		goto handled;
	}

	if (player_parse(words_room_308, 0)) {
		global[g150] = -1;
		player.commands_allowed = false;
		scratch._a2 = 2;
		goto handled;
	}

	if (global[walker_converse_state] != 0) {
		global[walker_converse_state] = 0;
		close_interface(CANDLE_FLY);
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_307_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (auto &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._92);
	s.syncAsSint16LE(scratch._96);
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
}

void room_307_preload() {
	room_init_code_pointer = room_307_init;
	room_pre_parser_code_pointer = room_307_pre_parser;
	room_parser_code_pointer = room_307_parser;
	room_daemon_code_pointer = room_307_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
