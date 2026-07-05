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
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
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
	int16 _ac;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo local->animation_info


static void room_302_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9a = kernel_run_animation_disp('e', 1, 0);
	extra_change_animation(scratch._9a, 104, 109, 68, 9);

	scratch._9c = kernel_run_animation_disp('r', 3, 0);
	extra_change_animation(scratch._9c, 137, 117, 73, 8);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 178;
		player.y = 124;
		player.facing = 1;
	}

	if (previous_room == 210) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 66;
		return;
	}
	if (previous_room == 301) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 67;
		return;
	}
	if (previous_room == 303 || previous_room == 321) {
		aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
		aainfo[0]._active = -1;
		scratch._92 = 90;
		return;
	}
	if (previous_room == 304) {
		aa[0] = kernel_run_animation(kernel_name('y', 4), 100);
		aainfo[0]._active = -1;
		scratch._92 = 53;
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
	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	}

	// default — other previous rooms
	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = true;
	player.walker_visible = true;
	kernel_flip_hotspot(119, global[g048] == 0 ? 1 : 0);
	kernel_flip_hotspot(27, global[g048]);
	kernel_flip_hotspot(75, global[g067] == 0 ? 1 : 0);
	kernel_flip_hotspot(37, global[g067]);
}

static void room_302_init() {
	scratch._aa = 0;
	scratch._a8 = -1;
	scratch._a4 = 0;
	global[walker_converse_now] = 0;
	scratch._ac = 0;
	global[g009] = 0;

	if (previous_room != 304)
		midi_stop();

	global[perform_displacements] = 240;
	global[play_background_sounds] = -1;

	if (object_is_here(rubber_band)) {
		ss[0] = kernel_load_series(kernel_name('P', 3), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 9);
		kernel_seq_loc(seq[0], 244, 105);
		kernel_seq_scale(seq[0], 65);
	} else {
		kernel_flip_hotspot(words_rubber_band, false);
	}

	ss[1] = kernel_load_series(kernel_name('p', 1), 0);
	ss[2] = kernel_load_series(kernel_name('p', 2), 0);
	ss[3] = kernel_load_series("*rm302p21", 0);

	if (global[g048]) {
		kernel_flip_hotspot(words_grass, false);
		kernel_flip_hotspot(words_room_210, true);
		seq[2] = kernel_seq_stamp(ss[2], false, KERNEL_FIRST);
		kernel_seq_depth(seq[2], 15);
		kernel_seq_loc(seq[2], 33, 86);
		kernel_seq_scale(seq[2], 52);
		seq[3] = kernel_seq_stamp(ss[3], false, KERNEL_FIRST);
		kernel_seq_depth(seq[3], 15);
		kernel_seq_loc(seq[3], 15, 86);
		kernel_seq_scale(seq[3], 52);
	} else {
		kernel_flip_hotspot(words_grass, true);
		kernel_flip_hotspot(words_room_210, false);
		seq[1] = kernel_seq_stamp(ss[1], false, KERNEL_FIRST);
		kernel_seq_depth(seq[1], 15);
		kernel_seq_loc(seq[1], 18, 90);
		kernel_seq_scale(seq[1], 55);
	}

	if (global[g067]) {
		kernel_flip_hotspot(words_bush, false);
		kernel_flip_hotspot(words_room_304, true);
	} else {
		kernel_flip_hotspot(words_bush, true);
		kernel_flip_hotspot(words_room_304, false);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int i = 0; i < 10; i++) {
			aainfo[i]._active = 0;
			aainfo[i]._frame = 0;
			aainfo[i]._val3 = 0;
			aainfo[i]._val4 = 0;
		}

		if (previous_room != 199) {
			if (flags[16] != 3)
				flags[16]++;
		}
	}

	room_302_init1();
}

static void room_302_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;
	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_302_anim2() {
	if (!scratch._a4)
		return;

	if (!scratch._ac)
		kernel_random_frame(scratch._a2, &global[g153], global[g156]);
	kernel_random_frame(scratch._9e, &global[g151], global[g154]);

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	int16 a4 = scratch._a4;
	if (a4 == 2) {
		global[g154] = 0;
		kernel_timing_trigger(30, 28);
		scratch._a4 = 3;
	} else if (a4 == 3) {
		global[g156] = 2;
		scratch._a4 = 4;
		digi_play_build(302, 'E', 1, 1);
	} else if (a4 == 4) {
		global[g156] = 0;
		kernel_timing_trigger(30, 28);
		scratch._a4 = 5;
	} else if (a4 == 5) {
		scratch._a4 = 6;
		kernel_abort_animation(scratch._a2);
		kernel_abort_animation(scratch._9e);
		player.commands_allowed = true;
		player.walker_visible = true;
		global[g017] = -1;
		aainfo[1]._active = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
	}
}

static void room_302_anim3() {
	int16 frame = (int16)kernel_anim[aa[2]].frame;
	if (frame != aainfo[2]._frame)
		aainfo[2]._frame = frame;

	if (global[player_hyperwalked] == -1 && frame <= 20) {
		aainfo[2]._frame = 20;
		kernel_reset_animation(aa[2], 20);
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
		return;
	}
	if (global[player_hyperwalked] == -1 || frame == 40) {
		aainfo[2]._frame = 39;
		kernel_reset_animation(aa[2], 39);
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
	}
}

static void room_302_anim4() {
	if (kernel_anim[aa[3]].frame != aainfo[3]._frame)
		aainfo[3]._frame = kernel_anim[aa[3]].frame;
}

static void room_302_anim5() {
	int16 frame = (int16)kernel_anim[aa[4]].frame;

	if (frame != aainfo[4]._frame) {
		aainfo[4]._frame = frame;

		if (scratch._a8 == 1) {
			if (frame == 13) {
				if (aainfo[4]._val3 == 23) {
					aainfo[4]._frame = 11;
					kernel_reset_animation(aa[4], 11);
				}
			} else if (frame <= 13) {
				if (frame == 5) {
					aainfo[4]._val3 = 21;
					digi_play_build_ii('e', 8, 1);
					scratch._a4 = 5;
				} else if (frame == 10) {
					if (aainfo[4]._val3 == 21) {
						aainfo[4]._frame = 5;
						kernel_reset_animation(aa[4], 5);
					}
				}
			}
		} else if (scratch._a8 == 2) {
			if (frame == 61) {
				if (aainfo[4]._val3 == 23) {
					aainfo[4]._frame = 58;
					kernel_reset_animation(aa[4], 58);
				}
			} else if (frame <= 61) {
				if (frame == 53) {
					aainfo[4]._val3 = 21;
					digi_play_build_ii('e', 9, 1);
					scratch._a4 = 500;
				} else if (frame == 57) {
					if (aainfo[4]._val3 == 21) {
						aainfo[4]._frame = 53;
						kernel_reset_animation(aa[4], 53);
					}
				}
			}
		} else if (scratch._a8 == 3) {
			if (frame == 5) {
				aainfo[4]._val3 = 21;
				digi_play_build(302, 'e', 2, 1);
				scratch._a4 = 50;
			} else if (frame == 12) {
				if (aainfo[4]._val3 == 21) {
					aainfo[4]._frame = 5;
					kernel_reset_animation(aa[4], 5);
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	int16 a4 = scratch._a4;
	if (a4 == 50) {
		aainfo[4]._val3 = 19;
		aainfo[4]._frame = 11;
		kernel_reset_animation(aa[4], 11);
		scratch._a4 = -1;
	} else if (a4 < 50) {
		if (a4 == 5) {
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 23;
			scratch._a4 = 6;
		} else if (a4 == 6) {
			aainfo[4]._val3 = 21;
			aainfo[4]._frame = 5;
			kernel_reset_animation(aa[4], 5);
			digi_play_build_ii('e', 10, 1);
			scratch._a4 = 7;
		} else if (a4 == 7) {
			aainfo[4]._val3 = 19;
			aainfo[4]._frame = 11;
			kernel_reset_animation(aa[4], 11);
			scratch._a4 = -1;
		}
	} else {
		if (a4 == 500) {
			kernel_timing_trigger(30, 28);
			aainfo[4]._val3 = 23;
			scratch._a4 = 501;
		} else if (a4 == 501) {
			aainfo[4]._val3 = 21;
			aainfo[4]._frame = 53;
			kernel_reset_animation(aa[4], 53);
			digi_play_build_ii('e', 10, 1);
			scratch._a4 = 502;
		} else if (a4 == 502) {
			aainfo[4]._val3 = 19;
			aainfo[4]._frame = 58;
			kernel_reset_animation(aa[4], 58);
			scratch._a4 = -1;
		}
	}
}

static void room_302_anim6() {
	int16 frame = (int16)kernel_anim[aa[5]].frame;

	if (frame != aainfo[5]._frame) {
		aainfo[5]._frame = frame;

		if (scratch._a8 == 1) {
			if (frame == 58) {
				if (aainfo[5]._val3 == 23) {
					digi_play_build(101, '_', 1, 2);
					scratch._a4 = 58;
				}
			} else if (frame < 58) {
				if (frame == 5) {
					aainfo[5]._val3 = 21;
					digi_play_build(302, 'e', 3, 1);
					scratch._a4 = 5;
				} else if (frame == 10) {
					if (aainfo[5]._val3 == 21) {
						aainfo[5]._frame = 5;
						kernel_reset_animation(aa[5], 5);
					}
				}
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a4 == 5) {
			aainfo[5]._val3 = 23;
			aainfo[5]._frame = 11;
			kernel_reset_animation(aa[5], 11);
			scratch._a4 = -1;
			kernel_seq_delete(seq[0]);
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 28) {
		int16 a4 = scratch._a4;
		if (a4 == 9 || a4 == 58)
			scratch._a4 = -1;
	}
}

static void room_302_anim7() {
	int16 result = -1;
	int16 frame = (int16)kernel_anim[aa[6]].frame;

	if (frame != aainfo[6]._frame) {
		aainfo[6]._frame = frame;

		if (scratch._98 == 104 && scratch._a8 == 1) {
			if (frame == 72) {
				if (aainfo[6]._val3 == 23) {
					aainfo[6]._frame = 70;
					kernel_reset_animation(aa[6], 70);
				}
			} else if (frame < 72) {
				if (frame == 14) {
					if (aainfo[6]._val3 == 23) {
						aainfo[6]._frame = 12;
						kernel_reset_animation(aa[6], 12);
					}
				} else if (frame < 14) {
					if (frame == 6) {
						aainfo[6]._val3 = 22;
						digi_play_build(302, 'r', 1, 1);
						scratch._a4 = 6;
						scratch._a8 = 1;
					} else if (frame == 11) {
						if (aainfo[6]._val3 == 22) {
							aainfo[6]._frame = 6;
							kernel_reset_animation(aa[6], 6);
						}
					}
				} else {
					if (frame == 66) {
						aainfo[6]._val3 = 22;
						digi_play_build(302, 'r', 2, 1);
						scratch._a4 = 66;
					} else if (frame == 70) {
						if (aainfo[6]._val3 == 22) {
							aainfo[6]._frame = 66;
							kernel_reset_animation(aa[6], 66);
						}
					}
				}
			}
		}
	}

	if (result >= 0) {
		aainfo[6]._frame = result;
		kernel_reset_animation(aa[6], result);
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	int16 a4 = scratch._a4;
	if (a4 == 67) {
		aainfo[6]._val3 = 19;
		aainfo[6]._frame = 71;
		kernel_reset_animation(aa[6], 71);
		scratch._a4 = -1;
	} else if (a4 > 67) {
		return;
	} else if (a4 == 6) {
		kernel_timing_trigger(45, 28);
		aainfo[6]._val3 = 23;
		scratch._a4 = 7;
	} else if (a4 == 7) {
		aainfo[6]._val3 = 19;
		aainfo[6]._frame = 12;
		kernel_reset_animation(aa[6], 12);
		scratch._a4 = -1;
	} else if (a4 == 66) {
		kernel_timing_trigger(45, 28);
		aainfo[6]._val3 = 23;
		scratch._a4 = 67;
	}
}

static void room_302_anim8() {
	if (kernel_anim[aa[7]].frame != aainfo[7]._frame)
		aainfo[7]._frame = kernel_anim[aa[7]].frame;
}

static void room_302_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
		} else {
			if (scratch._a4 == 200) {
				kernel_abort_animation(scratch._9e);
				scratch._a4 = -1;
				player.walker_visible = true;
				scratch._aa = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				global[g150] = -1;
				scratch._a8 = 1;
				scratch._90 = 104;
			} else if (scratch._a4 == 400) {
				kernel_abort_animation(scratch._9e);
				scratch._a4 = -1;
				player.walker_visible = true;
				scratch._aa = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				scratch._a8 = 1;
				global[g150] = -1;
				scratch._90 = 6;
			}
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
				aa[4] = kernel_run_animation(kernel_name('F', 1), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			} else if (scratch._a8 == 2) {
				aa[4] = kernel_run_animation(kernel_name('F', 2), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			}
		} else if (scratch._8c == 7) {
			global[play_background_sounds] = 0;
			aa[7] = kernel_run_animation(kernel_name('Z', 2), 106);
			aainfo[7]._active = -1;
			aainfo[7]._frame = 0;
			scratch._96 = 7;
			player.walker_visible = true;
			player.commands_allowed = false;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_ANIM, scratch._9a);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[7]);
			global[g133] = 1;
		} else if (scratch._8c == 8) {
			aa[4] = kernel_run_animation(kernel_name('E', 1), 102);
			aainfo[4]._active = -1;
			scratch._96 = 4;
			scratch._a8 = 3;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
		}
		break;

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

	case 26: {
		int16 a90 = scratch._90;
		if (a90 == 104) {
			if (scratch._a8 == 1) {
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				kernel_synch(KERNEL_ANIM, aa[scratch._9a], KERNEL_NOW, 0);
				kernel_synch(KERNEL_ANIM, aa[scratch._9c], KERNEL_NOW, 0);
				global[g133] = 1;
				global[g143] = 1;
				global[g131] = 0;
				global[g141] = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				player.walker_visible = true;
				player.commands_allowed = false;
				global[play_background_sounds] = 0;
				aa[6] = kernel_run_animation(kernel_name('R', 1), 105);
				aainfo[6]._active = -1;
				aainfo[6]._frame = 0;
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
				scratch._98 = 104;
				kernel_seq_delete(seq[1]);
			} else if (scratch._a8 == 2) {
				kernel_abort_animation(aa[6]);
				aa[6] = kernel_run_animation(kernel_name('R', 2), 103);
			}
		} else if (a90 < 104) {
			if (a90 == 1) {
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				global[g133] = 1;
				global[g143] = 1;
			} else if (a90 == 6) {
				if (scratch._a8 == 1) {
					aa[5] = kernel_run_animation(kernel_name('P', 1), 103);
					aainfo[5]._active = -1;
					scratch._98 = 5;
					kernel_reset_animation(scratch._9a, 0);
					kernel_reset_animation(scratch._9c, 0);
					kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
					global[g133] = 1;
					global[g143] = 1;
				}
			}
		}
		break;
	}

	case 27:
		scratch._ac = 0;
		global[walker_converse_now] = 0;
		kernel_abort_animation(scratch._a6);
		global[g156] = 0;
		scratch._a2 = kernel_run_animation_talk('e', 2, 0);
		extra_change_animation(scratch._a2, 104, 109, 68, 9);
		kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_NOW, 0);
		global[g154] = 2;
		scratch._a4 = 2;
		digi_play_build(302, 'b', 1, 1);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (flags[16] == 1) {
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a4 = 1;
			global[g017] = 0;
			player.walker_visible = false;
			global[g154] = 0;
			aainfo[1]._active = -1;
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g133] = 1;
			global[g143] = 0;
			scratch._9e = kernel_run_animation_talk('b', 7, 0);
			extra_change_animation(scratch._9e, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
			scratch._ac = -1;
			scratch._a6 = kernel_run_animation_write(0);
			kernel_reset_animation(scratch._a6, 41);
			extra_change_animation(scratch._a6, 104, 109, 68, 9);
			global[walker_converse_now] = 1;
			global[g007] = 1;
			kernel_synch(KERNEL_ANIM, scratch._a6, KERNEL_ANIM, scratch._9a);
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
		kernel_abort_animation(aa[scratch._96]);
		aainfo[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		global[play_background_sounds] = -1;
		player.commands_allowed = true;
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
			kernel_flip_hotspot(words_rubber_band, false);
			inter_move_object(rubber_band, PLAYER);
			global[play_background_sounds] = -1;
			player.commands_allowed = true;
		}
		break;

	case 105:
		if (scratch._98 == 104) {
			if (scratch._a8 == 1) {
				kernel_abort_animation(aa[6]);
				aa[6] = kernel_run_animation(kernel_name('R', 2), 105);
				scratch._a8 = 2;
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
			} else if (scratch._a8 == 2) {
				kernel_abort_animation(aa[6]);
				aainfo[6]._active = 0;
				global[g131] = -1;
				global[g141] = -1;
				kernel_reset_animation(scratch._9a, 1);
				kernel_reset_animation(scratch._9c, 1);
				kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
				global[g048] = -1;
				global[g133] = 0;
				global[g143] = 0;
				kernel_flip_hotspot(words_grass, false);
				kernel_flip_hotspot(words_room_210, true);
				seq[2] = kernel_seq_stamp(ss[2], 0, -1);
				kernel_seq_depth(seq[2], 15);
				kernel_seq_loc(seq[2], 33, 86);
				kernel_seq_scale(seq[2], 52);
				seq[3] = kernel_seq_stamp(ss[3], 0, -1);
				kernel_seq_depth(seq[3], 15);
				kernel_seq_loc(seq[3], 15, 86);
				kernel_seq_scale(seq[3], 52);
				player.commands_allowed = true;
			}
		}
		break;

	case 106:
		kernel_abort_animation(aa[7]);
		aainfo[7]._active = 0;
		aainfo[7]._frame = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		if (scratch._96 - 7 == 0) {
			global[tunnel_exit_3_opened] = -1;
			new_room = 210;
		}
		scratch._96 = 0;
		break;
	default:
		break;
	}

	global_anim1(1, scratch._9a, global[g131], &global[g132]);
	global_anim2(3, scratch._9c, global[g141], &global[g142]);

	if (aainfo[0]._active) room_302_anim1();
	if (aainfo[3]._active) room_302_anim4();
	if (aainfo[2]._active) room_302_anim3();
	if (aainfo[4]._active) room_302_anim5();
	if (aainfo[5]._active) room_302_anim6();
	if (aainfo[6]._active) room_302_anim7();
	if (aainfo[7]._active) room_302_anim8();
	if (aainfo[1]._active) room_302_anim2();

	if (global[walker_converse_now])
		global_anim3(scratch._a6, &global[g008]);

	if (scratch._aa)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);
}

static void room_302_pre_parser() {
	if (player_parse(words_walk_to, words_room_301, 0))
		player.walk_off_edge_to_room = 301;

	if (player_parse(words_walk_to, words_room_303, 0)) {
		if (global[g064])
			player.walk_off_edge_to_room = 321;
		else
			player.walk_off_edge_to_room = 303;
	}
}

static void room_302_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_walk_to, words_room_304, 0)) {
		new_room = 304;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_walk_to, words_room_210, 0)) {
		global[g135] = -1;
		scratch._8c = 7;
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_look_at, words_thistle, 0)) {
		global[play_background_sounds] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a8 = 1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_look_at, words_comfrey, 0)) {
		global[play_background_sounds] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a8 = 2;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_pick_up, words_rubber_band, 0)) {
		global[g154] = 2;
		global[play_background_sounds] = 0;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._9e = kernel_run_animation_talk('b', 6, 0);
		extra_change_animation(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 2, 1);
		scratch._a4 = 400;
		scratch._aa = -1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_click_on, words_grass, 0)) {
		global[g154] = 2;
		global[play_background_sounds] = 0;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._9e = kernel_run_animation_talk('b', 4, 0);
		extra_change_animation(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 1, 1);
		scratch._a4 = 200;
		scratch._aa = -1;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_click_on, words_bush, 0)) {
		global[play_background_sounds] = 0;
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 8;
		player.command_ready = 0;
	}
}

void room_302_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (auto &ai : aainfo) ai.synchronize(s);
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
}

void room_302_preload() {
	room_init_code_pointer = room_302_init;
	room_pre_parser_code_pointer = room_302_pre_parser;
	room_parser_code_pointer = room_302_parser;
	room_daemon_code_pointer = room_302_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
