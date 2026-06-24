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
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
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
};

static Scratch scratch;

#define local  (&scratch)
#define ss     local->sprite
#define seq    local->sequence
#define aa     local->animation
#define aainfo scratch.animation_info


static void room_204_init1() {
	global[g131] = 0;
	global[g141] = 0;
	scratch._9c = kernel_run_animation_disp('r', 2, 0);
	kernel_position_anim(scratch._9c, 171, 116, 73, 8);
	scratch._9a = kernel_run_animation_disp('e', 3, 0);
	kernel_position_anim(scratch._9a, 148, 120, 76, 7);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 194;
		player.y = 121;
		player.facing = 1;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		return;
	} else if (previous_room == 203) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		aainfo[0]._active = -1;
		scratch._92 = 74;
		return;
	} else if (previous_room == 205) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		aainfo[0]._active = -1;
		scratch._92 = 78;
		return;
	} else if (previous_room == 220) {
		aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
		aainfo[0]._active = -1;
		scratch._92 = 66;
		return;
	} else if (previous_room == 199) {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		restore_player();
	} else {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
	}

	player.commands_allowed = -1;
	player.walker_visible = true;
}

static void room_204_init() {
	global[player_score] = -1;
	midi_stop();
	scratch._a8 = 0;
	scratch._a6 = -1;

	if (object_is_here(9)) {
		ss[0] = kernel_load_series(kernel_name('p', 1), 0);
		seq[0] = kernel_seq_stamp(ss[0], false, KERNEL_FIRST);
		kernel_seq_depth(seq[0], 7);
		kernel_seq_loc(seq[0], 293, 87);
		kernel_seq_scale(seq[0], 54);
	} else {
		kernel_flip_hotspot(158, -1);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = false;
			player.commands_allowed = 0;
		}

		for (int count = 0; count < 10; count++) {
			aainfo[count]._active = 0;
			aainfo[count]._frame = 1;
			aainfo[count]._val3 = 0;
			aainfo[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[18] != 3)
			flags[18]++;
	}

	room_204_init1();
	if (flags[18] == -3)
		flags[18] = 1;
}

static void room_204_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_204_anim2() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame)
		aainfo[1]._frame = kernel_anim[aa[1]].frame;

	if (scratch._a4 == 0)
		return;

	kernel_random_frame(scratch._a2, &global[g153], global[g156]);
	kernel_random_frame(scratch._a0, &global[g152], global[g155]);

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a4 == 1) {
		global[g155] = 0;
		kernel_timing_trigger(10, 28);
		scratch._a4 = 2;
	} else if (scratch._a4 == 2) {
		global[g156] = 2;
		scratch._a4 = 3;
		digi_play_build(204, 'e', 1, 1);
	} else if (scratch._a4 == 3) {
		scratch._a4 = 4;
		kernel_abort_animation(scratch._a2);
		kernel_abort_animation(scratch._a0);
		player.commands_allowed = -1;
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

static void room_204_anim3() {
	if (kernel_anim[aa[3]].frame != aainfo[3]._frame)
		aainfo[3]._frame = kernel_anim[aa[3]].frame;
}

static void room_204_anim4() {
	if (kernel_anim[aa[2]].frame != aainfo[2]._frame)
		aainfo[2]._frame = kernel_anim[aa[2]].frame;
}

static void room_204_anim5() {
	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		aainfo[4]._frame = kernel_anim[aa[4]].frame;
		int16 ax = aainfo[4]._frame;

		if (scratch._a6 == 1) {
			if (ax == 51) {
				if (aainfo[4]._val3 == 13) {
					aainfo[4]._frame = 49;
					kernel_reset_animation(aa[4], 49);
				}
			} else if (ax < 51) {
				if (ax == 43) {
					aainfo[4]._val3 = 11;
					digi_play_build_ii('e', 4, 1);
					scratch._a4 = 43;
				} else if (ax == 48) {
					if (aainfo[4]._val3 == 11) {
						aainfo[4]._frame = 43;
						kernel_reset_animation(aa[4], 43);
					}
				}
			}
		} else if (scratch._a6 == 2) {
			if (ax == 13) {
				if (aainfo[4]._val3 == 13) {
					aainfo[4]._frame = 11;
					kernel_reset_animation(aa[4], 11);
				}
			} else if (ax < 13) {
				if (ax == 5) {
					aainfo[4]._val3 = 11;
					digi_play_build_ii('e', 9, 1);
					scratch._a4 = 500;
				} else if (ax == 10) {
					if (aainfo[4]._val3 == 11) {
						aainfo[4]._frame = 5;
						kernel_reset_animation(aa[4], 5);
					}
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a4 == 43) {
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 13;
		scratch._a4 = 44;
	} else if (scratch._a4 == 44) {
		aainfo[4]._val3 = 11;
		aainfo[4]._frame = 43;
		kernel_reset_animation(aa[4], 43);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 45;
	} else if (scratch._a4 == 45) {
		aainfo[4]._val3 = 14;
		aainfo[4]._frame = 50;
		kernel_reset_animation(aa[4], 50);
		scratch._a4 = -1;
	} else if (scratch._a4 == 500) {
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 13;
		scratch._a4 = 501;
	} else if (scratch._a4 == 501) {
		aainfo[4]._val3 = 11;
		aainfo[4]._frame = 5;
		kernel_reset_animation(aa[4], 5);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 502;
	} else if (scratch._a4 == 502) {
		aainfo[4]._val3 = 14;
		aainfo[4]._frame = 11;
		kernel_reset_animation(aa[4], 11);
		scratch._a4 = -1;
	}
}

static void room_204_anim6() {
	if (kernel_anim[aa[5]].frame != aainfo[5]._frame) {
		aainfo[5]._frame = kernel_anim[aa[5]].frame;
		int16 ax = aainfo[5]._frame;

		if (scratch._a6 == 3) {
			if (ax == 64) {
				if (aainfo[5]._val3 == 13) {
					digi_play_build(101, '_', 1, 2);
					scratch._a4 = 64;
				}
			} else if (ax < 64) {
				if (ax == 5) {
					aainfo[5]._val3 = 11;
					digi_play_build(204, 'e', 2, 1);
					scratch._a4 = 5;
				} else if (ax == 10) {
					if (aainfo[5]._val3 == 11) {
						aainfo[5]._frame = 5;
						kernel_reset_animation(aa[5], 5);
					}
				}
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a4 == 5) {
			aainfo[5]._val3 = 13;
			aainfo[5]._frame = 11;
			kernel_reset_animation(aa[5], 11);
			scratch._a4 = -1;
			kernel_seq_delete(seq[0]);
		}
	}

	if (kernel.trigger == 8 || kernel.trigger == 28) {
		if (scratch._a4 == 64)
			scratch._a4 = -1;
	}
}

static void room_204_daemon() {
	int trigger = kernel.trigger;

	if (trigger == 103) {
		goto trigger_103;
	} else if (trigger > 103) {
		goto bottom;
	} else if (trigger == 25) {
		goto trigger_25;
	} else if (trigger > 25) {
		// triggers 26-102
		if (trigger == 26) {
			goto trigger_26;
		} else if (trigger == 100) {
			goto trigger_100;
		} else if (trigger == 101) {
			// abort _94 slot anim, re-enable
			kernel_abort_animation(aa[scratch._94]);
			aainfo[scratch._94]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(3, scratch._9c, 4, 0);
			global[g143] = 0;
			player.commands_allowed = -1;
			goto bottom;
		} else if (trigger == 102) {
			// abort _96 slot anim, re-enable
			kernel_abort_animation(aa[scratch._96]);
			aainfo[scratch._96]._active = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(3, scratch._9a, 4, 0);
			global[g133] = 0;
			player.commands_allowed = -1;
			goto bottom;
		} else {
			goto bottom;
		}
	} else if (trigger == 0) {
		goto bottom;
	} else if (trigger == 7) {
		goto trigger_7;
	} else if (trigger == 24) {
		goto trigger_24;
	} else {
		goto bottom;
	}

trigger_7:
	if (global[walker_converse_state] != 0) {
		global[walker_converse_state] = 0;
		close_journal(3);
		player.commands_allowed = -1;
		goto bottom;
	}
	if (scratch._a4 != 400)
		goto bottom;
	kernel_abort_animation(scratch._9e);
	scratch._a4 = -1;
	scratch._a8 = 0;
	kernel_synch(2, 0, 4, 0);
	scratch._a6 = 3;
	player.walker_visible = true;
	global[g150] = -1;
	scratch._90 = 6;
	goto bottom;

trigger_24:
	if (scratch._8c == 3) {
		aa[2] = kernel_run_animation(kernel_name('E', 1), 102);
		aainfo[2]._active = -1;
		scratch._96 = 2;
		kernel_reset_animation(scratch._9a, 0);
		kernel_synch(3, aa[2], 3, scratch._9a);
		global[g133] = 1;
	} else if (scratch._8c == 5) {
		if (scratch._a6 == 1) {
			aa[4] = kernel_run_animation(kernel_name('F', 1), 102);
		} else if (scratch._a6 == 2) {
			aa[4] = kernel_run_animation(kernel_name('F', 2), 102);
		} else {
			goto bottom;
		}
		aainfo[4]._active = -1;
		scratch._96 = 4;
		kernel_reset_animation(scratch._9a, 0);
		kernel_synch(3, aa[4], 3, scratch._9a);
		global[g133] = 1;
	}
	goto bottom;

trigger_25:
	if (scratch._8e == 2) {
		aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
		aainfo[3]._active = -1;
		scratch._94 = 3;
		kernel_reset_animation(scratch._9c, 0);
		kernel_synch(3, aa[3], 3, scratch._9c);
		global[g143] = 1;
	}
	goto bottom;

trigger_26:
	if (scratch._90 == 1) {
		kernel_reset_animation(scratch._9a, 0);
		kernel_reset_animation(scratch._9c, 0);
		global[g133] = 1;
		global[g143] = 1;
	} else if (scratch._90 == 6) {
		if (scratch._a6 != 3)
			goto bottom;
		aa[5] = kernel_run_animation(kernel_name('P', 1), 103);
		aainfo[5]._active = -1;
		scratch._98 = 5;
		kernel_reset_animation(scratch._9a, 0);
		kernel_reset_animation(scratch._9c, 0);
		kernel_synch(3, aa[5], 4, 0);
		global[g133] = 1;
		global[g143] = 1;
	}
	goto bottom;

trigger_100:
	kernel_abort_animation(aa[0]);
	aainfo[0]._active = 0;
	if (flags[18] == 1) {
		kernel_synch(2, 0, 4, 0);
		global[g017] = 0;
		global[g154] = 0;
		kernel_synch(3, scratch._9a, 4, 0);
		global[g131] = -1;
		global[g133] = 1;
		global[g156] = 0;
		kernel_synch(3, scratch._9c, 4, 0);
		global[g143] = 1;
		scratch._a4 = 1;
		global[g141] = -1;
		player.walker_visible = true;
		aainfo[1]._active = -1;
		global[g155] = 2;
		scratch._a0 = kernel_run_animation_talk('r', 2, 0);
		kernel_position_anim(scratch._a0, 171, 116, 73, 8);
		scratch._a2 = kernel_run_animation_talk('e', 3, 0);
		kernel_position_anim(scratch._a2, 148, 120, 76, 7);
		kernel_synch(3, scratch._a2, 3, scratch._9a);
		kernel_synch(3, scratch._a0, 3, scratch._9c);
		kernel_synch(3, scratch._9e, 2, 0);
		digi_play_build(204, 114, 1, 1);
	} else {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_reset_animation(scratch._9c, 1);
		global[g133] = 0;
		global[g143] = 0;
		kernel_synch(3, scratch._9a, 4, 0);
		kernel_synch(3, scratch._9c, 4, 0);
		player.walker_visible = true;
		kernel_synch(2, 0, 4, 0);
		player.commands_allowed = -1;
	}
	goto bottom;

trigger_103:
	if (scratch._a6 != 3)
		goto bottom;
	kernel_abort_animation(aa[5]);
	aainfo[5]._active = 0;
	kernel_reset_animation(scratch._9c, 1);
	kernel_synch(3, scratch._9c, 4, 0);
	global[g143] = 0;
	kernel_reset_animation(scratch._9a, 1);
	kernel_synch(3, scratch._9a, 4, 0);
	global[g133] = 0;
	kernel_flip_hotspot(158, -1);
	inter_move_object(9, 2);
	global[player_score] = -1;
	player.commands_allowed = -1;

bottom:
	global_anim1(3, scratch._9a, global[g131], &global[g132]);
	global_anim2(2, scratch._9c, global[g141], &global[g142]);

	if (scratch._a8 != 0)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);

	if (aainfo[0]._active != 0) room_204_anim1();
	if (aainfo[1]._active != 0) room_204_anim2();
	if (aainfo[3]._active != 0) room_204_anim3();
	if (aainfo[2]._active != 0) room_204_anim4();
	if (aainfo[4]._active != 0) room_204_anim5();
	if (aainfo[5]._active != 0) room_204_anim6();
}

static void room_204_pre_parser() {
	if (player_parse(13, 23, 0))
		player.walk_off_edge_to_room = 203;

	if (player_parse(13, 25, 0))
		player.walk_off_edge_to_room = flags[13] ? 205 : 220;
}

static void room_204_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(114, 103, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		local->_8c = 5;
		local->_a6 = 1;
		goto handled;
	}

	if (player_parse(114, 82, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		local->_8c = 5;
		local->_a6 = 2;
		goto handled;
	}

	if (player_parse(126, 158, 0)) {
		global[g154] = 2;
		player.commands_allowed = false;
		kernel_run_animation_talk('b', 9, 0);
		kernel_position_anim(local->_9e, player.x, player.y, player.scale, player.depth);
		goto handled;
	}

	if (player_parse(78, 119, 0)) {
		player.commands_allowed = false;
		global[g135] = -1;
		local->_8c = 3;
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_204_synchronize(Common::Serializer &s) {
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
}

void room_204_preload() {
	room_init_code_pointer = room_204_init;
	room_pre_parser_code_pointer = room_204_pre_parser;
	room_parser_code_pointer = room_204_parser;
	room_daemon_code_pointer = room_204_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
