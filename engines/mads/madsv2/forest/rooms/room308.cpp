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

#include "mads/madsv2/forest/rooms/section3.h"
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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sprite.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];                 // 0x00–0x13
	int16 sequence[10];               // 0x14–0x27
	int16 animation[10];              // 0x28–0x3B
	AnimationInfo animation_info[10]; // 0x3C–0x8B
	int16 _8c;                        // 0x8C
	int16 _8e;                        // 0x8E
	int16 _90;                        // 0x90
	int16 _92;                        // 0x92
	int16 _94;                        // 0x94
	int16 _96;                        // 0x96
	int16 _98;                        // 0x98
	int16 _9a;                        // 0x9A
	int16 _9c;                        // 0x9C
	int16 _9e;                        // 0x9E
	int16 _a0;                        // 0xA0
	int16 _a2;                        // 0xA2
	int16 _a4;                        // 0xA4
	int16 _a6;                        // 0xA6
	int16 _a8;                        // 0xA8
	int16 _aa;                        // 0xAA
	int16 _ac;                        // 0xAC
	int16 _ae;                        // 0xAE
	int16 _b0;                        // 0xB0
	int16 _b2;                        // 0xB2
	int16 _b4;                        // 0xB4
	int16 _b6;                        // 0xB6
	int16 _b8;                        // 0xB8
	int16 _ba;                        // 0xBA
	int16 _bc;                        // 0xBC
	int16 _be;                        // 0xBE
	int16 _c0;                        // 0xC0
	byte  _c2;                        // 0xC2
	byte  _c2hi;                      // 0xC3
	byte  _c4;                        // 0xC4
	byte  _c5;                        // 0xC5
	byte  _c6;                        // 0xC6
};

static Scratch scratch;

#define local  (&scratch)
#define ss     local->sprite
#define seq    local->sequence
#define aa     local->animation
#define aainfo scratch.animation_info


static void room_308_anim1() {
	int16 cur = kernel_anim[ss[4]].frame;
	if (cur == aainfo[0]._active)
		return;
	aainfo[0]._active = cur;

	switch (cur) {
	case 40:
		global[player_score] = 0;
		break;
	case 56:
	case 84:
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 3);
		break;
	case 65:
		digi_play_build(308, 'b', 1, 1);
		scratch._b4 = -1;
		break;
	case 70:
		global[g009] = 0;
		global_midi_play(9);
		break;
	case 88:
		digi_initial_volume(60);
		digi_play_build(308, '_', 3, 2);
		break;
	case 102:
		digi_play_build(308, '_', 6, 2);
		break;
	case 107:
		digi_play_build(308, 'b', 2, 1);
		scratch._b4 = -1;
		break;
	case 119:
		digi_play_build(101, '_', 2, 2);
		break;
	case 121:
		digi_play_build(308, 'e', 1, 1);
		scratch._b4 = -1;
		break;
	default:
		break;
	}
}

static void room_308_anim2() {
	int16 cur = kernel_anim[ss[5]].frame;
	if (cur == aainfo[0]._val3)
		return;
	aainfo[0]._val3 = cur;
	if (cur == 6) {
		digi_play_build(308, 't', 1, 1);
		scratch._b4 = -1;
	}
}

static void room_308_anim3() {
	int16 cur = kernel_anim[ss[6]].frame;
	if (cur == aainfo[1]._active)
		return;
	aainfo[1]._active = cur;
	switch (cur) {
	case 2:
		digi_play_build(308, 'r', 1, 1);
		scratch._b4 = -1;
		break;
	case 4:
	case 8:
	case 13:
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
		break;
	default:
		break;
	}
}

static void room_308_anim4() {
	int16 cur = kernel_anim[ss[7]].frame;
	if (cur == aainfo[1]._val3)
		return;
	aainfo[1]._val3 = cur;

	if (cur == 13) {
		digi_play_build(308, 't', 2, 1);
		scratch._b4 = -1;
		return;
	}

	bool in_e72 = (cur == 1 || cur == 5) ||
	              (cur >= 10 && cur <= 58 && cur % 4 == 2);
	if (in_e72 && (config_file.forest1 == 0 || (cur != 10 && cur != 14))) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	}
}

static void room_308_anim5() {
	int16 cur = kernel_anim[ss[8]].frame;
	if (cur == aainfo[2]._active)
		return;
	aainfo[2]._active = cur;

	if (cur == 30 || cur == 104) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 2, 1);
		scratch._b4 = -1;
		return;
	}
	if (cur % 4 == 1 && cur <= 117) {
		if (cur == 1) {
			digi_play_build(308, 'e', 2, 1);
			scratch._b4 = -1;
		}
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	}
}

static void room_308_anim7() {
	int16 cur = kernel_anim[ss[9]].frame;
	if (cur == aainfo[2]._val3)
		return;
	aainfo[2]._val3 = cur;
	if (cur == 1 || cur == 5) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	}
}

static void room_308_anim8() {
	int16 cur = kernel_anim[seq[5]].frame;
	if (cur == aainfo[6]._val4)
		return;
	aainfo[6]._val4 = cur;

	if (cur == 57) {
		player.commands_allowed = true;
		dont_frag_the_palette();
		kernel_abort_animation(seq[5]);
		aainfo[2]._frame = 0;

		seq[0] = kernel_run_animation(kernel_name('t', 11), 0);
		aainfo[2]._val4 = -1;
		aainfo[3]._frame = 0;
		kernel_synch(KERNEL_ANIM, seq[0], KERNEL_NOW, 0);

		seq[1] = kernel_run_animation(kernel_name('t', 19), 0);
		aainfo[3]._val3 = -1;
		aainfo[4]._active = 0;
		kernel_synch(KERNEL_ANIM, seq[1], KERNEL_NOW, 0);

		seq[2] = kernel_run_animation(kernel_name('t', 13), 0);
		aainfo[4]._frame = -1;
		aainfo[4]._val4 = 0;
		kernel_synch(KERNEL_ANIM, seq[2], KERNEL_NOW, 0);

		seq[3] = kernel_run_animation(kernel_name('t', 18), 0);
		aainfo[5]._active = -1;
		aainfo[5]._val3 = 0;
		kernel_synch(KERNEL_ANIM, seq[3], KERNEL_NOW, 0);

		global[player_score] = -1;
		return;
	}
	if (cur > 57)
		return;
	if (cur == 6) {
		digi_play_build(308, 'e', 3, 1);
		scratch._b4 = -1;
		return;
	}
	if (cur == 27) {
		digi_play_build(308, 'b', 3, 1);
		scratch._b4 = -1;
	}
}

static void room_308_anim9() {
	int16 cur = kernel_anim[seq[0]].frame;
	if (cur == aainfo[3]._active)
		return;
	aainfo[3]._active = cur;

	int16 result = -1;
	if (cur == 11) {
		result = 10;
	} else if (cur < 11) {
		if (cur == 1) {
			if (aainfo[5]._val3 == 1) {
				result = 1;
				aainfo[5]._val3 = 0;
			} else {
				result = 0;
			}
		} else if (cur == 5) {
			result = 0;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(seq[0], result);
		aainfo[3]._active = result;
	}
}

static void room_308_anim10() {
	int16 cur = kernel_anim[seq[1]].frame;
	if (cur == aainfo[3]._val4)
		return;
	aainfo[3]._val4 = cur;

	int16 result = -1;
	if (cur == 17) {
		scratch._be++;
		if (scratch._be == 4) {
			kernel_timing_trigger(20, 108);
			aainfo[4]._active = 0;
			scratch._c2 = 4;
			scratch._be++;
			result = 0;
		} else if (scratch._be > 4) {
			result = 0;
		} else {
			scratch._c2 = 8;
			result = 7;
		}
	} else if (cur < 17) {
		if (cur == 1) {
			if (scratch._c2 == 8)
				scratch._c2 = 0;
			int16 active = aainfo[4]._active;
			if (active == 0) {
				if (scratch._be == 0) {
					result = 0;
				} else if (imath_random(1, 15) == 1 && scratch._c2 == 0) {
					result = 7;
					scratch._c2 = 8;
				} else {
					result = 0;
				}
			} else if (active == 9) {
				result = 7;
			}
		} else if (cur == 6) {
			result = 5;
		} else if (cur == 9) {
			aainfo[5]._val3 = 4;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(seq[1], result);
		aainfo[3]._val4 = result;
	}
}

static void room_308_anim11() {
	int16 cur = kernel_anim[seq[2]].frame;
	if (cur == aainfo[4]._val3)
		return;
	aainfo[4]._val3 = cur;

	int16 result = -1;
	if (cur == 6) {
		result = 5;
	} else if (cur == 1 || (cur >= 8 && cur <= 10)) {
		int16 v = aainfo[4]._val4;
		if (v == 0) {
			result = 0;
		} else if (v == 3) {
			result = imath_random(7, 9);
		} else if (v == 6) {
			if (scratch._c2 == 0 || scratch._c2 == 7) {
				aainfo[4]._val4 = 0;
				kernel_reset_animation(seq[0], 10);
				kernel_synch(KERNEL_ANIM, seq[0], KERNEL_ANIM, seq[2]);
				kernel_reset_animation(seq[3], 14);
				kernel_synch(KERNEL_ANIM, seq[3], KERNEL_ANIM, seq[2]);
				result = 5;
				seq[6] = kernel_run_animation(kernel_name('t', 10), 0);
				aainfo[7]._active = -1;
				kernel_synch(KERNEL_ANIM, seq[6], KERNEL_ANIM, seq[2]);
			} else {
				result = cur - 1;
			}
		} else if (v == 7) {
			if (scratch._c2 == 0) {
				result = cur - 1;
			} else {
				aainfo[4]._val4 = 0;
				if (scratch._b8 == 0) {
					kernel_reset_animation(seq[0], 10);
					kernel_synch(KERNEL_ANIM, seq[0], KERNEL_ANIM, seq[2]);
					kernel_reset_animation(seq[3], 14);
					kernel_synch(KERNEL_ANIM, seq[3], KERNEL_ANIM, seq[2]);
					result = 5;
					seq[6] = kernel_run_animation(kernel_name('t', 10), 0);
					aainfo[7]._active = -1;
					kernel_synch(KERNEL_ANIM, seq[6], KERNEL_ANIM, seq[2]);
				} else {
					result = 5;
					seq[7] = kernel_run_animation(kernel_name('t', 14), 0);
					aainfo[7]._val4 = -1;
					aainfo[8]._frame = 13;
					kernel_synch(KERNEL_ANIM, seq[7], KERNEL_ANIM, seq[2]);
					digi_play_build(101, '_', 1, 2);
				}
			}
		}
	}

	if (result >= 0) {
		kernel_reset_animation(seq[2], result);
		aainfo[4]._val3 = result;
	}
}

static void room_308_anim12() {
	int16 cur = kernel_anim[seq[3]].frame;
	if (cur == aainfo[5]._frame)
		return;
	aainfo[5]._frame = cur;

	int16 result = -1;
	bool do_e0 = false;

	if (cur == 2) {
		aainfo[5]._val3 = 1;
	} else if (cur == 7 || cur == 13) {
		result = 0;
	} else if (cur == 14) {
		result = 13;
	} else if (cur == 1 || (cur >= 16 && cur <= 22)) {
		if (scratch._c2 == 1 || scratch._c2 == 2 || scratch._c2 == 7 || scratch._c2 == 5)
			scratch._c2 = 0;
		if (scratch._ba == 0) {
			do_e0 = true;
		} else {
			scratch._ba = 0;
			if (global[walker_converse_state] == 0) {
				aainfo[5]._val3 = (scratch._c0 != 0) ? 11 : 10;
				do_e0 = true;
			} else if (scratch._b8 == 0) {
				aainfo[4]._val4 = 0;
				scratch._b6 = 1;
				aainfo[4]._val4 = 7;
				kernel_reset_animation(seq[0], 10);
				kernel_synch(KERNEL_ANIM, seq[0], KERNEL_ANIM, seq[2]);
				kernel_reset_animation(seq[3], 13);
				kernel_synch(KERNEL_ANIM, seq[3], KERNEL_ANIM, seq[2]);
				aainfo[5]._frame = 13;
				kernel_reset_animation(seq[2], 5);
				seq[6] = kernel_run_animation(kernel_name('t', 10), 0);
				aainfo[7]._active = -1;
				kernel_synch(KERNEL_ANIM, seq[6], KERNEL_ANIM, seq[2]);
				return;
			} else if (scratch._c0 == 0) {
				kernel_reset_animation(seq[2], 5);
				seq[7] = kernel_run_animation(kernel_name('t', 14), 0);
				aainfo[7]._val4 = -1;
				aainfo[8]._frame = 13;
				kernel_synch(KERNEL_ANIM, seq[7], KERNEL_ANIM, seq[2]);
				aainfo[5]._val3 = 8;
				do_e0 = true;
			} else {
				aainfo[5]._val3 = 11;
				do_e0 = true;
			}
		}
	}

	if (do_e0) {
		if (scratch._bc != 0) {
			aainfo[5]._val3 = 12;
			scratch._bc = 0;
		}
		switch (aainfo[5]._val3) {
		case 0:
			if (scratch._c2 == 0) {
				if (imath_random(1, 15) == 1) {
					result = 1;
					scratch._c2 = 7;
				} else {
					result = 0;
					scratch._c2 = 0;
				}
			} else {
				result = aainfo[5]._frame - 1;
			}
			break;
		case 3:
			result = imath_random(15, 21);
			scratch._c2 = 4;
			break;
		case 4:
			result = 7;
			aainfo[5]._val3 = 0;
			scratch._c2 = 5;
			break;
		case 8:
			result = 0;
			scratch._c2 = 3;
			break;
		case 10:
			seq[8] = kernel_run_animation(kernel_name('t', 21), 0);
			aainfo[8]._val3 = -1;
			kernel_synch(KERNEL_ANIM, seq[8], KERNEL_NOW, 0);
			kernel_reset_animation(seq[1], 5);
			kernel_synch(KERNEL_ANIM, seq[1], KERNEL_NOW, 0);
			kernel_reset_animation(seq[3], 13);
			kernel_synch(KERNEL_ANIM, seq[3], KERNEL_NOW, 0);
			scratch._c2 = 2;
			break;
		case 11:
			if (scratch._c0 != 0) {
				dont_frag_the_palette();
				kernel_abort_animation(seq[7]);
				aainfo[7]._val4 = 0;
			}
			dont_frag_the_palette();
			kernel_abort_animation(seq[3]);
			aainfo[5]._active = 0;
			seq[9] = kernel_run_animation(kernel_name('t', 20), 0);
			aainfo[9]._active = -1;
			kernel_synch(KERNEL_ANIM, seq[9], KERNEL_NOW, 0);
			kernel_reset_animation(seq[1], 5);
			kernel_synch(KERNEL_ANIM, seq[1], KERNEL_NOW, 0);
			scratch._c2 = 9;
			break;
		case 12:
			aa[0] = kernel_run_animation(kernel_name('t', 15), 0);
			kernel_reset_animation(aa[0], 6);
			aainfo[9]._val3 = -1;
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			kernel_reset_animation(seq[0], 10);
			kernel_synch(KERNEL_ANIM, seq[0], KERNEL_NOW, 0);
			aainfo[5]._val3 = 0;
			scratch._c2 = 1;
			break;
		default:
			break;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(seq[3], result);
		aainfo[5]._frame = result;
	}
}

static void room_308_anim13() {
	int16 cur = kernel_anim[seq[4]].frame;
	if (cur == aainfo[6]._active)
		return;
	aainfo[6]._active = cur;

	int16 result = -1;
	if (cur == 8) {
		digi_stop(2);
		digi_play_build(308, '_', 4, 2);
	} else if (cur == 10) {
		digi_stop(2);
		digi_play_build(203, '_', 1, 2);
	} else if (cur == 11) {
		scratch._c2hi++;
		if (scratch._c2hi == 3) {
			dont_frag_the_palette();
			kernel_abort_animation(seq[4]);
			aainfo[5]._val4 = 0;
			aa[1] = kernel_run_animation(kernel_name('t', 23), 0);
			kernel_reset_animation(aa[1], 2);
			scratch._8c = -1;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		} else {
			result = 7;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(seq[4], result);
		aainfo[6]._active = result;
	}
}

static void room_308_anim14() {
	int16 cur = kernel_anim[seq[6]].frame;
	if (cur == aainfo[7]._frame)
		return;
	aainfo[7]._frame = cur;

	if (cur == 6) {
		digi_play_build(101, '_', 1, 2);
	} else if (cur == 9) {
		ss[2] = kernel_seq_stamp(ss[0], 0, -1);
		kernel_seq_depth(ss[2], 4);
		kernel_seq_loc(ss[2], 99, 125);
		kernel_synch(KERNEL_SERIES, ss[2], KERNEL_ANIM, seq[6]);
		scratch._b8 = -1;
		global[g072] = -1;
	} else if (cur == 10) {
		dont_frag_the_palette();
		kernel_abort_animation(seq[6]);
		aainfo[7]._active = 0;
		if (scratch._b6 == 0) {
			kernel_reset_animation(seq[1], 7);
			kernel_synch(KERNEL_ANIM, seq[1], KERNEL_ANIM, seq[2]);
			kernel_reset_animation(seq[2], 0);
			kernel_synch(KERNEL_ANIM, seq[2], KERNEL_NOW, 0);
			kernel_reset_animation(seq[0], 0);
			kernel_synch(KERNEL_ANIM, seq[0], KERNEL_NOW, 0);
			kernel_reset_animation(seq[3], 0);
			kernel_synch(KERNEL_ANIM, seq[3], KERNEL_ANIM, seq[7]);
		} else if (scratch._b6 == 1) {
			seq[7] = kernel_run_animation(kernel_name('t', 14), 0);
			aainfo[7]._val4 = -1;
			aainfo[8]._frame = 13;
			kernel_synch(KERNEL_ANIM, seq[7], KERNEL_NOW, 0);
			kernel_reset_animation(seq[0], 0);
			kernel_synch(KERNEL_ANIM, seq[0], KERNEL_NOW, 0);
			kernel_reset_animation(seq[3], 0);
			kernel_synch(KERNEL_ANIM, seq[3], KERNEL_ANIM, seq[7]);
		}
	}
}

static void room_308_anim15() {
	int16 cur = kernel_anim[seq[7]].frame;
	if (cur == aainfo[8]._active)
		return;
	aainfo[8]._active = cur;

	int16 result = -1;
	if (cur == 7) {
		if (global[walker_converse_state] != 0) {
			dont_frag_the_palette();
			kernel_abort_animation(seq[7]);
			aainfo[7]._val4 = 0;
			scratch._c0 = 100;
			dont_frag_the_palette();
			kernel_abort_animation(seq[3]);
			aainfo[5]._active = 0;
			seq[9] = kernel_run_animation(kernel_name('t', 20), 0);
			aainfo[9]._active = -1;
			kernel_synch(KERNEL_ANIM, seq[9], KERNEL_NOW, 0);
			kernel_reset_animation(seq[1], 5);
			kernel_synch(KERNEL_ANIM, seq[1], KERNEL_NOW, 0);
			return;
		}
		if (scratch._c2 == 6)
			scratch._c2 = 0;
		if (scratch._c0 <= 3) {
			scratch._c2 = 6;
		} else {
			if (aainfo[8]._frame != 0 || scratch._c0 == 0 || scratch._c2 != 0 || imath_random(1, 15) != 1) {
				result = 6;
			} else {
				result = 7;
				scratch._c2 = 6;
			}
		}
	} else if (cur == 10) {
		aainfo[5]._val3 = 4;
	} else if (cur == 12) {
		digi_play_build(308, '_', 4, 2);
	} else if (cur == 13) {
		global[g073] = -1;
		scratch._c0++;
		if (scratch._c0 == 3) {
			kernel_timing_trigger(20, 109);
			aainfo[8]._frame = 0;
			scratch._c2 = 4;
			scratch._c0++;
		}
		result = 6;
	}

	if (result >= 0) {
		kernel_reset_animation(seq[7], result);
		aainfo[8]._active = result;
	}
}

static void room_308_anim16() {
	int16 cur = kernel_anim[seq[8]].frame;
	if (cur == aainfo[8]._val4)
		return;
	aainfo[8]._val4 = cur;

	if (cur == 1) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 3, 2);
	} else if (cur == 8) {
		dont_frag_the_palette();
		kernel_abort_animation(seq[8]);
		aainfo[8]._val3 = 0;
		kernel_reset_animation(seq[1], 0);
		aainfo[4]._active = 0;
		kernel_synch(KERNEL_ANIM, seq[1], KERNEL_NOW, 0);
		kernel_reset_animation(seq[3], 0);
		aainfo[5]._val3 = 0;
		kernel_synch(KERNEL_ANIM, seq[3], KERNEL_NOW, 0);
		player.commands_allowed = true;
	}
}

static void room_308_anim17() {
	int16 cur = kernel_anim[seq[9]].frame;
	if (cur == aainfo[9]._frame)
		return;
	aainfo[9]._frame = cur;

	int16 result = -1;
	if (cur == 1) {
		digi_play_build(308, '_', 3, 2);
	} else if (cur == 13) {
		kernel_reset_animation(seq[0], 10);
		kernel_synch(KERNEL_ANIM, seq[0], KERNEL_NOW, 0);
		seq[4] = kernel_run_animation(kernel_name('t', 22), 0);
		aainfo[5]._val4 = -1;
		kernel_synch(KERNEL_ANIM, seq[4], KERNEL_ANIM, seq[9]);
	} else if (cur == 14) {
		result = 13;
	} else if (cur == 23) {
		dont_frag_the_palette();
		kernel_abort_animation(seq[9]);
		aainfo[9]._active = 0;
		aa[2] = kernel_run_animation(kernel_name('t', 24), 0);
		kernel_reset_animation(aa[2], 4);
		scratch._90 = -1;
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
	}

	if (result >= 0) {
		kernel_reset_animation(seq[9], result);
		aainfo[9]._frame = result;
	}
}

static void room_308_anim18() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur == aainfo[9]._val4)
		return;
	aainfo[9]._val4 = cur;

	if (cur == 6) {
		digi_play_build(203, '_', 4, 2);
		kernel_timing_trigger(25, 111);
	} else if (cur == 11) {
		digi_play_build(308, '_', 4, 2);
	} else if (cur == 15) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[9]._val3 = 0;
		kernel_reset_animation(seq[0], 0);
		aainfo[3]._frame = 0;
		kernel_synch(KERNEL_ANIM, seq[0], KERNEL_NOW, 0);
		player.commands_allowed = true;
	}
}

static void room_308_anim19() {
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur == scratch._8e)
		return;
	scratch._8e = cur;

	int16 result = -1;
	if (cur == 2) {
		digi_play_build(203, '_', 4, 2);
		kernel_timing_trigger(20, 111);
	} else if (cur == 9 || cur == 11 || cur == 13) {
		digi_stop(2);
		digi_play_build(308, '_', 5, 2);
	} else if (cur == 14) {
		scratch._c4++;
		if (scratch._c4 == 2) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[1]);
			scratch._8c = 0;
			dont_frag_the_palette();
			kernel_abort_animation(seq[9]);
			aainfo[9]._active = 0;
			aa[2] = kernel_run_animation(kernel_name('t', 24), 0);
			scratch._90 = -1;
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
		} else {
			result = 8;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[1], result);
		scratch._8e = result;
	}
}

static void room_308_anim20() {
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur == scratch._92)
		return;
	scratch._92 = cur;

	int16 result = -1;
	if (cur == 6) {
		global[g009] = 0;
		global_midi_play(5);
		digi_play_build(308, 't', 5, 1);
		scratch._94 = 3;
		scratch._b4 = 3;
	} else if (cur == 7 || cur == 8 || cur == 9) {
		if (scratch._94 == 3)
			result = imath_random(6, 8);
	} else if (cur == 11) {
		kernel_seq_delete(ss[2]);
	} else if (cur == 17) {
		digi_play_build(203, '_', 1, 2);
	} else if (cur == 20) {
		digi_stop(2);
		digi_play_build(322, '_', 1, 2);
	} else if (cur == 27) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		scratch._90 = 0;
		aa[3] = kernel_run_animation(kernel_name('t', 30), 0);
		scratch._96 = -1;
		kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
	}

	if (result >= 0) {
		kernel_reset_animation(aa[2], result);
		scratch._92 = result;
	}
}

static void room_308_anim21() {
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur == scratch._98)
		return;
	scratch._98 = cur;

	int16 result = -1;
	if (cur == 3) {
		digi_play_build(308, 'r', 4, 1);
		scratch._b4 = 4;
		scratch._9a = 3;
	} else if (cur >= 4 && cur <= 9) {
		if (scratch._9a == 3)
			result = imath_random(7, 9);
	} else if (cur == 15) {
		kernel_abort_animation(aa[3]);
		scratch._96 = 0;
		aa[4] = kernel_run_animation(kernel_name('t', 26), 0);
		scratch._9c = -1;
		kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
		digi_play_build(308, 'b', 4, 1);
		scratch._b4 = -1;
	}

	if (result >= 0) {
		kernel_reset_animation(aa[3], result);
		scratch._98 = result;
	}
}

static void room_308_anim22() {
	int16 cur = kernel_anim[aa[4]].frame;
	if (cur == scratch._9e)
		return;
	scratch._9e = cur;

	if (cur == 70) {
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
		}
		object_set_quality(5, -1, -1);
		object_set_quality(1, -1, -1);
		object_set_quality(15, -1, -1);
		inter_move_object(1, NOWHERE);
		inter_move_object(15, NOWHERE);
		inter_move_object(4, PLAYER);
		inter_move_object(14, PLAYER);
		global[player_score] = -1;
		new_room = 401;
	}
}

static void room_308_anim23() {
	int16 cur = kernel_anim[aa[5]].frame;
	if (cur == scratch._a4)
		return;
	scratch._a4 = cur;

	if (cur == 47) {
		aa[6] = kernel_run_animation(kernel_name('t', 25), 110);
		scratch._a8 = -1;
		scratch._ac = 3;
		kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
	} else if (cur == 48 || cur == 64 || cur == 88 || cur == 108 || cur == 140) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	} else if (cur == 164) {
		object_set_quality(5, -1, -1);
		object_set_quality(1, -1, -1);
		object_set_quality(15, -1, -1);
		inter_move_object(4, PLAYER);
		inter_move_object(14, PLAYER);
		global[player_score] = -1;
		if (global[g064] != 0)
			new_room = 322;
		else
			new_room = 307;
	}
}

static void room_308_anim24() {
	int16 cur = kernel_anim[aa[6]].frame;
	if (cur == scratch._aa)
		return;
	scratch._aa = cur;

	int16 result = -1;
	if (cur == 1 || cur == 28) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 3, 1);
		scratch._b4 = -1;
	} else if (cur == 20) {
		digi_play_build(308, 't', 7, 1);
		scratch._b4 = 5;
	} else if (cur >= 21 && cur <= 26) {
		if (scratch._ac == 3)
			result = imath_random(20, 25);
	} else if (cur == 29) {
		global[g009] = 0;
		global_midi_play(5);
	}

	if (result >= 0) {
		kernel_reset_animation(aa[6], result);
		scratch._aa = result;
	}
}

static void room_308_anim25() {
	int16 cur = kernel_anim[aa[7]].frame;
	if (cur == scratch._b0)
		return;
	scratch._b0 = cur;

	if (cur == 64) {
		aa[6] = kernel_run_animation(kernel_name('t', 25), 110);
		scratch._a8 = -1;
		scratch._ac = 3;
		kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
	} else if (cur == 67 || cur == 92 || cur == 116) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	} else if (cur == 122) {
		object_set_quality(5, -1, -1);
		object_set_quality(1, -1, -1);
		object_set_quality(15, -1, -1);
		inter_move_object(1, NOWHERE);
		inter_move_object(15, NOWHERE);
		kernel_timing_trigger(140, 112);
	}
}

static void room_308_init() {
	inter_move_object(4, NOWHERE);
	inter_move_object(14, NOWHERE);

	if (!player_has_been_in_room(401)) {
		if (previous_room == 307 || previous_room == 322) {
			object_set_quality(5, 0, 0);
			object_set_quality(1, 0, 0);
			object_set_quality(15, 0, 0);
		}
	}

	if (previous_room != 199 && flags[22] != 3)
		flags[22]++;

	global[perform_displacements] = 180;

	if (player_has_been_in_room(401)) {
		kernel_flip_hotspot(words_tail, false);
		kernel_flip_hotspot(words_russel, false);
	}

	player.walker_visible = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		aa[9] = 0;
		aainfo[0]._frame  = 0;
		aainfo[0]._val4   = 0;
		aainfo[1]._frame  = 0;
		aainfo[1]._val4   = 0;
		aainfo[2]._frame  = 0;
		aainfo[6]._val3   = 0;
		aainfo[2]._val4   = 0;
		aainfo[3]._val3   = 0;
		aainfo[4]._frame  = 0;
		aainfo[5]._active = 0;
		aainfo[5]._val4   = 0;
		aainfo[7]._active = 0;
		aainfo[7]._val4   = 0;
		aainfo[8]._val3   = 0;
		aainfo[9]._active = 0;
		aainfo[9]._val3   = 0;
		scratch._8c = 0;
		scratch._90 = 0;
		scratch._96 = 0;
		scratch._9c = 0;
		scratch._a2 = 0;
		scratch._a8 = 0;
		scratch._ae = 0;
		scratch._ba = 0;
		scratch._c2 = 0;
		scratch._c0 = 0;
	}

	scratch._c6 = 0;
	scratch._b4 = -1;
	scratch._b8 = -1;
	scratch._b2 = 0;
	scratch._bc = 0;
	scratch._be = 0;
	scratch._c4 = 0;
	scratch._c5 = 0;
	stop_speech_on_run_animation = true;

	kernel_flip_hotspot(words_room_307, false);
	kernel_flip_hotspot(words_room_401, false);

	if (!player_has_been_in_room(401)) {
		ss[0] = kernel_load_series(kernel_name('p', 3), 0);
		if (global[g072] != 0) {
			ss[2] = kernel_seq_stamp(ss[0], false, -1);
			kernel_seq_depth(ss[2], 4);
			kernel_seq_loc(ss[2], 99, 125);
			kernel_synch(KERNEL_SERIES, ss[2], KERNEL_ANIM, seq[6]);
			scratch._c0 = 100;
			scratch._ba = -1;
		}
	}

	if (previous_room == 401) {
		player.commands_allowed = false;
		aa[5] = kernel_run_animation(kernel_name('t', 27), 0);
		scratch._a2 = -1;
		return;
	}

	if (previous_room == 307 || previous_room == 322) {
		player.commands_allowed = false;
		if (player_has_been_in_room(401)) {
			aa[7] = kernel_run_animation(kernel_name('t', 40), 0);
			kernel_reset_animation(aa[7], 13);
			scratch._ae = -1;
			return;
		}
		if (player.been_here_before && player_has(5) && player_has(1) && player_has(15)) {
			ss[7] = kernel_run_animation(kernel_name('t', 8), 103);
			aainfo[1]._frame = -1;
			kernel_synch(KERNEL_ANIM, ss[7], KERNEL_NOW, 0);
			global[g009] = -1;
			global_midi_play(15);
			player.commands_allowed = false;
			return;
		}
		if (!player.been_here_before) {
			ss[4] = kernel_run_animation(kernel_name('t', 1), 100);
			kernel_reset_animation(ss[4], 13);
			aa[9] = -1;
			return;
		}
		if (!player_has(5) || !player_has(1)) {
			ss[8] = kernel_run_animation(kernel_name('t', 6), 104);
			aainfo[1]._val4 = -1;
			kernel_synch(KERNEL_ANIM, ss[8], KERNEL_NOW, 0);
			global[g009] = -1;
			global_midi_play(15);
			return;
		}
		if (player_has(15)) {
			ss[4] = kernel_run_animation(kernel_name('t', 1), 100);
			kernel_reset_animation(ss[4], 13);
			aa[9] = -1;
			return;
		}
		ss[8] = kernel_run_animation(kernel_name('t', 6), 104);
		aainfo[1]._val4 = -1;
		kernel_synch(KERNEL_ANIM, ss[8], KERNEL_NOW, 0);
		global[g009] = -1;
		global_midi_play(15);
		return;
	}

	if (previous_room != KERNEL_RESTORING_GAME &&
	    !player_has_been_in_room(401) &&
	    previous_room != 199)
		return;

	seq[0] = kernel_run_animation(kernel_name('t', 11), 0);
	aainfo[2]._val4   = -1;
	aainfo[3]._frame  = 0;
	kernel_synch(KERNEL_ANIM, seq[0], KERNEL_NOW, 0);

	seq[1] = kernel_run_animation(kernel_name('t', 19), 0);
	aainfo[3]._val3   = -1;
	aainfo[4]._active = 0;
	kernel_synch(KERNEL_ANIM, seq[1], KERNEL_NOW, 0);

	seq[2] = kernel_run_animation(kernel_name('t', 13), 0);
	aainfo[4]._frame  = -1;
	aainfo[4]._val4   = 0;
	kernel_synch(KERNEL_ANIM, seq[2], KERNEL_NOW, 0);

	seq[3] = kernel_run_animation(kernel_name('t', 18), 0);
	aainfo[5]._active = -1;
	aainfo[5]._val3   = 0;
	kernel_synch(KERNEL_ANIM, seq[3], KERNEL_NOW, 0);

	if (global[g073] == 0) {
		global[g009] = -1;
		global_midi_play(15);
		return;
	}

	kernel_reset_animation(seq[2], 6);
	seq[7] = kernel_run_animation(kernel_name('t', 14), 0);
	kernel_reset_animation(seq[7], 7);
	aainfo[7]._val4  = -1;
	aainfo[8]._frame = 0;
	scratch._c2 = 100;
	global[g009] = -1;
	global_midi_play(15);
}

static void room_308_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (scratch._b4) {
		case 1:
			clear_selected_item();
			aainfo[5]._val3 = 0;
			scratch._b4 = -1;
			player.commands_allowed = true;
			scratch._c2 = 0;
			break;
		case 2:
			clear_selected_item();
			aainfo[5]._val3 = 0;
			aainfo[8]._frame = 0;
			scratch._b4 = -1;
			player.commands_allowed = true;
			scratch._c2 = 0;
			break;
		case 3:
			scratch._94 = 0;
			break;
		case 4:
			scratch._9a = 0;
			break;
		case 5:
			scratch._ac = 0;
			break;
		}
		break;

	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(ss[4]);
		aa[9] = 0;
		ss[5] = kernel_run_animation(kernel_name('t', 3), 101);
		aainfo[0]._frame = -1;
		kernel_synch(KERNEL_ANIM, ss[5], KERNEL_NOW, 0);
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(ss[5]);
		aainfo[0]._frame = 0;
		ss[6] = kernel_run_animation(kernel_name('t', 5), 102);
		aainfo[0]._val4 = -1;
		kernel_synch(KERNEL_ANIM, ss[6], KERNEL_NOW, 0);
		global[g009] = -1;
		global_midi_play(15);
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(ss[6]);
		aainfo[0]._val4 = 0;
		if (player_has(5) && player_has(1) && player_has(15)) {
			ss[7] = kernel_run_animation(kernel_name('t', 8), 103);
			aainfo[1]._frame = -1;
			kernel_synch(KERNEL_ANIM, ss[7], KERNEL_NOW, 0);
		} else {
			global[player_score] = -1;
			ss[8] = kernel_run_animation(kernel_name('t', 6), 104);
			aainfo[1]._val4 = -1;
			kernel_synch(KERNEL_ANIM, ss[8], KERNEL_NOW, 0);
		}
		break;

	case 103:
		dont_frag_the_palette();
		kernel_abort_animation(ss[7]);
		aainfo[1]._frame = 0;
		ss[9] = kernel_run_animation(kernel_name('t', 7), 105);
		kernel_reset_animation(ss[9], 6);
		aainfo[2]._frame = -1;
		kernel_synch(KERNEL_ANIM, ss[9], KERNEL_NOW, 0);
		break;

	case 104:
		object_set_quality(5, -1, -1);
		object_set_quality(1, -1, -1);
		object_set_quality(15, -1, -1);
		inter_move_object(4, PLAYER);
		inter_move_object(14, PLAYER);
		global[player_score] = -1;
		if (global[g064] != 0)
			new_room = 322;
		else
			new_room = 307;
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(ss[9]);
		aainfo[2]._frame = 0;
		seq[5] = kernel_run_animation(kernel_name('t', 9), 0);
		kernel_reset_animation(seq[5], 6);
		aainfo[6]._val3 = -1;
		kernel_synch(KERNEL_ANIM, seq[5], KERNEL_NOW, 0);
		break;

	case 108:
		digi_play_build(308, 't', 3, 1);
		aainfo[5]._val3 = 3;
		scratch._b4 = 1;
		break;

	case 109:
		digi_play_build(308, 't', 4, 1);
		aainfo[5]._val3 = 3;
		scratch._b4 = 2;
		break;

	case 110:
		kernel_abort_animation(aa[6]);
		scratch._a8 = 0;
		break;

	case 111:
		digi_stop(2);
		break;

	case 112:
		inter_move_object(4, PLAYER);
		inter_move_object(14, PLAYER);
		global[player_score] = -1;
		new_room = 401;
		break;

	default:
		break;
	}

	if (aa[9] != 0)              room_308_anim1();
	if (aainfo[0]._frame != 0)   room_308_anim2();
	if (aainfo[0]._val4 != 0)    room_308_anim3();
	if (aainfo[1]._frame != 0)   room_308_anim4();
	if (aainfo[1]._val4 != 0)    room_308_anim5();
	if (aainfo[2]._frame != 0)   room_308_anim7();
	if (aainfo[6]._val3 != 0)    room_308_anim8();
	if (aainfo[2]._val4 != 0)    room_308_anim9();
	if (aainfo[3]._val3 != 0)    room_308_anim10();
	if (aainfo[4]._frame != 0)   room_308_anim11();
	if (aainfo[5]._active != 0)  room_308_anim12();
	if (aainfo[5]._val4 != 0)    room_308_anim13();
	if (aainfo[7]._active != 0)  room_308_anim14();
	if (aainfo[7]._val4 != 0)    room_308_anim15();
	if (aainfo[8]._val3 != 0)    room_308_anim16();
	if (aainfo[9]._active != 0)  room_308_anim17();
	if (aainfo[9]._val3 != 0)    room_308_anim18();
	if (scratch._8c != 0)        room_308_anim19();
	if (scratch._90 != 0)        room_308_anim20();
	if (scratch._96 != 0)        room_308_anim21();
	if (scratch._9c != 0)        room_308_anim22();
	if (scratch._a2 != 0)        room_308_anim23();
	if (scratch._a8 != 0)        room_308_anim24();
	if (scratch._ae != 0)        room_308_anim25();
}

static void room_308_pre_parser() {
	if (player_parse(words_walk_to, words_room_307, 0)) {
		if (global[g064] != 0)
			player.walk_off_edge_to_room = 322;
		else
			player.walk_off_edge_to_room = 307;
	}

	if (player_parse(words_walk_to, words_room_401, 0))
		player.walk_off_edge_to_room = 401;
}

static void room_308_parser() {
	int sel;

	if (global[walker_converse_state] != 0) {
		scratch._bc = -1;
		player.commands_allowed = false;
		aainfo[5]._val3 = 8;
		goto handled;
	}

	if (player_parse(words_tail, 0)) {
		scratch._bc = -1;
		player.commands_allowed = false;
		goto handled;
	}

	if (player_parse(words_russel, 0)) {
		scratch._be = -1;
		player.commands_allowed = false;
		goto handled;
	}

	sel = global[player_selected_object];
	if (sel < 0)
		goto done;

	if (sel == 15 || sel == 1) {
		inv_enable_command = false;
		object_set_quality(1, 0, -1);
		object_set_quality(15, 0, -1);
		object_set_quality(5, 0, -1);
		scratch._b8 = 1;
		aainfo[4]._val4 = 7;
		aainfo[5]._val3 = 8;
		goto handled;
	}

	if (sel == 5) {
		inv_enable_command = false;
		object_set_quality(5, 0, -1);
		scratch._b8 = 0;
		aainfo[4]._val4 = 6;
		aainfo[5]._val3 = 8;
		goto handled;
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_308_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (auto &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
	s.syncAsSint16LE(scratch._90);
	s.syncAsSint16LE(scratch._96);
	s.syncAsSint16LE(scratch._9c);
	s.syncAsSint16LE(scratch._a2);
	s.syncAsSint16LE(scratch._a8);
	s.syncAsSint16LE(scratch._ae);
	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
	s.syncAsSint16LE(scratch._b8);
	s.syncAsSint16LE(scratch._ba);
	s.syncAsSint16LE(scratch._bc);
	s.syncAsSint16LE(scratch._be);
	s.syncAsSint16LE(scratch._c0);
	s.syncAsByte(scratch._c2);
	s.syncAsByte(scratch._c2hi);
	s.syncAsByte(scratch._c4);
	s.syncAsByte(scratch._c5);
	s.syncAsByte(scratch._c6);
}

void room_308_preload() {
	room_init_code_pointer       = room_308_init;
	room_pre_parser_code_pointer = room_308_pre_parser;
	room_parser_code_pointer     = room_308_parser;
	room_daemon_code_pointer     = room_308_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
