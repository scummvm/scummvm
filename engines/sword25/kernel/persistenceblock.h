// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef	SWORD25_PERSISTENCEBLOCK_H
#define SWORD25_PERSISTENCEBLOCK_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"


// -----------------------------------------------------------------------------
// Class definition
// -----------------------------------------------------------------------------

class BS_PersistenceBlock
{
public:
	static unsigned int GetSInt32Size() { return sizeof(signed int) + sizeof(unsigned char); }
	static unsigned int GetUInt32Size() { return sizeof(unsigned int) + sizeof(unsigned char); }
	static unsigned int GetFloat32Size() { return sizeof(float) + sizeof(unsigned char); }
	static unsigned int GetBoolSize() { return sizeof(unsigned char) + sizeof(unsigned char); }
	static unsigned int GetStringSize(const std::string & String) { return static_cast<unsigned int>(sizeof(unsigned int) + String.size() + sizeof(unsigned char)); }

protected:
	enum
	{
		SINT_MARKER,
		UINT_MARKER,
		FLOAT_MARKER,
		STRING_MARKER,
		BOOL_MARKER,
		BLOCK_MARKER,
	};

	// -----------------------------------------------------------------------------
	// Endianess Conversions
	// -----------------------------------------------------------------------------
	//
	// Alles wird in Little Endian gespeichert.
	// Auf Big Endian-Systemen muss die Bytereihenfolge daher vor dem Speichern und nach dem Einlesen gespeicherter Werte vertauscht werden.
	//

	template<typename T>
	static T ConvertEndianessFromSystemToStorage(T Value)
	{
		if (IsBigEndian()) ReverseByteOrder(&Value);
		return Value;
	}

	template<typename T>
	static T ConvertEndianessFromStorageToSystem(T Value)
	{
		if (IsBigEndian()) ReverseByteOrder(&Value);
		return Value;
	}

private:
	static bool IsBigEndian()
	{
		unsigned int Dummy = 1;
		unsigned char * DummyPtr = reinterpret_cast<unsigned char *>(&Dummy);
		return DummyPtr[0] == 0;
	}

	template<typename T>
	static void Swap(T & One, T & Two)
	{
		T Temp = One;
		One = Two;
		Two = Temp;
	}

	static void ReverseByteOrder(void * Ptr)
	{
		// Kehrt die Bytereihenfolge des 32-Bit Wortes um auf das Ptr zeigt.
		unsigned char * CharPtr = static_cast<unsigned char *>(Ptr);
		Swap(CharPtr[0], CharPtr[3]);
		Swap(CharPtr[1], CharPtr[2]);
	}
};

// -----------------------------------------------------------------------------
// Compile time asserts
// -----------------------------------------------------------------------------

#define CTASSERT(ex) typedef char ctassert_type[(ex) ? 1 : -1];
CTASSERT(sizeof(unsigned char) == 1);
CTASSERT(sizeof(signed int) == 4);
CTASSERT(sizeof(unsigned int) == 4);
CTASSERT(sizeof(float) == 4);
#undef CTASSERT

#endif
