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

#ifndef ULTIMA0_DATA_DEFINES_H
#define ULTIMA0_DATA_DEFINES_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima0 {

constexpr int DEFAULT_SCX = 640;			// Default Screen Size and Depth
constexpr int DEFAULT_SCY = 400;

constexpr int WORLD_MAP_SIZE = 21;			// Size of world map
constexpr int DUNGEON_MAP_SIZE = 11;		// Size of dungeon map
constexpr int MAX_MONSTERS = 10;			// Number of Monsters
constexpr int MAX_ATTR = 6;					// Attributes
constexpr int MAX_OBJ = 6;					// Objects
constexpr int WORLD_GRID_SIZE = 3;			// Visible part of map is axa
constexpr int MAX_NAME = 8;					// Max size player name
constexpr int MAX_VIEW_DEPTH = 9;			// Max viewing depth

constexpr int MINIMAP_TILE_SIZE = 5;
constexpr int WORLD_MINIMAP_SIZE = WORLD_MAP_SIZE * MINIMAP_TILE_SIZE;
constexpr int DUNGEON_MINIMAP_SIZE = DUNGEON_MAP_SIZE * MINIMAP_TILE_SIZE;

#define RND_MAX			    0x7fffffff
#define RND()			    (((double)g_engine->getRandomNumber())/RND_MAX)
#define urand()				g_engine->getRandomNumber()
#define	 AKVERSION			(1.000)			// Version number

											// Convert RGB to Colour Code
#define RGB(r,g,b)  ((r?4:0)+(g?2:0)+(b?1:0))

#define C_BLACK		RGB(0,0,0)				// Some Colours
#define C_RED		RGB(1,0,0)
#define C_GREEN		RGB(0,1,0)
#define C_BLUE		RGB(0,0,1)
#define C_YELLOW	RGB(1,1,0)
#define C_WHITE 	RGB(1,1,1)
#define C_CYAN		RGB(0,1,1)
#define C_PURPLE    RGB(1,0,1)
#define C_ROSE      8
#define C_VIOLET    9
#define C_GREY      10

#define C_TEXT_DEFAULT C_CYAN

#define	WT_SPACE		(0)					// World Tiles
#define WT_MOUNTAIN		(1)
#define WT_TREE			(2)
#define	WT_TOWN			(3)
#define	WT_DUNGEON		(4)
#define	WT_BRITISH		(5)
#define	WT_PLAYER		(-1)				// Used for the player graphic

#define	DT_SPACE		(0)					// Dungeon tiles
#define DT_SOLID		(1)
#define DT_TRAP			(2)
#define DT_HIDDENDOOR	(3)
#define DT_DOOR			(4)
#define DT_GOLD			(5)
#define DT_LADDERDN		(7)
#define DT_LADDERUP		(8)
#define DT_PIT			(9)

#define ISWALKTHRU(x)	((x) != DT_SOLID)	// Tests for them
#define	ISDRAWWALL(x)	((x) == DT_SOLID || (x) == DT_HIDDENDOOR)
#define ISDRAWDOOR(x)   ((x) == DT_DOOR)
#define ISDRAWOPEN(x)	(ISDRAWWALL(x) == 0 && ISDRAWDOOR(x) == 0)

#define	COL_WALL		(C_GREEN)			// Object Colours
#define	COL_LADDER		(C_RED)
#define COL_DOOR		(C_BLUE)
#define COL_HOLE		(C_RED)
#define COL_MONSTER		(C_WHITE)
#define COL_MOUNTAIN	(C_YELLOW)
#define	COL_TREE		(C_GREEN)
#define COL_DUNGEON		(C_RED)
#define COL_TOWN		(C_BLUE)
#define COL_BRITISH		(C_WHITE)
#define COL_PLAYER		(C_CYAN)

#define MN_SKELETON     (1)					// Monster types
#define MN_THIEF        (2)
#define MN_RAT          (3)
#define MN_ORC          (4)
#define MN_VIPER        (5)
#define MN_CARRION      (6)
#define MN_GREMLIN      (7)
#define MN_MIMIC        (8)
#define MN_DAEMON       (9)
#define MN_BALROG       (10)

#define AT_HP			(0)				// Player attributes
#define	AT_STRENGTH		(1)
#define	AT_DEXTERITY	(2)
#define AT_STAMINA		(3)
#define AT_WISDOM		(4)
#define AT_GOLD			(5)

#define OB_FOOD			(0)				// Object Attributes
#define OB_RAPIER		(1)
#define OB_AXE			(2)
#define OB_SHIELD		(3)
#define OB_BOW			(4)
#define OB_AMULET		(5)

} // namespace Ultima0
} // namespace Ultima

#endif
