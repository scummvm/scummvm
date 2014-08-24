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

#include "access/inventory.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/amazon/amazon_resources.h"
#include "access/martian/martian_resources.h"

namespace Access {

InventoryManager::InventoryManager(AccessEngine *vm) : Manager(vm) {
	_startInvItem = 0;
	_startInvBox = 0;
	_invChangeFlag = true;
	_invRefreshFlag = false;
	_invModeFlag = false;
	_startAboutItem = 0;
	_startTravelItem = 0;
	_iconDisplayFlag = false;

	const char *const *names;
	switch (vm->getGameID()) {
	case GType_Amazon:
		names = Amazon::INVENTORY_NAMES;
		_inv.resize(85);
		break;
	case GType_MartianMemorandum:
		names = Martian::INVENTORY_NAMES;
		_inv.resize(54);
		break;
	default:
		error("Unknown game");
	}

	for (uint i = 0; i < _inv.size(); ++i)
		_names.push_back(names[i]);
}

int &InventoryManager::operator[](int idx) {
	return _inv[idx];
}

int InventoryManager::useItem() { 
	return _vm->_useItem;
}

void InventoryManager::setUseItem(int itemId) { 
	_vm->_useItem = itemId; 
}

void InventoryManager::refreshInventory() {
	if (_vm->_screen->_vesaMode) {
		_invRefreshFlag = true;
		newDisplayInv();
	}
}

int InventoryManager::newDisplayInv() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Room &room = *_vm->_room;
	FileManager &files = *_vm->_files;

	_invModeFlag = true;
	_vm->_timers.saveTimers();
	
	if (room._tile && !_invRefreshFlag) {
		_vm->_buffer1.copyTo(&_savedBuffer1);
		screen.copyTo(&_savedScreen);
	}

	savedFields();
	screen.setPanel(1);
	events._cursorExitFlag = false;
	getList();
	initFields();

	files.loadScreen(99, 0);
	_vm->_buffer1.copyTo(&_vm->_buffer2);
	_vm->copyBF2Vid();

	// Set cells
	Common::Array<CellIdent> cells;
	cells.push_back(CellIdent(99, 99, 1));
	_vm->loadCells(cells);

	showAllItems();

	if (!_invRefreshFlag) {
		chooseItem();
		if (_vm->_useItem != -1) {
			int savedScale = _vm->_scale;
			_vm->_scale = 153;
			_vm->_screen->setScaleTable(_vm->_scale);
			_vm->_buffer1.clearBuffer();

			SpriteResource *spr = _vm->_objectsTable[99];
			SpriteFrame *frame = spr->getFrame(_vm->_useItem);

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
	if (!screen._vesaMode && !_invRefreshFlag) {
		screen.clearBuffer();
		screen.setPalette();
	}

	if (!room._tile && !_invRefreshFlag) {
		_savedBuffer1.copyTo(&_vm->_buffer1);
		_savedScreen.copyTo(_vm->_screen);
	} else {
		screen.setBufferScan();
		room.buildScreen();

		if (!screen._vesaMode) {
			screen.fadeOut();
			_vm->copyBF2Vid();
		}
	}

	events._cursorExitFlag = false;
	screen._screenChangeFlag = false;
	_invModeFlag = false;
	events.debounceLeft();
	_vm->_timers.restoreTimers();
	_vm->_startup = 1;

	int result = 0;
	if (!_invRefreshFlag) {
		if (_vm->_useItem == -1) {
			result = 2;
		}
	}

	_invRefreshFlag = false;
	_invChangeFlag = false;
	return result;
}

void InventoryManager::savedFields() {
	Screen &screen = *_vm->_screen;
	Room &room = *_vm->_room;

	_fields._vWindowHeight = screen._vWindowHeight;
	_fields._vWindowLinesTall = screen._vWindowLinesTall;
	_fields._vWindowWidth = screen._vWindowWidth;
	_fields._vWindowBytesWide = screen._vWindowBytesWide;
	_fields._playFieldHeight = room._playFieldHeight;
	_fields._playFieldWidth = room._playFieldWidth;
	_fields._windowXAdd = screen._windowXAdd;
	_fields._windowYAdd = screen._windowYAdd;
	_fields._screenYOff = screen._screenYOff;
	_fields._scrollX = screen._scrollX;
	_fields._scrollY = screen._scrollY;
	_fields._clipWidth = screen._clipWidth;
	_fields._clipHeight = screen._clipHeight;
	_fields._bufferStart = screen._bufferStart;
	_fields._scrollCol = screen._scrollCol;
	_fields._scrollRow = screen._scrollRow;
}

void InventoryManager::restoreFields() {
	Screen &screen = *_vm->_screen;
	Room &room = *_vm->_room;

	screen._vWindowHeight = _fields._vWindowHeight;
	screen._vWindowLinesTall = _fields._vWindowLinesTall;
	screen._vWindowWidth = _fields._vWindowWidth;
	screen._vWindowBytesWide = _fields._vWindowBytesWide;
	room._playFieldHeight = _fields._playFieldHeight;
	room._playFieldWidth = _fields._playFieldWidth;
	screen._windowXAdd = _fields._windowXAdd;
	screen._windowYAdd = _fields._windowYAdd;
	screen._screenYOff = _fields._screenYOff;
	screen._scrollX = _fields._scrollX;
	screen._scrollY = _fields._scrollY;
	screen._clipWidth = _fields._clipWidth;
	screen._clipHeight = _fields._clipHeight;
	screen._bufferStart = _fields._bufferStart;
	screen._scrollCol = _fields._scrollCol;
	screen._scrollRow = _fields._scrollRow;
}

void InventoryManager::initFields() {
	Screen &screen = *_vm->_screen;
	Room &room = *_vm->_room;

	screen._vWindowHeight = screen.h;
	room._playFieldHeight = screen.h;
	screen._vWindowLinesTall = screen.h;
	screen._clipHeight = screen.h;
	room._playFieldWidth = screen.w;
	screen._vWindowWidth = screen.w;
	screen._vWindowBytesWide = screen.w;
	screen._clipWidth = screen.w;

	screen._windowXAdd = 0;
	screen._windowYAdd = 0;
	screen._screenYOff = 0;
	screen._scrollX = screen._scrollY = 0;
	screen._bufferStart.x = 0;
	screen._bufferStart.y = 0;

	_vm->_buffer1.clearBuffer();
	_vm->_buffer2.clearBuffer();
	if (!_invRefreshFlag && !screen._vesaMode)
		screen.clearBuffer();

	screen.savePalette();
}

void InventoryManager::getList() {
	_items.clear();
	for (uint i = 0; i < _inv.size(); ++i) {
		if (_inv[i])
			_items.push_back(i);
	}
}

void InventoryManager::showAllItems() {
	for (uint i = 0; i < _items.size(); ++i)
		putInvIcon(i, _items[i]);
}

void InventoryManager::putInvIcon(int itemIndex, int itemId) {
	SpriteResource *spr = _vm->_objectsTable[99];
	assert(spr);
	Common::Point pt((itemIndex % 6) * 46 + 23, (itemIndex / 6) * 35 + 15);
	_vm->_buffer2.plotImage(spr, itemId, pt);

	if (_iconDisplayFlag) {
		_vm->_buffer1.copyBlock(&_vm->_buffer2, Common::Rect(pt.x, pt.y, pt.x + 46, pt.y + 35));
	}
}

void InventoryManager::chooseItem() {
	_vm->_useItem = -1;

	error("TODO: chooseItem");
}

void InventoryManager::freeInvCells() {
	delete _vm->_objectsTable[99];
	_vm->_objectsTable[99] = nullptr;
}

} // End of namespace Access
