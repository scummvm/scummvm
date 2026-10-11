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

#ifndef NANCY_SCRIPT_LUASTATE_H
#define NANCY_SCRIPT_LUASTATE_H

#include "common/str.h"

struct lua_State;

namespace Nancy {
namespace Script {

class SceneContext;

// A Lua interpreter with the engine's objects in it. Nancy19 and up keep all of
// their game data and logic in compiled Lua scripts; every script that gets run
// by itself (the boot files, and each scene of each stream) has a state of its own,
// so scripts only share what the engine keeps for them (flags, values, inventory).
class LuaState {
public:
	// The context is the scene the scripts of this state belong to, if any
	LuaState(SceneContext *context = nullptr);
	~LuaState();

	lua_State *get() const { return _state; }

	// Loads the script of that name and runs its main chunk. Returns false when
	// there is no such script, or when it fails.
	bool runScript(const Common::String &name);

	// Calls the function that sits below its arguments on the stack. A failure is
	// reported together with what was being done, and leaves no results.
	static bool call(lua_State *L, int numArgs, int numResults, const char *what);

	// The scene a state was made for; null for the boot scripts
	static SceneContext *getContext(lua_State *L);

private:
	lua_State *_state;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_LUASTATE_H
