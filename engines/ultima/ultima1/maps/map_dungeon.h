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

#ifndef ULTIMA_ULTIMA1_MAP_MAP_DUNGEON_H
#define ULTIMA_ULTIMA1_MAP_MAP_DUNGEON_H

#include "ultima/ultima1/maps/map_base.h"
#include "ultima/shared/core/rect.h"
#include "common/random.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {
	class DungeonMonster;
}

namespace Maps {

#define DUNGEON_WIDTH 11
#define DUNGEON_HEIGHT 11

enum DungeonTile {
	DTILE_HALLWAY = 0, DTILE_WALL = 1, DTILE_SECRET_DOOR = 2, DTILE_DOOR = 3, DTILE_LADDER_DOWN = 6,
	DTILE_LADDER_UP = 7, DTILE_BEAMS = 8
};

/**
 * Implements the dungeon map
 */
class MapDungeon : public MapBase {
private:
	Common::RandomSource _random;
	uint _dungeonLevel;					// Dungeon level number
private:
	/**
	 * Sets up a deterministic random seed for generating dungeon data
	 */
	void setRandomSeed();

	/**
	 * Gets a deterministic random number based on a given seed. Used in dungeon generation
	 * so that a given dungeon and level will always be built the same
	 */
	uint getDeterministicRandomNumber(uint min, uint max) { return min + _random.getRandomNumber(max - min); }

	/**
	 * Called when the dungeon is being left
	 */
	void leavingDungeon();
public:
	uint _dungeonExitHitPoints;
public:
	MapDungeon(Ultima1Game *game, Ultima1Map *map) : MapBase(game, map), _random("UltimaDungeons"),
		_dungeonLevel(0), _dungeonExitHitPoints(0) {}
	~MapDungeon() override {}

	/**
	 * Handles loading and saving viewport
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Load the map
	 */
	void load(Shared::Maps::MapId mapId) override;

	/**
	 * Gets a tile at a given position
	 */
	void getTileAt(const Point &pt, Shared::Maps::MapTile *tile, bool includePlayer = true) override;

	/**
	 * Changes the dungeon level by a given delta amount, and generates a new map
	 * @param delta		Delta to change dungeon level by
	 * @returns			False if dungeon left, true if still within dungeon
	 */
	bool changeLevel(int delta) override;

	/**
	 * Get the current map level
	 */
	uint getLevel() const override { return _dungeonLevel; }

	/**
	 * Updates the map at the end of a turn
	 */
	void update() override;

	/**
	 * Spawns a monster within dungeons
	 */
	void spawnMonster();

	/**
	 * Spawns a monster at a given position in the dungeon map
	 */
	void spawnMonsterAt(const Point &pt);

	/**
	 * Find a monster in the current direction being faced
	 */
	Widgets::DungeonMonster *findCreatureInCurrentDirection(uint maxDistance = 5);

	/**
	 * Perform an attack in a direction
	 * @param direction		Direction
	 * @param effectId		Sound effect to play
	 */
	void attack(int direction, int effectId) override;

	/**
	 * Perform an attack in a direction
	 * @param direction		Direction
	 * @param effectId		Sound effect to play
	 * @param maxDistance	Maximum distance in the given direction
	 * @param amount		Damage amount
	 * @param agility		Agility threshold
	 * @param widgetNa
	 */
	void attack(int direction, int effectId, uint maxDistance, uint amount, uint agility, const Common::String &hitWidget) override;

	/**
	 * Do an inform action
	 */
	void inform() override;

	/**
	 * Do a climb action
	 */
	void climb() override;

	/**
	 * Do an open action
	 */
	void open() override;

	/**
	 * Do an unlock action
	 */
	void unlock() override { open(); }

	/**
	 * Cast a specific spell
	 */
	void castSpell(uint spell) override;
};

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
