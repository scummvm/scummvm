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

#include "math/utils.h"
#include "mads/core/config.h"
#include "mads/core/pal.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/sound/mac_sound.h"
#include "mads/nebular/global.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

extern void room_101_preload();
extern void room_102_preload();
extern void room_103_preload();
extern void room_104_preload();
extern void room_105_preload();
extern void room_106_preload();
extern void room_107_preload();
extern void room_108_preload();
extern void room_109_preload();
extern void room_110_preload();
extern void room_111_preload();
extern void room_112_preload();

void section_1_walker() {
	g_engine->_soundManager->command(5, 0);
	Common::String oldName = (&player.series_name[0]);
	if (new_room <= 103 || new_room == 111) {
		if (global[kSexOfRex] == SEX_FEMALE)
			Common::strcpy_s(player.series_name, "ROX");
		else {
			Common::strcpy_s(player.series_name, "RXM");
			global[kSexOfRex] = SEX_MALE;
		}
	} else if (new_room <= 110) {
		Common::strcpy_s(player.series_name, "RXSW");
		global[kSexOfRex] = SEX_UNKNOWN;
	} else if (new_room == 112)
		Common::strcpy_s(player.series_name, "");

	if (oldName != (&player.series_name[0]))
		player.walker_must_reload = true;

	if (new_room == 105 || (new_room == 109 && global[kHoovicAlive])) {
		player.walker_must_reload = true;
		player.walker_loads_first = false;
	}

	player.scaling_velocity = 0;
	pal_change_color(16, 10, 63, 63);
	pal_change_color(17, 10, 45, 45);
}

void section_1_interface() {
	int idx = (new_room > 103 && new_room < 112) ? 1 : 0;
	Common::strcpy_s(kernel.interface, kernel_interface_name(idx));
}

void section_1_init() {
	player.scaling_velocity = true;
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
	case 102:
		room_preload_code_pointer = room_102_preload;
		break;
	case 103:
		room_preload_code_pointer = room_103_preload;
		break;
	case 104:
		room_preload_code_pointer = room_104_preload;
		break;
	case 105:
		room_preload_code_pointer = room_105_preload;
		break;
	case 106:
		room_preload_code_pointer = room_106_preload;
		break;
	case 107:
		room_preload_code_pointer = room_107_preload;
		break;
	case 108:
		room_preload_code_pointer = room_108_preload;
		break;
	case 109:
		room_preload_code_pointer = room_109_preload;
		break;
	case 110:
		room_preload_code_pointer = room_110_preload;
		break;
	case 111:
		room_preload_code_pointer = room_111_preload;
		break;
	case 112:
		room_preload_code_pointer = room_112_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_1_music() {
	if (config_file.music_flag) {
		switch (new_room) {
		case 101:
			if (!Sound::commandMacintoshSound(Sound::kMacSoundPlayPriority,
					1011, 0, 0, 0, true))
				g_engine->_soundManager->command(11, 0);
			break;
		case 102:
			g_engine->_soundManager->command(12, 0);
			break;
		case 103:
			g_engine->_soundManager->command(3, 0);
			g_engine->_soundManager->command(25, 0);
			break;
		case 109:
			g_engine->_soundManager->command(13, 0);
			break;
		case 110:
			g_engine->_soundManager->command(10, 0);
			break;
		case 111:
			g_engine->_soundManager->command(3, 0);
			break;
		case 112:
			g_engine->_soundManager->command(15, 0);
			break;
		default:
			if (previous_room < 104 || previous_room > 108)
				g_engine->_soundManager->command(10, 0);
			break;
		}
	}
}

void section_1_preload() {
	section_init_code_pointer = section_1_init;
	section_room_constructor = section_1_constructor;
	section_music_reset_pointer = section_1_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
