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

#include "common/array.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/data/map_dungeon.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

constexpr int OVERWORLD_WIDTH = 168;
constexpr int OVERWORLD_HEIGHT = 156;
constexpr int OVERWORLD_SIZE = OVERWORLD_WIDTH * OVERWORLD_HEIGHT;
constexpr int OVERWORLD_VISIBLE_WIDTH = 19;
constexpr int OVERWORLD_VISIBLE_HEIGHT = 9;
constexpr int OVERWORLD_VISIBLE_CENTER_X = 9;
constexpr int OVERWORLD_VISIBLE_CENTER_Y = 4;

constexpr int CITY_WIDTH = 38;
constexpr int CITY_HEIGHT = 18;
constexpr int CITY_SIZE = CITY_WIDTH * CITY_HEIGHT;

constexpr int LOCATION_COUNT = 84;

enum {
	MAP_OVERWORLD = 0,
	MAP_CITY1 = 1
};

enum Direction {
	DIR_LEFT = 1, DIR_RIGHT = 2, DIR_UP = 3, DIR_DOWN = 4
};

enum MapType {
	MAPTYPE_OVERWORLD = 0,
	MAPTYPE_CITY = 1,
	MAPTYPE_CASTLE = 2,
	MAPTYPE_DUNGEON = 3
};

typedef Common::Array< Common::Array<byte> > MapTiles;

extern const char *DIRECTION_NAMES[5];
extern const char *LOCATION_NAMES[LOCATION_COUNT];
extern const char *CONTINENT_NAMES[4];

struct Map : public MapDungeon {
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
		byte *_data = nullptr;

	public:
		Row() {}
		Row(Map *owner, byte *data) : _owner(owner), _data(data) {}

		// Give access to the byte for a column within the row. Returns a
		// mutable reference (even though the method is const - constness
		// here is about the Row's own members, not the mapped bytes) so
		// map[y][x] = value can be used to modify the map's contents
		byte &operator[](int x) const {
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
	Graphics::ManagedSurface _overworldTiles[OVERWORLD_TILES_COUNT];
	Graphics::ManagedSurface _cityTiles[CITY_TILES_COUNT];

	/**
	 * Used to generate the dungeon.
	 */
	void loadDungeonMap();

public:
	int _currentMap = -1;
	byte _playerTileId = 0;
	byte _tilePlayerCenter = 0;
	byte _tilePlayerUp = 0, _tilePlayerDown = 0,
		_tilePlayerLeft = 0, _tilePlayerRight = 0;
	int _woodsCount = 0, _grassCount = 0, _waterCount = 0;	// Tiles surrounding entered locations
	bool _isLordBritishCastle = false;
	int _mapStyle = 0;
	int _mapType = MAPTYPE_OVERWORLD;
	int _tavernCtr = 0;
	int _castleNum = 0, _castleNum2 = 0;
	Graphics::ManagedSurface *_tiles = nullptr;

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
	 * Get the tiles
	 */
	const Graphics::ManagedSurface *tiles() const {
		return _tiles;
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

	/**
	 * Get a tile at a given position.
	 * @param x		X position
	 * @param y		Y position
	 * @return		Tile Id
	 */
	int getMapTile(int x, int y) const;

	/**
	 * Returns a tile or creature/entity at a given co-ordinate
	 */
	int getTileAt(int x, int y, int creatureIndex = 1) const;

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
