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

#ifndef SHERLOCK_RESOURCES_H
#define SHERLOCK_RESOURCES_H

#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Sherlock {

typedef Common::Array<byte> CacheEntry;
typedef Common::HashMap<Common::String, CacheEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> CacheHash;

struct LibraryEntry {
	uint32 _offset, _size;
	int _index;

	LibraryEntry() : _index(0), _offset(0), _size(0) {}
	LibraryEntry(int index, uint32 offset, uint32 size) :
		_index(index), _offset(offset), _size(size) {}
};
typedef Common::HashMap<Common::String, LibraryEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LibraryIndex;
typedef Common::HashMap<Common::String, LibraryIndex, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LibraryIndexes;

class SherlockEngine;

class Cache {
private:
	SherlockEngine *_vm;
	CacheHash _resources;
public:
	Cache(SherlockEngine *_vm);

	/**
	 * Returns true if a given file is currently being cached
	 */
	bool isCached(const Common::String &filename) const;

	/**
	 * Loads a file into the cache if it's not already present, and returns it.
	 * If the file is LZW compressed, automatically decompresses it and loads
	 * the uncompressed version into memory
	 */
	void load(const Common::String &name);

	/**
	 * Load a cache entry based on a passed stream
	 */
	void load(const Common::String &name, Common::SeekableReadStream &stream);

	/**
	 * Get a file from the cache
	 */
	Common::SeekableReadStream *get(const Common::String &filename) const;
};

class Resources {
private:
	SherlockEngine *_vm;
	Cache _cache;
	LibraryIndexes _indexes;
	int _resourceIndex;

	/**
	 * Reads in the index from a library file, and caches it's index for later use
	 */
	void loadLibraryIndex(const Common::String &libFilename, Common::SeekableReadStream *stream);
public:
	Resources(SherlockEngine *vm);

	/**
	 * Adds the specified file to the cache. If it's a library file, takes care of
	 * loading it's index for future use
	 */
	void addToCache(const Common::String &filename);
	
	/**
	 * Adds a resource from a library file to the cache
	 */
	void addToCache(const Common::String &filename, const Common::String &libFilename);
	
	/**
	 * Adds a given stream to the cache under the given name
	 */
	void addToCache(const Common::String &filename, Common::SeekableReadStream &stream);

	bool isInCache(const Common::String &filename) const { return _cache.isCached(filename); }

	/**
	 * Checks the passed stream, and if is compressed, deletes it and replaces it with it's uncompressed data
	 */
	void decompressIfNecessary(Common::SeekableReadStream *&stream);

	/**
	 * Returns a stream for a given file
	 */
	Common::SeekableReadStream *load(const Common::String &filename);

	/**
	 * Loads a specific resource from a given library file
	 */
	Common::SeekableReadStream *load(const Common::String &filename, const Common::String &libraryFile);

	/**
	 * Returns true if the given file exists on disk or in the cache
	 */
	bool exists(const Common::String &filename) const;

	/**
	 * Returns the index of the last loaded resource in it's given library file.
	 * This will be used primarily when loading talk files, so the engine can
	 * update the given conversation number in the journal
	 */
	int resourceIndex() const;

	/**
	 * Decompresses an LZW block of data with a specified output size
	 */
	static Common::SeekableReadStream *decompressLZ(Common::SeekableReadStream &source, uint32 outSize);
	
	/**
	 * Decompress an LZW compressed resource
	 */
	Common::SeekableReadStream *decompressLZ(Common::SeekableReadStream &source);
};

struct ImageFrame {
	uint32 _size;
	uint16 _width, _height;
	int _paletteBase;
	bool _rleEncoded;
	Common::Point _offset;
	byte _rleMarker;
	Graphics::Surface _frame;
};

class ImageFile : public Common::Array<ImageFrame> {
private:
	static SherlockEngine *_vm;

	/**
	 * Load the data of the sprite
	 */
	void load(Common::SeekableReadStream &stream, bool skipPalette, bool animImages);

	/**
	 * Gets the palette at the start of the sprite file
	 */
	void loadPalette(Common::SeekableReadStream &stream);

	/**
	 * Decompress a single frame for the sprite
	 */
	void decompressFrame(ImageFrame  &frame, const byte *src);
public:
	byte _palette[256 * 3];
public:
	ImageFile(const Common::String &name, bool skipPal = false, bool animImages = false);
	ImageFile(Common::SeekableReadStream &stream, bool skipPal = false);
	~ImageFile();
	static void setVm(SherlockEngine *vm);
};

} // End of namespace Sherlock

#endif
