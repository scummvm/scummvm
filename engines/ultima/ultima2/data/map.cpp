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
#include "ultima/ultima2/data/map.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
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
	_mapX = player._mapX;
	_mapY = player._mapY;
	_playerTileId = (player._class + Data::PLAYER_TILES_OFFSET) * 2;
	clearTiles();
}

void Map::clearTiles() {
	Common::fill(&_mapTilesId[0][0], &_mapTilesId[0][0] + sizeof(VisibleTiles), 0);
	Common::fill(&_priorTileIds[0][0], &_priorTileIds[0][0] + sizeof(VisibleTiles), 0);
}

bool Map::canMoveToTile(int tileNum) {
	auto &game = g_engine->_game;
	const auto &player = g_engine->_player;
	int tileId = (tileNum & 0x7f) / 2;

	if (IS_PLANET(player._mapNum) || ((uint)_mapX < Data::MAP_WIDTH && (uint)_mapY < Data::MAP_HEIGHT)) {
		// Within the bounds of the map
		if (tileId == kTileSwamp) {
			game.subtractHp(5);
		} else if (tileId == kTileForcefield) {
			if (player._hasRing) {
				game.message("INFO", "\x8D""FIELD CAUSES 1000 DAMAGE!");
				game.message("CIRCLE");		// Flash damage circle

				game.subtractHp(1000);
			} else {
				game.message("INFO", "RING PROTECTS FROM FIELD!");
				g_engine->pauseMillis();
			}
		}

		// Check paralyzed
		if (player._paralyzedFlag) {
			game.message("INFO", "--PARALIZED!");
			return true;
		}

		switch (_playerTileId / 2) {
		case kTileRocket * 2:
			return true;
		case kTileAirplane:
			// TODO
			error("TODO: airplane");
			break;
		case kTileShip:
			if (tileId != kTileWater)
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
} // namespace Ultima2
} // namespace Ultima
