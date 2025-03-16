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

#ifndef HODJNPODJ_MAZEDOOM_GLOBALS_H
#define HODJNPODJ_MAZEDOOM_GLOBALS_H

#include "common/scummsys.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define MAIN_SCREEN		"mazedoom/art/doom2.bmp"

#define	WALL_X		0
#define	WALL_Y		22
#define	PATH_WIDTH	24
#define	PATH_HEIGHT	24
#define	PATH_X		48
#define	START_X		24
#define	EDGE_Y		46
#define	EDGE_WIDTH	5
#define	EDGE_HEIGHT	24
#define	TRAP_WIDTH	22
#define	TRAP_HEIGHT	22

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

#define NUM_NEIGHBORS	 9							// The "clump" area is 3 X 3 grid spaces

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

// Scroll button size and positioning information
#define IDC_SCROLL			850

#define SCROLL_BUTTON_X		250
#define SCROLL_BUTTON_Y		0
#define SCROLL_BUTTON_DX	140
#define SCROLL_BUTTON_DY	23

#define SCROLLUP_BMP	"art/scrollup.bmp"
#define SCROLLDOWN_BMP	"art/scrolldn.bmp"

#define IDB_HODJ_UP_BMP		"mazedoom/art/hodjup.bmp"
#define IDB_HODJ_RIGHT_BMP	"mazedoom/art/hodjrt.bmp"
#define IDB_HODJ_DOWN_BMP	"mazedoom/art/hodjdown.bmp"
#define IDB_HODJ_LEFT_BMP	"mazedoom/art/hodjlf.bmp"
#define IDB_PODJ_UP_BMP		"mazedoom/art/podjup.bmp"
#define IDB_PODJ_RIGHT_BMP	"mazedoom/art/podjrt.bmp"
#define IDB_PODJ_DOWN_BMP	"mazedoom/art/podjdown.bmp"
#define IDB_PODJ_LEFT_BMP	"mazedoom/art/podjlf.bmp"
#define IDB_HODJ_UP						330
#define IDB_HODJ_RIGHT					331
#define IDB_HODJ_DOWN					332
#define IDB_HODJ_LEFT					333
#define IDB_PODJ_UP						334
#define IDB_PODJ_RIGHT					335
#define IDB_PODJ_DOWN					336
#define IDB_PODJ_LEFT					337


#define IDB_PARTS_BMP       "mazedoom/art/modparts.bmp"
#define IDB_LOCALE_BMP		"mazedoom/art/locale.bmp"
#define IDB_BLANK_BMP		"mazedoom/art/blank.bmp"
#define	IDB_PARTS 			301

#define IDC_MOD_DNARROW                 116
#define IDC_MOD_UPARROW                 117
#define IDC_MOD_LFARROW                 118
#define IDC_MOD_RTARROW                 119
#define	IDC_MOD_NOARROW					120

#define IDD_SUBOPTIONS                  115
#define IDC_RULES						1002

#define IDC_DIFFICULTY                  1021
#define IDC_TIMELIMIT                   1022

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
