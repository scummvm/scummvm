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

#ifndef BS_OGGSTATE_H
#define BS_OGGSTATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "ogg/ogg.h"
// XXX
#include <iostream>
// XXX

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_OggState
{
public:
	BS_OggState();
	virtual ~BS_OggState();

	int		SyncPageout(ogg_page * OggPage);
	char *	SyncBuffer(long Size);
	int		SyncWrote(long Bytes);

	// XXX
	void DumpInternals()
	{
		std::cout << "bodybytes: " << m_SyncState.bodybytes << std::endl;
		std::cout << "fill: " << m_SyncState.fill << std::endl;
		std::cout << "headerbytes: " << m_SyncState.headerbytes << std::endl;
		std::cout << "returned: " << m_SyncState.returned << std::endl;
		std::cout << "storage: " << m_SyncState.storage << std::endl;
		std::cout << "unsynched: " << m_SyncState.unsynced << std::endl;
		std::cout << std::endl;
	}
	// XXX
private:
	ogg_sync_state				m_SyncState;
};

#endif
