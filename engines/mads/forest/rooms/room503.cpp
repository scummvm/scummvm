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

#include "mads/forest/rooms/section5.h"
#include "mads/forest/mads/inventory.h"
#include "mads/forest/mads/sounds.h"
#include "mads/forest/mads/words.h"
#include "mads/forest/digi.h"
#include "mads/forest/global.h"
#include "mads/forest/journal.h"
#include "mads/forest/midi.h"
#include "mads/core/game.h"
#include "mads/core/imath.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/mouse.h"
#include "mads/core/player.h"
#include "mads/core/sound.h"
#include "mads/core/text.h"
#include "mads/engine.h"

namespace MADS {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[12];
	AnimationInfo animation_info[12];
	int16 _pad_a0;
	int16 _a2;
	int16 _pad_a4;
	int16 _pad_a6;
	int16 _pad_a8;
	int16 _aa;
	int16 _pad_ac;
	int16 _ae;
	int16 _b0;
	int16 _b2;
	int16 _b4;
	int16 _b6;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_503_init1();
static void room_503_init2();
static void room_503_init3();

static void room_503_anim1();
static void room_503_anim2();
static void room_503_anim3();
static void room_503_anim12();
static void room_503_anim4();
static void room_503_anim5();
static void room_503_anim6();
static void room_503_anim7();
static void room_503_anim8();
static void room_503_anim9();
static void room_503_anim10();
static void room_503_anim11();

static void room_503_init() {
	scratch._b4 = -1;
	scratch._b0 = -1;
	scratch._b6 = 1;
	global[g009] = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.walker_visible = false;
		player.commands_allowed = false;

		for (int count = 0; count < 12; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame  = -1;
			aainfo[count]._val3   = 0;
			aainfo[count]._val4   = 0;
		}

		if (previous_room != 199) {
			if (flags[33] != 3)
				flags[33]++;
		}
	}

	switch (flags[33]) {
	case -3:
		flags[33] = 1;
		room_503_init1();
		break;
	case 1:
		room_503_init1();
		break;
	case 2:
		flags[33] = 3;
		room_503_init3();
		break;
	case 5:
		room_503_init2();
		break;
	case 6:
	case 7:
		room_503_init3();
		break;
	default:
		room_503_init3();
		break;
	}
}

static void room_503_init1() {
	global[play_background_sounds] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;

	if (previous_room == 199) {
		if (global[g100] != 0) {
			global_midi_play(14);
			viewing_at_y = 22;
			scratch._b6 = 2;
			aa[10] = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._active = -1;
			aa[8] = kernel_run_animation(kernel_name('F', 1), 0);
			aainfo[8]._active = -1;
			scratch._a2 = 104;
			flags[33] = 4;
		} else {
			aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
			aainfo[9]._active = -1;
			aa[10] = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._active = -1;

			if (aainfo[9]._active == -1) {
				kernel.trigger_setup_mode = 1;
				digi_initial_volume(30);
				digi_play_build(503, '_', 1, 2);
				scratch._b4 = 1;
			}

			aa[11] = kernel_run_animation(kernel_name('Z', 1), 0);
			aainfo[11]._active = -1;
		}
	} else {
		aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
		aainfo[9]._active = -1;
		aa[10] = kernel_run_animation(kernel_name('N', 2), 106);
		aainfo[10]._active = -1;

		if (aainfo[9]._active == -1) {
			kernel.trigger_setup_mode = 1;
			digi_initial_volume(30);
			digi_play_build(503, '_', 1, 2);
			scratch._b4 = 1;
		}

		global_midi_play(6);
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._a2 = 52;
	}
}

static void room_503_init2() {
	viewing_at_y = 22;
	global[play_background_sounds] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;
	mouse_hide();
	scratch._b6 = -1;
	aa[6] = kernel_run_animation(kernel_name('F', 2), 0);
	aainfo[6]._active = -1;
}

static void room_503_init3() {
	global[play_background_sounds] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;

	if (previous_room == 199) {
		if (global[g100] != 0) {
			global_midi_play(14);
			viewing_at_y = 22;
			scratch._b6 = 2;
			aa[10] = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._active = -1;
			aa[8] = kernel_run_animation(kernel_name('F', 1), 0);
			aainfo[8]._active = -1;
			scratch._a2 = 104;
			flags[33] = 4;
		} else {
			aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
			aainfo[9]._active = -1;
			aa[10] = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._active = -1;

			if (aainfo[9]._active == -1) {
				kernel.trigger_setup_mode = 1;
				digi_initial_volume(30);
				digi_play_build(503, '_', 1, 2);
				scratch._b4 = 1;
			}

			aa[11] = kernel_run_animation(kernel_name('Z', 1), 0);
			aainfo[11]._active = -1;
		}
	} else {
		aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
		aainfo[9]._active = -1;
		aa[10] = kernel_run_animation(kernel_name('N', 2), 106);
		aainfo[10]._active = -1;

		if (aainfo[9]._active == -1) {
			kernel.trigger_setup_mode = 1;
			digi_initial_volume(30);
			digi_play_build(503, '_', 1, 2);
			scratch._b4 = 1;
		}

		aa[0] = kernel_run_animation(kernel_name('y', 1), 107);
		aainfo[0]._active = -1;
		scratch._a2 = 50;
	}
}

static void room_503_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._a2 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_503_anim2() {
	if (kernel_anim[aa[11]].frame == aainfo[11]._frame)
		return;
	aainfo[11]._frame = kernel_anim[aa[11]].frame;
	if (aainfo[11]._frame == 69)
		new_room = 501;
}

static void room_503_anim3() {
	if (kernel_anim[aa[6]].frame != aainfo[6]._frame) {
		aainfo[6]._frame = kernel_anim[aa[6]].frame;
		int frame = aainfo[6]._frame;
		if (frame == 50) {
			kernel_abort_animation(aa[6]);
			aainfo[6]._active = 0;
			aa[7] = kernel_run_animation(kernel_name('F', 3), 0);
			aainfo[7]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_ANIM, aa[6]);
			digi_initial_volume(50);
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
			scratch._b2 = 1;
		} else if (frame < 50) {
			if (frame == 1) {
				kernel_seq_delete(seq[0]);
				kernel_seq_delete(seq[1]);
				kernel_seq_delete(seq[2]);
				kernel_seq_delete(seq[3]);
			} else if (frame == 2) {
				digi_initial_volume(50);
				digi_play_build(503, '_', 2, 3);
				digi_val2 = -1;
				scratch._b2 = 1;
			} else if (frame == 10) {
				digi_play_build(503, 'r', 7, 1);
				scratch._b0 = 10;
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b0 == 10)
			scratch._b0 = -1;
	}
	if (kernel.trigger == 8) {
		if (scratch._b2 == 1) {
			digi_initial_volume(80);
			scratch._b2++;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		} else if (scratch._b2 == 2) {
			digi_initial_volume(100);
			scratch._b2 = 2;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		}
	}
}

static void room_503_anim4() {
	if (kernel_anim[aa[7]].frame != aainfo[7]._frame) {
		aainfo[7]._frame = kernel_anim[aa[7]].frame;
		int frame = aainfo[7]._frame;
		if (frame == 59) {
			new_room = 510;
		} else if (frame < 59) {
			if (frame == 1) {
				digi_initial_volume(30);
				digi_play_build(503, '_', 3, 3);
				scratch._b2 = 1;
				digi_val2 = -1;
			} else if (frame == 30) {
				digi_play_build(503, 'e', 5, 1);
				scratch._b0 = 30;
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b0 == 30)
			scratch._b0 = -1;
	}
	if (kernel.trigger == 8) {
		if (scratch._b2 == 1) {
			digi_initial_volume(30);
			scratch._b2 = 1;
			digi_play_build(503, '_', 3, 3);
			digi_val2 = -1;
		}
	}
}

static void room_503_anim5() {
	if (kernel_anim[aa[8]].frame != aainfo[8]._frame) {
		aainfo[8]._frame = kernel_anim[aa[8]].frame;
		int frame = aainfo[8]._frame;
		if (frame == 104) {
			new_room = 509;
		} else if (frame < 104) {
			if (frame == 2) {
				digi_initial_volume(50);
				digi_play_build(503, '_', 2, 3);
				digi_val2 = -1;
				scratch._b2 = 1;
			} else if (frame == 3) {
				digi_play_build(503, 'r', 6, 1);
				scratch._b0 = 3;
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b0 == 3)
			scratch._b0 = -1;
	}
	if (kernel.trigger == 8) {
		if (scratch._b2 == 1) {
			digi_initial_volume(30);
			scratch._b2 = 1;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		}
	}
}

static void room_503_anim6() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame) {
		aainfo[1]._frame = kernel_anim[aa[1]].frame;
		switch (aainfo[1]._frame) {
		case 5:
			aainfo[1]._val3 = 11;
			digi_play_build(503, 'e', 1, 1);
			scratch._b0 = 5;
			break;
		case 10:
			if (aainfo[1]._val3 == 11) {
				aainfo[1]._frame = 5;
				kernel_reset_animation(aa[1], 5);
			}
			break;
		case 14:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 13;
				kernel_reset_animation(aa[1], 13);
			}
			break;
		case 15:
			aainfo[1]._val3 = 15;
			digi_play_build(503, 'g', 1, 1);
			scratch._b0 = 15;
			break;
		case 20:
			if (aainfo[1]._val3 == 15) {
				aainfo[1]._frame = 15;
				kernel_reset_animation(aa[1], 15);
			}
			break;
		case 24:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 23;
				kernel_reset_animation(aa[1], 23);
			}
			break;
		case 25:
			aainfo[1]._val3 = 12;
			digi_play_build(503, 'r', 1, 1);
			scratch._b0 = 25;
			break;
		case 30:
			if (aainfo[1]._val3 == 12) {
				aainfo[1]._frame = 25;
				kernel_reset_animation(aa[1], 25);
			}
			break;
		case 34:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 33;
				kernel_reset_animation(aa[1], 33);
			}
			break;
		case 35:
			aainfo[1]._val3 = 15;
			digi_play_build(503, 'g', 2, 1);
			scratch._b0 = 35;
			break;
		case 40:
			if (aainfo[1]._val3 == 15) {
				aainfo[1]._frame = 35;
				kernel_reset_animation(aa[1], 35);
			}
			break;
		case 44:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 43;
				kernel_reset_animation(aa[1], 43);
			}
			break;
		case 45:
			aainfo[1]._val3 = 12;
			digi_play_build(503, 'r', 2, 1);
			scratch._b0 = 45;
			break;
		case 50:
			if (aainfo[1]._val3 == 12) {
				aainfo[1]._frame = 45;
				kernel_reset_animation(aa[1], 45);
			}
			break;
		case 54:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 53;
				kernel_reset_animation(aa[1], 53);
			}
			break;
		case 55:
			aainfo[1]._val3 = 15;
			digi_play_build(503, 'g', 3, 1);
			scratch._b0 = 55;
			break;
		case 60:
			if (aainfo[1]._val3 == 15) {
				aainfo[1]._frame = 55;
				kernel_reset_animation(aa[1], 55);
			}
			break;
		case 64:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 63;
				kernel_reset_animation(aa[1], 63);
			}
			break;
		case 65:
			aainfo[1]._val3 = 12;
			digi_play_build(503, 'r', 3, 1);
			scratch._b0 = 65;
			break;
		case 70:
			if (aainfo[1]._val3 == 12) {
				aainfo[1]._frame = 65;
				kernel_reset_animation(aa[1], 65);
			}
			break;
		case 74:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 73;
				kernel_reset_animation(aa[1], 73);
			}
			break;
		case 75:
			aainfo[1]._val3 = 11;
			digi_play_build(503, 'e', 2, 1);
			scratch._b0 = 75;
			break;
		case 80:
			if (aainfo[1]._val3 == 11) {
				aainfo[1]._frame = 75;
				kernel_reset_animation(aa[1], 75);
			}
			break;
		case 84:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 83;
				kernel_reset_animation(aa[1], 83);
			}
			break;
		case 85:
			aainfo[1]._val3 = 12;
			digi_play_build(503, 'r', 4, 1);
			scratch._b0 = 85;
			break;
		case 90:
			if (aainfo[1]._val3 == 12) {
				aainfo[1]._frame = 85;
				kernel_reset_animation(aa[1], 85);
			}
			break;
		case 94:
			if (aainfo[1]._val3 == 13) {
				aainfo[1]._frame = 93;
				kernel_reset_animation(aa[1], 93);
			}
			break;
		case 95:
			dont_frag_the_palette();
			kernel_abort_animation(aa[1]);
			aainfo[1]._active = 0;
			aa[2] = kernel_run_animation("*RM503T21", 0);
			aainfo[2]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[1]);
			digi_initial_volume(20);
			scratch._b0 = 1;
			digi_play_build(503, '_', 1, 3);
			digi_val2 = -1;
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._b0) {
		case 5:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 6;
			break;
		case 6:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 10;
			kernel_reset_animation(aa[1], 10);
			scratch._b0 = -1;
			break;
		case 15:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 16;
			break;
		case 16:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 20;
			kernel_reset_animation(aa[1], 20);
			scratch._b0 = -1;
			break;
		case 25:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 26;
			break;
		case 26:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 30;
			kernel_reset_animation(aa[1], 30);
			scratch._b0 = -1;
			break;
		case 35:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 36;
			break;
		case 36:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 41;
			kernel_reset_animation(aa[1], 41);
			scratch._b0 = -1;
			break;
		case 45:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 46;
			break;
		case 46:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 50;
			kernel_reset_animation(aa[1], 50);
			scratch._b0 = -1;
			break;
		case 55:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 56;
			break;
		case 56:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 60;
			kernel_reset_animation(aa[1], 60);
			scratch._b0 = -1;
			break;
		case 65:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 66;
			break;
		case 66:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 70;
			kernel_reset_animation(aa[1], 70);
			scratch._b0 = -1;
			break;
		case 75:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 76;
			break;
		case 76:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 80;
			kernel_reset_animation(aa[1], 80);
			scratch._b0 = -1;
			break;
		case 85:
			kernel_timing_trigger(30, 28);
			aainfo[1]._val3 = 13;
			scratch._b0 = 86;
			break;
		case 86:
			aainfo[1]._val3 = 14;
			aainfo[1]._frame = 91;
			kernel_reset_animation(aa[1], 91);
			scratch._b0 = -1;
			break;
		}
	}
	if (kernel.trigger == 8) {
		if (scratch._b0 == 1) {
			digi_initial_volume(20);
			scratch._b0 = 1;
			digi_play_build(503, '_', 1, 3);
			digi_val2 = -1;
		}
	}
}

static void room_503_anim7() {
	if (kernel_anim[aa[2]].frame == aainfo[2]._frame)
		return;
	aainfo[2]._frame = kernel_anim[aa[2]].frame;
	if (aainfo[2]._frame == 31) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._active = 0;
		aa[3] = kernel_run_animation("*RM503T22", 0);
		aainfo[3]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[2]);
		digi_initial_volume(20);
		scratch._b0 = 1;
		digi_play_build(503, '_', 1, 3);
		digi_val2 = -1;
	}
}

static void room_503_anim8() {
	if (kernel_anim[aa[3]].frame == aainfo[3]._frame)
		goto check_trigger;
	aainfo[3]._frame = kernel_anim[aa[3]].frame;

	if (aainfo[3]._frame == 5) {
		aainfo[3]._val3 = 15;
		digi_play_build(503, 'g', 4, 1);
		scratch._b0 = 5;
	} else if (aainfo[3]._frame == 10) {
		if (aainfo[3]._val3 == 15) {
			aainfo[3]._frame = 5;
			kernel_reset_animation(aa[3], 5);
		}
	} else if (aainfo[3]._frame == 14) {
		if (aainfo[3]._val3 == 13) {
			aainfo[3]._frame = 13;
			kernel_reset_animation(aa[3], 13);
		}
	} else if (aainfo[3]._frame == 42) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._active = 0;
		aa[4] = kernel_run_animation(kernel_name('T', 3), 0);
		aainfo[4]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[4]);
		digi_initial_volume(20);
		scratch._b0 = 1;
		digi_play_build(503, '_', 1, 3);
		digi_val2 = -1;
	}

check_trigger:
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b0 == 5) {
			kernel_timing_trigger(30, 28);
			aainfo[3]._val3 = 13;
			scratch._b0 = 6;
		} else if (scratch._b0 == 6) {
			aainfo[3]._val3 = 14;
			aainfo[3]._frame = 10;
			kernel_reset_animation(aa[3], 10);
			scratch._b0 = -1;
		}
	}
}

static void room_503_anim9() {
	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		aainfo[4]._frame = kernel_anim[aa[4]].frame;
		switch (aainfo[4]._frame) {
		case 5:
			aainfo[4]._val3 = 11;
			digi_play_build(503, 'e', 3, 1);
			scratch._b0 = 5;
			break;
		case 9:
			if (aainfo[4]._val3 == 11) {
				aainfo[4]._frame = 5;
				kernel_reset_animation(aa[4], 5);
			}
			break;
		case 18:
			if (aainfo[4]._val3 == 13) {
				aainfo[4]._frame = 17;
				kernel_reset_animation(aa[4], 17);
			}
			break;
		case 19:
			aainfo[4]._val3 = 15;
			digi_play_build(503, 'g', 5, 1);
			scratch._b0 = 19;
			break;
		case 22:
			if (aainfo[4]._val3 == 15) {
				aainfo[4]._frame = 19;
				kernel_reset_animation(aa[4], 19);
			}
			break;
		case 25:
			if (aainfo[4]._val3 == 13) {
				aainfo[4]._frame = 24;
				kernel_reset_animation(aa[4], 24);
			}
			break;
		case 26:
			aainfo[4]._val3 = 11;
			digi_play_build(503, 'e', 4, 1);
			scratch._b0 = 26;
			break;
		case 30:
			if (aainfo[4]._val3 == 11) {
				aainfo[4]._frame = 26;
				kernel_reset_animation(aa[4], 26);
			}
			break;
		case 36:
			if (aainfo[4]._val3 == 13) {
				aainfo[4]._frame = 35;
				kernel_reset_animation(aa[4], 35);
			}
			break;
		case 37:
			aainfo[4]._val3 = 16;
			digi_play_build(503, 'b', 1, 1);
			scratch._b0 = 37;
			break;
		case 45:
			if (aainfo[4]._val3 == 16) {
				aainfo[4]._frame = 37;
				kernel_reset_animation(aa[4], 37);
			}
			break;
		case 50:
			if (aainfo[4]._val3 == 13) {
				aainfo[4]._frame = 49;
				kernel_reset_animation(aa[4], 49);
			}
			break;
		case 51:
			aainfo[4]._val3 = 12;
			digi_play_build(503, 'r', 5, 1);
			scratch._b0 = 51;
			break;
		case 55:
			if (aainfo[4]._val3 == 12) {
				aainfo[4]._frame = 51;
				kernel_reset_animation(aa[4], 51);
			}
			break;
		case 60:
			if (aainfo[4]._val3 == 13) {
				aainfo[4]._frame = 59;
				kernel_reset_animation(aa[4], 59);
			}
			break;
		case 61:
			aainfo[4]._val3 = 15;
			digi_play_build(503, 'g', 6, 1);
			scratch._b0 = 61;
			break;
		case 66:
			if (aainfo[4]._val3 == 15) {
				aainfo[4]._frame = 61;
				kernel_reset_animation(aa[4], 61);
			}
			break;
		case 69:
			if (aainfo[4]._val3 == 13) {
				aainfo[4]._frame = 68;
				kernel_reset_animation(aa[4], 68);
			}
			break;
		case 70:
			aainfo[4]._val3 = 10;
			digi_play_build(503, 'b', 2, 1);
			scratch._b0 = 70;
			break;
		case 73:
			if (aainfo[4]._val3 == 10) {
				aainfo[4]._frame = 70;
				kernel_reset_animation(aa[4], 70);
			}
			break;
		case 74:
			if (aainfo[4]._val3 == 13) {
				aainfo[4]._frame = 73;
				kernel_reset_animation(aa[4], 73);
			} else {
				dont_frag_the_palette();
				kernel_abort_animation(aa[4]);
				aainfo[4]._active = 0;
				aa[5] = kernel_run_animation(kernel_name('T', 4), 0);
				aainfo[5]._active = -1;
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_ANIM, aa[4]);
				digi_initial_volume(20);
				scratch._b0 = 1;
				digi_play_build(503, '_', 1, 3);
				digi_val2 = -1;
			}
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._b0) {
		case 5:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 13;
			scratch._b0 = 6;
			break;
		case 6:
			aainfo[4]._val3 = 14;
			aainfo[4]._frame = 9;
			kernel_reset_animation(aa[4], 9);
			scratch._b0 = -1;
			break;
		case 19:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 13;
			scratch._b0 = 20;
			break;
		case 20:
			aainfo[4]._val3 = 14;
			aainfo[4]._frame = 22;
			kernel_reset_animation(aa[4], 22);
			scratch._b0 = -1;
			break;
		case 26:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 13;
			scratch._b0 = 27;
			break;
		case 27:
			aainfo[4]._val3 = 14;
			aainfo[4]._frame = 30;
			kernel_reset_animation(aa[4], 30);
			scratch._b0 = -1;
			break;
		case 37:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 13;
			scratch._b0 = 38;
			break;
		case 38:
			aainfo[4]._val3 = 14;
			aainfo[4]._frame = 45;
			kernel_reset_animation(aa[4], 45);
			scratch._b0 = -1;
			break;
		case 51:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 13;
			scratch._b0 = 52;
			break;
		case 52:
			aainfo[4]._val3 = 14;
			aainfo[4]._frame = 55;
			kernel_reset_animation(aa[4], 55);
			scratch._b0 = -1;
			break;
		case 61:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 13;
			scratch._b0 = 62;
			break;
		case 62:
			aainfo[4]._val3 = 14;
			aainfo[4]._frame = 65;
			kernel_reset_animation(aa[4], 65);
			scratch._b0 = -1;
			break;
		case 70:
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 13;
			scratch._b0 = 71;
			break;
		case 71:
			aainfo[4]._val3 = 14;
			aainfo[4]._frame = 70;
			kernel_reset_animation(aa[4], 70);
			scratch._b0 = -1;
			break;
		}
	}
}

static void room_503_anim10() {
	if (kernel_anim[aa[5]].frame != aainfo[5]._frame) {
		aainfo[5]._frame = kernel_anim[aa[5]].frame;
		int frame = aainfo[5]._frame;
		if (frame == 40) {
			midi_stop();
			global[g100] = -1;
			display_journal();
		} else if (frame < 40) {
			if (frame == 5) {
				aainfo[5]._val3 = 16;
				digi_play_build(503, 'i', 1, 1);
				scratch._b0 = 5;
			} else if (frame == 10) {
				if (aainfo[5]._val3 == 16) {
					aainfo[5]._frame = 5;
					kernel_reset_animation(aa[5], 5);
				}
			} else if (frame == 14) {
				if (aainfo[5]._val3 == 13) {
					aainfo[5]._frame = 13;
					kernel_reset_animation(aa[5], 13);
				}
			} else if (frame == 31) {
				aainfo[5]._val3 = 16;
				digi_play_build(503, 'b', 3, 1);
				scratch._b0 = 31;
			} else if (frame == 36) {
				if (aainfo[5]._val3 == 16) {
					aainfo[5]._frame = 31;
					kernel_reset_animation(aa[5], 31);
				}
			} else if (frame == 39) {
				if (aainfo[5]._val3 == 13) {
					aainfo[5]._frame = 38;
					kernel_reset_animation(aa[5], 38);
				}
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b0 == 5) {
			kernel_timing_trigger(30, 28);
			aainfo[5]._val3 = 13;
			scratch._b0 = 6;
		} else if (scratch._b0 == 6) {
			aainfo[5]._val3 = 14;
			aainfo[5]._frame = 9;
			kernel_reset_animation(aa[5], 9);
			scratch._b0 = -1;
		} else if (scratch._b0 == 31) {
			kernel_timing_trigger(30, 28);
			aainfo[5]._val3 = 13;
			scratch._b0 = 32;
		} else if (scratch._b0 == 32) {
			aainfo[5]._val3 = 14;
			aainfo[5]._frame = 36;
			kernel_reset_animation(aa[5], 36);
			scratch._b0 = -1;
		}
	}
}

static void room_503_anim11() {
	if (kernel_anim[aa[10]].frame == aainfo[10]._frame)
		return;
	aainfo[10]._frame = kernel_anim[aa[10]].frame;

	int frame = aainfo[10]._frame;
	if (frame == 49) {
		aainfo[10]._frame = 1;
		kernel_reset_animation(aa[10], 1);
		return;
	}
	if (frame > 49)
		return;

	if (frame == 19 || frame == 30 || frame == 40) {
		if (imath_random(0, 100) > 20) {
			aainfo[10]._frame--;
			kernel_reset_animation(aa[10], aainfo[10]._frame);
		}
	} else if (frame == 34 || frame == 46) {
		if (imath_random(0, 100) > 40) {
			aainfo[10]._frame--;
			kernel_reset_animation(aa[10], aainfo[10]._frame);
		}
	} else if (frame == 20) {
		if (imath_random(0, 100) > 60) {
			aainfo[10]._frame -= 2;
			kernel_reset_animation(aa[10], aainfo[10]._frame);
		}
	}
}

static void room_503_anim12() {
	if (kernel_anim[aa[9]].frame == aainfo[9]._frame)
		return;
	aainfo[9]._frame = kernel_anim[aa[9]].frame;
	if (aainfo[9]._frame == 49) {
		aainfo[9]._frame = 1;
		kernel_reset_animation(aa[9], 1);
	}
}

static void room_503_daemon() {
	if (global[g101] != 0 && global[player_hyperwalked] == -1) {
		g_engine->saveAutosaveIfEnabled();
		new_room = 904;
	}

	switch (kernel.trigger) {
	case 8:
		if (scratch._b4 == 1) {
			digi_initial_volume(30);
			digi_play_build(503, '_', 1, 3);
			digi_val2 = -1;
			scratch._b4 = 1;
		}
		break;

	case 27:
		global[walker_converse_now] = 0;
		kernel_abort_animation(scratch._ae);
		kernel_reset_animation(scratch._aa, 1);
		kernel_synch(KERNEL_ANIM, scratch._aa, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		aa[1] = kernel_run_animation(kernel_name('t', 1), 1);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		aainfo[1]._active = -1;
		player.commands_allowed = false;
		digi_initial_volume(20);
		scratch._b0 = 1;
		digi_play_build(503, '_', 1, 3);
		digi_val2 = -1;
		break;

	case 104:
		if (flags[33] == 6) {
			aa[8] = kernel_run_animation(kernel_name('F', 1), 0);
			aainfo[8]._active = -1;
			scratch._a2 = 104;
			digi_initial_volume(50);
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
			scratch._b2 = 1;
		}
		break;

	case 105:
		if (flags[33] == 7) {
			aa[6] = kernel_run_animation(kernel_name('F', 2), 0);
			aainfo[6]._active = -1;
			digi_initial_volume(50);
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
			scratch._b2 = 1;
		}
		break;

	case 107:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		midi_stop();
		global[g100] = -1;
		display_journal();
		break;

	default:
		break;
	}

	if (aainfo[0]._active  != 0) room_503_anim1();
	if (aainfo[11]._active != 0) room_503_anim2();
	if (aainfo[6]._active  != 0) room_503_anim3();
	if (aainfo[7]._active  != 0) room_503_anim4();
	if (aainfo[8]._active  != 0) room_503_anim5();
	if (aainfo[1]._active  != 0) room_503_anim6();
	if (aainfo[2]._active  != 0) room_503_anim7();
	if (aainfo[3]._active  != 0) room_503_anim8();
	if (aainfo[4]._active  != 0) room_503_anim9();
	if (aainfo[5]._active  != 0) room_503_anim10();

	if (scratch._b6 == 1) {
		room_503_anim12();
		room_503_anim11();
	} else if (scratch._b6 == 2) {
		room_503_anim11();
	}

	if (global[walker_converse_now] != 0)
		global_anim3(scratch._ae, &global[g008]);
}

void room_503_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._pad_a0);
	s.syncAsSint16LE(scratch._a2);
	s.syncAsSint16LE(scratch._pad_a4);
	s.syncAsSint16LE(scratch._pad_a6);
	s.syncAsSint16LE(scratch._pad_a8);
	s.syncAsSint16LE(scratch._aa);
	s.syncAsSint16LE(scratch._pad_ac);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._b0);
	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
	s.syncAsSint16LE(scratch._b6);
}

void room_503_preload() {
	room_init_code_pointer       = room_503_init;
	room_daemon_code_pointer     = room_503_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADS
