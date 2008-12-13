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

#include "common/endian.h"
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/game.h"
#include "gob/inter.h"

namespace Gob {

Game_v6::Game_v6(GobEngine *vm) : Game_v2(vm) {
}

int16 Game_v6::addNewCollision(int16 id, uint16 left, uint16 top,
		uint16 right, uint16 bottom, int16 flags, int16 key,
		uint16 funcEnter, uint16 funcLeave) {
	Collision *ptr;

	debugC(5, kDebugCollisions, "addNewCollision");
	debugC(5, kDebugCollisions, "id = %X", id);
	debugC(5, kDebugCollisions, "left = %d, top = %d, right = %d, bottom = %d",
			left, top, right, bottom);
	debugC(5, kDebugCollisions, "flags = %X, key = %X", flags, key);
	debugC(5, kDebugCollisions, "funcEnter = %d, funcLeave = %d",
			funcEnter, funcLeave);

	for (int i = 0; i < 150; i++) {
		if ((_collisionAreas[i].left != 0xFFFF) && (_collisionAreas[i].id != id))
			continue;

		ptr = &_collisionAreas[i];

		if ((ptr->id & 0xBFFF) != (id & 0xBFFF))
			ptr->id = id;

		ptr->left = left;
		ptr->top = top;
		ptr->right = right;
		ptr->bottom = bottom;
		ptr->flags = flags;
		ptr->key = key;
		ptr->funcEnter = funcEnter;
		ptr->funcLeave = funcLeave;
		ptr->funcSub = 0;
		ptr->totFileData = _totFileData;

		return i;
	}
	error("Game_v6::addNewCollision(): Collision array full!\n");
	return 0;
}

void Game_v6::pushCollisions(char all) {
	Collision *srcPtr;
	Collision *destPtr;
	int16 size;

	debugC(1, kDebugCollisions, "pushCollisions");
	for (size = 0, srcPtr = _collisionAreas; srcPtr->left != 0xFFFF; srcPtr++) {
		if ( (all == 1) ||
		    ((all == 0) && (((uint16) srcPtr->id) >= 20)) ||
		    ((all == 2) && (((srcPtr->id & 0xF000) == 0xD000) ||
		                    ((srcPtr->id & 0xF000) == 0x4000) ||
		                    ((srcPtr->id & 0xF000) == 0xE000))))
			size++;
	}

	destPtr = new Collision[size];
	_collStack[_collStackSize] = destPtr;

	if (_vm->_inter->_terminate)
		return;

	_collStackElemSizes[_collStackSize] = size;

	if (_shouldPushColls != 0)
		_collStackElemSizes[_collStackSize] |= 0x8000;

	_shouldPushColls = 0;
	_collLasts[_collStackSize].key = _lastCollKey;
	_collLasts[_collStackSize].id = _lastCollId;
	_collLasts[_collStackSize].areaIndex = _lastCollAreaIndex;
	_lastCollKey = 0;
	_lastCollId = 0;
	_lastCollAreaIndex = 0;
	_collStackSize++;

	for (srcPtr = _collisionAreas; srcPtr->left != 0xFFFF; srcPtr++) {
		if ( (all == 1) ||
		    ((all == 0) && (((uint16) srcPtr->id) >= 20)) ||
		    ((all == 2) && (((srcPtr->id & 0xF000) == 0xD000) ||
		                    ((srcPtr->id & 0xF000) == 0x4000) ||
		                    ((srcPtr->id & 0xF000) == 0xE000)))) {

			memcpy(destPtr, srcPtr, sizeof(Collision));
			srcPtr->left = 0xFFFF;
			destPtr++;
		}
	}
}

} // End of namespace Gob
