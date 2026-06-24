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

#include "mads/madsv2/forest/rooms/section5.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
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
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];                  /* Sprite series handles */
	int16 sequence[10];                /* Sequence handles      */
	int16 animation[10];               /* Animation handles     */
	AnimationInfo animation_info[13];  /* 13 entries: _val3/_val4 initialised for [0..11],
	                                      _active/_frame for [1..12]; [10]._val3 = _90 flag,
	                                      [11]._val3 = _98 flag */
	int16 _pad_a4;                     /* 0xA4 unused */
	int16 _a6;                         /* visit-count / playback-stage select */
	int16 _pad_a8;                     /* 0xA8 unused */
	int16 _pad_aa;                     /* 0xAA unused */
	int16 _pad_ac;                     /* 0xAC unused */
	int16 _ae;                         /* converse-anim handle (kernel_reset_animation / kernel_synch) */
	int16 _pad_b0;                     /* 0xB0 unused */
	int16 _b2;                         /* converse-anim handle (kernel_abort_animation / global_anim3) */
	int16 _b4;                         /* ambient audio num arg / uninitialised flag */
	int16 _b6;                         /* ambient audio playing flag */
	int16 _b8;                         /* ambient loop active flag */
	int16 _ba;                         /* playback variant (1 / 2 / -1) */
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
	scratch._b8 = -1;
	scratch._b4 = -1;
	scratch._ba = 1;
	global[g009] = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.walker_visible = false;
		player.commands_allowed = 0;

		for (int count = 0; count < 12; count++) {
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = -1;
			aainfo[count + 1]._active = 0;
			aainfo[count + 1]._frame = 0;
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
	global[player_score] = 0;
	player.walker_visible = false;
	player.commands_allowed = 0;

	if (previous_room == 199) {
		if (global[g100] != 0) {
			global_midi_play(14);
			viewing_at_y = 22;
			scratch._ba = 2;
			aainfo[0]._active = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._val3 = -1;
			aa[8] = kernel_run_animation(kernel_name('F', 1), 0);
			aainfo[8]._val3 = -1;
			scratch._a6 = 104;
			flags[33] = 4;
		} else {
			aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
			aainfo[9]._val3 = -1;
			aainfo[0]._active = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._val3 = -1;

			if (aainfo[9]._val3 == -1) {
				kernel.trigger_setup_mode = 1;
				digi_initial_volume(30);
				digi_play_build(503, '_', 1, 2);
				scratch._b8 = 1;
			}

			aainfo[0]._frame = kernel_run_animation(kernel_name('Z', 1), 0);
			aainfo[11]._val3 = -1;
		}
	} else {
		aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
		aainfo[9]._val3 = -1;
		aainfo[0]._active = kernel_run_animation(kernel_name('N', 2), 106);
		aainfo[10]._val3 = -1;

		if (aainfo[9]._val3 == -1) {
			kernel.trigger_setup_mode = 1;
			digi_initial_volume(30);
			digi_play_build(503, '_', 1, 2);
			scratch._b8 = 1;
		}

		global_midi_play(6);
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._val3 = -1;
		scratch._a6 = 52;
	}
}

static void room_503_init2() {
	viewing_at_y = 22;
	global[player_score] = 0;
	player.walker_visible = false;
	player.commands_allowed = 0;
	mouse_hide();
	scratch._ba = -1;
	aa[6] = kernel_run_animation(kernel_name('F', 2), 0);
	aainfo[6]._val3 = -1;
}

static void room_503_init3() {
	global[player_score] = 0;
	player.walker_visible = false;
	player.commands_allowed = 0;

	if (previous_room == 199) {
		if (global[g100] != 0) {
			global_midi_play(14);
			viewing_at_y = 22;
			scratch._ba = 2;
			aainfo[0]._active = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._val3 = -1;
			aa[8] = kernel_run_animation(kernel_name('F', 1), 0);
			aainfo[8]._val3 = -1;
			scratch._a6 = 104;
			flags[33] = 4;
		} else {
			aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
			aainfo[9]._val3 = -1;
			aainfo[0]._active = kernel_run_animation(kernel_name('N', 2), 106);
			aainfo[10]._val3 = -1;

			if (aainfo[9]._val3 == -1) {
				kernel.trigger_setup_mode = 1;
				digi_initial_volume(30);
				digi_play_build(503, '_', 1, 2);
				scratch._b8 = 1;
			}

			aainfo[0]._frame = kernel_run_animation(kernel_name('Z', 1), 0);
			aainfo[11]._val3 = -1;
		}
	} else {
		aa[9] = kernel_run_animation(kernel_name('N', 1), 106);
		aainfo[9]._val3 = -1;
		aainfo[0]._active = kernel_run_animation(kernel_name('N', 2), 106);
		aainfo[10]._val3 = -1;

		if (aainfo[9]._val3 == -1) {
			kernel.trigger_setup_mode = 1;
			digi_initial_volume(30);
			digi_play_build(503, '_', 1, 2);
			scratch._b8 = 1;
		}

		aa[0] = kernel_run_animation(kernel_name('y', 1), 107);
		aainfo[0]._val3 = -1;
		scratch._a6 = 50;
	}
}

static void room_503_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._val4)
		aainfo[0]._val4 = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[0]._val4 = scratch._a6 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._val4);
	}
}

static void room_503_anim2() {
	if (kernel_anim[aainfo[0]._frame].frame == aainfo[11]._val4)
		return;
	aainfo[11]._val4 = kernel_anim[aainfo[0]._frame].frame;
	if (aainfo[11]._val4 == 69)
		new_room = 501;
}

static void room_503_anim3() {
	if (kernel_anim[aa[6]].frame != aainfo[6]._val4) {
		aainfo[6]._val4 = kernel_anim[aa[6]].frame;
		int frame = aainfo[6]._val4;
		if (frame == 50) {
			kernel_abort_animation(aa[6]);
			aainfo[6]._val3 = 0;
			aa[7] = kernel_run_animation(kernel_name('F', 3), 0);
			aainfo[7]._val3 = -1;
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_ANIM, aa[6]);
			digi_initial_volume(50);
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
			scratch._b6 = 1;
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
				scratch._b6 = 1;
			} else if (frame == 10) {
				digi_play_build(503, 'r', 7, 1);
				scratch._b4 = 10;
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b4 == 10)
			scratch._b4 = -1;
	}
	if (kernel.trigger == 8) {
		if (scratch._b6 == 1) {
			digi_initial_volume(80);
			scratch._b6++;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		} else if (scratch._b6 == 2) {
			digi_initial_volume(100);
			scratch._b6 = 2;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		}
	}
}

static void room_503_anim4() {
	if (kernel_anim[aa[7]].frame != aainfo[7]._val4) {
		aainfo[7]._val4 = kernel_anim[aa[7]].frame;
		int frame = aainfo[7]._val4;
		if (frame == 59) {
			new_room = 510;
		} else if (frame < 59) {
			if (frame == 1) {
				digi_initial_volume(30);
				digi_play_build(503, '_', 3, 3);
				scratch._b6 = 1;
				digi_val2 = -1;
			} else if (frame == 30) {
				digi_play_build(503, 'e', 5, 1);
				scratch._b4 = 30;
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b4 == 30)
			scratch._b4 = -1;
	}
	if (kernel.trigger == 8) {
		if (scratch._b6 == 1) {
			digi_initial_volume(30);
			scratch._b6 = 1;
			digi_play_build(503, '_', 3, 3);
			digi_val2 = -1;
		}
	}
}

static void room_503_anim5() {
	if (kernel_anim[aa[8]].frame != aainfo[8]._val4) {
		aainfo[8]._val4 = kernel_anim[aa[8]].frame;
		int frame = aainfo[8]._val4;
		if (frame == 104) {
			new_room = 509;
		} else if (frame < 104) {
			if (frame == 2) {
				digi_initial_volume(50);
				digi_play_build(503, '_', 2, 3);
				digi_val2 = -1;
				scratch._b6 = 1;
			} else if (frame == 3) {
				digi_play_build(503, 'r', 6, 1);
				scratch._b4 = 3;
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b4 == 3)
			scratch._b4 = -1;
	}
	if (kernel.trigger == 8) {
		if (scratch._b6 == 1) {
			digi_initial_volume(30);
			scratch._b6 = 1;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		}
	}
}
static void room_503_anim6() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._val4) {
		aainfo[1]._val4 = kernel_anim[aa[1]].frame;
		switch (aainfo[1]._val4) {
		case 5:
			aainfo[2]._active = 11;
			digi_play_build(503, 'e', 1, 1);
			scratch._b4 = 5;
			break;
		case 10:
			if (aainfo[2]._active == 11) {
				aainfo[1]._val4 = 5;
				kernel_reset_animation(aa[1], 5);
			}
			break;
		case 14:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 13;
				kernel_reset_animation(aa[1], 13);
			}
			break;
		case 15:
			aainfo[2]._active = 15;
			digi_play_build(503, 'g', 1, 1);
			scratch._b4 = 15;
			break;
		case 20:
			if (aainfo[2]._active == 15) {
				aainfo[1]._val4 = 15;
				kernel_reset_animation(aa[1], 15);
			}
			break;
		case 24:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 23;
				kernel_reset_animation(aa[1], 23);
			}
			break;
		case 25:
			aainfo[2]._active = 12;
			digi_play_build(503, 'r', 1, 1);
			scratch._b4 = 25;
			break;
		case 30:
			if (aainfo[2]._active == 12) {
				aainfo[1]._val4 = 25;
				kernel_reset_animation(aa[1], 25);
			}
			break;
		case 34:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 33;
				kernel_reset_animation(aa[1], 33);
			}
			break;
		case 35:
			aainfo[2]._active = 15;
			digi_play_build(503, 'g', 2, 1);
			scratch._b4 = 35;
			break;
		case 40:
			if (aainfo[2]._active == 15) {
				aainfo[1]._val4 = 35;
				kernel_reset_animation(aa[1], 35);
			}
			break;
		case 44:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 43;
				kernel_reset_animation(aa[1], 43);
			}
			break;
		case 45:
			aainfo[2]._active = 12;
			digi_play_build(503, 'r', 2, 1);
			scratch._b4 = 45;
			break;
		case 50:
			if (aainfo[2]._active == 12) {
				aainfo[1]._val4 = 45;
				kernel_reset_animation(aa[1], 45);
			}
			break;
		case 54:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 53;
				kernel_reset_animation(aa[1], 53);
			}
			break;
		case 55:
			aainfo[2]._active = 15;
			digi_play_build(503, 'g', 3, 1);
			scratch._b4 = 55;
			break;
		case 60:
			if (aainfo[2]._active == 15) {
				aainfo[1]._val4 = 55;
				kernel_reset_animation(aa[1], 55);
			}
			break;
		case 64:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 63;
				kernel_reset_animation(aa[1], 63);
			}
			break;
		case 65:
			aainfo[2]._active = 12;
			digi_play_build(503, 'r', 3, 1);
			scratch._b4 = 65;
			break;
		case 70:
			if (aainfo[2]._active == 12) {
				aainfo[1]._val4 = 65;
				kernel_reset_animation(aa[1], 65);
			}
			break;
		case 74:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 73;
				kernel_reset_animation(aa[1], 73);
			}
			break;
		case 75:
			aainfo[2]._active = 11;
			digi_play_build(503, 'e', 2, 1);
			scratch._b4 = 75;
			break;
		case 80:
			if (aainfo[2]._active == 11) {
				aainfo[1]._val4 = 75;
				kernel_reset_animation(aa[1], 75);
			}
			break;
		case 84:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 83;
				kernel_reset_animation(aa[1], 83);
			}
			break;
		case 85:
			aainfo[2]._active = 12;
			digi_play_build(503, 'r', 4, 1);
			scratch._b4 = 85;
			break;
		case 90:
			if (aainfo[2]._active == 12) {
				aainfo[1]._val4 = 85;
				kernel_reset_animation(aa[1], 85);
			}
			break;
		case 94:
			if (aainfo[2]._active == 13) {
				aainfo[1]._val4 = 93;
				kernel_reset_animation(aa[1], 93);
			}
			break;
		case 95:
			dont_frag_the_palette();
			kernel_abort_animation(aa[1]);
			aainfo[1]._val3 = 0;
			aa[2] = kernel_run_animation("*RM503T21", 0);
			aainfo[2]._val3 = -1;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[1]);
			digi_initial_volume(20);
			scratch._b4 = 1;
			digi_play_build(503, '_', 1, 3);
			digi_val2 = -1;
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._b4) {
		case 5:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 6;
			break;
		case 6:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 10;
			kernel_reset_animation(aa[1], 10);
			scratch._b4 = -1;
			break;
		case 15:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 16;
			break;
		case 16:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 20;
			kernel_reset_animation(aa[1], 20);
			scratch._b4 = -1;
			break;
		case 25:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 26;
			break;
		case 26:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 30;
			kernel_reset_animation(aa[1], 30);
			scratch._b4 = -1;
			break;
		case 35:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 36;
			break;
		case 36:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 41;
			kernel_reset_animation(aa[1], 41);
			scratch._b4 = -1;
			break;
		case 45:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 46;
			break;
		case 46:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 50;
			kernel_reset_animation(aa[1], 50);
			scratch._b4 = -1;
			break;
		case 55:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 56;
			break;
		case 56:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 60;
			kernel_reset_animation(aa[1], 60);
			scratch._b4 = -1;
			break;
		case 65:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 66;
			break;
		case 66:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 70;
			kernel_reset_animation(aa[1], 70);
			scratch._b4 = -1;
			break;
		case 75:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 76;
			break;
		case 76:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 80;
			kernel_reset_animation(aa[1], 80);
			scratch._b4 = -1;
			break;
		case 85:
			kernel_timing_trigger(30, 28);
			aainfo[2]._active = 13;
			scratch._b4 = 86;
			break;
		case 86:
			aainfo[2]._active = 14;
			aainfo[1]._val4 = 91;
			kernel_reset_animation(aa[1], 91);
			scratch._b4 = -1;
			break;
		}
	}
	if (kernel.trigger == 8) {
		if (scratch._b4 == 1) {
			digi_initial_volume(20);
			scratch._b4 = 1;
			digi_play_build(503, '_', 1, 3);
			digi_val2 = -1;
		}
	}
}
static void room_503_anim7() {
	if (kernel_anim[aa[2]].frame == aainfo[2]._val4)
		return;
	aainfo[2]._val4 = kernel_anim[aa[2]].frame;
	if (aainfo[2]._val4 == 31) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._val3 = 0;
		aa[3] = kernel_run_animation("*RM503T22", 0);
		aainfo[3]._val3 = -1;
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[2]);
		digi_initial_volume(20);
		scratch._b4 = 1;
		digi_play_build(503, '_', 1, 3);
		digi_val2 = -1;
	}
}

static void room_503_anim8() {
	if (kernel_anim[aa[3]].frame == aainfo[3]._val4)
		goto check_trigger;
	aainfo[3]._val4 = kernel_anim[aa[3]].frame;

	if (aainfo[3]._val4 == 5) {
		aainfo[4]._active = 15;
		digi_play_build(503, 'g', 4, 1);
		scratch._b4 = 5;
	} else if (aainfo[3]._val4 == 10) {
		if (aainfo[4]._active == 15) {
			aainfo[3]._val4 = 5;
			kernel_reset_animation(aa[3], 5);
		}
	} else if (aainfo[3]._val4 == 14) {
		if (aainfo[4]._active == 13) {
			aainfo[3]._val4 = 13;
			kernel_reset_animation(aa[3], 13);
		}
	} else if (aainfo[3]._val4 == 42) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._val3 = 0;
		aa[4] = kernel_run_animation(kernel_name('T', 3), 0);
		aainfo[4]._val3 = -1;
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[4]);
		digi_initial_volume(20);
		scratch._b4 = 1;
		digi_play_build(503, '_', 1, 3);
		digi_val2 = -1;
	}

check_trigger:
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b4 == 5) {
			kernel_timing_trigger(30, 28);
			aainfo[4]._active = 13;
			scratch._b4 = 6;
		} else if (scratch._b4 == 6) {
			aainfo[4]._active = 14;
			aainfo[3]._val4 = 10;
			kernel_reset_animation(aa[3], 10);
			scratch._b4 = -1;
		}
	}
}
static void room_503_anim9() {
	if (kernel_anim[aa[4]].frame != aainfo[4]._val4) {
		aainfo[4]._val4 = kernel_anim[aa[4]].frame;
		switch (aainfo[4]._val4) {
		case 5:
			aainfo[5]._active = 11;
			digi_play_build(503, 'e', 3, 1);
			scratch._b4 = 5;
			break;
		case 9:
			if (aainfo[5]._active == 11) {
				aainfo[4]._val4 = 5;
				kernel_reset_animation(aa[4], 5);
			}
			break;
		case 18:
			if (aainfo[5]._active == 13) {
				aainfo[4]._val4 = 17;
				kernel_reset_animation(aa[4], 17);
			}
			break;
		case 19:
			aainfo[5]._active = 15;
			digi_play_build(503, 'g', 5, 1);
			scratch._b4 = 19;
			break;
		case 22:
			if (aainfo[5]._active == 15) {
				aainfo[4]._val4 = 19;
				kernel_reset_animation(aa[4], 19);
			}
			break;
		case 25:
			if (aainfo[5]._active == 13) {
				aainfo[4]._val4 = 24;
				kernel_reset_animation(aa[4], 24);
			}
			break;
		case 26:
			aainfo[5]._active = 11;
			digi_play_build(503, 'e', 4, 1);
			scratch._b4 = 26;
			break;
		case 30:
			if (aainfo[5]._active == 11) {
				aainfo[4]._val4 = 26;
				kernel_reset_animation(aa[4], 26);
			}
			break;
		case 36:
			if (aainfo[5]._active == 13) {
				aainfo[4]._val4 = 35;
				kernel_reset_animation(aa[4], 35);
			}
			break;
		case 37:
			aainfo[5]._active = 16;
			digi_play_build(503, 'b', 1, 1);
			scratch._b4 = 37;
			break;
		case 45:
			if (aainfo[5]._active == 16) {
				aainfo[4]._val4 = 37;
				kernel_reset_animation(aa[4], 37);
			}
			break;
		case 50:
			if (aainfo[5]._active == 13) {
				aainfo[4]._val4 = 49;
				kernel_reset_animation(aa[4], 49);
			}
			break;
		case 51:
			aainfo[5]._active = 12;
			digi_play_build(503, 'r', 5, 1);
			scratch._b4 = 51;
			break;
		case 55:
			if (aainfo[5]._active == 12) {
				aainfo[4]._val4 = 51;
				kernel_reset_animation(aa[4], 51);
			}
			break;
		case 60:
			if (aainfo[5]._active == 13) {
				aainfo[4]._val4 = 59;
				kernel_reset_animation(aa[4], 59);
			}
			break;
		case 61:
			aainfo[5]._active = 15;
			digi_play_build(503, 'g', 6, 1);
			scratch._b4 = 61;
			break;
		case 66:
			if (aainfo[5]._active == 15) {
				aainfo[4]._val4 = 61;
				kernel_reset_animation(aa[4], 61);
			}
			break;
		case 69:
			if (aainfo[5]._active == 13) {
				aainfo[4]._val4 = 68;
				kernel_reset_animation(aa[4], 68);
			}
			break;
		case 70:
			aainfo[5]._active = 10;
			digi_play_build(503, 'b', 2, 1);
			scratch._b4 = 70;
			break;
		case 73:
			if (aainfo[5]._active == 10) {
				aainfo[4]._val4 = 70;
				kernel_reset_animation(aa[4], 70);
			}
			break;
		case 74:
			if (aainfo[5]._active == 13) {
				aainfo[4]._val4 = 73;
				kernel_reset_animation(aa[4], 73);
			} else {
				dont_frag_the_palette();
				kernel_abort_animation(aa[4]);
				aainfo[4]._val3 = 0;
				aa[5] = kernel_run_animation(kernel_name('T', 4), 0);
				aainfo[5]._val3 = -1;
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_ANIM, aa[4]);
				digi_initial_volume(20);
				scratch._b4 = 1;
				digi_play_build(503, '_', 1, 3);
				digi_val2 = -1;
			}
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		switch (scratch._b4) {
		case 5:
			kernel_timing_trigger(30, 28);
			aainfo[5]._active = 13;
			scratch._b4 = 6;
			break;
		case 6:
			aainfo[5]._active = 14;
			aainfo[4]._val4 = 9;
			kernel_reset_animation(aa[4], 9);
			scratch._b4 = -1;
			break;
		case 19:
			kernel_timing_trigger(30, 28);
			aainfo[5]._active = 13;
			scratch._b4 = 20;
			break;
		case 20:
			aainfo[5]._active = 14;
			aainfo[4]._val4 = 22;
			kernel_reset_animation(aa[4], 22);
			scratch._b4 = -1;
			break;
		case 26:
			kernel_timing_trigger(30, 28);
			aainfo[5]._active = 13;
			scratch._b4 = 27;
			break;
		case 27:
			aainfo[5]._active = 14;
			aainfo[4]._val4 = 30;
			kernel_reset_animation(aa[4], 30);
			scratch._b4 = -1;
			break;
		case 37:
			kernel_timing_trigger(30, 28);
			aainfo[5]._active = 13;
			scratch._b4 = 38;
			break;
		case 38:
			aainfo[5]._active = 14;
			aainfo[4]._val4 = 45;
			kernel_reset_animation(aa[4], 45);
			scratch._b4 = -1;
			break;
		case 51:
			kernel_timing_trigger(30, 28);
			aainfo[5]._active = 13;
			scratch._b4 = 52;
			break;
		case 52:
			aainfo[5]._active = 14;
			aainfo[4]._val4 = 55;
			kernel_reset_animation(aa[4], 55);
			scratch._b4 = -1;
			break;
		case 61:
			kernel_timing_trigger(30, 28);
			aainfo[5]._active = 13;
			scratch._b4 = 62;
			break;
		case 62:
			aainfo[5]._active = 14;
			aainfo[4]._val4 = 65;
			kernel_reset_animation(aa[4], 65);
			scratch._b4 = -1;
			break;
		case 70:
			kernel_timing_trigger(30, 28);
			aainfo[5]._active = 13;
			scratch._b4 = 71;
			break;
		case 71:
			aainfo[5]._active = 14;
			aainfo[4]._val4 = 70;
			kernel_reset_animation(aa[4], 70);
			scratch._b4 = -1;
			break;
		}
	}
}

static void room_503_anim10() {
	if (kernel_anim[aa[5]].frame != aainfo[5]._val4) {
		aainfo[5]._val4 = kernel_anim[aa[5]].frame;
		int frame = aainfo[5]._val4;
		if (frame == 40) {
			midi_stop();
			global[g100] = -1;
			display_journal();
		} else if (frame < 40) {
			if (frame == 5) {
				aainfo[6]._active = 16;
				digi_play_build(503, 'i', 1, 1);
				scratch._b4 = 5;
			} else if (frame == 10) {
				if (aainfo[6]._active == 16) {
					aainfo[5]._val4 = 5;
					kernel_reset_animation(aa[5], 5);
				}
			} else if (frame == 14) {
				if (aainfo[6]._active == 13) {
					aainfo[5]._val4 = 13;
					kernel_reset_animation(aa[5], 13);
				}
			} else if (frame == 31) {
				aainfo[6]._active = 16;
				digi_play_build(503, 'b', 3, 1);
				scratch._b4 = 31;
			} else if (frame == 36) {
				if (aainfo[6]._active == 16) {
					aainfo[5]._val4 = 31;
					kernel_reset_animation(aa[5], 31);
				}
			} else if (frame == 39) {
				if (aainfo[6]._active == 13) {
					aainfo[5]._val4 = 38;
					kernel_reset_animation(aa[5], 38);
				}
			}
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._b4 == 5) {
			kernel_timing_trigger(30, 28);
			aainfo[6]._active = 13;
			scratch._b4 = 6;
		} else if (scratch._b4 == 6) {
			aainfo[6]._active = 14;
			aainfo[5]._val4 = 9;
			kernel_reset_animation(aa[5], 9);
			scratch._b4 = -1;
		} else if (scratch._b4 == 31) {
			kernel_timing_trigger(30, 28);
			aainfo[6]._active = 13;
			scratch._b4 = 32;
		} else if (scratch._b4 == 32) {
			aainfo[6]._active = 14;
			aainfo[5]._val4 = 36;
			kernel_reset_animation(aa[5], 36);
			scratch._b4 = -1;
		}
	}
}

static void room_503_anim11() {
	if (kernel_anim[aainfo[0]._active].frame == aainfo[10]._val4)
		return;
	aainfo[10]._val4 = kernel_anim[aainfo[0]._active].frame;

	int frame = aainfo[10]._val4;
	if (frame == 49) {
		aainfo[10]._val4 = 1;
		kernel_reset_animation(aainfo[0]._active, 1);
		return;
	}
	if (frame > 49)
		return;

	if (frame == 19 || frame == 30 || frame == 40) {
		if (imath_random(0, 100) > 20) {
			aainfo[10]._val4--;
			kernel_reset_animation(aainfo[0]._active, aainfo[10]._val4);
		}
	} else if (frame == 34 || frame == 46) {
		if (imath_random(0, 100) > 40) {
			aainfo[10]._val4--;
			kernel_reset_animation(aainfo[0]._active, aainfo[10]._val4);
		}
	} else if (frame == 20) {
		if (imath_random(0, 100) > 60) {
			aainfo[10]._val4 -= 2;
			kernel_reset_animation(aainfo[0]._active, aainfo[10]._val4);
		}
	}
}

static void room_503_anim12() {
	if (kernel_anim[aa[9]].frame == aainfo[9]._val4)
		return;
	aainfo[9]._val4 = kernel_anim[aa[9]].frame;
	if (aainfo[9]._val4 == 49) {
		aainfo[9]._val4 = 1;
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
		if (scratch._b8 == 1) {
			digi_initial_volume(30);
			digi_play_build(503, '_', 1, 3);
			digi_val2 = -1;
			scratch._b8 = 1;
		}
		break;

	case 27:
		global[walker_converse_now] = 0;
		kernel_abort_animation(scratch._b2);
		kernel_reset_animation(scratch._ae, 1);
		kernel_synch(KERNEL_ANIM, scratch._ae, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = -1;
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._val3 = 0;
		aa[1] = kernel_run_animation(kernel_name('t', 1), 1);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		aainfo[1]._val3 = -1;
		player.commands_allowed = 0;
		digi_initial_volume(20);
		scratch._b4 = 1;
		digi_play_build(503, '_', 1, 3);
		digi_val2 = -1;
		break;

	case 104:
		if (flags[33] == 6) {
			aa[8] = kernel_run_animation(kernel_name('F', 1), 0);
			aainfo[8]._val3 = -1;
			scratch._a6 = 104;
			digi_initial_volume(50);
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
			scratch._b6 = 1;
		}
		break;

	case 105:
		if (flags[33] == 7) {
			aa[6] = kernel_run_animation(kernel_name('F', 2), 0);
			aainfo[6]._val3 = -1;
			digi_initial_volume(50);
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
			scratch._b6 = 1;
		}
		break;

	case 107:
		kernel_abort_animation(aa[0]);
		aainfo[0]._val3 = 0;
		midi_stop();
		global[g100] = -1;
		display_journal();
		break;

	default:
		break;
	}

	if (aainfo[0]._val3 != 0)  room_503_anim1();
	if (aainfo[11]._val3 != 0) room_503_anim2();
	if (aainfo[6]._val3 != 0)  room_503_anim3();
	if (aainfo[7]._val3 != 0)  room_503_anim4();
	if (aainfo[8]._val3 != 0)  room_503_anim5();
	if (aainfo[1]._val3 != 0)  room_503_anim6();
	if (aainfo[2]._val3 != 0)  room_503_anim7();
	if (aainfo[3]._val3 != 0)  room_503_anim8();
	if (aainfo[4]._val3 != 0)  room_503_anim9();
	if (aainfo[5]._val3 != 0)  room_503_anim10();

	if (scratch._ba == 1) {
		room_503_anim12();
		room_503_anim11();
	} else if (scratch._ba == 2) {
		room_503_anim11();
	}

	if (global[walker_converse_now] != 0)
		global_anim3(scratch._b2, &global[g008]);
}

void room_503_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._pad_a4);
	s.syncAsSint16LE(scratch._a6);
	s.syncAsSint16LE(scratch._pad_a8);
	s.syncAsSint16LE(scratch._pad_aa);
	s.syncAsSint16LE(scratch._pad_ac);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._pad_b0);
	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
	s.syncAsSint16LE(scratch._b6);
	s.syncAsSint16LE(scratch._b8);
	s.syncAsSint16LE(scratch._ba);
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
} // namespace MADSV2
} // namespace MADS
