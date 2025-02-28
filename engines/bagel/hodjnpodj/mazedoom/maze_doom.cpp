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

#include "common/textconsole.h"
#include "bagel/hodjnpodj/mazedoom/maze_doom.h"
#include "bagel/hodjnpodj/mazedoom/maze_gen.h"
#include "bagel/hodjnpodj/mazedoom/main_window.h"
#include "bagel/hodjnpodj/mazedoom/option_dialog.h"
#include "bagel/hodjnpodj/mazedoom/globals.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/boflib/bitmaps.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/mfc/afx.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

CBmpButton *m_pScrollButton;
CSprite *pPlayerSprite;
CPalette *pGamePalette,                   // Palette of current artwork
	*pOldPal;
CBitmap *pMazeBitmap,
	*pOldBmp,
	*pWallBitmap,
	*pPathBitmap,
	*pStartBitmap,
	*pTopEdgeBmp,
	*pRightEdgeBmp,
	*pBottomEdgeBmp,
	*pLeftEdgeBmp,
	*TrapBitmap[NUM_TRAP_MAPS];
CDC *pMazeDC;                    // DC for the MazeBitmap
CText *m_pTimeText;              // Time to be posted in Locale box of screen
CBitmap *pLocaleBitmap,          // Locale of game bitmap for title bar
	*pBlankBitmap;               // Blank area of locale for time display

bool bSuccess;
bool m_bIgnoreScrollClick;
bool bPlaying;
bool m_bGameOver;
CPoint m_PlayerPos;

int m_nDifficulty;
int m_nTime, nSeconds, nMinutes;
int tempDifficulty;
int tempTime;

// Data type for each square of the underlying Grid of the Maze
struct TILE {
	CPoint   m_nStart;   // Upper-left-hand corner where the bmp is to be drawn (24 X 24)
	uint    m_nWall;    // 0 = Path, 1 = Wall, 2 = Trap, etc...
	uint    m_nTrap;    // Index of trap bitmap to use for drawing
	CPoint   m_nDest;    // x,y Tile location of Trap exit point
	bool    m_bHidden;  // 0 = Visible, 1 = Invisible

	void clear() {
		m_nStart = CPoint();
		m_nWall = 0;
		m_nTrap = 0;
		m_nDest = CPoint();
		m_bHidden = false;
	}
} mazeTile[NUM_COLUMNS][NUM_ROWS];

CSound *pGameSound;                             // Game theme song

LPGAMESTRUCT pGameInfo;

static void init() {
	m_pScrollButton = nullptr;
	pPlayerSprite = nullptr;
	pGamePalette = nullptr,                   // Palette of current artwork
	pOldPal = nullptr;
	pMazeBitmap = nullptr;
	pOldBmp = nullptr;
	pWallBitmap = nullptr;
	pPathBitmap = nullptr;
	pStartBitmap = nullptr;
	pTopEdgeBmp = nullptr;
	pRightEdgeBmp = nullptr;
	pBottomEdgeBmp = nullptr;
	pLeftEdgeBmp = nullptr;
	Common::fill(TrapBitmap, TrapBitmap + NUM_TRAP_MAPS, (CBitmap *)nullptr);
	pMazeDC = nullptr;
	m_pTimeText = nullptr;
	pLocaleBitmap = nullptr;
	pBlankBitmap = nullptr;

	bSuccess = false;
	m_bIgnoreScrollClick = false;
	bPlaying = false;
	m_bGameOver = false;
	m_PlayerPos = CPoint();

	m_nDifficulty = 0;
	m_nTime = 0;
	nSeconds = 0;
	nMinutes = 0;
	tempDifficulty = 0;
	tempTime = 0;

	for (int x = 0; x < NUM_COLUMNS; ++x)
		for (int y = 0; y < NUM_ROWS; ++y)
			mazeTile[x][y].clear();

	for (int i = 0; i < MOVE_LIST_SIZE; ++i)
		move_list[i].clear();

	maze_size_x = maze_size_y = 0;
	sqnum = cur_sq_x = cur_sq_y = 0;
	start_x = start_y = end_x = end_y = 0;
}

void run() {
	CMainWindow *pMain;
	pGameInfo = &g_engine->gGameInfo;
	init();

#if 0
	ghParentWnd = hParentWnd;
#endif

	if ((pMain = new CMainWindow) != nullptr) {

		pMain->ShowWindow(SW_SHOWNORMAL);
		pMain->UpdateWindow();
		pMain->SetActiveWindow();
	}
}

void PaintMaze(CDC *pDC) {
	int x, y;

	for (x = 0; x < NUM_COLUMNS; x++) {
		for (y = 0; y < NUM_ROWS; y++) {
			mazeTile[x][y].m_nStart.x = x * SQ_SIZE_X;                              // Put in location info
			mazeTile[x][y].m_nStart.y = y * SQ_SIZE_Y;
			if ((mazeTile[x][y].m_nWall == PATH) || (mazeTile[x][y].m_nWall == EXIT) ||
				mazeTile[x][y].m_bHidden)      // Path or hidden obj. 
				PaintBitmap(pDC, pGamePalette, pPathBitmap,                        //...draw path bitmap
					mazeTile[x][y].m_nStart.x, mazeTile[x][y].m_nStart.y);
			else if (mazeTile[x][y].m_nWall == START)                             // Start of maze
				PaintBitmap(pDC, pGamePalette, pStartBitmap,                       //...draw start bitmap
					mazeTile[x][y].m_nStart.x, mazeTile[x][y].m_nStart.y);
			else                                                                    // Otherwise, it's a
				PaintBitmap(pDC, pGamePalette, pWallBitmap,                        //...wall
					mazeTile[x][y].m_nStart.x, mazeTile[x][y].m_nStart.y);
		} // end for y
	} // end for x

	for (x = 0; x < NUM_COLUMNS; x++) {                                    // Go through the grid
		for (y = 0; y < NUM_ROWS; y++) {                                   //...and for every square
			AddEdges(pDC, x, y, 0, 0);                                    //...add trim if needed
		} // end for y
	} // end for x

} // End PaintMaze

void SetUpMaze() {
	int     x, y;
	CPoint  m_pExit;

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
	m_pExit.x = x;

	if (mazeTile[x + 1][y].m_nWall == WALL) {          // If a wall runs into the top exit space
		mazeTile[x][y].m_nWall = WALL;                  //...make it a wall and put the exit
		m_pExit.y = y + 1;                              //...one space above that
	} else {
		mazeTile[x][y + 1].m_nWall = WALL;              // Put the exit in the top space
		m_pExit.y = y;                                  //...and store the y position in m_pExit
	}

	mazeTile[m_pExit.x][m_pExit.y].m_nWall = EXIT;      // Make exit grid space a Pathway


	SetInvisibleWalls();                                // Hide some walls
	SetTraps();                                         // Put in some traps
}

void AddEdges(CDC *pDC, int x, int y, int offset_x, int offset_y) {
	if ((mazeTile[x][y].m_bHidden == false) && (mazeTile[x][y].m_nWall == WALL)) {

		if ((y > 0) && ((((mazeTile[x][y - 1].m_nWall == PATH) || (mazeTile[x][y - 1].m_nWall == EXIT)) ||
			(mazeTile[x][y - 1].m_nWall == START)) || mazeTile[x][y - 1].m_bHidden))      // TOP
			PaintBitmap(pDC, pGamePalette, pBottomEdgeBmp,
				mazeTile[x][y - 1].m_nStart.x + offset_x,
				mazeTile[x][y - 1].m_nStart.y + offset_y + SQ_SIZE_Y - 1 - EDGE_SIZE);

		if ((x < (NUM_COLUMNS - 1)) && ((mazeTile[x + 1][y].m_nWall == PATH) ||
			mazeTile[x + 1][y].m_bHidden))                       // RIGHT
			PaintBitmap(pDC, pGamePalette, pLeftEdgeBmp,
				mazeTile[x + 1][y].m_nStart.x + offset_x,
				mazeTile[x + 1][y].m_nStart.y + offset_y);

		if ((y < (NUM_ROWS - 1)) && ((((mazeTile[x][y + 1].m_nWall == EXIT) ||
			(mazeTile[x][y + 1].m_nWall == PATH)) ||
			(mazeTile[x][y + 1].m_nWall == START)) || mazeTile[x][y + 1].m_bHidden))      // BOTTOM
			PaintBitmap(pDC, pGamePalette, pTopEdgeBmp,
				mazeTile[x][y + 1].m_nStart.x + offset_x,
				mazeTile[x][y + 1].m_nStart.y + offset_y);

		if ((x > 0) && ((mazeTile[x - 1][y].m_nWall == PATH) ||
			mazeTile[x - 1][y].m_bHidden))                                   // LEFT
			PaintBitmap(pDC, pGamePalette, pRightEdgeBmp,
				mazeTile[x - 1][y].m_nStart.x + offset_x + SQ_SIZE_X - 1 - EDGE_SIZE,
				mazeTile[x - 1][y].m_nStart.y + offset_y);
	} // end if WALL
}

void SetTraps() {
	int nTrapCount;
	int nNumTraps;
	CPoint In;

	nNumTraps = MIN_TRAPS + (m_nDifficulty / 2);          // 4 + ([1...10]/2) = 4 to 9

	for (nTrapCount = 0; nTrapCount < nNumTraps; nTrapCount++) {
		In = GetRandomPoint(false);                                       // Pick a random PATH square
		mazeTile[In.x][In.y].m_nWall = TRAP;                                // Make it a TRAP
		mazeTile[In.x][In.y].m_bHidden = true;                              // Hide it
		mazeTile[In.x][In.y].m_nTrap = nTrapCount % NUM_TRAP_MAPS;          // Assign unique trap bitmap ID
		mazeTile[In.x][In.y].m_nDest = GetRandomPoint(true);              // Pick a random Trap destination
	}
}

CPoint GetRandomPoint(bool bRight) {
	CPoint point;
	bool bLocated = false;

	if (bRight)                                                           // Get random column
		point.x = (brand() % (2 * (NUM_COLUMNS / 3))) + (NUM_COLUMNS / 3);         //...in the right half
	else
		point.x = brand() % (2 * (NUM_COLUMNS / 3));                             //...or the left half
	point.y = brand() % NUM_ROWS;                                            // Get random row

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

void SetInvisibleWalls() {
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
			} // end if
		} // end for y
	} // end for x

	if (m_nDifficulty > MIN_DIFFICULTY && m_nDifficulty < MAX_DIFFICULTY) {
		x = (brand() % (NUM_COLUMNS - 4)) + 2;                       // Avoid the edge walls
		y = (brand() % (NUM_ROWS - 4)) + 2;
		nMaxWalls = nTotalWalls - (int)(m_nDifficulty * (nTotalWalls / 10));

		while (nWallCount < nMaxWalls) {
			if (mazeTile[x][y].m_nWall == WALL && mazeTile[x][y].m_bHidden) {
				for (i = x - 1; i <= x + 1; i++) {
					for (j = y - 1; j <= y + 1; j++) {
						if (mazeTile[i][j].m_nWall == WALL && mazeTile[i][j].m_bHidden) {
							mazeTile[i][j].m_bHidden = false;       // so it's not hidden
							nWallCount++;                           // increment the count
						} // end if
					} // end j
				} // end i
			} // end if
			x += (brand() % NUM_NEIGHBORS);// + 1;                                   // Increment Column
			y += (brand() % NUM_NEIGHBORS);// + 1;                                   // Increment Row
			if (x >= (NUM_COLUMNS - 2))
				x = (brand() % (NUM_COLUMNS - 4)) + 2;               // If we're at the end,
			if (y >= (NUM_ROWS - 2))
				y = (brand() % (NUM_COLUMNS - 4)) + 2;               //...reset the counter
		}
	}
}

void initialize_maze() {
	int i, j, wall;

	maze_size_x = MAX_MAZE_SIZE_X;
	maze_size_y = MAX_MAZE_SIZE_Y;

	/* initialize all squares */
	for (i = 0; i < maze_size_x; i++) {
		for (j = 0; j < maze_size_y; j++) {
			maze[i][j] = 0;
		}
	}

	/* top wall */
	for (i = 0; i < maze_size_x; i++) {
		maze[i][0] |= WALL_TOP;
	}

	/* right wall */
	for (j = 0; j < maze_size_y; j++) {
		maze[maze_size_x - 1][j] |= WALL_RIGHT;
	}

	/* bottom wall */
	for (i = 0; i < maze_size_x; i++) {
		maze[i][maze_size_y - 1] |= WALL_BOTTOM;
	}

	/* left wall */
	for (j = 0; j < maze_size_y; j++) {
		maze[0][j] |= WALL_LEFT;
	}

	/* set start square */
	wall = 1;                           // Start on right side
	i = maze_size_x - 1;                // Set maze x location
	j = brand() % maze_size_y;       // Set a random y location not on the top row

	maze[i][j] |= START_SQUARE;
	maze[i][j] |= (DOOR_IN_TOP >> wall);
	maze[i][j] &= ~(WALL_TOP >> wall);
	start_x = i;
	start_y = j;
	cur_sq_x = i;
	cur_sq_y = j;
	sqnum = 0;

	/* set end square */
	wall = (wall + 2) % 4;
	switch (wall) {
	case 0:
		i = brand() % (maze_size_x);
		j = 0;
		break;
	case 1:
		i = maze_size_x - 1;
		j = brand() % (maze_size_y);
		break;
	case 2:
		i = brand() % (maze_size_x);
		j = maze_size_y - 1;
		break;
	case 3:
		i = 0;
		j = brand() % (maze_size_y);
		break;
	}
	maze[i][j] |= END_SQUARE;
	maze[i][j] |= (DOOR_OUT_TOP >> wall);
	maze[i][j] &= ~(WALL_TOP >> wall);
	end_x = i;
	end_y = j;
}

void create_maze() {
	int newdoor = 0;

	do {
		move_list[sqnum].x = cur_sq_x;
		move_list[sqnum].y = cur_sq_y;
		move_list[sqnum].dir = newdoor;
		while ((newdoor = choose_door()) == -1) { /* pick a door */
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

int choose_door() {
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

	return candidates[brand() % (num_candidates)];
}

int backup() {
	sqnum--;
	cur_sq_x = move_list[sqnum].x;
	cur_sq_y = move_list[sqnum].y;
	return sqnum;
}

CPoint ScreenToTile(CPoint pointScreen) {
	CPoint point;

	point.x = (pointScreen.x - SIDE_BORDER) / SQ_SIZE_X;
	point.y = (pointScreen.y - TOP_BORDER + SQ_SIZE_Y / 2) / SQ_SIZE_Y;

	return(point);
}

bool InArtRegion(CPoint point) {
	if ((point.x > SIDE_BORDER && point.x < GAME_WIDTH - SIDE_BORDER) &&     // See if point lies within
		(point.y > TOP_BORDER && point.y < GAME_HEIGHT - BOTTOM_BORDER))     //...ArtWork area
		return true;                                                            // Return true if it's inside
	else return false;                                                          //...and false if not
}

void GetSubOptions(CWnd *pParentWind) {
	COptnDlg OptionsDlg(pParentWind, pGamePalette);   // Call Specific Game
	//...Options dialog box
	OptionsDlg.m_nTime = m_nTime;
	OptionsDlg.nSeconds = nSeconds;             // Send clock info
	OptionsDlg.nMinutes = nMinutes;
	OptionsDlg.m_nDifficulty = m_nDifficulty;

	if (OptionsDlg.DoModal() == IDOK) {           // save values set in dialog box
		tempTime = OptionsDlg.m_nTime;;             // get new time limit,
		tempDifficulty = OptionsDlg.m_nDifficulty;  //...new Difficulty
	}
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
