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
#include "mads/forest/extra.h"
#include "mads/forest/global.h"
#include "mads/forest/midi.h"
#include "mads/core/game.h"
#include "mads/core/imath.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/player.h"
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
	int16 _94;
	int16 _96;
	int16 _98;
	int16 _9a;
	int16 _9c;
	int16 _9e;
	int16 _a0;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_221_init1();

static void room_221_init() {
	scratch._a0 = 0;
	midi_stop();
	global[play_background_sounds] = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
		}

		for (int16 count = 0; count < 10; count++) {
			scratch.animation_info[count]._active = 0;
			scratch.animation_info[count]._frame = -1;
			scratch.animation_info[count]._val3 = 0;
			scratch.animation_info[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[14] != 3)
			flags[14]++;
	}

	room_221_init1();
}

static void room_221_init1() {
	if (flags[14] == 1)
		player.walker_visible = 0;

	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 6, 0);
	extra_change_animation(scratch._9c, 122, 137, 100, 3);
	scratch._9a = kernel_run_animation_disp('e', 4, 0);
	extra_change_animation(scratch._9a, 183, 135, 100, 3);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 150;
		player.y = 100;
		player.facing = 2;
	}

	switch (previous_room) {
	case KERNEL_RESTORING_GAME:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	case 199:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = true;
		return;
	case 211:
		aa[1] = kernel_run_animation(kernel_name('b', 1), 101);
		aainfo[1]._active = -1;
		aainfo[1]._val3 = 4;
		scratch._92 = 3;
		return;
	case 220:
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 69;
		return;
	default:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = true;
		return;
	}
}

static void room_221_anim1() {
	int16 result = -1;

	if (global[player_hyperwalked] == -1 && aainfo[0]._frame < 43) {
		aainfo[0]._frame = 43;
		kernel_reset_animation(aa[0], 43);
	}

	if (kernel_anim[aa[0]].frame == aainfo[0]._frame)
		return;

	aainfo[0]._frame = kernel_anim[aa[0]].frame;

	switch (aainfo[0]._frame) {
	case 56:
		digi_play_build(221, 'R', 1, 1);
		scratch._9e = 1;
		break;
	case 60:
		result = 56;
		break;
	case 62:
		result = 61;
		break;
	case 63:
		digi_play_build(221, 'E', 1, 1);
		scratch._9e = 2;
		break;
	case 67:
		result = 63;
		break;
	case 69:
		result = 68;
		break;
	case 70:
		digi_play_build(221, 'r', 2, 1);
		scratch._9e = 5;
		break;
	case 74:
		result = 70;
		break;
	default:
		break;
	}

	if (result >= 0) {
		aainfo[0]._frame = result;
		kernel_reset_animation(aa[0], result);
	}
}

static void room_221_anim2() {
	int16 result = -1;

	if (kernel_anim[aa[1]].frame == aainfo[1]._frame)
		return;

	aainfo[1]._frame = kernel_anim[aa[1]].frame;

	if (aainfo[1]._val3 == 4) {
		switch (aainfo[1]._frame) {
		case 8:
			digi_play_build(221, 'B', 2, 1);
			scratch._9e = 12;
			break;
		case 24:
			digi_play_build(103, '_', 3, 2);
			break;
		case 27:
			result = 26;
			break;
		case 28:
			digi_play_build(221, 'B', 10, 1);
			inter_move_object(feather, PLAYER);
			scratch._9e = 13;
			break;
		case 34:
			result = 28;
			break;
		case 40:
			digi_play_build(101, '_', 1, 2);
			break;
		default:
			break;
		}
	} else if (aainfo[1]._val3 == 5) {
		switch (aainfo[1]._frame) {
		case 2:
			digi_play_build(221, 'R', 3, 1);
			scratch._9e = 14;
			break;
		case 6:
			result = 2;
			break;
		case 8:
			result = 7;
			break;
		case 9:
			digi_play_build(221, 'B', 3, 1);
			scratch._9e = 15;
			global[g009] = 0;
			global_midi_play(13);
			break;
		case 13:
			result = 9;
			break;
		case 18:
			result = 14;
			break;
		case 28:
			digi_play_build(221, 'E', 7, 1);
			scratch._9e = 16;
			break;
		case 32:
			result = 28;
			break;
		case 34:
			result = 33;
			break;
		case 35:
			digi_play_build(221, 'R', 4, 1);
			scratch._9e = 17;
			break;
		case 39:
			result = 35;
			break;
		case 46:
			digi_play_build(221, 'B', 4, 1);
			scratch._9e = 18;
			break;
		case 50:
			result = 46;
			break;
		case 52:
			result = 51;
			break;
		case 53:
			digi_play_build(221, 'E', 8, 1);
			scratch._9e = 19;
			break;
		case 57:
			result = 53;
			break;
		default:
			break;
		}
	}

	if (result >= 0) {
		aainfo[1]._frame = result;
		kernel_reset_animation(aa[1], result);
	}
}

static void room_221_anim3() {
	int16 result = -1;

	if (kernel_anim[aa[2]].frame == aainfo[2]._frame)
		return;

	aainfo[2]._frame = kernel_anim[aa[2]].frame;

	if (aainfo[2]._val3 == 2) {
		switch (aainfo[2]._frame) {
		case 1:
			if (scratch._a0 == 1) {
				aainfo[2]._frame = 10;
				kernel_reset_animation(aa[2], 10);
			}
			break;
		case 3:
			digi_play_build(221, 'E', 2, 1);
			scratch._9e = 6;
			scratch._a0 = 1;
			break;
		case 7:
			result = 3;
			break;
		case 9:
			result = 8;
			break;
		case 11:
			digi_play_build(221, 'E', 3, 1);
			scratch._9e = 7;
			break;
		case 14:
			result = 12;
			break;
		case 16:
			result = 15;
			break;
		case 19:
			digi_play_build(221, 'E', 4, 1);
			scratch._9e = 9;
			break;
		case 23:
			result = 19;
			break;
		default:
			break;
		}
	} else if (aainfo[2]._val3 == 3) {
		switch (aainfo[2]._frame) {
		case 6:
			digi_play_build(221, '_', 2, 2);
			break;
		case 13:
			digi_play_build(221, 'E', 6, 1);
			scratch._9e = 11;
			break;
		case 15:
			result = 13;
			break;
		default:
			break;
		}
	}

	if (result >= 0) {
		aainfo[2]._frame = result;
		kernel_reset_animation(aa[2], result);
	}
}

static void room_221_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (scratch._9e) {
		case 1:
			kernel_timing_trigger(50, 104);
			aainfo[0]._frame = 60;
			kernel_reset_animation(aa[0], 60);
			break;
		case 2:
			kernel_timing_trigger(50, 104);
			aainfo[0]._frame = 68;
			kernel_reset_animation(aa[0], 68);
			break;
		case 3:
		case 4:
		case 8:
			kernel_timing_trigger(50, 104);
			break;
		case 5:
			aainfo[0]._frame = 75;
			kernel_reset_animation(aa[0], 75);
			break;
		case 6:
			kernel_timing_trigger(50, 104);
			aainfo[2]._frame = 8;
			kernel_reset_animation(aa[2], 8);
			break;
		case 7:
			kernel_timing_trigger(50, 104);
			aainfo[2]._frame = 14;
			kernel_reset_animation(aa[2], 14);
			break;
		case 9:
			aainfo[2]._frame = 24;
			kernel_reset_animation(aa[2], 24);
			break;
		case 11:
			if (global[walker_converse_state] != 0) {
				global[walker_converse_state] = 0;
				close_interface(CANDLE_FLY);
			}
			new_room = 211;
			break;
		case 12:
			aainfo[1]._frame = 27;
			kernel_reset_animation(aa[1], 27);
			break;
		case 13:
			aainfo[1]._frame = 35;
			kernel_reset_animation(aa[1], 35);
			break;
		case 14:
			kernel_timing_trigger(50, 104);
			aainfo[1]._frame = 6;
			kernel_reset_animation(aa[1], 6);
			break;
		case 15:
			kernel_timing_trigger(50, 104);
			aainfo[1]._frame = 13;
			kernel_reset_animation(aa[1], 13);
			break;
		case 16:
			kernel_timing_trigger(50, 104);
			aainfo[1]._frame = 32;
			kernel_reset_animation(aa[1], 32);
			break;
		case 17:
			aainfo[1]._frame = 39;
			kernel_reset_animation(aa[1], 39);
			break;
		case 18:
			kernel_timing_trigger(50, 104);
			aainfo[1]._frame = 50;
			kernel_reset_animation(aa[1], 50);
			break;
		case 19:
			aainfo[1]._frame = 57;
			kernel_reset_animation(aa[1], 57);
			break;
		default:
			break;
		}
		break;

	case 26:
		if (scratch._8c == 2) {
			aa[2] = kernel_run_animation(kernel_name('E', 1), 105);
			aainfo[2]._active = -1;
			aainfo[2]._val3 = 2;
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._8c == 3) {
			aa[2] = kernel_run_animation(kernel_name('E', 2), 105);
			aainfo[2]._active = -1;
			aainfo[2]._val3 = 3;
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
			global[g133] = 1;
			global[g143] = 1;
		}
		break;

	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (previous_room == 211) {
			aa[1] = kernel_run_animation(kernel_name('b', 2), 5);
		} else {
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g133] = 0;
			global[g143] = 0;
			player.commands_allowed = true;
		}
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		aainfo[1]._frame = 0;
		aa[1] = kernel_run_animation(kernel_name('b', 2), 102);
		aainfo[1]._active = -1;
		aainfo[1]._val3 = 5;
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		aainfo[1]._frame = 0;
		aa[1] = kernel_run_animation(kernel_name('b', 3), 103);
		aainfo[1]._active = -1;
		aainfo[1]._val3 = 6;
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		break;

	case 103:
		aainfo[1]._active = 0;
		new_room = 301;
		break;

	case 104:
		switch (scratch._9e) {
		case 1:
			aainfo[0]._frame = 62;
			kernel_reset_animation(aa[0], 62);
			break;
		case 2:
			digi_play_build(221, 'B', 1, 1);
			scratch._9e = 3;
			break;
		case 3:
			digi_play_build(221, 'O', 1, 1);
			scratch._9e = 4;
			break;
		case 4:
			aainfo[0]._frame = 69;
			kernel_reset_animation(aa[0], 69);
			break;
		case 6:
			aainfo[2]._frame = 10;
			kernel_reset_animation(aa[2], 10);
			break;
		case 7:
			digi_play_build(221, 'O', 2, 1);
			scratch._9e = 8;
			break;
		case 8:
			aainfo[2]._frame = 17;
			kernel_reset_animation(aa[2], 17);
			break;
		case 14:
			aainfo[1]._frame = 8;
			kernel_reset_animation(aa[1], 8);
			break;
		case 15:
			aainfo[1]._frame = 18;
			kernel_reset_animation(aa[1], 18);
			break;
		case 16:
			aainfo[1]._frame = 34;
			kernel_reset_animation(aa[1], 34);
			break;
		case 17:
			aainfo[1]._frame = 41;
			kernel_reset_animation(aa[1], 41);
			break;
		case 18:
			aainfo[1]._frame = 52;
			kernel_reset_animation(aa[1], 52);
			break;
		default:
			break;
		}
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._active = 0;
		aainfo[2]._val3 = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = true;
		break;

	default:
		break;
	}

	global_anim1(4, scratch._9a, global[g131], &global[g132]);
	global_anim2(6, scratch._9c, global[g141], &global[g142]);
	if (aainfo[0]._active != 0)
		room_221_anim1();
	if (aainfo[1]._active != 0)
		room_221_anim2();
	if (aainfo[2]._active != 0)
		room_221_anim3();
}

static void room_221_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._8c = 3;
		player.command_ready = false;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = false;
		return;
	}

	if (player_parse(78, words_hole, 0)) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._8c = 2;
		player.command_ready = false;
		return;
	}

	if (player_parse(78, words_ivy_leaf, 0)) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._8c = 3;
		player.command_ready = false;
		return;
	}
}

void room_221_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)                 s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)               s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)              s.syncAsSint16LE(v);
	for (AnimationInfo &v : scratch.animation_info) v.synchronize(s);
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
}

void room_221_preload() {
	room_init_code_pointer = room_221_init;
	room_parser_code_pointer = room_221_parser;
	room_daemon_code_pointer = room_221_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
