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

#include "darkseed.h"
#include "inventory.h"

constexpr uint16 MAX_INVENTORY = 42;
constexpr uint16 MAX_ICONS = 9;

Darkseed::Inventory::Inventory() {
	_iconList.resize(MAX_ICONS);
	_inventory.resize(MAX_INVENTORY);
	reset();
}

void Darkseed::Inventory::reset() {
	_viewOffset = 0;
	_inventory[0] = 8;
	_inventoryLength = 1;
}

void Darkseed::Inventory::addItem(uint8 item) {
	_inventory[_inventoryLength] = item;
	_inventoryLength++;
	// TODO playsound(6, 5, -1);
}

void Darkseed::Inventory::draw() {
	if (g_engine->_cursor.getY() > 40 || g_engine->isPlayingAnimation_maybe || (g_engine->_objectVar[141] >= 1 && g_engine->_objectVar[141] <= 3)) {
		return;
	}
	if (_viewOffset != 0) {
		if (_inventoryLength <= _viewOffset + MAX_ICONS) {
			_viewOffset = _inventoryLength - MAX_ICONS;
		}
		if (_viewOffset > 50) {
			_viewOffset = 0;
		}
	}
	_iconList[0] = 4;
	if (_inventoryLength + 1 < MAX_ICONS) {
		_viewOffset = 0;
		for (int i = 0; i < _inventoryLength; i++) {
			_iconList[i+1] = _inventory[i];
		}
	} else {
		for (int i = 0; i < MAX_ICONS; i++) {
			_iconList[i+1] = _inventory[_viewOffset + i];
		}
		if (_viewOffset + 8 < _inventoryLength) {
			_iconList[8] = 43;
		}
		if (_viewOffset != 0) {
			_iconList[1] = 42;
		}
	}
	int numIcons = MIN(_inventoryLength + 1, 9);

	for(int i = 0; i < numIcons; i++) {
		int icon = _iconList[i];
		if (icon != 42 && icon != 43) {
			icon += 42;
		}

		const Sprite &animSprite = g_engine->_baseSprites.getSpriteAt(icon);
		g_engine->_sprites.addSpriteToDrawList(140 + i * 37, 20 - animSprite.height / 2, &animSprite, 255, animSprite.width, animSprite.height, false);
	}
}
