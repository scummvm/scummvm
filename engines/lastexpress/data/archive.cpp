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

#include "lastexpress/lastexpress.h"
#include "lastexpress/data/archive.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"

namespace LastExpress {

ArchiveManager::ArchiveManager(LastExpressEngine *engine) {
	_engine = engine;

	_cdFilePointer = new Common::File();
	_hdFilePointer = new Common::File();
}

ArchiveManager::~ArchiveManager() {
	SAFE_DELETE(_cdFilePointer);
	SAFE_DELETE(_hdFilePointer);
}

HPF *ArchiveManager::search(const char *name, HPF *archive, int archiveSize) {
	int left = 0;
	int right = archiveSize - 1;

	if (right < 0)
		return nullptr;

	while (left <= right) {
		int mid = (left + right) / 2;
		int compareResult = scumm_strnicmp(name, archive[mid].name, sizeof(archive[mid].name));

		if (compareResult < 0) {
			right = mid - 1;
		} else if (compareResult > 0) {
			left = mid + 1;
		} else { // Found it!
			return &archive[mid];
		}
	}

	return nullptr;
}

bool ArchiveManager::lockCD(int32 index) {
	char filename[80];

	if (_cdFilePointer && _cdArchiveIndex == index)
		return true;

	// Not present in the original, but we need it since we're about to reopen a new file...
	if (_cdFilePointer && _cdFilePointer->isOpen())
		_cdFilePointer->close();

	if (!isCDAvailable(index, filename, sizeof(filename)) || !lockCache(filename))
		return false;

	_cdArchiveIndex = index;
	return true;
}

bool ArchiveManager::isCDAvailable(int cdNum, char *outPath, int pathSize) {
	Common::sprintf_s(outPath, pathSize, "CD%d.hpf", cdNum);
	return true;
}

bool ArchiveManager::lockCache(char *filename) {
	uint32 remainingArchiveSize;
	uint32 curSubFilesNum;
	uint32 archiveIndex = 0;

	_cdFilePosition = 0;

	if (!_cdFilePointer || !_cdFilePointer->open(filename))
		return false;

	_cdArchiveNumFiles = _cdFilePointer->readUint32LE();

	if (_cdFilePointer->err()) {
		error("Error reading from file \"%s\"", filename);
	}

	remainingArchiveSize = _cdArchiveNumFiles;
	_cdFilePosition += 4;

	if (_cdArchiveNumFiles) {
		do {
			curSubFilesNum = 500;

			if (remainingArchiveSize <= 500)
				curSubFilesNum = remainingArchiveSize;

			// The original just did a mass fread of curSubFilesNum elements, with 22 as element size.
			// This is not quite portable as the bytes are directly associated to the struct, which might
			// have alignment differences, so instead we do it in the "ScummVM way"...
			for (uint i = 0; i < curSubFilesNum; i++) {
				_cdFilePointer->read(_cdArchive[archiveIndex + i].name, sizeof(_cdArchive[archiveIndex + i].name));
				_cdArchive[archiveIndex + i].offset = _cdFilePointer->readUint32LE();
				_cdArchive[archiveIndex + i].size = _cdFilePointer->readUint16LE();
				_cdArchive[archiveIndex + i].currentPos = _cdFilePointer->readUint16LE();
				_cdArchive[archiveIndex + i].status = _cdFilePointer->readUint16LE();
			}

			if (_cdFilePointer->err()) {
				error("Error reading from file \"%s\"", filename);
			}

			archiveIndex += curSubFilesNum;
			remainingArchiveSize -= curSubFilesNum;
			_engine->getSoundManager()->soundThread();
		} while (remainingArchiveSize);
	}

	_cdFilePosition += 22 * _cdArchiveNumFiles;

	return true;
}

void ArchiveManager::initHPFS() {
	_hdFilePosition = 0;

	if (!_hdFilePointer || !_hdFilePointer->open(_engine->isDemo() ? "DEMO.HPF" : "HD.hpf")) {
		error("Hard drive cache not found (please reinstall)");
	}

	_cdArchive = (HPF *)malloc(6500 * sizeof(HPF));

	if (!_cdArchive) {
		error("Out of memory");
	}

	_hdArchiveNumFiles = _hdFilePointer->readUint32LE();

	if (_hdFilePointer->err()) {
		error("Error reading from file \"%s\"", _engine->isDemo() ? "DEMO.HPF" : "HD.hpf");
	}

	_hdFilePosition += 4;
	_hdArchive = (HPF *)malloc(_hdArchiveNumFiles * sizeof(HPF));

	if (!_hdArchive) {
		error("Out of memory");
	}

	for (int i = 0; i < _hdArchiveNumFiles; i++) {
		_hdFilePointer->read(_hdArchive[i].name, sizeof(_hdArchive[i].name));
		_hdArchive[i].offset = _hdFilePointer->readUint32LE();
		_hdArchive[i].size = _hdFilePointer->readUint16LE();
		_hdArchive[i].currentPos = _hdFilePointer->readUint16LE();
		_hdArchive[i].status = _hdFilePointer->readUint16LE();
	}

	if (_hdFilePointer->err()) {
		error("Error reading from file \"%s\"", _engine->isDemo() ? "DEMO.HPF" : "HD.hpf");
	}

	_hdFilePosition += _hdArchiveNumFiles;
}

void ArchiveManager::shutDownHPFS() {
	unlockCD();

	if (_hdFilePointer)
		_hdFilePointer->close();

	if (_hdFilePointer && _hdFilePointer->isOpen()) {
		error("Error closing file \"%s\"", "HD cache file");
	}

	if (_hdArchive) {
		free(_hdArchive);
		_hdArchive = nullptr;
	}

	if (_cdArchive) {
		free(_cdArchive);
		_cdArchive = nullptr;
	}

	delete _hdFilePointer;
	_hdFilePointer = nullptr;

	_hdArchiveNumFiles = 0;
	_hdFilePosition = 0;
}

void ArchiveManager::unlockCD() {
	if (_cdFilePointer)
		_cdFilePointer->close();

	_cdArchiveNumFiles = 0;
	_cdFilePosition = 0;
	_cdArchiveIndex = 0;
}

HPF *ArchiveManager::openHPF(const char *filename) {
	HPF *result;
	Common::String filenameStr(filename);
	filenameStr.toUppercase();

	if (!_hdFilePointer)
		return nullptr;

	result = search(filenameStr.c_str(), _hdArchive, _hdArchiveNumFiles);
	if (!result) {
		if (_cdArchiveNumFiles)
			result = search(filenameStr.c_str(), _cdArchive, _cdArchiveNumFiles);

		if (!result)
			return nullptr;
	}

	if ((result->status & kHPFFileIsLoaded) != 0)
		return nullptr;

	result->status |= kHPFFileIsLoaded;
	result->currentPos = 0;

	return result;
}

void ArchiveManager::readHD(void *dstBuf, int offset, uint32 size) {
	if (_hdFilePointer && _hdFilePointer->isOpen()) {
		if (offset != _hdFilePosition) {	
			if (!_hdFilePointer->seek(offset * MEM_PAGE_SIZE, SEEK_SET)) {
				error("Error seeking in file \"%s\"", "HD cache file");
			}
		}

		uint32 readSize = _hdFilePointer->read(dstBuf, size * MEM_PAGE_SIZE);
		if (readSize != size * MEM_PAGE_SIZE) {
			error("Error reading from file \"%s\"", "HD cache file");
		}

		_hdFilePosition = (offset + size);
	}
}

void ArchiveManager::readCD(void *dstBuf, int offset, uint32 size) {
	if (_cdFilePointer && _cdFilePointer->isOpen()) {
		if (offset != _cdFilePosition) {
			if (!_cdFilePointer->seek(offset * MEM_PAGE_SIZE, SEEK_SET)) {
				error("Error seeking in file \"%s\"", "CD cache file");
			}
		}

		uint32 readSize = _cdFilePointer->read(dstBuf, size * MEM_PAGE_SIZE);
		if (readSize != size * MEM_PAGE_SIZE) {
			error("Error reading from file \"%s\"", "CD cache file");
		}

		_cdFilePosition = (offset + readSize);
	}
}

void ArchiveManager::readHPF(HPF *archive, void *dstBuf, uint32 size) {
	uint32 effSize;

	if ((archive->status & kHPFFileIsLoaded) != 0) {
		if (archive->size > archive->currentPos) {
			effSize = size;
			if (archive->currentPos + size > archive->size)
				effSize = archive->size - archive->currentPos;

			if ((archive->status & kHPFFileIsOnCD) != 0) {
				readHD(dstBuf, (archive->currentPos + archive->offset), effSize);
			} else {
				readCD(dstBuf, (archive->currentPos + archive->offset), effSize);
			}

			archive->currentPos += effSize;
		}
	}
}

void ArchiveManager::seekHPF(HPF *archive, uint32 position) {
	if ((archive->status & kHPFFileIsLoaded) != 0 && archive->size > position)
		archive->currentPos = position;
}

void ArchiveManager::closeHPF(HPF *archive) {
	if ((archive->status & kHPFFileIsLoaded) != 0) {
		archive->status &= ~kHPFFileIsLoaded;
	}
}

int ArchiveManager::loadBG(const char *filename) {
	TBM tbm;
	char bgFilename[84];

	memset(bgFilename, 0, sizeof(bgFilename));

	tbm.x = _engine->getGraphicsManager()->_renderBox1.x;
	tbm.y = _engine->getGraphicsManager()->_renderBox1.y;
	tbm.width = _engine->getGraphicsManager()->_renderBox1.width;
	tbm.height = _engine->getGraphicsManager()->_renderBox1.height;

	PixMap *bgSurface = _engine->getGraphicsManager()->_frontBuffer;

	if (_engine->getLogicManager()->_doubleClickFlag &&
		(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
		_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].property != kNodeAutoWalk) {
		return -1;
	}

	Common::strcpy_s(bgFilename, filename);
	Common::strcat_s(bgFilename, ".bg");

	HPF *archive = openHPF(bgFilename);

	if (!archive) {
		Common::strcpy_s(bgFilename, "DEFAULT.BG");
		archive = openHPF(bgFilename);
	}

	if (archive) {
		_engine->getGraphicsManager()->initDecomp(bgSurface, &_engine->getGraphicsManager()->_renderBox1);

		bool keepGoing = true;

		do {
			_engine->getSoundManager()->soundThread();
			_engine->getSubtitleManager()->subThread();

			readHPF(archive, _engine->getGraphicsManager()->_backgroundCompBuffer, 8);

			keepGoing = _engine->getGraphicsManager()->decomp16(_engine->getGraphicsManager()->_backgroundCompBuffer, 0x4000);
			keepGoing &= (!_engine->getLogicManager()->_doubleClickFlag ||
						 (!_engine->mouseHasLeftClicked() && !_engine->mouseHasRightClicked()) ||
						  _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].property == kNodeAutoWalk);
		} while (keepGoing);

		_engine->getGraphicsManager()->modifyPalette(bgSurface, 640 * 480);
		closeHPF(archive);

		if (_engine->getLogicManager()->_doubleClickFlag &&
			(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
			_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].property != kNodeAutoWalk) {
			return -1;
		} else {
			for (int32 i = _engine->getGraphicsManager()->_renderBox1.y - 1 + _engine->getGraphicsManager()->_renderBox1.height; i >= _engine->getGraphicsManager()->_renderBox1.y; i--) {
				memmove(
					&bgSurface[640 * i + _engine->getGraphicsManager()->_renderBox1.x],
					&bgSurface[_engine->getGraphicsManager()->_renderBox1.width * (i - _engine->getGraphicsManager()->_renderBox1.y)],
					2 * _engine->getGraphicsManager()->_renderBox1.width
				);
			}

			if (_engine->getGraphicsManager()->_renderBox1.x) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_frontBuffer, 0, 0, _engine->getGraphicsManager()->_renderBox1.x, 480);
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_frontBuffer, 640 - _engine->getGraphicsManager()->_renderBox1.x, 0, _engine->getGraphicsManager()->_renderBox1.x, 480);
			}

			if (_engine->getGraphicsManager()->_renderBox1.y) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_frontBuffer, _engine->getGraphicsManager()->_renderBox1.x, 0, _engine->getGraphicsManager()->_renderBox1.width, _engine->getGraphicsManager()->_renderBox1.y);
				_engine->getGraphicsManager()->clear(
					_engine->getGraphicsManager()->_frontBuffer,
					_engine->getGraphicsManager()->_renderBox1.x,
					480 - _engine->getGraphicsManager()->_renderBox1.y,
					_engine->getGraphicsManager()->_renderBox1.width,
					_engine->getGraphicsManager()->_renderBox1.y
				);
			}

			if (_engine->getLogicManager()->_doubleClickFlag &&
				(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
				_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_activeNode].property != kNodeAutoWalk) {
				return -1;

			} else {
				_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_frontBuffer, _engine->getGraphicsManager()->_backBuffer, 0, 0, 640, 480);

				if (tbm.x      != _engine->getGraphicsManager()->_renderBox1.x     ||
					tbm.y      != _engine->getGraphicsManager()->_renderBox1.y     ||
					tbm.width  != _engine->getGraphicsManager()->_renderBox1.width ||
					tbm.height != _engine->getGraphicsManager()->_renderBox1.height) {

					_engine->getGraphicsManager()->_renderBox2.x = tbm.x;
					_engine->getGraphicsManager()->_renderBox2.y = tbm.y;
					_engine->getGraphicsManager()->_renderBox2.width = tbm.width;
					_engine->getGraphicsManager()->_renderBox2.height = tbm.height;

					if (_engine->getGraphicsManager()->_renderBox2.x > _engine->getGraphicsManager()->_renderBox1.x)
						_engine->getGraphicsManager()->_renderBox2.x = _engine->getGraphicsManager()->_renderBox1.x;

					if (_engine->getGraphicsManager()->_renderBox2.y > _engine->getGraphicsManager()->_renderBox1.y)
						_engine->getGraphicsManager()->_renderBox2.y = _engine->getGraphicsManager()->_renderBox1.y;

					if (_engine->getGraphicsManager()->_renderBox1.height + _engine->getGraphicsManager()->_renderBox1.y > (_engine->getGraphicsManager()->_renderBox2.height + _engine->getGraphicsManager()->_renderBox2.y))
						_engine->getGraphicsManager()->_renderBox2.height = _engine->getGraphicsManager()->_renderBox1.height + _engine->getGraphicsManager()->_renderBox1.y - _engine->getGraphicsManager()->_renderBox2.y;

					if (_engine->getGraphicsManager()->_renderBox1.x + _engine->getGraphicsManager()->_renderBox1.width > (_engine->getGraphicsManager()->_renderBox2.x + _engine->getGraphicsManager()->_renderBox2.width))
						_engine->getGraphicsManager()->_renderBox2.width = _engine->getGraphicsManager()->_renderBox1.x + _engine->getGraphicsManager()->_renderBox1.width - _engine->getGraphicsManager()->_renderBox2.x;

					return 1;
				}

				return 0;
			}
		}
	} else {
		memset(_engine->getGraphicsManager()->_backBuffer, 0, (640 * 480 * sizeof(PixMap)));

		_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_backBuffer, _engine->getGraphicsManager()->_frontBuffer, 0, 0, 640, 480);
		_engine->getGraphicsManager()->_renderBox1.x = 0;
		_engine->getGraphicsManager()->_renderBox1.y = 0;
		_engine->getGraphicsManager()->_renderBox1.width = 640;
		_engine->getGraphicsManager()->_renderBox1.height = 480;

		_engine->getGraphicsManager()->_renderBox2.x = 0;
		_engine->getGraphicsManager()->_renderBox2.y = _engine->getGraphicsManager()->_renderBox1.y;
		_engine->getGraphicsManager()->_renderBox2.width = _engine->getGraphicsManager()->_renderBox1.width;
		_engine->getGraphicsManager()->_renderBox2.height = _engine->getGraphicsManager()->_renderBox1.height;

		return 1;
	}
}

Seq *ArchiveManager::loadSeq(const char *filename, uint8 ticksToWaitUntilCycleRestart, int character) {
	// Just like for LogicManager::loadTrain(), this function originally used
	// pointer fix-ups. This is maybe even worse in this case, since things like
	// the compressed data buffer don't have any size specified. This means that
	// other than the sequence itself we have to allocate and keep the raw file
	// data somewhere, in order to be actually able to deallocate it later...

	HPF *archive = openHPF(filename);
	if (!archive)
		return nullptr;

	byte *seqDataRaw = (byte *)_engine->getMemoryManager()->allocMem(MEM_PAGE_SIZE * archive->size, filename, character);
	if (!seqDataRaw)
		return nullptr;

	uint16 i;
	byte *seqDataRawCur = seqDataRaw;
	for (i = archive->size; i > 8; i -= 8) {
		_engine->getSoundManager()->soundThread();
		_engine->getSubtitleManager()->subThread();
		readHPF(archive, seqDataRawCur, 8);
		seqDataRawCur += (MEM_PAGE_SIZE * 8);
	}

	readHPF(archive, seqDataRawCur, i);
	closeHPF(archive);

	Seq *seq = new Seq();

	// Again, there is no such thing in the original...
	seq->rawSeqData = seqDataRaw;

	Common::SeekableReadStream *seqDataStream = new Common::MemoryReadStream(seqDataRaw, MEM_PAGE_SIZE * archive->size, DisposeAfterUse::NO);

	seq->numFrames = seqDataStream->readUint32LE();
	seqDataStream->readUint32LE(); // Empty sprite pointer
	seq->sprites = new Sprite[seq->numFrames];

	uint32 paletteOffset = 0;

	for (int frame = 0; frame < seq->numFrames; frame++) {
		uint32 compDataOffset = seqDataStream->readUint32LE();
		if (compDataOffset)
			seq->sprites[frame].compData = &seqDataRaw[compDataOffset];

		uint32 eraseDataOffset = seqDataStream->readUint32LE();
		if (eraseDataOffset)
			seq->sprites[frame].eraseMask = &seqDataRaw[eraseDataOffset];

		paletteOffset = seqDataStream->readUint32LE();
		if (paletteOffset)
			seq->sprites[frame].colorPalette = (uint16 *)&seqDataRaw[paletteOffset];

		seq->sprites[frame].rect.left = seqDataStream->readUint32LE();
		seq->sprites[frame].rect.top = seqDataStream->readUint32LE();
		seq->sprites[frame].rect.right = seqDataStream->readUint32LE();
		seq->sprites[frame].rect.bottom = seqDataStream->readUint32LE();
		seq->sprites[frame].rect.width = seqDataStream->readUint32LE();
		seq->sprites[frame].rect.height = seqDataStream->readUint32LE();

		seq->sprites[frame].hotspotX1 = seqDataStream->readUint16LE();
		seq->sprites[frame].hotspotX2 = seqDataStream->readUint16LE();
		seq->sprites[frame].hotspotY1 = seqDataStream->readUint16LE();
		seq->sprites[frame].hotspotY2 = seqDataStream->readUint16LE();

		seq->sprites[frame].compBits = seqDataStream->readByte();
		seq->sprites[frame].compType = seqDataStream->readByte();
		seq->sprites[frame].copyScreenAndRedrawFlag = seqDataStream->readByte();
		seq->sprites[frame].spritesUnk3 = seqDataStream->readByte();
		seq->sprites[frame].ticksToWaitUntilCycleRestart = seqDataStream->readByte();

		if (seq->sprites[frame].ticksToWaitUntilCycleRestart == 0)
			seq->sprites[frame].ticksToWaitUntilCycleRestart = ticksToWaitUntilCycleRestart;

		seq->sprites[frame].soundDelay = seqDataStream->readByte();
		seq->sprites[frame].soundAction = seqDataStream->readByte();
		seq->sprites[frame].flags = seqDataStream->readByte();
		seq->sprites[frame].position = seqDataStream->readByte();
		seq->sprites[frame].spritesUnk9 = seqDataStream->readByte();
		seq->sprites[frame].spritesUnk10 = seqDataStream->readByte();
		seq->sprites[frame].spritesUnk11 = seqDataStream->readByte();
		seq->sprites[frame].spritesUnk8 = (int)seqDataStream->readUint32LE();

		seq->sprites[frame].visibilityDist = seqDataStream->readUint16LE();
		seq->sprites[frame].hotspotPriority = seqDataStream->readUint16LE();

		seqDataStream->readUint32LE(); // Empty "next" sprite pointer
	}

	delete seqDataStream;

	// Where 68 is the original size of the Sprite struct and 8 is the
	// offset from the start of the sequence data to the begining of the
	// sprite data...
	uint16 *paletteAddr = (uint16 *)&seqDataRaw[8 + 68 * seq->numFrames]; 

	for (int j = 0; j < 184; j++) {
		paletteAddr[j] = READ_LE_UINT16(&paletteAddr[j]);
	}

	_engine->getGraphicsManager()->modifyPalette(paletteAddr, 184);
	return seq;
}

void ArchiveManager::loadMice() {
	HPF *archive = openHPF("CURSORS.TBM");

	if (archive) {
		readHPF(archive, _engine->_cursorsMemoryPool, archive->size);
		closeHPF(archive);

		for (int i = 0; i < 0xC000; i++) {
			_engine->getGraphicsManager()->_iconsBitmapData[i] = (PixMap)READ_LE_UINT16(&_engine->getGraphicsManager()->_iconsBitmapData[i]);
		}

		for (int i = 0; i < 48; i++) {
			_engine->getGraphicsManager()->_cursorsDataHeader[i].hotspotX = READ_LE_INT16(&_engine->getGraphicsManager()->_cursorsDataHeader[i].hotspotX);
			_engine->getGraphicsManager()->_cursorsDataHeader[i].hotspotY = READ_LE_INT16(&_engine->getGraphicsManager()->_cursorsDataHeader[i].hotspotY);
		}

		_engine->getGraphicsManager()->modifyPalette(_engine->getGraphicsManager()->_iconsBitmapData, 0xC000);
	}
}

} // End of namespace LastExpress
