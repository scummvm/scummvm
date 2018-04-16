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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/inv.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/systemui.h"

namespace Agi {

InventoryMgr::InventoryMgr(AgiEngine *agi, GfxMgr *gfx, TextMgr *text, SystemUI *systemUI) {
	_vm = agi;
	_gfx = gfx;
	_text = text;
	_systemUI = systemUI;

	_activeItemNr = -1;
}

InventoryMgr::~InventoryMgr() {
}

void InventoryMgr::getPlayerInventory() {
	AgiGame game = _vm->_game;
	int16 selectedInventoryItem = _vm->getVar(VM_VAR_SELECTED_INVENTORY_ITEM);
	uint16 objectNr = 0;
	int16 curRow = 2; // starting at position 2,1
	int16 curColumn = 1;

	_array.clear();
	_activeItemNr = 0;

	for (objectNr = 0; objectNr < game.numObjects; objectNr++) {
		if (_vm->objectGetLocation(objectNr) == EGO_OWNED) {
			// item is in the possession of ego, so add it to our internal list
			if (objectNr == selectedInventoryItem) {
				// it's the currently selected inventory item, remember that
				_activeItemNr = _array.size();
			}

			InventoryEntry inventoryEntry;

			inventoryEntry.objectNr = objectNr;
			inventoryEntry.name = _vm->objectName(objectNr);
			inventoryEntry.row = curRow;
			inventoryEntry.column = curColumn;
			if (inventoryEntry.column > 1) {
				// right side, adjust column accordingly
				inventoryEntry.column -= strlen(inventoryEntry.name);
			}
			_array.push_back(inventoryEntry);

			// go to next position
			if (curColumn == 1) {
				// current position is left side, go to right side
				curColumn = 39;
			} else {
				// current position is right side, so go to left side again and new row
				curColumn = 1;
				curRow++;
			}
		}
	}

	if (_array.size() == 0) {
		// empty inventory
		InventoryEntry inventoryEntry;

		inventoryEntry.objectNr = 0;
		inventoryEntry.name = _systemUI->getInventoryTextNothing();
		inventoryEntry.row = 2;
		inventoryEntry.column = 19 - (strlen(inventoryEntry.name) / 2);
		_array.push_back(inventoryEntry);
	}
}

void InventoryMgr::drawAll() {
	int16 inventoryCount = _array.size();
	int16 inventoryNr = 0;

	_text->charPos_Set(0, 11);
	_text->displayText(_systemUI->getInventoryTextYouAreCarrying());

	for (inventoryNr = 0; inventoryNr < inventoryCount; inventoryNr++) {
		drawItem(inventoryNr);
	}
}

void InventoryMgr::drawItem(int16 itemNr) {
	if (itemNr == _activeItemNr) {
		_text->charAttrib_Set(15, 0);
	} else {
		_text->charAttrib_Set(0, 15);
	}

	_text->charPos_Set(_array[itemNr].row, _array[itemNr].column);
	// original interpreter used printf here
	// this doesn't really make sense, because for length calculation it's using strlen without printf
	// which means right-aligned inventory items on the right side would not be displayed properly
	// in case printf-formatting was actually used
	// I have to assume that no game uses this, because behavior in original interpreter would have been buggy.
	_text->displayText(_array[itemNr].name);
}

void InventoryMgr::show() {
	bool selectItems = false;

	// figure out current inventory of the player
	getPlayerInventory();

	if (_vm->getFlag(VM_FLAG_STATUS_SELECTS_ITEMS)) {
		selectItems = true;
	} else {
		_activeItemNr = -1; // so that none is shown as active
	}

	drawAll();

	_text->charAttrib_Set(0, 15);
	if (selectItems) {
		_text->charPos_Set(24, 2);
		_text->displayText(_systemUI->getInventoryTextSelectItems());
	} else {
		_text->charPos_Set(24, 4);
		_text->displayText(_systemUI->getInventoryTextReturnToGame());
	}

	if (selectItems) {
		_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_INVENTORY);

		do {
			_vm->processAGIEvents();
		} while (_vm->cycleInnerLoopIsActive() && !(_vm->shouldQuit() || _vm->_restartGame));

		if (_activeItemNr >= 0) {
			// pass selected object number
			_vm->setVar(VM_VAR_SELECTED_INVENTORY_ITEM, _array[_activeItemNr].objectNr);
		} else {
			// nothing was selected
			_vm->setVar(VM_VAR_SELECTED_INVENTORY_ITEM, 0xff);
		}

	} else {
		// no selection is supposed to be possible, just wait for key and exit
		_vm->waitAnyKey();
	}
}

void InventoryMgr::keyPress(uint16 newKey) {
	switch (newKey) {
	case AGI_KEY_ENTER: {
		_vm->cycleInnerLoopInactive(); // exit show-loop
		break;
	}

	case AGI_KEY_ESCAPE: {
		_vm->cycleInnerLoopInactive(); // exit show-loop
		_activeItemNr = -1; // no item selected
		break;
	}

	case AGI_KEY_UP:
		changeActiveItem(-2);
		break;
	case AGI_KEY_DOWN:
		changeActiveItem(+2);
		break;
	case AGI_KEY_LEFT:
		changeActiveItem(-1);
		break;
	case AGI_KEY_RIGHT:
		changeActiveItem(+1);
		break;

	default:
		break;
	}
}

void InventoryMgr::changeActiveItem(int16 direction) {
	int16 orgItemNr = _activeItemNr;

	_activeItemNr += direction;

	if ((_activeItemNr >= 0) && (_activeItemNr < (int16)_array.size())) {
		// within bounds
		drawItem(orgItemNr);
		drawItem(_activeItemNr);
	} else {
		// out of bounds, revert change
		_activeItemNr = orgItemNr;
	}
}

} // End of namespace Agi
