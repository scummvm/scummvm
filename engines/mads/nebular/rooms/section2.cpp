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

#include "mads/nebular/rooms/section2.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

extern void room_201_preload();
extern void room_202_preload();
extern void room_203_preload();
extern void room_205_preload();
extern void room_207_preload();
extern void room_208_preload();
extern void room_209_preload();
extern void room_210_preload();
extern void room_211_preload();
extern void room_212_preload();
extern void room_213_preload();
extern void room_214_preload();
extern void room_215_preload();
extern void room_216_preload();

void section_2_walker() {
	_vm->_sound->command(5);
	Common::String oldName = _game._player._spritesPrefix;

	switch (new_room) {
	case 213:
	case 216:
		Common::strcpy_s(player.series_name, "");
		break;
	default:
		if (_globals[kSexOfRex] != SEX_MALE) {
			Common::strcpy_s(player.series_name, "ROX");
		} else {
			Common::strcpy_s(player.series_name, "RXM");
		}
		break;
	}

	player.scaling_velocity = (new_room <= 212);

	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	if ((new_room == 203 || new_room == 204) && _globals[kRhotundaStatus])
		_game._player._loadsFirst = false;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void section_2_interface() {
	int idx = (new_room == 216) ? 4 : 2;
	Common::strcpy_s(kernel.interface, kernel_interface_name(idx));
}

void section_2_init() {
	player.scaling_velocity = true;
}

void section_2_constructor() {
	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_room) {
	case 201:
		room_preload_code_pointer = room_201_preload;
		break;
	case 202:
		room_preload_code_pointer = room_202_preload;
		break;
	case 203:
		room_preload_code_pointer = room_203_preload;
		break;
	case 205:
		room_preload_code_pointer = room_205_preload;
		break;
	case 207:
		room_preload_code_pointer = room_207_preload;
		break;
	case 208:
		room_preload_code_pointer = room_208_preload;
		break;
	case 209:
		room_preload_code_pointer = room_209_preload;
		break;
	case 210:
		room_preload_code_pointer = room_210_preload;
		break;
	case 211:
		room_preload_code_pointer = room_211_preload;
		break;
	case 212:
		room_preload_code_pointer = room_212_preload;
		break;
	case 213:
		room_preload_code_pointer = room_213_preload;
		break;
	case 214:
		room_preload_code_pointer = room_214_preload;
		break;
	case 215:
		room_preload_code_pointer = room_215_preload;
		break;
	case 216:
		room_preload_code_pointer = room_216_preload;
		break;
	}

	room_himem_preload(new_room, SECTION);
}

void section_2_music() {
	if (_vm->_musicFlag) {
		switch (new_room) {
		case 201:
			if ((_globals[kTeleporterCommand] == 2) || (_globals[kTeleporterCommand] == 4) || (_globals[kMeteorologistStatus] != 1))
				_vm->_sound->command(9);
			else
				_vm->_sound->command(17);
			break;
		case 202:
		case 203:
		case 204:
		case 205:
		case 208:
		case 209:
		case 212:
			_vm->_sound->command(9);
			break;
		case 206:
		case 211:
		case 215:
			_vm->_sound->command(10);
			break;
		case 207:
		case 214:
			_vm->_sound->command(11);
			break;
		case 210:
			if (_globals[kTwinklesStatus] == 0)
				_vm->_sound->command(15);
			else
				_vm->_sound->command(10);
			break;
		case 213:
			if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
				_vm->_sound->command(1);
			else
				_vm->_sound->command(9);
			break;
		case 216:
			_vm->_sound->command(16);
			break;
		default:
			_vm->_sound->command(10);
			break;
		}
	} else
		_vm->_sound->command(2);
}

void section_2_preload() {
	section_init_code_pointer = section_2_init;
	section_room_constructor = section_2_constructor;
	section_music_reset_pointer = section_2_music;
	section_daemon_code_pointer = NULL;
	section_parser_code_pointer = NULL;
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
