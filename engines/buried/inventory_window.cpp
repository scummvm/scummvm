/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#include "buried/avi_frames.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_info.h"
#include "buried/inventory_window.h"
#include "buried/message.h"
#include "buried/resources.h"

#include "common/algorithm.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

InventoryWindow::InventoryWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_background = 0;
	_magSelected = false;
	_upSelected = false;
	_downSelected = false;
	_textSelected = -1;
	_itemComesFromInventory = false;
	_draggingObject = false;
	_draggingItemID = -1;
	_draggingItemSpriteData.image = 0;
	_draggingIconIndex = 0;
	_draggingItemInInventory = false;

	if (_vm->isDemo()) {
		// Demo gets its own items
		_itemArray.push_back(kItemBioChipInterface);
		_itemArray.push_back(kItemGrapplingHook);
	} else {
		// Default items
		_itemArray.push_back(kItemBioChipBlank);
		_itemArray.push_back(kItemBioChipCloak);
		_itemArray.push_back(kItemBioChipEvidence);
		_itemArray.push_back(kItemBioChipFiles);
		_itemArray.push_back(kItemBioChipInterface);
		_itemArray.push_back(kItemBioChipJump);
	}

	_curItem = 0;

	_infoWindow = 0;
	_letterViewWindow = 0;

	_scrollTimer = 0;

	rebuildPreBuffer();

	_textFont = _vm->_gfx->createFont(14);

	_rect = Common::Rect(182, 375, 450, 454);
	_curCursor = (int)kCursorNone;

	Common::String dragFramesFileName;

	if (_vm->isDemo()) {
		if (_vm->isTrueColor())
			dragFramesFileName = "COMMON/INVDRAG.BTV";
		else
			dragFramesFileName = "COMMON/INVDRAG8.BTV";
	} else {
		dragFramesFileName = _vm->getFilePath(IDS_INVENTORY_DRAG_FILENAME);
	}

	_dragFrames = new AVIFrames(dragFramesFileName);
}

InventoryWindow::~InventoryWindow() {
	if (_background) {
		_background->free();
		delete _background;
	}

	if (_draggingItemSpriteData.image) {
		_draggingItemSpriteData.image->free();
		delete _draggingItemSpriteData.image;
	}

	if (_scrollTimer != 0)
		killTimer(_scrollTimer);

	destroyInfoWindow();
	destroyBurnedLetterWindow();

	delete _textFont;
	delete _dragFrames;
}

bool InventoryWindow::rebuildPreBuffer() {
	if (_background) {
		_background->free();
		delete _background;
	}

	_background = _vm->_gfx->getBitmap(IDB_INVENTORY_BACKGROUND);
	Graphics::Surface *arrows = _vm->_gfx->getBitmap(IDB_INVENTORY_ARROWS);

	int leftOffset = 3;
	if (_magSelected)
		leftOffset += 69;
	if (_upSelected)
		leftOffset += 23;
	if (_downSelected)
		leftOffset += 46;

	_vm->_gfx->crossBlit(_background, 96, 7, 18, 69, arrows, leftOffset, 0);
	arrows->free();
	delete arrows;

	if (!_itemArray.empty()) {
		// Draw the icon for the current item
		Graphics::Surface *icon = _vm->_gfx->getBitmap(IDB_PICON_BITMAP_BASE + _itemArray[_curItem]);
		_vm->_gfx->crossBlit(_background, 17, 8, icon->w, icon->h, icon, 0, 0);
	}

	return true;
}

bool InventoryWindow::addItem(int itemID) {
	_itemArray.push_back(itemID);

	// Sort the array
	Common::sort(_itemArray.begin(), _itemArray.end());

	// Find the new position, and set the current selection to that
	for (int i = 0; i < (int)_itemArray.size(); i++) {
		if (_itemArray[i] == itemID) {
			_curItem = i;
			break;
		}
	}

	// Redraw
	rebuildPreBuffer();
	invalidateWindow(false);

	// TODO: Scoring flags
	return true;
}

bool InventoryWindow::removeItem(int itemID) {
	bool found = false;

	for (int i = 0; i < (int)_itemArray.size(); i++) {
		if (_itemArray[i] == itemID) {
			found = true;
			_itemArray.remove_at(i);
			break;
		}
	}

	if (!found)
		return false;

	if (_curItem >= (int)_itemArray.size())
		_curItem--;

	rebuildPreBuffer();
	invalidateWindow(false);

	return true;
}

bool InventoryWindow::startDraggingNewItem(int itemID, const Common::Point &pointStart) {
	_draggingItemInInventory = false;
	_itemComesFromInventory = false;
	_draggingObject = true;
	_draggingItemID = itemID;
	_draggingIconIndex = 0;

	InventoryElement staticItemData = getItemStaticData(_draggingItemID);
	_draggingItemSpriteData.image = _vm->_gfx->getBitmap(IDB_DRAG_BITMAP_BASE + staticItemData.firstDragID - 1);
	_draggingItemSpriteData.xPos = 0;
	_draggingItemSpriteData.yPos = 0;
	_draggingItemSpriteData.width = _draggingItemSpriteData.image->w;
	_draggingItemSpriteData.height = _draggingItemSpriteData.image->h;

	if (_vm->isTrueColor()) {
		_draggingItemSpriteData.redTrans = 255;
		_draggingItemSpriteData.greenTrans = 255;
		_draggingItemSpriteData.blueTrans = 255;
	} else {
		_draggingItemSpriteData.redTrans = _vm->_gfx->getDefaultPalette()[0];
		_draggingItemSpriteData.greenTrans = _vm->_gfx->getDefaultPalette()[1];
		_draggingItemSpriteData.blueTrans = _vm->_gfx->getDefaultPalette()[2];

		if (!_vm->isDemo()) {
			for (int y = 0; y < _draggingItemSpriteData.height; y++) {
				for (int x = 0; x < _draggingItemSpriteData.width; x++) {
					byte color = *((byte *)_draggingItemSpriteData.image->getBasePtr(x, y));

					if (color != 0)
						*((byte *)_draggingItemSpriteData.image->getBasePtr(x, y)) = color + 10;
				}
			}
		}
	}

	// TODO: Set capture
	onSetCursor(kMessageTypeLButtonDown);
	onMouseMove(pointStart, 0);

	return true;
}

bool InventoryWindow::displayBurnedLetterWindow() {
	if (_letterViewWindow)
		return true;

	// TODO

	return true;
}

bool InventoryWindow::destroyBurnedLetterWindow() {
	if (!_letterViewWindow)
		return false;

	delete _letterViewWindow;
	_letterViewWindow = 0;

	// TODO: Notify the scene view window

	return true;
}

void InventoryWindow::onPaint() {
	// Draw the prebuffer
	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(_background, absoluteRect.left, absoluteRect.top);

	// Draw inventory item names
	uint32 textColor = _vm->_gfx->getColor(212, 109, 0);
	for (int i = -2; i < 3; i++) {
		if ((i + _curItem) >= 0 && (i + _curItem) < (int)_itemArray.size()) {
			Common::Rect textRect = Common::Rect(120, (i + 2) * 13 + 8, 254, (i + 3) * 13 + 8);
			textRect.translate(absoluteRect.left, absoluteRect.top);
			Common::String text = _vm->getString(IDES_ITEM_TITLE_BASE + _itemArray[_curItem + i]);
			_textFont->drawString(_vm->_gfx->getScreen(), text, textRect.left, textRect.top, textRect.width(), textColor);
		}
	}
}

void InventoryWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (!isWindowEnabled())
		return;

	Common::Rect up(95, 8, 114, 29);
	Common::Rect down(95, 54, 114, 75);
	Common::Rect magnify(95, 30, 114, 52);
	Common::Rect picon(15, 8, 93, 72);

	Common::Rect inventoryText[5];
	for (int i = 0; i < 5; i++)
		inventoryText[i] = Common::Rect(120, i * 13 + 8, 254, (i + 1) * 13 + 8);

	bool redraw = false;
	if (up.contains(point)) {
		if (_curItem > 0) {
			_upSelected = true;
			redraw = true;
			_scrollTimer = setTimer(250);
		}
	}

	if (down.contains(point)) {
		if (_curItem < ((int)_itemArray.size() - 1)) {
			_downSelected = true;
			redraw = true;
			_scrollTimer = setTimer(250);
		}
	}

	if (magnify.contains(point)) {
		_magSelected = true;
		redraw = true;
	}

	for (int i = 0; i < 5; i++) {
		if (inventoryText[i].contains(point)) {
			_textSelected = i;
			break;
		}
	}

	if (picon.contains(point) && !_itemArray.empty() && !_infoWindow) {
		int itemID = _itemArray[_curItem];

		switch (itemID) {
		case kItemBioChipAI:
		case kItemBioChipBlank:
		case kItemBioChipCloak:
		case kItemBioChipEvidence:
		case kItemBioChipFiles:
		case kItemBioChipInterface:
		case kItemBioChipJump:
		case kItemBioChipTranslate:
			((GameUIWindow *)_parent)->_bioChipRightWindow->changeCurrentBioChip(itemID);
			return;
		}

		if (true) { // TODO: Check for auxilary window
			if (itemID == kItemBurnedLetter) {
				displayBurnedLetterWindow();
				return;
			}

			if (itemID == kItemLensFilter) {
				// TODO
				return;
			}

			// TODO: Support dragging
			return;

			InventoryElement staticItemData = getItemStaticData(_draggingItemID);

			if (staticItemData.firstDragID < 0)
				return;

			_draggingItemSpriteData.image = _vm->_gfx->getBitmap(IDB_DRAG_BITMAP_BASE + staticItemData.firstDragID - 1);
			_draggingItemInInventory = true;
			_itemComesFromInventory = true;
			_draggingObject = true;
			_draggingItemID = itemID;
			_draggingIconIndex = 0;

			_draggingItemSpriteData.xPos = 0;
			_draggingItemSpriteData.yPos = 0;
			_draggingItemSpriteData.width = _draggingItemSpriteData.image->w;
			_draggingItemSpriteData.height = _draggingItemSpriteData.image->h;

			if (_vm->isTrueColor()) {
				_draggingItemSpriteData.redTrans = 255;
				_draggingItemSpriteData.greenTrans = 255;
				_draggingItemSpriteData.blueTrans = 255;
			} else {
				_draggingItemSpriteData.redTrans = _vm->_gfx->getDefaultPalette()[0];
				_draggingItemSpriteData.greenTrans = _vm->_gfx->getDefaultPalette()[1];
				_draggingItemSpriteData.blueTrans = _vm->_gfx->getDefaultPalette()[2];

				if (!_vm->isDemo()) {
					for (int y = 0; y < _draggingItemSpriteData.height; y++) {
						for (int x = 0; x < _draggingItemSpriteData.width; x++) {
							byte color = *((byte *)_draggingItemSpriteData.image->getBasePtr(x, y));

							if (color != 0)
								*((byte *)_draggingItemSpriteData.image->getBasePtr(x, y)) = color + 10;
						}
					}
				}
			}

			// TODO: SetCapture();

			onSetCursor(kMessageTypeLButtonDown);
			// TODO: Change sprite status
			onMouseMove(point, 0);
		}
	}

	if (redraw) {
		rebuildPreBuffer();
		invalidateWindow(false);
	}
}

void InventoryWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (!isWindowEnabled())
		return;

	Common::Rect up(95, 8, 114, 29);
	Common::Rect down(95, 54, 114, 75);
	Common::Rect magnify(95, 30, 114, 52);
	Common::Rect picon(15, 8, 93, 72);

	Common::Rect inventoryText[5];
	for (int i = 0; i < 5; i++)
		inventoryText[i] = Common::Rect(120, i * 13 + 8, 254, (i + 1) * 13 + 8);

	bool redraw = _upSelected || _downSelected || _magSelected;

	if (up.contains(point) && _upSelected) {
		if (_curItem > 0)
			_curItem--;

		if (_infoWindow)
			_infoWindow->changeCurrentItem(_itemArray[_curItem]);

		if (_scrollTimer != 0) {
			killTimer(_scrollTimer);
			_scrollTimer = 0;
		}
	}

	if (down.contains(point) && _downSelected) {
		if (_curItem < ((int)_itemArray.size() - 1))
			_curItem++;

		if (_infoWindow)
			_infoWindow->changeCurrentItem(_itemArray[_curItem]);

		if (_scrollTimer != 0) {
			killTimer(_scrollTimer);
			_scrollTimer = 0;
		}
	}

	if (magnify.contains(point)) {
		destroyBurnedLetterWindow();

		if (_infoWindow) {
			destroyInfoWindow();
		} else {
			// TODO: Create window
			_magSelected = true;
			redraw = true;
		}
	}

	if (_textSelected >= 0) {
		for (int i = 0; i < 5; i++) {
			if (inventoryText[i].contains(point) && (_curItem + i - 2) >= 0 && (_curItem + i - 2) < (int)_itemArray.size() && i == _textSelected) {
				_curItem += i - 2;
				redraw = true;

				if (_infoWindow)
					_infoWindow->changeCurrentItem(_itemArray[_curItem]);
			}
		}
	}

	_upSelected = false;
	_downSelected = false;
	_textSelected = -1;

	if (_draggingObject) {
		_draggingObject = false;

		// TODO: Lots missing
	}

	if (redraw) {
		rebuildPreBuffer();
		invalidateWindow(false);
	}
}

void InventoryWindow::onMouseMove(const Common::Point &point, uint flags) {
	_curMousePos = point;

	if (_draggingObject) {
		// TODO
	} else {
		Common::Rect up(95, 8, 114, 29);
		Common::Rect down(95, 54, 114, 75);
		Common::Rect magnify(95, 30, 114, 52);

		if (_upSelected && !up.contains(point)) {
			_upSelected = false;
			rebuildPreBuffer();
			invalidateWindow(false);
		}

		if (_downSelected && !down.contains(point)) {
			_downSelected = false;
			rebuildPreBuffer();
			invalidateWindow(false);
		}

		if (_magSelected && !magnify.contains(point)) {
			_magSelected = false;
			rebuildPreBuffer();
			invalidateWindow(false);
		}
	}
}

bool InventoryWindow::onSetCursor(uint message) {
	if (!isWindowEnabled())
		return false;

	if (_draggingObject) {
		_vm->_gfx->setCursor(kCursorClosedHand);
	} else if (Common::Rect(15, 8, 93, 72).contains(_curMousePos)) {
		_vm->_gfx->setCursor(kCursorOpenHand);
	} else {
		_vm->_gfx->setCursor(kCursorArrow);
	}

	return true;
}

void InventoryWindow::onTimer(uint timer) {
	if (_upSelected) {
		if (_curItem > 0) {
			_curItem--;
			rebuildPreBuffer();
			invalidateWindow(false);
		}
	} else if (_downSelected) {
		if (_curItem < (int)_itemArray.size() - 1) {
			_curItem++;
			rebuildPreBuffer();
			invalidateWindow(false);
		}
	}
}

bool InventoryWindow::isItemInInventory(int itemID) {
	for (uint i = 0; i < _itemArray.size(); i++)
		if (_itemArray[i] == itemID)
			return true;

	return false;
}

InventoryElement InventoryWindow::getItemStaticData(int itemID) {
	// TODO
	return InventoryElement();
}

bool InventoryWindow::destroyInfoWindow() {
	if (!_infoWindow)
		return false;

	delete _infoWindow;
	_infoWindow = 0;

	// TODO: Notify the scene view

	_magSelected = false;
	rebuildPreBuffer();
	invalidateWindow(false);

	return true;
}

} // End of namespace Buried
