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

#ifndef M4_COMPRESSION_H
#define M4_COMPRESSION_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/endian.h"

#include "m4/m4.h"

namespace M4 {

struct MadsPackEntry {
public:
	uint16 hash;
	uint32 size;
	uint32 compressedSize;
	byte *data;
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
	MadsPack(const char *resourceName, MadsM4Engine *vm);
	~MadsPack();

	int getCount() const { return _count; }
	MadsPackEntry &getItem(int index) const { return _items[index]; }
	MadsPackEntry &operator[](int index) const { return _items[index]; }
	Common::SeekableReadStream *getItemStream(int index);
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

} // End of namespace M4


#endif
