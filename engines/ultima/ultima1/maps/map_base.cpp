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

#include "ultima/ultima1/maps/map_base.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

MapBase::MapBase(Ultima1Game *game, Ultima1Map *map) : Shared::Maps::MapBase(game, map), _game(game) {
}

void MapBase::getTileAt(const Point &pt, Shared::Maps::MapTile *tile) {
	Shared::Maps::MapBase::getTileAt(pt, tile);

	// Special handling for one of the city/town tile numbers
	if (tile->_tileNum >= 51 && dynamic_cast<MapCityCastle *>(this))
		tile->_tileNum = 1;

	// Setting dungeon flags
	if (dynamic_cast<MapDungeon *>(this)) {
		tile->_isHallway = tile->_tileNum == DTILE_HALLWAY;
		tile->_isDoor = tile->_tileNum == DTILE_DOOR;
		tile->_isSecretDoor = tile->_tileNum == DTILE_SECRET_DOOR;
		tile->_isWall = tile->_tileNum == DTILE_WALL;
		tile->_isLadderUp = tile->_tileNum == DTILE_LADDER_UP;
		tile->_isLadderDown = tile->_tileNum == DTILE_LADDER_DOWN;
		tile->_isBeams = tile->_tileNum == DTILE_BEAMS;
	}

	// Extended properties to set if an Ultima 1 map tile structure was passed in
	U1MapTile *mapTile = dynamic_cast<U1MapTile *>(tile);
	if (mapTile) {
		GameResources *res = _game->_res;
		mapTile->setMap(this);

		// Check for a location at the given position
		mapTile->_locationNum = -1;
		if (dynamic_cast<MapOverworld *>(this)) {
			for (int idx = 0; idx < LOCATION_COUNT; ++idx) {
				if (pt.x == res->LOCATION_X[idx] && pt.y == res->LOCATION_Y[idx]) {
					mapTile->_locationNum = idx + 1;
					break;
				}
			}
		}
	}
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
