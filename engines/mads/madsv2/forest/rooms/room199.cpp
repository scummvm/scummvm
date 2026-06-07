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

#include "mads/madsv2/core/digi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/rooms/room199.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];            /* 0x00 — sprite series handles  */
	int16 sequence[10];          /* 0x14 — sequence handles       */
	int16 animation[10];         /* 0x28 — animation handles      */
	AnimationInfo animation_info[10]; /* 0x3C */
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_199_init1() {
	if (flags[15] >= 1) {
		ss[1] = kernel_load_series("*rm199n2", 0); seq[7] = kernel_seq_stamp(ss[1], false, 1); kernel_seq_depth(seq[7], 5);
	}
	if (flags[16] >= 1) {
		ss[2] = kernel_load_series("*rm199n3", 0); seq[8] = kernel_seq_stamp(ss[2], false, 1); kernel_seq_depth(seq[8], 5);
	}
	if (flags[17] >= 1 || flags[24] >= 1) {
		ss[3] = kernel_load_series("*rm199n5", 0); seq[9] = kernel_seq_stamp(ss[3], false, 1); kernel_seq_depth(seq[9], 5);
	}
	if (flags[18] >= 1) {
		ss[4] = kernel_load_series("*rm199n4", 0); aa[0] = kernel_seq_stamp(ss[4], false, 1); kernel_seq_depth(aa[0], 5);
	}
	if (flags[19] >= 1) {
		ss[5] = kernel_load_series("*rm199n7", 0); aa[1] = kernel_seq_stamp(ss[5], false, 1); kernel_seq_depth(aa[1], 5);
	}
	if (flags[20] >= 1) {
		ss[6] = kernel_load_series("*rm199n8", 0); aa[2] = kernel_seq_stamp(ss[6], false, 1); kernel_seq_depth(aa[2], 5);
	}
	if (flags[21] >= 1 || flags[25] >= 1) {
		ss[7] = kernel_load_series("*rm199n6", 0); aa[3] = kernel_seq_stamp(ss[7], false, 1); kernel_seq_depth(aa[3], 5);
	}
	if (flags[22] >= 1) {
		ss[8] = kernel_load_series("*rm199n9", 0); aa[4] = kernel_seq_stamp(ss[8], false, 1); kernel_seq_depth(aa[4], 5);
	}
	if (flags[26] >= 1 || flags[31] >= 1) {
		ss[9] = kernel_load_series("*rm199n10", 0); aa[5] = kernel_seq_stamp(ss[9], false, 1); kernel_seq_depth(aa[5], 5);
	}
	if (flags[27] >= 1) {
		seq[0] = kernel_load_series("*rm199n11", 0); aa[6] = kernel_seq_stamp(seq[0], false, 1); kernel_seq_depth(aa[6], 5);
	}
	if (flags[28] >= 1) {
		seq[1] = kernel_load_series("*rm199n12", 0); aa[7] = kernel_seq_stamp(seq[1], false, 1); kernel_seq_depth(aa[7], 5);
	}
	if (flags[29] >= 1) {
		seq[2] = kernel_load_series("*rm199n13", 0); aa[8] = kernel_seq_stamp(seq[2], false, 1); kernel_seq_depth(aa[8], 5);
	}
	if (flags[30] >= 1) {
		seq[3] = kernel_load_series("*rm199n14", 0); aa[9] = kernel_seq_stamp(seq[3], false, 1); kernel_seq_depth(aa[9], 5);
	}
	if (flags[32] >= 1) {
		seq[4] = kernel_load_series("*rm199n15", 0); aainfo[0]._active = kernel_seq_stamp(seq[4], false, 1); kernel_seq_depth(aainfo[0]._active, 5);
	}
	if (flags[33] >= 1) {
		seq[5] = kernel_load_series("*rm199n16", 0); aainfo[0]._frame = kernel_seq_stamp(seq[5], false, 1); kernel_seq_depth(aainfo[0]._frame, 5);
	}
}

static void room_199_init() {
	global[player_score] = 0;
	global[g009] = -1;
	global_digi_play(7);

	aainfo[2]._val3 = -1;
	aainfo[2]._val4 = 0;
	aainfo[3]._active = 0;
	aainfo[3]._frame = -1;

	if (previous_room == 304 || previous_room == 402) {
		if (global[g070] != 0) {
			aainfo[2]._frame = 1;
			ss[1] = kernel_load_series("*rm199z1", 0);
			kernel_flip_hotspot(99, false);
			kernel_flip_hotspot(174, false);
			int16 slot = aainfo[2]._frame;
			seq[6 + slot] = kernel_seq_stamp(ss[slot], false, 1);
			kernel_seq_depth(seq[6 + slot], 1);
			aainfo[2]._active = slot;
			player.commands_allowed = 0;
			kernel_timing_trigger(300, 111);
			return;
		}
		room_199_init1();
		aainfo[2]._frame = 0;
		ss[0] = kernel_load_series("*rm199n1", 0);
	} else if (previous_room == 503) {
		aainfo[2]._frame = 3;
		ss[3] = kernel_load_series("*rm199v1", 0);
		kernel_flip_hotspot(99, false);
		kernel_flip_hotspot(174, false);
		int16 slot = aainfo[2]._frame;
		seq[6 + slot] = kernel_seq_stamp(ss[slot], false, 1);
		kernel_seq_depth(seq[6 + slot], 5);
		aainfo[2]._active = slot;
		player.commands_allowed = 0;
		kernel_timing_trigger(2, 112);
		return;
	} else {
		room_199_init1();
		aainfo[2]._frame = 0;
		ss[0] = kernel_load_series("*rm199n1", 0);
	}

	kernel_flip_hotspot(99, false);
	kernel_flip_hotspot(174, false);
	int16 slot = aainfo[2]._frame;
	seq[6 + slot] = kernel_seq_stamp(ss[slot], false, 1);
	kernel_seq_depth(seq[6 + slot], 1);
	aainfo[2]._active = slot;
}

static int room_199_anim1() {
	if (!player_has(6))  return 0;
	if (!player_has(12)) return 0;
	if (!player_has(0))  return 0;
	if (!player_has(2))  return 0;
	if (!player_has(14)) return 0;
	if (!player_has(4))  return 0;
	if (!player_has(8))  return 0;
	if (!player_has(10)) return 0;
	if (!player_has(9))  return 0;
	if (!player_has(11)) return 0;
	if (!player_has(7))  return 0;
	return 1;
}

static void room_199_anim2() {
	if (kernel.trigger == 7)
		kernel_timing_trigger(30, 28);
	if (kernel.trigger != 28)
		return;

	switch (aainfo[2]._val3) {
	case 1:
		if (!player_has(14)) digi_play_build(521, 'e', 6, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 2; return;
	case 2:
		if (!player_has(11)) digi_play_build(521, 'e', 10, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 3; return;
	case 3:
		if (!player_has(12)) digi_play_build(521, 'e', 3, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 4; return;
	case 4:
		if (!player_has(10)) digi_play_build(521, 'e', 8, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 5; return;
	case 5:
		if (!player_has(9)) digi_play_build(521, 'e', 9, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 6; return;
	case 6:
		if (!player_has(8)) digi_play_build(521, 'e', 20, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 7; return;
	case 7:
		if (!player_has(2)) digi_play_build(521, 'e', 5, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 8; return;
	case 8:
		if (!player_has(6)) digi_play_build(521, 'e', 2, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 9; return;
	case 9:
		if (!player_has(4)) digi_play_build(521, 'e', 7, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 10; return;
	case 10:
		if (!player_has(0)) digi_play_build(521, 'e', 4, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 11; return;
	case 11:
		if (!player_has(7)) digi_play_build(521, 'e', 21, 1);
		else kernel_timing_trigger(2, 28);
		aainfo[2]._val3 = 12; return;
	case 12:
		digi_play_build(521, 'b', 1, 1);
		aainfo[2]._val3 = 20; return;
	case 14:
		aainfo[2]._val4 = 0;
		aainfo[2]._val3 = -1;
		aainfo[3]._active = 0;
		kernel_timing_trigger(2, 113);
		return;
	default:
		return;
	}
}

static void room_199_anim3() {
	switch (aainfo[2]._val3) {
	case 1:  digi_play_build(521, 'e', 6, 1);  break;
	case 2:  digi_play_build(521, 'e', 10, 1); break;
	case 3:  digi_play_build(521, 'e', 3, 1);  break;
	case 4:  digi_play_build(521, 'e', 8, 1);  break;
	case 5:  digi_play_build(521, 'e', 9, 1);  break;
	case 6:  digi_play_build(521, 'e', 20, 1); break;
	case 7:  digi_play_build(521, 'e', 5, 1);  break;
	case 8:  digi_play_build(521, 'e', 2, 1);  break;
	case 9:  digi_play_build(521, 'e', 7, 1);  break;
	case 10: digi_play_build(521, 'e', 4, 1);  break;
	case 11: digi_play_build(521, 'e', 21, 1); break;
	default: break;
	}
}

static void room_199_anim4() {
	if (kernel.trigger == 7) {
		kernel_timing_trigger(15, 28);
		aainfo[2]._val3++;
	}
	if (kernel.trigger != 28)
		return;

	switch (aainfo[2]._val3) {
	case 1:
		if (player_has(14)) {
			ss[8] = kernel_load_series("*ob014i", 0);
			aa[4] = kernel_seq_stamp(ss[8], false, 1);
			kernel_seq_depth(aa[4], 1);
			kernel_seq_loc(aa[4], 213, 42);
			kernel_seq_scale(aa[4], 88);
		}
		kernel_timing_trigger(15, 115); return;
	case 2:
		if (player_has(11)) {
			seq[3] = kernel_load_series("*ob011i", 0);
			aa[9] = kernel_seq_stamp(seq[3], false, 1);
			kernel_seq_depth(aa[9], 1);
			kernel_seq_loc(aa[9], 250, 41);
		}
		kernel_timing_trigger(15, 115); return;
	case 3:
		if (player_has(12)) {
			ss[5] = kernel_load_series("*ob012i", 0);
			aa[1] = kernel_seq_stamp(ss[5], false, 1);
			kernel_seq_depth(aa[1], 1);
			kernel_seq_loc(aa[1], 287, 39);
			kernel_seq_scale(aa[1], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 4:
		if (player_has(10)) {
			seq[1] = kernel_load_series("*ob010i", 0);
			aa[7] = kernel_seq_stamp(seq[1], false, 1);
			kernel_seq_depth(aa[7], 1);
			kernel_seq_loc(aa[7], 213, 73);
			kernel_seq_scale(aa[7], 79);
		}
		kernel_timing_trigger(15, 115); return;
	case 5:
		if (player_has(9)) {
			seq[2] = kernel_load_series("*ob009i", 0);
			aa[8] = kernel_seq_stamp(seq[2], false, 1);
			kernel_seq_depth(aa[8], 1);
			kernel_seq_loc(aa[8], 248, 73);
			kernel_seq_scale(aa[8], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 6:
		if (player_has(8)) {
			seq[0] = kernel_load_series("*ob008i", 0);
			aa[6] = kernel_seq_stamp(seq[0], false, 1);
			kernel_seq_depth(aa[6], 1);
			kernel_seq_loc(aa[6], 287, 72);
			kernel_seq_scale(aa[6], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 7:
		if (player_has(2)) {
			ss[7] = kernel_load_series("*ob002i", 0);
			aa[3] = kernel_seq_stamp(ss[7], false, 1);
			kernel_seq_depth(aa[3], 1);
			kernel_seq_loc(aa[3], 212, 108);
			kernel_seq_scale(aa[3], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 8:
		if (player_has(6)) {
			ss[4] = kernel_load_series("*ob006i", 0);
			aa[0] = kernel_seq_stamp(ss[4], false, 1);
			kernel_seq_depth(aa[0], 1);
			kernel_seq_loc(aa[0], 249, 105);
			kernel_seq_scale(aa[0], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 9:
		if (player_has(4)) {
			ss[9] = kernel_load_series("*ob004i", 0);
			aa[5] = kernel_seq_stamp(ss[9], false, 1);
			kernel_seq_depth(aa[5], 1);
			kernel_seq_loc(aa[5], 285, 108);
			kernel_seq_scale(aa[5], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 10:
		if (player_has(0)) {
			ss[6] = kernel_load_series("*ob000i", 0);
			aa[2] = kernel_seq_stamp(ss[6], false, 1);
			kernel_seq_depth(aa[2], 1);
			kernel_seq_loc(aa[2], 230, 142);
			kernel_seq_scale(aa[2], 82);
		}
		kernel_timing_trigger(15, 115); return;
	case 11:
		if (player_has(7)) {
			seq[4] = kernel_load_series("*ob007i", 0);
			aainfo[0]._active = kernel_seq_stamp(seq[4], false, 1);
			kernel_seq_depth(aainfo[0]._active, 1);
			kernel_seq_loc(aainfo[0]._active, 270, 144);
			kernel_seq_scale(aainfo[0]._active, 85);
		}
		kernel_timing_trigger(15, 115); return;
	case 12:
		aainfo[2]._val4 = 0;
		aainfo[2]._val3 = -1;
		kernel_timing_trigger(30, 114);
		return;
	default:
		return;
	}
}

static void room_199_daemon() {
	int16 trig = kernel.trigger;

	if (trig == 7) {
		if (aainfo[3]._frame == 11) {
			aainfo[3]._frame = -1;
			kernel_timing_trigger(30, 113);
		}
	} else if (trig == 111) {
		global[g070] = 0;
		leave_journal();
		player.commands_allowed = -1;
	} else if (trig == 112) {
		aainfo[2]._val3 = 0;
		aainfo[2]._val4 = -1;
		digi_play_build(521, 'e', 1, 1);
	} else if (trig == 113) {
		leave_journal();
		player.commands_allowed = -1;
	} else if (trig == 114) {
		int result = room_199_anim1();
		global[g100] = result;
		if (result) {
			digi_play_build(521, 'e', 11, 1);
			aainfo[3]._frame = 11;
		} else {
			aainfo[2]._val3 = 1;
			aainfo[3]._active = -1;
			digi_play_build(521, 'e', 12, 1);
			room_199_anim2();
		}
	} else if (trig == 115) {
		room_199_anim3();
	}

	if (aainfo[2]._val4 != 0)
		room_199_anim4();
	if (aainfo[3]._active != 0)
		room_199_anim2();
}

static void room_199_pre_parser() {
	// No implementation
}

static void room_199_parser2() {
	if (flags[33] >= 1) {
		kernel_seq_delete(aainfo[0]._frame); matte_deallocate_series(seq[5], -1);
	}
	if (flags[32] >= 1) {
		kernel_seq_delete(aainfo[0]._active); matte_deallocate_series(seq[4], -1);
	}
	if (flags[30] >= 1) {
		kernel_seq_delete(aa[9]);            matte_deallocate_series(seq[3], -1);
	}
	if (flags[29] >= 1) {
		kernel_seq_delete(aa[8]);            matte_deallocate_series(seq[2], -1);
	}
	if (flags[28] >= 1) {
		kernel_seq_delete(aa[7]);            matte_deallocate_series(seq[1], -1);
	}
	if (flags[27] >= 1) {
		kernel_seq_delete(aa[6]);            matte_deallocate_series(seq[0], -1);
	}
	if (flags[26] >= 1 || flags[31] >= 1) {
		kernel_seq_delete(aa[5]); matte_deallocate_series(ss[9], -1);
	}
	if (flags[22] >= 1) {
		kernel_seq_delete(aa[4]);            matte_deallocate_series(ss[8], -1);
	}
	if (flags[21] >= 1 || flags[25] >= 1) {
		kernel_seq_delete(aa[3]); matte_deallocate_series(ss[7], -1);
	}
	if (flags[20] >= 1) {
		kernel_seq_delete(aa[2]);            matte_deallocate_series(ss[6], -1);
	}
	if (flags[19] >= 1) {
		kernel_seq_delete(aa[1]);            matte_deallocate_series(ss[5], -1);
	}
	if (flags[18] >= 1) {
		kernel_seq_delete(aa[0]);            matte_deallocate_series(ss[4], -1);
	}
	if (flags[17] >= 1 || flags[24] >= 1) {
		kernel_seq_delete(seq[9]); matte_deallocate_series(ss[3], -1);
	}
	if (flags[16] >= 1) {
		kernel_seq_delete(seq[8]);           matte_deallocate_series(ss[2], -1);
	}
	if (flags[15] >= 1) {
		kernel_seq_delete(seq[7]);           matte_deallocate_series(ss[1], -1);
	}
}

static void room_199_parser1() {
	if (aainfo[2]._active == 2)
		kernel_flip_hotspot(99, 0);
	if (aainfo[2]._active == 3)
		kernel_flip_hotspot(174, 0);

	int16 old_slot = aainfo[2]._active;
	kernel_seq_delete(seq[6 + old_slot]);
	matte_deallocate_series(ss[old_slot], -1);

	if (aainfo[2]._active == 0)
		room_199_parser2();

	int16 new_slot = aainfo[2]._frame;
	switch (new_slot) {
	case 0:
		room_199_init1();
		ss[new_slot] = kernel_load_series("*rm199n1", 0);
		break;
	case 1:
		ss[new_slot] = kernel_load_series("*rm199z1", 0);
		break;
	case 2:
		ss[new_slot] = kernel_load_series("*rm199i1", 0);
		kernel_flip_hotspot(99, -1);
		break;
	case 3:
		ss[new_slot] = kernel_load_series("*rm199v1", 0);
		kernel_flip_hotspot(174, -1);
		break;
	}

	seq[6 + new_slot] = kernel_seq_stamp(ss[new_slot], false, 1);
	kernel_seq_depth(seq[6 + new_slot], 1);
	aainfo[2]._active = aainfo[2]._frame;
	digi_play_build_ii('_', 1, 2);
}

static void room_199_parser() {
	if (player_parse(174, 121, 0)) { digi_play_build(521, 'e', 6, 2);  goto handled; }
	if (player_parse(174, 164, 0)) { digi_play_build(521, 'e', 10, 2); goto handled; }
	if (player_parse(174, 112, 0)) { digi_play_build(521, 'e', 3, 2);  goto handled; }
	if (player_parse(174, 163, 0)) { digi_play_build(521, 'e', 8, 2);  goto handled; }
	if (player_parse(174, 158, 0)) { digi_play_build(521, 'e', 9, 2);  goto handled; }
	if (player_parse(174, 154, 0)) { digi_play_build(521, 'e', 20, 2); goto handled; }
	if (player_parse(174, 96, 0))  { digi_play_build(521, 'e', 5, 2);  goto handled; }
	if (player_parse(174, 132, 0)) { digi_play_build(521, 'e', 2, 2);  goto handled; }
	if (player_parse(174, 113, 0)) { digi_play_build(521, 'e', 7, 2);  goto handled; }
	if (player_parse(174, 105, 0)) { digi_play_build(521, 'e', 4, 2);  goto handled; }
	if (player_parse(174, 145, 0)) { digi_play_build(521, 'e', 21, 2); goto handled; }

	if (player_parse(99, 82, 0))  { digi_play_build_ii('e', 9, 2);  goto handled; }
	if (player_parse(99, 131, 0)) { digi_play_build_ii('e', 5, 2);  goto handled; }
	if (player_parse(99, 142, 0)) { digi_play_build_ii('e', 3, 2);  goto handled; }
	if (player_parse(99, 146, 0)) { digi_play_build_ii('e', 6, 2);  goto handled; }
	if (player_parse(99, 150, 0)) { digi_play_build_ii('e', 8, 2);  goto handled; }
	if (player_parse(99, 95, 0))  { digi_play_build_ii('e', 11, 2); goto handled; }
	if (player_parse(99, 118, 0)) { digi_play_build_ii('e', 1, 2);  goto handled; }
	if (player_parse(99, 103, 0)) { digi_play_build_ii('e', 4, 2);  goto handled; }
	if (player_parse(99, 83, 0))  { digi_play_build_ii('e', 7, 2);  goto handled; }
	if (player_parse(99, 172, 0)) { digi_play_build_ii('e', 12, 2); goto handled; }
	if (player_parse(99, 77, 0))  { digi_play_build_ii('e', 2, 2);  goto handled; }

	if (player_parse(94, 0)) {
		leave_journal();
		goto handled;
	}

	if (player_parse(156, 0)) {
		if (aainfo[2]._active != 3) {
			aainfo[2]._active = aainfo[2]._frame;
			aainfo[2]._frame++;
		}
		if (aainfo[2]._frame != aainfo[2]._active)
			room_199_parser1();
		goto handled;
	}

	if (player_parse(90, 0)) {
		if (aainfo[2]._active != 0) {
			aainfo[2]._active = aainfo[2]._frame;
			aainfo[2]._frame--;
		}
		if (aainfo[2]._frame != aainfo[2]._active)
			room_199_parser1();
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_199_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
}

void room_199_preload() {
	room_init_code_pointer = room_199_init;
	room_pre_parser_code_pointer = room_199_pre_parser;
	room_parser_code_pointer = room_199_parser;
	room_daemon_code_pointer = room_199_daemon;

	global[g016] = -1;
	player.walker_visible = false;
	global_section_walker();
	global_section_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
