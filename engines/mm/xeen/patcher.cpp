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

#include "mm/xeen/patcher.h"
#include "mm/xeen/xeen.h"
#include "mm/xeen/map.h"
#include "mm/xeen/party.h"
#include "common/memstream.h"
#include "common/serializer.h"

namespace MM {
namespace Xeen {

struct ScriptEntry {
	uint _gameId;
	int _mapId;
	const byte *_data;
};

struct ObjectEntry {
	int _gameId;			///< Game Id
	int _removeMazeId;		///< Maze Id of copy to remove
	int _removeObjNumber;	///< Object number of copy to remove
	int _refMazeId;			///< Reference object maze id
	int _refObjNumber;		///< Reference object's number
};

static const byte DS_MAP54_LINE8[] = { 8, 10, 10, DIR_EAST, 8, OP_MoveWallObj, 20, 100, 100 };
static const byte SW_MAP53_LINE8[] = { 5, 14, 6, DIR_EAST, 8, OP_Exit };
static const byte DS_MAP116[] = { 9, 10, 6, 4, 2, OP_TakeOrGive, 0, 0, 103, 128 };
static const byte DS_MAP62_PIT1[] = { 9, 11, 8, DIR_ALL, 4, OP_FallToMap, 61, 11, 8, 0 };
static const byte DS_MAP62_PIT2[] = { 9, 7, 4, DIR_ALL, 4, OP_FallToMap, 61, 7, 4, 0 };

#define SCRIPT_PATCHES_COUNT 5
static const ScriptEntry SCRIPT_PATCHES[] = {
	{ GType_DarkSide, 54, DS_MAP54_LINE8 },	// Fix curtain on level 2 of Ellinger's Tower
	{ GType_Swords, 53, SW_MAP53_LINE8 },	// Fix chest in Hart having gems, but saying "Nothing Here"
	{ GType_DarkSide, 116, DS_MAP116 },		// Fix statue in Dark Tower setting invalid world flag
	{ GType_DarkSide, 62, DS_MAP62_PIT1 },	// Fix fall position for pit
	{ GType_DarkSide, 62, DS_MAP62_PIT2 }	// Fix fall position for pit
};

// List of objects that need to be removed. Most of these are for copies of objects that appear in
// the distance on the edge of other maps, so they don't simply pop into existence when the map changes.
// When the main object is removed, the original didn't properly also remove the object copies
#define REMOVE_OBJECTS_COUNT 6
static const ObjectEntry REMOVE_OBJECTS[] = {
	// Floating statue in the distance off SE corner of map
	{ GType_Clouds, 24, 15, 0, 0 },
	// Desert Paladin stones
	{ GType_DarkSide, 10, 9, 14, 1 },
	{ GType_DarkSide, 11, 5, 10, 0 },
	{ GType_DarkSide, 15, 5, 14, 4 },
	{ GType_DarkSide, 15, 6, 14, 5 },
	{ GType_DarkSide, 10, 10, 14, 5 }
};

/*------------------------------------------------------------------------*/

void Patcher::patch() {
	patchScripts();
	patchObjects();
}

void Patcher::patchScripts() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;

	uint gameId = g_vm->getSpecificGameId();

	for (int patchIdx = 0; patchIdx < SCRIPT_PATCHES_COUNT; ++patchIdx) {
		const ScriptEntry &se = SCRIPT_PATCHES[patchIdx];
		if (se._gameId != gameId || se._mapId != party._mazeId)
			continue;

		MazeEvent evt;
		Common::MemoryReadStream memStream(se._data, se._data[0] + 1);
		Common::Serializer s(&memStream, nullptr);
		evt.synchronize(s);

		// Scan through the events to find a matching line
		int idx = 0;
		while (idx < (int)map._events.size() && (evt._position != map._events[idx]._position
			|| evt._direction != map._events[idx]._direction || evt._line != map._events[idx]._line))
			++idx;

		// Set the event
		if (idx == (int)map._events.size())
			map._events.push_back(evt);
		else
			map._events[idx] = evt;
	}
}

void Patcher::patchObjects() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	const MazeData *mapData = map.mazeDataSurrounding();

	int gameId = g_vm->getSpecificGameId();

	for (int roCtr = 0; roCtr < REMOVE_OBJECTS_COUNT; ++roCtr) {
		const ObjectEntry &oe = REMOVE_OBJECTS[roCtr];
		if (oe._gameId != gameId || oe._removeMazeId != party._mazeId)
			continue;

		MazeObject &mazeObj = map._mobData._objects[oe._removeObjNumber];

		// If specified object has a linked reference object, we need to check if it's removed
		if (oe._refMazeId) {
			int mazeIndex = -1;
			while (++mazeIndex < 9) {
				if (mapData[mazeIndex]._mazeId == oe._refMazeId)
					break;
			}
			if (mazeIndex == 9)
				error("Could not find specified reference maze in object patcher");

			if (mapData[mazeIndex]._objectsPresent[oe._refObjNumber])
				// Object linked to is still present, so we don't remove the object yet
				continue;
		}

		// Ensure the object is marked as removed
		mazeObj._position.x = mazeObj._position.y = 128;
	}
}

} // End of namespace Xeen
} // End of namespace MM
