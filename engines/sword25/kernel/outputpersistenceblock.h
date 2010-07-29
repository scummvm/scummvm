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

#ifndef	BS_OUTPUTPERSISTENCEBLOCK_H
#define BS_OUTPUTPERSISTENCEBLOCK_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "kernel/persistenceblock.h"
#include "kernel/memlog_off.h"
#include <vector>
#include "kernel/memlog_on.h"


// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class BS_OutputPersistenceBlock : public BS_PersistenceBlock
{
public:
	BS_OutputPersistenceBlock();

	void Write(signed int Value);
	void Write(unsigned int Value);
	void Write(float Value);
	void Write(bool Value);
	void Write(const std::string & String);
	void Write(const void * BufferPtr, size_t Size);

	const void * GetData() const { return &m_Data[0]; }
	unsigned int GetDataSize() const { return m_Data.size(); }

private:
	void WriteMarker(unsigned char Marker);
	void RawWrite(const void * DataPtr, size_t Size);

	std::vector<unsigned char> m_Data;
};

#endif
