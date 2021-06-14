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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TILELOAD_H
#define SAGA2_TILELOAD_H

namespace Saga2 {

#ifdef _WIN32   //  Set structure alignment packing value to 1 byte
#pragma pack( push, 1 )
#endif

const int           maxBanks = 64;          // 64 banks maximum

/* ============================================================================ *
   TileBank request bits
 * ============================================================================ */

//  To facilitate loading of tile banks on demand, a bit-array
//  is used to indicate which banks are needed for each metatile
//  to render.

class BankBits {
public:
	uint32 b[maxBanks / 32];

	// constructors
	BankBits() {}
	BankBits(uint32 c, uint32 d) {
		b[0] = c;
		b[1] = d;
	}

	// BankBits operators
	friend BankBits operator+ (BankBits c, BankBits d) {
		return BankBits(c.b[0] + d.b[0], c.b[1] + d.b[1]);
	}

	friend BankBits operator- (BankBits c, BankBits d) {
		return BankBits(c.b[0] - d.b[0], c.b[1] - d.b[1]);
	}

	void operator>>=(int a) {
		bool c = (bool)(b[0] & 1);
		b[0] >>= (uint32)a;
		b[1] >>= (uint32)a;
		b[1] |= ((1 << 31) & (uint32)c);
	}
	void operator<<=(int a) {
		bool c = (bool)(b[1] & (1 << 31));
		b[0] >>= (uint32)a;
		b[0] |= (1 & (uint32)c);
		b[1] >>= (uint32)a;
	}

	friend BankBits operator& (BankBits c, BankBits d) {
		return BankBits(c.b[0] & d.b[0], c.b[1] & d.b[1]);
	}

	friend BankBits operator| (BankBits c, BankBits d) {
		return BankBits(c.b[0] | d.b[0], c.b[1] | d.b[1]);
	}

	friend BankBits operator|= (BankBits c, BankBits d) {
		return BankBits(c.b[0] |= d.b[0], c.b[1] |= d.b[1]);
	}

	friend bool operator!= (BankBits c, BankBits d) {
		return (c.b[0] != d.b[0] && c.b[1] != d.b[1]);
	}

	friend BankBits operator^ (BankBits c, BankBits d) {
		return BankBits(c.b[0] ^ d.b[0], c.b[1] ^ d.b[1]);
	}

	bool isSet(uint16 i) {
		return (bool)(b[i >> 5] & ((uint32) 1 << (i & 31)));
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

	// Point16 functions

};

class bitArray {
private:
	uint32  *b;
	uint16  size;
public:
	bitArray(uint16 newSize);
	~bitArray();

	bool operator[](uint16 i) {
		if (i < size) return (bool)(b[i / 32] & (1 << (i % 32)));
		else return false;
	}
	void resize(uint16 newSize);

};

template<int size> class FixedBitArray {
private:
	enum {
		lWords = ((size + 31) / 32),
	};

	int16 WORDNUM(int n) {
		return (n >> 5);
	}
	int16 BITNUM(int n) {
		return (n & 31);
	}
	int16 BITMASK(int n) {
		return (1 << (n & 31));
	}

	uint32  b[lWords];

	void clear(void) {
		memset(&b, 0, sizeof b);
	}

public:

	FixedBitArray(void) {
		clear();
	}

	uint32 getChunk(uint16 i) {
		return b[i];
	}

	bool operator[](uint32 ind) {
		return (ind < size && (b[WORDNUM(ind)] & BITMASK(ind)));
	}

	void Bit(uint32 ind, bool val) {
		if (ind < size) {
			if (val) b[WORDNUM(ind)] |=  BITMASK(ind);
			else     b[WORDNUM(ind)] &= ~BITMASK(ind);
		}
	}

	void clearAll(void) {
		clear();
	}

// Untested below here

	friend FixedBitArray operator& (FixedBitArray c, FixedBitArray d) {
		FixedBitArray   t;

		for (uint16 i = 0; i < lWords; i++)
			t.b[i] = c.b[i] & d.b[i];
		return t;
	}

	friend FixedBitArray operator| (FixedBitArray c, FixedBitArray d) {
		FixedBitArray t;

		for (uint16 i = 0; i < lWords; i++)
			t.b[i] = c.b[i] | d.b[i];
		return t;
	}

	friend FixedBitArray &operator|= (FixedBitArray c, FixedBitArray d) {
		for (uint16 i = 0; i < lWords; i++)
			c.b[i] |= d.b[i];
		return c;
	}

	friend bool operator!= (FixedBitArray c, FixedBitArray d) {
		for (uint16 i = 0; i < lWords; i++)
			if (c.b[i] != d.b[i]) return true;
		return false;
	}

	friend FixedBitArray operator^ (FixedBitArray c, FixedBitArray d) {
		FixedBitArray t;

		for (uint16 i = 0; i < lWords; i++)
			t.b[i] = c.b[i] ^ d.b[i];
		return t;
	}
};

} // end of namespace Saga2

#endif
