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
#include "mads/nebular/nebular.h"
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
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
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;
	if (_scene->_nextSceneId <= 103 || _scene->_nextSceneId == 111) {
		if (_globals[kSexOfRex] == SEX_FEMALE)
			Common::strcpy_s(player.series_name, "ROX");
		else {
			Common::strcpy_s(player.series_name, "RXM");
			_globals[kSexOfRex] = SEX_MALE;
		}
	} else if (_scene->_nextSceneId <= 110) {
		Common::strcpy_s(player.series_name, "RXSW");
		_globals[kSexOfRex] = SEX_UNKNOWN;
	} else if (_scene->_nextSceneId == 112)
		Common::strcpy_s(player.series_name, "");

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	if (_scene->_nextSceneId == 105 || (_scene->_nextSceneId == 109 && _globals[kHoovicAlive])) {
		_game._player._spritesChanged = true;
		_game._player._loadsFirst = false;
	}

	player.scaling_velocity = 0;
	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void section_1_interface() {
	int idx = (_scene->_nextSceneId > 103 && _scene->_nextSceneId < 112) ? 1 : 0;
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
		switch (_scene->_nextSceneId) {
		case 101:
			_vm->_sound->command(11);
			break;
		case 102:
			_vm->_sound->command(12);
			break;
		case 103:
			_vm->_sound->command(3);
			_vm->_sound->command(25);
			break;
		case 109:
			_vm->_sound->command(13);
			break;
		case 110:
			_vm->_sound->command(10);
			break;
		case 111:
			_vm->_sound->command(3);
			break;
		case 112:
			_vm->_sound->command(15);
			break;
		default:
			if (_scene->_priorSceneId < 104 || _scene->_priorSceneId > 108)
				_vm->_sound->command(10);
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
} // namespace MADSV2
} // namespace MADS
