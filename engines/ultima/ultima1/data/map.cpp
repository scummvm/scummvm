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

void Map::load(int mapNum) {
	const auto &player = g_engine->_player;

	if (mapNum == _currentMap)
		return;
	_currentMap = mapNum;

	Common::File f;
	if (!f.open(Common::String::format("MAPX%.2d", mapNum).c_str()))
		error("Could not open map %d", mapNum);

	f.read(_tiles, MAP_WIDTH * MAP_HEIGHT);

	// Set up copies of the map position and player tile to use
	_mapX = player._position.x;
	_mapY = player._position.y;
	_playerTileId = kTileParty;
	clearTiles();
}

void Map::clearTiles() {
	Common::fill(&_mapTilesId[0][0], &_mapTilesId[0][0] + sizeof(VisibleTiles), 0);
}

bool Map::canMoveToTile(int tileNum) {
	int tileId = tileNum & 0x7f;

	if ((uint)_mapX < Data::MAP_WIDTH && (uint)_mapY < Data::MAP_HEIGHT) {
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
