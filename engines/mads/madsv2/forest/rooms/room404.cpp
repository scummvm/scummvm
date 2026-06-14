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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/mads/inventory.h"
#include "mads/madsv2/forest/mads/sounds.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/rooms/room404.h"

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
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


static void room_404_init1() {
	global[g131] = 0;
	global[g141] = 0;

	if (global[g084] == 16) {
		object_set_quality(13, 0, 0L);
		scratch._9c = kernel_run_animation_disp('r', 6, 0);
		kernel_position_anim(scratch._9c, 123, 145, 92, 1);
		scratch._9a = kernel_run_animation_disp('e', 6, 0);
		kernel_position_anim(scratch._9a, 180, 139, 88, 5);
	} else {
		scratch._9c = kernel_run_animation_disp('r', 9, 0);
		kernel_position_anim(scratch._9c, 91, 153, 98, 2);
		scratch._9a = kernel_run_animation_disp('e', 9, 0);
		kernel_position_anim(scratch._9a, 64, 103, 62, 11);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 145;
		player.y = 110;
		player.facing = 8;
	}

	if (previous_room == KERNEL_RESTORING_GAME) {
		global[g131] = -1;
		global[g141] = -1;
		if (global[g084] == 16) {
			object_set_quality(13, 0, 0L);
			kernel_reset_animation(scratch._9c, 1);
			kernel_reset_animation(scratch._9a, 1);
		} else {
			kernel_reset_animation(scratch._9a, 2);
			kernel_reset_animation(scratch._9c, 2);
		}
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
		player.commands_allowed = -1;
		player.walker_visible = -1;
	} else if (previous_room == 401) {
		aa[0] = kernel_run_animation(kernel_name('y', 1), 100);
		scratch.animation_info[0]._active = -1;
		scratch._92 = 74;
		global[g082] = 0;
	} else if (previous_room == 405) {
		aa[0] = kernel_run_animation(kernel_name('y', 2), 100);
		scratch.animation_info[0]._active = -1;
		scratch._92 = 97;
		global[g081] = -1;
		global[g082] = 0;
		kernel_flip_hotspot(107, 0);
	} else {
		global[g131] = -1;
		global[g141] = -1;
		kernel_reset_animation(scratch._9a, 2);
		kernel_reset_animation(scratch._9c, 2);
		global[g133] = 0;
		global[g143] = 0;
		player.commands_allowed = -1;
		player.walker_visible = -1;
	}

	global[g009] = -1;
	kernel_timing_trigger(1, 106);
}

static void room_404_init() {
	for (int16 count = 0; count < 10; count++) {
		scratch.animation_info[count]._active = 0;
		scratch.animation_info[count]._frame = 0;
		scratch.animation_info[count]._val3 = 0;
		scratch.animation_info[count]._val4 = 0;
	}
	scratch._aa = 0;

	ss[0] = kernel_load_series(kernel_name('p', 1), 0);
	ss[1] = kernel_load_series(kernel_name('p', 3), 0);

	aa[7] = kernel_run_animation(kernel_name('N', 1), 105);
	scratch.animation_info[7]._active = -1;

	aa[8] = kernel_run_animation(kernel_name('N', 2), 105);
	scratch.animation_info[8]._active = -1;

	if (scratch.animation_info[7]._active == -1) {
		kernel.trigger_setup_mode = 1;
		kernel_timing_trigger(1, 110);
	}

	object_set_quality(13, 0, -1L);

	scratch._a2 = 0;
	scratch._a4 = -1;

	if (!global[g081] && previous_room != 405) {
		seq[0] = kernel_seq_stamp(ss[0], false, -1);
		kernel_seq_depth(seq[0], 2);
		kernel_seq_loc(seq[0], 271, 147);
		kernel_seq_scale(seq[0], 77);
	} else {
		kernel_flip_hotspot(107, 0);
		seq[1] = kernel_seq_stamp(ss[1], false, -1);
		kernel_seq_depth(seq[1], 1);
		kernel_seq_loc(seq[1], 229, 161);
		kernel_seq_scale(seq[1], 77);
	}

	global[player_score] = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room != 199) {
			player.walker_visible = 0;
			player.commands_allowed = 0;
		}

		for (int16 count = 0; count < 10; count++) {
			scratch.animation_info[count]._active = 0;
			scratch.animation_info[count]._frame = 1;
			scratch.animation_info[count]._val3 = 0;
			scratch.animation_info[count]._val4 = 0;
		}

		if (previous_room != 199 && flags[29] != 3) {
			flags[29]++;
		}
	}

	room_404_init1();
}

static void room_404_anim1() {
	int16 cur = kernel_anim[scratch.animation[0]].frame;
	if (cur != scratch.animation_info[0]._frame)
		scratch.animation_info[0]._frame = cur;
	if (global[player_hyperwalked] != -1)
		return;
	int16 f = scratch._92 - 1;
	scratch.animation_info[0]._frame = f;
	kernel_reset_animation(scratch.animation[0], f);
}

static void room_404_anim2() {
	int16 cur = kernel_anim[scratch.animation[9]].frame;
	if (cur != scratch.animation_info[9]._frame) {
		scratch.animation_info[9]._frame = cur;
		if (flags[29] == 1) {
			if (cur == 4) {
				scratch.animation_info[9]._val3 = 11;
				digi_play_build(404, 'e', 1, 1);
				scratch._a6 = 1;
			} else if (cur == 7) {
				if (scratch.animation_info[9]._val3 == 11) {
					scratch.animation_info[9]._frame = 5;
					kernel_reset_animation(scratch.animation[9], 5);
				}
			}
		} else {
			if (cur == 18) {
				if (scratch.animation_info[9]._val3 == 10) {
					scratch.animation_info[9]._frame = 15;
					kernel_reset_animation(scratch.animation[9], 15);
				}
			} else if (cur < 18) {
				if (cur == 2) {
					scratch.animation_info[9]._frame = 13;
					kernel_reset_animation(scratch.animation[9], 13);
				} else if (cur == 4) {
					if (scratch.animation_info[9]._val3 == 13) {
						scratch.animation_info[9]._frame = 3;
						kernel_reset_animation(scratch.animation[9], 3);
					}
				} else if (cur == 7) {
					if (scratch.animation_info[9]._val3 == 11) {
						scratch.animation_info[9]._frame = 5;
						kernel_reset_animation(scratch.animation[9], 5);
					}
				} else if (cur == 14) {
					scratch.animation_info[9]._val3 = 10;
					digi_play_build(404, 'b', 5, 1);
					scratch._a6 = 1;
				}
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (flags[29] == 1) {
		scratch.animation_info[9]._val3 = 13;
		scratch.animation_info[9]._frame = 18;
		kernel_reset_animation(scratch.animation[9], 18);
		scratch._a6 = -1;
		return;
	}

	if (scratch._a6 == 1) {
		global[g154] = 0;
		kernel_timing_trigger(30, 28);
		scratch.animation_info[9]._val3 = 13;
		scratch.animation_info[9]._frame = 3;
		kernel_reset_animation(scratch.animation[9], 3);
		scratch._a6 = 2;
	} else if (scratch._a6 == 2) {
		scratch.animation_info[9]._val3 = 11;
		scratch.animation_info[9]._frame = 5;
		kernel_reset_animation(scratch.animation[9], 5);
		scratch._a6 = 3;
		digi_play_build(404, 'e', 5, 1);
	} else if (scratch._a6 == 3) {
		scratch.animation_info[9]._val3 = 13;
		scratch.animation_info[9]._frame = 18;
		kernel_reset_animation(scratch.animation[9], 18);
		scratch._a6 = -1;
	}
}

static void room_404_anim3() {
	int16 cur = kernel_anim[scratch.animation[2]].frame;
	if (cur != scratch.animation_info[2]._frame)
		scratch.animation_info[2]._frame = cur;
}

static void room_404_anim4() {
	int16 cur = kernel_anim[scratch.animation[1]].frame;
	if (cur != scratch.animation_info[1]._frame)
		scratch.animation_info[1]._frame = cur;
}

static void room_404_anim5() {
	int16 cur = kernel_anim[scratch.animation[3]].frame;
	if (cur != scratch.animation_info[3]._frame) {
		scratch.animation_info[3]._frame = cur;
		if (cur == 5) {
			scratch.animation_info[3]._val3 = 11;
			digi_play_build(404, 'e', 3, 1);
			scratch._a6 = 5;
		} else if (cur == 9) {
			if (scratch.animation_info[3]._val3 == 11) {
				scratch.animation_info[3]._frame = 5;
				kernel_reset_animation(scratch.animation[3], 5);
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a6 == 5) {
		scratch.animation_info[3]._val3 = 14;
		scratch.animation_info[3]._frame = 9;
		kernel_reset_animation(scratch.animation[3], 9);
		scratch._a6 = -1;
	}
}

static void room_404_anim6() {
	int16 cur = kernel_anim[scratch.animation[5]].frame;
	if (cur != scratch.animation_info[5]._frame) {
		scratch.animation_info[5]._frame = cur;
		if (cur == 72) {
			if (scratch.animation_info[5]._val3 == 11) {
				scratch.animation_info[5]._frame = 67;
				kernel_reset_animation(scratch.animation[5], 67);
			}
		} else if (cur < 72) {
			if (cur == 5) {
				scratch.animation_info[5]._val3 = 10;
				digi_play_build(404, 'b', 2, 1);
				scratch._a6 = 5;
			} else if (cur == 10) {
				if (scratch.animation_info[5]._val3 == 10) {
					scratch.animation_info[5]._frame = 5;
					kernel_reset_animation(scratch.animation[5], 5);
				}
			} else if (cur == 67) {
				scratch.animation_info[5]._val3 = 11;
				digi_play_build(404, 'e', 2, 1);
				scratch._a6 = 67;
			}
		}
	}

	if (kernel.trigger != 7 && kernel.trigger != 28)
		return;

	if (scratch._a6 == 5) {
		scratch.animation_info[5]._val3 = 13;
		scratch.animation_info[5]._frame = 11;
		kernel_reset_animation(scratch.animation[5], 11);
		scratch._a6 = -1;
	} else if (scratch._a6 == 67) {
		scratch.animation_info[5]._val3 = 13;
		scratch.animation_info[5]._frame = 73;
		kernel_reset_animation(scratch.animation[5], 73);
		scratch._a6 = -1;
	}
}

static void room_404_anim7() {
	int16 cur = kernel_anim[scratch.animation[6]].frame;
	if (cur != scratch.animation_info[6]._frame) {
		scratch.animation_info[6]._frame = cur;
		if (cur == 57) {
			if (scratch.animation_info[6]._val3 == 10) {
				scratch.animation_info[6]._frame = 52;
				kernel_reset_animation(scratch.animation[6], 52);
			}
		} else if (cur > 57) {
			// no action
		} else if (cur < 40) {
			if (cur == 6) {
				scratch.animation_info[6]._val3 = 11;
				digi_play_build(404, 'e', 4, 1);
				scratch._a6 = 6;
				kernel_seq_delete(scratch.sequence[0]);
			} else if (cur == 11) {
				if (scratch.animation_info[6]._val3 == 11) {
					scratch.animation_info[6]._frame = 6;
					kernel_reset_animation(scratch.animation[6], 6);
				}
			} else if (cur == 39) {
				digi_play_build(404, '_', 1, 1);
				scratch._a6 = 39;
				scratch._aa++;
			}
		} else if (cur > 40) {
			if (cur == 41) {
				if (scratch._aa < 6) {
					if (scratch._aa == 4) {
						digi_play_build(404, '_', 2, 2);
						scratch._a6 = 39;
					}
					scratch.animation_info[6]._frame = 39;
					kernel_reset_animation(scratch.animation[6], 39);
					scratch._aa++;
				}
			} else if (cur == 46) {
				digi_stop(2);
				digi_play_build(404, '_', 3, 2);
				scratch._a6 = -1;
			} else if (cur == 52) {
				scratch.animation_info[6]._val3 = 10;
				digi_play_build(404, 'b', 3, 1);
				scratch._a6 = 52;
				scratch.sequence[1] = kernel_seq_stamp(scratch.sprite[1], 0, -1);
				kernel_seq_depth(scratch.sequence[1], 1);
				kernel_seq_loc(scratch.sequence[1], 229, 161);
				kernel_seq_scale(scratch.sequence[1], 77);
			}
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._a6 == 6) {
			scratch.animation_info[6]._val3 = 13;
			scratch.animation_info[6]._frame = 11;
			kernel_reset_animation(scratch.animation[6], 11);
			scratch._a6 = -1;
		} else if (scratch._a6 == 52) {
			scratch.animation_info[6]._val3 = 13;
			scratch.animation_info[6]._frame = 57;
			kernel_reset_animation(scratch.animation[6], 57);
			scratch._a6 = -1;
		}
	}

	if (kernel.trigger == 8 && (scratch._a6 == 39 || scratch._a6 == 46))
		scratch._a6 = -1;
}

static void room_404_anim8() {
	int16 cur = kernel_anim[scratch.animation[7]].frame;
	if (cur != scratch.animation_info[7]._frame) {
		scratch.animation_info[7]._frame = cur;
		if (cur == 99) {
			scratch.animation_info[7]._frame = 1;
			kernel_reset_animation(scratch.animation[7], 1);
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
				kernel_reset_animation(scratch.animation[7], scratch.animation_info[7]._frame);
			}
		}
	}

	if (kernel.trigger == 8 && scratch._a6 == 1)
		scratch._a6 = -1;
}

static void room_404_anim9() {
	int16 cur = kernel_anim[scratch.animation[8]].frame;
	if (cur == scratch.animation_info[8]._frame)
		return;
	scratch.animation_info[8]._frame = cur;

	if (cur == 49) {
		if (imath_random(0, 100) > 50) {
			scratch.animation_info[8]._frame--;
			kernel_reset_animation(scratch.animation[8], scratch.animation_info[8]._frame);
		} else {
			scratch.animation_info[8]._frame = 21;
			kernel_reset_animation(scratch.animation[8], 21);
		}
	} else if (cur < 49) {
		if (cur == 19 || cur == 30 || cur == 40) {
			if (imath_random(0, 100) > 20) {
				scratch.animation_info[8]._frame--;
				kernel_reset_animation(scratch.animation[8], scratch.animation_info[8]._frame);
			}
		} else if (cur == 34 || cur == 46) {
			if (imath_random(0, 100) > 40) {
				scratch.animation_info[8]._frame--;
				kernel_reset_animation(scratch.animation[8], scratch.animation_info[8]._frame);
			}
		} else if (cur == 20 || cur == 48) {
			if (imath_random(0, 100) > 60) {
				scratch.animation_info[8]._frame -= 2;
				kernel_reset_animation(scratch.animation[8], scratch.animation_info[8]._frame);
			}
		}
	}
}

static void room_404_daemon() {
	switch (kernel.trigger) {
	case 7:
		if (global[walker_converse_state] && global[g081]) {
			global[walker_converse_state] = 0;
			close_journal(3);
			player.commands_allowed = -1;
			if (digi_val1)
				kernel_timing_trigger(1, 110);
		} else if (scratch._a6 == 400) {
			kernel_abort_animation(scratch._9e);
			scratch._a6 = -1;
			scratch._a2 = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible = -1;
			player.commands_allowed = -1;
			if (digi_val1)
				kernel_timing_trigger(1, 110);
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
			kernel_timing_trigger(1, 110);
		} else if (scratch._8c == 7) {
			aa[3] = kernel_run_animation(kernel_name('L', 2), 102);
			scratch.animation_info[3]._active = -1;
			scratch._96 = 3;
			kernel_reset_animation(scratch._9a, 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_ANIM, scratch._9a);
			global[g133] = 1;
			kernel_timing_trigger(1, 110);
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
			kernel_timing_trigger(1, 110);
		}
		break;

	case 26:
		if (scratch._90 == 8) {
			// TODO: loc_6B27A
		} else if (scratch._90 < 8) {
			if (scratch._90 == 1) {
				// TODO: loc_6B318
			} else if (scratch._90 == 6) {
				aa[5] = kernel_run_animation(kernel_name('L', 1), 103);
				scratch.animation_info[5]._active = -1;
				scratch._98 = 5;
				kernel_reset_animation(scratch._9a, 0);
				kernel_reset_animation(scratch._9c, 0);
				kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_ANIM, aa[5]);
				kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_ANIM, aa[5]);
				player.walker_visible = 0;
				global[g133] = 1;
				global[g143] = 1;
				scratch._a4 = 1;
				kernel_timing_trigger(1, 110);
			}
		}
		break;

	case 27:
		global[walker_converse_now] = 0;
		kernel_abort_animation(scratch._a0);
		global[g131] = -1;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = -1;
		if (digi_val1)
			kernel_timing_trigger(1, 110);
		break;

	case 100:
		kernel_abort_animation(aa[0]);
		scratch.animation_info[0]._active = 0;
		if (previous_room == 401) {
			global[g084] = 15;
			if (flags[29] == 1 || global[g081]) {
				aa[9] = kernel_run_animation(kernel_name('t', 1), 107);
				kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, aa[0]);
				scratch.animation_info[9]._active = -1;
				player.commands_allowed = 0;
				kernel_timing_trigger(1, 110);
			} else {
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
				if (digi_val1)
					kernel_timing_trigger(1, 110);
			}
		} else if (previous_room == 405) {
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
			if (digi_val1)
				kernel_timing_trigger(1, 110);
		}
		break;

	case 101:
		kernel_abort_animation(aa[scratch._94]);
		scratch.animation_info[scratch._94]._active = 0;
		kernel_reset_animation(scratch._9c, 1);
		kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
		global[g143] = 0;
		player.commands_allowed = -1;
		if (digi_val1)
			kernel_timing_trigger(1, 110);
		break;

	case 102:
		kernel_abort_animation(aa[scratch._96]);
		scratch.animation_info[scratch._96]._active = 0;
		kernel_reset_animation(scratch._9a, 1);
		kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
		global[g133] = 0;
		player.commands_allowed = -1;
		if (digi_val1)
			kernel_timing_trigger(1, 110);
		break;

	case 103:
		if (scratch._a4 == 1) {
			kernel_abort_animation(aa[5]);
			scratch.animation_info[5]._active = 0;
			kernel_abort_animation(scratch._9a);
			kernel_abort_animation(scratch._9c);
			global[g084] = 16;
			object_set_quality(13, 0, 0L);
			scratch._9c = kernel_run_animation_disp('r', 6, 0);
			kernel_position_anim(scratch._9c, 123, 145, 92, 1);
			scratch._9a = kernel_run_animation_disp('e', 6, 0);
			kernel_position_anim(scratch._9a, 180, 139, 88, 5);
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			player.walker_visible = -1;
			global[g133] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g082] = 1;
			if (global[walker_converse_state]) {
				kernel_timing_trigger(5, 104);
			} else {
				player.commands_allowed = -1;
			}
			kernel_timing_trigger(1, 110);
		} else if (scratch._a4 == 2) {
			kernel_abort_animation(aa[6]);
			scratch.animation_info[6]._active = 0;
			aa[6] = kernel_run_animation(kernel_name('L', 4), 103);
			scratch._a4 = 3;
			kernel_timing_trigger(1, 110);
		} else if (scratch._a4 == 3) {
			kernel_abort_animation(aa[6]);
			scratch.animation_info[6]._active = 0;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			global[g143] = 0;
			kernel_reset_animation(scratch._9a, 1);
			kernel_synch(KERNEL_ANIM, scratch._9a, KERNEL_NOW, 0);
			player.walker_visible = -1;
			global[g133] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			global[g081] = -1;
			kernel_flip_hotspot(107, 0);
			object_set_quality(13, 0, 0L);
			inter_move_object(13, 1);
			display_interface();
			if (global[walker_converse_state]) {
				global[walker_converse_state] = 0;
				close_journal(3);
			}
			player.commands_allowed = -1;
			if (digi_val1)
				kernel_timing_trigger(1, 110);
		}
		break;

	case 104:
		player.commands_allowed = 0;
		if (global[walker_converse_state])
			inter_spin_object(13);
		global[g150] = -1;
		scratch._90 = 8;
		break;

	case 107:
		kernel_abort_animation(aa[9]);
		scratch.animation_info[9]._active = 0;
		if (flags[29] == 1) {
			global[g141] = -1;
			kernel_reset_animation(scratch._9c, 1);
			kernel_synch(KERNEL_ANIM, scratch._9c, KERNEL_NOW, 0);
			player.walker_visible = -1;
			global[g143] = 0;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			scratch._a0 = kernel_run_animation_write(0);
			kernel_reset_animation(scratch._a0, 1);
			kernel_position_anim(scratch._a0, 64, 103, 62, 11);
			global[walker_converse_now] = 1;
			global[g007] = 9;
			kernel_synch(KERNEL_ANIM, scratch._a0, KERNEL_NOW, 0);
			kernel_timing_trigger(1, 110);
		} else {
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
			if (digi_val1)
				kernel_timing_trigger(1, 110);
		}
		break;

	case 108:
		global[g150] = -1;
		scratch._90 = 6;
		break;

	case 109:
		player.commands_allowed = 0;
		player_walk(202, 133, 3);
		player_walk_trigger(104);
		break;

	case 110:
		digi_play_build(401, '_', 1, 3);
		digi_val2 = -1;
		break;
	}

	if (global[g084] == 16) {
		global_anim1(6, scratch._9a, global[g131], &global[g132]);
		global_anim2(6, scratch._9c, global[g141], &global[g142]);
	} else {
		global_anim1(9, scratch._9a, global[g131], &global[g132]);
		global_anim2(9, scratch._9c, global[g141], &global[g142]);
	}

	if (scratch._a2)
		kernel_random_frame(scratch._9e, &global[g151], global[g154]);

	if (scratch.animation_info[0]._active)  room_404_anim1();
	if (scratch.animation_info[9]._active)  room_404_anim2();
	if (scratch.animation_info[2]._active)  room_404_anim3();
	if (scratch.animation_info[1]._active)  room_404_anim4();
	if (scratch.animation_info[3]._active)  room_404_anim5();
	if (scratch.animation_info[5]._active)  room_404_anim6();
	if (scratch.animation_info[6]._active)  room_404_anim7();

	if (global[walker_converse_now])
		global_anim3(scratch._a0, &global[g008]);

	room_404_anim8();
	room_404_anim9();
}

static void room_404_pre_parser() {
	if (player_parse(13, 46, 0)) {
		global[g084] = 15;
		player.walk_off_edge_to_room = 401;
	}
}

static void room_404_parser() {
	if (global[walker_converse_state]) {
		player.commands_allowed = 0;
		if (global[g081]) {
			digi_play_build_ii('c', 1, 1);
			player.command_ready = false;
			return;
		}
		if (global[g084] == 16) {
			player.commands_allowed = 0;
			player_walk(202, 133, 3);
			kernel.trigger_setup_mode = 1;
			player_walk_trigger(104);
		} else {
			player_walk(202, 133, 3);
			kernel.trigger_setup_mode = 1;
			player_walk_trigger(108);
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(13, 50, 0)) {
		global[g084] = 15;
		new_room = 405;
		player.command_ready = false;
		return;
	}

	if (global[player_selected_object] >= 0) {
		if (global[player_selected_object] == 13) {
			if (!global[g081] && global[g082]) {
				kernel.trigger_setup_mode = 1;
				kernel_timing_trigger(2, 109);
			} else {
				display_interface();
			}
		} else {
			player.commands_allowed = 0;
			global[g135] = -1;
			scratch._8c = 7;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(114, 116, 0)) {
		player.commands_allowed = 0;
		global[g145] = -1;
		scratch._8e = 2;
		player.command_ready = false;
		return;
	}

	if (player_parse(126, 99, 0)) {
		player.commands_allowed = 0;
		global[g135] = -1;
		scratch._8c = 3;
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 107, 0)) {
		if (!global[g082]) {
			player.commands_allowed = 0;
			global[g150] = -1;
			scratch._90 = 6;
		} else {
			player.commands_allowed = 0;
			global[g135] = -1;
			scratch._8c = 7;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 175, 0)) {
		global[g154] = 2;
		player.commands_allowed = 0;
		player.walker_visible = 0;
		scratch._9e = kernel_run_animation_talk('b', 7, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build(404, 'b', 4, 1);
		scratch._a6 = 400;
		scratch._a2 = -1;
		kernel.trigger_setup_mode = 1;
		kernel_timing_trigger(1, 110);
		player.command_ready = false;
		return;
	}

	if (player_parse(78, 176, 0)) {
		global[g154] = 2;
		player.commands_allowed = 0;
		player.walker_visible = 0;
		scratch._9e = kernel_run_animation_talk('b', 9, 0);
		kernel_position_anim(scratch._9e, player.x, player.y, player.scale, player.depth);
		kernel_synch(KERNEL_ANIM, scratch._9e, KERNEL_PLAYER, 0);
		digi_play_build(404, 'b', 4, 1);
		scratch._a6 = 400;
		scratch._a2 = -1;
		kernel.trigger_setup_mode = 1;
		kernel_timing_trigger(1, 110);
		player.command_ready = false;
		return;
	}
}

void room_404_synchronize(Common::Serializer &s) {
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

void room_404_preload() {
	room_init_code_pointer = room_404_init;
	room_pre_parser_code_pointer = room_404_pre_parser;
	room_parser_code_pointer = room_404_parser;
	room_daemon_code_pointer = room_404_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
