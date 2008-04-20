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
 * $URL $
 * $Id$
 *
 */

#include "kyra/kyra_v3.h"

namespace Kyra {

void KyraEngine_v3::resetItem(int index) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::resetItem(%d)", index);
	_itemList[index].id = 0xFFFF;
	_itemList[index].sceneId = 0xFFFF;
	_itemList[index].x = 0;
	_itemList[index].y = 0;
	_itemList[index].unk8 = 0;
}

void KyraEngine_v3::resetItemList() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::resetItemList()");
	for (int i = 0; i < 50; ++i)
		resetItem(i);
}

int KyraEngine_v3::findFreeItem() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::findFreeItem()");
	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].id == 0xFFFF)
			return i;
	}
	return -1;
}

int KyraEngine_v3::findItem(uint16 sceneId, uint16 id) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::findItem(%u, %u)", sceneId, id);
	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].id == id && _itemList[i].sceneId == sceneId)
			return i;
	}
	return -1;
}

int KyraEngine_v3::checkItemCollision(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::checkItemCollision(%d, %d)", x, y);
	int itemIndex = -1;
	int maxItemY = -1;

	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].id == 0xFFFF || _itemList[i].sceneId != _mainCharacter.sceneId)
			continue;

		const int x1 = _itemList[i].x - 11;
		const int x2 = _itemList[i].x + 10;

		if (x < x1 || x > x2)
			continue;

		const int y1 = _itemList[i].y - _itemBuffer1[_itemList[i].id] - 3;
		const int y2 = _itemList[i].y + 3;

		if (y < y1 || y > y2)
			continue;

		if (_itemList[i].y >= maxItemY) {
			itemIndex = i;
			maxItemY = _itemList[i].y;
		}
	}

	return itemIndex;
}

void KyraEngine_v3::setItemMouseCursor() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::setItemMouseCursor()");
	_handItemSet = _itemInHand;
	if (_itemInHand == -1)
		_screen->setMouseCursor(0, 0, _gameShapes[0]);
	else
		_screen->setMouseCursor(0xC, 0x13, _gameShapes[_itemInHand+248]);
}

} // end of namespace Kyra
