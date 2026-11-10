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

// BrainInit:Create({ { name, isAutoUpdated, function }, ... }). The names
// end in "_BR".
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

		lua_rawgeti(L, -1, 1);
		lua_rawgeti(L, -2, 2);
		lua_rawgeti(L, -3, 3);
		if (!lua_isstring(L, -3) || !lua_isboolean(L, -2)) {
			return luaL_error(L, "Entry %d of the Brain does not have enough fields", i);
		}

		if (!lua_isfunction(L, -1)) {
			return luaL_error(L, "Entry %d of the Brain has no logic function", i);
		}

		Logic logic;
		logic.name = lua_tostring(L, -3);
		logic.isAutoUpdated = lua_toboolean(L, -2) != 0;

		if (!logic.name.hasSuffix("_BR")) {
			warning("Logic %s of the Brain is not properly tagged", logic.name.c_str());
			lua_pop(L, 4);
			continue;
		}

		if (brain->findLogic(logic.name)) {
			warning("Name %s appears more than once in the Brain", logic.name.c_str());
			lua_pop(L, 4);
			continue;
		}

		logic.functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
		brain->_logic.push_back(logic);
		lua_pop(L, 3);
	}

	return 0;
}

const BrainLogic::Logic *BrainLogic::findLogic(const Common::String &name) const {
	for (const Logic &logic : _logic) {
		if (logic.name.equalsIgnoreCase(name)) {
			return &logic;
		}
	}

	return nullptr;
}

// Runs the function of a condition, and keeps the result of one that isn't
// updated automatically. A function that fails changes nothing.
bool BrainLogic::evaluate(const Logic &logic, bool &value) {
	lua_State *L = _lua->get();
	lua_rawgeti(L, LUA_REGISTRYINDEX, logic.functionRef);
	const Common::String what = Common::String::format("Function for %s failed", logic.name.c_str());
	if (!LuaState::call(L, 0, 1, what.c_str())) {
		return false;
	}

	value = lua_toboolean(L, -1) != 0;
	lua_pop(L, 1);

	if (!logic.isAutoUpdated) {
		BrainData *data = (BrainData *)NancySceneState.getPuzzleData(BrainData::getTag());
		data->results.setVal(logic.name, value);
	}

	return true;
}

bool BrainLogic::getValue(const Common::String &name, bool &value) {
	const Logic *logic = findLogic(name);
	if (!logic) {
		return false;
	}

	value = false;
	if (!logic->isAutoUpdated) {
		const BrainData *data = (const BrainData *)NancySceneState.getPuzzleData(BrainData::getTag());
		if (data->results.tryGetVal(logic->name, value)) {
			return true;
		}
	}

	evaluate(*logic, value);
	return true;
}

bool BrainLogic::isAutoUpdated(const Common::String &name) const {
	const Logic *logic = findLogic(name);
	return logic && logic->isAutoUpdated;
}

void BrainLogic::update(const Common::String &name) {
	const Logic *logic = findLogic(name);
	bool value;
	if (logic) {
		evaluate(*logic, value);
	}
}

// The cheat menu sets the conditions that keep their results
void BrainLogic::overrideValue(const Common::String &name, bool value) {
	const Logic *logic = findLogic(name);
	if (logic && !logic->isAutoUpdated) {
		BrainData *data = (BrainData *)NancySceneState.getPuzzleData(BrainData::getTag());
		data->results.setVal(logic->name, value);
	}
}

void BrainLogic::getNames(Common::Array<Common::String> &names) const {
	for (const Logic &logic : _logic) {
		names.push_back(logic.name);
	}
}

static BrainLogic &getBrain(lua_State *L) {
	BrainLogic *brain = (BrainLogic *)const_cast<EngineData *>(g_nancy->getEngineData("BrainLogic"));
	if (!brain) {
		luaL_error(L, "The game has no Brain");
	}

	return *brain;
}

// Brain:AllNames(): an array of the names of all conditions
static int brainAllNames(lua_State *L) {
	Common::Array<Common::String> names;
	getBrain(L).getNames(names);

	lua_createtable(L, names.size(), 0);
	for (uint i = 0; i < names.size(); ++i) {
		lua_pushstring(L, names[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

static int brainIsAutoUpdated(lua_State *L) {
	lua_pushboolean(L, getBrain(L).isAutoUpdated(luaL_checkstring(L, 2)));
	return 1;
}

static int brainUpdate(lua_State *L) {
	getBrain(L).update(luaL_checkstring(L, 2));
	return 0;
}

static int brainOverrideFromCheat(lua_State *L) {
	getBrain(L).overrideValue(luaL_checkstring(L, 2), lua_toboolean(L, 3) != 0);
	return 0;
}

// Brain.<name>: the result of a condition
static int brainIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	bool value;
	if (getBrain(L).getValue(name, value)) {
		lua_pushboolean(L, value);
		return 1;
	}

	indexFunctions(L);
	if (lua_isnil(L, -1)) {
		return luaL_error(L, "There is no logic for %s", name);
	}

	return 1;
}

void registerBrainAPI(lua_State *L) {
	static const luaL_Reg kBrainFunctions[] = {
		{ "__index", brainIndex },
		{ "AllNames", brainAllNames },
		{ "IsAutoUpdated", brainIsAutoUpdated },
		{ "Update", brainUpdate },
		{ "OverrideFromCheat", brainOverrideFromCheat },
		{ nullptr, nullptr }
	};

	registerGlobal(L, "Brain", "ND.Brain", kBrainFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
