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

#include "mads/madsv2/forest/rooms/section2.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
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
	int16 _a0;
	int16 _a2;
	int16 _a4;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_205_init1() {
	global[g131] = 0;
	global[g141] = 0;
	scratch._9c = kernel_run_animation_disp('r', 9, 0);
	extra_change_animation(scratch._9c, 76, 130, 62, 5);
	scratch._9a = kernel_run_animation_disp('e', 9, 0);
	extra_change_animation(scratch._9a, 120, 138, 74, 4);

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	}

	player.x = 101;
	player.y = 124;
	player.facing = 3;

	if (previous_room == 101) {
		global[play_background_sounds] = 0;
		global[g009] = 0;
		digi_stop(3);
		midi_stop();
		viewing_at_y = 22;
		flags[10]--;
		player.commands_allowed = false;
		player.walker_visible = false;
		aa[6] = kernel_run_animation(kernel_name('t', 1), 104);
		aainfo[6]._active = -1;
		return;
	} else if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
	} else if (previous_room == 204) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 57;
		return;
	} else if (previous_room == 301) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 91;
		return;
	} else {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
	}

	player.commands_allowed = true;
	player.walker_visible = true;
}

static void room_205_init() {
	scratch._a4 = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int count = 0; count < 10; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame = -1;
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[10] != 3)
			flags[10]++;
	}

	if (flags[10] == -3)
		flags[10] = 1;
	room_205_init1();
}

static void room_205_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_205_anim2() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame)
		aainfo[1]._frame = kernel_anim[aa[1]].frame;

	if (scratch._a4 == 0)
		return;

	kernel_random_frame(scratch._9e, &global[g151], global[g154]);
	kernel_random_frame(scratch._a2, &global[g153], global[g156]);

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a4 == 1) {
		global[g154] = 0;
		kernel_timing_trigger(20, 28);
		scratch._a4 = 2;
	} else if (scratch._a4 == 2) {
		global[g156] = 2;
		scratch._a4 = 3;
		digi_play_build(205, 'e', 1, 1);
	} else if (scratch._a4 == 3) {
		scratch._a4 = 4;
		kernel_abort_animation(scratch._9e);
		kernel_abort_animation(scratch._a2);
		player.commands_allowed = true;
		player.walker_visible = true;
		global[g017] = -1;
		aainfo[1]._active = 0;
		kernel_synch(2, 0, 4, 0);
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(3, scratch._9a, 4, 0);
		kernel_synch(3, scratch._9c, 4, 0);
		global[g133] = 0;
		global[g143] = 0;
	}
}

static void room_205_anim3() {
	if (kernel_anim[aa[3]].frame != aainfo[3]._frame)
		aainfo[3]._frame = kernel_anim[aa[3]].frame;
	int16 frame = aainfo[3]._frame;

	if (global[player_hyperwalked] == -1 && frame <= 23) {
		aainfo[3]._frame = 23;
		kernel_reset_animation(aa[3], 23);
		kernel_synch(3, aa[3], 4, 0);
		return;
	}

	if (global[player_hyperwalked] == -1 || frame == 45) {
		aainfo[3]._frame = 44;
		kernel_reset_animation(aa[3], 44);
		kernel_synch(3, aa[3], 4, 0);
	}
}

static void room_205_anim4() {
	if (kernel_anim[aa[2]].frame != aainfo[2]._frame)
		aainfo[2]._frame = kernel_anim[aa[2]].frame;
	int16 frame = aainfo[2]._frame;

	if (global[player_hyperwalked] == -1 && frame <= 23) {
		aainfo[2]._frame = 23;
		kernel_reset_animation(aa[2], 23);
		kernel_synch(3, aa[2], 4, 0);
		return;
	}

	if (global[player_hyperwalked] == -1 || frame == 45) {
		aainfo[2]._frame = 44;
		kernel_reset_animation(aa[2], 44);
		kernel_synch(3, aa[2], 4, 0);
	}
}

static void room_205_anim5() {
	if (kernel_anim[aa[6]].frame == aainfo[6]._frame)
		return;
	aainfo[6]._frame = kernel_anim[aa[6]].frame;

	if (aainfo[6]._frame == 135) {
		global[g009] = 0;
		global_midi_play(9);
		kernel_timing_trigger(180, 105);
	} else if (aainfo[6]._frame == 138) {
		aainfo[6]._frame = 137;
		kernel_reset_animation(aa[6], 137);
	}
}

static void room_205_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (local->_a4) {
		case 120:
			if (global[walker_converse_state]) {
				global[walker_converse_state] = 0;
				close_interface(CANDLE_FLY);
				player.commands_allowed = true;
			}
			break;

		case 121:
			player.commands_allowed = true;
			if (player.x == 208 && player.y == 102) {
				kernel_timing_trigger(imath_random(5, 120), 28);
				local->_a4 = 122;
			} else {
				local->_a4 = 0;
			}
			break;

		default:
			break;
		}
		break;

	case 24:
		if (local->_8c == 3) {
			aa[2] = kernel_run_animation(kernel_name('E', 1), 102);
			aainfo[2]._active = -1;
			local->_96 = 2;
			kernel_reset_animation(local->_9a, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, local->_9a);
			global[g133] = 1;
		}
		break;

	case 25:
		if (local->_8e == 2) {
			aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[3]._active = -1;
			local->_94 = 3;
			kernel_reset_animation(local->_9c, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, local->_9c);
			global[g143] = 1;
		}
		break;

	case 26:
		kernel_reset_animation(local->_9a, 0);
		kernel_reset_animation(local->_9c, 0);
		global[g133] = 1;
		global[g143] = 1;
		break;

	case 28:
		if (local->_a4 == 122) {
			if (player.x == 208 && player.y == 102) {
				digi_play_build(221, '_', 1, 1);
				local->_a4 = 121;
			} else {
				local->_a4 = 0;
			}
		}
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;

		if (flags[10] == 1) {
			kernel_synch(2, 0, 4, 0);
			scratch._a4 = 1;
			global[g154] = 2;
			aainfo[1]._active = -1;
			global[g141] = -1;
			global[g017] = 0;
			player.walker_visible = false;
			global[g143] = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(3, scratch._9c, 4, 0);
			kernel_synch(3, scratch._9a, 4, 0);
			global[g131] = -1;
			global[g133] = 1;
			global[g156] = 0;
			scratch._a2 = kernel_run_animation_talk('e', 9, 0);
			extra_change_animation(scratch._a2, 120, 138, 74, 4);
			scratch._9e = kernel_run_animation_talk('b', 2, 0);
			extra_change_animation(scratch._9e, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_ANIM, scratch._9a);
			kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
			digi_play_build(205, 'b', 1, 1);
		} else {
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_PLAYER, 0);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_PLAYER, 0);
			player.walker_visible = true;
			global[g133] = 0;
			global[g143] = 0;
			kernel_synch(2, 0, 4, 0);
			player.commands_allowed = true;
		}
		break;

	case 101:
		kernel_abort_animation(aa[local->_94]);
		aainfo[local->_94]._active = false;
		kernel_reset_animation(local->_9c, 1);
		kernel_synch(KERNEL_ANIM, local->_9c, KERNEL_PLAYER, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		break;

	case 102:
		kernel_abort_animation(aa[local->_96]);
		aainfo[local->_96]._active = false;
		kernel_reset_animation(local->_9a, 1);
		kernel_synch(KERNEL_ANIM, local->_9a, KERNEL_PLAYER, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 104:
		aainfo[6]._active = 0;
		kernel_abort_animation(aa[6]);
		new_room = 101;
		break;

	case 105:
		aainfo[6]._frame = 139;
		kernel_reset_animation(aa[6], 139);
		break;

	default:
		break;
	}

	global_anim1(9, local->_9a, global[g131], &global[g132]);
	global_anim2(9, local->_9c, global[g141], &global[g142]);

	if (aainfo[0]._active)
		room_205_anim1();
	if (aainfo[1]._active)
		room_205_anim2();
	if (aainfo[3]._active)
		room_205_anim3();
	if (aainfo[2]._active)
		room_205_anim4();
	if (aainfo[6]._active)
		room_205_anim5();
}

static void room_205_pre_parser() {
	if (player_parse(words_walk_to, words_room_204, 0))
		player.walk_off_edge_to_room = 204;

	if (player_parse(words_walk_to, words_room_301, 0))
		player.walk_off_edge_to_room = 301;
}

static void room_205_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		scratch._a4 = 120;
		player.command_ready = false;
		return;
	}

	if (player_parse(words_click_on, words_hole, 0)) {
		player.commands_allowed = false;
		scratch._a4 = 121;
		digi_play_build(221, '_', 1, 1);
		player.command_ready = false;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = false;
		return;
	}

	if (player_parse(words_look_at, words_map, 0)) {
		player.commands_allowed = false;
		global[g145] = -1;
		scratch._8e = 2;
		player.command_ready = false;
		return;
	}

	if (player_parse(words_pick_up, words_flowers, 0) || player_parse(words_click_on, words_moss, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 3;
		player.command_ready = false;
		return;
	}
}

void room_205_synchronize(Common::Serializer &s) {
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
	s.syncAsSint16LE(scratch._a0);
	s.syncAsSint16LE(scratch._a2);
	s.syncAsSint16LE(scratch._a4);
}

void room_205_preload() {
	room_init_code_pointer = room_205_init;
	room_pre_parser_code_pointer = room_205_pre_parser;
	room_parser_code_pointer = room_205_parser;
	room_daemon_code_pointer = room_205_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
