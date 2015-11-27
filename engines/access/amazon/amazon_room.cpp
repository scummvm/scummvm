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
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_resources.h"
#include "access/amazon/amazon_room.h"

namespace Access {

namespace Amazon {

AmazonRoom::AmazonRoom(AccessEngine *vm) : Room(vm) {
	_game = (AmazonEngine *)vm;
	_antOutFlag = false;
	_icon = nullptr;
}

AmazonRoom::~AmazonRoom() {
}

void AmazonRoom::loadRoom(int roomNumber) {
	loadRoomData(&AMRES.ROOMTBL[roomNumber]._data[0]);
}

void AmazonRoom::reloadRoom() {
	loadRoom(_vm->_player->_roomNumber);

	if (_roomFlag != 1) {
		_vm->_currentMan = _roomFlag;
		_vm->_currentManOld = _roomFlag;
		_vm->_manScaleOff = 0;

		switch (_vm->_currentMan) {
		case 0:
			_vm->_player->loadSprites("MAN.LZ");
			break;

		case 2:
			_vm->_player->loadSprites("JMAN.LZ");
			break;

		case 3:
			_vm->_player->loadSprites("OVERHEAD.LZ");
			_vm->_manScaleOff = 1;
			break;

		default:
			break;
		}
	}

	reloadRoom1();
}

void AmazonRoom::reloadRoom1() {
	if (_vm->_player->_roomNumber == 29 || _vm->_player->_roomNumber == 31
			|| _vm->_player->_roomNumber == 42 || _vm->_player->_roomNumber == 44) {
		Resource *spriteData = _vm->_files->loadFile("MAYA.LZ");
		_game->_inactive._altSpritesPtr = new SpriteResource(_vm, spriteData);
		delete spriteData;
		_vm->_currentCharFlag = false;
	}

	_selectCommand = -1;
	_vm->_events->setNormalCursor(CURSOR_CROSSHAIRS);
	_vm->_mouseMode = 0;
	_vm->_boxSelect = true;
	_vm->_player->_playerOff = false;

	_vm->_screen->fadeOut();
	_vm->_screen->clearScreen();
	roomSet();

	if (_roomFlag != 1 && (_vm->_player->_roomNumber != 61 || !_antOutFlag)) {
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

	if (!_vm->_screen->_vesaMode) {
		_vm->copyBF2Vid();
	} else if (_vm->_player->_roomNumber != 20 && _vm->_player->_roomNumber != 24
			&& _vm->_player->_roomNumber != 33) {
		_vm->_screen->setPalette();
		_vm->copyBF2Vid();
	}

	// Stop player moving
	_vm->_player->_playerMove = false;
	_vm->_player->_frame = 0;

	// Clear any dirty rects from the old scene
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
}

void AmazonRoom::setupRoom() {
	Room::setupRoom();

	// WORKAROUND: The original engine doesn't handle vertical scrolling rooms
	Screen &screen = *_vm->_screen;
	if (screen._vWindowHeight == (_playFieldHeight - 1)) {
		_vm->_scrollRow = 1;
		_vm->_scrollY = 0;
	}
}

void AmazonRoom::roomSet() {
	_vm->_numAnimTimers = 0;
	_vm->_scripts->_sequence = 1000;
	_vm->_scripts->searchForSequence();
	_vm->_scripts->executeScript();
}

void AmazonRoom::roomMenu() {
	Resource *iconData = _vm->_files->loadFile("ICONS.LZ");
	SpriteResource *spr = new SpriteResource(_vm, iconData);
	delete iconData;

	Screen &screen = *_vm->_screen;
	screen.saveScreen();
	screen.setDisplayScan();
	_vm->_destIn = &screen;	// TODO: Redundant
	screen.plotImage(spr, 0, Common::Point(0, 177));
	screen.plotImage(spr, 1, Common::Point(143, 177));

	screen.restoreScreen();
	delete spr;
}

void AmazonRoom::mainAreaClick() {
	Common::Point &mousePos = _vm->_events->_mousePos;
	Common::Point pt = _vm->_events->calcRawMouse();
	Screen &screen = *_vm->_screen;
	Player &player = *_vm->_player;

	if (_selectCommand == -1) {
		if (player._roomNumber == 42 || player._roomNumber == 44 ||
				player._roomNumber == 31 || player._roomNumber == 29) {
			switch (checkBoxes1(pt)) {
			case 0:
				// Make Jason the active player
				_game->_jasMayaFlag = 0;
				return;
			case 1:
				// Make Maya the active player
				_game->_jasMayaFlag = 1;
				return;
			default:
				break;
			}
		}

		// WORKAROUND: In Amazon room 9, you can't leave the screen to the south due
		// to not being able to click a Y position that's high enough
		if (_vm->_scrollRow == 0 && pt.y > 178)
			pt.y = 200;

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

void AmazonRoom::walkCursor() {
	// WORKAROUND: For scene 29, which is a normal walkable scene, but yet can be
	// 'exited'. This workaround ensures the scene will only be left if you click
	// the Exit icon when the cursor is already a walk cursor
	EventsManager &events = *_vm->_events;

	if (_vm->_events->_middleButton || (_vm->_player->_roomNumber == 29 &&
			events._normalMouse != CURSOR_CROSSHAIRS)) {
		events.forceSetCursor(CURSOR_CROSSHAIRS);
		_selectCommand = -1;
		_vm->_boxSelect = true;
	} else {
		Room::walkCursor();
	}
}

void AmazonRoom::init4Quads() {
	if (!_vm->_screen->_vesaMode)
		return;

	// CHECKME: in the original, this call of tileScreen uses an useless parameter, "TILES.BLK"
	_game->tileScreen();
	_vm->_inventory->refreshInventory();
	_game->updateSummary(_game->_chapter);

	_vm->_screen->setPanel(0);
	_vm->_screen->clearScreen();
}

void AmazonRoom::clearRoom() {
	_game->freeInactivePlayer();
	Room::clearRoom();
}

} // End of namespace Amazon

} // End of namespace Access
