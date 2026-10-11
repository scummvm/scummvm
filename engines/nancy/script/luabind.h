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

#ifndef NANCY_SCRIPT_LUABIND_H
#define NANCY_SCRIPT_LUABIND_H

#include "common/array.h"
#include "common/str.h"

#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"

namespace Nancy {
namespace Script {

// Helpers for exposing engine objects to the scripts of Nancy19 and up.
//
// Every kind of object has a metatable of its own, kept in the registry under the
// name of the type ("ND.Rect", "ND.NavLogic"...). Scripts tell objects apart by
// that name: getmetatable() hands it out instead of the table.

// Makes the metatable of a type out of the given functions
void registerType(lua_State *L, const char *typeName, const luaL_Reg *functions);

// Makes one of the objects scripts reach through a global ("Rect", "Flags", "Scene"...),
// which carries nothing but the functions of its type
void registerGlobal(lua_State *L, const char *globalName, const char *typeName, const luaL_Reg *functions);

// Pushes a new object of a type. The memory belongs to the script, and goes away
// without a destructor call unless the type has a __gc function.
void *pushUserData(lua_State *L, size_t size, const char *typeName);

// The object at a stack index, or null when something else is there
void *testUserData(lua_State *L, int index, const char *typeName);

// Same, but something else is a script error
void *checkUserData(lua_State *L, int index, const char *typeName);

// __index for the types that have nothing but functions, and the last step of
// an __index function for a type with fields of its own: pushes the function of
// the type that is named by the key at index 2, or nil
int indexFunctions(lua_State *L);

// Pushes a field of the table at an index. A missing field pushes nothing and
// returns false.
bool pushField(lua_State *L, int tableIndex, const char *name);

// Reads a field of the table at an index. A missing field leaves the value alone
// and returns false, one of the wrong type is a script error.
bool readField(lua_State *L, int tableIndex, const char *name, Common::String &value);
bool readField(lua_State *L, int tableIndex, const char *name, double &value);
bool readField(lua_State *L, int tableIndex, const char *name, int &value);
bool readField(lua_State *L, int tableIndex, const char *name, bool &value);

// A string, or the strings of an array, into a list
void readStringList(lua_State *L, int index, Common::Array<Common::String> &list);

// Turns a stack index into one that stays valid when values get pushed
int absoluteIndex(lua_State *L, int index);

template<class T>
T *pushValue(lua_State *L, const char *typeName, const T &value) {
	T *data = (T *)pushUserData(L, sizeof(T), typeName);
	*data = value;
	return data;
}

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_LUABIND_H
