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
#include "mads/forest/rooms/section3.h"
#include "mads/forest/mads/sounds.h"

namespace MADS {
namespace Forest {
namespace Rooms {

extern void room_301_preload();
extern void room_302_preload();
extern void room_303_preload();
extern void room_304_preload();
extern void room_305_preload();
extern void room_306_preload();
extern void room_307_preload();
extern void room_308_preload();
extern void room_321_preload();
extern void room_322_preload();

void section_3_init() {
}

void section_3_music() {
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
	case 321:
		room_preload_code_pointer = room_321_preload;
		break;
	case 322:
		room_preload_code_pointer = room_322_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_3_preload() {
	section_init_code_pointer = section_3_init;
	section_room_constructor = section_3_constructor;
	section_music_reset_pointer = section_3_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADS
