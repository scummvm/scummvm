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

#include "darkseed/darkseed.h"
#include "darkseed/inventory.h"

namespace Darkseed {

constexpr uint16 MAX_INVENTORY = 42;
constexpr uint16 MAX_ICONS = 9;
static constexpr Common::Rect drawArea = {{139, 0}, 334, 40};

Inventory::Inventory() {
	_iconList.resize(MAX_ICONS);
	_inventory.resize(MAX_INVENTORY);
	reset();
}

void Inventory::reset() {
	_viewOffset = 0;
	_inventory[0] = 8;
	_inventoryLength = 1;
	update();
}

void Inventory::addItem(uint8 item) {
	_inventory[_inventoryLength] = item;
	_inventoryLength++;
	// TODO playsound(6, 5, -1);
	update();
}

void Inventory::removeItem(uint8 item) {
	for (int i = 0; i < _inventoryLength; i++) {
		if (_inventory[i] == item) {
			for (int j = i; j < _inventoryLength - 1; j++) {
				_inventory[j] = _inventory[j + 1];
			}
			_inventoryLength--;
			update();
			break;
		}
	}
}

void Inventory::update() {
	if (_viewOffset != 0) {
		if (_inventoryLength <= _viewOffset + (MAX_ICONS - 1)) {
			_viewOffset = _inventoryLength - (MAX_ICONS - 1);
		}
		if (_viewOffset > 50) {
			_viewOffset = 0;
		}
	}
	_iconList[0] = 4;
	if (_inventoryLength + 1 < MAX_ICONS) {
		_viewOffset = 0;
		for (int i = 0; i < _inventoryLength; i++) {
			_iconList[i + 1] = _inventory[i];
		}
	} else {
		for (int i = 0; i < MAX_ICONS - 1; i++) {
			_iconList[i + 1] = _inventory[_viewOffset + i];
		}
		if (_viewOffset + 8 < _inventoryLength) {
			_iconList[8] = 43;
		}
		if (_viewOffset != 0) {
			_iconList[1] = 42;
		}
	}
	_numIcons = MIN(_inventoryLength + 1, 9);
	_redraw = true;
}

void Inventory::restoreFrame() {
	g_engine->_frame.drawRect(drawArea);
	g_engine->_screen->addDirtyRect(drawArea);
}

void Inventory::draw() {
	if ((g_engine->_actionMode <= 4 && g_engine->_cursor.getY() > 40) || g_engine->_animation->_isPlayingAnimation_maybe || (g_engine->_objectVar[141] >= 1 && g_engine->_objectVar[141] <= 3)) {
		if (_isVisible) {
			restoreFrame();
			_isVisible = false;
		}
		return;
	}

	if (_redraw) {
		restoreFrame();
		_redraw = false;
	}

	_isVisible = true;
	for (int i = 0; i < _numIcons; i++) {
		int icon = _iconList[i];
		if (icon != 42 && icon != 43) {
			icon += 42;
		}

		if (g_engine->_actionMode == _iconList[i] && g_engine->_actionMode > 4) {
			const Sprite &selectedSprite = g_engine->_baseSprites.getSpriteAt(95);
			g_engine->_sprites.addSpriteToDrawList(139 + i * 37, 20 - selectedSprite._height / 2, &selectedSprite, 255, selectedSprite._width, selectedSprite._height, false);
		}
		const Sprite &iconSprite = g_engine->_baseSprites.getSpriteAt(icon);
		g_engine->_sprites.addSpriteToDrawList(140 + i * 37, 20 - iconSprite._height / 2, &iconSprite, 255, iconSprite._width, iconSprite._height, false);
	}
	g_engine->_screen->addDirtyRect(drawArea);
}

void Inventory::handleClick() {
	Common::Point clickPos = g_engine->_cursor.getPosition();
	if (clickPos.x < 140 || clickPos.x > 140 + _numIcons * 37) {
		return;
	}
	int iconIdx = (clickPos.x - 140) / 37;
	int icon = _iconList[iconIdx];
	if (icon == 42) {
		leftArrowClicked();
	} else if (icon == 43) {
		rightArrowClicked();
	} else if (icon == 4) {
		// TODO handle in-game menu
	} else if (icon == 21) {
		g_engine->_console->printTosText(935);
		g_engine->_objectVar[21] = 1;
		g_engine->_room->_collisionType = 1;
		g_engine->_room->removeObjectFromRoom(21);
		removeItem(21);
	} else if ((g_engine->_actionMode == 25 && icon == 20) ||
			   (g_engine->_actionMode == 20 && icon == 25)
			   ) {
		g_engine->handleObjCollision(icon);
	} else if (g_engine->_actionMode == kHandAction && icon == 35) {
		g_engine->_objectVar[35] = 0x7080;
		g_engine->_console->printTosText(669);
	} else if (g_engine->_actionMode == kLookAction) {
		g_engine->lookCode(icon);
	} else {
		g_engine->_actionMode = icon;
		g_engine->_console->printTosText(972);
		g_engine->_console->addToCurrentLine(Common::String::format("%s.", g_engine->_objectVar.getObjectName(icon)));
	}
}

void Inventory::leftArrowClicked() {
	if (_viewOffset > 0) {
		_viewOffset--;
		update();
	}
}

void Inventory::rightArrowClicked() {
	_viewOffset++;
	update();
}

Common::Error Inventory::sync(Common::Serializer &s) {
	s.syncAsSint16LE(_inventoryLength);
	for (int i = 0; i < _inventoryLength; i++) {
		s.syncAsByte(_inventory[i]);
	}
	_viewOffset = 0;
	update();
	return Common::kNoError;
}

void Inventory::endOfDayOutsideLogic() {
	for (int i = 0; i < _inventoryLength; i++) {
		g_engine->_objectVar.setMoveObjectRoom(_inventory[i], _inventory[i] == 28 ? 255 : 252);
	}
	_inventoryLength = 0;
	_viewOffset = 0;
	g_engine->_objectVar[53] = 2;
}

void Inventory::gotoJailLogic() {
	for (int i = 0; i < _inventoryLength; i++) {
		g_engine->_objectVar.setMoveObjectRoom(_inventory[i], 100);
	}
	g_engine->_objectVar.setMoveObjectRoom(28, 255);
	_inventoryLength = 0;
	_viewOffset = 0;
	update();
}

bool Inventory::hasObject(uint8 objNum) {
	for (int i = 0; i < _inventoryLength; i++) {
		if (_inventory[i] == objNum) {
			return true;
		}
	}
	return false;
}

} // End of namespace Darkseed
