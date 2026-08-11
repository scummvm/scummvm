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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/pal.h"
#include "mads/core/player.h"
#include "mads/core/room.h"
#include "mads/core/sound.h"
#include "mads/forest/global.h"
#include "mads/forest/rooms/section9.h"
#include "mads/forest/forest.h"

namespace MADS {
namespace Forest {
namespace Rooms {

extern void room_901_preload();
extern void room_903_preload();
extern void room_904_preload();

void section_9_init() {
}

void section_9_walker() {
	char temp_buf[80];
	Common::strcpy_s(temp_buf, player.series_name);
	global[g017] = true;

	if (!global[g016]) {
		*player.series_name = '\0';
	} else if (!player.force_series) {
		Common::strcpy_s(player.series_name, "B");
	}

	if (strcmp(temp_buf, player.series_name) != 0)
		player.walker_must_reload = true;
	player.scaling_velocity = true;
}

void section_9_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(0));
	pal_change_color(254, 56, 47, 32);
}

void section_9_music() {
}

void section_9_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 901:
		room_preload_code_pointer = room_901_preload;
		break;
	case 903:
		room_preload_code_pointer = room_903_preload;
		break;
	case 904:
		room_preload_code_pointer = room_904_preload;
		break;
	default:
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_9_preload() {
	section_init_code_pointer = section_9_init;
	section_room_constructor = section_9_constructor;
	section_music_reset_pointer = section_9_music;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADS
