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
#include "mads/madsv2/core/midi.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/rooms/room304.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[10];
	AnimationInfo animation_info[10];
	int16 _8c, _8e, _90, _92, _94, _96, _98, _9a, _9c, _9e, _a0;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info


static void room_304_init1() {
	global[g131] = 0;
	global[g141] = 0;

	scratch._9c = kernel_run_animation_disp('r', 3, 0);
	kernel_position_anim(scratch._9c, 60, 136, 93, 3);

	scratch._9a = kernel_run_animation_disp('e', 9, 0);
	kernel_position_anim(scratch._9a, 87, 150, 98, 3);

	if (global[g064]) {
		kernel_flip_hotspot(178, 0);
		kernel_flip_hotspot(179, 0);
	} else {
		kernel_timing_trigger(1, 107);
		aa[4] = kernel_run_animation(kernel_name('n', 2), 105);
		scratch.animation_info[4]._active = -1;
		aa[5] = kernel_run_animation(kernel_name('n', 1), 106);
		scratch.animation_info[5]._active = -1;
	}

	if (previous_room != KERNEL_LAST) {
		player.x = 154;
		player.y = 154;
		player.facing = 9;
	}

	if (previous_room == 302) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		scratch.animation_info[0]._active = -1;
		scratch._92 = 55;
		return;
	}

	if (previous_room == 199)
		restore_player();

	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 2);
	kernel_reset_animation(scratch._9c, 2);
	global[g133] = 0;
	global[g143] = 0;
	player.commands_allowed = -1;
	player.walker_visible = -1;
}

static void room_304_init() {
	midi_stop();
	digi_stop(2);
	global[g070] = 0;

	if (previous_room != KERNEL_LAST) {
		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
		}

		for (int i = 0; i < 10; i++) {
			scratch.animation_info[i]._active = 0;
			scratch.animation_info[i]._frame = 1;
			scratch.animation_info[i]._val3 = 0;
			scratch.animation_info[i]._val4 = 0;
		}

		if (previous_room != 199 && flags[18] != 3)
			flags[18]++;
	}

	room_304_init1();
}

static void room_304_finish() {
	player.commands_allowed = -1;
	if (digi_val1)
		kernel_timing_trigger(1, 107);
}

static void room_304_restore() {
	global[g131] = -1;
	global[g141] = -1;
	kernel_reset_animation(scratch._9a, 1);
	kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
	kernel_reset_animation(scratch._9c, 1);
	kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
	player.walker_visible = -1;
	global[g133] = 0;
	global[g143] = 0;
	kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
	room_304_finish();
}

static void room_304_anim1() {
	if (kernel_anim[aa[0]].frame != scratch.animation_info[0]._frame)
		scratch.animation_info[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		scratch.animation_info[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], scratch.animation_info[0]._frame);
	}
}

static void room_304_anim2() {
	int16 var_2 = -1;

	if (kernel_anim[aa[1]].frame == scratch.animation_info[1]._frame)
		return;

	scratch.animation_info[1]._frame = kernel_anim[aa[1]].frame;

	if (scratch._96 == 2) {
		int16 frame = scratch.animation_info[1]._frame;
		if (frame == 5) {
			digi_play_build(304, 'e', 3, 1);
			scratch._9e = 6;
		} else if (frame == 10) {
			var_2 = 5;
		}
	}

	if (var_2 >= 0) {
		scratch.animation_info[1]._frame = var_2;
		kernel_reset_animation(aa[1], var_2);
	}
}

static void room_304_anim3() {
	if (kernel_anim[aa[2]].frame != scratch.animation_info[2]._frame)
		scratch.animation_info[2]._frame = kernel_anim[aa[2]].frame;
}

static void room_304_anim4() {
	int16 var_2 = -1;

	if (kernel_anim[aa[3]].frame == scratch.animation_info[3]._frame)
		return;

	int16 frame = kernel_anim[aa[3]].frame;
	scratch.animation_info[3]._frame = frame;

	switch (scratch._98) {
	case 1:
		if (frame == 65) {
			scratch.animation_info[3]._frame = 60;
			kernel_reset_animation(aa[3], 60);
		} else if (frame < 65) {
			if (frame == 34) {
				scratch.animation_info[3]._frame = 33;
				kernel_reset_animation(aa[3], 33);
			} else if (frame > 34) {
				if (frame == 51) {
					digi_play_build(304, 'b', 2, 1);
					scratch._9e = 4;
				} else if (frame > 51) {
					if (frame == 56) {
						scratch.animation_info[3]._frame = 51;
						kernel_reset_animation(aa[3], 51);
					} else if (frame == 59) {
						scratch.animation_info[3]._frame = 57;
						kernel_reset_animation(aa[3], 57);
					} else if (frame == 60) {
						digi_play_build(304, 'e', 2, 1);
						scratch._9e = 5;
					}
				} else {
					if (frame == 39) {
						digi_play_build(304, 'e', 1, 1);
						scratch._9e = 3;
					} else if (frame == 44) {
						scratch.animation_info[3]._frame = 39;
						kernel_reset_animation(aa[3], 39);
					}
				}
			} else {
				if (frame == 8) {
					digi_play_build(304, 'r', 1, 1);
					scratch._9e = 1;
				} else if (frame == 13) {
					scratch.animation_info[3]._frame = 8;
					kernel_reset_animation(aa[3], 8);
				} else if (frame == 23) {
					digi_play_build(304, 'b', 1, 1);
					scratch._9e = 2;
					kernel_timing_trigger(240, 108);
				} else if (frame == 25) {
					scratch.animation_info[3]._frame = 23;
					kernel_reset_animation(aa[3], 23);
				} else if (frame == 28) {
					scratch.animation_info[3]._frame = 25;
					kernel_reset_animation(aa[3], 25);
				}
			}
		}
		break;
	case 3:
		if (frame == 57) {
			digi_play_build(304, 'e', 4, 1);
			scratch._9e = 7;
		} else if (frame == 62) {
			var_2 = 57;
		}
		break;
	case 5:
		if (frame == 1) {
			global[g064] = -1;
			digi_play_build(304, '_', 2, 2);
			scratch._a0 = 2;
		}
		break;
	case 6:
		if (frame == 1) {
			digi_play_build(304, '_', 3, 2);
			scratch._a0 = 3;
			global[g009] = 0;
			global_digi_play(14);
		} else if (frame == 11) {
			digi_stop(2);
		}
		break;
	case 7:
		if (frame == 4) {
			digi_play_build(304, 'b', 3, 1);
			scratch._9e = 8;
		} else if (frame == 10) {
			var_2 = 4;
		} else if (frame > 10) {
			if (frame == 12) {
				digi_play_build(304, '_', 716, 2);
				scratch._a0 = 4;
			} else if (frame >= 70 && frame <= 72) {
				digi_stop(2);
				scratch._a0 = 0;
			}
		}
		break;
	case 8:
		if (frame == 5) {
			digi_play_build(304, 'e', 2, 1);
			scratch._9e = 10;
		} else if (frame == 10) {
			var_2 = 5;
		}
		break;
	default:
		break;
	}

	if (var_2 >= 0) {
		scratch.animation_info[3]._frame = var_2;
		kernel_reset_animation(aa[3], var_2);
	}
}

static void room_304_anim5() {
	if (kernel_anim[aa[4]].frame == scratch.animation_info[4]._frame)
		return;

	int16 frame = kernel_anim[aa[4]].frame;
	scratch.animation_info[4]._frame = frame;

	if (frame == 4) {
		scratch.animation_info[4]._frame = 0;
		kernel_reset_animation(aa[4], 0);
	}
}

static void room_304_anim6() {
	if (kernel_anim[aa[5]].frame == scratch.animation_info[5]._frame)
		return;

	int16 frame = kernel_anim[aa[5]].frame;
	scratch.animation_info[5]._frame = frame;

	if (frame == 2) {
		scratch.animation_info[5]._frame = 0;
		kernel_reset_animation(aa[5], 0);
	} else if (frame == 4) {
		scratch.animation_info[5]._frame = 3;
		kernel_reset_animation(aa[5], 3);
	}
}

static void room_304_daemon() {
	switch (kernel.trigger) {
	case 8:
		if (scratch._a0 == 4) {
			digi_play_build(304, '_', 716, 2);
			scratch._a0 = 4;
		} else {
			scratch._a0 = 0;
		}
		break;

	case 24:
		if (scratch._8c == 2) {
			aa[1] = kernel_run_animation(kernel_name('e', 1), 102);
			scratch.animation_info[1]._active = -1;
			scratch._96 = 2;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
			kernel_timing_trigger(1, 107);
		}
		break;

	case 25:
		break;

	case 26:
		if (scratch._90 == 3) {
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 1;
			dont_frag_the_palette();
			kernel_abort_animation(aa[5]);
			scratch.animation_info[5]._active = 0;
			kernel_flip_hotspot(178, 0);
			kernel_flip_hotspot(179, 0);
			aa[3] = kernel_run_animation(kernel_name('e', 2), 103);
			scratch.animation_info[3]._active = -1;
			scratch._98 = 3;
			kernel_timing_trigger(1, 107);
		}
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		scratch.animation_info[0]._active = 0;
		if (flags[18] == 1) {
			aa[3] = kernel_run_animation(kernel_name('t', 1), 103);
			scratch.animation_info[3]._active = -1;
			scratch.animation_info[3]._frame = 0;
			scratch._98 = 1;
			kernel_timing_trigger(1, 107);
		} else if (global[g064]) {
			room_304_restore();
		} else {
			global[g141] = -1;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			player.walker_visible = -1;
			global[g143] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			aa[3] = kernel_run_animation(kernel_name('e', 1), 103);
			scratch.animation_info[3]._active = -1;
			scratch.animation_info[3]._frame = 0;
			scratch._98 = 8;
			kernel_timing_trigger(1, 107);
		}
		break;

	case 101:
		kernel_abort_animation(aa[scratch._94]);
		scratch.animation_info[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		room_304_finish();
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[1]);
		scratch.animation_info[1]._active = 0;
		if (scratch._96 == 2) {
			global[g070] = -1;
			display_journal();
		}
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		room_304_finish();
		break;

	case 103:
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		scratch.animation_info[3]._active = 0;
		switch (scratch._98) {
		case 1:
			room_304_restore();
			break;
		case 3:
			aa[3] = kernel_run_animation(kernel_name('e', 3), 103);
			scratch.animation_info[3]._active = -1;
			scratch.animation_info[3]._frame = 0;
			scratch._98 = 4;
			kernel_timing_trigger(1, 107);
			break;
		case 4:
			dont_frag_the_palette();
			kernel_abort_animation(aa[4]);
			scratch.animation_info[4]._active = 0;
			aa[3] = kernel_run_animation(kernel_name('e', 4), 103);
			scratch.animation_info[3]._active = -1;
			scratch.animation_info[3]._frame = 0;
			scratch._98 = 5;
			kernel_timing_trigger(1, 107);
			break;
		case 5:
			aa[3] = kernel_run_animation(kernel_name('e', 5), 103);
			scratch.animation_info[3]._active = -1;
			scratch.animation_info[3]._frame = 0;
			scratch._98 = 6;
			kernel_timing_trigger(1, 107);
			break;
		case 6:
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
			player.walker_visible = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			aa[3] = kernel_run_animation(kernel_name('b', 1), 103);
			scratch.animation_info[3]._active = -1;
			scratch.animation_info[3]._frame = 0;
			scratch._98 = 7;
			kernel_timing_trigger(1, 107);
			break;
		case 7:
			global[g131] = -1;
			global[g141] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			player.walker_visible = -1;
			global[g133] = 0;
			global[g143] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.commands_allowed = -1;
			global[walker_converse_state] = 0;
			close_journal(3);
			break;
		case 8:
			global[g131] = -1;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			global[g133] = 0;
			room_304_finish();
			break;
		default:
			break;
		}
		break;

	case 104:
		switch (scratch._9e) {
		case 2:
			scratch.animation_info[3]._frame = 37;
			kernel_reset_animation(aa[3], 37);
			break;
		case 4:
			scratch.animation_info[3]._frame = 59;
			kernel_reset_animation(aa[3], 59);
			break;
		default:
			break;
		}
		break;

	case 108:
		scratch.animation_info[3]._frame = 25;
		kernel_reset_animation(aa[3], 25);
		break;

	default:
		break;
	}

	global_anim1(9, scratch._9a, global[g131], &global[g132]);
	global_anim2(3, scratch._9c, global[g141], &global[g142]);

	if (scratch.animation_info[0]._active) room_304_anim1();
	if (scratch.animation_info[3]._active) room_304_anim4();
	if (scratch.animation_info[2]._active) room_304_anim3();
	if (scratch.animation_info[1]._active) room_304_anim2();
	if (scratch.animation_info[4]._active) room_304_anim5();
	if (scratch.animation_info[5]._active) room_304_anim6();
}

static void room_304_pre_parser() {
	if (player_parse(13, 35, 0))
		player.walk_off_edge_to_room = 302;
}

static void room_304_parser() {
	if (global[walker_converse_state]) {
		if (global[g064]) {
			player.commands_allowed = 0;
			digi_play_build_ii('c', 1, 1);
			scratch._9e = 9;
		} else {
			player.commands_allowed = 0;
			global[g150] = -1;
			scratch._90 = 3;
		}
		player.command_ready = 0;
		return;
	}

	if (global[player_selected_object] >= 0) {
		player.command_ready = 0;
		return;
	}

	if (player_parse(78, 178, 0)) {
		player.commands_allowed = 0;
		global[g150] = -1;
		scratch._90 = 3;
		player.command_ready = 0;
		return;
	}

	if (player_parse(78, 179, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 2;
		player.command_ready = 0;
	}
}

void room_304_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (auto &ai : scratch.animation_info) ai.synchronize(s);
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

void room_304_preload() {
	room_init_code_pointer = room_304_init;
	room_pre_parser_code_pointer = room_304_pre_parser;
	room_parser_code_pointer = room_304_parser;
	room_daemon_code_pointer = room_304_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
