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

#include "bagel/hodjnpodj/maze_doom/maze_gen.h"
#include "bagel/hodjnpodj/maze_doom/defines.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

void MazeGen::initializeMaze() {
	int i, j, wall;

	maze_size_x = MAX_MAZE_SIZE_X;
	maze_size_y = MAX_MAZE_SIZE_Y;

	// Initialize all squares
	for (i = 0; i < maze_size_x; i++) {
		for (j = 0; j < maze_size_y; j++) {
			maze[i][j] = 0;
		}
	}

	// Top wall
	for (i = 0; i < maze_size_x; i++) {
		maze[i][0] |= WALL_TOP;
	}

	// Right wall
	for (j = 0; j < maze_size_y; j++) {
		maze[maze_size_x - 1][j] |= WALL_RIGHT;
	}

	// Bottom wall
	for (i = 0; i < maze_size_x; i++) {
		maze[i][maze_size_y - 1] |= WALL_BOTTOM;
	}

	// Left wall
	for (j = 0; j < maze_size_y; j++) {
		maze[0][j] |= WALL_LEFT;
	}

	// Set start square
	wall = 1;				// Start on right side
	i = maze_size_x - 1;	// Set maze x location
	// Set a random y location not on the top row
	j = g_engine->getRandomNumber(maze_size_y - 1);

	maze[i][j] |= START_SQUARE;
	maze[i][j] |= (DOOR_IN_TOP >> wall);
	maze[i][j] &= ~(WALL_TOP >> wall);
	start_x = i;
	start_y = j;
	cur_sq_x = i;
	cur_sq_y = j;
	sqnum = 0;

	// set end square
	wall = (wall + 2) % 4;
	switch (wall) {
	case 0:
		i = g_engine->getRandomNumber(maze_size_x - 1);
		j = 0;
		break;
	case 1:
		i = maze_size_x - 1;
		j = g_engine->getRandomNumber(maze_size_y - 1);
		break;
	case 2:
		i = g_engine->getRandomNumber(maze_size_x - 1);
		j = maze_size_y - 1;
		break;
	case 3:
		i = 0;
		j = g_engine->getRandomNumber(maze_size_y - 1);
		break;
	}

	maze[i][j] |= END_SQUARE;
	maze[i][j] |= (DOOR_OUT_TOP >> wall);
	maze[i][j] &= ~(WALL_TOP >> wall);
	end_x = i;
	end_y = j;
}

void MazeGen::createMaze() {
	int newdoor = 0;

	do {
		move_list[sqnum].x = cur_sq_x;
		move_list[sqnum].y = cur_sq_y;
		move_list[sqnum].dir = newdoor;
		while ((newdoor = chooseDoor()) == -1) { /* pick a door */
			if (backup() == -1) { /* no more doors ... backup */
				return; /* done ... return */
			}
		}

		/* mark the out door */
		maze[cur_sq_x][cur_sq_y] |= (DOOR_OUT_TOP >> newdoor);

		switch (newdoor) {
		case 0: cur_sq_y--;
			break;
		case 1: cur_sq_x++;
			break;
		case 2: cur_sq_y++;
			break;
		case 3: cur_sq_x--;
			break;
		}
		sqnum++;

		/* mark the in door */
		maze[cur_sq_x][cur_sq_y] |= (DOOR_IN_TOP >> ((newdoor + 2) % 4));

		/* if end square set path length and save path */
	} while (1);

}

void MazeGen::setupMaze() {
	int x, y;
	Common::Point exitPos;

	for (y = 0; y < NUM_ROWS; y++)                         // Set the right wall solid
		mazeTile[NUM_COLUMNS - 1][y].m_nWall = WALL;

	for (x = 0; x < MAX_MAZE_SIZE_X; x++) {
		for (y = 0; y < MAX_MAZE_SIZE_Y; y++) {
			mazeTile[x * 2 + 1][y * 2 + 1].m_nWall = PATH;          // Always is PATH
			mazeTile[x * 2][y * 2].m_nWall = PATH;              // Will be changed to WALL if 
			if (maze[x][y] & WALL_TOP) {                   //...it is found below
				mazeTile[x * 2][y * 2].m_nWall = WALL;
				mazeTile[x * 2 + 1][y * 2].m_nWall = WALL;
			} else
				mazeTile[x * 2 + 1][y * 2].m_nWall = PATH;

			if (maze[x][y] & WALL_LEFT) {
				mazeTile[x * 2][y * 2].m_nWall = WALL;
				mazeTile[x * 2][y * 2 + 1].m_nWall = WALL;
			} else
				mazeTile[x * 2][y * 2 + 1].m_nWall = PATH;
		}
	}

	for (x = 0; x < NUM_COLUMNS; x++) {        // Now go through  mazeTile and fix up loose ends, as it were
		for (y = 0; y < NUM_ROWS; y++) {
			mazeTile[x][y].m_bHidden = false;
			if (mazeTile[x][y].m_nWall == PATH) {
				if (mazeTile[x + 1][y + 1].m_nWall == PATH && (mazeTile[x + 1][y].m_nWall == PATH &&
					(mazeTile[x][y + 1].m_nWall == PATH &&
						(mazeTile[x - 1][y].m_nWall == WALL && mazeTile[x][y - 1].m_nWall == WALL))))
					mazeTile[x][y].m_nWall = WALL;              // If it's a right-hand corner 

				if (mazeTile[x][y + 1].m_nWall == PATH && (mazeTile[x + 1][y - 1].m_nWall == PATH &&
					(mazeTile[x - 1][y - 1].m_nWall == PATH &&
						(mazeTile[x - 1][y + 1].m_nWall == PATH && (mazeTile[x + 1][y + 1].m_nWall == PATH &&
							(mazeTile[x - 1][y].m_nWall == PATH && mazeTile[x + 1][y].m_nWall == PATH))))))
					mazeTile[x][y].m_nWall = WALL;              // If it's two wide vertically from the top

				if (mazeTile[x][y - 1].m_nWall == PATH && (mazeTile[x - 1][y - 1].m_nWall == PATH &&
					(mazeTile[x - 1][y + 1].m_nWall == PATH &&
						(mazeTile[x][y + 1].m_nWall == PATH && (mazeTile[x + 1][y - 1].m_nWall == PATH &&
							(mazeTile[x + 1][y].m_nWall == PATH && mazeTile[x + 1][y + 1].m_nWall == PATH))))))
					mazeTile[x][y].m_nWall = WALL;              // If it's two wide horizontally from the left

				if (y == NUM_ROWS - 1)
					mazeTile[x][y].m_nWall = WALL;              // Make bottom wall
			}
		}
	}

	x = NUM_COLUMNS - 1;                                // Get the Entry point
	y = (start_y * 2) + 1;

	m_PlayerPos.x = x - 1;                              // Start player in one space from the entrance

	if (mazeTile[x - 1][y].m_nWall == WALL) {          // If a wall runs into the entry space 
		mazeTile[x][y].m_nWall = WALL;                  //...make it a wall and put the entry
		mazeTile[x][y + 1].m_nWall = START;             //...space under that
		m_PlayerPos.y = y;                              // Put the player there
	} else {
		mazeTile[x][y].m_nWall = START;                 // Put in the entry way where it was     
		mazeTile[x][y + 1].m_nWall = WALL;              //...and make sure the one below is a wall
		m_PlayerPos.y = y;                              // Put the player there
	}

	x = end_x * 2;                                      // This should be 0
	y = end_y * 2;
	exitPos.x = x;

	if (mazeTile[x + 1][y].m_nWall == WALL) {          // If a wall runs into the top exit space 
		mazeTile[x][y].m_nWall = WALL;                  //...make it a wall and put the exit
		exitPos.y = y + 1;                              //...one space above that
	} else {
		mazeTile[x][y + 1].m_nWall = WALL;              // Put the exit in the top space
		exitPos.y = y;                                  //...and store the y position in m_pExit
	}

	mazeTile[exitPos.x][exitPos.y].m_nWall = EXIT;      // Make exit grid space a Pathway

	setInvisibleWalls();                                // Hide some walls
	setTraps();                                         // Put in some traps
}

int MazeGen::chooseDoor() {
	int candidates[3];
	int num_candidates;

	num_candidates = 0;

	//topwall:
	/* top wall */
	if (maze[cur_sq_x][cur_sq_y] & DOOR_IN_TOP)
		goto rightwall;
	if (maze[cur_sq_x][cur_sq_y] & DOOR_OUT_TOP)
		goto rightwall;
	if (maze[cur_sq_x][cur_sq_y] & WALL_TOP)
		goto rightwall;
	if (maze[cur_sq_x][cur_sq_y - 1] & DOOR_IN_ANY) {
		maze[cur_sq_x][cur_sq_y] |= WALL_TOP;
		maze[cur_sq_x][cur_sq_y - 1] |= WALL_BOTTOM;
		goto rightwall;
	}
	candidates[num_candidates++] = 0;

rightwall:
	/* right wall */
	if (maze[cur_sq_x][cur_sq_y] & DOOR_IN_RIGHT)
		goto bottomwall;
	if (maze[cur_sq_x][cur_sq_y] & DOOR_OUT_RIGHT)
		goto bottomwall;
	if (maze[cur_sq_x][cur_sq_y] & WALL_RIGHT)
		goto bottomwall;
	if (maze[cur_sq_x + 1][cur_sq_y] & DOOR_IN_ANY) {
		maze[cur_sq_x][cur_sq_y] |= WALL_RIGHT;
		maze[cur_sq_x + 1][cur_sq_y] |= WALL_LEFT;
		goto bottomwall;
	}
	candidates[num_candidates++] = 1;

bottomwall:
	/* bottom wall */
	if (maze[cur_sq_x][cur_sq_y] & DOOR_IN_BOTTOM)
		goto leftwall;
	if (maze[cur_sq_x][cur_sq_y] & DOOR_OUT_BOTTOM)
		goto leftwall;
	if (maze[cur_sq_x][cur_sq_y] & WALL_BOTTOM)
		goto leftwall;
	if (maze[cur_sq_x][cur_sq_y + 1] & DOOR_IN_ANY) {
		maze[cur_sq_x][cur_sq_y] |= WALL_BOTTOM;
		maze[cur_sq_x][cur_sq_y + 1] |= WALL_TOP;
		goto leftwall;
	}
	candidates[num_candidates++] = 2;

leftwall:
	/* left wall */
	if (maze[cur_sq_x][cur_sq_y] & DOOR_IN_LEFT)
		goto donewall;
	if (maze[cur_sq_x][cur_sq_y] & DOOR_OUT_LEFT)
		goto donewall;
	if (maze[cur_sq_x][cur_sq_y] & WALL_LEFT)
		goto donewall;
	if (maze[cur_sq_x - 1][cur_sq_y] & DOOR_IN_ANY) {
		maze[cur_sq_x][cur_sq_y] |= WALL_LEFT;
		maze[cur_sq_x - 1][cur_sq_y] |= WALL_RIGHT;
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
	sqnum--;
	cur_sq_x = move_list[sqnum].x;
	cur_sq_y = move_list[sqnum].y;
	return sqnum;
}

void MazeGen::setInvisibleWalls() {
	int x, y, i, j;
	int nWallCount = 0;
	int nMaxWalls = 0;
	int nTotalWalls = 0;

	for (x = 1; x < (NUM_COLUMNS - 1); x++) {                     // Don't make edge walls invisible !!
		for (y = 1; y < (NUM_ROWS - 1); y++) {
			if (mazeTile[x][y].m_nWall == WALL) {
				if (m_nDifficulty > MIN_DIFFICULTY)               // Most difficult has all walls hidden  
					mazeTile[x][y].m_bHidden = true;                // Start with all walls hidden
				else
					mazeTile[x][y].m_bHidden = false;               // Least difficult has no walls hidden
				nTotalWalls++;
			}
		}
	}

	if (m_nDifficulty > MIN_DIFFICULTY && m_nDifficulty < MAX_DIFFICULTY) {
		x = g_engine->getRandomNumber((NUM_COLUMNS - 4) - 1) + 2;                       // Avoid the edge walls
		y = g_engine->getRandomNumber((NUM_ROWS - 4) - 1) + 2;
		nMaxWalls = nTotalWalls - (int)(m_nDifficulty * (nTotalWalls / 10));

		while (nWallCount < nMaxWalls) {
			if (mazeTile[x][y].m_nWall == WALL && mazeTile[x][y].m_bHidden) {
				for (i = x - 1; i <= x + 1; i++) {
					for (j = y - 1; j <= y + 1; j++) {
						if (mazeTile[i][j].m_nWall == WALL && mazeTile[i][j].m_bHidden) {
							mazeTile[i][j].m_bHidden = false;       // so it's not hidden
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

	nNumTraps = MIN_TRAPS + (m_nDifficulty / 2);          // 4 + ([1...10]/2) = 4 to 9 

	for (nTrapCount = 0; nTrapCount < nNumTraps; nTrapCount++) {
		In = getRandomPoint(false);                                       // Pick a random PATH square
		mazeTile[In.x][In.y].m_nWall = TRAP;                                // Make it a TRAP
		mazeTile[In.x][In.y].m_bHidden = true;                              // Hide it
		mazeTile[In.x][In.y].m_nTrap = nTrapCount % NUM_TRAP_MAPS;          // Assign unique trap bitmap ID
		mazeTile[In.x][In.y].m_nDest = getRandomPoint(true);              // Pick a random Trap destination
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
		if (mazeTile[point.x][point.y].m_nWall == PATH)
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

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
