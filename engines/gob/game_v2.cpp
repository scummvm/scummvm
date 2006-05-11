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

Game_v2::Game_v2(GobEngine *vm) : Game_v1(vm) {
}

void Game_v2::playTot(int16 skipPlay) {
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
			_vm->_draw->_cursorSpritesBack = _vm->_draw->_cursorSprites;

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
				if (READ_LE_UINT32(filePtr) == 0)
					_totTextData = (TotTextTable *) loadLocTexts();
				else
					_totTextData =
							(TotTextTable *) (curPtr +
							READ_LE_UINT32((char *)_totFileData + 0x30));

				if (_totTextData != 0) {
					_totTextData->itemsCount = (int16)READ_LE_UINT16(&_totTextData->itemsCount);

					for (i = 0; i < _totTextData->itemsCount; ++i) {
						_totTextData->items[i].offset = (int16)READ_LE_UINT16(&_totTextData->items[i].offset);
						_totTextData->items[i].size = (int16)READ_LE_UINT16(&_totTextData->items[i].size);
					}
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
				memset(_vm->_global->_inter_variables, 0, variablesCount * 4);
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

			for (i = 0; i < 60; i++)
				if ((_soundTypes[i] & 8) == 0)
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

void Game_v2::clearCollisions() {
	int16 i;

	_lastCollKey = 0;

	for (i = 0; i < 250; i++) {
		_collisionAreas[i].id = 0;
		_collisionAreas[i].left = -1;
	}
}

void Game_v2::addNewCollision(int16 id, int16 left, int16 top, int16 right, int16 bottom,
	    int16 flags, int16 key, int16 funcEnter, int16 funcLeave) {
	int16 i;
	Collision *ptr;

	debugC(5, DEBUG_COLLISIONS, "addNewCollision");
	debugC(5, DEBUG_COLLISIONS, "id = %x", id);
	debugC(5, DEBUG_COLLISIONS, "left = %d, top = %d, right = %d, bottom = %d", left, top, right, bottom);
	debugC(5, DEBUG_COLLISIONS, "flags = %x, key = %x", flags, key);
	debugC(5, DEBUG_COLLISIONS, "funcEnter = %d, funcLeave = %d", funcEnter, funcLeave);

	for (i = 0; i < 250; i++) {
		if ((_collisionAreas[i].left != -1) && (_collisionAreas[i].id != id))
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
		ptr->field_12 = 0;
		return;
	}
	error("addNewCollision: Collision array full!\n");
}

void Game_v2::pushCollisions(char all) {
	Collision *srcPtr;
	Collision *destPtr;
	int16 size;

	debugC(1, DEBUG_COLLISIONS, "pushCollisions");
	for (size = 0, srcPtr = _collisionAreas; srcPtr->left != -1;
	    srcPtr++) {
		if (all || (srcPtr->id >= 20))
			size++;
	}

	destPtr = new Collision[size];
	_collStack[_collStackSize] = destPtr;
	_collStackElemSizes[_collStackSize] = size;

	if (_shouldPushColls != 0)
		_collStackElemSizes[_collStackSize] |= 0x8000;

	_shouldPushColls = 0;
	_collLasts[_collStackSize].key = _lastCollKey;
	_collLasts[_collStackSize].id = _lastCollId;
	_collLasts[_collStackSize].areaIndex = _lastCollAreaIndex;
	_collStackSize++;

	for (srcPtr = _collisionAreas; srcPtr->left != -1; srcPtr++) {
		if (all || (srcPtr->id >= 20)) {
			memcpy(destPtr, srcPtr, sizeof(Collision));
			srcPtr->left = -1;
			destPtr++;
		}
	}
}

void Game_v2::popCollisions(void) {
	Collision *destPtr;
	Collision *srcPtr;

	debugC(1, DEBUG_COLLISIONS, "popCollision");

	_collStackSize--;

	_shouldPushColls = _collStackElemSizes[_collStackSize] & 0x8000 ? 1 : 0;
	_collStackElemSizes[_collStackSize] &= 0x7FFF;

	_lastCollKey = _collLasts[_collStackSize].key;
	_lastCollId = _collLasts[_collStackSize].id;
	_lastCollAreaIndex = _collLasts[_collStackSize].areaIndex;

	for (destPtr = _collisionAreas; destPtr->left != -1; destPtr++);

	srcPtr = _collStack[_collStackSize];
	memcpy(destPtr, srcPtr,
	    _collStackElemSizes[_collStackSize] *
	    sizeof(Collision));

	delete[] _collStack[_collStackSize];
}

int16 Game_v2::checkKeys(int16 *pMouseX, int16 *pMouseY, int16 *pButtons, char handleMouse) {
	_vm->_util->processInput();

	if ((_vm->_global->_inter_variables != 0) && (VAR(58) != 0)) {
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

int16 Game_v2::checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
	    int16 *pResIndex) {
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

		if ((_lastCollKey != 0) && (_lastCollId & 0x8000))
			collAreaSub(_lastCollAreaIndex, 1);
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

		// TODO: What of this is needed?
		int16 width;
		int16 height;
		int16 sWidth;
		int16 sHeight;
		int16 cursorRight;
		int16 cursorBottom;
		int16 oldWord_2FC9C;
		int16 oldWord_2FC9E;
		if ((_vm->_video->_extraMode) && (handleMouse != 0)) {
			width = _vm->_draw->_frontSurface->width;
			height = _vm->_draw->_frontSurface->height;
			if ((width > _vm->_global->_primaryWidth) || (height > _vm->_global->_primaryHeight)
					|| ((_off_2E51B != 0) && (height > _off_2E51B->height))) {
				sWidth = _vm->_global->_primaryWidth;
				sHeight = _vm->_global->_primaryHeight;
				if (_off_2E51B != 0)
					sHeight -= _off_2E51B->height;
				oldWord_2FC9E = _word_2FC9E;
				oldWord_2FC9C = _word_2FC9C;
				if ((width > sWidth) && (_vm->_global->_inter_mouseX >= _word_2FC9E)) {
					cursorRight = _vm->_global->_inter_mouseX + _vm->_draw->_cursorWidth;
					if (cursorRight > (_word_2FC9E + sWidth))
						_word_2FC9E = MIN(cursorRight - sWidth, width - sWidth);
				} else if(_vm->_global->_inter_mouseX < _word_2FC9E)
					_word_2FC9E = _vm->_global->_inter_mouseX;
				height = _vm->_draw->_frontSurface->height;
				if ((height > sHeight) && (_vm->_global->_inter_mouseY >= _word_2FC9C)) {
					cursorBottom = _vm->_global->_inter_mouseY + _vm->_draw->_cursorHeight;
					if (cursorBottom > (_word_2FC9C + sHeight))
						_word_2FC9C = MIN(cursorBottom - sHeight, height - sHeight);
				} else if(_vm->_global->_inter_mouseY < _word_2FC9C)
					_word_2FC9C = _vm->_global->_inter_mouseY;
				if ((oldWord_2FC9E != _word_2FC9E) || (oldWord_2FC9C != _word_2FC9C)) {
					if (_byte_2FC9B == 0) {
						_word_2FC9E = oldWord_2FC9E;
						_word_2FC9C = oldWord_2FC9C;
						if ((_vm->_draw->_frontSurface->width > sWidth) && 
								(_vm->_global->_inter_mouseX >= oldWord_2FC9E)) {
							if ((_vm->_global->_inter_mouseX + _vm->_draw->_cursorWidth) >
									(_word_2FC9E + sWidth))
								_vm->_global->_inter_mouseX = _word_2FC9E + sWidth - _vm->_draw->_cursorWidth;
						} else if(_vm->_global->_inter_mouseX < oldWord_2FC9E)
							_vm->_global->_inter_mouseX = oldWord_2FC9E;

						if ((_vm->_draw->_frontSurface->height > sHeight) && 
								(_vm->_global->_inter_mouseY >= _word_2FC9C)) {
							if ((_vm->_global->_inter_mouseY + _vm->_draw->_cursorHeight) >
									(_word_2FC9C + sHeight))
								_vm->_global->_inter_mouseY = _word_2FC9C + sHeight - _vm->_draw->_cursorHeight;
						} else if(_vm->_global->_inter_mouseY < oldWord_2FC9E)
							_vm->_global->_inter_mouseY = _word_2FC9C;
					} else {
						if (oldWord_2FC9E > _word_2FC9E) {
							_vm->_global->_inter_mouseX += (oldWord_2FC9E - _word_2FC9E) / 2;
							_word_2FC9E += (oldWord_2FC9E - _word_2FC9E) / 2;
						} else {
							_vm->_global->_inter_mouseX -= (_word_2FC9E - oldWord_2FC9E) / 2;
							_word_2FC9E -= (_word_2FC9E - oldWord_2FC9E) / 2;
						}
						if (oldWord_2FC9C > _word_2FC9C) {
							_vm->_global->_inter_mouseY += (oldWord_2FC9C - _word_2FC9C) / 2;
							_word_2FC9C += (oldWord_2FC9C - _word_2FC9C) / 2;
							if (_word_2FC9C < 2)
								_word_2FC9C = 0;
						} else {
							_vm->_global->_inter_mouseY -= (_word_2FC9C - oldWord_2FC9C) / 2;
							_word_2FC9C -= (_word_2FC9C - oldWord_2FC9C) / 2;
						}
						if (_off_2E51B == 0)
							warning("_vid_setPixelShift(_word_2FC9E, _word_2FC9C);");
						else
							warning("_vid_setPixelShift(_word_2FC9E, _word_2FC9C + _off_2E51B->height);");
					}
					_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
				}
			}
		}

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

			if (_lastCollKey != 0)
				collAreaSub(_lastCollAreaIndex, 0);

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

					if ((_lastCollKey != 0) && (key = _lastCollKey))
							collAreaSub(_lastCollAreaIndex, 0);

					_lastCollKey = 0;
					return key;
				}

				if (handleMouse & 4)
					return key;

				if (_lastCollKey != 0)
					collAreaSub(_lastCollAreaIndex, 0);

				_lastCollKey =
				    checkMousePoint(1, &_lastCollId,
				    &_lastCollAreaIndex);

				if (_lastCollKey != 0)
					collAreaSub(_lastCollAreaIndex, 1);

				// loc_189D3
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
						if (_lastCollKey != 0)
							collAreaSub(_lastCollAreaIndex, 0);

						_lastCollKey = key;

						if (_lastCollKey != 0)
							collAreaSub(_lastCollAreaIndex, 1);
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

void Game_v2::prepareStart(void) {
	int16 i;

	clearCollisions();

	_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
	_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;

	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	_vm->_draw->_backSurface = _vm->_video->initSurfDesc(_vm->_global->_videoMode, 320, 200, 0);

	sub_ADD2();
	_vm->_video->fillRect(_vm->_draw->_frontSurface, 0, 0, 319, 199, 1);

	_vm->_util->setMousePos(152, 92);

	_vm->_draw->_cursorX = 152;
	_vm->_global->_inter_mouseX = 152;

	_vm->_draw->_cursorY = 92;
	_vm->_global->_inter_mouseY = 92;
	_vm->_draw->_invalidatedCount = 0;
	_vm->_draw->_noInvalidated = 1;
	// byte_2E521 = 1;
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

	// byte_2F392 = 0;

	_vm->_draw->_renderFlags = 0;
	_vm->_draw->_backDeltaX = 0;
	_vm->_draw->_backDeltaY = 0;

	_startTimeKey = _vm->_util->getTimeKey();
}

} // End of namespace Gob
