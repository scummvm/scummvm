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

	_foundTotLoc = false;
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
		_soundFromExt[i] = 0;
		_soundADL[i] = false;
	}
	_infIns = 0;
	_infogrames = 0;

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

	warning("GOB2 Stub! _byte_2FC9B, _dword_2F2B6");
	_byte_2FC9B = 0;
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
	if (_infIns)
		delete _infIns;

	for (int i = 0; i < 60; i++)
		freeSoundSlot(i);
}

char *Game::loadExtData(int16 itemId, int16 *pResWidth, int16 *pResHeight, uint32 *dataSize) {
	int16 commonHandle;
	int16 itemsCount;
	int32 offset;
	uint32 size;
	uint32 realSize;
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
		debugC(7, kDebugFileIO, "loadExtData(%d, %d, %d)", itemId, *pResWidth, *pResHeight);
	}

	debugC(7, kDebugFileIO, "loadExtData(%d, 0, 0)", itemId);

	if (item->height == 0)
		size += (item->width & 0x7fff) << 16;

	debugC(7, kDebugFileIO, "size: %d off: %d", size, offset);
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

	debugC(7, kDebugFileIO, "off: %d size: %d", offset, tableSize);
	_vm->_dataio->seekData(handle, offset + tableSize, SEEK_SET);
	realSize = size;
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
		realSize = READ_LE_UINT32(packedBuf);
		dataBuf = new char[realSize];
		_vm->_pack->unpackData(packedBuf, dataBuf);
		delete[] packedBuf;
	}

	if (dataSize)
		*dataSize = realSize;
	return dataBuf;
}

void Game::freeCollision(int16 id) {
	int16 i;

	for (i = 0; i < 250; i++) {
		if (_collisionAreas[i].id == id)
			_collisionAreas[i].left = -1;
	}
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
	_vm->_draw->_spritesArray[30 + _captureCount] = 0;
}

char *Game::loadTotResource(int16 id, int16 *dataSize) {
	TotResItem *itemPtr;
	int32 offset;

	itemPtr = &_totResourceTable->items[id];
	offset = itemPtr->offset;
	if (dataSize)
		*dataSize = itemPtr->size;
	if (offset >= 0) {
		return _totResourceTable->dataPtr + szGame_TotResTable +
		    szGame_TotResItem * _totResourceTable->itemsCount + offset;
	} else {
		return (char *)(_imFileData + (int32)READ_LE_UINT32(&((int32 *)_imFileData)[-offset - 1]));
	}
}

void Game::loadSound(int16 slot, char *dataPtr, uint32 dataSize) {
	Snd::SoundDesc *soundDesc;
	byte *data = (byte *) dataPtr;

	soundDesc = new Snd::SoundDesc;

	_soundSamples[slot] = soundDesc;

	soundDesc->frequency = (data[4] << 8) + data[5];
	// Somehow, one sound in one CD version has a wrong size, leading to statics and crashes
	soundDesc->size = MIN((uint32) ((data[1] << 16) + (data[2] << 8) + data[3]), dataSize - 6);
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

	if (_soundADL[slot]) {
		if (_vm->_adlib && (_vm->_adlib->getIndex() == slot))
			_vm->_adlib->stopPlay();

		if (_soundFromExt[slot] == 1) {
			delete[] _soundSamples[slot]->data;
			_soundFromExt[slot] = 0;
		}

		delete _soundSamples[slot];
	} else {
		char* data = _soundSamples[slot]->data;

		_vm->_snd->freeSoundDesc(_soundSamples[slot], false);

		if (_soundFromExt[slot] == 1) {
			delete[] (data - 6);
			_soundFromExt[slot] = 0;
		}
	}
	_soundSamples[slot] = 0;
}

int16 Game::checkKeys(int16 *pMouseX, int16 *pMouseY, int16 *pButtons, char handleMouse) {
	_vm->_util->processInput();

	if (_vm->_mult->_multData && (_vm->_global->_inter_variables != 0) &&
			(VAR(58) != 0)) {
		if (_vm->_mult->_multData->frameStart != (int)VAR(58) - 1)
			_vm->_mult->_multData->frameStart++;
		else
			_vm->_mult->_multData->frameStart = 0;

		_vm->_mult->playMult(_vm->_mult->_multData->frameStart + VAR(57),
				_vm->_mult->_multData->frameStart + VAR(57), 1, handleMouse);
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

int16 Game::adjustKey(int16 key) {
	if (key <= 0x60 || key >= 0x7b)
		return key;

	return key - 0x20;
}

int32 Game::loadTotFile(char *path) {
	int16 handle;
	int32 size;

	size = -1;
	handle = _vm->_dataio->openData(path);
	if (handle >= 0) {
		_vm->_dataio->closeData(handle);
		size = _vm->_dataio->getDataSize(path);
		_totFileData = _vm->_dataio->getData(path);
	} else {
		_totFileData = 0;
	}

	return size;
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
	int i;

	_collisionAreas = new Collision[250];
	memset(_collisionAreas, 0, 250 * sizeof(Collision));

	prepareStart();
	playTot(-2);

	delete[] _collisionAreas;
	_vm->_draw->closeScreen();

	_vm->_draw->_spritesArray[20] = 0;
	for (i = 0; i < 50; i++) {
		_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[i]);
		_vm->_draw->_spritesArray[i] = 0;
	}
	_vm->_video->freeSurfDesc(_vm->_draw->_scummvmCursor);
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
	_variablesSizesArray[_backupedCount] = _vm->_global->_inter_variablesSizes;
	strcpy(_curTotFileArray[_backupedCount], _curTotFile);

	curBackupPos = _curBackupPos;
	_backupedCount++;
	_curBackupPos = _backupedCount;

	_totTextData = 0;
	_totFileData = 0;
	_totResourceTable = 0;
	if (flags & 1) {
		_vm->_global->_inter_variables = 0;
		_vm->_global->_inter_variablesSizes = 0;
	}

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

	if ((flags & 1) && (_vm->_global->_inter_variables != 0)) {
		delete[] _vm->_global->_inter_variables;
		delete[] _vm->_global->_inter_variablesSizes;
	}

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
	_vm->_global->_inter_variablesSizes = _variablesSizesArray[_backupedCount];
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
		_variablesSizesArray[_backupedCount] = _vm->_global->_inter_variablesSizes;
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
	_imFileData = _imFileDataArray[_curBackupPos];
	_extTable = _extTableArray[_curBackupPos];
	_extHandle = _extHandleArray[_curBackupPos];
	_vm->_global->_inter_variables = _variablesArray[_curBackupPos];
	_vm->_global->_inter_variablesSizes = _variablesSizesArray[_curBackupPos];
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
	_vm->_global->_inter_variablesSizes = _variablesSizesArray[_curBackupPos];
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

	handle = openLocTextFile(locTextFile, _vm->_global->_languageWanted);
	if (handle >= 0) {
		_foundTotLoc = true;
		_vm->_global->_language = _vm->_global->_languageWanted;
	}
	else if (!_foundTotLoc) {
		for (i = 0; i < 10; i++) {
			handle = openLocTextFile(locTextFile, i);
			if (handle >= 0) {
				_vm->_global->_language = i;
				break;
			}
		}
	}
	debugC(1, kDebugFileIO, "Using language %d for %s", _vm->_global->_language, _curTotFile);

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
		if ((_vm->_draw->_renderFlags & 8) && (left != -1)) {
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

void Game::collSub(uint16 offset) {
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

	if ((collId == 0xA000) || (collId == 0x9000)) {
		if (enter == 0)
			WRITE_VAR(17, _collisionAreas[index].id & 0x0FFF);
		else
			WRITE_VAR(17, -(_collisionAreas[index].id & 0x0FFF));
	}

	if (enter != 0) {
		if (_collisionAreas[index].funcEnter != 0)
			collSub(_collisionAreas[index].funcEnter);
	} else {
		if (_collisionAreas[index].funcLeave != 0)
			collSub(_collisionAreas[index].funcLeave);
	}
}

Snd::SoundDesc *Game::loadSND(const char *path, int8 arg_4) {
	Snd::SoundDesc *soundDesc;
	uint32 dSize;
	char *data;
	char *dataPtr;

	soundDesc = new Snd::SoundDesc;

	dSize = _vm->_dataio->getDataSize(path) - 6;
	data = _vm->_dataio->getData(path);
	soundDesc->data = new char[dSize];
	soundDesc->flag = *data ? (*data & 0x7F) : 8;
	dataPtr = data + 4;

	WRITE_LE_UINT16(dataPtr, READ_BE_UINT16(dataPtr));
	WRITE_LE_UINT32(data,
			(READ_LE_UINT32(data) >> 24) +
			((READ_LE_UINT16(data) & 0xFF00) << 8) +
			((READ_LE_UINT16(data + 2) & 0xFF) >> 8));

	soundDesc->size = MAX(dSize, READ_LE_UINT32(data));
	soundDesc->frequency = READ_LE_UINT16(dataPtr);
	soundDesc->timerTicks = 1193180 / READ_LE_UINT16(dataPtr);
	memcpy(soundDesc->data, data + 6, dSize - 6);

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

			if (_vm->_global->_videoMode == 0x14) {
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
					if ((_imdFile->surfDesc->vidMode & 0x7F) == 0x14) {
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
		if (imdPtr->stdX != 0) {
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
		warning("GOB2 Stub! loadImdFile, imdPtr->verMin & 0x4000");
		warning("Can't open IMD \"%s.IMD\"", path);
		return 0;
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

	_vm->_draw->_showCursor = 0;

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

	if (_imdFile->surfDesc->vidMode == 0x14) {
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
		if ((_imdFile->field_E & 0x100) && (_vm->_global->_videoMode == 0x14) &&
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
					if (_vm->_global->_videoMode == 0x14) {
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
					if (_vm->_global->_videoMode == 0x14)
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
		return (int16)0x8000;

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
			error("Image %d inaccessible in IMD", frame);
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
				retVal = (int16)0x8000;
				continue;
			} else if (tmp == 0xFFF2) {
				_vm->_dataio->readData(imdPtr->fileHandle, buf, 2);
				tmp = READ_LE_UINT16(buf);
				imdPtr->filePos += 2;
				_vm->_dataio->seekData(imdPtr->fileHandle, tmp, 1);
				imdPtr->filePos += tmp;
				retVal = (int16)0x8000;
				continue;
			} else if (tmp == 0xFFF3) {
				_vm->_dataio->readData(imdPtr->fileHandle, buf, 4);
				tmp = READ_LE_UINT32(buf);
				imdPtr->filePos += 4;
				_vm->_dataio->seekData(imdPtr->fileHandle, tmp, 1);
				imdPtr->filePos += tmp;
				retVal = (int16)0x8000;
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
				if (imdPtr->surfDesc->vidMode != 0x14)
					imdRenderFrame(imdPtr);
				else
					warning("GOB2 Stub! viedImd, sub_2C69A(imdPtr);");
			}
		} else
			retVal |= 0x800;
	} while (var_4 != 0);

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

void Game::imdDrawFrame(Imd *imdPtr, int16 frame, int16 x, int16 y,
		Video::SurfaceDesc *dest) {
	if (!dest)
		dest = _vm->_draw->_frontSurface;

	if (frame == 0)
		_vm->_video->drawSprite(imdPtr->surfDesc, dest, 0, 0,
				imdPtr->width - 1, imdPtr->height - 1, x, y, 1);
	else if ((imdPtr->frameCoords != 0) && (imdPtr->frameCoords[frame].left != -1))
		_vm->_video->drawSprite(imdPtr->surfDesc, dest,
				imdPtr->frameCoords[frame].left, imdPtr->frameCoords[frame].top,
				imdPtr->frameCoords[frame].right, imdPtr->frameCoords[frame].bottom,
				imdPtr->frameCoords[frame].left, imdPtr->frameCoords[frame].top, 1);
	else if (imdPtr->stdX != -1)
		_vm->_video->drawSprite(imdPtr->surfDesc, dest,
				imdPtr->stdX, imdPtr->stdY, imdPtr->stdX + imdPtr->stdWidth - 1,
				imdPtr->stdY + imdPtr->stdHeight - 1, x + imdPtr->stdX,
				y + imdPtr->stdY, 1);
	else
		_vm->_video->drawSprite(imdPtr->surfDesc, dest, 0, 0,
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

	while (frameLength > 0) {
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

void Game::playImd(const char *path, int16 x, int16 y, int16 startFrame, int16 frames,
		bool fade, bool interruptible) {
	int16 mouseX;
	int16 mouseY;
	int16 buttons;
	int curFrame;
	int endFrame;
	int backFrame;

	_vm->_util->setFrameRate(12);
	openImd(path, 0, 0, 0, 0);
	_vm->_video->fillRect(_vm->_draw->_frontSurface, x, y, x + _imdFile->width - 1,
			y + _imdFile->height - 1, 0);

	if (fade)
		_vm->_palanim->fade(0, -2, 0);

	endFrame = frames > 0 ? frames : _imdFile->framesCount;
	for (curFrame = 0; curFrame < endFrame; curFrame++) {
		viewImd(_imdFile, curFrame);
		imdDrawFrame(_imdFile, curFrame, x, y);
		if (fade) {
			_vm->_palanim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			fade = false;
		}
		_vm->_video->waitRetrace(_vm->_global->_videoMode);
		if ((interruptible && (checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B)) ||
				_vm->_quitRequested) {
			_vm->_palanim->fade(0, -2, 0);
			_vm->_video->clearSurf(_vm->_draw->_frontSurface);
			memset((char *) _vm->_draw->_vgaPalette, 0, 768);
			WRITE_VAR(4, buttons);
			WRITE_VAR(0, 0x11B);
			WRITE_VAR(57, (uint32) -1);
			break;
		}
		_vm->_util->waitEndFrame();
	}
	if (frames < 0) {
		endFrame = _imdFile->framesCount + frames;
		for (curFrame = _imdFile->framesCount - 1; curFrame >= endFrame; curFrame--) {
			for (backFrame = 0; backFrame <= curFrame; backFrame++)
				viewImd(_imdFile, backFrame);
			imdDrawFrame(_imdFile, curFrame, x, y);
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
			if ((interruptible && (checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B)) ||
					_vm->_quitRequested) {
				_vm->_palanim->fade(0, -2, 0);
				_vm->_video->clearSurf(_vm->_draw->_frontSurface);
				memset((char *) _vm->_draw->_vgaPalette, 0, 768);
				WRITE_VAR(4, buttons);
				WRITE_VAR(0, 0x11B);
				WRITE_VAR(57, (uint32) -1);
				break;
			}
			_vm->_util->waitEndFrame();
		}
	}
	closeImd();
}

int16 Game::sub_2C825(Imd *imdPtr) {
	warning("GOB2 Stub! sub_2C825()");
	return 0;
}

} // End of namespace Gob
