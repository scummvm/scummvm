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

#include "sword25/math/vertex.h"

extern "C"
{
	#include "sword25/util/lua/lua.h"
	#include "sword25/util/lua/lauxlib.h"
}

// -----------------------------------------------------------------------------

BS_Vertex & BS_Vertex::LuaVertexToVertex(lua_State * L, int StackIndex, BS_Vertex & Vertex)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Sicherstellen, dass wir wirklich eine Tabelle betrachten
	luaL_checktype(L, StackIndex, LUA_TTABLE);

	// X Komponente auslesen
	lua_pushstring(L, "X");
	lua_gettable(L, StackIndex);
	if (!lua_isnumber(L, -1)) luaL_argcheck(L, 0, StackIndex, "the X component has to be a number");
	Vertex.X = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	// Y Komponente auslesen
	lua_pushstring(L, "Y");
	lua_gettable(L, StackIndex);
	if (!lua_isnumber(L, -1)) luaL_argcheck(L, 0, StackIndex, "the Y component has to be a number");
	Vertex.Y = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return Vertex;
}

// -----------------------------------------------------------------------------

void BS_Vertex::VertexToLuaVertex(lua_State * L, const BS_Vertex & Vertex)
{
	// Neue Tabelle erstellen
	lua_newtable(L);

	// X-Wert in die Tabelle schreiben
	lua_pushstring(L, "X");
	lua_pushnumber(L, Vertex.X);
	lua_settable(L, -3);

	// Y-Wert in die Tabelle schreiben
	lua_pushstring(L, "Y");
	lua_pushnumber(L, Vertex.Y);
	lua_settable(L, -3);
}
