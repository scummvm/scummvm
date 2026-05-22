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
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

extern void room_601_preload();
extern void room_603_preload();
extern void room_604_preload();
extern void room_605_preload();
extern void room_606_preload();
extern void room_607_preload();
extern void room_609_preload();
extern void room_612_preload();
extern void room_613_preload();
extern void room_614_preload();

void section_6_init() {
	player.scaling_velocity = true;
}

void section_6_walker() {
	char temp_buf[80];
	int dark_background = true;
	int no_walker = false;

	sound_queue(N_NoiseFade);

	Common::strcpy_s(temp_buf, player.series_name);

	if (new_room == 603 || new_room == 601) {
		dark_background = false;
	}

	if (no_walker || global[no_load_walker]) {
		player.series_name[0] = 0;
	} else if (!player.force_series) {
		if (global[player_persona] == PLAYER_IS_KING) {
			Common::strcpy_s(player.series_name, "KG");
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

void section_6_interface() {
	RGBcolor text_color = { 43, 29, 15 };

	Common::strcpy_s(kernel.interface, kernel_interface_name(6));
	pal_change_color(INTER_MESSAGE_COLOR, 56, 47, 32);
}

void section_6_music() {
	if (sound_off) {
		sound_queue(N_NoiseOff);
	}

	if (music_off) {
		sound_queue(N_MusicFade);
		goto done;
	}

	switch (new_room) {
	case 601:
		if (global[vines_have_player]) {
			sound_play(N_EerieSounds);
		} else {
			sound_play(N_BackgroundMus);
		}
		break;

	case 603:
		sound_play(N_Bk603Music);
		break;

	case 604:
		sound_play(N_Bk604Music);
		break;

	case 605:
		if (global[rope_is_alive]) {
			sound_play(N_Bk605WithRope);
		} else {
			sound_play(N_Bk605Music);
		}
		break;

	case 606:
	case 607:
		sound_play(N_Bk606Music);
		break;

	case 609:
		sound_play(N_Bk609Music);
		break;

	case 612:
		sound_play(N_Bk612Music);
		break;

	case 613:
		sound_play(N_WaterFlows);
		break;

	case 614:
		if (global[wizard_dead]) {
			sound_play(N_BackgroundMus);
		} else {
			sound_play(N_Bk614Music);
		}
		break;

	default:
		sound_play(N_BackgroundMus);
		break;
	}

done:
	;
}

void section_6_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 601:
		room_preload_code_pointer = room_601_preload;
		break;
	case 603:
		room_preload_code_pointer = room_603_preload;
		break;
	case 604:
		room_preload_code_pointer = room_604_preload;
		break;
	case 605:
		room_preload_code_pointer = room_605_preload;
		break;
	case 606:
		room_preload_code_pointer = room_606_preload;
		break;
	case 607:
		room_preload_code_pointer = room_607_preload;
		break;
	case 609:
		room_preload_code_pointer = room_609_preload;
		break;
	case 612:
		room_preload_code_pointer = room_612_preload;
		break;
	case 613:
		room_preload_code_pointer = room_613_preload;
		break;
	case 614:
		room_preload_code_pointer = room_614_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_6_preload() {
	section_init_code_pointer = section_6_init;
	section_room_constructor = section_6_constructor;
	section_music_reset_pointer = section_6_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
