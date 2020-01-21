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

#ifndef ULTIMA_ULTIMA1_MAP_MAP_CITY_CASTLE_H
#define ULTIMA_ULTIMA1_MAP_MAP_CITY_CASTLE_H

#include "ultima/ultima1/maps/map_base.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

enum CityTile {
	CTILE_1 = 1, CTILE_51 = 51, CTILE_GATE = 11
};

/**
 * Common base class for city and castle maps
 */
class MapCityCastle : public MapBase {
protected:
	/**
	 * Load widget list for the map
	 */
	void loadWidgets();

	/**
	 * Load the base map for towns and castles
	 */
	void loadTownCastleData();
public:
	bool _guardsHostile;			// Flag for whether guards are hostile
public:
	/**
	 * Constructor
	 */
	MapCityCastle(Ultima1Game *game, Ultima1Map *map) : MapBase(game, map), _guardsHostile(false) {}

	/**
	 * Destructor
	 */
	virtual ~MapCityCastle() {}

	/**
	 * Load the map
	 */
	virtual void load(Shared::Maps::MapId mapId) override;

	/**
		* Clears all map data
		*/
	virtual void clear() override;

	/**
	 * Get the viewport position
	 */
	virtual Point getViewportPosition(const Point &viewportSize) override;
};

/**
 * City map
 */
class MapCity : public MapCityCastle {
public:
	/**
	 * Constructor
	 */
	MapCity(Ultima1Game *game, Ultima1Map *map) : MapCityCastle(game, map) {}

	/**
	 * Destructor
	 */
	virtual ~MapCity() {}

	/**
	 * Load the map
	 */
	virtual void load(Shared::Maps::MapId mapId) override;
};

/**
 * Castle map
 */
class MapCastle : public MapCityCastle {
public:
	uint _castleKey;					// Key for castle map lock
public:
	/**
	 * Constructor
	 */
	MapCastle(Ultima1Game *game, Ultima1Map *map) : MapCityCastle(game, map), _castleKey(0) {}

	/**
	 * Destructor
	 */
	virtual ~MapCastle() {}

	/**
	 * Load the map
	 */
	virtual void load(Shared::Maps::MapId mapId) override;
};

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
