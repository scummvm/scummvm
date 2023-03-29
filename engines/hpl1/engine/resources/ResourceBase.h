/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_RESOURCEBASE_H
#define HPL_RESOURCEBASE_H

#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

class iResourceBase {
public:
	iResourceBase(tString asName, unsigned long alPrio);

	virtual ~iResourceBase();

	/**
	 * virtual bool Reload()=0;
	 * \return true is reload was succesful, else false.
	 */
	virtual bool reload() = 0;

	/**
	 * Free most the memory, save info to get started again.
	 */
	virtual void unload() = 0;

	/**
	 * Free all memory.
	 */
	virtual void destroy() = 0;

	tString GetName() { return msName; }
	unsigned long GetHandle() { return mlHandle; }
	void SetHandle(unsigned long alHandle) { mlHandle = alHandle; }
	tString GetFilePath() { return msFilePath; }
	unsigned long GetTime() { return mlTime; }
	unsigned long GetPrio() { return mlPrio; }
	unsigned long GetSize() { return mlSize; }

	void SetLogDestruction(bool abX) { mbLogDestruction = abX; }

	unsigned int GetUserCount() { return mlUserCount; }
	void IncUserCount();
	void DecUserCount() {
		if (mlUserCount > 0)
			mlUserCount--;
	}
	bool HasUsers() { return mlUserCount > 0; }

	static bool GetLogCreateAndDelete() { return mbLogCreateAndDelete; }
	static void SetLogCreateAndDelete(bool abX) { mbLogCreateAndDelete = abX; }

protected:
	static bool mbLogCreateAndDelete;

	unsigned int mlPrio;  // dunno if this will be of any use.
	unsigned long mlTime; // Time for creation.
	unsigned long mlSize; // for completion. Not used yet.

	unsigned int mlUserCount;
	unsigned long mlHandle;
	tString msName;
	tString msFilePath;
	bool mbLogDestruction;
};

} // namespace hpl

#endif // HPL_RESOURCEBASE_H
