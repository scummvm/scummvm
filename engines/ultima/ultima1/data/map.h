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

#ifndef ULTIMA1_DATA_MAP_H
#define ULTIMA1_DATA_MAP_H

#include "common/archive.h"
#include "common/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

constexpr int OVERWORLD_WIDTH = 168;
constexpr int OVERWORLD_HEIGHT = 156;
constexpr int OVERWORLD_SIZE = OVERWORLD_WIDTH * OVERWORLD_HEIGHT;
constexpr int CITY_WIDTH = 38;
constexpr int CITY_HEIGHT = 18;
constexpr int CITY_SIZE = CITY_WIDTH * CITY_HEIGHT;

constexpr int MAP_VISIBLE_WIDTH = 19;
constexpr int MAP_VISIBLE_HEIGHT = 9;
constexpr int MAP_VISIBLE_CENTER_X = 9;
constexpr int MAP_VISIBLE_CENTER_Y = 4;

constexpr int LOCATION_COUNT = 84;

enum {
	MAP_OVERWORLD = 0,
	MAP_CITY1 = 1
};

enum Direction {
	DIR_LEFT = 1, DIR_RIGHT = 2, DIR_UP = 3, DIR_DOWN = 4
};

typedef Common::Array< Common::Array<byte> > MapTiles;
typedef byte VisibleTiles[MAP_VISIBLE_HEIGHT][MAP_VISIBLE_WIDTH];

extern const char *DIRECTION_NAMES[5];
extern const char *LOCATION_NAMES[LOCATION_COUNT];

struct Map {
private:
	Common::Array<byte> _overworldMap;		// Cached overworld map data
	Common::Array<byte> _cityMap[10];		// Cached city/castle/town maps

	MapTiles _mapTiles;						// Visible on-screen tiles
	int _mapWidth = 0, _mapHeight = 0;		// Map width/height
	byte _outsideMapTile = 0xff;			// -1 for wrapping, else tile for outside map boundaries

	/**
	 * Gives access to a row with the current map
	 */
	class Row {
	private:
		Map *_owner = nullptr;
		const byte *_data = nullptr;

	public:
		Row() {}
		Row(Map *owner, const byte *data) : _owner(owner), _data(data) {}

		// Give access to the byte for a column within the row
		const byte &operator[](int x) const {
			// Handle when the Y is already outside the map, or X is outside map
			if (!_data || (_owner->_outsideMapTile != 0xff && (x < 0 || x >= _owner->_mapWidth)))
				return _owner->_outsideMapTile;

			// Return cell within the row
			if (x < 0)
				return _data[_owner->_mapWidth + x];
			return _data[x % _owner->_mapWidth];
		}
	};
	friend class Row;

	Common::Array<Row> _mapRows;			// Used for array operator getting map contents

public:
	int _currentMap = -1;
	byte _playerTileId = 0;
	byte _tilePlayerCenter = 0;
	byte _tilePlayerUp = 0, _tilePlayerDown = 0,
		_tilePlayerLeft = 0, _tilePlayerRight = 0;
	VisibleTiles _mapTilesId = {};		// Calculated visible on-screen tiles
	int _woodsCount = 0, _grassCount = 0, _waterCount = 0;	// Tiles surrounding entered locations
	bool _isLordBritishCastle = false;

public:
	/**
	 * Initializes the map, loading necessary data.
	 */
	void init();

	/**
	 * Loads a specified map.
	 * @param mapNum 
	 */
	void load(int mapNum);

	/**
	 * Returns the map width
	 */
	int width() const {
		return _mapWidth;
	}

	/**
	 * Returns the map height
	 */
	int height() const {
		return _mapHeight;
	}

	/**
	 * Access tiles of the current map
	 */
	const Row &operator[](int y) const {
		if (_outsideMapTile != 0xff && (y < 0 || y >= _mapHeight))
			// In a row outside the map
			return _mapRows[_mapHeight];

		// Return valid row
		if (y < 0)
			return _mapRows[_mapHeight + y];
		return _mapRows[y % _mapHeight];
	}

	void clearTiles();

	/**
	 * Get a tile at a given position.
	 * @param x		X position
	 * @param y		Y position
	 * @return		Tile Id
	 */
	int getMapTile(int x, int y) const;

	/**
	 * Returns the continent number at a given position
	 */
	int getContinentAt(int x, int y) const;
	int getContinentAt(const Common::Point &pt) const {
		return getContinentAt(pt.x, pt.y);
	}

	/**
	 * Get the location at a given position
	 */
	int getLocationAt(int x, int y) const;
	int getLocationAt(const Common::Point &pt) const {
		return getLocationAt(pt.x, pt.y);
	}

	/**
	 * Returns true if a map X co-ordinate is far enough from a
	 * continent boundary to be a valid creature spawn/move position
	 */
	bool mapRangeCheckX(int xp) const;

	/**
	 * Returns true if a map Y co-ordinate is far enough from a
	 * continent boundary to be a valid creature spawn/move position
	 */
	bool mapRangeCheckY(int yp) const;
};

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
