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

#include "access/noctropolis/noctropolis_inventory.h"
#include "access/access.h"
#include "access/resources.h"

namespace Access {

namespace Noctropolis {

NoctropolisInventory::NoctropolisInventory(AccessEngine *vm) : InventoryManager(vm) {
}


int NoctropolisInventory::displayInv() {
	int selectedItem = -1;
	warning("TODO: finish NoctropolisInventory::displayInv");
	/*
	const byte kDownArrow   = 1;
	const byte kUpArrow     = 2;

	int hoveredItem = -1;
	byte slotItems[20];
	Common::Array<byte> items;
	SpriteResource *inventorySprites;
	StringResource *inventoryItemNames;
	GrabRect savedRect;
	bool upArrow, downArrow;
	uint prevInventoryTopItemIndex = -1;

	if (!_keepInventoryPosition) {
		int16 warpMouseX, warpMouseY;
		_inventoryBaseX = clipMouseCenter(_vm->_events->_mousePos.x, 252, 640, warpMouseX);
		_inventoryBaseY = clipMouseCenter(_vm->_events->_mousePos.y, 220, 400, warpMouseY);
		_keepInventoryPosition = true;
		if (warpMouseX != _vm->_events->_mousePos.x || warpMouseY != _vm->_events->_mousePos.y)
			_system->warpMouse(warpMouseX, warpMouseY);
	}

	// TODO: Maybe move/load these globally?

	inventorySprites = new SpriteResource();
	_vm->_res->load(inventorySprites, "INV.AP");

	inventoryItemNames = new StringResource();
	_vm->_res->load(inventoryItemNames, GID_NOCTROPOLIS, kResStringTable, 3);

	// Build the list of available inventory items
	for (int i = 0; i < 256; i++) {
		if (getInventoryItem(i) == 1)
			items.push_back(i);
	}

	_vgaScreen->lock();
	// Grab screen background
	_vgaScreen->saveRect(savedRect, _inventoryBaseX, _inventoryBaseY, 252, 220);
	// Draw inventory panel
	_vgaScreen->drawSprite(_icons, 0, _inventoryBaseX, _inventoryBaseY);
	_vgaScreen->unlock();

	prevInventoryTopItemIndex = -1;

	while (selectedItem == -1) {

		int x = _events->_mousePos.x - _inventoryBaseX - 23;
		int y = _events->_mousePos.y - _inventoryBaseY - 26;

		if (prevInventoryTopItemIndex != _inventoryTopItemIndex) {

			int slotIndex = 0, slotCount = 20, skipIndex = 0;

			memset(slotItems, 0, 20);

			prevInventoryTopItemIndex = _inventoryTopItemIndex;
			upArrow = false;
			downArrow = false;

			if (_inventoryTopItemIndex > 0) {
				upArrow = true;
				slotItems[slotIndex++] = kUpArrow;
				slotCount--;
				skipIndex = 1;
			}

			if (_inventoryTopItemIndex + slotCount + skipIndex < items.size()) {
				downArrow = true;
				slotItems[19] = kDownArrow;
				slotCount--;
			} else {
				slotCount = items.size() - _inventoryTopItemIndex - skipIndex;
			}

			for (int i = 0; i < slotCount; i++)
				slotItems[slotIndex++] = items[skipIndex + _inventoryTopItemIndex + i];

			_vgaScreen->lock();
			for (int slotNum = 0; slotNum < 20; slotNum++) {
				int spriteIndex = slotItems[slotNum];
				if (spriteIndex > 2)
					spriteIndex++;
				uint16 slotX = _inventoryBaseX + 23 + (slotNum % 5) * 41;
				uint16 slotY = _inventoryBaseY + 26 + (slotNum / 5) * 38;
				_vgaScreen->drawSprite(inventorySprites, 3, slotX, slotY); // draw empty slot
				slotX += 20 - inventorySprites->getSprite(spriteIndex)->width / 2;
				slotY += MAX(0, 38 - inventorySprites->getSprite(spriteIndex)->height) / 2;
				_vgaScreen->drawSprite(inventorySprites, spriteIndex, slotX, slotY);
				//_vgaScreen->drawText(_fonts[1], (const byte*)Common::String::format("%d", spriteIndex).c_str(), slotX, slotY, 238, 246, kFontBackground);
			}
			_vgaScreen->unlock();

		}

		// Support the mouse wheel for scrolling through the inventory items
		if (upArrow && _mouseWheelDelta < 0) {
			_inventoryTopItemIndex = MAX<int>(_inventoryTopItemIndex - 1, 0);
		} else if (downArrow && _mouseWheelDelta > 0) {
			_inventoryTopItemIndex = MIN<int>(_inventoryTopItemIndex + 1, items.size());
		}

		if (x >= 0 && x < 205 && y >= 0 && y < 152) {
			int slotIndex = (x / 41) + (y / 38) * 5;
			int item = slotItems[slotIndex];

			if (hoveredItem != item) {
				//debug("slotIndex = %d; item = %d", slotIndex, item);
				hoveredItem = item;
				_vgaScreen->lock();
				_vgaScreen->fillRect(_inventoryBaseX + 66, _inventoryBaseY + 184, _inventoryBaseX + 66 + 146, _inventoryBaseY + 184 + 8, 246);
				if (item != 0) {
					_vgaScreen->drawText(_fonts[1], inventoryItemNames->getString(item), _inventoryBaseX + 66, _inventoryBaseY + 184, 238, 246, kFontBackground);
				}
				_vgaScreen->unlock();
			}

			if (leftMouseButton()) {
				if (item == kDownArrow)
					_inventoryTopItemIndex = MIN<int>(_inventoryTopItemIndex + 1, items.size());
				else if (item == kUpArrow)
					_inventoryTopItemIndex = MAX<int>(_inventoryTopItemIndex - 1, 0);
				else
					selectedItem = item;
				waitUntilLeftButtonIsReleased();
			}

		}

		if (rightMouseButton()) {
			selectedItem = 255;
			waitUntilRightButtonIsReleased();
			_keepInventoryPosition = false;
		}

		updateEvents();
		updateScreen();

	}

	_flags[99] = selectedItem;

	// Restore screen background
	_vgaScreen->lock();
	_vgaScreen->restoreRect(savedRect);
	_vgaScreen->unlock();

	delete inventorySprites;
	delete inventoryItemNames;
	*/

	return selectedItem;
}


} // end namespace Noctropolis

} // end namespace Access
