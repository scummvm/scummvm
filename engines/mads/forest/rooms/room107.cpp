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
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/mouse.h"
#include "mads/core/player.h"
#include "mads/engine.h"

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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_107_anim1();

static void room_107_init() {
	flags[8] = 5;

	ss[0] = kernel_load_series(kernel_name('b', 1), 0);
	seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
	kernel_seq_depth(seq[0], 1);
	kernel_seq_loc(seq[0], 274, 147);
	kernel_seq_scale(seq[0], 100);

	ss[1] = kernel_load_series(kernel_name('e', 1), 0);
	seq[1] = kernel_seq_stamp(ss[1], false, KERNEL_FIRST);
	kernel_seq_depth(seq[1], 1);
	kernel_seq_loc(seq[1], 167, 115);
	kernel_seq_scale(seq[1], 100);

	ss[2] = kernel_load_series(kernel_name('r', 1), 0);
	seq[2] = kernel_seq_stamp(ss[2], false, KERNEL_FIRST);
	kernel_seq_depth(seq[2], 1);
	kernel_seq_loc(seq[2], 75, 149);
	kernel_seq_scale(seq[2], 100);

	viewing_at_y = 22;
	global[play_background_sounds] = 0;
	global[g009] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;
	mouse_hide();

	for (int16 i = 0; i < 10; i++) {
		aainfo[i]._active = 0;
		aainfo[i]._frame = -1;
	}

	kernel_timing_trigger(5, 100);
}

static void room_107_anim1() {
	int16 prev_frame = aainfo[0]._frame;
	int16 frame = kernel_anim[aa[0]].frame;

	if (frame != prev_frame) {
		aainfo[0]._frame = frame;

		if (frame == 30) {
			new_room = 104;
		} else if (frame < 30) {
			if (frame == 1) {
				kernel_seq_delete(seq[0]);
				kernel_seq_delete(seq[1]);
				kernel_seq_delete(seq[2]);
			} else if (frame == 21) {
				aainfo[0]._val3 = 1;
				digi_play_build(107, 'b', 1, 1);
				scratch._8e = 21;
			} else if (frame == 26) {
				if (aainfo[0]._val3 == 1) {
					aainfo[0]._frame = 21;
					kernel_reset_animation(aa[0], 21);
				}
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._8e == 21) {
			aainfo[0]._val3 = 2;
			scratch._8e = -1;
			global_midi_play(4);
			aainfo[0]._frame = 26;
			kernel_reset_animation(aa[0], 26);
		}
	}
}

static void room_107_daemon() {
	if (global[player_hyperwalked] == -1) {
		g_engine->saveAutosaveIfEnabled();
		new_room = 904;
	}

	if (kernel.trigger == 100) {
		aa[0] = kernel_run_animation(kernel_name('T', 1), 0);
		aainfo[0]._active = -1;
		scratch._8c = 30;
	}

	if (aainfo[0]._active != 0)
		room_107_anim1();
}

static void room_107_pre_parser() {
}

static void room_107_parser() {
}

void room_107_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
}

void room_107_preload() {
	room_init_code_pointer = room_107_init;
	room_pre_parser_code_pointer = room_107_pre_parser;
	room_parser_code_pointer = room_107_parser;
	room_daemon_code_pointer = room_107_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
