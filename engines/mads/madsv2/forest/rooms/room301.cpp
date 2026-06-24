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

#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"

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


static void room_301_init1() {
	global[g131] = 0;
	global[g141] = 0;
	scratch._9c = kernel_run_animation_disp('r', 2, 0);
	kernel_position_anim(scratch._9c, 130, 134, 72, 6);
	scratch._9a = kernel_run_animation_disp('e', 1, 0);
	kernel_position_anim(scratch._9a, 100, 127, 63, 6);

	if (previous_room != KERNEL_LAST) {
		player.x = 164;
		player.y = 132;
		player.facing = 3;
	}

	switch (previous_room) {
	case 221:
	case 101:
	case 205:
	case 302:
	case 305:
	case 307:
	case 322:
		aa[0] = kernel_run_animation(kernel_name('t', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 53;
		player.x = 61;
		player.y = 155;
		player.facing = 2;
		return;

	case 199:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = true;
		return;

	case KERNEL_LAST:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;

	default:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = -1;
		player.walker_visible = true;
		return;
	}
}

static void room_301_init2() {
	global[g131] = 0;
	global[g141] = 0;
	scratch._9c = kernel_run_animation_disp('r', 2, 0);
	kernel_position_anim(scratch._9c, 130, 134, 72, 6);
	scratch._9a = kernel_run_animation_disp('e', 1, 0);
	kernel_position_anim(scratch._9a, 100, 127, 63, 6);

	if (previous_room != KERNEL_LAST) {
		player.x = 164;
		player.y = 132;
		player.facing = 3;
	}

	switch (previous_room) {
	case 302:
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 88;
		return;

	case 205:
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 53;
		return;

	case 305:
		aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
		aainfo[0]._active = -1;
		scratch._92 = 85;
		return;

	case 307:
	case 322:
		aa[0] = kernel_run_animation(kernel_name('y', 4), 100);
		aainfo[0]._active = -1;
		scratch._92 = 149;
		return;

	case 199:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = true;
		return;

	case KERNEL_LAST:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;

	default:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = -1;
		player.walker_visible = true;
		return;
	}
}

static void room_301_init() {
	scratch._aa = 0;
	scratch._a8 = -1;
	scratch._a4 = 0;
	global[g009] = 0;
	midi_stop();

	if (object_is_here(15)) {
		ss[0] = kernel_load_series(kernel_name('p', 2), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, -1);
		kernel_seq_depth(seq[0], 10);
		kernel_seq_loc(seq[0], 195, 124);
		kernel_seq_scale(seq[0], 59);
	} else {
		kernel_flip_hotspot(101, 0);
	}

	if (object_is_here(10)) {
		ss[1] = kernel_load_series(kernel_name('p', 1), 0);
		seq[1] = kernel_seq_stamp(ss[1], false, -1);
		kernel_seq_depth(seq[1], 1);
		kernel_seq_loc(seq[1], 302, 143);
		kernel_seq_scale(seq[1], 85);
	} else {
		kernel_flip_hotspot(163, 0);
	}

	if (previous_room != KERNEL_LAST) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = 0;
		}

		for (int count = 0; count < 10; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame = 0;
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[30] != 3)
			flags[30]++;
	}

	if (flags[30] == 1)
		room_301_init1();
	else
		room_301_init2();
}

static void room_301_anim1() {
	if (kernel_anim[aa[0]].frame == aainfo[0]._frame)
		return;
	aainfo[0]._frame = kernel_anim[aa[0]].frame;
	int16 f = aainfo[0]._frame;

	if (flags[30] == 1) {
		if (f == 2) {
			digi_play_build(301, '_', 2, 1);
		} else if (f == 11) {
			aainfo[0]._frame = 8;
			kernel_reset_animation(aa[0], 8);
		}
	}

	if (flags[30] == 1 && kernel.trigger == 7) {
		aainfo[0]._frame = 12;
		kernel_reset_animation(aa[0], 12);
	}

	if (global[player_hyperwalked] == -1 && flags[30] != 1) {
		int16 reset_frame = scratch._92 - 1;
		aainfo[0]._frame = reset_frame;
		kernel_reset_animation(aa[0], reset_frame);
	}
}

static void room_301_anim2() {
	if (scratch._a4 == 0)
		return;

	kernel_random_frame(scratch._a2, &global[g153], global[g156]);
	kernel_random_frame(scratch._a0, &global[g152], global[g155]);
	kernel_random_frame(scratch._9e, &global[g151], global[g154]);

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	switch (scratch._a4) {
	case 2:
		global[g155] = 0;
		kernel_timing_trigger(30, 28);
		scratch._a4 = 3;
		break;
	case 3:
		scratch._a4 = 4;
		global[g156] = 2;
		digi_play_build(301, 'e', 2, 1);
		break;
	case 4:
		global[g156] = 0;
		kernel_timing_trigger(30, 28);
		scratch._a4 = 5;
		break;
	case 5:
		scratch._a4 = 6;
		global[g154] = 2;
		digi_play_build(301, 'b', 2, 1);
		break;
	case 6:
		global[g154] = 0;
		kernel_timing_trigger(30, 28);
		scratch._a4 = 7;
		break;
	case 7:
		global[g155] = 2;
		scratch._a4 = 8;
		digi_play_build(301, 'r', 3, 1);
		break;
	case 8:
		global[g155] = 0;
		kernel_timing_trigger(30, 28);
		scratch._a4 = 9;
		break;
	case 9:
		kernel_reset_animation(scratch._a2, 0);
		global[g156] = 1;
		scratch._a6 = kernel_run_animation_write(0);
		kernel_reset_animation(scratch._a6, 41);
		kernel_position_anim(scratch._a6, 24, 152, 97, 3);
		global[walker_converse_now] = 1;
		global[g007] = 3;
		kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_NOW, 0);
		kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_NOW, 0);
		scratch._a4 = 10;
		break;
	default:
		break;
	}
}

static void room_301_anim3() {
	if (kernel_anim[aa[2]].frame == aainfo[2]._frame)
		return;
	aainfo[2]._frame = kernel_anim[aa[2]].frame;
}

static void room_301_anim4() {
	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		int16 f = kernel_anim[aa[4]].frame;
		aainfo[4]._frame = f;

		if (scratch._a8 == 1) {
			if (f == 52) {
				if (aainfo[4]._val3 == 13) {
					aainfo[4]._frame = 50;
					kernel_reset_animation(aa[4], 50);
				}
			} else if (f < 52) {
				if (f == 43) {
					aainfo[4]._val3 = 11;
					digi_play_build_ii('e', 7, 1);
					scratch._a4 = 43;
				} else if (f == 48) {
					if (aainfo[4]._val3 == 11) {
						aainfo[4]._frame = 43;
						kernel_reset_animation(aa[4], 43);
					}
				}
			}
		} else if (scratch._a8 == 2) {
			if (f == 13) {
				if (aainfo[4]._val3 == 13) {
					aainfo[4]._frame = 11;
					kernel_reset_animation(aa[4], 11);
				}
			} else if (f < 13) {
				if (f == 5) {
					aainfo[4]._val3 = 11;
					digi_play_build_ii('e', 6, 1);
					scratch._a4 = 500;
				} else if (f == 10) {
					if (aainfo[4]._val3 == 11) {
						aainfo[4]._frame = 5;
						kernel_reset_animation(aa[4], 5);
					}
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	switch (scratch._a4) {
	case 43:
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 13;
		scratch._a4 = 44;
		break;
	case 44:
		aainfo[4]._val3 = 11;
		aainfo[4]._frame = 43;
		kernel_reset_animation(aa[4], 43);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 45;
		break;
	case 45:
		aainfo[4]._val3 = 14;
		aainfo[4]._frame = 50;
		kernel_reset_animation(aa[4], 50);
		scratch._a4 = -1;
		break;
	case 500:
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 13;
		scratch._a4 = 501;
		break;
	case 501:
		aainfo[4]._val3 = 11;
		aainfo[4]._frame = 5;
		kernel_reset_animation(aa[4], 5);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 502;
		break;
	case 502:
		aainfo[4]._val3 = 14;
		aainfo[4]._frame = 11;
		kernel_reset_animation(aa[4], 11);
		scratch._a4 = -1;
		break;
	default:
		break;
	}
}

static void room_301_anim5() {
	if (kernel_anim[aa[5]].frame != aainfo[5]._frame) {
		int16 f = kernel_anim[aa[5]].frame;
		aainfo[5]._frame = f;

		if (scratch._a8 == 1) {
			if (f == 40) {
				if (aainfo[5]._val3 == 13) {
					digi_play_build(101, '_', 1, 2);
					scratch._a4 = 40;
				}
			} else if (f < 40) {
				if (f == 5) {
					aainfo[5]._val3 = 11;
					digi_play_build(301, 'e', 3, 1);
					scratch._a4 = 5;
				} else if (f == 10) {
					if (aainfo[5]._val3 == 11) {
						aainfo[5]._frame = 5;
						kernel_reset_animation(aa[5], 5);
					}
				}
			}
		} else if (scratch._a8 == 2) {
			if (f == 41) {
				aainfo[5]._val3 = 11;
				digi_play_build(301, 'e', 4, 1);
				scratch._a4 = 41;
			} else if (f == 46) {
				if (aainfo[5]._val3 == 11) {
					aainfo[5]._frame = 41;
					kernel_reset_animation(aa[5], 41);
				}
			}
		} else if (scratch._a8 == 3) {
			if (f == 9 && aainfo[5]._val3 == 13) {
				digi_play_build(101, '_', 1, 2);
				scratch._a4 = 9;
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a4 == 5) {
			aainfo[5]._val3 = 13;
			aainfo[5]._frame = 11;
			kernel_reset_animation(aa[5], 11);
			scratch._a4 = -1;
			kernel_seq_delete(seq[0]);
		} else if (scratch._a4 == 41) {
			aainfo[5]._val3 = 13;
			aainfo[5]._frame = 47;
			kernel_reset_animation(aa[5], 47);
			scratch._a4 = -1;
			kernel_seq_delete(seq[1]);
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 28) {
		if (scratch._a4 == 9 || scratch._a4 == 40)
			scratch._a4 = -1;
	}
}

static void room_301_daemon() {
	switch (kernel.trigger) {
	case 25:
		if (scratch._8e == 2) {
			aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[3]._active = -1;
			scratch._94 = 3;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
		}
		break;

	case 24:
		if (scratch._8c == 3) {
			aa[2] = kernel_run_animation(kernel_name('E', 1), 102);
			aainfo[2]._active = -1;
			scratch._96 = 2;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
		} else if (scratch._8c == 5) {
			if (scratch._a8 == 1) {
				aa[4] = kernel_run_animation(kernel_name('F', 2), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			} else if (scratch._a8 == 2) {
				aa[4] = kernel_run_animation(kernel_name('F', 1), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			}
		}
		break;

	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = -1;
		} else if (scratch._a4 == 300) {
			kernel_abort_animation(scratch._9e);
			scratch._a4 = -1;
			scratch._aa = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a8 = 1;
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 6;
		} else if (scratch._a4 == 400) {
			kernel_abort_animation(scratch._9e);
			scratch._a4 = -1;
			scratch._aa = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a8 = 2;
			player.walker_visible = true;
			global[g150] = -1;
			scratch._90 = 6;
		}
		break;

	case 26:
		if (scratch._90 == 1) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
		} else if (scratch._90 == 6) {
			if (scratch._a8 == 1 || scratch._a8 == 2) {
				aa[5] = kernel_run_animation(kernel_name('P', scratch._a8), 103);
				aainfo[5]._active = -1;
				scratch._98 = 5;
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_ANIM, scratch._9a);
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_ANIM, scratch._9c);
			} else if (scratch._a8 == 3) {
				kernel_abort_animation(aa[5]);
				aa[5] = kernel_run_animation(kernel_name('P', 3), 103);
				break;
			} else {
				break;
			}
		} else {
			break;
		}
		global[g133] = 1;
		global[g143] = 1;
		break;

	case 27:
		global[walker_converse_now] = 0;
		kernel_abort_animation(scratch._a6);
		kernel_abort_animation(scratch._a2);
		kernel_abort_animation(scratch._9e);
		kernel_abort_animation(scratch._a0);
		aa[1] = kernel_run_animation(kernel_name('t', 2), 104);
		aainfo[1]._active = -1;
		scratch._a4 = 0;
		break;

	case 103:
		if (scratch._a8 == 1) {
			kernel_abort_animation(aa[5]);
			aainfo[5]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_flip_hotspot(101, 0);
			inter_move_object(15, 2);
			global[player_score] = -1;
			player.commands_allowed = -1;
		} else if (scratch._a8 == 2) {
			kernel_abort_animation(aa[5]);
			aa[5] = kernel_run_animation(kernel_name('P', 3), 103);
			scratch._a8 = 3;
			kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
		} else if (scratch._a8 == 3) {
			kernel_abort_animation(aa[5]);
			aainfo[5]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_flip_hotspot(163, 0);
			inter_move_object(10, 2);
			global[player_score] = -1;
			player.commands_allowed = -1;
		}
		break;

	case 102:
		kernel_abort_animation(aa[4]);
		aainfo[4]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		global[player_score] = -1;
		player.commands_allowed = -1;
		break;

	case 101:
		kernel_abort_animation(aa[scratch._94]);
		aainfo[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = -1;
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (flags[30] == 1) {
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			aainfo[1]._active = -1;
			global[g017] = 0;
			player.walker_visible = false;
			global[g154] = 0;
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g131] = -1;
			global[g133] = 1;
			global[g156] = 0;
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g141] = -1;
			global[g155] = 2;
			global[g143] = 1;
			scratch._a0 = kernel_run_animation_talk('r', 1, 0);
			kernel_position_anim(scratch._a0, 110, 152, 97, 3);
			scratch._9e = kernel_run_animation_talk('b', 3, 0);
			kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
			scratch._a2 = kernel_run_animation_talk('e', 2, 0);
			kernel_position_anim(scratch._a2, 24, 152, 97, 3);
			kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_ANIM, scratch._9c);
			kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
			kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_ANIM, scratch._9a);
			scratch._a4 = 2;
			digi_play_build(301, 'r', 2, 1);
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
			player.commands_allowed = -1;
		}
		break;

	case 104:
		kernel_abort_animation(aa[1]);
		player.x = 164;
		player.y = 132;
		player.facing = 3;
		player.commands_allowed = -1;
		player.walker_visible = true;
		global[g017] = -1;
		aainfo[1]._active = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = -1;
		break;

	default:
		break;
	}

	global_anim1(1, scratch._9a, global[g131], &global[g132]);
	global_anim2(2, scratch._9c, global[g141], &global[g142]);
	if (scratch._aa != 0)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);
	if (aainfo[0]._active != 0)
		room_301_anim1();
	if (aainfo[2]._active != 0)
		room_301_anim3();
	if (aainfo[4]._active != 0)
		room_301_anim4();
	if (aainfo[1]._active != 0)
		room_301_anim2();
	if (aainfo[5]._active != 0)
		room_301_anim5();
	if (global[walker_converse_now] != 0)
		global_anim3(scratch._a6, &global[g008]);
}

static void room_301_pre_parser() {
	if (player_parse(13, 25, 0))
		player.walk_off_edge_to_room = 205;

	if (player_parse(13, 38, 0))
		player.walk_off_edge_to_room = 305;

	if (player_parse(13, 40, 0)) {
		if (global[g064] != 0)
			player.walk_off_edge_to_room = 322;
		else
			player.walk_off_edge_to_room = 307;
	}

	if (player_parse(13, 44, 0))
		player.walk_off_edge_to_room = 322;
}

static void room_301_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = 0;
		digi_play_build_ii('c', 1, 1);
		player.command_ready = 0;
		return;
	}

	if (player_parse(114, 83, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a8 = 1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(114, 146, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a8 = 2;
		player.command_ready = 0;
		return;
	}

	if (player_parse(13, 35, 0)) {
		new_room = 302;
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (player_parse(126, 163, 0)) {
		global[g154] = 2;
		player.commands_allowed = 0;
		player.walker_visible = false;
		scratch._9e = kernel_run_animation_talk('b', 2, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 2, 1);
		scratch._a4 = 400;
		scratch._aa = -1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(126, 101, 0)) {
		global[g154] = 2;
		player.commands_allowed = 0;
		player.walker_visible = false;
		scratch._9e = kernel_run_animation_talk('b', 4, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 2, 1);
		scratch._a4 = 300;
		scratch._aa = -1;
		player.command_ready = 0;
		return;
	}
}

void room_301_synchronize(Common::Serializer &s) {
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

void room_301_preload() {
	room_init_code_pointer = room_301_init;
	room_pre_parser_code_pointer = room_301_pre_parser;
	room_parser_code_pointer = room_301_parser;
	room_daemon_code_pointer = room_301_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
