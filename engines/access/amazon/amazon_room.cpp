/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "access/access.h"
#include "access/amazon/amazon_resources.h"
#include "access/amazon/amazon_room.h"

namespace Access {

namespace Amazon {

AmazonRoom::AmazonRoom(AccessEngine *vm): Room(vm) {
	_antOutFlag = false;
}

void AmazonRoom::loadRoom(int roomNumber) {
	loadRoomData(ROOM_TABLE[roomNumber]);
}

void AmazonRoom::reloadRoom() {
	loadRoom(_vm->_player->_roomNumber);

	if (_roomFlag != 1) {
		_vm->_currentMan = _roomFlag;
		_vm->_currentManOld = _roomFlag;
		_vm->_manScaleOff = 0;
		
		switch (_vm->_currentMan) {
		case 0:
			_vm->_man1 = _vm->_files->loadFile("MAN.LZ");
			break;

		case 2:
			_vm->_man1 = _vm->_files->loadFile("JMAN.LZ");
			break;

		case 3:
			_vm->_man1 = _vm->_files->loadFile("OVERHEAD.LZ");
			_vm->_manScaleOff = 1;
			break;

		default:
			break;
		}
	}

	reloadRoom1();
}

void AmazonRoom::reloadRoom1() {
	if (_vm->_player->_roomNumber == 22 || _vm->_player->_roomNumber == 31 
			|| _vm->_player->_roomNumber == 42 || _vm->_player->_roomNumber == 44) {
		_vm->_inactive = _vm->_files->loadFile("MAYA.LZ");
		_vm->_currentCharFlag = false;
	}

	_vm->_selectCommand = -1;
	_vm->_normalMouse = 1;
	_vm->_mouseMode = 0;
	_vm->_boxSelect = true;
	_vm->_player->_playerOff = 0;

	_vm->_screen->fadeOut();
	_vm->_screen->clearScreen();
	roomSet();

	if (!_roomFlag && (_vm->_player->_roomNumber != 61 || !_antOutFlag)) {
		_vm->_player->load();
		_vm->_player->calcManScale();
	}

	if (_vm->_player->_roomNumber != 20 && _vm->_player->_roomNumber != 24
			&& _vm->_player->_roomNumber != 33 && _vm->_player->_roomNumber != 45) {
		roomMenu();
	}

	_vm->_screen->setBufferScan();
	setupRoom();
	setWallCodes();
	buildScreen();

	if (_vm->_player->_roomNumber != 20 && _vm->_player->_roomNumber != 24
			&& _vm->_player->_roomNumber != 33) {
		_vm->_screen->setPalette();
		_vm->_screen->copyBF2Vid();
	}

	_vm->_player->_frame = 0;
	_vm->_oldRect.clear();
	_vm->_newRect.clear();
}

void AmazonRoom::roomSet() {
	_vm->_numAnimTimers = 0;
	_vm->_scripts->_sequence = 1000;
	_vm->_scripts->searchForSeq();
	_vm->_scripts->executeCommand();
}

void AmazonRoom::roomMenu() {
	// TODO
}

} // End of namespace Amazon

} // End of namespace Access
