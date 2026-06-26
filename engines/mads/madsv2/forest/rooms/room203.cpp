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

#include "mads/madsv2/forest/rooms/section2.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
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
	int16 _ac;
	int16 _ae;
	int16 _b0;
	int16 _b2;
	int16 _b4;
};

static Scratch scratch;

#define local  (&scratch)
#define ss     local->sprite
#define seq    local->sequence
#define aa     local->animation
#define aainfo scratch.animation_info


static void room_203_init1() {
	global[g131] = 0;
	global[g141] = 0;

	if (global[g046] != 0) {
		object_set_quality(5, 0, -1);
		object_set_quality(8, 0, -1);
		scratch._a0 = kernel_run_animation_disp('r', 1, 0);
		kernel_position_anim(scratch._a0, 132, 133, 88, 6);
		scratch._9e = kernel_run_animation_disp('e', 3, 0);
		kernel_position_anim(scratch._9e, 105, 137, 90, 5);
		player.x = 91;
		player.y = 124;
		player.facing = 3;
	} else {
		object_set_quality(5, 0, 0);
		object_set_quality(8, 0, 0);
		room_203_flag = 0;
		scratch._a0 = kernel_run_animation_disp('r', 9, 0);
		kernel_position_anim(scratch._a0, 110, 153, 98, 2);
		scratch._9e = kernel_run_animation_disp('e', 9, 0);
		kernel_position_anim(scratch._9e, 86, 148, 96, 3);
		player.x = 70;
		player.y = 130;
		player.facing = 9;
	}

	if (previous_room == 201) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 101);
		aainfo[0]._active = -1;
		scratch._9a = 11;
		scratch._92 = 60;
		if (flags[8] == 1) {
			aa[4] = kernel_run_animation(kernel_name('w', 1), 105);
			aainfo[4]._active = -1;
			aainfo[4]._frame = 0;
			scratch._9c = 60;
		}
		return;
	} else if (previous_room == 204) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 101);
		aainfo[0]._active = -1;
		scratch._9a = 11;
		scratch._92 = 99;
		return;
	} else if (previous_room == 210) {
		aa[0] = kernel_run_animation(kernel_name('y', 3), 101);
		aainfo[0]._active = -1;
		scratch._9a = 11;
		scratch._92 = 74;
		global_midi_play(11);
		scratch._a6 = 5;
		return;
	}

	if (previous_room == 199)
		restore_player();

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9e, 2);
	kernel_reset_animation(scratch._a0, 2);
	kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_NOW, 0);
	kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_NOW, 0);
	global[g133] = 0;
	global[g143] = 0;

	if (global[g046] == 0) {
		global_midi_play(15);
		scratch._a6 = 2;
		aa[4] = kernel_run_animation(kernel_name('w', 4), 105);
		aainfo[4]._active = -1;
		aainfo[4]._frame = 0;
		scratch._9c = 63;
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		kernel_flip_hotspot(words_abigail, true);
		kernel_flip_hotspot(words_russel, true);
		kernel_flip_hotspot(words_edgar, true);

		if (global[g049] != 0) {
			aa[3] = kernel_run_animation(kernel_name('a', 1), 104);
			scratch._98 = 30;
			aainfo[3]._active = -1;
			aainfo[3]._frame = 25;
			kernel_reset_animation(aa[3], 25);
			global[g049] = -1;
			player.walker_visible = false;
			player.commands_allowed = false;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
		}

		kernel_flip_hotspot(119, global[g047] == 0 ? 1 : 0);
		kernel_flip_hotspot(27, global[g047]);
	} else {
		global_midi_play(11);
		scratch._a6 = 5;
	}
}

static void room_203_init() {
	scratch._b4 = 0;
	scratch._ac = 0;
	scratch._ae = 0;
	scratch._a8 = 0;

	if (previous_room != KERNEL_RESTORING_GAME && previous_room != 199 && global[g050] == 0)
		global[g046] = 0;

	scratch._a6 = 5;
	global[player_score] = -1;
	global[g009] = -1;
	kernel_flip_hotspot(words_abi_bubble, false);
	kernel_flip_hotspot(words_all_bubble, false);
	kernel_flip_hotspot(words_abigail, false);
	kernel_flip_hotspot(words_russel, false);
	kernel_flip_hotspot(words_edgar, false);

	ss[0] = kernel_load_series(kernel_name('p', 5), 0);
	ss[1] = kernel_load_series(kernel_name('p', 6), 0);

	if (global[g047] != 0) {
		kernel_flip_hotspot(words_moss, false);
		kernel_flip_hotspot(words_room_210, true);
		seq[1] = kernel_seq_stamp(ss[1], false, KERNEL_FIRST);
		kernel_seq_depth(seq[1], 15);
		kernel_seq_loc(seq[1], 128, 74);
		kernel_seq_scale(seq[1], 100);
	} else {
		kernel_flip_hotspot(words_moss, true);
		kernel_flip_hotspot(words_room_210, false);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 15);
		kernel_seq_loc(seq[0], 129, 79);
		kernel_seq_scale(seq[0], 100);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int i = 0; i < 10; i++) {
			aainfo[i]._active = 0;
			aainfo[i]._frame = 1;
			aainfo[i]._val3 = 0;
			aainfo[i]._val4 = 0;
		}

		if (previous_room != 199 && flags[8] != 3)
			flags[8]++;
	}

	room_203_init1();
}

static void room_203_anim5();
static void room_203_anim7();

static void room_203_anim1() {
	int result = -1;
	int frame;

	if (global[player_hyperwalked])
		local->_b0 = true;

	if (kernel_anim[aa[0]].frame == aainfo[0]._frame)
		return;
	aainfo[0]._frame = kernel_anim[aa[0]].frame;

	switch (local->_9a) {
	case 11:
		if (local->_9c == 60) {
			frame = aainfo[0]._frame;
			if (frame == 50) {
				aainfo[4]._frame = 22;
				kernel_reset_animation(aa[4], 22);
			} else if (frame < 50) {
				if (frame == 22) {
					if (local->_a6 != 1) {
						global_midi_play(10);
						local->_a6 = 1;
					}
					aainfo[4]._frame = 3;
					kernel_reset_animation(aa[4], 3);
				} else if (frame == 33) {
					aainfo[4]._frame = 8;
					kernel_reset_animation(aa[4], 8);
				}
			}
		}

		if (aainfo[0]._frame == 1) {
			if (global[g046] == 0 && flags[8] > 1 && local->_a6 != 1) {
				global_midi_play(10);
				local->_a6 = 1;
			}
		}

		if (local->_b0) {
			result = local->_92 - 1;
			if (local->_9c == 60) {
				aainfo[4]._frame = 22;
				kernel_abort_animation(aa[4]);
				aainfo[4]._active = 0;
			}
			local->_b0 = 0;
		}
		break;

	case 12:
		switch (aainfo[0]._frame) {
		case 4:
			digi_play_build(203, 'E', 1, 1);
			local->_a4 = 11;
			break;
		case 8:
			result = 4;
			break;
		case 11:
			result = 9;
			break;
		case 12:
			digi_play_build(203, 'R', 1, 1);
			local->_a4 = 21;
			break;
		case 16:
			result = 12;
			break;
		case 19:
			result = 17;
			break;
		case 20:
			digi_play_build(203, 'B', 1, 1);
			local->_a4 = 31;
			break;
		case 24:
			result = 20;
			break;
		case 27:
			result = 25;
			break;
		case 28:
			digi_play_build(203, 'E', 2, 1);
			local->_a4 = 12;
			break;
		case 32:
			result = 28;
			break;
		case 35:
			result = 33;
			break;
		case 36:
			digi_play_build(203, 'R', 2, 1);
			local->_a4 = 22;
			aainfo[4]._frame = 13;
			kernel_reset_animation(aa[4], 13);
			break;
		case 40:
			result = 36;
			break;
		case 42:
			if (aainfo[4]._frame > 20)
				result = 44;
			break;
		case 43:
			result = 41;
			break;
		case 50:
			if (aainfo[4]._frame < 27) {
				result = 49;
			} else {
				dont_frag_the_palette();
				kernel_abort_animation(aa[4]);
				aainfo[4]._active = 0;
				dont_frag_the_palette();
				kernel_abort_animation(aa[0]);
				aainfo[0]._active = 0;
				aa[0] = kernel_run_animation(kernel_name('x', 3), 101);
				aainfo[0]._active = -1;
				aainfo[0]._frame = 0;
				local->_9a = 13;
				aa[4] = kernel_run_animation(kernel_name('w', 4), 105);
				aainfo[4]._active = -1;
				aainfo[4]._frame = 0;
				local->_9c = 63;
			}
			break;
		default:
			break;
		}
		break;

	case 13: {
		frame = aainfo[0]._frame;
		if (frame == 29) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._active = 0;
			dont_frag_the_palette();
			kernel_abort_animation(aa[0]);
			aainfo[0]._active = 0;
			room_203_anim7();
		} else if (frame < 29) {
			if (frame == 22) {
				result = 20;
			} else if (frame < 22) {
				if (frame == 1) {
					digi_play_build(203, 'B', 2, 1);
					local->_a4 = 32;
				} else if (frame == 5) {
					result = 1;
				} else if (frame == 20) {
					local->_a4 = 23;
					if (flags[8] == 1) {
						digi_play_build(203, 'R', 3, 1);
					} else {
						digi_play("NULL", 1);
						result = 23;
					}
				}
			} else {
				if (frame == 24) {
					result = 23;
				} else if (frame == 25) {
					local->_a4 = 13;
					if (flags[8] == 1) {
						digi_play_build(203, 'E', 3, 1);
					} else {
						digi_play("NULL", 1);
						result = 23;
					}
				} else if (frame == 27) {
					result = 25;
				}
			}
		}
		break;

	case 15:
		if (aainfo[0]._frame == 1) {
			digi_play_build(203, '_', 500, 2);
		}
		break;

	case 16:
		switch (aainfo[0]._frame) {
		case 6:
			digi_play_build(203, 'E', 6, 1);
			local->_a4 = 16;
			break;
		case 11:
			result = 6;
			break;
		case 51:
			digi_play_build(203, 'E', 7, 1);
			local->_a4 = 17;
			break;
		case 56:
			result = 51;
			break;
		default:
			break;
		}
		break;
	}

	default:
		break;
	}

	if (result >= 0 && aainfo[0]._active) {
		aainfo[0]._frame = result;
		kernel_reset_animation(aa[0], result);
	}
}

static void room_203_anim2() {
	int result = -1;

	if (kernel_anim[aa[1]].frame != aainfo[1]._frame) {
		aainfo[1]._frame = kernel_anim[aa[1]].frame;

		switch (local->_96) {
		case 52:
		case 57:
			if (aainfo[1]._frame == 4) {
				if (local->_96 == 52) {
					digi_play_build(203, 'e', 4, 1);
					local->_a4 = 14;
				} else {
					digi_play_build(203, 'e', 5, 1);
					local->_a4 = 15;
				}
			} else if (aainfo[1]._frame == 10) {
				result = 4;
			}
			break;

		case 53:
			if (aainfo[1]._frame == 5)
				digi_play_build(203, '_', 2, 2);
			break;

		case 54:
		case 55:
			switch (aainfo[1]._frame) {
			case 1:
				aainfo[1]._val4++;
				if (aainfo[1]._val4 > 8)
					aainfo[1]._val4 = 0;
				else
					result = 0;
				break;
			case 2:
			case 9:
			case 20:
			case 29:
				digi_play_build(203, '_', 2, 2);
				break;
			case 6:
			case 24:
				digi_play_build(203, '_', 3, 2);
				break;
			case 8:
			case 26:
				digi_play_build(203, '_', 800, 1);
				local->_a4 = 54;
				break;
			case 16:
				if (local->_ac)
					result = 35;
				break;
			case 18:
				aainfo[1]._val4++;
				if (aainfo[1]._val4 > 4)
					aainfo[1]._val4 = 0;
				else
					result = 17;
				break;
			case 33:
				aainfo[1]._val4++;
				if (aainfo[1]._val4 > 4)
					aainfo[1]._val4 = 0;
				else
					result = 32;
				break;
			case 34:
				local->_a8++;
				if (local->_a8 <= 2) {
					result = 1;
				} else {
					dont_frag_the_palette();
					kernel_abort_animation(aa[1]);
					aainfo[1]._active = 0;
					dont_frag_the_palette();
					kernel_abort_animation(aa[4]);
					aainfo[4]._active = 0;
					aa[5] = kernel_run_animation(kernel_name('h', 1), 0);
					aainfo[5]._active = -1;
					aainfo[5]._frame = 0;
					aa[4] = kernel_run_animation(kernel_name('w', 5), 105);
					aainfo[4]._active = -1;
					aainfo[4]._frame = 0;
					local->_9c = 64;
				}
				break;
			case 39:
				dont_frag_the_palette();
				kernel_abort_animation(aa[1]);
				aainfo[1]._active = 0;
				aainfo[5]._frame = 1;
				kernel_reset_animation(aa[5], 1);
				kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
				aainfo[4]._frame = 1;
				kernel_reset_animation(aa[4], 1);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
				aa[2] = kernel_run_animation(kernel_name('R', 3), 102);
				aainfo[2]._active = -1;
				aainfo[2]._frame = 0;
				local->_94 = 41;
				local->_a8 = 0;
				kernel_reset_animation(local->_a0, 0);
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_a0);
				global[g143] = 1;
				break;
			default:
				break;
			}
			break;

		case 56:
			if (aainfo[1]._frame == 10 && local->_ac) {
				aainfo[3]._frame = 1;
				kernel_reset_animation(aa[3], 1);
			}
			break;

		default:
			break;
		}

		if (result >= 0 && aainfo[1]._active) {
			aainfo[1]._frame = result;
			kernel_reset_animation(aa[1], result);
		}
	}

	if (global[player_hyperwalked] && local->_96 == 50) {
		aainfo[1]._frame = 59;
		kernel_reset_animation(aa[1], 59);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
	}
}

static void room_203_anim3() {
	int result = -1;
	int16 frame;

	if (kernel_anim[aa[2]].frame == aainfo[2]._frame)
		return;
	aainfo[2]._frame = kernel_anim[aa[2]].frame;

	switch (local->_94) {
	case 41:
		frame = aainfo[2]._frame;
		if (frame == 8) {
			aainfo[4]._frame = 11;
			kernel_reset_animation(aa[4], 11);
			digi_play_build(203, '_', 1, 2);
			local->_a8++;
		} else if (frame == 12) {
			if (local->_ac) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[2]);
				aainfo[2]._active = 0;
				kernel_reset_animation(local->_a0, 1);
				kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
				global[g143] = 0;
				aainfo[3]._frame = 18;
				kernel_reset_animation(aa[3], 18);
			} else if (local->_a8 < 5) {
				if (imath_random(1, 100) > 80)
					result = 1;
				else
					result = 11;
			}
		}
		break;

	case 42:
		frame = aainfo[2]._frame;
		if (frame == 4) {
			digi_play_build(203, 'R', 6, 1);
			local->_a4 = 26;
		} else if (frame == 9) {
			result = 4;
		}
		break;

	case 43:
		frame = aainfo[2]._frame;
		if (frame == 20) {
			result = 13;
		} else if (frame < 20) {
			if (frame == 1) {
				digi_play_build(203, 'R', 4, 1);
				local->_a4 = 24;
			} else if (frame == 4) {
				result = 1;
			} else if (frame == 10) {
				result = 6;
			} else if (frame == 13) {
				digi_play_build(203, 'R', 5, 1);
				local->_a4 = 25;
			}
		}
		break;

	default:
		break;
	}

	if (result >= 0 && aainfo[2]._active) {
		aainfo[2]._frame = result;
		kernel_reset_animation(aa[2], result);
	}
}

static void room_203_flip_hotspots1() {
	kernel_flip_hotspot(words_abi_bubble, true);
	kernel_flip_hotspot(words_all_bubble, true);
	kernel_flip_hotspot(words_abigail, false);
	kernel_flip_hotspot(words_russel, false);
	kernel_flip_hotspot(words_edgar, false);
	kernel_flip_hotspot(words_moss, false);
	kernel_flip_hotspot(words_room_210, false);
	kernel_flip_hotspot(words_room_201, false);
	kernel_flip_hotspot(words_room_204, false);
}

static void room_203_flip_hotspots2() {
	kernel_flip_hotspot(words_abi_bubble, false);
	kernel_flip_hotspot(words_all_bubble, false);
	kernel_flip_hotspot(119, global[g047] < 1 ? 1 : 0);
	kernel_flip_hotspot(27, global[g047]);
	kernel_flip_hotspot(words_room_201, true);
	kernel_flip_hotspot(words_room_204, true);
}

static void room_203_anim4() {
	int result = -1;
	int frame;

	if (kernel_anim[aa[3]].frame == aainfo[3]._frame)
		return;
	aainfo[3]._frame = kernel_anim[aa[3]].frame;

	switch (local->_98) {
	case 30:
		frame = aainfo[3]._frame;
		if (frame == 42) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[3]);
			aainfo[3]._active = 0;
			room_203_flip_hotspots2();
			kernel_flip_hotspot(words_abigail, true);
			kernel_flip_hotspot(words_russel, true);
			kernel_flip_hotspot(words_edgar, true);
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = true;
			if (local->_ae) {
				global[g145] = -1;
				local->_8e = 41;
				player.commands_allowed = false;
				local->_a8 = 0;
				local->_ae = 0;
			}
		} else if (frame < 42) {
			if (frame == 29) {
				digi_play_build(203, 'b', 4, 1);
				local->_a4 = 34;
				local->_ac = 0;
			} else if (frame < 29) {
				if (frame == 1) {
					digi_play_build(203, 'b', 3, 1);
					local->_a4 = 33;
				} else if (frame == 3) {
					result = 1;
				} else if (frame == 26) {
					room_203_flip_hotspots1();
					player.commands_allowed = true;
				} else if (frame == 28) {
					result = 27;
				}
			} else {
				if (frame == 33) {
					result = 29;
				} else if (frame == 34) {
					digi_play_build(203, 'b', 5, 1);
					local->_a4 = 35;
					local->_ac = -1;
				} else if (frame == 38) {
					result = 34;
				} else if (frame == 40) {
					dont_frag_the_palette();
					kernel_abort_animation(aa[3]);
					aainfo[3]._active = 0;
					if (local->_ac) {
						aa[5] = kernel_run_animation(kernel_name('h', 1), 0);
						aainfo[5]._active = -1;
						aainfo[5]._frame = 4;
						kernel_reset_animation(aa[5], 4);
					}
					aa[3] = kernel_run_animation(kernel_name('a', 2), 104);
					aainfo[3]._active = -1;
					aainfo[3]._frame = 0;
					local->_98 = 31;
					player.commands_allowed = false;
					room_203_flip_hotspots2();
				}
			}
		}
		break;

	case 31:
		frame = aainfo[3]._frame;
		if (frame == 19) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[3]);
			aainfo[3]._active = 0;
			aa[3] = kernel_run_animation(kernel_name('a', 3), 104);
			local->_98 = 32;
			aainfo[3]._active = -1;
			aainfo[3]._frame = 0;
			local->_a8 = 0;
		} else if (frame < 19) {
			if (frame == 8) {
				digi_play_build(101, '_', 1, 2);
			} else if (frame == 16) {
				if (local->_ac) {
					global[g150] = -1;
					local->_90 = 55;
				}
			} else if (frame == 17) {
				if (local->_ac)
					result = 16;
			}
		}
		break;

	case 32:
		frame = aainfo[3]._frame;
		if (frame == 40) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[3]);
			aainfo[3]._active = 0;
			if (local->_ac) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[5]);
				aainfo[5]._active = 0;
			}
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._active = 0;
			aa[3] = kernel_run_animation(kernel_name('a', 4), 104);
			local->_98 = 33;
			aainfo[3]._active = -1;
			aainfo[3]._frame = 0;
			if (local->_ac) {
				aa[5] = kernel_run_animation(kernel_name('h', 1), 0);
				aainfo[5]._active = -1;
				aainfo[5]._frame = 1;
				kernel_reset_animation(aa[5], 1);
			}
			aa[4] = kernel_run_animation(kernel_name('w', 5), 105);
			local->_9c = 64;
			aainfo[4]._active = -1;
			aainfo[4]._frame = 0;
		} else if (frame < 40) {
			if (frame == 13 || frame == 21 || frame == 29) {
				aainfo[4]._frame = 1;
				kernel_reset_animation(aa[4], 1);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, aa[3]);
			} else if (frame < 13) {
				if (frame == 5) {
					digi_play_build(203, '_', 4, 2);
				} else if (frame == 11) {
					aainfo[4]._frame = 17;
					kernel_reset_animation(aa[4], 17);
					kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, aa[3]);
					if (imath_random(0, 100) > 65)
						local->_aa = 0;
					else
						local->_aa = -1;
				} else if (frame == 12) {
					local->_a8++;
					digi_play_build(203, '_', 5, 2);
				}
			} else {
				if (frame == 18) {
					if (local->_a8 > 3 || local->_ac)
						result = 34;
				} else if (frame == 34) {
					result = 9;
				}
			}
		}
		break;

	case 33:
		frame = aainfo[3]._frame;
		if (frame == 63) {
			result = 60;
		} else if (frame < 63) {
			if (frame == 34) {
				digi_play_build(101, '_', 1, 2);
				clear_selected_item();
			} else if (frame < 34) {
				if (frame == 1) {
					result = 0;
				} else if (frame == 4) {
					digi_play_build(203, '_', 4, 2);
				}
			} else {
				if (frame == 57) {
					if (local->_ac) {
						digi_play_build(203, 'b', 9, 1);
						local->_a4 = 39;
					} else {
						digi_play_build(203, 'b', 6, 1);
						local->_a4 = 36;
					}
				} else if (frame == 59) {
					result = 57;
				}
			}
		}
		break;

	default:
		break;
	}

	if (result >= 0 && aainfo[3]._active) {
		aainfo[3]._frame = result;
		kernel_reset_animation(aa[3], result);
	}
}

static void room_203_anim5() {
	int result = -1;

	if (kernel_anim[aa[4]].frame == aainfo[4]._frame)
		return;
	aainfo[4]._frame = kernel_anim[aa[4]].frame;

	switch (local->_9c) {
	case 60: {
		int16 frame = aainfo[4]._frame;
		if (frame == 22) {
			result = 18;
		} else if (frame < 22) {
			if (frame == 2)
				result = 1;
			else if (frame == 7)
				result = 5;
		}
		break;
	}
	case 61:
		if (aainfo[4]._frame == 13)
			result = 11;
		break;
	case 62: {
		int16 frame = aainfo[4]._frame;
		if (frame == 29) {
			result = 27;
		} else if (frame < 29) {
			if (frame == 1) {
				global_midi_play(12);
				local->_a6 = 4;
			} else if (frame == 19) {
				digi_play_build(103, '_', 3, 2);
				if (local->_a6 != 2) {
					global_midi_play(15);
					local->_a6 = 2;
				}
			} else if (frame == 22) {
				digi_play_build(203, '_', 500, 2);
			}
		}
		break;
	}
	case 63: {
		int16 frame = aainfo[4]._frame;
		if (frame == 19) {
			result = 17;
		} else if (frame < 19) {
			if (frame == 2) {
				if (local->_aa) {
					if (local->_a8 > 1)
						digi_play_build(203, '_', 500, 2);
					local->_aa = 0;
				}
			} else if (frame == 9) {
				result = 7;
			} else if (frame == 13) {
				result = 1;
			}
		}
		break;
	}
	case 64: {
		int16 frame = aainfo[4]._frame;
		if (frame == 25) {
			midi_stop();
			local->_a6 = 0;
		} else if (frame < 25) {
			if (frame == 1) {
				global_midi_play(5);
				local->_a6 = 3;
			} else if (frame == 18) {
				if (!local->_ac && local->_98 == 33) {
					aainfo[3]._frame = 1;
					kernel_reset_animation(aa[3], 1);
				}
			}
		}
		break;
	}
	default:
		break;
	}

	if (result >= 0 && aainfo[4]._active) {
		aainfo[4]._frame = result;
		kernel_reset_animation(aa[4], result);
	}
}

static void room_203_anim6() {
	int result = -1;

	if (kernel_anim[aa[5]].frame == aainfo[5]._frame)
		return;
	aainfo[5]._frame = kernel_anim[aa[5]].frame;

	if (aainfo[5]._frame == 2)
		result = 1;
	else if (aainfo[5]._frame == 5)
		result = 4;

	if (result >= 0) {
		aainfo[5]._frame = result;
		kernel_reset_animation(aa[5], result);
	}
}

static void room_203_anim7() {
	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(local->_9e, 1);
	kernel_reset_animation(local->_a0, 1);
	kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
	kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
	global[g133] = 0;
	global[g143] = 0;
	aa[4] = kernel_run_animation(kernel_name('w', 4), 105);
	aainfo[4]._active = -1;
	aainfo[4]._frame = 0;
	local->_9c = 63;
	local->_aa = -1;
	player.walker_visible = true;
	kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
	player.commands_allowed = -1;
	kernel_flip_hotspot(words_abigail, true);
	kernel_flip_hotspot(words_russel, true);
	kernel_flip_hotspot(words_edgar, true);
}

static void room_203_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (local->_a4) {
		case 11:
			kernel_timing_trigger(40, 106);
			aainfo[0]._frame = 9;
			kernel_reset_animation(aa[0], 9);
			break;

		case 12:
			kernel_timing_trigger(40, 106);
			aainfo[0]._frame = 33;
			kernel_reset_animation(aa[0], 33);
			break;

		case 13:
			aainfo[0]._frame = 28;
			kernel_reset_animation(aa[0], 28);
			break;

		case 14:
		case 15:
			aainfo[1]._frame = 11;
			kernel_reset_animation(aa[1], 11);
			break;

		case 16:
			aainfo[0]._frame = 12;
			kernel_reset_animation(aa[0], 12);
			break;

		case 17:
			aainfo[0]._frame = 57;
			kernel_reset_animation(aa[0], 57);
			break;

		case 21:
			kernel_timing_trigger(40, 106);
			aainfo[0]._frame = 17;
			kernel_reset_animation(aa[0], 17);
			break;

		case 22:
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._active = 0;

			aa[4] = kernel_run_animation(kernel_name('w', 3), 105);
			aainfo[4]._active = -1;
			aainfo[4]._frame = 0;
			local->_9c = 62;
			aainfo[0]._frame = 41;
			kernel_reset_animation(aa[0], 41);
			break;

		case 23:
			kernel_timing_trigger(40, 106);
			aainfo[0]._frame = 23;
			kernel_reset_animation(aa[0], 23);
			break;

		case 24:
			kernel_timing_trigger(40, 106);
			aainfo[2]._frame = 6;
			kernel_reset_animation(aa[2], 6);
			break;

		case 25:
			aainfo[2]._frame = 20;
			kernel_reset_animation(aa[2], 20);
			break;

		case 26:
			aainfo[2]._frame = 10;
			kernel_reset_animation(aa[2], 10);
			break;

		case 30:
			dont_frag_the_palette();
			kernel_abort_animation(local->_a2);
			player.walker_visible = true;
			local->_b2 = 0;
			global[g154] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g150] = -1;
			local->_90 = global[g046] ? 16 : 15;
			break;

		case 31:
			kernel_timing_trigger(40, 106);
			aainfo[0]._frame = 25;
			kernel_reset_animation(aa[0], 25);
			break;

		case 32:
			aainfo[0]._frame = 5;
			kernel_reset_animation(aa[0], 5);
			break;

		case 33:
			aainfo[3]._frame = 3;
			kernel_reset_animation(aa[3], 3);
			break;

		case 34:
		case 35:
			aainfo[3]._frame = 39;
			kernel_reset_animation(aa[3], 39);
			break;

		case 36:
			kernel_timing_trigger(40, 106);
			aainfo[3]._frame = 60;
			kernel_reset_animation(aa[3], 60);
			break;

		case 38:
		case 39:
			aainfo[3]._frame = 63;
			kernel_reset_animation(aa[3], 63);
			break;

		case 51:
			kernel_timing_trigger(40, 106);
			aainfo[0]._frame = 24;
			kernel_reset_animation(aa[0], 24);
			break;

		case 52:
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
			local->_a4 = 0;
			break;

		default:
			break;
		}
		break;

	case 24:
		switch (local->_8c) {
		case 50:
			aa[1] = kernel_run_animation(kernel_name('Z', 1), 103);
			aainfo[1]._active = -1;
			aainfo[1]._frame = 0;
			local->_96 = 50;
			player.commands_allowed = false;
			player.walker_visible = false;
			kernel_reset_animation(local->_9e, 0);

			global[g131] = 0;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, local->_9e);
			kernel_synch(KERNEL_ANIM, 0, KERNEL_PLAYER, aa[1]);
			global[g133] = 1;
			break;

		case 51:
			aa[1] = kernel_run_animation(kernel_name('e', 4), 103);
			aainfo[1]._active = -1;
			aainfo[1]._frame = 0;
			local->_96 = 52;
			player.commands_allowed = false;
			player.walker_visible = false;
			kernel_reset_animation(local->_9e, 0);

			global[g131] = 0;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, local->_9e);
			kernel_synch(KERNEL_ANIM, 0, KERNEL_PLAYER, aa[1]);
			global[g133] = 1;
			break;

		}
		break;

	case 25:
		player.commands_allowed = false;

		switch (local->_8e) {
		case 41:
			aa[2] = kernel_run_animation(kernel_name('R', 3), 102);
			aainfo[2]._active = -1;
			aainfo[2]._frame = 0;
			local->_94 = 41;
			local->_a8 = 0;
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_a0);
			global[g143] = 1;
			break;

		case 42:
			aa[2] = kernel_run_animation(kernel_name('R', 1), 102);
			aainfo[2]._active = -1;
			aainfo[2]._frame = 0;
			local->_94 = 42;
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_a0);
			global[g143] = 1;
			break;

		case 43:
			aa[2] = kernel_run_animation(kernel_name('R', 2), 102);
			aainfo[2]._active = -1;
			aainfo[2]._frame = 0;
			local->_94 = 43;
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_a0);
			global[g143] = 1;
			break;

		default:
			break;
		}
		break;

	case 26:
		kernel_reset_animation(local->_9e, 0);
		kernel_synch(KERNEL_ANIM, aa[local->_9e], KERNEL_NOW, 0);
		global[g133] = 1;
		global[g131] = 0;

		if (local->_90 != 55) {
			kernel_reset_animation(local->_a0, 0);
			kernel_synch(KERNEL_ANIM, aa[local->_a0], KERNEL_NOW, 0);
			global[g143] = 1;
			global[g141] = 0;
		}

		player.walker_visible = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = false;

		switch (local->_90) {
		case 14:
			aa[0] = kernel_run_animation(kernel_name('x', 4), 101);
			aainfo[0]._active = -1;
			aainfo[0]._frame = 0;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			local->_9a = 14;
			break;

		case 15:
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._active = 0;
			aa[0] = kernel_run_animation(kernel_name('z', 2), 101);
			aainfo[0]._active = -1;
			aainfo[0]._frame = 0;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			local->_9a = 15;
			break;

		case 16:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			aa[0] = kernel_run_animation(kernel_name('m', 1), 101);
			aainfo[0]._active = -1;
			aainfo[0]._frame = 0;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			local->_9a = 16;
			kernel_seq_delete(seq[0]);
			break;

		case 55:
			global[g143] = 0;
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, aa[local->_a0], KERNEL_NOW, 0);
			aa[1] = kernel_run_animation(kernel_name('e', 1), 103);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			aainfo[1]._active = -1;
			aainfo[1]._frame = 0;
			local->_a8 = 0;
			break;

		default:
			break;
		}
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;

		switch (local->_9a) {
		case 11:
			if (global[g046]) {
				global[g131] = -1;
				global[g141] = -1;
				kernel_reset_animation(local->_9e, 1);
				kernel_reset_animation(local->_a0, 1);
				player.walker_visible = true;
				global[g133] = 0;
				global[g143] = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
				kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
				player.commands_allowed = true;
			} else {
				if (local->_a6 != 1) {
					global_midi_play(10);
					local->_a6 = 1;
				}

				aa[0] = kernel_run_animation(kernel_name('x', 2), 101);
				aainfo[0]._active = -1;
				aainfo[0]._frame = 0;
				local->_9a = 12;

				if (flags[8] == 1) {
					aa[4] = kernel_run_animation(kernel_name('w', 2), 105);
					aainfo[4]._active = -1;
					aainfo[4]._frame = 0;
					local->_9c = 61;
				} else {
					aa[4] = kernel_run_animation(kernel_name('w', 3), 105);
					aainfo[4]._active = -1;
					aainfo[4]._frame = 0;
					local->_9c = 62;
					aainfo[0]._frame = 41;
					kernel_reset_animation(aa[0], 41);
				}
			}
			break;

		case 14:
			dont_frag_the_palette();
			kernel_abort_animation(local->_9e);
			dont_frag_the_palette();
			kernel_abort_animation(local->_a0);
			local->_a0 = kernel_run_animation_disp('r', 1, 0);
			kernel_position_anim(local->_a0, 132, 133, 88, 6);
			local->_9e = kernel_run_animation_disp('e', 3, 0);
			kernel_position_anim(local->_9e, 105, 137, 90, 5);
			close_interface(CANDLE_FLY);
			clear_selected_item();
			global[g046] = -1;

			if (local->_ac)
				global[g050] = -1;

			kernel_flip_hotspot(words_abigail, false);
			kernel_flip_hotspot(words_russel, false);
			kernel_flip_hotspot(words_edgar, false);

			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(local->_9e, 1);
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g133] = 0;
			global[g143] = 0;

			player.walker_visible = false;
			player_demand_facing(3);
			player_demand_location(124, 91);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = true;
			player.walker_visible = true;
			room_203_flag = true;
			break;

		case 15:
			new_room = 201;
			break;

		case 16:
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(local->_9e, 1);
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g133] = 0;
			global[g143] = 0;

			kernel_flip_hotspot(words_room_210, true);
			seq[1] = kernel_seq_stamp(ss[1], true, -1);
			kernel_seq_depth(seq[1], 15);
			kernel_seq_loc(seq[1], 128, 74);
			kernel_seq_scale(seq[1], 100);
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._active = 0;
		aainfo[2]._frame = 0;

		switch (local->_8e) {
		case 41:
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			aainfo[4]._active = 0;
			aa[4] = kernel_run_animation(kernel_name('w', 5), 105);
			aainfo[4]._active = -1;
			aainfo[4]._frame = 0;
			local->_9c = 64;
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g143] = 0;
			clear_selected_item();
			break;

		case 42:
			global[g141] = -1;
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_timing_trigger(1, 26);
			local->_90 = 14;
			break;

		case 43:
			global[g141] = -1;
			kernel_reset_animation(local->_a0, 1);
			kernel_synch(KERNEL_ANIM, local->_a0, KERNEL_NOW, 0);
			global[g143] = 0;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		break;

	case 103:
		dont_frag_the_palette();
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		aainfo[1]._frame = 0;

		switch (local->_96) {
		case 50:
			global[g040] = -1;
			new_room = 210;
			break;

		case 52:
			aa[1] = kernel_run_animation(kernel_name('e', 1), 103);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
			aainfo[1]._active = -1;
			aainfo[1]._frame = 0;
			local->_96 = 53;
			local->_a8 = 0;
			break;

		case 53:
			aa[1] = kernel_run_animation(kernel_name('e', 2), 103);
			aainfo[1]._active = -1;
			aainfo[1]._frame = 0;
			aainfo[1]._val4 = 0;
			aainfo[4]._frame = 17;
			kernel_reset_animation(aa[4], 0);
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
			local->_96 = local->_ac ? 55 : 54;
			break;

		case 56:
			if (local->_ac) {
				global[g131] = -1;
				kernel_reset_animation(local->_9e, 1);
				kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
				global[g133] = 0;
			} else {
				aa[1] = kernel_run_animation(kernel_name('e', 4), 103);
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
				aainfo[1]._active = -1;
				aainfo[1]._frame = 0;
				local->_96 = 57;
			}
			break;

		case 57:
			global[g131] = -1;
			kernel_reset_animation(local->_9e, 1);
			kernel_synch(KERNEL_ANIM, local->_9e, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_timing_trigger(1, 26);
			local->_90 = 14;
			break;

		default:
			local->_96 = 0;
			break;
		}
		break;

	case 104:
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._active = 0;

		if (local->_98 == 33) {
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g150] = -1;
			local->_90 = 14;
			local->_98 = 0;
			local->_94 = 0;
			local->_96 = 0;
		}
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(aa[4]);
		aainfo[4]._active = 0;

		if (local->_9c == 64) {
			if (local->_94 == 41 && local->_ac == 0) {
				global[g145] = -1;
				local->_8e = 42;
			}

			if (local->_96 == 54) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[5]);
				aainfo[5]._active = 0;
				aa[1] = kernel_run_animation(kernel_name('e', 3), 103);
				aainfo[1]._active = -1;
				aainfo[1]._frame = 0;
				local->_96 = 56;
			}

			object_set_quality(5, -1, -1);
			object_set_quality(8, -1, -1);

			if (local->_ac) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[5]);
				aainfo[5]._active = 0;
				aa[1] = kernel_run_animation(kernel_name('e', 3), 103);
				aainfo[1]._active = -1;
				aainfo[1]._frame = 0;
				local->_96 = 56;
			}
			break;
		}
		break;

	case 106:
		switch (local->_a4) {
		case 11:
			aainfo[0]._frame = 11;
			kernel_reset_animation(aa[0], 11);
			break;

		case 12:
			aainfo[0]._frame = 35;
			kernel_reset_animation(aa[0], 35);
			break;

		case 21:
			aainfo[0]._frame = 19;
			kernel_reset_animation(aa[0], 19);
			break;

		case 23:
			digi_play_build(203, 'W', 1, 1);
			local->_a4 = 51;
			aainfo[4]._frame = 1;
			kernel_reset_animation(aa[4], 1);
			break;

		case 24:
			aainfo[2]._frame = 10;
			kernel_reset_animation(aa[2], 10);
			break;

		case 31:
			aainfo[0]._frame = 27;
			kernel_reset_animation(aa[0], 27);
			break;

		case 36:
			aainfo[3]._frame = 57;
			kernel_reset_animation(aa[3], 57);
			digi_play_build(203, 'b', 8, 1);
			local->_a4 = 38;
			break;

		case 51:
			aainfo[0]._frame = 25;
			kernel_reset_animation(aa[0], 25);
			break;

		default:
			break;
		}
		break;

	case 107:
		player.commands_allowed = false;
		break;

	default:
		break;
	}

	if (global[g046]) {
		global_anim1(3, local->_9e, global[g131], &global[g132]);
		global_anim2(3, local->_a0, global[g141], &global[g142]);
	} else {
		global_anim1(9, local->_9e, global[g131], &global[g132]);
		global_anim2(9, local->_a0, global[g141], &global[g142]);
	}

	global[g030] = local->_a2;
	if (local->_b2)
		kernel_random_frame(local->_a2, &global[g151], global[g154]);
	if (aainfo[0]._active)
		room_203_anim1();
	if (aainfo[2]._active)
		room_203_anim3();
	if (aainfo[1]._active)
		room_203_anim2();
	if (aainfo[3]._active)
		room_203_anim4();
	if (aainfo[4]._active)
		room_203_anim5();
	if (aainfo[5]._active)
		room_203_anim6();
}

static void room_203_pre_parser() {
	if (global[g046] == 0) {
		player.need_to_walk = 0;
		return;
	}
	if (player_parse(words_walk_to, words_room_201, 0))
		player.walk_off_edge_to_room = 201;
	if (player_parse(words_walk_to, words_room_204, 0))
		player.walk_off_edge_to_room = 204;
}

static void room_203_parser() {
	if (player_parse(words_walk_to, words_room_210, 0)) {
		if (global[g046] != 0) {
			global[g135] = -1;
			scratch._8c = 50;
		} else {
			player.commands_allowed = false;
			global[g150] = -1;
			scratch._90 = 15;
		}
		goto handled;
	}

	if (player_parse(words_walk_to, words_room_201, 0)) {
		if (global[g046] == 0) {
			player.commands_allowed = false;
			global[g150] = -1;
			scratch._90 = 15;
		}
		goto handled;
	}

	if (player_parse(words_walk_to, words_room_204, 0) && global[g046] == 0) {
		player.commands_allowed = false;
		global[g150] = -1;
		scratch._90 = 15;
	}

	if (global[walker_converse_state] != 0) {
		if (global[g046] == 0) {
			player.commands_allowed = false;
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 107);
			scratch._a8 = 0;
			if (global[g049] != 0) {
				inter_spin_object(8);
				aainfo[3]._frame = 28;
				kernel_reset_animation(aa[3], 28);
				global[g049] = 0;
			} else {
				global[g135] = -1;
				scratch._8c = 51;
			}
		} else {
			player.commands_allowed = false;
			digi_play_build_ii('c', 1, 1);
			scratch._a4 = 52;
		}
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		if (global[player_selected_object] == 5) {
			if (global[g046] != 0) {
				// TODO: display_interface()
			} else {
				player.commands_allowed = false;
				kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
				kernel_timing_trigger(1, 107);
				if (global[g049] != 0) {
					aainfo[3]._frame = 41;
					kernel_reset_animation(aa[3], 41);
					global[g049] = 0;
					scratch._ae = -1;
				} else {
					global[g145] = -1;
					scratch._8e = 41;
					scratch._a8 = 0;
				}
			}
		} else if (global[player_selected_object] == 8) {
			if (global[g046] != 0) {
				// TODO: display_interface()
			} else {
				player.commands_allowed = false;
				kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
				kernel_timing_trigger(1, 107);
				if (global[g049] != 0) {
					scratch._ac = 0;
					aainfo[3]._frame = 28;
					kernel_reset_animation(aa[3], 28);
					global[g049] = 0;
				} else {
					kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
					aa[3] = kernel_run_animation(kernel_name('a', 2), 104);
					aainfo[3]._active = -1;
					aainfo[3]._frame = 0;
					scratch._98 = 31;
					player.walker_visible = false;
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				}
			}
		}
		goto handled;
	}

	if (player_parse(words_click_on, words_moss, 0)) {
		player.commands_allowed = false;
		if (global[g046] == 0) {
			global[g150] = -1;
			scratch._90 = 15;
		} else {
			global[g154] = 2;
			player.walker_visible = false;
			scratch._a2 = kernel_run_animation_talk('b', 7, 0);
			kernel_position_anim(scratch._a2, player.x, player.y, player.scale, player.depth);
			scratch._b4 = -1;
			kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_PLAYER, 0);
			digi_play_build_ii('b', 1, 1);
			scratch._a4 = 30;
		}
		goto handled;
	}

	if (player_parse(words_click_on, words_abigail, 0)) {
		if (player_has(5) && player_has(8)) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			aa[3] = kernel_run_animation(kernel_name('a', 1), 104);
			scratch._98 = 30;
			aainfo[3]._active = -1;
			global[g049] = -1;
			aainfo[3]._frame = 0;
			player.walker_visible = false;
			player.commands_allowed = false;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
		} else if (player_has(8)) {
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			aa[3] = kernel_run_animation(kernel_name('a', 2), 104);
			aainfo[3]._active = -1;
			aainfo[3]._frame = 0;
			scratch._98 = 31;
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = false;
		} else {
			player.commands_allowed = false;
			global[g150] = -1;
			scratch._90 = 15;
		}
		goto handled;
	}

	if (player_parse(words_click_on, words_edgar, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 51;
		goto handled;
	}

	if (player_parse(words_click_on, words_russel, 0)) {
		player.commands_allowed = false;
		global[g145] = -1;
		scratch._8e = 43;
		goto handled;
	}

	if (player_parse(words_click_on, words_abi_bubble, 0)) {
		player.commands_allowed = false;
		aainfo[3]._frame = 28;
		kernel_reset_animation(aa[3], 28);
		global[g049] = 0;
		goto handled;
	}

	if (player_parse(words_click_on, words_all_bubble, 0)) {
		player.commands_allowed = false;
		aainfo[3]._frame = 33;
		kernel_reset_animation(aa[3], 33);
		global[g049] = 0;
		goto handled;
	}

	if (global[g049] == 0)
		return;

	if (player_parse(words_walk_to, words_ground, 0)) {
		player.commands_allowed = false;
		aainfo[3]._frame = 41;
		kernel_reset_animation(aa[3], 41);
		global[g049] = 0;
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_203_synchronize(Common::Serializer &s) {
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
	s.syncAsSint16LE(scratch._ac);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._b0);
	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
}

void room_203_preload() {
	room_init_code_pointer       = room_203_init;
	room_pre_parser_code_pointer = room_203_pre_parser;
	room_parser_code_pointer     = room_203_parser;
	room_daemon_code_pointer     = room_203_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
