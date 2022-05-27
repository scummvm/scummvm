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

namespace MM {
namespace MM1 {
namespace Maps {

Map::Map(Maps *owner, const Common::String &name) : _name(name) {
	_mapId = owner->addMap(this);
	Common::fill(&_mapData[0], &_mapData[512], 0);
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
	f.read(_mapData, 512);
	f.close();
}

void Map::loadOverlay() {
	Common::File f;
	if (!f.open(Common::String::format("%s.ovr", _name.c_str())))
		error("Could not open %s.ovr overlay", _name.c_str());
	if (f.readUint16LE() != 0xF2)
		error("Invalid overlay header");

	int globalsOffset = f.readUint16LE();
	int globalsSize = f.readUint16LE();
	if (globalsOffset != 0xF48F)
		error("Invalid globals offset");
	_globals.resize(globalsSize);
	f.read(&_globals[0], globalsSize);

	int codeOffset = f.readUint16LE();
	int codeSize = f.readUint16LE();
	if (codeOffset != 0xc940)
		error("Invalid code offset");
}

} // namespace Maps
} // namespace MM1
} // namespace MM
