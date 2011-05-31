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

#include "m4/compression.h"
#include "m4/m4.h"

#include "common/memstream.h"
#include "common/textconsole.h"

namespace M4 {

const char *madsPackString = "MADSPACK";

bool MadsPack::isCompressed(Common::SeekableReadStream *stream) {
	// Check whether the passed stream is packed

	char tempBuffer[8];
	stream->seek(0);
	if (stream->read(tempBuffer, 8) == 8) {
		if (!strncmp(tempBuffer, madsPackString, 8))
			return true;
	}

	return false;
}

MadsPack::MadsPack(Common::SeekableReadStream *stream) {
	initialize(stream);
}

MadsPack::MadsPack(const char *resourceName, MadsM4Engine* vm) {
	Common::SeekableReadStream *stream = vm->_resourceManager->get(resourceName);
	initialize(stream);
	vm->_resourceManager->toss(resourceName);
}

void MadsPack::initialize(Common::SeekableReadStream *stream) {
	if (!MadsPack::isCompressed(stream))
		error("Attempted to decompress a resource that was not MadsPacked");

	stream->seek(14);
	_count = stream->readUint16LE();
	_items = new MadsPackEntry[_count];

	byte *headerData = new byte[0xA0];
	byte *header = headerData;
	stream->read(headerData, 0xA0);

	for (int i = 0; i < _count; ++i, header += 10) {
		// Get header data
		_items[i].hash = READ_LE_UINT16(header);
		_items[i].size = READ_LE_UINT32(header + 2);
		_items[i].compressedSize = READ_LE_UINT32(header + 6);

		_items[i].data = new byte[_items[i].size];
		if (_items[i].size == _items[i].compressedSize) {
			// Entry isn't compressed
			stream->read(_items[i].data, _items[i].size);
		} else {
			// Decompress the entry
			byte *compressedData = new byte[_items[i].compressedSize];
			stream->read(compressedData, _items[i].compressedSize);

			FabDecompressor fab;
			fab.decompress(compressedData, _items[i].compressedSize, _items[i].data, _items[i].size);
			delete[] compressedData;
		}
	}

	delete[] headerData;
	_dataOffset = stream->pos();
}

Common::SeekableReadStream *MadsPack::getItemStream(int index) {
	return new Common::MemoryReadStream(_items[index].data, _items[index].size, DisposeAfterUse::NO);
}

MadsPack::~MadsPack() {
	for (int i = 0; i < _count; ++i)
		delete[] _items[i].data;
	delete[] _items;
}

//--------------------------------------------------------------------------

void FabDecompressor::decompress(const byte *srcData, int srcSize, byte *destData, int destSize) {
	byte copyLen, copyOfsShift, copyOfsMask, copyLenMask;
	unsigned long copyOfs;
	byte *destP;

	// Validate that the data starts with the FAB header
	if (strncmp((const char *)srcData, "FAB", 3) != 0)
		error("FabDecompressor - Invalid compressed data");

	int shiftVal = srcData[3];
	if ((shiftVal < 10) || (shiftVal > 13))
		error("FabDecompressor - Invalid shift start");

	copyOfsShift = 16 - shiftVal;
	copyOfsMask = 0xFF << (shiftVal - 8);
	copyLenMask = (1 << copyOfsShift) - 1;
	copyOfs = 0xFFFF0000;
	destP = destData;

	// Initialize data fields
	_srcData = srcData;
	_srcP = _srcData + 6;
	_srcSize = srcSize;
	_bitsLeft = 16;
	_bitBuffer = READ_LE_UINT16(srcData + 4);

	for (;;) {
		if (getBit() == 0) {
			if (getBit() == 0) {
				copyLen = ((getBit() << 1) | getBit()) + 2;
				copyOfs = *_srcP++ | 0xFFFFFF00;
			} else {
				copyOfs = (((_srcP[1] >> copyOfsShift) | copyOfsMask) << 8) | _srcP[0];
				copyLen = _srcP[1] & copyLenMask;
				_srcP += 2;
				if (copyLen == 0) {
					copyLen = *_srcP++;
					if (copyLen == 0)
						break;
					else if (copyLen == 1)
						continue;
					else
						copyLen++;
				} else {
					copyLen += 2;
				}
				copyOfs |= 0xFFFF0000;
			}
			while (copyLen-- > 0) {
				if (destP - destData == destSize)
					error("FabDecompressor - Decompressed data exceeded specified size");

				*destP = destP[(signed int)copyOfs];
				destP++;
			}
		} else {
			if (_srcP - srcData == srcSize)
				error("FabDecompressor - Passed end of input buffer during decompression");
			if (destP - destData == destSize)
				error("FabDecompressor - Decompressed data exceeded specified size");

			*destP++ = *_srcP++;
		}
	}

	if (destP - destData != destSize)
		error("FabDecompressor - Decompressed data does not match header decompressed size");
}

int FabDecompressor::getBit() {
	_bitsLeft--;
	if (_bitsLeft == 0) {
		if (_srcP - _srcData == _srcSize)
			error("FabDecompressor - Passed end of input buffer during decompression");

		_bitBuffer = (READ_LE_UINT16(_srcP) << 1) | (_bitBuffer & 1);
		_srcP += 2;
		_bitsLeft = 16;
	}

	int bit = _bitBuffer & 1;
	_bitBuffer >>= 1;
	return bit;
}

} // End of namespace M4
