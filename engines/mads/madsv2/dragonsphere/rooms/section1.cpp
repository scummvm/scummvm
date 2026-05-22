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
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
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
extern void room_115_preload();
extern void room_116_preload();
extern void room_117_preload();
extern void room_118_preload();
extern void room_119_preload();


void section_1_init() {
	player.scaling_velocity = true;
}

void section_1_walker() {
	char temp_buf[80];
	int dark_background = false;
	int no_walker = false;

	sound_queue(N_NoiseFade);

	Common::strcpy_s(temp_buf, player.series_name);

	global[perform_displacements] = true;

	if (new_room == 111 ||
		new_room == 112 ||
		new_room == 117 ||
		new_room == 120 ||
		new_room == 119) {
		no_walker = true;
	}

	if (new_room == 106 && room_id == 120) {
		no_walker = true;
	}

	if (global[player_persona] == PLAYER_IS_KING) {
		if (new_room == 113) {
			no_walker = true;
		}
	}

	switch (new_room) {
	case 108:
	case 109:
	case 113:
	case 114:
	case 115:
		dark_background = true;
		break;
	}

	if (no_walker || global[no_load_walker]) {
		player.series_name[0] = 0;

	} else if (!player.force_series) {
		if (global[player_persona] == PLAYER_IS_KING || new_room == 108 || new_room == 109) {
			if (new_room == 113 || new_room == 114 || new_room == 115 || new_room == 116) {
				Common::strcpy_s(player.series_name, "PD");
			} else {
				Common::strcpy_s(player.series_name, "KG");
			}
		} else {
			Common::strcpy_s(player.series_name, "PD");
		}
		if (dark_background)
			Common::strcat_s(player.series_name, "D");
	}

	if (strcmp(temp_buf, player.series_name) != 0)
		player.walker_must_reload = true;

	player.scaling_velocity = true;
}

void section_1_interface() {
	int interface;
	RGBcolor text_color = { 43, 29, 15 };

	if (new_room == 116) {
		interface = 2;

	} else if (new_room == 110) {
		interface = 5;

	} else if (new_room == 120) {
		interface = 8;

	} else if ((new_room >= 113) && (new_room <= 119) && (new_room != 118)) {
		interface = 1;

	} else if ((new_room == 108) || (new_room == 109)) {
		interface = 3;

	} else {
		interface = 0;
	}

	Common::strcpy_s(kernel.interface, kernel_interface_name(interface));

	pal_change_color(INTER_MESSAGE_COLOR, 56, 47, 32);
}

void section_1_music() {
	if (sound_off) {
		sound_play(N_NoiseOff);
	}

	if (music_off) {
		sound_play(N_MusicFade);
		goto done;
	}

	switch (new_room) {
	case 104:
		if (global[player_persona] == PLAYER_IS_PID) {
			sound_play(N_SwordFightMus);
		} else {
			sound_play(N_BackgroundMus);
		}
		break;

	case 106:
		if (global[end_of_game]) {
			sound_play(N_RoyalDecreeMus);
		} else {
			sound_play(N_BackgroundMus);
		}
		break;

	case 108:
		if (player_has_been_in_room(109)) {
			sound_play(N_DungeonMus);
		} else {
			sound_play(N_DungeonIntro);
		}
		break;

	case 109:
		sound_play(N_DungeonMus);
		break;

	case 110:
		sound_play(N_WayStationMus);
		break;

	case 111:
		sound_play(N_OminousMus);
		break;

	case 112:
		sound_play(N_Bk112Mus);
		break;

	case 113:
		sound_play(N_NoiseFade);
		if (global[player_is_seal]) {
			sound_play(N_SealMus);
		} else {
			sound_play(N_UnderGroundMus);
		}
		break;

	case 114:
		sound_play(N_UnderGroundMus);
		break;

	case 115:
		sound_play(N_Bk115Mus);
		break;

	case 116:
		sound_play(N_Bk116Mus);
		break;

	case 117:
		sound_play(N_SealMus);
		break;

	case 119:
		sound_play(N_Bk119Mus);
		break;

	case 120:
		sound_play(N_WalkingMus);
		break;

	default:
		sound_play(N_BackgroundMus);
		break;
	}

done:
	;
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
	case 115:
		room_preload_code_pointer = room_115_preload;
		break;
	case 116:
		room_preload_code_pointer = room_116_preload;
		break;
	case 117:
		room_preload_code_pointer = room_117_preload;
		break;
	case 118:
		room_preload_code_pointer = room_118_preload;
		break;
	case 119:
		room_preload_code_pointer = room_119_preload;
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
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
