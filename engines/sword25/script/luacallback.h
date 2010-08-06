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

#ifndef SWORD25_LUACALLBACK_H
#define SWORD25_LUACALLBACK_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

namespace Lua {

struct lua_State;

}

using namespace Lua;

namespace Sword25 {

// -----------------------------------------------------------------------------
// Class definitions
// -----------------------------------------------------------------------------

class BS_LuaCallback {
public:
	BS_LuaCallback(lua_State *L);
	virtual ~BS_LuaCallback();

	// Funktion muss auf dem Lua-Stack liegen.
	void RegisterCallbackFunction(lua_State *L, unsigned int ObjectHandle);

	// Funktion muss auf dem Lua-Stack liegen.
	void UnregisterCallbackFunction(lua_State *L, unsigned int ObjectHandle);

	void RemoveAllObjectCallbacks(lua_State *L, unsigned int ObjectHandle);

	void InvokeCallbackFunctions(lua_State *L, unsigned int ObjectHandle);

protected:
	virtual int PreFunctionInvokation(lua_State *L) {
		return 0;
	}

private:
	void EnsureObjectCallbackTableExists(lua_State *L, unsigned int ObjectHandle);
	void PushCallbackTable(lua_State *L);
	void PushObjectCallbackTable(lua_State *L, unsigned int ObjectHandle);
};

} // End of namespace Sword25

#endif
