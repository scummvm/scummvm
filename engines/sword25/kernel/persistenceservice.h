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

#ifndef	BS_PERSISTENCESERVICE_H
#define BS_PERSISTENCESERVICE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "kernel/memlog_off.h"
#include <string>
#include <vector>
#include "kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class BS_PersistenceService
{
public:
	BS_PersistenceService();
	virtual ~BS_PersistenceService();

	// -----------------------------------------------------------------------------
	// Singleton-Methode
	// -----------------------------------------------------------------------------

	static BS_PersistenceService & GetInstance();


	// -----------------------------------------------------------------------------
	// Interface
	// -----------------------------------------------------------------------------

	static unsigned int	GetSlotCount();
	static std::string	GetSavegameDirectory();

	void			ReloadSlots();
	bool			IsSlotOccupied(unsigned int SlotID);
	bool			IsSavegameCompatible(unsigned int SlotID);
	std::string &	GetSavegameDescription(unsigned int SlotID);
	std::string &	GetSavegameFilename(unsigned int SlotID);
	
	bool			SaveGame(unsigned int SlotID, const std::string & ScreenshotFilename);
	bool			LoadGame(unsigned int SlotID);

private:
	struct Impl;
	Impl * m_impl;
};

#endif
