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
	warning("TeLuaContext::global: Unexpected type %d for global %s", type, name.c_str());
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


} // end namespace Tetraedge
