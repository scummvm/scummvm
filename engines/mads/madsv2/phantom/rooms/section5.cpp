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
#include "mads/madsv2/phantom/rooms/section5.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

extern void room_501_preload();
extern void room_502_preload();
extern void room_504_preload();
extern void room_505_preload();
extern void room_506_preload();


void section_5_walker() {
	char temp_buf[80];

	sound_queue(N_NoiseFade);

	Common::strcpy_s(temp_buf, player.series_name);

	if (!player.force_series)
		Common::strcpy_s(player.series_name, "RAL");

	if (strcmp(temp_buf, player.series_name) != 0)
		player.walker_must_reload = true;

	player.scaling_velocity = true;
}

void section_5_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(1));

	pal_change_color(INTER_MESSAGE_COLOR, 43, 47, 51);
}

void section_5_music() {
	if (sound_off) {
		sound_play(N_NoiseFade);
	}

	if (music_off) {
		sound_play(N_MusicFade);
		goto done;
	}

	if (global[coffin_status] == COFFIN_OPEN && !player_has_been_in_room(506) &&
		global[fight_status] == FIGHT_NOT_HAPPENED && room_id == 504) {
		sound_play(N_FightMusic504);
		/* we are in the fight at some stage */

	} else if (room_id == 505) {
		if (conv_restore_running == 20) { /* running coffin conv */
			sound_play(N_AngelMus505);

		} else {
			sound_play(N_BackgroundMus);
		}

	} else {
		sound_play(N_BackgroundMus);
	}

done:
	;
}

void section_5_init() {
	player.scaling_velocity = true;
}

void section_5_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 501:
		room_preload_code_pointer = room_501_preload;
		break;

	case 502:
		room_preload_code_pointer = room_502_preload;
		break;

	case 504:
		room_preload_code_pointer = room_504_preload;
		break;

	case 505:
		room_preload_code_pointer = room_505_preload;
		break;

	case 506:
		room_preload_code_pointer = room_506_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_5_preload() {
	section_init_code_pointer = section_5_init;
	section_room_constructor = section_5_constructor;
	section_music_reset_pointer = section_5_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
