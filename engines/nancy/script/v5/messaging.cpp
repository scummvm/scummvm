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

#include "common/hash-str.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/mathtypes.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/script/v5/messaging.h"

namespace Nancy {
namespace Script {

typedef Common::HashMap<Common::String, Record *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> AliasMap;

// The names are shared by the scenes of all the streams
static AliasMap &getAliases() {
	static AliasMap s_aliases;
	return s_aliases;
}

bool registerAlias(const Common::String &alias, Record *record) {
	AliasMap &aliases = getAliases();
	AliasMap::iterator it = aliases.find(alias);
	if (it != aliases.end() && it->_value != record) {
		// The scene a stream goes to is loaded while the one it leaves is still
		// around, and its records take over the names the old ones had
		const SceneContext &holder = it->_value->getContext();
		const SceneContext &taker = record->getContext();
		if (&holder == &taker || !holder.getStreamName().equalsIgnoreCase(taker.getStreamName())) {
			return false;
		}
	}

	aliases.setVal(alias, record);
	return true;
}

void deregisterAlias(const Common::String &alias, Record *record) {
	AliasMap &aliases = getAliases();
	AliasMap::iterator it = aliases.find(alias);
	if (it != aliases.end() && it->_value == record) {
		aliases.erase(it);
	}
}

void deregisterAliases(Record *record) {
	AliasMap &aliases = getAliases();
	Common::Array<Common::String> names;
	for (AliasMap::iterator it = aliases.begin(); it != aliases.end(); ++it) {
		if (it->_value == record) {
			names.push_back(it->_key);
		}
	}

	for (const Common::String &name : names) {
		aliases.erase(name);
	}
}

Record *findAlias(const Common::String &alias) {
	AliasMap &aliases = getAliases();
	AliasMap::iterator it = aliases.find(alias);
	return it != aliases.end() ? it->_value : nullptr;
}

// What the exe copies: numbers, texts, booleans and nil as they are, tables
// with everything in them, and the math types. Anything else turns into nil.
void copyValue(lua_State *from, int index, lua_State *to) {
	index = absoluteIndex(from, index);

	switch (lua_type(from, index)) {
	case LUA_TNUMBER:
		lua_pushnumber(to, lua_tonumber(from, index));
		return;
	case LUA_TSTRING: {
		size_t length = 0;
		const char *text = lua_tolstring(from, index, &length);
		lua_pushlstring(to, text, length);
		return;
	}
	case LUA_TBOOLEAN:
		lua_pushboolean(to, lua_toboolean(from, index));
		return;
	case LUA_TTABLE:
		lua_newtable(to);
		lua_pushnil(from);
		while (lua_next(from, index)) {
			copyValue(from, -2, to);
			copyValue(from, -1, to);
			lua_rawset(to, -3);
			lua_pop(from, 1);
		}

		return;
	default:
		break;
	}

	const ScriptRect *rect = testRect(from, index);
	if (rect) {
		pushRect(to, *rect);
		return;
	}

	const uint32 *color = testColor(from, index);
	if (color) {
		pushColor(to, *color);
		return;
	}

	const ScriptMatrix *matrix = testMatrix(from, index);
	if (matrix) {
		pushMatrix(to, *matrix);
		return;
	}

	const ScriptVector *vector = testVector(from, index);
	if (vector) {
		pushVector(to, *vector);
		return;
	}

	if (copyPoint(from, index, to)) {
		return;
	}

	lua_pushnil(to);
}

int sendMessage(lua_State *L, const Common::String &sender, const Common::String &recipient,
		const Common::String &action, int firstArg, int numArgs) {
	Record *record = findAlias(recipient);
	if (record) {
		return record->receiveMessage(L, sender, action, firstArg, numArgs);
	}

	// Streams go by their names as well
	return NancySceneState.getStreams().receiveNancy26Message(L, sender, recipient, action);
}

// ar:Send(recipient, action, ...) and ar:TrySend(...), which return what the
// receiver returns
int recordSend(lua_State *L) {
	Record *record = checkRecord(L, 1);
	const Common::String recipient = luaL_checkstring(L, 2);
	const Common::String action = luaL_checkstring(L, 3);

	const int top = lua_gettop(L);
	const int numResults = sendMessage(L, record->getName(), recipient, action, 4, MAX(0, top - 3));
	return numResults;
}

// ar:SendAsAlias(alias, recipient, action, ...) sends in the name of another
int recordSendAsAlias(lua_State *L) {
	checkRecord(L, 1);
	const Common::String alias = luaL_checkstring(L, 2);
	const Common::String recipient = luaL_checkstring(L, 3);
	const Common::String action = luaL_checkstring(L, 4);

	const int top = lua_gettop(L);
	return sendMessage(L, alias, recipient, action, 5, MAX(0, top - 4));
}

// ar:RegisterAlias(name), which returns whether the name was free
int recordRegisterAlias(lua_State *L) {
	Record *record = checkRecord(L, 1);
	lua_pushboolean(L, registerAlias(luaL_checkstring(L, 2), record));
	return 1;
}

int recordDeregisterAlias(lua_State *L) {
	deregisterAlias(luaL_checkstring(L, 2), checkRecord(L, 1));
	return 0;
}

} // End of namespace Script
} // End of namespace Nancy
