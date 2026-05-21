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
#include "access/asurface.h"

namespace Access {

namespace Noctropolis {

NoctropolisInventory::NoctropolisInventory(AccessEngine *vm) : InventoryManager(vm),
_keepInventoryPosition(false), _topItemIndex(0) {
}


int NoctropolisInventory::displayInv() {
	int selectedItem = -1;

	const byte kDownArrow   = 1;
	const byte kUpArrow     = 2;

	int hoveredItem = -1;
	byte slotItems[20];
	bool upArrow = false;
	bool downArrow = false;
	int prevTopItemIndex = -1;

	Screen *screen = _vm->_screen;

	setUseItem(-1);

	if (!_keepInventoryPosition) {
		int16 warpMouseX, warpMouseY;
		_inventoryBase.x = _vm->_events->clipMouseCenter(_vm->_events->_mousePos.x, 252, 640, warpMouseX);
		_inventoryBase.y = _vm->_events->clipMouseCenter(_vm->_events->_mousePos.y, 220, 400, warpMouseY);
		_keepInventoryPosition = true;
		if (warpMouseX != _vm->_events->_mousePos.x || warpMouseY != _vm->_events->_mousePos.y)
			g_system->warpMouse(warpMouseX, warpMouseY);
	}

	// TODO: Maybe move/load these globally?
	Resource *iconData = _vm->_files->loadRawFile("INV.AP");
	SpriteResource *inventorySprites = new SpriteResource(_vm, iconData);
	delete iconData;

	// Build the list of available inventory items
	getList();

	// Grab screen background
	screen->saveBlock(Common::Rect(_inventoryBase, 252, 220));

	// Draw inventory panel
	screen->plotImage(_vm->getIcons(), 0, _inventoryBase);

	const Font *font = _vm->_fonts.getFont(1);

	const int itemXOff = (_vm->getLanguage() == Common::DE_DEU ? 60 : 66);

	// TODO: Check these colours.
	Font::_fontColors[0] = 246;
	Font::_fontColors[1] = 238;

	while (selectedItem == -1 && !_vm->shouldQuitOrRestart()) {
		int x = _vm->_events->_mousePos.x - _inventoryBase.x - 23;
		int y = _vm->_events->_mousePos.y - _inventoryBase.y - 26;

		if (prevTopItemIndex != _topItemIndex) {

			int slotIndex = 0, slotCount = 20, skipIndex = 0;

			memset(slotItems, 0, 20);

			prevTopItemIndex = _topItemIndex;
			upArrow = false;
			downArrow = false;

			if (_topItemIndex > 0) {
				upArrow = true;
				slotItems[slotIndex++] = kUpArrow;
				slotCount--;
				skipIndex = 1;
			}

			if (_topItemIndex + slotCount + skipIndex < (int)_items.size()) {
				downArrow = true;
				slotItems[19] = kDownArrow;
				slotCount--;
			} else {
				slotCount = _items.size() - _topItemIndex - skipIndex;
			}

			for (int i = 0; i < slotCount; i++)
				slotItems[slotIndex++] = _items[skipIndex + _topItemIndex + i];

			for (int slotNum = 0; slotNum < 20; slotNum++) {
				int spriteIndex = slotItems[slotNum];
				if (spriteIndex > 2)
					spriteIndex++;
				uint16 slotX = _inventoryBase.x + 23 + (slotNum % 5) * 41;
				uint16 slotY = _inventoryBase.y + 26 + (slotNum / 5) * 38;
				screen->plotF(inventorySprites->getFrame(3), Common::Point(slotX, slotY)); // draw empty slot
				// Centre the item icon
				slotX += MAX(0, 20 - inventorySprites->getFrame(spriteIndex)->w / 2);
				slotY += MAX(0, 19 - inventorySprites->getFrame(spriteIndex)->h / 2);
				const Common::Point slotPt(slotX, slotY);
				screen->plotF(inventorySprites->getFrame(spriteIndex), slotPt);
				// Useful for debugging - draw sprite number
				//font->drawString(screen, Common::String::format("%d", spriteIndex), slotPt);
			}

		}

		// Support the mouse wheel for scrolling through the inventory items
		if (upArrow && _vm->_events->_wheelUp) {
			_startInvItem = MAX<int>(_topItemIndex - 1, 0);
		} else if (downArrow && _vm->_events->_wheelDown) {
			_startInvItem = MIN<int>(_topItemIndex + 1, _items.size());
		}

		if (x >= 0 && x < 205 && y >= 0 && y < 152) {
			int slotIndex = (x / 41) + (y / 38) * 5;
			int item = slotItems[slotIndex];

			if (hoveredItem != item) {
				//debug("slotIndex = %d; item = %d", slotIndex, item);
				hoveredItem = item;
				const Common::Point itemBase = Common::Point(_inventoryBase.x + itemXOff, _inventoryBase.y + 184);
				screen->fillRect(Common::Rect(itemBase, 146, 8), 246);
				if (item != 0) {
					font->drawString(screen, _inv[item]._name, itemBase);
				}
			}

			if (_vm->_events->_leftButton) {
				if (item == kDownArrow)
					_topItemIndex = MIN<int>(_topItemIndex + 1, _items.size());
				else if (item == kUpArrow)
					_topItemIndex = MAX<int>(_topItemIndex - 1, 0);
				else if (item)
					selectedItem = item;
				_vm->_events->debounceLeft();
			}

		}

		if (_vm->_events->_rightButton) {
			selectedItem = -1;
			_vm->_events->debounceRight();
			_keepInventoryPosition = false;
			break;
		}

		_vm->_events->pollEventsAndWait();
	}

	if (selectedItem == 0)
		selectedItem = -1;

	setUseItem(selectedItem);

	// Restore screen background
	_vm->_screen->restoreBlock();

	delete inventorySprites;

	return selectedItem;
}


} // end namespace Noctropolis

} // end namespace Access
