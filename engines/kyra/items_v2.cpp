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
#include "kyra/screen_v2.h"

namespace Kyra {

void KyraEngine_v2::initItemList(int size) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::initItemList(%d)", size);
	delete[] _itemList;

	_itemList = new Item[size];
	assert(_itemList);
	memset(_itemList, 0, sizeof(Item)*size);
	_itemListSize = size;

	resetItemList();
}

int KyraEngine_v2::findFreeItem() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::findFreeItem()");
	for (int i = 0; i < _itemListSize; ++i) {
		if (_itemList[i].id == 0xFFFF)
			return i;
	}
	return -1;
}

int KyraEngine_v2::countAllItems() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::countAllItems()");
	int num = 0;
	for (int i = 0; i < _itemListSize; ++i) {
		if (_itemList[i].id != 0xFFFF)
			++num;
	}
	return num;
}

int KyraEngine_v2::findItem(uint16 sceneId, uint16 id) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::findItem(%u, %u)", sceneId, id);
	for (int i = 0; i < _itemListSize; ++i) {
		if (_itemList[i].id == id && _itemList[i].sceneId == sceneId)
			return i;
	}
	return -1;
}

int KyraEngine_v2::findItem(uint16 item) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::findItem(%u)", item);
	for (int i = 0; i < _itemListSize; ++i) {
		if (_itemList[i].id == item)
			return i;
	}
	return -1;
}

void KyraEngine_v2::resetItemList() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::resetItemList()");
	for (int i = 0; i < _itemListSize; ++i)
		resetItem(i);
}

void KyraEngine_v2::resetItem(int index) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::resetItem(%d)", index);
	_itemList[index].id = 0xFFFF;
	_itemList[index].sceneId = 0xFFFF;
	_itemList[index].x = 0;
	_itemList[index].y = 0;
}

void KyraEngine_v2::setHandItem(uint16 item) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::setHandItem(%u)", item);
	Screen *scr = screen();
	scr->hideMouse();

	if (item == 0xFFFF) {
		removeHandItem();
	} else {
		setMouseCursor(item);
		_itemInHand = item;
	}

	scr->showMouse();
}

void KyraEngine_v2::removeHandItem() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::removeHandItem()");
	Screen *scr = screen();
	scr->hideMouse();
	scr->setMouseCursor(0, 0, getShapePtr(0));
	_itemInHand = -1;
	_mouseState = -1;
	scr->showMouse();
}

} // end of namesapce Kyra

