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

#ifndef ULTIMA0_DATA_H
#define ULTIMA0_DATA_H

#include "common/array.h"
#include "common/keyboard.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "ultima/ultima0/data/defines.h"

namespace Ultima {
namespace Ultima0 {

enum Direction { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST };

struct PlayerInfo;

struct ObjectInfo {
	const char *_name;
	int _cost;
	int _maxDamage;
	int _action;
};
struct MonsterInfo {
	const char *_name;
	int _level;
};

extern const ObjectInfo OBJECT_INFO[];
extern const MonsterInfo MONSTER_INFO[];
extern const char *ATTRIB_NAMES[];
extern const char *const DIRECTION_NAMES[];

/**
 * Monster structure
 */
struct MonsterEntry {
	Common::Point _loc;		// Position
	int _type = 0;			// Monster type
	int _strength = 0;		// Strength
	bool _alive = false;	// Alive flag

	void synchronize(Common::Serializer &s);
};

/**
 * Dungeon Map Structure
 */
struct DungeonMapInfo {
private:
	void addMonster(const PlayerInfo &player, int type);
	int generateContent(int c);

public:
	byte _map[DUNGEON_MAP_SIZE][DUNGEON_MAP_SIZE] = {};	// Map information
	Common::Array<MonsterEntry> _monsters;				// Monster records

	void create(const PlayerInfo &player);
	void synchronize(Common::Serializer &s);

	/**
	 * Find Monster ID at given location
	 */
	int findMonster(const Common::Point &c) const;

	void addMonsterAtPos(const PlayerInfo &player, const Common::Point &pt, int type);
};

/**
 * Player structure
 */
struct PlayerInfo {
	char _name[MAX_NAME + 1] = {};			// Player Name
	Common::Point _worldPos;				// World map position
	Common::Point _dungeonPos;				// Dungeon map position
	Common::Point _dungeonDir;				// Dungeon direction facing
	byte _class = '?';						// Player class (F or M)
	int _hpGain = 0;						// HPs gained in dungeon
	int _level = 0;						// Dungeon level, 0 = world map
	int _skill = 0;						// Skill level
	int _task = 0;							// Task set (-1 = none)
	bool _taskCompleted = 0;				// Task completed
	uint32 _luckyNumber = 0;					// Value used for seeding
	int _attr[MAX_ATTR] = {};				// Attribute values
	double _object[MAX_OBJ] = {};			// Object counts

	void init();
	void rollAttributes();
	void synchronize(Common::Serializer &s);

	/**
	 * Return the dungeon facing direction
	 */
	Direction dungeonDir() const;

	/**
	 * Sets the dungeon direction
	 */
	void setDungeonDir(Direction newDir);

	/**
	 * Turn left in the dungeon
	 */
	void dungeonTurnLeft();

	/**
	 * Turn right in the dungeon
	 */
	void dungeonTurnRight();
};

/**
 * World Map structure
 */
struct WorldMapInfo {
	byte _map[WORLD_MAP_SIZE][WORLD_MAP_SIZE] = {};	// Map information

	void init(PlayerInfo &p);
	int read(int x, int y) const;
	void synchronize(Common::Serializer &s);
};

} // namespace Ultima0
} // namespace Ultima

#endif
