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

int KyraEngine_v2::findItem(uint16 sceneId, int id) {
	for (int i = 0; i < 30; ++i) {
		if (_itemList[i].id == id && _itemList[i].sceneId == sceneId)
			return i;
	}
	return -1;
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

} // end of namespace Kyra
