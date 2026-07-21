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
#include "mads/forest/mads/words.h"
#include "mads/forest/digi.h"
#include "mads/forest/global.h"
#include "mads/forest/midi.h"
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/player.h"

namespace MADS {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[10];
	AnimationInfo animation_info[10];
	int16 _8c;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_211_init() {
	midi_stop();
	global[play_background_sounds] = -1;
	viewing_at_y = 22;
	global[g009] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;

	for (int i = 0; i < 10; i++) {
		aainfo[i]._active = 0;
		aainfo[i]._frame = -1;
	}

	kernel_timing_trigger(5, 100);
}

static void room_211_anim1() {
	if (kernel_anim[aa[0]].frame == aainfo[0]._frame)
		return;
	aainfo[0]._frame = kernel_anim[aa[0]].frame;
	int16 f = aainfo[0]._frame;

	if (f == 75) {
		new_room = 221;
		return;
	}
	if (f > 75)
		return;

	if (f == 3) {
		digi_play_build(211, '_', 500, 1);
	} else if (f == 13 || f == 19 || f == 23 || f == 28) {
		digi_play_build(220, '_', 1, 2);
	} else if (f == 34) {
		digi_play_build(211, '_', 700, 1);
	}
}

static void room_211_daemon() {
	switch (kernel.trigger) {
	case 100:
		aa[0] = kernel_run_animation(kernel_name('y', 1), 0);
		aainfo[0]._active = -1;
		scratch._8c = 75;
		break;
	case 101:
		digi_play_build(221, 'e', 6, 1);
		break;
	default:
		break;
	}

	if (aainfo[0]._active != 0)
		room_211_anim1();
}

static void room_211_pre_parser() {
	// No implementation
}

static void room_211_parser() {
	// No implementation
}

void room_211_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
}

void room_211_preload() {
	room_init_code_pointer = room_211_init;
	room_pre_parser_code_pointer = room_211_pre_parser;
	room_parser_code_pointer = room_211_parser;
	room_daemon_code_pointer = room_211_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADS
