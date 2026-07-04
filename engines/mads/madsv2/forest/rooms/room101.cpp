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
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/text.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[11];
	AnimationInfo animation_info[11];
	int16 _96;
	int16 _98;
	int16 _9a;
	int16 _9c;
	int16 _9e;
	int16 _a0;
	int16 _a2;
	int16 _edgar;
	int16 _russell;
	int16 _a8;
	int16 _aa;
	int16 _startCutsceneCtr;
	int16 _ae;
	int16 _b0;
	int16 _b2;
	int16 _b4;
	int16 _b6;
};

static Scratch scratch;

#define ss     scratch.sprite
#define seq    scratch.sequence
#define aa     scratch.animation
#define aainfo scratch.animation_info


static void room_101_init1() {
	global[g009] = -1;
	viewing_at_y = 22;
	global[player_score] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;
	mouse_hide();

	for (auto &v : aainfo) {
		v._active = 0;
		v._frame = -1;
	}

	stop_speech_on_run_animation = false;
	aa[8] = kernel_run_animation(kernel_name('I', 1), 0);
	aainfo[8]._active = -1;
	aainfo[8]._frame = 8;
	kernel_reset_animation(aa[8], 8);
}

static void room_101_init2() {
	player.commands_allowed = false;
	global[player_score] = 0;

	ss[3] = kernel_load_series(kernel_name('p', 7), 0);
	seq[3] = kernel_seq_stamp(ss[3], 0, -1);
	kernel_seq_depth(seq[3], 8);
	kernel_seq_loc(seq[3], 172, 119);
	kernel_seq_scale(seq[3], 70);

	global[g131] = 0;
	global[g141] = 0;

	scratch._russell = kernel_run_animation_disp('r', 3, 0);
	kernel_position_anim(scratch._russell, 123, 134, 69, 7);
	scratch._edgar = kernel_run_animation_disp('e', 1, 0);
	kernel_position_anim(scratch._edgar, 194, 145, 75, 4);

	global[g009] = -1;
	global_midi_play(8);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 96;
		player.y = 140;
		player.facing = 3;
	}

	aa[6] = kernel_run_animation(kernel_name('b', 1), 104);
	aainfo[6]._active = -1;
	scratch._9c = 1;
	scratch._startCutsceneCtr = 1;	
}

static void room_101_init3() {
	player.commands_allowed = false;
	global[player_score] = 0;
	global[g009] = 0;
	midi_stop();
	global[g131] = 0;
	global[g141] = 0;

	scratch._russell = kernel_run_animation_disp('r', 3, 0);
	kernel_position_anim(scratch._russell, 123, 134, 69, 7);
	scratch._edgar = kernel_run_animation_disp('e', 1, 0);
	kernel_position_anim(scratch._edgar, 194, 145, 75, 4);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 96;
		player.y = 140;
		player.facing = 3;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._edgar, 2);
		kernel_reset_animation(scratch._russell, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = true;
		return;
	}

	if (previous_room == 104) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._9c = 57;
		return;
	}

	if (previous_room == 106) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._9c = 77;
		return;
	}

	if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._edgar, 2);
		kernel_reset_animation(scratch._russell, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = true;
		player.walker_visible = true;
		return;
	}

	if (previous_room == 205) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._edgar, 2);
		kernel_reset_animation(scratch._russell, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = false;
		player.walker_visible = false;
		global[g009] = 0;
		aainfo[5]._active = -1;
		aa[5] = kernel_run_animation(kernel_name('b', 8), 111);
		scratch._ae = 4;
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._edgar, 2);
	kernel_reset_animation(scratch._russell, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = true;
	player.walker_visible = true;
}

static void room_101_init() {
	scratch._ae = scratch._b0 = scratch._b2 = 0;
	scratch._b4 = scratch._b6 = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (auto &s : aainfo) {
			s._active = 0;
			s._frame = 1;
			s._val3 = s._val4 = 0;
		}

		if (previous_room != 199 && flags[0] != 3)
			++flags[0];
	}

	if (object_is_here(planks)) {
		ss[0] = kernel_load_series("*rm101p11", 0);
		seq[0] = kernel_seq_stamp(ss[0], 0, -1);
		kernel_seq_depth(seq[0], 13);
		kernel_seq_loc(seq[0], 264, 131);
		kernel_seq_scale(seq[0], 68);
	} else {
		kernel_flip_hotspot(words_wood, false);
	}

	if (object_is_here(cogs)) {
		ss[2] = kernel_load_series("*rm101p10", 0);
		seq[2] = kernel_seq_stamp(ss[2], 0, -1);
		kernel_seq_depth(seq[2], 8);
		kernel_seq_loc(seq[2], 159, 116);
		kernel_seq_scale(seq[2], 100);
	} else {
		kernel_flip_hotspot(words_gears, false);
	}

	if (object_is_here(twine)) {
		ss[1] = kernel_load_series(kernel_name('p', 8), 0);
		seq[1] = kernel_seq_stamp(ss[1], 0, -1);
		kernel_seq_depth(seq[1], 1);
		kernel_seq_loc(seq[1], 21, 127);
		kernel_seq_scale(seq[1], 100);
	} else {
		kernel_flip_hotspot(words_twine, false);
	}

	switch (flags[0]) {
	case -3:
		// Intro
		room_101_init1();
		break;
	case -2:
		flags[0] = 1;
		room_101_init2();
		break;
	case 1:
		// Start of game
		room_101_init2();
		break;
	default:
		// Restoring savegame
		room_101_init3();
		break;
	}
}

static void room_101_anim1() {
	int16 frame = kernel_anim[aa[6]].frame;
	if (frame == aainfo[6]._frame)
		return;
	aainfo[6]._frame = frame;

	switch (scratch._startCutsceneCtr) {
	case 2:
		if (frame == 1) {
			aainfo[6]._val3 = 10;
			digi_play_build(101, 'b', 1, 1);
			scratch._aa = 1;
			break;
		}
		if (frame == 6) {
			if (aainfo[6]._val3 != 10) break;
			aainfo[6]._frame = 2;
			kernel_reset_animation(aa[6], 2);
			break;
		}
		if (frame == 10) {
			if (aainfo[6]._val3 != 9) break;
			aainfo[6]._frame = 6;
			kernel_reset_animation(aa[6], 6);
			break;
		}
		if (frame == 14) {
			if (aainfo[6]._val3 != 12) break;
			aainfo[6]._frame = 13;
			kernel_reset_animation(aa[6], 13);
		}
		break;

	case 4:
		if (frame == 10) {
			aainfo[6]._val3 = 9;
			digi_play_build(101, 'r', 2, 1);
			scratch._aa = 10;
			break;
		}
		if (frame == 15) {
			if (aainfo[6]._val3 != 9) break;
			aainfo[6]._frame = 11;
			kernel_reset_animation(aa[6], 11);
		}
		break;

	case 6:
		if (frame == 7) {
			aainfo[6]._val3 = 11;
			digi_play_build(101, 'e', 1, 1);
			scratch._aa = 20;
			break;
		}
		if (frame == 12) {
			if (aainfo[6]._val3 != 11) break;
			aainfo[6]._frame = 8;
			kernel_reset_animation(aa[6], 8);
		}
		break;

	case 9:
		if (frame == 28) {
			aainfo[6]._val3 = 10;
			digi_play_build(101, 'b', 3, 1);
			scratch._aa = 30;
			break;
		}
		if (frame == 30) {
			if (aainfo[6]._val3 != 10) break;
			aainfo[6]._frame = 29;
			kernel_reset_animation(aa[6], 29);
		}
		break;

	case 10:
		if (frame == 88) {
			aainfo[6]._val3 = 10;
			digi_play_build(101, 'b', 4, 1);
			scratch._aa = 40;
			break;
		}
		if (frame == 94) {
			if (aainfo[6]._val3 != 10) break;
			aainfo[6]._frame = 90;
			kernel_reset_animation(aa[6], 90);
		}
		break;

	default:
		break;
	}
}

static void room_101_anim2() {
	int16 frame = kernel_anim[aa[7]].frame;
	if (frame != aainfo[7]._frame) {
		aainfo[7]._frame = frame;
		if (frame == 18 && aainfo[7]._val3 == 7) {
			aainfo[7]._frame = 6;
			kernel_reset_animation(aa[7], 6);
		}
	}
}

static void room_101_anim3() {
	int16 frame = kernel_anim[aa[0]].frame;
	if (frame != aainfo[0]._frame)
		aainfo[0]._frame = frame;
	if (global[player_hyperwalked] != -1) return;
	int16 resetFrame = scratch._9c - 1;
	aainfo[0]._frame = resetFrame;
	kernel_reset_animation(aa[0], resetFrame);
}

static void room_101_anim4() {
	int16 frame = kernel_anim[aa[4]].frame;
	if (frame == aainfo[4]._frame)
		return;
	aainfo[4]._frame = frame;

	switch (aainfo[4]._val3) {
	case 13:
		if (frame == 37)
			digi_play_build(101, '_', 1, 2);
		return;

	case 14:
		if (frame == 26) {
			digi_play_build(101, '_', 1, 2);
			return;
		}
		if (frame > 26) return;
		if (frame == 2) {
			digi_play_build(101, 'e', 2, 1);
			scratch._aa = 50;
			return;
		}
		if (frame == 7) {
			if (scratch._aa != 50) return;
			aainfo[4]._frame = 3;
			kernel_reset_animation(aa[4], 3);
			return;
		}
		if (frame == 9) {
			if (scratch._aa != 50) return;
			aainfo[4]._frame = 8;
			kernel_reset_animation(aa[4], 8);
			return;
		}
		if (frame == 18) {
			if (scratch._aa != 51) return;
			aainfo[4]._frame = 14;
			kernel_reset_animation(aa[4], 14);
		}
		return;

	case 15:
		if (frame == 24) {
			digi_play_build(101, 'e', 4, 1);
			scratch._aa = 0;
			return;
		}
		if (frame == 41)
			digi_play_build(101, '_', 1, 2);
		return;
	}
}

static void room_101_anim5() {
	int16 frame = kernel_anim[aa[2]].frame;
	if (frame != aainfo[2]._frame)
		aainfo[2]._frame = frame;
}

static void room_101_anim6() {
	int16 frame = kernel_anim[aa[1]].frame;
	if (frame != aainfo[1]._frame)
		aainfo[1]._frame = frame;
}

static void room_101_anim7() {
	int16 frame = kernel_anim[aa[5]].frame;
	if (frame == aainfo[5]._frame)
		return;
	aainfo[5]._frame = frame;
	if (frame == 2) {
		if (scratch._ae != 4) return;
		digi_initial_volume(100);
		digi_play_build(101, '_', 500, 2);
		global[g009] = 0;
		return;
	}
	if (frame == 78) {
		if (scratch._ae != 5) return;
		kernel_abort_animation(aa[5]);
		aainfo[5]._active = 0;
		player.x = 96;
		player.y = 140;
		player.facing = 3;
		player.walker_visible = true;
		player.commands_allowed = true;
		mouse_show();
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
	}
}

static void room_101_anim8() {
	int16 frame = kernel_anim[aa[8]].frame;
	if (frame == aainfo[8]._frame)
		return;
	aainfo[8]._frame = frame;
	if (frame != 9) {
		if (frame != 81) return;
		dont_frag_the_palette();
		kernel_abort_animation(aa[8]);
		aainfo[8]._active = 0;
		stop_speech_on_run_animation = false;
		aa[9] = kernel_run_animation(kernel_name('I', 2), 0);
		aainfo[9]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, aa[8]);
	}
	kernel_timing_trigger(1, 113);
}

static void room_101_anim9() {
	int16 frame = kernel_anim[aa[9]].frame;
	if (frame != aainfo[9]._frame) {
		aainfo[9]._frame = frame;

		switch (frame) {
		case 1:
			digi_play_build(101, 'c', 1, 1);
			scratch._b4 = 1;
			break;

		case 67:
			digi_play_build(101, 'e', 5, 1);
			scratch._b4 = 67;
			break;

		case 99:
			scratch._b4 = 99;
			break;

		case 135:
			digi_play_build(101, 'm', 1, 1);
			scratch._b4 = 135;
			break;

		case 163:
			digi_play_build(101, 'b', 6, 1);
			scratch._b4 = 163;
			break;

		case 205:
			dont_frag_the_palette();
			kernel_abort_animation(aa[9]);
			aainfo[9]._active = 0;
			stop_speech_on_run_animation = false;
			aa[10] = kernel_run_animation(kernel_name('I', 3), 0);
			aainfo[10]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[10], KERNEL_ANIM, aa[9]);
			kernel_timing_trigger(1, 113);
			break;

		default:
			break;
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28) return;
	int16 b4 = scratch._b4;
	if (b4 == 1 || b4 == 67 || b4 == 99 || b4 == 135 || b4 == 163)
		scratch._b4 = -1;
}

static void room_101_anim10() {
	int16 frame = kernel_anim[aa[10]].frame;
	if (frame != aainfo[10]._frame) {
		aainfo[10]._frame = frame;
		if (frame == 90) {
			stop_speech_on_run_animation = true;
			kernel_abort_animation(aa[10]);
			aainfo[10]._active = 0;
			new_room = 106;
		} else if (frame < 90) {
			if (frame == 35) {
				scratch._b4 = 35;
			} else if (frame < 35) {
				if (frame == 5)
					global_midi_play(11);
				else if (frame == 30) {
					digi_play_build(101, '_', 3, 1);
					scratch._b4 = 30;
				}
			} else {
				if (frame == 47) {
					digi_play_build(101, 'b', 7, 1);
					scratch._b4 = 47;
				} else if (frame == 70) {
					digi_play_build(101, 'e', 6, 1);
					scratch._b4 = 70;
				} else if (frame == 81) {
					digi_play_build(101, '_', 4, 1);
					scratch._b4 = 81;
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28) return;
	int16 b4 = scratch._b4;
	if (b4 > 81) return;
	if (b4 == 30 || b4 == 35 || b4 == 47 || b4 == 70 || b4 == 81)
		scratch._b4 = -1;
}

static void room_101_daemon() {
	if (flags[0] < 0 && global[player_hyperwalked] == -1) {
		flags[0] = 0;
		flags[1] = 0;
		flags[2] = 0;
		flags[3] = 0;
		new_room = 904;
	}

	switch (kernel.trigger) {
	case 7:
		switch (scratch._aa) {
		case 1:
			aainfo[6]._frame = 13;
			kernel_reset_animation(aa[6], 13);
			aainfo[6]._val3 = 12;
			kernel_timing_trigger(25, 7);
			scratch._aa++;
			break;
		case 2:
			aainfo[6]._frame = 6;
			kernel_reset_animation(aa[6], 6);
			aainfo[6]._val3 = 9;
			digi_play_build(101, 'r', 1, 1);
			scratch._aa++;
			break;
		case 3:
			aainfo[6]._frame = 13;
			kernel_reset_animation(aa[6], 13);
			aainfo[6]._val3 = 12;
			kernel_timing_trigger(30, 106);
			break;
		case 4:
			aainfo[6]._frame = 12;
			kernel_reset_animation(aa[6], 12);
			aainfo[6]._val3 = 0;
			scratch._aa++;
			break;
		case 10:
			aainfo[6]._frame = 15;
			kernel_reset_animation(aa[6], 15);
			aainfo[6]._val3 = 0;
			scratch._aa++;
			break;
		case 20:
			aainfo[6]._frame = 13;
			kernel_reset_animation(aa[6], 13);
			aainfo[6]._val3 = 0;
			scratch._aa++;
			break;
		case 29:
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
			break;
		case 30:
			aainfo[6]._frame = 31;
			kernel_reset_animation(aa[6], 31);
			aainfo[6]._val3 = 0;
			scratch._aa++;
			break;
		case 40:
			aainfo[6]._frame = 95;
			kernel_reset_animation(aa[6], 95);
			aainfo[6]._val3 = 0;
			scratch._aa++;
			break;
		case 49:
			kernel_abort_animation(scratch._a8);
			scratch._b2 = 0;
			kernel_reset_animation(scratch._edgar, 0);
			kernel_reset_animation(scratch._russell, 0);
			global[g133] = 1;
			global[g143] = 1;
			inter_move_object(cogs, PLAYER);
			kernel_seq_delete(seq[2]);
			kernel_flip_hotspot(words_gears, false);
			aa[4] = kernel_run_animation(kernel_name('R', 5), 103);
			aainfo[4]._active = -1;
			aainfo[4]._val3 = 14;
			scratch._a2 = 4;
			kernel_synch(KERNEL_ANIM, scratch._edgar, KERNEL_ANIM, aa[4]);
			break;
		case 50:
			aainfo[4]._frame = 8;
			kernel_reset_animation(aa[4], 8);
			kernel_timing_trigger(45, 107);
			break;
		case 51:
			aainfo[4]._frame = 19;
			kernel_reset_animation(aa[4], 19);
			break;
		case 59:
			kernel_abort_animation(scratch._a8);
			scratch._b2 = 0;
			kernel_reset_animation(scratch._edgar, 0);
			kernel_reset_animation(scratch._russell, 0);
			global[g133] = 1;
			global[g143] = 1;
			inter_move_object(planks, PLAYER);
			kernel_seq_delete(seq[0]);
			kernel_flip_hotspot(words_wood, false);
			aa[4] = kernel_run_animation(kernel_name('R', 6), 103);
			aainfo[4]._active = -1;
			aainfo[4]._val3 = 15;
			scratch._a2 = 4;
			kernel_synch(KERNEL_ANIM, scratch._edgar, KERNEL_ANIM, aa[4]);
			break;
		case 69:
			global[g154] = 0;
			kernel_abort_animation(scratch._a8);
			player.walker_visible = true;
			scratch._b2 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			kernel_timing_trigger(45, 109);
			break;
		default:
			break;
		}
		break;

	case 24:
		if (scratch._96 == 3) {
			aa[1] = kernel_run_animation(kernel_name('E', 1), 102);
			aainfo[1]._active = -1;
			scratch._a0 = 1;
			kernel_reset_animation(scratch._edgar, 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, scratch._edgar);
			global[g133] = 1;
		}
		break;

	case 25:
		if (scratch._98 == 2) {
			aa[2] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[2]._active = -1;
			scratch._9e = 2;
			kernel_reset_animation(scratch._russell, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._russell);
			global[g143] = 1;
		}
		break;

	case 26:
		switch (scratch._9a) {
		case 15:
			scratch._aa = 59;
			global[g154] = 2;
			player.commands_allowed = false;
			scratch._a8 = kernel_run_animation_talk('b', 7, 0);
			kernel_position_anim(scratch._a8, player.x, player.y, player.scale, player.depth);
			scratch._b2 = -1;
			kernel_synch(KERNEL_ANIM, scratch._a8, KERNEL_PLAYER, 0);
			digi_play_build_ii('b', 2, 1);
			break;
		case 14:
			scratch._aa = 49;
			global[g154] = 2;
			player.commands_allowed = false;
			player.walker_visible = false;
			scratch._a8 = kernel_run_animation_talk('b', 6, 0);
			kernel_position_anim(scratch._a8, player.x, player.y, player.scale, player.depth);
			scratch._b2 = -1;
			kernel_synch(KERNEL_ANIM, scratch._a8, KERNEL_PLAYER, 0);
			digi_play_build_ii('b', 1, 1);
			break;

		case 13:
			scratch._aa = 69;
			player.commands_allowed = false;
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g154] = 2;
			scratch._a8 = kernel_run_animation_talk('b', 4, 0);
			kernel_position_anim(scratch._a8, player.x, player.y, player.scale, player.depth);
			scratch._b2 = -1;
			kernel_synch(KERNEL_ANIM, scratch._a8, KERNEL_PLAYER, 0);
			digi_play_build_ii('b', 2, 1);
			break;

		default:
			break;
		}
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._edgar, 1);
		kernel_synch(KERNEL_ANIM, scratch._edgar, KERNEL_NOW, 0);
		kernel_reset_animation(scratch._russell, 1);
		kernel_synch(KERNEL_ANIM, scratch._russell, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		break;

	case 101:
		kernel_abort_animation(aa[scratch._9e]);
		aainfo[scratch._9e]._active = 0;
		kernel_reset_animation(scratch._russell, 1);
		kernel_synch(KERNEL_ANIM, scratch._russell, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		break;

	case 102:
		kernel_abort_animation(aa[scratch._a0]);
		aainfo[scratch._a0]._active = 0;
		kernel_reset_animation(scratch._edgar, 1);
		kernel_synch(KERNEL_ANIM, scratch._edgar, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 103:
		kernel_abort_animation(aa[4]);
		aainfo[4]._active = 0;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._edgar, 1);
		kernel_synch(KERNEL_ANIM, scratch._edgar, KERNEL_NOW, 0);
		kernel_reset_animation(scratch._russell, 1);
		kernel_synch(KERNEL_ANIM, scratch._russell, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		break;

	case 104:
		// First cutscene at start of game
		switch (scratch._startCutsceneCtr) {
		case 1:
			kernel_abort_animation(aa[6]);
			aa[6] = kernel_run_animation(kernel_name('b', 2), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 2:
			kernel_abort_animation(aa[6]);
			aa[6] = kernel_run_animation(kernel_name('b', 3), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 3:
			kernel_abort_animation(aa[6]);
			aa[6] = kernel_run_animation(kernel_name('r', 1), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 4:
			kernel_abort_animation(aa[6]);
			aa[7] = kernel_run_animation(kernel_name('r', 2), 105);
			aainfo[7]._active = -1;
			aainfo[7]._val3 = 7;
			scratch._9c = 1;
			aa[6] = kernel_run_animation(kernel_name('e', 1), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 5:
			kernel_seq_delete(seq[3]);
			kernel_flip_hotspot(words_journal, false);
			kernel_abort_animation(aa[6]);
			aa[6] = kernel_run_animation(kernel_name('e', 2), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 6:
			kernel_abort_animation(aa[6]);
			global[g131] = -1;
			kernel_reset_animation(scratch._edgar, 1);
			kernel_synch(KERNEL_ANIM, scratch._edgar, KERNEL_NOW, 0);
			global[g133] = 0;
			aa[6] = kernel_run_animation(kernel_name('b', 4), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 7:
			kernel_abort_animation(aa[6]);
			aa[6] = kernel_run_animation(kernel_name('b', 5), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 8:
			kernel_abort_animation(aa[6]);
			aa[6] = kernel_run_animation(kernel_name('b', 6), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 9:
			kernel_abort_animation(aa[6]);
			aa[6] = kernel_run_animation(kernel_name('b', 9), 104);
			aainfo[6]._active = -1;
			scratch._9c = 1;
			scratch._startCutsceneCtr++;
			break;
		case 10:
			aainfo[7]._val3 = 8;
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			flags[0] = 2;
			flags[1] = 0;
			flags[2] = 0;
			flags[3] = 0;
			break;
		default:
			break;
		}
		break;

	case 105:
		if (aainfo[7]._val3 == 0) {
			aainfo[7]._active = 0;
			global[g141] = -1;
			kernel_reset_animation(scratch._russell, 1);
			kernel_synch(KERNEL_ANIM, scratch._russell, KERNEL_NOW, 0);
			global[g143] = 0;
			global[g009] = 0;
			player.commands_allowed = true;
		} else if (aainfo[7]._val3 == 8) {
			kernel_abort_animation(aa[7]);
			aa[7] = kernel_run_animation(kernel_name('r', 3), 105);
			aainfo[7]._active = -1;
			scratch._9c = 1;
			aainfo[7]._val3 = 0;
		}
		break;

	case 106:
		aainfo[6]._frame = 3;
		kernel_reset_animation(aa[6], 3);
		aainfo[6]._val3 = 10;
		digi_play_build(101, 'b', 2, 1);
		scratch._aa++;
		break;

	case 107:
		aainfo[4]._frame = 13;
		kernel_reset_animation(aa[4], 13);
		digi_play_build(101, 'B', 5, 1);
		scratch._aa++;
		break;

	case 109:
		kernel_reset_animation(scratch._edgar, 0);
		kernel_reset_animation(scratch._russell, 0);
		global[g133] = 1;
		global[g143] = 1;
		inter_move_object(twine, PLAYER);
		kernel_seq_delete(seq[1]);
		kernel_flip_hotspot(words_twine, false);
		aa[4] = kernel_run_animation(kernel_name('R', 4), 103);
		aainfo[4]._active = -1;
		aainfo[4]._val3 = 13;
		scratch._a2 = 4;
		kernel_synch(KERNEL_ANIM, scratch._edgar, KERNEL_ANIM, aa[4]);
		digi_play_build(101, 'e', 3, 1);
		scratch._aa = 0;
		break;

	case 111:
		switch (scratch._ae) {
		case 1:
			kernel_abort_animation(aa[5]);
			aa[5] = kernel_run_animation(kernel_name('b', 7), 111);
			scratch._ae = 2;
			break;
		case 2:
			kernel_abort_animation(aa[5]);
			aainfo[5]._active = 0;
			new_room = 205;
			break;
		case 4:
			kernel_abort_animation(aa[5]);
			aa[5] = kernel_run_animation(kernel_name('b', 9), 111);
			scratch._ae = 5;
			break;
		default:
			break;
		}
		break;

	case 113:
		digi_initial_volume(90);
		digi_play_build(101, '_', 5, 3);
		digi_val2 = -1;
		break;

	}

	if (kernel.trigger == 9 && flags[0] != -3)
		scratch._b0 = 0;

	if (scratch._b0 == 0 && flags[0] != -3) {
		if (imath_random(1, 150) == 1) {
			digi_initial_volume(25);
			digi_play_build(104, '_', 1, 3);
			scratch._b0 = -1;
		}
	}

	if (flags[0] != -3) {
		global_anim1(1, scratch._edgar, global[g131], &global[g132]);
		global_anim2(3, scratch._russell, global[g141], &global[g142]);
		if (mouse_hidden != 0)
			mouse_show();
	}

	if (aainfo[6]._active)  room_101_anim1();
	if (aainfo[7]._active)  room_101_anim2();
	if (aainfo[0]._active)  room_101_anim3();
	if (aainfo[4]._active)  room_101_anim4();
	if (aainfo[2]._active)  room_101_anim5();
	if (aainfo[1]._active)  room_101_anim6();
	if (aainfo[5]._active)  room_101_anim7();
	if (aainfo[8]._active)  room_101_anim8();
	if (aainfo[9]._active)  room_101_anim9();
	if (aainfo[10]._active) room_101_anim10();

	if (scratch._b2)
		kernel_random_frame(scratch._a8, &global[g151], global[g154]);
}

static void room_101_pre_parser() {
	if (player_parse(words_walk_to, words_room_104, 0))
		player.walk_off_edge_to_room = 104;

	if (player_parse(words_walk_to, words_room_106, 0))
		player.walk_off_edge_to_room = 106;
}

static void room_101_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		scratch._aa = 29;
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(words_look_at, words_map, 0)) {
		player.commands_allowed = false;
		global[g145] = -1;
		scratch._98 = 2;
		goto handled;
	}

	if (player_parse(words_pick_up, words_twine, 0)) {
		player.commands_allowed = false;
		global[g150] = -1;
		scratch._9a = 13;
		goto handled;
	}

	if (player_parse(words_pick_up, words_gears, 0)) {
		player.commands_allowed = false;
		global[g150] = -1;
		scratch._9a = 14;
		goto handled;
	}

	if (player_parse(words_pick_up, words_wood, 0)) {
		player.commands_allowed = false;
		global[g150] = -1;
		scratch._9a = 15;
		goto handled;
	}

	if (player_parse(words_walk_to, words_telescope, 0)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel.trigger_setup_mode = 1;
		aainfo[5]._active = -1;
		aa[5] = kernel_run_animation(kernel_name('b', 5), 111);
		scratch._ae = 1;
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_101_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)     s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)   s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)  s.syncAsSint16LE(v);
	for (auto  &v : scratch.animation_info) v.synchronize(s);
	s.syncAsSint16LE(scratch._96);
	s.syncAsSint16LE(scratch._98);
	s.syncAsSint16LE(scratch._9a);
	s.syncAsSint16LE(scratch._9c);
	s.syncAsSint16LE(scratch._9e);
	s.syncAsSint16LE(scratch._a0);
	s.syncAsSint16LE(scratch._a2);
	s.syncAsSint16LE(scratch._edgar);
	s.syncAsSint16LE(scratch._russell);
	s.syncAsSint16LE(scratch._a8);
	s.syncAsSint16LE(scratch._aa);
	s.syncAsSint16LE(scratch._startCutsceneCtr);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._b0);
	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
	s.syncAsSint16LE(scratch._b6);
}

void room_101_preload() {
	room_init_code_pointer = room_101_init;
	room_pre_parser_code_pointer = room_101_pre_parser;
	room_parser_code_pointer = room_101_parser;
	room_daemon_code_pointer = room_101_daemon;

	if (flags[0] == -4)
		global[g016] = -1;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
