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

#include "gob/gob.h"
#include "gob/game.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/script.h"
#include "gob/inter.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

Game::Game(GobEngine *vm) : _vm(vm) {
	_extTable = 0;
	_totResourceTable = 0;
	_imFileData = 0;
	_extHandle = 0;
	_lomHandle = -1;
	_collisionAreas = 0;
	_shouldPushColls = 0;

	_captureCount = 0;

	_foundTotLoc = false;
	_totTextData = 0;

	_collStackSize = 0;

	for (int i = 0; i < 5; i++) {
		_collStack[i] = 0;
		_collStackElemSizes[i] = 0;
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
	_menuLevel = 0;
	_noScroll = true;
	_preventScroll = false;
	_scrollHandleMouse = false;

	_noCd = false;

	_tempStr[0] = 0;
	_curImaFile[0] = 0;
	_collStr[0] = 0;

	_backupedCount = 0;
	_curBackupPos = 0;

	for (int i = 0; i < 5; i++) {
		_cursorHotspotXArray[i] = 0;
		_cursorHotspotYArray[i] = 0;
		_totTextDataArray[i] = 0;
		_totResourceTableArray[i] = 0;
		_extTableArray[i] = 0;
		_extHandleArray[i] = 0;
		_imFileDataArray[i] = 0;
		_variablesArray[i] = 0;
		_curTotFileArray[i][0] = 0;
	}

	_script = new Script(_vm);
}

Game::~Game() {
	if (_totTextData) {
		if (_totTextData->items)
			delete[] _totTextData->items;
		delete _totTextData;
	}
	if (_totResourceTable) {
		delete[] _totResourceTable->items;
		delete _totResourceTable;
	}

	delete _script;
}

byte *Game::loadExtData(int16 itemId, int16 *pResWidth,
		int16 *pResHeight, uint32 *dataSize) {
	int16 commonHandle;
	int16 itemsCount;
	int32 offset;
	uint32 size;
	uint32 realSize;
	ExtItem *item;
	bool isPacked;
	int16 handle;
	int32 tableSize;
	char path[20];
	byte *dataBuf;
	byte *packedBuf;
	byte *dataPtr;

	itemId -= 30000;
	if (_extTable == 0)
		return 0;

	commonHandle = -1;
	itemsCount = _extTable->itemsCount;
	item = &_extTable->items[itemId];
	tableSize = szGame_ExtTable + szGame_ExtItem * itemsCount;

	offset = item->offset;
	size = item->size;
	isPacked = (item->width & 0x8000) != 0;

	if ((pResWidth != 0) && (pResHeight != 0)) {
		*pResWidth = item->width & 0x7FFF;

		if (*pResWidth & 0x4000)
			size += 1 << 16;
		if (*pResWidth & 0x2000)
			size += 2 << 16;
		if (*pResWidth & 0x1000)
			size += 4 << 16;

		*pResWidth &= 0xFFF;

		*pResHeight = item->height;
		debugC(7, kDebugFileIO, "loadExtData(%d, %d, %d)",
				itemId, *pResWidth, *pResHeight);
	}

	debugC(7, kDebugFileIO, "loadExtData(%d, 0, 0)", itemId);

	if (item->height == 0)
		size += (item->width & 0x7FFF) << 16;

	debugC(7, kDebugFileIO, "size: %d off: %d", size, offset);
	if (offset < 0) {
		offset = -(offset + 1);
		tableSize = 0;
		_vm->_dataIO->closeData(_extHandle);
		strcpy(path, "commun.ex1");
		path[strlen(path) - 1] = *(_script->getData() + 0x3C) + '0';
		commonHandle = _vm->_dataIO->openData(path);
		handle = commonHandle;
	} else
		handle = _extHandle;

	DataStream *stream = _vm->_dataIO->openAsStream(handle);

	debugC(7, kDebugFileIO, "off: %d size: %d", offset, tableSize);
	stream->seek(offset + tableSize);
	realSize = size;
	if (isPacked)
		dataBuf = new byte[size + 2];
	else
		dataBuf = new byte[size];

	dataPtr = dataBuf;
	while (size > 32000) {
		// BUG: huge->far conversion. Need normalization?
		stream->read(dataPtr, 32000);
		size -= 32000;
		dataPtr += 32000;
	}
	stream->read(dataPtr, size);

	delete stream;
	if (commonHandle != -1) {
		_vm->_dataIO->closeData(commonHandle);
		_extHandle = _vm->_dataIO->openData(_curExtFile);
	}

	if (isPacked) {
		packedBuf = dataBuf;
		realSize = READ_LE_UINT32(packedBuf);
		dataBuf = new byte[realSize];
		_vm->_dataIO->unpackData(packedBuf, dataBuf);
		delete[] packedBuf;
	}

	if (dataSize)
		*dataSize = realSize;
	return dataBuf;
}

void Game::freeCollision(int16 id) {
	for (int i = 0; i < 250; i++) {
		if (_collisionAreas[i].id == id)
			_collisionAreas[i].left = 0xFFFF;
	}
}

void Game::capturePush(int16 left, int16 top, int16 width, int16 height) {
	int16 right;

	if (_captureCount == 20)
		error("Game::capturePush(): Capture stack overflow");

	_captureStack[_captureCount].left = left;
	_captureStack[_captureCount].top = top;
	_captureStack[_captureCount].right = left + width;
	_captureStack[_captureCount].bottom = top + height;

	_vm->_draw->_spriteTop = top;
	_vm->_draw->_spriteBottom = height;

	right = left + width - 1;
	left &= 0xFFF0;
	right |= 0xF;

	_vm->_draw->initSpriteSurf(30 + _captureCount, right - left + 1, height, 0);

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
		_vm->_draw->_spriteLeft = _vm->_draw->_destSpriteX & 0xF;
		_vm->_draw->_spriteTop = 0;
		_vm->_draw->spriteOperation(0);
	}
	_vm->_draw->freeSprite(30 + _captureCount);
}

byte *Game::loadTotResource(int16 id,
		int16 *dataSize, int16 *width, int16 *height) {

	TotResItem *itemPtr;
	int32 offset;

	if (id >= _totResourceTable->itemsCount) {
		warning("Trying to load non-existent TOT resource (%s, %d/%d)",
				_curTotFile, id, _totResourceTable->itemsCount - 1);
		return 0;
	}

	itemPtr = &_totResourceTable->items[id];
	offset = itemPtr->offset;

	if (dataSize)
		*dataSize = itemPtr->size;
	if (width)
		*width = itemPtr->width;
	if (height)
		*height = itemPtr->height;

	if (offset < 0) {
		offset = (-offset - 1) * 4;
		return _imFileData + (int32) READ_LE_UINT32(_imFileData + offset);
	} else
		return _totResourceTable->dataPtr + szGame_TotResTable +
		    szGame_TotResItem * _totResourceTable->itemsCount + offset;
}

void Game::freeSoundSlot(int16 slot) {
	if (slot == -1)
		slot = _script->readValExpr();

	_vm->_sound->sampleFree(_vm->_sound->sampleGetBySlot(slot));
}

void Game::evaluateScroll(int16 x, int16 y) {
	if (_preventScroll || !_scrollHandleMouse || (_menuLevel > 0))
		return;

	if (_noScroll ||
	   ((_vm->_global->_videoMode != 0x14) && (_vm->_global->_videoMode != 0x18)))
		return;

	if ((x == 0) && (_vm->_draw->_scrollOffsetX > 0)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorWidth, _vm->_draw->_scrollOffsetX);
		off = MAX(off / 2, 1);
		_vm->_draw->_scrollOffsetX -= off;
		_vm->_video->dirtyRectsAll();
	} else if ((y == 0) && (_vm->_draw->_scrollOffsetY > 0)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorHeight, _vm->_draw->_scrollOffsetY);
		off = MAX(off / 2, 1);
		_vm->_draw->_scrollOffsetY -= off;
		_vm->_video->dirtyRectsAll();
	}

	int16 cursorRight = x + _vm->_draw->_cursorWidth;
	int16 screenRight = _vm->_draw->_scrollOffsetX + _vm->_width;
	int16 cursorBottom = y + _vm->_draw->_cursorHeight;
	int16 screenBottom = _vm->_draw->_scrollOffsetY + _vm->_height;

	if ((cursorRight >= _vm->_width) &&
			(screenRight < _vm->_video->_surfWidth)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorWidth,
				(int16) (_vm->_video->_surfWidth - screenRight));
		off = MAX(off / 2, 1);

		_vm->_draw->_scrollOffsetX += off;
		_vm->_video->dirtyRectsAll();

		_vm->_util->setMousePos(_vm->_width - _vm->_draw->_cursorWidth, y);
	} else if ((cursorBottom >= (_vm->_height - _vm->_video->_splitHeight2)) &&
			(screenBottom < _vm->_video->_surfHeight)) {
		uint16 off;

		off = MIN(_vm->_draw->_cursorHeight,
				(int16) (_vm->_video->_surfHeight - screenBottom));
		off = MAX(off / 2, 1);

		_vm->_draw->_scrollOffsetY += off;
		_vm->_video->dirtyRectsAll();

		_vm->_util->setMousePos(x, _vm->_height - _vm->_video->_splitHeight2 -
				_vm->_draw->_cursorHeight);
	}

	_vm->_util->setScrollOffset();
}

int16 Game::checkKeys(int16 *pMouseX, int16 *pMouseY,
		int16 *pButtons, char handleMouse) {

	_vm->_util->processInput(true);

	if (_vm->_mult->_multData && _vm->_inter->_variables &&
			(VAR(58) != 0)) {
		if (_vm->_mult->_multData->frameStart != (int) VAR(58) - 1)
			_vm->_mult->_multData->frameStart++;
		else
			_vm->_mult->_multData->frameStart = 0;

		_vm->_mult->playMult(_vm->_mult->_multData->frameStart + VAR(57),
				_vm->_mult->_multData->frameStart + VAR(57), 1, handleMouse);
	}

	if ((_vm->_inter->_soundEndTimeKey != 0) &&
	    (_vm->_util->getTimeKey() >= _vm->_inter->_soundEndTimeKey)) {
		_vm->_sound->blasterStop(_vm->_inter->_soundStopVal);
		_vm->_inter->_soundEndTimeKey = 0;
	}

	if (pMouseX && pMouseY && pButtons) {
		_vm->_util->getMouseState(pMouseX, pMouseY, pButtons);

		if (*pButtons == 3)
			*pButtons = 0;
	}

	return _vm->_util->checkKey();
}

int16 Game::adjustKey(int16 key) {
	if (key <= 0x60 || key >= 0x7B)
		return key;

	return key - 0x20;
}

void Game::loadExtTable(void) {
	int16 count;

	// Function is correct. [sev]

	_extHandle = _vm->_dataIO->openData(_curExtFile);
	if (_extHandle < 0)
		return;

	DataStream *stream = _vm->_dataIO->openAsStream(_extHandle);
	count = stream->readUint16LE();

	stream->seek(0);
	_extTable = new ExtTable;
	_extTable->items = 0;
	if (count)
		_extTable->items = new ExtItem[count];

	_extTable->itemsCount = stream->readUint16LE();
	_extTable->unknown = stream->readByte();

	for (int i = 0; i < count; i++) {
		_extTable->items[i].offset = stream->readUint32LE();
		_extTable->items[i].size = stream->readUint16LE();
		_extTable->items[i].width = stream->readUint16LE();
		_extTable->items[i].height = stream->readUint16LE();
	}

	delete stream;
}

void Game::loadImFile(void) {
	char path[20];

	if ((_script->getData()[0x3D] != 0) && (_script->getData()[0x3B] == 0))
		return;

	strcpy(path, "commun.im1");
	if (_script->getData()[0x3B] != 0)
		path[strlen(path) - 1] = '0' + _script->getData()[0x3B];

	if (!_vm->_dataIO->existData(path))
		return;

	_imFileData = _vm->_dataIO->getData(path);
}

void Game::start(void) {
	_collisionAreas = new Collision[250];
	memset(_collisionAreas, 0, 250 * sizeof(Collision));

	prepareStart();
	playTot(-2);

	delete[] _collisionAreas;
	_vm->_draw->closeScreen();

	for (int i = 0; i < SPRITES_COUNT; i++)
		_vm->_draw->freeSprite(i);
	_vm->_draw->_scummvmCursor.reset();
}

// flagbits: 0 = freeInterVariables, 1 = skipPlay
void Game::totSub(int8 flags, const char *newTotFile) {
	warning("totSub");

	int8 curBackupPos;

	if (_backupedCount >= 5)
		return;

	_cursorHotspotXArray[_backupedCount] = _vm->_draw->_cursorHotspotXVar;
	_cursorHotspotYArray[_backupedCount] = _vm->_draw->_cursorHotspotYVar;
	_scriptArray[_backupedCount] = _script;
	_totTextDataArray[_backupedCount] = _totTextData;
	_totResourceTableArray[_backupedCount] = _totResourceTable;
	_extTableArray[_backupedCount] = _extTable;
	_extHandleArray[_backupedCount] = _extHandle;
	_imFileDataArray[_backupedCount] = _imFileData;
	_variablesArray[_backupedCount] = _vm->_inter->_variables;
	strcpy(_curTotFileArray[_backupedCount], _curTotFile);

	curBackupPos = _curBackupPos;
	_backupedCount++;
	_curBackupPos = _backupedCount;

	_totTextData = 0;
	_script = new Script(_vm);
	_totResourceTable = 0;
	if (flags & 1)
		_vm->_inter->_variables = 0;

	strncpy0(_curTotFile, newTotFile, 9);
//	if (_vm->getGameType() == kGameTypeGeisha)
//		strcat(_curTotFile, ".0OT");
//	else
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

	if ((flags & 1) && _vm->_inter->_variables) {
		_vm->_inter->delocateVars();
	}

	_backupedCount--;
	_curBackupPos = curBackupPos;

	_vm->_draw->_cursorHotspotXVar = _cursorHotspotXArray[_backupedCount];
	_vm->_draw->_cursorHotspotYVar = _cursorHotspotYArray[_backupedCount];
	_totTextData = _totTextDataArray[_backupedCount];
	_script = _scriptArray[_backupedCount];
	_totResourceTable = _totResourceTableArray[_backupedCount];
	_extTable = _extTableArray[_backupedCount];
	_extHandle = _extHandleArray[_backupedCount];
	_imFileData = _imFileDataArray[_backupedCount];
	_vm->_inter->_variables = _variablesArray[_backupedCount];
	strcpy(_curTotFile, _curTotFileArray[_backupedCount]);
	strcpy(_curExtFile, _curTotFile);
	_curExtFile[strlen(_curExtFile) - 4] = '\0';
	strcat(_curExtFile, ".EXT");
}

void Game::switchTotSub(int16 index, int16 skipPlay) {
	warning("switchTotSub");

	int16 backupedCount;
	int16 curBackupPos;

	if ((_backupedCount - index) < 1)
		return;

	int16 newPos = _curBackupPos - index - ((index >= 0) ? 1 : 0);
	// WORKAROUND: Some versions don't make the MOVEMENT menu item unselectable
	// in the dreamland screen, resulting in a crash when it's clicked.
	if ((_vm->getGameType() == kGameTypeGob2) && (index == -1) && (skipPlay == 7) &&
	    !scumm_stricmp(_curTotFileArray[newPos], "gob06.tot"))
		return;

	curBackupPos = _curBackupPos;
	backupedCount = _backupedCount;
	if (_curBackupPos == _backupedCount) {
		_cursorHotspotXArray[_backupedCount] = _vm->_draw->_cursorHotspotXVar;
		_cursorHotspotYArray[_backupedCount] = _vm->_draw->_cursorHotspotYVar;
		_totTextDataArray[_backupedCount] = _totTextData;
		_scriptArray[_backupedCount] = _script;
		_totResourceTableArray[_backupedCount] = _totResourceTable;
		_extTableArray[_backupedCount] = _extTable;
		_extHandleArray[_backupedCount] = _extHandle;
		_imFileDataArray[_backupedCount] = _imFileData;
		_variablesArray[_backupedCount] = _vm->_inter->_variables;
		strcpy(_curTotFileArray[_backupedCount], _curTotFile);
		_backupedCount++;
	}
	_curBackupPos -= index;
	if (index >= 0)
		_curBackupPos--;

	_vm->_draw->_cursorHotspotXVar = _cursorHotspotXArray[_curBackupPos];
	_vm->_draw->_cursorHotspotYVar = _cursorHotspotYArray[_curBackupPos];
	_totTextData = _totTextDataArray[_curBackupPos];
	_script = _scriptArray[_curBackupPos];
	_totResourceTable = _totResourceTableArray[_curBackupPos];
	_imFileData = _imFileDataArray[_curBackupPos];
	_extTable = _extTableArray[_curBackupPos];
	_extHandle = _extHandleArray[_curBackupPos];
	_vm->_inter->_variables = _variablesArray[_curBackupPos];
	strcpy(_curTotFile, _curTotFileArray[_curBackupPos]);
	strcpy(_curExtFile, _curTotFile);
	_curExtFile[strlen(_curExtFile) - 4] = '\0';
	strcat(_curExtFile, ".EXT");

	if (_vm->_inter->_terminate != 0)
		return;

	pushCollisions(0);
	playTot(skipPlay);

	if (_vm->_inter->_terminate != 2)
		_vm->_inter->_terminate = 0;

	popCollisions();

	_curBackupPos = curBackupPos;
	_backupedCount = backupedCount;
	_vm->_draw->_cursorHotspotXVar = _cursorHotspotXArray[_curBackupPos];
	_vm->_draw->_cursorHotspotYVar = _cursorHotspotYArray[_curBackupPos];
	_totTextData = _totTextDataArray[_curBackupPos];
	_script = _scriptArray[_curBackupPos];
	_totResourceTable = _totResourceTableArray[_curBackupPos];
	_extTable = _extTableArray[_curBackupPos];
	_extHandle = _extHandleArray[_curBackupPos];
	_imFileData = _imFileDataArray[_curBackupPos];
	_vm->_inter->_variables = _variablesArray[_curBackupPos];
	strcpy(_curTotFile, _curTotFileArray[_curBackupPos]);
	strcpy(_curExtFile, _curTotFile);
	_curExtFile[strlen(_curExtFile) - 4] = '\0';
	strcat(_curExtFile, ".EXT");
}

bool Game::getLocTextFile(char *locTextFile, int language) {
	int n = strlen(locTextFile);
	if (n < 4)
		return false;

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

	return _vm->_dataIO->existData(locTextFile);
}

byte *Game::loadLocTexts(int32 *dataSize) {
	char locTextFile[20];

	strcpy(locTextFile, _curTotFile);

	bool found = getLocTextFile(locTextFile, _vm->_global->_languageWanted);
	if (found) {

		_foundTotLoc = true;
		_vm->_global->_language = _vm->_global->_languageWanted;

	} else if (!_foundTotLoc) {
		// Trying US for GB and vice versa
		if (_vm->_global->_languageWanted == 2) {
			found = getLocTextFile(locTextFile, 5);
			if (found) {
				_vm->_global->_language = 5;
				found = true;
			}
		} else if (_vm->_global->_languageWanted == 5) {
			found = getLocTextFile(locTextFile, 2);
			if (found) {
				_vm->_global->_language = 2;
				found = true;
			}
		}

		if (!found) {
			// Looking for the first existing language
			for (int i = 0; i < 10; i++) {
				found = getLocTextFile(locTextFile, i);
				if (found) {
					_vm->_global->_language = i;
					break;
				}
			}
		}

	}

	debugC(1, kDebugFileIO, "Using language %d for %s",
			_vm->_global->_language, _curTotFile);

	if (found) {
		if (dataSize)
			*dataSize = _vm->_dataIO->getDataSize(locTextFile);

		return _vm->_dataIO->getData(locTextFile);
	}

	return 0;
}

void Game::setCollisions(byte arg_0) {
	uint16 left;
	uint16 top;
	uint16 width;
	uint16 height;
	Collision *collArea;

	for (collArea = _collisionAreas; collArea->left != 0xFFFF; collArea++) {
		if (((collArea->id & 0xC000) != 0x8000) || (collArea->funcSub == 0))
			continue;

		_script->call(collArea->funcSub);

		left = _script->readValExpr();
		top = _script->readValExpr();
		width = _script->readValExpr();
		height = _script->readValExpr();
		if ((_vm->_draw->_renderFlags & RENDERFLAG_CAPTUREPOP) &&
				(left != 0xFFFF)) {
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
		collArea->left = left;
		collArea->top = top;
		collArea->right = left + width - 1;
		collArea->bottom = top + height - 1;

		_script->pop();
	}
}

void Game::collSub(uint16 offset) {
	int16 collStackSize;

	_script->call(offset);

	_shouldPushColls = 1;
	collStackSize = _collStackSize;

	_vm->_inter->funcBlock(0);

	if (collStackSize != _collStackSize)
		popCollisions();

	_shouldPushColls = 0;

	_script->pop();

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

} // End of namespace Gob
