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

#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"

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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_405_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 7, 0);
	kernel_position_anim(scratch._9c, 308, 152, 69, 1);
	scratch._9a = kernel_run_animation_disp('e', 7, 0);
	kernel_position_anim(scratch._9a, 284, 154, 69, 1);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 246;
		player.y = 130;
		player.facing = 7;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
	} else if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
		player.commands_allowed = true;
		player.walker_visible = true;
	} else if (previous_room == 404) {
		global[g009] = -1;
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		scratch.animation_info[0]._active = -1;
		scratch._92 = 106;
	} else if (previous_room == 501) {
		global[g009] = -1;
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		scratch.animation_info[0]._active = -1;
		scratch._92 = 44;
	} else {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = true;
		player.walker_visible = true;
	}
}

static void room_405_init() {
	global[player_score] = 0;
	global[g009] = -1;

	aa[7] = kernel_run_animation(kernel_name('N', 1), 105);
	scratch.animation_info[7]._active = -1;
	kernel.trigger_setup_mode = 1;
	kernel_timing_trigger(1, 106);

	scratch._a0 = -1;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int16 count = 0; count < 10; count++) {
			scratch.animation_info[count]._active = 0;
			scratch.animation_info[count]._frame = 1;
			scratch.animation_info[count]._val3 = 0;
			scratch.animation_info[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[30] != 3) {
			flags[30]++;
		}
	}

	room_405_init1();
}

static void room_405_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur != scratch.animation_info[0]._frame)
		scratch.animation_info[0]._frame = cur;
	if (global[player_hyperwalked] != -1)
		return;
	int16 f = scratch._92 - 1;
	scratch.animation_info[0]._frame = f;
	kernel_reset_animation(aa[0], f);
}

static void room_405_anim2() {
	int16 cur = kernel_anim[aa[6]].frame;
	if (cur != scratch.animation_info[6]._frame) {
		scratch.animation_info[6]._frame = cur;
		if (cur == 5) {
			scratch.animation_info[6]._val3 = 12;
			digi_play_build(404, 'r', 1, 1);
			scratch._a2 = 5;
		} else if (cur == 10) {
			if (scratch.animation_info[6]._val3 == 12) {
				scratch.animation_info[6]._frame = 5;
				kernel_reset_animation(aa[6], 5);
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a2 == 5) {
		scratch.animation_info[6]._val3 = 13;
		scratch.animation_info[6]._frame = 10;
		kernel_reset_animation(aa[6], 10);
		scratch._a2 = -1;
	}
}

static void room_405_anim3() {
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur != scratch.animation_info[2]._frame)
		scratch.animation_info[2]._frame = cur;
}

static void room_405_anim4() {
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur != scratch.animation_info[1]._frame)
		scratch.animation_info[1]._frame = cur;
}

static void room_405_anim5() {
	int16 cur = kernel_anim[aa[5]].frame;
	if (cur != scratch.animation_info[5]._frame) {
		scratch.animation_info[5]._frame = cur;
		if (cur == 21) {
			if (scratch.animation_info[5]._val3 == 11) {
				scratch.animation_info[5]._frame = 16;
				kernel_reset_animation(aa[5], 16);
			}
		} else if (cur < 21) {
			if (cur == 6) {
				scratch.animation_info[5]._val3 = 10;
				digi_play_build(404, 'b', 4, 1);
				scratch._a2 = 6;
			} else if (cur == 11) {
				if (scratch.animation_info[5]._val3 == 10) {
					scratch.animation_info[5]._frame = 6;
					kernel_reset_animation(aa[5], 6);
				}
			} else if (cur == 16) {
				scratch.animation_info[5]._val3 = 11;
				digi_play_build(404, 'e', 5, 1);
				scratch._a2 = 16;
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a2 == 6) {
		scratch.animation_info[5]._val3 = 13;
		scratch.animation_info[5]._frame = 11;
		kernel_reset_animation(aa[5], 11);
		scratch._a2 = -1;
	} else if (scratch._a2 == 16) {
		scratch.animation_info[5]._val3 = 13;
		scratch.animation_info[5]._frame = 21;
		kernel_reset_animation(aa[5], 21);
		scratch._a2 = -1;
	}
}

static void room_405_anim6() {
	int16 cur = kernel_anim[aa[7]].frame;
	if (cur != scratch.animation_info[7]._frame) {
		scratch.animation_info[7]._frame = cur;
		if (cur == 99) {
			scratch.animation_info[7]._frame = 1;
			kernel_reset_animation(aa[7], 1);
		} else if (cur < 99) {
			int threshold = -1;
			if (cur == 10 || cur == 38 || cur == 55 || cur == 74 || cur == 83)
				threshold = 20;
			else if (cur == 19 || cur == 48 || cur == 86)
				threshold = 60;
			else if (cur == 1 || cur == 28 || cur == 31 || cur == 94)
				threshold = 80;
			if (threshold >= 0 && imath_random(0, 100) > threshold) {
				scratch.animation_info[7]._frame--;
				kernel_reset_animation(aa[7], scratch.animation_info[7]._frame);
			}
		}
	}

	if (kernel.trigger == 8 && scratch._a2 == 1)
		scratch._a2 = -1;
}

static void room_405_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state]) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
			if (config_file.forest1)
				kernel_timing_trigger(1, 106);
		}
		break;

	case 24:
		if (scratch._8c == 3) {
			aa[1] = kernel_run_animation(kernel_name('E', 1), 102);
			scratch.animation_info[1]._active = -1;
			scratch._96 = 1;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
			kernel_timing_trigger(1, 106);
		}
		break;

	case 25:
		if (scratch._8e == 2) {
			aa[2] = kernel_run_animation(kernel_name('R', 1), 101);
			scratch.animation_info[2]._active = -1;
			scratch._94 = 2;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
			kernel_timing_trigger(1, 106);
		}
		break;

	case 26:
		if (scratch._90 == 1) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			global[g133] = 1;
			global[g143] = 1;
		} else if (scratch._90 == 6) {
			aa[5] = kernel_run_animation(kernel_name('L', 1), 103);
			scratch.animation_info[5]._active = -1;
			scratch._98 = 5;
			kernel_reset_animation(scratch._9a, 0);
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[5]);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[5]);
			global[g133] = 1;
			global[g143] = 1;
			scratch._a0 = 1;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible = false;
			kernel_timing_trigger(1, 106);
		}
		break;

	case 27:
		global[walker_converse_now] = 0;
		kernel_abort_animation(scratch._9e);
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		if (config_file.forest1)
			kernel_timing_trigger(1, 106);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		scratch.animation_info[0]._active = 0;
		if (flags[30] == 1) {
			aa[6] = kernel_run_animation(kernel_name('t', 1), 104);
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_ANIM, aa[0]);
			scratch.animation_info[6]._active = -1;
			player.commands_allowed = false;
			kernel_timing_trigger(1, 106);
		} else {
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			player.walker_visible = true;
			global[g133] = 0;
			global[g143] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = true;
			if (config_file.forest1)
				kernel_timing_trigger(1, 106);
		}
		break;

	case 101:
		kernel_abort_animation(aa[scratch._94]);
		scratch.animation_info[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		if (config_file.forest1)
			kernel_timing_trigger(1, 106);
		break;

	case 102:
		kernel_abort_animation(aa[scratch._96]);
		scratch.animation_info[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		if (config_file.forest1)
			kernel_timing_trigger(1, 106);
		break;

	case 103:
		if (scratch._a0 == 1) {
			kernel_abort_animation(aa[5]);
			scratch.animation_info[5]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			player.walker_visible = true;
			global[g133] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g082] = 1;
			player.commands_allowed = true;
			if (config_file.forest1)
				kernel_timing_trigger(1, 106);
		}
		break;

	case 104:
		kernel_abort_animation(aa[6]);
		scratch.animation_info[6]._active = 0;
		global[g141] = -1;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		player.walker_visible = true;
		global[g143] = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		scratch._9e = kernel_run_animation_write(0);
		kernel_reset_animation(scratch._9e, 41);
		kernel_position_anim(scratch._9e, 284, 154, 69, 1);
		global[walker_converse_now] = 1;
		global[g007] = 7;
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_NOW, 0);
		kernel_timing_trigger(1, 106);
		break;

	case 106:
		digi_play_build(401, '_', 1, 3);
		digi_initial_volume(60);
		digi_val2 = -1;
		break;

	}

	global_anim1(7, scratch._9a, global[g131], &global[g132]);
	global_anim2(7, scratch._9c, global[g141], &global[g142]);

	if (scratch.animation_info[0]._active) room_405_anim1();
	if (scratch.animation_info[6]._active) room_405_anim2();
	if (scratch.animation_info[2]._active) room_405_anim3();
	if (scratch.animation_info[1]._active) room_405_anim4();
	if (scratch.animation_info[5]._active) room_405_anim5();

	if (global[walker_converse_now])
		global_anim3(scratch._9e, &global[g008]);

	room_405_anim6();
}

static void room_405_pre_parser() {
	if (player_parse(words_walk_to, words_room_501, 0))
		player.walk_off_edge_to_room = 501;
}

static void room_405_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		player.command_ready = false;
		return;
	}

	if (player_parse(words_walk_to, words_room_404, 0)) {
		new_room = 404;
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

	if (player_parse(words_pick_up, words_flowers, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 3;
		player.command_ready = false;
		return;
	}

	if (player_parse(words_click_on, words_dragon1, 0)) {
		player.commands_allowed = false;
		global[g150] = -1;
		scratch._90 = 6;
		player.command_ready = false;
		return;
	}
}

void room_405_synchronize(Common::Serializer &s) {
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
}

void room_405_preload() {
	room_init_code_pointer       = room_405_init;
	room_pre_parser_code_pointer = room_405_pre_parser;
	room_parser_code_pointer     = room_405_parser;
	room_daemon_code_pointer     = room_405_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
