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
		uint16 funcEnter, uint16 funcLeave, uint16 funcSub) {
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
		ptr->funcSub = funcSub;
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

	if (_collStackSize >= 5)
		error("Game_v6::pushCollisions: _collStackSize == %d", _collStackSize);

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

void Game_v6::collisionsBlock(void) {
	InputDesc descArray[20];
	int16 array[300];
	byte count;
	int16 collResId;
	byte *startIP;
	int16 curCmd;
	int16 cmd;
	int16 cmdHigh;
	int16 key;
	int16 flags;
	uint16 left;
	uint16 top;
	uint16 width;
	uint16 height;
	int16 deltaTime;
	int16 stackPos2;
	int16 descIndex;
	int16 timeVal;
	int16 offsetIP;
	char *str;
	int16 i;
	int16 counter;
	int16 var_24;
	int16 var_26;
	int16 collStackPos;
	Collision *collPtr;
	Collision *collArea;
	byte *savedIP;
	byte collAreaStart;
	int16 activeCollResId = 0;
	int16 activeCollIndex = 0;

	if (_shouldPushColls)
		pushCollisions(0);

	collAreaStart = 0;
	while (_collisionAreas[collAreaStart].left != 0xFFFF)
		collAreaStart++;
	collArea = &_collisionAreas[collAreaStart];

	_shouldPushColls = 0;
	collResId = -1;

	_vm->_global->_inter_execPtr++;
	count = *_vm->_global->_inter_execPtr++;

	_handleMouse = _vm->_global->_inter_execPtr[0];
	deltaTime = 1000 * _vm->_global->_inter_execPtr[1];
	stackPos2 = _vm->_global->_inter_execPtr[3];
	descIndex = _vm->_global->_inter_execPtr[4];
	byte var_42 = _vm->_global->_inter_execPtr[5];

	if ((stackPos2 != 0) || (descIndex != 0)) {
		deltaTime /= 100;
		if (_vm->_global->_inter_execPtr[1] == 100)
			deltaTime = 2;
	}

	timeVal = deltaTime;
	_vm->_global->_inter_execPtr += 6;

	startIP = _vm->_global->_inter_execPtr;
	WRITE_VAR(16, 0);

	byte var_41 = 0;
	int16 var_46 = 0;
	int16 var_1C = 0;
	int16 index = 0;
	int16 curEditIndex = 0;
	int right = 0, funcLeave = 0;

	for (curCmd = 0; curCmd < count; curCmd++) {
		array[curCmd] = 0;
		cmd = *_vm->_global->_inter_execPtr++;

		if ((cmd & 0x40) != 0) {
			cmd -= 0x40;
			cmdHigh = *_vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr++;
			cmdHigh <<= 8;
		} else
			cmdHigh = 0;

		if ((cmd & 0x80) != 0) {
			offsetIP = _vm->_global->_inter_execPtr - _totFileData;
			left = _vm->_parse->parseValExpr();
			top = _vm->_parse->parseValExpr();
			width = _vm->_parse->parseValExpr();
			height = _vm->_parse->parseValExpr();
		} else {
			offsetIP = 0;
			left = _vm->_inter->load16();
			top = _vm->_inter->load16();
			width = _vm->_inter->load16();
			height = _vm->_inter->load16();
		}

		if ((_vm->_draw->_renderFlags & RENDERFLAG_CAPTUREPOP) && (left != 0xFFFF)) {
			left += _vm->_draw->_backDeltaX;
			top += _vm->_draw->_backDeltaY;
		}

		if (left != 0xFFFF) {
			_vm->_draw->adjustCoords(0, &left, &top);
			if (((cmd & 0x3F) < 20) && ((cmd & 0x3F) >= 3)) {
				if (_vm->_draw->_needAdjust != 2)
					height &= 0xFFFE;
				_vm->_draw->adjustCoords(0, 0, &width);
			} else
				_vm->_draw->adjustCoords(0, &height, &width);
		}

		cmd &= 0x7F;
		debugC(1, kDebugCollisions, "collisionsBlock(%d)", cmd);

		switch (cmd) {
		case 0:
			_vm->_global->_inter_execPtr += 6;
			startIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);
			key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					cmd + cmdHigh, key, startIP - _totFileData,
					_vm->_global->_inter_execPtr - _totFileData, offsetIP);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);

			break;

		case 1:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16();

			startIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);

			if (key == 0)
				key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					(flags << 4) + cmd + cmdHigh, key, startIP - _totFileData,
					_vm->_global->_inter_execPtr - _totFileData, offsetIP);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);

			break;

		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			_vm->_util->clearKeyBuf();
			var_1C = 1;
			key = _vm->_parse->parseVarIndex();
			descArray[index].fontIndex = _vm->_inter->load16();
			descArray[index].backColor = *_vm->_global->_inter_execPtr++;
			descArray[index].frontColor = *_vm->_global->_inter_execPtr++;

			if ((cmd >= 5) && (cmd <= 8)) {
				descArray[index].ptr = _vm->_global->_inter_execPtr + 2;
				_vm->_global->_inter_execPtr +=
					READ_LE_UINT16(_vm->_global->_inter_execPtr) + 2;
			} else
				descArray[index].ptr = 0;

			if (left == 0xFFFF) {
				if ((cmd & 1) == 0) {
					_vm->_global->_inter_execPtr += 2;
					_vm->_global->_inter_execPtr +=
						READ_LE_UINT16(_vm->_global->_inter_execPtr);
				}
				break;
			}

			right = left + width - 1;
			if (!_vm->_draw->_fonts[descArray[index].fontIndex]->extraData)
				right = left + width * _vm->_draw->_fonts[descArray[index].fontIndex]->itemWidth - 1;

			funcLeave = 0;
			if (!(cmd & 1))
				funcLeave = _vm->_global->_inter_execPtr - _totFileData;

			addNewCollision(curCmd + 0x8000, left, top, right,
					top + height - 1, cmd, key, 0, funcLeave, 0);

			if (!(cmd & 1)) {
				_vm->_global->_inter_execPtr += 2;
				_vm->_global->_inter_execPtr +=
					READ_LE_UINT16(_vm->_global->_inter_execPtr);
			}

			index++;
			break;

		case 11:
			_vm->_global->_inter_execPtr += 6;
			for (i = 0; i < 150; i++) {
				if ((_collisionAreas[i].id & 0xF000) == 0xE000) {
					_collisionAreas[i].id &= 0xBFFF;
					_collisionAreas[i].funcEnter =
						_vm->_global->_inter_execPtr - _totFileData;
					_collisionAreas[i].funcLeave =
						_vm->_global->_inter_execPtr - _totFileData;
				}
			}
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;

		case 12:
			_vm->_global->_inter_execPtr += 6;
			for (i = 0; i < 150; i++) {
				if ((_collisionAreas[i].id & 0xF000) == 0xD000) {
					_collisionAreas[i].id &= 0xBFFF;
					_collisionAreas[i].funcEnter =
						_vm->_global->_inter_execPtr - _totFileData;
					_collisionAreas[i].funcLeave =
						_vm->_global->_inter_execPtr - _totFileData;
				}
			}
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;

		case 20:
			collResId = curCmd;
			// Fall through to case 2

		case 2:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16();

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					(flags << 4) + cmdHigh + 2, key, 0,
					_vm->_global->_inter_execPtr - _totFileData, offsetIP);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);

			break;

		case 21:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16() & 3;

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					(flags << 4) + cmdHigh + 2, key,
					_vm->_global->_inter_execPtr - _totFileData, 0, offsetIP);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr +=
				READ_LE_UINT16(_vm->_global->_inter_execPtr);

			break;
		}
	}

	if (var_42 != 0)
		setCollisions(1);

	_forceHandleMouse = 0;
	_vm->_util->clearKeyBuf();

	do {
		if (var_1C != 0) {
			key = multiEdit(deltaTime, index, &curEditIndex, descArray,
					&activeCollResId, &activeCollIndex);

			WRITE_VAR(55, curEditIndex);
			if (key == 0x1C0D) {
				for (i = 0; i < 150; i++) {
					if (_collisionAreas[i].left == 0xFFFF)
						break;

					if ((_collisionAreas[i].id & 0xC000) != 0x8000)
						continue;

					if ((_collisionAreas[i].flags & 1) != 0)
						continue;

					if ((_collisionAreas[i].flags & 0x0F) <= 2)
						continue;

					activeCollResId = _collisionAreas[i].id;
					collResId = _collisionAreas[i].id & 0x7FFF;
					activeCollIndex = i;
					break;
				}
				break;
			}
		} else
			key = checkCollisions(_handleMouse, -deltaTime,
					&activeCollResId, &activeCollIndex);

		if (((key & 0xFF) >= ' ') && ((key & 0xFF) <= 0xFF) &&
		    ((key >> 8) > 1) && ((key >> 8) < 12))
			key = '0' + (((key >> 8) - 1) % 10) + (key & 0xFF00);

		if (activeCollResId == 0) {
			if (key != 0) {
				for (i = 0; i < 150; i++) {
					if (_collisionAreas[i].left == 0xFFFF)
						break;

					if ((_collisionAreas[i].id & 0xC000) != 0x8000)
						continue;

					if ((_collisionAreas[i].key == key) ||
							(_collisionAreas[i].key == 0x7FFF)) {
						activeCollResId = _collisionAreas[i].id;
						activeCollIndex = i;
						break;
					}
				}

				if (activeCollResId == 0) {
					for (i = 0; i < 150; i++) {
						if (_collisionAreas[i].left == 0xFFFF)
							break;

						if ((_collisionAreas[i].id & 0xC000) != 0x8000)
							continue;

						if ((_collisionAreas[i].key & 0xFF00) != 0)
							continue;

						if (_collisionAreas[i].key == 0)
							continue;

						if (adjustKey(key & 0xFF) == adjustKey(_collisionAreas[i].key)) {
							activeCollResId = _collisionAreas[i].id;
							activeCollIndex = i;
							break;
						}
					}
				}
			} else if (deltaTime != 0) {
				if (stackPos2 != 0) {
					collStackPos = 0;

					for (i = 0, collPtr = collArea; collPtr->left != 0xFFFF; i++, collPtr++) {
						if ((collPtr->id & 0xF000) != 0x8000)
							continue;

						collStackPos++;
						if (collStackPos != stackPos2)
							continue;

						activeCollResId = collPtr->id;
						activeCollIndex = i + collAreaStart;
						_vm->_inter->storeMouse();
						if (VAR(16) != 0)
							break;

						if ((activeCollResId & 0xF000) == 0x8000)
							WRITE_VAR(16, array[activeCollResId & 0xFFF]);
						else
							WRITE_VAR(16, activeCollResId & 0xFFF);

						if (collPtr->funcLeave != 0) {
							uint32 timeKey = _vm->_util->getTimeKey();
							collSub(collPtr->funcLeave);

							if (timeVal != 2) {
								deltaTime = timeVal - (_vm->_util->getTimeKey() - timeKey);

								if ((deltaTime - var_46) < 3) {
									var_46 -= (deltaTime - 3);
									deltaTime = 3;
								} else if (var_46 != 0) {
									deltaTime -= var_46;
									var_46 = 0;
								}

								if (deltaTime > timeVal)
									deltaTime = timeVal;

							} else
								deltaTime = 2;

						}

						if (VAR(16) == 0)
							activeCollResId = 0;
						else
							var_41 = 1;

						break;
					}
				} else {
					if (descIndex != 0) {

						counter = 0;
						for (i = 0, collPtr = collArea; collPtr->left != 0xFFFF; i++, collPtr++) {
							if ((collPtr->id & 0xF000) == 0x8000)
								if (++counter == descIndex) {
									activeCollResId = collPtr->id;
									activeCollIndex = i + collAreaStart;
									break;
								}
						}

					} else {

						for (i = 0, collPtr = _collisionAreas; collPtr->left != 0xFFFF; i++, collPtr++) {
							if ((collPtr->id & 0xF000) == 0x8000) {
								activeCollResId = collPtr->id;
								activeCollIndex = i;
								break;
							}
						}
						if ((_lastCollKey != 0) &&
								(_collisionAreas[_lastCollAreaIndex].funcLeave != 0))
							collSub(_collisionAreas[_lastCollAreaIndex].funcLeave);

						_lastCollKey = 0;
					}

				}
			}
		}

		if (var_41 != 0)
			break;

		if ((activeCollResId == 0) ||
				(_collisionAreas[activeCollIndex].funcLeave != 0))
			continue;

		_vm->_inter->storeMouse();
		if ((activeCollResId & 0xF000) == 0x8000)
			WRITE_VAR(16, array[activeCollResId & 0xFFF]);
		else
			WRITE_VAR(16, activeCollResId & 0xFFF);

		if (_collisionAreas[activeCollIndex].funcEnter != 0)
			collSub(_collisionAreas[activeCollIndex].funcEnter);

		WRITE_VAR(16, 0);
		activeCollResId = 0;
	}
	while ((activeCollResId == 0) && !_vm->_inter->_terminate && !_vm->shouldQuit());

	if ((activeCollResId & 0xFFF) == collResId) {
		collStackPos = 0;
		var_24 = 0;
		var_26 = 1;
		for (i = 0; i < 150; i++) {
			if (_collisionAreas[i].left == 0xFFFF)
				continue;

			if ((_collisionAreas[i].id & 0xC000) != 0x8000)
				continue;

			if ((_collisionAreas[i].flags & 0x0F) < 3)
				continue;

			if ((_collisionAreas[i].flags & 0x0F) > 10)
				continue;

			if ((_collisionAreas[i].flags & 0x0F) > 8) {
				char *ptr;
				strncpy0(_tempStr, GET_VARO_STR(_collisionAreas[i].key), 255);
				while ((ptr = strchr(_tempStr, ' ')))
					_vm->_util->cutFromStr(_tempStr, (ptr - _tempStr), 1);
				if (_vm->_language == 2)
					while ((ptr = strchr(_tempStr, '.')))
						*ptr = ',';
				WRITE_VARO_STR(_collisionAreas[i].key, _tempStr);
			}

			if (((_collisionAreas[i].flags & 0x0F) >= 5) &&
			    ((_collisionAreas[i].flags & 0x0F) <= 8)) {
				str = (char *) descArray[var_24].ptr;

				strncpy0(_tempStr, GET_VARO_STR(_collisionAreas[i].key), 255);

				if ((_collisionAreas[i].flags & 0x0F) < 7)
					_vm->_util->prepareStr(_tempStr);

				int16 pos = 0;
				do {
					strncpy0(_collStr, str, 255);
					pos += strlen(str) + 1;

					str += strlen(str) + 1;

					if ((_collisionAreas[i].flags & 0x0F) < 7)
						_vm->_util->prepareStr(_collStr);

					if (strcmp(_tempStr, _collStr) == 0) {
						WRITE_VAR(17, VAR(17) + 1);
						WRITE_VAR(17 + var_26, 1);
						break;
					}
				} while (READ_LE_UINT16(descArray[var_24].ptr - 2) > pos);
				collStackPos++;
			} else {
				WRITE_VAR(17 + var_26, 2);
			}
			var_24++;
			var_26++;
		}

		if (collStackPos != (int16) VAR(17))
			WRITE_VAR(17, 0);
		else
			WRITE_VAR(17, 1);
	}

	if (_handleMouse == 1)
		_vm->_draw->blitCursor();

	savedIP = 0;
	if (!_vm->_inter->_terminate && (var_41 == 0)) {
		savedIP = _totFileData +
			_collisionAreas[activeCollIndex].funcLeave;

		_vm->_inter->storeMouse();
		if (VAR(16) == 0) {
			if ((activeCollResId & 0xF000) == 0x8000)
				WRITE_VAR(16, array[activeCollResId & 0xFFF]);
			else
				WRITE_VAR(16, activeCollResId & 0xFFF);
		}
	}

	for (curCmd = 0; curCmd < count; curCmd++)
		freeCollision(curCmd + 0x8000);

	for (i = 0; i < 150; i++) {
		if (((_collisionAreas[i].id & 0xF000) == 0xA000) ||
				((_collisionAreas[i].id & 0xF000) == 0x9000))
			_collisionAreas[i].id |= 0x4000;
	}

	_vm->_global->_inter_execPtr = savedIP;
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
		uint16 flags = 0;

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

static const byte adjustTable[] = {
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x21, 0x21, 0x21, 0x21, 0x21, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x81, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,
	0x12, 0x12, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x40, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x40, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x40, 0x40, 0x40, 0x40, 0x20,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int16 Game_v6::adjustKey(int16 key) {
	if (key == -1)
		return -1;

	if (adjustTable[key & 0xFF] & 8)
		return ((key & 0xFF) - 0x20);

	return key & 0xFF;
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
