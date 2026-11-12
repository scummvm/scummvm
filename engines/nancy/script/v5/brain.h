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

#ifndef NANCY_SCRIPT_V5_BRAIN_H
#define NANCY_SCRIPT_V5_BRAIN_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"

#include "engines/nancy/enginedata.h"

struct lua_State;

namespace Nancy {
namespace Script {

class LuaState;

// Nancy27. The conditions Brain_SC gives names to ("Hum_Ready_BR"), which
// scripts read as booleans through the Brain global. Each one is a function
// that keeps running in a Lua state of its own for the whole game. Those that
// are cached keep their last result, which is saved with the game, until a
// script asks for it to be worked out again.
// Nancy28 adds writable ones, which have no function: they are flags the
// scripts set with Brain.<name> = value, and are saved with the game as well.
class BrainLogic : public EngineData {
public:
	BrainLogic();
	~BrainLogic() override;

	bool exists(const Common::String &name) const { return findLogic(name) != nullptr; }
	bool getValue(const Common::String &name);
	bool isCached(const Common::String &name) const;
	bool isWritable(const Common::String &name) const;
	Common::String getDescription(const Common::String &name) const;

	void setValue(const Common::String &name, bool value);
	void update(const Common::String &name);
	void overrideValue(const Common::String &name, bool value);

	const Common::Array<Common::String> &getNames() const { return _names; }

private:
	struct Logic {
		Common::String name;
		Common::String description;	// Nancy28, what the designers wrote down about it
		bool isWritable = false;
		bool isCached = false;
		int functionRef = 0;		// 0 for none
		bool lastResult = false;		// what a function that fails gives
	};

	typedef Common::HashMap<Common::String, Logic, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LogicMap;

	static int brainInitCreate(lua_State *L);
	void readNancy27Entry(lua_State *L, int index);
	void readNancy28Entry(lua_State *L, int index);
	void addLogic(const Logic &logic);

	const Logic *findLogic(const Common::String &name) const;
	bool evaluate(Logic &logic);

	LuaState *_lua;
	LogicMap _logic;
	Common::Array<Common::String> _names;	// in the order Brain_SC gives them
	bool _isLoading = false;	// BrainInit:Create only works while Brain_SC runs
};

// The Brain global of the scene scripts
void registerBrainAPI(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_BRAIN_H
