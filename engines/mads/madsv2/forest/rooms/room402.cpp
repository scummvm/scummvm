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
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sprite.h"
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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_402_anim6() {
	scratch._9e = -1;
	room_spots[3].cursor_number = 8;
	room_spots[4].cursor_number = 8;
	room_spots[7].cursor_number = 8;
	room_spots[8].cursor_number = 8;
	room_spots[9].cursor_number = 8;
	room_spots[10].cursor_number = 8;
	room_spots[11].cursor_number = 8;
}

static void room_402_init2() {
	room_402_anim6();
}

static void room_402_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 1, 0);
	kernel_position_anim(scratch._9c, 71, 128, 63, 5);
	scratch._9a = kernel_run_animation_disp('e', 2, 0);
	kernel_position_anim(scratch._9a, 112, 131, 65, 5);

	if (previous_room != KERNEL_RESTORING_GAME && previous_room != 199) {
		player_demand_facing(3);
		player_demand_location(136, 142);
	}

	if (previous_room == 199) {
		restore_player();
	} else if (previous_room == 401) {
		if (global[g083] != 3) {
			aa[0] = kernel_run_animation(kernel_name('t', 1), 103);
			aainfo[0]._active = -1;
			scratch._92 = 115;
			scratch._98 = 1;
			return;
		} else {
			aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
			aainfo[0]._active = -1;
			scratch._92 = 131;
			return;
		}
	} else if (previous_room == 403) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 58;
		return;
	}

	switch (global[g083]) {
	case 0:
		aa[1] = kernel_run_animation(kernel_name('t', 4), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 20;
		room_402_init2();
		global[player_score] = 0;
		return;
	case 1:
		aa[1] = kernel_run_animation(kernel_name('L', 2), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 6;
		break;
	case 2:
		kernel_flip_hotspot(words_room_401, false);
		aa[1] = kernel_run_animation(kernel_name('N', 1), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 15;
		kernel_flip_hotspot(words_rock, true);
		player.commands_allowed = true;
		break;
	case 3:
		kernel_flip_hotspot(words_room_403, true);
		aa[1] = kernel_run_animation(kernel_name('L', 14), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 19;
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
		return;
	default:
		return;
	}

	global[player_score] = 0;
	global_midi_play(15);
}

static void room_402_init() {
	scratch._90 = 0;
	scratch._a0 = 0;
	global[player_score] = -1;

	if (previous_room == 403) {
		global[g009] = 0;
	} else {
		midi_stop();
		global[g009] = -1;
	}

	player.walker_visible = false;
	player.commands_allowed = false;

	for (int16 count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = 1;
		aainfo[count]._val3 = 0;
		aainfo[count]._val4 = 0;
	}

	if (previous_room != KERNEL_RESTORING_GAME && previous_room != 199 && flags[27] != 3)
		flags[27]++;

	kernel_flip_hotspot(words_rock, false);
	kernel_flip_hotspot(words_room_403, false);

	room_402_init1();
}

static void room_402_anim5() {
}

static void room_402_anim_case7() {
	int16 result = -1;
	int16 a2 = scratch._a2;

	if (a2 == -668) {
		global[walker_converse_state] = 0;
		close_journal(3);
		player.commands_allowed = true;
	} else if (a2 == 1) {
		result = 14;
		kernel_timing_trigger(30, 7);
		scratch._a2 = 2;
	} else if (a2 == 2) {
		result = 16;
	} else if (a2 == 3) {
		result = 23;
		kernel_timing_trigger(30, 7);
		scratch._a2 = 4;
	} else if (a2 == 5) {
		result = 39;
		kernel_timing_trigger(30, 7);
		scratch._a2 = 6;
	} else if (a2 == 6) {
		result = 42;
	} else if (a2 == 7) {
		result = 49;
	} else if (a2 == 8) {
		result = 9;
	} else if (a2 == 10) {
		result = 11;
	} else if (a2 == 11) {
		result = 36;
	} else if (a2 == 12) {
		result = 19;
	} else if (a2 == 13) {
		result = 31;
	} else if (a2 == 14) {
		result = 38;
	} else if (a2 == 16) {
		result = 37;
	}

	if (result >= 0) {
		aainfo[1]._frame = result;
		kernel_reset_animation(aa[1], result);
	}
}

static void room_402_anim_state() {
	switch (scratch._98) {
	case 1:
		aa[1] = kernel_run_animation(kernel_name('t', 2), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 2;
		break;
	case 2:
		aa[1] = kernel_run_animation(kernel_name('t', 3), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 3;
		break;
	case 3:
		aa[1] = kernel_run_animation(kernel_name('t', 4), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 20;
		break;
	case 4:
		new_room = 401;
		break;
	case 5:
		global[g083] = 1;
		global[g070] = -1;
		display_journal();
		break;
	case 6:
		aa[1] = kernel_run_animation(kernel_name('L', 22), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 7;
		break;
	case 7:
		aa[1] = kernel_run_animation(kernel_name('L', 23), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 8;
		break;
	case 8:
		aa[1] = kernel_run_animation(kernel_name('L', 5), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 9;
		break;
	case 9:
		aa[1] = kernel_run_animation(kernel_name('L', 6), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 10;
		break;
	case 10:
		global[g083] = 2;
		aa[1] = kernel_run_animation(kernel_name('L', 7), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 11;
		break;
	case 11:
		aa[1] = kernel_run_animation(kernel_name('L', 8), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 12;
		break;
	case 12:
		aa[1] = kernel_run_animation(kernel_name('L', 9), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 13;
		break;
	case 13:
		aa[1] = kernel_run_animation(kernel_name('L', 10), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 14;
		break;
	case 14:
		aa[1] = kernel_run_animation(kernel_name('N', 1), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 15;
		room_402_anim5();
		global[walker_converse_state] = 0;
		close_journal(3);
		kernel_flip_hotspot(words_rock, true);
		kernel_flip_hotspot(words_room_401, false);
		player.commands_allowed = true;
		break;
	case 15:
		aa[1] = kernel_run_animation(kernel_name('L', 11), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 16;
		break;
	case 16:
		aa[1] = kernel_run_animation(kernel_name('L', 12), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 17;
		kernel_flip_hotspot(words_rock, false);
		break;
	case 17:
		global_midi_play(5);
		global[g009] = 0;
		aa[1] = kernel_run_animation(kernel_name('L', 13), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 18;
		break;
	case 18:
		aa[1] = kernel_run_animation(kernel_name('L', 14), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 19;
		global[g083] = 3;
		kernel_flip_hotspot(words_room_403, true);
		kernel_flip_hotspot(words_room_401, true);
		kernel_position_anim(scratch._9c, 71, 128, 63, 5);
		kernel_position_anim(scratch._9a, 112, 131, 65, 5);
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g133] = 0;
		global[g143] = 0;
		player_demand_facing(3);
		player_demand_location(136, 142);
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		global[walker_converse_state] = 0;
		close_journal(3);
		global[player_score] = -1;
		break;
	case 20:
		aa[1] = kernel_run_animation(kernel_name('L', 1), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 5;
		break;
	default:
		break;
	}
}

static void room_402_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur != aainfo[0]._frame)
		aainfo[0]._frame = cur;

	if (global[player_hyperwalked] != -1)
		return;

	aainfo[0]._frame = scratch._92 - 1;
	kernel_reset_animation(aa[0], aainfo[0]._frame);
}

static void room_402_anim2() {
	int16 result = -1;

	int16 cur = kernel_anim[aa[1]].frame;
	if (cur == aainfo[1]._frame)
		return;
	aainfo[1]._frame = cur;

	switch (scratch._98) {
	case 3: {
		int16 frame = aainfo[1]._frame;
		if (frame == 49) {
			result = 43;
		} else if (frame < 49) {
			if (frame == 23) {
				result = 18;
			} else if (frame < 23) {
				if (frame == 5) {
					if (flags[27] != 1)
						result = 23;
				} else if (frame == 9) {
					digi_play_build(402, 'b', 1, 1);
					scratch._a2 = 1;
				} else if (frame == 14) {
					result = 9;
				} else if (frame == 16) {
					result = 15;
				} else if (frame == 18) {
					digi_play_build(402, 'x', 1, 1);
					scratch._a2 = 3;
				}
			} else {
				if (frame == 31) {
					if (flags[27] != 1)
						result = 51;
				} else if (frame == 33) {
					digi_play_build(402, 'e', 1, 1);
					scratch._a2 = 5;
				} else if (frame == 39) {
					result = 33;
				} else if (frame == 42) {
					result = 41;
				} else if (frame == 43) {
					digi_play_build(402, 'b', 2, 1);
					scratch._a2 = 7;
				}
			}
		}
		break;
	}

	case 5: {
		int16 frame = aainfo[1]._frame;
		if (frame < 22) {
			if (frame == 4) {
				digi_play_build(402, 'e', 2, 1);
				scratch._a2 = 8;
			} else if (frame == 9) {
				result = 4;
			}
		}
		break;
	}

	case 8: {
		int16 frame = aainfo[1]._frame;
		if (frame == 36) {
			result = 31;
		} else if (frame < 36) {
			if (frame == 5) {
				digi_play_build(402, 'b', 3, 1);
				scratch._a2 = 10;
			} else if (frame == 11) {
				result = 5;
			} else if (frame == 31) {
				digi_play_build(402, 'b', 4, 1);
				scratch._a2 = 11;
			}
		}
		break;
	}

	case 10: {
		int16 frame = aainfo[1]._frame;
		if (frame == 10) {
			digi_play_build(402, '_', 1, 2);
		} else if (frame == 14) {
			digi_play_build(402, '_', 2, 2);
		}
		break;
	}

	case 14: {
		int16 frame = aainfo[1]._frame;
		if (frame == 16) {
			digi_play_build(402, 'x', 2, 1);
			scratch._a2 = 12;
		} else if (frame == 18) {
			result = 16;
		}
		break;
	}

	case 15: {
		int16 frame = aainfo[1]._frame;
		if (frame == 22 || frame == 51) {
			if (scratch._90 == 17) {
				if (flags[27] != 1)
					result = 51;
			} else {
				result = (imath_random(0, 100) > 50) ? 0 : 23;
			}
		}
		break;
	}

	case 16: {
		int16 frame = aainfo[1]._frame;
		if (frame == 38) {
			result = 35;
		} else if (frame < 38) {
			if (frame == 25) {
				digi_play_build(402, 'x', 3, 1);
				scratch._a2 = 13;
			} else if (frame == 31) {
				result = 25;
			} else if (frame == 35) {
				digi_play_build(402, 'e', 4, 1);
				scratch._a2 = 14;
			}
		}
		break;
	}

	case 17: {
		int16 frame = aainfo[1]._frame;
		if (frame == 37) {
			result = 36;
		} else if (frame < 37) {
			if (frame == 1) {
				digi_play_build(402, 'x', 4, 1);
				scratch._a2 = 15;
			} else if (frame == 4) {
				digi_play_build(101, '_', 2, 2);
			} else if (frame == 10) {
				digi_play_build(402, '_', 1, 2);
			} else if (frame == 17) {
				digi_play_build(402, 'x', 5, 1);
				scratch._a2 = 16;
			}
		}
		break;
	}

	case 18: {
		int16 frame = aainfo[1]._frame;
		if (frame <= 74) {
			if (frame == 27) {
				kernel_timing_trigger(120, 105);
			} else if (frame == 28) {
				result = 27;
			} else if (frame == 71 || frame == 74) {
				digi_play_build(402, '_', 3, 2);
			}
		}
		break;
	}

	case 20: {
		int16 frame = aainfo[1]._frame;
		if (frame == 2) {
			global_midi_play(15);
			player.commands_allowed = true;
			global[g083] = 0;
			room_402_anim6();
		} else if (frame == 4) {
			if (scratch._a0 == 0) {
				result = 3;
			} else {
				kernel_abort_animation(aa[1]);
				aainfo[1]._active = 0;
				aa[1] = kernel_run_animation(kernel_name('z', 1), 103);
				aainfo[1]._active = -1;
				aainfo[1]._frame = 0;
				scratch._98 = 4;
			}
		}
		break;
	}

	case 19: {
		if (aainfo[1]._frame == 2)
			result = 1;
		break;
	}

	case 2: {
		int16 frame = aainfo[1]._frame;
		if (frame == 57) {
			global_midi_play(10);
			global[player_score] = 0;
		} else if (frame < 57) {
			if (frame == 37) {
				if (flags[27] != 1)
					result = 56;
			} else if (frame == 43) {
				digi_play_build(301, '_', 1, 2);
			}
		}
		break;
	}

	default:
		break;
	}

	if (result >= 0) {
		aainfo[1]._frame = result;
		kernel_reset_animation(aa[1], result);
	}
}

static void room_402_anim3() {
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur != aainfo[3]._frame)
		aainfo[3]._frame = cur;
}

static void room_402_anim4() {
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur != aainfo[2]._frame)
		aainfo[2]._frame = cur;
}

static void room_402_daemon() {
	switch (kernel.trigger) {
	case 7:
		room_402_anim_case7();
		break;

	case 24:
		kernel_reset_animation(scratch._9a, 0);
		global[g133] = 1;
		global[g154] = 2;
		global[g156] = 0;
		player.walker_visible = false;
		player.commands_allowed = false;
		break;

	case 25:
		if (scratch._8e != 2)
			break;
		aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
		aainfo[3]._active = -1;
		scratch._94 = 2;
		kernel_reset_animation(scratch._9c, 0);
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9c);
		global[g143] = 1;
		break;

	case 26:
		kernel_reset_animation(scratch._9a, 0);
		kernel_reset_animation(scratch._9c, 0);
		global[g133] = 1;
		global[g143] = 1;
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[1]);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[1]);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (global[g083] == -1) {
			aa[1] = kernel_run_animation(kernel_name('t', 2), 103);
			aainfo[1]._active = -1;
			scratch._98 = 3;
			break;
		}
		kernel_flip_hotspot(words_room_403, true);
		aa[1] = kernel_run_animation(kernel_name('L', 14), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 19;
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

	case 101:
		kernel_abort_animation(aa[3]);
		aainfo[3]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		break;

	case 102:
		kernel_abort_animation(aa[2]);
		aainfo[2]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 103:
		if (scratch._98 == 1) {
			kernel_abort_animation(aa[0]);
			aainfo[0]._active = 0;
		} else {
			dont_frag_the_palette();
			kernel_abort_animation(aa[1]);
			aainfo[1]._active = 0;
		}
		room_402_anim_state();
		break;

	case 105:
		aainfo[1]._frame = 28;
		kernel_reset_animation(aa[1], 28);
		break;

	default:
		break;
	}

	global_anim1(2, scratch._9a, global[g131], &global[g132]);
	global_anim2(1, scratch._9c, global[g141], &global[g142]);

	if (aainfo[0]._active)
		room_402_anim1();
	if (aainfo[1]._active)
		room_402_anim2();
	if (aainfo[3]._active)
		room_402_anim3();
	if (aainfo[2]._active)
		room_402_anim4();
}

static void room_402_pre_parser() {
	if (global[g083] == 0) {
		if (player_parse(words_walk_to, words_lever, 0)) {
			player.need_to_walk = 0;
			return;
		}
		scratch._a0 = -1;
		player.commands_allowed = false;
		return;
	}

	if (global[g083] == 2) {
		player.need_to_walk = 0;
		return;
	}

	if (player_parse(words_walk_to, words_room_401, 0)) {
		if (global[g080] == 0 || flags[31] != 0)
			player.walk_off_edge_to_room = 401;
		else
			player.walk_off_edge_to_room = 420;
	}

	if (player_parse(words_walk_to, words_room_403, 0))
		player.walk_off_edge_to_room = 403;
}

static void room_402_parser() {
	if (global[walker_converse_state] != 0) {
		if (global[g083] == 0) {
			player.commands_allowed = false;
			aainfo[1]._frame = 4;
			kernel_reset_animation(aa[1], 4);
		} else if (global[g083] == 2) {
			player.commands_allowed = false;
			scratch._90 = 17;
		} else {
			player.commands_allowed = false;
			digi_play_build_ii('c', 1, 1);
			scratch._a2 = -668;
		}
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (global[g083] == 0) {
		if (player_parse(words_walk_to, words_lever, 0)) {
			player.commands_allowed = false;
			aainfo[1]._frame = 4;
			kernel_reset_animation(aa[1], 4);
		} else {
			scratch._a0 = -1;
			player.commands_allowed = false;
		}
		player.command_ready = 0;
		return;
	}

	if (global[g083] == 2) {
		if (player_parse(words_click_on, words_rock, 0)) {
			player.commands_allowed = false;
			scratch._90 = 17;
		}
		player.command_ready = 0;
		return;
	}
}

void room_402_synchronize(Common::Serializer &s) {
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
}

void room_402_preload() {
	room_init_code_pointer = room_402_init;
	room_pre_parser_code_pointer = room_402_pre_parser;
	room_parser_code_pointer = room_402_parser;
	room_daemon_code_pointer = room_402_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
