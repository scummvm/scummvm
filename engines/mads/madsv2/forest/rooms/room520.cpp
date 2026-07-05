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
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/engine.h"

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

static void room_520_init() {
	ss[0] = kernel_load_series(kernel_name('n', 1), 0);
	seq[0] = kernel_seq_stamp(ss[0], false, -1);
	kernel_seq_depth(seq[0], 7);
	kernel_seq_loc(seq[0], 252, 75);
	kernel_seq_scale(seq[0], 100);

	ss[1] = kernel_load_series(kernel_name('n', 1), 0);
	seq[1] = kernel_seq_stamp(ss[1], false, -1);
	kernel_seq_depth(seq[1], 7);
	kernel_seq_loc(seq[1], 272, 80);
	kernel_seq_scale(seq[1], 100);

	ss[2] = kernel_load_series(kernel_name('p', 3), 0);
	seq[2] = kernel_seq_stamp(ss[2], false, -1);
	kernel_seq_depth(seq[2], 1);
	kernel_seq_loc(seq[2], 175, 90);
	kernel_seq_scale(seq[2], 50);

	viewing_at_y = 22;
	global[play_background_sounds] = 0;
	global[g009] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;
	mouse_hide();

	for (int i = 0; i < 10; i++) {
		aainfo[i]._active = 0;
		aainfo[i]._frame = -1;
	}

	kernel_timing_trigger(100, 101);
	kernel_timing_trigger(5, 100);
}

static void room_520_anim1() {
	int16 aa_frame = kernel_anim[aa[0]].frame;
	if (aa_frame != aainfo[0]._frame) {
		aainfo[0]._frame = aa_frame;
		if (aa_frame == 66) {
			new_room = 503;
		} else if (aa_frame < 66) {
			if (aa_frame == 1) {
				digi_initial_volume(90);
				digi_play_build(503, '_', 2, 3);
				digi_val2 = -1;
				scratch._8e = 1;
			} else if (aa_frame == 3) {
				kernel_seq_delete(seq[0]);
				kernel_seq_delete(seq[1]);
				kernel_seq_delete(seq[2]);
			}
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 28) {
		if (scratch._8e - 1 == 0) {
			digi_initial_volume(60);
			scratch._8e = 1;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		}
	}
}

static void room_520_daemon() {
	if (global[player_hyperwalked] == -1) {
		g_engine->saveAutosaveIfEnabled();
		new_room = 904;
	}

	switch (kernel.trigger) {
	case 100:
		aa[0] = kernel_run_animation(kernel_name('F', 1), 0);
		aainfo[0]._active = -1;
		scratch._8c = 66;
		break;
	case 101:
		global_midi_play(14);
		break;
	}

	if (aainfo[0]._active)
		room_520_anim1();
}

void room_520_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
	s.syncAsSint16LE(scratch._8e);
}

void room_520_preload() {
	room_init_code_pointer       = room_520_init;
	room_daemon_code_pointer     = room_520_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
