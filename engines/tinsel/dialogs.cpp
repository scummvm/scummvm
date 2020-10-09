/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Handles the inventory and conversation windows.
 *
 * And the save/load game windows. Some of this will be platform
 * specific - I'll try to separate this ASAP.
 *
 * And there's still a bit of tidying and commenting to do yet.
 */

#include "common/serializer.h"
#include "tinsel/dialogs.h"
#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/drives.h"
#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"
#include "tinsel/multiobj.h"
#include "tinsel/music.h"
#include "tinsel/palette.h"
#include "tinsel/pcode.h"
#include "tinsel/pdisplay.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/savescn.h"
#include "tinsel/sched.h"
#include "tinsel/scn.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "tinsel/sysvar.h"
#include "tinsel/text.h"
#include "tinsel/timers.h" // For ONE_SECOND constant
#include "tinsel/tinlib.h"
#include "tinsel/tinsel.h" // For engine access
#include "tinsel/token.h"

#include "common/textconsole.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

#define HOPPER_FILENAME "hopper"

#define INV_PICKUP PLR_SLEFT // Local names
#define INV_LOOK PLR_SRIGHT  //	for button events
#define INV_ACTION PLR_DLEFT //

/** attribute values - may become bit field if further attributes are added */
enum {
	IO_ONLYINV1 = 0x01,
	IO_ONLYINV2 = 0x02,
	IO_DROPCODE = 0x04
};

//-----------------------
// Moveable window translucent rectangle position limits
enum {
	MAXLEFT = 315, //
	MINRIGHT = 3,  // These values keep 2 pixcells
	MINTOP = -13,  // of header on the screen.
	MAXTOP = 195   //
};

//-----------------------
// Indices into hWinParts's reels

enum PARTS_INDEX {
	IX_SLIDE = 0, // Slider
	IX_V26 = 1,
	IX_V52 = 2,
	IX_V78 = 3,
	IX_V104 = 4,
	IX_V130 = 5,
	IX_H26 = 6,
	IX_H52 = 7,
	IX_H78 = 8,
	IX_H104 = 9,
	IX_H130 = 10,
	IX_H156 = 11,
	IX_H182 = 12,
	IX_H208 = 13,
	IX_H234 = 14,
	IX_TL = 15, // Top left corner
	IX_TR = 16, // Top right corner
	IX_BL = 17, // Bottom left corner
	IX_BR = 18, // Bottom right corner

	IX1_H25 = 19,
	IX1_V11 = 20,
	IX1_RTL = 21,      // Re-sizing top left corner
	IX1_RTR = 22,      // Re-sizing top right corner
	IX1_RBR = 23,      // Re-sizing bottom right corner
	IX1_CURLR = 24,    // }
	IX1_CURUD = 25,    // }
	IX1_CURDU = 26,    // } Custom cursors
	IX1_CURDD = 27,    // }
	IX1_CURUP = 28,    // }
	IX1_CURDOWN = 29,  // }
	IX1_MDGROOVE = 30, // 'Mixing desk' slider background
	IX1_MDSLIDER = 34, // 'Mixing desk' slider
	IX1_BLANK1 = 35,   //
	IX1_BLANK2 = 36,   //
	IX1_BLANK3 = 37,   //
	IX1_CIRCLE1 = 38,  //
	IX1_CIRCLE2 = 39,  //
	IX1_CROSS1 = 40,   //
	IX1_CROSS2 = 41,   //
	IX1_CROSS3 = 42,   //
	IX1_QUIT1 = 43,    //
	IX1_QUIT2 = 44,    //
	IX1_QUIT3 = 45,    //
	IX1_TICK1 = 46,    //
	IX1_TICK2 = 47,    //
	IX1_TICK3 = 48,    //
	IX1_NTR = 49,      // New top right corner

	IX2_RTL = 19,      // Re-sizing top left corner
	IX2_RTR = 20,      // Re-sizing top right corner
	IX2_RBR = 21,      // Re-sizing bottom right corner
	IX2_CURLR = 22,    // }
	IX2_CURUD = 23,    // }
	IX2_CURDU = 24,    // } Custom cursors
	IX2_CURDD = 25,    // }
	IX2_MDGROOVE = 26, // 'Mixing desk' slider background
	IX2_MDSLIDER = 27, // 'Mixing desk' slider
	IX2_CIRCLE1 = 28,  //
	IX2_CIRCLE2 = 29,  //
	IX2_CROSS1 = 30,   //
	IX2_CROSS2 = 31,   //
	IX2_CROSS3 = 32,   //
	IX2_TICK1 = 33,    //
	IX2_TICK2 = 34,    //
	IX2_TICK3 = 35,    //
	IX2_NTR = 36,      // New top right corner
	IX2_TR4 = 37,
	IX2_LEFT1 = 38,
	IX2_LEFT2 = 39,
	IX2_RIGHT1 = 40,
	IX2_RIGHT2 = 41,

	T1_HOPEDFORREELS = 50,
	T2_HOPEDFORREELS = 42
};

// The following defines select the correct constant depending on Tinsel version
#define IX_CROSS1 (TinselV2 ? IX2_CROSS1 : IX1_CROSS1)
#define IX_CURDD (TinselV2 ? IX2_CURDD : IX1_CURDD)
#define IX_CURDU (TinselV2 ? IX2_CURDU : IX1_CURDU)
#define IX_CURLR (TinselV2 ? IX2_CURLR : IX1_CURLR)
#define IX_CURUD (TinselV2 ? IX2_CURUD : IX1_CURUD)
#define IX_CURUL (TinselV2 ? IX2_CURUL : IX1_CURUL)
#define IX_MDGROOVE (TinselV2 ? IX2_MDGROOVE : IX1_MDGROOVE)
#define IX_MDSLIDER (TinselV2 ? IX2_MDSLIDER : IX1_MDSLIDER)
#define IX_NTR (TinselV2 ? IX2_NTR : IX1_NTR)
#define IX_RBR (TinselV2 ? IX2_RBR : IX1_RBR)
#define IX_RTL (TinselV2 ? IX2_RTL : IX1_RTL)
#define IX_RTR (TinselV2 ? IX2_RTR : IX1_RTR)
#define IX_TICK1 (TinselV2 ? IX2_TICK1 : IX1_TICK1)

#define NORMGRAPH 0
#define DOWNGRAPH 1
#define HIGRAPH 2
//-----------------------
#define FIX_UK 0
#define FIX_FR 1
#define FIX_GR 2
#define FIX_IT 3
#define FIX_SP 4
#define FIX_USA 5
#define HOPEDFORFREELS 6 // Expected flag reels
//-----------------------

#define MAX_ININV (TinselV2 ? 160 : 150) // Max in an inventory

#define ITEM_WIDTH (TinselV2 ? 50 : 25)  // Dimensions of an icon
#define ITEM_HEIGHT (TinselV2 ? 50 : 25) //
#define I_SEPARATION (TinselV2 ? 2 : 1)  // Item separation

#define NM_TOFF 11                // Title text Y offset from top
#define NM_TBT (TinselV2 ? 4 : 0) // Y, title box top
#define NM_TBB 33
#define NM_LSX (TinselV2 ? 4 : 0) // X, left side
#define NM_BSY (TinselV2 ? -9 : -M_TH + 1)
#define NM_RSX (TinselV2 ? -9 : -M_SW + 1)
#define NM_SBL (-27)
#define NM_SLH (TinselV2 ? 11 : 5) // Slider height
#define NM_SLX (-11)               // Slider X offset (from right)

#define NM_BG_POS_X (TinselV2 ? 9 : 1)    // }
#define NM_BG_POS_Y (TinselV2 ? 9 : 1)    // } Offset of translucent rectangle
#define NM_BG_SIZ_X (TinselV2 ? -18 : -3) // }
#define NM_BG_SIZ_Y (TinselV2 ? -18 : -3) // } How much larger it is than edges

#define NM_RS_T_INSET 3
#define NM_RS_B_INSET 4
#define NM_RS_L_INSET 3
#define NM_RS_R_INSET 4
#define NM_RS_THICKNESS 5
#define NM_MOVE_AREA_B_Y 30
#define NM_SLIDE_INSET (TinselV2 ? 18 : 9)     // X offset (from right) of left of scroll region
#define NM_SLIDE_THICKNESS (TinselV2 ? 13 : 7) // thickness of scroll region
#define NM_UP_ARROW_TOP 34                     // Y offset of top of up arrow
#define NM_UP_ARROW_BOTTOM 49                  // Y offset of bottom of up arrow
#define NM_DN_ARROW_TOP 22                     // Y offset (from bottom) of top of down arrow
#define NM_DN_ARROW_BOTTOM 5                   // Y offset (from bottom) of bottom of down arrow

#define MD_YBUTTOP (TinselV2 ? 2 : 9)
#define MD_YBUTBOT (TinselV2 ? 16 : 0)
#define MD_XLBUTL (TinselV2 ? 4 : 1)
#define MD_XLBUTR (TinselV2 ? 26 : 10)
#define MD_XRBUTL (TinselV2 ? 173 : 105)
#define MD_XRBUTR (TinselV2 ? 195 : 114)
#define ROTX1 60 // Rotate button's offsets from the center

#define MAX_NAME_RIGHT (TinselV2 ? 417 : 213)

#define SLIDE_RANGE (TinselV2 ? 120 : 81)
#define SLIDE_MINX (TinselV2 ? 25 : 8)
#define SLIDE_MAXX (TinselV2 ? 25 + 120 : 8 + 81)

#define MDTEXT_YOFF (TinselV2 ? -1 : 6)
#define MDTEXT_XOFF -4
#define TOG2_YOFF -22
#define ROT_YOFF 48
#define TYOFF (TinselV2 ? 4 : 0)
#define FLAGX (-5)
#define FLAGY 4

//----------------- LOCAL GLOBAL DATA --------------------

//----- Permanent data (compiled in) -----

// Save game name editing cursor

#define CURSOR_CHAR '_'
char sCursor[2] = {CURSOR_CHAR, 0};
static const int hFillers[MAXHICONS] = {
    IX_H26,  // 2 icons wide
    IX_H52,  // 3
    IX_H78,  // 4
    IX_H104, // 5
    IX_H130, // 6
    IX_H156, // 7
    IX_H182, // 8
    IX_H208, // 9
    IX_H234  // 10 icons wide
};
static const int vFillers[MAXVICONS] = {
    IX_V26,  // 2 icons high
    IX_V52,  // 3
    IX_V78,  // 4
    IX_V104, // 5
    IX_V130  // 6 icons high
};

//----- Data pertinant to configure (incl. load/save game) -----

#define COL_MAINBOX TBLUE1 // Base blue color
#define COL_BOX TBLUE1
#define COL_HILIGHT TBLUE4

#ifdef JAPAN
#define BOX_HEIGHT 17
#define EDIT_BOX1_WIDTH 149
#else
#define BOX_HEIGHT 13
#define EDIT_BOX1_WIDTH 145
#endif
#define EDIT_BOX2_WIDTH 166

#define T2_EDIT_BOX1_WIDTH 290
#define T2_EDIT_BOX2_WIDTH 322
#define T2_BOX_HEIGHT 26

#define NO_HEADING ((SCNHANDLE)-1)
#define USE_POINTER (-1)
#define SIX_LOAD_OPTION 0
#define SIX_SAVE_OPTION 1
#define SIX_RESTART_OPTION 2
#define SIX_SOUND_OPTION 3
#define SIX_CONTROL_OPTION 4
#ifndef JAPAN
#define SIX_SUBTITLES_OPTION 5
#endif
#define SIX_QUIT_OPTION 6
#define SIX_RESUME_OPTION 7
#define SIX_LOAD_HEADING 8
#define SIX_SAVE_HEADING 9
#define SIX_RESTART_HEADING 10
#define SIX_MVOL_SLIDER 11
#define SIX_SVOL_SLIDER 12
#define SIX_VVOL_SLIDER 13
#define SIX_DCLICK_SLIDER 14
#define SIX_DCLICK_TEST 15
#define SIX_SWAP_TOGGLE 16
#define SIX_TSPEED_SLIDER 17
#define SIX_STITLE_TOGGLE 18
#define SIX_QUIT_HEADING 19

struct CONFINIT {
	int h;
	int v;
	int x;
	int y;
	bool bExtraWin;
	CONFBOX *Box;
	int NumBoxes;
	uint32 ixHeading;
};

#define BW 44 // Width of crosses and ticks etc. buttons
#define BH 41 // Height of crosses and ticks etc. buttons

/*-------------------------------------------------------------*\
| This is the main menu (that comes up when you hit F1 on a PC)	|
\*-------------------------------------------------------------*/

#ifdef JAPAN
#define FBY 11 // y-offset of first button
#define FBX 13 // x-offset of first button
#else
#define FBY 20 // y-offset of first button
#define FBX 15 // x-offset of first button
#endif

#define OPTX 33
#define OPTY 30
#define BOX_V_SEP 7

#define BOXX 56 // X-position of text boxes
#define BOXY 50 // Y-position of text boxes
#define T2_OPTX 33
#define T2_OPTY 36
#define T2_BOX_V_SEP 12
#define T2_BOX_V2_SEP 6

static CONFBOX t1OptionBox[] = {

    {AATBUT, OPENLOAD, TM_NONE, NULL, SIX_LOAD_OPTION, FBX, FBY, EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENSAVE, TM_NONE, NULL, SIX_SAVE_OPTION, FBX, FBY + (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENREST, TM_NONE, NULL, SIX_RESTART_OPTION, FBX, FBY + 2 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENSOUND, TM_NONE, NULL, SIX_SOUND_OPTION, FBX, FBY + 3 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENCONT, TM_NONE, NULL, SIX_CONTROL_OPTION, FBX, FBY + 4 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
#ifdef JAPAN
    // TODO: If in JAPAN mode, simply disable the subtitles button?
    {AATBUT, OPENQUIT, NULL, SIX_QUIT_OPTION, FBX, FBY + 5 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
    {AATBUT, CLOSEWIN, NULL, SIX_RESUME_OPTION, FBX, FBY + 6 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0}
#else
    {AATBUT, OPENSUBT, TM_NONE, NULL, SIX_SUBTITLES_OPTION, FBX, FBY + 5 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENQUIT, TM_NONE, NULL, SIX_QUIT_OPTION, FBX, FBY + 6 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0},
    {AATBUT, CLOSEWIN, TM_NONE, NULL, SIX_RESUME_OPTION, FBX, FBY + 7 * (BOX_HEIGHT + 2), EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0}
#endif

};

static CONFINIT t1ciOption = {6, 5, 72, 23, false, t1OptionBox, ARRAYSIZE(t1OptionBox), NO_HEADING};

static CONFBOX t2OptionBox[] = {

    {AATBUT, OPENLOAD, TM_INDEX, NULL, SS_LOAD_OPTION, T2_OPTX, T2_OPTY, T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENSAVE, TM_INDEX, NULL, SS_SAVE_OPTION, T2_OPTX, T2_OPTY + (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENREST, TM_INDEX, NULL, SS_RESTART_OPTION, T2_OPTX, T2_OPTY + 2 * (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENSOUND, TM_INDEX, NULL, SS_SOUND_OPTION, T2_OPTX, T2_OPTY + 3 * (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {AATBUT, OPENQUIT, TM_INDEX, NULL, SS_QUIT_OPTION, T2_OPTX, T2_OPTY + 4 * (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0}

};

static CONFINIT t2ciOption = {6, 4, 144, 60, false, t2OptionBox, sizeof(t2OptionBox) / sizeof(CONFBOX), NO_HEADING};

#define ciOption (TinselV2 ? t2ciOption : t1ciOption)
#define optionBox (TinselV2 ? t2OptionBox : t1OptionBox)

/*-------------------------------------------------------------*\
| These are the load and save game menus.			|
\*-------------------------------------------------------------*/

#define NUM_RGROUP_BOXES 9

#ifdef JAPAN
#define NUM_RGROUP_BOXES 7 // number of visible slots
#define SY 32              // y-position of first slot
#else
#define NUM_RGROUP_BOXES 9 // number of visible slots
#define SY 31              // y-position of first slot
#endif

static CONFBOX t1LoadBox[NUM_RGROUP_BOXES + 2] = {
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY, EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 2 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 3 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 4 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 5 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 6 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
#ifndef JAPAN
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 7 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 8 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
#endif
    {ARSGBUT, LOADGAME, TM_NONE, NULL, USE_POINTER, 230, 44, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, USE_POINTER, 230, 44 + 47, 23, 19, NULL, IX1_CROSS1}};

static CONFBOX t2LoadBox[] = {
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY, T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 2 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 3 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 4 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 5 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 6 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 7 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, LOADGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 8 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},

    {ARSGBUT, LOADGAME, TM_NONE, NULL, 0, 460, 100, BW, BH, NULL, IX2_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, 0, 460, 100 + 100, BW, BH, NULL, IX2_CROSS1}};

static CONFINIT t1ciLoad = {10, 6, 20, 16, true, t1LoadBox, ARRAYSIZE(t1LoadBox), SIX_LOAD_HEADING};
static CONFINIT t2ciLoad = {10, 6, 40, 16, true, t2LoadBox, sizeof(t2LoadBox) / sizeof(CONFBOX), SS_LOAD_HEADING};

static CONFBOX t1SaveBox[NUM_RGROUP_BOXES + 2] = {
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY, EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 2 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 3 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 4 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 5 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 6 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
#ifndef JAPAN
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 7 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_NONE, NULL, USE_POINTER, 28, SY + 8 * (BOX_HEIGHT + 2), EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0},
#endif
    {ARSGBUT, SAVEGAME, TM_NONE, NULL, USE_POINTER, 230, 44, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, USE_POINTER, 230, 44 + 47, 23, 19, NULL, IX1_CROSS1}};

static CONFBOX t2SaveBox[] = {
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY, T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 2 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 3 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 4 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 5 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 6 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 7 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, SAVEGAME, TM_POINTER, NULL, 0, BOXX, BOXY + 8 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},

    {ARSGBUT, SAVEGAME, TM_NONE, NULL, 0, 460, 100, BW, BH, NULL, IX2_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, 0, 460, 100 + 100, BW, BH, NULL, IX2_CROSS1}};

static CONFINIT t1ciSave = {10, 6, 20, 16, true, t1SaveBox, ARRAYSIZE(t1SaveBox), SIX_SAVE_HEADING};
static CONFINIT t2ciSave = {10, 6, 40, 16, true, t2SaveBox, sizeof(t2SaveBox) / sizeof(CONFBOX), SS_SAVE_HEADING};

#define ciLoad (TinselV2 ? t2ciLoad : t1ciLoad)
#define loadBox (TinselV2 ? t2LoadBox : t1LoadBox)
#define ciSave (TinselV2 ? t2ciSave : t1ciSave)
#define saveBox (TinselV2 ? t2SaveBox : t1SaveBox)

/*-------------------------------------------------------------*\
| This is the restart confirmation 'menu'.			|
\*-------------------------------------------------------------*/

static CONFBOX t1RestartBox[] = {
#ifdef JAPAN
    {AAGBUT, INITGAME, TM_NONE, NULL, USE_POINTER, 96, 44, 23, 19, NULL, IX_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, USE_POINTER, 56, 44, 23, 19, NULL, IX_CROSS1}
#else
    {AAGBUT, INITGAME, TM_NONE, NULL, USE_POINTER, 70, 28, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, USE_POINTER, 30, 28, 23, 19, NULL, IX1_CROSS1}
#endif
};

static CONFBOX t1RestartBoxPSX[] = {
    {AAGBUT, INITGAME, TM_NONE, NULL, USE_POINTER, 122, 48, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, USE_POINTER, 82, 48, 23, 19, NULL, IX1_CROSS1}};

static CONFBOX t2RestartBox[] = {
    {AAGBUT, INITGAME, TM_NONE, NULL, 0, 140, 78, BW, BH, NULL, IX2_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, 0, 60, 78, BW, BH, NULL, IX2_CROSS1}};

#ifdef JAPAN
static CONFINIT t1ciRestart = {6, 2, 72, 53, false, t1RestartBox, ARRAYSIZE(t1RestartBox), SIX_RESTART_HEADING};
#else
static CONFINIT t1ciRestart = {4, 2, 98, 53, false, t1RestartBox, ARRAYSIZE(t1RestartBox), SIX_RESTART_HEADING};
#endif
static CONFINIT t1ciRestartPSX = {8, 2, 46, 53, false, t1RestartBoxPSX, ARRAYSIZE(t1RestartBoxPSX), SIX_RESTART_HEADING};
static CONFINIT t2ciRestart = {4, 2, 196, 53, false, t2RestartBox, sizeof(t2RestartBox) / sizeof(CONFBOX), SS_RESTART_HEADING};

#define ciRestart (TinselV2 ? t2ciRestart : (TinselV1PSX ? t1ciRestartPSX : t1ciRestart))

/*-------------------------------------------------------------*\
| This is the sound control 'menu'. In Discworld 2, it also		|
| contains the subtitles and language selection.				|
\*-------------------------------------------------------------*/

static CONFBOX t1SoundBox[] = {
    {SLIDER, MUSICVOL, TM_NONE, NULL, SIX_MVOL_SLIDER, 142, 25, Audio::Mixer::kMaxChannelVolume, 2, 0 /*&_vm->_config->_musicVolume*/, 0},
    {SLIDER, NOFUNC, TM_NONE, NULL, SIX_SVOL_SLIDER, 142, 25 + 40, Audio::Mixer::kMaxChannelVolume, 2, 0 /*&_vm->_config->_soundVolume*/, 0},
    {SLIDER, NOFUNC, TM_NONE, NULL, SIX_VVOL_SLIDER, 142, 25 + 2 * 40, Audio::Mixer::kMaxChannelVolume, 2, 0 /*&_vm->_config->_voiceVolume*/, 0}};

static CONFBOX t2SoundBox[] = {
    {SLIDER, MUSICVOL, TM_INDEX, NULL, SS_MVOL_SLIDER, 280, 50, Audio::Mixer::kMaxChannelVolume, 2, 0 /*&_vm->_config->_musicVolume*/, 0},
    {SLIDER, NOFUNC, TM_INDEX, NULL, SS_SVOL_SLIDER, 280, 50 + 30, Audio::Mixer::kMaxChannelVolume, 2, 0 /*&_vm->_config->_soundVolume*/, 0},
    {SLIDER, NOFUNC, TM_INDEX, NULL, SS_VVOL_SLIDER, 280, 50 + 2 * 30, Audio::Mixer::kMaxChannelVolume, 2, 0 /*&_vm->_config->_voiceVolume*/, 0},

    {SLIDER, NOFUNC, TM_INDEX, NULL, SS_TSPEED_SLIDER, 280, 160, 100, 2, 0 /*&_vm->_config->_textSpeed*/, 0},
    {TOGGLE2, NOFUNC, TM_INDEX, NULL, SS_STITLE_TOGGLE, 100, 220, BW, BH, 0 /*&_vm->_config->_useSubtitles*/, 0},
    {ROTATE, NOFUNC, TM_INDEX, NULL, SS_LANGUAGE_SELECT, 320, 220, BW, BH, NULL, 0}};

static CONFINIT t1ciSound = {10, 5, 20, 16, false, t1SoundBox, ARRAYSIZE(t1SoundBox), NO_HEADING};
static CONFINIT t2ciSound = {10, 5, 40, 16, false, t2SoundBox, sizeof(t2SoundBox) / sizeof(CONFBOX), SS_SOUND_HEADING};

#define ciSound (TinselV2 ? t2ciSound : t1ciSound)

/*-------------------------------------------------------------*\
| This is the (mouse) control 'menu'.				|
\*-------------------------------------------------------------*/

static int bFlipped = 0; // looks like this is just so the code has something to alter!

static CONFBOX controlBox[] = {
    {SLIDER, NOFUNC, TM_NONE, NULL, SIX_DCLICK_SLIDER, 142, 25, 3 * DOUBLE_CLICK_TIME, 1, 0 /*&_vm->_config->_dclickSpeed*/, 0},
    {FLIP, NOFUNC, TM_NONE, NULL, SIX_DCLICK_TEST, 142, 25 + 30, 23, 19, &bFlipped, IX1_CIRCLE1},
#ifdef JAPAN
    {TOGGLE, NOFUNC, TM_NONE, NULL, SIX_SWAP_TOGGLE, 205, 25 + 70, 23, 19, 0 /*&_vm->_config->_swapButtons*/, 0}
#else
    {TOGGLE, NOFUNC, TM_NONE, NULL, SIX_SWAP_TOGGLE, 155, 25 + 70, 23, 19, 0 /*&_vm->_config->_swapButtons*/, 0}
#endif
};

static CONFINIT ciControl = {10, 5, 20, 16, false, controlBox, ARRAYSIZE(controlBox), NO_HEADING};

/*-------------------------------------------------------------*\
| This is the subtitles 'menu'.					|
\*-------------------------------------------------------------*/

static CONFBOX subtitlesBox[] = {
    {SLIDER, NOFUNC, TM_NONE, NULL, SIX_TSPEED_SLIDER, 142, 20, 100, 2, 0 /*&_vm->_config->_textSpeed*/, 0},
    {TOGGLE, NOFUNC, TM_NONE, NULL, SIX_STITLE_TOGGLE, 142, 20 + 40, 23, 19, 0 /*&_vm->_config->_useSubtitles*/, 0},
};

static CONFBOX subtitlesBox3Flags[] = {
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 15, 118, 56, 32, NULL, FIX_FR},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 85, 118, 56, 32, NULL, FIX_GR},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 155, 118, 56, 32, NULL, FIX_SP},

    {SLIDER, NOFUNC, TM_NONE, NULL, SIX_TSPEED_SLIDER, 142, 20, 100, 2, 0 /*&_vm->_config->_textSpeed*/, 0},
    {TOGGLE, NOFUNC, TM_NONE, NULL, SIX_STITLE_TOGGLE, 142, 20 + 40, 23, 19, 0 /*&_vm->_config->_useSubtitles*/, 0},

    {ARSGBUT, CLANG, TM_NONE, NULL, USE_POINTER, 230, 110, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, RLANG, TM_NONE, NULL, USE_POINTER, 230, 140, 23, 19, NULL, IX1_CROSS1}};

static CONFBOX subtitlesBox4Flags[] = {
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 20, 100, 56, 32, NULL, FIX_FR},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 108, 100, 56, 32, NULL, FIX_GR},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 64, 137, 56, 32, NULL, FIX_IT},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 152, 137, 56, 32, NULL, FIX_SP},

    {SLIDER, NOFUNC, TM_NONE, NULL, SIX_TSPEED_SLIDER, 142, 20, 100, 2, 0 /*&_vm->_config->_textSpeed*/, 0},
    {TOGGLE, NOFUNC, TM_NONE, NULL, SIX_STITLE_TOGGLE, 142, 20 + 40, 23, 19, 0 /*&_vm->_config->_useSubtitles*/, 0},

    {ARSGBUT, CLANG, TM_NONE, NULL, USE_POINTER, 230, 110, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, RLANG, TM_NONE, NULL, USE_POINTER, 230, 140, 23, 19, NULL, IX1_CROSS1}};

static CONFBOX subtitlesBox5Flags[] = {
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 15, 100, 56, 32, NULL, FIX_UK},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 85, 100, 56, 32, NULL, FIX_FR},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 155, 100, 56, 32, NULL, FIX_GR},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 50, 137, 56, 32, NULL, FIX_IT},
    {FRGROUP, NOFUNC, TM_NONE, NULL, USE_POINTER, 120, 137, 56, 32, NULL, FIX_SP},

    {SLIDER, NOFUNC, TM_NONE, NULL, SIX_TSPEED_SLIDER, 142, 20, 100, 2, 0 /*&_vm->_config->_textSpeed*/, 0},
    {TOGGLE, NOFUNC, TM_NONE, NULL, SIX_STITLE_TOGGLE, 142, 20 + 40, 23, 19, 0 /*&_vm->_config->_useSubtitles*/, 0},

    {ARSGBUT, CLANG, TM_NONE, NULL, USE_POINTER, 230, 110, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, RLANG, TM_NONE, NULL, USE_POINTER, 230, 140, 23, 19, NULL, IX1_CROSS1}};

/*-------------------------------------------------------------*\
| This is the quit confirmation 'menu'.				|
\*-------------------------------------------------------------*/

static CONFBOX t1QuitBox[] = {
#ifdef JAPAN
    {AAGBUT, IQUITGAME, TM_NONE, NULL, USE_POINTER, 70, 44, 23, 19, NULL, IX_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, USE_POINTER, 30, 44, 23, 19, NULL, IX_CROSS1}
#else
    {AAGBUT, IQUITGAME, TM_NONE, NULL, USE_POINTER, 70, 28, 23, 19, NULL, IX1_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, USE_POINTER, 30, 28, 23, 19, NULL, IX1_CROSS1}
#endif
};

static CONFBOX t2QuitBox[] = {
    {AAGBUT, IQUITGAME, TM_NONE, NULL, 0, 140, 78, BW, BH, NULL, IX2_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, 0, 60, 78, BW, BH, NULL, IX2_CROSS1}};

static CONFINIT t1ciQuit = {4, 2, 98, 53, false, t1QuitBox, ARRAYSIZE(t1QuitBox), SIX_QUIT_HEADING};
static CONFINIT t2ciQuit = {4, 2, 196, 53, false, t2QuitBox, sizeof(t2QuitBox) / sizeof(CONFBOX), SS_QUIT_HEADING};

#define quitBox (TinselV2 ? t2QuitBox : t1QuitBox)
#define ciQuit (TinselV2 ? t2ciQuit : t1ciQuit)

/***************************************************************************\
|************************    Startup and shutdown    ***********************|
\***************************************************************************/

static CONFBOX hopperBox1[] = {
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY, T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 2 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 3 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 4 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 5 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 6 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 7 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, HOPPER2, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 8 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},

    {ARSGBUT, HOPPER2, TM_NONE, NULL, 0, 460, 100, BW, BH, NULL, IX2_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, 0, 460, 100 + 100, BW, BH, NULL, IX2_CROSS1}};

static CONFINIT ciHopper1 = {10, 6, 40, 16, true, hopperBox1, sizeof(hopperBox1) / sizeof(CONFBOX), SS_HOPPER1};

static CONFBOX hopperBox2[] = {
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY, T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 2 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 3 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 4 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 5 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 6 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 7 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
    {RGROUP, BF_CHANGESCENE, TM_STRINGNUM, NULL, 0, BOXX, BOXY + 8 * (T2_BOX_HEIGHT + T2_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},

    {ARSGBUT, BF_CHANGESCENE, TM_NONE, NULL, 0, 460, 50, BW, BH, NULL, IX2_TICK1},
    {AAGBUT, CLOSEWIN, TM_NONE, NULL, 0, 460, 200, BW, BH, NULL, IX2_CROSS1}};

static CONFINIT ciHopper2 = {10, 6, 40, 16, true, hopperBox2, sizeof(hopperBox2) / sizeof(CONFBOX), NO_HEADING};

/***************************************************************************\
|****************************    Top Window    *****************************|
\***************************************************************************/
static CONFBOX topwinBox[] = {
    {NOTHING, NOFUNC, TM_NONE, NULL, USE_POINTER, 0, 0, 0, 0, NULL, 0}};

static CONFINIT ciSubtitles = {10, 3, 20, 16, false, subtitlesBox, ARRAYSIZE(subtitlesBox), NO_HEADING};

static CONFINIT ciTopWin = {6, 5, 72, 23, false, topwinBox, 0, NO_HEADING};

#define NOBOX (-1)

// Conf window globals
static struct {
	CONFBOX *box;
	int NumBoxes;
	bool bExtraWin;
	uint32 ixHeading;
	bool editableRgroup;

	int selBox;
	int pointBox; // Box pointed to on last call
	int modifier;
	int extraBase;
	int numSaved;
} cd = {
    NULL, 0, false, 0, false,
    NOBOX, NOBOX, 0, 0, 0};

#define HL1 0 // Hilight that moves with the cursor
#define HL2 1 // Hilight on selected RGROUP box
#define HL3 2 // Text on selected RGROUP box
#define NUMHL 3

//----- LOCAL FORWARD REFERENCES -----

enum {
	IB_NONE = -1,      //
	IB_UP = -2,        // negative numbers returned
	IB_DOWN = -3,      // by WhichMenuBox()
	IB_SLIDE = -4,     //
	IB_SLIDE_UP = -5,  //
	IB_SLIDE_DOWN = -6 //
};

enum {
	HI_BIT = ((uint)MIN_INT >> 1), // The next to top bit
	IS_LEFT = HI_BIT,
	IS_SLIDER = (IS_LEFT >> 1),
	IS_RIGHT = (IS_SLIDER >> 1),
	IS_MASK = (IS_LEFT | IS_SLIDER | IS_RIGHT)
};

/*-------------------------------------------------------------------------*/
/***	Magic numbers	***/

#define M_SW 5 // Side width
#define M_TH 5 // Top height
#ifdef JAPAN
#define M_TOFF 6 // Title text Y offset from top
#define M_TBB 20 // Title box bottom Y offset
#else
#define M_TOFF 4 // Title text Y offset from top
#define M_TBB 14 // Title box bottom Y offset
#endif
#define M_SBL 26  // Scroll bar left X offset
#define M_SH 5    // Slider height (*)
#define M_SW 5    // Slider width (*)
#define M_SXOFF 9 // Slider X offset from right-hand side
#ifdef JAPAN
#define M_IUT 22 // Y offset of top of up arrow
#define M_IUB 30 // Y offset of bottom of up arrow
#else
#define M_IUT 16 // Y offset of top of up arrow
#define M_IUB 24 // Y offset of bottom of up arrow
#endif
#define M_IDT 10 // Y offset (from bottom) of top of down arrow
#define M_IDB 3  // Y offset (from bottom) of bottom of down arrow

#define START_ICONX (TinselV2 ? 12 : (M_SW + 1))         // } Relative offset of first icon
#define START_ICONY (TinselV2 ? 40 : (M_TBB + M_TH + 1)) // } within the inventory window

/*-------------------------------------------------------------------------*/

static void InvTinselEvent(INV_OBJECT *pinvo, TINSEL_EVENT event, PLR_EVENT be, int index);
static void InvPdProcess(CORO_PARAM, const void *param);

Dialogs::Dialogs() {
	_buttonEffect.bButAnim = false;
	_buttonEffect.box = nullptr;
	_buttonEffect.press = false;

	_hWinParts = 0;
	_flagFilm = 0;
	memset(_configStrings, 0, sizeof(_configStrings));

	_invObjects = nullptr;
	_numObjects = 0;
	_invFilms = nullptr;
	_noLanguage = false;

	memset(_objArray, 0, sizeof(_objArray));
	memset(_iconArray, 0, sizeof(_iconArray));
	memset(_dispObjArray, 0, sizeof(_dispObjArray));
	memset(_iconAnims, 0, sizeof(_iconAnims));
	memset(_permIcons, 0, sizeof(_permIcons));
	memset(_invD, 0, sizeof(_invD));

	_initialDirection = FORWARD;

	_heldItem = INV_NOICON;

	_heldFilm = 0;

	_numPermIcons = 0;
	_numEndIcons = 0;

	_activeInv = 0;

	_InventoryHidden = false;
	_InventoryMaximised = false;

	_SuppH = 0;
	_SuppV = 0;

	_yChange = 0;
	_yCompensate = 0;
	_xChange = 0;
	_xCompensate = 0;

	_ItemsChanged = 0;

	_reOpenMenu = 0;

	_TL = _TR = _BL = _BR = 0;
	_TLwidth = 0, _TLheight = 0;
	_TRwidth = 0;
	_BLheight = 0;

	_displayedLanguage = TXT_ENGLISH;

	_rectObject = nullptr;
	_slideObject = nullptr;

	_sliderYpos = 0;
	_sliderYmax = _sliderYmin = 0;

	memset(_mdSlides, 0, sizeof(_mdSlides));

	_numMdSlides = 0;
	_glitterIndex = 0;
	_thisIcon = 0;

	memset(&_thisConvFn, 0, sizeof(_thisConvFn));

	_thisConvPoly = 0;
	_thisConvActor = 0;
	_pointedIcon = INV_NOICON;
	_pointedWaitCount = 0;
	_sX = 0;
	_lX = 0;

	_bMoveOnUnHide = false;

	_pHopper = nullptr;
	_pEntries = nullptr;

	_numScenes = 0;

	_numEntries = 0;

	_pChosenScene = nullptr;

	_lastChosenScene = 0;
	_bRemember = false;

	_invCursor = IC_NORMAL;
	_inventoryState = IDLE_INV;
	_invDragging = ID_NONE;

	*_saveGameDesc = 0;
}

Dialogs::~Dialogs() {
	if (_objArray[0] != NULL) {
		DumpObjArray();
		DumpDobjArray();
		DumpIconArray();
	}
}

bool Dialogs::LanguageChange() {
	LANGUAGE nLang = _vm->_config->_language;

	if ((_vm->getFeatures() & GF_USE_3FLAGS) || (_vm->getFeatures() & GF_USE_4FLAGS) || (_vm->getFeatures() & GF_USE_5FLAGS)) {
		// Languages: TXT_ENGLISH, TXT_FRENCH, TXT_GERMAN, TXT_ITALIAN, TXT_SPANISH
		// 5 flag versions include English
		int selected = (_vm->getFeatures() & GF_USE_5FLAGS) ? cd.selBox : cd.selBox + 1;
		// Make sure that a language flag has been selected. If the user has
		// changed the language speed slider and hasn't clicked on a flag, it
		// won't be selected.
		if (selected >= 0 && selected <= 4) {
			nLang = (LANGUAGE)selected;

			// 3 flag versions don't include Italian
			if (selected >= 3 && (_vm->getFeatures() & GF_USE_3FLAGS))
				nLang = TXT_SPANISH;
		}
	}

	if (nLang != _vm->_config->_language) {
		KillInventory();
		ChangeLanguage(nLang);
		_vm->_config->_language = nLang;
		return true;
	} else
		return false;
}

/**************************************************************************/
/*****************************  Scene Hopper ******************************/
/**************************************************************************/

/**
 * Read in the scene hopper data file and set the
 *  pointers to the data and scene count.
 */
void Dialogs::PrimeSceneHopper() {
	Common::File f;
	char *pBuffer;
	uint32 vSize;

	// Open the file (it's on the CD)
	CdCD(Common::nullContext);
	if (!f.open(HOPPER_FILENAME))
		error(CANNOT_FIND_FILE, HOPPER_FILENAME);

	// Read in header
	if (f.readUint32LE() != CHUNK_SCENE_HOPPER)
		error(FILE_IS_CORRUPT, HOPPER_FILENAME);
	vSize = f.readUint32LE();

	// allocate a buffer for it all
	assert(_pHopper == NULL);
	uint32 size = f.size() - 8;

	// make sure memory allocated
	pBuffer = (char *)malloc(size);
	if (pBuffer == NULL)
		// cannot alloc buffer for index
		error(NO_MEM, "Scene hopper data");

	// load data
	if (f.read(pBuffer, size) != size)
		// file must be corrupt if we get to here
		error(FILE_IS_CORRUPT, HOPPER_FILENAME);

	// Set data pointers
	_pHopper = (PHOPPER)pBuffer;
	_pEntries = (PHOPENTRY)(pBuffer + vSize);
	_numScenes = vSize / sizeof(HOPPER);

	// close the file
	f.close();
}

/**
 * Free the scene hopper data file
 */
void Dialogs::FreeSceneHopper() {
	free(_pHopper);
	_pHopper = nullptr;
}

void Dialogs::FirstScene(int first) {
	int i;

	assert(_numScenes && _pHopper);

	if (_bRemember) {
		assert(first == 0);
		first = _lastChosenScene;
		_bRemember = false;
	}

	// Force it to a sensible value
	if (first > _numScenes - NUM_RGROUP_BOXES)
		first = _numScenes - NUM_RGROUP_BOXES;
	if (first < 0)
		first = 0;

	// Fill in the rest
	for (i = 0; i < NUM_RGROUP_BOXES && i + first < _numScenes; i++) {
		cd.box[i].textMethod = TM_STRINGNUM;
		cd.box[i].ixText = FROM_32(_pHopper[i + first].hSceneDesc);
	}
	// Blank out the spare ones (if any)
	while (i < NUM_RGROUP_BOXES) {
		cd.box[i].textMethod = TM_NONE;
		cd.box[i++].ixText = 0;
	}

	cd.extraBase = first;
}

void Dialogs::RememberChosenScene() {
	_bRemember = true;
}

void Dialogs::SetChosenScene() {
	_lastChosenScene = cd.selBox + cd.extraBase;
	_pChosenScene = &_pHopper[cd.selBox + cd.extraBase];
}

void Dialogs::FirstEntry(int first) {
	int i;

	_invD[INV_MENU].hInvTitle = FROM_32(_pChosenScene->hSceneDesc);

	// get number of entrances
	_numEntries = FROM_32(_pChosenScene->numEntries);

	// Force first to a sensible value
	if (first > _numEntries - NUM_RGROUP_BOXES)
		first = _numEntries - NUM_RGROUP_BOXES;
	if (first < 0)
		first = 0;

	for (i = 0; i < NUM_RGROUP_BOXES && i < _numEntries; i++) {
		cd.box[i].textMethod = TM_STRINGNUM;
		cd.box[i].ixText = FROM_32(_pEntries[FROM_32(_pChosenScene->entryIndex) + i + first].hDesc);
	}
	// Blank out the spare ones (if any)
	while (i < NUM_RGROUP_BOXES) {
		cd.box[i].textMethod = TM_NONE;
		cd.box[i++].ixText = 0;
	}

	cd.extraBase = first;
}

void Dialogs::HopAction() {
	PHOPENTRY pEntry = _pEntries + FROM_32(_pChosenScene->entryIndex) + cd.selBox + cd.extraBase;

	uint32 hScene = FROM_32(_pChosenScene->hScene);
	uint32 eNumber = FROM_32(pEntry->eNumber);
	debugC(DEBUG_BASIC, kTinselDebugAnimations, "Scene hopper chose scene %xh,%d\n", hScene, eNumber);

	if (FROM_32(pEntry->flags) & fCall) {
		SaveScene(Common::nullContext);
		NewScene(Common::nullContext, _pChosenScene->hScene, pEntry->eNumber, TRANS_FADE);
	} else if (FROM_32(pEntry->flags) & fHook)
		HookScene(hScene, eNumber, TRANS_FADE);
	else
		NewScene(Common::nullContext, hScene, eNumber, TRANS_CUT);
}

/**************************************************************************/
/******************** Some miscellaneous functions ************************/
/**************************************************************************/

/**
 * Delete all the objects in iconArray[]
 */
void Dialogs::DumpIconArray() {
	for (int i = 0; i < MAX_ICONS; i++) {
		if (_iconArray[i] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[i]);
			_iconArray[i] = nullptr;
		}
	}
}

/**
 * Delete all the objects in DobjArray[]
 */
void Dialogs::DumpDobjArray() {
	for (int i = 0; i < MAX_WCOMP; i++) {
		if (_dispObjArray[i] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _dispObjArray[i]);
			_dispObjArray[i] = nullptr;
		}
	}
}

/**
 * Delete all the objects in objArray[]
 */
void Dialogs::DumpObjArray() {
	for (int i = 0; i < MAX_WCOMP; i++) {
		if (_objArray[i] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _objArray[i]);
			_objArray[i] = nullptr;
		}
	}
}

/**
 * Convert item ID number to pointer to item's compiled data
 * i.e. Image data and Glitter code.
 */
INV_OBJECT *Dialogs::GetInvObject(int id) {
	INV_OBJECT *pObject = _invObjects;

	for (int i = 0; i < _numObjects; i++, pObject++) {
		if (pObject->id == id)
			return pObject;
	}

	error("GetInvObject(%d): Trying to manipulate undefined inventory icon", id);
}

/**
 * Returns true if the given id represents a valid inventory object
 */
bool Dialogs::GetIsInvObject(int id) {
	INV_OBJECT *pObject = _invObjects;

	for (int i = 0; i < _numObjects; i++, pObject++) {
		if (pObject->id == id)
			return true;
	}

	return false;
}

/**
 * Convert item ID number to index.
 */
int Dialogs::GetObjectIndex(int id) {
	INV_OBJECT *pObject = _invObjects;

	for (int i = 0; i < _numObjects; i++, pObject++) {
		if (pObject->id == id)
			return i;
	}

	error("GetObjectIndex(%d): Trying to manipulate undefined inventory icon", id);
}

/**
 * Returns position of an item in one of the inventories.
 * The actual position is not important for the uses that this is put to.
 */
int Dialogs::InventoryPos(int num) {
	int i;

	for (i = 0; i < _invD[INV_1].NoofItems; i++) // First inventory
		if (_invD[INV_1].contents[i] == num)
			return i;

	for (i = 0; i < _invD[INV_2].NoofItems; i++) // Second inventory
		if (_invD[INV_2].contents[i] == num)
			return i;

	if (_heldItem == num)
		return INV_HELDNOTIN; // Held, but not in either inventory

	return INV_NOICON; // Not held, not in either inventory
}

bool Dialogs::IsInInventory(int object, int invnum) {
	assert(invnum == INV_1 || invnum == INV_2);

	for (int i = 0; i < _invD[invnum].NoofItems; i++) // First inventory
		if (_invD[invnum].contents[i] == object)
			return true;

	return false;
}

/**
 * Returns which item is held (INV_NOICON (-1) if none)
 */
int Dialogs::WhichItemHeld() {
	return _heldItem;
}

/**
 * Called from the cursor module when it re-initializes (at the start of
 * a new scene). For if we are holding something at scene-change time.
 */
void Dialogs::InventoryIconCursor(bool bNewItem) {

	if (_heldItem != INV_NOICON) {
		if (TinselV2) {
			if (bNewItem) {
				int objIndex = GetObjectIndex(_heldItem);
				_heldFilm = _invFilms[objIndex];
			}
			_vm->_cursor->SetAuxCursor(_heldFilm);
		} else {
			INV_OBJECT *invObj = GetInvObject(_heldItem);
			_vm->_cursor->SetAuxCursor(invObj->hIconFilm);
		}
	}
}

/**
 * Returns true if the inventory is active.
 */
bool Dialogs::InventoryActive() {
	return (_inventoryState == ACTIVE_INV);
}

int Dialogs::WhichInventoryOpen() {
	if (_inventoryState != ACTIVE_INV)
		return 0;
	else
		return _activeInv;
}

/**************************************************************************/
/****************** Load/Save game specific functions *********************/
/**************************************************************************/

/**
 * Set first load/save file entry displayed.
 * Point Box[] text pointers to appropriate file descriptions.
 */
void Dialogs::FirstFile(int first) {
	int i, j;

	i = getList();

	cd.numSaved = i;

	if (first < 0)
		first = 0;
	else if (first > MAX_SAVED_FILES - NUM_RGROUP_BOXES)
		first = MAX_SAVED_FILES - NUM_RGROUP_BOXES;

	if (first == 0 && i < MAX_SAVED_FILES && cd.box == saveBox) {
		// Blank first entry for new save
		cd.box[0].boxText = nullptr;
		cd.modifier = j = 1;
	} else {
		cd.modifier = j = 0;
	}

	for (i = first; j < NUM_RGROUP_BOXES; j++, i++) {
		cd.box[j].boxText = ListEntry(i, LE_DESC);
	}

	cd.extraBase = first;
}

/**
 * Save the game using filename from selected slot & current description.
 */

void Dialogs::InvSaveGame() {
	if (cd.selBox != NOBOX) {
#ifndef JAPAN
		_saveGameDesc[strlen(_saveGameDesc) - 1] = 0; // Don't include the cursor!
#endif
		SaveGame(ListEntry(cd.selBox - cd.modifier + cd.extraBase, LE_NAME), _saveGameDesc);
	}
}

/**
 * Load the selected saved game.
 */
void Dialogs::InvLoadGame() {
	int rGame;

	if (cd.selBox != NOBOX && (cd.selBox + cd.extraBase < cd.numSaved)) {
		rGame = cd.selBox;
		cd.selBox = NOBOX;
		if (_iconArray[HL3] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL3]);
			_iconArray[HL3] = nullptr;
		}
		if (_iconArray[HL2] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL2]);
			_iconArray[HL2] = nullptr;
		}
		if (_iconArray[HL1] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL1]);
			_iconArray[HL1] = nullptr;
		}
		RestoreGame(rGame + cd.extraBase);
	}
}

/**
 * Edit the string in sedit[]
 * Returns true if the string was altered.
 */
#ifndef JAPAN
bool Dialogs::UpdateString(const Common::KeyState &kbd) {
	int cpos;

	if (!cd.editableRgroup)
		return false;

	cpos = strlen(_saveGameDesc) - 1;

	if (kbd.ascii == 0)
		return false;

	if (kbd.keycode == Common::KEYCODE_BACKSPACE) {
		if (!cpos)
			return false;
		_saveGameDesc[cpos] = 0;
		cpos--;
		_saveGameDesc[cpos] = CURSOR_CHAR;
		return true;
		//	} else if (isalnum(c) || c == ',' || c == '.' || c == '\'' || (c == ' ' && cpos != 0)) {
	} else if (IsCharImage(_vm->_font->GetTagFontHandle(), kbd.ascii) || (kbd.ascii == ' ' && cpos != 0)) {
		if (cpos == SG_DESC_LEN)
			return false;
		_saveGameDesc[cpos] = kbd.ascii;
		cpos++;
		_saveGameDesc[cpos] = CURSOR_CHAR;
		_saveGameDesc[cpos + 1] = 0;
		return true;
	}
	return false;
}
#endif

/**
 * Keystrokes get sent here when load/save screen is up.
 */
static bool InvKeyIn(const Common::KeyState &kbd) {
	if (kbd.keycode == Common::KEYCODE_PAGEUP ||
	    kbd.keycode == Common::KEYCODE_PAGEDOWN ||
	    kbd.keycode == Common::KEYCODE_HOME ||
	    kbd.keycode == Common::KEYCODE_END)
		return true; // Key needs processing

	if (kbd.keycode == 0 && kbd.ascii == 0) {
		;
	} else if (kbd.keycode == Common::KEYCODE_RETURN) {
		return true; // Key needs processing
	} else if (kbd.keycode == Common::KEYCODE_ESCAPE) {
		return true; // Key needs processing
	} else {
#ifndef JAPAN
		if (_vm->_dialogs->UpdateString(kbd)) {
			/*
			* Delete display of text currently being edited,
			* and replace it with freshly edited text.
			*/
			if (_vm->_dialogs->_iconArray[HL3] != NULL) {
				MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL3]);
				_vm->_dialogs->_iconArray[HL3] = nullptr;
			}
			_vm->_dialogs->_iconArray[HL3] = ObjectTextOut(
			    _vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_saveGameDesc, 0,
			    _vm->_dialogs->CurrentInventoryX() + cd.box[cd.selBox].xpos + 2,
			    _vm->_dialogs->CurrentInventoryY() + cd.box[cd.selBox].ypos + TYOFF,
			    _vm->_font->GetTagFontHandle(), 0);
			if (MultiRightmost(_vm->_dialogs->_iconArray[HL3]) > MAX_NAME_RIGHT) {
				MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL3]);
				_vm->_dialogs->UpdateString(Common::KeyState(Common::KEYCODE_BACKSPACE));
				_vm->_dialogs->_iconArray[HL3] = ObjectTextOut(
				    _vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_saveGameDesc, 0,
				    _vm->_dialogs->CurrentInventoryX() + cd.box[cd.selBox].xpos + 2,
				    _vm->_dialogs->CurrentInventoryY() + cd.box[cd.selBox].ypos + TYOFF,
				    _vm->_font->GetTagFontHandle(), 0);
			}
			MultiSetZPosition(_vm->_dialogs->_iconArray[HL3], Z_INV_ITEXT + 2);
		}
#endif
	}
	return false;
}

/**
 * Highlights selected box.
 * If it's editable (save game), copy existing description and add a cursor.
 */
void Dialogs::Select(int i, bool force) {
#ifdef JAPAN
	time_t secs_now;
	struct tm *time_now;
#endif

	i &= ~IS_MASK;

	if (cd.selBox == i && !force)
		return;

	cd.selBox = i;

	// Clear previous selected highlight and text
	if (_iconArray[HL2] != NULL) {
		MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL2]);
		_iconArray[HL2] = nullptr;
	}
	if (_iconArray[HL3] != NULL) {
		MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL3]);
		_iconArray[HL3] = nullptr;
	}

	// New highlight box
	switch (cd.box[i].boxType) {
	case RGROUP:
		_iconArray[HL2] = RectangleObject(_vm->_bg->BgPal(),
		                                   (TinselV2 ? HighlightColor() : COL_HILIGHT), cd.box[i].w, cd.box[i].h);
		MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL2]);
		MultiSetAniXY(_iconArray[HL2],
		              _invD[_activeInv].inventoryX + cd.box[i].xpos,
		              _invD[_activeInv].inventoryY + cd.box[i].ypos);

		// Z-position of box, and add edit text if appropriate
		if (cd.editableRgroup) {
			MultiSetZPosition(_iconArray[HL2], Z_INV_ITEXT + 1);

			if (TinselV2) {
				assert(cd.box[i].textMethod == TM_POINTER);
			} else {
				assert(cd.box[i].ixText == USE_POINTER);
			}
#ifdef JAPAN
			// Current date and time
			time(&secs_now);
			time_now = localtime(&secs_now);
			strftime(_saveGameDesc, SG_DESC_LEN, "%D %H:%M", time_now);
#else
			// Current description with cursor appended
			if (cd.box[i].boxText != NULL) {
				Common::strlcpy(_saveGameDesc, cd.box[i].boxText, SG_DESC_LEN + 2);
				Common::strlcat(_saveGameDesc, sCursor, SG_DESC_LEN + 2);
			} else {
				Common::strlcpy(_saveGameDesc, sCursor, SG_DESC_LEN + 2);
			}
#endif

			_iconArray[HL3] = ObjectTextOut(
			    _vm->_bg->GetPlayfieldList(FIELD_STATUS), _saveGameDesc, 0,
			    _invD[_activeInv].inventoryX + cd.box[i].xpos + 2,
#ifdef JAPAN
			    _invD[_activeInv].inventoryY + cd.box[i].ypos + 2,
#else
			    _invD[_activeInv].inventoryY + cd.box[i].ypos + TYOFF,
#endif
			    _vm->_font->GetTagFontHandle(), 0);
			MultiSetZPosition(_iconArray[HL3], Z_INV_ITEXT + 2);
		} else {
			MultiSetZPosition(_iconArray[HL2], Z_INV_ICONS + 1);
		}

		_vm->divertKeyInput(InvKeyIn);

		break;

	case FRGROUP:
		_iconArray[HL2] = RectangleObject(_vm->_bg->BgPal(), COL_HILIGHT, cd.box[i].w + 6, cd.box[i].h + 6);
		MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL2]);
		MultiSetAniXY(_iconArray[HL2],
		              _invD[_activeInv].inventoryX + cd.box[i].xpos - 2,
		              _invD[_activeInv].inventoryY + cd.box[i].ypos - 2);
		MultiSetZPosition(_iconArray[HL2], Z_INV_BRECT + 1);

		break;

	default:
		break;
	}
}

/**************************************************************************/
/***/
/**************************************************************************/

/**
 * Stop holding an item.
 */
void Dialogs::DropItem(int item) {
	if (_heldItem == item) {
		_heldItem = INV_NOICON;      // Item not held
		_vm->_cursor->DelAuxCursor(); // no longer aux cursor
	}

	// Redraw contents - held item was not displayed as a content.
	_ItemsChanged = true;
}

/**
 * Clears the specified inventory
 */
void Dialogs::ClearInventory(int invno) {
	assert(invno == INV_1 || invno == INV_2);

	_invD[invno].NoofItems = 0;
	memset(_invD[invno].contents, 0, sizeof(_invD[invno].contents));
}

/**
 * Stick the item into an inventory list (contents[]), and hold the
 * item if requested.
 */
void Dialogs::AddToInventory(int invno, int icon, bool hold) {
	int i;
	bool bOpen;
	INV_OBJECT *invObj;

	// Validate trying to add to a legal inventory
	assert(invno == INV_1 || invno == INV_2 || invno == INV_CONV || invno == INV_OPEN || (invno == INV_DEFAULT && TinselV2));

	if (invno == INV_OPEN) {
		assert(_inventoryState == ACTIVE_INV && (_activeInv == INV_1 || _activeInv == INV_2)); // addopeninv() with inventry not open
		invno = _activeInv;
		bOpen = true;

		// Make sure it doesn't get in both!
		RemFromInventory(_activeInv == INV_1 ? INV_2 : INV_1, icon);
	} else {
		bOpen = false;

		if (TinselV2 && invno == INV_DEFAULT) {
			invObj = GetInvObject(icon);
			if (invObj->attribute & DEFINV2)
				invno = INV_2;
			else if (invObj->attribute & DEFINV1)
				invno = INV_1;
			else
				invno = SysVar(SV_DEFAULT_INV);
		}
	}

	if (invno == INV_1)
		RemFromInventory(INV_2, icon);
	else if (invno == INV_2)
		RemFromInventory(INV_1, icon);

	// See if it's already there
	for (i = 0; i < _invD[invno].NoofItems; i++) {
		if (_invD[invno].contents[i] == icon)
			break;
	}

	// Add it if it isn't already there
	if (i == _invD[invno].NoofItems) {
		if (!bOpen) {
			if (invno == INV_CONV) {
				if (TinselV2) {
					int nei;

					// Count how many current contents have end attribute
					for (i = 0, nei = 0; i < _invD[INV_CONV].NoofItems; i++) {
						invObj = GetInvObject(_invD[INV_CONV].contents[i]);
						if (invObj->attribute & CONVENDITEM)
							nei++;
					}

					// For conversation, insert before end icons
					memmove(&_invD[INV_CONV].contents[i - nei + 1],
					        &_invD[INV_CONV].contents[i - nei], nei * sizeof(int));
					_invD[INV_CONV].contents[i - nei] = icon;
					_invD[INV_CONV].NoofItems++;
					_invD[INV_CONV].NoofHicons = _invD[INV_CONV].NoofItems;

					// Get the window to re-position
					_bMoveOnUnHide = true;
				} else {
					// For conversation, insert before last icon
					// which will always be the goodbye icon
					_invD[invno].contents[_invD[invno].NoofItems] = _invD[invno].contents[_invD[invno].NoofItems - 1];
					_invD[invno].contents[_invD[invno].NoofItems - 1] = icon;
					_invD[invno].NoofItems++;
				}
			} else {
				_invD[invno].contents[_invD[invno].NoofItems++] = icon;
			}
			_ItemsChanged = true;
		} else {
			// It could be that the index is beyond what you'd expect
			// as delinv may well have been called
			if (_glitterIndex < _invD[invno].NoofItems) {
				memmove(&_invD[invno].contents[_glitterIndex + 1],
				        &_invD[invno].contents[_glitterIndex],
				        (_invD[invno].NoofItems - _glitterIndex) * sizeof(int));
				_invD[invno].contents[_glitterIndex] = icon;
			} else {
				_invD[invno].contents[_invD[invno].NoofItems] = icon;
			}
			_invD[invno].NoofItems++;
		}

		// Move here after bug on Japenese DW1
		_ItemsChanged = true;
	}

	// Hold it if requested
	if (hold)
		HoldItem(icon);
}

/**
 * Take the item from the inventory list (contents[]).
 * Return FALSE if item wasn't present, true if it was.
 */
bool Dialogs::RemFromInventory(int invno, int icon) {
	int i;

	assert(invno == INV_1 || invno == INV_2 || invno == INV_CONV); // Trying to delete from illegal inventory

	// See if it's there
	for (i = 0; i < _invD[invno].NoofItems; i++) {
		if (_invD[invno].contents[i] == icon)
			break;
	}

	if (i == _invD[invno].NoofItems)
		return false; // Item wasn't there
	else {
		memmove(&_invD[invno].contents[i], &_invD[invno].contents[i + 1], (_invD[invno].NoofItems - i) * sizeof(int));
		_invD[invno].NoofItems--;

		if (TinselV2 && invno == INV_CONV) {
			_invD[INV_CONV].NoofHicons = _invD[invno].NoofItems;

			// Get the window to re-position
			_bMoveOnUnHide = true;
		}

		_ItemsChanged = true;
		return true; // Item removed
	}
}

/**
 * If the item is not already held, hold it.
 */
void Dialogs::HoldItem(int item, bool bKeepFilm) {
	INV_OBJECT *invObj;

	if (_heldItem != item) {
		if (TinselV2 && (_heldItem != NOOBJECT)) {
			// No longer holding previous item
			_vm->_cursor->DelAuxCursor(); // no longer aux cursor

			// If old held object is not in an inventory, and
			// has a default, stick it in its default inventory.
			if (!IsInInventory(_heldItem, INV_1) && !IsInInventory(_heldItem, INV_2)) {
				invObj = GetInvObject(_heldItem);

				if (invObj->attribute & DEFINV1)
					AddToInventory(INV_1, _heldItem);
				else if (invObj->attribute & DEFINV2)
					AddToInventory(INV_2, _heldItem);
				else
					// Hook for definable default inventory
					AddToInventory(INV_1, _heldItem);
			}

		} else if (!TinselV2) {
			if (item == INV_NOICON && _heldItem != INV_NOICON)
				_vm->_cursor->DelAuxCursor(); // no longer aux cursor

			if (item != INV_NOICON) {
				invObj = GetInvObject(item);
				_vm->_cursor->SetAuxCursor(invObj->hIconFilm); // and is aux. cursor
			}

			// WORKAROUND: If a held item is being removed that's not in either inventory (i.e. it was picked up
			// but never put in them), then when removing it from being held, drop it in the luggage
			if (_heldItem != INV_NOICON && InventoryPos(_heldItem) == INV_HELDNOTIN)
				AddToInventory(INV_1, _heldItem);
		}

		_heldItem = item; // Item held

		if (TinselV2) {
			InventoryIconCursor(!bKeepFilm);

			// Redraw contents - held item not displayed as a content.
			_ItemsChanged = true;
		}
	}

	if (!TinselV2)
		// Redraw contents - held item not displayed as a content.
		_ItemsChanged = true;
}

/**************************************************************************/
/***/
/**************************************************************************/

enum { I_NOTIN,
	   I_HEADER,
	   I_BODY,
	   I_TLEFT,
	   I_TRIGHT,
	   I_BLEFT,
	   I_BRIGHT,
	   I_TOP,
	   I_BOTTOM,
	   I_LEFT,
	   I_RIGHT,
	   I_UP,
	   I_SLIDE_UP,
	   I_SLIDE,
	   I_SLIDE_DOWN,
	   I_DOWN,
	   I_ENDCHANGE
};

#define EXTRA 1 // This was introduced when we decided to increase \
	            // the active area of the borders for re-sizing.

/*---------------------------------*/
#define LeftX _invD[_activeInv].inventoryX
#define TopY _invD[_activeInv].inventoryY
/*---------------------------------*/

/**
 * Work out which area of the inventory window the cursor is in.
 *
 * This used to be worked out with appropriately defined magic numbers.
 * Then the graphic changed and I got it right again. Then the graphic
 * changed and I got fed up of faffing about. It's probably easier just
 * to rework all this.
 */
int Dialogs::InvArea(int x, int y) {
	if (TinselV2) {
		int RightX = MultiRightmost(_rectObject) - NM_BG_SIZ_X - NM_BG_POS_X - NM_RS_R_INSET;
		int BottomY = MultiLowest(_rectObject) - NM_BG_SIZ_Y - NM_BG_POS_Y - NM_RS_B_INSET;

		// Outside the whole rectangle?
		if (x <= LeftX || x > RightX || y <= TopY || y > BottomY)
			return I_NOTIN;

		// The bottom line
		if (y > BottomY - NM_RS_THICKNESS) {
			// Below top of bottom line?
			if (x <= LeftX + NM_RS_THICKNESS)
				return I_BLEFT; // Bottom left corner
			else if (x > RightX - NM_RS_THICKNESS)
				return I_BRIGHT; // Bottom right corner
			else
				return I_BOTTOM; // Just plain bottom
		}

		// The top line
		if (y <= TopY + NM_RS_THICKNESS) {
			// Above bottom of top line?
			if (x <= LeftX + NM_RS_THICKNESS)
				return I_TLEFT; // Top left corner
			else if (x > RightX - NM_RS_THICKNESS)
				return I_TRIGHT; // Top right corner
			else
				return I_TOP; // Just plain top
		}

		// Sides
		if (x <= LeftX + NM_RS_THICKNESS) // Left of right of left side?
			return I_LEFT;
		else if (x > RightX - NM_RS_THICKNESS) // Right of left of right side?
			return I_RIGHT;

		// In the move area?
		if (y < TopY + NM_MOVE_AREA_B_Y)
			return I_HEADER;

		// Scroll bits
		if (!(_activeInv == INV_MENU && cd.bExtraWin)) {
			if (x > RightX - NM_SLIDE_INSET && x <= RightX - NM_SLIDE_INSET + NM_SLIDE_THICKNESS) {
				if (y > TopY + NM_UP_ARROW_TOP && y < TopY + NM_UP_ARROW_BOTTOM)
					return I_UP;
				if (y > BottomY - NM_DN_ARROW_TOP && y <= BottomY - NM_DN_ARROW_BOTTOM)
					return I_DOWN;

				/* '3' is a magic adjustment with no apparent sense */

				if (y >= TopY + _sliderYmin - 3 && y < TopY + _sliderYmax + NM_SLH) {
					if (y < TopY + _sliderYpos - 3)
						return I_SLIDE_UP;
					if (y < TopY + _sliderYpos + NM_SLH - 3)
						return I_SLIDE;
					else
						return I_SLIDE_DOWN;
				}
			}
		}
	} else {
		int RightX = MultiRightmost(_rectObject) + 1;
		int BottomY = MultiLowest(_rectObject) + 1;

		// Outside the whole rectangle?
		if (x <= LeftX - EXTRA || x > RightX + EXTRA || y <= TopY - EXTRA || y > BottomY + EXTRA)
			return I_NOTIN;

		// The bottom line
		if (y > BottomY - 2 - EXTRA) { // Below top of bottom line?
			if (x <= LeftX + 2 + EXTRA)
				return I_BLEFT; // Bottom left corner
			else if (x > RightX - 2 - EXTRA)
				return I_BRIGHT; // Bottom right corner
			else
				return I_BOTTOM; // Just plain bottom
		}

		// The top line
		if (y <= TopY + 2 + EXTRA) { // Above bottom of top line?
			if (x <= LeftX + 2 + EXTRA)
				return I_TLEFT; // Top left corner
			else if (x > RightX - 2 - EXTRA)
				return I_TRIGHT; // Top right corner
			else
				return I_TOP; // Just plain top
		}

		// Sides
		if (x <= LeftX + 2 + EXTRA) // Left of right of left side?
			return I_LEFT;
		else if (x > RightX - 2 - EXTRA) // Right of left of right side?
			return I_RIGHT;

		// From here down still needs fixing up properly
		/*
		 * In the move area?
		 */
		if (_activeInv != INV_CONF && x >= LeftX + M_SW - 2 && x <= RightX - M_SW + 3 &&
		    y >= TopY + M_TH - 2 && y < TopY + M_TBB + 2)
			return I_HEADER;

		/*
		 * Scroll bits
		 */
		if (!(_activeInv == INV_CONF && cd.bExtraWin)) {
			if (x > RightX - NM_SLIDE_INSET && x <= RightX - NM_SLIDE_INSET + NM_SLIDE_THICKNESS) {
				if (y > TopY + M_IUT + 1 && y < TopY + M_IUB - 1)
					return I_UP;
				if (y > BottomY - M_IDT + 4 && y <= BottomY - M_IDB + 1)
					return I_DOWN;

				if (y >= TopY + _sliderYmin && y < TopY + _sliderYmax + M_SH) {
					if (y < TopY + _sliderYpos)
						return I_SLIDE_UP;
					if (y < TopY + _sliderYpos + M_SH)
						return I_SLIDE;
					else
						return I_SLIDE_DOWN;
				}
			}
		}
	}

	return I_BODY;
}

/**
 * Returns the id of the icon displayed under the given position.
 * Also return co-ordinates of items tag display position, if requested.
 */
int Dialogs::InvItem(int *x, int *y, bool update) {
	int itop, ileft;
	int row, col;
	int item;
	int IconsX;

	itop = _invD[_activeInv].inventoryY + START_ICONY;

	IconsX = _invD[_activeInv].inventoryX + START_ICONX;

	for (item = _invD[_activeInv].FirstDisp, row = 0; row < _invD[_activeInv].NoofVicons; row++) {
		ileft = IconsX;

		for (col = 0; col < _invD[_activeInv].NoofHicons; col++, item++) {
			if (*x >= ileft && *x < ileft + ITEM_WIDTH &&
			    *y >= itop && *y < itop + ITEM_HEIGHT) {
				if (update) {
					*x = ileft + ITEM_WIDTH / 2;
					*y = itop /*+ ITEM_HEIGHT/4*/;
				}
				return item;
			}

			ileft += ITEM_WIDTH + 1;
		}
		itop += ITEM_HEIGHT + 1;
	}
	return INV_NOICON;
}

int Dialogs::InvItem(Common::Point &coOrds, bool update) {
	int x = coOrds.x;
	int y = coOrds.y;
	return InvItem(&x, &y, update);
	//coOrds.x = x;
	//coOrds.y = y;
}

/**
 * Returns the id of the icon displayed under the given position.
 */
int Dialogs::InvItemId(int x, int y) {
	int itop, ileft;
	int row, col;
	int item;

	if (_InventoryHidden || _inventoryState == IDLE_INV)
		return INV_NOICON;

	itop = _invD[_activeInv].inventoryY + START_ICONY;

	int IconsX = _invD[_activeInv].inventoryX + START_ICONX;

	for (item = _invD[_activeInv].FirstDisp, row = 0; row < _invD[_activeInv].NoofVicons; row++) {
		ileft = IconsX;

		for (col = 0; col < _invD[_activeInv].NoofHicons; col++, item++) {
			if (x >= ileft && x < ileft + ITEM_WIDTH &&
			    y >= itop && y < itop + ITEM_HEIGHT) {
				return _invD[_activeInv].contents[item];
			}

			ileft += ITEM_WIDTH + 1;
		}
		itop += ITEM_HEIGHT + 1;
	}
	return INV_NOICON;
}

/**
 * Finds which box the cursor is in.
 */
int Dialogs::WhichMenuBox(int curX, int curY, bool bSlides) {
	if (bSlides) {
		for (int i = 0; i < _numMdSlides; i++) {
			if (curY > MultiHighest(_mdSlides[i].obj) && curY < MultiLowest(_mdSlides[i].obj) && curX > MultiLeftmost(_mdSlides[i].obj) && curX < MultiRightmost(_mdSlides[i].obj))
				return _mdSlides[i].num | IS_SLIDER;
		}
	}

	curX -= _invD[_activeInv].inventoryX;
	curY -= _invD[_activeInv].inventoryY;

	for (int i = 0; i < cd.NumBoxes; i++) {
		switch (cd.box[i].boxType) {
		case SLIDER:
			if (bSlides) {
				if (curY >= cd.box[i].ypos + MD_YBUTTOP && curY < cd.box[i].ypos + MD_YBUTBOT) {
					if (curX >= cd.box[i].xpos + MD_XLBUTL && curX < cd.box[i].xpos + MD_XLBUTR)
						return i | IS_LEFT;
					if (curX >= cd.box[i].xpos + MD_XRBUTL && curX < cd.box[i].xpos + MD_XRBUTR)
						return i | IS_RIGHT;
				}
			}
			break;

		case AAGBUT:
		case ARSGBUT:
		case TOGGLE:
		case TOGGLE1:
		case TOGGLE2:
		case FLIP:
			if (curY > cd.box[i].ypos && curY < cd.box[i].ypos + cd.box[i].h && curX > cd.box[i].xpos && curX < cd.box[i].xpos + cd.box[i].w)
				return i;
			break;

		case ROTATE:
			if (_noLanguage)
				break;

			if (curY > cd.box[i].ypos && curY < cd.box[i].ypos + cd.box[i].h) {
				// Left one?
				if (curX > cd.box[i].xpos - ROTX1 && curX < cd.box[i].xpos - ROTX1 + cd.box[i].w) {
					cd.box[i].bi = IX2_LEFT1;
					return i;
				}
				// Right one?
				if (curX > cd.box[i].xpos + ROTX1 && curX < cd.box[i].xpos + ROTX1 + cd.box[i].w) {
					cd.box[i].bi = IX2_RIGHT1;
					return i;
				}
			}
			break;

		default:
			// 'Normal' box
			if (curY >= cd.box[i].ypos && curY < cd.box[i].ypos + cd.box[i].h && curX >= cd.box[i].xpos && curX < cd.box[i].xpos + cd.box[i].w)
				return i;
			break;
		}
	}

	// Slider on extra window
	if (cd.bExtraWin) {
		const Common::Rect r = TinselV2 ? Common::Rect(411, 46, 425, 339) : Common::Rect(20 + 181, 24 + 2, 20 + 181 + 8, 24 + 139 + 5);

		if (r.contains(curX, curY)) {

			if (curY < (r.top + (TinselV2 ? 18 : 5)))
				return IB_UP;
			else if (curY > (r.bottom - (TinselV2 ? 18 : 5)))
				return IB_DOWN;
			else if (curY + _invD[_activeInv].inventoryY < _sliderYpos)
				return IB_SLIDE_UP;
			else if (curY + _invD[_activeInv].inventoryY >= _sliderYpos + NM_SLH)
				return IB_SLIDE_DOWN;
			else
				return IB_SLIDE;
		}
	}

	return IB_NONE;
}

/**************************************************************************/
/***/
/**************************************************************************/

#define ROTX1 60 // Rotate button's offsets from the center

/**
 * InvBoxes
 */
void Dialogs::InvBoxes(bool InBody, int curX, int curY) {
	int rotateIndex = -1;
	int index; // Box pointed to on this call
	const FILM *pfilm;

	// Find out which icon is currently pointed to
	if (!InBody)
		index = -1;
	else {
		index = WhichMenuBox(curX, curY, false);
	}

	// If no icon pointed to, or points to (logical position of)
	// currently held icon, then no icon is pointed to!
	if (index < 0) {
		// unhigh-light box (if one was)
		cd.pointBox = NOBOX;
		if (_iconArray[HL1] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL1]);
			_iconArray[HL1] = nullptr;
		}
	} else if (index != cd.pointBox) {
		cd.pointBox = index;
		// A new box is pointed to - high-light it
		if (_iconArray[HL1] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL1]);
			_iconArray[HL1] = nullptr;
		}
		if ((cd.box[cd.pointBox].boxType == ARSBUT && cd.selBox != NOBOX) ||
		    ///* I don't agree */ cd.box[cd.pointBox].boxType == RGROUP ||
		    cd.box[cd.pointBox].boxType == AATBUT ||
		    cd.box[cd.pointBox].boxType == AABUT) {
			_iconArray[HL1] = RectangleObject(_vm->_bg->BgPal(),
			                                   (TinselV2 ? HighlightColor() : COL_HILIGHT),
			                                   cd.box[cd.pointBox].w, cd.box[cd.pointBox].h);
			MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL1]);
			MultiSetAniXY(_iconArray[HL1],
			              _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos,
			              _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos);
			MultiSetZPosition(_iconArray[HL1], Z_INV_ICONS + 1);
		} else if (cd.box[cd.pointBox].boxType == AAGBUT ||
		           cd.box[cd.pointBox].boxType == ARSGBUT ||
		           cd.box[cd.pointBox].boxType == TOGGLE ||
		           cd.box[cd.pointBox].boxType == TOGGLE1 ||
		           cd.box[cd.pointBox].boxType == TOGGLE2) {
			pfilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

			_iconArray[HL1] = AddObject(&pfilm->reels[cd.box[cd.pointBox].bi + HIGRAPH], -1);
			MultiSetAniXY(_iconArray[HL1],
			              _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos,
			              _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos);
			MultiSetZPosition(_iconArray[HL1], Z_INV_ICONS + 1);
		} else if (cd.box[cd.pointBox].boxType == ROTATE) {
			if (_noLanguage)
				return;

			pfilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

			rotateIndex = cd.box[cd.pointBox].bi;
			if (rotateIndex == IX2_LEFT1) {
				_iconArray[HL1] = AddObject(&pfilm->reels[IX2_LEFT2], -1);
				MultiSetAniXY(_iconArray[HL1],
				              _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos - ROTX1,
				              _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos);
				MultiSetZPosition(_iconArray[HL1], Z_INV_ICONS + 1);
			} else if (rotateIndex == IX2_RIGHT1) {
				_iconArray[HL1] = AddObject(&pfilm->reels[IX2_RIGHT2], -1);
				MultiSetAniXY(_iconArray[HL1],
				              _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos + ROTX1,
				              _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos);
				MultiSetZPosition(_iconArray[HL1], Z_INV_ICONS + 1);
			}
		}
	}
}

/**
 * Monitors for POINTED event for inventory icons.
 */
void Dialogs::InvLabels(bool InBody, int aniX, int aniY) {
	int index; // Icon pointed to on this call
	INV_OBJECT *invObj;

	// Find out which icon is currently pointed to
	if (!InBody)
		index = INV_NOICON;
	else {
		index = InvItem(&aniX, &aniY, false);
		if (index != INV_NOICON) {
			if (index >= _invD[_activeInv].NoofItems)
				index = INV_NOICON;
			else
				index = _invD[_activeInv].contents[index];
		}
	}

	// If no icon pointed to, or points to (logical position of)
	// currently held icon, then no icon is pointed to!
	if (index == INV_NOICON || index == _heldItem) {
		_pointedIcon = INV_NOICON;
	} else if (index != _pointedIcon) {
		// A new icon is pointed to - run its script with POINTED event
		invObj = GetInvObject(index);
		if (invObj->hScript)
			InvTinselEvent(invObj, POINTED, PLR_NOEVENT, index);
		_pointedIcon = index;
	}
}

/**************************************************************************/
/***/
/**************************************************************************/

/**
 * All to do with the slider.
 * I can't remember how it works - or, indeed, what it does.
 * It seems to set up slideStuff[], an array of possible first-displayed
 * icons set against the matching y-positions of the slider.
 */
void Dialogs::AdjustTop() {
	int tMissing, bMissing, nMissing;
	int nsliderYpos;
	int rowsWanted;
	int slideRange;
	int n, i;

	// Only do this if there's a slider
	if (!_slideObject)
		return;

	rowsWanted = (_invD[_activeInv].NoofItems - _invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons - 1) / _invD[_activeInv].NoofHicons;

	while (rowsWanted < _invD[_activeInv].NoofVicons) {
		if (_invD[_activeInv].FirstDisp) {
			_invD[_activeInv].FirstDisp -= _invD[_activeInv].NoofHicons;
			if (_invD[_activeInv].FirstDisp < 0)
				_invD[_activeInv].FirstDisp = 0;
			rowsWanted++;
		} else
			break;
	}
	tMissing = _invD[_activeInv].FirstDisp ? (_invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons - 1) / _invD[_activeInv].NoofHicons : 0;
	bMissing = (rowsWanted > _invD[_activeInv].NoofVicons) ? rowsWanted - _invD[_activeInv].NoofVicons : 0;

	nMissing = tMissing + bMissing;
	slideRange = _sliderYmax - _sliderYmin;

	if (!tMissing)
		nsliderYpos = _sliderYmin;
	else if (!bMissing)
		nsliderYpos = _sliderYmax;
	else {
		nsliderYpos = tMissing * slideRange / nMissing;
		nsliderYpos += _sliderYmin;
	}

	if (nMissing) {
		n = _invD[_activeInv].FirstDisp - tMissing * _invD[_activeInv].NoofHicons;
		for (i = 0; i <= nMissing; i++, n += _invD[_activeInv].NoofHicons) {
			_slideStuff[i].n = n;
			_slideStuff[i].y = (i * slideRange / nMissing) + _sliderYmin;
		}
		if (_slideStuff[0].n < 0)
			_slideStuff[0].n = 0;
		assert(i < MAX_ININV + 1);
		_slideStuff[i].n = -1;
	} else {
		_slideStuff[0].n = 0;
		_slideStuff[0].y = _sliderYmin;
		_slideStuff[1].n = -1;
	}

	if (nsliderYpos != _sliderYpos) {
		MultiMoveRelXY(_slideObject, 0, nsliderYpos - _sliderYpos);
		_sliderYpos = nsliderYpos;
	}
}

/**
 * Insert an inventory icon object onto the display list.
 */
OBJECT *Dialogs::AddInvObject(int num, const FREEL **pfreel, const FILM **pfilm) {
	INV_OBJECT *invObj;    // Icon data
	const MULTI_INIT *pmi; // Its INIT structure - from the reel
	IMAGE *pim;            // ... you get the picture
	OBJECT *pPlayObj;      // The object we insert

	invObj = GetInvObject(num);

	// Get pointer to image
	pim = _vm->_cursor->GetImageFromFilm(invObj->hIconFilm, 0, pfreel, &pmi, pfilm);

	// Poke in the background palette
	pim->hImgPal = TO_32(_vm->_bg->BgPal());

	// Set up the multi-object
	pPlayObj = MultiInitObject(pmi);
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), pPlayObj);

	return pPlayObj;
}

/**
 * Create display objects for the displayed icons in an inventory window.
 */
void Dialogs::FillInInventory() {
	int Index; // Index into contents[]
	int n = 0; // index into iconArray[]
	int xpos, ypos;
	int row, col;
	const FREEL *pfr;
	const FILM *pfilm;

	DumpIconArray();

	if (_invDragging != ID_SLIDE)
		AdjustTop(); // Set up slideStuff[]

	Index = _invD[_activeInv].FirstDisp; // Start from first displayed object
	n = 0;
	ypos = START_ICONY; // Y-offset of first display row

	for (row = 0; row < _invD[_activeInv].NoofVicons; row++, ypos += ITEM_HEIGHT + 1) {
		xpos = START_ICONX; // X-offset of first display column

		for (col = 0; col < _invD[_activeInv].NoofHicons; col++) {
			if (Index >= _invD[_activeInv].NoofItems)
				break;
			else if (_invD[_activeInv].contents[Index] != _heldItem) {
				// Create a display object and position it
				_iconArray[n] = AddInvObject(_invD[_activeInv].contents[Index], &pfr, &pfilm);
				MultiSetAniXY(_iconArray[n], _invD[_activeInv].inventoryX + xpos, _invD[_activeInv].inventoryY + ypos);
				MultiSetZPosition(_iconArray[n], Z_INV_ICONS);

				InitStepAnimScript(&_iconAnims[n], _iconArray[n], FROM_32(pfr->script), ONE_SECOND / FROM_32(pfilm->frate));

				n++;
			}
			Index++;
			xpos += ITEM_WIDTH + 1; // X-offset of next display column
		}
	}
}

enum { FROM_HANDLE,
	   FROM_STRING };

/**
 * Set up a rectangle as the background to the inventory window.
 *  Additionally, sticks the window title up.
 */
void Dialogs::AddBackground(OBJECT **rect, OBJECT **title, int extraH, int extraV, int textFrom) {
	// Why not 2 ????
	int width = _TLwidth + extraH + _TRwidth + NM_BG_SIZ_X;
	int height = _TLheight + extraV + _BLheight + NM_BG_SIZ_Y;

	// Create a rectangle object
	_rectObject = *rect = TranslucentObject(width, height);

	// add it to display list and position it
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), *rect);
	MultiSetAniXY(*rect, _invD[_activeInv].inventoryX + NM_BG_POS_X,
	              _invD[_activeInv].inventoryY + NM_BG_POS_Y);
	MultiSetZPosition(*rect, Z_INV_BRECT);

	if (title == NULL)
		return;

	// Create text object using title string
	if (textFrom == FROM_HANDLE) {
		LoadStringRes(_invD[_activeInv].hInvTitle, _vm->_font->TextBufferAddr(), TBUFSZ);
		*title = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_font->TextBufferAddr(), 0,
		                       _invD[_activeInv].inventoryX + width / 2, _invD[_activeInv].inventoryY + M_TOFF,
		                       _vm->_font->GetTagFontHandle(), TXT_CENTER);
		assert(*title); // Inventory title string produced NULL text
		MultiSetZPosition(*title, Z_INV_HTEXT);
	} else if (textFrom == FROM_STRING && cd.ixHeading != NO_HEADING) {
		LoadStringRes(_configStrings[cd.ixHeading], _vm->_font->TextBufferAddr(), TBUFSZ);
		*title = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_font->TextBufferAddr(), 0,
		                       _invD[_activeInv].inventoryX + width / 2, _invD[_activeInv].inventoryY + M_TOFF,
		                       _vm->_font->GetTagFontHandle(), TXT_CENTER);
		assert(*title); // Inventory title string produced NULL text
		MultiSetZPosition(*title, Z_INV_HTEXT);
	}
}

/**
 * Set up a rectangle as the background to the inventory window.
 */
void Dialogs::AddBackground(OBJECT **rect, int extraH, int extraV) {
	AddBackground(rect, NULL, extraH, extraV, 0);
}

/**
 * Adds a title for a dialog
 */
void Dialogs::AddTitle(POBJECT *title, int extraH) {
	int width = _TLwidth + extraH + _TRwidth + NM_BG_SIZ_X;

	// Create text object using title string
	if (_invD[_activeInv].hInvTitle != (SCNHANDLE)NO_HEADING) {
		LoadStringRes(_invD[_activeInv].hInvTitle, _vm->_font->TextBufferAddr(), TBUFSZ);
		*title = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_font->TextBufferAddr(), 0,
		                       _invD[_activeInv].inventoryX + (width / 2) + NM_BG_POS_X, _invD[_activeInv].inventoryY + NM_TOFF,
		                       _vm->_font->GetTagFontHandle(), TXT_CENTER, 0);
		assert(*title);
		MultiSetZPosition(*title, Z_INV_HTEXT);
	}
}

/**
 * Insert a part of the inventory window frame onto the display list.
 */
OBJECT *Dialogs::AddObject(const FREEL *pfreel, int num) {
	const MULTI_INIT *pmi; // Get the MULTI_INIT structure
	IMAGE *pim;
	OBJECT *pPlayObj;

	// Get pointer to image
	pim = _vm->_cursor->GetImageFromReel(pfreel, &pmi);

	// Poke in the background palette
	pim->hImgPal = TO_32(_vm->_bg->BgPal());

	// Horrible bodge involving global variables to save
	// width and/or height of some window frame components
	if (num == _TL) {
		_TLwidth = FROM_16(pim->imgWidth);
		_TLheight = FROM_16(pim->imgHeight) & ~C16_FLAG_MASK;
	} else if (num == _TR) {
		_TRwidth = FROM_16(pim->imgWidth);
	} else if (num == _BL) {
		_BLheight = FROM_16(pim->imgHeight) & ~C16_FLAG_MASK;
	}

	// Set up and insert the multi-object
	pPlayObj = MultiInitObject(pmi);
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), pPlayObj);

	return pPlayObj;
}

/**
 * Display the scroll bar slider.
 */

void Dialogs::AddSlider(OBJECT **slide, const FILM *pfilm) {
	_slideObject = *slide = AddObject(&pfilm->reels[IX_SLIDE], -1);
	MultiSetAniXY(*slide, MultiRightmost(_rectObject) + (TinselV2 ? NM_SLX : -M_SXOFF + 2),
	              _invD[_activeInv].inventoryY + _sliderYpos);
	MultiSetZPosition(*slide, Z_INV_MFRAME);
}

/**
 * Display a box with some text in it.
 */
void Dialogs::AddBox(int *pi, const int i) {
	int x = _invD[_activeInv].inventoryX + cd.box[i].xpos;
	int y = _invD[_activeInv].inventoryY + cd.box[i].ypos;
	int *pival = cd.box[i].ival;
	int xdisp;
	const FILM *pFilm;

	switch (cd.box[i].boxType) {
	default:
		// Ignore if it's a blank scene hopper box
		if (TinselV2 && (cd.box[i].textMethod == TM_NONE))
			break;

		// Give us a box
		_iconArray[*pi] = RectangleObject(_vm->_bg->BgPal(), TinselV2 ? BoxColor() : COL_BOX,
		                                   cd.box[i].w, cd.box[i].h);
		MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[*pi]);
		MultiSetAniXY(_iconArray[*pi], x, y);
		MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 1);
		*pi += 1;

		// Stick in the text
		if ((cd.box[i].textMethod == TM_POINTER) ||
		    (!TinselV2 && (cd.box[i].ixText == USE_POINTER))) {
			if (cd.box[i].boxText != NULL) {
				if (cd.box[i].boxType == RGROUP) {
					_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), cd.box[i].boxText, 0,
#ifdef JAPAN
					                                 x + 2, y + 2, GetTagFontHandle(), 0);
#else
					                                 x + 2, y + TYOFF, _vm->_font->GetTagFontHandle(), 0);
#endif
				} else {
					_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), cd.box[i].boxText, 0,
#ifdef JAPAN
					                                 // Note: it never seems to go here!
					                                 x + cd.box[i].w / 2, y + 2, GetTagFontHandle(), TXT_CENTER);
#else
					                                 x + cd.box[i].w / 2, y + TYOFF, _vm->_font->GetTagFontHandle(), TXT_CENTER);
#endif
				}

				MultiSetZPosition(_iconArray[*pi], Z_INV_ITEXT);
				*pi += 1;
			}
		} else {
			if (TinselV2) {
				if (cd.box[i].textMethod == TM_INDEX)
					LoadStringRes(SysString(cd.box[i].ixText), _vm->_font->TextBufferAddr(), TBUFSZ);
				else {
					assert(cd.box[i].textMethod == TM_STRINGNUM);
					LoadStringRes(cd.box[i].ixText, _vm->_font->TextBufferAddr(), TBUFSZ);
				}
			} else {
				LoadStringRes(_configStrings[cd.box[i].ixText], _vm->_font->TextBufferAddr(), TBUFSZ);
				assert(cd.box[i].boxType != RGROUP); // You'll need to add some code!
			}

			if (TinselV2 && (cd.box[i].boxType == RGROUP))
				_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_font->TextBufferAddr(),
				                                 0, x + 2, y + TYOFF, _vm->_font->GetTagFontHandle(), 0, 0);
			else
				_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS),
				                                 _vm->_font->TextBufferAddr(), 0,
#ifdef JAPAN
				                                 x + cd.box[i].w / 2, y + 2, GetTagFontHandle(), TXT_CENTER);
#else
				                                 x + cd.box[i].w / 2, y + TYOFF, _vm->_font->GetTagFontHandle(), TXT_CENTER);
#endif
			MultiSetZPosition(_iconArray[*pi], Z_INV_ITEXT);
			*pi += 1;
		}
		break;

	case AAGBUT:
	case ARSGBUT:
		pFilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

		_iconArray[*pi] = AddObject(&pFilm->reels[cd.box[i].bi + NORMGRAPH], -1);
		MultiSetAniXY(_iconArray[*pi], x, y);
		MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 1);
		*pi += 1;

		break;

	case FRGROUP:
		assert(_flagFilm != 0); // Language flags not declared!

		pFilm = (const FILM *)_vm->_handle->LockMem(_flagFilm);

		if (_vm->_config->_isAmericanEnglishVersion && cd.box[i].bi == FIX_UK)
			cd.box[i].bi = FIX_USA;

		_iconArray[*pi] = AddObject(&pFilm->reels[cd.box[i].bi], -1);
		MultiSetAniXY(_iconArray[*pi], x, y);
		MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 2);
		*pi += 1;

		break;

	case FLIP:
		pFilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

		if (*pival)
			_iconArray[*pi] = AddObject(&pFilm->reels[cd.box[i].bi], -1);
		else
			_iconArray[*pi] = AddObject(&pFilm->reels[cd.box[i].bi + 1], -1);
		MultiSetAniXY(_iconArray[*pi], x, y);
		MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 1);
		*pi += 1;

		// Stick in the text
		if (TinselV2) {
			assert(cd.box[i].textMethod == TM_INDEX);
			LoadStringRes(SysString(cd.box[i].ixText), _vm->_font->TextBufferAddr(), TBUFSZ);
		} else {
			assert(cd.box[i].ixText != USE_POINTER);
			LoadStringRes(_configStrings[cd.box[i].ixText], _vm->_font->TextBufferAddr(), TBUFSZ);
		}
		_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS),
		                                 _vm->_font->TextBufferAddr(), 0, x + MDTEXT_XOFF, y + MDTEXT_YOFF, _vm->_font->GetTagFontHandle(), TXT_RIGHT);
		MultiSetZPosition(_iconArray[*pi], Z_INV_ITEXT);
		*pi += 1;
		break;

	case TOGGLE:
	case TOGGLE1:
	case TOGGLE2:
		pFilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

		cd.box[i].bi = *pival ? IX_TICK1 : IX_CROSS1;
		_iconArray[*pi] = AddObject(&pFilm->reels[cd.box[i].bi + NORMGRAPH], -1);
		MultiSetAniXY(_iconArray[*pi], x, y);
		MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 1);
		*pi += 1;

		// Stick in the text
		if (TinselV2) {
			assert(cd.box[i].textMethod == TM_INDEX);
			LoadStringRes(SysString(cd.box[i].ixText), _vm->_font->TextBufferAddr(), TBUFSZ);
		} else {
			assert(cd.box[i].ixText != USE_POINTER);
			LoadStringRes(_configStrings[cd.box[i].ixText], _vm->_font->TextBufferAddr(), TBUFSZ);
		}

		if (cd.box[i].boxType == TOGGLE2) {
			_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS),
			                                 _vm->_font->TextBufferAddr(), 0, x + cd.box[i].w / 2, y + TOG2_YOFF,
			                                 _vm->_font->GetTagFontHandle(), TXT_CENTER, 0);
		} else {
			_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS),
			                                 _vm->_font->TextBufferAddr(), 0, x + MDTEXT_XOFF, y + MDTEXT_YOFF,
			                                 _vm->_font->GetTagFontHandle(), TXT_RIGHT, 0);
		}

		MultiSetZPosition(_iconArray[*pi], Z_INV_ITEXT);
		*pi += 1;
		break;

	case SLIDER:
		pFilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);
		xdisp = SLIDE_RANGE * (*pival) / cd.box[i].w;

		_iconArray[*pi] = AddObject(&pFilm->reels[IX_MDGROOVE], -1);
		MultiSetAniXY(_iconArray[*pi], x, y);
		MultiSetZPosition(_iconArray[*pi], Z_MDGROOVE);
		*pi += 1;
		_iconArray[*pi] = AddObject(&pFilm->reels[IX_MDSLIDER], -1);
		MultiSetAniXY(_iconArray[*pi], x + SLIDE_MINX + xdisp, y);
		MultiSetZPosition(_iconArray[*pi], Z_MDSLIDER);
		assert(_numMdSlides < MAXSLIDES);
		_mdSlides[_numMdSlides].num = i;
		_mdSlides[_numMdSlides].min = x + SLIDE_MINX;
		_mdSlides[_numMdSlides].max = x + SLIDE_MAXX;
		_mdSlides[_numMdSlides++].obj = _iconArray[*pi];
		*pi += 1;

		// Stick in the text
		if (TinselV2) {
			assert(cd.box[i].textMethod == TM_INDEX);
			LoadStringRes(SysString(cd.box[i].ixText), _vm->_font->TextBufferAddr(), TBUFSZ);
		} else {
			assert(cd.box[i].ixText != USE_POINTER);
			LoadStringRes(_configStrings[cd.box[i].ixText], _vm->_font->TextBufferAddr(), TBUFSZ);
		}
		_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS),
		                                 _vm->_font->TextBufferAddr(), 0, x + MDTEXT_XOFF, y + MDTEXT_YOFF, _vm->_font->GetTagFontHandle(), TXT_RIGHT);
		MultiSetZPosition(_iconArray[*pi], Z_INV_ITEXT);
		*pi += 1;
		break;

	case ROTATE:
		pFilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

		// Left one
		if (!_noLanguage) {
			_iconArray[*pi] = AddObject(&pFilm->reels[IX2_LEFT1], -1);
			MultiSetAniXY(_iconArray[*pi], x - ROTX1, y);
			MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 1);
			*pi += 1;

			// Right one
			_iconArray[*pi] = AddObject(&pFilm->reels[IX2_RIGHT1], -1);
			MultiSetAniXY(_iconArray[*pi], x + ROTX1, y);
			MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 1);
			*pi += 1;

			// Stick in the text
			assert(cd.box[i].textMethod == TM_INDEX);
			LoadStringRes(SysString(cd.box[i].ixText), _vm->_font->TextBufferAddr(), TBUFSZ);
			_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS),
			                                 _vm->_font->TextBufferAddr(), 0, x + cd.box[i].w / 2, y + TOG2_YOFF,
			                                 _vm->_font->GetTagFontHandle(), TXT_CENTER, 0);
			MultiSetZPosition(_iconArray[*pi], Z_INV_ITEXT);
			*pi += 1;
		}

		// Current language's text
		if (LanguageDesc(_displayedLanguage) == 0)
			break;

		LoadStringRes(LanguageDesc(_displayedLanguage), _vm->_font->TextBufferAddr(), TBUFSZ);
		_iconArray[*pi] = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_font->TextBufferAddr(), 0,
		                                 x + cd.box[i].w / 2, y + ROT_YOFF, _vm->_font->GetTagFontHandle(), TXT_CENTER, 0);
		MultiSetZPosition(_iconArray[*pi], Z_INV_ITEXT);
		*pi += 1;

		// Current language's flag
		pFilm = (const FILM *)_vm->_handle->LockMem(LanguageFlag(_displayedLanguage));
		_iconArray[*pi] = AddObject(&pFilm->reels[0], -1);
		MultiSetAniXY(_iconArray[*pi], x + FLAGX, y + FLAGY);
		MultiSetZPosition(_iconArray[*pi], Z_INV_BRECT + 1);
		*pi += 1;
		break;
	}
}

/**
 * Display some boxes.
 */
void Dialogs::AddBoxes(bool bPosnSlide) {
	int objCount = NUMHL; // Object count - allow for HL1, HL2 etc.

	DumpIconArray();
	_numMdSlides = 0;

	for (int i = 0; i < cd.NumBoxes; i++) {
		AddBox(&objCount, i);
	}

	if (cd.bExtraWin) {
		if (bPosnSlide && !TinselV2)
			_sliderYpos = _sliderYmin + (cd.extraBase * (_sliderYmax - _sliderYmin)) / (MAX_SAVED_FILES - NUM_RGROUP_BOXES);
		else if (bPosnSlide) {
			// Tinsel 2 bPosnSlide code
			int lastY = _sliderYpos;

			if (cd.box == loadBox || cd.box == saveBox)
				_sliderYpos = _sliderYmin + (cd.extraBase * (sliderRange)) /
				                                  (MAX_SAVED_FILES - NUM_RGROUP_BOXES);
			else if (cd.box == hopperBox1) {
				if (_numScenes <= NUM_RGROUP_BOXES)
					_sliderYpos = _sliderYmin;
				else
					_sliderYpos = _sliderYmin + (cd.extraBase * (sliderRange)) / (_numScenes - NUM_RGROUP_BOXES);
			} else if (cd.box == hopperBox2) {
				if (_numEntries <= NUM_RGROUP_BOXES)
					_sliderYpos = _sliderYmin;
				else
					_sliderYpos = _sliderYmin + (cd.extraBase * (sliderRange)) /
					                                  (_numEntries - NUM_RGROUP_BOXES);
			}

			MultiMoveRelXY(_slideObject, 0, _sliderYpos - lastY);
		}

		if (!TinselV2)
			MultiSetAniXY(_slideObject, _invD[_activeInv].inventoryX + 24 + 179, _sliderYpos);
	}

	assert(objCount < MAX_ICONS); // added too many icons
}

/**
 * Display the scroll bar slider.
 */
void Dialogs::AddEWSlider(OBJECT **slide, const FILM *pfilm) {
	_slideObject = *slide = AddObject(&pfilm->reels[IX_SLIDE], -1);
	MultiSetAniXY(*slide, _invD[_activeInv].inventoryX + 24 + 127, _sliderYpos);
	MultiSetZPosition(*slide, Z_INV_MFRAME);
}

/**
 * AddExtraWindow
 */
int Dialogs::AddExtraWindow(int x, int y, OBJECT **retObj) {
	int n = 0;
	const FILM *pfilm;

	// Get the frame's data
	pfilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

	x += TinselV2 ? 30 : 20;
	y += TinselV2 ? 38 : 24;

	// Draw the four corners
	retObj[n] = AddObject(&pfilm->reels[IX_RTL], -1); // Top left
	MultiSetAniXY(retObj[n], x, y);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_NTR], -1); // Top right
	MultiSetAniXY(retObj[n], x + (TinselV2 ? _TLwidth + 312 : 152), y);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_BL], -1); // Bottom left
	MultiSetAniXY(retObj[n], x, y + (TinselV2 ? _TLheight + 208 : 124));
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_BR], -1); // Bottom right
	MultiSetAniXY(retObj[n], x + (TinselV2 ? _TLwidth + 312 : 152),
	              y + (TinselV2 ? _TLheight + 208 : 124));
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;

	// Draw the edges
	retObj[n] = AddObject(&pfilm->reels[IX_H156], -1); // Top
	MultiSetAniXY(retObj[n], x + (TinselV2 ? _TLwidth : 6), y + NM_TBT);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_H156], -1); // Bottom
	MultiSetAniXY(retObj[n], x + (TinselV2 ? _TLwidth : 6), y + (TinselV2 ? _TLheight + 208 + _BLheight + NM_BSY : 143));
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_V104], -1); // Left
	MultiSetAniXY(retObj[n], x + NM_LSX, y + (TinselV2 ? _TLheight : 20));
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_V104], -1); // Right 1
	MultiSetAniXY(retObj[n], x + (TinselV2 ? _TLwidth + 312 + _TRwidth + NM_RSX : 179),
	              y + (TinselV2 ? _TLheight : 20));
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_V104], -1); // Right 2
	MultiSetAniXY(retObj[n], x + (TinselV2 ? _TLwidth + 312 + _TRwidth + NM_SBL : 188),
	              y + (TinselV2 ? _TLheight : 20));
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;

	if (TinselV2) {
		_sliderYpos = _sliderYmin = y + 27;
		_sliderYmax = y + 273;

		retObj[n++] = _slideObject = AddObject(&pfilm->reels[IX_SLIDE], -1);
		MultiSetAniXY(_slideObject,
		              x + _TLwidth + 320 + _TRwidth - NM_BG_POS_X + NM_BG_SIZ_X - 2,
		              _sliderYpos);
		MultiSetZPosition(_slideObject, Z_INV_MFRAME);
	} else {
		_sliderYpos = _sliderYmin = y + 9;
		_sliderYmax = y + 134;
		AddEWSlider(&retObj[n++], pfilm);
	}

	return n;
}

/**
 * Construct an inventory window - either a standard one, with
 * background, slider and icons, or a re-sizing window.
 */
void Dialogs::ConstructInventory(InventoryType filling) {
	int eH, eV; // Extra width and height
	int n = 0;  // Index into object array
	int zpos;   // Z-position of frame
	int invX = _invD[_activeInv].inventoryX;
	int invY = _invD[_activeInv].inventoryY;
	OBJECT **retObj;
	const FILM *pfilm;

	// Select the object array to use
	if (filling == FULL || filling == CONF) {
		retObj = _objArray; // Standard window
		zpos = Z_INV_MFRAME;
	} else {
		retObj = _dispObjArray; // Re-sizing window
		zpos = Z_INV_RFRAME;
	}

	// Dispose of anything it may be replacing
	for (int i = 0; i < MAX_WCOMP; i++) {
		if (retObj[i] != NULL) {
			MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), retObj[i]);
			retObj[i] = nullptr;
		}
	}

	// Get the frame's data
	pfilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

	// Standard window is of granular dimensions
	if (filling == FULL) {
		// Round-up/down to nearest number of icons
		if (_SuppH > ITEM_WIDTH / 2)
			_invD[_activeInv].NoofHicons++;
		if (_SuppV > ITEM_HEIGHT / 2)
			_invD[_activeInv].NoofVicons++;
		_SuppH = _SuppV = 0;
	}

	// Extra width and height
	eH = (_invD[_activeInv].NoofHicons - 1) * (ITEM_WIDTH + I_SEPARATION) + _SuppH;
	eV = (_invD[_activeInv].NoofVicons - 1) * (ITEM_HEIGHT + I_SEPARATION) + _SuppV;

	// Which window frame corners to use
	if (TinselV2 && (_activeInv == INV_CONV)) {
		_TL = IX_TL;
		_TR = IX2_TR4;
		_BL = IX_BL;
		_BR = IX_RBR;
	} else if ((filling == FULL) && (_activeInv != INV_CONV)) {
		_TL = IX_TL;
		_TR = IX_TR;
		_BL = IX_BL;
		_BR = IX_BR;
	} else {
		_TL = IX_RTL;
		_TR = IX_RTR;
		_BL = IX_BL;
		_BR = IX_RBR;
	}

	// Draw the four corners
	retObj[n] = AddObject(&pfilm->reels[_TL], _TL);
	MultiSetAniXY(retObj[n], invX, invY);
	MultiSetZPosition(retObj[n], zpos);
	n++;
	retObj[n] = AddObject(&pfilm->reels[_TR], _TR);
	MultiSetAniXY(retObj[n], invX + _TLwidth + eH, invY);
	MultiSetZPosition(retObj[n], zpos);
	n++;
	retObj[n] = AddObject(&pfilm->reels[_BL], _BL);
	MultiSetAniXY(retObj[n], invX, invY + _TLheight + eV);
	MultiSetZPosition(retObj[n], zpos);
	n++;
	retObj[n] = AddObject(&pfilm->reels[_BR], _BR);
	MultiSetAniXY(retObj[n], invX + _TLwidth + eH, invY + _TLheight + eV);
	MultiSetZPosition(retObj[n], zpos);
	n++;

	// Draw extra Top and bottom parts
	if (_invD[_activeInv].NoofHicons > 1) {
		// Top side
		retObj[n] = AddObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
		MultiSetAniXY(retObj[n], invX + _TLwidth, invY + NM_TBT);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Bottom of header box
		if (filling == FULL) {
			if (TinselV2) {
				retObj[n] = AddObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
				MultiSetAniXY(retObj[n], invX + _TLwidth, invY + NM_TBB);
				MultiSetZPosition(retObj[n], zpos);
				n++;
			} else {
				retObj[n] = AddObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
				MultiSetAniXY(retObj[n], invX + _TLwidth, invY + M_TBB + 1);
				MultiSetZPosition(retObj[n], zpos);
				n++;

				// Extra bits for conversation - hopefully temporary
				if (_activeInv == INV_CONV) {
					retObj[n] = AddObject(&pfilm->reels[IX_H26], -1);
					MultiSetAniXY(retObj[n], invX + _TLwidth - 2, invY + M_TBB + 1);
					MultiSetZPosition(retObj[n], zpos);
					n++;

					retObj[n] = AddObject(&pfilm->reels[IX_H52], -1);
					MultiSetAniXY(retObj[n], invX + eH - 10, invY + M_TBB + 1);
					MultiSetZPosition(retObj[n], zpos);
					n++;
				}
			}
		}

		// Bottom side
		retObj[n] = AddObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
		MultiSetAniXY(retObj[n], invX + _TLwidth, invY + _TLheight + eV + _BLheight + NM_BSY);

		MultiSetZPosition(retObj[n], zpos);
		n++;
	}
	if (_SuppH) {
		int offx = _TLwidth + eH - (TinselV2 ? ITEM_WIDTH + I_SEPARATION : 26);
		if (offx < _TLwidth) // Not too far!
			offx = _TLwidth;

		// Top side extra
		retObj[n] = AddObject(&pfilm->reels[IX_H26], -1);
		MultiSetAniXY(retObj[n], invX + offx, invY + NM_TBT);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Bottom side extra
		retObj[n] = AddObject(&pfilm->reels[IX_H26], -1);
		MultiSetAniXY(retObj[n], invX + offx, invY + _TLheight + eV + _BLheight + NM_BSY);

		MultiSetZPosition(retObj[n], zpos);
		n++;
	}

	// Draw extra side parts
	if (_invD[_activeInv].NoofVicons > 1) {
		// Left side
		retObj[n] = AddObject(&pfilm->reels[vFillers[_invD[_activeInv].NoofVicons - 2]], -1);
		MultiSetAniXY(retObj[n], invX + NM_LSX, invY + _TLheight);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Left side of scroll bar
		if (filling == FULL && _activeInv != INV_CONV) {
			retObj[n] = AddObject(&pfilm->reels[vFillers[_invD[_activeInv].NoofVicons - 2]], -1);
			if (TinselV2)
				MultiSetAniXY(retObj[n], invX + _TLwidth + eH + _TRwidth + NM_SBL, invY + _TLheight);
			else
				MultiSetAniXY(retObj[n], invX + _TLwidth + eH + M_SBL + 1, invY + _TLheight);
			MultiSetZPosition(retObj[n], zpos);
			n++;
		}

		// Right side
		retObj[n] = AddObject(&pfilm->reels[vFillers[_invD[_activeInv].NoofVicons - 2]], -1);
		MultiSetAniXY(retObj[n], invX + _TLwidth + eH + _TRwidth + NM_RSX, invY + _TLheight);
		MultiSetZPosition(retObj[n], zpos);
		n++;
	}
	if (_SuppV) {
		int offy = _TLheight + eV - (TinselV2 ? ITEM_HEIGHT + I_SEPARATION : 26);
		int minAmount = TinselV2 ? 20 : 5;
		if (offy < minAmount)
			offy = minAmount;

		// Left side extra
		retObj[n] = AddObject(&pfilm->reels[IX_V26], -1);
		MultiSetAniXY(retObj[n], invX + NM_LSX, invY + offy);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Right side extra
		retObj[n] = AddObject(&pfilm->reels[IX_V26], -1);
		MultiSetAniXY(retObj[n], invX + _TLwidth + eH + _TRwidth + NM_RSX, invY + offy);
		MultiSetZPosition(retObj[n], zpos);
		n++;
	}

	OBJECT **rect, **title;

	// Draw background, slider and icons
	if (TinselV2 && (filling != EMPTY)) {
		AddBackground(&retObj[n++], eH, eV);
		AddTitle(&retObj[n++], eH);
	}

	if (filling == FULL) {
		if (!TinselV2) {
			rect = &retObj[n++];
			title = &retObj[n++];

			AddBackground(rect, title, eH, eV, FROM_HANDLE);
		}

		if (_activeInv == INV_CONV) {
			_slideObject = nullptr;

			if (TinselV2) {
				// !!!!! MAGIC NUMBER ALERT !!!!!
				// Make sure it's big enough for the heading
				if (MultiLeftmost(retObj[n - 1]) < _invD[INV_CONV].inventoryX + 10) {
					_invD[INV_CONV].NoofHicons++;
					ConstructInventory(FULL);
				}
			}
		} else if (_invD[_activeInv].NoofItems > _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons) {
			_sliderYmin = _TLheight - (TinselV2 ? 1 : 2);
			_sliderYmax = _TLheight + eV + (TinselV2 ? 12 : 10);
			AddSlider(&retObj[n++], pfilm);
		}

		FillInInventory();
	} else if (filling == CONF) {
		if (!TinselV2) {
			rect = &retObj[n++];
			title = &retObj[n++];

			AddBackground(rect, title, eH, eV, FROM_STRING);
			if (cd.bExtraWin)
				n += AddExtraWindow(invX, invY, &retObj[n]);
		} else {
			if (cd.bExtraWin)
				AddExtraWindow(invX, invY, &retObj[n]);
		}

		AddBoxes(true);
	}

	assert(n < MAX_WCOMP); // added more parts than we can handle!

	// Reposition returns true if needs to move
	if (_invD[_activeInv].bMoveable && filling == FULL && RePosition()) {
		ConstructInventory(FULL);
	}
}

/**
 * Call this when drawing a 'FULL', movable inventory. Checks that the
 * position of the Translucent object is within limits. If it isn't,
 * adjusts the x/y position of the current inventory and returns true.
 */
bool Dialogs::RePosition() {
	int p;
	bool bMoveitMoveit = false;

	assert(_rectObject); // no recangle object!

	// Test for off-screen horizontally
	p = MultiLeftmost(_rectObject);
	if (p > MAXLEFT) {
		// Too far to the right
		_invD[_activeInv].inventoryX += MAXLEFT - p;
		bMoveitMoveit = true; // I like to....
	} else {
		// Too far to the left?
		p = MultiRightmost(_rectObject);
		if (p < MINRIGHT) {
			_invD[_activeInv].inventoryX += MINRIGHT - p;
			bMoveitMoveit = true; // I like to....
		}
	}

	// Test for off-screen vertically
	p = MultiHighest(_rectObject);
	if (p < MINTOP) {
		// Too high
		_invD[_activeInv].inventoryY += MINTOP - p;
		bMoveitMoveit = true; // I like to....
	} else if (p > MAXTOP) {
		// Too low
		_invD[_activeInv].inventoryY += MAXTOP - p;
		bMoveitMoveit = true; // I like to....
	}

	return bMoveitMoveit;
}

/**************************************************************************/
/***/
/**************************************************************************/

/**
 * Get the cursor's reel, poke in the background palette,
 * and customise the cursor.
 */
void Dialogs::AlterCursor(int num) {
	const FREEL *pfreel;
	IMAGE *pim;

	// Get pointer to image
	pim = _vm->_cursor->GetImageFromFilm(_hWinParts, num, &pfreel);

	// Poke in the background palette
	pim->hImgPal = TO_32(_vm->_bg->BgPal());

	_vm->_cursor->SetTempCursor(FROM_32(pfreel->script));
}

/**
 * InvCursor
 */
void Dialogs::InvCursor(InvCursorFN fn, int CurX, int CurY) {
	int area; // The part of the window the cursor is over
	bool restoreMain = false;

	// If currently dragging, don't be messing about with the cursor shape
	if (_invDragging != ID_NONE)
		return;

	switch (fn) {
	case IC_DROP:
		_invCursor = IC_NORMAL;
		InvCursor(IC_AREA, CurX, CurY);
		break;

	case IC_AREA:
		area = InvArea(CurX, CurY);

		// Check for POINTED events
		if (_activeInv == INV_CONF)
			InvBoxes(area == I_BODY, CurX, CurY);
		else
			InvLabels(area == I_BODY, CurX, CurY);

		// No cursor trails while within inventory window
		if (area == I_NOTIN)
			_vm->_cursor->UnHideCursorTrails();
		else
			_vm->_cursor->HideCursorTrails();

		switch (area) {
		case I_NOTIN:
			restoreMain = true;
			break;

		case I_TLEFT:
		case I_BRIGHT:
			if (!_invD[_activeInv].resizable)
				restoreMain = true;
			else if (_invCursor != IC_DR) {
				AlterCursor(IX_CURDD);
				_invCursor = IC_DR;
			}
			break;

		case I_TRIGHT:
		case I_BLEFT:
			if (!_invD[_activeInv].resizable)
				restoreMain = true;
			else if (_invCursor != IC_UR) {
				AlterCursor(IX_CURDU);
				_invCursor = IC_UR;
			}
			break;

		case I_TOP:
		case I_BOTTOM:
			if (!_invD[_activeInv].resizable) {
				restoreMain = true;
				break;
			}
			if (_invCursor != IC_TB) {
				AlterCursor(IX_CURUD);
				_invCursor = IC_TB;
			}
			break;

		case I_LEFT:
		case I_RIGHT:
			if (!_invD[_activeInv].resizable)
				restoreMain = true;
			else if (_invCursor != IC_LR) {
				AlterCursor(IX_CURLR);
				_invCursor = IC_LR;
			}
			break;

		case I_UP:
		case I_SLIDE_UP:
		case I_DOWN:
		case I_SLIDE_DOWN:
		case I_SLIDE:
		case I_HEADER:
		case I_BODY:
			restoreMain = true;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (restoreMain && _invCursor != IC_NORMAL) {
		_vm->_cursor->RestoreMainCursor();
		_invCursor = IC_NORMAL;
	}
}

/*-------------------------------------------------------------------------*/

/**************************************************************************/
/******************** Conversation specific functions *********************/
/**************************************************************************/

void Dialogs::ConvAction(int index) {
	assert(_activeInv == INV_CONV); // not conv. window!
	PMOVER pMover = TinselV2 ? GetMover(_vm->_actor->GetLeadId()) : NULL;

	switch (index) {
	case INV_NOICON:
		return;

	case INV_CLOSEICON:
		_thisIcon = -1; // Postamble
		break;

	case INV_OPENICON:
		// Store the direction the lead character is facing in when the conversation starts
		if (TinselV2)
			_initialDirection = GetMoverDirection(pMover);
		_thisIcon = -2; // Preamble
		break;

	default:
		_thisIcon = _invD[_activeInv].contents[index];
		break;
	}

	if (!TinselV2)
		RunPolyTinselCode(_thisConvPoly, CONVERSE, PLR_NOEVENT, true);
	else {
		// If the lead's direction has changed for any reason (such as having broken the
		// fourth wall and talked to the screen), reset back to the original direction
		DIRECTION currDirection = GetMoverDirection(pMover);
		if (currDirection != _initialDirection) {
			SetMoverDirection(pMover, _initialDirection);
			SetMoverStanding(pMover);
		}

		if (_thisConvPoly != NOPOLY)
			PolygonEvent(Common::nullContext, _thisConvPoly, CONVERSE, 0, false, 0);
		else
			ActorEvent(Common::nullContext, _thisConvActor, CONVERSE, false, 0);
	}
}

/**
 * Called to specify whether conversation window is going to
 * appear at the top or bottom of the screen.
 * Also to specify which polygon or actor is opening the conversation.
 *
 * Note: ano may (will probably) be set when it's a polygon.
 */
void Dialogs::SetConvDetails(CONV_PARAM fn, HPOLYGON hPoly, int ano) {
	_thisConvFn = fn;
	_thisConvPoly = hPoly;
	_thisConvActor = ano;

	_bMoveOnUnHide = true;

	// Get the Actor Tag's or Tagged Actor's label for the conversation window title
	if (hPoly != NOPOLY) {
		int x, y;
		GetTagTag(hPoly, &_invD[INV_CONV].hInvTitle, &x, &y);
	} else {
		_invD[INV_CONV].hInvTitle = _vm->_actor->GetActorTagHandle(ano);
	}
}

/*-------------------------------------------------------------------------*/

/**
 * Add an icon to the permanent conversation list.
 */
void Dialogs::PermaConvIcon(int icon, bool bEnd) {
	int i;

	// See if it's already there
	for (i = 0; i < _numPermIcons; i++) {
		if (_permIcons[i] == icon)
			break;
	}

	// Add it if it isn't already there
	if (i == _numPermIcons) {
		assert(_numPermIcons < MAX_PERMICONS);

		if (bEnd || !_numEndIcons) {
			// Add it at the end
			_permIcons[_numPermIcons++] = icon;
			if (bEnd)
				_numEndIcons++;
		} else {
			// Insert before end icons
			memmove(&_permIcons[_numPermIcons - _numEndIcons + 1],
			        &_permIcons[_numPermIcons - _numEndIcons],
			        _numEndIcons * sizeof(int));
			_permIcons[_numPermIcons - _numEndIcons] = icon;
			_numPermIcons++;
		}
	}
}

/*-------------------------------------------------------------------------*/

void Dialogs::convPos(int fn) {
	if (fn == CONV_DEF)
		_invD[INV_CONV].inventoryY = 8;
	else if (fn == CONV_BOTTOM)
		_invD[INV_CONV].inventoryY = 150;
}

void Dialogs::ConvPoly(HPOLYGON hPoly) {
	_thisConvPoly = hPoly;
}

int Dialogs::GetIcon() {
	return _thisIcon;
}

void Dialogs::CloseDownConv() {
	if (_inventoryState == ACTIVE_INV && _activeInv == INV_CONV) {
		KillInventory();
	}
}

void Dialogs::HideConversation(bool bHide) {
	int aniX, aniY;
	int i;

	if (_inventoryState == ACTIVE_INV && _activeInv == INV_CONV) {
		if (bHide) {
			// Move all the window and icons off-screen
			for (i = 0; i < MAX_WCOMP && _objArray[i]; i++) {
				MultiAdjustXY(_objArray[i], 2 * SCREEN_WIDTH, 0);
			}
			for (i = 0; i < MAX_ICONS && _iconArray[i]; i++) {
				MultiAdjustXY(_iconArray[i], 2 * SCREEN_WIDTH, 0);
			}

			// Window is hidden
			_InventoryHidden = true;

			// Remove any labels
			InvLabels(false, 0, 0);
		} else {
			// Window is not hidden
			_InventoryHidden = false;

			if (TinselV2 && _ItemsChanged)
				// Just rebuild the whole thing
				ConstructInventory(FULL);
			else {
				// Move it all back on-screen
				for (i = 0; i < MAX_WCOMP && _objArray[i]; i++) {
					MultiAdjustXY(_objArray[i], -2 * SCREEN_WIDTH, 0);
				}

				// Don't flash if items changed. If they have, will be redrawn anyway.
				if (TinselV2 || !_ItemsChanged) {
					for (i = 0; i < MAX_ICONS && _iconArray[i]; i++) {
						MultiAdjustXY(_iconArray[i], -2 * SCREEN_WIDTH, 0);
					}
				}
			}

			if (TinselV2 && _bMoveOnUnHide) {
				/*
				 * First time, position it appropriately
				 */
				int left, center;
				int x, y, deltay;

				// Only do it once per conversation
				_bMoveOnUnHide = false;

				// Current center of the window
				left = MultiLeftmost(_rectObject);
				center = (MultiRightmost(_rectObject) + left) / 2;

				// Get the x-offset for the conversation window
				if (_thisConvActor) {
					int Loffset, Toffset;

					_vm->_actor->GetActorMidTop(_thisConvActor, &x, &y);
					_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
					x -= Loffset;
					y -= Toffset;
				} else {
					x = SCREEN_WIDTH / 2;
					y = SCREEN_BOX_HEIGHT2 / 2;
				}

				// Save old y-position
				deltay = _invD[INV_CONV].inventoryY;

				switch (_thisConvFn) {
				case CONV_TOP:
					_invD[INV_CONV].inventoryY = SysVar(SV_CONV_TOPY);
					break;

				case CONV_BOTTOM:
					_invD[INV_CONV].inventoryY = SysVar(SV_CONV_BOTY);
					break;

				case CONV_DEF:
					_invD[INV_CONV].inventoryY = y - SysVar(SV_CONV_ABOVE_Y);
					break;

				default:
					break;
				}

				// Calculate y change
				deltay = _invD[INV_CONV].inventoryY - deltay;

				// Move it all
				for (i = 0; i < MAX_WCOMP && _objArray[i]; i++) {
					MultiMoveRelXY(_objArray[i], x - center, deltay);
				}
				for (i = 0; i < MAX_ICONS && _iconArray[i]; i++) {
					MultiMoveRelXY(_iconArray[i], x - center, deltay);
				}
				_invD[INV_CONV].inventoryX += x - center;

				/*
				 * Now positioned as worked out
				 * - but it must be in a sensible place
				*/
				if (MultiLeftmost(_rectObject) < SysVar(SV_CONV_MINX))
					x = SysVar(SV_CONV_MINX) - MultiLeftmost(_rectObject);
				else if (MultiRightmost(_rectObject) > SCREEN_WIDTH - SysVar(SV_CONV_MINX))
					x = SCREEN_WIDTH - SysVar(SV_CONV_MINX) - MultiRightmost(_rectObject);
				else
					x = 0;

				if (_thisConvFn == CONV_DEF && MultiHighest(_rectObject) < SysVar(SV_CONV_MINY) && _thisConvActor) {
					int Loffset, Toffset;

					_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
					y = _vm->_actor->GetActorBottom(_thisConvActor) - MultiHighest(_rectObject) +
					    SysVar(SV_CONV_BELOW_Y);
					y -= Toffset;
				} else
					y = 0;

				if (x || y) {
					for (i = 0; i < MAX_WCOMP && _objArray[i]; i++) {
						MultiMoveRelXY(_objArray[i], x, y);
					}
					for (i = 0; i < MAX_ICONS && _iconArray[i]; i++) {
						MultiMoveRelXY(_iconArray[i], x, y);
					}
					_invD[INV_CONV].inventoryX += x;
					_invD[INV_CONV].inventoryY += y;
				}

				/*
				 * Oh shit! We might have gone off the bottom
				 */
				if (MultiLowest(_rectObject) > SCREEN_BOX_HEIGHT2 - SysVar(SV_CONV_MINY)) {
					y = (SCREEN_BOX_HEIGHT2 - SysVar(SV_CONV_MINY)) - MultiLowest(_rectObject);
					for (i = 0; i < MAX_WCOMP && _objArray[i]; i++) {
						MultiMoveRelXY(_objArray[i], 0, y);
					}
					for (i = 0; i < MAX_ICONS && _iconArray[i]; i++) {
						MultiMoveRelXY(_iconArray[i], 0, y);
					}
					_invD[INV_CONV].inventoryY += y;
				}
			}

			_vm->_cursor->GetCursorXY(&aniX, &aniY, false);
			InvLabels(true, aniX, aniY);
		}
	}
}

bool Dialogs::ConvIsHidden() {
	return _InventoryHidden;
}

/**************************************************************************/
/******************* Open and closing functions ***************************/
/**************************************************************************/

/**
 * Start up an inventory window.
 */
void Dialogs::PopUpInventory(int invno) {
	assert(invno == INV_1 || invno == INV_2 || invno == INV_CONV || invno == INV_CONF || invno == INV_MENU); // Trying to open illegal inventory

	if (_inventoryState == IDLE_INV) {
		_reOpenMenu = false; // Better safe than sorry...

		DisableTags(); // Tags disabled during inventory
		if (TinselV2)
			DisablePointing(); // Pointing disabled during inventory

		if (invno == INV_CONV) { // Conversation window?
			if (TinselV2)
				// Quiet please..
				_vm->_pcmMusic->dim(false);

			// Start conversation with permanent contents
			memset(_invD[INV_CONV].contents, 0, MAX_ININV * sizeof(int));
			memcpy(_invD[INV_CONV].contents, _permIcons, _numPermIcons * sizeof(int));
			_invD[INV_CONV].NoofItems = _numPermIcons;
			if (TinselV2)
				_invD[INV_CONV].NoofHicons = _numPermIcons;
			else
				_thisIcon = 0;
		} else if (invno == INV_CONF) { // Configuration window?
			cd.selBox = NOBOX;
			cd.pointBox = NOBOX;
		}

		_activeInv = invno; // The open inventory

		_ItemsChanged = false;        // Nothing changed
		_invDragging = ID_NONE;       // Not dragging
		_inventoryState = ACTIVE_INV; // Inventory actiive
		_InventoryHidden = false;     // Not hidden
		_InventoryMaximised = _invD[_activeInv].bMax;
		if (invno != INV_CONF)        // Configuration window?
			ConstructInventory(FULL); // Draw it up
		else {
			ConstructInventory(CONF); // Draw it up
		}
	}
}

void Dialogs::SetMenuGlobals(CONFINIT *ci) {
	_invD[INV_CONF].MinHicons = _invD[INV_CONF].MaxHicons = _invD[INV_CONF].NoofHicons = ci->h;
	_invD[INV_CONF].MaxVicons = _invD[INV_CONF].MinVicons = _invD[INV_CONF].NoofVicons = ci->v;
	_invD[INV_CONF].inventoryX = ci->x;
	_invD[INV_CONF].inventoryY = ci->y;
	cd.bExtraWin = ci->bExtraWin;
	cd.box = ci->Box;
	cd.NumBoxes = ci->NumBoxes;
	cd.ixHeading = ci->ixHeading;

	if (TinselV2) {
		if ((ci->ixHeading != NO_HEADING) && SysString(ci->ixHeading))
			_invD[INV_MENU].hInvTitle = SysString(ci->ixHeading);
		else
			_invD[INV_MENU].hInvTitle = NO_HEADING;
	}
}

/**
 * PopupConf
 */
void Dialogs::OpenMenu(CONFTYPE menuType) {
	int curX, curY;

	// In the DW 1 demo, don't allow any menu to be opened
	if (TinselV0)
		return;

	if (_inventoryState != IDLE_INV)
		return;

	_invD[INV_CONF].resizable = false;
	_invD[INV_CONF].bMoveable = false;

	switch (menuType) {
	case MAIN_MENU:
		SetMenuGlobals(&ciOption);
		break;

	case SAVE_MENU:
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true); // Show VK when saving a game
		if (!TinselV2)
			_vm->_cursor->SetCursorScreenXY(262, 91);
		SetMenuGlobals(&ciSave);
		cd.editableRgroup = true;
		FirstFile(0);
		break;

	case LOAD_MENU:
		SetMenuGlobals(&ciLoad);
		cd.editableRgroup = false;
		FirstFile(0);
		break;

	case RESTART_MENU:
		if (TinselV2)
			_vm->_cursor->SetCursorScreenXY(360, 153);
		else if (_vm->getLanguage() == Common::JA_JPN)
			_vm->_cursor->SetCursorScreenXY(180, 106);
		else
			_vm->_cursor->SetCursorScreenXY(180, 90);

		SetMenuGlobals(&ciRestart);
		break;

	case SOUND_MENU:
		if (TinselV2)
			_displayedLanguage = TextLanguage();
#if 1
		// FIXME: Hack to setup CONFBOX pointer to data in the global Config object
		if (TinselV2) {
			t2SoundBox[0].ival = &_vm->_config->_musicVolume;
			t2SoundBox[1].ival = &_vm->_config->_soundVolume;
			t2SoundBox[2].ival = &_vm->_config->_voiceVolume;
			t2SoundBox[3].ival = &_vm->_config->_textSpeed;
			t2SoundBox[4].ival = &_vm->_config->_useSubtitles;
		} else {
			t1SoundBox[0].ival = &_vm->_config->_musicVolume;
			t1SoundBox[1].ival = &_vm->_config->_soundVolume;
			t1SoundBox[2].ival = &_vm->_config->_voiceVolume;
		}
#endif
		SetMenuGlobals(&ciSound);
		break;

	case CONTROLS_MENU:
#if 1
		// FIXME: Hack to setup CONFBOX pointer to data in the global Config object
		controlBox[0].ival = &_vm->_config->_dclickSpeed;
		controlBox[2].ival = &_vm->_config->_swapButtons;
#endif
		SetMenuGlobals(&ciControl);
		break;

	case QUIT_MENU:
		if (TinselV2)
			_vm->_cursor->SetCursorScreenXY(360, 153);
		else if (_vm->getLanguage() == Common::JA_JPN)
			_vm->_cursor->SetCursorScreenXY(180, 106);
		else
			_vm->_cursor->SetCursorScreenXY(180, 90);

		SetMenuGlobals(&ciQuit);
		break;

	case HOPPER_MENU1:
		PrimeSceneHopper();
		SetMenuGlobals(&ciHopper1);
		cd.editableRgroup = false;
		RememberChosenScene();
		FirstScene(0);
		break;

	case HOPPER_MENU2:
		SetMenuGlobals(&ciHopper2);
		cd.editableRgroup = false;
		SetChosenScene();
		FirstEntry(0);
		break;

	case SUBTITLES_MENU: {
		int hackOffset = 0;
		if (_vm->getFeatures() & GF_USE_3FLAGS) {
			hackOffset = 3;
			ciSubtitles.v = 6;
			ciSubtitles.Box = subtitlesBox3Flags;
			ciSubtitles.NumBoxes = ARRAYSIZE(subtitlesBox3Flags);
		} else if (_vm->getFeatures() & GF_USE_4FLAGS) {
			hackOffset = 4;
			ciSubtitles.v = 6;
			ciSubtitles.Box = subtitlesBox4Flags;
			ciSubtitles.NumBoxes = ARRAYSIZE(subtitlesBox4Flags);
		} else if (_vm->getFeatures() & GF_USE_5FLAGS) {
			hackOffset = 5;
			ciSubtitles.v = 6;
			ciSubtitles.Box = subtitlesBox5Flags;
			ciSubtitles.NumBoxes = ARRAYSIZE(subtitlesBox5Flags);
		} else {
			hackOffset = 0;
			ciSubtitles.v = 3;
			ciSubtitles.Box = subtitlesBox;
			ciSubtitles.NumBoxes = ARRAYSIZE(subtitlesBox);
		}
#if 1
		// FIXME: Hack to setup CONFBOX pointer to data in the global Config object
		ciSubtitles.Box[hackOffset].ival = &_vm->_config->_textSpeed;
		ciSubtitles.Box[hackOffset + 1].ival = &_vm->_config->_useSubtitles;
#endif

		SetMenuGlobals(&ciSubtitles);
	} break;

	case TOP_WINDOW:
		SetMenuGlobals(&ciTopWin);
		_activeInv = INV_CONF;
		ConstructInventory(CONF); // Draw it up
		_inventoryState = BOGUS_INV;
		return;

	default:
		return;
	}

	if (_heldItem != INV_NOICON)
		_vm->_cursor->DelAuxCursor(); // no longer aux cursor

	PopUpInventory(INV_CONF);

	// Make initial box selections if appropriate
	if (menuType == SAVE_MENU || menuType == LOAD_MENU || menuType == HOPPER_MENU1 || menuType == HOPPER_MENU2)
		Select(0, false);
	else if (menuType == SUBTITLES_MENU) {
		if (_vm->getFeatures() & GF_USE_3FLAGS) {
			// VERY quick dirty bodges
			if (_vm->_config->_language == TXT_FRENCH)
				Select(0, false);
			else if (_vm->_config->_language == TXT_GERMAN)
				Select(1, false);
			else
				Select(2, false);
		} else if (_vm->getFeatures() & GF_USE_4FLAGS) {
			Select(_vm->_config->_language - 1, false);
		} else if (_vm->getFeatures() & GF_USE_5FLAGS) {
			Select(_vm->_config->_language, false);
		}
	}

	_vm->_cursor->GetCursorXY(&curX, &curY, false);
	InvCursor(IC_AREA, curX, curY);
}

/**
 * Close down an inventory window.
 */
void Dialogs::KillInventory() {
	if (_objArray[0] != NULL) {
		DumpObjArray();
		DumpDobjArray();
		DumpIconArray();
	}

	if (_inventoryState == ACTIVE_INV) {
		EnableTags();
		if (TinselV2)
			EnablePointing();

		_invD[_activeInv].bMax = _InventoryMaximised;

		_vm->_cursor->UnHideCursorTrails();
		_vm->divertKeyInput(NULL);
	}

	_inventoryState = IDLE_INV;

	if (_reOpenMenu) {
		_reOpenMenu = false;
		OpenMenu(MAIN_MENU);

		// Write config changes
		_vm->_config->writeToDisk();

	} else if (_activeInv == INV_CONF)
		InventoryIconCursor(false);

	if (TinselV2)
		// Pump up the volume
		if (_activeInv == INV_CONV)
			_vm->_pcmMusic->unDim(false);

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false); // Hide VK after save dialog closes
}

void Dialogs::CloseInventory() {
	// If not active, ignore this
	if (_inventoryState != ACTIVE_INV)
		return;

	// If hidden, a conversation action is still underway - ignore this
	if (_InventoryHidden)
		return;

	// If conversation, this is a closeing event
	if (_activeInv == INV_CONV)
		ConvAction(INV_CLOSEICON);

	KillInventory();

	_vm->_cursor->RestoreMainCursor();
}

/**************************************************************************/
/*************** Drag stuff - Resizing and moving window ******************/
/**************************************************************************/

/**
 * Appears to find the nearest entry in slideStuff[] to the supplied
 * y-coordinate.
 */
int Dialogs::NearestSlideY(int fity) {
	int nearDist = 1000;
	int thisDist;
	int nearI = 0; // Index of nearest fit
	int i = 0;

	do {
		thisDist = ABS(_slideStuff[i].y - fity);
		if (thisDist < nearDist) {
			nearDist = thisDist;
			nearI = i;
		}
	} while (_slideStuff[++i].n != -1);
	return nearI;
}

/**
 * Gets called at the start and end of a drag on the slider, and upon
 * y-movement during such a drag.
 */
void Dialogs::SlideSlider(int y, SSFN fn) {
	static int newY = 0, lasti = 0; // FIXME: Avoid non-const global vars
	int gotoY, ati;

	// Only do this if there's a slider
	if (!_slideObject)
		return;

	switch (fn) {
	case S_START: // Start of a drag on the slider
		newY = _sliderYpos;
		lasti = NearestSlideY(_sliderYpos);
		break;

	case S_SLIDE:        // Y-movement during drag
		newY = newY + y; // New y-position

		if (newY < _sliderYmin)
			gotoY = _sliderYmin; // Above top limit
		else if (newY > _sliderYmax)
			gotoY = _sliderYmax; // Below bottom limit
		else
			gotoY = newY; // Hunky-Dory

		// Move slider to new position
		MultiMoveRelXY(_slideObject, 0, gotoY - _sliderYpos);
		_sliderYpos = gotoY;

		// Re-draw icons if necessary
		ati = NearestSlideY(_sliderYpos);
		if (ati != lasti) {
			_invD[_activeInv].FirstDisp = _slideStuff[ati].n;
			assert(_invD[_activeInv].FirstDisp >= 0); // negative first displayed
			_ItemsChanged = true;
			lasti = ati;
		}
		break;

	case S_END: // End of a drag on the slider
		// Draw icons from new start icon
		ati = NearestSlideY(_sliderYpos);
		_invD[_activeInv].FirstDisp = _slideStuff[ati].n;
		_ItemsChanged = true;
		break;

	default:
		break;
	}
}

/**
 * Gets called at the start and end of a drag on the slider, and upon
 * y-movement during such a drag.
 */
void Dialogs::SlideCSlider(int y, SSFN fn) {
	static int newY = 0; // FIXME: Avoid non-const global vars
	int gotoY;
	int fc;

	// Only do this if there's a slider
	if (!_slideObject)
		return;

	switch (fn) {
	case S_START: // Start of a drag on the slider
		newY = _sliderYpos;
		break;

	case S_SLIDE:        // Y-movement during drag
		newY = newY + y; // New y-position

		if (newY < _sliderYmin)
			gotoY = _sliderYmin; // Above top limit
		else if (newY > _sliderYmax)
			gotoY = _sliderYmax; // Below bottom limit
		else
			gotoY = newY; // Hunky-Dory

		// Move slider to new position
		if (TinselV2)
			MultiMoveRelXY(_slideObject, 0, gotoY - _sliderYpos);
		_sliderYpos = gotoY;

		fc = cd.extraBase;

		if ((cd.box == saveBox || cd.box == loadBox))
			FirstFile((_sliderYpos - _sliderYmin) * (MAX_SAVED_FILES - NUM_RGROUP_BOXES) /
			          (_sliderYmax - _sliderYmin));
		else if (cd.box == hopperBox1)
			FirstScene((_sliderYpos - _sliderYmin) * (_numScenes - NUM_RGROUP_BOXES) / sliderRange);
		else if (cd.box == hopperBox2)
			FirstEntry((_sliderYpos - _sliderYmin) * (_numEntries - NUM_RGROUP_BOXES) / sliderRange);

		// If extraBase has changed...
		if (fc != cd.extraBase) {
			AddBoxes(false);
			fc -= cd.extraBase;
			cd.selBox += fc;

			// Ensure within legal limits
			if (cd.selBox < 0)
				cd.selBox = 0;
			else if (cd.selBox >= NUM_RGROUP_BOXES)
				cd.selBox = NUM_RGROUP_BOXES - 1;

			Select(cd.selBox, true);
		}
		break;

	case S_END: // End of a drag on the slider
		break;

	default:
		break;
	}
}

/**
 * Gets called at the start and end of a drag on a mixing desk slider,
 * and upon x-movement during such a drag.
 */
void Dialogs::SlideMSlider(int x, SSFN fn) {
	static int newX = 0; // FIXME: Avoid non-const global vars
	int gotoX;
	int index, i;

	if (fn == S_END || fn == S_TIMEUP || fn == S_TIMEDN)
		;
	else if (!(cd.selBox & IS_SLIDER))
		return;

	// Work out the indices
	index = cd.selBox & ~IS_MASK;
	for (i = 0; i < _numMdSlides; i++)
		if (_mdSlides[i].num == index)
			break;
	assert(i < _numMdSlides);

	switch (fn) {
	case S_START: // Start of a drag on the slider
		// can use index as a throw-away value
		GetAniPosition(_mdSlides[i].obj, &newX, &index);
		_lX = _sX = newX;
		break;

	case S_SLIDE: // X-movement during drag
		if (x == 0)
			return;

		newX = newX + x; // New x-position

		if (newX < _mdSlides[i].min)
			gotoX = _mdSlides[i].min; // Below bottom limit
		else if (newX > _mdSlides[i].max)
			gotoX = _mdSlides[i].max; // Above top limit
		else
			gotoX = newX; // Hunky-Dory

		// Move slider to new position
		MultiMoveRelXY(_mdSlides[i].obj, gotoX - _sX, 0);
		_sX = gotoX;

		if (_lX != _sX) {
			*cd.box[index].ival = (_sX - _mdSlides[i].min) * cd.box[index].w / SLIDE_RANGE;
			if (cd.box[index].boxFunc == MUSICVOL)
				_vm->_music->SetMidiVolume(*cd.box[index].ival);
#ifdef MAC_OPTIONS
			if (cd.box[index].boxFunc == MASTERVOL)
				SetSystemVolume(*cd.box[index].ival);

			if (cd.box[index].boxFunc == SAMPVOL)
				SetSampleVolume(*cd.box[index].ival);
#endif
			_lX = _sX;
		}
		break;

	case S_TIMEUP:
	case S_TIMEDN:
		gotoX = SLIDE_RANGE * (*cd.box[index].ival) / cd.box[index].w;
		MultiSetAniX(_mdSlides[i].obj, _mdSlides[i].min + gotoX);

		if (cd.box[index].boxFunc == MUSICVOL)
			_vm->_music->SetMidiVolume(*cd.box[index].ival);
#ifdef MAC_OPTIONS
		if (cd.box[index].boxFunc == MASTERVOL)
			SetSystemVolume(*cd.box[index].ival);

		if (cd.box[index].boxFunc == SAMPVOL)
			SetSampleVolume(*cd.box[index].ival);
#endif
		break;

	case S_END:          // End of a drag on the slider
		AddBoxes(false); // Might change position slightly
		if (_activeInv == INV_CONF && cd.box == subtitlesBox)
			Select(_vm->_config->_language, false);
		break;

	default:
		break;
	}
}

/**
 * Called from ChangeingSize() during re-sizing.
 */
void Dialogs::GettingTaller() {
	if (_SuppV) {
		_yChange += _SuppV;
		if (_yCompensate == 'T')
			_invD[_activeInv].inventoryY += _SuppV;
		_SuppV = 0;
	}
	while (_yChange > (ITEM_HEIGHT + 1) && _invD[_activeInv].NoofVicons < _invD[_activeInv].MaxVicons) {
		_yChange -= (ITEM_HEIGHT + 1);
		_invD[_activeInv].NoofVicons++;
		if (_yCompensate == 'T')
			_invD[_activeInv].inventoryY -= (ITEM_HEIGHT + 1);
	}
	if (_invD[_activeInv].NoofVicons < _invD[_activeInv].MaxVicons) {
		_SuppV = _yChange;
		_yChange = 0;
		if (_yCompensate == 'T')
			_invD[_activeInv].inventoryY -= _SuppV;
	}
}

/**
 * Called from ChangeingSize() during re-sizing.
 */
void Dialogs::GettingShorter() {
	int StartNvi = _invD[_activeInv].NoofVicons;
	int StartUv = _SuppV;

	if (_SuppV) {
		_yChange += (_SuppV - (ITEM_HEIGHT + 1));
		_invD[_activeInv].NoofVicons++;
		_SuppV = 0;
	}
	while (_yChange < -(ITEM_HEIGHT + 1) && _invD[_activeInv].NoofVicons > _invD[_activeInv].MinVicons) {
		_yChange += (ITEM_HEIGHT + 1);
		_invD[_activeInv].NoofVicons--;
	}
	if (_invD[_activeInv].NoofVicons > _invD[_activeInv].MinVicons && _yChange) {
		_SuppV = (ITEM_HEIGHT + 1) + _yChange;
		_invD[_activeInv].NoofVicons--;
		_yChange = 0;
	}
	if (_yCompensate == 'T')
		_invD[_activeInv].inventoryY += (ITEM_HEIGHT + 1) * (StartNvi - _invD[_activeInv].NoofVicons) - (_SuppV - StartUv);
}

/**
 * Called from ChangeingSize() during re-sizing.
 */
void Dialogs::GettingWider() {
	int StartNhi = _invD[_activeInv].NoofHicons;
	int StartUh = _SuppH;

	if (_SuppH) {
		_xChange += _SuppH;
		_SuppH = 0;
	}
	while (_xChange > (ITEM_WIDTH + 1) && _invD[_activeInv].NoofHicons < _invD[_activeInv].MaxHicons) {
		_xChange -= (ITEM_WIDTH + 1);
		_invD[_activeInv].NoofHicons++;
	}
	if (_invD[_activeInv].NoofHicons < _invD[_activeInv].MaxHicons) {
		_SuppH = _xChange;
		_xChange = 0;
	}
	if (_xCompensate == 'L')
		_invD[_activeInv].inventoryX += (ITEM_WIDTH + 1) * (StartNhi - _invD[_activeInv].NoofHicons) - (_SuppH - StartUh);
}

/**
 * Called from ChangeingSize() during re-sizing.
 */
void Dialogs::GettingNarrower() {
	int StartNhi = _invD[_activeInv].NoofHicons;
	int StartUh = _SuppH;

	if (_SuppH) {
		_xChange += (_SuppH - (ITEM_WIDTH + 1));
		_invD[_activeInv].NoofHicons++;
		_SuppH = 0;
	}
	while (_xChange < -(ITEM_WIDTH + 1) && _invD[_activeInv].NoofHicons > _invD[_activeInv].MinHicons) {
		_xChange += (ITEM_WIDTH + 1);
		_invD[_activeInv].NoofHicons--;
	}
	if (_invD[_activeInv].NoofHicons > _invD[_activeInv].MinHicons && _xChange) {
		_SuppH = (ITEM_WIDTH + 1) + _xChange;
		_invD[_activeInv].NoofHicons--;
		_xChange = 0;
	}
	if (_xCompensate == 'L')
		_invD[_activeInv].inventoryX += (ITEM_WIDTH + 1) * (StartNhi - _invD[_activeInv].NoofHicons) - (_SuppH - StartUh);
}

/**
 * Called from Xmovement()/Ymovement() during re-sizing.
 */
void Dialogs::ChangeingSize() {
	/* Make it taller or shorter if necessary. */
	if (_yChange > 0)
		GettingTaller();
	else if (_yChange < 0)
		GettingShorter();

	/* Make it wider or narrower if necessary. */
	if (_xChange > 0)
		GettingWider();
	else if (_xChange < 0)
		GettingNarrower();

	ConstructInventory(EMPTY);
}

/**
 * Called from cursor module when cursor moves while inventory is up.
 */
void Dialogs::Xmovement(int x) {
	int aniX, aniY;
	int i;

	if (x && _objArray[0] != NULL) {
		switch (_invDragging) {
		case ID_MOVE:
			GetAniPosition(_objArray[0], &_invD[_activeInv].inventoryX, &aniY);
			_invD[_activeInv].inventoryX += x;
			MultiSetAniX(_objArray[0], _invD[_activeInv].inventoryX);
			for (i = 1; i < MAX_WCOMP && _objArray[i]; i++)
				MultiMoveRelXY(_objArray[i], x, 0);
			for (i = 0; i < MAX_ICONS && _iconArray[i]; i++)
				MultiMoveRelXY(_iconArray[i], x, 0);
			break;

		case ID_LEFT:
		case ID_TLEFT:
		case ID_BLEFT:
			_xChange -= x;
			ChangeingSize();
			break;

		case ID_RIGHT:
		case ID_TRIGHT:
		case ID_BRIGHT:
			_xChange += x;
			ChangeingSize();
			break;

		case ID_NONE:
			_vm->_cursor->GetCursorXY(&aniX, &aniY, false);
			InvCursor(IC_AREA, aniX, aniY);
			break;

		case ID_MDCONT:
			SlideMSlider(x, S_SLIDE);
			break;

		default:
			break;
		}
	}
}

/**
 * Called from cursor module when cursor moves while inventory is up.
 */
void Dialogs::Ymovement(int y) {
	int aniX, aniY;
	int i;

	if (y && _objArray[0] != NULL) {
		switch (_invDragging) {
		case ID_MOVE:
			GetAniPosition(_objArray[0], &aniX, &_invD[_activeInv].inventoryY);
			_invD[_activeInv].inventoryY += y;
			MultiSetAniY(_objArray[0], _invD[_activeInv].inventoryY);
			for (i = 1; i < MAX_WCOMP && _objArray[i]; i++)
				MultiMoveRelXY(_objArray[i], 0, y);
			for (i = 0; i < MAX_ICONS && _iconArray[i]; i++)
				MultiMoveRelXY(_iconArray[i], 0, y);
			break;

		case ID_SLIDE:
			SlideSlider(y, S_SLIDE);
			break;

		case ID_CSLIDE:
			SlideCSlider(y, S_SLIDE);
			break;

		case ID_BOTTOM:
		case ID_BLEFT:
		case ID_BRIGHT:
			_yChange += y;
			ChangeingSize();
			break;

		case ID_TOP:
		case ID_TLEFT:
		case ID_TRIGHT:
			_yChange -= y;
			ChangeingSize();
			break;

		case ID_NONE:
			_vm->_cursor->GetCursorXY(&aniX, &aniY, false);
			InvCursor(IC_AREA, aniX, aniY);
			break;

		default:
			break;
		}
	}
}

/**
 * Called when a drag is commencing.
 */
void Dialogs::InvDragStart() {
	int curX, curY; // cursor's animation position

	_vm->_cursor->GetCursorXY(&curX, &curY, false);

	/*
	 * Do something different for Save/Restore screens
	 */
	if (_activeInv == INV_CONF) {
		int whichbox;

		whichbox = WhichMenuBox(curX, curY, true);

		if (whichbox == IB_SLIDE) {
			_invDragging = ID_CSLIDE;
			SlideCSlider(0, S_START);
		} else if (whichbox > 0 && (whichbox & IS_MASK)) {
			_invDragging = ID_MDCONT; // Mixing desk control
			cd.selBox = whichbox;
			SlideMSlider(0, S_START);
		}
		return;
	}

	/*
	 * Normal operation
	 */
	switch (InvArea(curX, curY)) {
	case I_HEADER:
		if (_invD[_activeInv].bMoveable) {
			_invDragging = ID_MOVE;
		}
		break;

	case I_SLIDE:
		_invDragging = ID_SLIDE;
		SlideSlider(0, S_START);
		break;

	case I_BOTTOM:
		if (_invD[_activeInv].resizable) {
			_yChange = 0;
			_invDragging = ID_BOTTOM;
			_yCompensate = 'B';
		}
		break;

	case I_TOP:
		if (_invD[_activeInv].resizable) {
			_yChange = 0;
			_invDragging = ID_TOP;
			_yCompensate = 'T';
		}
		break;

	case I_LEFT:
		if (_invD[_activeInv].resizable) {
			_xChange = 0;
			_invDragging = ID_LEFT;
			_xCompensate = 'L';
		}
		break;

	case I_RIGHT:
		if (_invD[_activeInv].resizable) {
			_xChange = 0;
			_invDragging = ID_RIGHT;
			_xCompensate = 'R';
		}
		break;

	case I_TLEFT:
		if (_invD[_activeInv].resizable) {
			_yChange = 0;
			_yCompensate = 'T';
			_xChange = 0;
			_xCompensate = 'L';
			_invDragging = ID_TLEFT;
		}
		break;

	case I_TRIGHT:
		if (_invD[_activeInv].resizable) {
			_yChange = 0;
			_yCompensate = 'T';
			_xChange = 0;
			_xCompensate = 'R';
			_invDragging = ID_TRIGHT;
		}
		break;

	case I_BLEFT:
		if (_invD[_activeInv].resizable) {
			_yChange = 0;
			_yCompensate = 'B';
			_xChange = 0;
			_xCompensate = 'L';
			_invDragging = ID_BLEFT;
		}
		break;

	case I_BRIGHT:
		if (_invD[_activeInv].resizable) {
			_yChange = 0;
			_yCompensate = 'B';
			_xChange = 0;
			_xCompensate = 'R';
			_invDragging = ID_BRIGHT;
		}
		break;

	default:
		break;
	}
}

/**
 * Called when a drag is over.
 */
void Dialogs::InvDragEnd() {
	int curX, curY; // cursor's animation position

	_vm->_cursor->GetCursorXY(&curX, &curY, false);

	if (_invDragging != ID_NONE) {
		if (_invDragging == ID_SLIDE) {
			SlideSlider(0, S_END);
		} else if (_invDragging == ID_CSLIDE) {
			; // No action
		} else if (_invDragging == ID_MDCONT) {
			SlideMSlider(0, S_END);
		} else if (_invDragging == ID_MOVE) {
			; // No action
		} else {
			// Were re-sizing. Redraw the whole thing.
			DumpDobjArray();
			DumpObjArray();
			ConstructInventory(FULL);

			// If this was the maximised, it no longer is!
			if (_InventoryMaximised) {
				_InventoryMaximised = false;
				_invD[_activeInv].otherX = _invD[_activeInv].inventoryX;
				_invD[_activeInv].otherY = _invD[_activeInv].inventoryY;
			}
		}

		_invDragging = ID_NONE;
		ProcessedProvisional();
	}

	// Cursor could well now be inappropriate
	InvCursor(IC_AREA, curX, curY);

	_xChange = _yChange = 0; // Probably no need, but does no harm!
}

bool Dialogs::MenuDown(int lines) {
	if (cd.box == loadBox || cd.box == saveBox) {
		if (cd.extraBase < MAX_SAVED_FILES - NUM_RGROUP_BOXES) {
			FirstFile(cd.extraBase + lines);
			AddBoxes(true);
			return true;
		}
	} else if (cd.box == hopperBox1) {
		if (cd.extraBase < _numScenes - NUM_RGROUP_BOXES) {
			FirstScene(cd.extraBase + lines);
			AddBoxes(true);
			return true;
		}
	} else if (cd.box == hopperBox2) {
		if (cd.extraBase < _numEntries - NUM_RGROUP_BOXES) {
			FirstEntry(cd.extraBase + lines);
			AddBoxes(true);
			return true;
		}
	}
	return false;
}

bool Dialogs::MenuUp(int lines) {
	if (cd.extraBase > 0) {
		if (cd.box == loadBox || cd.box == saveBox)
			FirstFile(cd.extraBase - lines);
		else if (cd.box == hopperBox1)
			FirstScene(cd.extraBase - lines);
		else if (cd.box == hopperBox2)
			FirstEntry(cd.extraBase - lines);
		else
			return false;

		AddBoxes(true);
		return true;
	}
	return false;
}

void Dialogs::MenuRollDown() {
	if (MenuDown(1)) {
		if (cd.selBox > 0)
			cd.selBox--;
		Select(cd.selBox, true);
	}
}

void Dialogs::MenuRollUp() {
	if (MenuUp(1)) {
		if (cd.selBox < NUM_RGROUP_BOXES - 1)
			cd.selBox++;
		Select(cd.selBox, true);
	}
}

void Dialogs::MenuPageDown() {
	if (MenuDown(NUM_RGROUP_BOXES - 1)) {
		cd.selBox = NUM_RGROUP_BOXES - 1;
		Select(cd.selBox, true);
	}
}

void Dialogs::MenuPageUp() {
	if (MenuUp(NUM_RGROUP_BOXES - 1)) {
		cd.selBox = 0;
		Select(cd.selBox, true);
	}
}

void Dialogs::InventoryDown() {
	// This code is a copy of the IB_SLIDE_DOWN case in InvWalkTo
	// TODO: So share this duplicate code
	if (_invD[_activeInv].NoofVicons == 1)
		if (_invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons < _invD[_activeInv].NoofItems)
			_invD[_activeInv].FirstDisp += _invD[_activeInv].NoofHicons;
	for (int i = 1; i < _invD[_activeInv].NoofVicons; i++) {
		if (_invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons < _invD[_activeInv].NoofItems)
			_invD[_activeInv].FirstDisp += _invD[_activeInv].NoofHicons;
	}
	_ItemsChanged = true;
}

void Dialogs::InventoryUp() {
	// This code is a copy of the I_SLIDE_UP case in InvWalkTo
	// TODO: So share this duplicate code
	if (_invD[_activeInv].NoofVicons == 1)
		_invD[_activeInv].FirstDisp -= _invD[_activeInv].NoofHicons;
	for (int i = 1; i < _invD[_activeInv].NoofVicons; i++)
		_invD[_activeInv].FirstDisp -= _invD[_activeInv].NoofHicons;
	if (_invD[_activeInv].FirstDisp < 0)
		_invD[_activeInv].FirstDisp = 0;
	_ItemsChanged = true;
}

/**************************************************************************/
/************** Incoming events - further processing **********************/
/**************************************************************************/

/**
 * MenuAction
 */
void Dialogs::MenuAction(int i, bool dbl) {

	if (i >= 0) {
		switch (cd.box[i].boxType) {
		case FLIP:
			if (dbl) {
				*(cd.box[i].ival) ^= 1; // XOR with true
				AddBoxes(false);
			}
			break;

		case TOGGLE:
		case TOGGLE1:
		case TOGGLE2:
			if (!_buttonEffect.bButAnim) {
				_buttonEffect.bButAnim = true;
				_buttonEffect.box = &cd.box[i];
				_buttonEffect.press = false;
			}
			break;

		case RGROUP:
			if (dbl) {
				// Already highlighted
				switch (cd.box[i].boxFunc) {
				case SAVEGAME:
					KillInventory();
					InvSaveGame();
					break;
				case LOADGAME:
					KillInventory();
					InvLoadGame();
					break;
				case HOPPER2:
					KillInventory();
					OpenMenu(HOPPER_MENU2);
					break;
				case BF_CHANGESCENE:
					KillInventory();
					HopAction();
					FreeSceneHopper();
					break;
				default:
					break;
				}
			} else {
				Select(i, false);
			}
			break;

		case FRGROUP:
			if (dbl) {
				Select(i, false);
				LanguageChange();
			} else {
				Select(i, false);
			}
			break;

		case AAGBUT:
		case ARSGBUT:
		case ARSBUT:
		case AABUT:
		case AATBUT:
			if (_buttonEffect.bButAnim)
				break;

			_buttonEffect.bButAnim = true;
			_buttonEffect.box = &cd.box[i];
			_buttonEffect.press = true;
			break;
		default:
			break;
		}
	} else {
		ConfActionSpecial(i);
	}
}

void Dialogs::ConfActionSpecial(int i) {
	switch (i) {
	case IB_NONE:
	default:
		break;
	case IB_UP: // Scroll up
		if (cd.extraBase > 0) {
			if ((cd.box == loadBox) || (cd.box == saveBox))
				FirstFile(cd.extraBase - 1);
			else if (cd.box == hopperBox1)
				FirstScene(cd.extraBase - 1);
			else if (cd.box == hopperBox2)
				FirstEntry(cd.extraBase - 1);

			AddBoxes(true);
			if (cd.selBox < NUM_RGROUP_BOXES - 1)
				cd.selBox += 1;
			Select(cd.selBox, true);
		}
		break;
	case IB_DOWN: // Scroll down
		if ((cd.box == loadBox) || (cd.box == saveBox)) {
			if (cd.extraBase < MAX_SAVED_FILES - NUM_RGROUP_BOXES) {
				FirstFile(cd.extraBase + 1);
				AddBoxes(true);
				if (cd.selBox)
					cd.selBox -= 1;
				Select(cd.selBox, true);
			}
		} else if (cd.box == hopperBox1) {
			if (cd.extraBase < _numScenes - NUM_RGROUP_BOXES) {
				FirstScene(cd.extraBase + 1);
				AddBoxes(true);
				if (cd.selBox)
					cd.selBox -= 1;
				Select(cd.selBox, true);
			}
		} else if (cd.box == hopperBox2) {
			if (cd.extraBase < _numEntries - NUM_RGROUP_BOXES) {
				FirstEntry(cd.extraBase + 1);
				AddBoxes(true);
				if (cd.selBox)
					cd.selBox -= 1;
				Select(cd.selBox, true);
			}
		}
		break;

	case IB_SLIDE_UP:
		MenuPageUp();
		break;

	case IB_SLIDE_DOWN:
		MenuPageDown();
		break;
	}
}
// SLIDE_UP and SLIDE_DOWN on d click??????

void Dialogs::InvPutDown(int index) {
	int aniX, aniY;
	// index is the drop position
	int hiIndex; // Current position of held item (if in)

	// Find where the held item is positioned in this inventory (if it is)
	for (hiIndex = 0; hiIndex < _invD[_activeInv].NoofItems; hiIndex++)
		if (_invD[_activeInv].contents[hiIndex] == _heldItem)
			break;

	// If drop position would leave a gap, move it up
	if (index >= _invD[_activeInv].NoofItems) {
		if (hiIndex == _invD[_activeInv].NoofItems) // Not in, add it
			index = _invD[_activeInv].NoofItems;
		else
			index = _invD[_activeInv].NoofItems - 1;
	}

	if (hiIndex == _invD[_activeInv].NoofItems) { // Not in, add it
		if (_invD[_activeInv].NoofItems < _invD[_activeInv].MaxInvObj) {
			_invD[_activeInv].NoofItems++;

			// Don't leave it in the other inventory!
			if (InventoryPos(_heldItem) != INV_HELDNOTIN)
				RemFromInventory(_activeInv == INV_1 ? INV_2 : INV_1, _heldItem);
		} else {
			// No room at the inn!
			return;
		}
	}

	// Position it in the inventory
	if (index < hiIndex) {
		memmove(&_invD[_activeInv].contents[index + 1], &_invD[_activeInv].contents[index], (hiIndex - index) * sizeof(int));
		_invD[_activeInv].contents[index] = _heldItem;
	} else if (index > hiIndex) {
		memmove(&_invD[_activeInv].contents[hiIndex], &_invD[_activeInv].contents[hiIndex + 1], (index - hiIndex) * sizeof(int));
		_invD[_activeInv].contents[index] = _heldItem;
	} else {
		_invD[_activeInv].contents[index] = _heldItem;
	}

	_heldItem = INV_NOICON;
	_ItemsChanged = true;
	_vm->_cursor->DelAuxCursor();
	_vm->_cursor->RestoreMainCursor();
	_vm->_cursor->GetCursorXY(&aniX, &aniY, false);
	InvCursor(IC_DROP, aniX, aniY);
}

void Dialogs::InvPickup(int index) {
	INV_OBJECT *invObj;

	// Do nothing if not clicked on anything
	if (index == NOOBJECT)
		return;

	// If not holding anything
	if (_heldItem == INV_NOICON && _invD[_activeInv].contents[index] &&
	    (!TinselV2 || _invD[_activeInv].contents[index] != _heldItem)) {
		// Pick-up
		invObj = GetInvObject(_invD[_activeInv].contents[index]);
		_thisIcon = _invD[_activeInv].contents[index];
		if (TinselV2)
			InvTinselEvent(invObj, PICKUP, INV_PICKUP, index);
		else if (invObj->hScript)
			InvTinselEvent(invObj, WALKTO, INV_PICKUP, index);

	} else if (_heldItem != INV_NOICON) {
		// Put-down
		invObj = GetInvObject(_heldItem);

		// If DROPCODE set, send event, otherwise it's a putdown
		if (invObj->attribute & IO_DROPCODE && invObj->hScript)
			InvTinselEvent(invObj, PUTDOWN, INV_PICKUP, index);

		else if (!(invObj->attribute & IO_ONLYINV1 && _activeInv != INV_1) && !(invObj->attribute & IO_ONLYINV2 && _activeInv != INV_2)) {
			if (TinselV2)
				InvPutDown(index);
			else
				CoroScheduler.createProcess(PID_TCODE, InvPdProcess, &index, sizeof(index));
		}
	}
}

/**
 * Handle WALKTO event (Pick up/put down event)
 */
void Dialogs::InvWalkTo(const Common::Point &coOrds) {
	int i;

	switch (InvArea(coOrds.x, coOrds.y)) {
	case I_NOTIN:
		if (_activeInv == INV_CONV)
			ConvAction(INV_CLOSEICON);
		if ((cd.box == hopperBox1) || (cd.box == hopperBox2))
			FreeSceneHopper();
		KillInventory();
		break;

	case I_SLIDE_UP:
		if (_invD[_activeInv].NoofVicons == 1)
			_invD[_activeInv].FirstDisp -= _invD[_activeInv].NoofHicons;
		for (i = 1; i < _invD[_activeInv].NoofVicons; i++)
			_invD[_activeInv].FirstDisp -= _invD[_activeInv].NoofHicons;
		if (_invD[_activeInv].FirstDisp < 0)
			_invD[_activeInv].FirstDisp = 0;
		_ItemsChanged = true;
		break;

	case I_UP:
		_invD[_activeInv].FirstDisp -= _invD[_activeInv].NoofHicons;
		if (_invD[_activeInv].FirstDisp < 0)
			_invD[_activeInv].FirstDisp = 0;
		_ItemsChanged = true;
		break;

	case I_SLIDE_DOWN:
		if (_invD[_activeInv].NoofVicons == 1)
			if (_invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons < _invD[_activeInv].NoofItems)
				_invD[_activeInv].FirstDisp += _invD[_activeInv].NoofHicons;
		for (i = 1; i < _invD[_activeInv].NoofVicons; i++) {
			if (_invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons < _invD[_activeInv].NoofItems)
				_invD[_activeInv].FirstDisp += _invD[_activeInv].NoofHicons;
		}
		_ItemsChanged = true;
		break;

	case I_DOWN:
		if (_invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons < _invD[_activeInv].NoofItems) {
			_invD[_activeInv].FirstDisp += _invD[_activeInv].NoofHicons;
			_ItemsChanged = true;
		}
		break;

	case I_BODY:
		if (_activeInv == INV_CONF) {
			if (!_InventoryHidden)
				MenuAction(WhichMenuBox(coOrds.x, coOrds.y, false), false);
		} else {
			Common::Point pt = coOrds;
			i = InvItem(pt, false);

			// To cater for drop in dead space between icons,
			// look 1 pixel right, then 1 down, then 1 right and down.
			if (i == INV_NOICON && _heldItem != INV_NOICON &&
			    (_activeInv == INV_1 || _activeInv == INV_2)) {
				pt.x += 1; // 1 to the right
				i = InvItem(pt, false);
				if (i == INV_NOICON) {
					pt.x -= 1; // 1 down
					pt.y += 1;
					i = InvItem(pt, false);
					if (i == INV_NOICON) {
						pt.x += 1; // 1 down-right
						i = InvItem(pt, false);
					}
				}
			}

			if (_activeInv == INV_CONV) {
				ConvAction(i);
			} else
				InvPickup(i);
		}
		break;

	default:
		break;
	}
}

void Dialogs::InvAction() {
	int index;
	INV_OBJECT *invObj;
	int aniX, aniY;
	int i;

	_vm->_cursor->GetCursorXY(&aniX, &aniY, false);

	switch (InvArea(aniX, aniY)) {
	case I_BODY:
		if (_activeInv == INV_CONF) {
			if (!_InventoryHidden)
				MenuAction(WhichMenuBox(aniX, aniY, false), true);
		} else if (_activeInv == INV_CONV) {
			index = InvItem(&aniX, &aniY, false);
			ConvAction(index);
		} else {
			index = InvItem(&aniX, &aniY, false);
			if (index != INV_NOICON) {
				if (_invD[_activeInv].contents[index] && _invD[_activeInv].contents[index] != _heldItem) {
					invObj = GetInvObject(_invD[_activeInv].contents[index]);
					if (TinselV2)
						_thisIcon = _invD[_activeInv].contents[index];
					if (TinselV2 || (invObj->hScript))
						InvTinselEvent(invObj, ACTION, INV_ACTION, index);
				}
			}
		}
		break;

	case I_HEADER: // Maximise/unmaximise inventory
		if (!_invD[_activeInv].resizable)
			break;

		if (!_InventoryMaximised) {
			_invD[_activeInv].sNoofHicons = _invD[_activeInv].NoofHicons;
			_invD[_activeInv].sNoofVicons = _invD[_activeInv].NoofVicons;
			_invD[_activeInv].NoofHicons = _invD[_activeInv].MaxHicons;
			_invD[_activeInv].NoofVicons = _invD[_activeInv].MaxVicons;
			_InventoryMaximised = true;

			i = _invD[_activeInv].inventoryX;
			_invD[_activeInv].inventoryX = _invD[_activeInv].otherX;
			_invD[_activeInv].otherX = i;
			i = _invD[_activeInv].inventoryY;
			_invD[_activeInv].inventoryY = _invD[_activeInv].otherY;
			_invD[_activeInv].otherY = i;
		} else {
			_invD[_activeInv].NoofHicons = _invD[_activeInv].sNoofHicons;
			_invD[_activeInv].NoofVicons = _invD[_activeInv].sNoofVicons;
			_InventoryMaximised = false;

			i = _invD[_activeInv].inventoryX;
			_invD[_activeInv].inventoryX = _invD[_activeInv].otherX;
			_invD[_activeInv].otherX = i;
			i = _invD[_activeInv].inventoryY;
			_invD[_activeInv].inventoryY = _invD[_activeInv].otherY;
			_invD[_activeInv].otherY = i;
		}

		// Delete current, and re-draw
		DumpDobjArray();
		DumpObjArray();
		ConstructInventory(FULL);
		break;

	case I_UP:
		_invD[_activeInv].FirstDisp -= _invD[_activeInv].NoofHicons;
		if (_invD[_activeInv].FirstDisp < 0)
			_invD[_activeInv].FirstDisp = 0;
		_ItemsChanged = true;
		break;
	case I_DOWN:
		if (_invD[_activeInv].FirstDisp + _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons < _invD[_activeInv].NoofItems) {
			_invD[_activeInv].FirstDisp += _invD[_activeInv].NoofHicons;
			_ItemsChanged = true;
		}
		break;

	default:
		break;
	}
}

void Dialogs::InvLook(const Common::Point &coOrds) {
	int index;
	INV_OBJECT *invObj;
	Common::Point pt = coOrds;

	switch (InvArea(pt.x, pt.y)) {
	case I_BODY:
		index = InvItem(pt, false);
		if (index != INV_NOICON) {
			if (_invD[_activeInv].contents[index] && _invD[_activeInv].contents[index] != _heldItem) {
				invObj = GetInvObject(_invD[_activeInv].contents[index]);
				if (invObj->hScript)
					InvTinselEvent(invObj, LOOK, INV_LOOK, index);
			}
		}
		break;

	case I_NOTIN:
		if (_activeInv == INV_CONV)
			ConvAction(INV_CLOSEICON);
		KillInventory();
		break;

	default:
		break;
	}
}

/**************************************************************************/
/********************* Incoming events ************************************/
/**************************************************************************/

void Dialogs::EventToInventory(PLR_EVENT pEvent, const Common::Point &coOrds) {
	if (_InventoryHidden)
		return;

	switch (pEvent) {
	case PLR_PROV_WALKTO:
		if (MenuActive()) {
			ProcessedProvisional();
			InvWalkTo(coOrds);
		}
		break;

	case PLR_WALKTO: // PLR_SLEFT
		InvWalkTo(coOrds);
		break;

	case INV_LOOK: // PLR_SRIGHT
		if (MenuActive())
			InvWalkTo(coOrds);
		else
			InvLook(coOrds);
		break;

	case PLR_ACTION: // PLR_DLEFT
		if (_invDragging != ID_MDCONT)
			InvDragEnd();
		InvAction();
		break;

	case PLR_DRAG1_START: // Left drag start
		InvDragStart();
		break;

	case PLR_DRAG1_END: // Left drag end
		InvDragEnd();
		break;

	case PLR_ESCAPE:
		if (MenuActive()) {
			if (cd.box != optionBox && cd.box != hopperBox1 && cd.box != hopperBox2)
				_reOpenMenu = true;
			if ((cd.box == hopperBox1) || (cd.box == hopperBox2))
				FreeSceneHopper();
		}
		CloseInventory();
		break;

	case PLR_PGDN:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			MenuPageDown();
		} else {
			// Inventory window
			InventoryDown();
		}
		break;

	case PLR_PGUP:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			MenuPageUp();
		} else {
			// Inventory window
			InventoryUp();
		}
		break;

	case PLR_WHEEL_DOWN:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			MenuRollDown();
		} else {
			// Inventory window
			InventoryDown();
		}
		break;

	case PLR_WHEEL_UP:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			MenuRollUp();
		} else {
			// Inventory window
			InventoryUp();
		}
		break;

	case PLR_HOME:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			if (cd.box == loadBox || cd.box == saveBox)
				FirstFile(0);
			else if (cd.box == hopperBox1)
				FirstScene(0);
			else if (cd.box == hopperBox2)
				FirstEntry(0);
			else
				break;

			AddBoxes(true);
			cd.selBox = 0;
			Select(cd.selBox, true);
		} else {
			// Inventory window
			_invD[_activeInv].FirstDisp = 0;
			_ItemsChanged = true;
		}
		break;

	case PLR_END:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			if (cd.box == loadBox || cd.box == saveBox)
				FirstFile(MAX_SAVED_FILES); // Will get reduced to appropriate value
			else if (cd.box == hopperBox1)
				FirstScene(_numScenes); // Will get reduced to appropriate value
			else if (cd.box == hopperBox2)
				FirstEntry(_numEntries); // Will get reduced to appropriate value
			else
				break;

			AddBoxes(true);
			cd.selBox = 0;
			Select(cd.selBox, true);
		} else {
			// Inventory window
			_invD[_activeInv].FirstDisp = _invD[_activeInv].NoofItems - _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons;
			if (_invD[_activeInv].FirstDisp < 0)
				_invD[_activeInv].FirstDisp = 0;
			_ItemsChanged = true;
		}
		break;
	default:
		break;
	}
}

/**************************************************************************/
/************************* Odds and Ends **********************************/
/**************************************************************************/

/**
 * Called from Glitter function invdepict()
 * Changes (permanently) the animation film for that object.
 */
void Dialogs::SetObjectFilm(int object, SCNHANDLE hFilm) {
	INV_OBJECT *invObj;

	invObj = GetInvObject(object);
	invObj->hIconFilm = hFilm;

	if (_heldItem != object)
		_ItemsChanged = true;
}

/**
 * (Un)serialize the inventory data for save/restore game.
 */
void Dialogs::syncInvInfo(Common::Serializer &s) {
	for (int i = 0; i < NUM_INV; i++) {
		s.syncAsSint32LE(_invD[i].MinHicons);
		s.syncAsSint32LE(_invD[i].MinVicons);
		s.syncAsSint32LE(_invD[i].MaxHicons);
		s.syncAsSint32LE(_invD[i].MaxVicons);
		s.syncAsSint32LE(_invD[i].NoofHicons);
		s.syncAsSint32LE(_invD[i].NoofVicons);
		for (int j = 0; j < MAX_ININV; j++) {
			s.syncAsSint32LE(_invD[i].contents[j]);
		}
		s.syncAsSint32LE(_invD[i].NoofItems);
		s.syncAsSint32LE(_invD[i].FirstDisp);
		s.syncAsSint32LE(_invD[i].inventoryX);
		s.syncAsSint32LE(_invD[i].inventoryY);
		s.syncAsSint32LE(_invD[i].otherX);
		s.syncAsSint32LE(_invD[i].otherY);
		s.syncAsSint32LE(_invD[i].MaxInvObj);
		s.syncAsSint32LE(_invD[i].hInvTitle);
		s.syncAsSint32LE(_invD[i].resizable);
		s.syncAsSint32LE(_invD[i].bMoveable);
		s.syncAsSint32LE(_invD[i].sNoofHicons);
		s.syncAsSint32LE(_invD[i].sNoofVicons);
		s.syncAsSint32LE(_invD[i].bMax);
	}

	if (TinselV2) {
		for (int i = 0; i < _numObjects; ++i)
			s.syncAsUint32LE(_invFilms[i]);
		s.syncAsUint32LE(_heldFilm);
	}
}

/**************************************************************************/
/************************ Initialisation stuff ****************************/
/**************************************************************************/

/**
 * Called from PlayGame(), stores handle to inventory objects' data -
 * its id, animation film and Glitter script.
 */
// Note: the SCHANDLE type here has been changed to a void*
void Dialogs::RegisterIcons(void *cptr, int num) {
	_numObjects = num;
	_invObjects = (INV_OBJECT *)cptr;

	if (TinselV0) {
		// In Tinsel 0, the INV_OBJECT structure doesn't have an attributes field, so we
		// need to 'unpack' the source structures into the standard Tinsel v1/v2 format
		MEM_NODE *node = MemoryAllocFixed(_numObjects * sizeof(INV_OBJECT));
		assert(node);
		_invObjects = (INV_OBJECT *)MemoryDeref(node);
		assert(_invObjects);
		byte *srcP = (byte *)cptr;
		INV_OBJECT *destP = (INV_OBJECT *)_invObjects;

		for (int i = 0; i < num; ++i, ++destP, srcP += 12) {
			memmove(destP, srcP, 12);
			destP->attribute = 0;
		}
	} else if (TinselV2) {
		if (_invFilms == NULL) {
			// First time - allocate memory
			MEM_NODE *node = MemoryAllocFixed(_numObjects * sizeof(SCNHANDLE));
			assert(node);
			_invFilms = (SCNHANDLE *)MemoryDeref(node);
			if (_invFilms == NULL)
				error(NO_MEM, "inventory scripts");
			memset(_invFilms, 0, _numObjects * sizeof(SCNHANDLE));
		}

		// Add defined permanent conversation icons
		// and store all the films separately
		int i;
		INV_OBJECT *pio;
		for (i = 0, pio = _invObjects; i < _numObjects; i++, pio++) {
			if (pio->attribute & PERMACONV)
				PermaConvIcon(pio->id, pio->attribute & CONVENDITEM);

			_invFilms[i] = pio->hIconFilm;
		}
	}
}

/**
 * Called from Glitter function 'dec_invw()' - Declare the bits that the
 * inventory windows are constructed from, and special cursors.
 */
void Dialogs::setInvWinParts(SCNHANDLE hf) {
#ifdef DEBUG
	const FILM *pfilm;
#endif

	_hWinParts = hf;

#ifdef DEBUG
	pfilm = (const FILM *)LockMem(hf);
	assert(FROM_32(pfilm->numreels) >= (uint32)(TinselV2 ? T2_HOPEDFORREELS : T1_HOPEDFORREELS)); // not as many reels as expected
#endif
}

/**
 * Called from Glitter function 'dec_flags()' - Declare the language
 * flag films
 */
void Dialogs::setFlagFilms(SCNHANDLE hf) {
#ifdef DEBUG
	const FILM *pfilm;
#endif

	_flagFilm = hf;

#ifdef DEBUG
	pfilm = (const FILM *)LockMem(hf);
	assert(FROM_32(pfilm->numreels) >= HOPEDFORFREELS); // not as many reels as expected
#endif
}

/**
 * Called from Glitter function 'DecCStrings()'
 */
void Dialogs::setConfigStrings(SCNHANDLE *tp) {
	memcpy(_configStrings, tp, sizeof(_configStrings));
}

/**
 * Called from Glitter functions: dec_convw()/dec_inv1()/dec_inv2()
 * - Declare the heading text and dimensions etc.
 */
void Dialogs::idec_inv(int num, SCNHANDLE text, int MaxContents,
                       int MinWidth, int MinHeight,
                       int StartWidth, int StartHeight,
                       int MaxWidth, int MaxHeight,
                       int startx, int starty, bool moveable) {
	if (MaxWidth > MAXHICONS)
		MaxWidth = MAXHICONS; // Max window width
	if (MaxHeight > MAXVICONS)
		MaxHeight = MAXVICONS; // Max window height
	if (MaxContents > MAX_ININV)
		MaxContents = MAX_ININV; // Max contents

	if (StartWidth > MaxWidth)
		StartWidth = MaxWidth;
	if (StartHeight > MaxHeight)
		StartHeight = MaxHeight;

	_inventoryState = IDLE_INV;

	_invD[num].MaxHicons = MaxWidth;
	_invD[num].MinHicons = MinWidth;
	_invD[num].MaxVicons = MaxHeight;
	_invD[num].MinVicons = MinHeight;

	_invD[num].NoofHicons = StartWidth;
	_invD[num].NoofVicons = StartHeight;

	memset(_invD[num].contents, 0, sizeof(_invD[num].contents));
	_invD[num].NoofItems = 0;

	_invD[num].FirstDisp = 0;

	_invD[num].inventoryX = startx;
	_invD[num].inventoryY = starty;
	_invD[num].otherX = 21;
	_invD[num].otherY = 15;

	_invD[num].MaxInvObj = MaxContents;

	_invD[num].hInvTitle = text;

	if (MaxWidth != MinWidth && MaxHeight != MinHeight)
		_invD[num].resizable = true;

	_invD[num].bMoveable = moveable;

	_invD[num].bMax = false;
}

/**
 * Called from Glitter functions: dec_convw()/dec_inv1()/dec_inv2()
 * - Declare the heading text and dimensions etc.
 */
void Dialogs::idec_convw(SCNHANDLE text, int MaxContents,
                         int MinWidth, int MinHeight,
                         int StartWidth, int StartHeight,
                         int MaxWidth, int MaxHeight) {
	idec_inv(INV_CONV, text, MaxContents, MinWidth, MinHeight,
	         StartWidth, StartHeight, MaxWidth, MaxHeight,
	         20, 8, true);
}

/**
 * Called from Glitter functions: dec_convw()/dec_inv1()/dec_inv2()
 * - Declare the heading text and dimensions etc.
 */
void Dialogs::idec_inv1(SCNHANDLE text, int MaxContents,
                        int MinWidth, int MinHeight,
                        int StartWidth, int StartHeight,
                        int MaxWidth, int MaxHeight) {
	idec_inv(INV_1, text, MaxContents, MinWidth, MinHeight,
	         StartWidth, StartHeight, MaxWidth, MaxHeight,
	         100, 100, true);
}

/**
 * Called from Glitter functions: dec_convw()/dec_inv1()/dec_inv2()
 * - Declare the heading text and dimensions etc.
 */
void Dialogs::idec_inv2(SCNHANDLE text, int MaxContents,
                        int MinWidth, int MinHeight,
                        int StartWidth, int StartHeight,
                        int MaxWidth, int MaxHeight) {
	idec_inv(INV_2, text, MaxContents, MinWidth, MinHeight,
	         StartWidth, StartHeight, MaxWidth, MaxHeight,
	         100, 100, true);
}

/**
 * Called from Glitter function 'GetInvLimit()'
 */
int Dialogs::InvGetLimit(int invno) {
	assert(invno == INV_1 || invno == INV_2); // only INV_1 and INV_2 supported

	return _invD[invno].MaxInvObj;
}

/**
 * Called from Glitter function 'SetInvLimit()'
 */
void Dialogs::InvSetLimit(int invno, int MaxContents) {
	assert(invno == INV_1 || invno == INV_2);       // only INV_1 and INV_2 supported
	assert(MaxContents >= _invD[invno].NoofItems); // can't reduce maximum contents below current contents

	if (MaxContents > MAX_ININV)
		MaxContents = MAX_ININV; // Max contents

	_invD[invno].MaxInvObj = MaxContents;
}

/**
 * Called from Glitter function 'SetInvSize()'
 */
void Dialogs::InvSetSize(int invno, int MinWidth, int MinHeight,
                         int StartWidth, int StartHeight, int MaxWidth, int MaxHeight) {
	assert(invno == INV_1 || invno == INV_2); // only INV_1 and INV_2 supported

	if (StartWidth > MaxWidth)
		StartWidth = MaxWidth;
	if (StartHeight > MaxHeight)
		StartHeight = MaxHeight;

	_invD[invno].MaxHicons = MaxWidth;
	_invD[invno].MinHicons = MinWidth;
	_invD[invno].MaxVicons = MaxHeight;
	_invD[invno].MinVicons = MinHeight;

	_invD[invno].NoofHicons = StartWidth;
	_invD[invno].NoofVicons = StartHeight;

	if (MaxWidth != MinWidth && MaxHeight != MinHeight)
		_invD[invno].resizable = true;
	else
		_invD[invno].resizable = false;

	_invD[invno].bMax = false;
}

/**************************************************************************/

bool Dialogs::IsTopWindow() {
	return (_inventoryState == BOGUS_INV);
}

bool Dialogs::MenuActive() {
	return (_inventoryState == ACTIVE_INV && _activeInv == INV_CONF);
}

bool Dialogs::IsConvWindow() {
	return (_inventoryState == ACTIVE_INV && _activeInv == INV_CONV);
}

void Dialogs::CallFunction(BFUNC boxFunc) {
	switch (boxFunc) {
	case SAVEGAME:
		KillInventory();
		InvSaveGame();
		break;
	case LOADGAME:
		KillInventory();
		InvLoadGame();
		break;
	case IQUITGAME:
		_vm->quitGame();
		break;
	case CLOSEWIN:
		KillInventory();
		if ((cd.box == hopperBox1) || (cd.box == hopperBox2))
			FreeSceneHopper();
		break;
	case OPENLOAD:
		KillInventory();
		OpenMenu(LOAD_MENU);
		break;
	case OPENSAVE:
		KillInventory();
		OpenMenu(SAVE_MENU);
		break;
	case OPENREST:
		KillInventory();
		OpenMenu(RESTART_MENU);
		break;
	case OPENSOUND:
		KillInventory();
		OpenMenu(SOUND_MENU);
		break;
	case OPENCONT:
		KillInventory();
		OpenMenu(CONTROLS_MENU);
		break;
#ifndef JAPAN
	case OPENSUBT:
		KillInventory();
		OpenMenu(SUBTITLES_MENU);
		break;
#endif
	case OPENQUIT:
		KillInventory();
		OpenMenu(QUIT_MENU);
		break;
	case INITGAME:
		KillInventory();
		FnRestartGame();
		break;
	case CLANG:
		if (!LanguageChange())
			KillInventory();
		break;
	case RLANG:
		KillInventory();
		break;
	case HOPPER2:
		_vm->_dialogs->KillInventory();
		_vm->_dialogs->OpenMenu(HOPPER_MENU2);
		break;
	case BF_CHANGESCENE:
		_vm->_dialogs->KillInventory();
		_vm->_dialogs->HopAction();
		_vm->_dialogs->FreeSceneHopper();
		break;
	default:
		break;
	}
}

const FILM *Dialogs::GetWindowData() {
	return (const FILM *)_vm->_handle->LockMem(_hWinParts);
}

void Dialogs::Redraw() {
	if (DisplayObjectsActive()) {
		if (_ItemsChanged && !ConfigurationIsActive() && !InventoryIsHidden()) {
			FillInInventory();

			// Needed when clicking on scroll bar.
			int curX, curY;
			_vm->_cursor->GetCursorXY(&curX, &curY, false);
			InvCursor(IC_AREA, curX, curY);

			_ItemsChanged = false;
		}
		if (!ConfigurationIsActive()) {
			for (int i = 0; i < MAX_ICONS; i++) {
				if (_iconArray[i] != NULL)
					StepAnimScript(&_iconAnims[i]);
			}
		}
		if (IsMixingDeskControl()) {
			// Mixing desk control
			int sval, index, *pival;

			index = cd.selBox & ~IS_MASK;
			pival = cd.box[index].ival;
			sval = *pival;

			if (cd.selBox & IS_LEFT) {
				*pival -= cd.box[index].h;
				if (*pival < 0)
					*pival = 0;
			} else if (cd.selBox & IS_RIGHT) {
				*pival += cd.box[index].h;
				if (*pival > cd.box[index].w)
					*pival = cd.box[index].w;
			}

			if (sval != *pival) {
				SlideMSlider(0, (cd.selBox & IS_RIGHT) ? S_TIMEUP : S_TIMEDN);
			}
		}
	}
}

/**************************************************************************/
/************************ The inventory process ***************************/
/**************************************************************************/

static void InvPdProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GetToken(TOKEN_LEFT_BUT);
	CORO_SLEEP(_vm->_config->_dclickSpeed + 1);
	FreeToken(TOKEN_LEFT_BUT);

	// get the stuff copied to process when it was created
	const int *pindex = (const int *)param;

	_vm->_dialogs->InvPutDown(*pindex);

	CORO_END_CODE;
}

static void ButtonPress(CORO_PARAM, CONFBOX *box) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;

	assert(box->boxType == AAGBUT || box->boxType == ARSGBUT);

	// Replace highlight image with normal image
	pfilm = _vm->_dialogs->GetWindowData();
	if (_vm->_dialogs->_iconArray[HL1] != NULL)
		MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	pfilm = _vm->_dialogs->GetWindowData();
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->AddObject(&pfilm->reels[box->bi + NORMGRAPH], -1);
	MultiSetAniXY(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->CurrentInventoryX() + box->xpos, _vm->_dialogs->CurrentInventoryY() + box->ypos);
	MultiSetZPosition(_vm->_dialogs->_iconArray[HL1], Z_INV_ICONS + 1);

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// Replace normal image with depresses image
	pfilm = _vm->_dialogs->GetWindowData();
	MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->AddObject(&pfilm->reels[box->bi + DOWNGRAPH], -1);
	MultiSetAniXY(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->CurrentInventoryX() + box->xpos, _vm->_dialogs->CurrentInventoryY() + box->ypos);
	MultiSetZPosition(_vm->_dialogs->_iconArray[HL1], Z_INV_ICONS + 1);

	// Hold depressed image for 2 frames
	CORO_SLEEP(2);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// Replace depressed image with normal image
	pfilm = _vm->_dialogs->GetWindowData();
	MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->AddObject(&pfilm->reels[box->bi + NORMGRAPH], -1);
	MultiSetAniXY(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->CurrentInventoryX() + box->xpos, _vm->_dialogs->CurrentInventoryY() + box->ypos);
	MultiSetZPosition(_vm->_dialogs->_iconArray[HL1], Z_INV_ICONS + 1);

	CORO_SLEEP(1);

	CORO_END_CODE;
}

static void ButtonToggle(CORO_PARAM, CONFBOX *box) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;

	assert((box->boxType == TOGGLE) || (box->boxType == TOGGLE1) || (box->boxType == TOGGLE2));

	// Remove hilight image
	if (_vm->_dialogs->_iconArray[HL1] != NULL) {
		MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
		_vm->_dialogs->_iconArray[HL1] = nullptr;
	}

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (!_vm->_dialogs->InventoryIsActive())
		return;

	// Add depressed image
	pfilm = _vm->_dialogs->GetWindowData();
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->AddObject(&pfilm->reels[box->bi + DOWNGRAPH], -1);
	MultiSetAniXY(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->CurrentInventoryX() + box->xpos, _vm->_dialogs->CurrentInventoryY() + box->ypos);
	MultiSetZPosition(_vm->_dialogs->_iconArray[HL1], Z_INV_ICONS + 1);

	// Hold depressed image for 1 frame
	CORO_SLEEP(1);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// Toggle state
	(*box->ival) = *(box->ival) ^ 1; // XOR with true
	box->bi = *(box->ival) ? IX_TICK1 : IX_CROSS1;
	_vm->_dialogs->AddBoxes(false);
	// Keep highlight (e.g. flag)
	if (cd.selBox != NOBOX)
		_vm->_dialogs->Select(cd.selBox, true);

	// New state, depressed image
	pfilm = _vm->_dialogs->GetWindowData();
	if (_vm->_dialogs->_iconArray[HL1] != NULL)
		MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->AddObject(&pfilm->reels[box->bi + DOWNGRAPH], -1);
	MultiSetAniXY(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->CurrentInventoryX() + box->xpos, _vm->_dialogs->CurrentInventoryY() + box->ypos);
	MultiSetZPosition(_vm->_dialogs->_iconArray[HL1], Z_INV_ICONS + 1);

	// Hold new depressed image for 1 frame
	CORO_SLEEP(1);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// New state, normal
	MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = nullptr;

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (!_vm->_dialogs->InventoryIsActive())
		return;

	// New state, highlighted
	pfilm = _vm->_dialogs->GetWindowData();
	if (_vm->_dialogs->_iconArray[HL1] != NULL)
		MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->AddObject(&pfilm->reels[box->bi + HIGRAPH], -1);
	MultiSetAniXY(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->CurrentInventoryX() + box->xpos, _vm->_dialogs->CurrentInventoryY() + box->ypos);
	MultiSetZPosition(_vm->_dialogs->_iconArray[HL1], Z_INV_ICONS + 1);

	CORO_END_CODE;
}

/**
 * Redraws the icons if appropriate. Also handle button press/toggle effects
 */
extern void InventoryProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (NumberOfLanguages() <= 1)
		_vm->_dialogs->_noLanguage = true;

	while (1) {
		CORO_SLEEP(1); // allow scheduling

		_vm->_dialogs->Redraw();

		if (_vm->_dialogs->_buttonEffect.bButAnim) {
			assert(_vm->_dialogs->_buttonEffect.box);
			if (_vm->_dialogs->_buttonEffect.press) {
				if (_vm->_dialogs->_buttonEffect.box->boxType == AAGBUT || _vm->_dialogs->_buttonEffect.box->boxType == ARSGBUT)
					CORO_INVOKE_1(ButtonPress, _vm->_dialogs->_buttonEffect.box);
				_vm->_dialogs->CallFunction(_vm->_dialogs->_buttonEffect.box->boxFunc);
			} else
				CORO_INVOKE_1(ButtonToggle, _vm->_dialogs->_buttonEffect.box);

			_vm->_dialogs->_buttonEffect.bButAnim = false;
		}
	}
	CORO_END_CODE;
}

/**************************************************************************/
/************** Running inventory item's Glitter code *********************/
/**************************************************************************/

struct OP_INIT {
	INV_OBJECT *pinvo;
	TINSEL_EVENT event;
	PLR_EVENT bev;
	int myEscape;
};

/**
 * Run inventory item's Glitter code
 */
static void ObjectProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	INT_CONTEXT *pic;
	int ThisPointedWait; //	Fix the 'repeated pressing bug'
	CORO_END_CONTEXT(_ctx);

	// get the stuff copied to process when it was created
	const OP_INIT *to = (const OP_INIT *)param;

	CORO_BEGIN_CODE(_ctx);

	if (!TinselV2)
		CORO_INVOKE_1(AllowDclick, to->bev);

	_ctx->pic = InitInterpretContext(GS_INVENTORY, to->pinvo->hScript, to->event, NOPOLY, 0, to->pinvo,
	                                 to->myEscape);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	if (to->event == POINTED) {
		_ctx->ThisPointedWait = ++_vm->_dialogs->_pointedWaitCount;
		while (1) {
			CORO_SLEEP(1);
			int x, y;
			_vm->_cursor->GetCursorXY(&x, &y, false);
			if (_vm->_dialogs->InvItemId(x, y) != to->pinvo->id)
				break;

			// Fix the 'repeated pressing bug'
			if (_ctx->ThisPointedWait != _vm->_dialogs->_pointedWaitCount)
				CORO_KILL_SELF();
		}

		_ctx->pic = InitInterpretContext(GS_INVENTORY, to->pinvo->hScript, UNPOINT, NOPOLY, 0, to->pinvo);
		CORO_INVOKE_1(Interpret, _ctx->pic);
	}

	CORO_END_CODE;
}

/**
 * Run inventory item's Glitter code
 */
static void InvTinselEvent(INV_OBJECT *pinvo, TINSEL_EVENT event, PLR_EVENT be, int index) {
	OP_INIT to = {pinvo, event, be, 0};

	if (_vm->_dialogs->InventoryIsHidden() || (TinselV2 && !pinvo->hScript))
		return;

	_vm->_dialogs->_glitterIndex = index;
	CoroScheduler.createProcess(PID_TCODE, ObjectProcess, &to, sizeof(to));
}

extern void ObjectEvent(CORO_PARAM, int objId, TINSEL_EVENT event, bool bWait, int myEscape, bool *result) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	Common::PROCESS *pProc;
	INV_OBJECT *pInvo;
	OP_INIT op;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (result)
		*result = false;
	_ctx->pInvo = _vm->_dialogs->GetInvObject(objId);
	if (!_ctx->pInvo->hScript)
		return;

	_ctx->op.pinvo = _ctx->pInvo;
	_ctx->op.event = event;
	_ctx->op.myEscape = myEscape;

	CoroScheduler.createProcess(PID_TCODE, ObjectProcess, &_ctx->op, sizeof(_ctx->op));

	if (bWait)
		CORO_INVOKE_2(WaitInterpret, _ctx->pProc, result);
	else if (result)
		*result = false;

	CORO_END_CODE;
}

} // End of namespace Tinsel
