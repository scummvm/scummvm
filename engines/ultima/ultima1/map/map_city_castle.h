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

#include "ultima/ultima1/map/map.h"

namespace Ultima {
namespace Ultima1 {
namespace Map {

enum CityTile {
	CTILE_GATE = 11
};

class MapCityCastle : public Ultima1Map::MapBase {
private:
	/**
	 * Load widget list for the map
	 */
	void loadWidgets();

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
public:
	uint _castleKey;					// Key for castle map lock
public:
	/**
	 * Constructor
	 */
	MapCityCastle(Ultima1Game *game) : Ultima1Map::MapBase(game), _castleKey(0) {}

	/**
	 * Destructor
	 */
	virtual ~MapCityCastle() {}

	/**
	 * Load the map
	 */
	virtual void load(Shared::MapId mapId);

	/**
	 * Get the viewport position
	 */
	virtual Point getViewportPosition(const Point &viewportSize);
};

} // End of namespace Map
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
