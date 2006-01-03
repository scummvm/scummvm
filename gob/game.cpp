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

int16 Game::captureCount = 0;
Common::Rect Game::captureStack[20];

Game::Game(GobEngine *vm) : _vm(vm) {
	extTable = 0;
	totFileData = 0;
	totResourceTable = 0;
	imFileData = 0;
	extHandle = 0;
	collisionAreas = 0;
	shouldPushColls = 0;
	totTextData = 0;

	// Collisions stack
	collStackSize = 0;
	for (int i = 0; i < 3; i++) {
		collStack[i] = 0;
		collStackElemSizes[i] = 0;
	}

	for (int i = 0; i < 20; i++)
		soundSamples[i] = 0;

	curTotFile[0] = 0;
	curExtFile[0] = 0;
	totToLoad[0] = 0;

	startTimeKey = 0;
	mouseButtons = 0;

	lastCollKey = 0;
	lastCollAreaIndex = 0;
	lastCollId = 0;

	activeCollResId = 0;
	activeCollIndex = 0;
	ghandleMouse = 0;
	forceHandleMouse = 0;

	tempStr[0] = 0;
	curImaFile[0] = 0;
	soundFromExt[0] = 0;
	collStr[0] = 0;


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
	if (extTable == 0)
		return 0;

	commonHandle = -1;
	itemsCount = extTable->itemsCount;
	item = &extTable->items[itemId];
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
		handle = extHandle;
	} else {
		offset = -(offset + 1);
		tableSize = 0;
		_vm->_dataio->closeData(extHandle);
		strcpy(path, "commun.ex1");
		path[strlen(path) - 1] = *(totFileData + 0x3c) + '0';
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
		extHandle = _vm->_dataio->openData(curExtFile);
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
		collisionAreas[i].id = 0;
		collisionAreas[i].left = -1;
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
		if (collisionAreas[i].left != -1)
			continue;

		ptr = &collisionAreas[i];
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
		if (collisionAreas[i].id == id)
			collisionAreas[i].left = -1;
	}
}

void Game::pushCollisions(char all) {
	Collision *srcPtr;
	Collision *destPtr;
	int16 size;

	debug(4, "pushCollisions");
	for (size = 0, srcPtr = collisionAreas; srcPtr->left != -1;
	    srcPtr++) {
		if (all || (srcPtr->id & 0x8000))
			size++;
	}

	destPtr = (Collision *)malloc(size * sizeof(Collision));
	collStack[collStackSize] = destPtr;
	collStackElemSizes[collStackSize] = size;
	collStackSize++;

	for (srcPtr = collisionAreas; srcPtr->left != -1; srcPtr++) {
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

	collStackSize--;
	for (destPtr = collisionAreas; destPtr->left != -1; destPtr++);

	srcPtr = collStack[collStackSize];
	memcpy(destPtr, srcPtr,
	    collStackElemSizes[collStackSize] *
	    sizeof(Collision));

	free(collStack[collStackSize]);
}

int16 Game::checkMousePoint(int16 all, int16 *resId, int16 *resIndex) {
	Collision *ptr;
	int16 i;

	if (resId != 0)
		*resId = 0;

	*resIndex = 0;

	ptr = collisionAreas;
	for (i = 0; ptr->left != -1; ptr++, i++) {
		if (all) {
			if ((ptr->flags & 0xf) > 1)
				continue;

			if ((ptr->flags & 0xff00) != 0)
				continue;

			if (_vm->_global->inter_mouseX < ptr->left
			    || _vm->_global->inter_mouseX > ptr->right
			    || _vm->_global->inter_mouseY < ptr->top
			    || _vm->_global->inter_mouseY > ptr->bottom)
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

			if ((ptr->flags & 0xf0) >> 4 != mouseButtons - 1
			    && (ptr->flags & 0xf0) >> 4 != 2)
				continue;

			if (_vm->_global->inter_mouseX < ptr->left
			    || _vm->_global->inter_mouseX > ptr->right
			    || _vm->_global->inter_mouseY < ptr->top
			    || _vm->_global->inter_mouseY > ptr->bottom)
				continue;

			if (resId != 0)
				*resId = ptr->id;
			*resIndex = i;
			return ptr->key;
		}
	}

	if (mouseButtons != 1 && all == 0)
		return 0x11b;

	return 0;
}

void Game::capturePush(int16 left, int16 top, int16 width, int16 height) {
	int16 right;

	if (captureCount == 20)
		error("capturePush: Capture stack overflow!");

	captureStack[captureCount].left = left;
	captureStack[captureCount].top = top;
	captureStack[captureCount].right = left + width;
	captureStack[captureCount].bottom = top + height;

	_vm->_draw->spriteTop = top;
	_vm->_draw->spriteBottom = height;

	right = left + width - 1;
	left &= 0xfff0;
	right |= 0xf;

	_vm->_draw->spritesArray[30 + captureCount] =
	    _vm->_video->initSurfDesc(_vm->_global->videoMode, right - left + 1, height, 0);

	_vm->_draw->sourceSurface = 21;
	_vm->_draw->destSurface = 30 + captureCount;

	_vm->_draw->spriteLeft = left;
	_vm->_draw->spriteRight = right - left + 1;
	_vm->_draw->destSpriteX = 0;
	_vm->_draw->destSpriteY = 0;
	_vm->_draw->transparency = 0;
	_vm->_draw->spriteOperation(0);
	captureCount++;
}

void Game::capturePop(char doDraw) {
	if (captureCount <= 0)
		return;

	captureCount--;
	if (doDraw) {
		_vm->_draw->destSpriteX = captureStack[captureCount].left;
		_vm->_draw->destSpriteY = captureStack[captureCount].top;
		_vm->_draw->spriteRight =
		    captureStack[captureCount].width();
		_vm->_draw->spriteBottom =
		    captureStack[captureCount].height();

		_vm->_draw->transparency = 0;
		_vm->_draw->sourceSurface = 30 + captureCount;
		_vm->_draw->destSurface = 21;
		_vm->_draw->spriteLeft = _vm->_draw->destSpriteX & 0xf;
		_vm->_draw->spriteTop = 0;
		_vm->_draw->spriteOperation(0);
	}
	_vm->_video->freeSurfDesc(_vm->_draw->spritesArray[30 + captureCount]);
}

char *Game::loadTotResource(int16 id) {
	TotResItem *itemPtr;
	int32 offset;

	itemPtr = &totResourceTable->items[id];
	offset = itemPtr->offset;
	if (offset >= 0) {
		return ((char *)totResourceTable) + szGame_TotResTable +
		    szGame_TotResItem * totResourceTable->itemsCount + offset;
	} else {
		return (char *)(imFileData + (int32)READ_LE_UINT32(&((int32 *)imFileData)[-offset - 1]));
	}
}

void Game::loadSound(int16 slot, char *dataPtr) {
	Snd::SoundDesc *soundDesc;

	soundDesc = (Snd::SoundDesc *)malloc(sizeof(Snd::SoundDesc));

	soundSamples[slot] = soundDesc;

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
		_vm->_global->inter_execPtr += 9;
		return;
	}

	if (id >= 30000) {
		dataPtr = loadExtData(id, 0, 0);
		soundFromExt[slot] = 1;
	} else {
		dataPtr = loadTotResource(id);
		soundFromExt[slot] = 0;
	}

	loadSound(slot, dataPtr);
}

void Game::freeSoundSlot(int16 slot) {
	if (slot == -1)
		slot = _vm->_parse->parseValExpr();

	if (soundSamples[slot] == 0)
		return;

	if (soundFromExt[slot] == 1) {
		free(soundSamples[slot]->data - 6);
		soundFromExt[slot] = 0;
	}

	free(soundSamples[slot]);
	soundSamples[slot] = 0;
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

	if (_vm->_inter->soundEndTimeKey != 0
	    && _vm->_util->getTimeKey() >= _vm->_inter->soundEndTimeKey) {
		_vm->_snd->stopSound(_vm->_inter->soundStopVal);
		_vm->_inter->soundEndTimeKey = 0;
	}

	if (_vm->_global->useMouse == 0)
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
		lastCollKey = 0;
		lastCollAreaIndex = 0;
		lastCollId = 0;
	}

	if (pResId != 0)
		*pResId = 0;

	resIndex = 0;

	if (_vm->_draw->gcursorIndex == -1 && handleMouse != 0
	    && lastCollKey == 0) {
		lastCollKey =
		    checkMousePoint(1, &lastCollId,
		    &lastCollAreaIndex);

		if (lastCollKey != 0 && (lastCollId & 0x8000) != 0) {
			savedIP = _vm->_global->inter_execPtr;
			_vm->_global->inter_execPtr = (char *)totFileData +
			    collisionAreas[lastCollAreaIndex].funcEnter;

			_vm->_inter->funcBlock(0);
			_vm->_global->inter_execPtr = savedIP;
		}
	}

	if (handleMouse != 0)
		_vm->_draw->animateCursor(-1);

	timeKey = _vm->_util->getTimeKey();
	while (1) {
		if (_vm->_inter->terminate != 0) {
			if (handleMouse)
				_vm->_draw->blitCursor();
			return 0;
		}

		if (_vm->_draw->noInvalidated == 0) {
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

		key = checkKeys(&_vm->_global->inter_mouseX, &_vm->_global->inter_mouseY,
							 &mouseButtons, handleMouse);

		if (handleMouse == 0 && mouseButtons != 0) {
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

			if (lastCollKey != 0 &&
			    collisionAreas[lastCollAreaIndex].funcLeave != 0) {
				savedIP = _vm->_global->inter_execPtr;
				_vm->_global->inter_execPtr = (char *)totFileData +
				    collisionAreas[lastCollAreaIndex].funcLeave;

				_vm->_inter->funcBlock(0);
				_vm->_global->inter_execPtr = savedIP;
			}

			lastCollKey = 0;
			if (key != 0)
				return key;
		}

		if (handleMouse != 0) {
			if (mouseButtons != 0) {
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
						|| mouseButtons == 0))
						_vm->_draw->blitCursor();

					if (lastCollKey != 0 &&
						collisionAreas[lastCollAreaIndex].funcLeave != 0) {
						savedIP = _vm->_global->inter_execPtr;
						_vm->_global->inter_execPtr =
						    (char *)totFileData +
						    collisionAreas[lastCollAreaIndex].funcLeave;

						_vm->_inter->funcBlock(0);
						_vm->_global->inter_execPtr = savedIP;
					}
					lastCollKey = 0;
					return key;
				}

				if (lastCollKey != 0 &&
				    collisionAreas[lastCollAreaIndex].funcLeave != 0) {
					savedIP = _vm->_global->inter_execPtr;
					_vm->_global->inter_execPtr =
					    (char *)totFileData +
					    collisionAreas[lastCollAreaIndex].funcLeave;

					_vm->_inter->funcBlock(0);
					_vm->_global->inter_execPtr = savedIP;
				}

				lastCollKey =
				    checkMousePoint(1, &lastCollId,
				    &lastCollAreaIndex);

				if (lastCollKey != 0
				    && (lastCollId & 0x8000) != 0) {
					savedIP = _vm->_global->inter_execPtr;
					_vm->_global->inter_execPtr =
					    (char *)totFileData +
					    collisionAreas[lastCollAreaIndex].funcEnter;

					_vm->_inter->funcBlock(0);
					_vm->_global->inter_execPtr = savedIP;
				}
			} else {

				if (handleMouse != 0 &&
				    (_vm->_global->inter_mouseX != _vm->_draw->cursorX
					|| _vm->_global->inter_mouseY != _vm->_draw->cursorY)) {
					oldIndex = lastCollAreaIndex;
					oldId = lastCollId;

					key =
					    checkMousePoint(1,
					    &lastCollId,
					    &lastCollAreaIndex);

					if (key != lastCollKey) {
						if (lastCollKey != 0
						    && (oldId & 0x8000) != 0) {
							savedIP = _vm->_global->inter_execPtr;
							_vm->_global->inter_execPtr = (char *)totFileData +
							    collisionAreas[oldIndex].funcLeave;

							_vm->_inter->funcBlock(0);
							_vm->_global->inter_execPtr = savedIP;
						}

						lastCollKey = key;
						if (lastCollKey != 0 && (lastCollId & 0x8000) != 0) {
							savedIP = _vm->_global->inter_execPtr;
							_vm->_global->inter_execPtr = (char *)totFileData +
							    collisionAreas[lastCollAreaIndex].funcEnter;

							_vm->_inter->funcBlock(0);
							_vm->_global->inter_execPtr = savedIP;
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

	if (ghandleMouse != 0 &&
	    (_vm->_global->useMouse != 0 || forceHandleMouse != 0))
		handleMouse = 1;
	else
		handleMouse = 0;

	pos = strlen(str);
	pFont = _vm->_draw->fonts[fontIndex];
	editSize = width / pFont->itemWidth;

	while (1) {
		strcpy(tempStr, str);
		strcat(tempStr, " ");
		if (strlen(tempStr) > editSize)
			strcpy(tempStr, str);

		_vm->_draw->destSpriteX = xPos;
		_vm->_draw->destSpriteY = yPos;
		_vm->_draw->spriteRight = editSize * pFont->itemWidth;
		_vm->_draw->spriteBottom = height;

		_vm->_draw->destSurface = 21;
		_vm->_draw->backColor = backColor;
		_vm->_draw->frontColor = frontColor;
		_vm->_draw->textToPrint = tempStr;
		_vm->_draw->transparency = 1;
		_vm->_draw->fontIndex = fontIndex;
		_vm->_draw->spriteOperation(DRAW_FILLRECT);

		_vm->_draw->destSpriteY = yPos + (height - 8) / 2;

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
		if (pos == editSize)
			pos--;

		curSym = tempStr[pos];

		flag = 1;

		while (1) {
			tempStr[0] = curSym;
			tempStr[1] = 0;

			_vm->_draw->destSpriteX = xPos + pFont->itemWidth * pos;
			_vm->_draw->destSpriteY = yPos + height - 1;
			_vm->_draw->spriteRight = pFont->itemWidth;
			_vm->_draw->spriteBottom = 1;
			_vm->_draw->destSurface = 21;
			_vm->_draw->backColor = frontColor;
			_vm->_draw->spriteOperation(DRAW_FILLRECT);

			if (flag != 0) {
				key = checkCollisions(handleMouse, -1,
				    &activeCollResId,
				    &activeCollIndex);
			}
			flag = 0;

			key = checkCollisions(handleMouse, -300,
			    &activeCollResId, &activeCollIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					activeCollResId = 0;
					break;
				}
			}

			tempStr[0] = curSym;
			tempStr[1] = 0;
			_vm->_draw->destSpriteX = xPos + pFont->itemWidth * pos;
			_vm->_draw->destSpriteY = yPos + height - 1;
			_vm->_draw->spriteRight = pFont->itemWidth;
			_vm->_draw->spriteBottom = 1;
			_vm->_draw->destSurface = 21;
			_vm->_draw->backColor = backColor;
			_vm->_draw->frontColor = frontColor;
			_vm->_draw->textToPrint = tempStr;
			_vm->_draw->transparency = 1;
			_vm->_draw->spriteOperation(DRAW_FILLRECT);

			_vm->_draw->destSpriteY = yPos + (height - 8) / 2;
			_vm->_draw->spriteOperation(DRAW_PRINTTEXT);

			if (key != 0 || activeCollResId != 0)
				break;

			key = checkCollisions(handleMouse, -300,
			    &activeCollResId, &activeCollIndex);

			if (*pTotTime > 0) {
				*pTotTime -= 300;
				if (*pTotTime <= 1) {
					key = 0;
					activeCollResId = 0;
					break;
				}

			}
			if (key != 0 || activeCollResId != 0)
				break;

			if (_vm->_inter->terminate != 0)
				return 0;
		}

		if (key == 0 || activeCollResId != 0
		    || _vm->_inter->terminate != 0)
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
			if (_vm->_global->useMouse != 0)
				continue;

			forceHandleMouse = !forceHandleMouse;

			if (ghandleMouse != 0 &&
			    (_vm->_global->useMouse != 0 || forceHandleMouse != 0))
				handleMouse = 1;
			else
				handleMouse = 0;

			if (_vm->_global->pressedKeys[1] == 0)
				continue;

			while (_vm->_global->pressedKeys[1] != 0);
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
				tempStr[0] = key;
				tempStr[1] = 0;

				_vm->_util->insertStr(tempStr, str, pos - 1);

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
		collArea = &collisionAreas[i];

		if (collArea->left == -1)
			continue;

		if ((collArea->id & 0x8000) == 0)
			continue;

		if ((collArea->flags & 0x0f) < 3)
			continue;

		if ((collArea->flags & 0x0f) > 10)
			continue;

		strcpy(tempStr, _vm->_global->inter_variables + collArea->key);

		_vm->_draw->destSpriteX = collArea->left;
		_vm->_draw->destSpriteY = collArea->top;
		_vm->_draw->spriteRight = collArea->right - collArea->left + 1;
		_vm->_draw->spriteBottom = collArea->bottom - collArea->top + 1;

		_vm->_draw->destSurface = 21;

		_vm->_draw->backColor = inpDesc[descInd].backColor;
		_vm->_draw->frontColor = inpDesc[descInd].frontColor;
		_vm->_draw->textToPrint = tempStr;
		_vm->_draw->transparency = 1;
		_vm->_draw->fontIndex = inpDesc[descInd].fontIndex;
		_vm->_draw->spriteOperation(DRAW_FILLRECT);
		_vm->_draw->destSpriteY +=
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
			collArea = &collisionAreas[i];

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

		collArea = &collisionAreas[found];

		key = inputArea(collArea->left, collArea->top,
		    collArea->right - collArea->left + 1,
		    collArea->bottom - collArea->top + 1,
		    inpDesc[*pCurPos].backColor, inpDesc[*pCurPos].frontColor,
		    _vm->_global->inter_variables + collArea->key,
		    inpDesc[*pCurPos].fontIndex, collArea->flags, &time);

		if (_vm->_inter->terminate != 0)
			return 0;

		switch (key) {
		case 0:
			if (activeCollResId == 0)
				return 0;

			if ((collisionAreas[activeCollIndex].
				flags & 0x0f) < 3)
				return 0;

			if ((collisionAreas[activeCollIndex].
				flags & 0x0f) > 10)
				return 0;

			*pCurPos = 0;
			for (i = 0; i < 250; i++) {
				collArea = &collisionAreas[i];

				if (collArea->left == -1)
					continue;

				if ((collArea->id & 0x8000) == 0)
					continue;

				if ((collArea->flags & 0x0f) < 3)
					continue;

				if ((collArea->flags & 0x0f) > 10)
					continue;

				if (i == activeCollIndex)
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
	int16 collStackPos;
	Collision *collPtr;
	int16 timeKey;
	char *savedIP;

	if (shouldPushColls)
		pushCollisions(1);

	collResId = -1;
	_vm->_global->inter_execPtr++;
	count = *_vm->_global->inter_execPtr++;
	ghandleMouse = _vm->_global->inter_execPtr[0];
	deltaTime = 1000 * (byte)_vm->_global->inter_execPtr[1];
	descIndex2 = (byte)_vm->_global->inter_execPtr[2];
	stackPos2 = (byte)_vm->_global->inter_execPtr[3];
	descIndex = (byte)_vm->_global->inter_execPtr[4];

	if (stackPos2 != 0 || descIndex != 0)
		deltaTime /= 100;

	timeVal = deltaTime;
	_vm->_global->inter_execPtr += 6;

	startIP = _vm->_global->inter_execPtr;
	WRITE_VAR(16, 0);
	var_22 = 0;
	index = 0;
	curEditIndex = 0;

	for (curCmd = 0; curCmd < count; curCmd++) {
		array[curCmd] = 0;
		cmd = *_vm->_global->inter_execPtr++;

		if ((cmd & 0x40) != 0) {
			cmd -= 0x40;
			cmdHigh = (byte)*_vm->_global->inter_execPtr;
			_vm->_global->inter_execPtr++;
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
			descArray[index].backColor = *_vm->_global->inter_execPtr++;
			descArray[index].frontColor = *_vm->_global->inter_execPtr++;

			if (cmd < 5 || cmd > 8) {
				descArray[index].ptr = 0;
			} else {
				descArray[index].ptr = _vm->_global->inter_execPtr + 2;
				_vm->_global->inter_execPtr += _vm->_inter->load16();
			}

			if (left == -1)
				break;

			if ((cmd & 1) == 0) {
				addNewCollision(curCmd + 0x8000, left,
				    top,
				    left +
				    width *
				    _vm->_draw->fonts[descArray[index].fontIndex]->
				    itemWidth - 1, top + height - 1, cmd, key,
				    0,
				    _vm->_global->inter_execPtr - (char *)totFileData);

				_vm->_global->inter_execPtr += 2;
				_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr);
			} else {
				addNewCollision(curCmd + 0x8000, left,
				    top,
				    left +
				    width *
				    _vm->_draw->fonts[descArray[index].fontIndex]->
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
			    _vm->_global->inter_execPtr - (char *)totFileData, 0);

			_vm->_global->inter_execPtr += 2;
			_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr);
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
			    _vm->_global->inter_execPtr - (char *)totFileData);

			_vm->_global->inter_execPtr += 2;
			_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr);
			break;

		case 0:
			_vm->_global->inter_execPtr += 6;
			startIP = _vm->_global->inter_execPtr;
			_vm->_global->inter_execPtr += 2;
			_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr);
			key = curCmd + 0xA000;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    cmd + cmdHigh, key,
			    startIP - (char *)totFileData,
			    _vm->_global->inter_execPtr - (char *)totFileData);

			_vm->_global->inter_execPtr += 2;
			_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr);
			break;

		case 1:
			key = _vm->_inter->load16();
			array[curCmd] = _vm->_inter->load16();
			flags = _vm->_inter->load16() & 3;

			startIP = _vm->_global->inter_execPtr;
			_vm->_global->inter_execPtr += 2;
			_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr);
			if (key == 0)
				key = curCmd + 0xa000;

			addNewCollision(curCmd + 0x8000, left, top,
			    left + width - 1,
			    top + height - 1,
			    (flags << 4) + cmd + cmdHigh, key,
			    startIP - (char *)totFileData,
			    _vm->_global->inter_execPtr - (char *)totFileData);

			_vm->_global->inter_execPtr += 2;
			_vm->_global->inter_execPtr += READ_LE_UINT16(_vm->_global->inter_execPtr);
			break;
		}
	}

	forceHandleMouse = 0;
	_vm->_util->waitKey();

	do {
		if (var_22 != 0) {
			key =
			    multiEdit(deltaTime, index, &curEditIndex,
			    descArray);

			if (key == 0x1c0d) {
				for (i = 0; i < 250; i++) {
					if (collisionAreas[i].left == -1)
						continue;

					if ((collisionAreas[i].id & 0x8000) == 0)
						continue;

					if ((collisionAreas[i].flags & 1) != 0)
						continue;

					if ((collisionAreas[i].flags & 0x0f) <= 2)
						continue;

					collResId = collisionAreas[i].id;
					activeCollResId = collResId;
					collResId &= 0x7fff;
					activeCollIndex = i;
					break;
				}
				break;
			}
		} else {
			key =
			    checkCollisions(ghandleMouse, -deltaTime,
			    &activeCollResId, &activeCollIndex);
		}

		if ((key & 0xff) >= ' ' && (key & 0xff) <= 0xff &&
		    (key >> 8) > 1 && (key >> 8) < 12) {
			key = '0' + (((key >> 8) - 1) % 10) + (key & 0xff00);
		}

		if (activeCollResId == 0) {
			if (key != 0) {
				for (i = 0; i < 250; i++) {
					if (collisionAreas[i].left == -1)
						continue;

					if ((collisionAreas[i].
						id & 0x8000) == 0)
						continue;

					if (collisionAreas[i].key == key
					    || collisionAreas[i].key ==
					    0x7fff) {

						activeCollResId =
						    collisionAreas[i].id;
						activeCollIndex = i;
						break;
					}
				}

				if (activeCollResId == 0) {
					for (i = 0; i < 250; i++) {
						if (collisionAreas[i].left == -1)
							continue;

						if ((collisionAreas[i].id & 0x8000) == 0)
							continue;

						if ((collisionAreas[i].key & 0xff00) != 0)
							continue;

						if (collisionAreas[i].key == 0)
							continue;

						if (adjustKey(key & 0xff) == adjustKey(collisionAreas[i].key) || collisionAreas[i].key == 0x7fff) {
							activeCollResId = collisionAreas[i].id;
							activeCollIndex = i;
							break;
						}
					}
				}
			} else {

				if (deltaTime != 0 && VAR(16) == 0) {
					if (stackPos2 != 0) {
						collStackPos = 0;
						collPtr = collisionAreas;

						for (i = 0, collPtr = collisionAreas; collPtr->left != -1; i++, collPtr++) {
							if ((collPtr->id & 0x8000) == 0)
								continue;

							collStackPos++;
							if (collStackPos != stackPos2)
								continue;

							activeCollResId = collPtr->id;
							activeCollIndex = i;
							WRITE_VAR(2, _vm->_global->inter_mouseX);
							WRITE_VAR(3, _vm->_global->inter_mouseY);
							WRITE_VAR(4, mouseButtons);
							WRITE_VAR(16, array[(uint16)activeCollResId & ~0x8000]);

							if (collPtr->funcLeave != 0) {
								timeKey = _vm->_util->getTimeKey();
								savedIP = _vm->_global->inter_execPtr;
								_vm->_global->inter_execPtr = (char *)totFileData + collPtr->funcLeave;
								shouldPushColls = 1;
								savedCollStackSize = collStackSize;
								_vm->_inter->funcBlock(0);

								if (savedCollStackSize != collStackSize)
									popCollisions();

								shouldPushColls = 0;
								_vm->_global->inter_execPtr = savedIP;
								deltaTime = timeVal - (_vm->_util->getTimeKey() - timeKey);

								if (deltaTime < 2)
									deltaTime = 2;
							}

							if (VAR(16) == 0)
								activeCollResId = 0;
							break;
						}
					} else {
						if (descIndex != 0) {
							counter = 0;
							for (i = 0; i < 250; i++) {
								if (collisionAreas[i].left == -1)
									continue;

								if ((collisionAreas[i].id & 0x8000) == 0)
									continue;

								counter++;
								if (counter != descIndex)
									continue;

								activeCollResId = collisionAreas[i].id;
								activeCollIndex = i;
								break;
							}
						} else {
							for (i = 0; i < 250; i++) {
								if (collisionAreas[i].left == -1)
									continue;

								if ((collisionAreas[i].id & 0x8000) == 0)
									continue;

								activeCollResId = collisionAreas[i].id;
								activeCollIndex = i;
								break;
							}
						}
					}
				} else {
					if (descIndex2 != 0) {
						counter = 0;
						for (i = 0; i < 250; i++) {
							if (collisionAreas[i].left == -1)
								continue;

							if ((collisionAreas[i].id & 0x8000) == 0)
								continue;

							counter++;
							if (counter != descIndex2)
								continue;

							activeCollResId = collisionAreas[i].id;
							activeCollIndex = i;
							break;
						}
					}
				}
			}
		}

		if (activeCollResId == 0)
			continue;

		if (collisionAreas[activeCollIndex].funcLeave != 0)
			continue;

		WRITE_VAR(2, _vm->_global->inter_mouseX);
		WRITE_VAR(3, _vm->_global->inter_mouseY);
		WRITE_VAR(4, mouseButtons);
		WRITE_VAR(16, array[(uint16)activeCollResId & ~0x8000]);

		if (collisionAreas[activeCollIndex].funcEnter != 0) {
			savedIP = _vm->_global->inter_execPtr;
			_vm->_global->inter_execPtr = (char *)totFileData +
			    collisionAreas[activeCollIndex].
			    funcEnter;

			shouldPushColls = 1;

			collStackPos = collStackSize;
			_vm->_inter->funcBlock(0);
			if (collStackPos != collStackSize)
				popCollisions();
			shouldPushColls = 0;
			_vm->_global->inter_execPtr = savedIP;
		}

		WRITE_VAR(16, 0);
		activeCollResId = 0;
	}
	while (activeCollResId == 0 && _vm->_inter->terminate == 0);

	if (((uint16)activeCollResId & ~0x8000) == collResId) {
		collStackPos = 0;
		var_24 = 0;
		var_26 = 1;
		for (i = 0; i < 250; i++) {
			if (collisionAreas[i].left == -1)
				continue;

			if ((collisionAreas[i].id & 0x8000) == 0)
				continue;

			if ((collisionAreas[i].flags & 0x0f) < 3)
				continue;

			if ((collisionAreas[i].flags & 0x0f) > 10)
				continue;

			if ((collisionAreas[i].flags & 0x0f) > 8) {
				strcpy(tempStr,
				    _vm->_global->inter_variables + collisionAreas[i].key);
				while ((pos =
					_vm->_util->strstr(" ", tempStr)) != 0) {
					_vm->_util->cutFromStr(tempStr, pos - 1, 1);
					pos = _vm->_util->strstr(" ", tempStr);
				}
				strcpy(_vm->_global->inter_variables + collisionAreas[i].key, tempStr);
			}

			if ((collisionAreas[i].flags & 0x0f) >= 5 &&
			    (collisionAreas[i].flags & 0x0f) <= 8) {
				str = descArray[var_24].ptr;

				strcpy(tempStr, _vm->_global->inter_variables + collisionAreas[i].key);

				if ((collisionAreas[i].flags & 0x0f) < 7)
					_vm->_util->prepareStr(tempStr);

				pos = 0;
				do {
					strcpy(collStr, str);
					pos += strlen(str) + 1;

					str += strlen(str) + 1;

					if ((collisionAreas[i].flags & 0x0f) < 7)
						_vm->_util->prepareStr(collStr);

					if (strcmp(tempStr, collStr) == 0) {
						VAR(17)++;
						WRITE_VAR(17 + var_26, 1);
						break;
					}
				} while (READ_LE_UINT16(descArray[var_24].ptr - 2) > pos);
				collStackPos++;
			} else {
				VAR(17 + var_26) = 2;
			}
			var_24++;
			var_26++;
		}

		if (collStackPos != (int16)VAR(17))
			WRITE_VAR(17, 0);
		else
			WRITE_VAR(17, 1);
	}

	savedIP = 0;
	if (_vm->_inter->terminate == 0) {
		savedIP = (char *)totFileData +
		    collisionAreas[activeCollIndex].funcLeave;

		WRITE_VAR(2, _vm->_global->inter_mouseX);
		WRITE_VAR(3, _vm->_global->inter_mouseY);
		WRITE_VAR(4, mouseButtons);

		if (VAR(16) == 0) {
			WRITE_VAR(16, array[(uint16)activeCollResId & ~0x8000]);
		}
	}

	for (curCmd = 0; curCmd < count; curCmd++) {
		freeCollision(curCmd + 0x8000);
	}
	_vm->_global->inter_execPtr = savedIP;
}

void Game::prepareStart(void) {
	int16 i;

	clearCollisions();

	_vm->_global->pPaletteDesc->unused2 = _vm->_draw->unusedPalette2;
	_vm->_global->pPaletteDesc->unused1 = _vm->_draw->unusedPalette1;
	_vm->_global->pPaletteDesc->vgaPal = _vm->_draw->vgaPalette;

	_vm->_video->setFullPalette(_vm->_global->pPaletteDesc);

	_vm->_draw->backSurface = _vm->_video->initSurfDesc(_vm->_global->videoMode, 320, 200, 0);

	_vm->_video->fillRect(_vm->_draw->backSurface, 0, 0, 319, 199, 1);
	_vm->_draw->frontSurface = _vm->_global->pPrimarySurfDesc;
	_vm->_video->fillRect(_vm->_draw->frontSurface, 0, 0, 319, 199, 1);

	_vm->_util->setMousePos(152, 92);

	_vm->_draw->cursorX = 152;
	_vm->_global->inter_mouseX = 152;

	_vm->_draw->cursorY = 92;
	_vm->_global->inter_mouseY = 92;
	_vm->_draw->invalidatedCount = 0;
	_vm->_draw->noInvalidated = 1;
	_vm->_draw->applyPal = 0;
	_vm->_draw->paletteCleared = 0;
	_vm->_draw->cursorWidth = 16;
	_vm->_draw->cursorHeight = 16;
	_vm->_draw->transparentCursor = 1;

	for (i = 0; i < 40; i++) {
		_vm->_draw->cursorAnimLow[i] = -1;
		_vm->_draw->cursorAnimDelays[i] = 0;
		_vm->_draw->cursorAnimHigh[i] = 0;
	}

	_vm->_draw->cursorAnimLow[1] = 0;
	_vm->_draw->cursorSprites = _vm->_video->initSurfDesc(_vm->_global->videoMode, 32, 16, 2);
	_vm->_draw->cursorBack = _vm->_video->initSurfDesc(_vm->_global->videoMode, 16, 16, 0);
	_vm->_draw->renderFlags = 0;
	_vm->_draw->backDeltaX = 0;
	_vm->_draw->backDeltaY = 0;

	startTimeKey = _vm->_util->getTimeKey();
}

void Game::loadTotFile(char *path) {
	int16 handle;

	handle = _vm->_dataio->openData(path);
	if (handle >= 0) {
		_vm->_dataio->closeData(handle);
		totFileData = _vm->_dataio->getData(path);
	} else {
		totFileData = 0;
	}
}

void Game::loadExtTable(void) {
	int16 count, i;

	// Function is correct. [sev]

	extHandle = _vm->_dataio->openData(curExtFile);
	if (extHandle < 0)
		return;

	_vm->_dataio->readData(extHandle, (char *)&count, 2);
	count = FROM_LE_16(count);

	_vm->_dataio->seekData(extHandle, 0, 0);
	extTable = (ExtTable *)malloc(sizeof(ExtTable)
	    + sizeof(ExtItem) * count);

	_vm->_dataio->readData(extHandle, (char *)&extTable->itemsCount, 2);
	extTable->itemsCount = FROM_LE_16(extTable->itemsCount);
	_vm->_dataio->readData(extHandle, (char *)&extTable->unknown, 1);

	for (i = 0; i < count; i++) {
		_vm->_dataio->readData(extHandle, (char *)&extTable->items[i].offset, 4);
		extTable->items[i].offset = FROM_LE_32(extTable->items[i].offset);
		_vm->_dataio->readData(extHandle, (char *)&extTable->items[i].size, 2);
		extTable->items[i].size = FROM_LE_16(extTable->items[i].size);
		_vm->_dataio->readData(extHandle, (char *)&extTable->items[i].width, 2);
		extTable->items[i].width = FROM_LE_16(extTable->items[i].width);
		_vm->_dataio->readData(extHandle, (char *)&extTable->items[i].height, 2);
		extTable->items[i].height = FROM_LE_16(extTable->items[i].height);
	}
}

void Game::loadImFile(void) {
	char path[20];
	int16 handle;

	if (totFileData[0x3d] != 0 && totFileData[0x3b] == 0)
		return;

	strcpy(path, "commun.im1");
	if (totFileData[0x3b] != 0)
		path[strlen(path) - 1] = '0' + totFileData[0x3b];

	handle = _vm->_dataio->openData(path);
	if (handle < 0)
		return;

	_vm->_dataio->closeData(handle);
	imFileData = _vm->_dataio->getData(path);
}

void Game::playTot(int16 skipPlay) {
	char savedTotName[20];
	int16 *oldCaptureCounter;
	int16 *oldBreakFrom;
	int16 *oldNestLevel;
	int16 captureCounter;
	int16 breakFrom;
	int16 nestLevel;
	char needTextFree;
	char needFreeResTable;
	char *curPtr;
	int32 variablesCount;
	char *filePtr;
	char *savedIP;
	int16 i;

	oldNestLevel = _vm->_inter->nestLevel;
	oldBreakFrom = _vm->_inter->breakFromLevel;
	oldCaptureCounter = _vm->_scenery->pCaptureCounter;
	savedIP = _vm->_global->inter_execPtr;

	_vm->_inter->nestLevel = &nestLevel;
	_vm->_inter->breakFromLevel = &breakFrom;
	_vm->_scenery->pCaptureCounter = &captureCounter;
	strcpy(savedTotName, curTotFile);

	if (skipPlay == 0) {
		while (1) {
			for (i = 0; i < 4; i++) {
				_vm->_draw->fontToSprite[i].sprite = -1;
				_vm->_draw->fontToSprite[i].base = -1;
				_vm->_draw->fontToSprite[i].width = -1;
				_vm->_draw->fontToSprite[i].height = -1;
			}

			_vm->_cdrom->stopPlaying();
			_vm->_draw->animateCursor(4);
			_vm->_inter->initControlVars();
			_vm->_mult->initAll();
			_vm->_mult->zeroMultData();

			for (i = 0; i < 20; i++)
				_vm->_draw->spritesArray[i] = 0;

			_vm->_draw->spritesArray[20] = _vm->_draw->frontSurface;
			_vm->_draw->spritesArray[21] = _vm->_draw->backSurface;
			_vm->_draw->spritesArray[23] = _vm->_draw->cursorSprites;

			for (i = 0; i < 20; i++)
				soundSamples[i] = 0;

			totTextData = 0;
			totResourceTable = 0;
			imFileData = 0;
			extTable = 0;
			extHandle = -1;

			needFreeResTable = 1;
			needTextFree = 1;

			totToLoad[0] = 0;

			if (curTotFile[0] == 0 && totFileData == 0)
				break;

			loadTotFile(curTotFile);
			if (totFileData == 0) {
				_vm->_draw->blitCursor();
				break;
			}

			strcpy(curImaFile, curTotFile);
			strcpy(curExtFile, curTotFile);

			curImaFile[strlen(curImaFile) - 4] = 0;
			strcat(curImaFile, ".ima");

			curExtFile[strlen(curExtFile) - 4] = 0;
			strcat(curExtFile, ".ext");

			debug(4, "IMA: %s", curImaFile);
			debug(4, "EXT: %s", curExtFile);

			filePtr = (char *)totFileData + 0x30;

			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = totFileData;
				totTextData =
				    (TotTextTable *) (curPtr +
				    READ_LE_UINT32((char *)totFileData + 0x30));

				totTextData->itemsCount = (int16)READ_LE_UINT16(&totTextData->itemsCount);

				for (i = 0; i < totTextData->itemsCount; ++i) {
					totTextData->items[i].offset = (int16)READ_LE_UINT16(&totTextData->items[i].offset);
					totTextData->items[i].size = (int16)READ_LE_UINT16(&totTextData->items[i].size);
				}

				needTextFree = 0;
			}

			filePtr = (char *)totFileData + 0x34;
			if (READ_LE_UINT32(filePtr) != (uint32)-1) {
				curPtr = totFileData;

				totResourceTable =
					(TotResTable *)(curPtr +
				    READ_LE_UINT32((char *)totFileData + 0x34));

				totResourceTable->itemsCount = (int16)READ_LE_UINT16(&totResourceTable->itemsCount);

				for (i = 0; i < totResourceTable->itemsCount; ++i) {
					totResourceTable->items[i].offset = (int32)READ_LE_UINT32(&totResourceTable->items[i].offset);
					totResourceTable->items[i].size = (int16)READ_LE_UINT16(&totResourceTable->items[i].size);
					totResourceTable->items[i].width = (int16)READ_LE_UINT16(&totResourceTable->items[i].width);
					totResourceTable->items[i].height = (int16)READ_LE_UINT16(&totResourceTable->items[i].height);
				}

				needFreeResTable = 0;
			}

			loadImFile();
			loadExtTable();

			_vm->_global->inter_animDataSize = READ_LE_UINT16((char *)totFileData + 0x38);
			if (_vm->_global->inter_variables == 0) {
				variablesCount = READ_LE_UINT32((char *)totFileData + 0x2c);
				_vm->_global->inter_variables = (char *)malloc(variablesCount * 4);
				for (i = 0; i < variablesCount; i++)
					WRITE_VAR(i, 0);
			}

			_vm->_global->inter_execPtr = (char *)totFileData;
			_vm->_global->inter_execPtr += READ_LE_UINT32((char *)totFileData + 0x64);

			_vm->_inter->renewTimeInVars();

			WRITE_VAR(13, _vm->_global->useMouse);
			WRITE_VAR(14, _vm->_global->soundFlags);
			WRITE_VAR(15, _vm->_global->videoMode);
			WRITE_VAR(16, _vm->_global->language);

			_vm->_inter->callSub(2);

			if (totToLoad[0] != 0)
				_vm->_inter->terminate = 0;

			variablesCount = READ_LE_UINT32((char *)totFileData + 0x2c);
			_vm->_draw->blitInvalidated();
			free(totFileData);
			totFileData = 0;

			if (needTextFree)
				free(totTextData);
			totTextData = 0;

			if (needFreeResTable)
				free(totResourceTable);
			totResourceTable = 0;

			free(imFileData);
			imFileData = 0;

			free(extTable);
			extTable = 0;

			if (extHandle >= 0)
				_vm->_dataio->closeData(extHandle);

			extHandle = -1;

			for (i = 0; i < *_vm->_scenery->pCaptureCounter; i++)
				capturePop(0);

			_vm->_mult->checkFreeMult();
			_vm->_mult->freeAll();

			for (i = 0; i < 20; i++) {
				if (_vm->_draw->spritesArray[i] != 0)
					_vm->_video->freeSurfDesc(_vm->_draw->spritesArray[i]);
				_vm->_draw->spritesArray[i] = 0;
			}
			_vm->_snd->stopSound(0);

			for (i = 0; i < 20; i++)
				freeSoundSlot(i);

			if (totToLoad[0] == 0)
				break;

			strcpy(curTotFile, totToLoad);
		}
	}

	strcpy(curTotFile, savedTotName);

	_vm->_inter->nestLevel = oldNestLevel;
	_vm->_inter->breakFromLevel = oldBreakFrom;
	_vm->_scenery->pCaptureCounter = oldCaptureCounter;
	_vm->_global->inter_execPtr = savedIP;
}

void Game::start(void) {
	collisionAreas = (Collision *)malloc(250 * sizeof(Collision));
	prepareStart();
	playTot(0);

	free(collisionAreas);

	_vm->_video->freeSurfDesc(_vm->_draw->cursorSprites);
	_vm->_video->freeSurfDesc(_vm->_draw->cursorBack);
	_vm->_video->freeSurfDesc(_vm->_draw->backSurface);
}

} // End of namespace Gob
