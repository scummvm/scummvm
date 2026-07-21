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

#include "mads/forest/mads/inventory.h"
#include "mads/forest/mads/sounds.h"
#include "mads/forest/mads/words.h"
#include "mads/forest/digi.h"
#include "mads/forest/extra.h"
#include "mads/forest/global.h"
#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/imath.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/player.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[10];
	AnimationInfo animation_info[10];
	int16 _8c;
	int16 _8e;
	int16 _90;
	int16 _92;
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
	int16 _aa;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_305_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 1, 0);
	extra_change_animation(scratch._9c, 210, 124, 100, 13);

	scratch._9a = kernel_run_animation_disp('e', 2, 0);
	extra_change_animation(scratch._9a, 118, 142, 100, 6);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 183;
		player.y = 109;
		player.facing = 2;
	}

	if (previous_room == 301) {
		aa[1] = kernel_run_animation(kernel_name('t', 1), 104);
		aainfo[1]._active = -1;
		global_midi_play(14);
		return;
	}

	if (previous_room == 306) {
		if (global[phineas_status] == 3) {
			aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
			aainfo[0]._active = -1;
			scratch._92 = 76;
		} else {
			player.x = 138;
			player.y = 122;
			player.facing = 9;
			aa[0] = kernel_run_animation(kernel_name('y', 4), 100);
		}
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.walker_visible = true;
	player.commands_allowed = true;
}

static void room_305_init2() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 1, 0);
	extra_change_animation(scratch._9c, 210, 124, 100, 13);

	scratch._9a = kernel_run_animation_disp('e', 2, 0);
	extra_change_animation(scratch._9a, 118, 142, 100, 6);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 183;
		player.y = 109;
		player.facing = 2;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	}

	if (previous_room == 199) {
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
	}

	if (previous_room == 210) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 52;
		return;
	}

	if (previous_room == 301) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 70;
		return;
	}

	if (previous_room == 306) {
		if (global[phineas_status] == 3) {
			aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
			aainfo[0]._active = -1;
			scratch._92 = 76;
			return;
		}
		player.x = 138;
		player.y = 122;
		player.facing = 9;
		aa[0] = kernel_run_animation(kernel_name('y', 4), 100);
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = true;
	player.walker_visible = true;
}

static void room_305_init() {
	scratch._a8 = 0;
	scratch._aa = -1;

	if (global[g074] != 0)
		kernel_flip_hotspot(words_room_210h, false);

	if (global[g069] == 0) {
		ss[0] = kernel_load_series(kernel_name('p', 1), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, -1);
		kernel_seq_depth(seq[0], 4);
		kernel_seq_loc(seq[0], 229, 146);
		kernel_seq_scale(seq[0], 100);
	} else {
		kernel_flip_hotspot(words_sticks, false);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int count = 0; count < 10; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame = 1;
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[19] != 3)
			flags[19]++;
	}

	if (global[phineas_status] < 2)
		kernel_timing_trigger(1, 109);

	switch (flags[19] - 1) {
	case 0:
		room_305_init1();
		break;
	case 1:
	default:
		room_305_init2();
		break;
	}

	global[g009] = -1;
	if (global[phineas_status] == 1)
		global_midi_play(8);
}

static void room_305_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		int16 f = scratch._92 - 1;
		aainfo[0]._frame = f;
		kernel_reset_animation(aa[0], f);
	}
}

static void room_305_anim2() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame) {
		int16 f = kernel_anim[aa[1]].frame;
		aainfo[1]._frame = f;

		if (f == 61) {
			global[g009] = 0;
			aainfo[1]._val3 = 10;
			digi_play_build(305, 'b', 1, 1);
			scratch._a4 = 1;
		} else if (f == 66 && aainfo[1]._val3 == 10) {
			aainfo[1]._frame = 62;
			kernel_reset_animation(aa[1], 62);
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a4 == 1) {
			global[g154] = 0;
			aainfo[1]._frame = 67;
			kernel_reset_animation(aa[1], 67);
			scratch._a4 = 2;
		}
	}
}

static void room_305_anim3() {
	int16 f = aainfo[2]._frame;
	if (kernel_anim[aa[2]].frame != f)
		f = aainfo[2]._frame = kernel_anim[aa[2]].frame;

	if (global[player_hyperwalked] == -1 && f <= 23) {
		aainfo[2]._frame = 23;
		kernel_reset_animation(aa[2], 23);
		kernel_synch(3, aa[2], 4, 0);
		return;
	}
	if (global[player_hyperwalked] == -1 || f == 45) {
		aainfo[2]._frame = 44;
		kernel_reset_animation(aa[2], 44);
		kernel_synch(3, aa[2], 4, 0);
	}
}

static void room_305_anim4() {
	int16 f = aainfo[3]._frame;
	if (kernel_anim[aa[3]].frame != f)
		f = aainfo[3]._frame = kernel_anim[aa[3]].frame;

	if (global[player_hyperwalked] == -1 && f <= 23) {
		aainfo[3]._frame = 23;
		kernel_reset_animation(aa[3], 23);
		kernel_synch(3, aa[3], 4, 0);
		return;
	}
	if (global[player_hyperwalked] == -1 || f == 45) {
		aainfo[3]._frame = 44;
		kernel_reset_animation(aa[3], 44);
		kernel_synch(3, aa[3], 4, 0);
	}
}

static void room_305_anim5() {
	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		int16 f = kernel_anim[aa[4]].frame;
		aainfo[4]._frame = f;

		if (scratch._aa == 1) {
			if (f == 37) {
				if (aainfo[4]._val3 == 11) {
					aainfo[4]._frame = 35;
					kernel_reset_animation(aa[4], 35);
				}
			} else if (f <= 37) {
				if (f == 29) {
					aainfo[4]._val3 = 12;
					digi_play_build_ii('e', 1, 1);
					scratch._a4 = 29;
				} else if (f == 34 && aainfo[4]._val3 == 12) {
					aainfo[4]._frame = 29;
					kernel_reset_animation(aa[4], 29);
				}
			}
		} else if (scratch._aa == 2) {
			if (f == 15) {
				if (aainfo[4]._val3 == 11) {
					aainfo[4]._frame = 12;
					kernel_reset_animation(aa[4], 12);
				}
			} else if (f <= 15) {
				if (f == 5) {
					aainfo[4]._val3 = 12;
					digi_play_build_ii('e', 3, 1);
					scratch._a4 = 500;
				} else if (f == 11 && aainfo[4]._val3 == 12) {
					aainfo[4]._frame = 5;
					kernel_reset_animation(aa[4], 5);
				}
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._a4) {
		case 29:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 11;
			scratch._a4 = 30;
			break;
		case 30:
			aainfo[4]._val3 = 12;
			aainfo[4]._frame = 29;
			kernel_reset_animation(aa[4], 29);
			digi_play_build_ii('e', 10, 1);
			scratch._a4 = 31;
			break;
		case 31:
			aainfo[4]._val3 = 13;
			aainfo[4]._frame = 35;
			kernel_reset_animation(aa[4], 35);
			scratch._a4 = -1;
			break;
		case 500:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 11;
			scratch._a4 = 501;
			break;
		case 501:
			aainfo[4]._val3 = 12;
			aainfo[4]._frame = 5;
			kernel_reset_animation(aa[4], 5);
			digi_play_build_ii('e', 10, 1);
			scratch._a4 = 502;
			break;
		case 502:
			aainfo[4]._val3 = 13;
			aainfo[4]._frame = 11;
			kernel_reset_animation(aa[4], 11);
			scratch._a4 = -1;
			break;
		default:
			break;
		}
	}
}

static void room_305_anim6() {
	if (kernel_anim[aa[5]].frame != aainfo[5]._frame) {
		int16 result = -1;
		aainfo[5]._frame = kernel_anim[aa[5]].frame;
		int16 f = aainfo[5]._frame;

		if (f >= 3 && f <= 76) {
			switch (f) {
			case 3:
				digi_play_build(305, 'b', 2, 1);
				aainfo[5]._val3 = 3;
				scratch._a4 = 10;
				break;
			case 4: case 5: case 6: case 7: case 8: case 9:
				if (aainfo[5]._val3 == 3)
					result = imath_random(3, 8);
				else
					result = 9;
				break;
			case 15:
				result = 19;
				break;
			case 24:
				aainfo[5]._val3 = 5;
				digi_play_build(305, 'h', 1, 1);
				scratch._a4 = 12;
				break;
			case 25: case 26: case 27: case 28: case 29: case 30:
			case 31: case 32: case 33: case 34: case 35:
			case 57: case 58: case 59: case 60: case 61: case 62: case 63:
			case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: {
				int16 v = aainfo[5]._val3;
				if (v == 11)
					result = imath_random(68, 75);
				else if (v < 11) {
					if (v == 0)
						result = 24;
					else if (v == 4)
						result = imath_random(56, 62);
					else if (v == 5)
						result = imath_random(25, 33);
					else if (v == 6)
						result = 34;
				}
				break;
			}
			default:
				break;
			}
		}

		if (result >= 0) {
			kernel_reset_animation(aa[5], result);
			aainfo[5]._frame = result;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 105) {
		switch (scratch._a4) {
		case 10:
			aainfo[5]._val3 = 0;
			break;
		case 12:
			aainfo[5]._val3 = 6;
			kernel_timing_trigger(90, 105);
			scratch._a4++;
			break;
		case 13:
			aainfo[5]._val3 = 5;
			digi_play_build(305, 'h', 2, 1);
			scratch._a4++;
			break;
		case 14:
			aainfo[5]._val3 = 0;
			kernel_timing_trigger(45, 105);
			scratch._a4++;
			global[g009] = -1;
			global_midi_play(6);
			break;
		case 15:
			if (global[g065] != 0) {
				aainfo[5]._val3 = 4;
				digi_play_build(305, 'b', 3, 1);
			} else {
				aainfo[5]._val3 = 11;
				digi_play_build(305, 'e', 3, 1);
			}
			scratch._a4++;
			break;
		case 16:
			aainfo[5]._val3 = 0;
			kernel_timing_trigger(45, 105);
			scratch._a4++;
			break;
		case 17:
			if (global[g065] != 0) {
				aainfo[5]._val3 = 11;
				digi_play_build(305, 'e', 4, 1);
			} else {
				aainfo[5]._val3 = 4;
				digi_play_build(305, 'b', 4, 1);
			}
			scratch._a4++;
			break;
		case 18:
			kernel_abort_animation(aa[5]);
			aainfo[5]._active = 0;
			aa[6] = kernel_run_animation(kernel_name('h', 3), 0);
			aainfo[6]._active = -1;
			kernel_timing_trigger(1, 109);
			break;
		default:
			break;
		}
	}
}

static void room_305_anim7() {
	if (kernel_anim[aa[6]].frame != aainfo[6]._frame) {
		int16 result = -1;
		aainfo[6]._frame = kernel_anim[aa[6]].frame;
		int16 f = aainfo[6]._frame;

		if (f == 25) {
			aainfo[6]._val3 = 7;
			digi_play_build(305, 'h', 3, 1);
			scratch._a4 = 30;
		} else if ((f >= 26 && f <= 34) || (f >= 39 && f <= 46)) {
			int16 v = aainfo[6]._val3;
			if (v == 10)
				result = imath_random(38, 44);
			else if (v < 10) {
				if (v == 0)
					result = 25;
				else if (v == 2)
					result = 45;
				else if (v == 7)
					result = imath_random(26, 33);
			}
		}

		if (result >= 0) {
			kernel_reset_animation(aa[6], result);
			aainfo[6]._frame = result;
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 105)
		return;

	switch (scratch._a4) {
	case 30:
		aainfo[6]._val3 = 0;
		kernel_timing_trigger(45, 105);
		scratch._a4++;
		break;
	case 31:
	case 33:
		aainfo[6]._val3 = 2;
		kernel_timing_trigger(45, 105);
		scratch._a4++;
		break;
	case 32:
		aainfo[6]._val3 = 10;
		digi_play_build(305, 'h', 4, 1);
		scratch._a4++;
		break;
	case 34:
		kernel_abort_animation(aa[6]);
		aainfo[6]._active = 0;
		aa[7] = kernel_run_animation(kernel_name('h', 4), 106);
		kernel_timing_trigger(1, 109);
		break;
	default:
		break;
	}
}

static void room_305_anim8() {
	if (kernel_anim[aa[7]].frame != aainfo[7]._frame) {
		int16 f = kernel_anim[aa[7]].frame;
		aainfo[7]._frame = f;
		int16 result = -1;

		switch (f) {
		case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
		case 10: case 11: case 12: case 13:
		case 18: case 19: case 20: case 21: case 22: case 23:
		case 28: case 29: case 30: case 31: case 32: case 33: case 34:
		case 39:
		case 42: case 43: case 44: case 45: case 46:
		case 52: case 53: case 54: case 55: case 56: case 57: case 58: case 59: case 60:
			switch (aainfo[7]._val3) {
			case 0:  result = 0; break;
			case 4:  result = imath_random(51, 59); break;
			case 5:  result = imath_random(1, 4); break;
			case 9:  result = imath_random(28, 33); break;
			case 11: result = imath_random(41, 45); break;
			case 12: result = imath_random(17, 22); break;
			case 13: result = 25; break;
			case 14: result = 36; aainfo[7]._val3 = 0; break;
			default: break;
			}
			break;
		case 27:
			if (scratch._a4 == 42)
				digi_play_build(305, 'h', 6, 1);
			else
				digi_play_build(305, 'h', 7, 1);
			digi_initial_volume(100);
			aainfo[7]._val3 = 9;
			break;
		default:
			break;
		}

		if (result >= 0) {
			aainfo[7]._frame = result;
			kernel_reset_animation(aa[7], result);
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 105) {
		switch (scratch._a4) {
		case 40:
		case 44:
		case 48:
		case 50:
			aainfo[7]._val3 = 0;
			kernel_timing_trigger(45, 105);
			scratch._a4++;
			break;
		case 41:
			if (global[g080] != 0) {
				aainfo[7]._val3 = 5;
				digi_play_build(305, 'h', 5, 1);
			} else {
				aainfo[7]._val3 = 13;
			}
			scratch._a4++;
			break;
		case 42:
		case 46:
			if (global[g080] != 0) {
				aainfo[7]._val3 = 0;
				kernel_timing_trigger(90, 105);
			} else {
				aainfo[7]._val3 = 14;
				kernel_timing_trigger(90, 105);
			}
			scratch._a4++;
			break;
		case 43:
			if (global[g080] != 0) {
				aainfo[7]._val3 = 12;
				digi_play_build(305, 'r', 1, 1);
			} else {
				aainfo[7]._val3 = 11;
				digi_play_build(305, 'e', 5, 1);
			}
			scratch._a4++;
			break;
		case 45:
			aainfo[7]._val3 = 13;
			scratch._a4++;
			break;
		case 47:
			aainfo[7]._val3 = 4;
			digi_play_build(305, 'b', 7, 1);
			scratch._a4++;
			break;
		case 49:
			aainfo[7]._val3 = 5;
			digi_play_build(305, 'h', 8, 1);
			scratch._a4++;
			break;
		case 51:
			kernel_abort_animation(aa[7]);
			aainfo[7]._active = 0;
			aa[7] = kernel_run_animation(kernel_name('h', 6), 107);
			kernel_timing_trigger(1, 109);
			break;
		default:
			break;
		}
	}
}

static void room_305_anim9() {
	if (kernel_anim[aa[8]].frame != aainfo[8]._frame) {
		int16 f = kernel_anim[aa[8]].frame;
		aainfo[8]._frame = f;

		if (scratch._aa == 1) {
			if (f == 28) {
				if (aainfo[8]._val3 == 11) {
					digi_play_build('e', '_', 1, 2);
					scratch._a4 = 28;
				}
			} else if (f <= 28) {
				if (f == 5) {
					aainfo[8]._val3 = 12;
					digi_play_build(305, 'e', 2, 1);
					scratch._a4 = 5;
					kernel_seq_delete(seq[0]);
				} else if (f == 10 && aainfo[8]._val3 == 12) {
					aainfo[8]._frame = 5;
					kernel_reset_animation(aa[8], 5);
				}
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a4 == 5) {
			aainfo[8]._val3 = 11;
			aainfo[8]._frame = 11;
			kernel_reset_animation(aa[8], 11);
			scratch._a4 = -1;
		}
	}
	if (kernel.trigger != 8 && kernel.trigger != 28)
		return;
	if (scratch._a4 == 28)
		scratch._a4 = -1;
}

static void room_305_anim10() {
	if (kernel_anim[aa[9]].frame != aainfo[9]._frame) {
		int16 var_2 = -1;
		aainfo[9]._frame = kernel_anim[aa[9]].frame;
		int16 f = aainfo[9]._frame;

		if (f == 79) {
			kernel_abort_animation(aa[9]);
			aainfo[9]._active = 0;
			player.commands_allowed = true;
			if (config_file.misc2 != 0)
				kernel_timing_trigger(1, 109);
			global[g131] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
		} else if (f == 42) {
			digi_play_build(305, 'e', 1, 1);
			scratch._a4 = 666;
		} else if (f == 43 || f == 44) {
			if (aainfo[9]._val3 == 15)
				var_2 = imath_random(42, 43);
		}

		if (var_2 >= 0) {
			kernel_reset_animation(aa[9], var_2);
			aainfo[9]._frame = var_2;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 105) {
		if (scratch._a4 == 666)
			aainfo[9]._val3 = 0;
	}
}

static void room_305_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
			if (config_file.misc2 != 0)
				kernel_timing_trigger(1, 109);
		} else {
			if (scratch._a4 == 300) {
				kernel_abort_animation(scratch._9e);
				scratch._a4 = -1;
				scratch._a8 = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				scratch._aa = 1;
				player.walker_visible = true;
				global[g150] = -1;
				scratch._90 = 6;
			} else if (scratch._a4 == 451) {
				kernel_abort_animation(scratch._9e);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				scratch._a4 = -1;
				scratch._a8 = 0;
				scratch._8c = 3;
				player.walker_visible = true;
				global[g135] = -1;
				player.commands_allowed = false;
			}
		}
		break;

	case 24:
		if (scratch._8c == 3) {
			if (global[g074] == 0) {
				global[g074] = -1;
				aa[9] = kernel_run_animation(kernel_name('e', 1), 0);
				aainfo[9]._active = -1;
				aainfo[9]._val3 = 15;
				kernel_flip_hotspot(words_room_210h, false);
			} else {
				aa[0] = kernel_run_animation(kernel_name('Z', 1), 102);
				aainfo[0]._active = -1;
				scratch._92 = 38;
			}
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9a);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[2]);
			global[g133] = 1;
			kernel_timing_trigger(1, 109);
		} else if (scratch._8c == 5) {
			if (scratch._aa == 1 || scratch._aa == 2) {
				aa[4] = kernel_run_animation(kernel_name('F', scratch._aa), 108);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
				kernel_timing_trigger(1, 109);
			}
		}
		break;

	case 25:
		if (scratch._8e == 2) {
			aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[3]._active = -1;
			scratch._94 = 3;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
			kernel_timing_trigger(1, 109);
		}
		break;

	case 26:
		if (scratch._90 == 1) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._90 == 6 && scratch._aa == 1) {
			aa[8] = kernel_run_animation(kernel_name('P', 1), 103);
			aainfo[8]._active = -1;
			scratch._98 = 8;
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[8]);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[8]);
			global[g133] = 1;
			global[g143] = 1;
			kernel_timing_trigger(1, 109);
		}
		break;

	case 27:
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		if (config_file.misc2 != 0)
			kernel_timing_trigger(1, 109);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (global[phineas_status] == 1) {
			aa[5] = kernel_run_animation(kernel_name('b', 1), 0);
			aainfo[5]._active = -1;
			kernel_timing_trigger(1, 109);
		} else {
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			player.walker_visible = true;
			global[g133] = 0;
			global[g143] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = true;
			if (config_file.misc2 != 0)
				kernel_timing_trigger(1, 109);
		}
		break;

	case 101: {
		int16 idx = scratch._94;
		kernel_abort_animation(aa[idx]);
		aainfo[idx]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		if (config_file.misc2 != 0)
			kernel_timing_trigger(1, 109);
		break;
	}

	case 102:
		global[tunnel_exit_4_opened] = -1;
		new_room = 210;
		break;

	case 103:
		if (scratch._aa == 1) {
			kernel_abort_animation(aa[8]);
			aainfo[8]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_flip_hotspot(words_sticks, false);

			if (!player_has(sticks))
				inter_move_object(sticks, PLAYER);

			global[g069] = -1;
			global[play_background_sounds] = -1;
			player.commands_allowed = true;

			if (config_file.misc2 != 0)
				kernel_timing_trigger(1, 109);
		}
		break;

	case 104:
		kernel_abort_animation(aa[1]);
		// unused variable set to 0 in original
		global[g141] = -1;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		scratch._a6 = kernel_run_animation_write(0);
		kernel_reset_animation(scratch._a6, 41);
		extra_change_animation(scratch._a6, 118, 142, 100, 6);
		global[walker_converse_now] = 1;
		global[g007] = 2;
		kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_NOW, 0);
		kernel_timing_trigger(1, 109);
		break;

	case 106:
		kernel_abort_animation(aa[7]);
		aa[7] = kernel_run_animation(kernel_name('h', 5), 0);
		aainfo[7]._active = -1;
		if (global[g080] != 0)
			digi_play_build(305, 'b', 5, 1);
		else
			digi_play_build(305, 'b', 6, 1);
		aainfo[7]._val3 = 4;
		scratch._a4 = 40;
		kernel_timing_trigger(1, 109);
		break;

	case 107:
		kernel_abort_animation(aa[7]);
		global[phineas_status] = 2;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		if (config_file.misc2 != 0)
			kernel_timing_trigger(1, 109);
		break;

	case 108:
		kernel_abort_animation(aa[4]);
		aainfo[4]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		global[play_background_sounds] = -1;
		player.commands_allowed = true;
		if (config_file.misc2 != 0)
			kernel_timing_trigger(1, 109);
		break;

	case 109:
		if (global[phineas_status] < 2) {
			digi_initial_volume(15);
			digi_play_build(305, '_', 1, 3);
			digi_val2 = -1;
		}
		break;

	default:
		break;
	}

	global_anim1(2, scratch._9a, global[g131], &global[g132]);
	global_anim2(1, scratch._9c, global[g141], &global[g142]);
	if (scratch._a8 != 0)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);
	if (aainfo[0]._active != 0)
		room_305_anim1();
	if (aainfo[3]._active != 0)
		room_305_anim4();
	if (aainfo[2]._active != 0)
		room_305_anim3();
	if (aainfo[4]._active != 0)
		room_305_anim5();
	if (aainfo[8]._active != 0)
		room_305_anim9();
	if (aainfo[1]._active != 0)
		room_305_anim2();
	if (aainfo[5]._active != 0)
		room_305_anim6();
	if (aainfo[6]._active != 0)
		room_305_anim7();
	if (aainfo[7]._active != 0)
		room_305_anim8();
	if (aainfo[9]._active != 0)
		room_305_anim10();
	if (global[walker_converse_now] != 0)
		global_anim3(scratch._a6, &global[g008]);
}

static void room_305_pre_parser() {
	if (player_parse(words_walk_to, words_room_301, 0))
		player.walk_off_edge_to_room = 301;

	if (player_parse(words_walk_to, words_room_306, 0))
		player.walk_off_edge_to_room = 306;
}

static void room_305_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (player_parse(words_room_210h, 0)) {
		global[g154] = 2;
		player.walker_visible = false;
		player.commands_allowed = false;
		scratch._9e = kernel_run_animation_talk('b', 9, 0);
		extra_change_animation(scratch._9e, player.x, player.y, player.scale, player.depth);
		scratch._a8 = -1;
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._a4 = 451;
		kernel.trigger_setup_mode = 1;
		kernel_timing_trigger(1, 109);
		goto handled;
	}

	if (player_parse(words_room_210, 0)) {
		scratch._8c = 3;
		global[g135] = -1;
		player.commands_allowed = false;
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(words_look_at, words_mint, 0)) {
		global[play_background_sounds] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._aa = 1;
		goto handled;
	}

	if (player_parse(words_look_at, words_snapdragon, 0)) {
		global[play_background_sounds] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._aa = 2;
		goto handled;
	}

	if (player_parse(words_pick_up, words_sticks, 0)) {
		global[play_background_sounds] = 0;
		digi_stop(3);
		global[g154] = 2;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._9e = kernel_run_animation_talk('b', 1, 0);
		extra_change_animation(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 3, 1);
		scratch._a4 = 300;
		scratch._a8 = -1;
		kernel.trigger_setup_mode = 1;
		kernel_timing_trigger(1, 109);
		goto handled;
	}

	if (player_parse(words_click_on, words_moss, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 3;
		goto handled;
	}


	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_305_synchronize(Common::Serializer &s) {
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
	s.syncAsSint16LE(scratch._aa);
}

void room_305_preload() {
	room_init_code_pointer = room_305_init;
	room_pre_parser_code_pointer = room_305_pre_parser;
	room_parser_code_pointer = room_305_parser;
	room_daemon_code_pointer = room_305_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
