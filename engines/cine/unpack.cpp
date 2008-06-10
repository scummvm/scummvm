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

int CineUnpacker::rcr(int CF) {
	int rCF = (_chk & 1);
	_chk >>= 1;
	if (CF) {
		_chk |= 0x80000000;
	}
	return rCF;
}

int CineUnpacker::nextChunk() {
	int CF = rcr(0);
	if (_chk == 0) {
		_chk = readSource();
		_crc ^= _chk;
		CF = rcr(1);
	}
	return CF;
}

uint16 CineUnpacker::getCode(byte numChunks) {
	uint16 c = 0;
	while (numChunks--) {
		c <<= 1;
		if (nextChunk()) {
			c |= 1;
		}
	}
	return c;
}

void CineUnpacker::unpackHelper1(byte numChunks, byte addCount) {
	uint16 count = getCode(numChunks) + addCount + 1;
	_datasize -= count;
	while (count--) {
		*_dst = (byte)getCode(8);
		--_dst;
	}
}

void CineUnpacker::unpackHelper2(byte numChunks) {
	uint16 i = getCode(numChunks);
	uint16 count = _size + 1;
	_datasize -= count;
	while (count--) {
		*_dst = *(_dst + i);
		--_dst;
	}
}

bool CineUnpacker::unpack(byte *dst, const byte *src, int srcLen) {
	_src = src + srcLen - 4;
	_datasize = readSource();
	_dst = dst + _datasize - 1;
	_size = 0;
	_crc = readSource();
	_chk = readSource();
	_crc ^= _chk;
	do {
		if (!nextChunk()) {
			_size = 1;
			if (!nextChunk()) {
				unpackHelper1(3, 0);
			} else {
				unpackHelper2(8);
			}
		} else {
			uint16 c = getCode(2);
			if (c == 3) {
				unpackHelper1(8, 8);
			} else if (c < 2) {
				_size = c + 2;
				unpackHelper2(c + 9);
			} else {
				_size = getCode(8);
				unpackHelper2(12);
			}
		}
	} while (_datasize > 0 && _src >= src - 4);
	return _crc == 0;
}

} // End of namespace Cine
