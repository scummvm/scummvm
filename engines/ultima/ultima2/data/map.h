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

#ifndef ULTIMA2_DATA_MAP_H
#define ULTIMA2_DATA_MAP_H

#include "common/archive.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

#define IS_PLANET(MAPNUM) ((MAPNUM % 10) == 0)
#define IS_CITY(MAPNUM) ((MAPNUM % 10) >= 1 && (MAPNUM % 10) <= 3)
#define IS_DUNGEON(MAPNUM) ((MAPNUM % 10) >= 4)

constexpr int MAP_WIDTH = 64;
constexpr int MAP_HEIGHT = 66;
constexpr int MAP_VISIBLE_WIDTH = 20;
constexpr int MAP_VISIBLE_HEIGHT = 10;
constexpr int MAP_VISIBLE_CENTER_X = 9;
constexpr int MAP_VISIBLE_CENTER_Y = 4;
constexpr int PLAYER_TILES_OFFSET = 60;
typedef byte MapTiles[MAP_HEIGHT][MAP_WIDTH];
typedef byte VisibleTiles[MAP_VISIBLE_HEIGHT][MAP_VISIBLE_WIDTH];

struct Map {
private:
	int _currentMap = -1;
public:
	int _mapX = 0, _mapY = 0;
	byte _playerTileId = 0;

	MapTiles _tiles = {};
	int8 _outsideMapTile = -1;
	byte _tilePlayerCenter = 0;
	byte _tilePlayerUp = 0, _tilePlayerDown = 0,
		_tilePlayerLeft = 0, _tilePlayerRight = 0;
	VisibleTiles _mapTilesId = {};		// Calculated visible on-screen tiles
	VisibleTiles _priorTileIds = {};	// Prior on-screen tiles, used to determine which to draw

	const byte _flag1 = 0;

	void load(int mapNum);
	void clearTiles();

	bool canMoveToTile(int tileNum);
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
