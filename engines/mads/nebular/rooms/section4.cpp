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
#include "mads/core/global.h"
#include "mads/core/kernel.h"
#include "mads/core/pal.h"
#include "mads/core/player.h"
#include "mads/nebular/rooms/section4.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/global.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

extern void room_401_preload();
extern void room_402_preload();
extern void room_405_preload();
extern void room_406_preload();
extern void room_407_preload();
extern void room_408_preload();
extern void room_409_preload();
extern void room_410_preload();
extern void room_411_preload();
extern void room_413_preload();

void section_4_walker() {
	g_engine->_soundManager->command(5);
	Common::String oldName = player.series_name;

	if ((new_room == 403) || (new_room == 409))
		*player.series_name = '\0';
	else if (global[kSexOfRex] == REX_FEMALE)
		Common::strcpy_s(player.series_name, "ROX");
	else
		Common::strcpy_s(player.series_name, "RXM");

	player.scaling_velocity = true;

	if (oldName != Common::String(player.series_name))
		player.walker_must_reload = true;

	pal_change_color(16, 10, 63, 63);
	pal_change_color(17, 10, 45, 45);
}

void section_4_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(4));
}

void section_4_init() {
	player.scaling_velocity = true;
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
	case 401: room_preload_code_pointer = room_401_preload; break;
	case 402: room_preload_code_pointer = room_402_preload; break;
	case 405: room_preload_code_pointer = room_405_preload; break;
	case 406: room_preload_code_pointer = room_406_preload; break;
	case 407: room_preload_code_pointer = room_407_preload; break;
	case 408: room_preload_code_pointer = room_408_preload; break;
	case 409: room_preload_code_pointer = room_409_preload; break;
	case 410: room_preload_code_pointer = room_410_preload; break;
	case 411: room_preload_code_pointer = room_411_preload; break;
	case 413: room_preload_code_pointer = room_413_preload; break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_4_music() {
	if (!config_file.music_flag) {
		g_engine->_soundManager->command(2);
		return;
	}

	switch (new_room) {
	case 401:
		g_engine->_soundManager->startQueuedCommands();
		if (previous_room == 402)
			g_engine->_soundManager->command(12, 64);
		else
			g_engine->_soundManager->command(12, 1);
		break;

	case 402:
		g_engine->_soundManager->startQueuedCommands();
		g_engine->_soundManager->command(12, 127);
		break;

	case 405:
	case 407:
	case 409:
	case 410:
	case 413:
		g_engine->_soundManager->command(10);
		break;

	case 408:
		g_engine->_soundManager->command(52);
		break;

	default:
		break;
	}
}

void section_4_preload() {
	section_init_code_pointer = section_4_init;
	section_room_constructor = section_4_constructor;
	section_music_reset_pointer = section_4_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
