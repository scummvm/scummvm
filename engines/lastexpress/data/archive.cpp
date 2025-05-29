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

// Based on the Xentax Wiki documentation:
// http://wiki.xentax.com/index.php/The_Last_Express_SND

#include "lastexpress/lastexpress.h"
#include "lastexpress/data/archive.h"
#include "lastexpress/debug.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"
#include "common/savefile.h"

namespace LastExpress {

HPFArchive::HPFArchive(const Common::Path &path) {
	_filename = path;

	// Open a stream to the archive
	Common::SeekableReadStream *archive = SearchMan.createReadStreamForMember(_filename);
	if (!archive) {
		debugC(2, kLastExpressDebugResource, "Error opening file: %s", path.toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	debugC(2, kLastExpressDebugResource, "Opened archive: %s", path.toString(Common::Path::kNativeSeparator).c_str());

	// Read header to get the number of files
	uint32 numFiles = archive->readUint32LE();
	debugC(3, kLastExpressDebugResource, "Number of files in archive: %d", numFiles);

	// Read the list of files
	for (unsigned int i = 0; i < numFiles; ++i) {
		char name[13];
		HPFEntry entry;

		archive->read(&name, sizeof(char) * _archiveNameSize);
		entry.offset = archive->readUint32LE();
		entry.size = archive->readUint32LE();
		entry.isOnHD = archive->readUint16LE();

		// Terminate string
		name[12] = '\0';

		Common::String filename(name);
		filename.toLowercase();

		_files[filename] = entry;

		//debugC(9, kLastExpressDebugResource, "File entry: %s (offset:%d - Size: %d - HD: %u)", &eraseData, entry.offset, entry.size, entry.isOnHD);
	}

	// Close stream
	delete archive;
}

bool HPFArchive::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	return (_files.find(name) != _files.end());
}

int HPFArchive::listMembers(Common::ArchiveMemberList &list) const {
	int numMembers = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
		numMembers++;
	}

	return numMembers;
}

const Common::ArchiveMemberPtr HPFArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *HPFArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	FileMap::const_iterator fDesc = _files.find(name);
	if (fDesc == _files.end())
		return nullptr;

	Common::File *archive = new Common::File();
	if (!archive->open(_filename)) {
		delete archive;
		return nullptr;
	}

	return new Common::SeekableSubReadStream(archive, fDesc->_value.offset * _archiveSectorSize, fDesc->_value.offset * _archiveSectorSize + fDesc->_value.size * _archiveSectorSize, DisposeAfterUse::YES);
}

// NEW ARCHIVE

ArchiveManager::ArchiveManager(LastExpressEngine *engine) {
	_engine = engine;

	g_CDFilePointer = new Common::File();
	g_HDFilePointer = new Common::File();
}

ArchiveManager::~ArchiveManager() {
	SAFE_DELETE(g_CDFilePointer);
	SAFE_DELETE(g_HDFilePointer);
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

	if (g_CDFilePointer && g_CDArchiveIndex == index)
		return true;

	// Not present in the original, but we need it since we're about to reopen a new file...
	if (g_CDFilePointer && g_CDFilePointer->isOpen())
		g_CDFilePointer->close();

	if (!isCDAvailable(index, filename, sizeof(filename)) || !lockCache(filename))
		return false;

	g_CDArchiveIndex = index;
	return true;
}

bool ArchiveManager::isCDAvailable(int cdNum, char *outPath, int pathSize) {
	Common::sprintf_s(outPath, pathSize, "CD%ld.hpf", cdNum);
	return true;
}

bool ArchiveManager::lockCache(char *filename) {
	uint32 remainingArchiveSize;
	uint32 curSubFilesNum;
	uint32 archiveIndex = 0;

	g_CDFilePosition = 0;

	if (!g_CDFilePointer || !g_CDFilePointer->open(filename))
		return false;

	g_CDArchiveNumFiles = g_CDFilePointer->readUint32LE();

	if (g_CDFilePointer->err()) {
		error("Error reading from file \"%s\"", filename);
	}

	remainingArchiveSize = g_CDArchiveNumFiles;
	g_CDFilePosition += 4;

	if (g_CDArchiveNumFiles) {
		do {
			curSubFilesNum = 500;

			if (remainingArchiveSize <= 500)
				curSubFilesNum = remainingArchiveSize;

			// The original just did a mass fread of curSubFilesNum elements, with 22 as element size.
			// This is not quite portable as the bytes are directly associated to the struct, which might
			// have alignment differences, so instead we do it in the "ScummVM way"...
			for (uint i = 0; i < curSubFilesNum; i++) {
				g_CDFilePointer->read(g_CDArchive[archiveIndex + i].name, sizeof(g_CDArchive[archiveIndex + i].name));
				g_CDArchive[archiveIndex + i].offset = g_CDFilePointer->readUint32LE();
				g_CDArchive[archiveIndex + i].size = g_CDFilePointer->readUint16LE();
				g_CDArchive[archiveIndex + i].currentPos = g_CDFilePointer->readUint16LE();
				g_CDArchive[archiveIndex + i].status = g_CDFilePointer->readUint16LE();
			}

			if (g_CDFilePointer->err()) {
				error("Error reading from file \"%s\"", filename);
			}

			archiveIndex += curSubFilesNum;
			remainingArchiveSize -= curSubFilesNum;
			_engine->getSoundManager()->soundThread();
		} while (remainingArchiveSize);
	}

	g_CDFilePosition += 22 * g_CDArchiveNumFiles;

	return true;
}

void ArchiveManager::initHPFS() {
	g_HDFilePosition = 0;

	if (!g_HDFilePointer || !g_HDFilePointer->open(_engine->isDemo() ? "DEMO.HPF" : "HD.hpf")) {
		error("Hard drive cache not found (please reinstall)");
	}

	g_CDArchive = (HPF *)malloc(6500 * sizeof(HPF));

	if (!g_CDArchive) {
		error("Out of memory");
	}

	g_HDArchiveNumFiles = g_HDFilePointer->readUint32LE();

	if (g_HDFilePointer->err()) {
		error("Error reading from file \"%s\"", _engine->isDemo() ? "DEMO.HPF" : "HD.hpf");
	}

	g_HDFilePosition += 4;
	g_HDArchive = (HPF *)malloc(g_HDArchiveNumFiles * sizeof(HPF));

	if (!g_HDArchive) {
		error("Out of memory");
	}

	for (int i = 0; i < g_HDArchiveNumFiles; i++) {
		g_HDFilePointer->read(g_HDArchive[i].name, sizeof(g_HDArchive[i].name));
		g_HDArchive[i].offset = g_HDFilePointer->readUint32LE();
		g_HDArchive[i].size = g_HDFilePointer->readUint16LE();
		g_HDArchive[i].currentPos = g_HDFilePointer->readUint16LE();
		g_HDArchive[i].status = g_HDFilePointer->readUint16LE();
	}

	if (g_HDFilePointer->err()) {
		error("Error reading from file \"%s\"", _engine->isDemo() ? "DEMO.HPF" : "HD.hpf");
	}

	g_HDFilePosition += g_HDArchiveNumFiles;
}

void ArchiveManager::shutDownHPFS() {
	unlockCD();

	if (g_HDFilePointer)
		g_HDFilePointer->close();

	if (g_HDFilePointer && g_HDFilePointer->isOpen()) {
		error("Error closing file \"%s\"", "HD cache file");
	}

	if (g_HDArchive) {
		free(g_HDArchive);
		g_HDArchive = nullptr;
	}

	if (g_CDArchive) {
		free(g_CDArchive);
		g_CDArchive = nullptr;
	}

	delete g_HDFilePointer;
	g_HDFilePointer = nullptr;

	g_HDArchiveNumFiles = 0;
	g_HDFilePosition = 0;
}

void ArchiveManager::unlockCD() {
	if (g_CDFilePointer)
		g_CDFilePointer->close();

	g_CDArchiveNumFiles = 0;
	g_CDFilePosition = 0;
	g_CDArchiveIndex = 0;
}

HPF *ArchiveManager::openHPF(const char *filename) {
	HPF *result;
	Common::String filenameStr(filename);
	filenameStr.toUppercase();

	if (!g_HDFilePointer)
		return nullptr;

	result = search(filenameStr.c_str(), g_HDArchive, g_HDArchiveNumFiles);
	if (!result) {
		if (g_CDArchiveNumFiles)
			result = search(filenameStr.c_str(), g_CDArchive, g_CDArchiveNumFiles);

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
	if (g_HDFilePointer && g_HDFilePointer->isOpen()) {
		if (offset != g_HDFilePosition) {	
			if (!g_HDFilePointer->seek(offset * PAGE_SIZE, SEEK_SET)) {
				error("Error seeking in file \"%s\"", "HD cache file");
			}
		}

		uint32 readSize = g_HDFilePointer->read(dstBuf, size * PAGE_SIZE);
		if (readSize != size * PAGE_SIZE) {
			error("Error reading from file \"%s\"", "HD cache file");
		}

		g_HDFilePosition = (offset + size);
	}
}

void ArchiveManager::readCD(void *dstBuf, int offset, uint32 size) {
	if (g_CDFilePointer && g_CDFilePointer->isOpen()) {
		if (offset != g_CDFilePosition) {
			if (!g_CDFilePointer->seek(offset * PAGE_SIZE, SEEK_SET)) {
				error("Error seeking in file \"%s\"", "CD cache file");
			}
		}

		uint32 readSize = g_CDFilePointer->read(dstBuf, size * PAGE_SIZE);
		if (readSize != size * PAGE_SIZE) {
			error("Error reading from file \"%s\"", "CD cache file");
		}

		g_CDFilePosition = (offset + readSize);
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

	PixMap *bgSurface = _engine->getGraphicsManager()->_backgroundBuffer;

	if (_engine->getLogicManager()->_doubleClickFlag &&
		(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
		_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].car != 128) {
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
						 _engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].car == 0x80);
		} while (keepGoing);

		_engine->getGraphicsManager()->modifyPalette(bgSurface, 640 * 480);
		closeHPF(archive);

		if (_engine->getLogicManager()->_doubleClickFlag &&
			(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
			_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].car != 128) {
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
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_backgroundBuffer, 0, 0, _engine->getGraphicsManager()->_renderBox1.x, 480);
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_backgroundBuffer, 640 - _engine->getGraphicsManager()->_renderBox1.x, 0, _engine->getGraphicsManager()->_renderBox1.x, 480);
			}

			if (_engine->getGraphicsManager()->_renderBox1.y) {
				_engine->getGraphicsManager()->clear(_engine->getGraphicsManager()->_backgroundBuffer, _engine->getGraphicsManager()->_renderBox1.x, 0, _engine->getGraphicsManager()->_renderBox1.width, _engine->getGraphicsManager()->_renderBox1.y);
				_engine->getGraphicsManager()->clear(
					_engine->getGraphicsManager()->_backgroundBuffer,
					_engine->getGraphicsManager()->_renderBox1.x,
					480 - _engine->getGraphicsManager()->_renderBox1.y,
					_engine->getGraphicsManager()->_renderBox1.width,
					_engine->getGraphicsManager()->_renderBox1.y
				);
			}

			if (_engine->getLogicManager()->_doubleClickFlag &&
				(_engine->mouseHasLeftClicked() || _engine->mouseHasRightClicked()) &&
				_engine->getLogicManager()->_trainData[_engine->getLogicManager()->_trainNodeIndex].car != 128) {
				return -1;

			} else {
				_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_backgroundBuffer, _engine->getGraphicsManager()->_screenBuffer, 0, 0, 640, 480);

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
		memset(_engine->getGraphicsManager()->_screenBuffer, 0, (640 * 480 * sizeof(PixMap)));

		_engine->getGraphicsManager()->copy(_engine->getGraphicsManager()->_screenBuffer, _engine->getGraphicsManager()->_backgroundBuffer, 0, 0, 640, 480);
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

	byte *seqDataRaw = (byte *)_engine->getMemoryManager()->allocMem(PAGE_SIZE * archive->size, filename, character);
	if (!seqDataRaw)
		return nullptr;

	uint16 i;
	byte *seqDataRawCur = seqDataRaw;
	for (i = archive->size; i > 8; i -= 8) {
		_engine->getSoundManager()->soundThread();
		_engine->getSubtitleManager()->subThread();
		readHPF(archive, seqDataRawCur, 8);
		seqDataRawCur += (PAGE_SIZE * 8);
	}

	readHPF(archive, seqDataRawCur, i);
	closeHPF(archive);

	Seq *seq = new Seq();

	// Again, there is no such thing in the original...
	seq->rawSeqData = seqDataRaw;

	Common::SeekableReadStream *seqDataStream = new Common::MemoryReadStream(seqDataRaw, PAGE_SIZE * archive->size, DisposeAfterUse::NO);

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

	// Where 68 is the original size of the Sprite struct and 8 is the
	// offset from the start of the sequence data to the begining of the
	// sprite data...
	uint16 *paletteAddr = (uint16 *)&seqDataRaw[8 + 68 * seq->numFrames]; 

	_engine->getGraphicsManager()->modifyPalette(paletteAddr, 184);
	return seq;
}

void ArchiveManager::loadMice() {
	HPF *archive = openHPF("CURSORS.TBM");

	if (archive) {
		readHPF(archive, _engine->_cursorsMemoryPool, archive->size);
		closeHPF(archive);
		_engine->getGraphicsManager()->modifyPalette(_engine->getGraphicsManager()->_iconsBitmapData, 0xC000);
	}
}

} // End of namespace LastExpress
