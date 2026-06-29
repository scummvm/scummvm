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

#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/text.h"

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


static void room_321_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 6, 0);
	kernel_position_anim(scratch._9c, 83, 131, 92, 6);
	scratch._9a = kernel_run_animation_disp('e', 6, 0);
	kernel_position_anim(scratch._9a, 109, 139, 94, 5);

	aa[4] = kernel_run_animation(kernel_name('L', 5), 104);
	aainfo[4]._active = -1;
	aa[6] = kernel_run_animation(kernel_name('L', 4), 104);
	aainfo[6]._active = -1;

	if (global[g071] != 0) {
		aa[5] = kernel_run_animation(kernel_name('H', 1), 104);
		aainfo[5]._active = -1;
	}

	aa[7] = kernel_run_animation(kernel_name('L', 3), 104);
	aainfo[7]._active = -1;
	aa[8] = kernel_run_animation(kernel_name('L', 2), 104);
	aainfo[8]._active = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 138;
		player.y = 130;
		player.facing = 6;
	}

	if (previous_room == KERNEL_RESTORING_GAME || previous_room == 199) {
		restore_player();
	} else if (previous_room == 302) {
		if (scratch._aa != 0) {
			ss[5] = kernel_load_series(kernel_name('f', 2), 0);
			seq[5] = kernel_seq_stamp(ss[5], false, -1);
			kernel_seq_depth(seq[5], 3);
			kernel_seq_loc(seq[5], 274, 154);
			kernel_seq_scale(seq[5], 100);
		}
		aa[0] = kernel_run_animation("*RM303y1", 100);
		aainfo[0]._active = -1;
		scratch._92 = 51;
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

static void room_321_init() {
	scratch._a4 = 0;
	scratch._a6 = -1;
	scratch._aa = 0;

	if (player_has(reeds))
		kernel_flip_hotspot(words_reeds, false);

	if (global[g068] == 0) {
		ss[0] = kernel_load_series(kernel_name('p', 6), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, -1);
		kernel_seq_depth(seq[0], 14);
		kernel_seq_loc(seq[0], 278, 126);
		kernel_seq_scale(seq[0], 91);
	} else {
		kernel_flip_hotspot(words_pebbles, false);
	}

	if (global[g071] == 0) {
		ss[4] = kernel_load_series(kernel_name('p', 2), 0);
		seq[4] = kernel_seq_stamp(ss[4], false, -1);
		kernel_seq_depth(seq[4], 15);
		kernel_seq_loc(seq[4], 230, 105);
		kernel_seq_scale(seq[4], 83);
		kernel_flip_hotspot(words_nest, false);
		global_midi_play(13);
	} else {
		global_midi_play(3);
	}

	global[g009] = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int16 i = 0; i < 10; i++) {
			aainfo[i]._active = 0;
			aainfo[i]._frame = 1;
			aainfo[i]._val3 = 0;
			aainfo[i]._val4 = 0;
		}

		if (previous_room != 199 && flags[24] != 3)
			flags[24]++;
	}

	if (flags[24] == 1)
		scratch._aa = -1;
	room_321_init1();
}

static void room_321_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur != aainfo[0]._frame)
		aainfo[0]._frame = cur;

	if (global[player_hyperwalked] != -1)
		return;

	aainfo[0]._frame = scratch._92 - 1;
	kernel_reset_animation(aa[0], aainfo[0]._frame);
}

static void room_321_anim2() {
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur != aainfo[1]._frame)
		aainfo[1]._frame = cur;

	if (global[player_hyperwalked] == -1) {
		if (cur <= 23) {
			aainfo[1]._frame = 23;
			kernel_reset_animation(aa[1], 23);
		} else {
			aainfo[1]._frame = 44;
			kernel_reset_animation(aa[1], 44);
		}
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
	} else {
		if (cur == 45) {
			aainfo[1]._frame = 44;
			kernel_reset_animation(aa[1], 44);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		}
	}
}

static void room_321_anim3() {
	int16 var_2;
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur != aainfo[2]._frame) {
		aainfo[2]._frame = cur;

		if (scratch._a6 == 1) {
			if (cur == 15) {
				if (aainfo[2]._val3 == 13) {
					aainfo[2]._frame = 13;
					kernel_reset_animation(aa[2], 13);
				}
			} else if (cur < 15) {
				if (cur == 7) {
					aainfo[2]._val3 = 11;
					digi_play_build_ii('e', 2, 1);
					scratch._a8 = 7;
				} else if (cur == 12) {
					if (aainfo[2]._val3 == 11) {
						aainfo[2]._frame = 7;
						kernel_reset_animation(aa[2], 7);
					}
				}
			}
		} else if (scratch._a6 == 2) {
			if (cur == 36) {
				aainfo[2]._val3 = 11;
				digi_play_build_ii('e', 4, 1);
				scratch._a8 = 43;
			} else if (cur == 41) {
				if (aainfo[2]._val3 == 11) {
					aainfo[2]._frame = 36;
					kernel_reset_animation(aa[2], 36);
				}
			} else if (cur == 44) {
				if (aainfo[2]._val3 == 13) {
					aainfo[2]._frame = 42;
					kernel_reset_animation(aa[2], 42);
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	var_2 = scratch._a8;
	if (var_2 == 45) {
		aainfo[2]._val3 = 14;
		aainfo[2]._frame = 43;
		kernel_reset_animation(aa[2], 43);
		scratch._a8 = -1;
	} else if ((uint16)var_2 > 45) {
		return;
	} else if (var_2 == 9) {
		aainfo[2]._val3 = 14;
		aainfo[2]._frame = 13;
		kernel_reset_animation(aa[2], 13);
		scratch._a8 = -1;
	} else if (var_2 > 9) {
		if (var_2 == 43) {
			kernel_timing_trigger(30, 28);
			aainfo[2]._val3 = 13;
			scratch._a8 = 44;
		} else if (var_2 == 44) {
			aainfo[2]._val3 = 11;
			aainfo[2]._frame = 36;
			kernel_reset_animation(aa[2], 36);
			digi_play_build_ii('e', 10, 1);
			scratch._a8 = 45;
		}
	} else if (var_2 == 7) {
		kernel_timing_trigger(30, 28);
		aainfo[2]._val3 = 13;
		scratch._a8 = 8;
	} else if (var_2 == 8) {
		aainfo[2]._val3 = 11;
		aainfo[2]._frame = 7;
		kernel_reset_animation(aa[2], 7);
		digi_play_build_ii('e', 10, 1);
		scratch._a8 = 9;
	}
}

static void room_321_anim4() {
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur != aainfo[3]._frame) {
		aainfo[3]._frame = cur;

		if (scratch._a6 == 1) {
			if (cur == 48) {
				if (aainfo[3]._val3 == 13) {
					digi_play_build(101, '_', 1, 2);
					scratch._a8 = 48;
				}
			} else if (cur < 48) {
				if (cur == 9) {
					aainfo[3]._val3 = 11;
					digi_play_build(303, 'e', 2, 1);
					scratch._a8 = 9;
				} else if (cur == 15) {
					if (aainfo[3]._val3 == 11) {
						aainfo[3]._frame = 9;
						kernel_reset_animation(aa[3], 9);
					}
				} else if (cur == 35) {
					if (aainfo[3]._val3 == 13) {
						digi_play_build(303, '_', 1, 2);
						scratch._a8 = 35;
					}
				}
			}
		} else if (scratch._a6 == 2) {
			if (cur == 2) {
				kernel_seq_delete(seq[0]);
			} else if (cur == 4) {
				aainfo[3]._val3 = 11;
				digi_play_build(303, 'e', 700, 1);
				scratch._a8 = 40;
			} else if (cur == 9) {
				if (aainfo[3]._val3 == 11) {
					aainfo[3]._frame = 4;
					kernel_reset_animation(aa[3], 4);
				}
			} else if (cur == 53) {
				digi_play_build(101, '_', 1, 2);
				scratch._a8 = 530;
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		int16 a8 = scratch._a8;
		if (a8 == 9) {
			aainfo[3]._val3 = 13;
			aainfo[3]._frame = 16;
			kernel_reset_animation(aa[3], 16);
			scratch._a8 = -1;
			return;
		} else if (a8 == 40) {
			aainfo[3]._val3 = 13;
			aainfo[3]._frame = 10;
			kernel_reset_animation(aa[3], 10);
			scratch._a8 = -1;
			return;
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 28) {
		int16 a8 = scratch._a8;
		if (a8 == 35 || a8 == 48 || a8 == 530)
			scratch._a8 = -1;
	}
}

static void room_321_anim5() {
	int16 var_2 = -1;
	int16 cur = kernel_anim[aa[4]].frame;
	if (cur == aainfo[4]._frame)
		return;
	aainfo[4]._frame = cur;

	int16 idx = cur - 5;
	if (idx >= 0 && idx <= 13) {
		switch (idx) {
		case 0: case 7: case 13:
			var_2 = 1;
			break;
		case 2:
			digi_initial_volume(40);
			digi_play_build(308, '_', 1, 2);
			break;
		case 4:
			digi_play_build(321, '_', 503, 1);
			scratch._a8 = -1;
			break;
		case 9:
			digi_play_build(321, '_', 504, 2);
			break;
		default:
			break;
		}
	}

	if (var_2 >= 0) {
		aainfo[4]._frame = var_2;
		kernel_reset_animation(aa[4], var_2);
	}
}

static void room_321_anim7() {
	int16 var_2 = -1;
	int16 cur = kernel_anim[aa[6]].frame;
	if (cur == aainfo[6]._frame)
		return;
	aainfo[6]._frame = cur;

	if (cur == 9) {
		if (imath_random(0, 100) > 85)
			var_2 = 6;
		else
			var_2 = 0;
	}

	if (var_2 >= 0) {
		aainfo[6]._frame = var_2;
		kernel_reset_animation(aa[6], var_2);
	}
}

static void room_321_anim8() {
	int16 var_2 = -1;
	int16 cur = kernel_anim[aa[7]].frame;
	if (cur == aainfo[7]._frame)
		return;
	aainfo[7]._frame = cur;

	switch (cur) {
	case 1:
		var_2 = 0;
		break;
	case 6: case 18: case 30: case 40: case 53: case 63:
		digi_play_build(321, '_', 2, 2);
		break;
	case 77:
		digi_stop(2);
		var_2 = 0;
		break;
	default:
		break;
	}

	if (var_2 >= 0) {
		aainfo[7]._frame = var_2;
		kernel_reset_animation(aa[7], var_2);
	}
}

static void room_321_anim6() {
	int16 var_2 = -1;
	int16 cur = kernel_anim[aa[5]].frame;
	if (cur == aainfo[5]._frame)
		return;
	aainfo[5]._frame = cur;

	switch (cur) {
	case 1:
		var_2 = 0;
		break;
	case 2:
		digi_play_build(305, 'h', 4, 1);
		scratch._a8 = 501;
		break;
	case 12:
		var_2 = 10;
		break;
	case 17:
		var_2 = 16;
		break;
	default:
		break;
	}

	if (var_2 >= 0) {
		aainfo[5]._frame = var_2;
		kernel_reset_animation(aa[5], var_2);
	}
}

static void room_321_anim9() {
	int16 var_2 = -1;
	int16 cur = kernel_anim[aa[8]].frame;
	if (cur == aainfo[8]._frame)
		return;
	aainfo[8]._frame = cur;

	switch (cur) {
	case 1:
		var_2 = 0;
		break;
	case 3: case 15:
		digi_initial_volume(80);
		digi_play_build(308, '_', 1, 2);
		break;
	case 17:
		var_2 = 0;
		player.commands_allowed = true;
		break;
	default:
		break;
	}

	if (var_2 >= 0) {
		aainfo[8]._frame = var_2;
		kernel_reset_animation(aa[8], var_2);
	}
}

static void room_321_anim10() {
	int16 var_2 = -1;
	int16 cur = kernel_anim[aa[9]].frame;
	if (cur == aainfo[9]._frame)
		return;
	aainfo[9]._frame = cur;

	if (cur == 5) {
		digi_play_build(321, 'f', 1, 1);
		scratch._a8 = 500;
	} else if (cur == 10) {
		var_2 = 5;
	}

	if (var_2 >= 0) {
		aainfo[9]._frame = var_2;
		kernel_reset_animation(aa[9], var_2);
	}
}

static void room_321_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (scratch._a8) {
		case -666:
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
			break;

		case 300:
			dont_frag_the_palette();
			kernel_abort_animation(scratch._9e);
			scratch._a8 = -1;
			scratch._a4 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a6 = 2;
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 6;
			break;

		case 400:
			dont_frag_the_palette();
			kernel_abort_animation(scratch._9e);
			scratch._a8 = -1;
			scratch._a4 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a6 = 1;
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 6;
			break;

		case 500:
			scratch._a8 = -1;
			aainfo[9]._frame = 10;
			kernel_reset_animation(aa[9], 10);
			player.commands_allowed = true;
			break;

		case 501:
			scratch._a8 = -1;
			aainfo[5]._frame = 0;
			kernel_reset_animation(aa[5], 0);
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		break;

	case 24: {
		int16 t = scratch._8c - 3;
		if (t == 0) {
			aa[2] = kernel_run_animation(kernel_name('E', 1), 102);
			aainfo[2]._active = -1;
			scratch._96 = 3;
		} else if (t == 2) {
			if (scratch._a6 == 1) {
				aa[2] = kernel_run_animation(kernel_name('F', 1), 102);
			} else if (scratch._a6 == 2) {
				aa[2] = kernel_run_animation(kernel_name('F', 2), 102);
			} else {
				break;
			}
			aainfo[2]._active = -1;
			scratch._96 = 2;
		} else {
			break;
		}
		kernel_reset_animation(scratch._9a, 0);
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9a);
		global[g133] = 1;
		break;
	}

	case 25:
		if (scratch._8e - 2 != 0)
			break;
		aa[1] = kernel_run_animation(kernel_name('R', 1), 101);
		aainfo[1]._active = -1;
		scratch._94 = 1;
		kernel_reset_animation(scratch._9c, 0);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, scratch._9c);
		global[g143] = 1;
		break;

	case 26: {
		int16 t = scratch._90 - 1;
		if (t == 0) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			global[g133] = 1;
			global[g143] = 1;
		} else if (t == 5) {
			if (scratch._a6 == 1) {
				room_321_anim1();
				aa[3] = kernel_run_animation(kernel_name('P', 1), 103);
			} else if (scratch._a6 == 2) {
				room_321_anim1();
				aa[3] = kernel_run_animation(kernel_name('P', 2), 103);
			} else {
				break;
			}
			aainfo[3]._active = -1;
			scratch._98 = 3;
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
			global[g133] = 1;
			global[g143] = 1;
		}
		break;
	}

	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
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
		if (scratch._aa == 0) {
			player.commands_allowed = true;
			break;
		}
		kernel_seq_delete(seq[5]);
		matte_deallocate_series(ss[5], -1);
		aa[9] = kernel_run_animation(kernel_name('L', 1), 105);
		aainfo[9]._active = -1;
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(aa[scratch._94]);
		aainfo[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[scratch._96]);
		aainfo[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 103:
		if (scratch._a6 == 1) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[3]);
			aainfo[3]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			room_321_anim2();
			kernel_flip_hotspot(words_reeds, false);
			inter_move_object(6, PLAYER);
			global[player_score] = -1;
			player.commands_allowed = true;
		} else if (scratch._a6 == 2) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[3]);
			aainfo[3]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			room_321_anim2();
			kernel_flip_hotspot(words_pebbles, false);
			if (!player_has(pebbles))
				inter_move_object(pebbles, PLAYER);
			global[g068] = -1;
			global[player_score] = -1;
			player.commands_allowed = true;
		}
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(aa[9]);
		aainfo[9]._active = 0;
		player.commands_allowed = true;
		break;

	default:
		break;
	}


	global_anim1(6, scratch._9a, global[g131], &global[g132]);
	global_anim2(6, scratch._9c, global[g141], &global[g142]);
	if (scratch._a4 != 0)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);
	if (aainfo[0]._active != 0) room_321_anim1();
	if (aainfo[1]._active != 0) room_321_anim2();
	if (aainfo[2]._active != 0) room_321_anim3();
	if (aainfo[3]._active != 0) room_321_anim4();
	if (aainfo[4]._active != 0) room_321_anim5();
	if (aainfo[5]._active != 0) room_321_anim6();
	if (aainfo[6]._active != 0) room_321_anim7();
	if (aainfo[7]._active != 0) room_321_anim8();
	if (aainfo[8]._active != 0) room_321_anim9();
	if (aainfo[9]._active != 0) room_321_anim10();
}

static void room_321_pre_parser() {
	if (player_parse(words_walk_to, words_room_302, 0))
		player.walk_off_edge_to_room = 302;
}

static void room_321_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		scratch._a8 = -666;
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(words_look_at, words_chicory, 0)) {
		global[player_score] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a6 = 1;
		goto handled;
	}

	if (player_parse(words_look_at, words_foxglove, 0)) {
		global[player_score] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a6 = 2;
		goto handled;
	}

	if (player_parse(words_pick_up, words_reeds, 0)) {
		global[g154] = 2;
		global[player_score] = 0;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._9e = kernel_run_animation_talk('b', 1, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 3, 1);
		scratch._a8 = 400;
		scratch._a4 = -1;
		goto handled;
	}

	if (player_parse(words_pick_up, words_pebbles, 0)) {
		global[g154] = 2;
		global[player_score] = 0;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._9e = kernel_run_animation_talk('b', 3, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 3, 1);
		scratch._a8 = 300;
		scratch._a4 = -1;
		goto handled;
	}

	if (player_parse(words_click_on, words_turtle, 0)) {
		aainfo[8]._frame = 1;
		kernel_reset_animation(aa[8], 1);
		player.commands_allowed = false;
		goto handled;
	}

	if (player_parse(words_click_on, words_dragonfly, 0)) {
		aainfo[7]._frame = 5;
		kernel_reset_animation(aa[7], 5);
		goto handled;
	}

	if (player_parse(words_click_on, words_nest, 0)) {
		aainfo[5]._frame = 1;
		kernel_reset_animation(aa[5], 1);
		player.commands_allowed = false;
		goto handled;
	}

	if (player_parse(words_click_on, words_black_bird, 0)) {
		aainfo[4]._frame = 5;
		kernel_reset_animation(aa[4], 5);
		goto handled;
	}

	if (player_parse(words_click_on, words_blue_bird, 0)) {
		aainfo[4]._frame = 12;
		kernel_reset_animation(aa[4], 12);
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_321_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (auto &ai : scratch.animation_info) ai.synchronize(s);
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

void room_321_preload() {
	room_init_code_pointer       = room_321_init;
	room_pre_parser_code_pointer = room_321_pre_parser;
	room_parser_code_pointer     = room_321_parser;
	room_daemon_code_pointer     = room_321_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
