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

#define MAP_W 16
#define MAP_H 16
#define MAP_SIZE (MAP_W * MAP_H)

class Maps;

enum WallType {
	WALL_NONE = 0, WALL_NORMAL = 1, WALL_DOOR = 2,
	WALL_TORCH = 3
};

/**
 * The byte structure representing the wall types
 * for the four cardinal directions
 */
struct MapWalls {
	WallType _n : 2;
	WallType _e : 2;
	WallType _s : 2;
	WallType _w : 2;
};

class Map {
private:
	Common::String _name;
	uint16 _id;
	uint _mapId;
	MapWalls _walls[MAP_SIZE];
	byte _states[MAP_SIZE];
	Common::Array<byte> _data;
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
	Map(Maps *owner, const Common::String &name, uint16 id);
	virtual ~Map() {}

	/**
	 * Loads the map
	 */
	virtual void load();

	/**
	 * Gets the map name
	 */
	Common::String getName() const { return _name; }

	/**
	 * Returns the map Id
	 */
	uint16 getId() const { return _id; }

	/**
	 * Accesses the map data
	 */
	const byte &operator[](uint ofs) const {
		return _data[ofs];
	}
	byte &operator[](uint ofs) {
		return _data[ofs];
	}
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
