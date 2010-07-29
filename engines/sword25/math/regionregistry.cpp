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

#define BS_LOG_PREFIX "REGIONREGISTRY"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/outputpersistenceblock.h"
#include "kernel/inputpersistenceblock.h"
#include "regionregistry.h"
#include "region.h"

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

std::auto_ptr<BS_RegionRegistry> BS_RegionRegistry::m_InstancePtr;

// -----------------------------------------------------------------------------

void BS_RegionRegistry::LogErrorLn(const char * Message) const
{
	BS_LOG_ERRORLN(Message);
}

// -----------------------------------------------------------------------------

void BS_RegionRegistry::LogWarningLn(const char * Message) const
{
	BS_LOG_WARNINGLN(Message);
}

// -----------------------------------------------------------------------------

bool BS_RegionRegistry::Persist(BS_OutputPersistenceBlock & Writer)
{
	bool Result = true;

	// Das nächste zu vergebene Handle schreiben.
	Writer.Write(m_NextHandle);

	// Anzahl an BS_Regions schreiben.
	Writer.Write(m_Handle2PtrMap.size());

	// Alle BS_Regions persistieren.
	HANDLE2PTR_MAP::const_iterator Iter = m_Handle2PtrMap.begin();
	while (Iter != m_Handle2PtrMap.end())
	{
		// Handle persistieren.
		Writer.Write(Iter->first);

		// Objekt persistieren.
		Result &= Iter->second->Persist(Writer);

		++Iter;
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool BS_RegionRegistry::Unpersist(BS_InputPersistenceBlock & Reader)
{
	bool Result = true;

	// Das nächste zu vergebene Handle wieder herstellen.
	Reader.Read(m_NextHandle);

	// Alle vorhandenen BS_Regions zerstören.
	while (!m_Handle2PtrMap.empty()) delete m_Handle2PtrMap.begin()->second;

	// Anzahl an BS_Regions einlesen.
	unsigned int RegionCount;
	Reader.Read(RegionCount);

	// Alle gespeicherten BS_Regions wieder herstellen.
	for (unsigned int i = 0; i < RegionCount; ++i)
	{
		// Handle lesen.
		unsigned int Handle;
		Reader.Read(Handle);

		// BS_Region wieder herstellen.
		Result &= BS_Region::Create(Reader, Handle) != 0;
	}

	return Reader.IsGood() && Result;
}
