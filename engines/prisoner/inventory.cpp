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
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

int16 PrisonerEngine::registerInventoryItem(Common::String &pakName, int16 pakSlot, int16 id) {
	int16 inventoryItemIndex = _inventoryItems.getFreeSlot();
	InventoryItem *inventoryItem = &_inventoryItems[inventoryItemIndex];
	inventoryItem->resourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 9);
	inventoryItem->id = id;
	inventoryItem->status = 0;
	loadInventoryItemText(inventoryItemIndex);
	return inventoryItemIndex;
}

void PrisonerEngine::loadInventoryItemText(int16 inventoryItemIndex) {
	InventoryItem *inventoryItem = &_inventoryItems[inventoryItemIndex];
	Common::String textPakName = Common::String::format("%c_M%02d", _languageChar, _currModuleIndex);
	Common::String identifier = Common::String::format("%02d", inventoryItemIndex);
	int16 inventoryItemNamesSlot = loadTextResource(textPakName, 0);
	TextResource *inventoryItemNames = _res->get<TextResource>(inventoryItemNamesSlot);
	inventoryItem->name = inventoryItemNames->getText(identifier)->getChunkLineString(0, 0);
	_res->unload(inventoryItemNamesSlot);
}

int16 PrisonerEngine::addInventoryItemCombination(int16 inventoryItem1, int16 inventoryItem2, int16 scriptIndex) {

	debug(1, "PrisonerEngine::addInventoryItemCombination(%d, %d, %d)", inventoryItem1, inventoryItem2, scriptIndex);

	int16 combinationIndex = _inventoryItemCombinations.getFreeSlot();
	InventoryItemCombination *inventoryItemCombination = &_inventoryItemCombinations[combinationIndex];

	inventoryItemCombination->used = 1;
	inventoryItemCombination->inventoryItem1 = inventoryItem1;
	inventoryItemCombination->inventoryItem2 = inventoryItem2;
	inventoryItemCombination->scriptIndex = scriptIndex;
	_inventoryItems[inventoryItem1].combinationIndex = combinationIndex;
	_inventoryItems[inventoryItem2].combinationIndex = combinationIndex;

	return combinationIndex;
}

void PrisonerEngine::removeInventoryItemCombination(int16 combinationIndex) {
	InventoryItemCombination *inventoryItemCombination = &_inventoryItemCombinations[combinationIndex];
	inventoryItemCombination->used = 0;
	_inventoryItems[inventoryItemCombination->inventoryItem1].combinationIndex = -1;
	_inventoryItems[inventoryItemCombination->inventoryItem2].combinationIndex = -1;
}

int16 PrisonerEngine::getInventoryItemCombinationScript(int16 inventoryItem1, int16 inventoryItem2) {
	if (_inventoryItems[inventoryItem1].combinationIndex >= 0 &&
		_inventoryItems[inventoryItem2].combinationIndex >= 0 &&
		_inventoryItems[inventoryItem1].combinationIndex == _inventoryItems[inventoryItem2].combinationIndex) {
		return _inventoryItemCombinations[_inventoryItems[inventoryItem1].combinationIndex].scriptIndex;
	} else {
		return -1;
	}
}

void PrisonerEngine::addItemToInventory(int16 inventoryItemIndex) {
	InventoryItem *inventoryItem = &_inventoryItems[inventoryItemIndex];
	if (inventoryItem->status != 1) {
		inventoryItem->status = 1;
		_inventoryItemsCount++;
		removeSceneItem(inventoryItemIndex);
	}
}

void PrisonerEngine::removeItemFromInventory(int16 inventoryItemIndex) {
	InventoryItem *inventoryItem = &_inventoryItems[inventoryItemIndex];
	inventoryItem->status = -1;
	_inventoryItemsCount--;
	if (_inventoryItemCursor == inventoryItemIndex)
		_inventoryItemCursor = -1;
}

void PrisonerEngine::clearInventoryItems() {
	_inventoryItemsCount = 0;
	_inventoryItemsResourceCacheIndex = -1;
	_inventoryItemCursor = -1;
	_inventoryItems.clear();
	_inventoryItemCombinations.clear();
}

void PrisonerEngine::unloadInventoryItems() {
	for (int16 i = 0; i < _inventoryItems.count(); i++) {
		InventoryItem *inventoryItem = &_inventoryItems[i];
		if (inventoryItem->resourceCacheSlot != -1)
			_res->unload(inventoryItem->resourceCacheSlot);
	}
	if (_inventoryItemsResourceCacheIndex != -1)
		_res->unload(_inventoryItemsResourceCacheIndex);
}

/* Inventory bar */

void PrisonerEngine::loadInventoryItemsAnimation(Common::String &pakName, int16 pakSlot, int16 slotBaseIndex) {
	if (_inventoryItemsResourceCacheIndex != -1)
		_res->unload(_inventoryItemsResourceCacheIndex);
	_inventoryItemsResourceCacheIndex = _res->load<AnimationResource>(pakName, pakSlot, 11);
	_inventoryItemSlotBaseIndex = slotBaseIndex;
}

void PrisonerEngine::updateInventoryItems() {

	int16 clickBoxIndex = 0, x = 0;
	AnimationResource *inventoryAnimationResource = _res->get<AnimationResource>(_inventoryItemsResourceCacheIndex);

	setFontColors(_textFont, _inventoryFontColor.outlineColor, _inventoryFontColor.inkColor);
	setActiveFont(_textFont);

	for (int16 i = 0; i < kMaxInventoryItems; i++) {
		InventoryItem *inventoryItem = &_inventoryItems[i];
		if (inventoryItem->status == 1) {
			AnimationResource *inventoryItemAnimationResource;
			if (_buildInventoryClickBoxes) {
				// TODO
				addClickBox(x, 41, x + 39, 81, i);
				if (isPointInClickBox(clickBoxIndex, _mouseX, _mouseY)) {
					_selectedInventoryItemIndex = i;
					_inventoryClickBoxIndex = clickBoxIndex;
				}
				if (_selectedInventoryItemIndex == i)
					_inventoryClickBoxIndex = clickBoxIndex;
			}
			if (_inventoryClickBoxIndex == clickBoxIndex) {
				_selectedInventoryItemIndex = i;
				_screen->drawAnimationElement(inventoryAnimationResource,
					_currInventoryItemSlotBaseIndex + 1, x, 41, 0);
				drawTextEx(0, -1, 398, 479, inventoryItem->name);
				// Warp mouse
				if (_inventoryWarpMouse) {
					_system->warpMouse(x + 20, 61);
					_inventoryWarpMouse = false;
				}
			} else {
				_screen->drawAnimationElement(inventoryAnimationResource,
					_currInventoryItemSlotBaseIndex, x, 41, 0);
			}

			inventoryItemAnimationResource = _res->get<AnimationResource>(inventoryItem->resourceCacheSlot);
			_screen->drawAnimationElement(inventoryItemAnimationResource,
				inventoryItemAnimationResource->_anims[inventoryItem->id]->frames[0]->elementIndex,
				x, 41, 0);

			clickBoxIndex++;
			x += 40;
		}
	}

	_buildInventoryClickBoxes = false;
	addDirtyRect(0, 0, 640, 82, 1);
	addDirtyRect(0, 398, 640, 480, 1);

}

void PrisonerEngine::handleInventoryInput() {

	bool canLeave = true, backedupScreen;
	int16 oldInventoryItemCursor;

	// TODO: resetDirtyRects();
	_inventoryWarpMouse = false;
	_buildInventoryClickBoxes = true;

	_screen->fillRect(0, 0, 639, 81, 0);
	_screen->fillRect(0, 398, 639, 479, 0);
	backedupScreen = backupScreen();

	oldInventoryItemCursor = _inventoryItemCursor;

	// TODO: Keyboard stuff
	inpSetWaitRelease(true);

	_currInventoryItemSlotBaseIndex = _inventoryItemSlotBaseIndex;
	_clickBoxes.clear();

	if (_inventoryItemCursor != -1) {
		_selectedInventoryItemIndex = _inventoryItemCursor;
	} else {
		_selectedInventoryItemIndex = 0;
	}

	_inventoryBarFlag = false;

	while (1) {
		Common::KeyCode keyState;
		uint16 buttonState;
		int16 clickBoxIndex;

		updateEvents();

		if (canLeave) {
			if (_mouseY >= 82) {
				_inventoryBarFlag = true;
				debug("break");
				break;
			}
		} else {
			canLeave = true;
		}

		getInputStatus(keyState, buttonState);

		_screen->fillRect(0, 398, 639, 479, 0);

		clickBoxIndex = findClickBoxAtPos(_mouseX, _mouseY, -1);

		if (clickBoxIndex != -1) {
			_selectedInventoryItemIndex = getClickBoxTag(clickBoxIndex);
			_inventoryClickBoxIndex = clickBoxIndex;
		}

		updateInventoryItems();
		// TODO: drawDirtyRects(NULL);

		if ((buttonState & kLeftButton) && clickBoxIndex != -1) {
			if (_inventoryItemCursor != _selectedInventoryItemIndex) {
				if (_inventoryItemCursor != -1) {
					int16 combinationScript = getInventoryItemCombinationScript(_inventoryItemCursor, _selectedInventoryItemIndex);
					if (combinationScript != -1) {
						startScript(kModuleScriptProgram, combinationScript);
						_inventoryItemCursor = oldInventoryItemCursor;
						break;
					}
				} else {
					_inventoryItemCursor = _selectedInventoryItemIndex;
					_currInventoryItemCursor = -1;
				}
			} else {
				_inventoryItemCursor = -1;
				_currInventoryItemCursor = -1;
			}
			inpSetWaitRelease(true);
		} else if (buttonState & kRightButton) {
			if (_inventoryItemCursor == -1) {
				Common::String textPakName = Common::String::format("%c_M%02d", _languageChar, _currModuleIndex);
				Common::String identifier = Common::String::format("LOOK%02d", _selectedInventoryItemIndex);
				int16 textResourceSlot = loadTextResource(textPakName, 0);

				_inventoryActive = true;

				addCenteredScreenText(textResourceSlot, identifier);

				updateMouseCursor();
				inpSetWaitRelease(true);
				restoreScreen(true);

				while (_screenTextShowing) {
					updateEvents();
					getInputStatus(keyState, buttonState);
					updateBackground(true);
					updateInventoryItems();
					_screen->setClipRect(0, 82, 639, 397);
					drawSprites(_cameraX, _cameraY);
					updateScreenTexts();
					_screen->setClipRect(0, 0, 639, 479);
					updateMouseCursorAnimation();
					if (buttonState != 0 || _autoAdvanceScreenTexts) {
						advanceScreenTexts();
						inpSetWaitRelease(true);
						restoreScreen(true);
						updateInventoryItems();
					}
					// TODO: drawDirtyRects(false);
					_screen->update();
					_system->delayMillis(10);
				}

				_inventoryActive = false;
				_inventoryWarpMouse = true;
				canLeave = false;

			} else {
				_inventoryItemCursor = -1;
			}
			inpSetWaitRelease(true);
		}

		updateMouseCursor();

		_screen->update();
		_system->delayMillis(10);

	}

	restoreScreen(backedupScreen);

	updateScreen(true, -1, -1);

	debug("inv done");

}

} // End of namespace Prisoner
