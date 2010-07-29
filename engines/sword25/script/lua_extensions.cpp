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
// Includes
// -----------------------------------------------------------------------------

#include "sword25/script/luascript.h"
#include "sword25/script/luabindhelper.h"

// -----------------------------------------------------------------------------

static int Warning(lua_State * L)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	luaL_checkstring(L, 1);
	luaL_where(L, 1);
	lua_pushstring(L, "WARNING - ");
	lua_pushvalue(L, 1);
	lua_concat(L, 3);
	BS_Log::Log("%s\n", luaL_checkstring(L, -1));
	lua_pop(L, 1);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg GLOBAL_FUNCTIONS[] =
{
	"warning", Warning,
	0, 0,
};

// -----------------------------------------------------------------------------

bool BS_LuaScriptEngine::RegisterStandardLibExtensions()
{
	lua_State * L = m_State;
	BS_ASSERT(m_State);

	if (!BS_LuaBindhelper::AddFunctionsToLib(L, "", GLOBAL_FUNCTIONS)) return false;

	return true;
}
