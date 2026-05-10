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

#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/rooms/section1.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

extern void room_101_preload();
extern void room_102_preload();
extern void room_103_preload();
extern void room_104_preload();
extern void room_105_preload();
extern void room_106_preload();
extern void room_107_preload();
extern void room_108_preload();
extern void room_109_preload();
extern void room_110_preload();
extern void room_111_preload();
extern void room_112_preload();
extern void room_113_preload();
extern void room_114_preload();
extern void room_150_preload();


void section_1_walker() {
	char temp_buf[80];

	sound_queue(N_NoiseFade);

	Common::strcpy_s(temp_buf, player.series_name);

	if (!player.force_series)
		Common::strcpy_s(player.series_name, "RAL");

	if (strcmp(temp_buf, player.series_name) != 0)
		player.walker_must_reload = true;

	player.scaling_velocity = true;
}

void section_1_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(global[temp_interface]));

	pal_change_color(INTER_MESSAGE_COLOR, 43, 47, 51);
}

void section_1_music() {
	if (sound_off) {
		sound_play(N_NoiseFade);
	}

	if (music_off) {
		sound_play(N_MusicFade);
		goto done;
	}

	if (global[done_brie_conv_203] == YES_AND_CHASE) {
		sound_play(N_ChaseMusic001);

	} else {
		switch (new_room) {
		case 101:
			if (!player.been_here_before) {
				sound_play(N_BackMus1stTime);

			} else {
				sound_play(N_BackgroundMus);
			}
			break;

		case 111:
			if (previous_room == 150) {
				sound_play(N_BackMus1stTime);

			} else if (global[leave_angel_music_on]) {
				sound_play(N_AngelMus001);

			} else {
				sound_play(N_BackgroundMus);
			}
			break;

		case 113:
			if (global[leave_angel_music_on]) {
				sound_play(N_AngelMus001);

			} else {
				if (global[current_year] == 1993) {
					sound_play(N_Christine1993);

				} else {
					sound_play(N_Christine1881_001);
				}
			}
			break;

		case 103:
			if ((global[jacques_status] == JACQUES_IS_DEAD_RICH_GONE)) {
				sound_play(N_JacquesDeadMus);
				global[jacques_status] = JAC_DEAD_RICH_GONE_SEEN_BODY;

			} else {
				sound_play(N_BackgroundMus);
			}
			break;

		case 102:
			if (previous_room == 104) {
				sound_play(N_PlayerDies);

			} else {
				sound_play(N_BackgroundMus);
			}
			break;

		case 104:
			if ((conv_restore_running == 7) || (previous_room == 301)) {
				sound_play(N_1881Music);

			} else if ((global[room_103_104_transition] == PEEK_THROUGH) &&
				(!global[observed_phan_104])) {
				sound_play(N_PhantomAppears001);
				global[observed_phan_104] = true;

			} else {
				sound_play(N_BackgroundMus);
			}
			break;

		default:
			if ((previous_room != 204) && (new_room != 150)) {
				sound_play(N_BackgroundMus);
			}
			break;
		}
	}

done:
	;
}

void section_1_init() {
	player.scaling_velocity = true;
}

void section_1_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 101:
		room_preload_code_pointer = room_101_preload;
		break;
	case 102:
		room_preload_code_pointer = room_102_preload;
		break;
	case 103:
		room_preload_code_pointer = room_103_preload;
		break;
	case 104:
		room_preload_code_pointer = room_104_preload;
		break;
	case 105:
		room_preload_code_pointer = room_105_preload;
		break;
	case 106:
		room_preload_code_pointer = room_106_preload;
		break;
	case 107:
		room_preload_code_pointer = room_107_preload;
		break;
	case 108:
		room_preload_code_pointer = room_108_preload;
		break;
	case 109:
		room_preload_code_pointer = room_109_preload;
		break;
	case 110:
		room_preload_code_pointer = room_110_preload;
		break;
	case 111:
		room_preload_code_pointer = room_111_preload;
		break;
	case 112:
		room_preload_code_pointer = room_112_preload;
		break;
	case 113:
		room_preload_code_pointer = room_113_preload;
		break;
	case 114:
		room_preload_code_pointer = room_114_preload;
		break;
	case 150:
		room_preload_code_pointer = room_150_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_1_preload() {
	section_init_code_pointer = section_1_init;
	section_room_constructor = section_1_constructor;
	section_music_reset_pointer = section_1_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
