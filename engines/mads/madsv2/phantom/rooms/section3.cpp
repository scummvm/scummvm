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

extern void room_301_preload();
extern void room_302_preload();
extern void room_303_preload();
extern void room_304_preload();
extern void room_305_preload();
extern void room_306_preload();
extern void room_307_preload();
extern void room_308_preload();
extern void room_309_preload();
extern void room_310_preload();


void section_3_walker() {
	char temp_buf[80];

	sound_play(N_NoiseFade);

	if ((new_room == 304) || (new_room == 305) || (new_room == 306) || (new_room == 310)) {
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

void section_3_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(global[temp_interface]));

	pal_change_color(INTER_MESSAGE_COLOR, 43, 47, 51);
}

void section_3_music() {
	if (sound_off) {
		sound_play(N_NoiseFade);
	}

	if (music_off) {
		sound_play(N_MusicFade);
		goto done;
	}

	switch (new_room) {
	case 303:
	case 304:
	case 305:
	case 307:
	case 308:
		if (global[knocked_over_head]) {
			sound_play(N_FightMusic304);
		} else {
			sound_play(N_BackgroundMus);
		}
		break;

	case 310:
	case 320:
	case 330:
	case 340:
		sound_play(N_LakeMusic);
		break;

	default:
		if (new_room != 306) {
			sound_play(N_BackgroundMus);
		}
		break;
	}

done:
	;
}

void section_3_init() {
	player.scaling_velocity = true;
}

void section_3_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 301:
		room_preload_code_pointer = room_301_preload;
		break;

	case 302:
		room_preload_code_pointer = room_302_preload;
		break;

	case 303:
		room_preload_code_pointer = room_303_preload;
		break;

	case 304:
		room_preload_code_pointer = room_304_preload;
		break;

	case 305:
		room_preload_code_pointer = room_305_preload;
		break;

	case 306:
		room_preload_code_pointer = room_306_preload;
		break;

	case 307:
		room_preload_code_pointer = room_307_preload;
		break;

	case 308:
		room_preload_code_pointer = room_308_preload;
		break;

	case 309:
		room_preload_code_pointer = room_309_preload;
		break;

	case 310:
		room_preload_code_pointer = room_310_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_3_preload() {
	section_init_code_pointer = section_3_init;
	section_room_constructor = section_3_constructor;
	section_music_reset_pointer = section_3_music;
	section_parser_code_pointer = NULL;
	section_daemon_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
