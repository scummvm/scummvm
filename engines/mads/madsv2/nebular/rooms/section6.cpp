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
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/nebular/rooms/section6.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/global.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

extern void room_601_preload();
extern void room_602_preload();
extern void room_603_preload();
extern void room_604_preload();
extern void room_605_preload();
extern void room_607_preload();
extern void room_608_preload();
extern void room_609_preload();
extern void room_610_preload();
extern void room_611_preload();
extern void room_612_preload();
extern void room_620_preload();

void section_6_walker() {
	g_engine->_soundManager->command(5);
	Common::String oldName = player.series_name;

	if (global[kSexOfRex] == REX_FEMALE)
		Common::strcpy_s(player.series_name, "ROX");
	else
		Common::strcpy_s(player.series_name, "RXM");

	if (new_room == 605 || new_room == 620)
		*player.series_name = '\0';

	if (oldName != Common::String(player.series_name))
		player.walker_must_reload = true;

	pal_change_color(16, 10, 63, 63);
	pal_change_color(17, 10, 45, 45);
}

void section_6_interface() {
	Common::strcpy_s(kernel.interface, kernel_interface_name(5));
}

void section_6_init() {
	player.scaling_velocity = true;
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
	case 601: room_preload_code_pointer = room_601_preload; break;
	case 602: room_preload_code_pointer = room_602_preload; break;
	case 603: room_preload_code_pointer = room_603_preload; break;
	case 604: room_preload_code_pointer = room_604_preload; break;
	case 605: room_preload_code_pointer = room_605_preload; break;
	case 607: room_preload_code_pointer = room_607_preload; break;
	case 608: room_preload_code_pointer = room_608_preload; break;
	case 609: room_preload_code_pointer = room_609_preload; break;
	case 610: room_preload_code_pointer = room_610_preload; break;
	case 611: room_preload_code_pointer = room_611_preload; break;
	case 612: room_preload_code_pointer = room_612_preload; break;
	case 620: room_preload_code_pointer = room_620_preload; break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_6_music() {
	if (!config_file.music_flag) {
		g_engine->_soundManager->command(2);
		return;
	}

	switch (new_room) {
	case 601:
	case 602:
	case 603:
	case 604:
	case 605:
	case 607:
	case 608:
	case 609:
	case 610:
	case 612:
	case 620:
		g_engine->_soundManager->command(29);
		break;
	case 611:
		g_engine->_soundManager->command(24);
		break;
	default:
		break;
	}
}

void section_6_preload() {
	section_init_code_pointer = section_6_init;
	section_room_constructor = section_6_constructor;
	section_music_reset_pointer = section_6_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
