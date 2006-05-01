/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/video.h"
#include "gob/dataio.h"
#include "gob/pack.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/parse.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/util.h"
#include "gob/goblin.h"
#include "gob/cdrom.h"
#include "gob/music.h"

namespace Gob {

Game_v1::Game_v1(GobEngine *vm) : Game(vm) {
}

void Game_v1::playTot(int16 skipPlay) {
	char savedTotName[20];
	int16 *oldCaptureCounter;
	int16 *oldBreakFrom;
	int16 *oldNestLevel;
	int16 _captureCounter;
	int16 breakFrom;
	int16 nestLevel;
	char needTextFree;
	char needFreeResTable;
	char *curPtr;
	int32 variablesCount;
	char *filePtr;
	char *savedIP;
	int16 i;

	oldNestLevel = _vm->_inter->_nestLevel;
	oldBreakFrom = _vm->_inter->_breakFromLevel;
	oldCaptureCounter = _vm->_scenery->_pCaptureCounter;
	savedIP = _vm->_global->_inter_execPtr;

	_vm->_inter->_nestLevel = &nestLevel;
	_vm->_inter->_breakFromLevel = &breakFrom;
	_vm->_scenery->_pCaptureCounter = &_captureCounter;
	strcpy(savedTotName, _curTotFile);

	if (skipPlay == 0) {
		while (1) {
			for (i = 0; i < 4; i++) {
				_vm->_draw->_fontToSprite[i].sprite = -1;
				_vm->_draw->_fontToSprite[i].base = -1;
				_vm->_draw->_fontToSprite[i].width = -1;
				_vm->_draw->_fontToSprite[i].height = -1;
			}

			if(_vm->_features & GF_MAC)
				_vm->_music->stopPlay();
			else
				_vm->_cdrom->stopPlaying();
			_vm->_draw->animateCursor(4);
			_vm->_inter->initControlVars();
			_vm->_mult->initAll();
			_vm->_mult->zeroMultData();

			for (i = 0; i < 20; i++)
				_vm->_draw->_spritesArray[i] = 0;

			_vm->_draw->_spritesArray[20] = _vm->_draw->_frontSurface;
			_vm->_draw->_spritesArray[21] = _vm->_draw->_backSurface;
			_vm->_draw->_spritesArray[23] = _vm->_draw->_cursorSprites;

			for (i = 0; i < 20; i++)
				_soundSamples[i] = 0;

			_totTextData = 0;
			_totResourceTable = 0;
			_imFileData = 0;
			_extTable = 0;
			_extHandle = -1;

			needFreeResTable = 1;
			needTextFree = 1;

			_totToLoad[0] = 0;

			if (_curTotFile[0] == 0 && _totFileData == 0)
				break;

			loadTotFile(_curTotFile);
			if (_totFileData == 0) {
				_vm->_draw->blitCursor();
				break;
			}

			strcpy(_curImaFile, _curTotFile);
			strcpy(_curExtFile, _curTotFile);

			_curImaFile[strlen(_curImaFile) - 4] = 0;
			strcat(_curImaFile, ".ima");

			_curExtFile[strlen(_curExtFile) - 4] = 0;
			strcat(_curExtFile, ".ext");

			debugC(4, DEBUG_FILEIO, "IMA: %s", _curImaFile);
			debugC(4, DEBUG_FILEIO, "EXT: %s", _curExtFile);

			filePtr = (char *)_totFileData + 0x30;

			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = _totFileData;
				_totTextData =
						(TotTextTable *) (curPtr +
						READ_LE_UINT32((char *)_totFileData + 0x30));

				_totTextData->itemsCount = (int16)READ_LE_UINT16(&_totTextData->itemsCount);

				for (i = 0; i < _totTextData->itemsCount; ++i) {
					_totTextData->items[i].offset = (int16)READ_LE_UINT16(&_totTextData->items[i].offset);
					_totTextData->items[i].size = (int16)READ_LE_UINT16(&_totTextData->items[i].size);
				}

				needTextFree = 0;
			}

			filePtr = (char *)_totFileData + 0x34;
			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = _totFileData;

				_totResourceTable =
					(TotResTable *)(curPtr +
				    READ_LE_UINT32((char *)_totFileData + 0x34));

				_totResourceTable->itemsCount = (int16)READ_LE_UINT16(&_totResourceTable->itemsCount);

				for (i = 0; i < _totResourceTable->itemsCount; ++i) {
					_totResourceTable->items[i].offset = (int32)READ_LE_UINT32(&_totResourceTable->items[i].offset);
					_totResourceTable->items[i].size = (int16)READ_LE_UINT16(&_totResourceTable->items[i].size);
					_totResourceTable->items[i].width = (int16)READ_LE_UINT16(&_totResourceTable->items[i].width);
					_totResourceTable->items[i].height = (int16)READ_LE_UINT16(&_totResourceTable->items[i].height);
				}

				needFreeResTable = 0;
			}

			loadImFile();
			loadExtTable();

			_vm->_global->_inter_animDataSize = READ_LE_UINT16((char *)_totFileData + 0x38);
			if (_vm->_global->_inter_variables == 0) {
				variablesCount = READ_LE_UINT32((char *)_totFileData + 0x2c);
				_vm->_global->_inter_variables = new char[variablesCount * 4];
				for (i = 0; i < variablesCount; i++)
					WRITE_VAR(i, 0);
			}

			_vm->_global->_inter_execPtr = (char *)_totFileData;
			_vm->_global->_inter_execPtr += READ_LE_UINT32((char *)_totFileData + 0x64);

			_vm->_inter->renewTimeInVars();

			WRITE_VAR(13, _vm->_global->_useMouse);
			WRITE_VAR(14, _vm->_global->_soundFlags);
			WRITE_VAR(15, _vm->_global->_videoMode);
			WRITE_VAR(16, _vm->_global->_language);

			_vm->_inter->callSub(2);

			if (_totToLoad[0] != 0)
				_vm->_inter->_terminate = false;

			variablesCount = READ_LE_UINT32((char *)_totFileData + 0x2c);
			_vm->_draw->blitInvalidated();
			delete[] _totFileData;
			_totFileData = 0;

			if (needTextFree)
				delete[] _totTextData;
			_totTextData = 0;

			if (needFreeResTable)
				delete[] _totResourceTable;
			_totResourceTable = 0;

			delete[] _imFileData;
			_imFileData = 0;

			if (_extTable)
				delete[] _extTable->items;
			delete _extTable;
			_extTable = 0;

			if (_extHandle >= 0)
				_vm->_dataio->closeData(_extHandle);

			_extHandle = -1;

			for (i = 0; i < *_vm->_scenery->_pCaptureCounter; i++)
				capturePop(0);

			_vm->_mult->checkFreeMult();
			_vm->_mult->freeAll();

			for (i = 0; i < 20; i++) {
				if (_vm->_draw->_spritesArray[i] != 0)
					_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[i]);
				_vm->_draw->_spritesArray[i] = 0;
			}
			_vm->_snd->stopSound(0);

			for (i = 0; i < 20; i++)
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
	_vm->_global->_inter_execPtr = savedIP;
}

void Game_v1::clearCollisions() {
	int16 i;
	for (i = 0; i < 250; i++) {
		_collisionAreas[i].id = 0;
		_collisionAreas[i].left = -1;
	}
}

void Game_v1::addNewCollision(int16 id, int16 left, int16 top, int16 right, int16 bottom,
	    int16 flags, int16 key, int16 funcEnter, int16 funcLeave) {
	int16 i;
	Collision *ptr;

	debugC(5, DEBUG_COLLISIONS, "addNewCollision");
	debugC(5, DEBUG_COLLISIONS, "id = %x", id);
	debugC(5, DEBUG_COLLISIONS, "left = %d, top = %d, right = %d, bottom = %d", left, top, right, bottom);
	debugC(5, DEBUG_COLLISIONS, "flags = %x, key = %x", flags, key);
	debugC(5, DEBUG_COLLISIONS, "funcEnter = %d, funcLeave = %d", funcEnter, funcLeave);

	for (i = 0; i < 250; i++) {
		if (_collisionAreas[i].left != -1)
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
		return;
	}
	error("addNewCollision: Collision array full!\n");
}

void Game_v1::pushCollisions(char all) {
	Collision *srcPtr;
	Collision *destPtr;
	int16 size;

	debugC(1, DEBUG_COLLISIONS, "pushCollisions");
	for (size = 0, srcPtr = _collisionAreas; srcPtr->left != -1;
	    srcPtr++) {
		if (all || (srcPtr->id & 0x8000))
			size++;
	}

	destPtr = new Collision[size];
	_collStack[_collStackSize] = destPtr;
	_collStackElemSizes[_collStackSize] = size;
	_collStackSize++;

	for (srcPtr = _collisionAreas; srcPtr->left != -1; srcPtr++) {
		if (all || (srcPtr->id & 0x8000)) {
			memcpy(destPtr, srcPtr, sizeof(Collision));
			srcPtr->left = -1;
			destPtr++;
		}
	}
}

void Game_v1::popCollisions(void) {
	Collision *destPtr;
	Collision *srcPtr;

	debugC(1, DEBUG_COLLISIONS, "popCollision");

	_collStackSize--;
	for (destPtr = _collisionAreas; destPtr->left != -1; destPtr++);

	srcPtr = _collStack[_collStackSize];
	memcpy(destPtr, srcPtr,
	    _collStackElemSizes[_collStackSize] *
	    sizeof(Collision));

	delete[] _collStack[_collStackSize];
}

int16 Game_v1::checkKeys(int16 *pMouseX, int16 *pMouseY, int16 *pButtons, char handleMouse) {
	_vm->_util->processInput();

	if (VAR(58) != 0) {
		if (_vm->_mult->_frameStart != (int)VAR(58) - 1)
			_vm->_mult->_frameStart++;
		else
			_vm->_mult->_frameStart = 0;

		_vm->_mult->playMult(_vm->_mult->_frameStart + VAR(57), _vm->_mult->_frameStart + VAR(57), 1,
		    handleMouse);
	}

	if (_vm->_inter->_soundEndTimeKey != 0
	    && _vm->_util->getTimeKey() >= _vm->_inter->_soundEndTimeKey) {
		_vm->_snd->stopSound(_vm->_inter->_soundStopVal);
		_vm->_inter->_soundEndTimeKey = 0;
	}

	if (_vm->_global->_useMouse == 0)
		error("checkKeys: Can't work without mouse!");

	_vm->_util->getMouseState(pMouseX, pMouseY, pButtons);

	if (*pButtons == 3)
		*pButtons = 0;

	return _vm->_util->checkKey();
}

int16 Game_v1::checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
	    int16 *pResIndex) {
	char *savedIP;
	int16 resIndex;
	int16 key;
	int16 oldIndex;
	int16 oldId;
	uint32 timeKey;

	if (deltaTime >= -1) {
		_lastCollKey = 0;
		_lastCollAreaIndex = 0;
		_lastCollId = 0;
	}

	if (pResId != 0)
		*pResId = 0;

	resIndex = 0;

	if (_vm->_draw->_cursorIndex == -1 && handleMouse != 0
	    && _lastCollKey == 0) {
		_lastCollKey =
		    checkMousePoint(1, &_lastCollId,
		    &_lastCollAreaIndex);

		if (_lastCollKey != 0 && (_lastCollId & 0x8000) != 0) {
			savedIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr = (char *)_totFileData +
			    _collisionAreas[_lastCollAreaIndex].funcEnter;

			_vm->_inter->funcBlock(0);
			_vm->_global->_inter_execPtr = savedIP;
		}
	}

	if (handleMouse != 0)
		_vm->_draw->animateCursor(-1);

	timeKey = _vm->_util->getTimeKey();
	while (1) {
		if (_vm->_inter->_terminate) {
			if (handleMouse)
				_vm->_draw->blitCursor();
			return 0;
		}

		if (_vm->_draw->_noInvalidated == 0) {
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
			if (deltaTime == -1 || curtime + deltaTime > timeKey) {
				if (pResId != 0)
					*pResId = 0;

				if (pResIndex != 0)
					*pResIndex = 0;

				return 0;
			}
		}

		key = checkKeys(&_vm->_global->_inter_mouseX, &_vm->_global->_inter_mouseY,
							 &_mouseButtons, handleMouse);

		if (handleMouse == 0 && _mouseButtons != 0) {
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

			if (_lastCollKey != 0 &&
			    _collisionAreas[_lastCollAreaIndex].funcLeave != 0) {
				savedIP = _vm->_global->_inter_execPtr;
				_vm->_global->_inter_execPtr = (char *)_totFileData +
				    _collisionAreas[_lastCollAreaIndex].funcLeave;

				_vm->_inter->funcBlock(0);
				_vm->_global->_inter_execPtr = savedIP;
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
				} else if (deltaTime > 0) {
					_vm->_util->delay(deltaTime);
				}

				_vm->_draw->animateCursor(-1);
				if (pResId != 0)
					*pResId = 0;

				key = checkMousePoint(0, pResId, &resIndex);

				if (pResIndex != 0)
					*pResIndex = resIndex;

				if (key != 0 || (pResId != 0 && *pResId != 0)) {
					if (handleMouse == 1 && (deltaTime <= 0
						|| _mouseButtons == 0))
						_vm->_draw->blitCursor();

					if (_lastCollKey != 0 &&
						_collisionAreas[_lastCollAreaIndex].funcLeave != 0) {
						savedIP = _vm->_global->_inter_execPtr;
						_vm->_global->_inter_execPtr =
						    (char *)_totFileData +
						    _collisionAreas[_lastCollAreaIndex].funcLeave;

						_vm->_inter->funcBlock(0);
						_vm->_global->_inter_execPtr = savedIP;
					}
					_lastCollKey = 0;
					return key;
				}

				if (_lastCollKey != 0 &&
				    _collisionAreas[_lastCollAreaIndex].funcLeave != 0) {
					savedIP = _vm->_global->_inter_execPtr;
					_vm->_global->_inter_execPtr =
					    (char *)_totFileData +
					    _collisionAreas[_lastCollAreaIndex].funcLeave;

					_vm->_inter->funcBlock(0);
					_vm->_global->_inter_execPtr = savedIP;
				}

				_lastCollKey =
				    checkMousePoint(1, &_lastCollId,
				    &_lastCollAreaIndex);

				if (_lastCollKey != 0
				    && (_lastCollId & 0x8000) != 0) {
					savedIP = _vm->_global->_inter_execPtr;
					_vm->_global->_inter_execPtr =
					    (char *)_totFileData +
					    _collisionAreas[_lastCollAreaIndex].funcEnter;

					_vm->_inter->funcBlock(0);
					_vm->_global->_inter_execPtr = savedIP;
				}
			} else {

				if (handleMouse != 0 &&
				    (_vm->_global->_inter_mouseX != _vm->_draw->_cursorX
					|| _vm->_global->_inter_mouseY != _vm->_draw->_cursorY)) {
					oldIndex = _lastCollAreaIndex;
					oldId = _lastCollId;

					key =
					    checkMousePoint(1,
					    &_lastCollId,
					    &_lastCollAreaIndex);

					if (key != _lastCollKey) {
						if (_lastCollKey != 0
						    && (oldId & 0x8000) != 0) {
							savedIP = _vm->_global->_inter_execPtr;
							_vm->_global->_inter_execPtr = (char *)_totFileData +
							    _collisionAreas[oldIndex].funcLeave;

							_vm->_inter->funcBlock(0);
							_vm->_global->_inter_execPtr = savedIP;
						}

						_lastCollKey = key;
						if (_lastCollKey != 0 && (_lastCollId & 0x8000) != 0) {
							savedIP = _vm->_global->_inter_execPtr;
							_vm->_global->_inter_execPtr = (char *)_totFileData +
							    _collisionAreas[_lastCollAreaIndex].funcEnter;

							_vm->_inter->funcBlock(0);
							_vm->_global->_inter_execPtr = savedIP;
						}
					}
				}
			}
		}

		if (handleMouse != 0)
			_vm->_draw->animateCursor(-1);

		_vm->_util->delay(10);

		_vm->_snd->loopSounds();
	}
}

void Game_v1::prepareStart(void) {
	int16 i;

	clearCollisions();

	_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->_backSurface = _vm->_video->initSurfDesc(_vm->_global->_videoMode, 320, 200, 0);

	_vm->_video->fillRect(_vm->_draw->_backSurface, 0, 0, 319, 199, 1);
	_vm->_draw->_frontSurface = _vm->_global->_pPrimarySurfDesc;
	_vm->_video->fillRect(_vm->_draw->_frontSurface, 0, 0, 319, 199, 1);

	_vm->_util->setMousePos(152, 92);

	_vm->_draw->_cursorX = 152;
	_vm->_global->_inter_mouseX = 152;

	_vm->_draw->_cursorY = 92;
	_vm->_global->_inter_mouseY = 92;
	_vm->_draw->_invalidatedCount = 0;
	_vm->_draw->_noInvalidated = 1;
	_vm->_draw->_applyPal = 0;
	_vm->_draw->_paletteCleared = 0;
	_vm->_draw->_cursorWidth = 16;
	_vm->_draw->_cursorHeight = 16;
	_vm->_draw->_transparentCursor = 1;

	for (i = 0; i < 40; i++) {
		_vm->_draw->_cursorAnimLow[i] = -1;
		_vm->_draw->_cursorAnimDelays[i] = 0;
		_vm->_draw->_cursorAnimHigh[i] = 0;
	}

	_vm->_draw->_cursorAnimLow[1] = 0;
	_vm->_draw->_cursorSprites = _vm->_video->initSurfDesc(_vm->_global->_videoMode, 32, 16, 2);
	_vm->_draw->_cursorBack = _vm->_video->initSurfDesc(_vm->_global->_videoMode, 16, 16, 0);
	_vm->_draw->_renderFlags = 0;
	_vm->_draw->_backDeltaX = 0;
	_vm->_draw->_backDeltaY = 0;

	_startTimeKey = _vm->_util->getTimeKey();
}

} // End of namespace Gob
