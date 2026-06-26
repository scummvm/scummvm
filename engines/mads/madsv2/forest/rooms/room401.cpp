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
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];                  // 0x00
	int16 sequence[10];                // 0x14
	int16 animation[10];               // 0x28
	AnimationInfo animation_info[10];  // 0x3C (80 bytes, ends at 0x8B)
	int16 _8c;                         // 0x8C
	int16 _8e;                         // 0x8E
	int16 _90;                         // 0x90
	int16 _92;                         // 0x92 — hotspot ID for incoming transition
	int16 _94;                         // 0x94
	int16 _96;                         // 0x96
	int16 _98;                         // 0x98 — room state / variant index
	int16 _9a;                         // 0x9A — background anim handle (e-series)
	int16 _9c;                         // 0x9C — background anim handle (r-series)
	int16 _9e;                         // 0x9E
	int16 _a0;                         // 0xA0
	int16 _a2;                         // 0xA2
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_401_anim_state(int16 state) {
	switch (state) {
	case 15:
		aa[1] = kernel_run_animation(kernel_name('y', 3), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 19;
		kernel_timing_trigger(1, 106);
		break;
	case 16:
		aa[1] = kernel_run_animation(kernel_name('z', 2), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 21;
		kernel_timing_trigger(1, 106);
		break;
	case 17:
		aa[1] = kernel_run_animation(kernel_name('t', 2), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 18;
		kernel_timing_trigger(1, 106);
		break;
	case 18:
	case 19:
	case 25:
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		player.commands_allowed = true;
		player.walker_visible = true;
		if (config_file.forest1) {
			digi_stop(1);
			kernel_timing_trigger(1, 106);
		}
		break;
	case 21:
		new_room = 308;
		break;
	case 22:
		if (flags[26] == 1) {
			aa[1] = kernel_run_animation(kernel_name('t', 1), 103);
			scratch._98 = 17;
		} else {
			aa[1] = kernel_run_animation(kernel_name('y', 2), 103);
			scratch._98 = 15;
		}
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		kernel_timing_trigger(1, 106);
		break;
	case 23:
		aa[1] = kernel_run_animation(kernel_name('n', 2), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 24;
		kernel_timing_trigger(1, 106);
		break;
	case 24:
		aa[1] = kernel_run_animation(kernel_name('n', 3), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 25;
		kernel_timing_trigger(1, 106);
		break;
	default:
		break;
	}
}

static void room_401_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur != aainfo[0]._frame)
		aainfo[0]._frame = cur;

	if (global[player_hyperwalked] != -1)
		return;

	aainfo[0]._frame = scratch._92 - 1;
	kernel_reset_animation(aa[0], aainfo[0]._frame);
}

static void room_401_anim2() {
	int16 result = -1;

	if (global[player_hyperwalked] == -1) {
		int16 st = scratch._98;
		if (st == 15 || st == 17) {
			if (aainfo[1]._frame < 49)
				result = 49;
		} else if (st == 16) {
			if (aainfo[1]._frame < 32)
				result = 32;
		} else if (st == 21) {
			if (aainfo[1]._frame < 116)
				result = 116;
		}
		if (result >= 0) {
			aainfo[1]._frame = result;
			kernel_reset_animation(aa[1], result);
			result = -1;
		}
	}

	int16 cur = kernel_anim[aa[1]].frame;
	if (cur != aainfo[1]._frame) {
		aainfo[1]._frame = cur;

		switch (scratch._98) {
		case 15:
		case 17:
			if (aainfo[1]._frame == 42) {
				digi_play_build(307, '_', 2, 1);
				scratch._a0 = -1;
			}
			break;
		case 16:
			if (aainfo[1]._frame == 23) {
				digi_play_build(307, '_', 3, 1);
				scratch._a0 = -1;
			}
			break;
		case 18: {
			int16 frame = aainfo[1]._frame;
			if (frame == 96) {
				digi_play_build(301, '_', 1, 1);
				scratch._a0 = -1;
			} else if (frame < 96) {
				if (frame == 25) {
					result = 22;
				} else if (frame < 25) {
					if (frame == 15) {
						if (!global[g066])
							digi_play_build(401, 'r', 1, 1);
						else
							digi_play_build(401, 'r', 2, 1);
						scratch._a0 = 60;
					} else if (frame == 18) {
						result = 15;
					} else if (frame == 20) {
						result = 19;
					} else if (frame == 22) {
						digi_play_build(401, 'b', 1, 1);
						scratch._a0 = 62;
					}
				} else {
					if (frame == 27) {
						result = 26;
					} else if (frame == 29) {
						digi_play_build(401, 'r', 3, 1);
						scratch._a0 = 64;
					} else if (frame == 32) {
						result = 29;
					} else if (frame == 35) {
						if (config_file.forest1)
							kernel_timing_trigger(1, 106);
					}
				}
			}
			break;
		}
		case 23:
			if (aainfo[1]._frame == 1)
				result = 0;
			break;
		case 24:
			if (aainfo[1]._frame == 1) {
				digi_play_build(220, '_', 4, 1);
				scratch._a0 = 70;
			} else if (aainfo[1]._frame == 16) {
				result = 1;
			}
			break;
		default:
			break;
		}

		if (result >= 0) {
			aainfo[1]._frame = result;
			kernel_reset_animation(aa[1], result);
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	result = -1;
	int16 a0 = scratch._a0 - 60;
	if (a0 >= 0 && a0 <= 10) {
		switch (a0) {
		case 0:
			kernel_timing_trigger(30, 28);
			result = 19;
			scratch._a0 = 61;
			break;
		case 1:
			result = 21;
			scratch._a0 = 62;
			break;
		case 2:
			kernel_timing_trigger(30, 28);
			result = 26;
			scratch._a0 = 63;
			break;
		case 3:
			result = 28;
			scratch._a0 = 64;
			break;
		case 4:
			result = 33;
			scratch._a0 = -1;
			break;
		case 10:
			result = 16;
			scratch._a0 = -1;
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

static void room_401_anim3() {
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur != aainfo[3]._frame)
		aainfo[3]._frame = cur;
}

static void room_401_anim4() {
	int16 result = -1;

	int16 cur = kernel_anim[aa[2]].frame;
	if (cur != aainfo[2]._frame) {
		aainfo[2]._frame = cur;

		if (scratch._a2 == 1) {
			if (cur == 61) {
				if (aainfo[2]._val3 == 13)
					result = 59;
			} else if (cur < 61) {
				if (cur == 53) {
					aainfo[2]._val3 = 11;
					digi_play_build_ii('e', 8, 1);
					scratch._a0 = 53;
				} else if (cur == 58) {
					if (aainfo[2]._val3 == 11)
						result = 53;
				}
			}
		}

		if (result >= 0) {
			aainfo[2]._frame = result;
			kernel_reset_animation(aa[2], result);
			result = -1;
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	result = -1;
	int16 a0 = scratch._a0 - 53;
	if (a0 == 0) {
		kernel_timing_trigger(30, 28);
		aainfo[2]._val3 = 13;
		scratch._a0 = 54;
	} else if (a0 == 1) {
		aainfo[2]._val3 = 11;
		result = 53;
		digi_play_build_ii('e', 10, 1);
		scratch._a0 = 55;
	} else if (a0 == 2) {
		aainfo[2]._val3 = 14;
		result = 59;
		scratch._a0 = -1;
	}

	if (result >= 0) {
		aainfo[2]._frame = result;
		kernel_reset_animation(aa[2], result);
	}
}

static void room_401_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 3, 0);
	kernel_position_anim(scratch._9c, 228, 132, 82, 4);
	scratch._9a = kernel_run_animation_disp('e', 2, 0);
	kernel_position_anim(scratch._9a, 252, 115, 76, 7);

	if (previous_room != KERNEL_RESTORING_GAME && previous_room != 199) {
		player_demand_facing(7);
		player_demand_location(300, 138);
	}

	if (previous_room == 199) {
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
	}

	if (previous_room == 308) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 103);
		aainfo[0]._active = -1;
		scratch._92 = 49;
		scratch._98 = 22;
		return;
	}

	if (previous_room == 402) {
		aa[0] = kernel_run_animation(kernel_name('y', 4), 100);
		aainfo[0]._active = -1;
		scratch._92 = 100;
		return;
	}

	if (previous_room == 404) {
		aa[0] = kernel_run_animation(kernel_name('y', 5), 100);
		aainfo[0]._active = -1;
		scratch._92 = 85;
		return;
	}

	if (previous_room == 420) {
		aa[1] = kernel_run_animation(kernel_name('n', 1), 103);
		aainfo[1]._active = -1;
		aainfo[1]._frame = 0;
		scratch._98 = 23;
		kernel_timing_trigger(120, 105);
		return;
	}

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = true;
	player.walker_visible = true;
}

static void room_401_init() {
	scratch._a2 = -1;
	midi_stop();

	if (!flags[31] || !global[g066])
		kernel_flip_hotspot(words_room_404, false);

	kernel_timing_trigger(1, 106);
	global[player_score] = 0;
	scratch._98 = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = false;
		}

		for (int16 count = 0; count < 10; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame = -1;
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[26] != 3)
			flags[26]++;
	}

	room_401_init1();
}

static void room_401_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (scratch._a0 != 668)
			break;
		global[walker_converse_state] = 0;
		close_interface(CANDLE_FLY);
		player.commands_allowed = true;
		if (config_file.forest1)
			kernel_timing_trigger(1, 106);
		break;

	case 24:
		if (scratch._8c != 20)
			break;
		if (scratch._a2 != 1)
			break;
		aa[2] = kernel_run_animation(kernel_name('F', 1), 102);
		aainfo[2]._active = -1;
		scratch._96 = 2;
		kernel_reset_animation(scratch._9a, 0);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[2]);
		global[g133] = 1;
		kernel_timing_trigger(1, 106);
		break;

	case 25:
		if (scratch._8e != 2)
			break;
		aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
		aainfo[3]._active = -1;
		scratch._94 = 3;
		kernel_reset_animation(scratch._9c, 0);
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9c);
		global[g143] = 1;
		kernel_timing_trigger(1, 106);
		break;

	case 26:
		kernel_reset_animation(scratch._9a, 0);
		kernel_reset_animation(scratch._9c, 0);
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[1]);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[1]);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
		global[g133] = 1;
		global[g143] = 1;
		if (scratch._90 == 16) {
			aa[1] = kernel_run_animation(kernel_name('z', 1), 103);
			aainfo[1]._active = -1;
			aainfo[1]._frame = 0;
			scratch._98 = 16;
			kernel_timing_trigger(1, 106);
		}
		break;

	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
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
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(aa[scratch._94]);
		aainfo[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		kernel_timing_trigger(1, 106);
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[scratch._96]);
		aainfo[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		kernel_timing_trigger(1, 106);
		break;

	case 103:
		if (scratch._98 == 22) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[0]);
			aainfo[0]._active = 0;
		} else {
			dont_frag_the_palette();
			kernel_abort_animation(aa[1]);
			aainfo[1]._active = 0;
		}
		room_401_anim_state(scratch._98);
		break;

	case 105:
		aainfo[1]._frame = 1;
		kernel_reset_animation(aa[1], 1);
		break;

	case 106:
		digi_initial_volume(40);
		digi_play_build(401, '_', 1, 3);
		digi_val2 = -1;
		break;

	default:
		break;
	}

	global_anim1(2, scratch._9a, global[g131], &global[g132]);
	global_anim2(3, scratch._9c, global[g141], &global[g142]);

	if (aainfo[0]._active)
		room_401_anim1();
	if (aainfo[1]._active)
		room_401_anim2();
	if (aainfo[3]._active)
		room_401_anim3();
	if (aainfo[2]._active)
		room_401_anim4();
}

static void room_401_pre_parser() {
	if (player_parse(words_walk_to, words_room_402, 0))
		player.walk_off_edge_to_room = 402;

	if (player_parse(words_walk_to, words_room_404, 0))
		player.walk_off_edge_to_room = 404;
}

static void room_401_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		scratch._a0 = 668;
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_walk_to, words_room_308, 0)) {
		global[g150] = -1;
		scratch._90 = 16;
		player.commands_allowed = false;
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (player_parse(words_look_at, words_thistle, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 20;
		scratch._a2 = 1;
		player.command_ready = 0;
	}
}

void room_401_synchronize(Common::Serializer &s) {
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

void room_401_preload() {
	room_init_code_pointer = room_401_init;
	room_pre_parser_code_pointer = room_401_pre_parser;
	room_parser_code_pointer = room_401_parser;
	room_daemon_code_pointer = room_401_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
