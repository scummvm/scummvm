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

//#define TETRAEDGE_LUA_DEBUG 1

//static lua_State *globalState = nullptr;

static int luaPanicFunction(lua_State *state) {
	const char *msg = lua_tolstring(state, -1, nullptr);
	warning("Lua: %s", msg);
	lua_settop(state, -2);
	return 1;
}

#ifdef TETRAEDGE_LUA_DEBUG
static void luaDebugHook(lua_State *L, lua_Debug *ar) {
    if (!lua_getinfo(L, "Sn", ar))
        return;
    debug("LUA: %s %d", ar->source, ar->currentline);
}
#endif


TeLuaContext::TeLuaContext() : _luaState(nullptr) {
	_luaState = lua_open();
	luaL_openlibs(_luaState);
	lua_atpanic(_luaState, luaPanicFunction);
}

TeLuaContext::~TeLuaContext() {
	destroy();
}

void TeLuaContext::addBindings(void(*fn)(lua_State *)) {
	fn(_luaState);
}

void TeLuaContext::create() {
	_luaState = lua_open();
	luaL_openlibs(_luaState);
	lua_atpanic(_luaState, luaPanicFunction);
#ifdef TETRAEDGE_LUA_DEBUG
	lua_sethook(_luaState, luaDebugHook, LUA_MASKCALL | LUA_MASKLINE, 0);
#endif
}

void TeLuaContext::destroy() {
	if (_luaState)
		lua_close(_luaState);
	_luaState = nullptr;
}

TeVariant TeLuaContext::global(const Common::String &name) {
	lua_getglobal(_luaState, name.c_str());
	TeVariant retval;
	int type = lua_type(_luaState, -1);
	if (type == LUA_TBOOLEAN) {
		int result = lua_toboolean(_luaState, -1);
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

#define TETRAEDGE_LUA_DEBUG_SAVELOAD

Common::Error TeLuaContext::syncState(Common::Serializer &s) {
	// Save/Load globals.  The format of saving is:
	// [type][name][val] [type][name][val]...
	// Vals can be string, number (uint32), or boolean (byte)
	// The type of "None" (0) is the end of the list (and has no name/val).
	if (s.isSaving()) {
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
		debug("TeLuaContext::syncState: --- Saving globals: ---");
#endif
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
			uint vtype = lua_type(_luaState, -1);
			Common::String name = lua_tolstring(_luaState, -2, nullptr);
			if (vtype == LUA_TBOOLEAN) {
				TeLuaSaveVarType stype = Boolean;
				s.syncAsUint32LE(stype);
				s.syncString(name);
				bool val = lua_toboolean(_luaState, -1);
				s.syncAsByte(val);
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
				debug("TeLuaContext::syncState: bool %s = %s", name.c_str(), val ? "true" : "false");
#endif
			} else if (vtype == LUA_TNUMBER) {
				TeLuaSaveVarType stype = Number;
				s.syncAsUint32LE(stype);
				s.syncString(name);
				double val = lua_tonumber(_luaState, -1);
				s.syncAsDoubleLE(val);
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
				debug("TeLuaContext::syncState: num %s = %f", name.c_str(), val);
#endif
			} else if (vtype == LUA_TSTRING) {
				TeLuaSaveVarType stype = String;
				s.syncAsUint32LE(stype);
				s.syncString(name);
				Common::String val = lua_tostring(_luaState, -1);
				s.syncString(val);
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
				debug("TeLuaContext::syncState: str %s = '%s'", name.c_str(), val.c_str());
#endif
			}
			lua_settop(_luaState, -2);
			nextresult = lua_next(_luaState, -2);
		}
	} else {
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
		debug("TeLuaContext::syncState: --- Loading globals: --- ");
#endif
		// loading
		TeLuaSaveVarType vtype = None;
		s.syncAsUint32LE(vtype);
		while (vtype != None) {
			Common::String name;
			s.syncString(name);
			switch (vtype) {
				case Boolean: {
					byte b = 0;
					s.syncAsByte(b);
					lua_pushboolean(_luaState, b);
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
					debug("TeLuaContext::syncState: bool %s = %s", name.c_str(), b ? "true" : "false");
#endif
					break;
				}
				case Number: {
					float d = 0;
					s.syncAsDoubleLE(d);
					lua_pushnumber(_luaState, d);
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
					debug("TeLuaContext::syncState: num %s = %f", name.c_str(), d);
#endif
					break;
				}
				case String: {
					Common::String str;
					s.syncString(str);
					lua_pushstring(_luaState, str.c_str());
#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
					debug("TeLuaContext::syncState: str %s = '%s'", name.c_str(), str.c_str());
#endif
					break;
				}
				default:
					error("Unexpected lua type on load %d", (int)vtype);
			}
			lua_setglobal(_luaState, name.c_str());
			s.syncAsUint32LE(vtype);
		}
	}

#ifdef TETRAEDGE_LUA_DEBUG_SAVELOAD
	debug("TeLuaContext::syncState: -------- end --------");
#endif

	return Common::kNoError;
}

} // end namespace Tetraedge
