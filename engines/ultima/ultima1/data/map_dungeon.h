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

#ifndef ULTIMA1_DATA_MAP_DUNGEON_H
#define ULTIMA1_DATA_MAP_DUNGEON_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

constexpr int DUNGEON_WIDTH = 11;
constexpr int DUNGEON_HEIGHT = 11;
constexpr int DUNGEON_FLOORS = 10;
// Number of distinct monster "slots" filled in when a level is generated
constexpr int DUNGEON_MONSTER_SLOTS = 5;

// No monster is present in a cell. Real monster Ids are 0-24 (5 tiers of
// 5 types, tier = (dungeonLevel - 1) / 2), so this can't use 0 like the
// _itemId sentinel does
constexpr uint8 DUNGEON_NO_MONSTER = 0xff;

enum DungeonTileId {
	DTILE_HALLWAY = 0,
	DTILE_WALL = 1,
	DTILE_SECRET_DOOR = 2,
	DTILE_DOOR = 3,
	DTILE_CHEST = 4,
	DTILE_COFFIN = 5,
	DTILE_LADDER_DOWN = 6,
	DTILE_LADDER_UP = 7,
	DTILE_BEAMS = 8
};

// Values of DungeonCell::_itemId. Shares numbering with DungeonTileId's
// chest/coffin, since the original stores the same tile Id in both places
enum DungeonItemId {
	DITEM_NONE = 0,
	DITEM_CHEST = DTILE_CHEST,
	DITEM_COFFIN = DTILE_COFFIN
};

struct DungeonCell {
	uint8 _monsterId;
	uint8 _itemId;
	uint16 _monsterHp;
};

struct MapDungeon {
private:
	DungeonCell _cells[DUNGEON_HEIGHT][DUNGEON_WIDTH];
	uint16 _randomSeed = 0;
	// Tracks which of the 5 monster slots have already been filled while
	// generating the current level
	bool _monsterSlotUsed[DUNGEON_MONSTER_SLOTS] = {};

	/**
	 * Clear the dungeon content
	 */
	void clear();

	/**
	 * Sets the random seed used for generating the dungeon level layout
	 */
	void setRandomSeed();

	/**
	 * Gets a random number in the inclusive min/max range, using the same
	 * manually seeded algorithm as the original game, so the dungeon
	 * layouts will match
	 */
	byte getRandomNumber(int minVal, int maxVal);

	/**
	 * Picks a random unoccupied cell and, provided it isn't where the
	 * player is standing, spawns a new monster there appropriate for the
	 * current dungeon level
	 */
	void dungeonSpawnMonster();

protected:
	byte _dungeonTiles[DUNGEON_HEIGHT][DUNGEON_WIDTH];

	/**
	 * Loads a dungeon level
	 */
	void generateDungeonLevel();

public:
	MapDungeon();
};

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
