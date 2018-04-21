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

/**
 * Contains data about a given position within the map
 */
class MapTile {
public:
	int _tileNum;
public:
	/**
	 * Constructor
	 */
	MapTile() : _tileNum(-1) {}
	virtual ~MapTile() {}

	/**
	 * Clears the map tile information
	 */
	virtual void clear();
};

/**
 * Base class for managing maps within the game
 */
class Map {
	/**
	 * Stores state about the current viewport being displayed. It's kept as part of the Map class
	 * as a convenience to be alongside the current party position
	 */
	struct ViewportPosition {
		Point _topLeft;					// Top, left tile position for viewport
		Point _size;					// Size of the viewport. Just in case we ever allow it to change
		int _mapId;						// Maze the viewport is for. Used to detect when the map changes

		/**
		 * Constructor
		 */
		ViewportPosition() : _topLeft(-1, -1), _mapId(-1) {}

		/**
		 * Returns true if the viewport is in a valid state
		 */
		bool isValid() const { return _mapId != -1; }

		/**
		 * Resets the viewport position, so it'll get recalculated the next call to getViewportPosition
		 */
		void reset() { _mapId = -1; }
	};
protected:
	byte _mapId;						// The map Id
	MapType _mapType;
	uint _mapStyle;						// Map style category for towns & castles
	Common::Array<int16> _data;			// Data for the map
	Point _size;						// X, Y size of the map
	Point _tilesPerOrigTile;			// For enhanced modes, number of tiles per original game tile
	Point _position;					// Current position within the map
	Direction _direction;				// Current direction being faced in the underworld
	bool _fixed;						// Town/city type maps that don't scroll as the player moves
	ViewportPosition _viewportPos;		// Viewport position
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
	 * Returns the width of the map
	 */
	size_t width() const { return _size.x; }

	/**
	 * Returns the height of the map
	 */
	size_t height() const { return _size.y; }

	/**
	 * Load a given map
	 */
	virtual void loadMap(int mapId, uint videoMode);

	/**
	 * Set the position
	 */
	void setPosition(const Point &pt);

	/**
	 * Get the viewport position
	 */
	Point getViewportPosition(const Point &viewportSize);

	/**
	 * Gets a tile at a given position
	 */

};

} // End of namespace Shared
} // End of namespace Ultima

#endif
