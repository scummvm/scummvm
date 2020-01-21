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

#include "ultima/ultima1/map/map.h"
#include "ultima/ultima1/map/map_city_castle.h"
#include "ultima/ultima1/map/map_dungeon.h"
#include "ultima/ultima1/map/map_overworld.h"
#include "ultima/ultima1/core/dungeon_widgets.h"
#include "ultima/ultima1/core/people.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/game_state.h"
#include "ultima/shared/core/file.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace Map {

void SurroundingTotals::load(Ultima1Map *map) {
	U1MapTile mapTile;
	_water = _woods = _grass = 0;
	
	// Iterate through the surrounding tiles relative to the player
	for (int deltaY = -1; deltaY <= 1; ++deltaY) {
		for (int deltaX = -1; deltaX <= 1; ++deltaX) {
			Point delta(deltaX * map->getTilesPerOrigTile().x, deltaY * map->getTilesPerOrigTile().y);
			map->getTileAt(map->getDeltaPosition(delta), &mapTile);

			if (mapTile.isOriginalWater())
				++_water;
			else if (mapTile.isOriginalGrass())
				++_grass;
			else if (mapTile.isOriginalWoods())
				++_woods;
		}
	}
}

/*------------------------------------------------------------------------*/

Ultima1Map::MapBase::MapBase(Ultima1Game *game) : Shared::Map::MapBase(game), _game(game) {
}

void Ultima1Map::MapBase::getTileAt(const Point &pt, Shared::MapTile *tile) {
	Shared::Map::MapBase::getTileAt(pt, tile);

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
		mapTile->_map = this;

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

/*------------------------------------------------------------------------*/

void U1MapTile::clear() {
	_map = nullptr;
	_locationNum = -1;
}

bool U1MapTile::isWater() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == 0;
}

bool U1MapTile::isGrass() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == 1;
}

bool U1MapTile::isWoods() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == 2;
}

bool U1MapTile::isOriginalWater() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == 0;
}

bool U1MapTile::isOriginalGrass() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == 1;
}

bool U1MapTile::isOriginalWoods() const {
	return dynamic_cast<MapOverworld *>(_map) && _tileId == 2;
}

bool U1MapTile::isGround() const {
	if (dynamic_cast<MapCityCastle *>(_map) && (_tileId == 1 || _tileId >= 51))
		return true;
	else if (dynamic_cast<MapOverworld *>(_map))
		return _tileId != 0;
	return false;
}

/*------------------------------------------------------------------------*/

Ultima1Map::Ultima1Map(Ultima1Game *game) : Shared::Map(), _game(game), _mapType(MAP_OVERWORLD) {
	Ultima1Map::clear();
	_mapCityCastle = new MapCityCastle(game);
	_mapDungeon = new MapDungeon(game);
	_mapOverworld = new MapOverworld(game);
}

Ultima1Map::~Ultima1Map() {
	delete _mapCityCastle;
	delete _mapDungeon;
	delete _mapOverworld;
}

void Ultima1Map::clear() {
	_mapType = MAP_OVERWORLD;
}

void Ultima1Map::load(Shared::MapId mapId) {
	Shared::Map::load(mapId);

	// Switch to the correct map area
	if (mapId == MAPID_OVERWORLD) {
		_mapType = MAP_OVERWORLD;
		_mapArea = _mapOverworld;
	} else if (mapId < 41) {
		_mapArea = _mapCityCastle;
	} else if (mapId < 49) {
		error("TODO: load Pillar");
	} else {
		_mapArea = _mapDungeon;
	}

	// Load the map
	_mapArea->load(mapId);
}

bool Ultima1Map::isLordBritishCastle() const {
	return _mapType == MAP_CASTLE && static_cast<MapCityCastle *>(_mapArea)->getMapIndex() == 0;
}

} // End of namespace Map
} // End of namespace Ultima1
} // End of namespace Ultima
