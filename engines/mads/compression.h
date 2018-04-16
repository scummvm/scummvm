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

#ifndef MADS_COMPRESSION_H
#define MADS_COMPRESSION_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"

#include "mads/mads.h"

namespace MADS {

enum CompressionType { COMPRESS_NONE = 0, COMPRESS_FAB = 1 };

struct MadsPackEntry {
public:
	CompressionType _type;
	byte _priority;
	uint32 _size;
	uint32 _compressedSize;
	byte *_data;
};

class MadsPack {
private:
	MadsPackEntry *_items;
	int _count;
	int _dataOffset;

	void initialize(Common::SeekableReadStream *stream);
public:
	static bool isCompressed(Common::SeekableReadStream *stream);
	MadsPack(Common::SeekableReadStream *stream);
	MadsPack(const Common::String &resourceName, MADSEngine *_vm);
	~MadsPack();

	int getCount() const { return _count; }
	MadsPackEntry &getItem(int index) const {
		assert(index < _count);
		return _items[index]; }
	MadsPackEntry &operator[](int index) const {
		assert(index < _count);
		return _items[index];
	}
	Common::MemoryReadStream *getItemStream(int index) {
		assert(index < _count);
		return new Common::MemoryReadStream(_items[index]._data, _items[index]._size,
			DisposeAfterUse::NO);
	}
	int getDataOffset() const { return _dataOffset; }
};

class FabDecompressor {
private:
    int _bitsLeft;
    uint32 _bitBuffer;
	const byte *_srcData, *_srcP;
	int _srcSize;

	int getBit();
public:
	void decompress(const byte *srcData, int srcSize, byte *destData, int destSize);
};

} // End of namespace MADS

#endif
