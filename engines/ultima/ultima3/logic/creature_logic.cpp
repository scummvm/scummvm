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

#include "ultima/ultima3/logic/creature_logic.h"
#include "ultima/ultima3/logic/location_logic.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {

constexpr int MAP_MASK = Data::MAP_SIZE - 1;

// How a creature goes about things, from the top two bits of its flags
enum Behaviour {
	STAY_PUT = 0,
	WANDER = 1,
	APPROACH = 2,
	ATTACK = 3
};

// The monsters that appear in the world, and what each has to stand on
constexpr int MONSTER_TYPES = 13;
static const byte MONSTER_TILES[MONSTER_TYPES] = {
	0x60, 0x5C, 0x64, 0x50, 0x68, 0x6C, 0x34, 0x70, 0x58, 0x38, 0x3C, 0x74, 0x78
};
static const byte MONSTER_FLOORS[MONSTER_TYPES] = {
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00, 0x00, 0x04, 0x04
};

constexpr byte CELL_WATER = 0;
constexpr byte CELL_GRASS = 4;
constexpr byte CELL_BRUSH = 8;
constexpr byte CELL_FOREST = 0x0C;
constexpr byte CELL_FLOOR = 0x20;
constexpr byte FIRST_SEA_CREATURE = 0x2C;
constexpr byte FIRST_NON_CREATURE = 0x40;

static int randomBelow(int limit) {
	return Graphics::Views::g_events->getRandomNumber(limit - 1);
}

int CreatureLogic::stepToward(int from, int to) const {
	int delta = (byte)(to - from);

	// On the world map, the way round the edge can be shorter
	if (_G(savegame)._location == Data::LOCATION_SOSARIA)
		delta = (byte)(delta << 2);

	int8 signedDelta = (int8)delta;
	return (signedDelta > 0) - (signedDelta < 0);
}

void CreatureLogic::stepTowardParty(int index, int &x, int &y) const {
	const Data::Savegame &save = _G(savegame);
	const Data::Creatures &c = _G(map)._creatures;

	x = (c._x[index] + stepToward(c._x[index], save._posX)) & MAP_MASK;
	y = (c._y[index] + stepToward(c._y[index], save._posY)) & MAP_MASK;
}

int CreatureLogic::creatureAt(int x, int y) const {
	const Data::Creatures &c = _G(map)._creatures;

	for (int i = Data::CREATURE_COUNT - 1; i >= 0; --i) {
		if (c._tile[i] && c._x[i] == x && c._y[i] == y)
			return i;
	}

	return -1;
}

bool CreatureLogic::canMoveTo(int index, int x, int y) const {
	const Data::Creatures &c = _G(map)._creatures;
	byte cell = _G(map).cell(x, y);
	byte tile = c._tile[index];
	bool terrainOk;

	if (tile == CELL_GRASS) {
		terrainOk = cell == tile;
	} else if (tile < FIRST_NON_CREATURE && tile >= FIRST_SEA_CREATURE) {
		terrainOk = cell == CELL_WATER;
	} else {
		terrainOk = cell == CELL_GRASS || cell == CELL_BRUSH || cell == CELL_FOREST || cell == CELL_FLOOR;
	}

	return terrainOk && creatureAt(x, y) < 0;
}

void CreatureLogic::moveTo(int index, int x, int y) {
	Data::Map &map = _G(map);
	Data::Creatures &c = map._creatures;

	map.setCell(c._x[index], c._y[index], c._floor[index]);
	c._x[index] = x;
	c._y[index] = y;
	c._floor[index] = map.cell(x, y);
	map.setCell(x, y, c._tile[index]);
}

int CreatureLogic::update(bool moved) {
	const Data::Savegame &save = _G(savegame);
	const Data::Creatures &c = _G(map)._creatures;
	const bool world = save._location == Data::LOCATION_SOSARIA;

	// When riding or sailing, a step of the party only lets creatures move half the time
	if (save._transport != TRANSPORT_ON_FOOT) {
		if (!moved)
			_stepToggle = -1;

		_stepToggle = -_stepToggle;
		if (_stepToggle < 0)
			return -1;
	}

	// Time held still stops everyone
	if (_negateTimeTurns > 0) {
		--_negateTimeTurns;
		return -1;
	}

	for (int i = Data::CREATURE_COUNT - 1; i >= 0; --i) {
		if (!c._tile[i])
			continue;

		int x, y;
		int behaviour = world ? ATTACK : (c._flags[i] >> 6);

		if (behaviour == STAY_PUT)
			continue;

		if (behaviour == WANDER) {
			// Half the time, a step in a random direction on each axis
			if (randomBelow(255) < 128)
				continue;

			x = (c._x[i] + stepToward(0, randomBelow(255))) & MAP_MASK;
			if (x == 0)
				continue;

			y = (c._y[i] + stepToward(0, randomBelow(255))) & MAP_MASK;
			if (y == 0)
				continue;
		} else {
			stepTowardParty(i, x, y);

			if (behaviour == ATTACK && x == save._posX && y == save._posY)
				return i;
		}

		// If it can't go straight there, it tries one axis at a time
		if (!canMoveTo(i, x, y)) {
			if (canMoveTo(i, c._x[i], y)) {
				x = c._x[i];
			} else if (canMoveTo(i, x, c._y[i])) {
				y = c._y[i];
			} else {
				continue;
			}
		}

		if (x != save._posX || y != save._posY)
			moveTo(i, x, y);
	}

	spawnMonster();
	return -1;
}

void CreatureLogic::spawnMonster() {
	Data::Map &map = _G(map);
	Data::Creatures &c = map._creatures;
	const Data::Savegame &save = _G(savegame);

	if (save._location != Data::LOCATION_SOSARIA || randomBelow(0x87) < 0x80)
		return;

	int slot = 0;
	while (slot < Data::CREATURE_COUNT && c._tile[slot])
		++slot;
	if (slot == Data::CREATURE_COUNT)
		return;

	// Lower numbered kinds are more likely
	int type = randomBelow(MONSTER_TYPES) & randomBelow(MONSTER_TYPES);

	int x = randomBelow(Data::MAP_SIZE);
	int y = randomBelow(Data::MAP_SIZE);
	if (x == save._posX || y == save._posY || map.cell(x, y) != MONSTER_FLOORS[type])
		return;

	c._tile[slot] = MONSTER_TILES[type];
	c._floor[slot] = MONSTER_FLOORS[type];
	c._x[slot] = x;
	c._y[slot] = y;
	c._flags[slot] = ATTACK << 6;
	map.setCell(x, y, MONSTER_TILES[type]);
}

} // namespace Ultima3
} // namespace Ultima
