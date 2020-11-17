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

	readIndexFile();
}

Resource::~Resource() {
	delete _macResFork;
}

void Resource::readIndexFile() {
	Common::SeekableReadStream *indexFile;

	if (_platform == Common::kPlatformAmiga) {
		indexFile = SearchMan.createReadStreamForMember("data000.dir");
	} else if (_platform == Common::kPlatformMacintosh) {
		indexFile = _macResFork->getResource("Directory");
	} else {
		indexFile = SearchMan.createReadStreamForMember("data.dir");
	}

	if (!indexFile)
		error("Could not open directory file");

	while (!indexFile->eos() && !indexFile->err()) {
		_resources.push_back(getIndexEntry(indexFile));
	}

	delete indexFile;
}

Common::List<ResourceIndex> Resource::searchIndex(Common::String filename) {
	Common::List<ResourceIndex> result;

	for (Common::List<ResourceIndex>::const_iterator i = _resources.begin(), end = _resources.end(); i != end; ++i) {
		if (i->fileName.contains(filename)) {
			result.push_back(*i);
		}
	}

	return result;
}

ResourceIndex Resource::getIndex(Common::String filename) {
	ResourceIndex index;

	for (Common::List<ResourceIndex>::const_iterator i = _resources.begin(), end = _resources.end(); i != end; ++i) {
		if (filename.matchString(i->fileName, true)) {
			index = *i;
			index.foundData = true;
			return index;
		}
	}

	return index;
}

ResourceIndex Resource::getIndexEntry(Common::SeekableReadStream *indexFile) {
	ResourceIndex index;

	Common::String currentFile;
	for (byte i = 0; i < 8; i++) {
		char c = indexFile->readByte();
		if (c)
			currentFile += c;
	}

	// The demo version has an empty entry in the end
	if (currentFile.size() == 0)
		return index;

	currentFile += '.';

	// Read extension
	for (byte i = 0; i < 3; i++)
		currentFile += indexFile->readByte();

	index.fileName = currentFile;

	if (_isDemo && _platform == Common::kPlatformDOS) {
		indexFile->readByte();                       // Always 0?
		index.fileCount = indexFile->readUint16LE(); // Always 1
		assert(index.fileCount == 1);
		index.indexOffset = indexFile->readUint32LE();
		index.uncompressedSize = indexFile->readUint16LE();
	} else {
		if (_platform == Common::kPlatformAmiga)
			index.indexOffset = (indexFile->readByte() << 16) + (indexFile->readByte() << 8) + indexFile->readByte();
		else
			index.indexOffset = indexFile->readByte() + (indexFile->readByte() << 8) + (indexFile->readByte() << 16);

		if (index.indexOffset & (1 << 23)) {
			index.fileCount = (index.indexOffset >> 16) & 0x7F;
			index.indexOffset = index.indexOffset & 0xFFFF;
			if (index.fileCount == 0)
				error("fileCount is 0 for %s", index.fileName.c_str());
		} else {
			index.fileCount = 1;
		}
	}

	return index;
}

// Files can be accessed "sequentially" if their filenames are the same except for
// the last character being incremented by one.
Common::MemoryReadStreamEndian *Resource::loadSequentialFile(Common::String filename, int fileIndex) {
	Common::String basename, extension;

	for (int i = filename.size() - 1;; i--) {
		if (filename[i] == '.') {
			basename = filename;
			extension = filename;
			basename.replace(i, filename.size() - i, "");
			extension.replace(0, i + 1, "");
			break;
		}
	}

	if ((basename.lastChar() >= '1' && basename.lastChar() <= '9') ||
	    (basename.lastChar() >= 'b' && basename.lastChar() <= 'z') ||
	    (basename.lastChar() >= 'B' && basename.lastChar() <= 'Z')) {
		basename.setChar(basename.lastChar() - 1, basename.size() - 1);
		return loadFile(basename + "." + extension, fileIndex + 1);
	} else {
		return nullptr;
	}
}

uint32 Resource::getSequentialFileOffset(uint32 offset, int fileIndex) {
	Common::SeekableReadStream *dataRunFile = SearchMan.createReadStreamForMember("data.run"); // FIXME: Amiga & Mac need this implemented
	if (!dataRunFile)
		error("Could not open sequential file");

	dataRunFile->seek(offset);

	offset = dataRunFile->readByte() + (dataRunFile->readByte() << 8) + (dataRunFile->readByte() << 16);
	//offset &= 0xFFFFFE;

	for (uint16 i = 0; i < fileIndex; i++) {
		offset += dataRunFile->readUint16LE();
	}

	delete dataRunFile;

	return offset;
}

/**
 * TODO:
 *   - This is supposed to cache results, return same FileStream on multiple accesses.
 */
Common::MemoryReadStreamEndian *Resource::loadFile(Common::String filename, int fileIndex, bool errorOnNotFound) {
	bool bigEndian = _platform == Common::kPlatformAmiga;

	// Load external patches
	if (Common::File::exists(filename)) {
		Common::File *patch = new Common::File();
		patch->open(filename);
		int32 size = patch->size();
		byte *data = (byte *)malloc(size);
		patch->read(data, size);
		delete patch;
		return new Common::MemoryReadStreamEndian(data, size, bigEndian, DisposeAfterUse::YES);
	}

	ResourceIndex index = getIndex(filename);

	if (!index.foundData) {
		Common::MemoryReadStreamEndian *result = loadSequentialFile(filename, fileIndex);
		if (result) {
			return result;
		} else {
			if (errorOnNotFound)
				error("Could not find file \'%s\'", filename.c_str());
			else
				return nullptr;
		}
	}

	if (fileIndex >= index.fileCount)
		error("Tried to access file index %d for file '%s', which doesn't exist.", fileIndex, filename.c_str());

	Common::SeekableReadStream *dataFile = 0;

	if (_platform == Common::kPlatformAmiga) {
		dataFile = SearchMan.createReadStreamForMember("data.000");
	} else if (_platform == Common::kPlatformMacintosh) {
		dataFile = _macResFork->getDataFork();
	} else {
		dataFile = SearchMan.createReadStreamForMember("data.001");
	}

	if (!dataFile)
		error("Could not open data file");

	if (index.fileCount != 1)
		index.indexOffset = getSequentialFileOffset(index.indexOffset, fileIndex);
	dataFile->seek(index.indexOffset);

	Common::SeekableReadStream *stream;
	if (_isDemo && _platform == Common::kPlatformDOS) {
		stream = dataFile->readStream(index.uncompressedSize);
	} else {
		uint16 uncompressedSize = bigEndian ? dataFile->readUint16BE() : dataFile->readUint16LE();
		uint16   compressedSize = bigEndian ? dataFile->readUint16BE() : dataFile->readUint16LE();
		stream = decodeLZSS(dataFile->readStream(compressedSize), uncompressedSize);
	}

	delete dataFile;

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
