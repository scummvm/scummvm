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

#include "mads/madsv2/forest/rooms/section5.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"
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
	int16 _a4;
	int16 _a6;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

static void room_501_init1();
static void room_501_anim1();
static void room_501_anim2();
static void room_501_anim3();
static void room_501_anim4();
static void room_501_anim5();

static void room_501_init() {
	midi_stop();
	global[g009] = 0;
	global[player_score] = -1;
	scratch._a6 = -1;

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

		if (previous_room != 199) {
			if (flags[32] != 3)
				flags[32]++;
		}
	}

	room_501_init1();
}

static void room_501_init1() {
	global[g131] = 0;
	global[g141] = 0;
	scratch._9c = kernel_run_animation_disp('r', 4, 0);
	kernel_position_anim(scratch._9c, 132, 127, 49, 7);
	scratch._9a = kernel_run_animation_disp('e', 2, 0);
	kernel_position_anim(scratch._9a, 112, 127, 49, 7);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 79;
		player.y = 129;
		player.facing = 3;
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
	} else if (previous_room == 405) {
		if (flags[32] == 1) {
			aa[1] = kernel_run_animation(kernel_name('y', 1), 105);
			aainfo[1]._active = -1;
			scratch._a2 = 1;
			scratch._92 = 84;
		} else {
			aa[0] = kernel_run_animation(kernel_name('y', 3), 100);
			aainfo[0]._active = -1;
			scratch._92 = 104;
		}
	} else if (previous_room == 503) {
		aa[0] = kernel_run_animation(kernel_name('y', 4), 100);
		aainfo[0]._active = -1;
		scratch._92 = 98;
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

static void room_501_anim1() {
	if (kernel_anim[aa[0]].frame != aainfo[0]._frame)
		aainfo[0]._frame = kernel_anim[aa[0]].frame;

	if (global[player_hyperwalked] == -1) {
		aainfo[0]._frame = scratch._92 - 1;
		kernel_reset_animation(aa[0], aainfo[0]._frame);
	}
}

static void room_501_anim2() {
	if (kernel_anim[aa[3]].frame != aainfo[3]._frame)
		aainfo[3]._frame = kernel_anim[aa[3]].frame;
}

static void room_501_anim3() {
}

static void room_501_anim4() {
	if (kernel_anim[aa[4]].frame != aainfo[4]._frame) {
		aainfo[4]._frame = kernel_anim[aa[4]].frame;
		int16 frame = aainfo[4]._frame;

		if (scratch._a6 == 1) {
			if (frame == 42) {
				if (aainfo[4]._val3 == 23) {
					aainfo[4]._frame = 40;
					kernel_reset_animation(aa[4], 40);
				}
			} else if (frame < 42) {
				if (frame == 33) {
					aainfo[4]._val3 = 21;
					digi_play_build_ii('e', 3, 1);
					scratch._a4 = 33;
				} else if (frame == 38) {
					if (aainfo[4]._val3 == 21) {
						aainfo[4]._frame = 33;
						kernel_reset_animation(aa[4], 33);
					}
				}
			}
		} else if (scratch._a6 == 2) {
			if (frame == 49) {
				if (aainfo[4]._val3 == 23) {
					aainfo[4]._frame = 46;
					kernel_reset_animation(aa[4], 46);
				}
			} else if (frame < 49) {
				if (frame == 40) {
					aainfo[4]._val3 = 21;
					digi_play_build_ii('e', 3, 1);
					scratch._a4 = 400;
				} else if (frame == 45) {
					if (aainfo[4]._val3 == 21) {
						aainfo[4]._frame = 40;
						kernel_reset_animation(aa[4], 40);
					}
				}
			}
		} else if (scratch._a6 == 3) {
			if (frame == 40) {
				if (aainfo[4]._val3 == 23) {
					aainfo[4]._frame = 38;
					kernel_reset_animation(aa[4], 38);
				}
			} else if (frame < 40) {
				if (frame == 31) {
					aainfo[4]._val3 = 21;
					digi_play_build_ii('e', 5, 1);
					scratch._a4 = 310;
				} else if (frame == 36) {
					if (aainfo[4]._val3 == 21) {
						aainfo[4]._frame = 31;
						kernel_reset_animation(aa[4], 31);
					}
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a4 == 33) {
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 23;
		scratch._a4 = 34;
	} else if (scratch._a4 == 34) {
		aainfo[4]._val3 = 21;
		aainfo[4]._frame = 33;
		kernel_reset_animation(aa[4], 33);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 35;
	} else if (scratch._a4 == 35) {
		aainfo[4]._val3 = 19;
		aainfo[4]._frame = 39;
		kernel_reset_animation(aa[4], 39);
		scratch._a4 = -1;
	} else if (scratch._a4 == 310) {
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 23;
		scratch._a4 = 311;
	} else if (scratch._a4 == 311) {
		aainfo[4]._val3 = 21;
		aainfo[4]._frame = 31;
		kernel_reset_animation(aa[4], 31);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 312;
	} else if (scratch._a4 == 312) {
		aainfo[4]._val3 = 19;
		aainfo[4]._frame = 37;
		kernel_reset_animation(aa[4], 37);
		scratch._a4 = -1;
	} else if (scratch._a4 == 400) {
		kernel_timing_trigger(30, 28);
		aainfo[4]._val3 = 23;
		scratch._a4 = 401;
	} else if (scratch._a4 == 401) {
		aainfo[4]._val3 = 21;
		aainfo[4]._frame = 40;
		kernel_reset_animation(aa[4], 40);
		digi_play_build_ii('e', 10, 1);
		scratch._a4 = 402;
	} else if (scratch._a4 == 402) {
		aainfo[4]._val3 = 19;
		aainfo[4]._frame = 46;
		kernel_reset_animation(aa[4], 46);
		scratch._a4 = -1;
	}
}

static void room_501_anim5() {
	if (kernel_anim[aa[1]].frame != aainfo[1]._frame) {
		aainfo[1]._frame = kernel_anim[aa[1]].frame;

		if (scratch._a2 == 1) {
			int16 frame = aainfo[1]._frame;
			if (frame > 112) {
				// no action — fall through to trigger check
			} else if (frame == 112) {
				if (aainfo[1]._val3 == 21) {
					aainfo[1]._frame = 108;
					kernel_reset_animation(aa[1], 108);
				}
			} else if (frame > 98) {
				if (frame == 104) {
					if (aainfo[1]._val3 == 21) {
						aainfo[1]._frame = 100;
						kernel_reset_animation(aa[1], 100);
					}
				} else if (frame == 106) {
					if (aainfo[1]._val3 == 23) {
						aainfo[1]._frame = 105;
						kernel_reset_animation(aa[1], 105);
					}
				}
			} else if (frame == 98) {
				if (aainfo[1]._val3 == 23) {
					aainfo[1]._frame = 97;
					kernel_reset_animation(aa[1], 97);
				}
			} else {
				if (frame == 90) {
					aainfo[1]._val3 = 20;
					digi_play_build(501, 'b', 1, 1);
					scratch._a4 = 1;
				} else if (frame == 96) {
					if (aainfo[1]._val3 == 20) {
						aainfo[1]._frame = 91;
						kernel_reset_animation(aa[1], 91);
					}
				}
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a4 == 1) {
			global[g154] = 0;
			aainfo[1]._val3 = 23;
			aainfo[1]._frame = 97;
			kernel_timing_trigger(30, 28);
			kernel_reset_animation(aa[1], aainfo[1]._frame);
			scratch._a4 = 2;
		} else if (scratch._a4 == 2) {
			global[g156] = 2;
			aainfo[1]._val3 = 21;
			aainfo[1]._frame = 99;
			kernel_reset_animation(aa[1], 99);
			digi_play_build(501, 'e', 1, 1);
			scratch._a4 = 3;
		} else if (scratch._a4 == 3) {
			global_midi_play(14);
			global[g156] = 0;
			aainfo[1]._val3 = 23;
			aainfo[1]._frame = 105;
			kernel_timing_trigger(30, 28);
			kernel_reset_animation(aa[1], aainfo[1]._frame);
			scratch._a4 = 4;
		} else if (scratch._a4 == 4) {
			global[g155] = 2;
			aainfo[1]._val3 = 22;
			aainfo[1]._frame = 107;
			kernel_reset_animation(aa[1], 107);
			digi_play_build(501, 'r', 1, 1);
			scratch._a4 = 5;
		} else if (scratch._a4 == 5) {
			global[g156] = 0;
			aainfo[1]._val3 = 23;
			aainfo[1]._frame = 113;
			kernel_reset_animation(aa[1], 113);
			scratch._a4 = 6;
		}
	}

	if (scratch._a2 == 1 && global[player_hyperwalked] == -1) {
		int16 target = scratch._92 - 1;
		if (target > aainfo[1]._frame) {
			aainfo[1]._frame = target;
			kernel_reset_animation(aa[1], target);
		}
	}
}

static void room_501_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
			player.commands_allowed = true;
		}
		break;

	case 24:
		if (scratch._8c == 3) {
			aa[2] = kernel_run_animation(kernel_name('E', 1), 102);
			aainfo[2]._active = -1;
			scratch._96 = 2;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
		} else if (scratch._8c == 5) {
			if (scratch._a6 == 1) {
				aa[4] = kernel_run_animation(kernel_name('F', 1), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			} else if (scratch._a6 == 2) {
				aa[4] = kernel_run_animation(kernel_name('F', 2), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			} else if (scratch._a6 == 3) {
				aa[4] = kernel_run_animation(kernel_name('F', 3), 102);
				aainfo[4]._active = -1;
				scratch._96 = 4;
				kernel_reset_animation(scratch._9a, 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, scratch._9a);
				global[g133] = 1;
			}
		}
		break;

	case 25:
		if (scratch._8e == 2) {
			aa[3] = kernel_run_animation(kernel_name('R', 1), 101);
			aainfo[3]._active = -1;
			scratch._94 = 3;
			kernel_reset_animation(scratch._9c, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9c);
			global[g143] = 1;
		}
		break;

	case 26:
		kernel_reset_animation(scratch._9a, 0);
		kernel_reset_animation(scratch._9c, 0);
		global[g133] = 1;
		global[g143] = 1;
		scratch._90--;
		break;

	case 27:
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = true;
		break;

	case 100:
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
		kernel_abort_animation(aa[scratch._94]);
		aainfo[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = true;
		break;

	case 102:
		kernel_abort_animation(aa[scratch._96]);
		aainfo[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		global[player_score] = -1;
		player.commands_allowed = true;
		break;

	case 105:
		if (scratch._a2 == 1) {
			kernel_abort_animation(aa[1]);
			aa[1] = kernel_run_animation(kernel_name('y', 2), 105);
			scratch._a2 = 2;
		} else if (scratch._a2 == 2) {
			kernel_abort_animation(aa[1]);
			// word_7997C = 0 (unused variable, ignored)
			global[g141] = -1;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			player.walker_visible = true;
			global[g143] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a0 = kernel_run_animation_write(0);
			kernel_reset_animation(scratch._a0, 4);
			kernel_position_anim(scratch._a0, 112, 127, 49, 7);
			global[walker_converse_now] = 1;
			global[g007] = 2;
			kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_NOW, 0);
		}
		break;

	}

	global_anim1(2, scratch._9a, global[g131], &global[g132]);
	global_anim2(4, scratch._9c, global[g141], &global[g142]);

	if (aainfo[0]._active)
		room_501_anim1();
	if (aainfo[3]._active)
		room_501_anim2();
	if (aainfo[2]._active)
		room_501_anim3();
	if (aainfo[4]._active)
		room_501_anim4();
	if (aainfo[1]._active)
		room_501_anim5();

	if (global[walker_converse_now])
		global_anim3(scratch._a0, &global[g008]);
}

static void room_501_pre_parser() {
	if (player_parse(words_walk_to, words_room_405, 0))
		player.walk_off_edge_to_room = 405;
}

static void room_501_parser() {
	if (global[walker_converse_state] != 0) {
		player.commands_allowed = false;
		digi_play_build_ii('c', 1, 1);
		goto handled;
	}

	if (player_parse(words_walk_to, words_room_503, 0)) {
		new_room = 503;
		goto handled;
	}

	if (global[player_selected_object] >= 0) {
		goto handled;
	}

	if (player_parse(words_look_at, words_snapdragon, 0)) {
		global[player_score] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a6 = 1;
		goto handled;
	}

	if (player_parse(words_look_at, words_snapdragon2, 0)) {
		global[player_score] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a6 = 2;
		goto handled;
	}

	if (player_parse(words_look_at, words_primrose, 0)) {
		global[player_score] = 0;
		digi_stop(3);
		player.commands_allowed = false;
		global[g135] = -1;
		scratch._8c = 5;
		scratch._a6 = 3;
		goto handled;
	}

	if (player_parse(words_pick_up, words_flowers, 0) || player_parse(words_click_on, words_moss, 0)) {
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

void room_501_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)         s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)       s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)      s.syncAsSint16LE(v);
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
}

void room_501_preload() {
	room_init_code_pointer       = room_501_init;
	room_pre_parser_code_pointer = room_501_pre_parser;
	room_parser_code_pointer     = room_501_parser;
	room_daemon_code_pointer     = room_501_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
