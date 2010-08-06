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
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/kernel.h"
#include "sword25/script/luabindhelper.h"
#include "sword25/script/luascript.h"

#define BS_LOG_PREFIX "LUABINDHELPER"

// -----------------------------------------------------------------------------

namespace {
const char *METATABLES_TABLE_NAME = "__METATABLES";
const char *PERMANENTS_TABLE_NAME = "Permanents";

bool RegisterPermanent(lua_State *L, const Common::String &Name) {
	// A C function has to be on the stack
	if (!lua_iscfunction(L, -1)) return false;

	// Make sure that the Permanents-Table is on top of the stack
	lua_getfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);
	if (lua_isnil(L, -1)) {
		// Permanents-Table does not yet exist, so it has to be created

		// Pop nil from the stack
		lua_pop(L, 1);

		// Create Permanents-Table and insert a second reference to it on the stack
		lua_newtable(L);
		lua_pushvalue(L, -1);

		// Store the Permanents-Table in the registry. The second reference is left
		// on the stack to be used in the connection
		lua_setfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);
	}

	// C function with the name of an index in the Permanents-Table
	lua_insert(L, -2);
	lua_setfield(L, -2, Name.c_str());

	// Remove the Permanents-Table from the stack
	lua_pop(L, 1);

	return true;
}
}

namespace Sword25 {

// -----------------------------------------------------------------------------

/**
 * Registers a set of functions into a Lua library.
 * @param L             A pointer to the Lua VM
 * @param LibName       The name of the library.
 * If this is an empty string, the functions will be added to the global namespace.
 * @param Functions     An array of function pointers along with their names.
 * The array must be terminated with the enry (0, 0)
 * @return              Returns true if successful, otherwise false.
 */
bool BS_LuaBindhelper::AddFunctionsToLib(lua_State *L, const Common::String &LibName, const luaL_reg *Functions) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// If the table name is empty, the functions are to be added to the global namespace
	if (LibName.size() == 0) {
		for (; Functions->name; ++Functions) {
			lua_pushstring(L, Functions->name);
			lua_pushcclosure(L, Functions->func, 0);
			lua_settable(L, LUA_GLOBALSINDEX);

			// Function is being permanently registed, so persistence can be ignored
			lua_pushstring(L, Functions->name);
			lua_gettable(L, LUA_GLOBALSINDEX);
			RegisterPermanent(L, Functions->name);
		}
	}
	// If the table name is not empty, the functions are added to the given table
	else {
		// Ensure that the library table exists
		if (!_CreateTable(L, LibName)) return false;

		// Register each function into the table
		for (; Functions->name; ++Functions) {
			// Function registration
			lua_pushstring(L, Functions->name);
			lua_pushcclosure(L, Functions->func, 0);
			lua_settable(L, -3);

			// Function is being permanently registed, so persistence can be ignored
			lua_pushstring(L, Functions->name);
			lua_gettable(L, -2);
			RegisterPermanent(L, LibName + "." + Functions->name);
		}

		// Remove the library table from the Lua stack
		lua_pop(L, 1);
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

/**
 * Adds a set of constants to the Lua library
 * @param L             A pointer to the Lua VM
 * @param LibName       The name of the library.
 * If this is an empty string, the functions will be added to the global namespace.
 * @param Constants     An array of the constant values along with their names.
 * The array must be terminated with the enry (0, 0)
 * @return              Returns true if successful, otherwise false.
 */
bool BS_LuaBindhelper::AddConstantsToLib(lua_State *L, const Common::String &LibName, const lua_constant_reg *Constants) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// If the table is empty, the constants are added to the global namespace
	if (LibName.size() == 0) {
		for (; Constants->Name; ++Constants) {
			lua_pushstring(L, Constants->Name);
			lua_pushnumber(L, Constants->Value);
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
	// If the table name is nto empty, the constants are added to that table
	else {
		// Ensure that the library table exists
		if (!_CreateTable(L, LibName)) return false;

		// Register each constant in the table
		for (; Constants->Name; ++Constants) {
			lua_pushstring(L, Constants->Name);
			lua_pushnumber(L, Constants->Value);
			lua_settable(L, -3);
		}

		// Remove the library table from the Lua stack
		lua_pop(L, 1);
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

/**
 * Adds a set of methods to a Lua class
 * @param L             A pointer to the Lua VM
 * @param ClassName     The name of the class
 * When the class name specified does not exist, it is created.
 * @param Methods       An array of function pointers along with their method names.
 * The array must be terminated with the enry (0, 0)
 * @return              Returns true if successful, otherwise false.
 */
bool BS_LuaBindhelper::AddMethodsToClass(lua_State *L, const Common::String &ClassName, const luaL_reg *Methods) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Load the metatable onto the Lua stack
	if (!GetMetatable(L, ClassName)) return false;

	// Register each method in the Metatable
	for (; Methods->name; ++Methods) {
		lua_pushstring(L, Methods->name);
		lua_pushcclosure(L, Methods->func, 0);
		lua_settable(L, -3);

		// Function is being permanently registed, so persistence can be ignored
		lua_pushstring(L, Methods->name);
		lua_gettable(L, -2);
		RegisterPermanent(L, ClassName + "." + Methods->name);
	}

	// Remove the metatable from the stack
	lua_pop(L, 1);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

/**
 * Sets the garbage collector callback method when items of a particular class are deleted
 * @param L             A pointer to the Lua VM
 * @param ClassName     The name of the class
 * When the class name specified does not exist, it is created.
 * @param GCHandler     A function pointer
 * @return              Returns true if successful, otherwise false.
 */
bool BS_LuaBindhelper::SetClassGCHandler(lua_State *L, const Common::String &ClassName, lua_CFunction GCHandler) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Load the metatable onto the Lua stack
	if (!GetMetatable(L, ClassName)) return false;

	// Add the GC handler to the Metatable
	lua_pushstring(L, "__gc");
	lua_pushcclosure(L, GCHandler, 0);
	lua_settable(L, -3);

	// Function is being permanently registed, so persistence can be ignored
	lua_pushstring(L, "__gc");
	lua_gettable(L, -2);
	RegisterPermanent(L, ClassName + ".__gc");

	// Remove the metatable from the stack
	lua_pop(L, 1);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

} // End of namespace Sword25

// -----------------------------------------------------------------------------

namespace {
void PushMetatableTable(lua_State *L) {
	// Push the Metatable table onto the stack
	lua_getglobal(L, METATABLES_TABLE_NAME);

	// If the table doesn't yet exist, it must be created
	if (lua_isnil(L, -1)) {
		// Pop nil from stack
		lua_pop(L, 1);

		// New table has been created, so add it to the global table and leave reference on stack
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setglobal(L, METATABLES_TABLE_NAME);
	}
}
}

namespace Sword25 {

bool BS_LuaBindhelper::GetMetatable(lua_State *L, const Common::String &TableName) {
	// Push the Metatable table onto the stack
	PushMetatableTable(L);

	// Versuchen, die gewünschte Metatabelle auf den Stack zu legen. Wenn sie noch nicht existiert, muss sie erstellt werden.
	lua_getfield(L, -1, TableName.c_str());
	if (lua_isnil(L, -1)) {
		// Pop nil from stack
		lua_pop(L, 1);

		// Create new table
		lua_newtable(L);

		// Set the __index field in the table
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		// Flag the table as persisted. This ensures that objects within this table get stored
		lua_pushbooleancpp(L, true);
		lua_setfield(L, -2, "__persist");

		// Set the table name and push it onto the stack
		lua_pushvalue(L, -1);
		lua_setfield(L, -3, TableName.c_str());
	}

	// Remove the Metatable table from the stack
	lua_remove(L, -2);

	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaBindhelper::_CreateTable(lua_State *L, const Common::String &TableName) {
	const char *PartBegin = TableName.c_str();

	while (PartBegin) {
		const char *PartEnd = strchr(PartBegin, '.');
		if (!PartEnd) PartEnd = PartBegin + strlen(PartBegin);
		Common::String SubTableName(PartBegin, PartEnd - PartBegin);

		// Tables with an empty string as the name are not allowed
		if (SubTableName.size() == 0) return false;

		// Verify that the table with the name already exists
		// The first round will be searched in the global namespace, with later passages
		// in the corresponding parent table in the stack
		if (PartBegin == 0) {
			lua_pushstring(L, SubTableName.c_str());
			lua_gettable(L, LUA_GLOBALSINDEX);
		} else {
			lua_pushstring(L, SubTableName.c_str());
			lua_gettable(L, -2);
			if (!lua_isnil(L, -1)) lua_remove(L, -2);
		}

		// If it doesn't exist, create table
		if (lua_isnil(L, -1)) {
			// Pop nil from stack
			lua_pop(L, 1);

			// Create new table
			lua_newtable(L);
			lua_pushstring(L, SubTableName.c_str());
			lua_pushvalue(L, -2);
			if (PartBegin == 0)
				lua_settable(L, LUA_GLOBALSINDEX);
			else {
				lua_settable(L, -4);
				lua_remove(L, -2);
			}
		}

		PartBegin = PartEnd + 1;
	}

	return true;
}

} // End of namespace Sword25

namespace {
Common::String GetLuaValueInfo(lua_State *L, int StackIndex) {
	switch (lua_type(L, StackIndex)) {
	case LUA_TNUMBER:
		lua_pushstring(L, lua_tostring(L, StackIndex));
		break;

	case LUA_TSTRING:
		lua_pushfstring(L, "\"%s\"", lua_tostring(L, StackIndex));
		break;

	case LUA_TBOOLEAN:
		lua_pushstring(L, (lua_toboolean(L, StackIndex) ? "true" : "false"));
		break;

	case LUA_TNIL:
		lua_pushliteral(L, "nil");
		break;

	default:
		lua_pushfstring(L, "%s: %p", luaL_typename(L, StackIndex), lua_topointer(L, StackIndex));
		break;
	}

	Common::String Result(lua_tostring(L, -1));
	lua_pop(L, 1);

	return Result;
}
}

namespace Sword25 {

Common::String BS_LuaBindhelper::StackDump(lua_State *L) {
	Common::String oss;

	int i = lua_gettop(L);
	oss += "------------------- Stack Dump -------------------\n";

	while (i) {
		oss += i + ": " + GetLuaValueInfo(L, i) + "\n";
		i--;
	}

	oss += "-------------- Stack Dump Finished ---------------\n";

	return oss;
}

Common::String BS_LuaBindhelper::TableDump(lua_State *L) {
	Common::String oss;

	oss += "------------------- Table Dump -------------------\n";

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		// Get the value of the current element on top of the stack, including the index
		oss += GetLuaValueInfo(L, -2) + " : " + GetLuaValueInfo(L, -1) + "\n";

		// Pop value from the stack. The index is then ready for the next call to lua_next()
		lua_pop(L, 1);
	}

	oss += "-------------- Table Dump Finished ---------------\n";

	return oss;
}

} // End of namespace Sword25
