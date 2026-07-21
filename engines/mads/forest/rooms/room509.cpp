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

#include "mads/forest/digi.h"
#include "mads/forest/global.h"
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/mouse.h"
#include "mads/core/player.h"
#include "mads/core/sprite.h"
#include "mads/mads.h"

namespace MADS {
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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_509_anim1();
static void room_509_anim2();
static void room_509_anim3();
static void room_509_anim4();
static void room_509_anim5();

static void room_509_init() {
	global[g101] = -1;
	mouse_hide();
	global[play_background_sounds] = 0;
	global[g009] = -1;
	global_midi_play(15);
	global[g009] = -1;
	viewing_at_y = 22;
	global[play_background_sounds] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;

	for (int count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = -1;
	}

	aa[0] = kernel_run_animation(kernel_name('F', 1), 0);
	aainfo[0]._active = -1;
}

static void room_509_daemon() {
	if (global[player_hyperwalked] == -1) {
		g_engine->saveAutosaveIfEnabled();
		new_room = 904;
	}

	if (kernel.trigger == 102)
		new_room = 520;

	if (aainfo[0]._active) room_509_anim1();
	if (aainfo[1]._active) room_509_anim2();
	if (aainfo[2]._active) room_509_anim3();
	if (aainfo[3]._active) room_509_anim4();
	if (aainfo[4]._active) room_509_anim5();
}

static void room_509_anim1() {
	int16 aa_frame = kernel_anim[aa[0]].frame;
	if (aa_frame != aainfo[0]._frame) {
		aainfo[0]._frame = aa_frame;
		if (aa_frame == 65) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[0]);
			aainfo[0]._active = 0;
			aa[1] = kernel_run_animation(kernel_name('F', 2), 0);
			aainfo[1]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, aa[0]);
		} else if (aa_frame < 65) {
			if (aa_frame == 2) {
				digi_initial_volume(30);
				scratch._90 = 2;
				digi_play_build(503, '_', 2, 3);
				digi_val2 = -1;
			} else if (aa_frame == 15) {
				digi_play_build(509, 'b', 1, 1);
				scratch._8e = 15;
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._8e == 15) {
			kernel_timing_trigger(30, 28);
			scratch._8e = 16;
		} else if (scratch._8e == 16) {
			scratch._8e = 17;
			digi_play_build(509, 'r', 1, 1);
		} else if (scratch._8e == 17) {
			scratch._8e = -1;
		}
	}

	if (kernel.trigger == 8 && scratch._90 == 2) {
		digi_initial_volume(60);
		scratch._90 = 2;
		digi_play_build(503, '_', 2, 3);
		digi_val2 = -1;
	}
}

static void room_509_anim2() {
	int16 aa_frame = kernel_anim[aa[1]].frame;
	if (aa_frame != aainfo[1]._frame) {
		aainfo[1]._frame = aa_frame;
		if (aa_frame == 2) {
			digi_initial_volume(30);
			scratch._90 = 2;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		} else if (aa_frame == 42) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[1]);
			aainfo[1]._active = 0;
			aa[2] = kernel_run_animation(kernel_name('F', 3), 0);
			aainfo[2]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[1]);
		}
	}

	if (kernel.trigger == 8 && scratch._90 == 2) {
		digi_initial_volume(60);
		scratch._90 = 2;
		digi_play_build(503, '_', 2, 3);
		digi_val2 = -1;
	}
}

static void room_509_anim3() {
	int16 aa_frame = kernel_anim[aa[2]].frame;
	if (aa_frame != aainfo[2]._frame) {
		aainfo[2]._frame = aa_frame;
		if (aa_frame == 2) {
			digi_initial_volume(30);
			scratch._90 = 2;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		} else if (aa_frame == 20) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[2]);
			aainfo[2]._active = 0;
			aa[3] = kernel_run_animation(kernel_name('F', 4), 0);
			aainfo[3]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[2]);
		}
	}

	if (kernel.trigger == 8 && scratch._90 == 2) {
		digi_initial_volume(60);
		scratch._90 = 2;
		digi_play_build(503, '_', 2, 3);
		digi_val2 = -1;
	}
}

static void room_509_anim4() {
	int16 aa_frame = kernel_anim[aa[3]].frame;
	if (aa_frame != aainfo[3]._frame) {
		aainfo[3]._frame = aa_frame;
		if (aa_frame == 53) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[3]);
			aainfo[3]._active = 0;
			aa[4] = kernel_run_animation(kernel_name('F', 5), 0);
			aainfo[4]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, aa[3]);
		} else if (aa_frame < 53) {
			if (aa_frame == 2) {
				digi_initial_volume(30);
				scratch._90 = 2;
				digi_play_build(503, '_', 2, 3);
				digi_val2 = -1;
			} else if (aa_frame == 27) {
				digi_play_build(509, 'r', 2, 1);
				scratch._8e = 27;
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._8e == 27)
			scratch._8e = -1;
	}

	if (kernel.trigger == 8 && scratch._90 == 2) {
		digi_initial_volume(60);
		scratch._90 = 2;
		digi_play_build(503, '_', 2, 3);
		digi_val2 = -1;
	}
}

static void room_509_anim5() {
	int16 aa_frame = kernel_anim[aa[4]].frame;
	if (aa_frame != aainfo[4]._frame) {
		aainfo[4]._frame = aa_frame;
		if (aa_frame == 80) {
			kernel_reset_animation(aa[4], 79);
			aainfo[4]._frame = 79;
		} else if (aa_frame < 80) {
			if (aa_frame == 2) {
				digi_initial_volume(30);
				scratch._90 = 2;
				digi_play_build(503, '_', 2, 3);
				digi_val2 = -1;
			} else if (aa_frame == 27) {
				digi_play_build(509, 'e', 1, 1);
				scratch._8e = 27;
			} else if (aa_frame == 79) {
				kernel_timing_trigger(40, 102);
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._8e == 27) {
			kernel_timing_trigger(2, 28);
			scratch._8e = 16;
		} else if ((uint16)scratch._8e <= 27) {
			if (scratch._8e == 16) {
				scratch._8e = 17;
				digi_play_build(509, 'b', 2, 1);
			} else if (scratch._8e == 17) {
				kernel_timing_trigger(2, 28);
				scratch._8e = 18;
			} else if (scratch._8e == 18) {
				digi_play_build(509, 'r', 3, 1);
				scratch._8e = 19;
			} else if (scratch._8e == 19) {
				scratch._8e = -1;
			}
		}
	}

	if (kernel.trigger == 8 && scratch._90 == 2) {
		digi_initial_volume(60);
		scratch._90 = 2;
		digi_play_build(503, '_', 2, 3);
		digi_val2 = -1;
	}
}

void room_509_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
	s.syncAsSint16LE(scratch._8e);
	s.syncAsSint16LE(scratch._90);
}

void room_509_preload() {
	room_init_code_pointer = room_509_init;
	room_daemon_code_pointer = room_509_daemon;

	global[g016] = -1;
	global_section_walker();
	global_section_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADS
