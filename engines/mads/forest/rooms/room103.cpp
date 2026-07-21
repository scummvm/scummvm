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
#include "mads/forest/mads/inventory.h"
#include "mads/forest/mads/sounds.h"
#include "mads/forest/mads/words.h"
#include "mads/forest/digi.h"
#include "mads/forest/global.h"
#include "mads/forest/midi.h"
#include "mads/core/game.h"
#include "mads/core/imath.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/matte.h"
#include "mads/core/sound.h"
#include "mads/core/sprite.h"
#include "mads/core/text.h"

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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_103_init() {
	scratch._90 = -1;
	scratch._92 = -1;

	for (int count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = 1;
		aainfo[count]._val3 = 0;
		aainfo[count]._val4 = 0;
	}

	global[play_background_sounds] = -1;
	global[g009] = -1;
	viewing_at_y = 22;
	player.walker_visible = false;
	player.commands_allowed = false;

	for (int count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = -1;
	}

	aa[0] = kernel_run_animation(kernel_name('F', 1), 0);
	aainfo[0]._active = -1;
}

static void room_103_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame) {
		int16 frame = kernel_anim[aa[0]].frame;
		aainfo[0]._frame = frame;

		if (frame == 1) {
			digi_initial_volume(20);
			digi_play_build(101, '_', 5, 3);
			scratch._92 = 1;
			digi_val2 = -1;
		} else if (frame == 273) {
			kernel_abort_animation(aa[0]);
			aainfo[0]._active = 0;
			stop_speech_on_run_animation = false;
			aa[1] = kernel_run_animation(kernel_name('F', 2), 0);
			aainfo[1]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, aa[0]);
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 40) {
		if (scratch._92 == 1) {
			digi_initial_volume(20);
			scratch._92 = 1;
			digi_play_build(101, '_', 5, 3);
			digi_val2 = -1;
		}
	}
}

static void room_103_anim2() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame) {
		int16 frame = kernel_anim[aa[1]].frame;
		aainfo[1]._frame = frame;

		if (frame == 165) {
			digi_val2 = 0;
		} else if (frame > 165) {
			if (frame == 186) {
				aainfo[1]._val3 = 11;
				digi_play_build(103, 'e', 1, 1);
				scratch._90 = 186;
			} else if (frame == 195) {
				if (aainfo[1]._val3 == 11) {
					aainfo[1]._frame = 186;
					kernel_reset_animation(aa[1], 186);
				}
			} else if (frame == 196) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[1]);
				aainfo[1]._active = 0;
				stop_speech_on_run_animation = false;
				aa[2] = kernel_run_animation(kernel_name('F', 3), 0);
				aainfo[2]._active = -1;
				kernel_reset_animation(aa[2], 3);
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, aa[1]);
			}
		} else {
			if (frame == 1) {
				digi_initial_volume(50);
				digi_play_build(101, '_', 5, 3);
				scratch._92 = 1;
				digi_val2 = -1;
			} else if (frame == 160) {
				digi_initial_volume(60);
				digi_play_build(103, '_', 1, 1);
				scratch._90 = 160;
				digi_val2 = -1;
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 40) {
		if (scratch._90 == 160) {
			scratch._90 = -1;
		} else if (scratch._90 == 186) {
			scratch._90 = -1;
			aainfo[1]._val3 = 13;
			aainfo[1]._frame = 195;
			kernel_reset_animation(aa[1], 195);
		}
	}

	if (kernel.trigger == 8) {
		if (scratch._92 == 1) {
			digi_initial_volume(90);
			scratch._92++;
			digi_play_build(101, '_', 5, 3);
			digi_val2 = -1;
		}
	}
}

static void room_103_anim3() {
	if (kernel_anim[aa[2]].frame != aainfo[2]._frame) {
		int16 frame = kernel_anim[aa[2]].frame;
		aainfo[2]._frame = frame;

		switch (frame) {
		case 29:
			aainfo[2]._val3 = 12;
			digi_play_build(103, 'r', 1, 1);
			scratch._90 = 29;
			break;
		case 38:
			if (aainfo[2]._val3 == 12) {
				aainfo[2]._frame = 29;
				kernel_reset_animation(aa[2], 29);
			}
			break;
		case 44:
			aainfo[2]._val3 = 10;
			digi_play_build(103, 'b', 1, 1);
			scratch._90 = 44;
			break;
		case 50:
			if (aainfo[2]._val3 == 10) {
				aainfo[2]._frame = 44;
				kernel_reset_animation(aa[2], 44);
			}
			break;
		case 54:
			aainfo[2]._val3 = 12;
			digi_play_build(103, 'r', 2, 1);
			scratch._90 = 54;
			break;
		case 60:
			if (aainfo[2]._val3 == 12) {
				aainfo[2]._frame = 54;
				kernel_reset_animation(aa[2], 54);
			}
			break;
		case 72:
			aainfo[2]._val3 = 14;
			digi_play_build(103, 'm', 1, 1);
			scratch._90 = 72;
			break;
		case 81:
			if (aainfo[2]._val3 == 14) {
				aainfo[2]._frame = 72;
				kernel_reset_animation(aa[2], 72);
			}
			break;
		case 85:
			aainfo[2]._val3 = 14;
			digi_play_build(103, 'm', 2, 1);
			scratch._90 = 85;
			break;
		case 91:
			if (aainfo[2]._val3 == 14) {
				aainfo[2]._frame = 85;
				kernel_reset_animation(aa[2], 85);
			}
			break;
		case 109:
			aainfo[2]._val3 = 14;
			digi_play_build(103, 'm', 3, 1);
			scratch._90 = 109;
			break;
		case 113:
			if (aainfo[2]._val3 == 14) {
				aainfo[2]._frame = 109;
				kernel_reset_animation(aa[2], 109);
			}
			break;
		case 120:
			digi_play_build(104, '_', 2, 2);
			scratch._92 = 120;
			break;
		case 140:
			digi_play_build(103, '_', 3, 2);
			scratch._92 = 140;
			break;
		case 166:
			dont_frag_the_palette();
			kernel_abort_animation(aa[2]);
			aainfo[2]._active = 0;
			stop_speech_on_run_animation = false;
			aa[3] = kernel_run_animation(kernel_name('F', 4), 0);
			aainfo[3]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, aa[2]);
			break;
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 40) {
		switch (scratch._90) {
		case 29:
			scratch._90 = -1;
			aainfo[2]._val3 = 13;
			aainfo[2]._frame = 40;
			kernel_reset_animation(aa[2], 40);
			break;
		case 44:
			scratch._90 = -1;
			aainfo[2]._val3 = 13;
			aainfo[2]._frame = 50;
			kernel_reset_animation(aa[2], 50);
			break;
		case 54:
			scratch._90 = -1;
			aainfo[2]._val3 = 13;
			aainfo[2]._frame = 60;
			kernel_reset_animation(aa[2], 60);
			break;
		case 72:
			scratch._90 = -1;
			aainfo[2]._val3 = 13;
			aainfo[2]._frame = 81;
			kernel_reset_animation(aa[2], 81);
			break;
		case 85:
			scratch._90 = -1;
			global[play_background_sounds] = 0;
			midi_stop();
			aainfo[2]._val3 = 13;
			aainfo[2]._frame = 92;
			kernel_reset_animation(aa[2], 92);
			break;
		case 109:
			scratch._90 = -1;
			aainfo[2]._val3 = 13;
			aainfo[2]._frame = 113;
			kernel_reset_animation(aa[2], 113);
			break;
		}
	}

	if (kernel.trigger == 8) {
		scratch._92 = -1;
	}
}

static void room_103_anim4() {
	if (kernel_anim[aa[3]].frame != aainfo[3]._frame) {
		int16 frame = kernel_anim[aa[3]].frame;
		aainfo[3]._frame = frame;

		if (frame == 50) {
			stop_speech_on_run_animation = true;
			kernel_abort_animation(aa[3]);
			aainfo[3]._active = 0;
			new_room = 104;
		} else if (frame < 50) {
			switch (frame) {
			case 16:
				global_midi_play(8);
				aainfo[3]._val3 = 15;
				digi_play_build(103, 'c', 1, 1);
				scratch._90 = 16;
				break;
			case 21:
				if (aainfo[3]._val3 == 15) {
					aainfo[3]._frame = 16;
					kernel_reset_animation(aa[3], 16);
				}
				break;
			case 37:
				aainfo[3]._val3 = 15;
				digi_play_build(103, 'c', 2, 1);
				scratch._90 = 37;
				break;
			case 40:
				if (aainfo[3]._val3 == 15) {
					aainfo[3]._frame = 37;
					kernel_reset_animation(aa[3], 37);
				}
				break;
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 40) {
		if (scratch._90 == 16) {
			scratch._90 = -1;
			aainfo[3]._val3 = 13;
			aainfo[3]._frame = 21;
			kernel_reset_animation(aa[3], 21);
		} else if (scratch._90 == 37) {
			scratch._90 = -1;
			aainfo[3]._val3 = 13;
			aainfo[3]._frame = 40;
			kernel_reset_animation(aa[3], 40);
		} else {
			scratch._90 = -1;
		}
	}

	if (kernel.trigger == 8) {
		scratch._92 = -1;
	}
}

static void room_103_daemon() {
	if (flags[0] < 0 && global[player_hyperwalked] == -1) {
		flags[0] = 0;
		flags[1] = 0;
		flags[2] = 0;
		flags[3] = 0;
		new_room = 904;
	}

	if (aainfo[0]._active != 0) room_103_anim1();
	if (aainfo[1]._active != 0) room_103_anim2();
	if (aainfo[2]._active != 0) room_103_anim3();
	if (aainfo[3]._active != 0) room_103_anim4();
}

static void room_103_pre_parser() {
	if (player_parse(words_walk_to, words_room_101, 0))
		player.walk_off_edge_to_room = 101;
}


static void room_103_parser() {
	// No implementation
}

void room_103_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)         s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)       s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)      s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
	s.syncAsSint16LE(scratch._8e);
	s.syncAsSint16LE(scratch._90);
	s.syncAsSint16LE(scratch._92);
}

void room_103_preload() {
	room_init_code_pointer = room_103_init;
	room_pre_parser_code_pointer = room_103_pre_parser;
	room_parser_code_pointer = room_103_parser;
	room_daemon_code_pointer = room_103_daemon;

	if (flags[1] == -4)
		global[g016] = -1;
	if (previous_room != 199) {
		global_section_walker();
		global_section_interface();
	}

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
