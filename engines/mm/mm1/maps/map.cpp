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

Map::Map(uint index, const Common::String &name, uint16 id,
		byte defaultSection, const char *desc) :
		_mapIndex(index), _name(name), _id(id), _defaultSection(defaultSection) {
	_description = desc ? Common::String(desc) : _name;
	_description.setChar(toupper(_description[0]), 0);
	if (_description.hasPrefix("Area")) {
		_description.setChar(toupper(_description[4]), 4);
		_description.insertChar(' ', 4);
	}

	Common::fill((byte *)&_walls[0], (byte *)&_walls[MAP_SIZE], 0);
	Common::fill(&_states[0], (byte *)&_states[MAP_SIZE], 0);
	Common::fill(&_visited[0], &_visited[MAP_SIZE], 0);
}

void Map::load() {
	loadMazeData();
	loadOverlay();

	g_globals->_heardRumor = false;
}

void Map::loadMazeData() {
	Common::File f;
	if (!f.open("mazedata.dta"))
		error("Could not open mazedata.dta");

	f.seek(512 * _mapIndex);
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

bool Map::checkPartyDead() {
	return g_globals->_party.checkPartyDead();
}

uint16 Map::dataWord(uint16 ofs) const {
	return READ_LE_UINT16(&_data[ofs]);
}

void Map::dataWord(uint16 ofs, uint16 val) {
	WRITE_LE_UINT16(&_data[ofs], val);
}

void Map::reduceHP() {
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		g_globals->_party[i]._hpCurrent /= 2;
}

void Map::updateGame() {
	g_events->send("Game", GameMessage("UPDATE"));
}

void Map::redrawGame() {
	g_events->send("Game", GameMessage("REDRAW"));
}

void Map::encounter(const byte *id1, const byte *id2) {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();

	enc.clearMonsters();
	for (int i = 0; i < 14 && *id1; ++i, ++id1, ++id2)
		enc.addMonster(*id1, *id2);

	enc._manual = true;
	enc._levelIndex = 64;
	enc.execute();
}

void Map::unlockDoor() {
	g_maps->_currentState = _states[g_maps->_mapOffset] ^=
		g_maps->_forwardMask & 0x55;
}

void Map::visitedSpecial() {
	if (!_visited[g_maps->_mapOffset])
		_visited[g_maps->_mapOffset] = VISITED_SPECIAL;
}

void Map::visitedExit() {
	_visited[g_maps->_mapOffset] = VISITED_EXIT;
}

void Map::visitedBusiness() {
	_visited[g_maps->_mapOffset] = VISITED_BUSINESS;
}

} // namespace Maps
} // namespace MM1
} // namespace MM
