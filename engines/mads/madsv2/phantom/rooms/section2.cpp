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
#include "mads/madsv2/phantom/rooms/section2.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

extern void room_201_preload();
extern void room_202_preload();
extern void room_203_preload();
extern void room_204_preload();
extern void room_205_preload();
extern void room_206_preload();
extern void room_207_preload();
extern void room_208_preload();
extern void room_250_preload();


void section_2_walker() {
	char temp_buf[80];

	sound_queue(N_NoiseFade);

	if (new_room == 208) {
		player.series_name[0] = 0;

	} else {
		Common::strcpy_s(temp_buf, player.series_name);
		if (!player.force_series)
			Common::strcpy_s(player.series_name, "RAL");
		if (strcmp(temp_buf, player.series_name) != 0)
			player.walker_must_reload = true;
	}

	player.scaling_velocity = true;
}



void section_2_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(global[temp_interface]));

	pal_change_color(INTER_MESSAGE_COLOR, 43, 47, 51);
}

void section_2_music() {
	if (sound_off) {
		sound_play(N_NoiseFade);
	}

	if (music_off) {
		sound_play(N_MusicFade);
		goto done;
	}

	switch (new_room) {
	case 208:
		sound_play(N_IsabelWedding);
		break;

	case 206:
		if (!global[knocked_over_head]) {
			sound_play(N_BackgroundMus);
		}
		break;

	default:
		if (new_room != 250) {
			sound_play(N_BackgroundMus);
		}
		break;
	}

done:
	;
}

void section_2_init() {
	player.scaling_velocity = true;
}

void section_2_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 201:
		room_preload_code_pointer = room_201_preload;
		break;
	case 202:
		room_preload_code_pointer = room_202_preload;
		break;
	case 203:
		room_preload_code_pointer = room_203_preload;
		break;
	case 204:
		room_preload_code_pointer = room_204_preload;
		break;
	case 205:
		room_preload_code_pointer = room_205_preload;
		break;
	case 206:
		room_preload_code_pointer = room_206_preload;
		break;
	case 207:
		room_preload_code_pointer = room_207_preload;
		break;
	case 208:
		room_preload_code_pointer = room_208_preload;
		break;
	case 250:
		room_preload_code_pointer = room_250_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_2_preload() {
	section_init_code_pointer = section_2_init;
	section_room_constructor = section_2_constructor;
	section_music_reset_pointer = section_2_music;
	section_parser_code_pointer = NULL;
	section_daemon_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
