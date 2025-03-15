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

int MazeGen::chooseDoor() {
	int candidates[3];
	register int num_candidates;

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

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
