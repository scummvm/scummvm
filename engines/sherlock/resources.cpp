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

#include "sherlock/resources.h"
#include "sherlock/decompress.h"
#include "common/debug.h"

namespace Sherlock {

Cache::Cache() {
}

/**
 * Returns true if a given file is currently being cached
 */
bool Cache::isCached(const Common::String &filename) const {
	return _resources.contains(filename);
}

/**
 * Loads a file into the cache if it's not already present, and returns it.
 * If the file is LZW compressed, automatically decompresses it and loads
 * the uncompressed version into memory
 */
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

/**
 * Load a cache entry based on a passed stream
 */
void Cache::load(const Common::String &name, Common::SeekableReadStream &stream) {
	// First check if the entry already exists
	if (_resources.contains(name))
		return;

	// Check whether the file is compressed
	const char LZW_HEADER[5] = { "LZV\x1a" };
	char header[5];
	stream.read(header, 5);
	bool isCompressed = !strncmp(header, LZW_HEADER, 5);
	stream.seek(0);

	// Allocate a new cache entry
	_resources[name] = CacheEntry();
	CacheEntry &cacheEntry = _resources[name];

	if (isCompressed) {
		// It's compressed, so decompress the file and store it's data in the cache entry
		Common::SeekableReadStream *decompressed = decompressLZ(stream);
		cacheEntry.resize(decompressed->size());
		decompressed->read(&cacheEntry[0], decompressed->size());

		delete decompressed;
	} else {
		// It's not, so read the raw data of the file into the cache entry
		cacheEntry.resize(stream.size());
		stream.read(&cacheEntry[0], stream.size());
	}
}

/**
 * Get a file from the cache
 */
Common::SeekableReadStream *Cache::get(const Common::String &filename) const {
	// Return a memory stream that encapsulates the data
	const CacheEntry &cacheEntry = _resources[filename];
	return new Common::MemoryReadStream(&cacheEntry[0], cacheEntry.size());
}

/*----------------------------------------------------------------*/

Resources::Resources() {
	_resourceIndex = -1;

	addToCache("vgs.lib");
	addToCache("talk.lib");
	addToCache("sequence.txt");
	addToCache("journal.txt");
	addToCache("portrait.lib");
}

/**
 * Adds the specified file to the cache. If it's a library file, takes care of
 * loading it's index for future use
 */
void Resources::addToCache(const Common::String &filename) { 
	_cache.load(filename); 

	// Check to see if the file is a library
	Common::SeekableReadStream *stream = load(filename);
	uint32 header = stream->readUint32BE();
	if (header == MKTAG('L', 'I', 'B', 26))
		loadLibraryIndex(filename, stream);

	delete stream;
}

/**
 * Adds a resource from a library file tot he cache
 */
void Resources::addToCache(const Common::String &filename, const Common::String &libFilename) {
	// Get the resource
	Common::SeekableReadStream *stream = load(filename, libFilename);

	_cache.load(filename, *stream);

	delete stream;
}

Common::SeekableReadStream *Resources::load(const Common::String &filename) {
	// First check if the file is directly in the cache
	if (_cache.isCached(filename))
		return _cache.get(filename);

	// Secondly, iterate through any loaded library file looking for a resource
	// that has the same name
	LibraryIndexes::iterator i;
	for (i = _indexes.begin(); i != _indexes.end(); ++i) {
		if ((*i)._value.contains(filename)) {
			// Get a stream reference to the given library file
			Common::SeekableReadStream *stream = load((*i)._key);
			LibraryEntry &entry = (*i)._value[filename];
			_resourceIndex = entry._index;

			stream->seek(entry._offset);
			Common::SeekableReadStream *resStream = stream->readStream(entry._size);

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

	return stream;
}

/**
 * Loads a specific resource from a given library file
 */
Common::SeekableReadStream *Resources::load(const Common::String &filename, const Common::String &libraryFile) {
	// Open up the library for access
	Common::SeekableReadStream *libStream = load(libraryFile);

	// Check if the library has already had it's index read, and if not, load it
	if (!_indexes.contains(libraryFile))
		loadLibraryIndex(libraryFile, libStream);

	// Extract the data for the specified resource and return it
	LibraryEntry &entry = _indexes[libraryFile][filename];
	libStream->seek(entry._offset);
	Common::SeekableReadStream *stream = libStream->readStream(entry._size);

	delete libStream;
	return stream;
}


/**
 * Reads in the index from a library file, and caches it's index for later use
 */
void Resources::loadLibraryIndex(const Common::String &libFilename,
		Common::SeekableReadStream *stream) {
	uint32 offset, nextOffset;

	// Create an index entry
	_indexes[libFilename] = LibraryIndex();
	LibraryIndex &index = _indexes[libFilename];

	// Read in the number of resources
	stream->seek(4);
	int count = stream->readUint16LE();

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
}

/**
 * Returns the index of the last loaded resource in it's given library file.
 * This will be used primarily when loading talk files, so the engine can
 * update the given conversation number in the journal
 */
int Resources::resourceIndex() const {
	return _resourceIndex;
}

} // End of namespace Sherlock
