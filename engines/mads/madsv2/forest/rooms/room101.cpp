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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/digi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/midi.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/rooms/room101.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Struct8 {
	int16 _val1;
	int16 _val2;
	int16 _val3;
	int16 _val4;

	void synchronize(Common::Serializer &s) {
		s.syncMultipleLE(_val1, _val2, _val3, _val4);
	}
};

struct Scratch {
	// Tentative field definitions
	int16 sprite[10];       // Offset 00h
	int16 sequence[10];     // Offset 14h
	int16 animation[11];	// Offset 28h
	Struct8 array1[11];		// Offset 3eh
	int16 _96;				// Offset 96h
	int16 _98;				// Offset 98h
	int16 _9a;				// Offset 9ah
	int16 _9c;				// Offset 9ch
	int16 _9e;				// Offset 9eh
	int16 _a0;				// Offset a0h
	int16 _a2;				// Offset a2h
	int16 _a4;
	int16 _a6;
	int16 _a8;
	int16 _aa;
	int16 _ac;
	int16 _ae;
	int16 _b0;
	int16 _b2;
	int16 _b4;
	int16 _b6;
};

static Scratch scratch;

#define ss    scratch.sprite
#define seq   scratch.sequence
#define aa    scratch.animation

static void room_101_init1() {
	global[g009] = -1;
	viewing_at_y = 22;
	global[player_score] = 0;
	player.walker_visible = false;
	player.commands_allowed = false;
	mouse_hide();

	for (auto &v : scratch.array1) {
		v._val1 = 0;
		v._val2 = -1;
	}

	stop_speech_on_run_animation = false;
	aa[8] = kernel_run_animation(kernel_name('I', 1), 0);
	scratch.array1[8]._val1 = -1;
	scratch.array1[8]._val2 = 8;
	kernel_reset_animation(aa[8], 8);
}

static void room_101_init2() {
	player.commands_allowed = false;
	global[player_score] = 0;

	ss[3] = kernel_load_series(kernel_name('p', 7), 0);
	seq[3] = kernel_seq_stamp(ss[3], 0, -1);
	kernel_seq_depth(seq[3], 8);
	kernel_seq_loc(seq[3], 172, 119);
	kernel_seq_scale(seq[3], 70);

	global[g131] = 0;
	global[g141] = 0;

	scratch._a6 = kernel_run_animation_disp('r', 3, 0);
	kernel_position_anim(scratch._a6, 123, 14, 69, 7);
	scratch._a4 = kernel_run_animation_disp('e', 1, 0);
	kernel_position_anim(scratch._a4, 194, 145, 75, 4);

	global[g009] = -1;
	global_digi_play(8);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 96;
		player.y = 140;
		player.facing = 3;
	}

	aa[6] = kernel_run_animation(kernel_name('b', 1), 104);
	scratch.array1[6]._val1 = -1;
	scratch._9c = 1;
	scratch._ac = 1;	
}

static void room_101_init3() {
	player.commands_allowed = 0;
	global[player_score] = 0;
	global[g009] = 0;
	midi_stop();
	global[g131] = 0;
	global[g141] = 0;

	scratch._a6 = kernel_run_animation_disp('r', 3, 0);
	kernel_position_anim(scratch._a6, 123, 134, 69, 7);
	scratch._a4 = kernel_run_animation_disp('e', 1, 0);
	kernel_position_anim(scratch._a4, 194, 145, 75, 4);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 96;
		player.y = 140;
		player.facing = 3;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._a4, 2);
		kernel_reset_animation(scratch._a6, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = -1;
		return;
	}

	if (previous_room == 104) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		scratch.array1[0]._val1 = -1;
		scratch._9c = 57;
		return;
	}

	if (previous_room == 106) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		scratch.array1[0]._val1 = -1;
		scratch._9c = 77;
		return;
	}

	if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._a4, 2);
		kernel_reset_animation(scratch._a6, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = -1;
		player.walker_visible = -1;
		return;
	}

	if (previous_room == 205) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._a4, 2);
		kernel_reset_animation(scratch._a6, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = 0;
		player.walker_visible = 0;
		global[g009] = 0;
		scratch.array1[5]._val1 = -1;
		aa[5] = kernel_run_animation(kernel_name('b', 8), 111);
		scratch._ae = 4;
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._a4, 2);
	kernel_reset_animation(scratch._a6, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = -1;
	player.walker_visible = -1;
}

static void room_101_init() {
	scratch._ae = scratch._b0 = scratch._b2 = 0;
	scratch._b4 = scratch._b6 = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (Struct8 &s : scratch.array1) {
			s._val1 = 0;
			s._val2 = 1;
			s._val3 = s._val4 = 0;
		}

		if (previous_room != 199 && flags[0] != 3)
			++flags[0];
	}

	if (object_is_here(11)) {
		ss[0] = kernel_load_series("*rm101p11", 0);
		seq[0] = kernel_seq_stamp(ss[0], 0, -1);
		kernel_seq_depth(seq[0], 13);
		kernel_seq_loc(seq[0], 264, 131);
		kernel_seq_scale(seq[0], 68);
	} else {
		kernel_flip_hotspot(164, false);
	}

	if (object_is_here(0)) {
		ss[2] = kernel_load_series("*rm101p10", 0);
		seq[2] = kernel_seq_stamp(ss[2], 0, -1);
		kernel_seq_depth(seq[2], 8);
		kernel_seq_loc(seq[2], 159, 116);
		kernel_seq_scale(seq[2], 100);
	} else {
		kernel_flip_hotspot(105, false);
	}

	if (object_is_here(8)) {
		ss[1] = kernel_load_series(kernel_name('p', 8), 0);
		seq[1] = kernel_seq_stamp(ss[1], 0, -1);
		kernel_seq_depth(seq[1], 1);
		kernel_seq_loc(seq[1], 21, 127);
		kernel_seq_scale(seq[1], 100);
	} else {
		kernel_flip_hotspot(154, false);
	}

	switch (flags[0]) {
	case -3:
		room_101_init1();
		break;
	case -2:
		flags[0] = 1;
		room_101_init2();
		break;
	case 1:
		room_101_init2();
		break;
	default:
		room_101_init3();
		break;
	}
}

static void room_101_daemon() {
	// TODO
}

static void room_101_pre_parser() {
	if (player_parse(13, 16, 0))
		player.walk_off_edge_to_room = 104;

	if (player_parse(13, 17, 0))
		player.walk_off_edge_to_room = 106;
}

static void room_101_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = 0;
		digi_play_build_ii('c', 1, 1);
		scratch._aa = 29;
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(114, 116, 0)) {
		player.commands_allowed = 0;
		global[g145] = -1;
		scratch._98 = 2;
		goto handled;
	}

	if (player_parse(126, 154, 0)) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._9a = 13;
		goto handled;
	}

	if (player_parse(126, 105, 0)) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._9a = 14;
		goto handled;
	}

	if (player_parse(126, 164, 0)) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._9a = 15;
		goto handled;
	}

	if (player_parse(13, 149, 0)) {
		player.commands_allowed = 0;
		player.walker_visible = 0;
		kernel.trigger_setup_mode = 1;
		scratch.array1[5]._val1 = -1;
		aa[5] = kernel_run_animation(kernel_name('b', 5), 111);
		scratch._ae = 1;
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_101_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (auto &v : scratch.array1)     v.synchronize(s);
}

void room_101_preload() {
	room_init_code_pointer = room_101_init;
	room_pre_parser_code_pointer = room_101_pre_parser;
	room_parser_code_pointer = room_101_parser;
	room_daemon_code_pointer = room_101_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
