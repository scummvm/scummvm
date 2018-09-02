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

#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map_city_castle.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

void U1MapTile::clear() {
	_map = nullptr;
	_locationNum = -1;
}

bool U1MapTile::isWater() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == TILE_WATER;
}

bool U1MapTile::isGrass() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == TILE_GRASS;
}

bool U1MapTile::isWoods() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == TILE_WOODS;
}

bool U1MapTile::isOriginalWater() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == TILE_WATER;
}

bool U1MapTile::isOriginalGrass() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == TILE_GRASS;
}

bool U1MapTile::isOriginalWoods() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == TILE_WOODS;
}

bool U1MapTile::isGround() const {
	if (dynamic_cast<MapCityCastle *>(_map) && (_tileId == 1 || _tileId >= 51))
		return true;
	else if (dynamic_cast<MapOverworld *>(_map))
		// Not water or mountains
		return _tileId != TILE_WATER && _tileId != TILE_MOUNTAINS;
	return false;
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
