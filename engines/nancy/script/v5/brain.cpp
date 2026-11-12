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

#include "engines/nancy/nancy.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/v5/brain.h"
#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Script {

static const char *const kBrainInitType = "ND.BrainInit";

BrainLogic::BrainLogic() : _lua(new LuaState()) {
	static const luaL_Reg kBrainInitFunctions[] = {
		{ "__index", indexFunctions },
		{ "Create", brainInitCreate },
		{ nullptr, nullptr }
	};

	// The BrainInit global holds the Brain that BrainInit:Create adds to
	lua_State *L = _lua->get();
	registerType(L, kBrainInitType, kBrainInitFunctions);
	pushValue(L, kBrainInitType, this);
	lua_setglobal(L, "BrainInit");

	_isLoading = true;
	const bool isLoaded = _lua->runScript("Brain_SC");
	_isLoading = false;

	if (!isLoaded) {
		error("Failed to load boot script Brain_SC");
	}
}

BrainLogic::~BrainLogic() {
	delete _lua;
}

// BrainInit:Create({ entry, ... }). Nancy27's entries are
// { name, isAutoUpdated, function }, Nancy28's are { id, enabled, name, tags,
// isWritable, isCached, function, description, date }. The names end in "_BR".
int BrainLogic::brainInitCreate(lua_State *L) {
	BrainLogic *brain = *(BrainLogic **)checkUserData(L, 1, kBrainInitType);
	luaL_checktype(L, 2, LUA_TTABLE);
	if (!brain->_isLoading) {
		return luaL_error(L, "BrainInit:Create can only be called by Brain_SC");
	}

	const int numEntries = (int)lua_objlen(L, 2);
	for (int i = 1; i <= numEntries; ++i) {
		lua_rawgeti(L, 2, i);
		luaL_checktype(L, -1, LUA_TTABLE);
		if (g_nancy->getGameType() >= kGameTypeNancy28) {
			brain->readNancy28Entry(L, i);
		} else {
			brain->readNancy27Entry(L, i);
		}

		lua_pop(L, 1);
	}

	return 0;
}

// The entry is on the top of the stack
void BrainLogic::readNancy27Entry(lua_State *L, int index) {
	lua_rawgeti(L, -1, 1);
	lua_rawgeti(L, -2, 2);
	lua_rawgeti(L, -3, 3);
	if (!lua_isstring(L, -3) || !lua_isboolean(L, -2)) {
		luaL_error(L, "Entry %d of the Brain does not have enough fields", index);
	}

	if (!lua_isfunction(L, -1)) {
		luaL_error(L, "Entry %d of the Brain has no logic function", index);
	}

	Logic logic;
	logic.name = lua_tostring(L, -3);
	logic.isCached = lua_toboolean(L, -2) == 0;

	if (!logic.name.hasSuffix("_BR")) {
		warning("Logic %s of the Brain is not properly tagged", logic.name.c_str());
		lua_pop(L, 3);
		return;
	}

	if (findLogic(logic.name)) {
		warning("Name %s appears more than once in the Brain", logic.name.c_str());
		lua_pop(L, 3);
		return;
	}

	logic.functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_pop(L, 2);
	addLogic(logic);
}

// Brain_SC keeps every version of an entry, and the last one counts. Entries
// without the second field are left out.
void BrainLogic::readNancy28Entry(lua_State *L, int index) {
	lua_rawgeti(L, -1, 2);
	const bool isEnabled = !lua_isnil(L, -1);
	lua_pop(L, 1);
	if (!isEnabled) {
		return;
	}

	Logic logic;
	logic.isWritable = true;
	logic.isCached = true;

	lua_rawgeti(L, -1, 3);
	if (!lua_isstring(L, -1)) {
		luaL_error(L, "Entry %d of the Brain has no name", index);
	}

	logic.name = lua_tostring(L, -1);

	lua_rawgeti(L, -2, 5);
	if (lua_isboolean(L, -1)) {
		logic.isWritable = lua_toboolean(L, -1) != 0;
	}

	lua_rawgeti(L, -3, 6);
	if (lua_isboolean(L, -1)) {
		logic.isCached = lua_toboolean(L, -1) != 0;
	}

	lua_rawgeti(L, -4, 8);
	if (lua_isstring(L, -1)) {
		logic.description = lua_tostring(L, -1);
	}

	lua_pop(L, 4);

	// The descriptions have their commas as backquotes
	for (uint i = 0; i < logic.description.size(); ++i) {
		if (logic.description[i] == '`') {
			logic.description.setChar(',', i);
		}
	}

	// A writable entry is a flag, and has no function. Some of the others have
	// none either, and are always false.
	if (!logic.isWritable) {
		lua_rawgeti(L, -1, 7);
		if (lua_isfunction(L, -1)) {
			logic.functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
		} else {
			lua_pop(L, 1);
		}
	}

	LogicMap::iterator previous = _logic.find(logic.name);
	if (previous != _logic.end()) {
		if (previous->_value.functionRef) {
			luaL_unref(L, LUA_REGISTRYINDEX, previous->_value.functionRef);
		}

		logic.name = previous->_value.name;
		previous->_value = logic;
	} else {
		addLogic(logic);
	}
}

void BrainLogic::addLogic(const Logic &logic) {
	_names.push_back(logic.name);
	_logic.setVal(logic.name, logic);
}

const BrainLogic::Logic *BrainLogic::findLogic(const Common::String &name) const {
	LogicMap::const_iterator it = _logic.find(name);
	return it != _logic.end() ? &it->_value : nullptr;
}

// Runs the function of a condition, and keeps the result of a cached one. A
// function that fails gives what it gave the last time.
bool BrainLogic::evaluate(Logic &logic) {
	lua_State *L = _lua->get();
	lua_rawgeti(L, LUA_REGISTRYINDEX, logic.functionRef);
	const Common::String what = Common::String::format("Function for %s failed", logic.name.c_str());
	if (!LuaState::call(L, 0, 1, what.c_str())) {
		return logic.lastResult;
	}

	logic.lastResult = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);

	if (logic.isCached) {
		BrainData *data = (BrainData *)NancySceneState.getPuzzleData(BrainData::getTag());
		data->results.setVal(logic.name, logic.lastResult);
	}

	return logic.lastResult;
}

// A writable condition that was never set is false, and a cached one that was
// never worked out is worked out now
bool BrainLogic::getValue(const Common::String &name) {
	LogicMap::iterator it = _logic.find(name);
	if (it == _logic.end()) {
		return false;
	}

	Logic &logic = it->_value;
	if (logic.isWritable || logic.isCached) {
		const BrainData *data = (const BrainData *)NancySceneState.getPuzzleData(BrainData::getTag());
		bool value = false;
		if (data->results.tryGetVal(logic.name, value) || logic.isWritable) {
			return value;
		}
	}

	return evaluate(logic);
}

bool BrainLogic::isCached(const Common::String &name) const {
	const Logic *logic = findLogic(name);
	return logic && logic->isCached;
}

bool BrainLogic::isWritable(const Common::String &name) const {
	const Logic *logic = findLogic(name);
	return logic && logic->isWritable;
}

Common::String BrainLogic::getDescription(const Common::String &name) const {
	const Logic *logic = findLogic(name);
	return logic ? logic->description : Common::String();
}

void BrainLogic::setValue(const Common::String &name, bool value) {
	const Logic *logic = findLogic(name);
	if (logic && logic->isWritable) {
		BrainData *data = (BrainData *)NancySceneState.getPuzzleData(BrainData::getTag());
		data->results.setVal(logic->name, value);
	}
}

void BrainLogic::update(const Common::String &name) {
	LogicMap::iterator it = _logic.find(name);
	if (it != _logic.end() && !it->_value.isWritable) {
		evaluate(it->_value);
	}
}

// The cheat menu sets the conditions that keep their results
void BrainLogic::overrideValue(const Common::String &name, bool value) {
	const Logic *logic = findLogic(name);
	if (logic && logic->isCached && !logic->isWritable) {
		BrainData *data = (BrainData *)NancySceneState.getPuzzleData(BrainData::getTag());
		data->results.setVal(logic->name, value);
	}
}

static BrainLogic &getBrain(lua_State *L) {
	BrainLogic *brain = (BrainLogic *)const_cast<EngineData *>(g_nancy->getEngineData("BrainLogic"));
	if (!brain) {
		luaL_error(L, "The game has no Brain");
	}

	return *brain;
}

// The name of a condition, from the argument at the index
static Common::String checkLogicName(lua_State *L, int index) {
	const Common::String name = luaL_checkstring(L, index);
	if (!getBrain(L).exists(name)) {
		luaL_error(L, "There is no flag named %s", name.c_str());
	}

	return name;
}

// Brain:AllNames(): an array of the names of all conditions
static int brainAllNames(lua_State *L) {
	const Common::Array<Common::String> &names = getBrain(L).getNames();
	lua_createtable(L, names.size(), 0);
	for (uint i = 0; i < names.size(); ++i) {
		lua_pushstring(L, names[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

// Nancy27
static int brainIsAutoUpdated(lua_State *L) {
	const Common::String name = checkLogicName(L, 2);
	lua_pushboolean(L, !getBrain(L).isCached(name) && !getBrain(L).isWritable(name));
	return 1;
}

// Nancy28
static int brainIsCached(lua_State *L) {
	lua_pushboolean(L, getBrain(L).isCached(checkLogicName(L, 2)));
	return 1;
}

// Nancy28
static int brainIsWritable(lua_State *L) {
	lua_pushboolean(L, getBrain(L).isWritable(checkLogicName(L, 2)));
	return 1;
}

// Brain:Logic(name), Nancy28: the description of a condition, for the cheat menu
static int brainLogic(lua_State *L) {
	lua_pushstring(L, getBrain(L).getDescription(checkLogicName(L, 2)).c_str());
	return 1;
}

// Nancy28 only lets the cheat menu and Brain:Update() work on cached conditions
static void checkCachedLogic(lua_State *L, const Common::String &name) {
	if (g_nancy->getGameType() < kGameTypeNancy28) {
		return;
	}

	if (getBrain(L).isWritable(name)) {
		luaL_error(L, "Flag %s is writable", name.c_str());
	}

	if (!getBrain(L).isCached(name)) {
		luaL_error(L, "Flag %s is not cached", name.c_str());
	}
}

// Brain:Update(name) works out a condition again
static int brainUpdate(lua_State *L) {
	const Common::String name = checkLogicName(L, 2);
	checkCachedLogic(L, name);
	getBrain(L).update(name);
	return 0;
}

static int brainOverrideFromCheat(lua_State *L) {
	const Common::String name = checkLogicName(L, 2);
	checkCachedLogic(L, name);
	getBrain(L).overrideValue(name, lua_toboolean(L, 3) != 0);
	return 0;
}

// Brain.<name>: the result of a condition
static int brainIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);
	BrainLogic &brain = getBrain(L);
	if (brain.exists(name)) {
		lua_pushboolean(L, brain.getValue(name));
		return 1;
	}

	indexFunctions(L);
	if (lua_isnil(L, -1)) {
		return luaL_error(L, "There is no logic for %s", name);
	}

	return 1;
}

// Brain.<name> = value, Nancy28: sets a writable condition
static int brainNewIndex(lua_State *L) {
	const Common::String name = checkLogicName(L, 2);
	if (!getBrain(L).isWritable(name)) {
		return luaL_error(L, "Flag %s is not writable", name.c_str());
	}

	getBrain(L).setValue(name, lua_toboolean(L, 3) != 0);
	return 0;
}

void registerBrainAPI(lua_State *L) {
	static const luaL_Reg kBrainFunctions[] = {
		{ "__index", brainIndex },
		{ "__newindex", brainNewIndex },
		{ "AllNames", brainAllNames },
		{ "IsAutoUpdated", brainIsAutoUpdated },
		{ "IsCached", brainIsCached },
		{ "IsWritable", brainIsWritable },
		{ "Logic", brainLogic },
		{ "Update", brainUpdate },
		{ "OverrideFromCheat", brainOverrideFromCheat },
		{ nullptr, nullptr }
	};

	registerGlobal(L, "Brain", "ND.Brain", kBrainFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
