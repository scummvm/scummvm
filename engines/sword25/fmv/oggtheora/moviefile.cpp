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

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "MOVIEFILE"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"
#include "sword25/fmv/oggtheora/oggstate.h"
#include "sword25/fmv/oggtheora/moviefile.h"

// -----------------------------------------------------------------------------

BS_MovieFile::BS_MovieFile(const std::string & Filename, unsigned int ReadBlockSize, bool & Success) :
	m_Data(0), m_Size(0), m_ReadPos(0), m_ReadBlockSize(ReadBlockSize)
{
	m_Data = reinterpret_cast<char *>(BS_Kernel::GetInstance()->GetPackage()->GetFile(Filename, &m_Size));
	if (!m_Data)
	{
		BS_LOG_ERRORLN("Could not load movie file \"%s\".", Filename.c_str());
		Success = false;
	}
	else
		Success = true;
}

// -----------------------------------------------------------------------------

BS_MovieFile::~BS_MovieFile()
{
	if (m_Data) delete [] m_Data;
}

// -----------------------------------------------------------------------------

int BS_MovieFile::BufferData(BS_OggState & OggState)
{
	if (!m_Data || !m_Size || m_ReadPos >= m_Size)
	{
		BS_LOG_ERRORLN("Tried to read past the movie buffer's end.");
		return 0;
	}

	// just grab some more compressed bitstream and sync it for page extraction 
	char * Buffer = OggState.SyncBuffer(m_ReadBlockSize);
	if (!Buffer)
	{
		BS_LOG_ERRORLN("ogg_sync_buffer() failed.");
		return 0;
	}

	// Feststellen wie viele Bytes kopiert werden sollen, maximal READ_BLOCK_SIZE, weniger falls das Ende der Daten erreicht ist.
	int Bytes = (m_Size - m_ReadPos) > m_ReadBlockSize ? m_ReadBlockSize : m_Size - m_ReadPos;
	memcpy(Buffer, &m_Data[m_ReadPos], Bytes);
	m_ReadPos += Bytes;

	OggState.SyncWrote(Bytes);

	return Bytes;
}

// -----------------------------------------------------------------------------

bool BS_MovieFile::IsEOF() const
{
	return m_ReadPos >= m_Size;
}
