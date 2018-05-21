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
#include "ultima/ultima1/core/dungeon_widgets.h"
#include "ultima/ultima1/core/people.h"
#include "ultima/ultima1/core/transports.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/game_state.h"
#include "ultima/shared/core/file.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {

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

Ultima1Map::Ultima1Map(Ultima1Game *game) : Shared::Map(), _game(game), _mapType(MAP_OVERWORLD),
		_random("UltimaDungeons") {
	Ultima1Map::clear();
}

void Ultima1Map::clear() {
	_currentTransport = nullptr;
	_mapType = MAP_OVERWORLD;
	_mapStyle = _mapIndex = 0;
	_name.clear();
	_fixed = false;
	_castleKey = 0;
	_currentTransport = nullptr;
}

void Ultima1Map::loadMap(int mapId, uint videoMode) {
	Map::loadMap(mapId, videoMode);

	if (mapId == MAPID_OVERWORLD)
		loadOverworldMap();
	else if (mapId < 41)
		loadTownCastleMap();
	else if (mapId < 49)
		error("TODO: load Pillar");
	else
		loadDungeonMap();
}

void Ultima1Map::loadOverworldMap() {
	setDimensions(Point(168, 156));
	_mapType = MAP_OVERWORLD;
	_tilesPerOrigTile = Point(1, 1);

	Shared::File f("map.bin");
	byte b;
	for (int y = 0; y < _size.y; ++y) {
		for (int x = 0; x < _size.x; x += 2) {
			b = f.readByte();
			_data[y][x] = b >> 4;
			_data[y][x + 1] = b & 0xf;
		}
	}

	// Load widgets
	loadWidgets();
}

void Ultima1Map::loadTownCastleMap() {
	setDimensions(Point(38, 18));
	_tilesPerOrigTile = Point(1, 1);
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
	Shared::File f("tcd.bin");
	f.seek(_mapStyle * 684);
	for (int x = 0; x < _size.x; ++x) {
		for (int y = 0; y < _size.y; ++y)
			_data[y][x] = f.readByte();
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
	_data[_mapStyle ? 4 : 14][35] = CTILE_GATE;
	_data[_mapStyle ? 4 : 14][31] = CTILE_GATE;

	// Load up the widgets for the given map
	loadWidgets();
	setPosition(Common::Point(0, height() / 2));		// Start at center left edge of map
}

void Ultima1Map::getTileAt(const Point &pt, Shared::MapTile *tile) {
	Shared::Map::getTileAt(pt, tile);

	// Special handling for one of the city/town tile numbers
	if (tile->_tileNum >= 51 && (_mapType == MAP_CITY || _mapType == MAP_CASTLE))
		tile->_tileNum = 1;

	// Setting dungeon flags
	if (_mapType == MAP_DUNGEON) {
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

void Ultima1Map::loadDungeonMap() {
	setDimensions(Point(DUNGEON_WIDTH, DUNGEON_HEIGHT));
	setRandomSeed();
	_mapType = MAP_DUNGEON;
	_tilesPerOrigTile = Point(1, 1);
	_dungeonLevel = 1;
	_position = Point(1, 1);
	_direction = Shared::DIR_DOWN;

	changeDungeonLevel(0);
}

bool Ultima1Map::changeDungeonLevel(int delta) {
	_dungeonLevel += delta;
	if (_dungeonLevel <= 0)
		return false;

	// Set up widget for the player
	_currentTransport = new TransportOnFoot(_game, this);
	addWidget(_currentTransport);

	// Place walls around the edge of the map
	for (int y = 0; y < DUNGEON_HEIGHT; ++y) {
		_data[y][0] = DTILE_WALL;
		_data[y][DUNGEON_WIDTH - 1] = DTILE_WALL;
	}
	for (int x = 0; x < DUNGEON_WIDTH; ++x) {
		_data[0][x] = DTILE_WALL;
		_data[DUNGEON_HEIGHT - 1][x] = DTILE_WALL;
	}

	// Set up walls vertically across the dungeon
	for (int x = 2; x < (DUNGEON_WIDTH - 1); x += 2)
		for (int y = 2; y < (DUNGEON_HEIGHT - 1); y += 2)
			_data[y][x] = DTILE_WALL;

	// Randomly set up random tiles for all alternate positions in wall columns
	for (int x = 2; x < (DUNGEON_WIDTH - 1); x += 2)
		for (int y = 1; y < DUNGEON_HEIGHT; y += 2)
			_data[y][x] = getRandomNumber(DTILE_HALLWAY, DTILE_DOOR);

	// Set up wall and beams randomly to subdivide the blank columns
	const byte DATA1[15] = { 8, 5, 2, 8, 1, 5, 4, 6, 1, 3, 7, 3, 9, 2, 6 };
	const byte DATA2[15] = { 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 8, 8, 9, 9 };
	for (uint ctr = 0; ctr < (_dungeonLevel * 2); ++ctr) {
		byte newTile = (getRandomNumber(0, 255) <= 160) ? DTILE_WALL : DTILE_BEAMS;
		uint idx = getRandomNumber(0, 14);

		if (_dungeonLevel & 1) {
			_data[DATA2[idx]][DATA1[idx]] = newTile;
		} else {
			_data[DATA1[idx]][DATA2[idx]] = newTile;
		}
	}

	// Place chests and/or coffins randomly throughout the level
	_random.setSeed(_random.getSeed() + 1777);
	for (uint ctr = 0; ctr <= _dungeonLevel; ++ctr) {
		Point pt(getRandomNumber(10, 99) / 10, getRandomNumber(10, 99) / 10);
		byte currTile = _data[pt.y][pt.x];

		if (currTile != DTILE_WALL && currTile != DTILE_SECRET_DOOR && currTile != DTILE_BEAMS) {
			_widgets.push_back(Shared::MapWidgetPtr(new DungeonWidget(_game, this, pt,
				(getRandomNumber(1, 100) & 1) ? DITEM_COFFIN : DITEM_CHEST)));
		}
	}

	// Set up ladders
	_data[2][1] = DTILE_HALLWAY;
	if (_dungeonLevel & 1) {
		_data[3][7] = DTILE_LADDER_UP;
		_data[6][3] = DTILE_LADDER_DOWN;
	} else {
		_data[3][7] = DTILE_LADDER_DOWN;
		_data[6][3] = DTILE_LADDER_UP;
	}

	if (_dungeonLevel == 10)
		_data[3][7] = DTILE_HALLWAY;
	if (_dungeonLevel == 1) {
		_data[1][1] = DTILE_LADDER_UP;
		_data[3][7] = DTILE_HALLWAY;
	}

	for (int ctr = 0; ctr < 3; ++ctr)
		spawnMonster();

	return true;
}

void Ultima1Map::setRandomSeed() {
	const Shared::GameState &gs = *_game->_gameState;
	uint32 seed = gs._randomSeed + gs._worldMapPos.x * 5 + gs._worldMapPos.y * 3 + _dungeonLevel * 17;
	_random.setSeed(seed);
}

void Ultima1Map::spawnMonster() {
	// TODO
}


} // End of namespace Ultima1
} // End of namespace Ultima
