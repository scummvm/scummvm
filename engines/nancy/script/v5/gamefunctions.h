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
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/keyboard.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"

#include "engines/nancy/enginedata.h"

struct lua_State;

namespace Nancy {
namespace Script {

class Record;

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

	// Nancy28. The scripts of the game, in alphabetical order, the ones the
	// filter of the cheat menu let through, and the index of the first one of
	// those that comes after the filter
	Common::Array<Common::String> scriptNames;
	Common::Array<Common::String> filteredScriptNames;
	uint closestFilteredMatch = 0;

	// Nancy28. The achievements the game knows of, which Achievements:InitIDs()
	// gives once
	Common::Array<Common::String> achievementIds;
	bool hasAchievementIds = false;

	// Nancy29, indexed by item id: whether each item is enabled, and whether
	// its Brain flag forces it in the hand or on the face. The item worn on
	// the face is stored in the EquippedItemData of the scene.
	Common::Array<bool> isItemEnabled;
	Common::Array<bool> isItemForcedOn;
	uint faceItemChangeCount = 0;

	// The pictures of the letters of the fonts made of pictures, kept once loaded
	typedef Common::HashMap<Common::Path, Common::SharedPtr<Graphics::ManagedSurface>, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FontImageMap;
	FontImageMap fontImages;

	// Nancy26: the records by the names they go by for messages, shared by
	// the scenes of all the streams
	typedef Common::HashMap<Common::String, Record *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> AliasMap;
	AliasMap aliases;

	// How many records were made, which numbers their own names
	uint numRecords = 0;

	// Nancy26: where the player's input is, for the records (see scriptinput.h)
	struct InputState {
		Common::Point32 mouse;
		bool hasMouse = false;
		Record *hovered = nullptr;
		Record *pressed[2] = { nullptr, nullptr };

		uint32 lastClickTime[2] = { 0, 0 };
		Common::Point32 lastClickPoint[2];
		bool isDoubleClick[2] = { false, false };

		uint16 heldMoveKeys = 0;
		bool isEscHeld = false;
		Common::Array<Common::KeyCode> keysToRelease;
	};

	InputState input;
};

// Nancy29: applies the Brain flags of the inventory items, which equip them
// in the hand or on the face, or remove them from there. Called every frame.
void updateEquippedItems();

// What Nancy26 adds to the objects of the earlier games: the saved games and
// settings in Game, the inventory by the names of the items, Flags:All(),
// Key:IsDown(), the volume of the groups of sounds and more. Fields that mean
// something else from Nancy26 on are read here before the earlier games' code
// gets them. Called once the objects of the earlier games are there.
void registerNancy26GameAPI(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_GAMEFUNCTIONS_H
