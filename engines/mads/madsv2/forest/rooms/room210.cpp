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
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/speech.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];              // 0x00–0x13
	int16 sequence[10];            // 0x14–0x27
	int16 animation[10];           // 0x28–0x3B
	AnimationInfo animation_info[10]; // 0x3C–0x8B
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
	int16 _a6;
	int16 _a8;                     // 0xA8
	int16 _aa;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_210_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._a4 = kernel_run_animation_disp('r', 9, 0);
	kernel_position_anim(scratch._a4, 147, 155, 80, 1);

	scratch._a2 = kernel_run_animation_disp('e', 7, 0);
	kernel_position_anim(scratch._a2, 95, 155, 80, 1);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 120;
		player.y = 144;
		player.facing = 2;
	}

	if (previous_room != 199) {
		player.walker_visible = false;
		player.commands_allowed = false;
	}

	if (previous_room == 321)
		previous_room = 303;
	if (previous_room == 322)
		previous_room = 307;

	if (previous_room == global[tunnel_1_room]) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		aainfo[0]._frame = 0;
		scratch._92 = 79;
		return;
	}

	if (previous_room == global[tunnel_2_room]) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		aainfo[0]._frame = 0;
		scratch._92 = 72;
		return;
	}

	if (previous_room == global[tunnel_3_room]) {
		aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
		aainfo[0]._active = -1;
		aainfo[0]._frame = 0;
		scratch._92 = 66;
		return;
	}

	if (previous_room == global[tunnel_4_room]) {
		aa[0] = kernel_run_animation(kernel_name('y', 4), 100);
		aainfo[0]._active = -1;
		aainfo[0]._frame = 0;
		scratch._92 = 77;
		return;
	}

	if (previous_room == global[tunnel_5_room]) {
		aa[0] = kernel_run_animation(kernel_name('y', 5), 100);
		scratch._92 = 88;
		aainfo[0]._active = -1;
		aainfo[0]._frame = 0;
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._a2, 2);
	kernel_reset_animation(scratch._a4, 2);
	global[g133] = 0;
	global[g143] = 0;

	if (previous_room == 199)
		restore_player();

	player.commands_allowed = true;
	player.walker_visible = true;
}

static void room_210_init() {
	midi_stop();
	scratch._a8 = -1;
	global[player_score] = 0;
	// TODO: identify word_7130E — used as an index to select which 16-bit half of
	// global.walker_timing to zero: global[walker_timing + word_7130E] = 0
	global[walker_timing] = 0;

	for (int i = 0; i < 10; i++) {
		aainfo[i]._active = 0;
		aainfo[i]._frame = 0;
		aainfo[i]._val3 = 0;
		aainfo[i]._val4 = 0;
	}

	aa[2] = kernel_run_animation(kernel_name('t', 1), 102);
	aa[3] = kernel_run_animation(kernel_name('t', 2), 102);
	aa[4] = kernel_run_animation(kernel_name('t', 3), 102);
	aa[5] = kernel_run_animation(kernel_name('t', 4), 102);
	aa[6] = kernel_run_animation(kernel_name('t', 5), 102);

	kernel_timing_trigger(imath_random(10, 300), 103);
	kernel_timing_trigger(imath_random(10, 300), 104);
	kernel_timing_trigger(imath_random(10, 300), 105);
	kernel_timing_trigger(imath_random(10, 300), 106);
	kernel_timing_trigger(imath_random(10, 300), 107);
	kernel_timing_trigger(1, 109);

	if (previous_room != 199 && flags[11] != 3)
		flags[11]++;

	room_210_init1();
}

static void room_210_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_210_anim2() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame)
		aainfo[1]._frame = kernel_anim[aa[1]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[1]._frame = scratch._94 - 1;
		kernel_reset_animation(aa[1], aainfo[1]._frame);
	}
}

static void room_210_anim3() {
	if (kernel_anim[aa[7]].frame != aainfo[7]._frame) {
		aainfo[7]._frame = kernel_anim[aa[7]].frame;

		int16 f = aainfo[7]._frame;
		if (f == 1) {
			kernel_timing_trigger(10, 114);
		} else if (f == 6) {
			aainfo[7]._frame = 2;
			kernel_reset_animation(aa[7], 2);
		}
	}
}

static void room_210_anim4() {
	int16 result = -1;

	if (kernel_anim[aa[8]].frame != aainfo[8]._frame) {
		aainfo[8]._frame = kernel_anim[aa[8]].frame;
		int16 f = aainfo[8]._frame;

		if (f == 11) {
			result = 9;
		} else if (f < 11) {
			if (f == 1) {
				digi_play_build(210, 'R', 1, 1);
				scratch._a6 = 2;
			} else if (f == 3) {
				result = 1;
			} else if (f == 9) {
				digi_play_build(210, 'E', 1, 1);
				scratch._a6 = 3;
			}
		}

		if (result >= 0) {
			aainfo[8]._frame = result;
			kernel_reset_animation(aa[8], result);
		}
	}
}

static void room_210_anim5() {
	if (kernel_anim[aa[2]].frame != aainfo[2]._frame) {
		aainfo[2]._frame = kernel_anim[aa[2]].frame;
		int16 f = aainfo[2]._frame;
		if (f == 1) {
			aainfo[2]._frame = 0;
			kernel_reset_animation(aa[2], 0);
		} else if (f == 52 || f == 78) {
			aainfo[2]._frame = 0;
			kernel_reset_animation(aa[2], 0);
			kernel_timing_trigger(imath_random(100, 300), 103);
		}
	}

	if (kernel_anim[aa[3]].frame != aainfo[3]._frame) {
		aainfo[3]._frame = kernel_anim[aa[3]].frame;
		int16 f = aainfo[3]._frame;
		if (f == 1) {
			aainfo[3]._frame = 0;
			kernel_reset_animation(aa[3], 0);
		} else if (f == 19 || f == 63) {
			aainfo[3]._frame = 0;
			kernel_reset_animation(aa[3], 0);
			kernel_timing_trigger(imath_random(100, 300), 104);
		}
	}

	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		aainfo[4]._frame = kernel_anim[aa[4]].frame;
		int16 f = aainfo[4]._frame;
		if (f == 1) {
			aainfo[4]._frame = 0;
			kernel_reset_animation(aa[4], 0);
		} else if (f == 20 || f == 82) {
			aainfo[4]._frame = 0;
			kernel_reset_animation(aa[4], 0);
			kernel_timing_trigger(imath_random(100, 300), 105);
		}
	}

	if (kernel_anim[aa[5]].frame != aainfo[5]._frame) {
		aainfo[5]._frame = kernel_anim[aa[5]].frame;
		int16 f = aainfo[5]._frame;
		if (f == 1) {
			aainfo[5]._frame = 0;
			kernel_reset_animation(aa[5], 0);
		} else if (f == 34 || f == 57) {
			aainfo[5]._frame = 0;
			kernel_reset_animation(aa[5], 0);
			kernel_timing_trigger(imath_random(100, 300), 106);
		}
	}

	if (kernel_anim[aa[6]].frame == aainfo[6]._frame)
		return;
	aainfo[6]._frame = kernel_anim[aa[6]].frame;
	int16 f = aainfo[6]._frame;
	if (f == 1) {
		aainfo[6]._frame = 0;
		kernel_reset_animation(aa[6], 0);
	} else if (f == 27 || f == 53) {
		aainfo[6]._frame = 0;
		kernel_reset_animation(aa[6], 0);
		kernel_timing_trigger(imath_random(100, 300), 107);
	}
}

static void room_210_change_room(int roomNum) {
	if (roomNum == 106) {
		if (global[tunnel_exit_1_opened] != 0) { new_room = 106; return; }
	} else if (roomNum == 203) {
		if (global[tunnel_exit_2_opened] != 0) { new_room = 203; return; }
	} else if (roomNum == 302) {
		if (global[tunnel_exit_3_opened] != 0) { new_room = 302; return; }
	} else if (roomNum == 305) {
		if (global[tunnel_exit_4_opened] != 0) { new_room = 305; return; }
	} else if (roomNum == 307) {
		if (global[tunnel_exit_5_opened] != 0) {
			if (global[g064] != 0)
				new_room = 322;
			else
				new_room = 307;
			return;
		}
	} else {
		return;
	}
	kernel_timing_trigger(imath_random(40, 80), 108);
}

static void room_210_anim7() {
	switch (scratch._aa) {
	case 1:
		aa[0] = kernel_run_animation(kernel_name('a', 1), 110);
		aainfo[0]._active = -1;
		scratch._92 = 65;
		break;
	case 2:
		aa[0] = kernel_run_animation(kernel_name('a', 2), 110);
		aainfo[0]._active = -1;
		scratch._92 = 60;
		break;
	case 3:
		aa[0] = kernel_run_animation(kernel_name('a', 3), 110);
		aainfo[0]._active = -1;
		scratch._92 = 64;
		break;
	case 4:
		aa[0] = kernel_run_animation(kernel_name('a', 4), 110);
		aainfo[0]._active = -1;
		scratch._92 = 70;
		break;
	case 5:
		aa[0] = kernel_run_animation(kernel_name('a', 5), 110);
		aainfo[0]._active = -1;
		scratch._92 = 89;
		break;
	default:
		break;
	}
	kernel_timing_trigger(1, 109);
}

static void room_210_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (scratch._a6) {
		case 1:
			aainfo[7]._frame = 6;
			kernel_reset_animation(aa[7], 6);
			break;
		case 2:
			aainfo[8]._frame = 4;
			kernel_reset_animation(aa[8], 4);
			break;
		case 3:
			aainfo[8]._frame = 12;
			kernel_reset_animation(aa[8], 12);
			if (config_file.forest1 != 0) kernel_timing_trigger(1, 109);
			break;
		case 4:
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
			if (config_file.forest1 != 0) kernel_timing_trigger(1, 109);
			break;
		}
		scratch._a6 = 0;
		break;

	case 26:
		kernel_abort_animation(scratch._a2);
		kernel_abort_animation(scratch._a4);
		scratch._a2 = -1;

		switch (scratch._90) {
		case 1:
			aa[1] = kernel_run_animation(kernel_name('z', 1), 101);
			aainfo[1]._active = -1;
			scratch._94 = 36;
			break;
		case 2:
			aa[1] = kernel_run_animation(kernel_name('z', 2), 101);
			aainfo[1]._active = -1;
			scratch._94 = 35;
			break;
		case 3:
			aa[1] = kernel_run_animation(kernel_name('z', 3), 101);
			aainfo[1]._active = -1;
			scratch._94 = 59;
			break;
		case 4:
			aa[1] = kernel_run_animation(kernel_name('z', 4), 101);
			aainfo[1]._active = -1;
			scratch._94 = 71;
			break;
		case 5:
			aa[1] = kernel_run_animation(kernel_name('z', 5), 101);
			aainfo[1]._active = -1;
			scratch._94 = 84;
			break;
		}
		aainfo[1]._active = -1;
		aainfo[1]._frame = 2;
		kernel_reset_animation(aa[1], 2);
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		kernel_timing_trigger(1, 109);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		if (flags[11] == 1) {
			aa[8] = kernel_run_animation(kernel_name('x', 1), 112);
			kernel_synch(KERNEL_ANIM, aa[8], KERNEL_NOW, 0);
			aainfo[8]._active = -1;
			scratch._a8 = 0;
			kernel_timing_trigger(1, 109);
		} else {
			global[g131] = -1; global[g141] = -1;
			kernel_reset_animation(scratch._a2, 1);
			kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._a4, 1);
			kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_NOW, 0);
			player.walker_visible = true;
			global[g133] = 0; global[g143] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = true;
			scratch._a8 = -1;
		}
		break;

	case 101:
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		switch (scratch._90) {
		case 1:
			room_210_change_room(global[tunnel_1_room]);
			scratch._aa = 1;
			break;
		case 2:
			room_210_change_room(global[tunnel_2_room]);
			scratch._aa = 2;
			break;
		case 3:
			room_210_change_room(global[tunnel_3_room]);
			scratch._aa = 3;
			break;
		case 4:
			room_210_change_room(global[tunnel_4_room]);
			scratch._aa = 4;
			break;
		case 5:
			room_210_change_room(global[tunnel_5_room]);
			scratch._aa = 5;
			break;
		default:
			speech_ems_play("d5", kernel.trigger, false);
			break;
		}
		break;

	case 102:
		break;

	case 103:
		aainfo[2]._frame = imath_random(0, 1) ? 60 : 2;
		kernel_reset_animation(aa[2], aainfo[2]._frame);
		break;

	case 104:
		aainfo[3]._frame = imath_random(0, 1) ? 40 : 2;
		kernel_reset_animation(aa[3], aainfo[3]._frame);
		break;

	case 105:
		aainfo[4]._frame = imath_random(0, 1) ? 30 : 2;
		kernel_reset_animation(aa[4], aainfo[4]._frame);
		break;

	case 106:
		aainfo[5]._frame = imath_random(0, 1) ? 40 : 2;
		kernel_reset_animation(aa[5], aainfo[5]._frame);
		break;

	case 107:
		aainfo[6]._frame = imath_random(0, 1) ? 30 : 2;
		kernel_reset_animation(aa[6], aainfo[6]._frame);
		break;

	case 108:
		room_210_anim7();
		break;

	case 109:
		digi_initial_volume(60);
		digi_play_build(210, '_', 1, 3);
		digi_val2 = 100;
		break;

	case 110:
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		aa[7] = kernel_run_animation(kernel_name('a', 0), 111);
		kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);
		aainfo[7]._active = -1;
		scratch._a8 = 0;
		kernel_timing_trigger(1, 109);
		break;

	case 111:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._a2, 1);
		kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_NOW, 0);
		kernel_reset_animation(scratch._a4, 1);
		kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g133] = 0; global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		scratch._a8 = -1;
		kernel_timing_trigger(1, 109);
		break;

	case 112:
		kernel_abort_animation(aa[8]);
		aainfo[8]._active = 0;
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._a2, 1);
		kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_NOW, 0);
		kernel_reset_animation(scratch._a4, 1);
		kernel_synch(KERNEL_ANIM, scratch._a4, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		scratch._a8 = -1;
		break;

	case 114:
		digi_play_build(210, 'B', 1, 1);
		scratch._a6 = 1;
		break;

	default:
		if (kernel.trigger >= 100)
			speech_ems_play("d4", kernel.trigger, false);
		break;
	}

	if (scratch._a2 >= 0) {
		global_anim1(7, scratch._a2, global[g131], &global[g132]);
		global_anim2(9, scratch._a4, global[g141], &global[g142]);
	}
	if (aainfo[0]._active != 0) room_210_anim1();
	if (aainfo[1]._active != 0) room_210_anim2();
	if (aainfo[7]._active != 0) room_210_anim3();
	if (aainfo[8]._active != 0) room_210_anim4();
	room_210_anim5();
}

static void room_210_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		scratch._a6 = 4;
		player.command_ready = false;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = false;
		return;
	}

	if (player_parse(words_click_on, words_door_1, 0)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		global[g150] = -1;
		scratch._90 = 1;
		player.command_ready = false;
		return;
	}

	if (player_parse(words_click_on, words_door_2, 0)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		global[g150] = -1;
		scratch._90 = 2;
		player.command_ready = false;
		return;
	}

	if (player_parse(words_click_on, words_door_3, 0)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		global[g150] = -1;
		scratch._90 = 3;
		player.command_ready = false;
		return;
	}

	if (player_parse(words_click_on, words_door_4, 0)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		global[g150] = -1;
		scratch._90 = 4;
		player.command_ready = false;
		return;
	}

	if (player_parse(words_click_on, words_door_5, 0)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		global[g150] = -1;
		scratch._90 = 5;
		player.command_ready = false;
		return;
	}
}

void room_210_synchronize(Common::Serializer &s) {
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
	s.syncAsSint16LE(scratch._a6);
	s.syncAsSint16LE(scratch._a8);
	s.syncAsSint16LE(scratch._aa);
}

void room_210_preload() {
	room_init_code_pointer = room_210_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = room_210_parser;
	room_daemon_code_pointer = room_210_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
