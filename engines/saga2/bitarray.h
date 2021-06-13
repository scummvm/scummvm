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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_BITARRAY_H
#define SAGA2_BITARRAY_H

namespace Saga2 {

class BitArray {
private:
	uint16  size;
	uint32  *b;

	void clear(uint16 n) {
		if (n)  for (int i = 0; i < n / 32 + 1; i++)  b[i] = 0;
	}

public:

	BitArray(uint16 newSize = 0) {
		if (newSize) b = (uint32 *)malloc(sizeof(uint32) * (newSize / 32 + 1));
		size = newSize;
		clear(newSize);
	}

	~BitArray() {
		/*if (size) delete[] b;*/ size = 0;
	}

	void resize(uint16 newSize) {
		uint32  *t = b;
		if (newSize) {
			b = (uint32 *)malloc(sizeof(uint32) * (newSize / 32 + 1));
			clear(newSize);
			if (size)  for (int i = 0; i < MIN(size, newSize) / 32 + 1; i++)  b[i] = t[i];
		}
		//if ( size ) delete[] t;
		size = newSize;
	}

	uint16 currentSize(void) {
		return size;
	}

	uint32 getChunk(uint16 i) {
		return b[i];
	}

	bool operator[](uint32 ind) {
		return (ind < size && (b[ind / 32] & ((uint32) 1 << (ind % 32))));
	}

	void Bit(uint32 ind, bool val) {
		if (ind < size) {
			if (val) b[ind / 32] |= ((uint32) 1 << (ind % 32));
			else     b[ind / 32] &= ~((uint32) 1 << (ind % 32));
		}
	}

	void clearAll(void) {
		clear(size);
	}

// Untested below here

	friend BitArray operator& (BitArray c, BitArray d) {
		BitArray t(MAX(c.currentSize(), d.currentSize()));
		for (uint16 i = 0; i < t.currentSize(); i++)    t.b[i] = c.b[i] & d.b[i];
		return t;
	}

	friend BitArray operator| (BitArray c, BitArray d) {
		BitArray t(MAX(c.currentSize(), d.currentSize()));
		for (uint16 i = 0; i < t.currentSize(); i++)    t.b[i] = c.b[i] | d.b[i];
		return t;
	}

	friend BitArray operator|= (BitArray c, BitArray d) {
		for (uint16 i = 0; i < c.currentSize(); i++)    c.b[i] |= d.b[i];
		return c;
	}

	friend bool operator!= (BitArray c, BitArray d) {
		for (uint16 i = 0; i < c.currentSize(); i++)    if (c.b[i] != d.b[i]) return true;
		return false;
	}

	friend BitArray operator^ (BitArray c, BitArray d) {
		BitArray t(MAX(c.currentSize(), d.currentSize()));
		for (uint16 i = 0; i < t.currentSize(); i++)    t.b[i] = c.b[i] ^ d.b[i];
		return t;
	}

#ifdef THIS_SHOULD_NOT_BE_DEFINED
	void operator>>=(int a) {
		bool c = b[0] & 1;
		b[0] >>= (uint32)a;
		b[1] >>= (uint32)a;
		b[1] |= ((1 << 31) & (uint32)c);
	}

	void operator<<=(int a) {
		bool c = b[1] & (1 << 31);
		b[0] >>= (uint32)a;
		b[0] |= (1 & (uint32)c);
		b[1] >>= (uint32)a;
	}

	friend BitArray operator+ (BitArray c, BitArray d) {
		BitArray t(MAX(c.currentSize(), d.currentSize()));
		for (uint16 i = 0; i < t.currentSize(); i++)    t.b[i] = c.b[i] + d.b[i];
		return t;
	}

	bool isSet(int i) {
		return b[i >> 5] & ((uint32) 1 << (i & 31));
	}

	void SetBit(int16 i) {
		b[i / 32] |= ((uint32) 1 << (i % 32))  ;
	}
	void NotBit(int16 i) {
		b[i / 32] &= ~((uint32) 1 << (i % 32));
	}
	void Reset(uint32 c, uint32 d) {
		b[0] = c;
		b[1] = d;
	}
	bool Test() {
		return (b[0] || b[1]);
	}
#endif

};

} // end of namespace Saga2

#endif
