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
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_220_init() {
	global[player_score] = -1;
	global[g009] = -1;
	global_midi_play(10);
	viewing_at_y = 22;
	player.walker_visible = false;
	player.commands_allowed = false;

	for (int i = 0; i < 10; i++) {
		aainfo[i]._active = 0;
		aainfo[i]._frame = -1;
	}

	aa[0] = kernel_run_animation("*RM220Y11", 0);
	aainfo[0]._active = -1;
	scratch._92 = 121;
}

static void room_220_anim1() {
	int16 result = -1;

	if (kernel_anim[aa[0]].frame == aainfo[0]._frame)
		return;
	aainfo[0]._frame = kernel_anim[aa[0]].frame;
	int16 f = aainfo[0]._frame;

	if (f == 121) {
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		global[g009] = -1;
		global_midi_play(15);
		aa[1] = kernel_run_animation("*RM220Y12", 0);
		aainfo[1]._active = -1;
		scratch._92 = 55;
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_ANIM, aa[1]);
	} else if (f < 121) {
		if (f == 52) {
			result = 51;
		} else if (f < 52) {
			if (f == 43) {
				result = 41;
			} else if (f < 43) {
				if (f == 41) {
					digi_play_build(220, 'E', 1, 1);
					scratch._9e = 3;
				}
			} else {
				if (f == 45) {
					result = 44;
				} else if (f == 47) {
					digi_play_build(220, 'R', 1, 1);
					scratch._9e = 1;
				} else if (f == 50) {
					result = 47;
				}
			}
		} else {
			if (f == 53) {
				digi_play_build(220, 'B', 1, 1);
				scratch._9e = 4;
			} else if (f == 57) {
				result = 53;
			} else if (f == 58) {
				result = 57;
			} else if (f == 85 || f == 92 || f == 98 || f == 103) {
				digi_play_build(220, '_', 1, 2);
			} else if (f == 115) {
				midi_stop();
			}
		}
	}

	if (result >= 0) {
		aainfo[0]._frame = result;
		kernel_reset_animation(aa[0], result);
	}
}

static void room_220_anim2() {
	int16 result = -1;

	if (kernel_anim[aa[1]].frame == aainfo[1]._frame)
		return;
	aainfo[1]._frame = kernel_anim[aa[1]].frame;
	int16 f = aainfo[1]._frame;

	if (f == 55) {
		new_room = 221;
	} else if (f < 55) {
		if (f == 28) {
			digi_play_build(220, '_', 1, 2);
		} else if (f > 28) {
			if (f == 30) {
				digi_play_build(220, 'R', 1, 2);
				scratch._9e = 2;
			} else if (f == 46 || f == 54) {
				digi_play_build(220, '_', 1, 2);
			}
		} else {
			if (f == 4) {
				digi_play_build(220, '_', 2, 2);
			} else if (f == 9) {
				digi_play_build(103, '_', 3, 2);
			} else if (f == 15) {
				digi_play_build(220, '_', 3, 2);
			}
		}
	}

	if (result >= 0) {
		aainfo[1]._frame = result;
		kernel_reset_animation(aa[1], result);
	}
}

static void room_220_daemon() {
	switch (kernel.trigger) {
	case 7:
		kernel_timing_trigger(40, 102);
		switch (scratch._9e) {
		case 1:
			aainfo[0]._frame = 51;
			kernel_reset_animation(aa[0], 51);
			break;
		case 3:
			aainfo[0]._frame = 44;
			kernel_reset_animation(aa[0], 44);
			break;
		case 4:
			aainfo[0]._frame = 57;
			kernel_reset_animation(aa[0], 57);
			break;
		default:
			break;
		}
		break;
	case 102:
		switch (scratch._9e) {
		case 1:
			aainfo[0]._frame = 52;
			kernel_reset_animation(aa[0], 52);
			break;
		case 3:
			aainfo[0]._frame = 46;
			kernel_reset_animation(aa[0], 46);
			break;
		case 4:
			aainfo[0]._frame = 58;
			kernel_reset_animation(aa[0], 58);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	if (aainfo[0]._active != 0)
		room_220_anim1();
	if (aainfo[1]._active != 0)
		room_220_anim2();
}

static void room_220_pre_parser() {
	// No implementation
}

static void room_220_parser() {
	// No implementation
}

void room_220_synchronize(Common::Serializer &s) {
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
}

void room_220_preload() {
	room_init_code_pointer = room_220_init;
	room_pre_parser_code_pointer = room_220_pre_parser;
	room_parser_code_pointer = room_220_parser;
	room_daemon_code_pointer = room_220_daemon;

	flags[13] = 7;
	global[g016] = -1;
	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
