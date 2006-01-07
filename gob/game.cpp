/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */
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

namespace Gob {

int16 Game::_captureCount = 0;
Common::Rect Game::_captureStack[20];

Game::Game(GobEngine *vm) : _vm(vm) {
	_extTable = 0;
	_totFileData = 0;
	_totResourceTable = 0;
	_imFileData = 0;
	_extHandle = 0;
	_collisionAreas = 0;
	_shouldPushColls = 0;
	_totTextData = 0;

	// Collisions stack
	_collStackSize = 0;
	for (int i = 0; i < 3; i++) {
		_collStack[i] = 0;
		_collStackElemSizes[i] = 0;
	}

	for (int i = 0; i < 20; i++)
		_soundSamples[i] = 0;

	_curTotFile[0] = 0;
	_curExtFile[0] = 0;
	_totToLoad[0] = 0;

	_startTimeKey = 0;
	_mouseButtons = 0;

	_lastCollKey = 0;
	_lastCollAreaIndex = 0;
	_lastCollId = 0;

	_activeCollResId = 0;
	_activeCollIndex = 0;
	_handleMouse = 0;
	_forceHandleMouse = 0;

	_tempStr[0] = 0;
	_curImaFile[0] = 0;
	_soundFromExt[0] = 0;
	_collStr[0] = 0;


	// Capture
}

char *Game::loadExtData(int16 itemId, int16 *pResWidth, int16 *pResHeight) {
	int16 commonHandle;
	int16 itemsCount;
	int32 offset;
	uint32 size;
	ExtItem *item;
	char isPacked;
	int16 handle;
	int32 tableSize;
	char path[20];
	char *dataBuf;
	char *packedBuf;
	char *dataPtr;

	itemId -= 30000;
	if (_extTable == 0)
		return 0;

	commonHandle = -1;
	itemsCount = _extTable->itemsCount;
	item = &_extTable->items[itemId];
	tableSize = szGame_ExtTable + szGame_ExtItem * itemsCount;

	offset = item->offset;
	size = item->size;
	if (item->width & 0x8000)
		isPacked = 1;
	else
		isPacked = 0;

	if (pResWidth != 0) {
		*pResWidth = item->width & 0x7fff;
		*pResHeight = item->height;
		debug(7, "loadExtData(%d, %d, %d)", itemId, *pResWidth, *pResHeight);
	}

	debug(7, "loadExtData(%d, 0, 0)", itemId);

	if (item->height == 0)
		size += (item->width & 0x7fff) << 16;

	debug(7, "size: %d off: %d", size, offset);
	if (offset >= 0) {
		handle = _extHandle;
	} else {
		offset = -(offset + 1);
		tableSize = 0;
		_vm->_dataio->closeData(_extHandle);
		strcpy(path, "commun.ex1");
		path[strlen(path) - 1] = *(_totFileData + 0x3c) + '0';
		commonHandle = _vm->_dataio->openData(path);
		handle = commonHandle;
	}

	debug(7, "off: %ld size: %ld", offset, tableSize);
	_vm->_dataio->seekData(handle, offset + tableSize, SEEK_SET);
	if (isPacked)
		dataBuf = (char *)malloc(size);
	else
		dataBuf = (char *)malloc(size);

	dataPtr = dataBuf;
	while (size > 32000) {
		// BUG: huge->far conversion. Need normalization?
		_vm->_dataio->readData(handle, (char *)dataPtr, 32000);
		size -= 32000;
		dataPtr += 32000;
	}
	_vm->_dataio->readData(handle, (char *)dataPtr, size);
	if (commonHandle != -1) {
		_vm->_dataio->closeData(commonHandle);
		_extHandle = _vm->_dataio->openData(_curExtFile);
	}

	if (isPacked != 0) {
		packedBuf = dataBuf;
		dataBuf = (char *)malloc(READ_LE_UINT32(packedBuf));
		_vm->_pack->unpackData(packedBuf, dataBuf);
		free(packedBuf);
	}

	return dataBuf;

}

void Game::clearCollisions() {
	int16 i;
	for (i = 0; i < 250; i++) {
		_collisionAreas[i].id = 0;
		_collisionAreas[i].left = -1;
	}
}

void Game::addNewCollision(int16 id, int16 left, int16 top, int16 right, int16 bottom,
	    int16 flags, int16 key, int16 funcEnter, int16 funcLeave) {
	int16 i;
	Collision *ptr;

	debug(5, "addNewCollision");
	debug(5, "id = %x", id);
	debug(5, "left = %d, top = %d, right = %d, bottom = %d", left, top, right, bottom);
	debug(5, "flags = %x, key = %x", flags, key);
	debug(5, "funcEnter = %d, funcLeave = %d", funcEnter, funcLeave);

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

void Game::freeCollision(int16 id) {
	int16 i;

	for (i = 0; i < 250; i++) {
		if (_collisionAreas[i].id == id)
			_collisionAreas[i].left = -1;
	}
}

void Game::pushCollisions(char all) {
	Collision *srcPtr;
	Collision *destPtr;
	int16 size;

	debug(4, "pushCollisions");
	for (size = 0, srcPtr = _collisionAreas; srcPtr->left != -1;
	    srcPtr++) {
		if (all || (srcPtr->id & 0x8000))
			size++;
	}

	destPtr = (Collision *)malloc(size * sizeof(Collision));
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

void Game::popCollisions(void) {
	Collision *destPtr;
	Collision *srcPtr;

	debug(4, "popCollision");

	_collStackSize--;
	for (destPtr = _collisionAreas; destPtr->left != -1; destPtr++);

	srcPtr = _collStack[_collStackSize];
	memcpy(destPtr, srcPtr,
	    _collStackElemSizes[_collStackSize] *
	    sizeof(Collision));

	free(_collStack[_collStackSize]);
}

int16 Game::checkMousePoint(int16 all, int16 *resId, int16 *resIndex) {
	Collision *ptr;
	int16 i;

	if (resId != 0)
		*resId = 0;

	*resIndex = 0;

	ptr = _collisionAreas;
	for (i = 0; ptr->left != -1; ptr++, i++) {
		if (all) {
			if ((ptr->flags & 0xf) > 1)
				continue;

			if ((ptr->flags & 0xff00) != 0)
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
			if ((ptr->flags & 0xff00) != 0)
				continue;

			if ((ptr->flags & 0xf) != 1 && (ptr->flags & 0xf) != 2)
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
			return ptr->key;
		}
	}

	if (_mouseButtons != 1 && all == 0)
		return 0x11b;

	return 0;
}

void Game::capturePush(int16 left, int16 top, int16 width, int16 height) {
	int16 right;

	if (_captureCount == 20)
		error("capturePush: Capture stack overflow!");

	_captureStack[_captureCount].left = left;
	_captureStack[_captureCount].top = top;
	_captureStack[_captureCount].right = left + width;
	_captureStack[_captureCount].bottom = top + height;

	_vm->_draw->_spriteTop = top;
	_vm->_draw->_spriteBottom = height;

	right = left + width - 1;
	left &= 0xfff0;
	right |= 0xf;

	_vm->_draw->_spritesArray[30 + _captureCount] =
	    _vm->_video->initSurfDesc(_vm->_global->_videoMode, right - left + 1, height, 0);

	_vm->_draw->_sourceSurface = 21;
	_vm->_draw->_destSurface = 30 + _captureCount;

	_vm->_draw->_spriteLeft = left;
	_vm->_draw->_spriteRight = right - left + 1;
	_vm->_draw->_destSpriteX = 0;
	_vm->_draw->_destSpriteY = 0;
	_vm->_draw->_transparency = 0;
	_vm->_draw->spriteOperation(0);
	_captureCount++;
}

void Game::capturePop(char doDraw) {
	if (_captureCount <= 0)
		return;

	_captureCount--;
	if (doDraw) {
		_vm->_draw->_destSpriteX = _captureStack[_captureCount].left;
		_vm->_draw->_destSpriteY = _captureStack[_captureCount].top;
		_vm->_draw->_spriteRight =
		    _captureStack[_captureCount].width();
		_vm->_draw->_spriteBottom =
		    _captureStack[_captureCount].height();

		_vm->_draw->_transparency = 0;
		_vm->_draw->_sourceSurface = 30 + _captureCount;
		_vm->_draw->_destSurface = 21;
		_vm->_draw->_spriteLeft = _vm->_draw->_destSpriteX & 0xf;
		_vm->_draw->_spriteTop = 0;
		_vm->_draw->spriteOperation(0);
	}
	_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[30 + _captureCount]);
}

char *Game::loadTotResource(int16 id) {
	TotResItem *itemPtr;
	int32 offset;

	itemPtr = &_totResourceTable->items[id];
	offset = itemPtr->offset;
	if (offset >= 0) {
		return ((char *)_totResourceTable) + szGame_TotResTable +
		    szGame_TotResItem * _totResourceTable->itemsCount + offset;
	} else {
		return (char *)(_imFileData + (int32)READ_LE_UINT32(&((int32 *)_imFileData)[-offset - 1]));
	}
}

void Game::loadSound(int16 slot, char *dataPtr) {
	Snd::SoundDesc *soundDesc;

	soundDesc = (Snd::SoundDesc *)malloc(sizeof(Snd::SoundDesc));

	_soundSamples[slot] = soundDesc;

	soundDesc->frequency = (dataPtr[4] << 8) + dataPtr[5];
	soundDesc->size = (dataPtr[1] << 16) + (dataPtr[2] << 8) + dataPtr[3];
	soundDesc->data = dataPtr + 6;
	soundDesc->timerTicks = (int32)1193180 / (int32)soundDesc->frequency;

	soundDesc->inClocks = (soundDesc->frequency * 10) / 182;
	soundDesc->flag = 0;
}

void Game::interLoadSound(int16 slot) {
	char *dataPtr;
	int16 id;

	if (slot == -1)
		slot = _vm->_parse->parseValExpr();

	id = _vm->_inter->load16();
	if (id == -1) {
		_vm->_global->_inter_execPtr += 9;
		return;
	}

	if (id >= 30000) {
		dataPtr = loadExtData(id, 0, 0);
		_soundFromExt[slot] = 1;
	} else {
		dataPtr = loadTotResource(id);
		_soundFromExt[slot] = 0;
	}

	loadSound(slot, dataPtr);
}

void Game::freeSoundSlot(int16 slot) {
	if (slot == -1)
		slot = _vm->_parse->parseValExpr();

	if (_soundSamples[slot] == 0)
		return;

	if (_soundFromExt[slot] == 1) {
		free(_soundSamples[slot]->data - 6);
		_soundFromExt[slot] = 0;
	}

	free(_soundSamples[slot]);
	_soundSamples[slot] = 0;
}

int16 Game::checkKeys(int16 *pMouseX, int16 *pMouseY, int16 *pButtons, char handleMouse) {
	_vm->_util->processInput();

	if (VAR(58) != 0) {
		if (_vm->_mult->frameStart != (int)VAR(58) - 1)
			_vm->_mult->frameStart++;
		else
			_vm->_mult->frameStart = 0;

		_vm->_mult->playMult(_vm->_mult->frameStart + VAR(57), _vm->_mult->frameStart + VAR(57), 1,
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

int16 Game::checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
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

int16 Game::inputArea(int16 xPos, int16 yPos, int16 width, int16 height, int16 backColor,
	    int16 frontColor, char *str, int16 fontIndex, char inpType, int16 *pTotTime) {
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

	if (_handleMouse != 0 &&
	    (_vm->_global->_useMouse != 0 || _forceHandleMouse != 0))
		handleMouse = 1;
	else
		handleMouse = 0;

	pos = strlen(str);
	pFont = _vm->_draw->_fonts[fontIndex];
	editSize = width / pFont->itemWidth;

	while (1) {
		strcpy(_tempStr, str);
		strcat(_tempStr, " ");
		if (strlen(_tempStr) > editSize)
			strcpy(_tempStr, str);

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

			if (flag != 0) {
				key = checkCollisions(handleMouse, -1,
				    &_activeCollResId,
				    &_activeCollIndex);
			}
			flag = 0;

			key = checkCollisions(handleMouse, -300,
			    &_activeCollResId, &_activeCollIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					_activeCollResId = 0;
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

			if (key != 0 || _activeCollResId != 0)
				break;

			key = checkCollisions(handleMouse, -300,
			    &_activeCollResId, &_activeCollIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					_activeCollResId = 0;
					break;
				}

			}
			if (key != 0 || _activeCollResId != 0)
				break;

			if (_vm->_inter->_terminate)
				return 0;
		}

		if (key == 0 || _activeCollResId != 0 || _vm->_inter->_terminate)
			return 0;

		switch (key) {
		case 0x4d00:	// Right Arrow
			if (pos < strlen(str) && pos < editSize - 1) {
				pos++;
				continue;
			}
			return 0x5000;

		case 0x4b00:	// Left Arrow
			if (pos > 0) {
				pos--;
				continue;
			}
			return 0x4800;

		case 0xe08:	// Backspace
			if (pos > 0) {
				_vm->_util->cutFromStr(str, pos - 1, 1);
				pos--;
				continue;
			}

		case 0x5300:	// Del

			if (pos >= strlen(str))
				continue;

			_vm->_util->cutFromStr(str, pos, 1);
			continue;

		case 0x1c0d:	// Enter
		case 0x3b00:	// F1
		case 0x3c00:	// F2
		case 0x3d00:	// F3
		case 0x3e00:	// F4
		case 0x3f00:	// F5
		case 0x4000:	// F6
		case 0x4100:	// F7
		case 0x4200:	// F8
		case 0x4300:	// F9
		case 0x4400:	// F10
		case 0x4800:	// Up arrow
		case 0x5000:	// Down arrow
			return key;

		case 0x11b:	// Escape
			if (_vm->_global->_useMouse != 0)
				continue;

			_forceHandleMouse = !_forceHandleMouse;

			if (_handleMouse != 0 &&
			    (_vm->_global->_useMouse != 0 || _forceHandleMouse != 0))
				handleMouse = 1;
			else
				handleMouse = 0;

			if (_vm->_global->_pressedKeys[1] == 0)
				continue;

			while (_vm->_global->_pressedKeys[1] != 0);
			continue;

		default:

			savedKey = key;
			key &= 0xff;

			if ((inpType == 9 || inpType == 10) && key >= ' '
			    && key <= 0xff) {
				str1 = "0123456789-.,+ ";
				str2 = "0123456789-,,+ ";

				if ((savedKey >> 8) > 1
				    && (savedKey >> 8) < 12)
					key = ((savedKey >> 8) - 1) % 10 + '0';

				for (i = 0; str1[i] != 0; i++) {
					if (key == str1[i]) {
						key = str2[i];
						break;
					}
				}

				if (i == (int16)strlen(str1))
					key = 0;
			}

			if (key >= ' ' && key <= 0xff) {
				if (editSize == strlen(str))
					_vm->_util->cutFromStr(str, strlen(str) - 1,
					    1);

				if (key >= 'a' && key <= 'z')
					key += ('A' - 'a');

				pos++;
				_tempStr[0] = key;
				_tempStr[1] = 0;

				_vm->_util->insertStr(_tempStr, str, pos - 1);

				//strupr(str);
			}
		}
	}
}

int16 Game::multiEdit(int16 time, int16 index, int16 *pCurPos, InputDesc * inpDesc) {
	Collision *collArea;
	int16 descInd;
	int16 key;
	int16 found = -1;
	int16 i;

	descInd = 0;
	for (i = 0; i < 250; i++) {
		collArea = &_collisionAreas[i];

		if (collArea->left == -1)
			continue;

		if ((collArea->id & 0x8000) == 0)
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
		_vm->_draw->spriteOperation(DRAW_FILLRECT);
		_vm->_draw->_destSpriteY +=
		    ((collArea->bottom - collArea->top + 1) - 8) / 2;

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
		descInd++;
	}

	for (i = 0; i < 40; i++) {
		WRITE_VAR_OFFSET(i * 4 + 0x44, 0);
	}

	while (1) {
		descInd = 0;

		for (i = 0; i < 250; i++) {
			collArea = &_collisionAreas[i];

			if (collArea->left == -1)
				continue;

			if ((collArea->id & 0x8000) == 0)
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
			if (_activeCollResId == 0)
				return 0;

			if ((_collisionAreas[_activeCollIndex].
				flags & 0x0f) < 3)
				return 0;

			if ((_collisionAreas[_activeCollIndex].
				flags & 0x0f) > 10)
				return 0;

			*pCurPos = 0;
			for (i = 0; i < 250; i++) {
				collArea = &_collisionAreas[i];

				if (collArea->left == -1)
					continue;

				if ((collArea->id & 0x8000) == 0)
					continue;

				if ((collArea->flags & 0x0f) < 3)
					continue;

				if ((collArea->flags & 0x0f) > 10)
					continue;

				if (i == _activeCollIndex)
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

int16 Game::adjustKey(int16 key) {
	if (key <= 0x60 || key >= 0x7b)
		return key;

	return key - 0x20;
}

void Game::collisionsBlock(void) {
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
	int16 var_22;
	int16 index;
	int16 curEditIndex;
	int16 deltaTime;
	int16 descIndex2;
	int16 stackPos2;
	int16 descIndex;
	int16 timeVal;
	char *str;
	int16 pos;
	int16 savedCollStackSize;
	int16 i;
	int16 counter;
	int16 var_24;
	int16 var_26;
	int16 _collStackPos;
	Collision *collPtr;
	int16 timeKey;
	char *savedIP;

	if (_shouldPushColls)
		pushCollisions(1);

	collResId = -1;
	_vm->_global->_inter_execPtr++;
	count = *_vm->_global->_inter_execPtr++;
	_handleMouse = _vm->_global->_inter_execPtr[0];
	deltaTime = 1000 * (byte)_vm->_global->_inter_execPtr[1];
	descIndex2 = (byte)_vm->_global->_inter_execPtr[2];
	stackPos2 = (byte)_vm->_global->_inter_execPtr[3];
	descIndex = (byte)_vm->_global->_inter_execPtr[4];

	if (stackPos2 != 0 || descIndex != 0)
		deltaTime /= 100;

	timeVal = deltaTime;
	_vm->_global->_inter_execPtr += 6;

	startIP = _vm->_global->_inter_execPtr;
	WRITE_VAR(16, 0);
	var_22 = 0;
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
		} else {
			cmdHigh = 0;
		}

		if ((cmd & 0x80) != 0) {
			left = _vm->_parse->parseValExpr();
			top = _vm->_parse->parseValExpr();
			width = _vm->_parse->parseValExpr();
			height = _vm->_parse->parseValExpr();
		} else {
			left = _vm->_inter->load16();
			top = _vm->_inter->load16();
			width = _vm->_inter->load16();
			height = _vm->_inter->load16();
		}
		cmd &= 0x7f;

		debug(4, "collisionsBlock(%d)", cmd);

		switch (cmd) {
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:

			_vm->_util->waitKey();
			var_22 = 1;
			key = _vm->_parse->parseVarIndex();
			descArray[index].fontIndex = _vm->_inter->load16();
			descArray[index].backColor = *_vm->_global->_inter_execPtr++;
			descArray[index].frontColor = *_vm->_global->_inter_execPtr++;

			if (cmd < 5 || cmd > 8) {
				descArray[index].ptr = 0;
			} else {
				descArray[index].ptr = _vm->_global->_inter_execPtr + 2;
				_vm->_global->_inter_execPtr += _vm->_inter->load16();
			}

			if (left == -1)
				break;

			if ((cmd & 1) == 0) {
				addNewCollision(curCmd + 0x8000, left,
				    top,
				    left +
				    width *
				    _vm->_draw->_fonts[descArray[index].fontIndex]->
				    itemWidth - 1, top + height - 1, cmd, key,
				    0,
				    _vm->_global->_inter_execPtr - (char *)_totFileData);

				_vm->_global->_inter_execPtr += 2;
				_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			} else {
				addNewCollision(curCmd + 0x8000, left,
				    top,
				    left +
				    width *
				    _vm->_draw->_fonts[descArray[index].fontIndex]->
				    itemWidth - 1, top + height - 1, cmd, key,
				    0, 0);
			}
			index++;
			break;

		case 21:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16() & 3;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmdHigh + 2, key,
			    _vm->_global->_inter_execPtr - (char *)_totFileData, 0);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;

		case 20:
			collResId = curCmd;
			// Fall through to case 2

		case 2:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16() & 3;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmdHigh + 2, key, 0,
			    _vm->_global->_inter_execPtr - (char *)_totFileData);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;

		case 0:
			_vm->_global->_inter_execPtr += 6;
			startIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    cmd + cmdHigh, key,
			    startIP - (char *)_totFileData,
			    _vm->_global->_inter_execPtr - (char *)_totFileData);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;

		case 1:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16() & 3;

			startIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			if (key == 0)
				key = curCmd + 0xa000;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmd + cmdHigh, key,
			    startIP - (char *)_totFileData,
			    _vm->_global->_inter_execPtr - (char *)_totFileData);

			_vm->_global->_inter_execPtr += 2;
			_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr);
			break;
		}
	}

	_forceHandleMouse = 0;
	_vm->_util->waitKey();

	do {
		if (var_22 != 0) {
			key =
			    multiEdit(deltaTime, index, &curEditIndex,
			    descArray);

			if (key == 0x1c0d) {
				for (i = 0; i < 250; i++) {
					if (_collisionAreas[i].left == -1)
						continue;

					if ((_collisionAreas[i].id & 0x8000) == 0)
						continue;

					if ((_collisionAreas[i].flags & 1) != 0)
						continue;

					if ((_collisionAreas[i].flags & 0x0f) <= 2)
						continue;

					collResId = _collisionAreas[i].id;
					_activeCollResId = collResId;
					collResId &= 0x7fff;
					_activeCollIndex = i;
					break;
				}
				break;
			}
		} else {
			key =
			    checkCollisions(_handleMouse, -deltaTime,
			    &_activeCollResId, &_activeCollIndex);
		}

		if ((key & 0xff) >= ' ' && (key & 0xff) <= 0xff &&
		    (key >> 8) > 1 && (key >> 8) < 12) {
			key = '0' + (((key >> 8) - 1) % 10) + (key & 0xff00);
		}

		if (_activeCollResId == 0) {
			if (key != 0) {
				for (i = 0; i < 250; i++) {
					if (_collisionAreas[i].left == -1)
						continue;

					if ((_collisionAreas[i].
						id & 0x8000) == 0)
						continue;

					if (_collisionAreas[i].key == key
					    || _collisionAreas[i].key ==
					    0x7fff) {

						_activeCollResId =
						    _collisionAreas[i].id;
						_activeCollIndex = i;
						break;
					}
				}

				if (_activeCollResId == 0) {
					for (i = 0; i < 250; i++) {
						if (_collisionAreas[i].left == -1)
							continue;

						if ((_collisionAreas[i].id & 0x8000) == 0)
							continue;

						if ((_collisionAreas[i].key & 0xff00) != 0)
							continue;

						if (_collisionAreas[i].key == 0)
							continue;

						if (adjustKey(key & 0xff) == adjustKey(_collisionAreas[i].key) || _collisionAreas[i].key == 0x7fff) {
							_activeCollResId = _collisionAreas[i].id;
							_activeCollIndex = i;
							break;
						}
					}
				}
			} else {

				if (deltaTime != 0 && VAR(16) == 0) {
					if (stackPos2 != 0) {
						_collStackPos = 0;
						collPtr = _collisionAreas;

						for (i = 0, collPtr = _collisionAreas; collPtr->left != -1; i++, collPtr++) {
							if ((collPtr->id & 0x8000) == 0)
								continue;

							_collStackPos++;
							if (_collStackPos != stackPos2)
								continue;

							_activeCollResId = collPtr->id;
							_activeCollIndex = i;
							WRITE_VAR(2, _vm->_global->_inter_mouseX);
							WRITE_VAR(3, _vm->_global->_inter_mouseY);
							WRITE_VAR(4, _mouseButtons);
							WRITE_VAR(16, array[(uint16)_activeCollResId & ~0x8000]);

							if (collPtr->funcLeave != 0) {
								timeKey = _vm->_util->getTimeKey();
								savedIP = _vm->_global->_inter_execPtr;
								_vm->_global->_inter_execPtr = (char *)_totFileData + collPtr->funcLeave;
								_shouldPushColls = 1;
								savedCollStackSize = _collStackSize;
								_vm->_inter->funcBlock(0);

								if (savedCollStackSize != _collStackSize)
									popCollisions();

								_shouldPushColls = 0;
								_vm->_global->_inter_execPtr = savedIP;
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
							for (i = 0; i < 250; i++) {
								if (_collisionAreas[i].left == -1)
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
								if (_collisionAreas[i].left == -1)
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
							if (_collisionAreas[i].left == -1)
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
		WRITE_VAR(16, array[(uint16)_activeCollResId & ~0x8000]);

		if (_collisionAreas[_activeCollIndex].funcEnter != 0) {
			savedIP = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr = (char *)_totFileData +
			    _collisionAreas[_activeCollIndex].
			    funcEnter;

			_shouldPushColls = 1;

			_collStackPos = _collStackSize;
			_vm->_inter->funcBlock(0);
			if (_collStackPos != _collStackSize)
				popCollisions();
			_shouldPushColls = 0;
			_vm->_global->_inter_execPtr = savedIP;
		}

		WRITE_VAR(16, 0);
		_activeCollResId = 0;
	}
	while (_activeCollResId == 0 && !_vm->_inter->_terminate);

	if (((uint16)_activeCollResId & ~0x8000) == collResId) {
		_collStackPos = 0;
		var_24 = 0;
		var_26 = 1;
		for (i = 0; i < 250; i++) {
			if (_collisionAreas[i].left == -1)
				continue;

			if ((_collisionAreas[i].id & 0x8000) == 0)
				continue;

			if ((_collisionAreas[i].flags & 0x0f) < 3)
				continue;

			if ((_collisionAreas[i].flags & 0x0f) > 10)
				continue;

			if ((_collisionAreas[i].flags & 0x0f) > 8) {
				strcpy(_tempStr,
				    _vm->_global->_inter_variables + _collisionAreas[i].key);
				while ((pos =
					_vm->_util->strstr(" ", _tempStr)) != 0) {
					_vm->_util->cutFromStr(_tempStr, pos - 1, 1);
					pos = _vm->_util->strstr(" ", _tempStr);
				}
				strcpy(_vm->_global->_inter_variables + _collisionAreas[i].key, _tempStr);
			}

			if ((_collisionAreas[i].flags & 0x0f) >= 5 &&
			    (_collisionAreas[i].flags & 0x0f) <= 8) {
				str = descArray[var_24].ptr;

				strcpy(_tempStr, _vm->_global->_inter_variables + _collisionAreas[i].key);

				if ((_collisionAreas[i].flags & 0x0f) < 7)
					_vm->_util->prepareStr(_tempStr);

				pos = 0;
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

	savedIP = 0;
	if (!_vm->_inter->_terminate) {
		savedIP = (char *)_totFileData +
		    _collisionAreas[_activeCollIndex].funcLeave;

		WRITE_VAR(2, _vm->_global->_inter_mouseX);
		WRITE_VAR(3, _vm->_global->_inter_mouseY);
		WRITE_VAR(4, _mouseButtons);

		if (VAR(16) == 0) {
			WRITE_VAR(16, array[(uint16)_activeCollResId & ~0x8000]);
		}
	}

	for (curCmd = 0; curCmd < count; curCmd++) {
		freeCollision(curCmd + 0x8000);
	}
	_vm->_global->_inter_execPtr = savedIP;
}

void Game::prepareStart(void) {
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

void Game::loadTotFile(char *path) {
	int16 handle;

	handle = _vm->_dataio->openData(path);
	if (handle >= 0) {
		_vm->_dataio->closeData(handle);
		_totFileData = _vm->_dataio->getData(path);
	} else {
		_totFileData = 0;
	}
}

void Game::loadExtTable(void) {
	int16 count, i;

	// Function is correct. [sev]

	_extHandle = _vm->_dataio->openData(_curExtFile);
	if (_extHandle < 0)
		return;

	_vm->_dataio->readData(_extHandle, (char *)&count, 2);
	count = FROM_LE_16(count);

	_vm->_dataio->seekData(_extHandle, 0, 0);
	_extTable = (ExtTable *)malloc(sizeof(ExtTable)
	    + sizeof(ExtItem) * count);

	_vm->_dataio->readData(_extHandle, (char *)&_extTable->itemsCount, 2);
	_extTable->itemsCount = FROM_LE_16(_extTable->itemsCount);
	_vm->_dataio->readData(_extHandle, (char *)&_extTable->unknown, 1);

	for (i = 0; i < count; i++) {
		_vm->_dataio->readData(_extHandle, (char *)&_extTable->items[i].offset, 4);
		_extTable->items[i].offset = FROM_LE_32(_extTable->items[i].offset);
		_vm->_dataio->readData(_extHandle, (char *)&_extTable->items[i].size, 2);
		_extTable->items[i].size = FROM_LE_16(_extTable->items[i].size);
		_vm->_dataio->readData(_extHandle, (char *)&_extTable->items[i].width, 2);
		_extTable->items[i].width = FROM_LE_16(_extTable->items[i].width);
		_vm->_dataio->readData(_extHandle, (char *)&_extTable->items[i].height, 2);
		_extTable->items[i].height = FROM_LE_16(_extTable->items[i].height);
	}
}

void Game::loadImFile(void) {
	char path[20];
	int16 handle;

	if (_totFileData[0x3d] != 0 && _totFileData[0x3b] == 0)
		return;

	strcpy(path, "commun.im1");
	if (_totFileData[0x3b] != 0)
		path[strlen(path) - 1] = '0' + _totFileData[0x3b];

	handle = _vm->_dataio->openData(path);
	if (handle < 0)
		return;

	_vm->_dataio->closeData(handle);
	_imFileData = _vm->_dataio->getData(path);
}

void Game::playTot(int16 skipPlay) {
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
	oldCaptureCounter = _vm->_scenery->pCaptureCounter;
	savedIP = _vm->_global->_inter_execPtr;

	_vm->_inter->_nestLevel = &nestLevel;
	_vm->_inter->_breakFromLevel = &breakFrom;
	_vm->_scenery->pCaptureCounter = &_captureCounter;
	strcpy(savedTotName, _curTotFile);

	if (skipPlay == 0) {
		while (1) {
			for (i = 0; i < 4; i++) {
				_vm->_draw->_fontToSprite[i].sprite = -1;
				_vm->_draw->_fontToSprite[i].base = -1;
				_vm->_draw->_fontToSprite[i].width = -1;
				_vm->_draw->_fontToSprite[i].height = -1;
			}

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

			debug(4, "IMA: %s", _curImaFile);
			debug(4, "EXT: %s", _curExtFile);

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
				_vm->_global->_inter_variables = (char *)malloc(variablesCount * 4);
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
			free(_totFileData);
			_totFileData = 0;

			if (needTextFree)
				free(_totTextData);
			_totTextData = 0;

			if (needFreeResTable)
				free(_totResourceTable);
			_totResourceTable = 0;

			free(_imFileData);
			_imFileData = 0;

			free(_extTable);
			_extTable = 0;

			if (_extHandle >= 0)
				_vm->_dataio->closeData(_extHandle);

			_extHandle = -1;

			for (i = 0; i < *_vm->_scenery->pCaptureCounter; i++)
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
	_vm->_scenery->pCaptureCounter = oldCaptureCounter;
	_vm->_global->_inter_execPtr = savedIP;
}

void Game::start(void) {
	_collisionAreas = (Collision *)malloc(250 * sizeof(Collision));
	prepareStart();
	playTot(0);

	free(_collisionAreas);

	_vm->_video->freeSurfDesc(_vm->_draw->_cursorSprites);
	_vm->_video->freeSurfDesc(_vm->_draw->_cursorBack);
	_vm->_video->freeSurfDesc(_vm->_draw->_backSurface);
}

} // End of namespace Gob
