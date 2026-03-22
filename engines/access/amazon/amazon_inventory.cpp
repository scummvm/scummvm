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

#include "access/amazon/amazon_inventory.h"
#include "access/access.h"
#include "access/resources.h"

namespace Access {

namespace Amazon {

AmazonInventory::AmazonInventory(AccessEngine *vm) : InventoryManager(vm) {
}


int AmazonInventory::displayInv() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Room &room = *_vm->_room;
	FileManager &files = *_vm->_files;

	_invModeFlag = true;
	_vm->_timers.saveTimers();

	if (!room._tile && !_invRefreshFlag) {
		saveScreens();
	}

	savedFields();
	screen.setPanel(1);
	events._cursorExitFlag = false;
	getList();
	initFields();

	files._setPaletteFlag = false;
	files.loadScreen(&_vm->_buffer1, 99, 0);
	_vm->_buffer1.copyTo(&_vm->_buffer2);
	_vm->copyBF2Vid();

	// Set cells
	Common::Array<CellIdent> cells;
	cells.push_back(CellIdent(99, 99, 1));
	_vm->loadCells(cells);

	showAllItems();

	if (!_invRefreshFlag) {
		chooseItem();
		if (useItem() != -1) {
			int savedScale = _vm->_scale;
			_vm->_scale = 153;
			_vm->_screen->setScaleTable(_vm->_scale);
			_vm->_buffer1.clearBuffer();

			SpriteResource *spr = _vm->_objectsTable[99];
			const SpriteFrame *frame = spr->getFrame(_vm->_useItem);

			int w = screen._scaleTable1[46];
			int h = screen._scaleTable1[35];
			_vm->_buffer1.sPlotF(frame, Common::Rect(0, 0, w, h));
			events.setCursorData(&_vm->_buffer1, Common::Rect(0, 0, w, h));

			_vm->_scale = savedScale;
			screen.setScaleTable(_vm->_scale);
		}
	}

	freeInvCells();
	screen.setPanel(0);
	events.debounceLeft();

	restoreFields();
	screen.restorePalette();
	// The original was testing the vesa mode too.
	// We removed this check as we don't use pre-rendering
	if (!_invRefreshFlag) {
		screen.clearScreen();
		screen.setPalette();
	}

	if (!room._tile && !_invRefreshFlag) {
		restoreScreens();
	} else {
		screen.setBufferScan();
		room.buildScreen();

		// The original was doing a check on the vesa mode at this point.
		// We don't need it as we don't do inventory pre-rendering
		screen.fadeOut();
		_vm->copyBF2Vid();
	}

	events._cursorExitFlag = false;
	screen._screenChangeFlag = false;
	_invModeFlag = false;
	events.debounceLeft();
	_vm->_timers.restoreTimers();
	_vm->_startup = 1;

	int result = 0;
	if (!_invRefreshFlag) {
		if (useItem() == -1) {
			result = 2;
			events.forceSetCursor(CURSOR_CROSSHAIRS);
		} else
			events.forceSetCursor(CURSOR_INVENTORY);
	}

	_invRefreshFlag = false;
	_invChangeFlag = false;
	return result;
}

} // end namespace Amazon

} // end namespace Access
