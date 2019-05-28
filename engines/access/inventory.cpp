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

#include "access/inventory.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/amazon/amazon_resources.h"
#include "access/martian/martian_resources.h"

namespace Access {

void InventoryEntry::load(const Common::String &name, const int *data) {
	_value = ITEM_NOT_FOUND;
	_name = name;
	if (data) {
		_otherItem1 = *data++;
		_newItem1 = *data++;
		_otherItem2 = *data++;
		_newItem2 = *data;
	} else {
		_otherItem1 = -1;
		_newItem1 = -1;
		_otherItem2 = -1;
		_newItem2 = -1;
	}
}

int InventoryEntry::checkItem(int itemId) {
	if (_otherItem1 == itemId)
		return _newItem1;
	else if (_otherItem2 == itemId)
		return _newItem2;
	else
		return -1;
}

/*------------------------------------------------------------------------*/

InventoryManager::InventoryManager(AccessEngine *vm) : Manager(vm) {
	_startInvItem = 0;
	_startInvBox = 0;
	_invChangeFlag = true;
	_invRefreshFlag = false;
	_invModeFlag = false;
	_startAboutItem = 0;
	_startTravelItem = 0;
	_iconDisplayFlag = true;
	_boxNum = 0;

	_inv.resize(_vm->_res->INVENTORY.size());
	for (uint idx = 0; idx < _inv.size(); ++idx)
		_inv[idx].load(_vm->_res->INVENTORY[idx]._desc, _vm->_res->INVENTORY[idx]._combo);

	for (uint i = 0; i < 26; ++i) {
		const int *r = INVCOORDS[i];
		_invCoords.push_back(Common::Rect(r[0], r[2], r[1], r[3]));
	}
}

int &InventoryManager::operator[](int idx) {
	// WORKAROUND: At least in Amazon, some game scripts accidentally do reads
	// beyond the length of the inventory array
	static int invalid = 0;
	return (idx >= (int)_inv.size()) ? invalid : _inv[idx]._value;
}

int InventoryManager::useItem() {
	return _vm->_useItem;
}

void InventoryManager::setUseItem(int itemId) {
	_vm->_useItem = itemId;
}

void InventoryManager::refreshInventory() {
	// The original version was using pre-rendering for the inventory to spare some time.
	// This is not needed on modern hardware, and it breaks a couple of things.
	// Therefore it was removed in order to keep the same logic than for the CD version
	// if (_vm->_screen->_vesaMode) {
	//		_invRefreshFlag = true;
	//		newDisplayInv();
	// }
}

int InventoryManager::newDisplayInv() {
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
		if (_vm->_useItem == -1) {
			result = 2;
			events.forceSetCursor(CURSOR_CROSSHAIRS);
		} else
			events.forceSetCursor(CURSOR_INVENTORY);
	}

	_invRefreshFlag = false;
	_invChangeFlag = false;
	return result;
}

int InventoryManager::displayInv() {
	int *inv = (int *) malloc(_vm->_res->INVENTORY.size() * sizeof(int));
	const char **names = (const char **)malloc(_vm->_res->INVENTORY.size() * sizeof(const char *));

	for (uint i = 0; i < _vm->_res->INVENTORY.size(); i++) {
		inv[i] = _inv[i]._value;
		names[i] = _inv[i]._name.c_str();
	}
	_vm->_events->forceSetCursor(CURSOR_CROSSHAIRS);
	_vm->_invBox->getList(names, inv);

	int btnSelected = 0;
	int boxX = _vm->_invBox->doBox_v1(_startInvItem, _startInvBox, btnSelected);
	_startInvItem = _vm->_boxDataStart;
	_startInvBox = _vm->_boxSelectY;

	if (boxX == -1)
		btnSelected = 2;

	if (btnSelected != 2)
		_vm->_useItem = _vm->_invBox->_tempListIdx[boxX];
	else
		_vm->_useItem = -1;

	free(names);
	free(inv);
	return 0;
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
	_fields._scrollX = _vm->_scrollX;
	_fields._scrollY = _vm->_scrollY;
	_fields._clipWidth = screen._clipWidth;
	_fields._clipHeight = screen._clipHeight;
	_fields._bufferStart = screen._bufferStart;
	_fields._scrollCol = _vm->_scrollCol;
	_fields._scrollRow = _vm->_scrollRow;
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
	_vm->_scrollX = _fields._scrollX;
	_vm->_scrollY = _fields._scrollY;
	screen._clipWidth = _fields._clipWidth;
	screen._clipHeight = _fields._clipHeight;
	screen._bufferStart = _fields._bufferStart;
	_vm->_scrollCol = _fields._scrollCol;
	_vm->_scrollRow = _fields._scrollRow;
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
	screen._bufferStart.x = 0;
	screen._bufferStart.y = 0;
	_vm->_scrollX = _vm->_scrollY = 0;

	_vm->_buffer1.clearBuffer();
	_vm->_buffer2.clearBuffer();
	// The original was doing at this point a check on vesa mode
	// We don't need it as we don't do inventory pre-rendering
	if (!_invRefreshFlag)
		screen.clearBuffer();

	screen.savePalette();
}

void InventoryManager::getList() {
	_items.clear();
	_tempLOff.clear();

	for (uint i = 0; i < _inv.size(); ++i) {
		if (_inv[i]._value == ITEM_IN_INVENTORY) {
			_items.push_back(i);
			_tempLOff.push_back(_inv[i]._name);
		}
	}
}

void InventoryManager::showAllItems() {
	_iconDisplayFlag = true;

	for (uint i = 0; i < _items.size(); ++i)
		putInvIcon(i, _items[i]);
}

void InventoryManager::putInvIcon(int itemIndex, int itemId) {
	SpriteResource *spr = _vm->_objectsTable[99];
	assert(spr);
	Common::Point pt((itemIndex % 6) * 46 + 23, (itemIndex / 6) * 35 + 15);
	_vm->_buffer2.plotImage(spr, itemId, pt);

	if (_iconDisplayFlag) {
		_vm->_screen->copyBlock(&_vm->_buffer2, Common::Rect(pt.x, pt.y, pt.x + 46, pt.y + 35));
	}
}

void InventoryManager::chooseItem() {
	EventsManager &events = *_vm->_events;
	_vm->_useItem = -1;

	while (!_vm->shouldQuit()) {
		// Check for events
		events.pollEventsAndWait();

		int selIndex;
		// Poll events and wait for a click on a known area
		if (!events._leftButton || ((selIndex = coordIndexOf()) == -1))
			continue;

		if (selIndex > 23) {
			if (selIndex == 25)
				_vm->_useItem = -1;
			break;
		} else if (selIndex < (int)_items.size() && _items[selIndex] != -1) {
			_boxNum = selIndex;
			_vm->copyBF2Vid();
			combineItems();
			_vm->copyBF2Vid();
			outlineIcon(_boxNum);
			_vm->_useItem = _items[_boxNum];
		}
	}
}

void InventoryManager::freeInvCells() {
	delete _vm->_objectsTable[99];
	_vm->_objectsTable[99] = nullptr;
}

int InventoryManager::coordIndexOf() {
	const Common::Point pt = _vm->_events->_mousePos;

	for (int i = 0; i < (int)_invCoords.size(); ++i) {
		if (_invCoords[i].contains(pt))
			return i;
	}

	return -1;
}

void InventoryManager::saveScreens() {
	_vm->_buffer1.copyTo(&_savedBuffer1);
	_vm->_screen->copyTo(&_savedScreen);
	_vm->_newRects.push_back(Common::Rect(0, 0, _savedScreen.w, _savedScreen.h));

}

void InventoryManager::restoreScreens() {
	_vm->_buffer1.w = _vm->_buffer1.pitch;
	_savedBuffer1.copyTo(&_vm->_buffer1);
	_savedScreen.copyTo(_vm->_screen);

	_savedBuffer1.free();
	_savedScreen.free();
}

void InventoryManager::outlineIcon(int itemIndex) {
	Screen &screen = *_vm->_screen;
	screen.frameRect(_invCoords[itemIndex], 7);

	Common::String s = _tempLOff[itemIndex];
	Font &font = *_vm->_fonts._font2;
	int strWidth = font.stringWidth(s);

	font._fontColors[0] = 0;
	font._fontColors[1] = 10;
	font._fontColors[2] = 11;
	font._fontColors[3] = 12;
	font.drawString(&screen, s, Common::Point((screen.w - strWidth) / 2, 184));
}

void InventoryManager::combineItems() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	screen._leftSkip = screen._rightSkip = 0;
	screen._topSkip = screen._bottomSkip = 0;
	screen._screenYOff = 0;

	Common::Point tempMouse = events._mousePos;
	Common::Point lastMouse = events._mousePos;

	Common::Rect &inv = _invCoords[_boxNum];
	Common::Rect r(inv.left, inv.top, inv.left + 46, inv.top + 35);
	Common::Point tempBox(inv.left, inv.top);
	Common::Point lastBox(inv.left, inv.top);

	_vm->_buffer2.copyBlock(&_vm->_buffer1, r);
	SpriteResource *sprites = _vm->_objectsTable[99];
	int invItem = _items[_boxNum];
	events.pollEvents();

	// Item drag handling loop if left button is held down
	while (!_vm->shouldQuit() && events._leftButton) {
		// Poll for events
		events.pollEventsAndWait();

		// Check positioning
		if (lastMouse == events._mousePos)
			continue;

		lastMouse = events._mousePos;
		Common::Rect lastRect(lastBox.x, lastBox.y, lastBox.x + 46, lastBox.y + 35);
		screen.copyBlock(&_vm->_buffer2, lastRect);

		Common::Point newPt;
		newPt.x = MAX(events._mousePos.x - tempMouse.x + tempBox.x, 0);
		newPt.y = MAX(events._mousePos.y - tempMouse.y + tempBox.y, 0);

		screen.plotImage(sprites, invItem, newPt);
		lastBox = newPt;
	}

	int destBox = events.checkMouseBox1(_invCoords);
	if (destBox >= 0 && destBox != _boxNum && destBox < (int)_items.size()
			&& _items[destBox] != -1) {
		int itemA = invItem;
		int itemB = _items[destBox];

		// Check whether the items can be combined
		int combinedItem = _inv[itemA].checkItem(itemB);
		if (combinedItem != -1) {
			_inv[combinedItem]._value = 1;
			_inv[itemA]._value = 2;
			_inv[itemB]._value = 2;
			_items[_boxNum] = -1;
			_items[destBox] = combinedItem;
			_tempLOff[destBox] = _inv[combinedItem]._name;
			events.hideCursor();

			// Shrink down the first item on top of the second item
			zoomIcon(itemA, itemB, destBox, true);

			// Shrink down the second item
			Common::Rect destRect(_invCoords[destBox].left, _invCoords[destBox].top,
				_invCoords[destBox].left + 46, _invCoords[destBox].top + 35);
			_vm->_buffer2.copyBlock(&_vm->_buffer1, destRect);
			screen._screenYOff = 0;
			zoomIcon(itemB, -1, destBox, true);

			// Exand up the new combined item from nothing to full size
			zoomIcon(combinedItem, -1, destBox, false);

			_boxNum = destBox;
			events.showCursor();
			return;
		}
	}

	_iconDisplayFlag = true;
	putInvIcon(_boxNum, invItem);
}

void InventoryManager::zoomIcon(int zoomItem, int backItem, int zoomBox, bool shrink) {
	Screen &screen = *_vm->_screen;
	screen._screenYOff = 0;
	SpriteResource *sprites = _vm->_objectsTable[99];

	int oldScale = _vm->_scale;
	int zoomScale = shrink ? 255 : 1;
	int zoomInc = shrink ? -1 : 1;
	Common::Rect boxRect(_invCoords[zoomBox].left, _invCoords[zoomBox].top,
		_invCoords[zoomBox].left + 46, _invCoords[zoomBox].top + 35);

	while (!_vm->shouldQuit() && zoomScale != 0 && zoomScale != 256) {
		_vm->_events->pollEventsAndWait();

		_vm->_buffer2.copyBlock(&_vm->_buffer1, boxRect);
		if (backItem != -1) {
			_iconDisplayFlag = false;
			putInvIcon(zoomBox, backItem);
		}

		_vm->_scale = zoomScale;
		screen.setScaleTable(zoomScale);

		int xv = screen._scaleTable1[boxRect.width() + 1];
		if (xv) {
			int yv = screen._scaleTable1[boxRect.height() + 1];
			if (yv) {
				// The zoomed size is positive in both directions, so show zoomed item
				Common::Rect scaledBox(xv, yv);
				scaledBox.moveTo(boxRect.left + (boxRect.width() - xv + 1) / 2,
					boxRect.top + (boxRect.height() - yv + 1) / 2);

				_vm->_buffer2.sPlotF(sprites->getFrame(zoomItem), scaledBox);
			}
		}

		screen.copyBlock(&_vm->_buffer2, boxRect);

		zoomScale += zoomInc;
	}

	if (!shrink) {
		// Handle the final full-size version
		_vm->_buffer2.copyBlock(&_vm->_buffer1, boxRect);
		_vm->_buffer2.plotImage(sprites, zoomItem,
			Common::Point(boxRect.left, boxRect.top));
		screen.copyBlock(&_vm->_buffer2, boxRect);
	}

	_vm->_scale = oldScale;
	screen.setScaleTable(oldScale);
}

void InventoryManager::synchronize(Common::Serializer &s) {
	int count = _inv.size();
	s.syncAsUint16LE(count);

	if (!s.isSaving())
		_inv.resize(count);

	for (int i = 0; i < count; ++i)
		s.syncAsUint16LE(_inv[i]._value);
}

} // End of namespace Access
