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
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/parse.h"

namespace Gob {

Game_v6::Game_v6(GobEngine *vm) : Game_v2(vm) {
	_dword_63E44 = 0;
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

int16 Game_v6::checkCollisions(byte handleMouse, int16 deltaTime, int16 *pResId,
	    int16 *pResIndex) {
	int16 resIndex;
	int16 key;
	uint32 timeKey;

	_scrollHandleMouse = handleMouse != 0;

	if (deltaTime >= -1) {
		_lastCollKey = 0;
		_lastCollAreaIndex = 0;
		_lastCollId = 0;
	}

	if (pResId != 0)
		*pResId = 0;

	resIndex = 0;

	if ((_lastCollKey != 0) &&
	    ( (_collisionAreas[_lastCollAreaIndex].id  != _lastCollId) ||
	      (_collisionAreas[_lastCollAreaIndex].key != _lastCollKey))) {

		_lastCollKey = 0;
		_lastCollAreaIndex = 0;
		_lastCollId = 0;
	}

	if ((_vm->_draw->_cursorIndex == -1) &&
			(handleMouse != 0) && (_lastCollKey == 0)) {
		_lastCollKey = checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);

		if ((_lastCollKey != 0) && (_lastCollId & 0x8000))
			collAreaSub(_lastCollAreaIndex, 1);
	}

	if (handleMouse != 0)
		_vm->_draw->animateCursor(-1);

	timeKey = _vm->_util->getTimeKey();
	_vm->_draw->blitInvalidated();
	while (1) {
		if (_vm->_inter->_terminate || _vm->shouldQuit()) {
			if (handleMouse)
				_vm->_draw->blitCursor();
			return 0;
		}

		sub_1BA78();
		if (!_vm->_draw->_noInvalidated) {
			if (handleMouse != 0)
				_vm->_draw->animateCursor(-1);
			else
				_vm->_draw->blitInvalidated();
			_vm->_video->waitRetrace();
		}

		key = checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_mouseButtons, handleMouse);

		if ((handleMouse == 0) && (_mouseButtons != 0)) {
			_vm->_util->waitMouseRelease(0);
			key = 3;
		}

		if (key != 0) {

			if (handleMouse & 1)
				_vm->_draw->blitCursor();

			if (pResId != 0)
				*pResId = 0;

			if (pResIndex != 0)
				*pResIndex = 0;

			if ((_lastCollKey != 0) && (_lastCollId & 0x8000))
				collAreaSub(_lastCollAreaIndex, 0);

			_lastCollKey = 0;
			if (key != 0)
				return key;

			if (handleMouse)
				_vm->_draw->animateCursor(-1);
		}

		if (handleMouse != 0) {
			if (_mouseButtons != 0) {

				if (deltaTime > 0) {
					_vm->_draw->animateCursor(2);
					_vm->_util->delay(deltaTime);
				} else if (handleMouse & 1)
					_vm->_util->waitMouseRelease(1);

				_vm->_draw->animateCursor(-1);

				if (pResId != 0)
					*pResId = 0;

				key = checkMousePoint(0, pResId, &resIndex);
				if (pResIndex != 0)
					*pResIndex = resIndex;

				if ((key != 0) || ((pResId != 0) && (*pResId != 0))) {
					if ((handleMouse & 1) &&
							((deltaTime <= 0) || (_mouseButtons == 0)))
						_vm->_draw->blitCursor();

					if (key != _lastCollKey)
						collAreaSub(_lastCollAreaIndex, 0);

					_lastCollKey = 0;
					return key;
				}

				if (handleMouse & 4)
					return 0;

				if (_lastCollKey != 0)
					collAreaSub(_lastCollAreaIndex, 0);

				_lastCollKey = checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);
				if ((_lastCollKey != 0) && (_lastCollId & 0x8000))
					collAreaSub(_lastCollAreaIndex, 1);
			} else 
				sub_1BA78();
		}

		if ((deltaTime == -2) && (key == 0) && (_mouseButtons == 0)) {
			if (pResId != 0)
				*pResId = 0;

			if (pResIndex != 0)
				*pResIndex = 0;

			return 0;
			
		} else if (handleMouse != 0)
			_vm->_draw->animateCursor(-1);

		if ((deltaTime < 0) && (key == 0) && (_mouseButtons == 0)) {
			uint32 curtime = _vm->_util->getTimeKey();
			if ((curtime + deltaTime) > timeKey) {
				if (pResId != 0)
					*pResId = 0;

				if (pResIndex != 0)
					*pResIndex = 0;

				return 0;
			}
		}

		_vm->_util->delay(10);
	}
}

void Game_v6::setCollisions(byte arg_0) {
	for (Collision *collArea = _collisionAreas; collArea->left != 0xFFFF; collArea++) {
		if (((collArea->id & 0xC000) != 0x8000) || (collArea->funcSub == 0))
			continue;

		if (arg_0 == 0)
			if (collArea->flags & 0x80)
				continue;

		byte *totFileData = collArea->totFileData;

		if (!totFileData)
			totFileData = _totFileData;

		byte *savedIP = _vm->_global->_inter_execPtr;

		_vm->_global->_inter_execPtr = totFileData + collArea->funcSub;

		int16 left   = _vm->_parse->parseValExpr();
		int16 top    = _vm->_parse->parseValExpr();
		int16 width  = _vm->_parse->parseValExpr();
		int16 height = _vm->_parse->parseValExpr();
		uint16 flags;

		if ((collArea->id & 0xF000) == 0xA000)
			flags = _vm->_parse->parseValExpr();

		if ((_vm->_draw->_renderFlags & RENDERFLAG_CAPTUREPOP) &&
				(left != -1)) {
			left += _vm->_draw->_backDeltaX;
			top += _vm->_draw->_backDeltaY;
		}
		if (_vm->_draw->_needAdjust != 2) {
			_vm->_draw->adjustCoords(0, &left, &top);
			if ((collArea->flags & 0x0F) < 3)
				_vm->_draw->adjustCoords(2, &width, &height);
			else {
				height &= 0xFFFE;
				_vm->_draw->adjustCoords(2, 0, &height);
			}
		}

		if (left < 0) {
			width += left;
			left = 0;
		}

		if (top < 0) {
			height += top;
			top = 0;
		}

		collArea->left = left;
		collArea->top = top;
		collArea->right = left + width - 1;
		collArea->bottom = top + height - 1;

		if ((collArea->id & 0xF000) == 0xA000)
			collArea->flags = flags;

		_vm->_global->_inter_execPtr = savedIP;
	}
}

void Game_v6::collSub(uint16 offset) {
	byte *savedIP;
	int16 collStackSize;

	savedIP = _vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr = _totFileData + offset;

	_shouldPushColls = 1;
	collStackSize = _collStackSize;

	_vm->_inter->funcBlock(0);

	if (collStackSize != _collStackSize)
		popCollisions();

	_shouldPushColls = 0;
	_vm->_global->_inter_execPtr = savedIP;

	if ((_vm->_util->getTimeKey() - _dword_63E44) > 500)
		setCollisions(0);
}

void Game_v6::sub_1BA78() {
	int16 lastCollAreaIndex = _lastCollAreaIndex;
	int16 lastCollId = _lastCollId;
	int16 collKey = checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);

	if (collKey == _lastCollKey)
		return;

	if ((_lastCollKey != 0) && (lastCollId & 0x8000))
		collAreaSub(lastCollAreaIndex, 0);

	_lastCollKey = collKey;

	if ((_lastCollKey != 0) && (_lastCollId & 0x8000))
		collAreaSub(_lastCollAreaIndex, 1);
}

} // End of namespace Gob
