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

class Ultima1Game;

namespace Map {

enum MapType {
	MAP_OVERWORLD = 0, MAP_CITY = 1, MAP_CASTLE = 2, MAP_DUNGEON = 3, MAP_UNKNOWN = 4
};

enum MapIdent {
	MAPID_OVERWORLD = 0
};

class U1MapTile;
class Ultima1Map;
class MapCity;
class MapCastle;
class MapDungeon;
class MapOverworld;

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

/**
 * Ultima 1 map manager
 */
class Ultima1Map : public Shared::Map {
public:
	/**
	 * Intermediate base class for Ultima 1 map types
	 */
	class MapBase : public Shared::Map::MapBase {
	protected:
		Ultima1Game *_game;
	public:
		/**
		 * Constructor
		 */
		MapBase(Ultima1Game *game, Ultima1Map *map);
		
		/**
		 * Destructor
		 */
		virtual ~MapBase() {}

		/**
		 * Gets a tile at a given position
		 */
		virtual void getTileAt(const Point &pt, Shared::MapTile *tile) override;
	};

private:
	Ultima1Game *_game;
	MapCity *_mapCity;
	MapCastle *_mapCastle;
	MapDungeon *_mapDungeon;
	MapOverworld *_mapOverworld;
public:
	MapType _mapType;					// Type of map
	Point _worldPos;					// Point in the world map, updated when entering locations
public:
	/**
	 * Constructor
	 */
	Ultima1Map(Ultima1Game *game);

	/**
	 * Destructor
	 */
	virtual ~Ultima1Map();

	/**
	 * Clears all map data
	 */
	virtual void clear();

	/**
	 * Load a given map
	 */
	virtual void load(Shared::MapId mapId) override;

	/**
	 * Handles loading and saving the map's data
	 */
	virtual void synchronize(Common::Serializer &s) override;

	/**
	 * Returns true if Lord British's castle is the currently active map
	 */
	bool isLordBritishCastle() const;

	/**
	 * Instantiates a widget type by name
	*/
	virtual Shared::MapWidget *createWidget(Shared::Map::MapBase *map, const Common::String &name) override;
};

/**
 * Derived map tile class for Ultima 1 that adds extra properties
 */
class U1MapTile : public Shared::MapTile {
	friend class Ultima1Map;
private:
	Ultima1Map::MapBase *_map;
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

} // End of namespace Map
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
