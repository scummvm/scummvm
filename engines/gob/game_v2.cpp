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

	if (skipPlay <= 0) {
		while (!_vm->_quitRequested) {
			_vm->_draw->animateCursor(4);
			if (_vm->_features & GF_MAC)
				_vm->_music->stopPlay();
			else
				_vm->_cdrom->stopPlaying();

			if (skipPlay != -1) {
				_vm->_inter->initControlVars(1);

				for (i = 0; i < 4; i++) {
					_vm->_draw->_fontToSprite[i].sprite = -1;
					_vm->_draw->_fontToSprite[i].base = -1;
					_vm->_draw->_fontToSprite[i].width = -1;
					_vm->_draw->_fontToSprite[i].height = -1;
				}

				_vm->_mult->initAll();
				_vm->_mult->zeroMultData();

				for (i = 0; i < 20; i++)
					_vm->_draw->_spritesArray[i] = 0;

				_vm->_draw->_spritesArray[20] = _vm->_draw->_frontSurface;
				_vm->_draw->_spritesArray[21] = _vm->_draw->_backSurface;
				_vm->_draw->_cursorSpritesBack = _vm->_draw->_cursorSprites;
			} else
				_vm->_inter->initControlVars(0);

			for (i = 0; i < 20; i++)
				_soundSamples[i] = 0;

			_totTextData = 0;
			_totResourceTable = 0;
			_imFileData = 0;
			_extTable = 0;
			_extHandle = -1;

			_totToLoad[0] = 0;

			if (_curTotFile[0] == 0 && _totFileData == 0)
				break;

			loadTotFile(_curTotFile);
			if (_totFileData == 0) {
				_vm->_draw->blitCursor();
				break;
			}

			if (skipPlay == -2)
				skipPlay = 0;

			strcpy(_curImaFile, _curTotFile);
			strcpy(_curExtFile, _curTotFile);

			_curImaFile[strlen(_curImaFile) - 4] = 0;
			strcat(_curImaFile, ".ima");

			_curExtFile[strlen(_curExtFile) - 4] = 0;
			strcat(_curExtFile, ".ext");

			debugC(4, DEBUG_FILEIO, "IMA: %s", _curImaFile);
			debugC(4, DEBUG_FILEIO, "EXT: %s", _curExtFile);

			filePtr = (char *)_totFileData + 0x30;

			_totTextData = 0;
			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				_totTextData = new TotTextTable;
				if (READ_LE_UINT32(filePtr) == 0)
					_totTextData->dataPtr = loadLocTexts();
				else
					_totTextData->dataPtr = (_totFileData + READ_LE_UINT32((char *)_totFileData + 0x30));

				_totTextData->items = 0;
				if (_totTextData->dataPtr != 0) {
					Common::MemoryReadStream totTextData((byte *) _totTextData->dataPtr, 4294967295U);
					_totTextData->itemsCount = totTextData.readSint16LE();

					_totTextData->items = new TotTextItem[_totTextData->itemsCount];
					for (i = 0; i < _totTextData->itemsCount; ++i) {
						_totTextData->items[i].offset = totTextData.readSint16LE();
						_totTextData->items[i].size = totTextData.readSint16LE();
					}
				}
			}

			filePtr = (char *)_totFileData + 0x34;
			_totResourceTable = 0;
			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				_totResourceTable = new TotResTable;
				_totResourceTable->dataPtr = _totFileData + READ_LE_UINT32((char *)_totFileData + 0x34);
				Common::MemoryReadStream totResTable((byte *) _totResourceTable->dataPtr, 4294967295U);

				_totResourceTable->itemsCount = totResTable.readSint16LE();
				_totResourceTable->unknown = totResTable.readByte();

				_totResourceTable->items = new TotResItem[_totResourceTable->itemsCount];
				for (i = 0; i < _totResourceTable->itemsCount; ++i) {
					_totResourceTable->items[i].offset = totResTable.readSint32LE();
					_totResourceTable->items[i].size = totResTable.readSint16LE();
					_totResourceTable->items[i].width = totResTable.readSint16LE();
					_totResourceTable->items[i].height = totResTable.readSint16LE();
				}
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

			if (_totTextData) {
				if (_totTextData->items)
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
				_vm->_dataio->closeData(_extHandle);

			_extHandle = -1;

			for (i = 0; i < *_vm->_scenery->_pCaptureCounter; i++)
				capturePop(0);

/*			_vm->_mult->checkFreeMult();
			_vm->_mult->freeAll();*/

			if (skipPlay != -1) {
				_vm->_goblin->freeObjects();

				for (i = 0; i < 20; i++) {
					if (_vm->_draw->_spritesArray[i] != 0)
						_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[i]);
					_vm->_draw->_spritesArray[i] = 0;
				}
				_vm->_snd->stopSound(0);

				for (i = 0; i < 60; i++)
					if ((_soundTypes[i] & 8) == 0)
						freeSoundSlot(i);
			}

			closeImd();
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

	for (i = 0; i < 150; i++) {
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

		_vm->_mult->playMult(_vm->_mult->_frameStart + VAR(57),
				_vm->_mult->_frameStart + VAR(57), 1, handleMouse);
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

	if ((_vm->_draw->_cursorIndex == -1) && (handleMouse != 0) && (_lastCollKey == 0)) {
		_lastCollKey = checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);

		if ((_lastCollKey != 0) && (_lastCollId & 0x8000))
			collAreaSub(_lastCollAreaIndex, 1);
	}

	if (handleMouse != 0)
		_vm->_draw->animateCursor(-1);

	timeKey = _vm->_util->getTimeKey();
	while (1) {
		if (_vm->_inter->_terminate || _vm->_quitRequested) {
			if (handleMouse)
				_vm->_draw->blitCursor();
			return 0;
		}

		if (_vm->_draw->_noInvalidated == 0) {
			if (handleMouse != 0)
				_vm->_draw->animateCursor(-1);
			else
				_vm->_draw->blitInvalidated();
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
				} else if (_vm->_global->_inter_mouseX < _word_2FC9E)
					_word_2FC9E = _vm->_global->_inter_mouseX;
				height = _vm->_draw->_frontSurface->height;
				if ((height > sHeight) && (_vm->_global->_inter_mouseY >= _word_2FC9C)) {
					cursorBottom = _vm->_global->_inter_mouseY + _vm->_draw->_cursorHeight;
					if (cursorBottom > (_word_2FC9C + sHeight))
						_word_2FC9C = MIN(cursorBottom - sHeight, height - sHeight);
				} else if (_vm->_global->_inter_mouseY < _word_2FC9C)
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
						} else if (_vm->_global->_inter_mouseX < oldWord_2FC9E)
							_vm->_global->_inter_mouseX = oldWord_2FC9E;

						if ((_vm->_draw->_frontSurface->height > sHeight) && 
								(_vm->_global->_inter_mouseY >= _word_2FC9C)) {
							if ((_vm->_global->_inter_mouseY + _vm->_draw->_cursorHeight) >
									(_word_2FC9C + sHeight))
								_vm->_global->_inter_mouseY = _word_2FC9C + sHeight - _vm->_draw->_cursorHeight;
						} else if (_vm->_global->_inter_mouseY < oldWord_2FC9E)
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

			if (_lastCollKey != 0)
				collAreaSub(_lastCollAreaIndex, 0);

			_lastCollKey = 0;
			if (key != 0)
				return key;
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
					if ((handleMouse & 1) && ((deltaTime <= 0) || (_mouseButtons == 0)))
						_vm->_draw->blitCursor();

					if ((_lastCollKey != 0) && (key != _lastCollKey))
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
			} else if ((_vm->_global->_inter_mouseX != _vm->_draw->_cursorX) ||
					(_vm->_global->_inter_mouseY != _vm->_draw->_cursorY)) {

				oldIndex = _lastCollAreaIndex;
				oldId = _lastCollId;
				key = checkMousePoint(1, &_lastCollId, &_lastCollAreaIndex);

				if (key != _lastCollKey) {
					if ((_lastCollKey != 0) && (oldId & 0x8000))
						collAreaSub(oldIndex, 0);

					_lastCollKey = key;

					if ((key != 0) && (_lastCollId & 0x8000))
						collAreaSub(_lastCollAreaIndex, 1);
				}
			}
		}

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

void Game_v2::collisionsBlock(void) {
	InputDesc descArray[20];
	int16 array[250];
	char count;
	int16 collResId;
	char *startIP;
	int16 curCmd;
	int16 cmd;
	int16 cmdHigh;
	int16 key;
	int16 flags;
	int16 left;
	int16 top;
	int16 width;
	int16 height;
	int16 var_1C;
	int16 index;
	int16 curEditIndex;
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
	int16 _collStackPos;
	Collision *collPtr;
	Collision *collArea;
	int16 timeKey;
	char *savedIP;

	if (_shouldPushColls)
		pushCollisions(0);

	collArea = _collisionAreas;
	while (collArea->left != -1)
		collArea++;

	_shouldPushColls = 0;
	collResId = -1;

	_vm->_global->_inter_execPtr++;
	count = *_vm->_global->_inter_execPtr++;

	_handleMouse = _vm->_global->_inter_execPtr[0];
	deltaTime = 1000 * (byte)_vm->_global->_inter_execPtr[1];
	stackPos2 = (byte)_vm->_global->_inter_execPtr[3];
	descIndex = (byte)_vm->_global->_inter_execPtr[4];

	if ((stackPos2 != 0) || (descIndex != 0))
		deltaTime /= 100;

	timeVal = deltaTime;
	_vm->_global->_inter_execPtr += 6;

	startIP = _vm->_global->_inter_execPtr;
	WRITE_VAR(16, 0);

	var_1C = 0;
	index = 0;
	curEditIndex = 0;

	for (curCmd = 0; curCmd < count; curCmd++) {
		array[curCmd] = 0;
		cmd = *_vm->_global->_inter_execPtr++;

		if ((cmd & 0x40) != 0) {
			cmd -= 0x40;
			cmdHigh = (byte)*_vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr++;
			cmdHigh <<= 8;
		} else
			cmdHigh = 0;

		if ((cmd & 0x80) != 0) {
			offsetIP = _vm->_global->_inter_execPtr - (char *) _totFileData;
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

		if ((_vm->_draw->_renderFlags & 8) && (left != -1)) {
			left += _vm->_draw->_backDeltaX;
			top += _vm->_draw->_backDeltaY;
		}

		if (left != -1) {
			_vm->_draw->adjustCoords(0, &left, &top);
			if (((cmd & 0x3F) < 20) && ((cmd & 0x3F) >= 3)) {
				if (_vm->_draw->_word_2E8E2 != 2)
					height &= 0xFFFE;
				_vm->_draw->adjustCoords(0, 0, &width);
			} else
				_vm->_draw->adjustCoords(0, &height, &width);
		}

		cmd &= 0x7f;
		debugC(1, DEBUG_COLLISIONS, "collisionsBlock(%d)", cmd);

		switch (cmd) {
		case 0:
			_vm->_global->_inter_execPtr += 6;
			startIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top, left + width - 1, top + height - 1,
					cmd + cmdHigh, key, startIP - (char *)_totFileData,
					_vm->_global->_inter_execPtr - (char *)_totFileData);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);

			if (key <= 150)
				_collisionAreas[key].field_12 = offsetIP;
			break;

		case 1:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16();

			startIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			if (key == 0)
				key = curCmd + 0xa000;

			addNewCollision(curCmd + 0x8000, left, top, left + width - 1, top + height - 1,
					(flags << 4) + cmd + cmdHigh, key, startIP - (char *)_totFileData,
					_vm->_global->_inter_execPtr - (char *)_totFileData);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);

			if (key <= 150)
				_collisionAreas[key].field_12 = offsetIP;
			break;

		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			_vm->_util->waitKey();
			var_1C = 1;
			key = _vm->_parse->parseVarIndex();
			descArray[index].fontIndex = _vm->_inter->load16();
			descArray[index].backColor = *_vm->_global->_inter_execPtr++;
			descArray[index].frontColor = *_vm->_global->_inter_execPtr++;

			if ((cmd >= 5) && (cmd <= 8)) {
				descArray[index].ptr = _vm->_global->_inter_execPtr + 2;
				_vm->_global->_inter_execPtr += _vm->_inter->load16();;
			} else
				descArray[index].ptr = 0;

			if (left == -1) {
				if ((cmd & 1) == 0) {
					_vm->_global->_inter_execPtr += 2;
					_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
				}
				break;
			}

			if ((cmd & 1) == 0) {
				addNewCollision(curCmd + 0x8000, left, top,
						left + width * _vm->_draw->_fonts[descArray[index].fontIndex]-> itemWidth - 1,
						top + height - 1, cmd, key, 0,
						_vm->_global->_inter_execPtr - (char *)_totFileData);

				_vm->_global->_inter_execPtr += 2;
				_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			} else
				addNewCollision(curCmd + 0x8000, left, top,
						left + width * _vm->_draw->_fonts[descArray[index].fontIndex]-> itemWidth - 1,
						top + height - 1, cmd, key, 0, 0);

			index++;
			break;

		case 11:
			_vm->_global->_inter_execPtr += 6;
			for (i = 0; i < 150; i++) {
				if ((_collisionAreas[i].id & 0xF000) == 0xE000) {
					_collisionAreas[i].id &= 0xBFFF;
					_collisionAreas[i].funcEnter = _vm->_global->_inter_execPtr - (char *) _totFileData;
					_collisionAreas[i].funcLeave = _vm->_global->_inter_execPtr - (char *) _totFileData;
				}
			}
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;

		case 12:
			_vm->_global->_inter_execPtr += 6;
			for (i = 0; i < 150; i++) {
				if ((_collisionAreas[i].id & 0xF000) == 0xD000) {
					_collisionAreas[i].id &= 0xBFFF;
					_collisionAreas[i].funcEnter = _vm->_global->_inter_execPtr - (char *) _totFileData;
					_collisionAreas[i].funcLeave = _vm->_global->_inter_execPtr - (char *) _totFileData;
				}
			}
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;

		case 20:
			collResId = curCmd;
			// Fall through to case 2

		case 2:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16();

			addNewCollision(curCmd + 0x8000, left, top, left + width - 1, top + height - 1,
					(flags << 4) + cmdHigh + 2, key, 0,
					_vm->_global->_inter_execPtr - (char *)_totFileData);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);

			if (key <= 150)
				_collisionAreas[key].field_12 = offsetIP;
			break;

		case 21:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16() & 3;

			addNewCollision(curCmd + 0x8000, left, top, left + width - 1, top + height - 1,
					(flags << 4) + cmdHigh + 2, key,
					_vm->_global->_inter_execPtr - (char *)_totFileData, 0);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);

			if (key <= 150)
				_collisionAreas[key].field_12 = offsetIP;
			break;
		}
	}

	_forceHandleMouse = 0;
	_vm->_util->waitKey();

	do {
		if (var_1C != 0) {
			key = multiEdit(deltaTime, index, &curEditIndex, descArray,
					&_activeCollResId, &_activeCollIndex);

			WRITE_VAR(55, curEditIndex);
			if (key == 0x1c0d) {
				for (i = 0; i < 150; i++) {
					if (_collisionAreas[i].left == -1)
						continue;

					if ((_collisionAreas[i].id & 0xC000) != 0x8000)
						continue;

					if ((_collisionAreas[i].flags & 1) != 0)
						continue;

					if ((_collisionAreas[i].flags & 0x0f) <= 2)
						continue;

					_activeCollResId = _collisionAreas[i].id;
					collResId = _collisionAreas[i].id & 0x7fff;
					_activeCollIndex = i;
					break;
				}
				break;
			}
		} else
			key = checkCollisions(_handleMouse, -deltaTime, &_activeCollResId, &_activeCollIndex);

		if ((key & 0xff) >= ' ' && (key & 0xff) <= 0xff &&
		    (key >> 8) > 1 && (key >> 8) < 12) {
			key = '0' + (((key >> 8) - 1) % 10) + (key & 0xff00);
		}

		if (_activeCollResId == 0) {
			if (key != 0) {
				for (i = 0; i < 150; i++) {
					if (_collisionAreas[i].left == -1)
						continue;

					if ((_collisionAreas[i].id & 0xC000) != 0x8000)
						continue;

					if ((_collisionAreas[i].key == key) || (_collisionAreas[i].key == 0x7fff)) {
						_activeCollResId = _collisionAreas[i].id;
						_activeCollIndex = i;
						break;
					}
				}

				if (_activeCollResId == 0) {
					for (i = 0; i < 150; i++) {
						if (_collisionAreas[i].left == -1)
							continue;

						if ((_collisionAreas[i].id & 0xC000) != 0x8000)
							continue;

						if ((_collisionAreas[i].key & 0xff00) != 0)
							continue;

						if (_collisionAreas[i].key == 0)
							continue;

						if ((adjustKey(key & 0xff) == adjustKey(_collisionAreas[i].key)) ||
								(_collisionAreas[i].key == 0x7fff)) {
							_activeCollResId = _collisionAreas[i].id;
							_activeCollIndex = i;
							break;
						}
					}
				}
			} else if (deltaTime != 0) {
				if (stackPos2 != 0) {
					_collStackPos = 0;
					collPtr = _collisionAreas;

					for (i = 0, collPtr = _collisionAreas; collPtr->left != -1; i++, collPtr++) {
						if ((collPtr->id & 0xF000) != 0x8000)
							continue;

						_collStackPos++;
						if (_collStackPos != stackPos2)
							continue;

						_activeCollResId = collPtr->id;
						_activeCollIndex = i;
						_vm->_inter->storeMouse();
						if (VAR(16) != 0)
							break;

						if ((_activeCollResId & 0xF000) == 0x8000)
							WRITE_VAR(16, array[_activeCollResId & 0xFFF]);
						else
							WRITE_VAR(16, _activeCollResId & 0xFFF);

						if (collPtr->funcLeave != 0) {
							timeKey = _vm->_util->getTimeKey();
							collSub(collPtr->funcLeave);
							_vm->_inter->animPalette();

							deltaTime = timeVal - (_vm->_util->getTimeKey() - timeKey);

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
						for (i = 0, collPtr = collArea; collPtr->left != -1; i++, collPtr++) {
							if ((collPtr->id & 0xF000) == 0x8000)
								if (++counter == descIndex) {
									_activeCollResId = collPtr->id;
									_activeCollIndex = i;
									break;
								}
						}

					} else {

						for (i = 0, collPtr = _collisionAreas; collPtr->left != -1; i++, collPtr++) {
							if ((collPtr->id & 0xF000) == 0x8000) {
								_activeCollResId = collPtr->id;
								_activeCollIndex = i;
								break;
							}
						}
						if ((_lastCollKey != 0) && (_collisionAreas[_lastCollAreaIndex].funcLeave != 0))
							collSub(_collisionAreas[_lastCollAreaIndex].funcLeave);
						_lastCollKey = 0;
					}

				}
			}
		}

		if ((_activeCollResId == 0) || (_collisionAreas[_activeCollIndex].funcLeave != 0))
			continue;

		_vm->_inter->storeMouse();
		if ((_activeCollResId & 0xF000) == 0x8000)
			WRITE_VAR(16, array[_activeCollResId & 0xFFF]);
		else
			WRITE_VAR(16, _activeCollResId & 0xFFF);

		if (_collisionAreas[_activeCollIndex].funcEnter != 0)
			collSub(_collisionAreas[_activeCollIndex].funcEnter);

		WRITE_VAR(16, 0);
		_activeCollResId = 0;
	}
	while (_activeCollResId == 0 && !_vm->_inter->_terminate && !_vm->_quitRequested);

	if ((_activeCollResId & 0xFFF) == collResId) {
		_collStackPos = 0;
		var_24 = 0;
		var_26 = 1;
		for (i = 0; i < 150; i++) {
			if (_collisionAreas[i].left == -1)
				continue;

			if ((_collisionAreas[i].id & 0xC000) == 0x8000)
				continue;

			if ((_collisionAreas[i].flags & 0x0f) < 3)
				continue;

			if ((_collisionAreas[i].flags & 0x0f) > 10)
				continue;

			if ((_collisionAreas[i].flags & 0x0f) > 8) {
				char *ptr;
				strcpy(_tempStr,
				    _vm->_global->_inter_variables + _collisionAreas[i].key);
				while ((ptr = strchr(_tempStr, ' ')) != 0) {
					_vm->_util->cutFromStr(_tempStr, (ptr - _tempStr), 1);
					ptr = strchr(_tempStr, ' ');
				}
				strcpy(_vm->_global->_inter_variables + _collisionAreas[i].key, _tempStr);
				if (_vm->_language == 2) { // loc_16080
					warning("GOB2 Stub! Game_v2::collisionsBlock(), language == 2");
				}
			}

			if ((_collisionAreas[i].flags & 0x0f) >= 5 &&
			    (_collisionAreas[i].flags & 0x0f) <= 8) {
				str = descArray[var_24].ptr;

				strcpy(_tempStr, _vm->_global->_inter_variables + _collisionAreas[i].key);

				if ((_collisionAreas[i].flags & 0x0f) < 7)
					_vm->_util->prepareStr(_tempStr);

				int16 pos = 0;
				do {
					strcpy(_collStr, str);
					pos += strlen(str) + 1;

					str += strlen(str) + 1;

					if ((_collisionAreas[i].flags & 0x0f) < 7)
						_vm->_util->prepareStr(_collStr);

					if (strcmp(_tempStr, _collStr) == 0) {
						VAR(17)++;
						WRITE_VAR(17 + var_26, 1);
						break;
					}
				} while (READ_LE_UINT16(descArray[var_24].ptr - 2) > pos);
				_collStackPos++;
			} else {
				VAR(17 + var_26) = 2;
			}
			var_24++;
			var_26++;
		}

		if (_collStackPos != (int16)VAR(17))
			WRITE_VAR(17, 0);
		else
			WRITE_VAR(17, 1);
	}

	if (_handleMouse == 1)
		_vm->_draw->blitCursor();

	savedIP = 0;
	if (!_vm->_inter->_terminate) {
		savedIP = (char *)_totFileData + _collisionAreas[_activeCollIndex].funcLeave;

		_vm->_inter->storeMouse();
		if ((_activeCollResId & 0xF000) == 0x8000)
			WRITE_VAR(16, array[_activeCollResId & 0xFFF]);
		else
			WRITE_VAR(16, _activeCollResId & 0xFFF);
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

int16 Game_v2::multiEdit(int16 time, int16 index, int16 *pCurPos, InputDesc * inpDesc,
		int16 *collResId, int16 *collIndex) {
	Collision *collArea;
	int16 descInd;
	int16 key;
	int16 found = -1;
	int16 i;
	void *fontExtraBak;
	int16 w2E8E2Bak;

	descInd = 0;
	for (i = 0; i < 150; i++) {
		collArea = &_collisionAreas[i];

		if (collArea->left == -1)
			continue;

		if ((collArea->id & 0xC000) == 0x8000)
			continue;

		if ((collArea->flags & 0x0f) < 3)
			continue;

		if ((collArea->flags & 0x0f) > 10)
			continue;

		strcpy(_tempStr, _vm->_global->_inter_variables + collArea->key);

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

		fontExtraBak = _vm->_draw->_fonts[_vm->_draw->_fontIndex]->extraData;
		w2E8E2Bak = _vm->_draw->_word_2E8E2;
		_vm->_draw->_word_2E8E2 = 2;
		_vm->_draw->_fonts[_vm->_draw->_fontIndex]->extraData = 0;

		_vm->_draw->spriteOperation(DRAW_FILLRECT | 0x10);

		_vm->_draw->_destSpriteY += ((collArea->bottom - collArea->top + 1) -
				_vm->_draw->_fonts[_vm->_draw->_fontIndex]->itemHeight) / 2;

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT | 0x10);

		_vm->_draw->_word_2E8E2 = w2E8E2Bak;
		_vm->_draw->_fonts[_vm->_draw->_fontIndex]->extraData = fontExtraBak;

		descInd++;
	}

	for (i = 0; i < 40; i++) {
		WRITE_VAR_OFFSET(i * 4 + 0x44, 0);
	}

	while (1) {
		descInd = 0;

		for (i = 0; i < 150; i++) {
			collArea = &_collisionAreas[i];

			if (collArea->left == -1)
				continue;

			if ((collArea->id & 0xC000) == 0x8000)
				continue;

			if ((collArea->flags & 0x0f) < 3)
				continue;

			if ((collArea->flags & 0x0f) > 10)
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
		    _vm->_global->_inter_variables + collArea->key,
		    inpDesc[*pCurPos].fontIndex, collArea->flags, &time);

		if (_vm->_inter->_terminate)
			return 0;

		switch (key) {
		case 0:
			if (*collResId == 0)
				return 0;

			if ((_collisionAreas[*collIndex].flags & 0x0f) < 3)
				return 0;

			if ((_collisionAreas[*collIndex].flags & 0x0f) > 10)
				return 0;

			*pCurPos = 0;
			for (i = 0; i < 150; i++) {
				collArea = &_collisionAreas[i];

				if (collArea->left == -1)
					continue;

				if ((collArea->id & 0xC000) == 0x8000)
					continue;

				if ((collArea->flags & 0x0f) < 3)
					continue;

				if ((collArea->flags & 0x0f) > 10)
					continue;

				if (i == *collIndex)
					break;

				pCurPos[0]++;
			}
			break;

		case 0x3b00:
		case 0x3c00:
		case 0x3d00:
		case 0x3e00:
		case 0x3f00:
		case 0x4000:
		case 0x4100:
		case 0x4200:
		case 0x4300:
		case 0x4400:
			return key;

		case 0x1c0d:

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

int16 Game_v2::checkMousePoint(int16 all, int16 *resId, int16 *resIndex) {
	Collision *ptr;
	int16 i;

	if (resId != 0)
		*resId = 0;

	*resIndex = 0;

	ptr = _collisionAreas;
	for (i = 0; ptr->left != -1; ptr++, i++) {
		if (ptr->id & 0x4000)
			continue;

		if (all) {
			if ((ptr->flags & 0xf) > 1)
				continue;

			if ((ptr->flags & 0xf00) != 0)
				continue;

			if (_vm->_global->_inter_mouseX < ptr->left
			    || _vm->_global->_inter_mouseX > ptr->right
			    || _vm->_global->_inter_mouseY < ptr->top
			    || _vm->_global->_inter_mouseY > ptr->bottom)
				continue;

			if (resId != 0)
				*resId = ptr->id;

			*resIndex = i;
			return ptr->key;
		} else {
			if ((ptr->flags & 0xf00) != 0)
				continue;

			if ((ptr->flags & 0xf) < 1)
				continue;

			if ((ptr->flags & 0xf0) >> 4 != _mouseButtons - 1
			    && (ptr->flags & 0xf0) >> 4 != 2)
				continue;

			if (_vm->_global->_inter_mouseX < ptr->left
			    || _vm->_global->_inter_mouseX > ptr->right
			    || _vm->_global->_inter_mouseY < ptr->top
			    || _vm->_global->_inter_mouseY > ptr->bottom)
				continue;

			if (resId != 0)
				*resId = ptr->id;
			*resIndex = i;
			if (((ptr->flags & 0xf) == 1) || ((ptr->flags & 0xf) == 2))
				return ptr->key;
			return 0;
		}
	}

	if (_mouseButtons != 1 && all == 0)
		return 0x11b;

	return 0;
}

} // End of namespace Gob
