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

#include "kyra/kyra_v2.h"

namespace Kyra {

int KyraEngine_v2::findFreeItem() {
	for (int i = 0; i < 30; ++i) {
		if (_itemList[i].id == 0xFFFF)
			return i;
	}
	return -1;
}

int KyraEngine_v2::countAllItems() {
	int num = 0;
	for (int i = 0; i < 30; ++i) {
		if (_itemList[i].id != 0xFFFF)
			++num;
	}
	return num;
}

int KyraEngine_v2::findItem(uint16 sceneId, uint16 id) {
	for (int i = 0; i < 30; ++i) {
		if (_itemList[i].id == id && _itemList[i].sceneId == sceneId)
			return i;
	}
	return -1;
}

int KyraEngine_v2::checkItemCollision(int x, int y) {
	int itemPos = -1, yPos = -1;

	for (int i = 0; i < 30; ++i) {
		const Item &curItem = _itemList[i];

		if (curItem.id == 0xFFFF || curItem.sceneId != _mainCharacter.sceneId)
			continue;

		int itemX1 = curItem.x - 8 - 3;
		int itemX2 = curItem.x + 7 + 3;

		if (x < itemX1 || x > itemX2)
			continue;

		int itemY1 = curItem.y - _itemHtDat[curItem.id] - 3;
		int itemY2 = curItem.y + 3;

		if (y < itemY1 || y > itemY2)
			continue;

		if (curItem.y >= yPos) {
			itemPos = i;
			yPos = curItem.y;
		}
	}
	
	return itemPos;
}

void KyraEngine_v2::resetItemList() {
	for (int i = 0; i < 30; ++i) {
		_itemList[i].id = 0xFFFF;
		_itemList[i].sceneId = 0xFFFF;
		_itemList[i].x = 0;
		_itemList[i].y = 0;
		_itemList[i].unk7 = 0;
	}
}

bool KyraEngine_v2::dropItem(int unk1, uint16 item, int x, int y, int unk2) {
	if (_handItemSet <= -1)
		return false;

	bool success = processItemDrop(_mainCharacter.sceneId, item, x, y, unk1, unk2);
	if (!success) {
		//snd_playSfx(0x0d);
		if (countAllItems() >= 30)
			showMessageFromCCode(5, 0x84, 0);
	}

	return success;
}

bool KyraEngine_v2::processItemDrop(uint16 sceneId, uint16 item, int x, int y, int unk1, int unk2) {
	int itemPos = checkItemCollision(x, y);

	if (unk1)
		itemPos = -1;

	if (itemPos >= 0) {
		exchangeMouseItem(itemPos);
		return false;
	}

	//XXX
	return false;
}

void KyraEngine_v2::exchangeMouseItem(int itemPos) {
	_screen->hideMouse();

	deleteItemAnimEntry(itemPos);

	int itemId = _itemList[itemPos].id;
	_itemList[itemPos].id = _itemInHand;
	_itemInHand = itemId;

	addItemToAnimList(itemPos);
	//snd_playSfx(0x0b);
	setMouseCursor(_itemInHand);
	int str2 = 7;
		
	if (_lang == 1)
		str2 = getItemCommandString(itemId);
	
	updateCommandLineEx(itemId + 54, str2, 0xD6);
	_screen->showMouse();

	runSceneScript6();
}

bool KyraEngine_v2::pickUpItem(int x, int y) {
	int itemPos = checkItemCollision(x, y);
	
	if (itemPos <= -1)
		return false;

	if (_itemInHand >= 0) {
		exchangeMouseItem(itemPos);
	} else {
		_screen->hideMouse();
		deleteItemAnimEntry(itemPos);
		int itemId = _itemList[itemPos].id;
		_itemList[itemPos].id = 0xFFFF;
		//snd_playSfx(0x0b);
		setMouseCursor(itemId);
		int str2 = 7;
		
		if (_lang == 1)
			str2 = getItemCommandString(itemId);
		
		updateCommandLineEx(itemId + 54, str2, 0xD6);
		_itemInHand = itemId;
		_screen->showMouse();

		runSceneScript6();
	}

	return true;
}

int KyraEngine_v2::getItemCommandString(uint16 item) {
	// This is just needed for French version
	static const uint8 index[] = {
		2,    2,    0,    0,    2,    2,    2,    0,
		2,    2,    0,    0,    0,    2,    0,    0,
		0,    0,    0,    0,    2,    0,    0,    0,
		0,    1,    0,    2,    2,    2,    2,    0,
		3,    0,    3,    2,    2,    2,    3,    2,
		2,    2,    0,    0,    0,    0,    0,    0,
		0,    0,    0,    0,    2,    0,    0,    0,
		0,    0,    0,    0,    0,    2,    0,    0,
		2,    0,    0,    0,    0,    0,    0,    2,
		2,    0,    0,    0,    2,    2,    2,    2,
		2,    2,    2,    2,    2,    2,    2,    2,
		2,    2,    2,    2,    2,    2,    2,    0,
		2,    2,    2,    0,    0,    1,    3,    2,
		2,    2,    2,    2,    2,    0,    0,    0,
		0,    2,    2,    1,    0,    1,    2,    0,
		0,    0,    0,    0,    0,    2,    2,    2,
		2,    2,    2,    2,    0,    2,    2,    2,
		2,    3,    2,    0,    0,    0,    0,    1,
		2,    0,    0,    0,    0,    0,    0,    0,
		0,    0,    0,    0,    0,    0,    0,    0,
		2,    2,    0,    0,    0,    0,    0,    2,
		0,    2,    0,    0,    0,    0,    0,    0
	};

	assert(item < ARRAYSIZE(index));

	static const int stringId[] = {
		0x02B, 0x102, 0x007, 0x105, 0x02D, 0x103,
		0x003, 0x106, 0x02C, 0x104, 0x008, 0x107
	};

	assert(index[item] < ARRAYSIZE(index));
	return stringId[index[item]];
}

void KyraEngine_v2::setMouseCursor(uint16 item) {
	int shape = 0;
	int hotX = 1;
	int hotY = 1;

	if (item != 0xFFFF) {
		hotX = 8;
		hotY = 15;
		shape = item+64;
	}

	_screen->setMouseCursor(hotX, hotY, getShapePtr(shape));
}

} // end of namespace Kyra
