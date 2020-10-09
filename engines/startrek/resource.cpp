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
 */


#include "common/file.h"
#include "common/macresman.h"

#include "engines/util.h"
#include "video/qt_decoder.h"

#include "startrek/iwfile.h"
#include "startrek/lzss.h"
#include "startrek/resource.h"

namespace StarTrek {

Resource::Resource(Common::Platform platform, bool isDemo) : _platform(platform), _isDemo(isDemo), _macResFork(nullptr) {
	if (_platform == Common::kPlatformMacintosh) {
		_macResFork = new Common::MacResManager();
		if (!_macResFork->open("Star Trek Data"))
			error("Could not load Star Trek Data");
		assert(_macResFork->hasDataFork() && _macResFork->hasResFork());
	}
}

Resource::~Resource() {
	delete _macResFork;
}

/**
 * TODO:
 *   - Should return nullptr on failure to open a file?
 *   - This is supposed to cache results, return same FileStream on multiple accesses.
 *   - This is supposed to read from a "patches" folder which overrides files in the
 *     packed blob.
 */
Common::MemoryReadStreamEndian *Resource::loadFile(Common::String filename, int fileIndex) {
	filename.toUppercase();

	Common::String basename, extension;

	bool bigEndian = _platform == Common::kPlatformAmiga;

	for (int i = filename.size() - 1; ; i--) {
		if (filename[i] == '.') {
			basename = filename;
			extension = filename;
			basename.replace(i, filename.size() - i, "");
			extension.replace(0, i + 1, "");
			break;
		}
	}

	// FIXME: don't know if this is right, or if it goes here
	while (!basename.empty() && basename.lastChar() == ' ') {
		basename.erase(basename.size() - 1, 1);
	}

	filename = basename + '.' + extension;

	// TODO: Re-enable this when more work has been done on the demo
	/*
	// The Judgment Rites demo has its files not in the standard archive
	if (getGameType() == GType_STJR && _isDemo) {
		Common::File *file = new Common::File();
		if (!file->open(filename.c_str())) {
			delete file;
			error("Could not find file \'%s\'", filename.c_str());
		}
		int32 size = file->size();
		byte *data = (byte *)malloc(size);
		file->read(data, size);
		delete file;
		return new Common::MemoryReadStreamEndian(data, size, bigEndian, DisposeAfterUse::YES);
	}
	*/

	Common::SeekableReadStream *indexFile = 0;

	if (_platform == Common::kPlatformAmiga) {
		indexFile = SearchMan.createReadStreamForMember("data000.dir");
		if (!indexFile)
			error("Could not open data000.dir");
	} else if (_platform == Common::kPlatformMacintosh) {
		indexFile = _macResFork->getResource("Directory");
		if (!indexFile)
			error("Could not find 'Directory' resource in 'Star Trek Data'");
	} else {
		indexFile = SearchMan.createReadStreamForMember("data.dir");
		if (!indexFile)
			error("Could not open data.dir");
	}

	uint32 indexOffset = 0;
	bool foundData = false;
	uint16 fileCount = 1;
	uint16 uncompressedSize = 0;

	while (!indexFile->eos() && !indexFile->err()) {
		Common::String testfile;
		for (byte i = 0; i < 8; i++) {
			char c = indexFile->readByte();
			if (c)
				testfile += c;
		}
		testfile += '.';

		for (byte i = 0; i < 3; i++)
			testfile += indexFile->readByte();

		if (_isDemo && _platform == Common::kPlatformDOS) {
			indexFile->readByte(); // Always 0?
			fileCount = indexFile->readUint16LE(); // Always 1
			indexOffset = indexFile->readUint32LE();
			uncompressedSize = indexFile->readUint16LE();
		} else {
			if (_platform == Common::kPlatformAmiga)
				indexOffset = (indexFile->readByte() << 16) + (indexFile->readByte() << 8) + indexFile->readByte();
			else
				indexOffset = indexFile->readByte() + (indexFile->readByte() << 8) + (indexFile->readByte() << 16);

			if (indexOffset & (1 << 23)) {
				fileCount = (indexOffset >> 16) & 0x7F;
				indexOffset = indexOffset & 0xFFFF;
				assert(fileCount > 1);
			} else {
				fileCount = 1;
			}
		}

		if (filename.matchString(testfile)) {
			foundData = true;
			break;
		}
	}

	delete indexFile;

	if (!foundData) {
		// Files can be accessed "sequentially" if their filenames are the same except for
		// the last character being incremented by one.
		if ((basename.lastChar() >= '1' && basename.lastChar() <= '9') ||
		        (basename.lastChar() >= 'B' && basename.lastChar() <= 'Z')) {
			basename.setChar(basename.lastChar() - 1, basename.size() - 1);
			return loadFile(basename + "." + extension, fileIndex + 1);
		} else
			error("Could not find file \'%s\'", filename.c_str());
	}

	if (fileIndex >= fileCount)
		error("Tried to access file index %d for file '%s' which doesn't exist.", fileIndex, filename.c_str());

	Common::SeekableReadStream *dataFile = 0;
	Common::SeekableReadStream *dataRunFile = 0; // FIXME: Amiga & Mac need this implemented

	if (_platform == Common::kPlatformAmiga) {
		dataFile = SearchMan.createReadStreamForMember("data.000");
		if (!dataFile)
			error("Could not open data.000");
	} else if (_platform == Common::kPlatformMacintosh) {
		dataFile = _macResFork->getDataFork();
		if (!dataFile)
			error("Could not get 'Star Trek Data' data fork");
	} else {
		dataFile = SearchMan.createReadStreamForMember("data.001");
		if (!dataFile)
			error("Could not open data.001");
		dataRunFile = SearchMan.createReadStreamForMember("data.run");
		if (!dataFile)
			error("Could not open data.run");
	}

	Common::SeekableReadStream *stream;
	if (_isDemo && _platform == Common::kPlatformDOS) {
		assert(fileCount == 1); // Sanity check...
		stream = dataFile->readStream(uncompressedSize);
	} else {
		if (fileCount != 1) {
			dataRunFile->seek(indexOffset);

			indexOffset = dataRunFile->readByte() + (dataRunFile->readByte() << 8) + (dataRunFile->readByte() << 16);
			//indexOffset &= 0xFFFFFE;

			for (uint16 i = 0; i < fileIndex; i++) {
				uint16 size = dataRunFile->readUint16LE();
				indexOffset += size;
			}
		}
		dataFile->seek(indexOffset);

		uncompressedSize = (_platform == Common::kPlatformAmiga) ? dataFile->readUint16BE() : dataFile->readUint16LE();
		uint16 compressedSize = (_platform == Common::kPlatformAmiga) ? dataFile->readUint16BE() : dataFile->readUint16LE();

		stream = decodeLZSS(dataFile->readStream(compressedSize), uncompressedSize);
	}

	delete dataFile;
	delete dataRunFile;

	int32 size = stream->size();
	byte *data = (byte *)malloc(size);
	stream->read(data, size);
	delete stream;

	return new Common::MemoryReadStreamEndian(data, size, bigEndian, DisposeAfterUse::YES);
}

Common::MemoryReadStreamEndian *Resource::loadBitmapFile(Common::String baseName) {
	return loadFile(baseName + ".BMP");
}

Common::MemoryReadStreamEndian *Resource::loadFileWithParams(Common::String filename, bool unk1, bool unk2, bool unk3) {
	return loadFile(filename);
}

Common::String Resource::getLoadedText(int textIndex) {
	Common::MemoryReadStreamEndian *txtFile = loadFile(_txtFilename + ".txt");

	Common::String str;
	byte cur;
	int curIndex = 0;

	while (!txtFile->eos()) {
		do {
			cur = txtFile->readByte();
			str += cur;
		} while (cur != '\0');

		if (curIndex == textIndex) {
			delete txtFile;
			return str;
		}

		curIndex++;
		str = "";
	}
	
	delete txtFile;
	return "";
}

} // End of namespace StarTrek
