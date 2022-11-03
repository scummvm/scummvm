/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PXGLOBALVARIABLES
#define ICB_PXGLOBALVARIABLES

#include "common/noncopyable.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_clu_api.h"

namespace ICB {

#define MAX_global_vars 256

#define GLOBAL_VAR_NOT_SET (0)
#define GLOBAL_VAR_SET (1)

class CpxVariable {
public:
	uint32 hash;
	int32 value;
};

class CpxGlobalScriptVariables : Common::NonCopyable {
private:
	CpxVariable m_vars[MAX_global_vars];

	// This is not part of the CpxVariable class - because of memory packing reasons
	uint8 m_varInit[MAX_global_vars];

	uint32 m_no_vars;
	uint32 m_sorted;

public:
	CpxGlobalScriptVariables();

	void SortVariables();
	int32 GetVariable(uint32 hash, const char *name = NULL, int32 warn = 1); // Get a variable by hash
	int32 GetVariable(const char *n) {                                     // Get a variable
		return GetVariable(EngineHashString(n), n);
	}

	void SetVariable(uint32, int32);           // Set a variable by hash
	void SetVariable(const char *n, int32 i) { // Set a variable
		SetVariable(EngineHashString(n), i);
	}

	void InitVariable(uint32, int32, const char *name = NULL); // First time initialise a variable by hash
	void InitVariable(const char *n, int32 i) {                // First time initialise a variable
		InitVariable(EngineHashString(n), i, n);
	}

	int32 FindVariable(uint32);         // is this variable taken
	int32 FindVariable(const char *n) { // is this variable taken
		return FindVariable(EngineHashString(n));
	}

	uint32 GetNoItems() { return (m_no_vars); }

	const CpxVariable &operator[](uint32 n) { return (m_vars[n]); } // Return reference to variable itself (const)
};

} // End of namespace ICB

#endif
