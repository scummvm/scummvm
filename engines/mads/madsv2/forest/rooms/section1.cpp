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
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/rooms/section1.h"
#include "mads/madsv2/forest/mads/sounds.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

extern void room_101_preload();
extern void room_103_preload();
extern void room_104_preload();
extern void room_106_preload();
extern void room_107_preload();
extern void room_199_preload();

void section_1_init() {
}

void section_1_music() {
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
	case 103:
		room_preload_code_pointer = room_103_preload;
		break;
	case 104:
		room_preload_code_pointer = room_104_preload;
		break;
	case 106:
		room_preload_code_pointer = room_106_preload;
		break;
	case 107:
		room_preload_code_pointer = room_107_preload;
		break;
	case 199:
		room_preload_code_pointer = room_199_preload;
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
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
