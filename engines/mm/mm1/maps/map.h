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

#ifndef MM1_MAPS_MAP_H
#define MM1_MAPS_MAP_H

#include "common/array.h"
#include "common/str.h"

namespace MM {
namespace MM1 {
namespace Maps {

class Maps;

class Map {
private:
	Common::String _name;
	uint _mapId;
	byte _mapData[512];
	Common::Array<byte> _globals;
private:
	/**
	 * Loads the map's maze data
	 */
	void loadMazeData();

	/**
	 * Load the map's overlay file
	 */
	void loadOverlay();
public:
	Map(Maps *owner, const Common::String &name);

	/**
	 * Loads the map
	 */
	virtual void load();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
