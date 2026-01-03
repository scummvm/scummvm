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

#include "ultima/ultima0/data/data.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

const ObjectInfo OBJECT_INFO[] = {
	{ "Food", 1, 0, Common::KEYCODE_f },
	{ "Rapier", 8, 10, Common::KEYCODE_r },
	{ "Axe", 5, 5, Common::KEYCODE_a },
	{ "Shield", 6, 1, Common::KEYCODE_s },
	{ "Bow and Arrow", 3, 4, Common::KEYCODE_b },
	{ "Magic Amulet", 15, 0, Common::KEYCODE_m }
};

const MonsterInfo MONSTER_INFO[] = {
	{ nullptr, 0 },
	{ "Skeleton", 1 },
	{ "Thief", 2 },
	{ "Giant Rat", 3 },
	{ "Orc", 4 },
	{ "Viper", 5 },
	{ "Carrion Crawler", 6 },
	{ "Gremlin", 7 },
	{ "Mimic", 8 },
	{ "Daemon", 9 },
	{ "Balrog", 10 }
};

const char *ATTRIB_NAMES[] = { "Hit Points", "Strength", "Dexterity", "Stamina", "Wisdom", "Gold" };
const char *const DIRECTION_NAMES[] = { "North", "East", "South", "West" };

/*-------------------------------------------------------------------*/

void PlayerInfo::init() {
	Common::fill(Name, Name + MAX_NAME + 1, '\0');
	World.x = World.y = 0;
	Dungeon.x = Dungeon.y = 0;
	DungDir.x = DungDir.y = 0;
	Class = '?';
	HPGain = 0;
	LuckyNumber = 0;
	Level = 0;
	Skill = 0;
	Task = 0;
	TaskCompleted = 0;
	Common::fill(Attr, Attr + MAX_ATTR, 0);
	Common::fill(Object, Object + MAX_OBJ, 0);
}

void PlayerInfo::rollAttributes() {
	for (int i = 0; i < MAX_ATTR; ++i)
		Attr[i] = g_engine->getRandomNumber(21) + 4;
}

void PlayerInfo::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)Name, MAX_NAME + 1);
	s.syncAsSint16LE(World.x);
	s.syncAsSint16LE(World.y);
	s.syncAsSint16LE(Dungeon.x);
	s.syncAsSint16LE(Dungeon.y);
	s.syncAsSint16LE(DungDir.x);
	s.syncAsSint16LE(DungDir.y);
	s.syncAsByte(Class);
	s.syncAsSint32LE(HPGain);
	s.syncAsSint32LE(Level);
	s.syncAsSint32LE(Skill);
	s.syncAsSint32LE(Task);
	s.syncAsSint32LE(TaskCompleted);
	s.syncAsUint32LE(LuckyNumber);

	for (int i = 0; i < MAX_ATTR; ++i)
		s.syncAsUint32LE(Attr[i]);
	for (int i = 0; i < MAX_OBJ; ++i) {
		uint32 val = (uint32)Object[i];
		s.syncAsUint32LE(val);
		if (s.isLoading())
			Object[i] = (double)val;
	}
}

Direction PlayerInfo::dungeonDir() const {
	if (DungDir.y < 0)
		return DIR_NORTH;
	else if (DungDir.x > 0)
		return DIR_EAST;
	else if (DungDir.y > 0)
		return DIR_SOUTH;
	else
		return DIR_WEST;
}

void PlayerInfo::setDungeonDir(Direction newDir) {
	DungDir.x = 0;
	DungDir.y = 0;
	switch (newDir) {
	case DIR_NORTH:
		DungDir.y = -1;
		break;
	case DIR_EAST:
		DungDir.x = 1;
		break;
	case DIR_SOUTH:
		DungDir.y = 1;
		break;
	case DIR_WEST:
		DungDir.x = -1;
		break;
	}
}

void PlayerInfo::dungeonTurnLeft() {
	Direction dir = dungeonDir();
	setDungeonDir((dir == DIR_NORTH) ? DIR_WEST : (Direction)((int)dir - 1));
}

void PlayerInfo::dungeonTurnRight() {
	Direction dir = dungeonDir();
	setDungeonDir((dir == DIR_WEST) ? DIR_NORTH : (Direction)((int)dir + 1));
}

/*-------------------------------------------------------------------*/

void WorldMapInfo::init(PlayerInfo &p) {
	int c, x, y, size;

	g_engine->setRandomSeed(p.LuckyNumber);
	size = WORLD_MAP_SIZE - 1;

	// Set the boundaries
	for (x = 0; x <= size; x++) {
		Map[size][x] = WT_MOUNTAIN;
		Map[0][x] = WT_MOUNTAIN;
		Map[x][size] = WT_MOUNTAIN;
		Map[x][0] = WT_MOUNTAIN;
	}

	// Set up the map contents
	for (x = 1; x < size; x++) {
		for (y = 1; y < size; y++) {
			c = (int)(pow(RND(), 5.0) * 4.5);	// Calculate what's there
			if (c == WT_TOWN && RND() > .5)		// Remove half the towns
				c = WT_SPACE;
			Map[x][y] = c;
		}
	}

	// Calculate player start
	x = g_engine->getRandomNumber(1, size - 1);
	y = g_engine->getRandomNumber(1, size - 1);
	p.World.x = x; p.World.y = y;			// Save it
	Map[x][y] = WT_TOWN;					// Make it a town

	// Find place for castle
	do {
		x = g_engine->getRandomNumber(1, size - 1);
		y = g_engine->getRandomNumber(1, size - 1);
	} while (Map[x][y] != WT_SPACE);

	Map[x][y] = WT_BRITISH;				// Put LBs castle there
}

int WorldMapInfo::read(int x, int y) const {
	if (x < 0 || y < 0)
		return WT_MOUNTAIN;
	if (x >= WORLD_MAP_SIZE || y >= WORLD_MAP_SIZE)
		return WT_MOUNTAIN;
	return Map[x][y];
}

void WorldMapInfo::synchronize(Common::Serializer &s) {
	for (int y = 0; y < WORLD_MAP_SIZE; ++y)
		for (int x = 0; x < WORLD_MAP_SIZE; ++x)
			s.syncAsByte(Map[x][y]);
}

/*-------------------------------------------------------------------*/

void DungeonMapInfo::create(const PlayerInfo &player) {
	int i, x, y;
	const int SIZE = DUNGEON_MAP_SIZE - 1;

	// Seed the random number
	g_engine->setRandomSeed(player.LuckyNumber - player.World.x * 40 -
		player.World.y * 1000 - player.Level);

	// Clear the dungeon
	Common::fill((byte *)Map, (byte *)Map + DUNGEON_MAP_SIZE * DUNGEON_MAP_SIZE, DT_SPACE);

	// Draw the boundaries
	for (x = 0; x <= SIZE; x++) {
		Map[SIZE][x] = DT_SOLID;
		Map[0][x] = DT_SOLID;
		Map[x][SIZE] = DT_SOLID;
		Map[x][0] = DT_SOLID;
	}

	// Fill with checkerboard
	for (x = 2; x < SIZE; x = x + 2) {
		for (y = 1; y < SIZE; y++) {
			Map[x][y] = DT_SOLID;
			Map[y][x] = DT_SOLID;
		}
	}

	// Fill with stuff
	for (x = 2; x < SIZE; x = x + 2) {
		for (y = 1; y < SIZE; y = y + 2) {
			Map[x][y] = generateContent(Map[x][y]);
			Map[y][x] = generateContent(Map[y][x]);
		}
	}

	// Put stairs in
	Map[2][1] = DT_SPACE;

	// Different ends each level
	if (player.Level % 2 == 0) {
		Map[SIZE - 3][3] = DT_LADDERDN;
		Map[3][SIZE - 3] = DT_LADDERUP;
	} else {
		Map[SIZE - 3][3] = DT_LADDERUP;
		Map[3][SIZE - 3] = DT_LADDERDN;
	}

	// On first floor
	if (player.Level == 1) {
		Map[1][1] = DT_LADDERUP;		// Ladder at top left
		Map[SIZE - 3][3] = DT_SPACE;	// No other ladder up
	}

	// Add monsters
	_monsters.clear();
	for (i = 1; i <= MAX_MONSTERS; ++i)
		addMonster(player, i);
}

int DungeonMapInfo::generateContent(int c) {
	if (RND() > .95) 	c = DT_TRAP;
	if (RND() > .6) 	c = DT_HIDDENDOOR;
	if (RND() > .6) 	c = DT_DOOR;
	if (RND() > .97) 	c = DT_PIT;
	if (RND() > .94) 	c = DT_GOLD;
	return c;
}

void DungeonMapInfo::addMonster(const PlayerInfo &player, int Type) {
	int x, y;
	int level = MONSTER_INFO[Type].Level;

	// Limit monsters to levels
	if (level - 2 > player.Level)
		return;
	// Not always there anyway
	if (RND() > 0.4)
		return;

	// Fill in details */
	MonsterEntry m;
	m._type = Type;
	m._strength = level + 3 + player.Level;
	m._alive = true;

	// Find a place for it. Must be empty, not player
	do {
		x = urand() % DUNGEON_MAP_SIZE;
		y = urand() % DUNGEON_MAP_SIZE;
	} while (Map[x][y] != DT_SPACE ||
		(x == player.Dungeon.x && y == player.Dungeon.y));

	// Record position
	m._loc.x = x; m._loc.y = y;

	_monsters.push_back(m);
}

void DungeonMapInfo::synchronize(Common::Serializer &s) {
	for (int y = 0; y < DUNGEON_MAP_SIZE; ++y)
		for (int x = 0; x < DUNGEON_MAP_SIZE; ++x)
			s.syncAsByte(Map[x][y]);
}

int DungeonMapInfo::findMonster(const Common::Point &c) const {
	int n = -1;

	for (uint i = 0; i < _monsters.size(); i++) {
		const auto &m = _monsters[i];
		if (m._loc == c && m._alive)
			n = i;
	}
	return n;
}

} // namespace Ultima0
} // namespace Ultima
