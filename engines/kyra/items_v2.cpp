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
		snd_playSoundEffect(0x0d);
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

	int freeItemSlot = -1;

	if (unk1 != 3) {
		for (int i = 0; i < 30; ++i) {
			if (_itemList[i].id == 0xFFFF) {
				freeItemSlot = i;
				break;
			}
		}
	}

	if (freeItemSlot == -1)
		return false;

	if (sceneId != _mainCharacter.sceneId) {
		//addToItemList(sceneId, item, freeItemSlot, x, y);
		//return true;
		//XXX
		return false;
	}

	int itemHeight = _itemHtDat[item];

	// no idea why it's '&&' here and not single checks for x and y
	if (x == -1 && y == -1) {
		x = _rnd.getRandomNumberRng(0x10, 0x130);
		y = _rnd.getRandomNumberRng(0x10, 0x87);
	}

	int posX = x, posY = y;
	int itemX = -1, itemY = -1;
	bool needRepositioning = true;

	while (needRepositioning) {
		if ((_screen->getDrawLayer(posX, posY) <= 1 && _screen->getDrawLayer2(posX, posY, itemHeight) <= 1 && isDropable(posX, posY)) || posY == 136) {
			int posX2 = posX, posX3 = posX;
			bool repositioning = true;

			while (repositioning) {
				if (isDropable(posX3, posY) && _screen->getDrawLayer(posX3, posY) < 7 && checkItemCollision(posX3, posY) == -1) {
					itemX = posX3;
					itemY = posY;
					needRepositioning = false;
					repositioning = false;
				}

				if (isDropable(posX2, posY) && _screen->getDrawLayer(posX2, posY) < 7 && checkItemCollision(posX2, posY) == -1) {
					itemX = posX2;
					itemY = posY;
					needRepositioning = false;
					repositioning = false;
				}

				if (repositioning) {
					posX3 = MAX(posX3 - 2, 16);
					posX2 = MIN(posX2 + 2, 304);

					if (posX3 <= 16 && posX2 >= 304)
						repositioning = false;
				}
			}
		}

		if (posY == 136)
			needRepositioning = false;
		else
			posY = MIN(posY + 2, 136);
	}

	if (itemX == -1 || itemY == -1)
		return false;

	if (unk1 == 3) {
		_itemList[freeItemSlot].x = itemX;
		_itemList[freeItemSlot].y = itemY;
		return true;
	} else if (unk1 == 2) {
		itemDropDown(x, y, itemX, itemY, freeItemSlot, item);
	}

	if (!unk1)
		removeHandItem();

	itemDropDown(x, y, itemX, itemY, freeItemSlot, item);

	if (!unk1 && unk2) {
		int itemStr = 3;
		if (_lang == 1)
			itemStr = getItemCommandStringDrop(item);
		updateCommandLineEx(item+54, itemStr, 0xD6);
	}

	return true;
}

void KyraEngine_v2::itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, uint16 item) {
	uint8 *itemShape = getShapePtr(item + 64);

	if (startX == dstX && startY == dstY) {
		if (_layerFlagTable[_screen->getLayer(dstX, dstY)] && item != 13) {
			updateCharFacing();
			snd_playSoundEffect(0x2d);
			removeHandItem();
			//XXX sub_277FA(getTableString(0xFF, _cCodeBuffer), 1, 0, 0x83, 0xFF);
		} else {
			_itemList[itemSlot].x = dstX;
			_itemList[itemSlot].y = dstY;
			_itemList[itemSlot].id = item;
			_itemList[itemSlot].sceneId = _mainCharacter.sceneId;
			snd_playSoundEffect(0x0c);
			addItemToAnimList(itemSlot);
		}
	} else {
		_screen->hideMouse();

		if (startY <= dstY) {
			int speed = 2;
			int curY = startY;
			int curX = startX - 8;

			backUpGfxRect24x24(curX, curY-16);
			while (curY < dstY) {
				restoreGfxRect24x24(curX, curY-16);

				curY = MIN(curY + speed, dstY);
				++speed;

				backUpGfxRect24x24(curX, curY-16);
				uint32 endDelay = _system->getMillis() + _tickLength;

				_screen->drawShape(0, itemShape, curX, curY-16, 0, 0);
				_screen->updateScreen();

				// XXX: original doesn't update game state while delaying
				// our implementation *could* do it, so maybe check this again
				delayUntil(endDelay);
			}

			if (dstX != dstY || (dstY - startY > 16)) {
				snd_playSoundEffect(0x69);
				speed = MAX(speed, 6);
				int speedX = ((dstX - startX) << 4) / speed;
				int origSpeed = speed;
				speed >>= 1;

				if (dstY - startY <= 8)
					speed >>= 1;

				speed = -speed;

				curX = startX << 4;

				int x = 0, y = 0;
				while (--origSpeed) {
					x = (curX >> 4) - 8;
					y = curY - 16;

					restoreGfxRect24x24(x, y);
					curY = MIN(curY + speed, dstY);
					curX += speedX;
					++speed;

					x = (curX >> 4) - 8;
					y = curY - 16;
					backUpGfxRect24x24(x, y);

					uint16 endDelay = _system->getMillis() + _tickLength;
					_screen->drawShape(0, itemShape, x, y, 0, 0);
					_screen->updateScreen();

					// XXX: original doesn't update game state while delaying
					// our implementation *could* do it, so maybe check this again
					delayUntil(endDelay);
				}

				restoreGfxRect24x24(x, y);
			} else {
				restoreGfxRect24x24(curX, curY-16);
			}
		}

		if (_layerFlagTable[_screen->getLayer(dstX, dstY)] && item != 13) {
			updateCharFacing();
			snd_playSoundEffect(0x2d);
			removeHandItem();
			_screen->showMouse();
			//XXX sub_277FA(getTableString(0xFF, _cCodeBuffer), 1, 0, 0x83, 0xFF);
		} else {
			_itemList[itemSlot].x = dstX;
			_itemList[itemSlot].y = dstY;
			_itemList[itemSlot].id = item;
			_itemList[itemSlot].sceneId = _mainCharacter.sceneId;
			snd_playSoundEffect(0x0c);
			addItemToAnimList(itemSlot);
			_screen->showMouse();
		}
	}
}

void KyraEngine_v2::exchangeMouseItem(int itemPos) {
	_screen->hideMouse();

	deleteItemAnimEntry(itemPos);

	int itemId = _itemList[itemPos].id;
	_itemList[itemPos].id = _itemInHand;
	_itemInHand = itemId;

	addItemToAnimList(itemPos);
	snd_playSoundEffect(0x0b);
	setMouseCursor(_itemInHand);
	int str2 = 7;

	if (_lang == 1)
		str2 = getItemCommandStringPickUp(itemId);

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
		snd_playSoundEffect(0x0b);
		setMouseCursor(itemId);
		int str2 = 7;

		if (_lang == 1)
			str2 = getItemCommandStringPickUp(itemId);

		updateCommandLineEx(itemId + 54, str2, 0xD6);
		_itemInHand = itemId;
		_screen->showMouse();

		runSceneScript6();
	}

	return true;
}

bool KyraEngine_v2::isDropable(int x, int y) {
	if (x < 14 || x > 304 || y < 14 || y > 136)
		return false;

	x -= 8;
	y -= 1;

	for (int xpos = x; xpos < x + 16; ++xpos) {
		if (_screen->getShapeFlag1(xpos, y) == 0)
			return false;
	}

	return true;
}

int KyraEngine_v2::getItemCommandStringDrop(uint16 item) {
	assert(item < _itemStringMapSize);
	int stringId = _itemStringMap[item];

	static const int dropStringIds[] = {
		0x2D, 0x103, 0x003, 0x106
	};
	assert(stringId < ARRAYSIZE(dropStringIds));

	return dropStringIds[stringId];
}

int KyraEngine_v2::getItemCommandStringPickUp(uint16 item) {
	assert(item < _itemStringMapSize);
	int stringId = _itemStringMap[item];

	static const int pickUpStringIds[] = {
		0x02B, 0x102, 0x007, 0x105
	};
	assert(stringId < ARRAYSIZE(pickUpStringIds));

	return pickUpStringIds[stringId];
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

void KyraEngine_v2::setHandItem(uint16 item) {
	_screen->hideMouse();

	if (item == 0xFFFF) {
		removeHandItem();
	} else {
		setMouseCursor(item);
		_itemInHand = item;
	}

	_screen->showMouse();
}

void KyraEngine_v2::removeHandItem() {
	_screen->hideMouse();
	_screen->setMouseCursor(0, 0, _defaultShapeTable[0]);
	_itemInHand = -1;
	_handItemSet = -1;
	_screen->showMouse();
}

} // end of namespace Kyra

