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

#ifndef HODJNPODJ_MAZEDOOM_MAZE_DOOM_H
#define HODJNPODJ_MAZEDOOM_MAZE_DOOM_H

#include "bagel/bagel.h"
#include "bagel/hodjnpodj/libs/types.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gfx/sprite.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

// Border info              
#define SIDE_BORDER 	 20
#define TOP_BORDER		 28	 
#define BOTTOM_BORDER	 16	 
#define HILITE_BORDER	  3

// Dimensions constants
#define ART_WIDTH	600
#define ART_HEIGHT	432

#define SQ_SIZE_X	24
#define SQ_SIZE_Y   24 
#define EDGE_SIZE	 5

// Starting value defaults
#define MAX_DIFFICULTY	10	//8
#define MIN_DIFFICULTY	 1	//0 

// Timer constants         
#define MIN_TIME			 15						//  15 Seconds
#define MAX_TIME			180						// 180 Seconds = 3 minute max
#define TIMER_MIN			  0						// Increment scrollbar in steps of 15 Secs
#define TIMER_MAX			 10						// if Time > MAX_TIME, No Time Limit

#define NUM_COLUMNS		25
#define NUM_ROWS		19
#define NUM_NEIGHBORS	 9							// The "clump" area is 3 X 3 grid spaces

#define	NUM_TRAP_MAPS	 7							// There are seven trap icons available
#define	MIN_TRAPS		 4

#define NUM_CELS		 8

#define PATH	0
#define WALL	1
#define TRAP	2
#define START	3
#define EXIT	4

#define HODJ	0
#define PODJ	4

// Timer stuff
#define GAME_TIMER 		1
#define CLICK_TIME	 1000		// Every Second, update timer clock 

// Rules files
#define	RULES_TEXT		"MAZEOD.TXT"
#define	RULES_WAV		".\\SOUND\\MAZEOD.WAV"

// Sound files                          
#define WIN_SOUND	".\\sound\\fanfare2.wav"
#define LOSE_SOUND	".\\sound\\buzzer.wav"
#define HIT_SOUND	".\\sound\\thud.wav"
#define TRAP_SOUND	".\\sound\\boing.wav"

#define GAME_THEME	".\\sound\\mazeod.mid"

// Backdrop bitmaps
#define MAINSCREEN	".\\ART\\DOOM2.BMP"

// New Game button area
#define	NEWGAME_LOCATION_X	 15
#define	NEWGAME_LOCATION_Y	  0
#define	NEWGAME_WIDTH		217
#define NEWGAME_HEIGHT		 20

// Time Display area
#define	TIME_LOCATION_X		420
#define	TIME_LOCATION_Y		  4
#define	TIME_WIDTH			195
#define TIME_HEIGHT			 15

extern void run();

/**
 * Draw the surrounding wall and start/end squares
 */
extern void initialize_maze();

/**
 * Create a maze layout given the intiialized maze 
 */
extern void create_maze();

/**
 * Pick a new path
 */
extern int choose_door();

/**
 * Back up a move
 */
extern int backup();

/**
 * Translates the random maze generated into the mazeTile grid for the game
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      maze[][]            The randomly generated maze
 *      struct TILE mazeTile[][] grid
 *      start_y
 *      exit_y
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
 */
extern void SetUpMaze();

/*****************************************************************
 * Randomly sets a number of walls invisible
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
 *      m_nDifficulty       If it's MIN_DIFFICULTY, no walls are invisible
 *                          If it's MAX_DIFFICULTY, all walls are invisible
 *                          Otherwise, every m_nDifficulty-th wall is visible
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
**/
extern void SetInvisibleWalls();

/*****************************************************************
 * Sets traps in the maze
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] array
 *      m_nDifficulty       The the number of traps = difficulty setting
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] array
**/
extern void SetTraps();

/*****************************************************************
 * Draws fancy edgework around a piece of wall on the sides
 * where possible
 *
 *  FORMAL PARAMETERS:
 *
 *      CDC     *pDC        The Device context to which it will draw
 *      int     x           Column of piece to check
 *      int     y           Row of piece to check
 *      int     offset_x    Width offset for drawing the bitmap ( Main window has a border)
 *      int     offset_y    Hieght offset as above
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][]
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      [External data modified]
**/
extern void AddEdges(CDC *pDC, int x, int y, int offset_x, int offset_y);

extern void PaintMaze(CDC *pDC);

/*****************************************************************
 * Gets a random Grid Point in the maze, which is a PATH (not START or EXIT)
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      struct TILE mazeTile[][] grid
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      CPoint      the random X and Y of a path space in the mazeTile grid
**/
extern CPoint GetRandomPoint(bool bRight);

/*****************************************************************
 * Converts a point in screen coordinates to x & y location on maze grid
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint pointScreen      a point in screen coordinates
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      CPoint  point       the x & y grid coordinates where pointScreen fell
**/
extern CPoint ScreenToTile(CPoint pointScreen);

/*****************************************************************
 * Checks to see if a point is within the Artwork region of the window
 *
 *  FORMAL PARAMETERS:
 *
 *      CPoint point    The point to check
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      Extents of the main game window, and the extents of the artwork
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      bool:   true if point is within the Art Region,
 *              false if point is outside the Art Region
**/
extern bool InArtRegion(CPoint point);


extern CBmpButton *m_pScrollButton;
extern CSprite *pPlayerSprite;
extern CPalette *pGamePalette,                   // Palette of current artwork 
	*pOldPal;
extern CBitmap *pMazeBitmap,
	*pOldBmp,
	*pWallBitmap,
	*pPathBitmap,
	*pStartBitmap,
	*pTopEdgeBmp,
	*pRightEdgeBmp,
	*pBottomEdgeBmp,
	*pLeftEdgeBmp,
	*TrapBitmap[NUM_TRAP_MAPS];
extern CDC *pMazeDC;                    // DC for the MazeBitmap
extern CText *m_pTimeText;              // Time to be posted in Locale box of screen
extern CBitmap *pLocaleBitmap,          // Locale of game bitmap for title bar
	*pBlankBitmap;               // Blank area of locale for time display

extern bool bSuccess;
extern bool m_bIgnoreScrollClick;
extern bool bPlaying;
extern bool m_bGameOver;
extern POINT m_PlayerPos;
extern uint m_nPlayerID;         // Hodj = 0, Podj = 4 to Offset the Bitmap ID for player

extern int m_nDifficulty;
extern int m_nTime, nSeconds, nMinutes;
extern int tempDifficulty;
extern int tempTime;

extern LPGAMESTRUCT pGameInfo;

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
