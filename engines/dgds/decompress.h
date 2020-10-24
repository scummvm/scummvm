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

#ifndef DGDS_DECOMPRESS_H
#define DGDS_DECOMPRESS_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Dgds {

class RleDecompressor {
public:
	uint32 decompress(byte *dest, uint32 sz, Common::SeekableReadStream &input);
};

class LzwDecompressor {
public:
	uint32 decompress(byte *dest, uint32 sz, Common::SeekableReadStream &input);

protected:
	void reset();
	uint32 getCode(uint32 totalBits, Common::SeekableReadStream &input);

private:
        struct {
            byte str[256];
            uint8 len;
        } _codeTable[0x4000];

        byte _codeCur[256];

        uint32 _bitsData, _bitsSize;

        uint32 _codeSize, _codeLen, _cacheBits;

        uint32 _tableSize, _tableMax;
        bool _tableFull;
};

class Decompressor {
public:
	Decompressor();
	virtual ~Decompressor();

	void decompress(byte compression, byte *data, int uncompressedSize, Common::SeekableReadStream *input, int size);

private:
	RleDecompressor *_rleDecompressor;
	LzwDecompressor *_lzwDecompressor;

	const char *_compressionDescr[3] = {"None", "RLE", "LZW"};
};

} // End of namespace Dgds

#endif // DGDS_DECOMPRESS_H

