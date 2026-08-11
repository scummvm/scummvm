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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/global.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

extern void room_701_preload();
extern void room_702_preload();
extern void room_703_preload();
extern void room_704_preload();
extern void room_705_preload();
extern void room_706_preload();
extern void room_707_preload();
extern void room_710_preload();
extern void room_711_preload();
extern void room_751_preload();
extern void room_752_preload();

void section_7_walker() {
	g_engine->_soundManager->command(5);

	Common::String oldName = player.series_name;

	if ((new_room == 703) || (new_room == 704) || (new_room == 705)
	 || (new_room == 707) || (new_room == 710) || (new_room == 711))
		*player.series_name = '\0';
	else if (global[kSexOfRex] == REX_MALE)
		Common::strcpy_s(player.series_name, "RXM");
	else
		Common::strcpy_s(player.series_name, "ROX");

	player.scaling_velocity = true;

	if (oldName != Common::String(player.series_name))
		player.walker_must_reload = true;

	pal_change_color(16, 10, 63, 63);
	pal_change_color(17, 10, 45, 45);
}

void section_7_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(5));
}

void section_7_init() {
	player.scaling_velocity = true;
}

void section_7_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 701: room_preload_code_pointer = room_701_preload; break;
	case 702: room_preload_code_pointer = room_702_preload; break;
	case 703: room_preload_code_pointer = room_703_preload; break;
	case 704: room_preload_code_pointer = room_704_preload; break;
	case 705: room_preload_code_pointer = room_705_preload; break;
	case 706: room_preload_code_pointer = room_706_preload; break;
	case 707: room_preload_code_pointer = room_707_preload; break;
	case 710: room_preload_code_pointer = room_710_preload; break;
	case 711: room_preload_code_pointer = room_711_preload; break;
	case 751: room_preload_code_pointer = room_751_preload; break;
	case 752: room_preload_code_pointer = room_752_preload; break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_7_music() {
	if (!config_file.music_flag) {
		g_engine->_soundManager->command(2);
		return;
	}

	switch (new_room) {
	case 701:
	case 702:
	case 704:
	case 705:
	case 751:
		g_engine->_soundManager->command(38);
		break;
	case 703:
		if (global[kMonsterAlive] == 0)
			g_engine->_soundManager->command(24);
		else
			g_engine->_soundManager->command(27);
		break;
	case 706:
	case 707:
	case 710:
	case 711:
		g_engine->_soundManager->command(25);
		break;
	default:
		break;
	}
}

void section_7_preload() {
	section_init_code_pointer = section_7_init;
	section_room_constructor = section_7_constructor;
	section_music_reset_pointer = section_7_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
