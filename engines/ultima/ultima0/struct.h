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

#ifndef ULTIMA_ULTIMA0_STRUCT_H
#define ULTIMA_ULTIMA0_STRUCT_H

#include "ultima/ultima0/defines.h"

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/************************************************************************/
/*																		*/
/*							Data Structures								*/
/*																		*/
/************************************************************************/
/************************************************************************/

typedef struct _Coord						/* Coordinate structure */
{
	int x, y;
} COORD;

/************************************************************************/

typedef struct _Rect						/* Rectangle structure */
{
	int left, top, right, bottom;				/* Rectangle coords */
} RECT;

/************************************************************************/

typedef struct _WorldMap					/* World Map structure */
{
	int MapSize;							/* Size of map */
	unsigned char Map[WORLD_MAP_SIZE]		/* Map information */
		[WORLD_MAP_SIZE];
} WORLDMAP;

/************************************************************************/

typedef struct _Monster						/* Monster structure */
{
	COORD Loc;								/* Position */
	int	  Type;								/* Monster type */
	int	  Strength;							/* Strength */
	int	  Alive;							/* Alive flag */
} MONSTER;

/************************************************************************/

typedef struct _DungeonMap					/* Dungeon Map Structure */
{
	int MapSize;							/* Size of Map */
	unsigned char Map[DUNGEON_MAP_SIZE]		/* Map information */
		[DUNGEON_MAP_SIZE];
	int	MonstCount;							/* Number of Monsters */
	MONSTER Monster[MAX_MONSTERS];			/* Monster records */
} DUNGEONMAP;

/************************************************************************/

typedef struct _Player						/* Player structure */
{
	char  Name[MAX_NAME + 1];					/* Player Name */
	COORD World;							/* World map position */
	COORD Dungeon;							/* Dungeon map position */
	COORD DungDir;							/* Dungeon direction facing */
	char  Class;							/* Player class (F or M) */
	int   HPGain;							/* HPs gained in dungeon */
	int	  Level;							/* Dungeon level, 0 = world map */
	int	  Skill;							/* Skill level */
	int	  Task;								/* Task set (-1 = none) */
	int	  TaskCompleted;					/* Task completed */
	int	  LuckyNumber;						/* Value used for seeding */
	int	  Attributes;						/* Number of attributes */
	int	  Objects;							/* Number of objects */
	int   Attr[MAX_ATTR];					/* Attribute values */
	double Object[MAX_OBJ];					/* Object counts */
} PLAYER;

} // namespace Ultima0
} // namespace Ultima

#endif
