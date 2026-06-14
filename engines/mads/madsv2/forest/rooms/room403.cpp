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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/digi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/midi.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/rooms/room403.h"

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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_403_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 1, 0);
	kernel_position_anim(scratch._9c, 159, 99, 81, 12);
	scratch._9a = kernel_run_animation_disp('e', 3, 0);
	kernel_position_anim(scratch._9a, 129, 98, 81, 12);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 104;
		player.y = 103;
		player.facing = 3;
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
		player.commands_allowed = -1;
		player.walker_visible = -1;
		return;
	}

	if (previous_room == 402) {
		if (flags[28] == 1) {
			aa[6] = kernel_run_animation(kernel_name('t', 1), 104);
			aainfo[6]._active = -1;
		} else {
			aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
			aainfo[0]._active = -1;
		}
		scratch._92 = 72;
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = -1;
	player.walker_visible = -1;
}

static void room_403_init() {
	scratch._a0 = 0;
	scratch._a2 = -1;
	midi_stop();
	global[g009] = 0;
	global[player_score] = -1;
	kernel_timing_trigger(1, 105);

	if (object_is_here(13)) {
		ss[0] = kernel_load_series(kernel_name('p', 2), 0);
		seq[0] = kernel_seq_stamp(ss[0], 0, -1);
		kernel_seq_depth(seq[0], 15);
		kernel_seq_loc(seq[0], 276, 154);
		kernel_seq_scale(seq[0], 99);
	} else {
		kernel_flip_hotspot(166, 0);
	}

	if (global[g080] == 0) {
		ss[1] = kernel_load_series(kernel_name('p', 1), 0);
		seq[1] = kernel_seq_stamp(ss[1], -1, -1);
		kernel_seq_depth(seq[1], 6);
		kernel_seq_loc(seq[1], 51, 154);
		kernel_seq_scale(seq[1], 100);
	} else {
		kernel_flip_hotspot(95, 0);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
		}

		for (int16 count = 0; count < 10; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame = 1;
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[28] != 3)
			flags[28]++;
	}

	room_403_init1();
}

static void room_403_anim1() {
	if (kernel_anim[aa[0]].frame == aainfo[0]._frame)
		return;
	aainfo[0]._frame = kernel_anim[aa[0]].frame;
	if (global[player_hyperwalked] != -1)
		return;
	int16 ax = scratch._92 - 1;
	aainfo[0]._frame = ax;
	kernel_reset_animation(aa[0], ax);
}

static void room_403_anim2() {
	if (kernel_anim[aa[6]].frame != aainfo[6]._frame) {
		int16 frame = kernel_anim[aa[6]].frame;
		aainfo[6]._frame = frame;
		if (frame == 75) {
			digi_play_build(403, 'e', 1, 1);
			scratch._a4 = 1;
		} else if (frame == 80) {
			aainfo[6]._frame = 76;
			kernel_reset_animation(aa[6], 76);
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a4 == 1) {
			kernel_timing_trigger(30, 28);
			scratch._a4 = 0;
			aainfo[6]._frame = 81;
			kernel_reset_animation(aa[6], 81);
		}
	}

	if (global[player_hyperwalked] != -1)
		return;
	int16 ax = scratch._92 - 1;
	if (ax <= aainfo[6]._frame)
		return;
	aainfo[6]._frame = ax;
	kernel_reset_animation(aa[6], ax);
}

static void room_403_anim3() {
	if (kernel_anim[aa[1]].frame == aainfo[1]._frame)
		return;
	aainfo[1]._frame = kernel_anim[aa[1]].frame;
}

static void room_403_anim4() {
	if (kernel_anim[aa[3]].frame != aainfo[3]._frame) {
		int16 ax = kernel_anim[aa[3]].frame;
		aainfo[3]._frame = ax;

		if (scratch._a2 == 1) {
			if (ax == 28) {
				aainfo[3]._val3 = 21;
				digi_play_build_ii('e', 3, 1);
				scratch._a4 = 28;
			} else if (ax == 33) {
				if (aainfo[3]._val3 == 21) {
					aainfo[3]._frame = 28;
					kernel_reset_animation(aa[3], 28);
				}
			} else if (ax == 37) {
				if (aainfo[3]._val3 == 23) {
					aainfo[3]._frame = 34;
					kernel_reset_animation(aa[3], 34);
				}
			}
		} else if (scratch._a2 == 2) {
			if (ax == 45) {
				aainfo[3]._val3 = 21;
				digi_play_build_ii('e', 7, 1);
				scratch._a4 = 450;
			} else if (ax == 50) {
				if (aainfo[3]._val3 == 21) {
					aainfo[3]._frame = 45;
					kernel_reset_animation(aa[3], 45);
				}
			} else if (ax == 54) {
				if (aainfo[3]._val3 == 23) {
					aainfo[3]._frame = 51;
					kernel_reset_animation(aa[3], 51);
				}
			}
		} else if (scratch._a2 == 3) {
			if (ax == 49) {
				aainfo[3]._val3 = 21;
				digi_play_build_ii('e', 5, 1);
				scratch._a4 = 4900;
			} else if (ax == 54) {
				if (aainfo[3]._val3 == 21) {
					aainfo[3]._frame = 49;
					kernel_reset_animation(aa[3], 49);
				}
			} else if (ax == 57) {
				if (aainfo[3]._val3 == 23) {
					aainfo[3]._frame = 55;
					kernel_reset_animation(aa[3], 55);
				}
			}
		} else if (scratch._a2 == 4) {
			if (ax == 45) {
				aainfo[3]._val3 = 21;
				digi_play_build_ii('e', 6, 1);
				scratch._a4 = 45;
			} else if (ax == 50) {
				if (aainfo[3]._val3 == 21) {
					aainfo[3]._frame = 45;
					kernel_reset_animation(aa[3], 45);
				}
			} else if (ax == 54) {
				if (aainfo[3]._val3 == 23) {
					aainfo[3]._frame = 51;
					kernel_reset_animation(aa[3], 51);
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	int16 a4 = scratch._a4;
	if (a4 == 28) {
		kernel_timing_trigger(30, 28);
		aainfo[3]._val3 = 23;
		scratch._a4 = 29;
	} else if (a4 == 29) {
		aainfo[3]._val3 = 21;
		aainfo[3]._frame = 28;
		kernel_reset_animation(aa[3], 28);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 30;
	} else if (a4 == 30) {
		aainfo[3]._val3 = 19;
		aainfo[3]._frame = 34;
		kernel_reset_animation(aa[3], 34);
		scratch._a4 = -1;
	} else if (a4 == 45) {
		kernel_timing_trigger(30, 28);
		aainfo[3]._val3 = 23;
		scratch._a4 = 46;
	} else if (a4 == 46) {
		aainfo[3]._val3 = 21;
		aainfo[3]._frame = 45;
		kernel_reset_animation(aa[3], 45);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 47;
	} else if (a4 == 47) {
		aainfo[3]._val3 = 19;
		aainfo[3]._frame = 51;
		kernel_reset_animation(aa[3], 51);
		scratch._a4 = -1;
	} else if (a4 == 450) {
		kernel_timing_trigger(30, 28);
		aainfo[3]._val3 = 23;
		scratch._a4 = 451;
	} else if (a4 == 451) {
		aainfo[3]._val3 = 21;
		aainfo[3]._frame = 45;
		kernel_reset_animation(aa[3], 45);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 452;
	} else if (a4 == 452) {
		aainfo[3]._val3 = 19;
		aainfo[3]._frame = 51;
		kernel_reset_animation(aa[3], 51);
		scratch._a4 = -1;
	} else if (a4 == 4900) {
		kernel_timing_trigger(30, 28);
		aainfo[3]._val3 = 23;
		scratch._a4 = 4901;
	} else if (a4 == 4901) {
		aainfo[3]._val3 = 21;
		aainfo[3]._frame = 49;
		kernel_reset_animation(aa[3], 49);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 4902;
	} else if (a4 == 4902) {
		aainfo[3]._val3 = 19;
		aainfo[3]._frame = 55;
		kernel_reset_animation(aa[3], 55);
		scratch._a4 = -1;
	}
}

static void room_403_anim5() {
	if (kernel_anim[aa[2]].frame == aainfo[2]._frame)
		return;
	aainfo[2]._frame = kernel_anim[aa[2]].frame;
}

static void room_403_anim6() {
	if (kernel_anim[aa[5]].frame != aainfo[5]._frame) {
		int16 ax = kernel_anim[aa[5]].frame;
		aainfo[5]._frame = ax;

		if (scratch._a2 == 1) {
			if (ax == 5) {
				aainfo[5]._val3 = 21;
				digi_play_build(403, 'e', 3, 1);
				scratch._a4 = 5;
			} else if (ax == 10) {
				if (aainfo[5]._val3 == 21) {
					aainfo[5]._frame = 5;
					kernel_reset_animation(aa[5], 5);
				}
			} else if (ax == 42) {
				if (aainfo[5]._val3 == 23) {
					digi_play_build(101, '_', 1, 2);
					scratch._a6 = 42;
				}
			}
		} else if (scratch._a2 == 2) {
			if (ax == 6) {
				aainfo[5]._val3 = 21;
				digi_play_build(403, 'e', 2, 1);
				scratch._a4 = 60;
			} else if (ax == 11) {
				if (aainfo[5]._val3 == 21) {
					aainfo[5]._frame = 6;
					kernel_reset_animation(aa[5], 6);
				}
			} else if (ax == 53) {
				if (aainfo[5]._val3 == 23) {
					digi_play_build(101, '_', 1, 2);
					scratch._a6 = 53;
				}
			} else if (ax == 89) {
				aainfo[5]._val3 = 21;
				digi_play_build(403, 'b', 1, 1);
				scratch._a4 = 89;
			} else if (ax == 95) {
				if (aainfo[5]._val3 == 21) {
					aainfo[5]._frame = 89;
					kernel_reset_animation(aa[5], 89);
				}
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		int16 a4 = scratch._a4;
		if (a4 == 5) {
			aainfo[5]._val3 = 23;
			aainfo[5]._frame = 11;
			kernel_reset_animation(aa[5], 11);
			scratch._a4 = -1;
			kernel_seq_delete(seq[0]);
		} else if (a4 == 60) {
			aainfo[5]._val3 = 23;
			aainfo[5]._frame = 12;
			kernel_reset_animation(aa[5], 12);
			scratch._a4 = -1;
			global_digi_play(14);
			kernel_seq_delete(seq[1]);
			global[g080] = -1;
		} else if (a4 == 89) {
			aainfo[5]._val3 = 23;
			aainfo[5]._frame = 96;
			kernel_reset_animation(aa[5], 96);
			scratch._a4 = -1;
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 28) {
		int16 a6 = scratch._a6;
		if (a6 == 42 || a6 == 53)
			scratch._a6 = -1;
	}
}

static void room_403_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = -1;
			if (digi_val1 != 0)
				kernel_timing_trigger(1, 105);
		} else if (scratch._a4 == 300) {
			kernel_abort_animation(scratch._9e);
			scratch._a4 = -1;
			scratch._a0 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a2 = 2;
			player.walker_visible = -1;
			global[g150] = -1;
			scratch._90 = 6;
		} else if (scratch._a4 == 400) {
			kernel_abort_animation(scratch._9e);
			scratch._a4 = -1;
			scratch._a0 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a2 = 1;
			player.walker_visible = -1;
			global[g150] = -1;
			scratch._90 = 6;
		}
		break;

	case 17: {
		if (scratch._8c == 3) {
			aa[1] = kernel_run_animation(kernel_name('E', 1), 102);
			aainfo[1]._active = -1;
			scratch._96 = 1;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
			kernel_timing_trigger(1, 105);
		} else if (scratch._8c == 5) {
			int16 fnum;
			if (scratch._a2 == 1)      fnum = 3;
			else if (scratch._a2 == 2) fnum = 2;
			else if (scratch._a2 == 3) fnum = 4;
			else if (scratch._a2 == 4) fnum = 5;
			else break;
			aa[3] = kernel_run_animation(kernel_name('F', fnum), 102);
			aainfo[3]._active = -1;
			scratch._96 = 3;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
			kernel_timing_trigger(1, 105);
		}
		break;
	}

	case 18:
		if (scratch._8e == 2) {
			aa[2] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[2]._active = -1;
			scratch._94 = 2;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
			kernel_timing_trigger(1, 105);
		}
		break;

	case 19:
		if (scratch._90 == 1) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._90 == 6) {
			if (scratch._a2 == 1) {
				aa[5] = kernel_run_animation(kernel_name('P', 1), 103);
				aainfo[5]._active = -1;
				scratch._98 = 5;
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[5]);
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[5]);
				global[g133] = 1;
				global[g143] = 1;
				kernel_timing_trigger(1, 105);
			} else if (scratch._a2 == 2) {
				aa[5] = kernel_run_animation(kernel_name('F', 1), 103);
				aainfo[5]._active = -1;
				scratch._98 = 5;
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[5]);
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[5]);
				player.walker_visible = 0;
				global[g133] = 1;
				global[g143] = 1;
				kernel_timing_trigger(1, 105);
			}
		}
		break;

	case 27:
		global[walker_converse_now] = 0;
		kernel_abort_animation(scratch._a8);
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = -1;
		if (digi_val1 != 0)
			kernel_timing_trigger(1, 105);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
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
		if (digi_val1 != 0)
			kernel_timing_trigger(1, 105);
		break;

	case 101:
		kernel_abort_animation(aa[scratch._94]);
		aainfo[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = -1;
		if (digi_val1 != 0)
			kernel_timing_trigger(1, 105);
		break;

	case 102:
		kernel_abort_animation(aa[scratch._96]);
		aainfo[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = -1;
		if (digi_val1 != 0)
			kernel_timing_trigger(1, 105);
		break;

	case 103:
		if (scratch._a2 == 1) {
			kernel_abort_animation(aa[5]);
			aainfo[5]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_flip_hotspot(166, 0);
			inter_move_object(13, 2);
			display_interface();
			player.commands_allowed = -1;
			if (digi_val1 != 0)
				kernel_timing_trigger(1, 105);
		} else if (scratch._a2 == 2) {
			kernel_abort_animation(aa[5]);
			aainfo[5]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			player.walker_visible = -1;
			global[g133] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			kernel_flip_hotspot(95, 0);
			player.commands_allowed = -1;
			if (digi_val1 != 0)
				kernel_timing_trigger(1, 105);
		}
		break;

	case 104:
		kernel_abort_animation(aa[6]);
		aainfo[6]._active = 0;
		global[g141] = -1;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		player.walker_visible = -1;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		scratch._a8 = kernel_run_animation_write(0);
		kernel_reset_animation(scratch._a8, 3);
		kernel_position_anim(scratch._a8, 129, 98, 81, 12);
		global[walker_converse_now] = 1;
		global[g007] = 3;
		kernel_synch(KERNEL_ANIM, scratch._a8, KERNEL_NOW, 0);
		kernel_timing_trigger(1, 105);
		break;

	case 105:
		digi_initial_volume(15);
		digi_play_build(306, '_', 2, 3);
		digi_val2 = -1;
		break;

	default:
		break;
	}

	global_anim1(3, scratch._9a, global[g131], &global[g132]);
	global_anim2(1, scratch._9c, global[g141], &global[g142]);

	if (scratch._a0 != 0)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);

	if (aainfo[0]._active)
		room_403_anim1();
	if (aainfo[6]._active)
		room_403_anim2();
	if (aainfo[1]._active)
		room_403_anim3();
	if (aainfo[3]._active)
		room_403_anim4();
	if (aainfo[2]._active)
		room_403_anim5();
	if (aainfo[5]._active)
		room_403_anim6();

	if (global[walker_converse_now] != 0)
		global_anim3(scratch._a8, &global[g008]);
}

static void room_403_pre_parser() {
	if (player_parse(13, 47, 0))
		player.walk_off_edge_to_room = 402;
}

static void room_403_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = 0;
		digi_play_build_ii('c', 1, 1);
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (player_parse(114, 142, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a2 = 1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(114, 83, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a2 = 2;
		player.command_ready = 0;
		return;
	}

	if (player_parse(114, 131, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a2 = 3;
		player.command_ready = 0;
		return;
	}

	if (player_parse(114, 146, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a2 = 4;
		player.command_ready = 0;
		return;
	}

	if (player_parse(114, 116, 0)) {
		player.commands_allowed = 0;
		global[g145] = -1;
		scratch._8e = 2;
		player.command_ready = 0;
		return;
	}

	if (player_parse(126, 166, 0)) {
		global[g154] = 2;
		player.walker_visible = 0;
		player.commands_allowed = 0;
		scratch._9e = kernel_run_animation_talk('b', 2, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._a4 = 400;
		scratch._a0 = -1;
		kernel.trigger_setup_mode = 1;
		kernel_timing_trigger(1, 105);
		player.command_ready = 0;
		return;
	}

	if (player_parse(126, 95, 0)) {
		global[g154] = 2;
		player.walker_visible = 0;
		player.commands_allowed = 0;
		scratch._9e = kernel_run_animation_talk('b', 4, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._a4 = 300;
		scratch._a0 = -1;
		kernel.trigger_setup_mode = 1;
		kernel_timing_trigger(1, 105);
		player.command_ready = 0;
		return;
	}

	if (player_parse(126, 99, 0) || player_parse(78, 119, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 3;
		player.command_ready = 0;
	}
}

void room_403_synchronize(Common::Serializer &s) {
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
}

void room_403_preload() {
	room_init_code_pointer = room_403_init;
	room_pre_parser_code_pointer = room_403_pre_parser;
	room_parser_code_pointer = room_403_parser;
	room_daemon_code_pointer = room_403_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
