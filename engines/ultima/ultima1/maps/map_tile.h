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

#ifndef ULTIMA_ULTIMA1_MAPS_MAP_TILE_H
#define ULTIMA_ULTIMA1_MAPS_MAP_TILE_H

#include "ultima/shared/maps/map_tile.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {
class DungeonItem;
}

namespace Maps {

enum TileId {
	TILE_WATER = 0, TILE_GRASS = 1, TILE_WOODS = 2, TILE_MOUNTAINS = 3
};

class MapBase;
class Ultima1Map;

/**
 * Derived map tile class for Ultima 1 that adds extra properties
 */
class U1MapTile : public Shared::Maps::MapTile {
private:
	MapBase *_map;
public:
	int _locationNum;
	Widgets::DungeonItem *_item;
public:
	/**
	 * Constructor
	 */
	U1MapTile() : Shared::Maps::MapTile(), _item(0), _locationNum(-1), _map(nullptr) {}

	/**
	 * Set the active map
	 */
	void setMap(MapBase *map) { _map = map; }

	/**
	 * Clears tile data
	 */
	void clear() override;

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

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
