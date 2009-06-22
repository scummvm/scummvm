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
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/script.h"
#include "gob/draw.h"
#include "gob/inter.h"
#include "gob/mult.h"
#include "gob/video.h"
#include "gob/scenery.h"
#include "gob/sound/sound.h"

namespace Gob {

Game_v1::Game_v1(GobEngine *vm) : Game(vm) {
}

void Game_v1::playTot(int16 skipPlay) {
	int16 _captureCounter;
	int16 breakFrom;
	int16 nestLevel;
	int32 variablesCount;

	int16 *oldNestLevel = _vm->_inter->_nestLevel;
	int16 *oldBreakFrom = _vm->_inter->_breakFromLevel;
	int16 *oldCaptureCounter = _vm->_scenery->_pCaptureCounter;

	_script->push();

	_vm->_inter->_nestLevel = &nestLevel;
	_vm->_inter->_breakFromLevel = &breakFrom;
	_vm->_scenery->_pCaptureCounter = &_captureCounter;

	char savedTotName[20];
	strcpy(savedTotName, _curTotFile);

	if (skipPlay <= 0) {
		while (!_vm->shouldQuit()) {
			for (int i = 0; i < 4; i++) {
				_vm->_draw->_fontToSprite[i].sprite = -1;
				_vm->_draw->_fontToSprite[i].base = -1;
				_vm->_draw->_fontToSprite[i].width = -1;
				_vm->_draw->_fontToSprite[i].height = -1;
			}

			if ((_vm->getPlatform() == Common::kPlatformMacintosh)||
				  (_vm->getPlatform() == Common::kPlatformWindows))
				_vm->_sound->adlibStop();
			else
				_vm->_sound->cdStop();

			_vm->_draw->animateCursor(4);
			_vm->_inter->initControlVars(1);
			_vm->_mult->initAll();
			_vm->_mult->zeroMultData();

			for (int i = 0; i < SPRITES_COUNT; i++)
				_vm->_draw->freeSprite(i);

			_vm->_draw->_spritesArray[20] = _vm->_draw->_frontSurface;
			_vm->_draw->_spritesArray[21] = _vm->_draw->_backSurface;
			_vm->_draw->_spritesArray[23] = _vm->_draw->_cursorSprites;

			for (int i = 0; i < 20; i++)
				freeSoundSlot(i);

			_totTextData = 0;
			_totResourceTable = 0;
			_imFileData = 0;
			_extTable = 0;
			_extHandle = -1;

			_totToLoad[0] = 0;

			if ((_curTotFile[0] == 0) && !_script->isLoaded())
				break;

			if (!_script->load(_curTotFile)) {
				_vm->_draw->blitCursor();
				break;
			}

			strcpy(_curImaFile, _curTotFile);
			strcpy(_curExtFile, _curTotFile);

			_curImaFile[strlen(_curImaFile) - 4] = 0;
			strcat(_curImaFile, ".ima");

			_curExtFile[strlen(_curExtFile) - 4] = 0;
			strcat(_curExtFile, ".ext");

			debugC(4, kDebugFileIO, "IMA: %s", _curImaFile);
			debugC(4, kDebugFileIO, "EXT: %s", _curExtFile);

			byte *filePtr = _script->getData() + 0x30;

			_totTextData = 0;
			if (READ_LE_UINT32(filePtr) != (uint32) -1) {
				_totTextData = new TotTextTable;
				_totTextData->dataPtr =
					(_script->getData() + READ_LE_UINT32(_script->getData() + 0x30));
				Common::MemoryReadStream totTextData(_totTextData->dataPtr,
						4294967295U);

				_totTextData->itemsCount = totTextData.readSint16LE();

				_totTextData->items = new TotTextItem[_totTextData->itemsCount];
				for (int i = 0; i < _totTextData->itemsCount; ++i) {
					_totTextData->items[i].offset = totTextData.readSint16LE();
					_totTextData->items[i].size = totTextData.readSint16LE();
				}
			}

			filePtr = _script->getData() + 0x34;
			_totResourceTable = 0;
			if (READ_LE_UINT32(filePtr) != (uint32) -1) {
				_totResourceTable = new TotResTable;
				_totResourceTable->dataPtr =
					_script->getData() + READ_LE_UINT32(_script->getData() + 0x34);
				Common::MemoryReadStream totResTable(_totResourceTable->dataPtr,
						4294967295U);

				_totResourceTable->itemsCount = totResTable.readSint16LE();
				_totResourceTable->unknown = totResTable.readByte();

				_totResourceTable->items =
					new TotResItem[_totResourceTable->itemsCount];
				for (int i = 0; i < _totResourceTable->itemsCount; ++i) {
					_totResourceTable->items[i].offset = totResTable.readSint32LE();
					_totResourceTable->items[i].size = totResTable.readSint16LE();
					_totResourceTable->items[i].width = totResTable.readSint16LE();
					_totResourceTable->items[i].height = totResTable.readSint16LE();
				}
			}

			loadImFile();
			loadExtTable();

			_vm->_global->_inter_animDataSize =
				READ_LE_UINT16(_script->getData() + 0x38);
			if (!_vm->_inter->_variables)
				_vm->_inter->allocateVars(READ_LE_UINT16(_script->getData() + 0x2C));

			_script->seek(READ_LE_UINT32(_script->getData() + 0x64));

			_vm->_inter->renewTimeInVars();

			WRITE_VAR(13, _vm->_global->_useMouse);
			WRITE_VAR(14, _vm->_global->_soundFlags);
			WRITE_VAR(15, _vm->_global->_videoMode);
			WRITE_VAR(16, _vm->_global->_language);

			_vm->_inter->callSub(2);
			_script->setFinished(false);

			if (_totToLoad[0] != 0)
				_vm->_inter->_terminate = 0;

			variablesCount = READ_LE_UINT32(_script->getData() + 0x2C);
			_vm->_draw->blitInvalidated();

			_script->unload();

			if (_totTextData) {
				delete[] _totTextData->items;
				delete _totTextData;
			}
			_totTextData = 0;

			if (_totResourceTable) {
				delete[] _totResourceTable->items;
				delete _totResourceTable;
			}
			_totResourceTable = 0;

			delete[] _imFileData;
			_imFileData = 0;

			if (_extTable)
				delete[] _extTable->items;
			delete _extTable;
			_extTable = 0;

			if (_extHandle >= 0)
				_vm->_dataIO->closeData(_extHandle);

			_extHandle = -1;

			for (int i = 0; i < *_vm->_scenery->_pCaptureCounter; i++)
				capturePop(0);

			_vm->_mult->checkFreeMult();
			_vm->_mult->freeAll();

			for (int i = 0; i < SPRITES_COUNT; i++)
				_vm->_draw->freeSprite(i);
			_vm->_sound->blasterStop(0);

			for (int i = 0; i < 20; i++)
				freeSoundSlot(i);

			if (_totToLoad[0] == 0)
				break;

			strcpy(_curTotFile, _totToLoad);
		}
	}

	strcpy(_curTotFile, savedTotName);

	_vm->_inter->_nestLevel = oldNestLevel;
	_vm->_inter->_breakFromLevel = oldBreakFrom;
	_vm->_scenery->_pCaptureCounter = oldCaptureCounter;

	_script->pop();
}

void Game_v1::clearCollisions() {
	for (int i = 0; i < 250; i++) {
		_collisionAreas[i].id = 0;
		_collisionAreas[i].left = 0xFFFF;
	}
}

int16 Game_v1::addNewCollision(int16 id, uint16 left, uint16 top,
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

	for (int i = 0; i < 250; i++) {
		if (_collisionAreas[i].left != 0xFFFF)
			continue;

		ptr = &_collisionAreas[i];
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
		ptr->script = 0;

		return i;
	}
	error("Game_v1::addNewCollision(): Collision array full");
	return 0;
}

void Game_v1::pushCollisions(char all) {
	Collision *srcPtr;
	Collision *destPtr;
	int16 size;

	debugC(1, kDebugCollisions, "pushCollisions");
	for (size = 0, srcPtr = _collisionAreas; srcPtr->left != 0xFFFF; srcPtr++) {
		if (all || (srcPtr->id & 0x8000))
			size++;
	}

	destPtr = new Collision[size];
	_collStack[_collStackSize] = destPtr;
	_collStackElemSizes[_collStackSize] = size;
	_collStackSize++;

	for (srcPtr = _collisionAreas; srcPtr->left != 0xFFFF; srcPtr++) {
		if (all || (srcPtr->id & 0x8000)) {
			memcpy(destPtr, srcPtr, sizeof(Collision));
			srcPtr->left = 0xFFFF;
			destPtr++;
		}
	}
}

void Game_v1::popCollisions(void) {
	Collision *destPtr;
	Collision *srcPtr;

	debugC(1, kDebugCollisions, "popCollision");

	_collStackSize--;
	for (destPtr = _collisionAreas; destPtr->left != 0xFFFF; destPtr++)
		;

	srcPtr = _collStack[_collStackSize];
	memcpy(destPtr, srcPtr,
			_collStackElemSizes[_collStackSize] * sizeof(Collision));

	delete[] _collStack[_collStackSize];
}

int16 Game_v1::checkCollisions(byte handleMouse, int16 deltaTime,
		int16 *pResId, int16 *pResIndex) {
	int16 resIndex;
	int16 key;
	int16 oldIndex;
	int16 oldId;
	uint32 timeKey;
	bool firstIteration = true;

	if (deltaTime >= -1) {
		_lastCollKey = 0;
		_lastCollAreaIndex = 0;
		_lastCollId = 0;
	}

	if (pResId != 0)
		*pResId = 0;

	resIndex = 0;

	timeKey = _vm->_util->getTimeKey();

	if ((_vm->_draw->_cursorIndex == -1) && (handleMouse != 0) &&
			(_lastCollKey == 0)) {
		_lastCollKey = checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);

		if ((_lastCollKey != 0) && ((_lastCollId & 0x8000) != 0)) {
			_script->call(_collisionAreas[_lastCollAreaIndex].funcEnter);
			_vm->_inter->funcBlock(0);
			_script->pop();
		}
	}

	if (handleMouse != 0)
		_vm->_draw->animateCursor(-1);

	while (1) {
		if (_vm->_inter->_terminate) {
			if (handleMouse)
				_vm->_draw->blitCursor();
			return 0;
		}

		if (!_vm->_draw->_noInvalidated) {
			if (handleMouse)
				_vm->_draw->animateCursor(-1);
			else
				_vm->_draw->blitInvalidated();
		}

		// NOTE: the original asm does the below checkKeys call
		// _before_ this check. However, that can cause keypresses to get lost
		// since there's a return statement in this check.
		// Additionally, I added a 'deltaTime == -1' check there, since
		// when this function is called with deltaTime == -1 in inputArea,
		// and the return value is then discarded.
		if (deltaTime < 0) {
			uint32 curtime = _vm->_util->getTimeKey();
			if ((deltaTime == -1) || (((curtime + deltaTime) > timeKey) && !firstIteration)) {
				if (pResId != 0)
					*pResId = 0;

				if (pResIndex != 0)
					*pResIndex = 0;

				return 0;
			}
		}

		key = checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_mouseButtons, handleMouse);

		if ((handleMouse == 0) && (_mouseButtons != 0)) {
			_vm->_util->waitMouseRelease(0);
			key = 3;
		}

		if (key != 0) {

			if (handleMouse == 1)
				_vm->_draw->blitCursor();

			if (pResId != 0)
				*pResId = 0;

			if (pResIndex != 0)
				*pResIndex = 0;

			if ((_lastCollKey != 0) &&
			    (_collisionAreas[_lastCollAreaIndex].funcLeave != 0)) {

				_script->call(_collisionAreas[_lastCollAreaIndex].funcLeave);
				_vm->_inter->funcBlock(0);
				_script->pop();
			}

			_lastCollKey = 0;
			if (key != 0)
				return key;
		}

		if (handleMouse != 0) {
			if (_mouseButtons != 0) {
				oldIndex = 0;

				_vm->_draw->animateCursor(2);
				if (deltaTime <= 0) {
					if (handleMouse == 1)
						_vm->_util->waitMouseRelease(1);
				} else if (deltaTime > 0)
					_vm->_util->delay(deltaTime);

				_vm->_draw->animateCursor(-1);
				if (pResId != 0)
					*pResId = 0;

				key = checkMousePoint(0, pResId, &resIndex);

				if (pResIndex != 0)
					*pResIndex = resIndex;

				if ((key != 0) || ((pResId != 0) && (*pResId != 0))) {
					if ((handleMouse == 1) &&
							((deltaTime <= 0) || (_mouseButtons == 0)))
						_vm->_draw->blitCursor();

					if ((_lastCollKey != 0) &&
						(_collisionAreas[_lastCollAreaIndex].funcLeave != 0)) {

						_script->call(_collisionAreas[_lastCollAreaIndex].funcLeave);
						_vm->_inter->funcBlock(0);
						_script->pop();

					}
					_lastCollKey = 0;
					return key;
				}

				if ((_lastCollKey != 0) &&
				    (_collisionAreas[_lastCollAreaIndex].funcLeave != 0)) {

					_script->call(_collisionAreas[_lastCollAreaIndex].funcLeave);
					_vm->_inter->funcBlock(0);
					_script->pop();

				}

				_lastCollKey =
					checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);

				if ((_lastCollKey != 0) && ((_lastCollId & 0x8000) != 0)) {

					_script->call(_collisionAreas[_lastCollAreaIndex].funcEnter);
					_vm->_inter->funcBlock(0);
					_script->pop();

				}
			} else {

				if ((handleMouse != 0) &&
				    ((_vm->_global->_inter_mouseX != _vm->_draw->_cursorX) ||
						(_vm->_global->_inter_mouseY != _vm->_draw->_cursorY))) {
					oldIndex = _lastCollAreaIndex;
					oldId = _lastCollId;

					key = checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);

					if (key != _lastCollKey) {
						if ((_lastCollKey != 0) && ((oldId & 0x8000) != 0)) {

							_script->call(_collisionAreas[oldIndex].funcLeave);
							_vm->_inter->funcBlock(0);
							_script->pop();

						}

						_lastCollKey = key;
						if ((_lastCollKey != 0) && ((_lastCollId & 0x8000) != 0)) {

							_script->call(_collisionAreas[_lastCollAreaIndex].funcEnter);
							_vm->_inter->funcBlock(0);
							_script->pop();

						}

					}
				}
			}
		}

		if (handleMouse != 0)
			_vm->_draw->animateCursor(-1);

		if (deltaTime < -10)
      _vm->_util->delay(10);

		firstIteration = false;
	}
}

void Game_v1::prepareStart(void) {
	clearCollisions();

	_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->initScreen();
	_vm->_video->fillRect(*_vm->_draw->_backSurface, 0, 0, 319, 199, 1);
	_vm->_draw->_frontSurface = _vm->_global->_primarySurfDesc;
	_vm->_video->fillRect(*_vm->_draw->_frontSurface, 0, 0, 319, 199, 1);

	_vm->_util->setMousePos(152, 92);
	_vm->_draw->_cursorX = _vm->_global->_inter_mouseX = 152;
	_vm->_draw->_cursorY = _vm->_global->_inter_mouseY = 92;

	_vm->_draw->_invalidatedCount = 0;
	_vm->_draw->_noInvalidated = true;
	_vm->_draw->_applyPal = false;
	_vm->_draw->_paletteCleared = false;
	_vm->_draw->_cursorWidth = 16;
	_vm->_draw->_cursorHeight = 16;
	_vm->_draw->_transparentCursor = 1;

	for (int i = 0; i < 40; i++) {
		_vm->_draw->_cursorAnimLow[i] = -1;
		_vm->_draw->_cursorAnimDelays[i] = 0;
		_vm->_draw->_cursorAnimHigh[i] = 0;
	}

	_vm->_draw->_cursorAnimLow[1] = 0;
	_vm->_draw->_cursorSprites =
		_vm->_video->initSurfDesc(_vm->_global->_videoMode, 32, 16, 2);
	_vm->_draw->_scummvmCursor =
		_vm->_video->initSurfDesc(_vm->_global->_videoMode, 16, 16, SCUMMVM_CURSOR);
	_vm->_draw->_renderFlags = 0;
	_vm->_draw->_backDeltaX = 0;
	_vm->_draw->_backDeltaY = 0;

	_startTimeKey = _vm->_util->getTimeKey();
}

void Game_v1::collisionsBlock(void) {
	InputDesc descArray[20];
	int16 array[250];
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
	int16 var_22;
	int16 index;
	int16 curEditIndex;
	int16 deltaTime;
	int16 descIndex2;
	int16 stackPos2;
	int16 descIndex;
	int16 timeVal;
	char *str;
	int16 savedCollStackSize;
	int16 i;
	int16 counter;
	int16 var_24;
	int16 var_26;
	int16 collStackPos;
	Collision *collPtr;
	uint32 timeKey;

	if (_shouldPushColls)
		pushCollisions(1);

	collResId = -1;
	_script->skip(1);
	count = _script->readByte();
	_handleMouse = _script->readByte();
	deltaTime = 1000 * _script->readByte();
	descIndex2 = _script->readByte();
	stackPos2 = _script->readByte();
	descIndex = _script->readByte();

	if ((stackPos2 != 0) || (descIndex != 0))
		deltaTime /= 100;

	timeVal = deltaTime;
	_script->skip(1);

	uint32 startPos = _script->pos();

	WRITE_VAR(16, 0);
	var_22 = 0;
	index = 0;
	curEditIndex = 0;

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
			left = _script->readValExpr();
			top = _script->readValExpr();
			width = _script->readValExpr();
			height = _script->readValExpr();
		} else {
			left = _script->readUint16();
			top = _script->readUint16();
			width = _script->readUint16();
			height = _script->readUint16();
		}
		cmd &= 0x7F;

		debugC(1, kDebugCollisions, "collisionsBlock(%d)", cmd);

		switch (cmd) {
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:

			_vm->_util->clearKeyBuf();
			var_22 = 1;
			key = _script->readVarIndex();
			descArray[index].fontIndex = _script->readInt16();
			descArray[index].backColor = _script->readByte();
			descArray[index].frontColor = _script->readByte();

			if ((cmd < 5) || (cmd > 8)) {
				descArray[index].ptr = 0;
			} else {
				descArray[index].ptr = _script->getData() + _script->pos() + 2;
				_script->skip(_script->readInt16());
			}

			if (left == 0xFFFF)
				break;

			if ((cmd & 1) == 0) {
				addNewCollision(curCmd + 0x8000, left, top, left + width *
						_vm->_draw->_fonts[descArray[index].fontIndex]->itemWidth - 1,
						top + height - 1, cmd, key, 0, _script->pos());

				_script->skip(_script->peekUint16(2) + 2);
			} else {
				addNewCollision(curCmd + 0x8000, left, top, left + width *
						_vm->_draw->_fonts[descArray[index].fontIndex]->itemWidth - 1,
						top + height - 1, cmd, key, 0, 0);
			}
			index++;
			break;

		case 21:
			key = _script->readInt16();
			array[curCmd] = _script->readInt16();
			flags = _script->readInt16() & 3;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1, top + height - 1,
			    (flags << 4) + cmdHigh + 2, key, _script->pos(), 0);

			_script->skip(_script->peekUint16(2) + 2);
			break;

		case 20:
			collResId = curCmd;
			// Fall through to case 2

		case 2:
			key = _script->readInt16();
			array[curCmd] = _script->readInt16();
			flags = _script->readInt16() & 3;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmdHigh + 2, key, 0, _script->pos());

			_script->skip(_script->peekUint16(2) + 2);
			break;

		case 0:
			_script->skip(6);
			startPos = _script->pos();
			_script->skip(_script->peekUint16(2) + 2);
			key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1, top + height - 1,
			    cmd + cmdHigh, key,
			    startPos, _script->pos());

			_script->skip(_script->peekUint16(2) + 2);
			break;

		case 1:
			key = _script->readInt16();
			array[curCmd] = _script->readInt16();
			flags = _script->readInt16() & 3;

			startPos = _script->pos();
			_script->skip(_script->peekUint16(2) + 2);
			if (key == 0)
				key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1, top + height - 1,
			    (flags << 4) + cmd + cmdHigh, key,
			    startPos, _script->pos());

			_script->skip(_script->peekUint16(2) + 2);
			break;
		}
	}

	_forceHandleMouse = 0;
	_vm->_util->clearKeyBuf();

	do {
		if (var_22 != 0) {
			key = multiEdit(deltaTime, index, &curEditIndex,
					descArray, &_activeCollResId, &_activeCollIndex);

			if (key == 0x1C0D) {
				for (i = 0; i < 250; i++) {
					if (_collisionAreas[i].left == 0xFFFF)
						continue;

					if ((_collisionAreas[i].id & 0x8000) == 0)
						continue;

					if ((_collisionAreas[i].flags & 1) != 0)
						continue;

					if ((_collisionAreas[i].flags & 0x0F) <= 2)
						continue;

					collResId = _collisionAreas[i].id;
					_activeCollResId = collResId;
					collResId &= 0x7FFF;
					_activeCollIndex = i;
					break;
				}
				break;
			}
		} else
			key = checkCollisions(_handleMouse, -deltaTime,
			    &_activeCollResId, &_activeCollIndex);

		if (((key & 0xFF) >= ' ') && ((key & 0xFF) <= 0xFF) &&
		    ((key >> 8) > 1) && ((key >> 8) < 12))
			key = '0' + (((key >> 8) - 1) % 10) + (key & 0xFF00);

		if (_activeCollResId == 0) {
			if (key != 0) {
				for (i = 0; i < 250; i++) {
					if (_collisionAreas[i].left == 0xFFFF)
						continue;

					if ((_collisionAreas[i].id & 0x8000) == 0)
						continue;

					if ((_collisionAreas[i].key == key) ||
							(_collisionAreas[i].key == 0x7FFF)) {

						_activeCollResId = _collisionAreas[i].id;
						_activeCollIndex = i;
						break;
					}
				}

				if (_activeCollResId == 0) {
					for (i = 0; i < 250; i++) {
						if (_collisionAreas[i].left == 0xFFFF)
							continue;

						if ((_collisionAreas[i].id & 0x8000) == 0)
							continue;

						if ((_collisionAreas[i].key & 0xFF00) != 0)
							continue;

						if (_collisionAreas[i].key == 0)
							continue;

						if ((adjustKey(key & 0xFF) == adjustKey(_collisionAreas[i].key)) ||
								(_collisionAreas[i].key == 0x7FFF)) {
							_activeCollResId = _collisionAreas[i].id;
							_activeCollIndex = i;
							break;
						}
					}
				}
			} else {

				if ((deltaTime != 0) && (VAR(16) == 0)) {
					if (stackPos2 != 0) {
						collStackPos = 0;
						collPtr = _collisionAreas;

						for (i = 0, collPtr = _collisionAreas;
								collPtr->left != 0xFFFF; i++, collPtr++) {

							if ((collPtr->id & 0x8000) == 0)
								continue;

							collStackPos++;
							if (collStackPos != stackPos2)
								continue;

							_activeCollResId = collPtr->id;
							_activeCollIndex = i;
							WRITE_VAR(2, _vm->_global->_inter_mouseX);
							WRITE_VAR(3, _vm->_global->_inter_mouseY);
							WRITE_VAR(4, _mouseButtons);
							WRITE_VAR(16, array[(uint16) _activeCollResId & ~0x8000]);

							if (collPtr->funcLeave != 0) {
								timeKey = _vm->_util->getTimeKey();

								uint32 savedPos = _script->pos();

								_script->seek(collPtr->funcLeave);

								_shouldPushColls = 1;
								savedCollStackSize = _collStackSize;
								_vm->_inter->funcBlock(0);

								if (savedCollStackSize != _collStackSize)
									popCollisions();

								_shouldPushColls = 0;

								_script->seek(savedPos);

								deltaTime = timeVal -
									(_vm->_util->getTimeKey() - timeKey);

								if (deltaTime < 2)
									deltaTime = 2;
							}

							if (VAR(16) == 0)
								_activeCollResId = 0;
							break;
						}
					} else {
						if (descIndex != 0) {
							counter = 0;
							for (i = 0; i < 250; i++) {
								if (_collisionAreas[i].left == 0xFFFF)
									continue;

								if ((_collisionAreas[i].id & 0x8000) == 0)
									continue;

								counter++;
								if (counter != descIndex)
									continue;

								_activeCollResId = _collisionAreas[i].id;
								_activeCollIndex = i;
								break;
							}
						} else {
							for (i = 0; i < 250; i++) {
								if (_collisionAreas[i].left == 0xFFFF)
									continue;

								if ((_collisionAreas[i].id & 0x8000) == 0)
									continue;

								_activeCollResId = _collisionAreas[i].id;
								_activeCollIndex = i;
								break;
							}
						}
					}
				} else {
					if (descIndex2 != 0) {
						counter = 0;
						for (i = 0; i < 250; i++) {
							if (_collisionAreas[i].left == 0xFFFF)
								continue;

							if ((_collisionAreas[i].id & 0x8000) == 0)
								continue;

							counter++;
							if (counter != descIndex2)
								continue;

							_activeCollResId = _collisionAreas[i].id;
							_activeCollIndex = i;
							break;
						}
					}
				}
			}
		}

		if (_activeCollResId == 0)
			continue;

		if (_collisionAreas[_activeCollIndex].funcLeave != 0)
			continue;

		WRITE_VAR(2, _vm->_global->_inter_mouseX);
		WRITE_VAR(3, _vm->_global->_inter_mouseY);
		WRITE_VAR(4, _mouseButtons);
		WRITE_VAR(16, array[(uint16) _activeCollResId & ~0x8000]);

		if (_collisionAreas[_activeCollIndex].funcEnter != 0) {
			uint32 savedPos = _script->pos();

			_script->seek(_collisionAreas[_activeCollIndex].funcEnter);

			_shouldPushColls = 1;

			collStackPos = _collStackSize;
			_vm->_inter->funcBlock(0);
			if (collStackPos != _collStackSize)
				popCollisions();
			_shouldPushColls = 0;

			_script->seek(savedPos);
		}

		WRITE_VAR(16, 0);
		_activeCollResId = 0;
	}
	while ((_activeCollResId == 0) && !_vm->_inter->_terminate && !_vm->shouldQuit());

	if (((uint16) _activeCollResId & ~0x8000) == collResId) {
		collStackPos = 0;
		var_24 = 0;
		var_26 = 1;
		for (i = 0; i < 250; i++) {
			if (_collisionAreas[i].left == 0xFFFF)
				continue;

			if ((_collisionAreas[i].id & 0x8000) == 0)
				continue;

			if ((_collisionAreas[i].flags & 0x0F) < 3)
				continue;

			if ((_collisionAreas[i].flags & 0x0F) > 10)
				continue;

			if ((_collisionAreas[i].flags & 0x0F) > 8) {
				char *ptr;
				strncpy0(_tempStr, GET_VARO_STR(_collisionAreas[i].key), 255);
				while ((ptr = strchr(_tempStr, ' ')) != 0) {
					_vm->_util->cutFromStr(_tempStr, (ptr - _tempStr), 1);
					ptr = strchr(_tempStr, ' ');
				}
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

	if (!_vm->_inter->_terminate) {
		_script->seek(_collisionAreas[_activeCollIndex].funcLeave);

		WRITE_VAR(2, _vm->_global->_inter_mouseX);
		WRITE_VAR(3, _vm->_global->_inter_mouseY);
		WRITE_VAR(4, _mouseButtons);

		if (VAR(16) == 0)
			WRITE_VAR(16, array[(uint16) _activeCollResId & ~0x8000]);
	} else
		_script->setFinished(true);

	for (curCmd = 0; curCmd < count; curCmd++)
		freeCollision(curCmd + 0x8000);
}

int16 Game_v1::multiEdit(int16 time, int16 index, int16 *pCurPos,
		InputDesc * inpDesc, int16 *collResId, int16 *collIndex, bool mono) {
	Collision *collArea;
	int16 descInd;
	int16 key;
	int16 found = -1;
	int16 i;

	descInd = 0;
	for (i = 0; i < 250; i++) {
		collArea = &_collisionAreas[i];

		if (collArea->left == 0xFFFF)
			continue;

		if ((collArea->id & 0x8000) == 0)
			continue;

		if ((collArea->flags & 0x0F) < 3)
			continue;

		if ((collArea->flags & 0x0F) > 10)
			continue;

		strncpy0(_tempStr, GET_VARO_STR(collArea->key), 255);

		_vm->_draw->_destSpriteX = collArea->left;
		_vm->_draw->_destSpriteY = collArea->top;
		_vm->_draw->_spriteRight = collArea->right - collArea->left + 1;
		_vm->_draw->_spriteBottom = collArea->bottom - collArea->top + 1;

		_vm->_draw->_destSurface = 21;

		_vm->_draw->_backColor = inpDesc[descInd].backColor;
		_vm->_draw->_frontColor = inpDesc[descInd].frontColor;
		_vm->_draw->_textToPrint = _tempStr;
		_vm->_draw->_transparency = 1;
		_vm->_draw->_fontIndex = inpDesc[descInd].fontIndex;
		_vm->_draw->spriteOperation(DRAW_FILLRECT);
		_vm->_draw->_destSpriteY +=
		    ((collArea->bottom - collArea->top + 1) - 8) / 2;

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
		descInd++;
	}

	for (i = 0; i < 40; i++)
		WRITE_VAR_OFFSET(i * 4 + 0x44, 0);

	while (1) {
		descInd = 0;

		for (i = 0; i < 250; i++) {
			collArea = &_collisionAreas[i];

			if (collArea->left == 0xFFFF)
				continue;

			if ((collArea->id & 0x8000) == 0)
				continue;

			if ((collArea->flags & 0x0F) < 3)
				continue;

			if ((collArea->flags & 0x0F) > 10)
				continue;

			if (descInd == *pCurPos) {
				found = i;
				break;
			}

			descInd++;
		}

		assert(found != -1);

		collArea = &_collisionAreas[found];

		key = inputArea(collArea->left, collArea->top,
		    collArea->right - collArea->left + 1,
		    collArea->bottom - collArea->top + 1,
		    inpDesc[*pCurPos].backColor, inpDesc[*pCurPos].frontColor,
				GET_VARO_STR(collArea->key), inpDesc[*pCurPos].fontIndex,
				collArea->flags, &time, collResId, collIndex);

		if (_vm->_inter->_terminate)
			return 0;

		switch (key) {
		case 0:
			if (*collResId == 0)
				return 0;

			if ((_collisionAreas[*collIndex].flags & 0x0F) < 3)
				return 0;

			if ((_collisionAreas[*collIndex].flags & 0x0F) > 10)
				return 0;

			*pCurPos = 0;
			for (i = 0; i < 250; i++) {
				collArea = &_collisionAreas[i];

				if (collArea->left == 0xFFFF)
					continue;

				if ((collArea->id & 0x8000) == 0)
					continue;

				if ((collArea->flags & 0x0F) < 3)
					continue;

				if ((collArea->flags & 0x0F) > 10)
					continue;

				if (i == *collIndex)
					break;

				pCurPos[0]++;
			}
			break;

		case 0x3B00:
		case 0x3C00:
		case 0x3D00:
		case 0x3E00:
		case 0x3F00:
		case 0x4000:
		case 0x4100:
		case 0x4200:
		case 0x4300:
		case 0x4400:
			return key;

		case 0x1C0D:

			if (index == 1)
				return key;

			if (*pCurPos == index - 1) {
				*pCurPos = 0;
				break;
			}

			pCurPos[0]++;
			break;

		case 0x5000:
			if (index - 1 > *pCurPos)
				pCurPos[0]++;
			break;

		case 0x4800:
			if (*pCurPos > 0)
				pCurPos[0]--;
			break;
		}
	}
}

int16 Game_v1::inputArea(int16 xPos, int16 yPos, int16 width, int16 height,
		int16 backColor, int16 frontColor, char *str, int16 fontIndex,
		char inpType, int16 *pTotTime, int16 *collResId, int16 *collIndex, bool mono) {
	int16 handleMouse;
	uint32 editSize;
	Video::FontDesc *pFont;
	char curSym;
	int16 key;
	const char *str1;
	const char *str2;
	int16 i;
	uint32 pos;
	int16 flag;
	int16 savedKey;

	if ((_handleMouse != 0) &&
	    ((_vm->_global->_useMouse != 0) || (_forceHandleMouse != 0)))
		handleMouse = 1;
	else
		handleMouse = 0;

	pos = strlen(str);
	pFont = _vm->_draw->_fonts[fontIndex];
	editSize = width / pFont->itemWidth;

	while (1) {
		strncpy0(_tempStr, str, 254);
		strcat(_tempStr, " ");
		if (strlen(_tempStr) > editSize)
			strncpy0(_tempStr, str, 255);

		_vm->_draw->_destSpriteX = xPos;
		_vm->_draw->_destSpriteY = yPos;
		_vm->_draw->_spriteRight = editSize * pFont->itemWidth;
		_vm->_draw->_spriteBottom = height;

		_vm->_draw->_destSurface = 21;
		_vm->_draw->_backColor = backColor;
		_vm->_draw->_frontColor = frontColor;
		_vm->_draw->_textToPrint = _tempStr;
		_vm->_draw->_transparency = 1;
		_vm->_draw->_fontIndex = fontIndex;
		_vm->_draw->spriteOperation(DRAW_FILLRECT);

		_vm->_draw->_destSpriteY = yPos + (height - 8) / 2;

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
		if (pos == editSize)
			pos--;

		curSym = _tempStr[pos];

		flag = 1;

		while (1) {
			_tempStr[0] = curSym;
			_tempStr[1] = 0;

			_vm->_draw->_destSpriteX = xPos + pFont->itemWidth * pos;
			_vm->_draw->_destSpriteY = yPos + height - 1;
			_vm->_draw->_spriteRight = pFont->itemWidth;
			_vm->_draw->_spriteBottom = 1;
			_vm->_draw->_destSurface = 21;
			_vm->_draw->_backColor = frontColor;
			_vm->_draw->spriteOperation(DRAW_FILLRECT);

			if (flag != 0)
				key = checkCollisions(handleMouse, -1, collResId, collIndex);
			flag = 0;

			key = checkCollisions(handleMouse, -300, collResId, collIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					*collResId = 0;
					break;
				}
			}

			_tempStr[0] = curSym;
			_tempStr[1] = 0;
			_vm->_draw->_destSpriteX = xPos + pFont->itemWidth * pos;
			_vm->_draw->_destSpriteY = yPos + height - 1;
			_vm->_draw->_spriteRight = pFont->itemWidth;
			_vm->_draw->_spriteBottom = 1;
			_vm->_draw->_destSurface = 21;
			_vm->_draw->_backColor = backColor;
			_vm->_draw->_frontColor = frontColor;
			_vm->_draw->_textToPrint = _tempStr;
			_vm->_draw->_transparency = 1;
			_vm->_draw->spriteOperation(DRAW_FILLRECT);

			_vm->_draw->_destSpriteY = yPos + (height - 8) / 2;
			_vm->_draw->spriteOperation(DRAW_PRINTTEXT);

			if ((key != 0) || (*collResId != 0))
				break;

			key = checkCollisions(handleMouse, -300, collResId, collIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					*collResId = 0;
					break;
				}
			}

			if ((key != 0) || (*collResId != 0))
				break;

			if (_vm->_inter->_terminate)
				return 0;
		}

		if ((key == 0) || (*collResId != 0) || _vm->_inter->_terminate)
			return 0;

		switch (key) {
		case 0x4D00: // Right Arrow
			if ((pos < strlen(str)) && (pos < (editSize - 1))) {
				pos++;
				continue;
			}
			return 0x5000;

		case 0x4B00: // Left Arrow
			if (pos > 0) {
				pos--;
				continue;
			}
			return 0x4800;

		case 0xE08: // Backspace
			if (pos > 0) {
				_vm->_util->cutFromStr(str, pos - 1, 1);
				pos--;
				continue;
			}

		case 0x5300: // Del

			if (pos >= strlen(str))
				continue;

			_vm->_util->cutFromStr(str, pos, 1);
			continue;

		case 0x1C0D: // Enter
		case 0x3B00: // F1
		case 0x3C00: // F2
		case 0x3D00: // F3
		case 0x3E00: // F4
		case 0x3F00: // F5
		case 0x4000: // F6
		case 0x4100: // F7
		case 0x4200: // F8
		case 0x4300: // F9
		case 0x4400: // F10
		case 0x4800: // Up arrow
		case 0x5000: // Down arrow
			return key;

		case 0x11B: // Escape
			if (_vm->_global->_useMouse != 0)
				continue;

			_forceHandleMouse = !_forceHandleMouse;

			if ((_handleMouse != 0) &&
			    ((_vm->_global->_useMouse != 0) || (_forceHandleMouse != 0)))
				handleMouse = 1;
			else
				handleMouse = 0;

			if (_vm->_global->_pressedKeys[1] == 0)
				continue;

			while (_vm->_global->_pressedKeys[1] != 0)
				;
			continue;

		default:

			savedKey = key;
			key &= 0xFF;

			if (((inpType == 9) || (inpType == 10)) &&
					(key >= ' ') && (key <= 0xFF)) {
				str1 = "0123456789-.,+ ";
				str2 = "0123456789-,,+ ";

				if (((savedKey >> 8) > 1) && ((savedKey >> 8) < 12))
					key = ((savedKey >> 8) - 1) % 10 + '0';

				for (i = 0; str1[i] != 0; i++) {
					if (key == str1[i]) {
						key = str2[i];
						break;
					}
				}

				if (i == (int16) strlen(str1))
					key = 0;
			}

			if ((key >= ' ') && (key <= 0xFF)) {
				if (editSize == strlen(str))
					_vm->_util->cutFromStr(str, strlen(str) - 1, 1);

				if ((key >= 'a') && (key <= 'z'))
					key += ('A' - 'a');

				pos++;
				_tempStr[0] = key;
				_tempStr[1] = 0;

				_vm->_util->insertStr(_tempStr, str, pos - 1);
			}

		}
	}
}

int16 Game_v1::checkMousePoint(int16 all, int16 *resId, int16 *resIndex) {
	Collision *ptr;
	int16 i;

	if (resId != 0)
		*resId = 0;

	*resIndex = 0;

	ptr = _collisionAreas;
	for (i = 0; ptr->left != 0xFFFF; ptr++, i++) {
		if (all) {
			if ((ptr->flags & 0xF) > 1)
				continue;

			if ((ptr->flags & 0xFF00) != 0)
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
			if ((ptr->flags & 0xFF00) != 0)
				continue;

			if (((ptr->flags & 0xF) != 1) && ((ptr->flags & 0xF) != 2))
				continue;

			if ((((ptr->flags & 0xF0) >> 4) != (_mouseButtons - 1))
					&& (((ptr->flags & 0xF0) >> 4) != 2))
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
		}
	}

	if ((_mouseButtons != 1) && (all == 0))
		return 0x11B;

	return 0;
}

} // End of namespace Gob
