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

#ifndef ULTIMA_ULTIMA1_MAPS_MAP_H
#define ULTIMA_ULTIMA1_MAPS_MAP_H

#include "ultima/shared/maps/map.h"
#include "ultima/shared/maps/map_widget.h"
#include "ultima/ultima1/maps/map_base.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

namespace Maps {

enum MapType {
	MAP_OVERWORLD = 0, MAP_CITY = 1, MAP_CASTLE = 2, MAP_DUNGEON = 3, MAP_UNKNOWN = 4
};

enum MapIdent {
	MAPID_OVERWORLD = 0
};

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
class Ultima1Map : public Shared::Maps::Map {
private:
//	Ultima1Game *_game;
	MapCity *_mapCity;
	MapCastle *_mapCastle;
	MapDungeon *_mapDungeon;
	MapOverworld *_mapOverworld;
public:
	MapType _mapType;					// Type of map
	Point _worldPos;					// Point in the world map, updated when entering locations
	uint _moveCounter;					// Movement counter
public:
	/**
	 * Constructor
	 */
	Ultima1Map(Ultima1Game *game);

	/**
	 * Destructor
	 */
	~Ultima1Map() override;

	/**
	 * Clears all map data
	 */
	void clear() override;

	/**
	 * Load a given map
	 */
	void load(Shared::Maps::MapId mapId) override;

	/**
	 * Handles loading and saving the map's data
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Action pass-throughs
	 */
	#define PASS_METHOD(NAME) void NAME() { static_cast<MapBase *>(_mapArea)->NAME(); }
	PASS_METHOD(board)
	PASS_METHOD(cast)
	PASS_METHOD(drop)
	PASS_METHOD(enter)
	PASS_METHOD(get)
	PASS_METHOD(hyperjump)
	PASS_METHOD(inform)
	PASS_METHOD(climb)
	PASS_METHOD(open)
	PASS_METHOD(steal)
	PASS_METHOD(talk)
	PASS_METHOD(unlock)
	PASS_METHOD(view)
	PASS_METHOD(disembark)

	void attack(int direction, int effectId) {
		static_cast<MapBase *>(_mapArea)->attack(direction, effectId);
	}

	/**
	 * Handles dropping an amount of coins
	 */
	void dropCoins(uint amount);

	/**
	 * Returns the overworld map
	 */
	MapOverworld *getOverworldMap() { return _mapOverworld; }
};

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
