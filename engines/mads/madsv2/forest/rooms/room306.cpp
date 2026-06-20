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

#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/rooms/room306.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[10];
	AnimationInfo animation_info[10];
	int16 _pad8C[2];
	int16 x90;
	int16 x92;
	int16 x94;
	int16 _pad96[2];
	int16 x9a;
	int16 x9c;
	int16 x9e;
	int16 xa0;
	int16 xa2;
	int16 xa4;
	int16 xa6;
	int16 xa8;
	int16 xaa;
	int16 xac;
	int16 xae;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_306_init1() {
	if (previous_room == KERNEL_RESTORING_GAME) {
		scratch.xaa = -1;
		scratch.x9c = kernel_run_animation_disp('r', 6, 0);
		kernel_position_anim(scratch.x9c, 63, 134, 91, 3);
		global[g143] = 1;
		scratch.x9a = kernel_run_animation_disp('e', 2, 0);
		kernel_position_anim(scratch.x9a, 88, 122, 86, 5);
		global[g133] = 1;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch.x9a, 2);
		kernel_reset_animation(scratch.x9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.walker_visible = -1;
		player.commands_allowed = -1;
	} else {
		global[g131] = 0;
		global[g141] = 0;
		global[g133] = 1;
		global[g143] = 1;
		player.x = 103;
		player.y = 136;
		player.facing = 8;
	}

	if (previous_room == 199) {
		scratch.xaa = -1;
		global[g131] = 0;
		global[g141] = 0;
		scratch.x9c = kernel_run_animation_disp('r', 6, 0);
		kernel_position_anim(scratch.x9c, 63, 134, 91, 3);
		scratch.x9a = kernel_run_animation_disp('e', 2, 0);
		kernel_position_anim(scratch.x9a, 88, 122, 86, 5);
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch.x9a, 2);
		kernel_reset_animation(scratch.x9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = -1;
	}

	if (global[g066] == 3)
		return;

	aa[4] = kernel_run_animation(kernel_name('t', 5), 0);
	aainfo[4]._active = -1;
	aainfo[4]._val3 = 3;
	aa[2] = kernel_run_animation(kernel_name('s', 1), 0);
	aainfo[2]._active = -1;
	aainfo[2]._val3 = (global[g066] == 0) ? 4 : 3;
	aa[5] = kernel_run_animation(kernel_name('t', 7), 0);
	aainfo[5]._active = -1;
	aainfo[5]._val3 = 1;
	if (global[g066] == 0) {
		aa[7] = kernel_run_animation(kernel_name('t', 2), 0);
		aainfo[7]._active = -1;
		aainfo[7]._val3 = 4;
	}

	if (previous_room != KERNEL_RESTORING_GAME && previous_room != 199) {
		aa[6] = kernel_run_animation(kernel_name('y', 1), 104);
	}
}

static void room_306_init2() {
	if (previous_room == KERNEL_RESTORING_GAME) {
		scratch.xaa = -1;
		scratch.x9c = kernel_run_animation_disp('r', 6, 0);
		kernel_position_anim(scratch.x9c, 63, 134, 91, 3);
		global[g143] = 1;
		scratch.x9a = kernel_run_animation_disp('e', 2, 0);
		kernel_position_anim(scratch.x9a, 88, 122, 86, 5);
		global[g133] = 1;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch.x9a, 2);
		kernel_reset_animation(scratch.x9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.walker_visible = -1;
		player.commands_allowed = -1;
	} else {
		global[g131] = 0;
		global[g141] = 0;
		global[g133] = 1;
		global[g143] = 1;
		player.x = 103;
		player.y = 136;
		player.facing = 8;
	}

	if (previous_room == 199) {
		scratch.xaa = -1;
		global[g131] = 0;
		global[g141] = 0;
		scratch.x9c = kernel_run_animation_disp('r', 6, 0);
		kernel_position_anim(scratch.x9c, 63, 134, 91, 3);
		scratch.x9a = kernel_run_animation_disp('e', 2, 0);
		kernel_position_anim(scratch.x9a, 88, 122, 86, 5);
		player.x = 116;
		player.y = 130;
		player.facing = 4;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch.x9a, 2);
		kernel_reset_animation(scratch.x9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = -1;
	}

	if (global[g066] == 3)
		return;

	aa[8] = kernel_run_animation(kernel_name('y', 1), 0);
	aainfo[8]._active = -1;
	aa[3] = kernel_run_animation("*rm306t11", 0);
	aainfo[3]._active = -1;
	aainfo[3]._val3 = 4;
	aa[1] = kernel_run_animation("*rm306t10", 0);
	kernel_reset_animation(aa[1], 60);
	aainfo[1]._active = -1;
	aainfo[1]._val3 = 11;
	aa[4] = kernel_run_animation(kernel_name('t', 5), 0);
	aainfo[4]._active = -1;
	aainfo[4]._val3 = 3;
	aa[2] = kernel_run_animation(kernel_name('s', 1), 0);
	aainfo[2]._active = -1;
	aainfo[2]._val3 = (global[g066] == 0) ? 4 : 3;
	aa[5] = kernel_run_animation(kernel_name('t', 7), 0);
	aainfo[5]._active = -1;
	aainfo[5]._val3 = 1;
}

static void room_306_init3() {
	scratch.xaa = -1;
	global[g131] = 0;
	global[g141] = 0;
	scratch.x9c = kernel_run_animation_disp('r', 6, 0);
	kernel_position_anim(scratch.x9c, 63, 134, 91, 3);
	scratch.x9a = kernel_run_animation_disp('e', 2, 0);
	kernel_position_anim(scratch.x9a, 88, 122, 86, 5);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 116;
		player.y = 130;
		player.facing = 4;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch.x9a, 2);
		kernel_reset_animation(scratch.x9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	}

	if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch.x9a, 2);
		kernel_reset_animation(scratch.x9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = -1;
		return;
	}

	if (previous_room == 305) {
		aa[0] = kernel_run_animation(kernel_name('t', 9), 100);
		aainfo[0]._active = -1;
		scratch.x92 = 50;
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch.x9a, 2);
	kernel_reset_animation(scratch.x9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = -1;
	player.walker_visible = -1;
}

static void room_306_init() {
	scratch.xa2 = 0;
	scratch.xac = -1;
	scratch.xae = -1;
	scratch.xa4 = 0;

	if (global[g066] >= 2)
		kernel_flip_hotspot(180, 0);

	if (previous_room != KERNEL_RESTORING_GAME) {
		scratch.xaa = 0;
		scratch.x9e = -1;

		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
		}

		for (int16 i = 0; i < 10; i++) {
			aainfo[i]._active = 0;
			aainfo[i]._frame = 1;
			aainfo[i]._val3 = 0;
			aainfo[i]._val4 = 0;
		}

		if (previous_room != 199) {
			if (flags[20] != 3)
				flags[20]++;
		}
	}

	switch (flags[20]) {
	case 1:
		room_306_init1();
		break;
	case 2:
		room_306_init2();
		break;
	case 3:
		room_306_init3();
		break;
	}

	switch (global[g066]) {
	case 0:
	case 1:
		scratch.xa0 = 30;
		kernel_timing_trigger(1, 117);
		global[player_score] = 0;
		break;
	case 2:
		kernel_timing_trigger(10, 110);
		global[player_score] = 0;
		break;
	case 3:
		kernel_timing_trigger(1, 118);
		global[player_score] = -1;
		break;
	default:
		break;
	}

	midi_stop();
}

static void room_306_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur != aainfo[0]._frame)
		aainfo[0]._frame = cur;
	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch.x92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_306_anim7() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[6]].frame;
	if (cur != aainfo[6]._frame) {
		aainfo[6]._frame = cur;
		if (cur == 12) {
			if (global[g066] == 3) {
				digi_play_build_ii('b', 4, 1);
				scratch.x9e = 90;
				aainfo[6]._val3 = 12;
			} else {
				digi_play_build(306, 'r', 1, 1);
				scratch.x9e++;
				aainfo[6]._val3 = 6;
			}
		} else if (cur == 39) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[6]);
			aainfo[6]._active = 0;
			scratch.xaa = 666;
			scratch.x9c = kernel_run_animation_disp('r', 6, 0);
			kernel_position_anim(scratch.x9c, 63, 134, 91, 3);
			global[g141] = -1;
			kernel_reset_animation(scratch.x9c, 2);
			kernel_synch(KERNEL_ANIM, scratch.x9c, KERNEL_NOW, 0);
			global[g143] = 0;
			if (!player.been_here_before) {
				scratch.xac = kernel_run_animation_write(0);
				kernel_reset_animation(scratch.xac, 4);
				kernel_position_anim(scratch.xac, 88, 122, 86, 5);
				global[walker_converse_now] = 1;
				global[g007] = 2;
				kernel_synch(KERNEL_ANIM, scratch.xac, KERNEL_NOW, 0);
			} else {
				scratch.xaa = -1;
				scratch.x9a = kernel_run_animation_disp('e', 2, 0);
				kernel_position_anim(scratch.x9a, 88, 122, 86, 5);
				global[g131] = -1;
				kernel_reset_animation(scratch.x9a, 2);
				kernel_synch(KERNEL_ANIM, scratch.x9a, KERNEL_NOW, 0);
				global[g133] = 0;
				player.commands_allowed = -1;
			}
			if (global[g066] != 3) {
				scratch.xa0 = 30;
				kernel_timing_trigger(1, 117);
			} else {
				kernel_timing_trigger(1, 118);
			}
		} else {
			bool dispatch = (cur == 1) ||
			                (cur >= 13 && cur <= 19) ||
			                (cur == 24) ||
			                (cur >= 26 && cur <= 31);
			if (dispatch) {
				switch (aainfo[6]._val3) {
				case 4:  result = 0; break;
				case 6:  result = imath_random(13, 18); break;
				case 7:
					result = 6;
					if (global[g066] == 3)
						aainfo[6]._val3 = 12;
					break;
				case 8:
					result = 22;
					aainfo[6]._val3 = (scratch.x9e == 90) ? 13 : 4;
					break;
				case 12: result = imath_random(26, 30); break;
				case 13:
					result = 31;
					player.walker_visible = -1;
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[6]);
					player_walk(116, 130, 4);
					break;
				default: break;
				}
			}
		}
		if (result >= 0) {
			kernel_reset_animation(aa[6], result);
			aainfo[6]._frame = result;
		}
	}
	if (kernel.trigger == 7 || kernel.trigger == 105) {
		if (scratch.x9e == 90)
			aainfo[6]._val3 = 8;
	}
}

static void room_306_anim8() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[7]].frame;
	if (cur != aainfo[7]._frame) {
		aainfo[7]._frame = cur;
		if (cur >= 1 && cur <= 8) {
			switch (aainfo[7]._val3) {
			case 3:  result = imath_random(2, 7); break;
			case 4:  result = 0; break;
			case 5:  result = 23; break;
			default: break;
			}
		} else if (cur == 24) {
			kernel_abort_animation(aa[7]);
			aainfo[7]._active = 0;
			aa[9] = kernel_run_animation(kernel_name('t', 6), 0);
			aainfo[9]._active = -1;
			aainfo[2]._val3 = 3;
			scratch.xa0 = 100;
			digi_initial_volume(100);
			digi_play_build(305, '_', 1, 3);
			digi_val2 = -1;
		}
		if (result >= 0) {
			kernel_reset_animation(aa[7], result);
			aainfo[7]._frame = result;
		}
	}
	if (kernel.trigger != 7 && kernel.trigger != 105)
		return;
	switch (scratch.x9e) {
	case 10:
		aainfo[7]._val3 = 4;
		kernel_timing_trigger(45, 105);
		scratch.x9e++;
		return;
	case 11:
		digi_play_build(306, 'k', 1, 1);
		scratch.x9e++;
		aainfo[2]._val3 = 3;
		digi_stop(3);
		scratch.xa4 = -1;
		return;
	case 12:
		aainfo[2]._val3 = 4;
		aainfo[7]._val3 = 4;
		kernel_timing_trigger(5, 105);
		scratch.x9e++;
		scratch.xa4 = 0;
		digi_stop(3);
		digi_initial_volume(30);
		digi_play_build(305, '_', 1, 3);
		kernel_timing_trigger(1, 106);
		digi_val2 = -1;
		scratch.xa0 = 30;
		return;
	case 13:
		aainfo[6]._val3 = 7;
		return;
	case 14:
		aainfo[6]._val3 = 8;
		kernel_timing_trigger(45, 105);
		scratch.x9e++;
		return;
	case 15:
		aainfo[6]._val3 = 4;
		digi_stop(3);
		scratch.xa4 = -1;
		digi_play_build(306, 'l', 1, 1);
		scratch.x9e++;
		aainfo[4]._val3 = 16;
		return;
	case 16:
		aainfo[4]._val3 = 3;
		aainfo[7]._val3 = 5;
		kernel_timing_trigger(45, 105);
		scratch.x9e++;
		scratch.xa4 = 0;
		digi_initial_volume(30);
		digi_play_build(305, '_', 1, 3);
		kernel_timing_trigger(1, 106);
		digi_val2 = -1;
		scratch.xa0 = 30;
		return;
	default:
		return;
	}
}

static void room_306_anim9() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[8]].frame;
	if (cur == aainfo[8]._frame)
		return;
	aainfo[8]._frame = cur;
	if (cur == 49) {
		if (scratch.xae == 0)
			kernel_timing_trigger(10, 112);
	} else if (cur == 50) {
		result = 49;
	}
	if (result >= 0) {
		kernel_reset_animation(aa[8], result);
		aainfo[8]._frame = result;
	}
}

static void room_306_anim2() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur == aainfo[1]._frame)
		return;
	aainfo[1]._frame = cur;
	if (cur == 59) {
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		aainfo[3]._val3 = 10;
	} else if (cur == 60) {
		if (aainfo[1]._val3 == 11)
			result = 59;
	}
	if (result >= 0) {
		kernel_reset_animation(aa[1], result);
		aainfo[1]._frame = result;
	}
}

static void room_306_anim3() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur == aainfo[2]._frame)
		return;
	aainfo[2]._frame = cur;
	if (cur >= 1 && cur <= 6) {
		switch (aainfo[2]._val3) {
		case 3:  result = imath_random(1, 2); break;
		case 4:  result = 0; break;
		case 14: result = imath_random(4, 5); break;
		case 15: result = 3; break;
		default: break;
		}
	}
	if (result >= 0) {
		kernel_reset_animation(aa[2], result);
		aainfo[2]._frame = result;
	}
}

static void room_306_anim4() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur == aainfo[3]._frame)
		return;
	aainfo[3]._frame = cur;
	if (cur == 34) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._active = 0;
		global[g066] = 3;
		global[player_score] = -1;
		kernel_timing_trigger(1, 118);
		aainfo[6]._val3 = 7;
	} else if (cur == 1 || cur == 5) {
		switch (aainfo[3]._val3) {
		case 4:
			result = (cur == 1) ? 0 : 4;
			break;
		case 9:
			result = 1;
			aainfo[3]._val3 = 4;
			break;
		case 10:
			result = 6;
			break;
		default:
			break;
		}
	}
	if (result >= 0) {
		kernel_reset_animation(aa[3], result);
		aainfo[3]._frame = result;
	}
}

static void room_306_anim5() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[4]].frame;
	if (cur == aainfo[4]._frame)
		return;
	aainfo[4]._frame = cur;
	if ((cur >= 4 && cur <= 7) || (cur >= 10 && cur <= 16)) {
		if (aainfo[4]._val3 == 3)
			result = imath_random(3, 6);
		else if (aainfo[4]._val3 == 16)
			result = imath_random(9, 15);
	}
	if (result >= 0) {
		kernel_reset_animation(aa[4], result);
		aainfo[4]._frame = result;
	}
}

static void room_306_anim6() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[5]].frame;
	if (cur == aainfo[5]._frame)
		return;
	aainfo[5]._frame = cur;
	if (cur >= 1 && cur <= 7) {
		switch (aainfo[5]._val3) {
		case 0: result = 0; break;
		case 1: result = imath_random(1, 6); break;
		default: break;
		}
	}
	if (result >= 0) {
		kernel_reset_animation(aa[5], result);
		aainfo[5]._frame = result;
	}
}

static void room_306_anim10() {
	int16 result = -1;
	int16 cur = kernel_anim[aa[9]].frame;
	if (cur == aainfo[9]._frame)
		return;
	aainfo[9]._frame = cur;
	if (cur == 28) {
		kernel_abort_animation(aa[9]);
		aainfo[9]._active = 0;
		global[g066] = 1;
		aainfo[6]._val3 = 13;
	} else if (cur == 15) {
		digi_initial_volume(scratch.xa0);
		digi_play_build(305, '_', 1, 3);
		kernel_timing_trigger(1, 107);
		digi_val2 = -1;
	} else if (cur == 20) {
		digi_play_build(203, '_', 1, 2);
	}
	if (result >= 0) {
		kernel_reset_animation(aa[9], result);
		aainfo[9]._frame = result;
	}
}

static void room_306_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = -1;
			if (config_file.forest1 != 0) {
				if (global[g066] == 3) {
					kernel_timing_trigger(1, 118);
				} else {
					scratch.xa0 = 30;
					kernel_timing_trigger(1, 117);
				}
			}
		} else {
			switch (scratch.x9e) {
			case 100:
				kernel_timing_trigger(20, 113);
				global[player_score] = -1;
				break;

			case 2000:
				kernel_abort_animation(scratch.xa6);
				scratch.xa2 = 0;
				scratch.x9e = -1;
				kernel_reset_animation(scratch.x9a, 1);
				kernel_synch(KERNEL_ANIM, scratch.x9a, KERNEL_NOW, 0);
				global[g133] = 0;
				player.commands_allowed = -1;
				if (config_file.forest1 != 0) {
					if (global[g066] == 3) {
						kernel_timing_trigger(1, 118);
					} else {
						scratch.xa0 = 30;
						kernel_timing_trigger(1, 117);
					}
				}
				break;

			case 2500:
				kernel_abort_animation(scratch.xa8);
				scratch.xa2 = 0;
				scratch.x9e = -1;
				player.walker_visible = -1;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				kernel_timing_trigger(20, 114);
				break;

			case 3000:
				aainfo[2]._val3 = 3;
				scratch.xa2 = 0;
				scratch.x9e = -1;
				player.commands_allowed = -1;
				if (config_file.forest1 != 0) {
					if (global[g066] == 3) {
						kernel_timing_trigger(1, 118);
					} else {
						scratch.xa0 = 30;
						kernel_timing_trigger(1, 117);
					}
				}
				break;

			default:
				break;
			}
		}
		break;

	case 24:
		kernel_reset_animation(scratch.x9a, 0);
		global[g133] = 1;
		scratch.xa6 = kernel_run_animation_talk('e', 3, 0);
		kernel_position_anim(scratch.xa6, 88, 122, 86, 5);
		global[g156] = 2;
		kernel_synch(KERNEL_ANIM, scratch.xa6, KERNEL_ANIM, scratch.x9a);
		digi_play_build(306, 'e', 1, 1);
		scratch.x9e = 2000;
		scratch.xa2 = -1;
		if (global[g066] == 3) {
			kernel_timing_trigger(1, 118);
		} else {
			kernel_timing_trigger(1, 117);
		}
		break;

	case 26:
		kernel_reset_animation(scratch.x9a, 0);
		kernel_reset_animation(scratch.x9c, 0);
		global[g133] = 1;
		global[g143] = 1;
		break;

	case 27:
		kernel_abort_animation(scratch.xac);
		scratch.xac = -1;
		scratch.x9a = kernel_run_animation_disp('e', 2, 0);
		kernel_position_anim(scratch.x9a, 88, 122, 86, 5);
		global[g131] = -1;
		kernel_reset_animation(scratch.x9a, 2);
		kernel_synch(KERNEL_ANIM, scratch.x9a, KERNEL_NOW, 0);
		global[g133] = 0;
		scratch.xaa = -1;
		player.commands_allowed = -1;
		scratch.xa0 = 30;
		kernel_timing_trigger(1, 117);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (config_file.forest1 != 0) {
			if (global[g066] == 3) {
				kernel_timing_trigger(1, 118);
			} else {
				scratch.xa0 = 30;
				kernel_timing_trigger(1, 117);
			}
		}
		player.commands_allowed = -1;
		player.walker_visible = -1;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch.x9a, 1);
		kernel_synch(KERNEL_ANIM, scratch.x9a, KERNEL_NOW, 0);
		kernel_reset_animation(scratch.x9c, 1);
		kernel_synch(KERNEL_ANIM, scratch.x9c, KERNEL_NOW, 0);
		global[g133] = 0;
		global[g143] = 0;
		break;

	case 101:
		kernel_abort_animation(aa[scratch.x94]);
		aainfo[scratch.x94]._active = 0;
		kernel_reset_animation(scratch.x9c, 1);
		kernel_synch(KERNEL_ANIM, scratch.x9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = -1;
		if (config_file.forest1 != 0) {
			if (global[g066] == 3) {
				kernel_timing_trigger(1, 118);
			} else {
				scratch.xa0 = 30;
				kernel_timing_trigger(1, 117);
			}
		}
		break;

	case 104:
		kernel_abort_animation(aa[6]);
		kernel_abort_animation(aa[7]);
		aa[6] = kernel_run_animation(kernel_name('t', 4), 0);
		aainfo[6]._active = -1;
		aainfo[6]._val3 = 4;
		aa[7] = kernel_run_animation(kernel_name('t', 2), 0);
		aainfo[7]._active = -1;
		aainfo[7]._val3 = 4;
		digi_play_build(306, 'h', 1, 1);
		aainfo[7]._val3 = 3;
		scratch.x9e = 10;
		digi_stop(3);
		scratch.xa4 = -1;
		break;

	case 106:
		if (scratch.xa0 < 101) {
			scratch.xa0++;
			digi_set_volume(scratch.xa0, 3);
			if (scratch.xa0 < 101)
				kernel_timing_trigger(1, 106);
		}
		break;

	case 107:
		if (scratch.xa0 > 30) {
			scratch.xa0--;
			digi_set_volume(scratch.xa0, 3);
			if (scratch.xa0 > 30)
				kernel_timing_trigger(1, 107);
		}
		break;

	case 110:
		kernel_timing_trigger(1300, 112);
		digi_play_build(306, '_', 999, 1);
		scratch.x9e = 100;
		break;

	case 111:
		if (global[g065] != 0)
			global[g071] = -1;
		kernel_abort_animation(aa[5]);
		aainfo[5]._active = 0;
		kernel_abort_animation(aa[2]);
		aainfo[2]._active = 0;
		kernel_abort_animation(aa[4]);
		aainfo[4]._active = 0;
		kernel_reset_animation(aa[1], 0);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		break;

	case 112:
		aainfo[3]._val3 = 9;
		kernel_timing_trigger(20, 111);
		break;

	case 113:
		kernel_abort_animation(aa[8]);
		aainfo[8]._active = 0;
		aa[6] = kernel_run_animation(kernel_name('t', 4), 0);
		kernel_reset_animation(aa[6], 7);
		aainfo[6]._active = -1;
		aainfo[6]._val3 = 7;
		kernel_timing_trigger(1, 118);
		break;

	case 114:
		aainfo[2]._val3 = 14;
		digi_play_build(306, 'k', 2, 1);
		scratch.x9e = 3000;
		break;

	case 117:
		if (global[g066] <= 1 && scratch.xa4 == 0) {
			digi_initial_volume(scratch.xa0);
			digi_play_build(305, '_', 1, 3);
		}
		break;

	case 118:
		if (global[g066] == 3) {
			digi_initial_volume(15);
			digi_play_build(306, '_', 2, 3);
			digi_val2 = -1;
		}
		break;

	default:
		break;
	}

	if (scratch.xa2 != 0) {
		if (scratch.x9e == 2000) {
			kernel_random_frame(scratch.xa6, &global[g153], global[g156]);
		} else if (scratch.x9e == 2500) {
			kernel_random_frame(scratch.xa8, &global[g151], global[g154]);
		}
	}

	if (scratch.xaa != 0) {
		if (scratch.xaa != 666)
			global_anim1(2, scratch.x9a, global[g131], &global[g132]);
		global_anim2(6, scratch.x9c, global[g141], &global[g142]);
	}

	if (scratch.xac >= 0)
		global_anim3(scratch.xac, &global[g008]);

	if (aainfo[0]._active != 0) room_306_anim1();
	if (aainfo[5]._active != 0) room_306_anim6();
	if (aainfo[6]._active != 0) room_306_anim7();
	if (aainfo[7]._active != 0) room_306_anim8();
	if (aainfo[8]._active != 0) room_306_anim9();
	if (aainfo[9]._active != 0) room_306_anim10();
	if (aainfo[3]._active != 0) room_306_anim4();
	if (aainfo[1]._active != 0) room_306_anim2();
	if (aainfo[2]._active != 0) room_306_anim3();
	if (aainfo[4]._active != 0) room_306_anim5();
}

static void room_306_pre_parser() {
	if (player_parse(13, 38, 0))
		player.walk_off_edge_to_room = 305;
}

static void room_306_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = 0;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (player_parse(180, 0)) {
		aainfo[2]._val3 = 15;
		player.commands_allowed = 0;
		player.walker_visible = 0;
		scratch.xa8 = kernel_run_animation_talk('b', 9, 0);
		kernel_position_anim(scratch.xa8, player.x, player.y, player.scale, player.depth);
		global[g154] = 2;
		kernel_synch(KERNEL_ANIM, scratch.xa8, KERNEL_PLAYER, 0);
		digi_play_build(306, 'b', 1, 1);
		scratch.x9e = 2500;
		scratch.xa2 = -1;
		if (config_file.forest1 == 0) {
			kernel.trigger_setup_mode = 1;
			kernel_timing_trigger(1, 117);
		}
		goto handled;
	}

	if (player_parse(133, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_306_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (auto &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch.x90);
	s.syncAsSint16LE(scratch.x92);
	s.syncAsSint16LE(scratch.x94);
	s.syncAsSint16LE(scratch.x9a);
	s.syncAsSint16LE(scratch.x9c);
	s.syncAsSint16LE(scratch.x9e);
	s.syncAsSint16LE(scratch.xa0);
	s.syncAsSint16LE(scratch.xa2);
	s.syncAsSint16LE(scratch.xa4);
	s.syncAsSint16LE(scratch.xa6);
	s.syncAsSint16LE(scratch.xa8);
	s.syncAsSint16LE(scratch.xaa);
	s.syncAsSint16LE(scratch.xac);
	s.syncAsSint16LE(scratch.xae);
}

void room_306_preload() {
	room_init_code_pointer = room_306_init;
	room_pre_parser_code_pointer = room_306_pre_parser;
	room_parser_code_pointer = room_306_parser;
	room_daemon_code_pointer = room_306_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
