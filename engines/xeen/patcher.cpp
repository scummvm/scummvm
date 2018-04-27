/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "xeen/patcher.h"
#include "xeen/xeen.h"
#include "xeen/map.h"
#include "xeen/party.h"
#include "common/memstream.h"
#include "common/serializer.h"

namespace Xeen {

struct ScriptEntry {
	uint _gameId;
	int _mapId;
	const byte *_data;
};

const byte DS_MAP54_LINE8[] = { 8, 10, 10, DIR_EAST, 8, OP_MoveWallObj, 20, 100, 100 };
const byte SW_MAP53_LINE8[] = { 5, 14, 6, DIR_EAST, 8, OP_Exit };
const byte DS_MAP116[] = { 9, 10, 6, 4, 2, OP_TakeOrGive, 0, 0, 103, 127 };

#define SCRIPT_PATCHES_COUNT 3
static const ScriptEntry SCRIPT_PATCHES[] = {
	{ GType_DarkSide, 54, DS_MAP54_LINE8 },	// Fix curtain on level 2 of Ellinger's Tower
	{ GType_Swords, 53, SW_MAP53_LINE8 },	// Fix chest in Hart having gems, but saying "Nothing Here"
	{ GType_DarkSide, 116, DS_MAP116 }		// Fix statue in Dark Tower setting invalid world flag
};

/*------------------------------------------------------------------------*/

void Patcher::patch() {
	patchScripts();
	patchObjects();
}

void Patcher::patchScripts() {
	FileManager &files = *g_vm->_files;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	
	uint gameId = g_vm->getGameID();
	if (gameId == GType_WorldOfXeen)
		gameId = files._ccNum ? GType_DarkSide : GType_Clouds;

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
	FileManager &files = *g_vm->_files;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;

	if ((g_vm->getGameID() == GType_Clouds || (g_vm->getGameID() == GType_WorldOfXeen && !files._ccNum)) &&
			party._mazeId == 24) {
		// Remove floating statue in the distance off SE corner of Clouds of Xeen map
		map._mobData._objects[15]._position = Common::Point(-128, -128);
	}
}

} // End of namespace Xeen
