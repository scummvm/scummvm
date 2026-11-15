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
#include "common/random.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource/resource.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luastate.h"

namespace Nancy {
namespace Script {

// Registry keys of the scene a state belongs to, and of the state itself
static const char kContextKey = 0;
static const char kStateKey = 0;

static void openLibrary(lua_State *L, lua_CFunction open, const char *name) {
	lua_pushcfunction(L, open);
	lua_pushstring(L, name);
	lua_call(L, 1, 0);
}

static void removeGlobal(lua_State *L, const char *name) {
	lua_pushnil(L);
	lua_setglobal(L, name);
}

static void setLibraryFunction(lua_State *L, const char *library, const char *name, lua_CFunction function) {
	lua_getglobal(L, library);
	lua_pushcfunction(L, function);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

// Whether the comparison function of a sort puts the value at index a in
// front of the one at index b. It gets told the table, then each key with its value.
static bool sortsBefore(lua_State *L, int a, int b) {
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 1);
	lua_pushinteger(L, a);
	lua_pushinteger(L, a);
	lua_gettable(L, 1);
	lua_pushinteger(L, b);
	lua_pushinteger(L, b);
	lua_gettable(L, 1);
	lua_call(L, 5, 1);

	bool result = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return result;
}

// Nancy20's table.sort(table, function) leaves the table as it is, and gives
// back a new one with the values in order. Values the function finds equal keep
// their order.
//
// The values are Lua tables, and only the function of the script knows how they
// compare, so the keys are sorted here and each comparison is a call to Lua.
// The sort is a merge sort rather than Common::sort, since that one doesn't keep
// equal values in their order.
static int tableSort(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	lua_settop(L, 2);

	const int count = lua_objlen(L, 1);

	// The keys in order, and room to merge them. A failing comparison leaves
	// with a longjmp, which a Common::Array wouldn't survive, so Lua keeps them.
	int *order = (int *)lua_newuserdata(L, 2 * count * sizeof(int));
	int *merged = order + count;
	for (int i = 0; i < count; ++i) {
		order[i] = i + 1;
	}

	for (int width = 1; width < count; width *= 2) {
		for (int low = 0; low < count; low += 2 * width) {
			const int middle = MIN(low + width, count);
			const int high = MIN(low + 2 * width, count);

			int left = low;
			int right = middle;
			int out = low;
			while (left < middle && right < high) {
				if (sortsBefore(L, order[right], order[left])) {
					merged[out++] = order[right++];
				} else {
					merged[out++] = order[left++];
				}
			}

			while (left < middle) {
				merged[out++] = order[left++];
			}

			while (right < high) {
				merged[out++] = order[right++];
			}
		}

		memcpy(order, merged, count * sizeof(int));
	}

	lua_createtable(L, count, 0);
	for (int i = 0; i < count; ++i) {
		lua_pushinteger(L, order[i]);
		lua_gettable(L, 1);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

// One part of a sort key of table.keysort(). Nils go first, then numbers,
// then strings; other values count as nil.
struct KeySortPart {
	int type = LUA_TNIL;
	lua_Number number = 0;
	const char *string = nullptr;
};

struct KeySortEntry {
	Common::Array<KeySortPart> parts;
	int index = 0;
};

// Reads the value on top of the stack and pops it. The strings stay valid while
// the table they are in is on the stack.
static KeySortPart readKeySortPart(lua_State *L) {
	KeySortPart part;
	part.type = lua_type(L, -1);
	if (part.type == LUA_TNUMBER) {
		part.number = lua_tonumber(L, -1);
	} else if (part.type == LUA_TSTRING) {
		part.string = lua_tostring(L, -1);
	} else {
		part.type = LUA_TNIL;
	}

	lua_pop(L, 1);
	return part;
}

static int compareKeySortParts(const KeySortPart &a, const KeySortPart &b, bool ignoreCase) {
	static const int kOrder[] = { 0, 0, 0, 1, 2 };	// by Lua type: nil, -, -, number, string
	if (kOrder[a.type] != kOrder[b.type]) {
		return kOrder[a.type] < kOrder[b.type] ? -1 : 1;
	}

	if (a.type == LUA_TNUMBER) {
		return a.number < b.number ? -1 : (a.number > b.number ? 1 : 0);
	}

	if (a.type == LUA_TSTRING) {
		return ignoreCase ? scumm_stricmp(a.string, b.string) : strcmp(a.string, b.string);
	}

	return 0;
}

// Keys compare part by part; a key that runs out of parts first goes first
static bool keySortsBefore(const KeySortEntry &a, const KeySortEntry &b, bool ignoreCase) {
	for (uint i = 0; i < a.parts.size(); ++i) {
		if (i >= b.parts.size()) {
			return false;
		}

		const int result = compareKeySortParts(a.parts[i], b.parts[i], ignoreCase);
		if (result != 0) {
			return result < 0;
		}
	}

	return a.parts.size() < b.parts.size();
}

// Nancy30's table.keysort(keys, count[, ignoreCase]) sorts the keys 1 to count
// of the table and gives back their indices in order. A key is a number, a
// string or an array of them; equal keys keep their order. List:Sort() moves
// the items of a list with it.
static int tableKeySort(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	const int count = luaL_checkint(L, 2);
	const bool ignoreCase = lua_gettop(L) > 2 && lua_toboolean(L, 3);

	Common::Array<KeySortEntry> entries;
	for (int i = 1; i <= count; ++i) {
		KeySortEntry entry;
		entry.index = i;

		lua_rawgeti(L, 1, i);
		if (lua_istable(L, -1)) {
			const int numParts = lua_objlen(L, -1);
			for (int j = 1; j <= numParts; ++j) {
				lua_rawgeti(L, -1, j);
				entry.parts.push_back(readKeySortPart(L));
			}

			lua_pop(L, 1);
		} else {
			entry.parts.push_back(readKeySortPart(L));
		}

		// An insertion sort, which keeps equal keys in their order
		uint position = entries.size();
		while (position > 0 && keySortsBefore(entry, entries[position - 1], ignoreCase)) {
			--position;
		}

		entries.insert_at(position, entry);
	}

	lua_createtable(L, count > 0 ? count : 0, 0);
	for (uint i = 0; i < entries.size(); ++i) {
		lua_pushinteger(L, entries[i].index);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

// Nancy20's math.isnan(number)
static int mathIsNaN(lua_State *L) {
	const lua_Number number = luaL_checknumber(L, 1);
	lua_pushboolean(L, number != number);
	return 1;
}

// Nancy21's string.compare(a, b[, ordered]) and string.comparenocase(): whether
// the strings are the same, or with "ordered" set, a number below, at or above 0
static int compareStrings(lua_State *L, bool ignoreCase) {
	const char *a = luaL_checkstring(L, 1);
	const char *b = luaL_checkstring(L, 2);
	const int result = ignoreCase ? scumm_stricmp(a, b) : strcmp(a, b);

	if (lua_gettop(L) > 2 && lua_toboolean(L, 3)) {
		lua_pushinteger(L, result);
	} else {
		lua_pushboolean(L, result == 0);
	}

	return 1;
}

static int stringCompare(lua_State *L) {
	return compareStrings(L, false);
}

static int stringCompareNoCase(lua_State *L) {
	return compareStrings(L, true);
}

static LuaState *getState(lua_State *L) {
	lua_pushlightuserdata(L, (void *)const_cast<char *>(&kStateKey));
	lua_rawget(L, LUA_REGISTRYINDEX);
	LuaState *state = (LuaState *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return state;
}

// The code of a coroutine runs on a thread of its own. The engine's functions
// work on the thread that is running, which the two below keep track of; a
// record that a coroutine reads a field of has to find the key on its stack.
void LuaState::resume(lua_State *L, int functionIndex) {
	LuaState *state = getState(L);
	lua_State *previous = state->_running;
	state->_running = lua_tothread(L, functionIndex + 1);

	const int status = lua_pcall(L, lua_gettop(L) - functionIndex, LUA_MULTRET, 0);
	state->_running = previous;
	if (status != 0) {
		lua_error(L);
	}
}

// coroutine.resume(coroutine, ...)
int LuaState::resumeCoroutine(lua_State *L) {
	luaL_argcheck(L, lua_isthread(L, 1), 1, "coroutine expected");

	lua_pushvalue(L, lua_upvalueindex(1));
	lua_insert(L, 1);
	resume(L, 1);
	return lua_gettop(L);
}

// coroutine.wrap(function) gives a function that resumes a new coroutine, and
// raises its errors where it was called, like the one of the library
int LuaState::wrapCoroutine(lua_State *L) {
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushvalue(L, 1);
	lua_call(L, 1, 1);

	lua_pushvalue(L, lua_upvalueindex(2));
	lua_pushcclosure(L, callWrappedCoroutine, 2);
	return 1;
}

int LuaState::callWrappedCoroutine(lua_State *L) {
	lua_pushvalue(L, lua_upvalueindex(2));
	lua_insert(L, 1);
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_insert(L, 2);
	resume(L, 1);

	if (!lua_toboolean(L, 1)) {
		lua_settop(L, 2);
		if (lua_isstring(L, -1)) {
			luaL_where(L, 1);
			lua_insert(L, -2);
			lua_concat(L, 2);
		}

		lua_error(L);
	}

	return lua_gettop(L) - 1;
}

LuaState::LuaState(SceneContext *context) {
	_state = luaL_newstate();
	if (!_state) {
		error("Could not create a Lua state");
	}

	_running = _state;

	// Scripts get the base, table, string and math libraries, minus what
	// would let them run other code or reach outside of the game
	openLibrary(_state, luaopen_base, "");
	openLibrary(_state, luaopen_table, LUA_TABLIBNAME);
	openLibrary(_state, luaopen_string, LUA_STRLIBNAME);
	openLibrary(_state, luaopen_math, LUA_MATHLIBNAME);

	// resume and wrap go through the originals, the latter through create
	lua_getglobal(_state, "coroutine");
	lua_getfield(_state, -1, "create");
	lua_getfield(_state, -2, "resume");
	lua_pushvalue(_state, -1);
	lua_pushcclosure(_state, resumeCoroutine, 1);
	lua_setfield(_state, -4, "resume");
	lua_pushcclosure(_state, wrapCoroutine, 2);
	lua_setfield(_state, -2, "wrap");
	lua_pop(_state, 1);

	if (g_nancy->getGameType() >= kGameTypeNancy20) {
		setLibraryFunction(_state, LUA_TABLIBNAME, "sort", tableSort);
		setLibraryFunction(_state, LUA_MATHLIBNAME, "isnan", mathIsNaN);
	}

	if (g_nancy->getGameType() >= kGameTypeNancy21) {
		setLibraryFunction(_state, LUA_STRLIBNAME, "compare", stringCompare);
		setLibraryFunction(_state, LUA_STRLIBNAME, "comparenocase", stringCompareNoCase);
	}

	// Nancy30 has the debug library. Introspection_SC reads the local variables
	// and upvalues of a calling function with debug.getlocal, debug.getinfo and
	// debug.getupvalue. Lambda_SC compiles functions from strings that see the
	// variables of the code that made them, which it gets from
	// Introspection_SC, and adds debug.traceback to the error when one doesn't
	// compile. The collection scripts (Dict, List, Set, View) use these
	// lambdas, e.g. to filter and sort.
	if (g_nancy->getGameType() >= kGameTypeNancy30) {
		openLibrary(_state, luaopen_debug, LUA_DBLIBNAME);
		setLibraryFunction(_state, LUA_TABLIBNAME, "keysort", tableKeySort);
	}

	removeGlobal(_state, "collectgarbage");
	removeGlobal(_state, "dofile");
	removeGlobal(_state, "load");
	removeGlobal(_state, "loadfile");
	removeGlobal(_state, "print");

	// Nancy1New has loadstring again, which QueryTable_SC builds its lambdas with
	if (g_nancy->getGameType() == kGameTypeNancy22) {
		removeGlobal(_state, "loadstring");
	}

	if (g_nancy->getGameType() >= kGameTypeNancy22) {
		removeGlobal(_state, "setfenv");
	}

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

	lua_pushlightuserdata(_state, (void *)const_cast<char *>(&kStateKey));
	lua_pushlightuserdata(_state, this);
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
	int status = luaL_loadbuffer(_running, buffer, size, chunkName.c_str());
	delete[] buffer;

	if (status != 0) {
		warning("Could not load script %s: %s", name.c_str(), lua_tostring(_running, -1));
		lua_pop(_running, 1);
		return false;
	}

	Common::String what = "Failed initial run-through of script " + name;
	return call(_running, 0, 0, what.c_str());
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
