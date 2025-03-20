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

#include "common/rect.h"
#include "bagel/hodjnpodj/gfx/gfx_surface.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define NUM_COLUMNS		25
#define NUM_ROWS		19
#define	NUM_TRAP_MAPS	 7							// There are seven trap icons available

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

extern uint16 _maze[MAX_MAZE_SIZE_X][MAX_MAZE_SIZE_Y];

struct PosDir {
	byte x = 0;
	byte y = 0;
	byte dir = 0;

	void clear() {
		x = y = dir = 9;
	}
};

// Data type for each square of the underlying Grid of the Maze
struct Tile {
	Common::Point m_nStart;	// Upper-left-hand corner where the bmp is to be drawn (24 X 24)
	uint m_nWall = 0;		// 0 = Path, 1 = Wall, 2 = Trap, etc...
	uint m_nTrap = 0;		// Index of trap bitmap to use for drawing
	Common::Point m_nDest;	// x,y Tile location of Trap exit point 
	bool m_bHidden = false;	// 0 = Visible, 1 = Invisible
};


struct MazeGen {
	uint16 _maze[MAX_MAZE_SIZE_X][MAX_MAZE_SIZE_Y];
	PosDir _moveList[MOVE_LIST_SIZE];
	Tile _mazeTile[NUM_COLUMNS][NUM_ROWS];
	int _mazeSizeX = 0, _mazeSizeY = 0;
	int _sqNum = 0, _curSqX = 0, _curSqY = 0;
	int _startX = 0, _startY = 0, _endX = 0, _endY = 0;

	Common::Point _playerPos;
	int _difficulty = 0;
	GfxSurface _mazeBitmap;
	GfxSurface _wallBitmap,
		_pathBitmap, _startBitmap;
	GfxSurface _leftEdgeBmp, _rightEdgeBmp,
		_topEdgeBmp, _bottomEdgeBmp;
	GfxSurface _trapBitmap[NUM_TRAP_MAPS];
	bool _showOverlays = false;

	/**
	 * Set the surrounding wall and start/end squares
	 */
	void initializeMaze();

	/**
	 * Create a maze layout given the intiialized maze
	 */
	void createMaze();

	/**
	 * Translates the random maze generated into
	 * the mazeTile grid for the game
	 *  IMPLICIT INPUT PARAMETERS:
	 *
	 *      maze[][]            The randomly generated maze
	 *      Tile mazeTile[][] grid
	 *      start_y
	 *      exit_y
	 *
	 *  IMPLICIT OUTPUT PARAMETERS:
	 *
	 *      Tile mazeTile[][] grid
	 */
	void setupMaze();

	/**
	 * Pick a new path
	 */
	int chooseDoor();

	/**
	 * Back up a move
	 */
	int backup();

	/**
	 * Randomly sets a number of walls invisible
	 *
	 *  IMPLICIT INPUT PARAMETERS:
	 *
	 *      Tile mazeTile[][] grid
	 *      m_nDifficulty       If it's MIN_DIFFICULTY, no walls are invisible
	 *                          If it's MAX_DIFFICULTY, all walls are invisible
	 *                          Otherwise, every m_nDifficulty-th wall is visible
	 *
	 *  IMPLICIT OUTPUT PARAMETERS:
	 *
	 *      Tile mazeTile[][] grid
	 */
	void setInvisibleWalls();

	/**
	 * Sets traps in the maze
	 *
	 *  IMPLICIT INPUT PARAMETERS:
	 *
	 *      Tile mazeTile[][] array
	 *      m_nDifficulty       The the number of traps = difficulty setting
	 *
	 *  IMPLICIT OUTPUT PARAMETERS:
	 *
	 *      Tile mazeTile[][] array
	 */
	void setTraps();

	/**
	 * Gets a random Grid Point in the maze, which is a PATH (not START or EXIT)
	 *
	 *  IMPLICIT INPUT PARAMETERS:
	 *
	 *      Tile mazeTile[][] grid
	 *
	 *  RETURN VALUE:
	 *
	 *      Point      The random X and Y of a path space in the mazeTile grid
	 */
	Common::Point getRandomPoint(bool bRight);

	/**
	 * Paints the maze onto a passed bitmap
	 */
	void paintMaze();

	/**
	 * Draws fancy edgework around a piece of wall on the sides where possible
	 *
	 *  FORMAL PARAMETERS:
	 *
	 *      int     x           Column of piece to check
	 *      int     y           Row of piece to check
	 *      int     offset_x    Width offset for drawing the bitmap
	 *      int     offset_y    Hieght offset as above
	 */
	void addEdges(int x, int y, int offset_x = 0, int offset_y = 0);
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
