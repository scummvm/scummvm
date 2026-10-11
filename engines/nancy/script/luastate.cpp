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

#include "common/random.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luastate.h"

namespace Nancy {
namespace Script {

// Registry key of the scene a state belongs to
static const char kContextKey = 0;

static void openLibrary(lua_State *L, lua_CFunction open, const char *name) {
	lua_pushcfunction(L, open);
	lua_pushstring(L, name);
	lua_call(L, 1, 0);
}

static void removeGlobal(lua_State *L, const char *name) {
	lua_pushnil(L);
	lua_setglobal(L, name);
}

LuaState::LuaState(SceneContext *context) {
	_state = luaL_newstate();
	if (!_state) {
		error("Could not create a Lua state");
	}

	// Scripts get the base, table, string and math libraries, minus what
	// would let them run other code or reach outside of the game
	openLibrary(_state, luaopen_base, "");
	openLibrary(_state, luaopen_table, LUA_TABLIBNAME);
	openLibrary(_state, luaopen_string, LUA_STRLIBNAME);
	openLibrary(_state, luaopen_math, LUA_MATHLIBNAME);

	removeGlobal(_state, "collectgarbage");
	removeGlobal(_state, "dofile");
	removeGlobal(_state, "load");
	removeGlobal(_state, "loadfile");
	removeGlobal(_state, "print");

	// The random numbers of the library come from one generator that all
	// states share, so it is seeded with the first one
	static bool isSeeded = false;
	if (!isSeeded) {
		isSeeded = true;

		lua_getglobal(_state, "math");
		lua_getfield(_state, -1, "randomseed");
		lua_pushinteger(_state, g_nancy->_randomSource->getRandomNumber(0x7FFFFFFE));
		lua_call(_state, 1, 0);
		lua_pop(_state, 1);
	}

	lua_pushlightuserdata(_state, (void *)const_cast<char *>(&kContextKey));
	lua_pushlightuserdata(_state, context);
	lua_rawset(_state, LUA_REGISTRYINDEX);

	registerAPI(_state);
}

LuaState::~LuaState() {
	lua_close(_state);
}

SceneContext *LuaState::getContext(lua_State *L) {
	lua_pushlightuserdata(L, (void *)const_cast<char *>(&kContextKey));
	lua_rawget(L, LUA_REGISTRYINDEX);
	SceneContext *context = (SceneContext *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return context;
}

bool LuaState::runScript(const Common::String &name) {
	Common::SeekableReadStream *stream = g_nancy->_resource->loadScript(Common::Path(name));
	if (!stream) {
		warning("Could not find script %s", name.c_str());
		return false;
	}

	uint32 size = stream->size();
	char *buffer = new char[size];
	stream->read(buffer, size);
	delete stream;

	Common::String chunkName = "=" + name;
	int status = luaL_loadbuffer(_state, buffer, size, chunkName.c_str());
	delete[] buffer;

	if (status != 0) {
		warning("Could not load script %s: %s", name.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, 1);
		return false;
	}

	Common::String what = "Failed initial run-through of script " + name;
	return call(_state, 0, 0, what.c_str());
}

bool LuaState::call(lua_State *L, int numArgs, int numResults, const char *what) {
	if (lua_pcall(L, numArgs, numResults, 0) != 0) {
		warning("%s: %s", what, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
