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
#include "mads/madsv2/dragonsphere/rooms/section2.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

extern void room_201_preload();
extern void room_203_preload();
extern void room_204_preload();
extern void room_205_preload();
extern void room_206_preload();


void section_2_init() {
	player.scaling_velocity = true;
}

void section_2_walker() {
	char temp_buf[80];
	int dark_background = false;
	int no_walker = false;

	sound_queue(N_NoiseFade);

	Common::strcpy_s(temp_buf, player.series_name);

	if (new_room == 204) {
		dark_background = true;
	}

	if (no_walker || global[no_load_walker]) {
		player.series_name[0] = 0;
	} else if (!player.force_series) {
		if (global[player_persona] == PLAYER_IS_KING) {
			Common::strcpy_s(player.series_name, "KG");
		} else {
			Common::strcpy_s(player.series_name, "PD");
		}
		if (dark_background) Common::strcat_s(player.series_name, "D");
	}

	if (strcmp(temp_buf, player.series_name) != 0) player.walker_must_reload = true;

	player.scaling_velocity = true;
}

void section_2_interface() {
	int interface = 9;
	RGBcolor text_color = { 43, 29, 15 };

	Common::strcpy_s(kernel.interface, kernel_interface_name(interface));

	pal_change_color(INTER_MESSAGE_COLOR, 56, 47, 32);
}

void section_2_music() {
	if (sound_off) {
		sound_queue(N_NoiseOff);
	}

	if (music_off) {
		sound_queue(N_MusicFade);
		goto done;
	}

	switch (new_room) {
	case 201:
		sound_play(N_BackgroundMus);
		break;

	case 203:
		sound_play(N_Bk203Music);
		break;

	case 204:
		sound_play(N_DreamMusic);
		break;

	case 202:
	case 205:
	case 206:
		sound_play(N_SlathanMus);
		break;

	default:
		sound_play(N_BackgroundMus);
		break;
	}

done:
	;
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
	}

	room_himem_preload(new_room, SECTION);
}

void section_2_preload() {
	section_init_code_pointer = section_2_init;
	section_room_constructor = section_2_constructor;
	section_music_reset_pointer = section_2_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
