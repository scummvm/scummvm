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

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/lua/lua.h"
#include "common/lua/lualib.h"
#include "common/lua/lauxlib.h"

#include "tetraedge/te/te_lua_context.h"

namespace Tetraedge {

//static lua_State *globalState = nullptr;

static int luaPanicFunction(lua_State *state) {
	const char *msg = lua_tolstring(state, -1, nullptr);
	warning("Lua: %s\n",msg);
	lua_settop(state, -2);
	return 1;
}

TeLuaContext::TeLuaContext() : _luaState(nullptr) {
	_luaState = lua_open();
	luaL_openlibs(_luaState);
	lua_atpanic(_luaState, luaPanicFunction);
}

void TeLuaContext::addBindings(void(*fn)(lua_State *)) {
	fn(_luaState);
}

void TeLuaContext::create() {
	_luaState = lua_open();
	luaL_openlibs(_luaState);
	lua_atpanic(_luaState, luaPanicFunction);
}

void TeLuaContext::destroy() {
	if (_luaState)
		lua_close(_luaState);
}

TeVariant TeLuaContext::global(const Common::String &name) {
	lua_getglobal(_luaState, name.c_str());
	TeVariant retval;
	int type = lua_type(_luaState, -1);
	if (type == LUA_TBOOLEAN) {
		int result = lua_toboolean(_luaState,-1);
		lua_settop(_luaState, -2);
		return TeVariant(result > 0);
	} else if (type == LUA_TNUMBER) {
		double result = lua_tonumber(_luaState, -1);
		lua_settop(_luaState, -2);
		return TeVariant(result);
	} else if (type == LUA_TSTRING) {
		const char *str = lua_tolstring(_luaState, -1, nullptr);
		lua_settop(_luaState, -2);
		return TeVariant(str);
	}
	if (type != LUA_TNIL)
		warning("TeLuaContext::global: Unexpected type %d for global %s", type, name.c_str());
	else
		debug("TeLuaContext::global: Request for nil global %s", name.c_str());
	return TeVariant();
}

void TeLuaContext::setGlobal(const Common::String &name, int val) {
	lua_pushinteger(_luaState, val);
	lua_setglobal(_luaState, name.c_str());
}

void TeLuaContext::setGlobal(const Common::String &name, bool val) {
	lua_pushboolean(_luaState, val);
	lua_setglobal(_luaState, name.c_str());
}

void TeLuaContext::setGlobal(const Common::String &name, const Common::String &val) {
	lua_pushstring(_luaState, val.c_str());
	lua_setglobal(_luaState, name.c_str());
}

void TeLuaContext::removeGlobal(const Common::String &name) {
	lua_pushnil(_luaState);
	lua_setglobal(_luaState, name.c_str());
}

void TeLuaContext::registerCFunction(const Common::String &name, int(*fn)(lua_State *)) {
	lua_pushcclosure(_luaState, fn, 0);
	lua_setglobal(_luaState, name.c_str());
}

void TeLuaContext::setInRegistry(const Common::String &name, TeLuaGUI *gui) {
	lua_pushstring(_luaState, name.c_str());
	lua_pushlightuserdata(_luaState, gui);
	lua_settable(_luaState, LUA_REGISTRYINDEX);
}

// Types for save file.  Aligned with the Lua types at type of
// writing, but don't save them just in case they could change.
enum TeLuaSaveVarType {
	None = 0,
	Boolean = 1,
	Number = 3,
	String = 4
};

Common::Error TeLuaContext::syncState(Common::Serializer &s) {
	// Save/Load globals.  The format of saving is:
	// [type][name][val] [type][name][val]...
	// The type of "None" (0) is the end of the list (and has no name/val).
	if (s.isSaving()) {
		lua_pushvalue(_luaState, LUA_GLOBALSINDEX);
		lua_pushnil(_luaState);
		int nextresult = lua_next(_luaState, -2);
		while (true) {
			if (nextresult == 0) {
				TeLuaSaveVarType stype = None;
				s.syncAsUint32LE(stype);
				lua_settop(_luaState, -2);
				break;
			}
			unsigned int vtype = lua_type(_luaState, -1);
			if (vtype == LUA_TBOOLEAN) {
				TeLuaSaveVarType stype = Boolean;
				Common::String name = lua_tolstring(_luaState, -2, nullptr);
				s.syncAsUint32LE(stype);
				s.syncString(name);
				bool val = lua_toboolean(_luaState, -1);
				s.syncAsByte(val);
			} else if (vtype == LUA_TNUMBER) {
				TeLuaSaveVarType stype = Number;
				Common::String name = lua_tolstring(_luaState, -2, nullptr);
				s.syncAsUint32LE(stype);
				s.syncString(name);
				double val = lua_tonumber(_luaState, -1);
				s.syncAsDoubleLE(val);
			} else if (vtype == LUA_TSTRING) {
				TeLuaSaveVarType stype = String;
				Common::String name = lua_tolstring(_luaState, -2, nullptr);
				s.syncAsUint32LE(stype);
				s.syncString(name);
				Common::String val = lua_tostring(_luaState, -1);
				s.syncString(val);
			}
			lua_settop(_luaState, -2);
			nextresult = lua_next(_luaState, -2);
		}
	} else {
		warning("Confirm loading in TeLuaContext::syncState");
		// loading
		TeLuaSaveVarType vtype = None;
		s.syncAsUint32LE(vtype);
		while (vtype != None) {
			switch (vtype) {
				case Boolean: {
					byte b;
					s.syncAsByte(b);
					lua_pushboolean(_luaState, b);
					break;
				}
				case Number: {
					float d;
					s.syncAsDoubleLE(d);
					lua_pushnumber(_luaState, d);
					break;
				}
				case String: {
					Common::String str;
					s.syncString(str);
					lua_pushstring(_luaState, str.c_str());
					break;
				}
				default:
					error("Unexpected lua type on load %d", (int)vtype);
			}
			Common::String name;
			s.syncString(name);
			lua_setglobal(_luaState, name.c_str());
			s.syncAsUint32LE(vtype);
		}
	}

	return Common::kNoError;
}

} // end namespace Tetraedge
