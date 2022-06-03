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

#include "common/file.h"
#include "mm/mm1/maps/map.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/events.h"

namespace MM {
namespace MM1 {
namespace Maps {

Map::Map(Maps *owner, const Common::String &name, uint16 id) :
		_name(name), _id(id) {
	_mapId = owner->addMap(this);
	Common::fill((byte *)&_walls[0], (byte *)&_walls[MAP_SIZE], 0);
	Common::fill(&_states[0], (byte *)&_states[MAP_SIZE], 0);
}

void Map::load() {
	loadMazeData();
	loadOverlay();
}

void Map::loadMazeData() {
	Common::File f;
	if (!f.open("mazedata.dta"))
		error("Could not open mazedata.dta");

	f.seek(512 * _mapId);
	f.read((byte *)_walls, MAP_SIZE);
	f.read(_states, MAP_SIZE);
	f.close();
}

void Map::loadOverlay() {
	Common::File f;
	if (!f.open(Common::String::format("%s.ovr", _name.c_str())))
		error("Could not open %s.ovr overlay", _name.c_str());

	int magicId = f.readUint16LE();
	int codePtr = f.readUint16LE();
	int codeSize = f.readUint16LE();
	f.readUint16LE();	// dataPtr
	int dataSize = f.readUint16LE();
	f.readUint16LE();	// extras size
	f.readUint16LE();	// code entry-point

	if (magicId != 0xF2 || codePtr != 0xF48F)
		error("Invalid map overlay header");

	// Skip over code segment, since each map's
	// code is going to be reimplemented in C++
	f.skip(codeSize);

	// Read in the data segment
	_data.resize(dataSize);
	f.read(&_data[0], dataSize);
}

void Map::checkPartyDead() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if ((c._condition & (ASLEEP | STONE | DEAD | BAD_CONDITION)) == 0)
			return;
	}

	// At this point, there's no good characters.
	// So redirect to the death screen
	g_events->replaceView("Dead");
}

} // namespace Maps
} // namespace MM1
} // namespace MM
