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
#include "common/file.h"

#include "gob/gob.h"
#include "gob/game.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/inter.h"
#include "gob/draw.h"

namespace Gob {

Game_v6::Game_v6(GobEngine *vm) : Game_v2(vm) {
	_someTimeDly = 0;
}

// flagbits: 5 = freeInterVariables, 6 = skipPlay
void Game_v6::totSub(int8 flags, const char *newTotFile) {
	int8 curBackupPos;

	if ((flags == 16) || (flags == 17))
		warning("Urban Stub: Game_v6::totSub(), flags == %d", flags);

	if (_numEnvironments >= Environments::kEnvironmentCount)
		return;

	_environments->set(_numEnvironments);

	curBackupPos = _curEnvironment;
	_numEnvironments++;
	_curEnvironment = _numEnvironments;

	_script = new Script(_vm);
	_resources = new Resources(_vm);

	if (flags & 0x80)
		warning("Urban Stub: Game_v6::totSub(), flags & 0x80");

	if (flags & 5)
		_vm->_inter->_variables = 0;

	strncpy0(_curTotFile, newTotFile, 9);
	strcat(_curTotFile, ".TOT");

	if (_vm->_inter->_terminate != 0) {
		clearUnusedEnvironment();
		return;
	}

	pushCollisions(0);

	if (flags & 6)
		playTot(-1);
	else
		playTot(0);

	if (_vm->_inter->_terminate < 2)
		_vm->_inter->_terminate = 0;

	clearCollisions();
	popCollisions();

	if ((flags & 5) && _vm->_inter->_variables) {
		_vm->_inter->delocateVars();
	}

	clearUnusedEnvironment();

	_numEnvironments--;
	_curEnvironment = curBackupPos;
	_environments->get(_numEnvironments);
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
		ptr->script = _script;

		return i;
	}
	error("Game_v6::addNewCollision(): Collision array full");
	return 0;
}

void Game_v6::prepareStart(void) {
	_noCd = false;

	if (Common::File::exists("cd1.itk") && Common::File::exists("cd2.itk") &&
	    Common::File::exists("cd3.itk") && Common::File::exists("cd4.itk")) {
		_noCd = true;
	}

	Game_v2::prepareStart();
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

		collSubReenter();
		if (!_vm->_draw->_noInvalidated) {
			if (handleMouse != 0)
				_vm->_draw->animateCursor(-1);
			else
				_vm->_draw->blitInvalidated();
			_vm->_video->waitRetrace();
		}

		key = checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_mouseButtons, handleMouse);

		if ((handleMouse == 0) && (_mouseButtons != kMouseButtonsNone)) {
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
			if (_mouseButtons != kMouseButtonsNone) {

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
							((deltaTime <= 0) || (_mouseButtons == kMouseButtonsNone)))
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
				collSubReenter();
		}

		if ((deltaTime == -2) && (key == 0) && (_mouseButtons == kMouseButtonsNone)) {
			if (pResId != 0)
				*pResId = 0;

			if (pResIndex != 0)
				*pResIndex = 0;

			return 0;

		} else if (handleMouse != 0)
			_vm->_draw->animateCursor(-1);

		if ((deltaTime < 0) && (key == 0) && (_mouseButtons == kMouseButtonsNone)) {
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
	char *str;
	int16 i;
	int16 counter;
	int16 var_24;
	int16 var_26;
	int16 collStackPos;
	Collision *collPtr;
	Collision *collArea;
	byte collAreaStart;
	int16 activeCollResId = 0;
	int16 activeCollIndex = 0;
	uint32 startPos;
	uint32 offsetPos;

	if (_shouldPushColls)
		pushCollisions(0);

	collAreaStart = 0;
	while (_collisionAreas[collAreaStart].left != 0xFFFF)
		collAreaStart++;
	collArea = &_collisionAreas[collAreaStart];

	_shouldPushColls = 0;
	collResId = -1;

	_script->skip(1);
	count = _script->readByte();

	_handleMouse = _script->peekByte(0);
	deltaTime = 1000 * _script->peekByte(1);
	stackPos2 = _script->peekByte(3);
	descIndex = _script->peekByte(4);
	byte var_42 = _script->peekByte(5);

	if ((stackPos2 != 0) || (descIndex != 0)) {
		deltaTime /= 100;
		if (_script->peekByte(1) == 100)
			deltaTime = 2;
	}

	timeVal = deltaTime;
	_script->skip(6);

	startPos = _script->pos();
	WRITE_VAR(16, 0);

	byte var_41 = 0;
	int16 var_46 = 0;
	int16 var_1C = 0;
	int16 index = 0;
	int16 curEditIndex = 0;
	int right = 0, funcLeave = 0;

	for (curCmd = 0; curCmd < count; curCmd++) {
		array[curCmd] = 0;
		cmd = _script->readByte();

		if ((cmd & 0x40) != 0) {
			cmd -= 0x40;
			cmdHigh = _script->readByte();
			cmdHigh <<= 8;
		} else
			cmdHigh = 0;

		if ((cmd & 0x80) != 0) {
			offsetPos = _script->pos();
			left = _script->readValExpr();
			top = _script->readValExpr();
			width = _script->readValExpr();
			height = _script->readValExpr();
		} else {
			offsetPos = 0;
			left = _script->readUint16();
			top = _script->readUint16();
			width = _script->readUint16();
			height = _script->readUint16();
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
			_script->skip(6);
			startPos = _script->pos();
			_script->skip(_script->peekUint16(2) + 2);
			key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					cmd + cmdHigh, key, startPos,
					_script->pos(), offsetPos);

			_script->skip(_script->peekUint16(2) + 2);

			break;

		case 1:
			key = _script->readInt16();
			array[curCmd] = _script->readInt16();
			flags = _script->readInt16();

			startPos = _script->pos();
			_script->skip(_script->peekUint16(2) + 2);

			if (key == 0)
				key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					(flags << 4) + cmd + cmdHigh, key, startPos,
					_script->pos(), offsetPos);

			_script->skip(_script->peekUint16(2) + 2);

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
			key = _script->readVarIndex();
			descArray[index].fontIndex = _script->readInt16();
			descArray[index].backColor = _script->readByte();
			descArray[index].frontColor = _script->readByte();

			if ((cmd >= 5) && (cmd <= 8)) {
				descArray[index].ptr = _script->getData() + _script->pos() + 2;
				_script->skip(_script->peekUint16() + 2);
			} else
				descArray[index].ptr = 0;

			if (left == 0xFFFF) {
				if ((cmd & 1) == 0) {
					_script->skip(_script->peekUint16(2) + 2);
				}
				break;
			}

			right = left + width - 1;
			if (!_vm->_draw->_fonts[descArray[index].fontIndex]->extraData)
				right = left + width * _vm->_draw->_fonts[descArray[index].fontIndex]->itemWidth - 1;

			funcLeave = 0;
			if (!(cmd & 1))
				funcLeave = _script->pos();

			addNewCollision(curCmd + 0x8000, left, top, right,
					top + height - 1, cmd, key, 0, funcLeave, 0);

			if (!(cmd & 1)) {
				_script->skip(_script->peekUint16(2) + 2);
			}

			index++;
			break;

		case 11:
			_script->skip(6);
			for (i = 0; i < 150; i++) {
				if ((_collisionAreas[i].id & 0xF000) == 0xE000) {
					_collisionAreas[i].id &= 0xBFFF;
					_collisionAreas[i].funcEnter =
						_script->pos();
					_collisionAreas[i].funcLeave =
						_script->pos();
				}
			}
			_script->skip(_script->peekUint16(2) + 2);
			break;

		case 12:
			_script->skip(6);
			for (i = 0; i < 150; i++) {
				if ((_collisionAreas[i].id & 0xF000) == 0xD000) {
					_collisionAreas[i].id &= 0xBFFF;
					_collisionAreas[i].funcEnter =
						_script->pos();
					_collisionAreas[i].funcLeave =
						_script->pos();
				}
			}
			_script->skip(_script->peekUint16(2) + 2);
			break;

		case 20:
			collResId = curCmd;
			// Fall through to case 2

		case 2:
			key = _script->readInt16();
			array[curCmd] = _script->readInt16();
			flags = _script->readInt16();

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					(flags << 4) + cmdHigh + 2, key, 0,
					_script->pos(), offsetPos);

			_script->skip(_script->peekUint16(2) + 2);

			break;

		case 21:
			key = _script->readInt16();
			array[curCmd] = _script->readInt16();
			flags = _script->readInt16() & 3;

			addNewCollision(curCmd + 0x8000, left, top,
					left + width - 1, top + height - 1,
					(flags << 4) + cmdHigh + 2, key,
					_script->pos(), 0, offsetPos);

			_script->skip(_script->peekUint16(2) + 2);

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
					&activeCollResId, &activeCollIndex, false);

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
					_vm->_util->cleanupStr(_tempStr);

				int16 pos = 0;
				do {
					strncpy0(_collStr, str, 255);
					pos += strlen(str) + 1;

					str += strlen(str) + 1;

					if ((_collisionAreas[i].flags & 0x0F) < 7)
						_vm->_util->cleanupStr(_collStr);

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

	if (!_vm->_inter->_terminate && (var_41 == 0)) {
		_script->seek(_collisionAreas[activeCollIndex].funcLeave);

		_vm->_inter->storeMouse();
		if (VAR(16) == 0) {
			if ((activeCollResId & 0xF000) == 0x8000)
				WRITE_VAR(16, array[activeCollResId & 0xFFF]);
			else
				WRITE_VAR(16, activeCollResId & 0xFFF);
		}
	} else
		_script->setFinished(true);

	for (curCmd = 0; curCmd < count; curCmd++)
		freeCollision(curCmd + 0x8000);

	for (i = 0; i < 150; i++) {
		if (((_collisionAreas[i].id & 0xF000) == 0xA000) ||
				((_collisionAreas[i].id & 0xF000) == 0x9000))
			_collisionAreas[i].id |= 0x4000;
	}
}

void Game_v6::setCollisions(byte arg_0) {
	for (Collision *collArea = _collisionAreas; collArea->left != 0xFFFF; collArea++) {
		if (((collArea->id & 0xC000) != 0x8000) || (collArea->funcSub == 0))
			continue;

		if (arg_0 == 0)
			if (collArea->flags & 0x80)
				continue;

		Script *curScript = _script;

		_script = collArea->script;
		if (!_script)
			_script = curScript;

		_script->call(collArea->funcSub);

		int16 left   = _script->readValExpr();
		int16 top    = _script->readValExpr();
		int16 width  = _script->readValExpr();
		int16 height = _script->readValExpr();
		uint16 flags = 0;

		if ((collArea->id & 0xF000) == 0xA000)
			flags = _script->readValExpr();

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

		_script->pop();

		_script = curScript;
	}
}

void Game_v6::collSub(uint16 offset) {
	int16 collStackSize;

	uint32 savedPos = _script->pos();

	_script->seek(offset);

	_shouldPushColls = 1;
	collStackSize = _collStackSize;

	_vm->_inter->funcBlock(0);

	if (collStackSize != _collStackSize)
		popCollisions();

	_shouldPushColls = 0;

	_script->seek(savedPos);

	if ((_vm->_util->getTimeKey() - _someTimeDly) > 500)
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

int16 Game_v6::checkMousePoint(int16 all, int16 *resId, int16 *resIndex) {
	Collision *ptr;
	int16 i;

	if (resId != 0)
		*resId = 0;

	*resIndex = 0;

	ptr = _collisionAreas;
	for (i = 0; ptr->left != 0xFFFF; ptr++, i++) {
		if (ptr->id & 0x4000)
			continue;

		if (all) {
			if ((ptr->flags & 0xF) > 1)
				continue;

			if ((ptr->flags & 0xF00) != 0)
				continue;

			if ((_vm->_global->_inter_mouseX < ptr->left) ||
			    (_vm->_global->_inter_mouseX > ptr->right) ||
			    (_vm->_global->_inter_mouseY < ptr->top) ||
			    (_vm->_global->_inter_mouseY > ptr->bottom))
				continue;

			if (resId != 0)
				*resId = ptr->id;

			*resIndex = i;
			return ptr->key;
		} else {
			if ((ptr->flags & 0xF00) != 0)
				continue;

			if ((ptr->flags & 0xF) < 1)
				continue;

			if ((((ptr->flags & 0x70) >> 4) != (((int32) _mouseButtons) - 1)) &&
					(((ptr->flags & 0x70) >> 4) != 2))
				continue;

			if ((_vm->_global->_inter_mouseX < ptr->left) ||
			    (_vm->_global->_inter_mouseX > ptr->right) ||
			    (_vm->_global->_inter_mouseY < ptr->top) ||
			    (_vm->_global->_inter_mouseY > ptr->bottom))
				continue;

			if (resId != 0)
				*resId = ptr->id;
			*resIndex = i;
			if (((ptr->flags & 0xF) == 1) || ((ptr->flags & 0xF) == 2))
				return ptr->key;
			return 0;
		}
	}

	if ((_mouseButtons != kMouseButtonsLeft) && (all == 0))
		return 0x11B;

	return 0;
}

void Game_v6::collSubReenter() {
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
