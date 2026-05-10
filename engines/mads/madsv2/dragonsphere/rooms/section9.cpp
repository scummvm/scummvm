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
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section9.h"
#include "mads/madsv2/dragonsphere/dragonsphere.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

extern void room_909_preload();

void section_9_init() {
	player.scaling_velocity = true;
}

void section_9_walker() {
	sound_queue(5);
	global[perform_displacements] = 0;
	*player.series_name = '\0';
	player.scaling_velocity = -1;
}

void section_9_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(7));
	pal_change_color(47, 56, 254, 32);
}

void section_9_music() {
	if (!g_engine->_soundFlag)
		sound_queue(4);

	if (!g_engine->_musicFlag)
		sound_queue(3);
	else if (new_room == 909)
		sound_play(62);
}

void section_9_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	if (new_room == 909) {
		room_preload_code_pointer = room_909_preload;
	}

	room_himem_preload(new_room, SECTION);
}

void section_9_preload() {
	section_init_code_pointer = section_9_init;
	section_room_constructor = section_9_constructor;
	section_music_reset_pointer = section_9_music;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
