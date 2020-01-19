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

#include "ultima/ultima1/core/map.h"
#include "ultima/ultima1/core/people.h"
#include "ultima/ultima1/core/transports.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/file.h"

namespace Ultima {
namespace Ultima1 {

using Shared::File;

void SurroundingTotals::load(Ultima1Map *map) {
	U1MapTile mapTile;
	_water = _woods = _grass = 0;
	
	// Iterate through the surrounding tiles relative to the player
	for (int deltaY = -1; deltaY <= 1; ++deltaY) {
		for (int deltaX = -1; deltaX <= 1; ++deltaX) {
			Point delta(deltaX * map->_tilesPerOrigTile.x, deltaY * map->_tilesPerOrigTile.y);
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

void U1MapTile::clear() {
	_map = nullptr;
	_locationNum = -1;
}

bool U1MapTile::isWater() const {
	return _map->_mapType == MAP_OVERWORLD && _tileId == 0;
}

bool U1MapTile::isGrass() const {
	return _map->_mapType == MAP_OVERWORLD && _tileId == 1;
}

bool U1MapTile::isWoods() const {
	return _map->_mapType == MAP_OVERWORLD && _tileId == 2;
}

bool U1MapTile::isOriginalWater() const {
	return _map->_mapType == MAP_OVERWORLD && _tileId == 0;
}

bool U1MapTile::isOriginalGrass() const {
	return _map->_mapType == MAP_OVERWORLD && _tileId == 1;
}

bool U1MapTile::isOriginalWoods() const {
	return _map->_mapType == MAP_OVERWORLD && _tileId == 2;
}

bool U1MapTile::isGround() const {
	if ((_map->_mapType == MAP_CITY || _map->_mapType == MAP_CASTLE) && (_tileId == 1 || _tileId >= 51))
		return true;
	else if (_map->_mapType == MAP_OVERWORLD)
		return _tileId != 0;
	return false;
}

/*------------------------------------------------------------------------*/

Ultima1Map::Ultima1Map(Ultima1Game *game) : Shared::Map(), _game(game), _mapType(MAP_OVERWORLD) {
	clearFields();
}

void Ultima1Map::clearFields() {
	_currentTransport = nullptr;
	_mapType = MAP_OVERWORLD;
	_mapStyle = _mapIndex = 0;
	_name.clear();
	_fixed = false;
	_castleKey = 0;
}

void Ultima1Map::loadMap(int mapId, uint videoMode) {
	Map::loadMap(mapId, videoMode);

	if (mapId == MAPID_OVERWORLD)
		loadOverworldMap();
	else
		loadTownCastleMap();
}

void Ultima1Map::loadOverworldMap() {
	_mapType = MAP_OVERWORLD;
	_size = Point(168, 156);
	_tilesPerOrigTile = Point(1, 1);
	_data.resize(_size.x * _size.y);

	File f("map.bin");
	byte b;
	for (int y = 0; y < _size.y; ++y) {
		for (int x = 0; x < _size.x; x += 2) {
			b = f.readByte();
			_data[y * _size.x + x] = b >> 4;
			_data[y * _size.x + x + 1] = b & 0xf;
		}
	}

	// Load widgets
	loadWidgets();
}

void Ultima1Map::loadTownCastleMap() {
	_size = Point(38, 18);
	_tilesPerOrigTile = Point(1, 1);
	_data.resize(_size.x * _size.y);
	_fixed = true;

	// Set up properties for the map
	if (_mapId < 33)
		// Town/city
		loadTown();
	else
		// Castle
		loadCastle();
}

void Ultima1Map::loadTownCastleData() {
	// Load the contents of the map
	File f("tcd.bin");
	f.seek(_mapStyle * 684);
	for (int x = 0; x < _size.x; ++x) {
		for (int y = 0; y < _size.y; ++y)
			_data[y * _size.x + x] = f.readByte();
	}
}

void Ultima1Map::loadTown() {
	_mapType = MAP_CITY;
	_mapStyle = (_mapId % 8) + 2;
	_mapIndex = _mapId;
	_name = Common::String::format("%s %s", _game->_res->THE_CITY_OF, _game->_res->LOCATION_NAMES[_mapId - 1]);

	loadTownCastleData();

	// Load up the widgets for the given map
	loadWidgets();
	setPosition(Common::Point(width() / 2, height() - 1));		// Start at bottom center edge of map
}

void Ultima1Map::loadCastle() {
	_mapType = MAP_CASTLE;
	_mapIndex = _mapId - 33;
	_mapStyle = _mapIndex % 2;
	_name = _game->_res->LOCATION_NAMES[_mapId - 1];
	_castleKey = _game->getRandomNumber(255) & 1 ? 61 : 60;

	loadTownCastleData();

	// Set up door locks
	_data[35 + (_mapStyle ? 4 : 14) * _size.x] = 11;
	_data[31 + (_mapStyle ? 4 : 14) * _size.x] = 11;

	// Load up the widgets for the given map
	loadWidgets();
	setPosition(Common::Point(0, height() / 2));		// Start at center left edge of map
}

void Ultima1Map::getTileAt(const Point &pt, Shared::MapTile *tile) {
	Shared::Map::getTileAt(pt, tile);

	// Special handling for one of the city/town tile numbers
	if (tile->_tileNum >= 51 && (_mapType == MAP_CITY || _mapType == MAP_CASTLE))
		tile->_tileNum = 1;

	// Extended properties to set if an Ultima 1 map tile structure was passed in
	U1MapTile *mapTile = dynamic_cast<U1MapTile *>(tile);
	if (mapTile) {
		GameResources *res = _game->_res;
		mapTile->_map = this;

		// Check for a location at the given position
		mapTile->_locationNum = -1;
		if (_mapType == MAP_OVERWORLD) {
			for (int idx = 0; idx < LOCATION_COUNT; ++idx) {
				if (pt.x == res->LOCATION_X[idx] && pt.y == res->LOCATION_Y[idx]) {
					mapTile->_locationNum = idx + 1;
					break;
				}
			}
		}
	}
}

void Ultima1Map::loadWidgets() {
	_widgets.clear();

	// Set up widget for the player
	_currentTransport = new TransportOnFoot(_game, this);
	addWidget(_currentTransport);

	if (_mapType == MAP_CITY || _mapType == MAP_CASTLE) {
		for (int idx = 0; idx < 15; ++idx) {
			const int *lp = _game->_res->LOCATION_PEOPLE[_mapStyle * 15 + idx];
			if (lp[0] == -1)
				break;

			Person *person = new Person(_game, this, lp[0], lp[3]);
			person->_position = Point(lp[1], lp[2]);
			addWidget(person);
		}
	}
}

} // End of namespace Ultima1
} // End of namespace Ultima
