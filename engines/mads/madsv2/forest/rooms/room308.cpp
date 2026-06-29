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
#include "mads/madsv2/forest/mads/inventory.h"
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
	int16 sprite[10];
	int16 sequence[10];
	int16 animation[24];
	int16 _unused;
	AnimationInfo animation_info[24];
	int16 _b2;
	int16 _b4;
	int16 _b6;
	int16 _b8;
	int16 _ba;
	int16 _bc;
	int16 _be;
	int16 _c0;
	byte _c2;
	byte _c3;
	byte _c4;
};

static Scratch scratch;

#define local  (&scratch)
#define ss     local->sprite
#define seq    local->sequence
#define aa     local->animation
#define aainfo scratch.animation_info


static void room_308_anim1() {
	int16 cur = kernel_anim[aa[0]].frame;
	if (cur == aainfo[0]._frame)
		return;
	aainfo[0]._frame = cur;

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
	int16 cur = kernel_anim[aa[1]].frame;
	if (cur == aainfo[1]._frame)
		return;
	aainfo[1]._frame = cur;
	if (cur == 6) {
		digi_play_build(308, 't', 1, 1);
		scratch._b4 = -1;
	}
}

static void room_308_anim3() {
	int16 cur = kernel_anim[aa[2]].frame;
	if (cur == aainfo[2]._frame)
		return;
	aainfo[2]._frame = cur;
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
	int16 cur = kernel_anim[aa[3]].frame;
	if (cur == aainfo[3]._frame)
		return;
	aainfo[3]._frame = cur;

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
	int16 cur = kernel_anim[aa[4]].frame;
	if (cur == aainfo[4]._frame)
		return;
	aainfo[4]._frame = cur;

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
	int16 cur = kernel_anim[aa[5]].frame;
	if (cur == aainfo[5]._frame)
		return;
	aainfo[5]._frame = cur;
	if (cur == 1 || cur == 5) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	}
}

static void room_308_anim8() {
	int16 cur = kernel_anim[aa[11]].frame;
	if (cur == aainfo[11]._frame)
		return;
	aainfo[11]._frame = cur;

	if (cur == 57) {
		player.commands_allowed = true;
		dont_frag_the_palette();
		kernel_abort_animation(aa[11]);
		aainfo[5]._active = 0;

		aa[6] = kernel_run_animation(kernel_name('t', 11), 0);
		aainfo[6]._active = -1;
		aainfo[6]._val3 = 0;
		kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);

		aa[7] = kernel_run_animation(kernel_name('t', 19), 0);
		aainfo[7]._active = -1;
		aainfo[7]._val3 = 0;
		kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);

		aa[8] = kernel_run_animation(kernel_name('t', 13), 0);
		aainfo[8]._active = -1;
		aainfo[8]._val3 = 0;
		kernel_synch(KERNEL_ANIM, aa[8], KERNEL_NOW, 0);

		aa[9] = kernel_run_animation(kernel_name('t', 18), 0);
		aainfo[9]._active = -1;
		aainfo[9]._val3 = 0;
		kernel_synch(KERNEL_ANIM, aa[9], KERNEL_NOW, 0);

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
	int16 cur = kernel_anim[aa[6]].frame;
	if (cur == aainfo[6]._frame)
		return;
	aainfo[6]._frame = cur;

	int16 result = -1;
	if (cur == 11) {
		result = 10;
	} else if (cur < 11) {
		if (cur == 1) {
			if (aainfo[9]._val3 == 1) {
				result = 1;
				aainfo[9]._val3 = 0;
			} else {
				result = 0;
			}
		} else if (cur == 5) {
			result = 0;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[6], result);
		aainfo[6]._frame = result;
	}
}

static void room_308_anim10() {
	int16 cur = kernel_anim[aa[7]].frame;
	if (cur == aainfo[7]._frame)
		return;
	aainfo[7]._frame = cur;

	int16 result = -1;
	if (cur == 17) {
		scratch._be++;
		if (scratch._be == 4) {
			kernel_timing_trigger(20, 108);
			aainfo[7]._val3 = 0;
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
			int16 active = aainfo[7]._val3;
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
			aainfo[9]._val3 = 4;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[7], result);
		aainfo[7]._frame = result;
	}
}

static void room_308_anim11() {
	int16 cur = kernel_anim[aa[8]].frame;
	if (cur == aainfo[8]._frame)
		return;
	aainfo[8]._frame = cur;

	int16 result = -1;
	if (cur == 6) {
		result = 5;
	} else if (cur == 1 || (cur >= 8 && cur <= 10)) {
		int16 v = aainfo[8]._val3;
		if (v == 0) {
			result = 0;
		} else if (v == 3) {
			result = imath_random(7, 9);
		} else if (v == 6) {
			if (scratch._c2 == 0 || scratch._c2 == 7) {
				aainfo[8]._val3 = 0;
				kernel_reset_animation(aa[6], 10);
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_ANIM, aa[8]);
				kernel_reset_animation(aa[9], 14);
				kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, aa[8]);
				result = 5;
				aa[12] = kernel_run_animation(kernel_name('t', 10), 0);
				aainfo[12]._active = -1;
				kernel_synch(KERNEL_ANIM, aa[12], KERNEL_ANIM, aa[8]);
			} else {
				result = cur - 1;
			}
		} else if (v == 7) {
			if (scratch._c2 == 0) {
				result = cur - 1;
			} else {
				aainfo[8]._val3 = 0;
				if (scratch._b8 == 0) {
					kernel_reset_animation(aa[6], 10);
					kernel_synch(KERNEL_ANIM, aa[6], KERNEL_ANIM, aa[8]);
					kernel_reset_animation(aa[9], 14);
					kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, aa[8]);
					result = 5;
					aa[12] = kernel_run_animation(kernel_name('t', 10), 0);
					aainfo[12]._active = -1;
					kernel_synch(KERNEL_ANIM, aa[12], KERNEL_ANIM, aa[8]);
				} else {
					result = 5;
					aa[13] = kernel_run_animation(kernel_name('t', 14), 0);
					aainfo[13]._active = -1;
					aainfo[13]._val3 = 13;
					kernel_synch(KERNEL_ANIM, aa[13], KERNEL_ANIM, aa[8]);
					digi_play_build(101, '_', 1, 2);
				}
			}
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[8], result);
		aainfo[8]._frame = result;
	}
}

static void room_308_anim12() {
	int16 cur = kernel_anim[aa[9]].frame;
	if (cur == aainfo[9]._frame)
		return;
	aainfo[9]._frame = cur;

	int16 result = -1;
	bool do_e0 = false;

	if (cur == 2) {
		aainfo[9]._val3 = 1;
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
				aainfo[9]._val3 = (scratch._c0 != 0) ? 11 : 10;
				do_e0 = true;
			} else if (scratch._b8 == 0) {
				aainfo[8]._val3 = 0;
				scratch._b6 = 1;
				aainfo[8]._val3 = 7;
				kernel_reset_animation(aa[6], 10);
				kernel_synch(KERNEL_ANIM, aa[6], KERNEL_ANIM, aa[8]);
				kernel_reset_animation(aa[9], 13);
				kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, aa[8]);
				aainfo[9]._frame = 13;
				kernel_reset_animation(aa[8], 5);
				aa[12] = kernel_run_animation(kernel_name('t', 10), 0);
				aainfo[12]._active = -1;
				kernel_synch(KERNEL_ANIM, aa[12], KERNEL_ANIM, aa[8]);
				return;
			} else if (scratch._c0 == 0) {
				kernel_reset_animation(aa[8], 5);
				aa[13] = kernel_run_animation(kernel_name('t', 14), 0);
				aainfo[13]._active = -1;
				aainfo[13]._val3 = 13;
				kernel_synch(KERNEL_ANIM, aa[13], KERNEL_ANIM, aa[8]);
				aainfo[9]._val3 = 8;
				do_e0 = true;
			} else {
				aainfo[9]._val3 = 11;
				do_e0 = true;
			}
		}
	}

	if (do_e0) {
		if (scratch._bc != 0) {
			aainfo[9]._val3 = 12;
			scratch._bc = 0;
		}
		switch (aainfo[9]._val3) {
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
				result = aainfo[9]._frame - 1;
			}
			break;
		case 3:
			result = imath_random(15, 21);
			scratch._c2 = 4;
			break;
		case 4:
			result = 7;
			aainfo[9]._val3 = 0;
			scratch._c2 = 5;
			break;
		case 8:
			result = 0;
			scratch._c2 = 3;
			break;
		case 10:
			aa[14] = kernel_run_animation(kernel_name('t', 21), 0);
			aainfo[14]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[14], KERNEL_NOW, 0);
			kernel_reset_animation(aa[7], 5);
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);
			kernel_reset_animation(aa[9], 13);
			kernel_synch(KERNEL_ANIM, aa[9], KERNEL_NOW, 0);
			scratch._c2 = 2;
			break;
		case 11:
			if (scratch._c0 != 0) {
				dont_frag_the_palette();
				kernel_abort_animation(aa[13]);
				aainfo[13]._active = 0;
			}
			dont_frag_the_palette();
			kernel_abort_animation(aa[9]);
			aainfo[9]._active = 0;
			aa[15] = kernel_run_animation(kernel_name('t', 20), 0);
			aainfo[15]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[15], KERNEL_NOW, 0);
			kernel_reset_animation(aa[7], 5);
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);
			scratch._c2 = 9;
			break;
		case 12:
			aa[16] = kernel_run_animation(kernel_name('t', 15), 0);
			kernel_reset_animation(aa[16], 6);
			aainfo[16]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[16], KERNEL_NOW, 0);
			kernel_reset_animation(aa[6], 10);
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
			aainfo[9]._val3 = 0;
			scratch._c2 = 1;
			break;
		default:
			break;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[9], result);
		aainfo[9]._frame = result;
	}
}

static void room_308_anim13() {
	int16 cur = kernel_anim[aa[10]].frame;
	if (cur == aainfo[10]._frame)
		return;
	aainfo[10]._frame = cur;

	int16 result = -1;
	if (cur == 8) {
		digi_stop(2);
		digi_play_build(308, '_', 4, 2);
	} else if (cur == 10) {
		digi_stop(2);
		digi_play_build(203, '_', 1, 2);
	} else if (cur == 11) {
		scratch._c3++;
		if (scratch._c3 == 3) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[10]);
			aainfo[10]._active = 0;
			aa[17] = kernel_run_animation(kernel_name('t', 23), 0);
			kernel_reset_animation(aa[17], 2);
			aainfo[17]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[17], KERNEL_NOW, 0);
		} else {
			result = 7;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[10], result);
		aainfo[10]._frame = result;
	}
}

static void room_308_anim14() {
	int16 cur = kernel_anim[aa[12]].frame;
	if (cur == aainfo[12]._frame)
		return;
	aainfo[12]._frame = cur;

	if (cur == 6) {
		digi_play_build(101, '_', 1, 2);
	} else if (cur == 9) {
		seq[0] = kernel_seq_stamp(ss[0], 0, -1);
		kernel_seq_depth(seq[0], 4);
		kernel_seq_loc(seq[0], 99, 125);
		kernel_synch(KERNEL_SERIES, seq[0], KERNEL_ANIM, aa[12]);
		scratch._b8 = -1;
		global[g072] = -1;
	} else if (cur == 10) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[12]);
		aainfo[12]._active = 0;
		if (scratch._b6 == 0) {
			kernel_reset_animation(aa[7], 7);
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_ANIM, aa[8]);
			kernel_reset_animation(aa[8], 0);
			kernel_synch(KERNEL_ANIM, aa[8], KERNEL_NOW, 0);
			kernel_reset_animation(aa[6], 0);
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
			kernel_reset_animation(aa[9], 0);
			kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, aa[13]);
		} else if (scratch._b6 == 1) {
			aa[13] = kernel_run_animation(kernel_name('t', 14), 0);
			aainfo[13]._active = -1;
			aainfo[13]._val3 = 13;
			kernel_synch(KERNEL_ANIM, aa[13], KERNEL_NOW, 0);
			kernel_reset_animation(aa[6], 0);
			kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
			kernel_reset_animation(aa[9], 0);
			kernel_synch(KERNEL_ANIM, aa[9], KERNEL_ANIM, aa[13]);
		}
	}
}

static void room_308_anim15() {
	int16 cur = kernel_anim[aa[13]].frame;
	if (cur == aainfo[13]._frame)
		return;
	aainfo[13]._frame = cur;

	int16 result = -1;
	if (cur == 7) {
		if (global[walker_converse_state] != 0) {
			dont_frag_the_palette();
			kernel_abort_animation(aa[13]);
			aainfo[13]._active = 0;
			scratch._c0 = 100;
			dont_frag_the_palette();
			kernel_abort_animation(aa[9]);
			aainfo[9]._active = 0;
			aa[15] = kernel_run_animation(kernel_name('t', 20), 0);
			aainfo[15]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[15], KERNEL_NOW, 0);
			kernel_reset_animation(aa[7], 5);
			kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);
			return;
		}
		if (scratch._c2 == 6)
			scratch._c2 = 0;
		if (scratch._c0 <= 3) {
			scratch._c2 = 6;
		} else {
			if (aainfo[13]._val3 != 0 || scratch._c0 == 0 || scratch._c2 != 0 || imath_random(1, 15) != 1) {
				result = 6;
			} else {
				result = 7;
				scratch._c2 = 6;
			}
		}
	} else if (cur == 10) {
		aainfo[9]._val3 = 4;
	} else if (cur == 12) {
		digi_play_build(308, '_', 4, 2);
	} else if (cur == 13) {
		global[g073] = -1;
		scratch._c0++;
		if (scratch._c0 == 3) {
			kernel_timing_trigger(20, 109);
			aainfo[13]._val3 = 0;
			scratch._c2 = 4;
			scratch._c0++;
		}
		result = 6;
	}

	if (result >= 0) {
		kernel_reset_animation(aa[13], result);
		aainfo[13]._frame = result;
	}
}

static void room_308_anim16() {
	int16 cur = kernel_anim[aa[14]].frame;
	if (cur == aainfo[14]._frame)
		return;
	aainfo[14]._frame = cur;

	if (cur == 1) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 3, 2);
	} else if (cur == 8) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[14]);
		aainfo[14]._active = 0;
		kernel_reset_animation(aa[7], 0);
		aainfo[7]._val3 = 0;
		kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);
		kernel_reset_animation(aa[9], 0);
		aainfo[9]._val3 = 0;
		kernel_synch(KERNEL_ANIM, aa[9], KERNEL_NOW, 0);
		player.commands_allowed = true;
	}
}

static void room_308_anim17() {
	int16 cur = kernel_anim[aa[15]].frame;
	if (cur == aainfo[15]._frame)
		return;
	aainfo[15]._frame = cur;

	int16 result = -1;
	if (cur == 1) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 3, 2);
	} else if (cur == 13) {
		kernel_reset_animation(aa[6], 10);
		kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
		aa[10] = kernel_run_animation(kernel_name('t', 22), 0);
		aainfo[10]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[10], KERNEL_ANIM, aa[15]);
	} else if (cur == 14) {
		result = 13;
	} else if (cur == 23) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[15]);
		aainfo[15]._active = 0;
		aa[18] = kernel_run_animation(kernel_name('t', 24), 0);
		kernel_reset_animation(aa[18], 4);
		aainfo[18]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[18], KERNEL_NOW, 0);
	}

	if (result >= 0) {
		kernel_reset_animation(aa[15], result);
		aainfo[15]._frame = result;
	}
}

static void room_308_anim18() {
	int16 cur = kernel_anim[aa[16]].frame;
	if (cur == aainfo[16]._frame)
		return;
	aainfo[16]._frame = cur;

	if (cur == 6) {
		digi_play_build(203, '_', 4, 2);
		kernel_timing_trigger(25, 111);
	} else if (cur == 11) {
		digi_play_build(308, '_', 4, 2);
	} else if (cur == 15) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[16]);
		aainfo[16]._active = 0;
		kernel_reset_animation(aa[6], 0);
		aainfo[6]._val3 = 0;
		kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);
		player.commands_allowed = true;
	}
}

static void room_308_anim19() {
	int16 cur = kernel_anim[aa[17]].frame;
	if (cur == aainfo[17]._frame)
		return;
	aainfo[17]._frame = cur;

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
			kernel_abort_animation(aa[17]);
			aainfo[17]._active = 0;
			dont_frag_the_palette();
			kernel_abort_animation(aa[15]);
			aainfo[15]._active = 0;
			aa[18] = kernel_run_animation(kernel_name('t', 24), 0);
			aainfo[18]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[18], KERNEL_NOW, 0);
		} else {
			result = 8;
		}
	}

	if (result >= 0) {
		kernel_reset_animation(aa[17], result);
		aainfo[17]._frame = result;
	}
}

static void room_308_anim20() {
	int16 cur = kernel_anim[aa[18]].frame;
	if (cur == aainfo[18]._frame)
		return;
	aainfo[18]._frame = cur;

	int16 result = -1;
	if (cur == 6) {
		global[g009] = 0;
		global_midi_play(5);
		digi_play_build(308, 't', 5, 1);
		aainfo[18]._val3 = 3;
		scratch._b4 = 3;
	} else if (cur == 7 || cur == 8 || cur == 9) {
		if (aainfo[18]._val3 == 3)
			result = imath_random(6, 8);
	} else if (cur == 11) {
		kernel_seq_delete(seq[0]);
	} else if (cur == 17) {
		digi_play_build(203, '_', 1, 2);
	} else if (cur == 20) {
		digi_stop(2);
		digi_play_build(322, '_', 1, 2);
	} else if (cur == 27) {
		dont_frag_the_palette();
		kernel_abort_animation(aa[18]);
		aainfo[18]._active = 0;
		aa[19] = kernel_run_animation(kernel_name('t', 30), 0);
		aainfo[19]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[19], KERNEL_NOW, 0);
	}

	if (result >= 0) {
		kernel_reset_animation(aa[18], result);
		aainfo[18]._frame = result;
	}
}

static void room_308_anim21() {
	int16 cur = kernel_anim[aa[19]].frame;
	if (cur == aainfo[19]._frame)
		return;
	aainfo[19]._frame = cur;

	int16 result = -1;
	if (cur == 3) {
		digi_play_build(308, 'r', 4, 1);
		scratch._b4 = 4;
		aainfo[19]._val3 = 3;
	} else if (cur >= 4 && cur <= 9) {
		if (aainfo[19]._val3 == 3)
			result = imath_random(7, 9);
	} else if (cur == 15) {
		kernel_abort_animation(aa[19]);
		aainfo[19]._active = 0;
		aa[20] = kernel_run_animation(kernel_name('t', 26), 0);
		aainfo[20]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[20], KERNEL_NOW, 0);
		digi_play_build(308, 'b', 4, 1);
		scratch._b4 = -1;
	}

	if (result >= 0) {
		kernel_reset_animation(aa[19], result);
		aainfo[19]._frame = result;
	}
}

static void room_308_anim22() {
	int16 cur = kernel_anim[aa[20]].frame;
	if (cur == aainfo[20]._frame)
		return;
	aainfo[20]._frame = cur;

	if (cur == 70) {
		if (global[walker_converse_state] != 0) {
			global[walker_converse_state] = 0;
			close_interface(CANDLE_FLY);
		}
		object_set_quality(pebbles, -1, -1);
		object_set_quality(rubber_band, -1, -1);
		object_set_quality(forked_stick, -1, -1);
		inter_move_object(rubber_band, NOWHERE);
		inter_move_object(forked_stick, NOWHERE);
		inter_move_object(lily_pad, PLAYER);
		inter_move_object(stick, PLAYER);
		global[player_score] = -1;
		new_room = 401;
	}
}

static void room_308_anim23() {
	int16 cur = kernel_anim[aa[21]].frame;
	if (cur == aainfo[21]._frame)
		return;
	aainfo[21]._frame = cur;

	if (cur == 47) {
		aa[22] = kernel_run_animation(kernel_name('t', 25), 110);
		aainfo[22]._active = -1;
		aainfo[22]._val3 = 3;
		kernel_synch(KERNEL_ANIM, aa[22], KERNEL_NOW, 0);
	} else if (cur == 48 || cur == 64 || cur == 88 || cur == 108 || cur == 140) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	} else if (cur == 164) {
		object_set_quality(pebbles, -1, -1);
		object_set_quality(rubber_band, -1, -1);
		object_set_quality(forked_stick, -1, -1);
		inter_move_object(lily_pad, PLAYER);
		inter_move_object(stick, PLAYER);
		global[player_score] = -1;
		if (global[g064] != 0)
			new_room = 322;
		else
			new_room = 307;
	}
}

static void room_308_anim24() {
	int16 cur = kernel_anim[aa[22]].frame;
	if (cur == aainfo[22]._frame)
		return;
	aainfo[22]._frame = cur;

	int16 result = -1;
	if (cur == 1 || cur == 27) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 3, 1);
		scratch._b4 = -1;
	} else if (cur == 19) {
		digi_play_build(308, 't', 7, 1);
		scratch._b4 = 5;
	} else if (cur >= 20 && cur <= 25) {
		if (aainfo[22]._val3 == 3)
			result = imath_random(20, 25);
	} else if (cur == 28) {
		global[g009] = 0;
		global_midi_play(5);
	}

	if (result >= 0) {
		kernel_reset_animation(aa[22], result);
		aainfo[22]._frame = result;
	}
}

static void room_308_anim25() {
	int16 cur = kernel_anim[aa[23]].frame;
	if (cur == aainfo[23]._frame)
		return;
	aainfo[23]._frame = cur;

	if (cur == 64) {
		aa[22] = kernel_run_animation(kernel_name('t', 25), 110);
		aainfo[22]._active = -1;
		aainfo[22]._val3 = 3;
		kernel_synch(KERNEL_ANIM, aa[22], KERNEL_NOW, 0);
	} else if (cur == 67 || cur == 92 || cur == 116) {
		digi_initial_volume(60);
		digi_play_build(308, '_', 1, 2);
	} else if (cur == 122) {
		object_set_quality(pebbles, -1, -1);
		object_set_quality(rubber_band, -1, -1);
		object_set_quality(forked_stick, -1, -1);
		inter_move_object(rubber_band, NOWHERE);
		inter_move_object(forked_stick, NOWHERE);
		kernel_timing_trigger(140, 112);
	}
}

static void room_308_init() {
	inter_move_object(lily_pad, NOWHERE);
	inter_move_object(stick, NOWHERE);

	if (!player_has_been_in_room(401)) {
		if (previous_room == 307 || previous_room == 322) {
			object_set_quality(pebbles, 0, 0);
			object_set_quality(rubber_band, 0, 0);
			object_set_quality(forked_stick, 0, 0);
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
		for (int i = 0; i < 24; i++)
			aainfo[i]._active = 0;
		scratch._b8 = 0;
		scratch._c0 = 0;
		scratch._be = 0;
	}

	scratch._c4 = 0;
	scratch._b4 = -1;
	scratch._b6 = -1;
	scratch._b2 = 0;
	scratch._ba = 0;
	scratch._bc = 0;
	scratch._c2 = 0;
	scratch._c3 = 0;
	stop_speech_on_run_animation = true;

	kernel_flip_hotspot(words_room_307, false);
	kernel_flip_hotspot(words_room_401, false);

	if (!player_has_been_in_room(401)) {
		ss[0] = kernel_load_series(kernel_name('p', 3), 0);
		if (global[g072] != 0) {
			seq[0] = kernel_seq_stamp(ss[0], false, -1);
			kernel_seq_depth(seq[0], 4);
			kernel_seq_loc(seq[0], 99, 125);
			kernel_synch(KERNEL_SERIES, seq[0], KERNEL_ANIM, aa[12]);
			scratch._be = 100;
			scratch._b8 = -1;
		}
	}

	if (previous_room == 401) {
		player.commands_allowed = false;
		aa[21] = kernel_run_animation(kernel_name('t', 27), 0);
		aainfo[21]._active = -1;
		return;
	}

	if (previous_room == 307 || previous_room == 322) {
		player.commands_allowed = false;
		if (player_has_been_in_room(401)) {
			aa[23] = kernel_run_animation(kernel_name('t', 40), 0);
			kernel_reset_animation(aa[23], 13);
			aainfo[23]._active = -1;
			return;
		}
		if (player.been_here_before && player_has(pebbles) && player_has(rubber_band) && player_has(forked_stick)) {
			aa[3] = kernel_run_animation(kernel_name('t', 8), 103);
			aainfo[3]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
			global[g009] = -1;
			global_midi_play(15);
			player.commands_allowed = false;
			return;
		}
		if (!player.been_here_before) {
			aa[0] = kernel_run_animation(kernel_name('t', 1), 100);
			kernel_reset_animation(aa[0], 13);
			aainfo[0]._active = -1;
			return;
		}
		if (!player_has(pebbles) || !player_has(rubber_band)) {
			aa[4] = kernel_run_animation(kernel_name('t', 6), 104);
			aainfo[4]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
			global[g009] = -1;
			global_midi_play(15);
			return;
		}
		if (player_has(forked_stick)) {
			aa[0] = kernel_run_animation(kernel_name('t', 1), 100);
			kernel_reset_animation(aa[0], 13);
			aainfo[0]._active = -1;
			return;
		}
		aa[4] = kernel_run_animation(kernel_name('t', 6), 104);
		aainfo[4]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
		global[g009] = -1;
		global_midi_play(15);
		return;
	}

	if (previous_room != KERNEL_RESTORING_GAME &&
	    !player_has_been_in_room(401) &&
	    previous_room != 199)
		return;

	aa[6] = kernel_run_animation(kernel_name('t', 11), 0);
	aainfo[6]._active = -1;
	aainfo[6]._val3   = 0;
	kernel_synch(KERNEL_ANIM, aa[6], KERNEL_NOW, 0);

	aa[7] = kernel_run_animation(kernel_name('t', 19), 0);
	aainfo[7]._active = -1;
	aainfo[7]._val3   = 0;
	kernel_synch(KERNEL_ANIM, aa[7], KERNEL_NOW, 0);

	aa[8] = kernel_run_animation(kernel_name('t', 13), 0);
	aainfo[8]._active = -1;
	aainfo[8]._val3   = 0;
	kernel_synch(KERNEL_ANIM, aa[8], KERNEL_NOW, 0);

	aa[9] = kernel_run_animation(kernel_name('t', 18), 0);
	aainfo[9]._active = -1;
	aainfo[9]._val3   = 0;
	kernel_synch(KERNEL_ANIM, aa[9], KERNEL_NOW, 0);

	if (global[g073] == 0) {
		global[g009] = -1;
		global_midi_play(15);
		return;
	}

	kernel_reset_animation(aa[8], 6);
	aa[13] = kernel_run_animation(kernel_name('t', 14), 0);
	kernel_reset_animation(aa[13], 7);
	aainfo[13]._active = -1;
	aainfo[13]._val3   = 0;
	scratch._c0 = 100;
	global[g009] = -1;
	global_midi_play(15);
}

static void room_308_daemon() {
	switch (kernel.trigger) {
	case 7:
		switch (scratch._b4) {
		case 1:
			clear_selected_item();
			aainfo[9]._val3 = 0;
			scratch._b4 = -1;
			player.commands_allowed = true;
			scratch._c2 = 0;
			break;
		case 2:
			clear_selected_item();
			aainfo[9]._val3 = 0;
			aainfo[13]._val3 = 0;
			scratch._b4 = -1;
			player.commands_allowed = true;
			scratch._c2 = 0;
			break;
		case 3:
			aainfo[18]._val3 = 0;
			break;
		case 4:
			aainfo[19]._val3 = 0;
			break;
		case 5:
			aainfo[22]._val3 = 0;
			break;
		}
		break;

	case 100:
		dont_frag_the_palette();
		kernel_abort_animation(aa[0]);
		aainfo[0]._active = 0;
		aa[1] = kernel_run_animation(kernel_name('t', 3), 101);
		aainfo[1]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
		break;

	case 101:
		dont_frag_the_palette();
		kernel_abort_animation(aa[1]);
		aainfo[1]._active = 0;
		aa[2] = kernel_run_animation(kernel_name('t', 5), 102);
		aainfo[2]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
		global[g009] = -1;
		global_midi_play(15);
		break;

	case 102:
		dont_frag_the_palette();
		kernel_abort_animation(aa[2]);
		aainfo[2]._active = 0;
		if (player_has(pebbles) && player_has(rubber_band) && player_has(forked_stick)) {
			aa[3] = kernel_run_animation(kernel_name('t', 8), 103);
			aainfo[3]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
		} else {
			global[player_score] = -1;
			aa[4] = kernel_run_animation(kernel_name('t', 6), 104);
			aainfo[4]._active = -1;
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
		}
		break;

	case 103:
		dont_frag_the_palette();
		kernel_abort_animation(aa[3]);
		aainfo[3]._active = 0;
		aa[5] = kernel_run_animation(kernel_name('t', 7), 105);
		kernel_reset_animation(aa[5], 6);
		aainfo[5]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[5], KERNEL_NOW, 0);
		break;

	case 104:
		object_set_quality(pebbles, -1, -1);
		object_set_quality(rubber_band, -1, -1);
		object_set_quality(forked_stick, -1, -1);
		inter_move_object(lily_pad, PLAYER);
		inter_move_object(stick, PLAYER);
		global[player_score] = -1;
		if (global[g064] != 0)
			new_room = 322;
		else
			new_room = 307;
		break;

	case 105:
		dont_frag_the_palette();
		kernel_abort_animation(aa[5]);
		aainfo[5]._active = 0;
		aa[11] = kernel_run_animation(kernel_name('t', 9), 0);
		kernel_reset_animation(aa[11], 6);
		aainfo[11]._active = -1;
		kernel_synch(KERNEL_ANIM, aa[11], KERNEL_NOW, 0);
		break;

	case 108:
		digi_play_build(308, 't', 3, 1);
		aainfo[9]._val3 = 3;
		scratch._b4 = 1;
		break;

	case 109:
		digi_play_build(308, 't', 4, 1);
		aainfo[9]._val3 = 3;
		scratch._b4 = 2;
		break;

	case 110:
		kernel_abort_animation(aa[22]);
		aainfo[22]._active = 0;
		break;

	case 111:
		digi_stop(2);
		break;

	case 112:
		inter_move_object(lily_pad, PLAYER);
		inter_move_object(stick, PLAYER);
		global[player_score] = -1;
		new_room = 401;
		break;

	default:
		break;
	}

	if (aainfo[0]._active != 0)  room_308_anim1();
	if (aainfo[1]._active != 0)  room_308_anim2();
	if (aainfo[2]._active != 0)  room_308_anim3();
	if (aainfo[3]._active != 0)  room_308_anim4();
	if (aainfo[4]._active != 0)  room_308_anim5();
	if (aainfo[5]._active != 0)  room_308_anim7();
	if (aainfo[11]._active != 0) room_308_anim8();
	if (aainfo[6]._active != 0)  room_308_anim9();
	if (aainfo[7]._active != 0)  room_308_anim10();
	if (aainfo[8]._active != 0)  room_308_anim11();
	if (aainfo[9]._active != 0)  room_308_anim12();
	if (aainfo[10]._active != 0) room_308_anim13();
	if (aainfo[12]._active != 0) room_308_anim14();
	if (aainfo[13]._active != 0) room_308_anim15();
	if (aainfo[14]._active != 0) room_308_anim16();
	if (aainfo[15]._active != 0) room_308_anim17();
	if (aainfo[16]._active != 0) room_308_anim18();
	if (aainfo[17]._active != 0) room_308_anim19();
	if (aainfo[18]._active != 0) room_308_anim20();
	if (aainfo[19]._active != 0) room_308_anim21();
	if (aainfo[20]._active != 0) room_308_anim22();
	if (aainfo[21]._active != 0) room_308_anim23();
	if (aainfo[22]._active != 0) room_308_anim24();
	if (aainfo[23]._active != 0) room_308_anim25();
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
		scratch._ba = -1;
		player.commands_allowed = false;
		aainfo[9]._val3 = 8;
		goto handled;
	}

	if (player_parse(words_tail, 0)) {
		scratch._ba = -1;
		player.commands_allowed = false;
		goto handled;
	}

	if (player_parse(words_russel, 0)) {
		scratch._bc = -1;
		player.commands_allowed = false;
		goto handled;
	}

	sel = global[player_selected_object];
	if (sel < 0)
		goto done;

	if (sel == 15 || sel == 1) {
		inv_enable_command = false;
		object_set_quality(rubber_band, -1, -1);
		object_set_quality(forked_stick, -1, -1);
		object_set_quality(pebbles, -1, -1);
		scratch._b6 = 1;
		aainfo[8]._val3 = 7;
		aainfo[9]._val3 = 8;
		goto handled;
	}

	if (sel == 5) {
		inv_enable_command = false;
		object_set_quality(pebbles, -1, -1);
		scratch._b6 = 0;
		aainfo[8]._val3 = 6;
		aainfo[9]._val3 = 8;
		goto handled;
	}

	goto handled;
handled:
	player.command_ready = false;
done:
	;
}

void room_308_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);

	s.syncAsSint16LE(scratch._unused);

	// Sync aainfo entries individually, with val3 only where present
	// Indices with val3: 6,7,8,9,10,12,13,18,19,20,21,22
	for (int i = 0; i < 24; i++) {
		s.syncAsSint16LE(aainfo[i]._active);
		s.syncAsSint16LE(aainfo[i]._frame);
		switch (i) {
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 12:
		case 13:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
			s.syncAsSint16LE(aainfo[i]._val3);
			break;
		default:
			break;
		}
	}

	s.syncAsSint16LE(scratch._b2);
	s.syncAsSint16LE(scratch._b4);
	s.syncAsSint16LE(scratch._b6);
	s.syncAsSint16LE(scratch._b8);
	s.syncAsSint16LE(scratch._ba);
	s.syncAsSint16LE(scratch._bc);
	s.syncAsSint16LE(scratch._be);
	s.syncAsSint16LE(scratch._c0);
	s.syncAsByte(scratch._c2);
	s.syncAsByte(scratch._c3);
	s.syncAsByte(scratch._c4);
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
