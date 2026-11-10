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
#include "common/str.h"

#include "engines/nancy/enginedata.h"

struct lua_State;

namespace Nancy {
namespace Script {

class LuaState;

// Nancy27. The conditions Brain_SC gives names to ("Hum_Ready_BR"), which
// scripts read as booleans through the Brain global. Each one is a function
// that keeps running in a Lua state of its own for the whole game. Those that
// aren't updated automatically keep their last result, which is saved with
// the game, until a script asks for it to be worked out again.
class BrainLogic : public EngineData {
public:
	BrainLogic();
	~BrainLogic() override;

	// Returns false when there is no condition of that name
	bool getValue(const Common::String &name, bool &value);
	bool isAutoUpdated(const Common::String &name) const;
	void update(const Common::String &name);
	void overrideValue(const Common::String &name, bool value);

	void getNames(Common::Array<Common::String> &names) const;

private:
	struct Logic {
		Common::String name;
		bool isAutoUpdated = true;
		int functionRef = 0;
	};

	static int brainInitCreate(lua_State *L);

	const Logic *findLogic(const Common::String &name) const;
	bool evaluate(const Logic &logic, bool &value);

	LuaState *_lua;
	Common::Array<Logic> _logic;
	bool _isLoading = false;	// BrainInit:Create only works while Brain_SC runs
};

// The Brain global of the scene scripts
void registerBrainAPI(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_BRAIN_H
