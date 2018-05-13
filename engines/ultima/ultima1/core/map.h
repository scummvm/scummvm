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

#ifndef ULTIMA_ULTIMA1_CORE_MAP_H
#define ULTIMA_ULTIMA1_CORE_MAP_H

#include "ultima/shared/core/map.h"

namespace Ultima {
namespace Ultima1 {

#define DUNGEON_WIDTH 11
#define DUNGEON_HEIGHT 11

enum MapType {
	MAP_OVERWORLD = 0, MAP_CITY = 1, MAP_CASTLE = 2, MAP_DUNGEON = 3, MAP_UNKNOWN = 4
};

enum MapId {
	MAPID_OVERWORLD = 0
};

class Ultima1Game;
class Ultima1Map;
class WidgetTransport;

class U1MapTile : public Shared::MapTile {
	friend class Ultima1Map;
private:
	Ultima1Map *_map;
public:
	int _locationNum;
public:
	/**
	 * Clears tile data
	 */
	virtual void clear();

	/**
	 * Return true if the tile base is water
	 */
	bool isWater() const;

	/**
	 * Return true if the tile base is grass
	 */
	bool isGrass() const;

	/**
	 * Return true if the tile base is woods
	 */
	bool isWoods() const;

	/**
	 * Return true if the tile base in the original map is water
	 */
	bool isOriginalWater() const;

	/**
	 * Return true if the tile base in the original map is grass
	 */
	bool isOriginalGrass() const;

	/**
	 * Return true if the tile base in the original map is woods
	 */
	bool isOriginalWoods() const;

	/**
	 * Returns true if the tile is a ground type tool
	 */
	bool isGround() const;
};

/**
 * Used to hold the total number of tiles surrounding location entrances
 */
struct SurroundingTotals {
	uint _water;
	uint _grass;
	uint _woods;

	/**
	 * Constructor
	 */
	SurroundingTotals() : _water(0), _grass(0), _woods(0) {}

	/**
	 * Loads the totals from a passed map
	 */
	void load(Ultima1Map *map);
};

class Ultima1Map : public Shared::Map {
private:
	Ultima1Game *_game;
private:
	/**
	 * Clears Ultima 1 extended fields
	 */
	void clearFields();

	/**
	 * Load the overworld map
	 */
	void loadOverworldMap();

	/**
	 * Load a town/castle map
	 */
	void loadTownCastleMap();

	/**
	 * Load the base map for towns and castles
	 */
	void loadTownCastleData();

	/**
	 * Loads a town/city
	 */
	void loadTown();

	/**
	 * Loads a castle
	 */
	void loadCastle();

	/**
	 * Load widget list for the given map
	 */
	void loadWidgets();

	/**
	 * Loads a underworld/dungeon map
	 */
	void loadDungeonMap();
public:
	MapType _mapType;					// Type of map
	uint _mapStyle;						// Map style category for towns & castles
	uint _mapIndex;						// Map index within cateogry, such as city/castle #; not to be confused with mapId
	Common::String _name;				// Name of map, if applicable
	WidgetTransport *_currentTransport;	// Current means of transport, even if on foot
	uint _castleKey;					// Key for castle map lock
public:
	/**
	 * Constructor
	 */
	Ultima1Map(Ultima1Game *game);

	/**
	 * Load a given map
	 */
	virtual void loadMap(int mapId, uint videoMode);

	/**
	 * Gets a tile at a given position
	 */
	virtual void getTileAt(const Point &pt, Shared::MapTile *tile);

	/**
	 * Returns true if Lord British's castle is the currently active map
	 */
	bool isLordBritishCastle() const { return _mapType == MAP_CASTLE && _mapIndex == 0; }

	/**
	 * Spawns a monster within dungeons
	 */
	void spawnMonster();
};

} // End of namespace Ultima1
} // End of namespace Ultima

#endif
