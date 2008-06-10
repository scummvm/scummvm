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
 * $URL$
 * $Id$
 *
 */


#include "common/endian.h"

#include "cine/unpack.h"

namespace Cine {

uint32 CineUnpacker::readSource() {
	uint32 value = READ_BE_UINT32(_src);
	_src -= 4;
	return value;
}

int CineUnpacker::rcr(int inputCarry) {
	int outputCarry = (_chunk32b & 1);
	_chunk32b >>= 1;
	if (inputCarry) {
		_chunk32b |= 0x80000000;
	}
	return outputCarry;
}

int CineUnpacker::nextBit() {
	int carry = rcr(0);
	if (_chunk32b == 0) {
		_chunk32b = readSource();
		_crc ^= _chunk32b;
		carry = rcr(1);
	}
	return carry;
}

uint16 CineUnpacker::getBits(byte numBits) {
	uint16 c = 0;
	while (numBits--) {
		c <<= 1;
		c |= nextBit();
	}
	return c;
}

void CineUnpacker::unpackBytes(uint16 numBytes) {
	_datasize -= numBytes;
	while (numBytes--) {
		*_dst = (byte)getBits(8);
		--_dst;
	}
}

void CineUnpacker::copyRelocatedBytes(uint16 offset, uint16 numBytes) {
	_datasize -= numBytes;
	while (numBytes--) {
		*_dst = *(_dst + offset);
		--_dst;
	}
}

bool CineUnpacker::unpack(byte *dst, const byte *src, int srcLen) {
	_src = src + srcLen - 4;
	_datasize = readSource();
	_dst = dst + _datasize - 1;
	_crc = readSource();
	_chunk32b = readSource();
	_crc ^= _chunk32b;
	do {
		if (!nextBit()) {
			if (!nextBit()) {
				uint16 numBytes = getBits(3) + 1;
				unpackBytes(numBytes);
			} else {
				uint16 numBytes = 2;
				uint16 offset   = getBits(8);
				copyRelocatedBytes(offset, numBytes);
			}
		} else {
			uint16 c = getBits(2);
			if (c == 3) {
				uint16 numBytes = getBits(8) + 9;
				unpackBytes(numBytes);
			} else if (c < 2) { // c == 0 || c == 1
				uint16 numBytes = c + 3;
				uint16 offset   = getBits(c + 9);
				copyRelocatedBytes(offset, numBytes);
			} else { // c == 2
				uint16 numBytes = getBits(8) + 1;
				uint16 offset   = getBits(12);
				copyRelocatedBytes(offset, numBytes);
			}
		}
	} while (_datasize > 0 && _src >= src - 4);
	return _crc == 0;
}

} // End of namespace Cine
