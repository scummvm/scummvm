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
#include "mm/mm1/events.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define MAP_W 16
#define MAP_H 16
#define MAP_SIZE (MAP_W * MAP_H)

class Maps;

enum DataOffset {
	MAP_ID = 0,
	MAP_SURFACE_DEST_ID1 = 11,
	MAP_SURFACE_DEST_ID2 = 12,
	MAP_SURFACE_DEST_SECTION = 13,
	MAP_20 = 20,
	MAP_21 = 21,
	MAP_FLEE_THRESHOLD = 22,
	MAP_FLEE_X = 23,
	MAP_FLEE_Y = 24,
	MAP_SURRENDER_THRESHOLD = 25,
	MAP_SURRENDER_X = 26,
	MAP_SURRENDER_Y = 27,
	MAP_BRIBE_THRESHOLD = 28,
	MAP_33 = 33,
	MAP_MAX_MONSTERS = 34,
	MAP_35 = 35,
	MAP_36 = 36,
	MAP_37 = 37,
	MAP_DISPEL_THRESHOLD = 38,
	MAP_SURFACE_X = 42,
	MAP_SURFACE_Y = 43,
	MAP_FLAGS = 46,
	MAP_47 = 47
};

enum WallType {
	WALL_NONE = 0, WALL_NORMAL = 1, WALL_DOOR = 2,
	WALL_TORCH = 3
};

enum CellState {
	CELL_SPECIAL = 0x80, CELL_DARK = 0x20
};

class Maps;

class Map {
protected:
	Common::String _name;
	uint16 _id;
	uint _mapIndex;
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
	byte _walls[MAP_SIZE];
	byte _states[MAP_SIZE];
	uint8 _visited[MAP_SIZE];
public:
	Map(uint index, const Common::String &name, uint16 id);
	virtual ~Map() {}

	/**
	 * Loads the map
	 */
	virtual void load();

	/**
	 * Handles all special stuff that happens on the map
	 */
	virtual void special() = 0;

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
	byte dataByte(uint ofs) const {
		return _data[ofs];
	}
	uint16 dataWord(uint16 ofs) const;
	void dataWord(uint16 ofs, uint16 val);

	/**
	 * Checks whether the party is dead or out of action,
	 * and if so, switches to the death screen
	 */
	void checkPartyDead();

	/**
	 * Send a message to a UI element
	 */
	template<class T>
	bool send(const T &msg) {
		return g_events->send(msg);
	}
	template<class T>
	bool send(const Common::String &name, const T &msg) {
		return g_events->send(name, msg);
	}

};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
