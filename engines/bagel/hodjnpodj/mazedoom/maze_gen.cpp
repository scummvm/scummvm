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

#include "bagel/hodjnpodj/mazedoom/maze_gen.h"
#include "bagel/hodjnpodj/mazedoom/defines.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

void MazeGen::initializeMaze() {
	int i, j, wall;

	_mazeSizeX = MAX_MAZE_SIZE_X;
	_mazeSizeY = MAX_MAZE_SIZE_Y;

	// Initialize all squares
	for (i = 0; i < _mazeSizeX; i++) {
		for (j = 0; j < _mazeSizeY; j++) {
			_maze[i][j] = 0;
		}
	}

	// Top wall
	for (i = 0; i < _mazeSizeX; i++) {
		_maze[i][0] |= WALL_TOP;
	}

	// Right wall
	for (j = 0; j < _mazeSizeY; j++) {
		_maze[_mazeSizeX - 1][j] |= WALL_RIGHT;
	}

	// Bottom wall
	for (i = 0; i < _mazeSizeX; i++) {
		_maze[i][_mazeSizeY - 1] |= WALL_BOTTOM;
	}

	// Left wall
	for (j = 0; j < _mazeSizeY; j++) {
		_maze[0][j] |= WALL_LEFT;
	}

	// Set start square
	wall = 1;				// Start on right side
	i = _mazeSizeX - 1;	// Set maze x location
	// Set a random y location not on the top row
	j = g_engine->getRandomNumber(_mazeSizeY - 1);

	_maze[i][j] |= START_SQUARE;
	_maze[i][j] |= (DOOR_IN_TOP >> wall);
	_maze[i][j] &= ~(WALL_TOP >> wall);
	_startX = i;
	_startY = j;
	_curSqX = i;
	_curSqY = j;
	_sqNum = 0;

	// set end square
	wall = (wall + 2) % 4;
	switch (wall) {
	case 0:
		i = g_engine->getRandomNumber(_mazeSizeX - 1);
		j = 0;
		break;
	case 1:
		i = _mazeSizeX - 1;
		j = g_engine->getRandomNumber(_mazeSizeY - 1);
		break;
	case 2:
		i = g_engine->getRandomNumber(_mazeSizeX - 1);
		j = _mazeSizeY - 1;
		break;
	case 3:
		i = 0;
		j = g_engine->getRandomNumber(_mazeSizeY - 1);
		break;
	}

	_maze[i][j] |= END_SQUARE;
	_maze[i][j] |= (DOOR_OUT_TOP >> wall);
	_maze[i][j] &= ~(WALL_TOP >> wall);
	_endX = i;
	_endY = j;
}

void MazeGen::createMaze() {
	int newdoor = 0;

	do {
		_moveList[_sqNum].x = _curSqX;
		_moveList[_sqNum].y = _curSqY;
		_moveList[_sqNum].dir = newdoor;
		while ((newdoor = chooseDoor()) == -1) { /* pick a door */
			if (backup() == -1) { /* no more doors ... backup */
				return; /* done ... return */
			}
		}

		/* mark the out door */
		_maze[_curSqX][_curSqY] |= (DOOR_OUT_TOP >> newdoor);

		switch (newdoor) {
		case 0: _curSqY--;
			break;
		case 1: _curSqX++;
			break;
		case 2: _curSqY++;
			break;
		case 3: _curSqX--;
			break;
		}
		_sqNum++;

		/* mark the in door */
		_maze[_curSqX][_curSqY] |= (DOOR_IN_TOP >> ((newdoor + 2) % 4));

		/* if end square set path length and save path */
	} while (1);

}

void MazeGen::setupMaze() {
	int x, y;
	Common::Point exitPos;

	for (y = 0; y < NUM_ROWS; y++)                         // Set the right wall solid
		_mazeTile[NUM_COLUMNS - 1][y].m_nWall = WALL;

	for (x = 0; x < MAX_MAZE_SIZE_X; x++) {
		for (y = 0; y < MAX_MAZE_SIZE_Y; y++) {
			_mazeTile[x * 2 + 1][y * 2 + 1].m_nWall = PATH;          // Always is PATH
			_mazeTile[x * 2][y * 2].m_nWall = PATH;              // Will be changed to WALL if 
			if (_maze[x][y] & WALL_TOP) {                   //...it is found below
				_mazeTile[x * 2][y * 2].m_nWall = WALL;
				_mazeTile[x * 2 + 1][y * 2].m_nWall = WALL;
			} else
				_mazeTile[x * 2 + 1][y * 2].m_nWall = PATH;

			if (_maze[x][y] & WALL_LEFT) {
				_mazeTile[x * 2][y * 2].m_nWall = WALL;
				_mazeTile[x * 2][y * 2 + 1].m_nWall = WALL;
			} else
				_mazeTile[x * 2][y * 2 + 1].m_nWall = PATH;
		}
	}

	for (x = 0; x < NUM_COLUMNS; x++) {        // Now go through  mazeTile and fix up loose ends, as it were
		for (y = 0; y < NUM_ROWS; y++) {
			_mazeTile[x][y].m_bHidden = false;
			if (_mazeTile[x][y].m_nWall == PATH) {
				if (_mazeTile[x + 1][y + 1].m_nWall == PATH && (_mazeTile[x + 1][y].m_nWall == PATH &&
					(_mazeTile[x][y + 1].m_nWall == PATH &&
						(_mazeTile[x - 1][y].m_nWall == WALL && _mazeTile[x][y - 1].m_nWall == WALL))))
					_mazeTile[x][y].m_nWall = WALL;              // If it's a right-hand corner 

				if (_mazeTile[x][y + 1].m_nWall == PATH && (_mazeTile[x + 1][y - 1].m_nWall == PATH &&
					(_mazeTile[x - 1][y - 1].m_nWall == PATH &&
						(_mazeTile[x - 1][y + 1].m_nWall == PATH && (_mazeTile[x + 1][y + 1].m_nWall == PATH &&
							(_mazeTile[x - 1][y].m_nWall == PATH && _mazeTile[x + 1][y].m_nWall == PATH))))))
					_mazeTile[x][y].m_nWall = WALL;              // If it's two wide vertically from the top

				if (_mazeTile[x][y - 1].m_nWall == PATH && (_mazeTile[x - 1][y - 1].m_nWall == PATH &&
					(_mazeTile[x - 1][y + 1].m_nWall == PATH &&
						(_mazeTile[x][y + 1].m_nWall == PATH && (_mazeTile[x + 1][y - 1].m_nWall == PATH &&
							(_mazeTile[x + 1][y].m_nWall == PATH && _mazeTile[x + 1][y + 1].m_nWall == PATH))))))
					_mazeTile[x][y].m_nWall = WALL;              // If it's two wide horizontally from the left

				if (y == NUM_ROWS - 1)
					_mazeTile[x][y].m_nWall = WALL;              // Make bottom wall
			}
		}
	}

	x = NUM_COLUMNS - 1;                                // Get the Entry point
	y = (_startY * 2) + 1;

	_playerPos.x = x - 1;                              // Start player in one space from the entrance

	if (_mazeTile[x - 1][y].m_nWall == WALL) {          // If a wall runs into the entry space 
		_mazeTile[x][y].m_nWall = WALL;                  //...make it a wall and put the entry
		_mazeTile[x][y + 1].m_nWall = START;             //...space under that
		_playerPos.y = y;                              // Put the player there
	} else {
		_mazeTile[x][y].m_nWall = START;                 // Put in the entry way where it was     
		_mazeTile[x][y + 1].m_nWall = WALL;              //...and make sure the one below is a wall
		_playerPos.y = y;                              // Put the player there
	}

	x = _endX * 2;                                      // This should be 0
	y = _endY * 2;
	exitPos.x = x;

	if (_mazeTile[x + 1][y].m_nWall == WALL) {          // If a wall runs into the top exit space 
		_mazeTile[x][y].m_nWall = WALL;                  //...make it a wall and put the exit
		exitPos.y = y + 1;                              //...one space above that
	} else {
		_mazeTile[x][y + 1].m_nWall = WALL;              // Put the exit in the top space
		exitPos.y = y;                                  //...and store the y position in m_pExit
	}

	_mazeTile[exitPos.x][exitPos.y].m_nWall = EXIT;      // Make exit grid space a Pathway

	setInvisibleWalls();                                // Hide some walls
	setTraps();                                         // Put in some traps
}

int MazeGen::chooseDoor() {
	int candidates[3];
	int num_candidates;

	num_candidates = 0;

	//topwall:
	/* top wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_TOP)
		goto rightwall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_TOP)
		goto rightwall;
	if (_maze[_curSqX][_curSqY] & WALL_TOP)
		goto rightwall;
	if (_maze[_curSqX][_curSqY - 1] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_TOP;
		_maze[_curSqX][_curSqY - 1] |= WALL_BOTTOM;
		goto rightwall;
	}
	candidates[num_candidates++] = 0;

rightwall:
	/* right wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_RIGHT)
		goto bottomwall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_RIGHT)
		goto bottomwall;
	if (_maze[_curSqX][_curSqY] & WALL_RIGHT)
		goto bottomwall;
	if (_maze[_curSqX + 1][_curSqY] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_RIGHT;
		_maze[_curSqX + 1][_curSqY] |= WALL_LEFT;
		goto bottomwall;
	}
	candidates[num_candidates++] = 1;

bottomwall:
	/* bottom wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_BOTTOM)
		goto leftwall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_BOTTOM)
		goto leftwall;
	if (_maze[_curSqX][_curSqY] & WALL_BOTTOM)
		goto leftwall;
	if (_maze[_curSqX][_curSqY + 1] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_BOTTOM;
		_maze[_curSqX][_curSqY + 1] |= WALL_TOP;
		goto leftwall;
	}
	candidates[num_candidates++] = 2;

leftwall:
	/* left wall */
	if (_maze[_curSqX][_curSqY] & DOOR_IN_LEFT)
		goto donewall;
	if (_maze[_curSqX][_curSqY] & DOOR_OUT_LEFT)
		goto donewall;
	if (_maze[_curSqX][_curSqY] & WALL_LEFT)
		goto donewall;
	if (_maze[_curSqX - 1][_curSqY] & DOOR_IN_ANY) {
		_maze[_curSqX][_curSqY] |= WALL_LEFT;
		_maze[_curSqX - 1][_curSqY] |= WALL_RIGHT;
		goto donewall;
	}
	candidates[num_candidates++] = 3;

donewall:
	if (num_candidates == 0)
		return (-1);
	if (num_candidates == 1)
		return (candidates[0]);

	return candidates[g_engine->getRandomNumber(
		num_candidates - 1)];
}

int MazeGen::backup() {
	_sqNum--;
	_curSqX = _moveList[_sqNum].x;
	_curSqY = _moveList[_sqNum].y;
	return _sqNum;
}

void MazeGen::setInvisibleWalls() {
	int x, y, i, j;
	int nWallCount = 0;
	int nMaxWalls = 0;
	int nTotalWalls = 0;

	for (x = 1; x < (NUM_COLUMNS - 1); x++) {                     // Don't make edge walls invisible !!
		for (y = 1; y < (NUM_ROWS - 1); y++) {
			if (_mazeTile[x][y].m_nWall == WALL) {
				if (_difficulty > MIN_DIFFICULTY)               // Most difficult has all walls hidden  
					_mazeTile[x][y].m_bHidden = true;                // Start with all walls hidden
				else
					_mazeTile[x][y].m_bHidden = false;               // Least difficult has no walls hidden
				nTotalWalls++;
			}
		}
	}

	if (_difficulty > MIN_DIFFICULTY && _difficulty < MAX_DIFFICULTY) {
		x = g_engine->getRandomNumber((NUM_COLUMNS - 4) - 1) + 2;                       // Avoid the edge walls
		y = g_engine->getRandomNumber((NUM_ROWS - 4) - 1) + 2;
		nMaxWalls = nTotalWalls - (int)(_difficulty * (nTotalWalls / 10));

		while (nWallCount < nMaxWalls) {
			if (_mazeTile[x][y].m_nWall == WALL && _mazeTile[x][y].m_bHidden) {
				for (i = x - 1; i <= x + 1; i++) {
					for (j = y - 1; j <= y + 1; j++) {
						if (_mazeTile[i][j].m_nWall == WALL && _mazeTile[i][j].m_bHidden) {
							_mazeTile[i][j].m_bHidden = false;       // so it's not hidden
							nWallCount++;                           // increment the count
						}
					}
				}
			}

			x += g_engine->getRandomNumber(NUM_NEIGHBORS - 1);// + 1;                                   // Increment Column 
			y += g_engine->getRandomNumber(NUM_NEIGHBORS - 1);// + 1;                                   // Increment Row
			if (x >= (NUM_COLUMNS - 2))
				x = g_engine->getRandomNumber((NUM_COLUMNS - 4) - 1) + 2;               // If we're at the end,
			if (y >= (NUM_ROWS - 2))
				y = g_engine->getRandomNumber((NUM_COLUMNS - 4) - 1) + 2;               //...reset the counter
		}
	}
}

void MazeGen::setTraps() {
	int nTrapCount;
	int nNumTraps;
	Common::Point In;

	nNumTraps = MIN_TRAPS + (_difficulty / 2);          // 4 + ([1...10]/2) = 4 to 9 

	for (nTrapCount = 0; nTrapCount < nNumTraps; nTrapCount++) {
		In = getRandomPoint(false);                                       // Pick a random PATH square
		_mazeTile[In.x][In.y].m_nWall = TRAP;                                // Make it a TRAP
		_mazeTile[In.x][In.y].m_bHidden = true;                              // Hide it
		_mazeTile[In.x][In.y].m_nTrap = nTrapCount % NUM_TRAP_MAPS;          // Assign unique trap bitmap ID
		_mazeTile[In.x][In.y].m_nDest = getRandomPoint(true);              // Pick a random Trap destination
	}
}

Common::Point MazeGen::getRandomPoint(bool bRight) {
	Common::Point point;
	bool bLocated = false;

	// Get random column
	if (bRight)
		point.x = g_engine->getRandomNumber((2 * (NUM_COLUMNS / 3)) - 1) +
			(NUM_COLUMNS / 3);
	else
		point.x = g_engine->getRandomNumber(2 * (NUM_COLUMNS / 3) - 1);

	// Get random row
	point.y = g_engine->getRandomNumber(NUM_ROWS - 1);

	while (!bLocated) {
		if (_mazeTile[point.x][point.y].m_nWall == PATH)
			bLocated = true;                                                // OK if it's a pathway
		else {                                                              // Otherwise, keep lookin'
			point.x++;                                                      // Increment Column 
			point.y++;                                                      // Increment Row
			if (point.x == NUM_COLUMNS) point.x = 1;                      // If we're at the end,
			if (point.y == NUM_ROWS) point.y = 1;                         //...reset the counter
		}
	}

	return point;
}

void MazeGen::paintMaze() {
	int x, y;

	for (x = 0; x < NUM_COLUMNS; x++) {
		for (y = 0; y < NUM_ROWS; y++) {
			// Set the tile location
			_mazeTile[x][y].m_nStart.x = x * SQ_SIZE_X;                              // Put in location info
			_mazeTile[x][y].m_nStart.y = y * SQ_SIZE_Y;

			// Handle drawing the correct tile
			if ((_mazeTile[x][y].m_nWall == PATH) ||
				(_mazeTile[x][y].m_nWall == EXIT) ||
				_mazeTile[x][y].m_bHidden)
				// Path or hidden obj 
				_mazeBitmap.blitFrom(_pathBitmap, Common::Point(
					_mazeTile[x][y].m_nStart.x, _mazeTile[x][y].m_nStart.y));
			else if (_mazeTile[x][y].m_nWall == START)
				// Start of maze
				_mazeBitmap.blitFrom(_startBitmap, Common::Point(
					_mazeTile[x][y].m_nStart.x, _mazeTile[x][y].m_nStart.y));
			else if (_mazeTile[x][y].m_nWall == TRAP)
				// Revealed trap
				// Start of maze
				_mazeBitmap.blitFrom(_trapBitmap[_mazeTile[x][y].m_nTrap],
					_mazeTile[x][y].m_nStart);
			else
				// Otherwise, it's a wall
				_mazeBitmap.blitFrom(_wallBitmap, Common::Point(
					_mazeTile[x][y].m_nStart.x, _mazeTile[x][y].m_nStart.y));

			if (_showOverlays && _mazeTile[x][y].m_nWall != 0) {
				Common::Rect r(SQ_SIZE_X, SQ_SIZE_Y);
				r.moveTo(_mazeTile[x][y].m_nStart);
				_mazeBitmap.fillRect(r, _mazeTile[x][y].m_nWall * 10);
			}
		}
	}

	for (x = 0; x < NUM_COLUMNS; x++) {		// Go through the grid
		for (y = 0; y < NUM_ROWS; y++) {	//...and for every square
			addEdges(x, y);					//...add trim if needed
		}
	}
}

void MazeGen::addEdges(int x, int y, int offset_x, int offset_y) {
	if ((_mazeTile[x][y].m_bHidden == false) && (_mazeTile[x][y].m_nWall == WALL)) {
		if ((y > 0) && ((((_mazeTile[x][y - 1].m_nWall == PATH) || (_mazeTile[x][y - 1].m_nWall == EXIT)) ||
				(_mazeTile[x][y - 1].m_nWall == START)) || _mazeTile[x][y - 1].m_bHidden))
			// TOP
			_mazeBitmap.blitFrom(_bottomEdgeBmp, Common::Point(
				_mazeTile[x][y - 1].m_nStart.x + offset_x,
				_mazeTile[x][y - 1].m_nStart.y + offset_y + SQ_SIZE_Y - 1 - EDGE_SIZE));

		if ((x < (NUM_COLUMNS - 1)) && ((_mazeTile[x + 1][y].m_nWall == PATH) ||
				_mazeTile[x + 1][y].m_bHidden))
			// RIGHT
			_mazeBitmap.blitFrom(_leftEdgeBmp, Common::Point(
				_mazeTile[x + 1][y].m_nStart.x + offset_x,
				_mazeTile[x + 1][y].m_nStart.y + offset_y));

		if ((y < (NUM_ROWS - 1)) && ((((_mazeTile[x][y + 1].m_nWall == EXIT) ||
				(_mazeTile[x][y + 1].m_nWall == PATH)) ||
				(_mazeTile[x][y + 1].m_nWall == START)) || _mazeTile[x][y + 1].m_bHidden))
			// BOTTOM
			_mazeBitmap.blitFrom(_topEdgeBmp, Common::Point(
				_mazeTile[x][y + 1].m_nStart.x + offset_x,
				_mazeTile[x][y + 1].m_nStart.y + offset_y));

		if ((x > 0) && ((_mazeTile[x - 1][y].m_nWall == PATH) ||
			_mazeTile[x - 1][y].m_bHidden))
			// LEFT
			_mazeBitmap.blitFrom(_rightEdgeBmp, Common::Point(
				_mazeTile[x - 1][y].m_nStart.x + offset_x + SQ_SIZE_X - 1 - EDGE_SIZE,
				_mazeTile[x - 1][y].m_nStart.y + offset_y));
	}
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
