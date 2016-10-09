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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/resources.h"
#include "sherlock/screen.h"
#include "sherlock/sherlock.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace Sherlock {

Cache::Cache(SherlockEngine *vm) : _vm(vm) {
}

bool Cache::isCached(const Common::String &filename) const {
	return _resources.contains(filename);
}

void Cache::load(const Common::String &name) {
	// First check if the entry already exists
	if (_resources.contains(name))
		return;

	// Open the file for reading
	Common::File f;
	if (!f.open(name))
		error("Could not read file - %s", name.c_str());

	load(name, f);

	f.close();
}

void Cache::load(const Common::String &name, Common::SeekableReadStream &stream) {
	// First check if the entry already exists
	if (_resources.contains(name))
		return;

	int32 signature = stream.readUint32BE();
	stream.seek(0);

	// Allocate a new cache entry
	_resources[name] = CacheEntry();
	CacheEntry &cacheEntry = _resources[name];

	// Check whether the file is compressed
	if (signature == MKTAG('L', 'Z', 'V', 26)) {
		// It's compressed, so decompress the file and store its data in the cache entry
		Common::SeekableReadStream *decompressed = _vm->_res->decompress(stream);
		cacheEntry.resize(decompressed->size());
		decompressed->read(&cacheEntry[0], decompressed->size());

		delete decompressed;
	} else {
		// It's not, so read the raw data of the file into the cache entry
		cacheEntry.resize(stream.size());
		stream.read(&cacheEntry[0], stream.size());
	}
}

Common::SeekableReadStream *Cache::get(const Common::String &filename) const {
	// Return a memory stream that encapsulates the data
	const CacheEntry &cacheEntry = _resources[filename];
	return new Common::MemoryReadStream(&cacheEntry[0], cacheEntry.size());
}

/*----------------------------------------------------------------*/

Resources::Resources(SherlockEngine *vm) : _vm(vm), _cache(vm) {
	_resourceIndex = -1;

	if (_vm->_interactiveFl) {
		if (!IS_3DO) {
			addToCache("vgs.lib");
			addToCache("talk.lib");
			addToCache("journal.txt");

			if (IS_SERRATED_SCALPEL) {
				addToCache("sequence.txt");
				addToCache("portrait.lib");
			} else {
				addToCache("walk.lib");
			}
		} else {
			// 3DO

			// ITEM data from VGS.LIB is in ITEM.LIB
			addToCache("item.lib");

			// talk.lib - resources themselves seem to be the same, although a few texts were slightly changed
			addToCache("talk.lib");

			// remaining files are missing
			// portraits were replaced with FMV
		}
	}
}

void Resources::addToCache(const Common::String &filename) {
	// Return immediately if the library has already been loaded
	if (_indexes.contains(filename))
		return;

	_cache.load(filename);

	// Check to see if the file is a library
	Common::SeekableReadStream *stream = load(filename);
	uint32 header = stream->readUint32BE();

	if (header == MKTAG('L', 'I', 'B', 26))
		loadLibraryIndex(filename, stream, false);
	else if (header == MKTAG('L', 'I', 'C', 26))
		loadLibraryIndex(filename, stream, true);

	delete stream;
}

void Resources::addToCache(const Common::String &filename, const Common::String &libFilename) {
	// Get the resource
	Common::SeekableReadStream *stream = load(filename, libFilename);

	_cache.load(filename, *stream);

	delete stream;
}

void Resources::addToCache(const Common::String &filename, Common::SeekableReadStream &stream) {
	_cache.load(filename, stream);
}

Common::SeekableReadStream *Resources::load(const Common::String &filename) {
	// First check if the file is directly in the cache
	if (_cache.isCached(filename))
		return _cache.get(filename);

	// Secondly, iterate through any loaded library file looking for a resource
	// that has the same name
	for (LibraryIndexes::iterator i = _indexes.begin(); i != _indexes.end(); ++i) {
		if (i->_value.contains(filename)) {
			// Get a stream reference to the given library file
			Common::SeekableReadStream *stream = load(i->_key);
			LibraryEntry &entry = i->_value[filename];
			_resourceIndex = entry._index;

			stream->seek(entry._offset);
			Common::SeekableReadStream *resStream = stream->readStream(entry._size);
			decompressIfNecessary(resStream);

			delete stream;
			return resStream;
		}
	}

	// At this point, fall back on a physical file with the given name
	Common::File f;
	if (!f.open(filename))
		error("Could not load file - %s", filename.c_str());

	Common::SeekableReadStream *stream = f.readStream(f.size());
	f.close();
	decompressIfNecessary(stream);

	return stream;
}

void Resources::decompressIfNecessary(Common::SeekableReadStream *&stream) {
	bool isCompressed = stream->readUint32BE() == MKTAG('L', 'Z', 'V', 26);

	if (isCompressed) {
		int outSize = stream->readUint32LE();
		Common::SeekableReadStream *newStream = decompressLZ(*stream, outSize);
		delete stream;
		stream = newStream;
	} else {
		stream->seek(-4, SEEK_CUR);
	}
}

Common::SeekableReadStream *Resources::load(const Common::String &filename, const Common::String &libraryFile,
		bool suppressErrors) {
	// Open up the library for access
	Common::SeekableReadStream *libStream = load(libraryFile);

	// Check if the library has already had its index read, and if not, load it
	if (!_indexes.contains(libraryFile))
		loadLibraryIndex(libraryFile, libStream, false);
	LibraryIndex &libIndex = _indexes[libraryFile];

	// Handle if resource is not present
	if (!libIndex.contains(filename)) {
		if (!suppressErrors)
			error("Could not find resource - %s", filename.c_str());

		delete libStream;
		return nullptr;
	}

	// Extract the data for the specified resource and return it
	LibraryEntry &entry = libIndex[filename];
	libStream->seek(entry._offset);
	Common::SeekableReadStream *stream = libStream->readStream(entry._size);
	decompressIfNecessary(stream);

	delete libStream;
	return stream;
}

bool Resources::exists(const Common::String &filename) const {
	Common::File f;
	return f.exists(filename) || _cache.isCached(filename);
}

void Resources::loadLibraryIndex(const Common::String &libFilename,
		Common::SeekableReadStream *stream, bool isNewStyle) {
	uint32 offset, nextOffset;

	// Return immediately if the library has already been loaded
	if (_indexes.contains(libFilename))
		return;

	// Create an index entry
	_indexes[libFilename] = LibraryIndex();
	LibraryIndex &index = _indexes[libFilename];

	// Read in the number of resources
	stream->seek(4);
	int count = 0;

	if (!IS_3DO) {
		// PC
		count = stream->readUint16LE();

		if (isNewStyle)
			stream->seek((count + 1) * 8, SEEK_CUR);

		// Loop through reading in the entries
		for (int idx = 0; idx < count; ++idx) {
			// Read the name of the resource
			char resName[13];
			stream->read(resName, 13);
			resName[12] = '\0';

			// Read the offset
			offset = stream->readUint32LE();

			if (idx == (count - 1)) {
				nextOffset = stream->size();
			} else {
				// Read the size by jumping forward to read the next entry's offset
				stream->seek(13, SEEK_CUR);
				nextOffset = stream->readUint32LE();
				stream->seek(-17, SEEK_CUR);
			}

			// Add the entry to the index
			index[resName] = LibraryEntry(idx, offset, nextOffset - offset);
		}

	} else {
		// 3DO
		count = stream->readUint16BE();

		// 3DO header
		// Loop through reading in the entries

		// Read offset of first entry
		offset = stream->readUint32BE();

		for (int idx = 0; idx < count; ++idx) {

			// Read the name of the resource
			char resName[13];
			stream->read(resName, 13);
			resName[12] = '\0';

			stream->skip(3); // filler

			if (idx == (count - 1)) {
				nextOffset = stream->size();
			} else {
				// Read the offset of the next entry
				nextOffset = stream->readUint32BE();
			}

			// Add the entry to the index
			index[resName] = LibraryEntry(idx, offset, nextOffset - offset);

			// use next offset as current offset
			offset = nextOffset;
		}
	}
}

int Resources::resourceIndex() const {
	return _resourceIndex;
}

void Resources::getResourceNames(const Common::String &libraryFile, Common::StringArray &names) {
	addToCache(libraryFile);
	LibraryIndex &libIndex = _indexes[libraryFile];
	for (LibraryIndex::iterator i = libIndex.begin(); i != libIndex.end(); ++i) {
		names.push_back(i->_key);
	}
}

Common::SeekableReadStream *Resources::decompress(Common::SeekableReadStream &source) {
	// This variation can't be used by Rose Tattoo, since compressed resources include the input size,
	// not the output size. Which means their decompression has to be done via passed buffers
	assert(IS_SERRATED_SCALPEL);

	uint32 id = source.readUint32BE();
	assert(id == MKTAG('L', 'Z', 'V', 0x1A));

	uint32 outputSize = source.readUint32LE();
	return decompressLZ(source, outputSize);
}

Common::SeekableReadStream *Resources::decompress(Common::SeekableReadStream &source, uint32 outSize) {
	int inSize = IS_ROSE_TATTOO ? source.readSint32LE() : -1;
	byte *outBuffer = (byte *)malloc(outSize);
	Common::MemoryReadStream *outStream = new Common::MemoryReadStream(outBuffer, outSize, DisposeAfterUse::YES);

	decompressLZ(source, outBuffer, outSize, inSize);

	return outStream;
}

void Resources::decompress(Common::SeekableReadStream &source, byte *buffer, uint32 outSize) {
	int inputSize = IS_ROSE_TATTOO ? source.readSint32LE() : -1;

	decompressLZ(source, buffer, outSize, inputSize);
}

Common::SeekableReadStream *Resources::decompressLZ(Common::SeekableReadStream &source, uint32 outSize) {
	byte *dataOut = (byte *)malloc(outSize);
	decompressLZ(source, dataOut, outSize, -1);

	return new Common::MemoryReadStream(dataOut, outSize, DisposeAfterUse::YES);
}

void Resources::decompressLZ(Common::SeekableReadStream &source, byte *outBuffer, int32 outSize, int32 inSize) {
	byte lzWindow[4096];
	uint16 lzWindowPos;
	uint16 cmd;

	byte *outBufferEnd = outBuffer + outSize;
	int endPos = source.pos() + inSize;

	memset(lzWindow, 0xFF, 0xFEE);
	lzWindowPos = 0xFEE;
	cmd = 0;

	do {
		cmd >>= 1;
		if (!(cmd & 0x100))
			cmd = source.readByte() | 0xFF00;

		if (cmd & 1) {
			byte literal = source.readByte();
			*outBuffer++ = literal;
			lzWindow[lzWindowPos] = literal;
			lzWindowPos = (lzWindowPos + 1) & 0x0FFF;
		} else {
			int copyPos, copyLen;
			copyPos = source.readByte();
			copyLen = source.readByte();
			copyPos = copyPos | ((copyLen & 0xF0) << 4);
			copyLen = (copyLen & 0x0F) + 3;
			while (copyLen--) {
				byte literal = lzWindow[copyPos];
				copyPos = (copyPos + 1) & 0x0FFF;
				*outBuffer++ = literal;
				lzWindow[lzWindowPos] = literal;
				lzWindowPos = (lzWindowPos + 1) & 0x0FFF;
			}
		}
	} while ((outSize == -1 || outBuffer < outBufferEnd) && (inSize == -1 || source.pos() < endPos));
}

} // End of namespace Sherlock
