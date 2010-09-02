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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_PERSISTENCEBLOCK_H
#define SWORD25_PERSISTENCEBLOCK_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Class definition
// -----------------------------------------------------------------------------

class PersistenceBlock {
public:
	static uint GetSInt32Size() {
		return sizeof(signed int) + sizeof(byte);
	}
	static uint GetUInt32Size() {
		return sizeof(uint) + sizeof(byte);
	}
	static uint GetFloat32Size() {
		return sizeof(float) + sizeof(byte);
	}
	static uint GetBoolSize() {
		return sizeof(byte) + sizeof(byte);
	}
	static uint GetStringSize(const Common::String &String) {
		return static_cast<uint>(sizeof(uint) + String.size() + sizeof(byte));
	}

protected:
	enum {
		SINT_MARKER,
		UINT_MARKER,
		FLOAT_MARKER,
		STRING_MARKER,
		BOOL_MARKER,
		BLOCK_MARKER
	};

	// -----------------------------------------------------------------------------
	// Endianess Conversions
	// -----------------------------------------------------------------------------
	//
	// Everything is stored in Little Endian
	// Big Endian Systems will need to be byte swapped during both saving and reading of saved values
	//

	template<typename T>
	static T ConvertEndianessFromSystemToStorage(T Value) {
		if (IsBigEndian()) ReverseByteOrder(&Value);
		return Value;
	}

	template<typename T>
	static T ConvertEndianessFromStorageToSystem(T Value) {
		if (IsBigEndian()) ReverseByteOrder(&Value);
		return Value;
	}

private:
	static bool IsBigEndian() {
		uint Dummy = 1;
		byte *DummyPtr = reinterpret_cast<byte *>(&Dummy);
		return DummyPtr[0] == 0;
	}

	template<typename T>
	static void Swap(T &One, T &Two) {
		T Temp = One;
		One = Two;
		Two = Temp;
	}

	static void ReverseByteOrder(void *Ptr) {
		// Reverses the byte order of the 32-bit word pointed to by Ptr
		byte *CharPtr = static_cast<byte *>(Ptr);
		Swap(CharPtr[0], CharPtr[3]);
		Swap(CharPtr[1], CharPtr[2]);
	}
};

// -----------------------------------------------------------------------------
// Compile time asserts
// -----------------------------------------------------------------------------

#define CTASSERT(ex) typedef char ctassert_type[(ex) ? 1 : -1]
CTASSERT(sizeof(byte) == 1);
CTASSERT(sizeof(signed int) == 4);
CTASSERT(sizeof(uint) == 4);
CTASSERT(sizeof(float) == 4);
#undef CTASSERT

} // End of namespace Sword25

#endif
