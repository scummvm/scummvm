/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

constexpr int OVERWORLD_WIDTH = 168;
constexpr int OVERWORLD_HEIGHT = 168;
constexpr int OVERWORLD_SIZE = OVERWORLD_WIDTH * OVERWORLD_HEIGHT;
constexpr int CITY_WIDTH = 38;
constexpr int CITY_HEIGHT = 18;
constexpr int CITY_SIZE = CITY_WIDTH * CITY_HEIGHT;

void Map::init() {
	// Load the overworld map
	Common::File ow;
	byte v;
	byte *offset;

	if (!ow.open("map.bin"))
		error("Could not open map.bin");
	_overworldMap.reserve(OVERWORLD_SIZE);

	for (int i = 0; i < OVERWORLD_SIZE / 2; ++i) {
		v = ow.readByte();
		_overworldMap.push_back(v >> 4);
		_overworldMap.push_back(v & 0xf);
	}

	ow.close();

	// Load the city/castle maps
	Common::File tc;
	if (!tc.open("tcd.bin"))
		error("Could not open tcd.bin");

	for (int i = 0; i < 10; ++i) {
		auto &m = _cityMap[i];
		m.resize(CITY_SIZE);

		// Individual cities are top-to-bottom first, then left-to-right. For consistency
		// with the overworld, we switch them to left-to-right, top-to-bottom
		for (int x = 0; x < CITY_WIDTH; ++x) {
			offset = &m[x];

			for (int y = 0; y < CITY_HEIGHT; ++y, offset += CITY_WIDTH)
				*offset = tc.readByte();
		}
	}

	// Load the overworld map by default
	load(MAP_OVERWORLD);
}

void Map::load(int mapNum) {
	const auto &player = g_engine->_player;

	_currentMap = mapNum;

	if (mapNum == MAP_OVERWORLD) {
		_mapWidth = OVERWORLD_WIDTH;
		_mapHeight = OVERWORLD_HEIGHT;
		_outsideMapTile = 0xff;

		_mapRows.clear();
		_mapRows.reserve(OVERWORLD_HEIGHT);
		for (int y = 0; y < OVERWORLD_HEIGHT; ++y)
			_mapRows.push_back(Row(this, &_overworldMap[y * OVERWORLD_WIDTH]));

		_mapX = player._position.x;
		_mapY = player._position.y;

	} else {
		_mapWidth = CITY_WIDTH;
		_mapHeight = CITY_HEIGHT;
		_outsideMapTile = 0;

		_mapRows.clear();
		_mapRows.reserve(CITY_HEIGHT + 1);		// One extra row for out-of-bounds y access
		for (int y = 0; y < OVERWORLD_HEIGHT; ++y)
			_mapRows.push_back(Row(this, &_cityMap[mapNum - 1][y * CITY_WIDTH]));
		_mapRows.push_back(Row(this, nullptr));

		_mapX = 19;
		_mapY = 17;
	}

	// Set up copies of the map position and player tile to use
	_playerTileId = kTileParty;
	clearTiles();
}

void Map::clearTiles() {
	Common::fill(&_mapTilesId[0][0], &_mapTilesId[0][0] + sizeof(VisibleTiles), 0);
}

bool Map::canMoveToTile(int tileNum) {
	int tileId = tileNum & 0x7f;

	if (_mapX < _mapWidth && _mapY < _mapHeight) {
		// Within the bounds of the map
		switch (_playerTileId) {
		case kTileTimeMachine:
			return true;
		case kTileAircar:
			// TODO
			error("TODO: aircar");
			break;
		case kTileFrigate:
			if (tileId != kTileOcean)
				return true;

			goto update_xy;
		case kTileHorse:
			// TODO
			break;
		default:
			// TODO
			break;
		}
	} else {
		// TODO
	}

update_xy:
	_mapX &= 63;
	_mapY &= 63;

	return false;
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
