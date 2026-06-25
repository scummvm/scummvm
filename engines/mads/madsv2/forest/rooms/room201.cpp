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
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
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
	int16 _a6;
	int16 _a8;
	int16 _aa;
	int16 _ac;
	int16 _ae;
	int16 _b0;
};

static Scratch scratch;

#define local  (&scratch)
#define ss     local->sprite
#define seq    local->sequence
#define aa     local->animation
#define aainfo scratch.animation_info


static void room_201_init1() {
	global[player_score] = -1;
	global[g009] = -1;
	global_midi_play(11);
	global[g131] = 0;
	global[g141] = 0;

	if (object_is_here(elm_leaves)) {
		ss[0] = kernel_load_series(kernel_name('p', 2), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 2);
		kernel_seq_loc(seq[0], 204, 153);
		kernel_seq_scale(seq[0], 100);
	} else {
		kernel_flip_hotspot(words_elm_leaves, false);
	}

	if (object_is_here(sticks)) {
		ss[1] = kernel_load_series(kernel_name('p', 1), 0);
		seq[1] = kernel_seq_stamp(ss[1], false, KERNEL_FIRST);
		kernel_seq_depth(seq[1], 10);
		kernel_seq_loc(seq[1], 46, 103);
		kernel_seq_scale(seq[1], 69);
	} else {
		kernel_flip_hotspot(words_sticks, false);
	}

	scratch._9c = kernel_run_animation_disp('r', 1, 0);
	kernel_position_anim(scratch._9c, 208, 116, 77, 7);
	scratch._9a = kernel_run_animation_disp('e', 1, 0);
	kernel_position_anim(scratch._9a, 186, 115, 76, 7);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 155;
		player.y = 115;
		player.facing = 3;
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
	case 106:
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 65;
		return;
	case 203:
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 75;
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
		player.walker_visible = true;
		return;
	default:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = true;
		player.walker_visible = true;
		return;
	}
}

static void room_201_init() {
	scratch._a6 = 0;
	scratch._a4 = -1;

	if (previous_room != KERNEL_RESTORING_GAME && previous_room != 199) {
		player.walker_visible = false;
		player.commands_allowed = false;
	}

	scratch._ac = 0;
	scratch._b0 = 0;
	for (int i = 0; i < 10; i++) {
		aainfo[i]._active = 0;
		aainfo[i]._frame = 1;
		aainfo[i]._val3 = 0;
		aainfo[i]._val4 = 0;
	}

	if (previous_room != 199 && flags[7] != 3)
		flags[7]++;

	if (flags[7] == -3)
		flags[7] = 1;
	room_201_init1();
}

static void room_201_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] != -1)
		return;

	aainfo[0]._frame = scratch._92 - 1;
	kernel_reset_animation(aa[0], scratch._92 - 1);
}

static void room_201_anim2() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (scratch._a4 != 0)
		kernel_random_frame(scratch._a2, &global[g153], global[g156]);

	if (scratch._a4 == 0)
		return;

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	switch (scratch._a4) {
	case 1:
		global[g156] = 0;
		kernel_timing_trigger(10, 28);
		scratch._a4 = 2;
		return;
	case 2:
		kernel_abort_animation(scratch._a2);
		scratch._a4 = -1;
		player.commands_allowed = true;
		player.walker_visible = true;
		aainfo[1]._active = 0;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		global[g143] = 0;
		global[g017] = -1;
		return;
	default:
		return;
	}
}

static void room_201_anim3() {
	if (kernel_anim[aa[3]].frame != aainfo[3]._frame)
		aainfo[3]._frame = kernel_anim[aa[3]].frame;

	if (global[player_hyperwalked] == -1 && aainfo[3]._frame <= 23) {
		aainfo[3]._frame = 23;
		kernel_reset_animation(aa[3], 23);
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
		return;
	}

	if (global[player_hyperwalked] == -1 || aainfo[3]._frame == 45) {
		aainfo[3]._frame = 44;
		kernel_reset_animation(aa[3], 44);
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
	}
}

static void room_201_anim4() {
	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		aainfo[4]._frame = kernel_anim[aa[4]].frame;
		int16 frame = aainfo[4]._frame;

		if (scratch._a8 == 1) {
			if (frame == 10) {
				if (aainfo[4]._val3 == 10) {
					aainfo[4]._frame = 9;
					kernel_reset_animation(aa[4], 9);
				}
			} else if (frame <= 10) {
				if (frame == 4) {
					aainfo[4]._val3 = 9;
					digi_play_build_ii('e', 2, 1);
					scratch._a4 = 28;
				} else if (frame == 8) {
					if (aainfo[4]._val3 == 9) {
						aainfo[4]._frame = 4;
						kernel_reset_animation(aa[4], 4);
					}
				}
			}
		} else if (scratch._a8 == 2) {
			if (frame == 13) {
				if (aainfo[4]._val3 == 10) {
					aainfo[4]._frame = 11;
					kernel_reset_animation(aa[4], 11);
				}
			} else if (frame <= 13) {
				if (frame == 5) {
					aainfo[4]._val3 = 9;
					digi_play_build_ii('e', 4, 1);
					scratch._a4 = 450;
				} else if (frame == 10) {
					if (aainfo[4]._val3 == 9) {
						aainfo[4]._frame = 5;
						kernel_reset_animation(aa[4], 5);
					}
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	switch (scratch._a4) {
	case 28:
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 10;
		scratch._a4 = 29;
		return;
	case 29:
		aainfo[4]._val3 = 9;
		aainfo[4]._frame = 4;
		kernel_reset_animation(aa[4], 4);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 30;
		return;
	case 30:
		aainfo[4]._val3 = 11;
		aainfo[4]._frame = 9;
		kernel_reset_animation(aa[4], 9);
		scratch._a4 = -1;
		return;
	case 450:
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 10;
		scratch._a4 = 451;
		return;
	case 451:
		aainfo[4]._val3 = 9;
		aainfo[4]._frame = 5;
		kernel_reset_animation(aa[4], 5);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 452;
		return;
	case 452:
		aainfo[4]._val3 = 11;
		aainfo[4]._frame = 10;
		kernel_reset_animation(aa[4], 10);
		scratch._a4 = -1;
		return;
	default:
		return;
	}
}

static void room_201_anim5() {
	if (kernel_anim[aa[3]].frame == scratch._aa)
		return;

	int16 var_2 = -1;
	scratch._aa = kernel_anim[aa[3]].frame;

	if (scratch._aa == 23)
		digi_play_build(101, '_', 1, 2);

	if (var_2 >= 0) {
		kernel_reset_animation(aa[3], var_2);
		scratch._aa = var_2;
	}
}

static void room_201_anim6() {
	if (kernel_anim[aa[3]].frame == scratch._ae)
		return;

	int16 var_2 = -1;
	scratch._ae = kernel_anim[aa[3]].frame;

	if (scratch._ae == 40)
		digi_play_build(101, '_', 1, 2);

	if (var_2 >= 0) {
		kernel_reset_animation(aa[3], var_2);
		scratch._ae = var_2;
	}
}

static void room_201_daemon() {
	int16 trigger = kernel.trigger;

	if (trigger == 106) {
		kernel_abort_animation(aa[3]);
		kernel_reset_animation(scratch._9c, 1);
		scratch._b0 = 0;
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		kernel_abort_animation(scratch._a2);
		scratch._a6 = 0;
		scratch._a4 = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		kernel_flip_hotspot(words_sticks, false);
		inter_move_object(sticks, PLAYER);
		player.commands_allowed = true;
	} else if (trigger == 26) {
		if (scratch._a8 == 1) {
			kernel_reset_animation(scratch._9a, 0);
			global[g133] = 1;
			scratch._a2 = kernel_run_animation_talk('e', 2, 0);
			kernel_position_anim(scratch._a2, 186, 115, 76, 7);
			global[g156] = 2;
			kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_ANIM, scratch._9a);
			digi_play_build(201, 'E', 2, 1);
			scratch._a4 = 1000;
			scratch._a6 = -1;
		} else if (scratch._a8 == 2) {
			kernel_reset_animation(scratch._9a, 0);
			global[g133] = 1;
			scratch._a2 = kernel_run_animation_talk('e', 1, 0);
			kernel_position_anim(scratch._a2, 186, 115, 76, 7);
			global[g156] = 2;
			kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_ANIM, scratch._9a);
			digi_play_build(201, 'E', 3, 1);
			scratch._a4 = 2000;
			scratch._a6 = -1;
		}
	} else if (trigger > 26) {
		if (trigger == 100) {
			kernel_abort_animation(aa[0]);
			aainfo[0]._active = 0;
			if (flags[7] == 1) {
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				global[g141] = -1;
				global[g017] = 0;
				global[g154] = 0;
				global[g143] = 0;
				kernel_reset_animation(scratch._9c, 1);
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
				kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
				global[g156] = 2;
				global[g131] = -1;
				player.walker_visible = true;
				aainfo[1]._active = -1;
				global[g133] = 1;
				scratch._a2 = kernel_run_animation_talk('e', 1, 0);
				kernel_position_anim(scratch._a2, 186, 115, 76, 7);
				kernel_synch(KERNEL_ANIM, scratch._a2, KERNEL_ANIM, scratch._9a);
				kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_PLAYER, 0);
				digi_play_build(201, 'e', 1, 1);
				scratch._a4 = 1;
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
			}
		} else if (trigger == 101) {
			kernel_abort_animation(aa[scratch._94]);
			aainfo[scratch._94]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			player.commands_allowed = true;
		} else if (trigger == 102) {
			kernel_abort_animation(aa[scratch._96]);
			aainfo[scratch._96]._active = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			player.commands_allowed = true;
		} else if (trigger == 105) {
			kernel_abort_animation(aa[3]);
			kernel_reset_animation(scratch._9c, 1);
			scratch._ac = 0;
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_abort_animation(scratch._a2);
			scratch._a6 = 0;
			scratch._a4 = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			kernel_flip_hotspot(words_elm_leaves, false);
			inter_move_object(elm_leaves, PLAYER);
			player.commands_allowed = true;
		}
	} else if (trigger == 7) {
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = true;
		} else {
			switch (scratch._a4) {
			case 40:
				kernel_abort_animation(scratch._a0);
				scratch._a4 = -1;
				scratch._a6 = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				player.walker_visible = true;
				global[g150] = -1;
				scratch._a8 = 1;
				break;
			case 41:
				kernel_abort_animation(scratch._a0);
				scratch._a4 = -1;
				scratch._a6 = 0;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				player.walker_visible = true;
				global[g150] = -1;
				scratch._a8 = 2;
				break;
			case 1000:
				global[g156] = 0;
				kernel_seq_delete(seq[0]);
				kernel_reset_animation(scratch._9c, 0);
				global[g143] = 1;
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
				aa[3] = kernel_run_animation(kernel_name('t', 1), 105);
				scratch._ac = -1;
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
				break;
			case 2000:
				global[g156] = 0;
				kernel_seq_delete(seq[1]);
				kernel_reset_animation(scratch._9c, 0);
				global[g143] = 1;
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
				aa[3] = kernel_run_animation(kernel_name('t', 2), 106);
				scratch._b0 = -1;
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
				break;
			default:
				break;
			}
		}
	} else if (trigger == 24) {
		if (scratch._8c == 5) {
			if (scratch._a8 == 1) {
				aa[4] = kernel_run_animation(kernel_name('F', 1), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			} else if (scratch._a8 == 2) {
				aa[4] = kernel_run_animation(kernel_name('F', 2), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			}
		}
	} else if (trigger == 25) {
		if (scratch._8e == 2) {
			aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[3]._active = -1;
			scratch._94 = 3;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
		}
	}

	if (scratch._a6 != 0) {
		if (scratch._a4 == 40 || scratch._a4 == 41)
			kernel_random_frame(scratch._a0, &global[g151], global[g154]);
		else if (scratch._a4 == 1 || scratch._a4 == 2 || scratch._a4 == 1000 || scratch._a4 == 2000)
			kernel_random_frame(scratch._a2, &global[g153], global[g156]);
	}

	global_anim1(1, scratch._9a, global[g131], &global[g132]);
	global_anim2(1, scratch._9c, global[g141], &global[g142]);

	if (aainfo[0]._active != 0) room_201_anim1();
	if (aainfo[3]._active != 0) room_201_anim3();
	if (aainfo[4]._active != 0) room_201_anim4();
	if (aainfo[1]._active != 0) room_201_anim2();
	if (scratch._ac != 0) room_201_anim5();
	if (scratch._b0 != 0) room_201_anim6();
}

static void room_201_pre_parser() {
	if (player_parse(words_walk_to, words_room_106, 0))
		player.walk_off_edge_to_room = 106;
	if (player_parse(words_walk_to, words_room_203, 0))
		player.walk_off_edge_to_room = 203;
}

static void room_201_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (player_parse(words_elm_leaves, 0)) {
		global[g154] = 2;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._a0 = kernel_run_animation_talk(98, 3, 0);
		kernel_position_anim(scratch._a0, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 3, 1);
		scratch._a4 = 40;
		scratch._a6 = -1;
		goto handled;
	}

	if (player_parse(words_sticks, 0)) {
		global[g154] = 2;
		player.commands_allowed = false;
		player.walker_visible = false;
		scratch._a0 = kernel_run_animation_talk(98, 4, 0);
		kernel_position_anim(scratch._a0, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_PLAYER, 0);
		digi_play_build_ii('b', 3, 1);
		scratch._a4 = 41;
		scratch._a6 = -1;
		goto handled;
	}

	if (player_parse(words_look_at, words_chicory, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a8 = 1;
		goto handled;
	}

	if (player_parse(words_look_at, words_foxglove, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a8 = 2;
		player.command_ready = false;
		return;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(words_look_at, words_map, 0)) {
		player.commands_allowed = false;
		global[g145] = -1;
		scratch._8e = 2;
		goto handled;
	}

	if (player_parse(words_flowers, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 3;
		goto handled;
	}

	if (player_parse(words_click_on, words_moss, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 3;
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_201_synchronize(Common::Serializer &s) {
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
	s.syncAsSint16LE(scratch._ac);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._b0);
}

void room_201_preload() {
	room_init_code_pointer       = room_201_init;
	room_pre_parser_code_pointer = room_201_pre_parser;
	room_parser_code_pointer     = room_201_parser;
	room_daemon_code_pointer     = room_201_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
