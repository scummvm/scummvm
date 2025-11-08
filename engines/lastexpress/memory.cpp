/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lastexpress/memory.h"
#include "lastexpress/lastexpress.h"

#include "common/memstream.h"

namespace LastExpress {

MemoryManager::MemoryManager(LastExpressEngine *engine) {
	_engine = engine;
}

void MemoryManager::initMem() {
	// Memory structure of _globalMemoryPool
	// (total: 1800 pages = 3,686,400 bytes = 3.51 MB):
	//
	// +-------------------------------------------+
	// | Sound Memory Pool (pages 0-269)           |
	// | 270 pages = 552,960 bytes = 540 KB        |
	// | [0x0-0x86FFF]                             |
	// +-------------------------------------------+
	// | Background Buffer (pages 270-569)         |
	// | 300 pages = 614,400 bytes = 600 KB        |
	// | [0x87000-0x11CFFF]                        |
	// +-------------------------------------------+
	// | Generic Mem Pages Pool (pages 570-1799)   |
	// | 1230 pages = 2,519,040 bytes = 2.4 MB     |
	// | [0x11D000-0x389FFF]                       |
	// | Used for dynamic allocations of sequences |
	// | via allocMem                              |
	// +-------------------------------------------+
	//
	// Each page weighs MEM_PAGE_SIZE bytes (0x800 = 2048 bytes = 2 KB).

	_engine->_globalMemoryPool = (byte *)malloc(1800 * MEM_PAGE_SIZE);
	memset(_engine->_globalMemoryPool, 0, 1800 * MEM_PAGE_SIZE);

	_engine->getGraphicsManager()->_backBuffer = (PixMap *)malloc(640 * 480 * sizeof(PixMap));
	_engine->getLogicManager()->_trainData = (Node *)malloc(2048 * sizeof(Node));
	_engine->_cursorsMemoryPool = (byte *)malloc(49 * MEM_PAGE_SIZE);
	_engine->_characters = new Characters();
	_engine->getSubtitleManager()->_font = new FontData();
	_engine->getSubtitleManager()->_subtitlesData = (uint16 *)malloc(0x4400 * sizeof(uint16));
	_engine->getGraphicsManager()->_backgroundCompBuffer = (byte *)malloc(8 * MEM_PAGE_SIZE);
	_engine->getLogicManager()->_items = (Item *)malloc(32 * sizeof(Item));
	_engine->getLogicManager()->_doors = (Door *)malloc(128 * sizeof(Door));
	_engine->getLogicManager()->_blockedViews = (int32 *)malloc(1000 * sizeof(int32));
	_engine->getLogicManager()->_blockedX = (int32 *)malloc(16 * sizeof(int32));
	_engine->getLogicManager()->_softBlockedX = (int32 *)malloc(16 * sizeof(int32));
	_engine->getLogicManager()->_globals = (int32 *)malloc(128 * sizeof(int32));
	_engine->getLogicManager()->_doneNIS = (byte *)malloc(512);
	_engine->getMessageManager()->_autoMessages = (Message *)malloc(128 * sizeof(Message));

	for (int i = 0; i < 16; i++) {
		_engine->getLogicManager()->_blockedX[i] = 0;
		_engine->getLogicManager()->_softBlockedX[i] = 0;
	}

	for (int i = 0; i < 32; i++) {
		_engine->getLogicManager()->_items[i].clear();
	}

	for (int i = 0; i < 128; i++) {
		_engine->getLogicManager()->_doors[i].clear();
		_engine->getMessageManager()->_autoMessages[i].clear();
	}

	memset(_engine->getLogicManager()->_globals, 0, 128 * sizeof(int32));
	memset(_engine->getLogicManager()->_doneNIS, 0, 512);

	_engine->getGraphicsManager()->_cursorsDataHeader = (CursorHeader *)_engine->_cursorsMemoryPool;
	_engine->getGraphicsManager()->_iconsBitmapData = (PixMap *)(_engine->_cursorsMemoryPool + sizeof(CursorHeader) * kCursorMAX);
	_engine->_soundMemoryPool = _engine->_globalMemoryPool;
	_engine->getGraphicsManager()->_frontBuffer = (PixMap *)(_engine->_globalMemoryPool + (270 * MEM_PAGE_SIZE));

	_memoryPages[0].memPageSize = 1230 * MEM_PAGE_SIZE;
	_memoryPages[0].memPagePtr = _engine->_globalMemoryPool + (570 * MEM_PAGE_SIZE);
	_memoryPages[0].allocatedFlag = 0;

	_nisSeqMemFlag = kMemoryFlagSeqFree | kMemoryFlagInit;
	_nisSeqMemAvailForLocking = 1230 * MEM_PAGE_SIZE;
}

void *MemoryManager::allocMem(uint32 size, const char *name, int character) {
	void *memPagePtr = nullptr;
	uint32 sizeToAlloc = (size + 15) & 0xFFFFFFF0;

	for (int i = 0; !memPagePtr && i < _numAllocatedMemPages; i++) {
		if (!_memoryPages[i].allocatedFlag && _memoryPages[i].memPageSize >= sizeToAlloc) {
			if (_memoryPages[i].memPageSize > sizeToAlloc && _numAllocatedMemPages < 127) {
				memmove(&_memoryPages[i + 1], &_memoryPages[i], sizeof(MemPage) * (_numAllocatedMemPages - i));
				_numAllocatedMemPages++;
				_memoryPages[i].memPageSize = sizeToAlloc;
				_memoryPages[i + 1].memPageSize -= sizeToAlloc;
				_memoryPages[i].memPagePtr = (byte *)_memoryPages[i].memPagePtr + _memoryPages[i + 1].memPageSize;
			}

			_memoryPages[i].character = character;
			_memoryPages[i].allocatedFlag = true;
			Common::strcpy_s(_memoryPages[i].pageName, name);
			memPagePtr = _memoryPages[i].memPagePtr;
		}
	}

	return memPagePtr;
}

void MemoryManager::freeMem(void *data) {
	for (int i = 0; i < _numAllocatedMemPages; i++) {
		if (_memoryPages[i].memPagePtr == data) {
			_memoryPages[i].allocatedFlag = false;
			_memoryPages[i].character = 0;
			Common::strcpy_s(_memoryPages[i].pageName, "FREE MEM");

			if (i > 0 && !_memoryPages[i - 1].allocatedFlag) {
				i--;
				_memoryPages[i].memPageSize += _memoryPages[i + 1].memPageSize;
				_memoryPages[i].memPagePtr = _memoryPages[i + 1].memPagePtr;
				_numAllocatedMemPages--;
				memmove(&_memoryPages[i + 1], &_memoryPages[i + 2], sizeof(MemPage) * (_numAllocatedMemPages - i - 1));
			}

			if (i < _numAllocatedMemPages - 1 && !_memoryPages[i + 1].allocatedFlag) {
				_memoryPages[i].memPageSize += _memoryPages[i + 1].memPageSize;
				_memoryPages[i].memPagePtr = _memoryPages[i + 1].memPagePtr;
				_numAllocatedMemPages--;
				memmove(&_memoryPages[i + 1], &_memoryPages[i + 2], sizeof(MemPage) * (_numAllocatedMemPages - i - 1));
			}

			break;
		}
	}
}

void MemoryManager::releaseMemory() {
	SAFE_FREE(_engine->_globalMemoryPool);
	SAFE_FREE(_engine->getGraphicsManager()->_backBuffer);

	for (int i = 0; i < _engine->getLogicManager()->_numberOfScenes; i++) {
		if (_engine->getLogicManager()->_trainData[i].link) {
			SAFE_DELETE(_engine->getLogicManager()->_trainData[i].link);
		}
	}

	SAFE_FREE(_engine->getLogicManager()->_trainData);
	SAFE_FREE(_engine->_cursorsMemoryPool);
	SAFE_DELETE(_engine->_characters);
	SAFE_DELETE(_engine->getSubtitleManager()->_font);
	SAFE_FREE(_engine->getSubtitleManager()->_subtitlesData);
	SAFE_FREE(_engine->getGraphicsManager()->_backgroundCompBuffer);
	SAFE_FREE(_engine->getLogicManager()->_items);
	SAFE_FREE(_engine->getLogicManager()->_doors);
	SAFE_FREE(_engine->getLogicManager()->_blockedViews);
	SAFE_FREE(_engine->getLogicManager()->_blockedX);
	SAFE_FREE(_engine->getLogicManager()->_softBlockedX);
	SAFE_FREE(_engine->getLogicManager()->_globals);
	SAFE_FREE(_engine->getLogicManager()->_doneNIS);
}

void MemoryManager::freeFX() {
	if ((_nisSeqMemFlag & kMemoryFlagFXFree) == 0) {
		_nisSeqMemFlag |= kMemoryFlagFXFree;
		_memoryPages[_numAllocatedMemPages - 1].memPageSize += (300 * MEM_PAGE_SIZE);
		_memoryPages[_numAllocatedMemPages - 1].memPagePtr = (byte *)_memoryPages[_numAllocatedMemPages - 1].memPagePtr - (300 * MEM_PAGE_SIZE);
		_nisSeqMemAvailForLocking += 614400;
	}
}

void MemoryManager::lockFX() {
	if ((_nisSeqMemFlag & kMemoryFlagFXFree) != 0) {
		_nisSeqMemFlag &= ~kMemoryFlagFXFree;
		_memoryPages[_numAllocatedMemPages - 1].memPageSize -= (300 * MEM_PAGE_SIZE);
		_memoryPages[_numAllocatedMemPages - 1].memPagePtr = (byte *)_memoryPages[_numAllocatedMemPages - 1].memPagePtr + (300 * MEM_PAGE_SIZE);
		_nisSeqMemAvailForLocking -= (300 * MEM_PAGE_SIZE);
	}
}

void MemoryManager::lockSeqMem(uint32 size) {
	_engine->getOtisManager()->wipeLooseSprites();
	_nisSeqMemFlag &= ~kMemoryFlagSeqFree;
	uint32 curSize = 0;
	int idx = 1;

	while (curSize < size) {
		int invIdx = _numAllocatedMemPages - idx;

		if (_memoryPages[_numAllocatedMemPages - idx].allocatedFlag) {
			if (_memoryPages[invIdx].character) {
				void *memPagePtr = _memoryPages[invIdx].memPagePtr;
				_engine->getOtisManager()->wipeGSysInfo(_memoryPages[invIdx].character);

				for (int i = 0; i < _numAllocatedMemPages; ++i) {
					if (_memoryPages[i].memPagePtr == memPagePtr)
						freeMem(memPagePtr);
				}
			} else {
				freeMem(_memoryPages[invIdx].memPagePtr);
			}

			curSize = 0;
			idx = 1;
		} else {
			curSize += _memoryPages[invIdx].memPageSize;
			idx++;
		}
	}
}

void MemoryManager::freeSeqMem() {
	_nisSeqMemFlag |= kMemoryFlagSeqFree;
}

Seq *MemoryManager::copySeq(Seq *sequenceToCopy) {
	MemPage tmpPage;

	int oldSeqMemPageIdx = -1;
	for (int i = 0; oldSeqMemPageIdx == -1 && i < _numAllocatedMemPages; ++i) {
		if (_memoryPages[i].memPagePtr == sequenceToCopy->rawSeqData)
			oldSeqMemPageIdx = i;
	}

	if (oldSeqMemPageIdx == -1)
		return nullptr;

	tmpPage.copyFrom(_memoryPages[oldSeqMemPageIdx]);

	byte *newSeqDataRaw = (byte *)allocMem(tmpPage.memPageSize, tmpPage.pageName, tmpPage.character);
	if (!newSeqDataRaw)
		return nullptr;

	memcpy(newSeqDataRaw, tmpPage.memPagePtr, tmpPage.memPageSize);

	Seq *newSeq = new Seq();
	newSeq->rawSeqData = newSeqDataRaw;

	Common::SeekableReadStream *newSeqDataStream = new Common::MemoryReadStream(newSeqDataRaw, tmpPage.memPageSize, DisposeAfterUse::NO);

	newSeq->numFrames = newSeqDataStream->readUint32LE();
	newSeqDataStream->readUint32LE(); // Empty sprite pointer
	newSeq->sprites = new Sprite[newSeq->numFrames];

	uint32 paletteOffset = 0;

	for (int frame = 0; frame < newSeq->numFrames; frame++) {
		uint32 compDataOffset = newSeqDataStream->readUint32LE();
		if (compDataOffset)
			newSeq->sprites[frame].compData = &newSeqDataRaw[compDataOffset];

		uint32 eraseDataOffset = newSeqDataStream->readUint32LE();
		if (eraseDataOffset)
			newSeq->sprites[frame].eraseMask = &newSeqDataRaw[eraseDataOffset];

		paletteOffset = newSeqDataStream->readUint32LE();
		if (paletteOffset)
			newSeq->sprites[frame].colorPalette = (uint16 *)&newSeqDataRaw[paletteOffset];

		newSeq->sprites[frame].rect.left = newSeqDataStream->readUint32LE();
		newSeq->sprites[frame].rect.top = newSeqDataStream->readUint32LE();
		newSeq->sprites[frame].rect.right = newSeqDataStream->readUint32LE();
		newSeq->sprites[frame].rect.bottom = newSeqDataStream->readUint32LE();
		newSeq->sprites[frame].rect.width = newSeqDataStream->readUint32LE();
		newSeq->sprites[frame].rect.height = newSeqDataStream->readUint32LE();

		newSeq->sprites[frame].hotspotX1 = newSeqDataStream->readUint16LE();
		newSeq->sprites[frame].hotspotX2 = newSeqDataStream->readUint16LE();
		newSeq->sprites[frame].hotspotY1 = newSeqDataStream->readUint16LE();
		newSeq->sprites[frame].hotspotY2 = newSeqDataStream->readUint16LE();

		newSeq->sprites[frame].compBits = newSeqDataStream->readByte();
		newSeq->sprites[frame].compType = newSeqDataStream->readByte();
		newSeq->sprites[frame].copyScreenAndRedrawFlag = newSeqDataStream->readByte();
		newSeq->sprites[frame].spritesUnk3 = newSeqDataStream->readByte();
		newSeq->sprites[frame].ticksToWaitUntilCycleRestart = newSeqDataStream->readByte();

		// This variable could have been edited afterwards...
		if (newSeq->sprites[frame].ticksToWaitUntilCycleRestart != sequenceToCopy->sprites[frame].ticksToWaitUntilCycleRestart) {
			newSeq->sprites[frame].ticksToWaitUntilCycleRestart = sequenceToCopy->sprites[frame].ticksToWaitUntilCycleRestart;
		}

		newSeq->sprites[frame].soundDelay = newSeqDataStream->readByte();
		newSeq->sprites[frame].soundAction = newSeqDataStream->readByte();
		newSeq->sprites[frame].flags = newSeqDataStream->readByte();
		newSeq->sprites[frame].position = newSeqDataStream->readByte();
		newSeq->sprites[frame].spritesUnk9 = newSeqDataStream->readByte();
		newSeq->sprites[frame].spritesUnk10 = newSeqDataStream->readByte();
		newSeq->sprites[frame].spritesUnk11 = newSeqDataStream->readByte();
		newSeq->sprites[frame].spritesUnk8 = (int)newSeqDataStream->readUint32LE();

		newSeq->sprites[frame].visibilityDist = newSeqDataStream->readUint16LE();
		newSeq->sprites[frame].hotspotPriority = newSeqDataStream->readUint16LE();

		// This variable could have been edited afterwards...
		if (newSeq->sprites[frame].hotspotPriority != sequenceToCopy->sprites[frame].hotspotPriority) {
			newSeq->sprites[frame].hotspotPriority = sequenceToCopy->sprites[frame].hotspotPriority;
		}

		newSeqDataStream->readUint32LE(); // Empty "next" sprite pointer
	}

	delete newSeqDataStream;

	return newSeq;
}

} // End of namespace LastExpress
