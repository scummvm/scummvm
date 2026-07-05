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

#include "common/textconsole.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/forest/mads/words.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

#define MOVE_YOUR_BUTT_TIMEOUT 3600

int16 flags[40];
bool inv_enable_command;

long noise_clock;
long noise_timer;
int noise_length = -1;
int last_bird_sound = -1;
bool lets_get_a_move_on_anim = true;
int move_your_butt_anim_handle = -1;
bool move_your_butt_enabled = true;
long move_your_butt_timer;
long move_your_butt_clock;
int move_your_butt_anim_frame = -1;

namespace Rooms {

// Section preloads
extern void section_1_preload();
extern void section_2_preload();
extern void section_3_preload();
extern void section_4_preload();
extern void section_5_preload();
extern void section_9_preload();

extern void room_101_synchronize(Common::Serializer &s);
extern void room_101_synchronize(Common::Serializer &s);
extern void room_103_synchronize(Common::Serializer &s);
extern void room_104_synchronize(Common::Serializer &s);
extern void room_106_synchronize(Common::Serializer &s);
extern void room_107_synchronize(Common::Serializer &s);
extern void room_199_synchronize(Common::Serializer &s);
extern void room_201_synchronize(Common::Serializer &s);
extern void room_203_synchronize(Common::Serializer &s);
extern void room_204_synchronize(Common::Serializer &s);
extern void room_205_synchronize(Common::Serializer &s);
extern void room_210_synchronize(Common::Serializer &s);
extern void room_211_synchronize(Common::Serializer &s);
extern void room_220_synchronize(Common::Serializer &s);
extern void room_221_synchronize(Common::Serializer &s);
extern void room_301_synchronize(Common::Serializer &s);
extern void room_302_synchronize(Common::Serializer &s);
extern void room_303_synchronize(Common::Serializer &s);
extern void room_304_synchronize(Common::Serializer &s);
extern void room_305_synchronize(Common::Serializer &s);
extern void room_306_synchronize(Common::Serializer &s);
extern void room_307_synchronize(Common::Serializer &s);
extern void room_308_synchronize(Common::Serializer &s);
extern void room_321_synchronize(Common::Serializer &s);
extern void room_322_synchronize(Common::Serializer &s);
extern void room_401_synchronize(Common::Serializer &s);
extern void room_402_synchronize(Common::Serializer &s);
extern void room_403_synchronize(Common::Serializer &s);
extern void room_404_synchronize(Common::Serializer &s);
extern void room_405_synchronize(Common::Serializer &s);
extern void room_420_synchronize(Common::Serializer &s);
extern void room_501_synchronize(Common::Serializer &s);
extern void room_503_synchronize(Common::Serializer &s);
extern void room_509_synchronize(Common::Serializer &s);
extern void room_510_synchronize(Common::Serializer &s);
extern void room_520_synchronize(Common::Serializer &s);
extern void room_901_synchronize(Common::Serializer &s);
extern void room_903_synchronize(Common::Serializer &s);
extern void room_904_synchronize(Common::Serializer &s);

} // namespace Rooms

void global_init() {
	noise_clock = 0;
	noise_timer = 0;
	noise_length = -1;
	last_bird_sound = -1;
	lets_get_a_move_on_anim = true;
	move_your_butt_anim_handle = -1;
	move_your_butt_enabled = true;
	move_your_butt_timer = 0;
	move_your_butt_clock = 0;
	move_your_butt_anim_frame = -1;
}

void global_section_constructor() {
	section_preload_code_pointer = NULL;
	section_room_constructor = NULL;
	section_init_code_pointer = NULL;
	section_parser_code_pointer = NULL;
	section_daemon_code_pointer = NULL;

	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_section) {
	case 1:
		section_preload_code_pointer = Rooms::section_1_preload;
		break;
	case 2:
		section_preload_code_pointer = Rooms::section_2_preload;
		break;
	case 3:
		section_preload_code_pointer = Rooms::section_3_preload;
		break;
	case 4:
		section_preload_code_pointer = Rooms::section_4_preload;
		break;
	case 5:
		section_preload_code_pointer = Rooms::section_5_preload;
		break;
	case 9:
		section_preload_code_pointer = Rooms::section_9_preload;
		break;
	}
}

void sync_room(Common::Serializer &s) {
	switch (new_room) {
	case 101: Rooms::room_101_synchronize(s); break;
	case 103: Rooms::room_103_synchronize(s); break;
	case 104: Rooms::room_104_synchronize(s); break;
	case 106: Rooms::room_106_synchronize(s); break;
	case 107: Rooms::room_107_synchronize(s); break;
	case 199: Rooms::room_199_synchronize(s); break;
	case 201: Rooms::room_201_synchronize(s); break;
	case 203: Rooms::room_203_synchronize(s); break;
	case 204: Rooms::room_204_synchronize(s); break;
	case 205: Rooms::room_205_synchronize(s); break;
	case 210: Rooms::room_210_synchronize(s); break;
	case 211: Rooms::room_211_synchronize(s); break;
	case 220: Rooms::room_220_synchronize(s); break;
	case 221: Rooms::room_221_synchronize(s); break;
	case 301: Rooms::room_301_synchronize(s); break;
	case 302: Rooms::room_302_synchronize(s); break;
	case 303: Rooms::room_303_synchronize(s); break;
	case 304: Rooms::room_304_synchronize(s); break;
	case 305: Rooms::room_305_synchronize(s); break;
	case 306: Rooms::room_306_synchronize(s); break;
	case 307: Rooms::room_307_synchronize(s); break;
	case 308: Rooms::room_308_synchronize(s); break;
	case 321: Rooms::room_321_synchronize(s); break;
	case 322: Rooms::room_322_synchronize(s); break;
	case 401: Rooms::room_401_synchronize(s); break;
	case 402: Rooms::room_402_synchronize(s); break;
	case 403: Rooms::room_403_synchronize(s); break;
	case 404: Rooms::room_404_synchronize(s); break;
	case 405: Rooms::room_405_synchronize(s); break;
	case 420: Rooms::room_420_synchronize(s); break;
	case 501: Rooms::room_501_synchronize(s); break;
	case 503: Rooms::room_503_synchronize(s); break;
	case 509: Rooms::room_509_synchronize(s); break;
	case 510: Rooms::room_510_synchronize(s); break;
	case 520: Rooms::room_520_synchronize(s); break;
	case 901: Rooms::room_901_synchronize(s); break;
	case 903: Rooms::room_903_synchronize(s); break;
	case 904: Rooms::room_904_synchronize(s); break;
	default: break;
	}
}

void global_section_walker() {
	char buf[80];

	Common::strcpy_s(buf, player.series_name);
	global[g017] = -1;

	if (global[g016]) {
		*player.series_name = '\0';
	} else if (!player.force_series) {
		Common::strcpy_s(player.series_name, "B");
	}

	if (strcmp(player.series_name, buf))
		player.walker_must_reload = true;

	player.scaling_velocity = true;
}

void global_section_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(0));
	pal_change_color(254, 56, 47, 32);
}

static void global_anim1_1(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_4 = -1;

	if (arg_2 == 0) {
		var_4 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g133] == 1)
				var_4 = 0;
			break;

		case 2:
		case 17:
			if (global[g133] == 0) {
				var_4 = 1;
				global[g134]++;
				if (global[g134] > 17) {
					var_4 = imath_random(1, 2);
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_4 = 1;
				global[g136] = -1;
			}
			break;

		case 6:
		case 8:
		case 13:
			if (global[g133] == 0) {
				global[g134]++;
				if (imath_random(5, 10) >= global[g134]) {
					if (*arg_4 == 8)
						var_4 = 6;
					else
						var_4 = *arg_4 - 1;
				} else {
					int16 pick = imath_random(1, 4) - 1;
					if (pick == 0)      var_4 = 5;
					else if (pick == 1) var_4 = 7;
					else if (pick == 2) var_4 = 8;
					else if (pick == 3) var_4 = 13;
					if (*arg_4 == 13)
						var_4 = 7;
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_4 = 13;
			}
			break;

		case 10:
			if (global[g133] == 0) {
				global[g134]++;
				if (imath_random(5, 10) >= global[g134]) {
					var_4 = *arg_4 - 1;
				} else {
					var_4 = imath_random(9, 10);
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_4 = 10;
			}
			break;

		default:
			break;
		}
	}

	if (var_4 >= 0) {
		kernel_reset_animation(arg_0, var_4);
		*arg_4 = var_4;
	}
}

static void global_anim1_2(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g133] == 1)
				var_2 = 0;
			break;

		case 2:
		case 20:
			if (global[g133] == 0) {
				var_2 = 1;
				global[g134]++;
				if (global[g134] > 17) {
					var_2 = imath_random(1, 2);
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_2 = 1;
				global[g136] = -1;
			}
			break;

		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			if (global[g133] == 0) {
				global[g134]++;
				if (global[g134] > 17) {
					var_2 = imath_random(7, 13);
					if (var_2 == 13)
						var_2 = 14;
					global[g134] = 0;
				} else {
					var_2 = *arg_4 - 1;
				}
			} else if (global[g133] == 1) {
				var_2 = 14;
			}
			break;

		case 14:
			var_2 = 7;
			global[g134] = 0;
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

static void global_anim1_3(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g133] == 1)
				var_2 = 0;
			break;

		case 2:
		case 10:
			if (global[g133] == 0) {
				var_2 = 1;
				global[g134]++;
				if (global[g134] > 17) {
					var_2 = imath_random(1, 2);
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_2 = 1;
				global[g136] = -1;
			}
			break;

		case 5:
		case 7:
			if (global[g133] == 0) {
				global[g134]++;
				if (imath_random(5, 10) < global[g134]) {
					var_2 = 7;
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_2 = 7;
			}
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

static void global_anim1_4(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g133] == 1)
				var_2 = 0;
			break;

		case 2:
		case 3:
		case 4:
			if (global[g133] == 0) {
				global[g134]++;
				if (imath_random(7, 15) >= global[g134]) {
					var_2 = *arg_4 - 1;
				} else {
					var_2 = imath_random(1, 3);
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_2 = 4;
			}
			break;

		case 5:
			var_2 = 4;
			global[g136] = -1;
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

static void global_anim1_5(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g133] == 1)
				var_2 = 0;
			break;

		case 2:
		case 3:
			if (global[g133] == 0) {
				global[g134]++;
				if (imath_random(7, 15) >= global[g134]) {
					var_2 = *arg_4 - 1;
				} else {
					var_2 = imath_random(1, 2);
					global[g134] = 0;
				}
			} else if (global[g133] == 1) {
				var_2 = 3;
			}
			break;

		case 4:
			var_2 = 3;
			global[g136] = -1;
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

void global_anim1(int arg_0, int arg_2, int arg_4, int16 *arg_6) {
	if (global[g150]) {
		global[g135] = -1;
		global[g145] = -1;
	}

	if (global[g135])
		global[g133] = 1;

	if (global[g150]) {
		if (global[g146] && global[g136]) {
			global[g146] = 0;
			global[g136] = 0;
			global[g145] = 0;
			global[g135] = 0;
			global[g150] = 0;
			kernel_timing_trigger(1, 26);
		}
	} else {
		if (global[g135] && global[g136]) {
			global[g136] = 0;
			global[g135] = 0;
			kernel_timing_trigger(1, 24);
		}
	}

	switch (arg_0) {
	case 1:
	case 3:
		global_anim1_1(arg_2, arg_4, arg_6);
		break;
	case 2:
		global_anim1_2(arg_2, arg_4, arg_6);
		break;
	case 4:
	case 6:
		global_anim1_3(arg_2, arg_4, arg_6);
		break;
	case 7:
	case 9:
		global_anim1_4(arg_2, arg_4, arg_6);
		break;
	case 8:
		global_anim1_5(arg_2, arg_4, arg_6);

		break;
	default:
		break;
	}
}

static void global_anim2_1(int arg_0, int arg_2, int16 *arg_4);
static void global_anim2_2(int arg_0, int arg_2, int16 *arg_4);
static void global_anim2_3(int arg_0, int arg_2, int16 *arg_4);
static void global_anim2_4(int arg_0, int arg_2, int16 *arg_4);
static void global_anim2_5(int arg_0, int arg_2, int16 *arg_4);

void global_anim2(int arg_0, int arg_2, int arg_4, int16 *arg_6) {
	if (global[g150]) {
		global[g135] = -1;
		global[g145] = -1;
	}

	if (global[g145])
		global[g143] = 1;

	if (global[g150]) {
		if (global[g146] && global[g136]) {
			global[g146] = 0;
			global[g136] = 0;
			global[g145] = 0;
			global[g135] = 0;
			global[g150] = 0;
			kernel_timing_trigger(1, 26);
		}
	} else {
		if (global[g145] && global[g146]) {
			global[g146] = 0;
			global[g145] = 0;
			kernel_timing_trigger(1, 25);
		}
	}

	switch (arg_0) {
	case 1:
	case 3:
		global_anim2_1(arg_2, arg_4, arg_6);
		break;
	case 2:
		global_anim2_2(arg_2, arg_4, arg_6);
		break;
	case 4:
	case 6:
		global_anim2_3(arg_2, arg_4, arg_6);
		break;
	case 7:
	case 9:
		global_anim2_4(arg_2, arg_4, arg_6);
		break;
	case 8:
		global_anim2_5(arg_2, arg_4, arg_6);
		break;
	default:
		break;
	}
}

static void global_anim2_1(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_4 = -1;

	if (arg_2 == 0) {
		var_4 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g143] == 1)
				var_4 = 0;
			break;

		case 2:
		case 3:
		case 11:
			if (global[g143] == 0) {
				global[g144]++;
				if (imath_random(10, 17) >= global[g144]) {
					if (*arg_4 == 11)
						var_4 = 1;
					else
						var_4 = *arg_4 - 1;
				} else {
					var_4 = imath_random(1, 3);
					global[g144] = 0;
				}
			} else if (global[g143] == 1) {
				var_4 = 11;
			}
			break;

		case 5:
		case 6:
		case 8:
			if (global[g143] == 0) {
				global[g144]++;
				if (imath_random(1, 3) >= global[g144]) {
					if (*arg_4 == 8)
						var_4 = 6;
					else if (*arg_4 == 5)
						var_4 = 5;
					else
						var_4 = *arg_4 - 1;
				} else {
					int16 pick = imath_random(1, 3) - 1;
					if (pick == 0)      var_4 = 5;
					else if (pick == 1) var_4 = 6;
					else if (pick == 2) var_4 = 8;
					global[g144] = 0;
				}
			} else if (global[g143] == 1) {
				global[g144] = 0;
				var_4 = 8;
			}
			break;

		case 10:
			global[g144] = 0;
			break;

		case 12:
			var_4 = 11;
			global[g146] = -1;
			break;

		default:
			break;
		}
	}

	if (var_4 >= 0) {
		kernel_reset_animation(arg_0, var_4);
		*arg_4 = var_4;
	}
}

static void global_anim2_2(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g143] == 1)
				var_2 = 0;
			break;

		case 2:
		case 15:
			if (global[g143] == 0) {
				var_2 = 1;
				global[g144]++;
				if (global[g144] > 17) {
					var_2 = imath_random(1, 2);
					global[g144] = 0;
				}
			} else if (global[g143] == 1) {
				var_2 = 15;
			}
			break;

		case 7:
			digi_play_build_ii('_', 2, 3);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			if (global[g143] == 0) {
				global[g144]++;
				if (imath_random(5, 10) >= global[g144]) {
					var_2 = imath_random(8, 10);
				} else {
					var_2 = imath_random(8, 11);
					global[g144] = 0;
				}
			} else if (global[g143] == 1) {
				var_2 = 11;
			}
			break;

		case 16:
			var_2 = 15;
			global[g146] = -1;
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

static void global_anim2_3(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g143] == 1)
				var_2 = 0;
			break;

		case 2:
		case 8:
			if (global[g143] == 0) {
				var_2 = 1;
				global[g144]++;
				if (imath_random(15, 20) < global[g144]) {
					var_2 = imath_random(1, 2);
					global[g144] = 0;
				}
			} else if (global[g143] == 1) {
				var_2 = 8;
			}
			break;

		case 9:
			var_2 = 8;
			global[g146] = -1;
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

static void global_anim2_4(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g143] == 1)
				var_2 = 0;
			break;

		case 2:
		case 7:
			if (global[g143] == 0) {
				global[g144]++;
				if (imath_random(7, 15) >= global[g144]) {
					var_2 = 1;
				} else {
					var_2 = imath_random(1, 2);
					global[g144] = 0;
				}
			} else if (global[g143] == 1) {
				var_2 = 7;
			}
			break;

		case 5:
			if (imath_random(1, 2) == 1)
				var_2 = 3;
			break;

		case 8:
			var_2 = 7;
			global[g146] = -1;
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

static void global_anim2_5(int arg_0, int arg_2, int16 *arg_4) {
	if (kernel_anim[arg_0].frame == *arg_4)
		return;

	*arg_4 = kernel_anim[arg_0].frame;
	int16 var_2 = -1;

	if (arg_2 == 0) {
		var_2 = 0;
	} else {
		switch (*arg_4) {
		case 1:
			if (global[g143] == 1)
				var_2 = 0;
			break;

		case 2:
		case 3:
			if (global[g143] == 0) {
				global[g144]++;
				if (imath_random(7, 15) >= global[g144]) {
					var_2 = *arg_4 - 1;
				} else {
					var_2 = imath_random(1, 2);
					global[g144] = 0;
				}
			} else if (global[g143] == 1) {
				var_2 = 3;
			}
			break;

		case 4:
			var_2 = 3;
			global[g146] = -1;
			break;

		default:
			break;
		}
	}

	if (var_2 >= 0) {
		kernel_reset_animation(arg_0, var_2);
		*arg_4 = var_2;
	}
}

void global_room_init() {
	global[g133] = 1;
	global[g143] = 1;
	global[g132] = -1;
	global[g137] = -1;
	global[g142] = -1;
	global[g147] = -1;
	global[g151] = -1;
	global[g152] = -1;
	global[g153] = -1;
	lets_get_a_move_on_anim = true;
	global[g131] = 0;
	global[g130] = 0;
	global[g135] = 0;
	global[g136] = 0;
	global[g134] = 0;
	global[g141] = 0;
	global[g140] = 0;
	global[g145] = 0;
	global[g146] = 0;
	global[g144] = 0;
	global[g148] = 0;
	global[perform_displacements] = 0;
	digi_flag1 = false;
	digi_flag2 = false;
	digi_val2 = 0;
}

void global_error_code() {
	int16 randVal = imath_random(1, 1000);
	int16 text_id = 0;

	if (player_parse(words_take, 0)) {
		if (player_has(object_named(player_main_noun)) && player.main_object_source != 4) {
			text_id = 25;
		} else {
			if (randVal <= 333)      text_id = 1;
			else if (randVal <= 666) text_id = 2;
			else                     text_id = 3;
		}
	} else if (player_parse(words_push, 0)) {
		text_id = (randVal < 750) ? 4 : 5;
	} else if (player_parse(words_pull, 0)) {
		text_id = (randVal < 750) ? 6 : 7;
	} else if (player_parse(words_open, 0)) {
		if (randVal <= 500)      text_id = 8;
		else if (randVal <= 750) text_id = 9;
		else                     text_id = 10;
	} else if (player_parse(words_close, 0)) {
		if (randVal <= 500)      text_id = 11;
		else if (randVal <= 750) text_id = 12;
		else                     text_id = 13;
	} else if (player_parse(words_put, 0)) {
		if (player_has(object_named(player_main_noun))) {
			text_id = 26;
		} else if (player.main_object_source == 4 && player.second_object_source == 4) {
			text_id = 28;
		} else {
			text_id = (randVal < 500) ? 14 : 15;
		}
	} else if (player_parse(words_talk_to, 0)) {
		text_id = (randVal <= 500) ? 16 : 17;
	} else if (player_parse(words_give, 0)) {
		if (player_has(object_named(player_main_noun))) {
			text_id = 27;
		} else if (player.main_object_source == 4 && player.second_object_source == 4) {
			text_id = 28;
		} else {
			text_id = 18;
		}
	} else if (player_parse(words_throw, 0)) {
		if (player_has(object_named(player_main_noun))) {
			text_id = 19;
		} else {
			text_id = 28;
		}
	} else if (player_parse(words_look, 0)) {
		object_named(player_main_noun);
		if (randVal <= 333)      text_id = 20;
		else if (randVal <= 666) text_id = 21;
		else                     text_id = 22;
	} else if (!player_parse(words_walk_to, 0) && !player_parse(words_walk_down, 0)) {
		text_id = (randVal < 500) ? 23 : 24;
	}

	if (text_id)
		text_show(text_id);
}

void global_midi_play(int num) {
	static const char *NAMES[15] = {
		"adven2", "foolarnd", "homeag", "humorus1", "humorus2", "pianogtr", "raindrop",
		"xad", "xcarey", "xuspens1", "travels1", "xcarey2", "birdsong", "adventur", "action1"
	};

	assert(num >= 1 && num <= 15);
	Common::String name = Common::String::format("*%s.hmi", NAMES[num - 1]);

	midi_play(name.c_str());
}

void global_daemon_code() {
	global[player_selected_object] = -1;

	digi_read_another_chunk();

	if (global[9])
		midi_loop();

	if (section_id != 9) {
		do_interface();
	}
}

void global_anim3(int handle, int16 *frame) {
	if (kernel_anim[handle].frame == *frame)
		return;
	int16 var_2 = -1;
	*frame = (int16)kernel_anim[handle].frame;
	int16 f = *frame;

	switch (f) {
	case 18:
		digi_play_build(301, '_', 1, 2);
		break;
	case 39:
		global[walker_converse_now] = 3;
		break;
	case 40:
		if (global[walker_converse_now] == 3) {
			kernel_timing_trigger(6, 27);
			global[walker_converse_now] = 0;
		}
		var_2 = 39;
		break;
	case 43:
		var_2 = 3;
		break;
	case 44:
		if (global[g007] == 3) {
			if (global[walker_converse_now] == 2) {
				kernel_timing_trigger(6, 27);
				global[walker_converse_now] = 3;
			}
			var_2 = f - 1;
		} else if (global[g007] != 6) {
			var_2 = 45;
		}
		break;
	case 45:
		if (global[g007] == 6) {
			if (global[walker_converse_now] == 2) {
				kernel_timing_trigger(6, 27);
				global[walker_converse_now] = 3;
			}
			var_2 = f - 1;
		}
		break;
	case 46:
		if (global[g007] == 9) {
			if (global[walker_converse_now] == 2) {
				kernel_timing_trigger(6, 27);
				global[walker_converse_now] = 3;
			}
			var_2 = f - 1;
		}
		break;
	case 47:
		if (global[g007] == 1) {
			if (global[walker_converse_now] == 2) {
				kernel_timing_trigger(6, 27);
				global[walker_converse_now] = 3;
			}
			var_2 = f - 1;
		} else if (global[g007] != 4) {
			var_2 = 48;
		}
		break;
	case 48:
		if (global[g007] == 4) {
			if (global[walker_converse_now] == 2) {
				kernel_timing_trigger(6, 27);
				global[walker_converse_now] = 3;
			}
			var_2 = f - 1;
		}
		break;
	case 49:
		if (global[g007] == 7) {
			if (global[walker_converse_now] == 2) {
				kernel_timing_trigger(6, 27);
				global[walker_converse_now] = 3;
			}
			var_2 = f - 1;
		}
		break;
	case 50:
		if (global[g007] == 8) {
			if (global[walker_converse_now] == 2) {
				kernel_timing_trigger(6, 27);
				global[walker_converse_now] = 3;
			}
			var_2 = f - 1;
		}
		break;
	default:
		break;
	}

	if (var_2 >= 0) {
		kernel_reset_animation(handle, var_2);
		*frame = var_2;
	}
}

static void do_looping_sounds() {
	switch (room_id) {
	case 304:
		// do water trickle
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 107);  // PLAY_MORE_TRICKLE
		break;

	case 305:
		// do bird crowd
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 109);  // DO_CROWD
		break;

	case 306:
		if (global[phineas_status] <= PHIN_IS_IN_CONTROL_AGAIN) {
			// bird crowd talking
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 117);  // DO_CROWD
		} else {
			// water flowing
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 118);  // DO_WATER
		}
		break;

	case 401:
		// do dragon noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 106);  // DRAGON_NOISE
		break;

	case 403:
		// do water noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 105);  // WATER
		break;

	case 404:
		// do dragon noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 110);  // DRAGON_NOISE
		break;

	case 405:
		// do dragon noise
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 106);  // DRAGON_NOISE
		break;

	case 210:
		// rush MORE_RUSH_RUSH
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 109);  // MORE_RUSH_RUSH
		break;

	}
}

void global_game_main_loop() {
	static char temp_buf_2[20];
	static char temp_buf_3[20];
	static char temp_buf_4[20];
	int yy;
	long dif;

	// Please play the damn targets
	if (global[play_background_sounds]) {
		// This is for the background sound efx
		dif = kernel.clock - noise_clock;
		if ((dif >= 0) && (dif <= 4)) {
			noise_timer += dif;
		} else {
			noise_timer += 1;
		}

		noise_clock = kernel.clock;

		if (noise_length == -1) {
			if (room_id == 220 || room_id == 221 || room_id == 307 || room_id == 322 || room_id == 420 ||
				room_id == 308 || room_id == 204 || room_id == 211) {
				// Night time
				noise_length = imath_random(50, 80);

			} else {
				// Day time
				noise_length = imath_random(150, 220);
			}
		}

		if (noise_timer >= (noise_length + global[perform_displacements])) {
			if (room_id == 220 || room_id == 221 || room_id == 307 || room_id == 322 || room_id == 420 ||
				room_id == 308 || room_id == 204 || room_id == 211) {
				// Night time
				digi_trigger_effect = false;
				digi_play_build(220, '_', 5, 2);
			} else {
				// Day time

				// Pick a new bird sound. Don't repeat identical sounds
				do {
					yy = imath_random(1, 6);
				} while (yy == last_bird_sound);
				last_bird_sound = (byte)yy;

				digi_trigger_effect = false;

				switch (yy) {
				case 1: digi_play_build(321, '_', 500, 2); break;
				case 2: digi_play_build(321, '_', 501, 2); break;
				case 3: digi_play_build(321, '_', 502, 2); break;
				case 4: digi_play_build(321, '_', 503, 2); break;
				case 5: digi_play_build(321, '_', 504, 2); break;
				case 6: digi_play_build(321, '_', 505, 2); break;
				}
			}

			noise_length = -1;
			noise_timer = 0;
		}
	}

	if (player.walker_visible && player.commands_allowed && section_id != 9 && lets_get_a_move_on_anim &&
		!player.walking && !player.need_to_walk && move_your_butt_anim_handle == -1) {
		move_your_butt_enabled = true;
	} else {
		move_your_butt_enabled = false;
		move_your_butt_timer = 0;
	}

	if (move_your_butt_enabled) {
		// This is for the background sound efx
		dif = kernel.clock - move_your_butt_clock;
		if ((dif >= 0) && (dif <= 4)) {
			move_your_butt_timer += dif;
		} else {
			move_your_butt_timer += 1;
		}
		move_your_butt_clock = kernel.clock;

		if (move_your_butt_timer >= MOVE_YOUR_BUTT_TIMEOUT) {
			player.commands_allowed = false;
			player.walker_visible = false;
			player.clock = kernel.clock;
			move_your_butt_anim_handle = kernel_run_animation("*b_2t", 0);
			extra_change_animation(move_your_butt_anim_handle, player.x, player.y, player.scale, player.depth);
			kernel_synch(KERNEL_ANIM, move_your_butt_anim_handle, KERNEL_NOW, 0);

			switch (player.facing) {
			case 8:
				kernel_reset_animation(move_your_butt_anim_handle, 21);
				break;

			case 9:
				kernel_reset_animation(move_your_butt_anim_handle, 22);
				break;

			case 6:
				kernel_reset_animation(move_your_butt_anim_handle, 23);
				break;

			case 7:
				kernel_reset_animation(move_your_butt_anim_handle, 26);
				break;

			case 4:
				kernel_reset_animation(move_your_butt_anim_handle, 27);
				break;

			default:
				kernel_reset_animation(move_your_butt_anim_handle, 24);
				break;
			}

			do_looping_sounds();
		}
	}

	if (move_your_butt_anim_handle != -1) {
		if (kernel_anim[move_your_butt_anim_handle].frame != move_your_butt_anim_frame) {
			move_your_butt_anim_frame = kernel_anim[move_your_butt_anim_handle].frame;

			switch (move_your_butt_anim_frame) {
			case 24:  // ab in a 2
			case 28:  // ab in a 2
				// AB we need to keep moving, michelle needs those herbs
				digi_trigger_dialog = false;
				digi_play_build_ii('b', 4, 1);
				kernel_reset_animation(move_your_butt_anim_handle, 0);
				move_your_butt_anim_frame = 0;
				break;

			case 20:
				// end of talking
				// end AB we need to keep moving, michelle needs those herbs
				dont_frag_the_palette();
				kernel_abort_animation(move_your_butt_anim_handle);
				move_your_butt_anim_handle = -1;

				move_your_butt_timer = 0;
				player.walker_visible = true;
				player.commands_allowed = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
				player_demand_facing(FACING_SOUTH);

				if (config_file.misc2) {
					do_looping_sounds();
				}
				break;
			}
		}
	}
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
