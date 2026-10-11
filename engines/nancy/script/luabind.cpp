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

#include "common/array.h"

#include "engines/nancy/script/luabind.h"

namespace Nancy {
namespace Script {

void registerType(lua_State *L, const char *typeName, const luaL_Reg *functions) {
	if (luaL_newmetatable(L, typeName)) {
		luaL_register(L, nullptr, functions);

		lua_pushstring(L, typeName);
		lua_setfield(L, -2, "__metatable");
	}

	lua_pop(L, 1);
}

void registerGlobal(lua_State *L, const char *globalName, const char *typeName, const luaL_Reg *functions) {
	registerType(L, typeName, functions);
	pushUserData(L, 1, typeName);
	lua_setglobal(L, globalName);
}

void *pushUserData(lua_State *L, size_t size, const char *typeName) {
	void *data = lua_newuserdata(L, size);
	luaL_getmetatable(L, typeName);
	lua_setmetatable(L, -2);
	return data;
}

void *testUserData(lua_State *L, int index, const char *typeName) {
	void *data = lua_touserdata(L, index);
	if (!data || !lua_getmetatable(L, index)) {
		return nullptr;
	}

	luaL_getmetatable(L, typeName);
	if (!lua_rawequal(L, -1, -2)) {
		data = nullptr;
	}

	lua_pop(L, 2);
	return data;
}

void *checkUserData(lua_State *L, int index, const char *typeName) {
	return luaL_checkudata(L, index, typeName);
}

int indexFunctions(lua_State *L) {
	if (!lua_getmetatable(L, 1)) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushvalue(L, 2);
	lua_rawget(L, -2);
	return 1;
}

int absoluteIndex(lua_State *L, int index) {
	if (index < 0 && index > LUA_REGISTRYINDEX) {
		return lua_gettop(L) + index + 1;
	}

	return index;
}

bool pushField(lua_State *L, int tableIndex, const char *name) {
	lua_getfield(L, tableIndex, name);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return false;
	}

	return true;
}

bool readField(lua_State *L, int tableIndex, const char *name, Common::String &value) {
	if (!pushField(L, tableIndex, name)) {
		return false;
	}

	if (!lua_isstring(L, -1)) {
		luaL_error(L, "field %s must be a string", name);
	}

	value = lua_tostring(L, -1);
	lua_pop(L, 1);
	return true;
}

bool readField(lua_State *L, int tableIndex, const char *name, double &value) {
	if (!pushField(L, tableIndex, name)) {
		return false;
	}

	if (!lua_isnumber(L, -1)) {
		luaL_error(L, "field %s must be a number", name);
	}

	value = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return true;
}

bool readField(lua_State *L, int tableIndex, const char *name, int &value) {
	double number;
	if (!readField(L, tableIndex, name, number)) {
		return false;
	}

	value = (int)number;
	return true;
}

bool readField(lua_State *L, int tableIndex, const char *name, bool &value) {
	if (!pushField(L, tableIndex, name)) {
		return false;
	}

	if (!lua_isboolean(L, -1)) {
		luaL_error(L, "field %s must be a boolean", name);
	}

	value = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);
	return true;
}

void readStringList(lua_State *L, int index, Common::Array<Common::String> &list) {
	index = absoluteIndex(L, index);

	if (lua_type(L, index) == LUA_TSTRING) {
		list.push_back(lua_tostring(L, index));
		return;
	}

	luaL_checktype(L, index, LUA_TTABLE);

	for (int i = 1; ; ++i) {
		lua_rawgeti(L, index, i);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}

		list.push_back(luaL_checkstring(L, -1));
		lua_pop(L, 1);
	}
}

} // End of namespace Script
} // End of namespace Nancy
