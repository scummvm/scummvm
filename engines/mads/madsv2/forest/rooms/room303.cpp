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
	int16 _8c, _8e, _90, _92, _94, _96, _98, _9a, _9c, _9e, _a0, _a2;
	int16 _a4;  // scratch+0xA4
	int16 _a6;  // scratch+0xA6
	int16 _a8;  // scratch+0xA8
	int16 _aa;  // scratch+0xAA
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_303_init1();
static void room_303_init2();

static void room_303_anim1() {
	if (kernel_anim[aa[0]].frame != scratch.animation_info[0]._frame)
		scratch.animation_info[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		scratch.animation_info[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], scratch.animation_info[0]._frame);
	}
}

static void room_303_anim2() {
	int16 ax = scratch.animation_info[1]._frame;
	if (kernel_anim[aa[1]].frame != ax) {
		ax = kernel_anim[aa[1]].frame;
		scratch.animation_info[1]._frame = ax;
	}

	if (global[player_hyperwalked] == -1) {
		if (ax <= 23) {
			scratch.animation_info[1]._frame = 23;
			kernel_reset_animation(aa[1], 23);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			return;
		}
		scratch.animation_info[1]._frame = 44;
		kernel_reset_animation(aa[1], 44);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
	} else {
		if (ax == 45) {
			scratch.animation_info[1]._frame = 44;
			kernel_reset_animation(aa[1], 44);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		}
	}
}

static void room_303_anim3() {
	int16 ax = scratch.animation_info[2]._frame;
	if (kernel_anim[aa[2]].frame != ax) {
		ax = kernel_anim[aa[2]].frame;
		scratch.animation_info[2]._frame = ax;
	}

	if (global[player_hyperwalked] == -1) {
		if (ax <= 23) {
			scratch.animation_info[2]._frame = 23;
			kernel_reset_animation(aa[2], 23);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
			return;
		}
		scratch.animation_info[2]._frame = 44;
		kernel_reset_animation(aa[2], 44);
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
	} else {
		if (ax == 45) {
			scratch.animation_info[2]._frame = 44;
			kernel_reset_animation(aa[2], 44);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
		}
	}
}

static void room_303_anim4_trigger() {
	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	int16 a8 = scratch._a8;
	if (a8 == 7) {
		kernel_timing_trigger(30, 28);
		scratch.animation_info[3]._val3 = 13;
		scratch._a8 = 8;
	} else if (a8 == 8) {
		scratch.animation_info[3]._val3 = 11;
		scratch.animation_info[3]._frame = 7;
		kernel_reset_animation(aa[3], 7);
		digi_play_build_ii('e', 10, 1);
		scratch._a8 = 9;
	} else if (a8 == 9) {
		scratch.animation_info[3]._val3 = 14;
		scratch.animation_info[3]._frame = 13;
		kernel_reset_animation(aa[3], 13);
		scratch._a8 = -1;
	} else if (a8 == 43) {
		kernel_timing_trigger(30, 28);
		scratch.animation_info[3]._val3 = 13;
		scratch._a8 = 44;
	} else if (a8 == 44) {
		scratch.animation_info[3]._val3 = 11;
		scratch.animation_info[3]._frame = 43;
		kernel_reset_animation(aa[3], 43);
		digi_play_build_ii('e', 10, 1);
		scratch._a8 = 45;
	} else if (a8 == 45) {
		scratch.animation_info[3]._val3 = 14;
		scratch.animation_info[3]._frame = 49;
		kernel_reset_animation(aa[3], 49);
		scratch._a8 = -1;
	}
}

static void room_303_anim4() {
	int16 ax = scratch.animation_info[3]._frame;
	if (kernel_anim[aa[3]].frame == ax) {
		room_303_anim4_trigger();
		return;
	}

	ax = kernel_anim[aa[3]].frame;
	scratch.animation_info[3]._frame = ax;

	if (scratch._a6 == 1) {
		if (ax == 15) {
			if (scratch.animation_info[3]._val3 == 13) {
				scratch.animation_info[3]._frame = 13;
				kernel_reset_animation(aa[3], 13);
			}
		} else if (ax < 15) {
			if (ax == 7) {
				scratch.animation_info[3]._val3 = 11;
				digi_play_build_ii('e', 2, 1);
				scratch._a8 = 7;
			} else if (ax == 12) {
				if (scratch.animation_info[3]._val3 == 11) {
					scratch.animation_info[3]._frame = 7;
					kernel_reset_animation(aa[3], 7);
				}
			}
		}
	} else if (scratch._a6 == 2) {
		if (ax == 51) {
			if (scratch.animation_info[3]._val3 == 13) {
				scratch.animation_info[3]._frame = 49;
				kernel_reset_animation(aa[3], 49);
			}
		} else if (ax < 51) {
			if (ax == 43) {
				scratch.animation_info[3]._val3 = 11;
				digi_play_build_ii('e', 4, 1);
				scratch._a8 = 43;
			} else if (ax == 48) {
				if (scratch.animation_info[3]._val3 == 11) {
					scratch.animation_info[3]._frame = 43;
					kernel_reset_animation(aa[3], 43);
				}
			}
		}
	}

	room_303_anim4_trigger();
}

static void room_303_anim6_trigger() {
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		int16 a8 = scratch._a8;
		if (a8 == 9) {
			scratch.animation_info[5]._val3 = 13;
			scratch.animation_info[5]._frame = 16;
			kernel_reset_animation(aa[5], 16);
			scratch._a8 = -1;
		} else if (a8 == 40) {
			scratch.animation_info[5]._val3 = 13;
			scratch.animation_info[5]._frame = 10;
			kernel_reset_animation(aa[5], 10);
			scratch._a8 = -1;
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 28) {
		int16 a8 = scratch._a8;
		if (a8 == 35 || a8 == 48 || a8 == 530)
			scratch._a8 = -1;
	}
}

static void room_303_anim6() {
	int16 ax = scratch.animation_info[5]._frame;
	if (kernel_anim[aa[5]].frame == ax) {
		room_303_anim6_trigger();
		return;
	}

	ax = kernel_anim[aa[5]].frame;
	scratch.animation_info[5]._frame = ax;

	if (scratch._a6 == 1) {
		if (ax == 48) {
			if (scratch.animation_info[5]._val3 == 13) {
				digi_play_build(101, '_', 1, 2);
				scratch._a8 = 48;
			}
		} else if (ax < 48) {
			if (ax == 9) {
				scratch.animation_info[5]._val3 = 11;
				digi_play_build(303, 'e', 2, 1);
				scratch._a8 = 9;
			} else if (ax == 15) {
				if (scratch.animation_info[5]._val3 == 11) {
					scratch.animation_info[5]._frame = 9;
					kernel_reset_animation(aa[5], 9);
				}
			} else if (ax == 35) {
				if (scratch.animation_info[5]._val3 == 13) {
					digi_play_build(303, '_', 1, 2);
					scratch._a8 = 35;
				}
			}
		}
	} else if (scratch._a6 == 2) {
		if (ax == 53) {
			digi_play_build(101, '_', 1, 2);
			scratch._a8 = 530;
		} else if (ax < 53) {
			if (ax == 2) {
				kernel_seq_delete(seq[0]);
			} else if (ax == 4) {
				scratch.animation_info[5]._val3 = 11;
				digi_play_build(303, 'e', 700, 1);
				scratch._a8 = 40;
			} else if (ax == 9) {
				if (scratch.animation_info[5]._val3 == 11) {
					scratch.animation_info[5]._frame = 4;
					kernel_reset_animation(aa[5], 4);
				}
			}
		}
	}

	room_303_anim6_trigger();
}

static void room_303_anim7() {
	int16 result = -1;

	if (kernel_anim[aa[6]].frame != scratch.animation_info[6]._frame) {
		int16 frame = kernel_anim[aa[6]].frame;
		scratch.animation_info[6]._frame = frame;

		bool check_val3 = (frame == 1) ||
		                  (frame >= 5 && frame <= 10) ||
		                  (frame >= 12 && frame <= 16);

		if (check_val3) {
			int16 v = scratch.animation_info[6]._val3;
			if (v == 0)
				result = 0;
			else if (v == 1)
				result = imath_random(11, 15);
			else if (v == 2)
				result = imath_random(4, 9);
		}

		if (result >= 0) {
			kernel_reset_animation(aa[6], result);
			scratch.animation_info[6]._frame = result;
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	int16 idx = scratch._a8 - 100;
	if (idx < 0 || idx > 9)
		return;

	switch (idx) {
	case 0:
		scratch.animation_info[7]._val3 = 0;
		kernel_timing_trigger(30, 28);
		scratch._a8++;
		break;
	case 1:
		scratch.animation_info[6]._val3 = 1;
		digi_play_build(303, 'r', 1, 1);
		scratch._a8++;
		break;
	case 2:
		scratch.animation_info[6]._val3 = 0;
		kernel_timing_trigger(30, 28);
		scratch._a8++;
		break;
	case 3:
		scratch.animation_info[7]._val3 = 3;
		digi_play_build(303, 'f', 2, 1);
		scratch._a8++;
		break;
	case 4:
		scratch.animation_info[7]._val3 = 0;
		kernel_timing_trigger(30, 28);
		scratch._a8++;
		break;
	case 5:
		scratch.animation_info[6]._val3 = 2;
		digi_play_build(303, 'e', 1, 1);
		scratch._a8++;
		break;
	case 6:
		scratch.animation_info[6]._val3 = 0;
		kernel_timing_trigger(30, 28);
		scratch._a8++;
		break;
	case 7:
		scratch.animation_info[7]._val3 = 3;
		digi_play_build(303, 'f', 3, 1);
		scratch._a8++;
		break;
	case 8:
		scratch.animation_info[7]._val3 = 0;
		kernel_timing_trigger(30, 28);
		scratch._a8++;
		break;
	case 9:
		dont_frag_the_palette();
		kernel_abort_animation(aa[7]);
		scratch.animation_info[7]._active = 0;
		aa[7] = kernel_run_animation(kernel_name('t', 6), 105);
		break;
	}
}

static void room_303_anim8() {
	int16 var_2 = -1;

	if (kernel_anim[aa[7]].frame == scratch.animation_info[7]._frame)
		return;

	int16 frame = kernel_anim[aa[7]].frame;
	scratch.animation_info[7]._frame = frame;

	bool check_val3 = (frame == 1) || (frame >= 4 && frame <= 10);

	if (check_val3) {
		int16 v = scratch.animation_info[7]._val3;
		if (v == 0)
			var_2 = 0;
		else if (v == 3)
			var_2 = imath_random(3, 9);
	}

	if (var_2 >= 0) {
		kernel_reset_animation(aa[7], var_2);
		scratch.animation_info[7]._frame = var_2;
	}
}

static void room_303_init() {
	global[player_score] = 0;
	scratch._a4 = 0;
	scratch._a6 = -1;
	scratch._aa = -1;

	ss[1] = kernel_load_series(kernel_name('p', 1), 0);
	seq[1] = kernel_seq_stamp(ss[1], false, KERNEL_FIRST);
	kernel_seq_depth(seq[1], 15);

	if (!player_has(6)) {
		scratch._a6 = -1;
	} else {
		kernel_flip_hotspot(words_reeds, false);
	}

	if (!global[g068]) {
		ss[0] = kernel_load_series(kernel_name('p', 2), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 7);
		kernel_seq_loc(seq[0], 280, 130);
		kernel_seq_scale(seq[0], 91);
	} else {
		kernel_flip_hotspot(words_pebbles, false);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int i = 0; i < 10; i++) {
			scratch.animation_info[i]._active = 0;
			scratch.animation_info[i]._frame = 1;
			scratch.animation_info[i]._val3 = 0;
			scratch.animation_info[i]._val4 = 0;
		}

		if (previous_room != 199 && flags[34] != 3)
			flags[34]++;
	}

	int16 f = flags[34];
	if (f == 1) {
		room_303_init1();
	} else if (f == 2 || f == 3) {
		room_303_init2();
	}
}

static void room_303_init1() {
	global[g133] = 1;
	global[g143] = 1;
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 6, 0);
	kernel_position_anim(scratch._9c, 83, 131, 92, 6);

	scratch._9a = kernel_run_animation_disp('e', 6, 0);
	kernel_position_anim(scratch._9a, 109, 139, 94, 5);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 138;
		player.y = 130;
		player.facing = 6;
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

	if (previous_room == 302) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		scratch.animation_info[0]._active = -1;
		scratch._92 = 51;
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

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = true;
	player.walker_visible = true;
}

static void room_303_init2() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 6, 0);
	kernel_position_anim(scratch._9c, 83, 131, 92, 6);

	scratch._9a = kernel_run_animation_disp('e', 6, 0);
	kernel_position_anim(scratch._9a, 109, 139, 94, 5);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 138;
		player.y = 130;
		player.facing = 6;
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

	if (previous_room == 302) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		scratch.animation_info[0]._active = -1;
		scratch._92 = 51;
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

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = true;
	player.walker_visible = true;
}

static void room_303_daemon() {
	switch (kernel.trigger) {
	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		scratch.animation_info[0]._active = 0;
		if (global[g065]) {
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
		} else {
			global[g065] = -1;
			aa[6] = kernel_run_animation(kernel_name('t', 5), 0);
			scratch.animation_info[6]._active = -1;
			scratch.animation_info[6]._val3 = 0;
			aa[7] = kernel_run_animation(kernel_name('t', 1), 104);
		}
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(aa[scratch._94]);
		scratch.animation_info[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[scratch._96]);
		scratch.animation_info[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 103:
		if (scratch._a6 == 1) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[5]);
			scratch.animation_info[5]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_flip_hotspot(words_reeds, false);
			inter_move_object(reeds, PLAYER);
			player.commands_allowed = true;
		} else if (scratch._a6 == 2) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[5]);
			scratch.animation_info[5]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_flip_hotspot(words_pebbles, false);
			if (!player_has(pebbles))
				inter_move_object(pebbles, PLAYER);
			global[g068] = -1;
			player.commands_allowed = true;
		}
		break;

	case 104:
		dont_frag_the_palette();
		kernel_abort_animation(aa[7]);
		aa[7] = kernel_run_animation(kernel_name('t', 3), 106);
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(aa[7]);
		dont_frag_the_palette();
		kernel_abort_animation(aa[6]);
		scratch.animation_info[6]._active = 0;
		global[g141] = -1;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		scratch._aa = kernel_run_animation_write(0);
		kernel_reset_animation(scratch._aa, 1);
		kernel_position_anim(scratch._aa, 109, 139, 94, 5);
		global[walker_converse_now] = 1;
		global[g007] = 6;
		kernel_synch(KERNEL_ANIM, scratch._aa, KERNEL_NOW, 0);
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		break;

	case 106:
		dont_frag_the_palette();
		kernel_abort_animation(aa[7]);
		aa[7] = kernel_run_animation(kernel_name('t', 2), 0);
		scratch.animation_info[7]._active = -1;
		scratch.animation_info[7]._val3 = 0;
		kernel_timing_trigger(10, 107);
		break;

	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
		} else if (scratch._a8 == 123) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._9e);
			scratch._a8 = -1;
			scratch._a4 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible = true;
			player.commands_allowed = true;
		} else if (scratch._a8 == 300) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._9e);
			scratch._a8 = -1;
			scratch._a4 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a6 = 2;
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 6;
		} else if (scratch._a8 == 400) {
			dont_frag_the_palette();
			kernel_abort_animation(scratch._9e);
			scratch._a8 = -1;
			scratch._a4 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a6 = 1;
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 6;
		}
		break;

	case 24:
		if (scratch._8c == 3) {
			aa[1] = kernel_run_animation(kernel_name('E', 1), 102);
			scratch.animation_info[1]._active = -1;
			scratch._96 = 1;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
		} else if (scratch._8c == 5) {
			if (scratch._a6 == 1) {
				aa[3] = kernel_run_animation(kernel_name('F', 1), 102);
			} else if (scratch._a6 == 2) {
				aa[3] = kernel_run_animation(kernel_name('F', 2), 102);
			} else {
				break;
			}
			scratch.animation_info[3]._active = -1;
			scratch._96 = 3;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
		}
		break;

	case 25:
		if (scratch._8e == 2) {
			aa[2] = kernel_run_animation(kernel_name('R', 1), 101);
			scratch.animation_info[2]._active = -1;
			scratch._94 = 2;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
		}
		break;

	case 26:
		if (scratch._90 == 1) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._90 == 6) {
			if (scratch._a6 == 1) {
				aa[5] = kernel_run_animation(kernel_name('P', 1), 103);
				scratch.animation_info[5]._active = -1;
				scratch._98 = 5;
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
				global[g133] = 1;
				global[g143] = 1;
			} else if (scratch._a6 == 2) {
				aa[5] = kernel_run_animation(kernel_name('P', 2), 103);
				scratch.animation_info[5]._active = -1;
				scratch._98 = 5;
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[5]);
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[5]);
				global[g133] = 1;
				global[g143] = 1;
			}
		}
		break;

	case 27:
		kernel_abort_animation(scratch._aa);
		scratch._aa = -1;
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 107:
		digi_play_build(303, 'f', 1, 1);
		scratch._a8 = 100;
		scratch.animation_info[7]._val3 = 3;
		break;

	default:
		break;
	}

	global_anim1(6, scratch._9a, global[g131], &global[g132]);
	global_anim2(6, scratch._9c, global[g141], &global[g142]);

	if (scratch._a4 != 0)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);

	if (scratch._aa >= 0)
		global_anim3(scratch._aa, &global[g008]);

	if (scratch.animation_info[0]._active) room_303_anim1();
	if (scratch.animation_info[2]._active) room_303_anim3();
	if (scratch.animation_info[1]._active) room_303_anim2();
	if (scratch.animation_info[3]._active) room_303_anim4();
	if (scratch.animation_info[5]._active) room_303_anim6();
	if (scratch.animation_info[6]._active) room_303_anim7();
	if (scratch.animation_info[7]._active) room_303_anim8();
}

static void room_303_pre_parser() {
	if (player_parse(words_walk_to, words_room_302, 0))
		player.walk_off_edge_to_room = 302;
}

static void room_303_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_nest, 0)) {
		global[g154] = 2;
		player.walker_visible = false;
		player.commands_allowed = false;
		scratch._9e = kernel_run_animation_talk('b', 7, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build(303, 'b', 1, 1);
		scratch._a8 = 123;
		scratch._a4 = -1;
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_look_at, words_chicory, 0)) {
		global[player_score] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a6 = 1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_look_at, words_foxglove, 0)) {
		global[player_score] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a6 = 2;
		player.command_ready = 0;
		return;
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
		player.command_ready = 0;
		return;
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
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_click_on, words_moss, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 3;
		player.command_ready = 0;
	}
}

void room_303_synchronize(Common::Serializer &s) {
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

void room_303_preload() {
	room_init_code_pointer = room_303_init;
	room_pre_parser_code_pointer = room_303_pre_parser;
	room_parser_code_pointer = room_303_parser;
	room_daemon_code_pointer = room_303_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
