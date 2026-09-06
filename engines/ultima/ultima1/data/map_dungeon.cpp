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

#include "ultima/ultima1/data/map_dungeon.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

MapDungeon::MapDungeon() {
	clear();
}

void MapDungeon::clear() {
	Common::fill(&_dungeonTiles[0][0], &_dungeonTiles[0][0] + DUNGEON_WIDTH * DUNGEON_HEIGHT, 0);
	for (auto &y : _cells) {
		for (auto &x : y) {
			x._itemId = DITEM_NONE;
			// Unlike _itemId, 0 is a real monster Id, so "no monster" needs
			// its own sentinel value
			x._monsterId = DUNGEON_NO_MONSTER;
			x._monsterHp = 0;
		}
	}
}

void MapDungeon::setRandomSeed() {
	_randomSeed = _G(savegame)._randomSeed + _G(savegame)._overworldPos.x * 5 +
		_G(savegame)._overworldPos.y * 3 + _G(savegame)._dungeonLevel * 17;
}

byte MapDungeon::getRandomNumber(int minVal, int maxVal) {
	// Rotate the seed right by 3 bits after adding a fixed value
	uint16 low3 = _randomSeed & 7;
	_randomSeed = (uint16)(_randomSeed >> 3) | (uint16)(low3 << 13);

	int clampedMax = (maxVal > 255) ? 255 : maxVal;
	int range = clampedMax - minVal;
	int result = _randomSeed & 0x3ff;
	while (result > range)
		result -= range;

	return (byte)(result + minVal);
}

void MapDungeon::generateDungeonLevel() {
	setRandomSeed();
	clear();

	int dungeonLevel = _G(savegame)._dungeonLevel;

	// Outer border walls
	for (int i = 0; i < DUNGEON_WIDTH; ++i) {
		_dungeonTiles[0][i] = DTILE_WALL;
		_dungeonTiles[DUNGEON_HEIGHT - 1][i] = DTILE_WALL;
		_dungeonTiles[i][0] = DTILE_WALL;
		_dungeonTiles[i][DUNGEON_WIDTH - 1] = DTILE_WALL;
	}

	// Randomize the tiles at the grid intersections that divide the level
	// into a rough 4x4 arrangement of rooms
	for (int x = 2; x <= 8; x += 2) {
		for (int y = 1; y <= 9; ++y)
			_dungeonTiles[y][x] = getRandomNumber(1, 4) - 1;
	}

	// Scatter some extra walls/support beams around the level, at one of
	// 16 predefined spots (mirrored across the diagonal on even levels, so
	// odd/even levels don't look identical)
	static const int DUNGEON_DATA1[16] = { 4, 8, 5, 2, 8, 1, 5, 4, 6, 1, 3, 7, 3, 9, 2, 6 };
	static const int DUNGEON_DATA2[16] = { 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 8, 8, 9, 9 };

	int decorCount = dungeonLevel * 2;
	for (int i = 0; i <= decorCount; ++i) {
		int newTile = (getRandomNumber(1, 255) > 160) ? DTILE_BEAMS : DTILE_WALL;
		int idx = getRandomNumber(1, 16) - 1;

		if (dungeonLevel & 1)
			_dungeonTiles[DUNGEON_DATA2[idx]][DUNGEON_DATA1[idx]] = newTile;
		else
			_dungeonTiles[DUNGEON_DATA1[idx]][DUNGEON_DATA2[idx]] = newTile;
	}

	// Re-seed partway through, so item placement below doesn't correlate
	// too closely with the maze layout generated above
	_randomSeed += 1777;

	// Scatter chests/coffins onto floor cells
	int itemCount = dungeonLevel;
	for (int i = 0; i <= itemCount; ++i) {
		int newItem = DTILE_CHEST + (getRandomNumber(1, 100) & 1);
		int x = getRandomNumber(10, 99) / 10;
		int y = getRandomNumber(10, 99) / 10;

		int tile = _dungeonTiles[y][x];
		if (tile != DTILE_WALL && tile != DTILE_SECRET_DOOR && tile != DTILE_BEAMS)
			_cells[y][x]._itemId = newItem;
	}

	// Place the ladders connecting to the levels above/below, alternating
	// which of the two fixed spots is up/down between odd/even levels
	_dungeonTiles[2][1] = DTILE_HALLWAY;
	if (dungeonLevel & 1) {
		_dungeonTiles[3][7] = DTILE_LADDER_UP;
		_dungeonTiles[7][3] = DTILE_LADDER_DOWN;
	} else {
		_dungeonTiles[3][7] = DTILE_LADDER_DOWN;
		_dungeonTiles[7][3] = DTILE_LADDER_UP;
	}

	if (dungeonLevel == DUNGEON_FLOORS)
		// No level below the deepest one
		_dungeonTiles[3][7] = DTILE_HALLWAY;

	if (dungeonLevel == 1) {
		// The topmost level connects back to the overworld instead, via a
		// separate fixed spot
		_dungeonTiles[1][1] = DTILE_LADDER_UP;
		_dungeonTiles[3][7] = DTILE_HALLWAY;
	}

	// Spawn 3 of the level's 5 possible monster slots
	for (int i = 0; i < DUNGEON_MONSTER_SLOTS; ++i)
		_monsterSlotUsed[i] = false;

	dungeonSpawnMonster();
	dungeonSpawnMonster();
	dungeonSpawnMonster();
}

void MapDungeon::dungeonSpawnMonster() {
	int x = 0, y = 0;
	bool valid = false;

	for (int attempt = 0; attempt <= 500 && !valid; ++attempt) {
		x = getRandomNumber(0, 242) % 9 + 1;
		y = getRandomNumber(0, 242) % 9 + 1;

		valid = _cells[y][x]._monsterId == DUNGEON_NO_MONSTER &&
			_dungeonTiles[y][x] != DTILE_WALL && _dungeonTiles[y][x] != DTILE_SECRET_DOOR &&
			_dungeonTiles[y][x] != DTILE_BEAMS && _cells[y][x]._itemId != DITEM_CHEST;
	}

	if (!valid)
		return;

	// Never spawn directly on top of the player
	if (Common::Point(x, y) == _G(savegame)._locationPosition)
		return;

	// Find one of the 5 monster slots that hasn't been used yet this
	// generation
	int slot;
	do {
		slot = getRandomNumber(0, 49) / 10;
	} while (_monsterSlotUsed[slot]);
	_monsterSlotUsed[slot] = true;

	int dungeonLevel = _G(savegame)._dungeonLevel;
	int monsterId = (dungeonLevel - 1) / 2 * DUNGEON_MONSTER_SLOTS + slot;

	_cells[y][x]._monsterId = monsterId;
	_cells[y][x]._monsterHp = getRandomNumber(1, dungeonLevel * dungeonLevel + 1) + 10 + monsterId;
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
