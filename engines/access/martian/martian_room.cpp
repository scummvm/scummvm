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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/martian/martian_game.h"
#include "access/martian/martian_resources.h"
#include "access/martian/martian_room.h"

namespace Access {

namespace Martian {

MartianRoom::MartianRoom(AccessEngine *vm) : Room(vm) {
	_game = (MartianEngine *)vm;
}

MartianRoom::~MartianRoom() {
}

void MartianRoom::loadRoom(int roomNumber) {
	loadRoomData(&MMRES.ROOMTBL[roomNumber]._data[0]);
}

void MartianRoom::reloadRoom() {
//	_vm->_currentMan = _roomFlag;
//	_vm->_currentManOld = _roomFlag;
//	_vm->_manScaleOff = 0;

	_vm->_player->loadTexPalette();
	_vm->_player->loadSprites("TEX.LZ");

	loadRoom(_vm->_player->_roomNumber);

	reloadRoom1();
}

void MartianRoom::reloadRoom1() {
	_selectCommand = -1;
	_vm->_boxSelect = false; //-1
	_vm->_player->_playerOff = false;

	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_screen->clearScreen();
	_vm->_events->showCursor();
	roomSet();
	_vm->_player->load();

	if (_vm->_player->_roomNumber != 47)
		_vm->_player->calcManScale();

	_vm->_events->hideCursor();
	roomMenu();
	_vm->_screen->setBufferScan();
	setupRoom();
	setWallCodes();
	buildScreen();
	_vm->copyBF2Vid();

	_vm->_screen->setManPalette();
	_vm->_events->showCursor();
	_vm->_player->_frame = 0;
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_events->clearEvents();
}

void MartianRoom::roomSet() {
	_vm->_numAnimTimers = 0;
	_vm->_scripts->_sequence = 1000;
	_vm->_scripts->searchForSequence();
	_vm->_scripts->executeScript();

	for (int i = 0; i < 30; i++)
		_byte26CD2[i] = 0;

	for (int i = 0; i < 10; i++)
		_byte26CBC[i] = 0;
}

void MartianRoom::roomMenu() {
	Resource *iconData = _vm->_files->loadFile("ICONS.LZ");
	SpriteResource *spr = new SpriteResource(_vm, iconData);
	delete iconData;

	_vm->_screen->saveScreen();
	_vm->_screen->setDisplayScan();
	_vm->_destIn = _vm->_screen;	// TODO: Redundant
	_vm->_screen->plotImage(spr, 0, Common::Point(5, 184));
	_vm->_screen->plotImage(spr, 1, Common::Point(155, 184));

	_vm->_screen->restoreScreen();
	delete spr;
}

void MartianRoom::mainAreaClick() {
	Common::Point &mousePos = _vm->_events->_mousePos;
	Common::Point pt = _vm->_events->calcRawMouse();
	Screen &screen = *_vm->_screen;
	Player &player = *_vm->_player;

	if (_selectCommand == -1) {
		player._moveTo = pt;
		player._playerMove = true;
	} else if (mousePos.x >= screen._windowXAdd &&
		mousePos.x <= (screen._windowXAdd + screen._vWindowBytesWide) &&
		mousePos.y >= screen._windowYAdd &&
		mousePos.y <= (screen._windowYAdd + screen._vWindowLinesTall)) {
			if (checkBoxes1(pt) >= 0) {
				checkBoxes3();
			}
	}
}

} // End of namespace Martian
} // End of namespace Access
