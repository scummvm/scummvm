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
#include "mads/madsv2/dragonsphere/rooms/section5.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

extern void room_501_preload();
extern void room_502_preload();
extern void room_503_preload();
extern void room_504_preload();
extern void room_505_preload();
extern void room_506_preload();
extern void room_507_preload();
extern void room_508_preload();
extern void room_509_preload();
extern void room_510_preload();
extern void room_511_preload();
extern void room_512_preload();
extern void room_557_preload();

void section_5_init() {
	player.scaling_velocity = true;
}

void section_5_walker() {
}

void section_5_interface() {
}

void section_5_music() {
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
	case 503:
		room_preload_code_pointer = room_503_preload;
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
	case 507:
		room_preload_code_pointer = room_507_preload;
		break;
	case 508:
		room_preload_code_pointer = room_508_preload;
		break;
	case 509:
		room_preload_code_pointer = room_509_preload;
		break;
	case 510:
		room_preload_code_pointer = room_510_preload;
		break;
	case 511:
		room_preload_code_pointer = room_511_preload;
		break;
	case 512:
		room_preload_code_pointer = room_512_preload;
		break;
	case 557:
		room_preload_code_pointer = room_557_preload;
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
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
