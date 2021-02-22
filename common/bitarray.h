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

#ifndef COMMON_BITARRAY_H
#define COMMON_BITARRAY_H

#include "common/scummsys.h"

namespace Common {

class BitArray {
public:
	BitArray() {
		_bitcount = 0;
		_bits = nullptr;
	}

	explicit BitArray(uint bitcount) {
		_bits = nullptr;
		set_size(bitcount);
	}

	~BitArray() {
		free(_bits);
	}

	void set_size(uint bitcount) {
		free(_bits);
		_bitcount = bitcount;
		_bits = (byte*)malloc((bitcount + 7) / 8);
		clear();
	}

	void clear() {
		memset(_bits, 0, (_bitcount + 7) / 8);
	}

	void set(uint bit) {
		_bits[bit / 8] |= (1 << (bit % 8));
	}

	void unset(uint bit) {
		_bits[bit / 8] &= ~(1 << (bit % 8));
	}

	bool get(uint bit) {
		return _bits[bit / 8] & (1 << (bit % 8));
	}

private:
	uint _bitcount;
	byte *_bits;
};

} // End of namespace Common

#endif
