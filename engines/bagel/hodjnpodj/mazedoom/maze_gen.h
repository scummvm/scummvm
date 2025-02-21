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

#ifndef HODJNPODJ_MAZEDOOM_MAZE_GEN_H
#define HODJNPODJ_MAZEDOOM_MAZE_GEN_H

#include "common/scummsys.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

//	Defines for Maze generation routines:

#define MAX_MAZE_SIZE_X	12
#define MAX_MAZE_SIZE_Y	9

#define MOVE_LIST_SIZE  (MAX_MAZE_SIZE_X * MAX_MAZE_SIZE_Y)

#define WALL_TOP	0x8000
#define WALL_RIGHT	0x4000
#define WALL_BOTTOM	0x2000
#define WALL_LEFT	0x1000

#define DOOR_IN_TOP		0x800
#define DOOR_IN_RIGHT	0x400
#define DOOR_IN_BOTTOM	0x200
#define DOOR_IN_LEFT	0x100
#define DOOR_IN_ANY		0xF00

#define DOOR_OUT_TOP	0x80
#define DOOR_OUT_RIGHT	0x40
#define DOOR_OUT_BOTTOM	0x20
#define DOOR_OUT_LEFT	0x10

#define START_SQUARE	0x2
#define END_SQUARE		0x1

extern uint16 maze[MAX_MAZE_SIZE_X][MAX_MAZE_SIZE_Y];

struct PosDir {
	byte x;
	byte y;
	byte dir;

	void clear() {
		x = y = dir = 9;
	}
};
extern PosDir move_list[MOVE_LIST_SIZE];

extern int maze_size_x, maze_size_y;
extern int sqnum, cur_sq_x, cur_sq_y;
extern int start_x, start_y, end_x, end_y;

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
