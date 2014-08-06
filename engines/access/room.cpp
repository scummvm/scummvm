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
#include "access/resources.h"
#include "access/room.h"

namespace Access {

Room::Room(AccessEngine *vm) : _vm(vm) {
	_function = 0;
	_roomFlag = 0;
}

void Room::doRoom() {
	bool reloadFlag = false;

	while (!_vm->shouldQuit()) {
		if (!reloadFlag) {
			_vm->_numImages = 0;
			_vm->_newRect.clear();
			_vm->_oldRect.clear();
			_vm->_nextImage = 0;
			_vm->_numAnimTimers = 0;

			reloadRoom();
		}

		reloadFlag = false;
		_vm->_startup = 0;
		_function = 0;

		while (!_vm->shouldQuit()) {
			_vm->_numImages = 0;
			if (_vm->_startup != -1 && --_vm->_startup != 0) {
				--_vm->_startup;
				_vm->_events->showCursor();
				_vm->_screen->fadeIn();
			}

			_vm->_events->pollEvents();
			_vm->_nextImage = 0;
			_vm->_player->walk();
			_vm->_sound->midiRepeat();
			_vm->_screen->checkScroll();
			doCommands();

			// DOROOMFLASHBACK jump point
			if (_function == 1) {
				clearRoom();
				break;
			} else if (_function == 2) {
				clearRoom();
				return;
			} else if (_function == 3) {
				reloadRoom1();
				reloadFlag = true;
				break;
			} else if (_function == 4) {
				break;
			}

			if (_vm->_screen->_scrollFlag) {
				_vm->_screen->copyBF1BF2();
				_vm->_newRect.clear();
				_function = 0;
				roomLoop();

				if (_function == 1) {
					clearRoom();
					break;
				} else {
					_vm->_screen->plotList();
					_vm->_screen->copyRects();

					_vm->_screen->copyBF2Vid();
				}
			} else {
				_vm->_screen->copyBF1BF2();
				_vm->_newRect.clear();
				_function = 0;
				roomLoop();

				if (_function == 1) {
					clearRoom();
					break;
				} else {
					_vm->_screen->plotList();
					_vm->_screen->copyBlocks();
				}
			}
		}
	}
}

void Room::clearRoom() {
	if (_vm->_sound->_music) {
		_vm->_sound->stopSong();
		delete[] _vm->_sound->_music;
		_vm->_sound->_music = nullptr;
	}

	_vm->_sound->freeSounds();
	_vm->_numAnimTimers = 0;

	_vm->freeAnimationData();
	_vm->_scripts->freeScriptData();
	_vm->freeCells();
	_vm->freePlayField();
	_vm->freeInactiveData();
	_vm->freeManData();
}

void Room::loadRoom(int room) {
	// TODO
}

void Room::roomLoop() {
	// TODO
}


void Room::doCommands() {
	// TODO
}

void Room::setupRoom() {
	// TODO
}

void Room::setWallCodes() {
	// TODO
}

void Room::buildScreen() {
	// TODO
}

} // End of namespace Access
