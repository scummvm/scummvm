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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

extern void room_401_preload();
extern void room_402_preload();
extern void room_403_preload();
extern void room_404_preload();
extern void room_405_preload();
extern void room_406_preload();
extern void room_407_preload();
extern void room_408_preload();
extern void room_409_preload();
extern void room_410_preload();
extern void room_411_preload();
extern void room_412_preload();
extern void room_454_preload();

void section_4_init() {
	player.scaling_velocity = true;
}

void section_4_walker() {
}

void section_4_interface() {
}

void section_4_music() {
}

void section_4_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 401:
		room_preload_code_pointer = room_401_preload;
		break;
	case 402:
		room_preload_code_pointer = room_402_preload;
		break;
	case 403:
		room_preload_code_pointer = room_403_preload;
		break;
	case 404:
		room_preload_code_pointer = room_404_preload;
		break;
	case 405:
		room_preload_code_pointer = room_405_preload;
		break;
	case 406:
		room_preload_code_pointer = room_406_preload;
		break;
	case 407:
		room_preload_code_pointer = room_407_preload;
		break;
	case 408:
		room_preload_code_pointer = room_408_preload;
		break;
	case 409:
		room_preload_code_pointer = room_409_preload;
		break;
	case 410:
		room_preload_code_pointer = room_410_preload;
		break;
	case 411:
		room_preload_code_pointer = room_411_preload;
		break;
	case 412:
		room_preload_code_pointer = room_412_preload;
		break;
	case 454:
		room_preload_code_pointer = room_454_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_4_preload() {
	section_init_code_pointer = section_4_init;
	section_room_constructor = section_4_constructor;
	section_music_reset_pointer = section_4_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
