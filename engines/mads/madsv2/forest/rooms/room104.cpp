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

#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];       /* Sprite series handles */
	int16 sequence[10];     /* Sequence handles      */
	int16 animation[10];    /* Animation handles     */
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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_104_init1();
static void room_104_init2();
static void room_104_init3();
static void room_104_anim1();
static void room_104_anim2();
static void room_104_anim3();
static void room_104_anim4();
static void room_104_anim5();
static void room_104_anim6();
static void room_104_anim7();
static void room_104_anim8();

static void room_104_init() {
	scratch._a0 = -1;
	global[player_score] = 0;

	for (int count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = 1;
		aainfo[count]._val3 = 0;
		aainfo[count]._val4 = 0;
	}

	if (previous_room != -2 && previous_room != 199) {
		player.walker_visible = 0;
		player.commands_allowed = 0;
		if (flags[4] != 3)
			flags[4]++;
	}

	switch (flags[4]) {
	case -3:
		if (previous_room == 107) { flags[4] = 5; room_104_init2(); return; }
		room_104_init1();
		return;
	case -2:
		if (previous_room == 107) { flags[4] = 5; room_104_init2(); return; }
		flags[4] = -3;
		room_104_init1();
		return;
	case -1:
		if (previous_room == 107) { flags[4] = 5; room_104_init2(); return; }
		flags[4] = -3;
		room_104_init1();
		return;
	case 1:
		if (previous_room == 107) { flags[4] = 5; room_104_init2(); return; }
		room_104_init3();
		return;
	case 2:
		if (previous_room == 107) { flags[4] = 5; room_104_init2(); return; }
		room_104_init3();
		return;
	case 3:
		if (previous_room != 107) { room_104_init3(); return; }
		flags[4] = 5;
		room_104_init2();
		return;
	case 5:
		if (previous_room == 107) flags[4] = 5;
		room_104_init2();
		return;
	case 6:
		if (previous_room == 107) { flags[4] = 5; room_104_init2(); return; }
		room_104_init3();
		return;
	case 7:
		if (previous_room == 107) { flags[4] = 5; room_104_init2(); return; }
		room_104_init3();
		return;
	default:
		room_104_init3();
		return;
	}
}

static void room_104_init1() {
	global[player_score] = 0;
	global[g009] = -1;
	global_midi_play(8);
	viewing_at_y = 22;
	player.walker_visible = 0;
	player.commands_allowed = 0;
	mouse_hide();

	for (int count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = -1;
	}

	aa[6] = kernel_run_animation(kernel_name('s', 1), 0);
	aainfo[6]._active = -1;
}

static void room_104_init2() {
	global[player_score] = 0;
	global[g009] = 0;
	midi_stop();
	viewing_at_y = 22;
	player.walker_visible = 0;
	player.commands_allowed = 0;
	mouse_hide();

	for (int count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = -1;
	}

	aa[7] = kernel_run_animation(kernel_name('T', 1), 0);
	aainfo[7]._active = -1;
}

static void room_104_init3() {
	global[g009] = -1;
	global_midi_play(8);
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 4, 0);
	kernel_position_anim(scratch._9c, 265, 111, 87, 14);
	scratch._9a = kernel_run_animation_disp('e', 4, 0);
	kernel_position_anim(scratch._9a, 248, 137, 94, 3);

	aa[4] = kernel_run_animation(kernel_name('C', 1), 0);
	aainfo[4]._active = -1;
	aa[5] = kernel_run_animation(kernel_name('M', 1), 0);
	aainfo[5]._active = -1;

	if (previous_room != -2) {
		player.x = 214;
		player.y = 142;
		player.facing = 4;
	}

	if (previous_room == -2) {
		if (global[g101] != 0) return;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	} else if (previous_room == 101) {
		scratch._a0 = 0;
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 31;
		return;
	} else if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = -1;
	} else {
		if (global[g101] != 0) return;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = -1;
		player.walker_visible = -1;
	}
}

static void room_104_anim1() {
	if (kernel_anim[aa[6]].frame != aainfo[6]._frame) {
		int16 frame = kernel_anim[aa[6]].frame;
		aainfo[6]._frame = frame;
		scratch._a0 = 0;

		switch (frame) {
		case 1:
			aainfo[6]._val3 = 7;
			digi_play_build(104, 'c', 1, 1);
			scratch._a2 = 1;
			break;
		case 5:
			digi_initial_volume(40);
			digi_play_build(104, '_', 2, 2);
			scratch._a4 = 5;
			break;
		case 32:
			if (aainfo[6]._val3 == 7) {
				aainfo[6]._frame = 13;
				kernel_reset_animation(aa[6], 13);
			} else if (aainfo[6]._val3 == 9) {
				aainfo[6]._frame = 31;
				kernel_reset_animation(aa[6], 31);
			}
			break;
		case 33:
			aainfo[6]._val3 = 8;
			digi_play_build(104, 'e', 1, 1);
			scratch._a2 = 33;
			break;
		case 42:
			if (aainfo[6]._val3 == 8) {
				aainfo[6]._frame = 33;
				kernel_reset_animation(aa[6], 33);
			}
			break;
		case 43:
			digi_initial_volume(40);
			digi_play_build(104, '_', 2, 2);
			scratch._a4 = 43;
			break;
		case 45:
			if (aainfo[6]._val3 == 9) {
				aainfo[6]._frame = 44;
				kernel_reset_animation(aa[6], 44);
			}
			break;
		case 46:
			aainfo[6]._val3 = 7;
			digi_play_build(104, 'c', 2, 1);
			scratch._a2 = 46;
			break;
		case 69:
			if (aainfo[6]._val3 == 7) {
				aainfo[6]._frame = 46;
				kernel_reset_animation(aa[6], 46);
			}
			break;
		case 71:
			if (aainfo[6]._val3 == 9) {
				aainfo[6]._frame = 70;
				kernel_reset_animation(aa[6], 70);
			}
			break;
		case 75:
			digi_initial_volume(40);
			digi_play_build(104, '_', 2, 2);
			scratch._a4 = 75;
			break;
		case 82:
			digi_play_build(101, '_', 1, 1);
			scratch._a4 = 82;
			break;
		case 95:
			if (global[g023] != 0) {
				new_room = 101;
				global[g023] = 0;
			} else {
				new_room = 904;
			}
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._a2) {
		case 1:
			kernel_timing_trigger(30, 28);
			aainfo[6]._val3 = 9;
			scratch._a2 = 2;
			break;
		case 2:
			aainfo[6]._val3 = 10;
			aainfo[6]._frame = 31;
			kernel_reset_animation(aa[6], 31);
			scratch._a2 = -1;
			break;
		case 32:
			kernel_timing_trigger(30, 28);
			aainfo[6]._val3 = 9;
			scratch._a2 = 33;
			break;
		case 33:
			aainfo[6]._val3 = 10;
			aainfo[6]._frame = 42;
			kernel_reset_animation(aa[6], 42);
			scratch._a2 = -1;
			break;
		case 46:
			kernel_timing_trigger(30, 28);
			aainfo[6]._val3 = 9;
			scratch._a2 = 47;
			break;
		case 47:
			aainfo[6]._val3 = 7;
			aainfo[6]._frame = 13;
			digi_play_build(104, 'c', 3, 1);
			kernel_reset_animation(aa[6], aainfo[6]._frame);
			scratch._a2 = 48;
			break;
		case 48:
			kernel_timing_trigger(30, 28);
			aainfo[6]._val3 = 9;
			scratch._a2 = 49;
			break;
		case 49:
			aainfo[6]._val3 = 10;
			aainfo[6]._frame = 69;
			kernel_reset_animation(aa[6], 69);
			scratch._a2 = -1;
			break;
		}
	}

	if (kernel.trigger == 8) {
		if (scratch._a4 == 5 || scratch._a4 == 43 || scratch._a4 == 75)
			scratch._a4 = -1;
	}
}

static void room_104_anim2() {
	if (kernel_anim[aa[7]].frame != aainfo[7]._frame) {
		int16 frame = kernel_anim[aa[7]].frame;
		aainfo[7]._frame = frame;
		scratch._a0 = 0;

		switch (frame) {
		case 1:
			digi_initial_volume(40);
			digi_play_build(104, '_', 2, 2);
			scratch._a4 = 5;
			break;
		case 14:
			aainfo[7]._val3 = 7;
			digi_play_build(104, 'c', 4, 1);
			scratch._a2 = 20;
			break;
		case 30:
			aainfo[7]._val3 = 8;
			digi_play_build(104, 'e', 2, 1);
			scratch._a2 = 30;
			break;
		case 31: case 32: case 33: case 34: case 35:
			if (aainfo[7]._val3 == 8) {
				aainfo[7]._frame = imath_random(30, 34);
				kernel_reset_animation(aa[7], aainfo[7]._frame);
			} else {
				aainfo[7]._frame = 38;
				kernel_reset_animation(aa[7], 38);
			}
			break;
		case 39:
			if (aainfo[7]._val3 == 9) {
				aainfo[7]._frame = 38;
				kernel_reset_animation(aa[7], 38);
			}
			break;
		case 40:
			aainfo[7]._val3 = 11;
			digi_play_build(104, 'b', 1, 1);
			scratch._a2 = 40;
			break;
		case 41: case 42: case 43: case 44: case 45:
			if (aainfo[7]._val3 == 11) {
				aainfo[7]._frame = imath_random(40, 44);
				kernel_reset_animation(aa[7], aainfo[7]._frame);
			} else {
				aainfo[7]._frame = 45;
				kernel_reset_animation(aa[7], 45);
			}
			break;
		case 49:
			if (aainfo[7]._val3 == 9) {
				aainfo[7]._frame = 48;
				kernel_reset_animation(aa[7], 48);
			}
			break;
		case 50:
			aainfo[7]._val3 = 7;
			digi_play_build(104, 'c', 5, 1);
			scratch._a2 = 50;
			break;
		case 51: case 52: case 53: case 54: case 55:
			if (aainfo[7]._val3 == 7) {
				aainfo[7]._frame = imath_random(50, 54);
				kernel_reset_animation(aa[7], aainfo[7]._frame);
			} else {
				aainfo[7]._frame = 55;
				kernel_reset_animation(aa[7], 55);
			}
			break;
		case 59:
			if (aainfo[7]._val3 == 9) {
				aainfo[7]._frame = 58;
				kernel_reset_animation(aa[7], 58);
			}
			break;
		case 64:
			dont_frag_the_palette();
			kernel_abort_animation(aa[7]);
			aainfo[7]._active = 0;
			stop_speech_on_run_animation = false;
			aa[8] = kernel_run_animation(kernel_name('T', 2), 0);
			aainfo[8]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_ANIM, aa[8]);
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._a2) {
		case 20:
			aainfo[7]._val3 = 10;
			scratch._a2 = -1;
			break;
		case 30:
			kernel_timing_trigger(2, 28);
			aainfo[7]._val3 = 9;
			scratch._a2 = 31;
			break;
		case 31:
			aainfo[7]._val3 = 10;
			aainfo[7]._frame = 38;
			kernel_reset_animation(aa[7], 38);
			scratch._a2 = -1;
			break;
		case 40:
			kernel_timing_trigger(5, 28);
			aainfo[7]._val3 = 9;
			scratch._a2 = 41;
			break;
		case 41:
			aainfo[7]._val3 = 10;
			aainfo[7]._frame = 45;
			kernel_reset_animation(aa[7], 45);
			scratch._a2 = -1;
			break;
		case 50:
			kernel_timing_trigger(5, 28);
			aainfo[7]._val3 = 9;
			scratch._a2 = 51;
			break;
		case 51:
			aainfo[7]._val3 = 10;
			aainfo[7]._frame = 55;
			kernel_reset_animation(aa[7], 55);
			scratch._a2 = -1;
			break;
		}
	}

	if (kernel.trigger == 8) {
		if (scratch._a4 == 5)
			scratch._a4 = -1;
	}
}

static void room_104_anim3() {
	if (kernel_anim[aa[8]].frame != aainfo[8]._frame) {
		int16 frame = kernel_anim[aa[8]].frame;
		aainfo[8]._frame = frame;
		scratch._a0 = 0;

		switch (frame) {
		case 1:
			digi_initial_volume(40);
			digi_play_build(104, '_', 5, 2);
			scratch._a4 = 1;
			break;
		case 41:
			aainfo[8]._val3 = 7;
			digi_play_build(104, 'c', 6, 1);
			scratch._a2 = 41;
			break;
		case 45:
			if (aainfo[8]._val3 == 7) {
				aainfo[8]._frame = 41;
				kernel_reset_animation(aa[8], 41);
			}
			break;
		case 47:
			if (aainfo[8]._val3 == 9) {
				aainfo[8]._frame = 46;
				kernel_reset_animation(aa[8], 46);
			}
			break;
		case 48:
			digi_play_build(104, '_', 750, 2);
			scratch._a4 = 48;
			break;
		case 59:
			aainfo[8]._val3 = 12;
			digi_play_build(104, 'm', 1, 1);
			scratch._a2 = 59;
			break;
		case 69:
			if (aainfo[8]._val3 == 12) {
				aainfo[8]._frame = 60;
				kernel_reset_animation(aa[8], 60);
			}
			break;
		case 74:
			if (aainfo[8]._val3 == 9) {
				aainfo[8]._frame = 73;
				kernel_reset_animation(aa[8], 73);
			}
			break;
		case 75:
			aainfo[8]._val3 = 11;
			digi_play_build(104, 'b', 2, 1);
			scratch._a2 = 75;
			break;
		case 79:
			if (aainfo[8]._val3 == 11) {
				aainfo[8]._frame = 75;
				kernel_reset_animation(aa[8], 75);
			}
			break;
		case 83:
			if (aainfo[8]._val3 == 9) {
				aainfo[8]._frame = 82;
				kernel_reset_animation(aa[8], 82);
			}
			break;
		case 84:
			aainfo[8]._val3 = 8;
			digi_play_build(104, 'e', 3, 1);
			scratch._a2 = 84;
			global_midi_play(14);
			break;
		case 89:
			if (aainfo[8]._val3 == 8) {
				aainfo[8]._frame = 84;
				kernel_reset_animation(aa[8], 84);
			}
			break;
		case 94:
			new_room = 510;
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._a2) {
		case 41:
			kernel_timing_trigger(30, 28);
			aainfo[8]._val3 = 9;
			scratch._a2 = 42;
			break;
		case 42:
			aainfo[8]._val3 = 10;
			aainfo[8]._frame = 45;
			kernel_reset_animation(aa[8], 45);
			scratch._a2 = -1;
			break;
		case 59:
			kernel_timing_trigger(30, 28);
			aainfo[8]._val3 = 9;
			scratch._a2 = 60;
			break;
		case 60:
			aainfo[8]._val3 = 10;
			aainfo[8]._frame = 70;
			kernel_reset_animation(aa[8], 70);
			scratch._a2 = -1;
			break;
		case 75:
			kernel_timing_trigger(30, 28);
			aainfo[8]._val3 = 9;
			scratch._a2 = 76;
			break;
		case 76:
			aainfo[8]._val3 = 10;
			aainfo[8]._frame = 79;
			kernel_reset_animation(aa[8], 79);
			scratch._a2 = -1;
			break;
		case 84:
			kernel_timing_trigger(30, 28);
			aainfo[8]._val3 = 9;
			scratch._a2 = 85;
			break;
		case 85:
			aainfo[8]._val3 = 10;
			aainfo[8]._frame = 89;
			kernel_reset_animation(aa[8], 89);
			scratch._a2 = -1;
			break;
		}
	}

	if (kernel.trigger == 8) {
		if (scratch._a4 == 1 || scratch._a4 == 48)
			scratch._a4 = -1;
	}
}

static void room_104_anim4() {
	int16 var_2 = -1;

	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		int16 frame = kernel_anim[aa[4]].frame;
		aainfo[4]._frame = frame;
		aainfo[4]._val4++;

		if (frame == 7) {
			var_2 = 0;
		} else if (frame < 7) {
			if (frame == 1) {
				int r1 = imath_random(10, 40);
				if (r1 >= aainfo[4]._val4) {
					var_2 = 0;
				} else {
					aainfo[4]._val4 = 0;
					int r2 = imath_random(1, 100);
					var_2 = (r2 > 35) ? 2 : 1;
					if (scratch._a0 != 0)
						digi_play_build(104, '_', 1, 2);
				}
			} else if (frame == 2) {
				int r = imath_random(1, 2);
				if (r >= aainfo[4]._val4) {
					var_2 = 1;
				} else {
					aainfo[4]._val4 = 0;
					var_2 = 0;
				}
			}
		}
	}

	if (var_2 >= 0) {
		aainfo[4]._frame = var_2;
		kernel_reset_animation(aa[4], var_2);
	}
}

static void room_104_anim5() {
	int16 var_2 = -1;

	if (kernel_anim[aa[5]].frame != aainfo[5]._frame) {
		int16 frame = kernel_anim[aa[5]].frame;
		aainfo[5]._frame = frame;
		aainfo[5]._val4++;

		if (frame == 12 || frame == 5) {
			var_2 = 0;
		} else if (frame < 12 && frame == 1) {
			if (scratch._a0 == 0) {
				var_2 = 0;
			} else {
				int r1 = imath_random(8, 12);
				if (r1 >= aainfo[5]._val4) {
					var_2 = 0;
				} else {
					aainfo[5]._val4 = 0;
					int r2 = imath_random(1, 100);
					if (r2 > 20) {
						var_2 = 5;
					} else {
						var_2 = 1;
						digi_play_build(104, '_', 2, 2);
					}
				}
			}
		}
	}

	if (var_2 >= 0) {
		aainfo[5]._frame = var_2;
		kernel_reset_animation(aa[5], var_2);
	}
}

static void room_104_anim6() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_104_anim7() {
	if (kernel_anim[aa[2]].frame != aainfo[2]._frame)
		aainfo[2]._frame = kernel_anim[aa[2]].frame;
	int16 frame = aainfo[2]._frame;

	if (global[player_hyperwalked] == -1) {
		if (frame <= 23) {
			aainfo[2]._frame = 23;
			kernel_reset_animation(aa[2], 23);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
		} else {
			aainfo[2]._frame = 44;
			kernel_reset_animation(aa[2], 44);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
		}
	} else if (frame == 45) {
		aainfo[2]._frame = 44;
		kernel_reset_animation(aa[2], 44);
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
	}
}

static void room_104_anim8() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame)
		aainfo[1]._frame = kernel_anim[aa[1]].frame;
	int16 frame = aainfo[1]._frame;

	if (global[player_hyperwalked] == -1) {
		if (frame <= 23) {
			aainfo[1]._frame = 23;
			kernel_reset_animation(aa[1], 23);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		} else {
			aainfo[1]._frame = 44;
			kernel_reset_animation(aa[1], 44);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		}
	} else if (frame == 45) {
		aainfo[1]._frame = 44;
		kernel_reset_animation(aa[1], 44);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
	}
}

static void room_104_daemon() {
	if (flags[0] < 0 && global[player_hyperwalked] == -1) {
		flags[0] = 0;
		flags[1] = 0;
		flags[2] = 0;
		flags[3] = 0;
		new_room = 904;
	}

	if (global[g101] != 0 && global[player_hyperwalked] == -1) {
		game_save_name(0);
		kernel_save_game(save_game_buf);
		new_room = 904;
	}

	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = -1;
		}
		break;

	case 24:
		if (scratch._8c == 3) {
			aa[1] = kernel_run_animation(kernel_name('e', 1), 102);
			aainfo[1]._active = -1;
			scratch._96 = 1;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
		} else {
			speech_ems_play("d1", kernel.trigger, scratch._8c);
		}
		break;

	case 25:
		if (scratch._8e == 2) {
			aa[2] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[2]._active = -1;
			scratch._94 = 2;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
		}
		break;

	case 26:
		kernel_reset_animation(scratch._9a, 0);
		kernel_reset_animation(scratch._9c, 0);
		global[g133] = 1;
		global[g143] = 1;
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		// TODO: word_79954 = 0;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		player.walker_visible = -1;
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = -1;
		scratch._a0 = -1;
		break;

	case 101:
		kernel_abort_animation(aa[scratch._94]);
		aainfo[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = -1;
		break;

	case 102:
		kernel_abort_animation(aa[scratch._96]);
		aainfo[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = -1;
		break;
	}

	if (flags[4] != -3 && flags[4] != 5) {
		global_anim1(4, scratch._9a, global[g131], &global[g132]);
		global_anim2(4, scratch._9c, global[g141], &global[g142]);
	}

	if (aainfo[0]._active != 0) room_104_anim6();
	if (aainfo[2]._active != 0) room_104_anim7();
	if (aainfo[1]._active != 0) room_104_anim8();
	if (aainfo[4]._active != 0) room_104_anim4();
	if (aainfo[5]._active != 0) room_104_anim5();
	if (aainfo[6]._active != 0) room_104_anim1();
	if (aainfo[7]._active != 0) room_104_anim2();
	if (aainfo[8]._active != 0) room_104_anim3();
}

static void room_104_pre_parser() {
	if (player_parse(13, 15, 0)) {
		global[g009] = 0;
		midi_stop();
		player.walk_off_edge_to_room = 101;
	}
}

static void room_104_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = 0;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(114, 116, 0)) {
		player.commands_allowed = 0;
		global[g145] = -1;
		scratch._8e = 2;
		goto handled;
	}

	if (player_parse(126, 99, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 3;
		goto handled;
	}

	if (player_parse(78, 119, 0)) {
		player.commands_allowed = 0;
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

void room_104_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)         s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)       s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)      s.syncAsSint16LE(v);
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
}

void room_104_preload() {
	room_init_code_pointer       = room_104_init;
	room_pre_parser_code_pointer = room_104_pre_parser;
	room_parser_code_pointer     = room_104_parser;
	room_daemon_code_pointer     = room_104_daemon;

	global[g016] = (global[g101] || room_id == 103) ? -1 : 0;
	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
