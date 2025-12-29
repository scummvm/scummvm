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

#include "common/rect.h"
#include "ultima/ultima0/data/defines.h"

namespace Ultima {
namespace Ultima0 {


struct _OInfStruct {
	const char *Name;
	int Cost;
	int MaxDamage;
	char Key;
};
struct _MInfStruct {
	const char *Name; int Level;
};

extern const _OInfStruct OBJECT_INFO[];
extern const _MInfStruct MONSTER_INFO[];
extern const char *ATTRIB_NAMES[];


// point/rect types
typedef Common::Point COORD;
typedef Common::Rect RECT;

/**
 * Monster structure
 */
struct MONSTER {
	COORD Loc;				// Position
	int	  Type = 0;			// Monster type
	int	  Strength = 0;		// Strength
	int	  Alive = 0;		// Alive flag
};

/**
 * Dungeon Map Structure
 */
struct DUNGEONMAP {
	int MapSize = 0;								// Size of Map
	byte Map[DUNGEON_MAP_SIZE][DUNGEON_MAP_SIZE] = {};	// Map information
	int	MonstCount = 0;							// Number of Monsters
	MONSTER Monster[MAX_MONSTERS];			// Monster records
};

/**
 * Player structure
 */
struct PLAYER {
	char  Name[MAX_NAME + 1] = {};			// Player Name
	COORD World;							// World map position
	COORD Dungeon;							// Dungeon map position
	COORD DungDir;							// Dungeon direction facing
	byte  Class = '?';						// Player class (F or M)
	int   HPGain = 0;						// HPs gained in dungeon
	int	  Level = 0;						// Dungeon level, 0 = world map
	int	  Skill = 0;						// Skill level
	int	  Task = 0;							// Task set (-1 = none)
	int	  TaskCompleted = 0;				// Task completed
	uint32 LuckyNumber = 0;					// Value used for seeding
	const int Attributes = MAX_ATTR;		// Number of attributes
	const int Objects = MAX_OBJ;			// Number of objects
	int   Attr[MAX_ATTR] = {};				// Attribute values
	double Object[MAX_OBJ] = {};			// Object counts

	void init();
	void rollAttributes();
};

/**
 * World Map structure
 */
struct WORLDMAP {
	const int MapSize = WORLD_MAP_SIZE - 1;			// Size of map
	byte Map[WORLD_MAP_SIZE][WORLD_MAP_SIZE] = {};	// Map information

	void init(PLAYER &p);

	int read(int x, int y) const;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
