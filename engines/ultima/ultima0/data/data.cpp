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
	{ "Food", 1, 0, KEYBIND_FOOD },
	{ "Rapier", 8, 10, KEYBIND_RAPIER },
	{ "Axe", 5, 5, KEYBIND_AXE },
	{ "Shield", 6, 1, KEYBIND_SHIELD },
	{ "Bow and Arrow", 3, 4, KEYBIND_BOW },
	{ "Magic Amulet", 15, 0, KEYBIND_AMULET }
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
	Common::fill(_name, _name + MAX_NAME + 1, '\0');
	_worldPos.x = _worldPos.y = 0;
	_dungeonPos.x = _dungeonPos.y = 0;
	_dungeonDir.x = _dungeonDir.y = 0;
	_class = '?';
	_hpGain = 0;
	_luckyNumber = 0;
	_level = 0;
	_skill = 0;
	_task = 0;
	_taskCompleted = false;
	Common::fill(_attr, _attr + MAX_ATTR, 0);
	Common::fill(_object, _object + MAX_OBJ, 0);
}

void PlayerInfo::rollAttributes() {
	for (int i = 0; i < MAX_ATTR; ++i)
		_attr[i] = g_engine->getRandomNumber(21) + 4;
}

void PlayerInfo::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_name, MAX_NAME + 1);
	s.syncAsSint16LE(_worldPos.x);
	s.syncAsSint16LE(_worldPos.y);
	s.syncAsSint16LE(_dungeonPos.x);
	s.syncAsSint16LE(_dungeonPos.y);
	s.syncAsSint16LE(_dungeonDir.x);
	s.syncAsSint16LE(_dungeonDir.y);
	s.syncAsByte(_class);
	s.syncAsSint32LE(_hpGain);
	s.syncAsSint32LE(_level);
	s.syncAsSint32LE(_skill);
	s.syncAsSint32LE(_task);
	s.syncAsSint32LE(_taskCompleted);
	s.syncAsUint32LE(_luckyNumber);

	for (int i = 0; i < MAX_ATTR; ++i)
		s.syncAsUint32LE(_attr[i]);
	for (int i = 0; i < MAX_OBJ; ++i) {
		uint32 val = (uint32)_object[i];
		s.syncAsUint32LE(val);
		if (s.isLoading())
			_object[i] = (double)val;
	}
}

Direction PlayerInfo::dungeonDir() const {
	if (_dungeonDir.y < 0)
		return DIR_NORTH;
	else if (_dungeonDir.x > 0)
		return DIR_EAST;
	else if (_dungeonDir.y > 0)
		return DIR_SOUTH;
	else
		return DIR_WEST;
}

void PlayerInfo::setDungeonDir(Direction newDir) {
	_dungeonDir.x = 0;
	_dungeonDir.y = 0;
	switch (newDir) {
	case DIR_NORTH:
		_dungeonDir.y = -1;
		break;
	case DIR_EAST:
		_dungeonDir.x = 1;
		break;
	case DIR_SOUTH:
		_dungeonDir.y = 1;
		break;
	case DIR_WEST:
		_dungeonDir.x = -1;
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

	g_engine->setRandomSeed(p._luckyNumber);
	size = WORLD_MAP_SIZE - 1;

	// Set the boundaries
	for (x = 0; x <= size; x++) {
		_map[size][x] = WT_MOUNTAIN;
		_map[0][x] = WT_MOUNTAIN;
		_map[x][size] = WT_MOUNTAIN;
		_map[x][0] = WT_MOUNTAIN;
	}

	// Set up the map contents
	for (x = 1; x < size; x++) {
		for (y = 1; y < size; y++) {
			c = (int)(pow(RND(), 5.0) * 4.5);	// Calculate what's there
			if (c == WT_TOWN && RND() > .5)		// Remove half the towns
				c = WT_SPACE;
			_map[x][y] = c;
		}
	}

	// Calculate player start
	x = g_engine->getRandomNumber(1, size - 1);
	y = g_engine->getRandomNumber(1, size - 1);
	p._worldPos.x = x; p._worldPos.y = y;		// Save it
	_map[x][y] = WT_TOWN;						// Make it a town

	// Find place for castle
	do {
		x = g_engine->getRandomNumber(1, size - 1);
		y = g_engine->getRandomNumber(1, size - 1);
	} while (_map[x][y] != WT_SPACE);

	_map[x][y] = WT_BRITISH;					// Put LBs castle there
}

int WorldMapInfo::read(int x, int y) const {
	if (x < 0 || y < 0)
		return WT_MOUNTAIN;
	if (x >= WORLD_MAP_SIZE || y >= WORLD_MAP_SIZE)
		return WT_MOUNTAIN;
	return _map[x][y];
}

void WorldMapInfo::synchronize(Common::Serializer &s) {
	for (int y = 0; y < WORLD_MAP_SIZE; ++y)
		for (int x = 0; x < WORLD_MAP_SIZE; ++x)
			s.syncAsByte(_map[x][y]);
}

/*-------------------------------------------------------------------*/

void DungeonMapInfo::create(const PlayerInfo &player) {
	int i, x, y;
	const int SIZE = DUNGEON_MAP_SIZE - 1;

	// Seed the random number
	g_engine->setRandomSeed(player._luckyNumber - player._worldPos.x * 40 -
		player._worldPos.y * 1000 - player._level);

	// Clear the dungeon
	Common::fill((byte *)_map, (byte *)_map + DUNGEON_MAP_SIZE * DUNGEON_MAP_SIZE, DT_SPACE);

	// Draw the boundaries
	for (x = 0; x <= SIZE; x++) {
		_map[SIZE][x] = DT_SOLID;
		_map[0][x] = DT_SOLID;
		_map[x][SIZE] = DT_SOLID;
		_map[x][0] = DT_SOLID;
	}

	// Fill with checkerboard
	for (x = 2; x < SIZE; x = x + 2) {
		for (y = 1; y < SIZE; y++) {
			_map[x][y] = DT_SOLID;
			_map[y][x] = DT_SOLID;
		}
	}

	// Fill with stuff
	for (x = 2; x < SIZE; x = x + 2) {
		for (y = 1; y < SIZE; y = y + 2) {
			_map[x][y] = generateContent(_map[x][y]);
			_map[y][x] = generateContent(_map[y][x]);
		}
	}

	// Put stairs in
	_map[2][1] = DT_SPACE;

	// Different ends each level
	if (player._level % 2 == 0) {
		_map[SIZE - 3][3] = DT_LADDERDN;
		_map[3][SIZE - 3] = DT_LADDERUP;
	} else {
		_map[SIZE - 3][3] = DT_LADDERUP;
		_map[3][SIZE - 3] = DT_LADDERDN;
	}

	// On first floor
	if (player._level == 1) {
		_map[1][1] = DT_LADDERUP;		// Ladder at top left
		_map[SIZE - 3][3] = DT_SPACE;	// No other ladder up
	}

	// WORKAROUND: Make sure dungeon is completable
	byte mapTrace[DUNGEON_MAP_SIZE][DUNGEON_MAP_SIZE];
	bool isValid;
	do {
		isValid = false;

		// Start a new trace, mark solid spaces
		for (y = 0; y < DUNGEON_MAP_SIZE; ++y)
			for (x = 0; x < DUNGEON_MAP_SIZE; ++x)
				mapTrace[x][y] = _map[x][y] == DT_SOLID ? DT_SOLID : DT_SPACE;

		// Iterate through figuring out route
		Common::Queue<Common::Point> points;
		if (player._level % 2 == 0)
			points.push(Common::Point(SIZE - 3, 3));
		else
			points.push(Common::Point(3, SIZE - 3));
		while (!points.empty() && !isValid) {
			Common::Point pt = points.pop();
			isValid = _map[pt.x][pt.y] == DT_LADDERUP;
			if (isValid)
				break;

			mapTrace[pt.x][pt.y] = DT_LADDERDN;
			if (pt.x > 1 && _map[pt.x - 1][pt.y] != DT_SOLID && mapTrace[pt.x - 1][pt.y] == DT_SPACE)
				points.push(Common::Point(pt.x - 1, pt.y));
			if (pt.x < SIZE && _map[pt.x + 1][pt.y] != DT_SOLID && mapTrace[pt.x + 1][pt.y] == DT_SPACE)
				points.push(Common::Point(pt.x + 1, pt.y));
			if (pt.y > 1 && _map[pt.x][pt.y - 1] != DT_SOLID && mapTrace[pt.x][pt.y - 1] == DT_SPACE)
				points.push(Common::Point(pt.x, pt.y - 1));
			if (pt.y < SIZE && _map[pt.x][pt.y + 1] != DT_SOLID && mapTrace[pt.x][pt.y + 1] == DT_SPACE)
				points.push(Common::Point(pt.x, pt.y + 1));
		}

		if (!isValid) {
			// If a path wasn't found, randomly replace a solid square. We'll then
			// loop to check whether the path can now be completed
			do {
				x = g_engine->getRandomNumber(1, SIZE);
				y = g_engine->getRandomNumber(1, SIZE);
			} while (_map[x][y] != DT_SOLID);

			_map[x][y] = DT_HIDDENDOOR;
		}
	} while (!isValid);

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

void DungeonMapInfo::addMonster(const PlayerInfo &player, int type) {
	int x, y;
	int level = MONSTER_INFO[type]._level;

	// Limit monsters to levels
	if (level - 2 > player._level)
		return;

	// Not always there anyway
	if (RND() > 0.6)
		return;

	// Find a place for it. Must be empty, not player
	do {
		x = urand() % DUNGEON_MAP_SIZE;
		y = urand() % DUNGEON_MAP_SIZE;
	} while (_map[x][y] != DT_SPACE ||
		(x == player._dungeonPos.x && y == player._dungeonPos.y));

	addMonsterAtPos(player, Common::Point(x, y), type);
}

void DungeonMapInfo::addMonsterAtPos(const PlayerInfo &player, const Common::Point &pt, int type) {
	int level = MONSTER_INFO[type]._level;

	// Fill in details
	MonsterEntry m;
	m._type = type;
	m._strength = level + 3 + player._level;
	m._alive = true;

	// Record position
	m._loc = pt;

	_monsters.push_back(m);
}

void DungeonMapInfo::synchronize(Common::Serializer &s) {
	// Map data
	for (int y = 0; y < DUNGEON_MAP_SIZE; ++y)
		for (int x = 0; x < DUNGEON_MAP_SIZE; ++x)
			s.syncAsByte(_map[x][y]);

	// Monsters
	uint count = _monsters.size();
	s.syncAsByte(count);
	if (s.isLoading())
		_monsters.resize(count);

	for (auto &m : _monsters)
		m.synchronize(s);
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

/*-------------------------------------------------------------------*/

void MonsterEntry::synchronize(Common::Serializer &s) {
	s.syncAsByte(_loc.x);
	s.syncAsByte(_loc.y);
	s.syncAsByte(_type);
	s.syncAsByte(_strength);
	s.syncAsByte(_alive);
}

} // namespace Ultima0
} // namespace Ultima
