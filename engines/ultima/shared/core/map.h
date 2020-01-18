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

#ifndef ULTIMA_SHARED_CORE_MAP_H
#define ULTIMA_SHARED_CORE_MAP_H

#include "common/array.h"
#include "ultima/shared/core/rect.h"

namespace Ultima {
namespace Shared {

enum MapType {
	MAP_OVERWORLD = 0, MAP_TOWN = 1, MAP_CASTLE = 2, MAP_DUNGEON = 3, MAP_UNKNOWN = 4
};

enum Direction {
	DIR_NORTH = 0, DIR_SOUTH = 1, DIR_EAST = 2, DIR_WEST = 3
};

struct MapTile {
	int _tileNum;

};

class Map {
protected:
	byte _mapId;						// The map Id
	MapType _mapType;
	uint _mapStyle;						// Map style category for towns & castles
	Common::Array<int16> _data;			// Data for the map
	Point _size;				// X, Y size of the map
	Point _tilesPerOrigTile;	// For enhanced modes, number of tiles per original game tile
	Point _currentPos;			// Current position within the map
	Direction _direction;				// Current direction being faced in the underworld
	bool _fixed;						// Town/city type maps that don't scroll as the player moves
protected:
	/**
	 * Gets a point relative to the current position
	 */
	virtual Point getRelativePosition(const Point &delta);
public:
	/**
	 * Constructor
	 */
	Map();
	virtual ~Map() {}

	/**
	 * Load a given map
	 */
	virtual void loadMap(int mapId, uint videoMode);

	/**
	 * Set the position
	 */
	void setPosition(const Point &pt);
};

} // End of namespace Shared
} // End of namespace Xeen

#endif
