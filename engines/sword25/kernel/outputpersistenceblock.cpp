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

#define BS_LOG_PREFIX "OUTPUTPERSISTENCEBLOCK"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/outputpersistenceblock.h"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

namespace {
const unsigned int INITIAL_BUFFER_SIZE = 1024 * 64;
}

namespace Sword25 {

// -----------------------------------------------------------------------------
// Construction / Destruction
// -----------------------------------------------------------------------------

BS_OutputPersistenceBlock::BS_OutputPersistenceBlock() {
	m_Data.reserve(INITIAL_BUFFER_SIZE);
}

// -----------------------------------------------------------------------------
// Writing
// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::Write(signed int Value) {
	WriteMarker(SINT_MARKER);
	Value = ConvertEndianessFromSystemToStorage(Value);
	RawWrite(&Value, sizeof(Value));
}

// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::Write(unsigned int Value) {
	WriteMarker(UINT_MARKER);
	Value = ConvertEndianessFromSystemToStorage(Value);
	RawWrite(&Value, sizeof(Value));
}

// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::Write(float Value) {
	WriteMarker(FLOAT_MARKER);
	Value = ConvertEndianessFromSystemToStorage(Value);
	RawWrite(&Value, sizeof(Value));
}

// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::Write(bool Value) {
	WriteMarker(BOOL_MARKER);

	unsigned int UIntBool = Value ? 1 : 0;
	UIntBool = ConvertEndianessFromSystemToStorage(UIntBool);
	RawWrite(&UIntBool, sizeof(UIntBool));
}

// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::Write(const Common::String &String) {
	WriteMarker(STRING_MARKER);

	Write(String.size());
	RawWrite(String.c_str(), String.size());
}

// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::Write(const void *BufferPtr, size_t Size) {
	WriteMarker(BLOCK_MARKER);

	Write(Size);
	RawWrite(BufferPtr, Size);
}

// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::WriteMarker(unsigned char Marker) {
	m_Data.push_back(Marker);
}

// -----------------------------------------------------------------------------

void BS_OutputPersistenceBlock::RawWrite(const void *DataPtr, size_t Size) {
	if (Size > 0) {
		unsigned int OldSize = m_Data.size();
		m_Data.resize(OldSize + Size);
		memcpy(&m_Data[OldSize], DataPtr, Size);
	}
}

} // End of namespace Sword25
