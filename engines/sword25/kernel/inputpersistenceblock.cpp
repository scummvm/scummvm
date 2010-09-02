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

#define BS_LOG_PREFIX "INPUTPERSISTENCEBLOCK"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/inputpersistenceblock.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Constructor / Destructor
// -----------------------------------------------------------------------------

InputPersistenceBlock::InputPersistenceBlock(const void *Data, unsigned int DataLength) :
	m_Data(static_cast<const byte *>(Data), DataLength),
	m_ErrorState(NONE) {
	m_Iter = m_Data.begin();
}

// -----------------------------------------------------------------------------

InputPersistenceBlock::~InputPersistenceBlock() {
	if (m_Iter != m_Data.end()) BS_LOG_WARNINGLN("Persistence block was not read to the end.");
}

// -----------------------------------------------------------------------------
// Reading
// -----------------------------------------------------------------------------

void InputPersistenceBlock::Read(int16 &Value) {
	signed int v;
	Read(v);
	Value = static_cast<int16>(v);
}

// -----------------------------------------------------------------------------

void InputPersistenceBlock::Read(signed int &Value) {
	if (CheckMarker(SINT_MARKER)) {
		RawRead(&Value, sizeof(signed int));
		Value = ConvertEndianessFromStorageToSystem(Value);
	} else {
		Value = 0;
	}
}

// -----------------------------------------------------------------------------

void InputPersistenceBlock::Read(unsigned int &Value) {
	if (CheckMarker(UINT_MARKER)) {
		RawRead(&Value, sizeof(unsigned int));
		Value = ConvertEndianessFromStorageToSystem(Value);
	} else {
		Value = 0;
	}
}

// -----------------------------------------------------------------------------

void InputPersistenceBlock::Read(float &Value) {
	if (CheckMarker(FLOAT_MARKER)) {
		RawRead(&Value, sizeof(float));
		Value = ConvertEndianessFromStorageToSystem(Value);
	} else {
		Value = 0.0f;
	}
}

// -----------------------------------------------------------------------------

void InputPersistenceBlock::Read(bool &Value) {
	if (CheckMarker(BOOL_MARKER)) {
		unsigned int UIntBool;
		RawRead(&UIntBool, sizeof(float));
		UIntBool = ConvertEndianessFromStorageToSystem(UIntBool);
		Value = UIntBool == 0 ? false : true;
	} else {
		Value = 0.0f;
	}
}

// -----------------------------------------------------------------------------

void InputPersistenceBlock::Read(Common::String &Value) {
	Value = "";

	if (CheckMarker(STRING_MARKER)) {
		unsigned int Size;
		Read(Size);

		if (CheckBlockSize(Size)) {
			Value = Common::String(reinterpret_cast<const char *>(&*m_Iter), Size);
			m_Iter += Size;
		}
	}
}

// -----------------------------------------------------------------------------

void InputPersistenceBlock::Read(Common::Array<byte> &Value) {
	if (CheckMarker(BLOCK_MARKER)) {
		unsigned int Size;
		Read(Size);

		if (CheckBlockSize(Size)) {
			Value = Common::Array<byte>(m_Iter, Size);
			m_Iter += Size;
		}
	}
}

// -----------------------------------------------------------------------------

void InputPersistenceBlock::RawRead(void *DestPtr, size_t Size) {
	if (CheckBlockSize(Size)) {
		memcpy(DestPtr, &*m_Iter, Size);
		m_Iter += Size;
	}
}

// -----------------------------------------------------------------------------

bool InputPersistenceBlock::CheckBlockSize(int Size) {
	if (m_Data.end() - m_Iter >= Size) {
		return true;
	} else {
		m_ErrorState = END_OF_DATA;
		BS_LOG_ERRORLN("Unexpected end of persistence block.");
		return false;
	}
}

// -----------------------------------------------------------------------------

bool InputPersistenceBlock::CheckMarker(byte Marker) {
	if (!IsGood() || !CheckBlockSize(1)) return false;

	if (*m_Iter++ == Marker) {
		return true;
	} else {
		m_ErrorState = OUT_OF_SYNC;
		BS_LOG_ERRORLN("Wrong type marker found in persistence block.");
		return false;
	}
}

} // End of namespace Sword25
