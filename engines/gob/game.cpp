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
#include "gob/palanim.h"

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
	int i;

	for (i = 0; i < 5; i++) {
		_collStack[i] = 0;
		_collStackElemSizes[i] = 0;
	}

	for (i = 0; i < 60; i++) {
		_soundSamples[i] = 0;
		_soundIds[i] = 0;
		_soundTypes[i] = 0;
	}

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

	_backupedCount = 0;
	_curBackupPos = 0;
	
	for (i = 0; i < 5; i++) {
		_cursorXDeltaArray[i] = 0;
		_cursorYDeltaArray[i] = 0;
		_totTextDataArray[i] = 0;
		_totFileDataArray[i] = 0;
		_totResourceTableArray[i] = 0;
		_extTableArray[i] = 0;
		_extHandleArray[i] = 0;
		_imFileDataArray[i] = 0;
		_variablesArray[i] = 0;
		_curTotFileArray[i][0] = 0;
	}

	_imdFile = 0;
	_curImdFile[0] = 0;
	_imdX = 0;
	_imdY = 0;
	_imdFrameDataSize = 0;
	_imdVidBufferSize = 0;
	_imdFrameData = 0;
	_imdVidBuffer = 0;

	warning("GOB2 Stub! _byte_2FC82, _byte_2FC83, _word_2FC80");
	_byte_2FC82 = 0;
	_byte_2FC83 = 0;
	_word_2FC80 = 0;

	warning("GOB2 Stub! _byte_2FC9B, _word_2FC9C, _word_2FC9E, _word_2E51F, _off_2E51B, _off_2E517, _dword_2F2B6");
	_byte_2FC9B = 0;
	_word_2FC9C = 0;
	_word_2FC9E = 0;
	_word_2E51F = 0;
	_off_2E51B = 0;
	_off_2E517 = 0;
	_dword_2F2B6 = 0;
}

Game::~Game() {
	if (_imdFile) {
		if (_imdFile->palette)
			delete[] _imdFile->palette;
		if (_imdFile->surfDesc &&
				(_imdFile->surfDesc != _vm->_draw->_spritesArray[20]) &&
				(_imdFile->surfDesc != _vm->_draw->_spritesArray[21]))
			_vm->_video->freeSurfDesc(_imdFile->surfDesc);
		if (_imdFile->framesPos)
			delete[] _imdFile->framesPos;
		if (_imdFile->frameCoords)
			delete[] _imdFile->frameCoords;
		delete _imdFile;
	}
	if (_imdFrameData)
		delete[] _imdFrameData;
	if (_imdVidBuffer)
		delete[] _imdVidBuffer;
	if (_word_2FC80)
		delete[] _word_2FC80;
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
		debugC(7, DEBUG_FILEIO, "loadExtData(%d, %d, %d)", itemId, *pResWidth, *pResHeight);
	}

	debugC(7, DEBUG_FILEIO, "loadExtData(%d, 0, 0)", itemId);

	if (item->height == 0)
		size += (item->width & 0x7fff) << 16;

	debugC(7, DEBUG_FILEIO, "size: %d off: %d", size, offset);
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

	debugC(7, DEBUG_FILEIO, "off: %d size: %d", offset, tableSize);
	_vm->_dataio->seekData(handle, offset + tableSize, SEEK_SET);
	// CHECKME: is the below correct?
	if (isPacked)
		dataBuf = new char[size];
	else
		dataBuf = new char[size];

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
		dataBuf = new char[READ_LE_UINT32(packedBuf)];
		_vm->_pack->unpackData(packedBuf, dataBuf);
		delete[] packedBuf;
	}

	return dataBuf;

}

void Game::freeCollision(int16 id) {
	int16 i;

	for (i = 0; i < 250; i++) {
		if (_collisionAreas[i].id == id)
			_collisionAreas[i].left = -1;
	}
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

	soundDesc = new Snd::SoundDesc;

	_soundSamples[slot] = soundDesc;

	soundDesc->frequency = (dataPtr[4] << 8) + dataPtr[5];
	soundDesc->size = (dataPtr[1] << 16) + (dataPtr[2] << 8) + dataPtr[3];
	soundDesc->data = dataPtr + 6;
	soundDesc->timerTicks = (int32)1193180 / (int32)soundDesc->frequency;

	soundDesc->inClocks = (soundDesc->frequency * 10) / 182;
	soundDesc->flag = 0;
}

void Game::freeSoundSlot(int16 slot) {
	if (slot == -1)
		slot = _vm->_parse->parseValExpr();

	if ((slot < 0) || (slot >= 60) || (_soundSamples[slot] == 0))
		return;

	char* data = _soundSamples[slot]->data;

	_vm->_snd->freeSoundDesc(_soundSamples[slot], false);
	_soundSamples[slot] = 0;

	if (_soundFromExt[slot] == 1) {
		delete[] (data - 6);
		_soundFromExt[slot] = 0;
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

		debugC(1, DEBUG_COLLISIONS, "collisionsBlock(%d)", cmd);

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
	while (_activeCollResId == 0 && !_vm->_inter->_terminate && !_vm->_quitRequested);

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
				char *ptr;
				strcpy(_tempStr,
				    _vm->_global->_inter_variables + _collisionAreas[i].key);
				while ((ptr = strchr(_tempStr, ' ')) != 0) {
					_vm->_util->cutFromStr(_tempStr, (ptr - _tempStr), 1);
					ptr = strchr(_tempStr, ' ');
				}
				strcpy(_vm->_global->_inter_variables + _collisionAreas[i].key, _tempStr);
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
	_extTable = new ExtTable;
	_extTable->items = 0;
	if (count)
		_extTable->items = new ExtItem[count];

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

void Game::start(void) {
	_collisionAreas = new Collision[250];
	prepareStart();
	playTot(0);

	delete[] _collisionAreas;

	_vm->_video->freeSurfDesc(_vm->_draw->_cursorSprites);
	_vm->_video->freeSurfDesc(_vm->_draw->_cursorBack);
	_vm->_video->freeSurfDesc(_vm->_draw->_backSurface);
}

// flagbits: 0 = freeInterVariables, 1 = skipPlay
void Game::totSub(int8 flags, char *newTotFile) {
	int8 curBackupPos;

	if (_backupedCount >= 5)
		return;

	_cursorXDeltaArray[_backupedCount] = _vm->_draw->_cursorXDeltaVar;
	_cursorYDeltaArray[_backupedCount] = _vm->_draw->_cursorYDeltaVar;
	_totTextDataArray[_backupedCount] = _totTextData;
	_totFileDataArray[_backupedCount] = _totFileData;
	_totResourceTableArray[_backupedCount] = _totResourceTable;
	_extTableArray[_backupedCount] = _extTable;
	_extHandleArray[_backupedCount] = _extHandle;
	_imFileDataArray[_backupedCount] = _imFileData;
	_variablesArray[_backupedCount] = _vm->_global->_inter_variables;
	strcpy(_curTotFileArray[_backupedCount], _curTotFile);

	curBackupPos = _curBackupPos;
	_backupedCount++;
	_curBackupPos = _backupedCount;

	_totTextData = 0;
	_totFileData = 0;
	_totResourceTable = 0;
	if (flags & 1)
		_vm->_global->_inter_variables = 0;

	strcpy(_curTotFile, newTotFile);
	strcat(_curTotFile, ".TOT");

	if (_vm->_inter->_terminate != 0)
		return;

	pushCollisions(0);

	if (flags & 2)
		playTot(-1);
	else
		playTot(0);

	if (_vm->_inter->_terminate != 2)
		_vm->_inter->_terminate = 0;

	popCollisions();

	if ((flags & 1) && (_vm->_global->_inter_variables != 0))
		delete[] _vm->_global->_inter_variables;

	_backupedCount--;
	_curBackupPos = curBackupPos;

	_vm->_draw->_cursorXDeltaVar = _cursorXDeltaArray[_backupedCount];
	_vm->_draw->_cursorYDeltaVar = _cursorYDeltaArray[_backupedCount];
	_totTextData = _totTextDataArray[_backupedCount];
	_totFileData = _totFileDataArray[_backupedCount];
	_totResourceTable = _totResourceTableArray[_backupedCount];
	_extTable = _extTableArray[_backupedCount];
	_extHandle = _extHandleArray[_backupedCount];
	_imFileData = _imFileDataArray[_backupedCount];
	_vm->_global->_inter_variables = _variablesArray[_backupedCount];
	strcpy(_curTotFile, _curTotFileArray[_backupedCount]);
	strcpy(_curExtFile, _curTotFile);
	_curExtFile[strlen(_curExtFile)-4] = '\0';
	strcat(_curExtFile, ".EXT");
}

void Game::switchTotSub(int16 index, int16 skipPlay) {
	int16 backupedCount;
	int16 curBackupPos;

	if ((_backupedCount - index) < 1)
		return;

	curBackupPos = _curBackupPos;
	backupedCount = _backupedCount;
	if (_curBackupPos == _backupedCount) {
		_cursorXDeltaArray[_backupedCount] = _vm->_draw->_cursorXDeltaVar;
		_cursorYDeltaArray[_backupedCount] = _vm->_draw->_cursorYDeltaVar;
		_totTextDataArray[_backupedCount] = _totTextData;
		_totFileDataArray[_backupedCount] = _totFileData;
		_totResourceTableArray[_backupedCount] = _totResourceTable;
		_extTableArray[_backupedCount] = _extTable;
		_extHandleArray[_backupedCount] = _extHandle;
		_imFileDataArray[_backupedCount] = _imFileData;
		_variablesArray[_backupedCount] = _vm->_global->_inter_variables;
		strcpy(_curTotFileArray[_backupedCount], _curTotFile);
		_backupedCount++;
	}
	_curBackupPos -= index;
	if (index >= 0)
		_curBackupPos--;

	_vm->_draw->_cursorXDeltaVar = _cursorXDeltaArray[_curBackupPos];
	_vm->_draw->_cursorYDeltaVar = _cursorYDeltaArray[_curBackupPos];
	_totTextData = _totTextDataArray[_curBackupPos];
	_totFileData = _totFileDataArray[_curBackupPos];
	_totResourceTable = _totResourceTableArray[_curBackupPos];
	_extTable = _extTableArray[_curBackupPos];
	_extHandle = _extHandleArray[_curBackupPos];
	_imFileData = _imFileDataArray[_curBackupPos];
	_vm->_global->_inter_variables = _variablesArray[_curBackupPos];
	strcpy(_curTotFile, _curTotFileArray[_curBackupPos]);
	strcpy(_curExtFile, _curTotFile);
	_curExtFile[strlen(_curExtFile)-4] = '\0';
	strcat(_curExtFile, ".EXT");

	if (_vm->_inter->_terminate != 0)
		return;

	_vm->_game->pushCollisions(0);
	_vm->_game->playTot(skipPlay);

	if (_vm->_inter->_terminate != 2)
		_vm->_inter->_terminate = 0;

	_vm->_game->popCollisions();

	_curBackupPos = curBackupPos;
	_backupedCount = backupedCount;
	_vm->_draw->_cursorXDeltaVar = _cursorXDeltaArray[_curBackupPos];
	_vm->_draw->_cursorYDeltaVar = _cursorYDeltaArray[_curBackupPos];
	_totTextData = _totTextDataArray[_curBackupPos];
	_totFileData = _totFileDataArray[_curBackupPos];
	_totResourceTable = _totResourceTableArray[_curBackupPos];
	_extTable = _extTableArray[_curBackupPos];
	_extHandle = _extHandleArray[_curBackupPos];
	_imFileData = _imFileDataArray[_curBackupPos];
	_vm->_global->_inter_variables = _variablesArray[_curBackupPos];
	strcpy(_curTotFile, _curTotFileArray[_curBackupPos]);
	strcpy(_curExtFile, _curTotFile);
	_curExtFile[strlen(_curExtFile)-4] = '\0';
	strcat(_curExtFile, ".EXT");
}

int16 Game::openLocTextFile(char *locTextFile, int language) {
	int n;

	n = strlen(locTextFile);
	if (n < 4)
		return -1;

	locTextFile[n - 4] = 0;
	switch (language) {
	case 0:
		strcat(locTextFile, ".dat");
		break;
	case 1:
		strcat(locTextFile, ".all");
		break;
	case 3:
		strcat(locTextFile, ".esp");
		break;
	case 4:
		strcat(locTextFile, ".ita");
		break;
	case 5:
		strcat(locTextFile, ".usa");
		break;
	case 6:
		strcat(locTextFile, ".ndl");
		break;
	case 7:
		strcat(locTextFile, ".kor");
		break;
	case 8:
		strcat(locTextFile, ".isr");
		break;
	default:
		strcat(locTextFile, ".ang");
		break;
	}
	return _vm->_dataio->openData(locTextFile);
}

char *Game::loadLocTexts(void) {
	char locTextFile[20];
	int16 handle;
	int i;

	strcpy(locTextFile, _curTotFile);

	handle = openLocTextFile(locTextFile, _vm->_global->_language);
	if ((handle < 0) && !scumm_stricmp(_vm->_game->_curTotFile, "intro0.tot")) {
		warning("Your game version doesn't support the requested language, using the first one available");
		for (i = 0; i < 10; i++) {
			handle = openLocTextFile(locTextFile, i);
			if (handle >= 0)
				_vm->_global->_language = i;
		}
	}

	if (handle >= 0) {
		_vm->_dataio->closeData(handle);
		return _vm->_dataio->getData(locTextFile);
	}
	return 0;
}

void Game::setCollisions(void) {
	char *savedIP;
	int16 left;
	int16 top;
	int16 width;
	int16 height;
	Collision *collArea;

	for (collArea = _collisionAreas; collArea->left != -1; collArea++) {
		if (((collArea->id & 0xC000) != 0x8000) || (collArea->field_12 == 0))
			continue;

		savedIP = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr = _totFileData + collArea->field_12;
		left = _vm->_parse->parseValExpr();
		top = _vm->_parse->parseValExpr();
		width = _vm->_parse->parseValExpr();
		height = _vm->_parse->parseValExpr();
		if ((_vm->_draw->_renderFlags != 8) && (left != -1)) {
			left += _vm->_draw->_backDeltaX;
			top += _vm->_draw->_backDeltaY;
		}
		if (_vm->_draw->_word_2E8E2 != 2) {
			_vm->_draw->adjustCoords(0, &left, &top);
			if ((collArea->flags & 0x0F) < 3)
				_vm->_draw->adjustCoords(2, &width, &height);
			else {
				height &= 0xFFFE;
				_vm->_draw->adjustCoords(2, 0, &height);
			}
		}
		collArea->left = left;
		collArea->top = top;
		collArea->right = left + width - 1;
		collArea->bottom = top + height - 1;
		_vm->_global->_inter_execPtr = savedIP;
	}
}

void Game::collSub(int16 offset) {
	char *savedIP;
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
	setCollisions();
}

void Game::collAreaSub(int16 index, int8 enter) {
	uint16 collId;

	collId = _collisionAreas[index].id & 0xF000;

	if ((collId == 0xA000) || (collId == 0x9000))
		WRITE_VAR(17, collId);
	else if (enter == 0)
		WRITE_VAR(17, -(collId & 0x0FFF));
	else
		WRITE_VAR(17, collId & 0x0FFF);
	
	if (enter != 0) {
		if (_collisionAreas[index].funcEnter != 0)
			collSub(_collisionAreas[index].funcEnter);
	} else {
		if (_collisionAreas[index].funcLeave != 0)
			collSub(_collisionAreas[index].funcLeave);
	}
}

// "DEVinitscreen"
void Game::sub_ADD2(void) {
	_word_2FC9C = 0;
	_word_2FC9E = 0;

	if (_word_2E51F != 0) {
		_off_2E51B = new Video::SurfaceDesc;
		memcpy(_off_2E51B, _vm->_draw->_frontSurface, sizeof(Video::SurfaceDesc));
		_off_2E51B->height = _vm->_global->_primaryHeight - _word_2E51F;
		_vm->_draw->_frontSurface->height -= _off_2E51B->height;
		_vm->_draw->_frontSurface->vidPtr =
			_off_2E51B->vidPtr + ((_off_2E51B->width * _off_2E51B->height) / 4);

		_off_2E517 = new Video::SurfaceDesc;
		memcpy(_off_2E517, _off_2E51B, sizeof(Video::SurfaceDesc));
		_off_2E517->width = _vm->_global->_primaryWidth;
		_off_2E517->vidPtr = _vm->_draw->_frontSurface->vidPtr +
			((_vm->_draw->_frontSurface->width * _vm->_draw->_frontSurface->height ) / 4);
	}
	_vm->_draw->initBigSprite(21, 320, 200, 0);
	_vm->_draw->_backSurface = _vm->_draw->_spritesArray[21];
	_vm->_video->clearSurf(_vm->_draw->_backSurface);
	
	_vm->_draw->initBigSprite(23, 32, 16, 2);
	// TODO: That assignment is not in the original assembly, why?
	_vm->_draw->_cursorSpritesBack = _vm->_draw->_spritesArray[23];
	_vm->_draw->_cursorSprites = _vm->_draw->_cursorSpritesBack;
	_vm->_draw->_cursorBack = _vm->_video->initSurfDesc(_vm->_global->_videoMode, 16, 16, 0);

	_vm->_draw->_spritesArray[20] = _vm->_draw->_frontSurface;
	_vm->_draw->_spritesArray[21] = _vm->_draw->_backSurface;

/*	if (_word_2E51F != 0) {
		dword_2F92D = _off_2E51B;
		dword_2F931 = _off_2E517;
	}*/
}

// "DEVclosescreen"
void Game::sub_BB28(void) {
	_vm->_draw->freeSprite(23);
	_vm->_video->freeSurfDesc(_vm->_draw->_cursorBack);
	if (_off_2E51B != 0) {
		memcpy(_vm->_draw->_frontSurface, _off_2E51B, sizeof(Video::SurfaceDesc));
		_vm->_draw->_frontSurface->width = 320;
		_vm->_draw->_frontSurface->height = 200;
		delete _off_2E51B;
		delete _off_2E517;
		_off_2E51B = 0;
		_off_2E517 = 0;
	}
	if (_vm->_draw->_frontSurface != _vm->_draw->_backSurface)
		_vm->_draw->freeSprite(21);
}

Snd::SoundDesc *Game::loadSND(const char *path, int8 arg_4) {
	Snd::SoundDesc *soundDesc;
	int32 dsize;
	char *data;
	char *dataPtr;

	soundDesc = new Snd::SoundDesc;

	data = _vm->_dataio->getData(path);
	if (data == 0) {
		delete soundDesc;
		return 0;
	}
	soundDesc->data = data;
	soundDesc->flag = *data & 0x7F;
	if (*data == 0)
		soundDesc->flag = 8;
	dataPtr = data + 4;

	WRITE_LE_UINT16(dataPtr, READ_BE_UINT16(dataPtr));

	WRITE_LE_UINT32(data, (READ_LE_UINT32(data) >> 24) + ((READ_LE_UINT16(data) & 0xFF00) << 8) + ((READ_LE_UINT16(data + 2) & 0xFF) >> 8));

	soundDesc->size = READ_LE_UINT32(data);
	dsize = _vm->_dataio->getDataSize(path) - 6;
	if (dsize > soundDesc->size)
		soundDesc->size = dsize;

	soundDesc->frequency = READ_LE_UINT16(dataPtr);
	soundDesc->data += 6;
	soundDesc->timerTicks = 1193180 / READ_LE_UINT16(dataPtr);

	if (arg_4 & 2)
		arg_4 |= 1;
	if ((soundDesc->frequency < 4700) && (arg_4 & 1))
		arg_4 &= 0xFE;

	if (arg_4 & 1) {
		if ((_vm->_global->_soundFlags & BLASTER_FLAG) || (_vm->_global->_soundFlags & PROAUDIO_FLAG)) {
		}
	}

	return soundDesc;
}

int8 Game::openImd(const char *path, int16 x, int16 y, int16 repeat, int16 flags) {
	int i;
	int j;
	const char *src;
	byte *vidMem;
	Video::SurfaceDesc *surfDesc;

	if (path[0] != 0) {
		if (_imdFile == 0)
			_curImdFile[0] = 0;

		src = strrchr(path, '\\');
		src = src == 0 ? path : src+1;

		if (strcmp(_curImdFile, src) != 0) {
			closeImd();
			_imdFile = loadImdFile(path, 0, 2);
			if (_imdFile == 0)
				return 0;

			_imdX = _imdFile->x;
			_imdY = _imdFile->y;
			strcpy(_curImdFile, src);
			_imdFrameData = new byte[_imdFrameDataSize + 1000];
			_imdVidBuffer = new byte[_imdVidBufferSize + 1000];
			memset(_imdFrameData, 0, _imdFrameDataSize + 1000);
			memset(_imdVidBuffer, 0, _imdVidBufferSize + 1000);

			if (_vm->_video->_extraMode) {
				_byte_2FC83 = (flags & 0x80) ? 1 : 0;
				if (!(_imdFile->field_E & 0x100) || (_imdFile->field_E & 0x2000)) {
					setImdXY(_imdFile, 0, 0);
					_imdFile->surfDesc =
						_vm->_video->initSurfDesc(0x13, _imdFile->width, _imdFile->height, 0);
				} else {
					if (_byte_2FC82 == 0)
						_imdFile->surfDesc = _vm->_draw->_spritesArray[21];
					else
						_imdFile->surfDesc = _vm->_draw->_spritesArray[20];
					if ((x != -1) || (y != -1)) {
						_imdX = x != -1 ? x : _imdX;
						_imdY = y != -1 ? y : _imdY;
						setImdXY(_imdFile, _imdX, _imdY);
					}
				}
				if (flags & 0x40) {
					_imdX = x != -1 ? x : _imdX;
					_imdY = y != -1 ? y : _imdY;
					if (_vm->_video->_extraMode && ((_imdFile->surfDesc->vidMode & 0x7F) == 0x13)) {
						surfDesc = _vm->_video->initSurfDesc(0x13, _imdFile->width, _imdFile->height, 0);
						_vm->_video->drawSprite(_vm->_draw->_spritesArray[21], surfDesc, _imdX, _imdY,
								_imdX + _imdFile->width - 1, _imdY + _imdFile->height - 1, 0, 0, 0);
						vidMem = _imdFile->surfDesc->vidPtr;
						for (i = 0; i < _imdFile->height; i++)
							for (j = 0; j < _imdFile->width; j++, vidMem++) {
								*(vidMem) = *(surfDesc->vidPtr
										+ (j / 4)
										+ (surfDesc->width / 4 * i)
										+ (surfDesc->reserved2 * (j & 3)));
							}
						_vm->_video->freeSurfDesc(surfDesc);
					}
				}
			} else {
				if ((x != -1) || (y != -1)) {
					_imdX = x != -1 ? x : _imdX;
					_imdY = y != -1 ? y : _imdY;
					setImdXY(_imdFile, _imdX, _imdY);
				}
				_byte_2FC83 = (flags & 0x80) ? 1 : 0;
				if (_byte_2FC83 == 0)
					_imdFile->surfDesc = _vm->_draw->_spritesArray[21];
				else
					_imdFile->surfDesc = _vm->_draw->_spritesArray[20];
			}
		}
	}

	if (_imdFile == 0)
		return 0;

	if (repeat == -1) {
		closeImd();
		return 0;
	}

	_imdX = x != -1 ? x : _imdX;
	_imdY = y != -1 ? y : _imdY;

	WRITE_VAR(7, _imdFile->framesCount);

	return 1;
}

void Game::closeImd(void) {
	if (_imdFile == 0)
		return;

	if ((_imdFile->surfDesc != _vm->_draw->_spritesArray[20]) &&
			(_imdFile->surfDesc != _vm->_draw->_spritesArray[21]))
		_vm->_video->freeSurfDesc(_imdFile->surfDesc);

	finishImd(_imdFile);

	delete[] _imdFrameData;
	delete[] _imdVidBuffer;
	_imdFrameData = 0;
	_imdVidBuffer = 0;

	_imdFile = 0;
}

void Game::finishImd(Game::Imd *imdPtr) {
	if (imdPtr == 0)
		return;

/*
	if (dword_31345 != 0) {
		_vm->_sound->stopSound(0);
		dword_31345 = 0;
		delete off_31461;
		byte_31344 = 0;
	}
*/

	_vm->_dataio->closeData(imdPtr->fileHandle);

	if (imdPtr->frameCoords != 0)
		delete[] imdPtr->frameCoords;
	if (imdPtr->palette != 0)
		delete[] imdPtr->palette;
	if (imdPtr->framesPos != 0)
		delete[] imdPtr->framesPos;

	delete imdPtr;
	imdPtr = 0;
}

// flagsBit: 0 = read and set palette
//           1 = read palette
Game::Imd *Game::loadImdFile(const char *path, Video::SurfaceDesc *surfDesc, int8 flags) {
	int i;
	Imd *imdPtr;
	int16 handle;
	int16 setAllPalBak;
	char buf[18];
	Video::Color *palBak;

	int32 byte_31449 = 0;
	int32 byte_3144D = 0;

	buf[0] = 0;
	strcpy(buf, path);
	strcat(buf, ".IMD");

	handle = _vm->_dataio->openData(buf);

	if (handle < 0) {
		warning("Can't open IMD \"%s\"", buf);
		return 0;
	}

	imdPtr = new Imd;
	memset(imdPtr, 0, sizeof(Imd));

	imdPtr->palette = 0;

	_vm->_dataio->readData(handle, buf, 18);

	// "fileHandle" holds the major version while loading
	imdPtr->fileHandle = READ_LE_UINT16(buf);
	imdPtr->verMin = READ_LE_UINT16(buf + 2);
	imdPtr->framesCount = READ_LE_UINT16(buf + 4);
	imdPtr->x = READ_LE_UINT16(buf + 6);
	imdPtr->y = READ_LE_UINT16(buf + 8);
	imdPtr->width = READ_LE_UINT16(buf + 10);
	imdPtr->height = READ_LE_UINT16(buf + 12);
	imdPtr->field_E = READ_LE_UINT16(buf + 14);
	imdPtr->curFrame = READ_LE_UINT16(buf + 16);

	if (imdPtr->fileHandle != 0)
		imdPtr->verMin = 0;

	if ((imdPtr->verMin & 0xFF) < 2) {
		warning("IMD version incorrect (%d,%d)", imdPtr->fileHandle, imdPtr->verMin);
		_vm->_dataio->closeData(handle);
		delete imdPtr;
		return 0;
	}

	imdPtr->surfDesc = surfDesc;
	imdPtr->framesPos = 0;
	imdPtr->firstFramePos = imdPtr->curFrame;
	
	if (flags & 3) {
		imdPtr->palette = new Video::Color[256];
		_vm->_dataio->readData(handle, (char *) imdPtr->palette, 768);
	} else {
		_vm->_dataio->seekData(handle, 768, 1);
		imdPtr->palette = 0;
	}
	if ((flags & 3) == 1) {
		palBak = _vm->_global->_pPaletteDesc->vgaPal;
		setAllPalBak = _vm->_global->_setAllPalette;
		_vm->_global->_pPaletteDesc->vgaPal = imdPtr->palette;
		_vm->_global->_setAllPalette = 1;
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		_vm->_global->_setAllPalette = setAllPalBak;
		_vm->_global->_pPaletteDesc->vgaPal = palBak;
	}

	if ((imdPtr->verMin & 0xFF) >= 3) {
		_vm->_dataio->readData(handle, buf, 2);
		imdPtr->stdX = READ_LE_UINT16(buf);
		if (imdPtr->stdX > 1) {
			warning("IMD ListI incorrect (%d)", imdPtr->stdX);
			_vm->_dataio->closeData(handle);
			delete imdPtr;
			return 0;
		}
		if(imdPtr->stdX != 0) {
			_vm->_dataio->readData(handle, buf, 8);
			imdPtr->stdX = READ_LE_UINT16(buf);
			imdPtr->stdY = READ_LE_UINT16(buf + 2);
			imdPtr->stdWidth = READ_LE_UINT16(buf + 4);
			imdPtr->stdHeight = READ_LE_UINT16(buf + 6);
		} else
			imdPtr->stdX = -1;
	} else
		imdPtr->stdX = -1;

	if ((imdPtr->verMin & 0xFF) >= 4) {
		_vm->_dataio->readData(handle, buf, 4);
		byte_31449 = READ_LE_UINT32(buf);
		imdPtr->framesPos = byte_31449 == 0 ? 0 : new int32[imdPtr->framesCount];
	} else
		imdPtr->framesPos = 0;

	if (imdPtr->verMin & 0x8000) {
		_vm->_dataio->readData(handle, buf, 4);
		byte_3144D = READ_LE_UINT32(buf);
	}

	if (imdPtr->verMin & 0x4000) {
		// loc_29C4F
		error("GOB2 Stub! loadImdFile, imdPtr->verMin & 0x4000");
		// Sound stuff, I presume...
	}

	if (imdPtr->verMin & 0x2000) {
		_vm->_dataio->readData(handle, buf, 4);
		imdPtr->frameDataSize = READ_LE_UINT16(buf);
		imdPtr->vidBufferSize = READ_LE_UINT16(buf + 2);
	} else {
		imdPtr->frameDataSize = imdPtr->width * imdPtr->height + 1000;
		imdPtr->vidBufferSize = imdPtr->width * imdPtr->height + 1000;
	}

	if (imdPtr->framesPos != 0) {
		_vm->_dataio->seekData(handle, byte_31449, 0);
		for (i = 0; i < imdPtr->framesCount; i++) {
			_vm->_dataio->readData(handle, buf, 4);
			imdPtr->framesPos[i] = READ_LE_UINT32(buf);
		}
	}

	if (imdPtr->verMin & 0x8000) {
		_vm->_dataio->seekData(handle, byte_3144D, 0);
		imdPtr->frameCoords = new ImdCoord[imdPtr->framesCount];
		for (i = 0; i < imdPtr->framesCount; i++) {
			_vm->_dataio->readData(handle, buf, 8);
			imdPtr->frameCoords[i].left = READ_LE_UINT16(buf);
			imdPtr->frameCoords[i].top = READ_LE_UINT16(buf + 2);
			imdPtr->frameCoords[i].right = READ_LE_UINT16(buf + 4);
			imdPtr->frameCoords[i].bottom = READ_LE_UINT16(buf + 6);
		}
	} else
		imdPtr->frameCoords = 0;

	_vm->_dataio->seekData(handle, imdPtr->firstFramePos, 0);
	imdPtr->curFrame = 0;
	imdPtr->fileHandle = handle;
	imdPtr->filePos = imdPtr->firstFramePos;
	_imdFrameDataSize = imdPtr->frameDataSize;
	_imdVidBufferSize = imdPtr->vidBufferSize;
	if (flags & 0x80) {
		imdPtr->verMin |= 0x1000;
		warning("GOB2 Stub! loadImdFile(), flags & 0x80");
	}

	return imdPtr;
}

void Game::setImdXY(Game::Imd *imdPtr, int16 x, int16 y) {
	int i;

	if (imdPtr->stdX != -1) {
		imdPtr->stdX = imdPtr->stdX - imdPtr->x + x;
		imdPtr->stdY = imdPtr->stdY - imdPtr->y + y;
	}
	
	if (imdPtr->frameCoords != 0) {
		for (i = 0; i < imdPtr->framesCount; i++) {
			imdPtr->frameCoords[i].left -= imdPtr->frameCoords[i].left - imdPtr->x + x;
			imdPtr->frameCoords[i].top -= imdPtr->frameCoords[i].top - imdPtr->y + y;
			imdPtr->frameCoords[i].right -= imdPtr->frameCoords[i].right - imdPtr->x + x;
			imdPtr->frameCoords[i].bottom -= imdPtr->frameCoords[i].bottom - imdPtr->y + y;
		}
	}

	imdPtr->x = x;
	imdPtr->y = y;
}

void Game::playImd(int16 frame, int16 arg_2, int16 arg_4, int16 arg_6, int16 arg_8, int16 lastFrame) {
	int16 var_1;
	int16 var_4 = 0;
	byte *vidMemBak;
	Video::SurfaceDesc *surfDescBak;
	Video::SurfaceDesc frontSurfBak;

	int8 byte_31344 = 0;

	if ((frame < 0) || (frame > lastFrame))
		return;

	if ((frame == arg_8) || ((frame == lastFrame) && (arg_2 == 8))) { // loc_1C3F0
		var_1 = 1;
		_vm->_draw->_applyPal = 0;
		if (arg_2 >= 4) {
			if (arg_4 != -1)
				memcpy( ((char *) (_vm->_global->_pPaletteDesc->vgaPal)) + arg_4 * 3,
						((char *) (_imdFile->palette)) + arg_4 * 3, (arg_6 - arg_4 + 1) * 3);
			else
				memcpy((char *) _vm->_global->_pPaletteDesc->vgaPal, (char *) _imdFile->palette, 768);
		}
	} else
		var_1 = 0;

	if ((var_1 == 1) && (arg_2 == 8) && (_byte_2FC83 != 0))
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	if (_vm->_video->_extraMode && (_imdFile->surfDesc->vidMode == 0x13)) {
		if ((_byte_2FC82 != 0) && (_word_2FC80 == _vm->_draw->_spritesArray[20]->vidPtr)) {
			vidMemBak = _vm->_draw->_spritesArray[20]->vidPtr;
			_vm->_draw->_spritesArray[20]->vidPtr = _vm->_draw->_spritesArray[21]->vidPtr;
			var_4 = viewImd(_imdFile, frame);
			_vm->_draw->_spritesArray[20]->vidPtr = vidMemBak;
		} else
			var_4 = viewImd(_imdFile, frame);
		if (_byte_2FC82 == 0) {
			if ((_imdFile->frameCoords == 0) || (_imdFile->frameCoords[frame].left == -1))
				_vm->_draw->invalidateRect(_imdX, _imdY,
						_imdX + _imdFile->width - 1, _imdY + _imdFile->height - 1);
			else
				_vm->_draw->invalidateRect(_imdFile->frameCoords[frame].left,
						_imdFile->frameCoords[frame].top, _imdFile->frameCoords[frame].right,
						_imdFile->frameCoords[frame].bottom);
		}
	} else {
		if ((_imdFile->field_E & 0x100) && (_vm->_video->_extraMode) &&
				(_byte_2FC82 != 0) && (sub_2C825(_imdFile) & 0x8000) && (_byte_2FC83 == 0)) {
			surfDescBak = _imdFile->surfDesc;
			if (_word_2FC80 == _vm->_draw->_spritesArray[20]->vidPtr)
				_imdFile->surfDesc = _vm->_draw->_spritesArray[21];
			else
				_imdFile->surfDesc = _vm->_draw->_spritesArray[20];
			setImdXY(_imdFile, _imdX, _imdY);
			var_4 = viewImd(_imdFile, frame);
			_imdFile->surfDesc = surfDescBak;
			setImdXY(_imdFile, 0, 0);
		} else {
			var_4 = viewImd(_imdFile, frame);
			if (!(var_4 & 0x800)) {
				if (_byte_2FC83 == 0) {
					if (_vm->_video->_extraMode) {
						if (_byte_2FC82 == 0) {
							memcpy((char *) &frontSurfBak, (char *) &_vm->_draw->_frontSurface,
									sizeof(Video::SurfaceDesc));
							memcpy((char *) &_vm->_draw->_frontSurface, (char *) &_vm->_draw->_spritesArray[21],
									sizeof(Video::SurfaceDesc));
							imdDrawFrame(_imdFile, frame, _imdX, _imdY);
							memcpy((char *) &_vm->_draw->_frontSurface, (char *) &frontSurfBak,
									sizeof(Video::SurfaceDesc));
							if ((_imdFile->frameCoords == 0) || (_imdFile->frameCoords[frame].left == -1))
								_vm->_draw->invalidateRect(_imdX, _imdY, _imdX + _imdFile->width - 1,
										_imdY + _imdFile->height - 1);
							else
								_vm->_draw->invalidateRect(_imdFile->frameCoords[frame].left,
										_imdFile->frameCoords[frame].top, _imdFile->frameCoords[frame].right,
										_imdFile->frameCoords[frame].bottom);
						} else {
							if (_word_2FC80 == _vm->_draw->_spritesArray[20]->vidPtr) { // loc_1C68D
								memcpy((char *) &frontSurfBak, (char *) &_vm->_draw->_frontSurface,
										sizeof(Video::SurfaceDesc));
								memcpy((char *) &_vm->_draw->_frontSurface, (char *) &_vm->_draw->_spritesArray[21],
										sizeof(Video::SurfaceDesc));
								imdDrawFrame(_imdFile, frame, _imdX, _imdY);
								memcpy((char *) &_vm->_draw->_frontSurface, (char *) &frontSurfBak,
										sizeof(Video::SurfaceDesc));
							} else
								imdDrawFrame(_imdFile, frame, _imdX, _imdY);
						}
					} else {
						if ((_imdFile->frameCoords == 0) || (_imdFile->frameCoords[frame].left == -1))
							_vm->_draw->invalidateRect(_imdX, _imdY, _imdX + _imdFile->width - 1,
									_imdY + _imdFile->height - 1);
						else
							_vm->_draw->invalidateRect(_imdFile->frameCoords[frame].left,
									_imdFile->frameCoords[frame].top, _imdFile->frameCoords[frame].right,
									_imdFile->frameCoords[frame].bottom);
					}
				} else
					if (_vm->_video->_extraMode)
						imdDrawFrame(_imdFile, frame, _imdX, _imdY);
			}
		}
	}

	if ((var_1 != 0) && (arg_2 == 16)) {
		if ((_vm->_draw->_spritesArray[20] != _vm->_draw->_spritesArray[21]) && (_byte_2FC83 == 0))
			_vm->_video->drawSprite(_vm->_draw->_spritesArray[21],
					_vm->_draw->_spritesArray[20], 0, 0,
					_vm->_draw->_spritesArray[21]->width - 1,
					_vm->_draw->_spritesArray[21]->height - 1, 0, 0, 0);
		_vm->_palanim->fade(_vm->_global->_pPaletteDesc, -2, 0);
		_vm->_draw->_noInvalidated = 1;
	}
	if ((var_1 != 0) && (arg_2 == 8) && (_byte_2FC83 == 0))
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	if (!(var_4 & 0x800)) {
		if (_vm->_draw->_cursorIndex == -1) {
			if (_byte_2FC82 != 0) {
				if (_word_2FC80 == _vm->_draw->_spritesArray[20]->vidPtr)
					_word_2FC80 = _vm->_draw->_spritesArray[21]->vidPtr;
				else
					_word_2FC80 = _vm->_draw->_spritesArray[20]->vidPtr;
				warning("GOB2 Stub! sub_1BC3A(_word_2FC80);");
			} else
				_vm->_draw->blitInvalidated();
		} else
			_vm->_draw->animateCursor(-1);
	}

	if ((var_1 != 0) && ((arg_2 == 2) || (arg_2 == 4)))
		_vm->_palanim->fade(_vm->_global->_pPaletteDesc, -2, 0);

	// To allow quitting, etc. during IMDs
	_vm->_util->processInput();
	if (_vm->_quitRequested) 
		return;

	if (byte_31344 != 2) {
		if (var_4 & 0x800) {
			if (_dword_2F2B6 == 0)
				_vm->_util->delay(30);
			else {
				_dword_2F2B6 -= 30;
				if (_dword_2F2B6 < 0)
					_dword_2F2B6 = 0;
			}
		} else
			_vm->_util->waitEndFrame();
	}
	_vm->_inter->animPalette();
}

int16 Game::viewImd(Game::Imd *imdPtr, int16 frame) {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 retVal;
	uint32 tmp;
	char buf[4];

	int8 var_4;
	int32 var_12 = 0;

	// .---
	int16 word_31451 = 0;
	int8 byte_31344 = 0;
	int8 byte_2DA60 = 0;
	int16 word_2DA61 = -1;
	// '---

	word_31451 = 0;

	if (imdPtr == 0)
		return 0x8000;

	retVal = 0;
	var_4 = 0;

	if (frame != imdPtr->curFrame) {
		retVal |= 0x2000;
		if (frame == 0)
			imdPtr->filePos = imdPtr->firstFramePos;
		else if (frame == 1) {
			imdPtr->filePos = imdPtr->firstFramePos;
			_vm->_dataio->seekData(imdPtr->fileHandle, imdPtr->filePos, 0);
			_vm->_dataio->readData(imdPtr->fileHandle, buf, 2);
			tmp = READ_LE_UINT16(buf);
			imdPtr->filePos += tmp + 4;
		} else if (imdPtr->framesPos != 0)
			imdPtr->filePos = imdPtr->framesPos[frame];
		else
			error("Image %d innaccessible in IMD", frame);
		imdPtr->curFrame = frame;
		_vm->_dataio->seekData(imdPtr->fileHandle, imdPtr->filePos, 0);
	}

	x = imdPtr->x;
	y = imdPtr->y;
	width = imdPtr->width;
	height = imdPtr->height;

	do {
		if (frame != 0) {
			if (imdPtr->stdX != -1) {
				imdPtr->x = imdPtr->stdX;
				imdPtr->y = imdPtr->stdY;
				imdPtr->width = imdPtr->stdWidth;
				imdPtr->height = imdPtr->stdHeight;
				retVal |= 0x1000;
			}
			if ((imdPtr->frameCoords != 0) && (imdPtr->frameCoords[frame].left != -1)) {
				var_4 |= 0x400;
				imdPtr->x = imdPtr->frameCoords[frame].left;
				imdPtr->y = imdPtr->frameCoords[frame].top;
				imdPtr->width = imdPtr->frameCoords[frame].right - imdPtr->x + 1;
				imdPtr->height = imdPtr->frameCoords[frame].bottom - imdPtr->y + 1;
			}
		}

		_vm->_dataio->readData(imdPtr->fileHandle, buf, 2);
		tmp = READ_LE_UINT16(buf);

		imdPtr->filePos += 2;

		if ((tmp & 0xFFF8) == 0xFFF0) {
			if (tmp == 0xFFF0) {
				_vm->_dataio->readData(imdPtr->fileHandle, buf, 2);
				tmp = READ_LE_UINT16(buf);
				if (var_4 == 0)
					word_31451 = tmp;
				_vm->_dataio->readData(imdPtr->fileHandle, buf, 2);
				tmp = READ_LE_UINT16(buf);
				imdPtr->filePos += 4;
			} else if (tmp == 0xFFF1) {
				retVal = 0x8000;
				continue;
			} else if (tmp == 0xFFF2) {
				_vm->_dataio->readData(imdPtr->fileHandle, buf, 2);
				tmp = READ_LE_UINT16(buf);
				imdPtr->filePos += 2;
				_vm->_dataio->seekData(imdPtr->fileHandle, tmp, 1);
				imdPtr->filePos += tmp;
				retVal = 0x8000;
				continue;
			} else if (tmp == 0xFFF3) {
				_vm->_dataio->readData(imdPtr->fileHandle, buf, 4);
				tmp = READ_LE_UINT32(buf);
				imdPtr->filePos += 4;
				_vm->_dataio->seekData(imdPtr->fileHandle, tmp, 1);
				imdPtr->filePos += tmp;
				retVal = 0x8000;
				continue;
			}
		}
		if (byte_31344 != 0) {
			if ((var_4 == 0) && (_vm->_global->_soundFlags & 0x14) && (byte_31344 == 2)) { // loc_2A503
				var_12 = _vm->_util->getTimeKey();
				warning("GOB2 Stub! viewImd, IMD sound stuff");
			}
		}
		var_4 = 0;
		if (tmp == 0xFFFD) {
			_vm->_dataio->readData(imdPtr->fileHandle, buf, 2);
			frame = READ_LE_UINT16(buf);
			if ((imdPtr->framesPos != 0) && (byte_2DA60 == 0)) {
				word_2DA61 = frame;
				imdPtr->filePos = imdPtr->framesPos[frame];
				_vm->_dataio->seekData(imdPtr->fileHandle, imdPtr->filePos, 0);
				var_4 = 1;
				retVal |= 0x200;
				imdPtr->curFrame = frame;
			} else
				imdPtr->filePos += 2;
			continue;
		}
		if (tmp != 0) {
			imdPtr->filePos += tmp + 2;
			if (byte_2DA60 != 0) {
				_vm->_dataio->seekData(imdPtr->fileHandle, tmp + 2, 1);
			} else {
				_vm->_dataio->readData(imdPtr->fileHandle, (char *) _imdFrameData, tmp + 2);
				retVal |= *_imdFrameData;
				if (imdPtr->surfDesc == 0)
					continue;
				if (!(_vm->_video->_extraMode && (imdPtr->surfDesc->vidMode == 0x13)))
					imdRenderFrame(imdPtr);
				else
					warning("GOB2 Stub! viedImd, sub_2C69A(imdPtr);");
			}
		} else
			retVal |= 0x800;
	} while(var_4 != 0);

	if (byte_2DA60 != 0) {
		byte_2DA60 = 0;
		retVal |= 0x100;
	}

	imdPtr->x = x;
	imdPtr->y = y;
	imdPtr->width = width;
	imdPtr->height = height;
	imdPtr->curFrame++;
	
	return retVal;
}

void Game::imdDrawFrame(Imd *imdPtr, int16 frame, int16 x, int16 y) {
	// In the original asm, "sub_2C348" is called instead of Video::drawSprite();
	// it basically just blits.

	if (frame == 0)
		_vm->_video->drawSprite(imdPtr->surfDesc, _vm->_draw->_frontSurface, 0, 0,
				imdPtr->width - 1, imdPtr->height - 1, x, y, 1);
	else if ((imdPtr->frameCoords != 0) && (imdPtr->frameCoords[frame].left != -1))
		_vm->_video->drawSprite(imdPtr->surfDesc, _vm->_draw->_frontSurface,
				imdPtr->frameCoords[frame].left, imdPtr->frameCoords[frame].top,
				imdPtr->frameCoords[frame].right, imdPtr->frameCoords[frame].bottom,
				imdPtr->frameCoords[frame].left, imdPtr->frameCoords[frame].top, 1);
	else if (imdPtr->stdX != -1)
		_vm->_video->drawSprite(imdPtr->surfDesc, _vm->_draw->_frontSurface,
				imdPtr->stdX, imdPtr->stdY, imdPtr->stdX + imdPtr->stdWidth - 1,
				imdPtr->stdY + imdPtr->stdHeight - 1, x + imdPtr->stdX,
				y + imdPtr->stdY, 1);
	else
		_vm->_video->drawSprite(imdPtr->surfDesc, _vm->_draw->_frontSurface, 0, 0,
				imdPtr->width - 1, imdPtr->height - 1, x, y, 0);
}

void Game::imdRenderFrame(Imd *imdPtr) {
	int i;
	int16 imdX;
	int16 imdY;
	int16 imdW;
	int16 imdH;
	int16 sW;
	uint16 pixCount, pixWritten;
	uint8 type;
	byte *imdVidMem;
	byte *imdVidMemBak;
	byte *dataPtr = 0;
	byte *srcPtr = 0;
	byte *srcPtrBak = 0;

	dataPtr = (byte *) _imdFrameData;
	imdX = imdPtr->x;
	imdY = imdPtr->y;
	imdW = imdPtr->width;
	imdH = imdPtr->height;
	sW = imdPtr->surfDesc->width;
	imdVidMem = imdPtr->surfDesc->vidPtr + sW * imdY + imdX;

	type = *dataPtr++;
	srcPtr = dataPtr;

	if (type & 0x10) {
		type ^= 0x10;
		dataPtr++; // => 0x3C8       |_ palette
		dataPtr += 48; // => 0x3C9   |  stuff
	}

	srcPtr = dataPtr;
	if (type & 0x80) {
		srcPtr = (byte *) _imdVidBuffer;
		type &= 0x7F;
		if ((type == 2) && (imdW == sW)) {
			imdFrameUncompressor(imdVidMem, dataPtr);
			return;
		} else
			imdFrameUncompressor(srcPtr, dataPtr);
	}

	if (type == 2) {
		for (i = 0; i < imdH; i++) {
			memcpy(imdVidMem, srcPtr, imdW);
			srcPtr += imdW;
			imdVidMem += sW;
		}
	} else if (type == 1) {
		imdVidMemBak = imdVidMem;
		for (i = 0; i < imdH; i++) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) {
					pixCount = (pixCount & 0x7F) + 1;
					// Just to be safe
					pixCount = (pixWritten + pixCount) > imdW ? imdW - pixWritten : pixCount;
					pixWritten += pixCount;
					memcpy(imdVidMem, srcPtr, pixCount);
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else {
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			imdVidMemBak += sW;
			imdVidMem = imdVidMemBak;
		}
	} else if (type == 0x42) { // loc_2AFC4
		warning("=> type == 0x42");
	} else if ((type & 0xF) == 2) { // loc_2AFEC
		warning("=> (type & 0xF) == 2");
	} else { // loc_2B021
		srcPtrBak = srcPtr;
		for (i = 0; i < imdH; i += 2) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) {
					pixCount = (pixCount & 0x7F) + 1;
					// Just to be safe
					pixCount = (pixWritten + pixCount) > imdW ? imdW - pixWritten : pixCount;
					pixWritten += pixCount;
					memcpy(imdVidMem, srcPtr, pixCount);
					memcpy(imdVidMem + sW, srcPtr, pixCount);
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else {
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			srcPtrBak += sW + sW;
			srcPtr = srcPtrBak;
		}
	}
}

void Game::imdFrameUncompressor(byte *dest, byte *src) {
	int i;
	byte buf[4370];
	int16 chunkLength;
	int16 frameLength;
	uint16 bufPos1;
	uint16 bufPos2;
	uint16 tmp;
	uint8 chunkBitField;
	uint8 chunkCount;
	bool mode;

	frameLength = READ_LE_UINT16(src);
	src += 4;
	bufPos1 = 4078;
	mode = 0; // 275h (jnz +2)
	if ((READ_LE_UINT16(src) == 0x1234) && (READ_LE_UINT16(src + 2) == 0x5678)) {
		src += 4;
		bufPos1 = 273;
		mode = 1; // 123Ch (cmp al, 12h)
	}
	memset(buf, 32, bufPos1);
	chunkCount = 1;
	chunkBitField = 0;

	while(frameLength > 0) {
		chunkCount--;
		if (chunkCount == 0) {
			tmp = *src++;
			chunkCount = 8;
			chunkBitField = tmp;
		}
		if (chunkBitField % 2) {
			chunkBitField >>= 1;
			buf[bufPos1] = *src;
			*dest++ = *src++;
			bufPos1 = (bufPos1 + 1) % 4096;
			frameLength--;
			continue;
		}
		chunkBitField >>= 1;

		tmp = READ_LE_UINT16(src);
		src += 2;
		chunkLength = ((tmp & 0xF00) >> 8) + 3;

		if ((mode && ((chunkLength & 0xFF) == 0x12)) || (!mode && (chunkLength == 0)))
			chunkLength = *src++ + 0x12;

		bufPos2 = (tmp & 0xFF) + ((tmp >> 4) & 0x0F00);
		if (((tmp + chunkLength) >= 4096) || ((chunkLength + bufPos1) >= 4096)) {
			for (i = 0; i < chunkLength; i++, dest++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
				bufPos1 = (bufPos1 + 1) % 4096;
				bufPos2 = (bufPos2 + 1) % 4096;
			}
			frameLength -= chunkLength;
		} else if (((tmp + chunkLength) < bufPos1) || ((chunkLength + bufPos1) < bufPos2)) {
			memcpy(dest, buf + bufPos2, chunkLength);
			dest += chunkLength;
			memmove(buf + bufPos1, buf + bufPos2, chunkLength);
			bufPos1 += chunkLength;
			bufPos2 += chunkLength;
			frameLength -= chunkLength;
		} else {
			for (i = 0; i < chunkLength; i++, dest++, bufPos1++, bufPos2++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
			}
			frameLength -= chunkLength;
		}
	}
}

int16 Game::sub_2C825(Imd *imdPtr) {
	warning("GOB2 Stub! sub_2C825()");
	return 0;
}

} // End of namespace Gob
