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

#include "mads/forest/rooms/section1.h"
#include "mads/forest/digi.h"
#include "mads/forest/global.h"
#include "mads/forest/midi.h"
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/core/player.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[10];
	AnimationInfo animation_info[10];
	int16 _8e;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo local->animation_info


static void room_420_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur == aainfo[0]._frame)
		return;
	aainfo[0]._frame = cur;
	if (cur != 100)
		return;
	kernel_abort_animation(aa[0]);
	aainfo[0]._active = 0;
	aa[1] = kernel_run_animation(kernel_name('T', 2), 0);
	aainfo[1]._active = -1;
	kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, aa[0]);
}

static void room_420_anim2() {
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur != aainfo[1]._frame) {
		aainfo[1]._frame = cur;
		if (cur == 50) {
			kernel_abort_animation(aa[1]);
			aainfo[1]._active = 0;
			aa[2] = kernel_run_animation(kernel_name('N', 1), 0);
			aainfo[2]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[1]);
		} else if (cur < 50) {
			if (cur == 24) {
				scratch._8e = 24;
				aainfo[1]._val3 = 11;
				if (global[phineas_status] == 2 || global[phineas_status] == 3)
					digi_play_build(420, 'e', 1, 1);
				else
					digi_play_build(420, 'e', 1, 2);
			} else if (cur < 24) {
				if (cur == 5) {
					scratch._8e = 5;
					aainfo[1]._val3 = 9;
					digi_play_build(420, 'r', 1, 1);
				} else if (cur == 19) {
					if (aainfo[1]._val3 == 9) {
						aainfo[1]._frame = 5;
						kernel_reset_animation(aa[1], 5);
					}
				} else if (cur == 23) {
					if (aainfo[1]._val3 == 12) {
						aainfo[1]._frame = 22;
						kernel_reset_animation(aa[1], 22);
					}
				}
			} else {
				if (cur == 29) {
					if (aainfo[1]._val3 == 11) {
						aainfo[1]._frame = 24;
						kernel_reset_animation(aa[1], 24);
					}
				} else if (cur == 33) {
					if (aainfo[1]._val3 == 12) {
						aainfo[1]._frame = 32;
						kernel_reset_animation(aa[1], 32);
					}
				} else if (cur == 34) {
					scratch._8e = 34;
					aainfo[1]._val3 = 10;
					digi_play_build(420, 'b', 1, 1);
					global_midi_play(3);
				} else if (cur == 39) {
					if (aainfo[1]._val3 == 10) {
						aainfo[1]._frame = 34;
						kernel_reset_animation(aa[1], 34);
					}
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	int16 val = scratch._8e;
	if (val == 34) {
		aainfo[1]._val3 = 13;
		aainfo[1]._frame = 39;
		kernel_reset_animation(aa[1], 39);
		scratch._8e = -1;
	} else if ((uint16)val > 34) {
		return;
	} else if (val == 5) {
		aainfo[1]._val3 = 12;
		kernel_timing_trigger(30, 28);
		scratch._8e = 6;
	} else if (val == 6) {
		aainfo[1]._val3 = 13;
		aainfo[1]._frame = 19;
		kernel_reset_animation(aa[1], 19);
		scratch._8e = -1;
	} else if (val == 24) {
		aainfo[1]._val3 = 12;
		kernel_timing_trigger(30, 28);
		scratch._8e = 25;
	} else if (val == 25) {
		aainfo[1]._val3 = 13;
		aainfo[1]._frame = 29;
		kernel_reset_animation(aa[1], 29);
		scratch._8e = -1;
	}
}

static void room_420_anim3() {
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur == aainfo[2]._frame)
		return;
	aainfo[2]._frame = cur;
	if (cur != 82)
		return;
	kernel_abort_animation(aa[2]);
	aainfo[2]._active = 0;
	aa[3] = kernel_run_animation(kernel_name('N', 2), 0);
	aainfo[3]._active = -1;
	kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[2]);
}

static void room_420_anim4() {
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur == aainfo[3]._frame)
		return;
	aainfo[3]._frame = cur;
	if (cur == 10) {
		aainfo[3]._active = 0;
		new_room = 401;
	} else if ((uint16)cur > 10) {
		return;
	} else if (cur == 2) {
		aainfo[3]._val4 = 0;
	} else if (cur == 8) {
		if (aainfo[3]._val4 < 20) {
			aainfo[3]._frame = 7;
			kernel_reset_animation(aa[3], 7);
			aainfo[3]._val4++;
		}
	}
}

static void room_420_init() {
    flags[31] = 1;
    midi_stop();
    global[play_background_sounds] = -1;
    global[g009] = 0;
    player.walker_visible = false;
    player.commands_allowed = false;

    for (int16 count = 0; count < 10; count++) {
        aainfo[count]._active = 0;
        aainfo[count]._frame = -1;
    }

    aa[0] = kernel_run_animation(kernel_name('T', 1), 0);
    aainfo[0]._active = -1;
}

static void room_420_daemon() {
    if (aainfo[0]._active) room_420_anim1();
    if (aainfo[1]._active) room_420_anim2();
    if (aainfo[2]._active) room_420_anim3();
    if (aainfo[3]._active) room_420_anim4();
}

void room_420_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : aainfo) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8e);
}

void room_420_preload() {
	room_init_code_pointer = room_420_init;
	room_daemon_code_pointer = room_420_daemon;

	if (flags[31] == -4)
		global[g016] = -1;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
