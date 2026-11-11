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

#ifndef NANCY_SCRIPT_V5_GAMEFUNCTIONS_H
#define NANCY_SCRIPT_V5_GAMEFUNCTIONS_H

#include "common/array.h"

#include "engines/nancy/enginedata.h"

struct lua_State;

namespace Nancy {
namespace Script {

// Nancy26. What the Game and Sound objects of the scripts keep while the game
// runs, across new games
struct ScriptGameState : public EngineData {
	// What the cheat tools switch on
	bool isCheatMode = false;
	bool isFastConvo = false;
	bool showsDiagnostics = false;	// Nancy27

	// How many times a new game began since the game was started
	uint numRestarts = 0;

	// Nancy27. The channels Sound:PauseAbandoned() paused, and the sounds on them
	struct PausedChannel {
		uint16 channel;
		uint32 serial;
	};

	Common::Array<PausedChannel> pausedAbandonedChannels;
};

// What Nancy26 adds to the objects of the earlier games: the saved games and
// settings in Game, the inventory by the names of the items, Flags:All(),
// Key:IsDown(), the volume of the groups of sounds and more. Fields that mean
// something else from Nancy26 on are read here before the earlier games' code
// gets them. Called once the objects of the earlier games are there.
void registerNancy26GameAPI(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_GAMEFUNCTIONS_H
