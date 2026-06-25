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
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];            /* 0x00 — sprite series handles  */
	int16 sequence[10];          /* 0x14 — sequence handles       */
	int16 animation[10];         /* 0x28 — animation handles      */
	AnimationInfo animation_info[10]; /* 0x3C */
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
	int16 _aa;                   /* 0xAA — initialised to 0      */
	int16 _ac;
	int16 _ae;                   /* 0xAE — initialised to -1     */
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_106_init1();
static void room_106_init2();
static void room_106_anim1();
static void room_106_anim2();
static void room_106_anim3();
static void room_106_anim4();
static void room_106_anim5();
static void room_106_anim6();
static void room_106_anim7();
static void room_106_anim8();
static void room_106_anim9();
static void room_106_anim10();

static void room_106_init() {
	scratch._aa = 0;
	scratch._ae = -1;

	if (global[g022] != 0)
		kernel_flip_hotspot(words_door, false);

	if (object_is_here(5)) {
		ss[0] = kernel_load_series(kernel_name('p', 2), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 10);
		kernel_seq_loc(seq[0], 235, 99);
		kernel_seq_scale(seq[0], 58);
	} else {
		kernel_flip_hotspot(words_pebbles, false);
	}

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

		if (previous_room != 199 && flags[3] != 3)
			flags[3]++;
	}

	switch (flags[3]) {
	case -3:
		room_106_init1();
		break;
	case -2:
		flags[3] = 1;
		room_106_init2();
		break;
	default:
		room_106_init2();
		break;
	}
}

static void room_106_init1() {
	global[player_score] = -1;
	global[g009] = -1;
	viewing_at_y = 22;
	player.walker_visible = false;
	player.commands_allowed = false;

	for (int16 i = 0; i < 10; i++) {
		aainfo[i]._active = 0;
		aainfo[i]._frame = -1;
	}

	aa[9] = kernel_run_animation(kernel_name('I', 1), 0);
	aainfo[9]._active = -1;
}

static void room_106_init2() {
	midi_stop();
	global[player_score] = -1;
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 1, 0);
	kernel_position_anim(scratch._9c, 242, 84, 52, 11);
	scratch._9a = kernel_run_animation_disp('e', 2, 0);
	kernel_position_anim(scratch._9a, 277, 92, 55, 9);

	if (previous_room != KERNEL_LAST) {
		player.x = 259;
		player.y = 98;
		player.facing = 3;
	}

	switch (previous_room) {
	case KERNEL_LAST:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	case 101:
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 95;
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
	case 201:
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 44;
		return;
	case 210:
		aa[1] = kernel_run_animation("*RM106Y31", 0);
		aainfo[1]._active = -1;
		scratch._92 = 54;
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

static void room_106_anim1() {
	int16 frame = kernel_anim[aa[9]].frame;
	if (frame != aainfo[9]._frame) {
		aainfo[9]._frame = frame;
		if (frame == 121) {
			new_room = 103;
		} else if (frame < 121) {
			if (frame == 61) {
				digi_play_build(106, 'r', 1, 1);
				scratch._ae = 61;
			} else if (frame < 61) {
				if (frame == 1) {
					kernel_timing_trigger(1, 109);
				} else if (frame == 4) {
					digi_play_build(106, 'm', 1, 1);
					scratch._ae = 4;
				} else if (frame == 54) {
					digi_play_build(101, '_', 4, 1);
					scratch._ae = 54;
				}
			} else { // 62..120
				if (frame == 75) {
					digi_stop(3);
				} else if (frame == 83) {
					digi_play_build(106, 'm', 2, 1);
					scratch._ae = 83;
				} else if (frame == 114) {
					digi_play_build(106, 'm', 3, 1);
					scratch._ae = 114;
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;
	if (config_file.forest1 != 0)
		kernel_timing_trigger(1, 109);
	if (scratch._ae == 114) { scratch._ae = -1; return; }
	if (scratch._ae > 114)  return;
	if (scratch._ae == 4 || scratch._ae == 54 || scratch._ae == 61 || scratch._ae == 83)
		scratch._ae = -1;
}

static void room_106_anim2() {
	int16 frame = kernel_anim[aa[6]].frame;
	if (frame == aainfo[6]._frame)
		return;
	aainfo[6]._frame = frame;
	if (scratch._a2 == 1) {
		if (frame == 81) {
			digi_play_build_ii('e', 5, 1);
			scratch._ae = 81;
		}
	} else if (scratch._a2 == 2) {
		if (frame == 75) {
			digi_play_build_ii('e', 1, 1);
			scratch._ae = 75;
		}
	}
}

static void room_106_anim3() {
	int16 frame = kernel_anim[aa[0]].frame;
	if (frame != aainfo[0]._frame)
		aainfo[0]._frame = frame;
	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_106_anim4() {
	int16 frame = kernel_anim[aa[7]].frame;
	if (frame != aainfo[7]._frame) {
		aainfo[7]._frame = frame;
		if (frame == 53) {
			if (aainfo[7]._val3 == 10) {
				aainfo[7]._frame = 48;
				kernel_reset_animation(aa[7], 48);
			}
		} else if (frame < 53) {
			if (frame == 7) {
				if (aainfo[7]._val3 == 13) {
					aainfo[7]._frame = 5;
					kernel_reset_animation(aa[7], 5);
				}
			} else if (frame < 7) {
				if (frame == 1) {
					aainfo[7]._val3 = 10;
					digi_play_build(106, 'b', 1, 1);
					scratch._ae = 101;
				} else if (frame == 4) {
					if (aainfo[7]._val3 == 10) {
						aainfo[7]._frame = 2;
						kernel_reset_animation(aa[7], 2);
					}
				}
			} else { // 8..52
				if (frame == 12) {
					if (aainfo[7]._val3 == 11) {
						aainfo[7]._frame = 8;
						kernel_reset_animation(aa[7], 8);
					}
				} else if (frame == 19) {
					aainfo[7]._val3 = 10;
					global[g154] = 2;
					digi_play_build(106, 'b', 2, 1);
					scratch._ae = 105;
					aainfo[7]._frame = 2;
					kernel_reset_animation(aa[7], 2);
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	switch (scratch._ae) {
	case 101:
		global[g154] = 0;
		kernel_timing_trigger(30, 28);
		aainfo[7]._val3 = 13;
		aainfo[7]._frame = 5;
		kernel_reset_animation(aa[7], 5);
		scratch._ae = 102;
		return;
	case 102:
		global[g156] = 2;
		aainfo[7]._val3 = 11;
		aainfo[7]._frame = 8;
		kernel_reset_animation(aa[7], 8);
		digi_play_build(106, 'e', 1, 1);
		scratch._ae = 103;
		return;
	case 103:
		global[g156] = 0;
		aainfo[7]._val3 = 13;
		aainfo[7]._frame = 13;
		kernel_reset_animation(aa[7], 13);
		return;
	case 105:
		global[g009] = 0;
		global_midi_play(4);
		global[g154] = 0;
		aainfo[7]._val3 = 13;
		aainfo[7]._frame = 20;
		kernel_reset_animation(aa[7], 20);
		return;
	}
}

static void room_106_anim5() {
	int16 frame = kernel_anim[aa[8]].frame;
	if (frame != aainfo[8]._frame) {
		aainfo[8]._frame = frame;
		if (scratch._ac == 2) {
			if (frame == 44) {
				aainfo[8]._val3 = 11;
				digi_play_build(106, 'e', 2, 1);
				scratch._ae = 501;
			} else if (frame == 49) {
				if (aainfo[8]._val3 == 11) {
					aainfo[8]._frame = 47;
					kernel_reset_animation(aa[8], 47);
				}
			}
		}
	}
	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;
	if (scratch._ae == 501) {
		global[g156] = 0;
		aainfo[8]._val3 = 13;
		aainfo[8]._frame = 51;
		kernel_reset_animation(aa[8], 51);
	}
}

static void room_106_anim6() {
	int16 frame = kernel_anim[aa[1]].frame;
	if (frame != aainfo[1]._frame)
		aainfo[1]._frame = frame;

	if (global[player_hyperwalked] != -1 && aainfo[1]._frame != scratch._92)
		return;

	kernel_abort_animation(aa[1]);
	aainfo[1]._active = 0;
	aa[2] = kernel_run_animation("*RM106Y32", 100);
	aainfo[2]._active = -1;
	scratch._92 = 134;
	aainfo[2]._frame = (global[player_hyperwalked] == -1) ? 133 : 54;
	kernel_reset_animation(aa[2], aainfo[2]._frame);
}

static void room_106_anim7() {
	int16 frame = kernel_anim[aa[2]].frame;
	if (frame != aainfo[2]._frame)
		aainfo[2]._frame = frame;
	if (global[player_hyperwalked] == -1) {
		aainfo[2]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[2], aainfo[2]._frame);
	}
}

static void room_106_anim8() {
	int16 frame = kernel_anim[aa[3]].frame;
	if (frame != aainfo[3]._frame)
		aainfo[3]._frame = frame;
	if (global[player_hyperwalked] == -1) {
		aainfo[3]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[3], aainfo[3]._frame);
	}
}

static void room_106_anim9() {
	int16 frame = kernel_anim[aa[5]].frame;
	if (frame == aainfo[5]._frame)
		return;
	aainfo[5]._frame = frame;
	if (frame == 13)
		digi_play_build(101, '_', 1, 2);
}

static void room_106_anim10() {
	int16 frame = kernel_anim[aa[4]].frame;
	if (frame != aainfo[4]._frame)
		aainfo[4]._frame = frame;

	if (global[player_hyperwalked] == -1 && frame <= 23) {
		aainfo[4]._frame = 23;
		kernel_reset_animation(aa[4], 23);
		kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
		return;
	}
	if (global[player_hyperwalked] == -1 || frame == 45) {
		aainfo[4]._frame = 44;
		kernel_reset_animation(aa[4], 44);
		kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
	}
}

static void room_106_daemon() {
	if (flags[0] < 0 && global[player_hyperwalked] == -1) {
		flags[0] = flags[1] = flags[2] = flags[3] = 0;
		new_room = 904;
	}

	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = true;
		} else if (scratch._ae == 75) {
			kernel_timing_trigger(30, 106);
		} else if (scratch._ae == 81) {
			kernel_timing_trigger(30, 105);
		} else if (scratch._ae == 89) {
			kernel_abort_animation(scratch._a6);
			scratch._ae = -1;
			global[g154] = 0;
			scratch._aa = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 8;
		} else if (scratch._ae == 99) {
			kernel_abort_animation(scratch._a6);
			scratch._ae = -1;
			scratch._aa = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 7;
		} else if (scratch._ae == 1000) {
			global[g156] = 0;
			kernel_seq_delete(seq[0]);
			kernel_reset_animation(scratch._9c, 0);
			global[g143] = 1;
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			aa[5] = kernel_run_animation(kernel_name('R', 1), 107);
			aainfo[5]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
		}
		break;
	case 24:
		if (scratch._a2 == 1)
			aa[6] = kernel_run_animation(kernel_name('E', 1), 102);
		else if (scratch._a2 == 2)
			aa[6] = kernel_run_animation(kernel_name('M', 1), 102);
		else
			break;
		aainfo[6]._active = -1;
		kernel_reset_animation(scratch._9a, 0);
		kernel_synch(KERNEL_ANIM, aa[6], KERNEL_ANIM, scratch._9a);
		global[g133] = 1;
		break;
	case 25:
		if (scratch._8e == 2) {
			aa[5] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[5]._active = -1;
			scratch._94 = 5;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[5], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
		}
		break;
	case 26:
		if (scratch._90 == 6) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			global[g133] = 1;
			global[g143] = 1;
			aa[3] = kernel_run_animation(kernel_name('Z', 3), 103);
			aainfo[3]._active = -1;
			scratch._98 = 3;
			scratch._92 = 101;
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[3]);
		} else if (scratch._90 == 7) {
			kernel_reset_animation(scratch._9a, 0);
			global[g133] = 1;
			scratch._a8 = kernel_run_animation_talk('e', 4, 0);
			kernel_position_anim(scratch._a8, 277, 92, 55, 9);
			global[g156] = 2;
			kernel_synch(KERNEL_ANIM, scratch._a8, KERNEL_ANIM, scratch._9a);
			digi_play_build(106, 'E', 3, 1);
			scratch._ae = 1000;
			scratch._aa = -1;
		} else if (scratch._90 == 8) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			global[g133] = 1;
			global[g143] = 1;
			aa[8] = kernel_run_animation(kernel_name('x', 1), 108);
			aainfo[8]._active = -1;
			scratch._ac = 1;
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[8]);
		}
		break;
	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (flags[3] == 1) {
			aa[7] = kernel_run_animation(kernel_name('t', 1), 104);
			aainfo[7]._active = -1;
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
		}
		break;
	case 101:
		kernel_abort_animation(aa[scratch._94]);
		aainfo[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		break;
	case 102:
		kernel_abort_animation(aa[6]);
		aainfo[6]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;
	case 103:
		if (scratch._90 == 6) {
			global[g021] = -1;
			new_room = 210;
		}
		break;
	case 104:
		kernel_abort_animation(aa[7]);
		aainfo[7]._active = 0;
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
		break;
	case 105:
	case 106:
		digi_play_build_ii('e', 10, 1);
		scratch._ae = -1;
		break;
	case 107:
		kernel_abort_animation(aa[5]);
		aainfo[5]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		kernel_abort_animation(scratch._a8);
		scratch._aa = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		kernel_flip_hotspot(words_pebbles, false);
		inter_move_object(5, PLAYER);
		player.commands_allowed = true;
		break;
	case 108:
		if (scratch._ac == 1) {
			kernel_abort_animation(aa[8]);
			aa[8] = kernel_run_animation(kernel_name('x', 2), 108);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[8]);
			scratch._ac = 2;
		} else if (scratch._ac == 2) {
			kernel_abort_animation(aa[8]);
			aa[8] = kernel_run_animation(kernel_name('x', 3), 108);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[8]);
			scratch._ac = 3;
		} else if (scratch._ac == 3) {
			kernel_abort_animation(aa[8]);
			aainfo[8]._active = 0;
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			player.commands_allowed = true;
			global[g133] = 0;
			global[g143] = 0;
			scratch._ae = 0;
			scratch._ac = 0;
		}
		break;
	case 109:
		digi_initial_volume(50);
		digi_play_build(101, '_', 5, 3);
		digi_val2 = -1;
		break;
	}

	if (flags[3] != -3) {
		global_anim1(2, scratch._9a, global[g131], &global[g132]);
		global_anim2(1, scratch._9c, global[g141], &global[g142]);
	}
	if (aainfo[0]._active != 0)  room_106_anim3();
	if (aainfo[1]._active != 0)  room_106_anim6();
	if (aainfo[2]._active != 0)  room_106_anim7();
	if (aainfo[3]._active != 0)  room_106_anim8();
	if (aainfo[5]._active != 0)  room_106_anim9();
	if (aainfo[4]._active != 0)  room_106_anim10();
	if (aainfo[6]._active != 0)  room_106_anim2();
	if (aainfo[9]._active != 0)  room_106_anim1();
	if (aainfo[7]._active != 0)  room_106_anim4();
	if (aainfo[8]._active != 0)  room_106_anim5();

	if (scratch._aa != 0) {
		kernel_random_frame(scratch._a6, &global[g151], global[g154]);
		if (scratch._ae != 99 && scratch._ae != 89) {
			kernel_random_frame(scratch._a4, &global[g152], global[g155]);
			kernel_random_frame(scratch._a8, &global[g153], global[g156]);
		}
	}
}

static void room_106_pre_parser() {
	if (player_parse(words_walk_to, words_room_201, 0))
		player.walk_off_edge_to_room = 201;
}

static void room_106_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_mint, 0)) {
		global[g135] = -1;
		scratch._a2 = 2;
		player.commands_allowed = false;
	}

	if (player_parse(words_primrose, 0)) {
		global[g135] = -1;
		scratch._a2 = 1;
		player.commands_allowed = false;
	}

	if (player_parse(words_pebbles, 0)) {
		global[g154] = 2;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._a6 = kernel_run_animation_talk('b', 3, 0);
		kernel_position_anim(scratch._a6, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 3, 1);
		scratch._ae = 99;
		scratch._aa = -1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_walk_to, words_room_101, 0)) {
		new_room = 101;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_room_210, words_door, 0) && global[g022] == 0) {
		global[g154] = 2;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._a6 = kernel_run_animation_talk('b', 9, 0);
		kernel_position_anim(scratch._a6, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._ae = 89;
		scratch._aa = -1;
		kernel_flip_hotspot(words_door, false);
		global[g022] = -1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_walk_to, words_room_210, 0)) {
		global[g150] = -1;
		scratch._90 = 6;
		player.commands_allowed = false;
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_look_at, words_map, 0)) {
		player.commands_allowed = false;
		global[g145] = -1;
		scratch._8e = 2;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_pick_up, words_flowers, 0) || player_parse(words_click_on, words_moss, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 3;
		player.command_ready = 0;
		return;
	}
}

void room_106_synchronize(Common::Serializer &s) {
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
	s.syncAsSint16LE(scratch._a6);
	s.syncAsSint16LE(scratch._a8);
	s.syncAsSint16LE(scratch._aa);
	s.syncAsSint16LE(scratch._ac);
	s.syncAsSint16LE(scratch._ae);
}

void room_106_preload() {
	room_init_code_pointer = room_106_init;
	room_pre_parser_code_pointer = room_106_pre_parser;
	room_parser_code_pointer = room_106_parser;
	room_daemon_code_pointer = room_106_daemon;

	if (flags[3] == -4)
		global[g016] = -1;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
