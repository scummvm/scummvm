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

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "REGIONREGISTRY"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/math/regionregistry.h"
#include "sword25/math/region.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

Common::SharedPtr<RegionRegistry> RegionRegistry::m_InstancePtr;

// -----------------------------------------------------------------------------

void RegionRegistry::LogErrorLn(const char *Message) const {
	BS_LOG_ERRORLN(Message);
}

// -----------------------------------------------------------------------------

void RegionRegistry::LogWarningLn(const char *Message) const {
	BS_LOG_WARNINGLN(Message);
}

// -----------------------------------------------------------------------------

bool RegionRegistry::persist(OutputPersistenceBlock &writer) {
	bool Result = true;

	// write out the next handle
	writer.write(m_NextHandle);

	// Number of regions to write
	writer.write(m_Handle2PtrMap.size());

	// Persist all the BS_Regions
	HANDLE2PTR_MAP::const_iterator Iter = m_Handle2PtrMap.begin();
	while (Iter != m_Handle2PtrMap.end()) {
		// Handle persistence
		writer.write(Iter->_key);

		// Persist object
		Result &= Iter->_value->persist(writer);

		++Iter;
	}

	return Result;
}

// -----------------------------------------------------------------------------

bool RegionRegistry::unpersist(InputPersistenceBlock &reader) {
	bool Result = true;

	// read in the next handle
	reader.read(m_NextHandle);

	// Destroy all existing BS_Regions
//FIXME: This doesn't seem right - the value is being deleted but not the actual hash node itself?
	while (!m_Handle2PtrMap.empty()) delete m_Handle2PtrMap.begin()->_value;

	// read in the number of BS_Regions
	uint RegionCount;
	reader.read(RegionCount);

	// Restore all the BS_Regions objects
	for (uint i = 0; i < RegionCount; ++i)  {
		// Handle read
		uint Handle;
		reader.read(Handle);

		// BS_Region restore
		Result &= Region::Create(reader, Handle) != 0;
	}

	return reader.isGood() && Result;
}

} // End of namespace Sword25
