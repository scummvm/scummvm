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
#include "tinsel/noir/notebook.h"
#include "tinsel/noir/sysreel.h"

#include "common/textconsole.h"

#include "backends/keymapper/keymapper.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

#define HOPPER_FILENAME "hopper"

#define INV_PICKUP PLR_SLEFT // Local names
#define INV_LOOK PLR_SRIGHT  //	for button events
#define INV_ACTION PLR_DLEFT //

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

	IX3_TICK = 27,
	IX3_CROSS = 28,

	T1_HOPEDFORREELS = 50,
	T2_HOPEDFORREELS = 42
};

// The following defines select the correct constant depending on Tinsel version
#define IX_CROSS1 ((TinselVersion >= 2) ? IX2_CROSS1 : IX1_CROSS1)
#define IX_CURDD ((TinselVersion >= 2) ? IX2_CURDD : IX1_CURDD)
#define IX_CURDU ((TinselVersion >= 2) ? IX2_CURDU : IX1_CURDU)
#define IX_CURLR ((TinselVersion >= 2) ? IX2_CURLR : IX1_CURLR)
#define IX_CURUD ((TinselVersion >= 2) ? IX2_CURUD : IX1_CURUD)
#define IX_MDGROOVE ((TinselVersion >= 2) ? IX2_MDGROOVE : IX1_MDGROOVE)
#define IX_MDSLIDER ((TinselVersion >= 2) ? IX2_MDSLIDER : IX1_MDSLIDER)
#define IX_NTR ((TinselVersion >= 2) ? IX2_NTR : IX1_NTR)
#define IX_RBR ((TinselVersion >= 2) ? IX2_RBR : IX1_RBR)
#define IX_RTL ((TinselVersion >= 2) ? IX2_RTL : IX1_RTL)
#define IX_RTR ((TinselVersion >= 2) ? IX2_RTR : IX1_RTR)
#define IX_TICK1 ((TinselVersion >= 2) ? IX2_TICK1 : IX1_TICK1)

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
//-----------------------

#define MAX_ININV ((TinselVersion >= 2) ? 160 : 150) // Max in an inventory

#define ITEM_WIDTH ((TinselVersion >= 2) ? 50 : 25)  // Dimensions of an icon
#define ITEM_HEIGHT ((TinselVersion >= 2) ? 50 : 25) //
#define I_SEPARATION ((TinselVersion >= 2) ? 2 : 1)  // Item separation

#define NM_TOFF ((TinselVersion == 3) ? 21 : 11) // Title text Y offset from top
#define NM_TBT ((TinselVersion >= 2) ? 4 : 0) // Y, title box top
#define NM_TBB 33
#define NM_LSX ((TinselVersion >= 2) ? 4 : 0) // X, left side
#define NM_BSY ((TinselVersion >= 2) ? -9 : -M_TH + 1)
#define NM_RSX ((TinselVersion >= 2) ? -9 : -M_SW + 1)
#define NM_SBL (-27)
#define NM_SLH ((TinselVersion >= 2) ? 11 : 5) // Slider height
#define NM_SLX (-11)               // Slider X offset (from right)

#define NM_BG_POS_X ((TinselVersion >= 2) ? 9 : 1)    // }
#define NM_BG_POS_Y ((TinselVersion >= 2) ? 9 : 1)    // } Offset of translucent rectangle
#define NM_BG_SIZ_X ((TinselVersion >= 2) ? -18 : -3) // }
#define NM_BG_SIZ_Y ((TinselVersion >= 2) ? -18 : -3) // } How much larger it is than edges

#define NM_RS_B_INSET 4
#define NM_RS_R_INSET 4
#define NM_RS_THICKNESS 5
#define NM_MOVE_AREA_B_Y 30
#define NM_SLIDE_INSET ((TinselVersion >= 2) ? 18 : 9)     // X offset (from right) of left of scroll region
#define NM_SLIDE_THICKNESS ((TinselVersion >= 2) ? 13 : 7) // thickness of scroll region
#define NM_UP_ARROW_TOP 34                     // Y offset of top of up arrow
#define NM_UP_ARROW_BOTTOM 49                  // Y offset of bottom of up arrow
#define NM_DN_ARROW_TOP 22                     // Y offset (from bottom) of top of down arrow
#define NM_DN_ARROW_BOTTOM 5                   // Y offset (from bottom) of bottom of down arrow

#define MD_YBUTTOP ((TinselVersion >= 2) ? 2 : 9)
#define MD_YBUTBOT ((TinselVersion >= 2) ? 16 : 0)
#define MD_XLBUTL ((TinselVersion >= 2) ? 4 : 1)
#define MD_XLBUTR ((TinselVersion >= 2) ? 26 : 10)
#define MD_XRBUTL ((TinselVersion >= 2) ? 173 : 105)
#define MD_XRBUTR ((TinselVersion >= 2) ? 195 : 114)
#define ROTX1 60 // Rotate button's offsets from the center

#define MAX_NAME_RIGHT ((TinselVersion >= 2) ? 417 : 213)

#define SLIDE_RANGE ((TinselVersion >= 2) ? 120 : 81)
#define SLIDE_MINX ((TinselVersion >= 2) ? 25 : 8)
#define SLIDE_MAXX ((TinselVersion >= 2) ? 25 + 120 : 8 + 81)

#define MDTEXT_YOFF ((TinselVersion >= 2) ? -1 : 6)
#define MDTEXT_XOFF -4
#define TOG2_YOFF -22
#define ROT_YOFF 48
#define TYOFF ((TinselVersion >= 2) ? 4 : 0)
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

#define BW_T3 49
#define BH_T3 45

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

#define BOXX 56 // X-position of text boxes
#define BOXY 50 // Y-position of text boxes
#define T3_BOXX 60
#define T3_BOXY 69
#define T2_OPTX 33
#define T2_OPTY 36
#define T2_BOX_V_SEP 12
#define T2_BOX_V2_SEP 6
#define T3_BOX_V2_SEP 7

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

static CONFBOX t3OptionBox[] = {
	{ARSBUT, OPENLOAD, TM_INDEX, NULL, SS_LOAD_OPTION, T2_OPTX, T2_OPTY, T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{ARSBUT, OPENSAVE, TM_INDEX, NULL, SS_SAVE_OPTION, T2_OPTX, T2_OPTY + (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{ARSBUT, OPENREST, TM_INDEX, NULL, SS_RESTART_OPTION, T2_OPTX, T2_OPTY + 2 * (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{ARSBUT, OPENSOUND, TM_INDEX, NULL, SS_SOUND_OPTION, T2_OPTX, T2_OPTY + 3 * (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{ARSBUT, OPENQUIT, TM_INDEX, NULL, SS_QUIT_OPTION, T2_OPTX, T2_OPTY + 4 * (T2_BOX_HEIGHT + T2_BOX_V_SEP), T2_EDIT_BOX1_WIDTH, T2_BOX_HEIGHT, NULL, 0}
};

static CONFINIT t3ciOption = {6, 4, 144, 60, false, t3OptionBox, sizeof(t3OptionBox) / sizeof(CONFBOX), NO_HEADING};

static CONFINIT* ciOptionLookup[] = {
	&t1ciOption,
	&t1ciOption,
	&t2ciOption,
	&t3ciOption
};

static CONFBOX* ciOptionBoxLookup[] = {
	t1OptionBox,
	t1OptionBox,
	t2OptionBox,
	t3OptionBox
};

#define ciOption (*ciOptionLookup[TinselVersion])
#define optionBox (ciOptionBoxLookup[TinselVersion])

/*-------------------------------------------------------------*\
| These are the load and save game menus.			|
\*-------------------------------------------------------------*/

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

static CONFBOX t3LoadBox[] = {
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY, T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 2 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 3 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 4 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 5 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 6 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, LOADGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 7 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},

	{AABUT, LOADGAME, TM_NONE, NULL, 0, 460, 100, BW, BH, NULL, IX2_TICK1},
	{AATBUT, CLOSEWIN, TM_NONE, NULL, 0, 460, 100 + 100, BW, BH, NULL, IX2_CROSS1}};

static CONFINIT t1ciLoad = {10, 6, 20, 16, true, t1LoadBox, ARRAYSIZE(t1LoadBox), SIX_LOAD_HEADING};
static CONFINIT t2ciLoad = {10, 6, 40, 16, true, t2LoadBox, sizeof(t2LoadBox) / sizeof(CONFBOX), SS_LOAD_HEADING};
static CONFINIT t3ciLoad = {10, 6, 40, 16, true, t3LoadBox, sizeof(t3LoadBox) / sizeof(CONFBOX), SS_LOAD_HEADING};

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

static CONFBOX t3SaveBox[] = {
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY, T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 2 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 3 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 4 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 5 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 6 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},
	{RGROUP, SAVEGAME, TM_POINTER, NULL, 0, T3_BOXX, T3_BOXY + 7 * (T2_BOX_HEIGHT + T3_BOX_V2_SEP), T2_EDIT_BOX2_WIDTH, T2_BOX_HEIGHT, NULL, 0},

	{AABUT, SAVEGAME, TM_NONE, NULL, 0, 460, 100, BW, BH, NULL, IX2_TICK1},
	{AATBUT, CLOSEWIN, TM_NONE, NULL, 0, 460, 100 + 100, BW, BH, NULL, IX2_CROSS1}};

static CONFINIT t1ciSave = {10, 6, 20, 16, true, t1SaveBox, ARRAYSIZE(t1SaveBox), SIX_SAVE_HEADING};
static CONFINIT t2ciSave = {10, 6, 40, 16, true, t2SaveBox, sizeof(t2SaveBox) / sizeof(CONFBOX), SS_SAVE_HEADING};
static CONFINIT t3ciSave = {10, 6, 40, 16, true, t3SaveBox, sizeof(t3SaveBox) / sizeof(CONFBOX), SS_SAVE_HEADING};

static CONFINIT* ciLoadLookup[] = {
	&t1ciLoad,
	&t1ciLoad,
	&t2ciLoad,
	&t3ciLoad
};

static CONFBOX* ciLoadBoxLookup[] = {
	t1LoadBox,
	t1LoadBox,
	t2LoadBox,
	t3LoadBox
};

static CONFINIT* ciSaveLookup[] = {
	&t1ciSave,
	&t1ciSave,
	&t2ciSave,
	&t3ciSave
};

static CONFBOX* ciSaveBoxLookup[] = {
	t1SaveBox,
	t1SaveBox,
	t2SaveBox,
	t3SaveBox
};

#define ciLoad (*ciLoadLookup[TinselVersion])
#define loadBox (ciLoadBoxLookup[TinselVersion])
#define ciSave (*ciSaveLookup[TinselVersion])
#define saveBox (ciSaveBoxLookup[TinselVersion])

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

static CONFBOX t3RestartBox[] = {
	{AATBUT, INITGAME, TM_UNK4, NULL, 0, 140, 64, BW_T3, BH_T3, NULL, IX2_TICK1},
	{AATBUT, CLOSEWIN, TM_UNK4, NULL, 0, 60, 64, BW_T3, BH_T3, NULL, IX2_CROSS1}};

#ifdef JAPAN
static CONFINIT t1ciRestart = {6, 2, 72, 53, false, t1RestartBox, ARRAYSIZE(t1RestartBox), SIX_RESTART_HEADING};
#else
static CONFINIT t1ciRestart = {4, 2, 98, 53, false, t1RestartBox, ARRAYSIZE(t1RestartBox), SIX_RESTART_HEADING};
#endif
static CONFINIT t1ciRestartPSX = {8, 2, 46, 53, false, t1RestartBoxPSX, ARRAYSIZE(t1RestartBoxPSX), SIX_RESTART_HEADING};
static CONFINIT t2ciRestart = {4, 2, 196, 53, false, t2RestartBox, sizeof(t2RestartBox) / sizeof(CONFBOX), SS_RESTART_HEADING};
static CONFINIT t3ciRestart = {4, 2, 196, 53, false, t3RestartBox, sizeof(t3RestartBox) / sizeof(CONFBOX), SS_RESTART_HEADING};

static CONFINIT* ciRestartLookup[] = {
	&t1ciRestart,
	&t1ciRestart,
	&t2ciRestart,
	&t3ciRestart
};

#define ciRestart (TinselV1PSX ? t1ciRestartPSX : *ciRestartLookup[TinselVersion])

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

static CONFBOX t3SoundBox[] = {
	{ARSGBUT, MUSICVOL, TM_INDEX, NULL, SS_MVOL_SLIDER, 280, 50, 100, 2, /*&_vm->_config->_musicVolume*/ 0, 0},
	{ARSGBUT, NOFUNC, TM_INDEX, NULL, SS_SVOL_SLIDER, 280, 50 + 30 * 1, 100, 2, /*&_vm->_config->_soundVolume*/ 0, 0},
	{ARSGBUT, NOFUNC, TM_INDEX, NULL, SS_VVOL_SLIDER, 280, 50 + 30 * 2, 100, 2, /*&_vm->_config->_voiceVolume*/ 0, 0},
	{ARSGBUT, NOFUNC, TM_INDEX, NULL, SS_TSPEED_SLIDER, 280, 160, 100, 2, /*&_vm->_config->_textSpeed*/ 0, 0},
	{SLIDER, NOFUNC, TM_INDEX, NULL, SS_STITLE_TOGGLE, 100, 220, BW_T3, BH_T3, /*&_vm->_config->_useSubtitles*/ 0, 0}, // Should have type 7
	{TOGGLE1, NOFUNC, TM_INDEX, NULL, SS_LANGUAGE_SELECT, 320, 220, BW_T3, BH_T3, NULL, 0}
};

static CONFINIT t1ciSound = {10, 5, 20, 16, false, t1SoundBox, ARRAYSIZE(t1SoundBox), NO_HEADING};
static CONFINIT t2ciSound = {10, 5, 40, 16, false, t2SoundBox, sizeof(t2SoundBox) / sizeof(CONFBOX), SS_SOUND_HEADING};
static CONFINIT t3ciSound = {10, 5, 40, 16, false, t3SoundBox, sizeof(t3SoundBox) / sizeof(CONFBOX), SS_SOUND_HEADING};

static CONFINIT* ciSoundLookup[] = {
	&t1ciSound,
	&t1ciSound,
	&t2ciSound,
	&t3ciSound
};

#define ciSound (*ciSoundLookup[TinselVersion])

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

static CONFBOX t3QuitBox[] = {
	{AATBUT, IQUITGAME, TM_NONE, NULL, 0, 140, 64, BW_T3, BH_T3, NULL, IX3_CROSS},
	{AATBUT, CLOSEWIN, TM_NONE, NULL, 0, 60, 64, BW_T3, BH_T3, NULL, IX3_TICK}};

static CONFINIT t1ciQuit = {4, 2, 98, 53, false, t1QuitBox, ARRAYSIZE(t1QuitBox), SIX_QUIT_HEADING};
static CONFINIT t2ciQuit = {4, 2, 196, 53, false, t2QuitBox, sizeof(t2QuitBox) / sizeof(CONFBOX), SS_QUIT_HEADING};
static CONFINIT t3ciQuit = {4, 2, 196, 53, false, t3QuitBox, sizeof(t3QuitBox) / sizeof(CONFBOX), SS_QUIT_HEADING};

static CONFINIT* ciQuitLookup[] = {
	&t1ciQuit,
	&t1ciQuit,
	&t2ciQuit,
	&t3ciQuit
};

#define ciQuit (*ciQuitLookup[TinselVersion])

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
	IB_DOWN = -3,      // by whichMenuBox()
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

#define START_ICONX ((TinselVersion >= 2) ? 12 : (M_SW + 1))         // } Relative offset of first icon
#define START_ICONY ((TinselVersion >= 2) ? 40 : (M_TBB + M_TH + 1)) // } within the inventory window

/*-------------------------------------------------------------------------*/

static void InvTinselEvent(const InventoryObject *pinvo, TINSEL_EVENT event, PLR_EVENT be, int index);
static void InvPdProcess(CORO_PARAM, const void *param);

Dialogs::Dialogs() {
	_buttonEffect.bButAnim = false;
	_buttonEffect.box = nullptr;
	_buttonEffect.press = false;

	_hWinParts = 0;
	_flagFilm = 0;
	memset(_configStrings, 0, sizeof(_configStrings));

	_invObjects = nullptr;
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
	delete _invObjects;
	if (_objArray[0] != NULL) {
		dumpObjArray();
		dumpDobjArray();
		dumpIconArray();
	}
}

bool Dialogs::languageChange() {
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
		killInventory();
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
void Dialogs::primeSceneHopper() {
	Common::File f;
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

	_numScenes = vSize / sizeof(HOPPER);

	_pHopper = new HOPPER[_numScenes];

	for (int i = 0; i < _numScenes; i++) {
		_pHopper[i].hScene = FROM_32(f.readUint32LE());
		_pHopper[i].hSceneDesc = FROM_32(f.readUint32LE());
		_pHopper[i].numEntries = FROM_32(f.readUint32LE());
		_pHopper[i].entryIndex = FROM_32(f.readUint32LE());

		if (f.err()) {
			// file must be corrupt if we get to here
			error(FILE_IS_CORRUPT, HOPPER_FILENAME);
		}
	}

	_pEntries = new HOPENTRY[_numScenes];

	for (int i = 0; i < _numScenes; i++) {
		_pEntries[i].eNumber = FROM_32(f.readUint32LE());
		_pEntries[i].hDesc = FROM_32(f.readUint32LE());
		_pEntries[i].flags = FROM_32(f.readUint32LE());

		if (f.err()) {
			// file must be corrupt if we get to here
			error(FILE_IS_CORRUPT, HOPPER_FILENAME);
		}
	}

	// close the file
	f.close();
}

/**
 * Free the scene hopper data
 */
void Dialogs::freeSceneHopper() {
	delete[] _pEntries;
	_pEntries = nullptr;

	_pChosenScene = nullptr;

	delete[] _pHopper;
	_pHopper = nullptr;
}

void Dialogs::firstScene(int first) {
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
		cd.box[i].ixText = _pHopper[i + first].hSceneDesc;
	}
	// Blank out the spare ones (if any)
	while (i < NUM_RGROUP_BOXES) {
		cd.box[i].textMethod = TM_NONE;
		cd.box[i++].ixText = 0;
	}

	cd.extraBase = first;
}

void Dialogs::rememberChosenScene() {
	_bRemember = true;
}

void Dialogs::setChosenScene() {
	_lastChosenScene = cd.selBox + cd.extraBase;
	_pChosenScene = &_pHopper[cd.selBox + cd.extraBase];
}

void Dialogs::firstEntry(int first) {
	int i;

	_invD[INV_MENU].hInvTitle = _pChosenScene->hSceneDesc;

	// get number of entrances
	_numEntries = _pChosenScene->numEntries;

	// Force first to a sensible value
	if (first > _numEntries - NUM_RGROUP_BOXES)
		first = _numEntries - NUM_RGROUP_BOXES;
	if (first < 0)
		first = 0;

	for (i = 0; i < NUM_RGROUP_BOXES && i < _numEntries; i++) {
		cd.box[i].textMethod = TM_STRINGNUM;
		cd.box[i].ixText = _pEntries[_pChosenScene->entryIndex + i + first].hDesc;
	}
	// Blank out the spare ones (if any)
	while (i < NUM_RGROUP_BOXES) {
		cd.box[i].textMethod = TM_NONE;
		cd.box[i++].ixText = 0;
	}

	cd.extraBase = first;
}

void Dialogs::hopAction() {
	HOPENTRY *pEntry = _pEntries + _pChosenScene->entryIndex + cd.selBox + cd.extraBase;

	uint32 hScene = _pChosenScene->hScene;
	uint32 eNumber = pEntry->eNumber;
	debugC(DEBUG_BASIC, kTinselDebugAnimations, "Scene hopper chose scene %xh,%d\n", hScene, eNumber);

	if (pEntry->flags & fCall) {
		SaveScene(Common::nullContext);
		NewScene(Common::nullContext, _pChosenScene->hScene, pEntry->eNumber, TRANS_FADE);
	} else if (pEntry->flags & fHook)
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
void Dialogs::dumpIconArray() {
	for (int i = 0; i < MAX_ICONS; i++) {
		MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[i]);
	}
}

/**
 * Delete all the objects in DobjArray[]
 */
void Dialogs::dumpDobjArray() {
	for (int i = 0; i < MAX_WCOMP; i++) {
		MultiDeleteObjectIfExists(FIELD_STATUS, &_dispObjArray[i]);
	}
}

/**
 * Delete all the objects in objArray[]
 */
void Dialogs::dumpObjArray() {
	for (int i = 0; i < MAX_WCOMP; i++) {
		MultiDeleteObjectIfExists(FIELD_STATUS, &_objArray[i]);
	}
}

/**
 * Convert item ID number to pointer to item's compiled data
 * i.e. Image data and Glitter code.
 */
const InventoryObject *Dialogs::getInvObject(int id) {
	auto object = _invObjects->GetInvObject(id);
	if (!object) {
		error("getInvObject(%d): Trying to manipulate undefined inventory icon", id);
	}
	return object;
}

/**
 * Convert item ID number to pointer to item's compiled data
 * i.e. Image data and Glitter code.
 */
const InventoryObjectT3 *Dialogs::getInvObjectT3(int id) {
	auto object = _invObjects->GetInvObjectT3(id);
	if (!object) {
		error("getInvObjectT3(%d): Trying to manipulate undefined inventory icon", id);
	}
	return object;
}

/**
 * Returns true if the given id represents a valid inventory object
 */
bool Dialogs::getIsInvObject(int id) {
	int index = _invObjects->GetObjectIndexIfExists(id);
	return index != -1;
}

/**
 * Convert item ID number to index.
 */
int Dialogs::getObjectIndex(int id) const {
	int index = _invObjects->GetObjectIndexIfExists(id);
	if (index == -1) {
		error("getObjectIndex(%d): Trying to manipulate undefined inventory icon", id);
	}
	return index;
}

/**
 * Returns position of an item in one of the inventories.
 * The actual position is not important for the uses that this is put to.
 */
int Dialogs::inventoryPos(int num) {
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

bool Dialogs::isInInventory(int object, int invnum) {
	assert(invnum == INV_1 || invnum == INV_2);

	for (int i = 0; i < _invD[invnum].NoofItems; i++) // First inventory
		if (_invD[invnum].contents[i] == object)
			return true;

	return false;
}

/**
 * Returns which item is held (INV_NOICON (-1) if none)
 */
int Dialogs::whichItemHeld() {
	return _heldItem;
}

/**
 * Called from the cursor module when it re-initializes (at the start of
 * a new scene). For if we are holding something at scene-change time.
 */
void Dialogs::inventoryIconCursor(bool bNewItem) {

	if (_heldItem != INV_NOICON) {
		if (TinselVersion >= 2) {
			if (bNewItem) {
				int objIndex = getObjectIndex(_heldItem);

				if (TinselVersion == 3) {
					auto invObj = getInvObject(_heldItem);

					if (invObj->hasAttribute(InvObjAttr::NOTEBOOK_CLUE)) {
						_heldFilm = _vm->_systemReel->get((SysReel)objIndex);
					} else {
						_heldFilm = _invFilms[objIndex];
					}
				} else {
					_heldFilm = _invFilms[objIndex];
				}
			}
			_vm->_cursor->SetAuxCursor(_heldFilm);
		} else {
			auto invObj = getInvObject(_heldItem);
			_vm->_cursor->SetAuxCursor(invObj->getIconFilm());
		}
	}
}

/**
 * Returns true if the inventory is active.
 */
bool Dialogs::inventoryActive() {
	return _inventoryState == ACTIVE_INV;
}

bool Dialogs::inventoryOrNotebookActive() {
	return inventoryActive() || ((TinselVersion == 3) && _vm->_notebook->isOpen());
}

int Dialogs::whichInventoryOpen() {
	if (TinselVersion == 3 && _vm->_notebook->isOpen()) {
		return INV_NOTEBOOK;
	}
	if (_inventoryState != ACTIVE_INV)
		return 0;
	else {
		return _activeInv;
	}
}

/**************************************************************************/
/****************** Load/Save game specific functions *********************/
/**************************************************************************/

/**
 * Set first load/save file entry displayed.
 * Point Box[] text pointers to appropriate file descriptions.
 */
void Dialogs::firstFile(int first) {
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

void Dialogs::invSaveGame() {
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
void Dialogs::invLoadGame() {
	int rGame;

	if (cd.selBox != NOBOX && (cd.selBox + cd.extraBase < cd.numSaved)) {
		rGame = cd.selBox;
		cd.selBox = NOBOX;
		MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[HL3]);
		MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[HL2]);
		MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[HL1]);
		RestoreGame(rGame + cd.extraBase);
	}
}

/**
 * Edit the string in sedit[]
 * Returns true if the string was altered.
 */
#ifndef JAPAN
bool Dialogs::updateString(const Common::KeyState &kbd) {
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
static bool InvKeyIn(const Common::KeyState &kbd, const Common::CustomEventType &customType) {
	if (customType == kActionPageUp ||
		customType == kActionPageDown ||
		customType == kActionHome ||
		customType == kActionEnd)
		return true; // Key needs processing

	if (kbd.keycode == 0 && kbd.ascii == 0) {
		;
	} else if (kbd.keycode == Common::KEYCODE_RETURN) {
		return true; // Key needs processing
	} else if (customType == kActionEscape) {
		return true; // Key needs processing
	} else {
#ifndef JAPAN
		if (_vm->_dialogs->updateString(kbd)) {
			/*
			* Delete display of text currently being edited,
			* and replace it with freshly edited text.
			*/
			MultiDeleteObjectIfExists(FIELD_STATUS, &_vm->_dialogs->_iconArray[HL3]);
			_vm->_dialogs->_iconArray[HL3] = ObjectTextOut(
			    _vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_saveGameDesc, 0,
				_vm->_dialogs->currentInventoryX() + cd.box[cd.selBox].xpos + 2,
				_vm->_dialogs->currentInventoryY() + cd.box[cd.selBox].ypos + TYOFF,
			    _vm->_font->GetTagFontHandle(), 0);
			if (MultiRightmost(_vm->_dialogs->_iconArray[HL3]) > MAX_NAME_RIGHT) {
				MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL3]);
				_vm->_dialogs->updateString(Common::KeyState(Common::KEYCODE_BACKSPACE));
				_vm->_dialogs->_iconArray[HL3] = ObjectTextOut(
				    _vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_saveGameDesc, 0,
					_vm->_dialogs->currentInventoryX() + cd.box[cd.selBox].xpos + 2,
					_vm->_dialogs->currentInventoryY() + cd.box[cd.selBox].ypos + TYOFF,
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
void Dialogs::select(int i, bool force) {
#ifdef JAPAN
	time_t secs_now;
	struct tm *time_now;
#endif

	i &= ~IS_MASK;

	if (cd.selBox == i && !force)
		return;

	cd.selBox = i;

	// Clear previous selected highlight and text
	MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[HL2]);
	MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[HL3]);

	// New highlight box
	switch (cd.box[i].boxType) {
	case RGROUP:
		_iconArray[HL2] = RectangleObject(_vm->_bg->BgPal(),
		                                   ((TinselVersion >= 2) ? HighlightColor() : COL_HILIGHT), cd.box[i].w, cd.box[i].h);
		MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL2]);
		MultiSetAniXY(_iconArray[HL2],
		              _invD[_activeInv].inventoryX + cd.box[i].xpos,
		              _invD[_activeInv].inventoryY + cd.box[i].ypos);

		// Z-position of box, and add edit text if appropriate
		if (cd.editableRgroup) {
			MultiSetZPosition(_iconArray[HL2], Z_INV_ITEXT + 1);

			if (TinselVersion >= 2) {
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
		MultiSetAniXYZ(_iconArray[HL2],
		               _invD[_activeInv].inventoryX + cd.box[i].xpos - 2,
		               _invD[_activeInv].inventoryY + cd.box[i].ypos - 2,
		               Z_INV_BRECT + 1);

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
void Dialogs::dropItem(int item) {
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
void Dialogs::clearInventory(int invno) {
	assert(invno == INV_1 || invno == INV_2);

	_invD[invno].NoofItems = 0;
	memset(_invD[invno].contents, 0, sizeof(_invD[invno].contents));
}

/**
 * Stick the item into an inventory list (contents[]), and hold the
 * item if requested.
 */
void Dialogs::addToInventory(int invno, int icon, bool hold) {
	int i;
	bool bOpen;

	// Validate trying to add to a legal inventory
	assert(invno == INV_1 || invno == INV_2 || invno == INV_3 || invno == INV_CONV || invno == INV_OPEN || (invno == INV_DEFAULT && TinselVersion >= 2));

	if (invno == INV_OPEN) {
		assert(_inventoryState == ACTIVE_INV && (_activeInv == INV_1 || _activeInv == INV_2)); // addopeninv() with inventry not open
		invno = _activeInv;
		bOpen = true;

		// Make sure it doesn't get in both!
		remFromInventory(_activeInv == INV_1 ? INV_2 : INV_1, icon);
	} else {
		bOpen = false;

		if ((TinselVersion >= 2) && invno == INV_DEFAULT) {
			auto invObj = getInvObject(icon);
			if (invObj->hasAttribute(InvObjAttr::DEFINV2))
				invno = INV_2;
			else if (invObj->hasAttribute(InvObjAttr::DEFINV1))
				invno = INV_1;
			else
				invno = SysVar(SV_DEFAULT_INV);
		}
	}

	if (invno == INV_1)
		remFromInventory(INV_2, icon);
	else if (invno == INV_2)
		remFromInventory(INV_1, icon);

	// See if it's already there
	for (i = 0; i < _invD[invno].NoofItems; i++) {
		if (_invD[invno].contents[i] == icon)
			break;
	}

	// Add it if it isn't already there
	if (i == _invD[invno].NoofItems) {
		if (!bOpen) {
			if (invno == INV_CONV) {
				if (TinselVersion >= 2) {
					int nei;

					// Count how many current contents have end attribute
					for (i = 0, nei = 0; i < _invD[INV_CONV].NoofItems; i++) {
						auto invObj = getInvObject(_invD[INV_CONV].contents[i]);
						if (invObj->hasAttribute(InvObjAttr::CONVENDITEM))
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
		holdItem(icon);
}

/**
 * Take the item from the inventory list (contents[]).
 * Return FALSE if item wasn't present, true if it was.
 */
bool Dialogs::remFromInventory(int invno, int icon) {
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

		if ((TinselVersion >= 2) && invno == INV_CONV) {
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
void Dialogs::holdItem(int item, bool bKeepFilm) {
	if (_heldItem != item) {
		if ((TinselVersion >= 2) && (_heldItem != INV_NOICON)) {
			// No longer holding previous item
			_vm->_cursor->DelAuxCursor(); // no longer aux cursor

			// If old held object is not in an inventory, and
			// has a default, stick it in its default inventory.
			if (!isInInventory(_heldItem, INV_1) && !isInInventory(_heldItem, INV_2)) {
				auto invObj = getInvObject(_heldItem);

				if (invObj->hasAttribute(InvObjAttr::DEFINV1))
					addToInventory(INV_1, _heldItem);
				else if (invObj->hasAttribute(InvObjAttr::DEFINV2))
					addToInventory(INV_2, _heldItem);
				else {
					if ((TinselVersion < 3) || (!(invObj->hasAttribute(InvObjAttr::NOTEBOOK_CLUE)) && !(invObj->hasAttribute(InvObjAttr::V3ATTR_X400)))) {
						// Hook for definable default inventory
						addToInventory(INV_1, _heldItem);
					}
				}
			}

		} else if (TinselVersion <= 1) {
			if (item == INV_NOICON && _heldItem != INV_NOICON)
				_vm->_cursor->DelAuxCursor(); // no longer aux cursor

			if (item != INV_NOICON) {
				auto invObj = getInvObject(item);
				_vm->_cursor->SetAuxCursor(invObj->getIconFilm()); // and is aux. cursor
			}

			// WORKAROUND: If a held item is being removed that's not in either inventory (i.e. it was picked up
			// but never put in them), then when removing it from being held, drop it in the luggage
			if (_heldItem != INV_NOICON && inventoryPos(_heldItem) == INV_HELDNOTIN)
				addToInventory(INV_1, _heldItem);
		}

		_heldItem = item; // Item held

		if (TinselVersion >= 2) {
			inventoryIconCursor(!bKeepFilm);

			// Redraw contents - held item not displayed as a content.
			_ItemsChanged = true;
		}
	}

	if (TinselVersion <= 1)
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

#define EXTRA 1 // This was introduced when we decided to increase
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
int Dialogs::invArea(int x, int y) {
	if (TinselVersion == 3) {
		if (_vm->_notebook->isOpen()) {
			if (_vm->_notebook->handlePointer(Common::Point(x, y)) != 0) {
				return I_ENDCHANGE;
			}
			return I_NOTIN;
		}
	}
	if (TinselVersion >= 2) {
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
int Dialogs::invItem(int *x, int *y, bool update) {
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

int Dialogs::invItem(Common::Point &coOrds, bool update) {
	int x = coOrds.x;
	int y = coOrds.y;
	return invItem(&x, &y, update);
	//coOrds.x = x;
	//coOrds.y = y;
}

/**
 * Returns the id of the icon displayed under the given position.
 */
int Dialogs::invItemId(int x, int y) {
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
int Dialogs::whichMenuBox(int curX, int curY, bool bSlides) {
	if (bSlides) {
		for (int i = 0; i < _numMdSlides; i++) {
			Common::Rect bounds = MultiBounds(_mdSlides[i].obj);
			if (curY > bounds.top && curY < bounds.bottom && curX > bounds.left && curX < bounds.right)
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
		const Common::Rect r = (TinselVersion >= 2) ? Common::Rect(411, 46, 425, 339) : Common::Rect(20 + 181, 24 + 2, 20 + 181 + 8, 24 + 139 + 5);

		if (r.contains(curX, curY)) {

			if (curY < (r.top + ((TinselVersion >= 2) ? 18 : 5)))
				return IB_UP;
			else if (curY > (r.bottom - ((TinselVersion >= 2) ? 18 : 5)))
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
 * invBoxes
 */
void Dialogs::invBoxes(bool InBody, int curX, int curY) {
	int rotateIndex = -1;
	int index; // Box pointed to on this call
	const FILM *pfilm;

	// Find out which icon is currently pointed to
	if (!InBody)
		index = -1;
	else {
		index = whichMenuBox(curX, curY, false);
	}

	// If no icon pointed to, or points to (logical position of)
	// currently held icon, then no icon is pointed to!
	if (index < 0) {
		// unhigh-light box (if one was)
		cd.pointBox = NOBOX;
		MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[HL1]);
	} else if (index != cd.pointBox) {
		cd.pointBox = index;
		// A new box is pointed to - high-light it
		MultiDeleteObjectIfExists(FIELD_STATUS, &_iconArray[HL1]);
		if ((cd.box[cd.pointBox].boxType == ARSBUT && cd.selBox != NOBOX) ||
		    ///* I don't agree */ cd.box[cd.pointBox].boxType == RGROUP ||
		    cd.box[cd.pointBox].boxType == AATBUT ||
		    cd.box[cd.pointBox].boxType == AABUT) {
			_iconArray[HL1] = RectangleObject(_vm->_bg->BgPal(),
			                                   ((TinselVersion >= 2) ? HighlightColor() : COL_HILIGHT),
			                                   cd.box[cd.pointBox].w, cd.box[cd.pointBox].h);
			MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[HL1]);
			MultiSetAniXYZ(_iconArray[HL1],
			               _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos,
			               _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos,
			               Z_INV_ICONS + 1);
		} else if (cd.box[cd.pointBox].boxType == AAGBUT ||
		           cd.box[cd.pointBox].boxType == ARSGBUT ||
		           cd.box[cd.pointBox].boxType == TOGGLE ||
		           cd.box[cd.pointBox].boxType == TOGGLE1 ||
		           cd.box[cd.pointBox].boxType == TOGGLE2) {
			pfilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

			_iconArray[HL1] = addObject(&pfilm->reels[cd.box[cd.pointBox].bi + HIGRAPH], -1);
			MultiSetAniXYZ(_iconArray[HL1],
			               _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos,
			               _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos,
			               Z_INV_ICONS + 1);
		} else if (cd.box[cd.pointBox].boxType == ROTATE) {
			if (_noLanguage)
				return;

			pfilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

			rotateIndex = cd.box[cd.pointBox].bi;
			if (rotateIndex == IX2_LEFT1) {
				_iconArray[HL1] = addObject(&pfilm->reels[IX2_LEFT2], -1);
				MultiSetAniXYZ(_iconArray[HL1],
				               _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos - ROTX1,
				               _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos,
				               Z_INV_ICONS + 1);
			} else if (rotateIndex == IX2_RIGHT1) {
				_iconArray[HL1] = addObject(&pfilm->reels[IX2_RIGHT2], -1);
				MultiSetAniXYZ(_iconArray[HL1],
				               _invD[_activeInv].inventoryX + cd.box[cd.pointBox].xpos + ROTX1,
				               _invD[_activeInv].inventoryY + cd.box[cd.pointBox].ypos,
				               Z_INV_ICONS + 1);
			}
		}
	}
}

/**
 * Monitors for POINTED event for inventory icons.
 */
void Dialogs::invLabels(bool InBody, int aniX, int aniY) {
	int index; // Icon pointed to on this call

	// Find out which icon is currently pointed to
	if (!InBody)
		index = INV_NOICON;
	else {
		index = invItem(&aniX, &aniY, false);
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
		auto invObj = getInvObject(index);
		if (invObj->getScript())
			InvTinselEvent(invObj, POINTED, PLR_NOEVENT, index);
		_pointedIcon = index;
	}
}

void Dialogs::invPointEvent(const InventoryObject *invObj, int index) {
	InvTinselEvent(invObj, POINTED, PLR_NOEVENT, index);
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
void Dialogs::adjustTop() {
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
OBJECT *Dialogs::addInvObject(int num, SCNHANDLE *hNewScript, int *aniSpeed) {
	auto invObj = getInvObject(num);
	const FILM *pFilm = (const FILM *)_vm->_handle->LockMem(invObj->getIconFilm());
	const FREEL *pfr = (const FREEL *)&pFilm->reels[0];
	const MULTI_INIT *pmi = pfr->GetMultiInit();
	OBJECT *pPlayObj; // The object we insert

	*hNewScript = FROM_32(pfr->script);
	*aniSpeed = ONE_SECOND / FROM_32(pFilm->frate);

	PokeInPalette(pmi);
	pPlayObj = MultiInitObject(pmi);	// Needs to be initialized after the palette is set

	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), pPlayObj);

	return pPlayObj;
}

/**
 * Create display objects for the displayed icons in an inventory window.
 */
void Dialogs::fillInInventory() {
	int index; // Index into contents[]
	int n = 0; // index into iconArray[]
	int xpos, ypos;
	int row, col;
	SCNHANDLE hNewScript;
	int aniSpeed;

	dumpIconArray();

	if (_invDragging != ID_SLIDE)
		adjustTop(); // Set up slideStuff[]

	index = _invD[_activeInv].FirstDisp; // Start from first displayed object
	n = 0;
	ypos = START_ICONY; // Y-offset of first display row

	for (row = 0; row < _invD[_activeInv].NoofVicons; row++, ypos += ITEM_HEIGHT + 1) {
		xpos = START_ICONX; // X-offset of first display column

		for (col = 0; col < _invD[_activeInv].NoofHicons; col++) {
			if (index >= _invD[_activeInv].NoofItems)
				break;
			else if (_invD[_activeInv].contents[index] != _heldItem) {
				// Create a display object and position it
				_iconArray[n] = addInvObject(_invD[_activeInv].contents[index], &hNewScript, &aniSpeed);
				MultiSetAniXYZ(_iconArray[n],
				               _invD[_activeInv].inventoryX + xpos,
				               _invD[_activeInv].inventoryY + ypos,
				               Z_INV_ICONS);

				InitStepAnimScript(&_iconAnims[n], _iconArray[n], hNewScript, aniSpeed);

				n++;
			}
			index++;
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
void Dialogs::addBackground(OBJECT **rect, const Common::Rect &bounds, OBJECT **title, int textFrom) {
	// Why not 2 ????
	int width = bounds.width();
	int height = bounds.height();

	// Create a rectangle object
	_rectObject = *rect = TranslucentObject(width, height);

	// add it to display list and position it
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), *rect);
	positionInventory(*rect,
					  (TinselVersion < 3 ? NM_BG_POS_X : 0),
					  (TinselVersion < 3 ? NM_BG_POS_Y : 0),
					  Z_INV_BRECT);

	if (title == NULL)
		return;

	assert(TinselVersion < 3);

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

Common::Rect MultiBounds(OBJECT *obj) {
	Common::Rect bounds;
	bounds.left = MultiLeftmost(obj);
	bounds.right = MultiRightmost(obj);
	bounds.top = MultiHighest(obj);
	bounds.bottom = MultiLowest(obj);
	return bounds;
}

/**
 * Adds a title for a dialog
 */
void Dialogs::addTitle(OBJECT **title, const Common::Rect &rect) {
	if (_invD[_activeInv].hInvTitle != (SCNHANDLE)NO_HEADING) {
		LoadStringRes(_invD[_activeInv].hInvTitle, _vm->_font->TextBufferAddr(), TBUFSZ);
		
		int xOffset = (TinselVersion == 3) ? 0 : NM_BG_POS_X;
		*title = ObjectTextOut(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_font->TextBufferAddr(), 0,
							   _invD[_activeInv].inventoryX + (rect.width() / 2) + xOffset, _invD[_activeInv].inventoryY + NM_TOFF,
							   _vm->_font->GetTagFontHandle(), TXT_CENTER, 0);
		assert(*title);
		MultiSetZPosition(*title, Z_INV_HTEXT);
	}
}

/**
 * Insert a part of the inventory window frame onto the display list.
 */
OBJECT *Dialogs::addObject(const FREEL *pfreel, int num) {
	const MULTI_INIT *pmi = pfreel->GetMultiInit();
	const FRAME *pFrame = pmi->GetFrame();
	const IMAGE *pim;
	OBJECT *pPlayObj;

	PokeInPalette(pmi);

	pim = _vm->_handle->GetImage(READ_32(pFrame));

	// Horrible bodge involving global variables to save
	// width and/or height of some window frame components
	if (num == _TL) {
		_TLwidth = pim->imgWidth;
		_TLheight = pim->imgHeight & ~C16_FLAG_MASK;
	} else if (num == _TR) {
		_TRwidth = pim->imgWidth;
	} else if (num == _BL) {
		_BLheight = pim->imgHeight & ~C16_FLAG_MASK;
	}

	delete pim;

	// Set up and insert the multi-object
	pPlayObj = MultiInitObject(pmi);
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), pPlayObj);

	return pPlayObj;
}

/**
 * Display the scroll bar slider.
 */

void Dialogs::addSlider(OBJECT **slide, const FILM *pfilm) {
	_slideObject = *slide = addObject(&pfilm->reels[IX_SLIDE], -1);
	MultiSetAniXYZ(*slide, MultiRightmost(_rectObject) + ((TinselVersion >= 2) ? NM_SLX : -M_SXOFF + 2),
	               _invD[_activeInv].inventoryY + _sliderYpos,
	               Z_INV_MFRAME);
}

/**
 * Display a box with some text in it.
 */
void Dialogs::addBox(int *pi, const int i) {
	int x = _invD[_activeInv].inventoryX + cd.box[i].xpos;
	int y = _invD[_activeInv].inventoryY + cd.box[i].ypos;
	int *pival = cd.box[i].ival;
	int xdisp;
	const FILM *pFilm;

	switch (cd.box[i].boxType) {
	default:
		// Ignore if it's a blank scene hopper box
		if ((TinselVersion >= 2) && (cd.box[i].textMethod == TM_NONE))
			break;

		// Give us a box
		_iconArray[*pi] = RectangleObject(_vm->_bg->BgPal(), (TinselVersion >= 2) ? BoxColor() : COL_BOX,
		                                   cd.box[i].w, cd.box[i].h);
		MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _iconArray[*pi]);
		MultiSetAniXYZ(_iconArray[*pi], x, y, Z_INV_BRECT + 1);
		*pi += 1;

		// Stick in the text
		if ((cd.box[i].textMethod == TM_POINTER) ||
		    ((TinselVersion <= 1) && (cd.box[i].ixText == USE_POINTER))) {
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
			if (TinselVersion >= 2) {
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

			if ((TinselVersion >= 2) && (cd.box[i].boxType == RGROUP))
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

		_iconArray[*pi] = addObject(&pFilm->reels[cd.box[i].bi + NORMGRAPH], -1);
		MultiSetAniXYZ(_iconArray[*pi], x, y, Z_INV_BRECT + 1);
		*pi += 1;

		break;

	case FRGROUP:
		assert(_flagFilm != 0); // Language flags not declared!

		pFilm = (const FILM *)_vm->_handle->LockMem(_flagFilm);

		if (_vm->_config->_isAmericanEnglishVersion && cd.box[i].bi == FIX_UK)
			cd.box[i].bi = FIX_USA;

		_iconArray[*pi] = addObject(&pFilm->reels[cd.box[i].bi], -1);
		MultiSetAniXYZ(_iconArray[*pi], x, y, Z_INV_BRECT + 2);
		*pi += 1;

		break;

	case FLIP:
		pFilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

		if (*pival)
			_iconArray[*pi] = addObject(&pFilm->reels[cd.box[i].bi], -1);
		else
			_iconArray[*pi] = addObject(&pFilm->reels[cd.box[i].bi + 1], -1);
		MultiSetAniXYZ(_iconArray[*pi], x, y, Z_INV_BRECT + 1);
		*pi += 1;

		// Stick in the text
		if (TinselVersion >= 2) {
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
		_iconArray[*pi] = addObject(&pFilm->reels[cd.box[i].bi + NORMGRAPH], -1);
		MultiSetAniXYZ(_iconArray[*pi], x, y, Z_INV_BRECT + 1);
		*pi += 1;

		// Stick in the text
		if (TinselVersion >= 2) {
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

		_iconArray[*pi] = addObject(&pFilm->reels[IX_MDGROOVE], -1);
		MultiSetAniXYZ(_iconArray[*pi], x, y, Z_MDGROOVE);
		*pi += 1;
		_iconArray[*pi] = addObject(&pFilm->reels[IX_MDSLIDER], -1);
		MultiSetAniXYZ(_iconArray[*pi], x + SLIDE_MINX + xdisp, y, Z_MDSLIDER);
		assert(_numMdSlides < MAXSLIDES);
		_mdSlides[_numMdSlides].num = i;
		_mdSlides[_numMdSlides].min = x + SLIDE_MINX;
		_mdSlides[_numMdSlides].max = x + SLIDE_MAXX;
		_mdSlides[_numMdSlides++].obj = _iconArray[*pi];
		*pi += 1;

		// Stick in the text
		if (TinselVersion >= 2) {
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
			_iconArray[*pi] = addObject(&pFilm->reels[IX2_LEFT1], -1);
			MultiSetAniXYZ(_iconArray[*pi], x - ROTX1, y, Z_INV_BRECT + 1);
			*pi += 1;

			// Right one
			_iconArray[*pi] = addObject(&pFilm->reels[IX2_RIGHT1], -1);
			MultiSetAniXYZ(_iconArray[*pi], x + ROTX1, y, Z_INV_BRECT + 1);
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
		_iconArray[*pi] = addObject(&pFilm->reels[0], -1);
		MultiSetAniXYZ(_iconArray[*pi], x + FLAGX, y + FLAGY, Z_INV_BRECT + 1);
		*pi += 1;
		break;
	}
}

/**
 * Display some boxes.
 */
void Dialogs::addBoxes(bool bPosnSlide) {
	int objCount = NUMHL; // Object count - allow for HL1, HL2 etc.

	dumpIconArray();
	_numMdSlides = 0;

	for (int i = 0; i < cd.NumBoxes; i++) {
		addBox(&objCount, i);
	}

	if (cd.bExtraWin) {
		if (bPosnSlide && TinselVersion <= 1)
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

		if (TinselVersion <= 1)
			MultiSetAniXY(_slideObject, _invD[_activeInv].inventoryX + 24 + 179, _sliderYpos);
	}

	assert(objCount < MAX_ICONS); // added too many icons
}

/**
 * Display the scroll bar slider.
 */
void Dialogs::addEWSlider(OBJECT **slide, const FILM *pfilm) {
	_slideObject = *slide = addObject(&pfilm->reels[IX_SLIDE], -1);
	MultiSetAniXYZ(*slide, _invD[_activeInv].inventoryX + 24 + 127, _sliderYpos, Z_INV_MFRAME);
}

/**
 * addExtraWindow
 */
int Dialogs::addExtraWindow(int x, int y, OBJECT **retObj) {
	int n = 0;
	const FILM *pfilm;

	// Get the frame's data
	pfilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);

	x += (TinselVersion >= 2) ? 30 : 20;
	y += (TinselVersion >= 2) ? 38 : 24;

	// Draw the four corners
	retObj[n] = addObject(&pfilm->reels[IX_RTL], -1); // Top left
	MultiSetAniXYZ(retObj[n++], x, y, Z_INV_MFRAME);
	retObj[n] = addObject(&pfilm->reels[IX_NTR], -1); // Top right
	MultiSetAniXYZ(retObj[n++], x + ((TinselVersion >= 2) ? _TLwidth + 312 : 152), y, Z_INV_MFRAME);
	retObj[n] = addObject(&pfilm->reels[IX_BL], -1); // Bottom left
	MultiSetAniXYZ(retObj[n++], x, y + ((TinselVersion >= 2) ? _TLheight + 208 : 124), Z_INV_MFRAME);
	retObj[n] = addObject(&pfilm->reels[IX_BR], -1); // Bottom right
	MultiSetAniXYZ(retObj[n++], x + ((TinselVersion >= 2) ? _TLwidth + 312 : 152),
	               y + ((TinselVersion >= 2) ? _TLheight + 208 : 124),
	               Z_INV_MFRAME);

	// Draw the edges
	retObj[n] = addObject(&pfilm->reels[IX_H156], -1); // Top
	MultiSetAniXYZ(retObj[n++], x + ((TinselVersion >= 2) ? _TLwidth : 6), y + NM_TBT, Z_INV_MFRAME);
	retObj[n] = addObject(&pfilm->reels[IX_H156], -1); // Bottom
	MultiSetAniXYZ(retObj[n++], x + ((TinselVersion >= 2) ? _TLwidth : 6),
	               y + ((TinselVersion >= 2) ? _TLheight + 208 + _BLheight + NM_BSY : 143),
	               Z_INV_MFRAME);
	retObj[n] = addObject(&pfilm->reels[IX_V104], -1); // Left
	MultiSetAniXYZ(retObj[n++], x + NM_LSX, y + ((TinselVersion >= 2) ? _TLheight : 20), Z_INV_MFRAME);
	retObj[n] = addObject(&pfilm->reels[IX_V104], -1); // Right 1
	MultiSetAniXYZ(retObj[n++], x + ((TinselVersion >= 2) ? _TLwidth + 312 + _TRwidth + NM_RSX : 179),
	               y + ((TinselVersion >= 2) ? _TLheight : 20),
	               Z_INV_MFRAME);
	retObj[n] = addObject(&pfilm->reels[IX_V104], -1); // Right 2
	MultiSetAniXYZ(retObj[n++], x + ((TinselVersion >= 2) ? _TLwidth + 312 + _TRwidth + NM_SBL : 188),
	               y + ((TinselVersion >= 2) ? _TLheight : 20),
	               Z_INV_MFRAME);

	if (TinselVersion >= 2) {
		_sliderYpos = _sliderYmin = y + 27;
		_sliderYmax = y + 273;

		retObj[n++] = _slideObject = addObject(&pfilm->reels[IX_SLIDE], -1);
		MultiSetAniXYZ(_slideObject,
		               x + _TLwidth + 320 + _TRwidth - NM_BG_POS_X + NM_BG_SIZ_X - 2,
		               _sliderYpos,
		               Z_INV_MFRAME);
	} else {
		_sliderYpos = _sliderYmin = y + 9;
		_sliderYmax = y + 134;
		addEWSlider(&retObj[n++], pfilm);
	}

	return n;
}

void Dialogs::constructInventoryCommon(SysReel reel, bool hasTitle) {
	dumpObjArray();

	// Get the frame's data
	_objArray[0] = InsertSystemReelObj(reel);

	// Center the inventory.
	auto bounds = MultiBounds(_objArray[0]);
	_invD[_activeInv].inventoryX = (SCREEN_WIDTH - bounds.width()) / 2;
	_invD[_activeInv].inventoryY = (SCREEN_HEIGHT - bounds.height()) / 2;
	positionInventory(_objArray[0], 0, 0, Z_INV_MFRAME);
	MultiSetZPosition(_objArray[0], 16);

	addBackground(&_objArray[1], bounds);
	if (hasTitle) {
		addTitle(&_objArray[2], bounds);
		if (_objArray[2]) {
			// We currently skip this, as addTitle still needs ObjTextOut updates.
			warning("TODO: Align title");
		}
	}
}

void Dialogs::constructMainInventory() {
	warning("TODO: Complete implementation of constructMainInventory");
	constructInventoryCommon(SysReel::INVMAIN, false);
	_invD[_activeInv].FirstDisp = 0;

	// TODO: Slider, Scrolling

	fillInInventory();
}

void Dialogs::positionInventory(OBJECT *pMultiObj, int xOffset, int yOffset, int zPosition) {
	MultiSetAniXYZ(pMultiObj, _invD[_activeInv].inventoryX + xOffset, _invD[_activeInv].inventoryY + yOffset, zPosition);
}

SysReel GetSysReelForMenu(int menuId) {
	switch(menuId) {
	case MAIN_MENU:
		return SysReel::OPTIONS_MENU;
		break;
	case LOAD_MENU:
	case SAVE_MENU:
		return SysReel::LOADSAVE_MENU;
		break;
	case QUIT_MENU:
		return SysReel::QUIT_MENU;
		break;
	case SOUND_MENU:
		return SysReel::SUBTITLES_MENU;
		break;
	default:
		error("Unknown menu: %d", menuId);
	}
}

void Dialogs::constructConversationInventory() {
	warning("TODO: Complete implementation of constructConversationInventory");
	constructInventoryCommon(SysReel::CONVERSATION_FRAME, true);
}

void Dialogs::constructOtherInventory(int menuId) {
	warning("TODO: Complete implementation of constructOtherInventory");
	SysReel reel = GetSysReelForMenu(menuId);
	constructInventoryCommon(reel, true);

	if (cd.bExtraWin) {
		warning("TODO: Complete scrollbar implementation");
		SCNHANDLE sliderReel = _vm->_systemReel->get(SysReel::SLIDER);
		const FILM *pfilm = (const FILM *)_vm->_handle->LockMem(sliderReel);
		_objArray[3] = _slideObject = InsertReelObj(pfilm->reels);
		MultiSetAniXYZ(_slideObject,
					   _invD[_activeInv].inventoryX + 420,
					   _sliderYpos,
					   Z_INV_MFRAME - 1);
	}
	addBoxes(true);

}

/**
 * Construct an inventory window - either a standard one, with
 * background, slider and icons, or a re-sizing window.
 */
void Dialogs::constructInventory(InventoryType filling) {
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
		MultiDeleteObjectIfExists(FIELD_STATUS, &retObj[i]);
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
	if ((TinselVersion >= 2) && (_activeInv == INV_CONV)) {
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
	retObj[n] = addObject(&pfilm->reels[_TL], _TL);
	MultiSetAniXYZ(retObj[n], invX, invY, zpos);
	n++;
	retObj[n] = addObject(&pfilm->reels[_TR], _TR);
	MultiSetAniXYZ(retObj[n], invX + _TLwidth + eH, invY, zpos);
	n++;
	retObj[n] = addObject(&pfilm->reels[_BL], _BL);
	MultiSetAniXYZ(retObj[n], invX, invY + _TLheight + eV, zpos);
	n++;
	retObj[n] = addObject(&pfilm->reels[_BR], _BR);
	MultiSetAniXYZ(retObj[n], invX + _TLwidth + eH, invY + _TLheight + eV, zpos);
	n++;

	// Draw extra Top and bottom parts
	if (_invD[_activeInv].NoofHicons > 1) {
		// Top side
		retObj[n] = addObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
		MultiSetAniXYZ(retObj[n], invX + _TLwidth, invY + NM_TBT, zpos);
		n++;

		// Bottom of header box
		if (filling == FULL) {
			if (TinselVersion >= 2) {
				retObj[n] = addObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
				MultiSetAniXYZ(retObj[n], invX + _TLwidth, invY + NM_TBB, zpos);
				n++;
			} else {
				retObj[n] = addObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
				MultiSetAniXYZ(retObj[n], invX + _TLwidth, invY + M_TBB + 1, zpos);
				n++;

				// Extra bits for conversation - hopefully temporary
				if (_activeInv == INV_CONV) {
					retObj[n] = addObject(&pfilm->reels[IX_H26], -1);
					MultiSetAniXYZ(retObj[n], invX + _TLwidth - 2, invY + M_TBB + 1, zpos);
					n++;

					retObj[n] = addObject(&pfilm->reels[IX_H52], -1);
					MultiSetAniXYZ(retObj[n], invX + eH - 10, invY + M_TBB + 1, zpos);
					n++;
				}
			}
		}

		// Bottom side
		retObj[n] = addObject(&pfilm->reels[hFillers[_invD[_activeInv].NoofHicons - 2]], -1);
		MultiSetAniXYZ(retObj[n], invX + _TLwidth, invY + _TLheight + eV + _BLheight + NM_BSY, zpos);
		n++;
	}
	if (_SuppH) {
		int offx = _TLwidth + eH - ((TinselVersion >= 2) ? ITEM_WIDTH + I_SEPARATION : 26);
		if (offx < _TLwidth) // Not too far!
			offx = _TLwidth;

		// Top side extra
		retObj[n] = addObject(&pfilm->reels[IX_H26], -1);
		MultiSetAniXYZ(retObj[n], invX + offx, invY + NM_TBT, zpos);
		n++;

		// Bottom side extra
		retObj[n] = addObject(&pfilm->reels[IX_H26], -1);
		MultiSetAniXYZ(retObj[n], invX + offx, invY + _TLheight + eV + _BLheight + NM_BSY, zpos);
		n++;
	}

	// Draw extra side parts
	if (_invD[_activeInv].NoofVicons > 1) {
		// Left side
		retObj[n] = addObject(&pfilm->reels[vFillers[_invD[_activeInv].NoofVicons - 2]], -1);
		MultiSetAniXYZ(retObj[n], invX + NM_LSX, invY + _TLheight, zpos);
		n++;

		// Left side of scroll bar
		if (filling == FULL && _activeInv != INV_CONV) {
			retObj[n] = addObject(&pfilm->reels[vFillers[_invD[_activeInv].NoofVicons - 2]], -1);
			if (TinselVersion >= 2)
				MultiSetAniXY(retObj[n], invX + _TLwidth + eH + _TRwidth + NM_SBL, invY + _TLheight);
			else
				MultiSetAniXY(retObj[n], invX + _TLwidth + eH + M_SBL + 1, invY + _TLheight);
			MultiSetZPosition(retObj[n], zpos);
			n++;
		}

		// Right side
		retObj[n] = addObject(&pfilm->reels[vFillers[_invD[_activeInv].NoofVicons - 2]], -1);
		MultiSetAniXYZ(retObj[n], invX + _TLwidth + eH + _TRwidth + NM_RSX, invY + _TLheight, zpos);
		n++;
	}
	if (_SuppV) {
		int offy = _TLheight + eV - ((TinselVersion >= 2) ? ITEM_HEIGHT + I_SEPARATION : 26);
		int minAmount = (TinselVersion >= 2) ? 20 : 5;
		if (offy < minAmount)
			offy = minAmount;

		// Left side extra
		retObj[n] = addObject(&pfilm->reels[IX_V26], -1);
		MultiSetAniXYZ(retObj[n], invX + NM_LSX, invY + offy, zpos);
		n++;

		// Right side extra
		retObj[n] = addObject(&pfilm->reels[IX_V26], -1);
		MultiSetAniXYZ(retObj[n], invX + _TLwidth + eH + _TRwidth + NM_RSX, invY + offy, zpos);
		n++;
	}

	OBJECT **rect, **title;

	Common::Rect bounds;
	bounds.right = _TLwidth + eH + _TRwidth + NM_BG_SIZ_X;
	bounds.bottom = _TLheight + eV + _BLheight + NM_BG_SIZ_Y;
	// Draw background, slider and icons
	if ((TinselVersion >= 2) && (filling != EMPTY)) {
		addBackground(&retObj[n++], bounds);
		addTitle(&retObj[n++], bounds);
	}

	if (filling == FULL) {
		if (TinselVersion <= 1) {
			rect = &retObj[n++];
			title = &retObj[n++];

			addBackground(rect, bounds, title, FROM_HANDLE);
		}

		if (_activeInv == INV_CONV) {
			_slideObject = nullptr;

			if (TinselVersion >= 2) {
				// !!!!! MAGIC NUMBER ALERT !!!!!
				// Make sure it's big enough for the heading
				if (MultiLeftmost(retObj[n - 1]) < _invD[INV_CONV].inventoryX + 10) {
					_invD[INV_CONV].NoofHicons++;
					constructInventory(FULL);
				}
			}
		} else if (_invD[_activeInv].NoofItems > _invD[_activeInv].NoofHicons * _invD[_activeInv].NoofVicons) {
			_sliderYmin = _TLheight - ((TinselVersion >= 2) ? 1 : 2);
			_sliderYmax = _TLheight + eV + ((TinselVersion >= 2) ? 12 : 10);
			addSlider(&retObj[n++], pfilm);
		}

		fillInInventory();
	} else if (filling == CONF) {
		if (TinselVersion <= 1) {
			rect = &retObj[n++];
			title = &retObj[n++];

			addBackground(rect, bounds, title, FROM_STRING);
			if (cd.bExtraWin)
				n += addExtraWindow(invX, invY, &retObj[n]);
		} else {
			if (cd.bExtraWin)
				addExtraWindow(invX, invY, &retObj[n]);
		}

		addBoxes(true);
	}

	assert(n < MAX_WCOMP); // added more parts than we can handle!

	// Reposition returns true if needs to move
	if (_invD[_activeInv].bMoveable && filling == FULL && rePosition()) {
		constructInventory(FULL);
	}
}

/**
 * Call this when drawing a 'FULL', movable inventory. Checks that the
 * position of the Translucent object is within limits. If it isn't,
 * adjusts the x/y position of the current inventory and returns true.
 */
bool Dialogs::rePosition() {
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
void Dialogs::alterCursor(int num) {
	const FILM *pFilm = (const FILM *)_vm->_handle->LockMem(_hWinParts);
	const FREEL *pfr = (const FREEL *)&pFilm->reels[num];
	const MULTI_INIT *pmi = pfr->GetMultiInit();

	PokeInPalette(pmi);

	_vm->_cursor->SetTempCursor(FROM_32(pfr->script));
}

/**
 * invCursor
 */
void Dialogs::invCursor(InvCursorFN fn, int CurX, int CurY) {
	int area; // The part of the window the cursor is over
	bool restoreMain = false;

	// If currently dragging, don't be messing about with the cursor shape
	if (_invDragging != ID_NONE)
		return;

	switch (fn) {
	case IC_DROP:
		_invCursor = IC_NORMAL;
		invCursor(IC_AREA, CurX, CurY);
		break;

	case IC_AREA:
		area = invArea(CurX, CurY);

		// Check for POINTED events
		if (_activeInv == INV_CONF)
			invBoxes(area == I_BODY, CurX, CurY);
		else
			invLabels(area == I_BODY, CurX, CurY);

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
				alterCursor(IX_CURDD);
				_invCursor = IC_DR;
			}
			break;

		case I_TRIGHT:
		case I_BLEFT:
			if (!_invD[_activeInv].resizable)
				restoreMain = true;
			else if (_invCursor != IC_UR) {
				alterCursor(IX_CURDU);
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
				alterCursor(IX_CURUD);
				_invCursor = IC_TB;
			}
			break;

		case I_LEFT:
		case I_RIGHT:
			if (!_invD[_activeInv].resizable)
				restoreMain = true;
			else if (_invCursor != IC_LR) {
				alterCursor(IX_CURLR);
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

void Dialogs::convAction(int index) {
	assert(_activeInv == INV_CONV); // not conv. window!
	MOVER *pMover = (TinselVersion >= 2) ? GetMover(_vm->_actor->GetLeadId()) : NULL;

	switch (index) {
	case NOOBJECT:
		return;

	case INV_CLOSEICON:
		_thisIcon = NOOBJECT; // Postamble
		break;

	case INV_OPENICON:
		// Store the direction the lead character is facing in when the conversation starts
		if (TinselVersion >= 2)
			_initialDirection = GetMoverDirection(pMover);
		_thisIcon = INV_CLOSEICON; // Preamble
		break;

	default:
		_thisIcon = _invD[_activeInv].contents[index];
		break;
	}

	if (TinselVersion <= 1)
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
void Dialogs::setConvDetails(CONV_PARAM fn, HPOLYGON hPoly, int ano) {
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
void Dialogs::permaConvIcon(int icon, bool bEnd) {
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

void Dialogs::convPoly(HPOLYGON hPoly) {
	_thisConvPoly = hPoly;
}

int Dialogs::getIcon() {
	return _thisIcon;
}

void Dialogs::closeDownConv() {
	if (_inventoryState == ACTIVE_INV && _activeInv == INV_CONV) {
		killInventory();
	}
}

void Dialogs::hideConversation(bool bHide) {
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
			invLabels(false, 0, 0);
		} else {
			// Window is not hidden
			_InventoryHidden = false;

			if ((TinselVersion >= 2) && _ItemsChanged)
				// Just rebuild the whole thing
				constructInventory(FULL);
			else {
				// Move it all back on-screen
				for (i = 0; i < MAX_WCOMP && _objArray[i]; i++) {
					MultiAdjustXY(_objArray[i], -2 * SCREEN_WIDTH, 0);
				}

				// Don't flash if items changed. If they have, will be redrawn anyway.
				if ((TinselVersion >= 2) || !_ItemsChanged) {
					for (i = 0; i < MAX_ICONS && _iconArray[i]; i++) {
						MultiAdjustXY(_iconArray[i], -2 * SCREEN_WIDTH, 0);
					}
				}
			}

			if ((TinselVersion >= 2) && _bMoveOnUnHide) {
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
			invLabels(true, aniX, aniY);
		}
	}
}

bool Dialogs::convIsHidden() {
	return _InventoryHidden;
}

/**************************************************************************/
/******************* Open and closing functions ***************************/
/**************************************************************************/

/**
 * Start up an inventory window.
 */
void Dialogs::popUpInventory(int invno, int menuId) {
	assert(invno == INV_1 || invno == INV_2 || invno == INV_CONV || invno == INV_CONF || invno == INV_MENU); // Trying to open illegal inventory

	if (_inventoryState == IDLE_INV) {
		_reOpenMenu = false; // Better safe than sorry...

		DisableTags(); // Tags disabled during inventory
		if (TinselVersion >= 2)
			DisablePointing(); // Pointing disabled during inventory

		if (invno == INV_CONV) { // Conversation window?
			if (TinselVersion >= 2)
				// Quiet please..
				_vm->_pcmMusic->dim(false);

			// Start conversation with permanent contents
			memset(_invD[INV_CONV].contents, 0, MAX_ININV * sizeof(int));
			memcpy(_invD[INV_CONV].contents, _permIcons, _numPermIcons * sizeof(int));
			_invD[INV_CONV].NoofItems = _numPermIcons;
			if (TinselVersion >= 2)
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
		if (TinselVersion == 3) {
			switch (invno) {
			case INV_CONV:
				constructConversationInventory();
				 break;
			case INV_1:
			case INV_2:
			case INV_3:
			case INV_4:
				constructMainInventory();
				break;
			default: // Should be menu.
				constructOtherInventory(menuId);
				break;
			}
		} else {
			if (invno != INV_CONF)        // Configuration window?
				constructInventory(FULL); // Draw it up
			else {
				constructInventory(CONF); // Draw it up
			}
		}
	}
}

void Dialogs::setMenuGlobals(CONFINIT *ci) {
	if (TinselVersion < 3) {
		_invD[INV_CONF].MinHicons = _invD[INV_CONF].MaxHicons = _invD[INV_CONF].NoofHicons = ci->h;
		_invD[INV_CONF].MaxVicons = _invD[INV_CONF].MinVicons = _invD[INV_CONF].NoofVicons = ci->v;
	}
	_invD[INV_CONF].inventoryX = ci->x;
	_invD[INV_CONF].inventoryY = ci->y;
	cd.bExtraWin = ci->bExtraWin;
	cd.box = ci->Box;
	cd.NumBoxes = ci->NumBoxes;
	cd.ixHeading = ci->ixHeading;

	if (TinselVersion >= 2) {
		if ((ci->ixHeading != NO_HEADING) && SysString(ci->ixHeading))
			_invD[INV_MENU].hInvTitle = SysString(ci->ixHeading);
		else
			_invD[INV_MENU].hInvTitle = NO_HEADING;
	}
}

/**
 * PopupConf
 */
void Dialogs::openMenu(CONFTYPE menuType) {
	int curX, curY;

	// In the DW 1 demo, don't allow any menu to be opened
	if (TinselVersion == 0)
		return;

	if (_inventoryState != IDLE_INV)
		return;

	_invD[INV_CONF].resizable = false;
	_invD[INV_CONF].bMoveable = false;

	Common::Keymapper *keymapper = _vm->getEventManager()->getKeymapper();
	if (menuType == SAVE_MENU || menuType == LOAD_MENU) {
		keymapper->getKeymap("game-shortcuts")->setEnabled(false);
		keymapper->getKeymap("saveload-shortcuts")->setEnabled(true);
	}
	switch (menuType) {
	case MAIN_MENU:
		setMenuGlobals(&ciOption);
		break;

	case SAVE_MENU:
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true); // Show VK when saving a game
		if (TinselVersion <= 1)
			_vm->_cursor->SetCursorScreenXY(262, 91);
		setMenuGlobals(&ciSave);
		cd.editableRgroup = true;
		firstFile(0);
		break;

	case LOAD_MENU:
		setMenuGlobals(&ciLoad);
		cd.editableRgroup = false;
		firstFile(0);
		break;

	case RESTART_MENU:
		if (TinselVersion >= 2)
			_vm->_cursor->SetCursorScreenXY(360, 153);
		else if (_vm->getLanguage() == Common::JA_JPN)
			_vm->_cursor->SetCursorScreenXY(180, 106);
		else
			_vm->_cursor->SetCursorScreenXY(180, 90);

		setMenuGlobals(&ciRestart);
		break;

	case SOUND_MENU:
		if (TinselVersion >= 2)
			_displayedLanguage = TextLanguage();
#if 1
		// FIXME: Hack to setup CONFBOX pointer to data in the global Config object
		if (TinselVersion == 3) {
			t3SoundBox[0].ival = &_vm->_config->_musicVolume;
			t3SoundBox[1].ival = &_vm->_config->_soundVolume;
			t3SoundBox[2].ival = &_vm->_config->_voiceVolume;
			t3SoundBox[3].ival = &_vm->_config->_textSpeed;
			t3SoundBox[4].ival = &_vm->_config->_useSubtitles;
		} else if (TinselVersion >= 2) {
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
		setMenuGlobals(&ciSound);
		break;

	case CONTROLS_MENU:
#if 1
		// FIXME: Hack to setup CONFBOX pointer to data in the global Config object
		controlBox[0].ival = &_vm->_config->_dclickSpeed;
		controlBox[2].ival = &_vm->_config->_swapButtons;
#endif
		setMenuGlobals(&ciControl);
		break;

	case QUIT_MENU:
		if (TinselVersion >= 2)
			_vm->_cursor->SetCursorScreenXY(360, 153);
		else if (_vm->getLanguage() == Common::JA_JPN)
			_vm->_cursor->SetCursorScreenXY(180, 106);
		else
			_vm->_cursor->SetCursorScreenXY(180, 90);

		setMenuGlobals(&ciQuit);
		break;

	case HOPPER_MENU1:
		primeSceneHopper();
		setMenuGlobals(&ciHopper1);
		cd.editableRgroup = false;
		rememberChosenScene();
		firstScene(0);
		break;

	case HOPPER_MENU2:
		setMenuGlobals(&ciHopper2);
		cd.editableRgroup = false;
		setChosenScene();
		firstEntry(0);
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

		setMenuGlobals(&ciSubtitles);
	} break;

	case TOP_WINDOW:
		setMenuGlobals(&ciTopWin);
		_activeInv = INV_CONF;
		constructInventory(CONF); // Draw it up
		_inventoryState = BOGUS_INV;
		return;

	default:
		return;
	}

	if (_heldItem != INV_NOICON)
		_vm->_cursor->DelAuxCursor(); // no longer aux cursor

	popUpInventory(INV_CONF, menuType);

	// Make initial box selections if appropriate
	if (menuType == SAVE_MENU || menuType == LOAD_MENU || menuType == HOPPER_MENU1 || menuType == HOPPER_MENU2)
		select(0, false);
	else if (menuType == SUBTITLES_MENU) {
		if (_vm->getFeatures() & GF_USE_3FLAGS) {
			// VERY quick dirty bodges
			if (_vm->_config->_language == TXT_FRENCH)
				select(0, false);
			else if (_vm->_config->_language == TXT_GERMAN)
				select(1, false);
			else
				select(2, false);
		} else if (_vm->getFeatures() & GF_USE_4FLAGS) {
			select(_vm->_config->_language - 1, false);
		} else if (_vm->getFeatures() & GF_USE_5FLAGS) {
			select(_vm->_config->_language, false);
		}
	}

	_vm->_cursor->GetCursorXY(&curX, &curY, false);
	invCursor(IC_AREA, curX, curY);
}

/**
 * Close down an inventory window.
 */
void Dialogs::killInventory() {
	if (_objArray[0] != NULL) {
		dumpObjArray();
		dumpDobjArray();
		dumpIconArray();
	}

	if (_inventoryState == ACTIVE_INV) {
		EnableTags();
		if (TinselVersion >= 2)
			EnablePointing();

		_invD[_activeInv].bMax = _InventoryMaximised;

		_vm->_cursor->UnHideCursorTrails();
		_vm->divertKeyInput(NULL);
	}

	_inventoryState = IDLE_INV;

	if (_reOpenMenu) {
		_reOpenMenu = false;
		openMenu(MAIN_MENU);

		// Write config changes
		_vm->_config->writeToDisk();

	} else if (_activeInv == INV_CONF)
		inventoryIconCursor(false);

	if (TinselVersion >= 2)
		// Pump up the volume
		if (_activeInv == INV_CONV)
			_vm->_pcmMusic->unDim(false);

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false); // Hide VK after save dialog closes
}

void Dialogs::closeInventory() {
	// If not active, ignore this
	if (_inventoryState != ACTIVE_INV)
		return;

	// If hidden, a conversation action is still underway - ignore this
	if (_InventoryHidden)
		return;

	// If conversation, this is a closeing event
	if (_activeInv == INV_CONV)
		convAction(INV_CLOSEICON);

	killInventory();

	_vm->_cursor->RestoreMainCursor();
}

/**************************************************************************/
/*************** Drag stuff - Resizing and moving window ******************/
/**************************************************************************/

/**
 * Appears to find the nearest entry in slideStuff[] to the supplied
 * y-coordinate.
 */
int Dialogs::nearestSlideY(int fity) {
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
void Dialogs::slideSlider(int y, SSFN fn) {
	static int newY = 0, lasti = 0; // FIXME: Avoid non-const global vars
	int gotoY, ati;

	// Only do this if there's a slider
	if (!_slideObject)
		return;

	switch (fn) {
	case S_START: // Start of a drag on the slider
		newY = _sliderYpos;
		lasti = nearestSlideY(_sliderYpos);
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
		ati = nearestSlideY(_sliderYpos);
		if (ati != lasti) {
			_invD[_activeInv].FirstDisp = _slideStuff[ati].n;
			assert(_invD[_activeInv].FirstDisp >= 0); // negative first displayed
			_ItemsChanged = true;
			lasti = ati;
		}
		break;

	case S_END: // End of a drag on the slider
		// Draw icons from new start icon
		ati = nearestSlideY(_sliderYpos);
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
void Dialogs::slideCSlider(int y, SSFN fn) {
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
		if (TinselVersion >= 2)
			MultiMoveRelXY(_slideObject, 0, gotoY - _sliderYpos);
		_sliderYpos = gotoY;

		fc = cd.extraBase;

		if ((cd.box == saveBox || cd.box == loadBox))
			firstFile((_sliderYpos - _sliderYmin) * (MAX_SAVED_FILES - NUM_RGROUP_BOXES) /
					  (_sliderYmax - _sliderYmin));
		else if (cd.box == hopperBox1)
			firstScene((_sliderYpos - _sliderYmin) * (_numScenes - NUM_RGROUP_BOXES) / sliderRange);
		else if (cd.box == hopperBox2)
			firstEntry((_sliderYpos - _sliderYmin) * (_numEntries - NUM_RGROUP_BOXES) / sliderRange);

		// If extraBase has changed...
		if (fc != cd.extraBase) {
			addBoxes(false);
			fc -= cd.extraBase;
			cd.selBox += fc;

			// Ensure within legal limits
			if (cd.selBox < 0)
				cd.selBox = 0;
			else if (cd.selBox >= NUM_RGROUP_BOXES)
				cd.selBox = NUM_RGROUP_BOXES - 1;

			select(cd.selBox, true);
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
void Dialogs::slideMSlider(int x, SSFN fn) {
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
		addBoxes(false); // Might change position slightly
		if (_activeInv == INV_CONF && cd.box == subtitlesBox)
			select(_vm->_config->_language, false);
		break;

	default:
		break;
	}
}

/**
 * Called from changeingSize() during re-sizing.
 */
void Dialogs::gettingTaller() {
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
 * Called from changeingSize() during re-sizing.
 */
void Dialogs::gettingShorter() {
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
 * Called from changeingSize() during re-sizing.
 */
void Dialogs::gettingWider() {
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
 * Called from changeingSize() during re-sizing.
 */
void Dialogs::gettingNarrower() {
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
 * Called from xMovement()/yMovement() during re-sizing.
 */
void Dialogs::changeingSize() {
	/* Make it taller or shorter if necessary. */
	if (_yChange > 0)
		gettingTaller();
	else if (_yChange < 0)
		gettingShorter();

	/* Make it wider or narrower if necessary. */
	if (_xChange > 0)
		gettingWider();
	else if (_xChange < 0)
		gettingNarrower();

	constructInventory(EMPTY);
}

/**
 * Called from cursor module when cursor moves while inventory is up.
 */
void Dialogs::xMovement(int x) {
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
			changeingSize();
			break;

		case ID_RIGHT:
		case ID_TRIGHT:
		case ID_BRIGHT:
			_xChange += x;
			changeingSize();
			break;

		case ID_NONE:
			_vm->_cursor->GetCursorXY(&aniX, &aniY, false);
			invCursor(IC_AREA, aniX, aniY);
			break;

		case ID_MDCONT:
			slideMSlider(x, S_SLIDE);
			break;

		default:
			break;
		}
	}
}

/**
 * Called from cursor module when cursor moves while inventory is up.
 */
void Dialogs::yMovement(int y) {
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
			slideSlider(y, S_SLIDE);
			break;

		case ID_CSLIDE:
			slideCSlider(y, S_SLIDE);
			break;

		case ID_BOTTOM:
		case ID_BLEFT:
		case ID_BRIGHT:
			_yChange += y;
			changeingSize();
			break;

		case ID_TOP:
		case ID_TLEFT:
		case ID_TRIGHT:
			_yChange -= y;
			changeingSize();
			break;

		case ID_NONE:
			_vm->_cursor->GetCursorXY(&aniX, &aniY, false);
			invCursor(IC_AREA, aniX, aniY);
			break;

		default:
			break;
		}
	}
}

/**
 * Called when a drag is commencing.
 */
void Dialogs::invDragStart() {
	int curX, curY; // cursor's animation position

	_vm->_cursor->GetCursorXY(&curX, &curY, false);

	/*
	 * Do something different for Save/Restore screens
	 */
	if (_activeInv == INV_CONF) {
		int whichbox;

		whichbox = whichMenuBox(curX, curY, true);

		if (whichbox == IB_SLIDE) {
			_invDragging = ID_CSLIDE;
			slideCSlider(0, S_START);
		} else if (whichbox > 0 && (whichbox & IS_MASK)) {
			_invDragging = ID_MDCONT; // Mixing desk control
			cd.selBox = whichbox;
			slideMSlider(0, S_START);
		}
		return;
	}

	/*
	 * Normal operation
	 */
	switch (invArea(curX, curY)) {
	case I_HEADER:
		if (_invD[_activeInv].bMoveable) {
			_invDragging = ID_MOVE;
		}
		break;

	case I_SLIDE:
		_invDragging = ID_SLIDE;
		slideSlider(0, S_START);
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
void Dialogs::invDragEnd() {
	int curX, curY; // cursor's animation position

	_vm->_cursor->GetCursorXY(&curX, &curY, false);

	if (_invDragging != ID_NONE) {
		if (_invDragging == ID_SLIDE) {
			slideSlider(0, S_END);
		} else if (_invDragging == ID_CSLIDE) {
			; // No action
		} else if (_invDragging == ID_MDCONT) {
			slideMSlider(0, S_END);
		} else if (_invDragging == ID_MOVE) {
			; // No action
		} else {
			// Were re-sizing. Redraw the whole thing.
			dumpDobjArray();
			dumpObjArray();
			constructInventory(FULL);

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
	invCursor(IC_AREA, curX, curY);

	_xChange = _yChange = 0; // Probably no need, but does no harm!
}

bool Dialogs::menuDown(int lines) {
	if (cd.box == loadBox || cd.box == saveBox) {
		if (cd.extraBase < MAX_SAVED_FILES - NUM_RGROUP_BOXES) {
			firstFile(cd.extraBase + lines);
			addBoxes(true);
			return true;
		}
	} else if (cd.box == hopperBox1) {
		if (cd.extraBase < _numScenes - NUM_RGROUP_BOXES) {
			firstScene(cd.extraBase + lines);
			addBoxes(true);
			return true;
		}
	} else if (cd.box == hopperBox2) {
		if (cd.extraBase < _numEntries - NUM_RGROUP_BOXES) {
			firstEntry(cd.extraBase + lines);
			addBoxes(true);
			return true;
		}
	}
	return false;
}

bool Dialogs::menuUp(int lines) {
	if (cd.extraBase > 0) {
		if (cd.box == loadBox || cd.box == saveBox)
			firstFile(cd.extraBase - lines);
		else if (cd.box == hopperBox1)
			firstScene(cd.extraBase - lines);
		else if (cd.box == hopperBox2)
			firstEntry(cd.extraBase - lines);
		else
			return false;

		addBoxes(true);
		return true;
	}
	return false;
}

void Dialogs::menuRollDown() {
	if (menuDown(1)) {
		if (cd.selBox > 0)
			cd.selBox--;
		select(cd.selBox, true);
	}
}

void Dialogs::menuRollUp() {
	if (menuUp(1)) {
		if (cd.selBox < NUM_RGROUP_BOXES - 1)
			cd.selBox++;
		select(cd.selBox, true);
	}
}

void Dialogs::menuPageDown() {
	if (menuDown(NUM_RGROUP_BOXES - 1)) {
		cd.selBox = NUM_RGROUP_BOXES - 1;
		select(cd.selBox, true);
	}
}

void Dialogs::menuPageUp() {
	if (menuUp(NUM_RGROUP_BOXES - 1)) {
		cd.selBox = 0;
		select(cd.selBox, true);
	}
}

void Dialogs::inventoryDown() {
	// This code is a copy of the IB_SLIDE_DOWN case in invWalkTo
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

void Dialogs::inventoryUp() {
	// This code is a copy of the I_SLIDE_UP case in invWalkTo
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
 * menuAction
 */
void Dialogs::menuAction(int i, bool dbl) {

	if (i >= 0) {
		switch (cd.box[i].boxType) {
		case FLIP:
			if (dbl) {
				*(cd.box[i].ival) ^= 1; // XOR with true
				addBoxes(false);
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
					killInventory();
					invSaveGame();
					break;
				case LOADGAME:
					killInventory();
					invLoadGame();
					break;
				case HOPPER2:
					killInventory();
					openMenu(HOPPER_MENU2);
					break;
				case BF_CHANGESCENE:
					killInventory();
					hopAction();
					freeSceneHopper();
					break;
				default:
					break;
				}
			} else {
				select(i, false);
			}
			break;

		case FRGROUP:
			if (dbl) {
				select(i, false);
				languageChange();
			} else {
				select(i, false);
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
		confActionSpecial(i);
	}
}

void Dialogs::confActionSpecial(int i) {
	switch (i) {
	case IB_NONE:
	default:
		break;
	case IB_UP: // Scroll up
		if (cd.extraBase > 0) {
			if ((cd.box == loadBox) || (cd.box == saveBox))
				firstFile(cd.extraBase - 1);
			else if (cd.box == hopperBox1)
				firstScene(cd.extraBase - 1);
			else if (cd.box == hopperBox2)
				firstEntry(cd.extraBase - 1);

			addBoxes(true);
			if (cd.selBox < NUM_RGROUP_BOXES - 1)
				cd.selBox += 1;
			select(cd.selBox, true);
		}
		break;
	case IB_DOWN: // Scroll down
		if ((cd.box == loadBox) || (cd.box == saveBox)) {
			if (cd.extraBase < MAX_SAVED_FILES - NUM_RGROUP_BOXES) {
				firstFile(cd.extraBase + 1);
				addBoxes(true);
				if (cd.selBox)
					cd.selBox -= 1;
				select(cd.selBox, true);
			}
		} else if (cd.box == hopperBox1) {
			if (cd.extraBase < _numScenes - NUM_RGROUP_BOXES) {
				firstScene(cd.extraBase + 1);
				addBoxes(true);
				if (cd.selBox)
					cd.selBox -= 1;
				select(cd.selBox, true);
			}
		} else if (cd.box == hopperBox2) {
			if (cd.extraBase < _numEntries - NUM_RGROUP_BOXES) {
				firstEntry(cd.extraBase + 1);
				addBoxes(true);
				if (cd.selBox)
					cd.selBox -= 1;
				select(cd.selBox, true);
			}
		}
		break;

	case IB_SLIDE_UP:
		menuPageUp();
		break;

	case IB_SLIDE_DOWN:
		menuPageDown();
		break;
	}
}
// SLIDE_UP and SLIDE_DOWN on d click??????

void Dialogs::invPutDown(int index) {
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
			if (inventoryPos(_heldItem) != INV_HELDNOTIN)
				remFromInventory(_activeInv == INV_1 ? INV_2 : INV_1, _heldItem);
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
	invCursor(IC_DROP, aniX, aniY);
}

void Dialogs::invPickup(int index) {
	// Do nothing if not clicked on anything
	if (index == NOOBJECT)
		return;

	// If not holding anything
	if (_heldItem == INV_NOICON && _invD[_activeInv].contents[index] &&
	    ((TinselVersion <= 1) || _invD[_activeInv].contents[index] != _heldItem)) {
		// Pick-up
		auto invObj = getInvObject(_invD[_activeInv].contents[index]);
		_thisIcon = _invD[_activeInv].contents[index];
		if (TinselVersion >= 2)
			InvTinselEvent(invObj, PICKUP, INV_PICKUP, index);
		else if (invObj->getScript())
			InvTinselEvent(invObj, WALKTO, INV_PICKUP, index);

	} else if (_heldItem != INV_NOICON) {
		// Put-down
		auto invObj = getInvObject(_heldItem);

		// If DROPCODE set, send event, otherwise it's a putdown
		if (invObj->hasAttribute(InvObjAttr::IO_DROPCODE) && invObj->getScript())
			InvTinselEvent(invObj, PUTDOWN, INV_PICKUP, index);

		else if (!(invObj->hasAttribute(InvObjAttr::IO_ONLYINV1) && _activeInv != INV_1) && !(invObj->hasAttribute(InvObjAttr::IO_ONLYINV2) && _activeInv != INV_2)) {
			if (TinselVersion >= 2)
				invPutDown(index);
			else
				CoroScheduler.createProcess(PID_TCODE, InvPdProcess, &index, sizeof(index));
		}
	}
}

/**
 * Handle WALKTO event (Pick up/put down event)
 */
void Dialogs::invWalkTo(const Common::Point &coOrds) {
	int i;

	switch (invArea(coOrds.x, coOrds.y)) {
	case I_NOTIN:
		if (_activeInv == INV_CONV)
			convAction(INV_CLOSEICON);
		if ((cd.box == hopperBox1) || (cd.box == hopperBox2))
			freeSceneHopper();
		killInventory();
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
				menuAction(whichMenuBox(coOrds.x, coOrds.y, false), false);
		} else {
			Common::Point pt = coOrds;
			i = invItem(pt, false);

			// To cater for drop in dead space between icons,
			// look 1 pixel right, then 1 down, then 1 right and down.
			if (i == INV_NOICON && _heldItem != INV_NOICON &&
			    (_activeInv == INV_1 || _activeInv == INV_2)) {
				pt.x += 1; // 1 to the right
				i = invItem(pt, false);
				if (i == INV_NOICON) {
					pt.x -= 1; // 1 down
					pt.y += 1;
					i = invItem(pt, false);
					if (i == INV_NOICON) {
						pt.x += 1; // 1 down-right
						i = invItem(pt, false);
					}
				}
			}

			if (_activeInv == INV_CONV) {
				convAction(i);
			} else
				invPickup(i);
		}
		break;

	default:
		break;
	}
}

void Dialogs::invAction() {
	int index;
	const InventoryObject *invObj;
	int aniX, aniY;
	int i;

	_vm->_cursor->GetCursorXY(&aniX, &aniY, false);

	switch (invArea(aniX, aniY)) {
	case I_BODY:
		if (_activeInv == INV_CONF) {
			if (!_InventoryHidden)
				menuAction(whichMenuBox(aniX, aniY, false), true);
		} else if (_activeInv == INV_CONV) {
			index = invItem(&aniX, &aniY, false);
			convAction(index);
		} else {
			index = invItem(&aniX, &aniY, false);
			if (index != INV_NOICON) {
				if (_invD[_activeInv].contents[index] && _invD[_activeInv].contents[index] != _heldItem) {
					invObj = getInvObject(_invD[_activeInv].contents[index]);
					if (TinselVersion >= 2)
						_thisIcon = _invD[_activeInv].contents[index];
					if ((TinselVersion >= 2) || (invObj->getScript()))
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
		dumpDobjArray();
		dumpObjArray();
		constructInventory(FULL);
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

void Dialogs::invLook(const Common::Point &coOrds) {
	int index;
	Common::Point pt = coOrds;

	switch (invArea(pt.x, pt.y)) {
	case I_BODY:
		index = invItem(pt, false);
		if (index != INV_NOICON) {
			if (_invD[_activeInv].contents[index] && _invD[_activeInv].contents[index] != _heldItem) {
				auto invObj = getInvObject(_invD[_activeInv].contents[index]);
				if (invObj->getScript())
					InvTinselEvent(invObj, LOOK, INV_LOOK, index);
			}
		}
		break;

	case I_NOTIN:
		if (_activeInv == INV_CONV)
			convAction(INV_CLOSEICON);
		killInventory();
		break;

	default:
		break;
	}
}

/**************************************************************************/
/********************* Incoming events ************************************/
/**************************************************************************/

void Dialogs::eventToInventory(PLR_EVENT pEvent, const Common::Point &coOrds) {
	if (_InventoryHidden)
		return;

	if (TinselVersion == 3) {
		// If the Notebook handles the event, it has been consumed.
		if (_vm->_notebook->handleEvent(pEvent, coOrds)) {
			return;
		}
	}

	switch (pEvent) {
	case PLR_PROV_WALKTO:
		if (menuActive()) {
			ProcessedProvisional();
			invWalkTo(coOrds);
		}
		break;

	case PLR_WALKTO: // PLR_SLEFT
		invWalkTo(coOrds);
		break;

	case INV_LOOK: // PLR_SRIGHT
		if (menuActive())
			invWalkTo(coOrds);
		else
			invLook(coOrds);
		break;

	case PLR_ACTION: // PLR_DLEFT
		if (_invDragging != ID_MDCONT)
			invDragEnd();
		invAction();
		break;

	case PLR_DRAG1_START: // Left drag start
		if (TinselVersion < 3 || _inventoryState == ACTIVE_INV) // inventoryActive, but not Notebook
			invDragStart();
		break;

	case PLR_DRAG1_END: // Left drag end
		invDragEnd();
		break;

	case PLR_ESCAPE:
		if (menuActive()) {
			if (cd.box != optionBox && cd.box != hopperBox1 && cd.box != hopperBox2)
				_reOpenMenu = true;
			if ((cd.box == hopperBox1) || (cd.box == hopperBox2))
				freeSceneHopper();
		}
		closeInventory();
		break;

	case PLR_PGDN:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			menuPageDown();
		} else {
			// Inventory window
			inventoryDown();
		}
		break;

	case PLR_PGUP:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			menuPageUp();
		} else {
			// Inventory window
			inventoryUp();
		}
		break;

	case PLR_WHEEL_DOWN:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			menuRollDown();
		} else {
			// Inventory window
			inventoryDown();
		}
		break;

	case PLR_WHEEL_UP:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			menuRollUp();
		} else {
			// Inventory window
			inventoryUp();
		}
		break;

	case PLR_HOME:
		if (_activeInv == INV_MENU) {
			// Load or Save screen
			if (cd.box == loadBox || cd.box == saveBox)
				firstFile(0);
			else if (cd.box == hopperBox1)
				firstScene(0);
			else if (cd.box == hopperBox2)
				firstEntry(0);
			else
				break;

			addBoxes(true);
			cd.selBox = 0;
			select(cd.selBox, true);
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
				firstFile(MAX_SAVED_FILES); // Will get reduced to appropriate value
			else if (cd.box == hopperBox1)
				firstScene(_numScenes); // Will get reduced to appropriate value
			else if (cd.box == hopperBox2)
				firstEntry(_numEntries); // Will get reduced to appropriate value
			else
				break;

			addBoxes(true);
			cd.selBox = 0;
			select(cd.selBox, true);
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

const FILM *Dialogs::getObjectFilm(int object) const {
	return (const FILM*)_vm->_handle->LockMem(_invFilms[getObjectIndex(object)]);
}

/**
 * Called from Glitter function invdepict()
 * Changes (permanently) the animation film for that object.
 */
void Dialogs::setObjectFilm(int object, SCNHANDLE hFilm) {
	_invObjects->SetObjectFilm(object, hFilm);

	if (TinselVersion == 3) {
		_invFilms[getObjectIndex(object)] = hFilm;
	}

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

	if (TinselVersion >= 2) {
		for (int i = 0; i < _invObjects->numObjects(); ++i)
			s.syncAsUint32LE(_invFilms[i]);
		s.syncAsUint32LE(_heldFilm);
	}
}

// Let the debugger know all the available clues.
Common::Array<int> Dialogs::getAllNotebookClues() const {
	Common::Array<int> clues;
	for (int i = 0; i < _invObjects->numObjects(); i++) {
		auto obj = _invObjects->GetObjectByIndex(i);
		if (obj->hasAttribute(InvObjAttr::NOTEBOOK_CLUE)) {
			clues.push_back(obj->getId());
		}
	}
	return clues;
}

/**************************************************************************/
/************************ Initialisation stuff ****************************/
/**************************************************************************/

/**
 * Called from PlayGame(), stores handle to inventory objects' data -
 * its id, animation film and Glitter script.
 */
// Note: the SCHANDLE type here has been changed to a void*
void Dialogs::registerIcons(void *cptr, int num) {
	int numObjects = num;
	_invObjects = InstantiateInventoryObjects((const byte*)cptr, numObjects);
	if (TinselVersion >= 2) {
		if (_invFilms == NULL) {
			// First time - allocate memory
			MEM_NODE *node = MemoryAllocFixed(numObjects * sizeof(SCNHANDLE));
			assert(node);
			_invFilms = (SCNHANDLE *)MemoryDeref(node);
			if (_invFilms == NULL)
				error(NO_MEM, "inventory scripts");
			memset(_invFilms, 0, numObjects * sizeof(SCNHANDLE));
		}

		// Add defined permanent conversation icons
		// and store all the films separately
		for (int i = 0; i < numObjects; i++) {
			auto pio = _invObjects->GetObjectByIndex(i);
			if (pio->hasAttribute(InvObjAttr::PERMACONV))
				permaConvIcon(pio->getId(), pio->hasAttribute(InvObjAttr::CONVENDITEM));

			_invFilms[i] = pio->getIconFilm();
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
	pfilm = (const FILM *)_vm->_handle->LockMem(hf);
	assert(FROM_32(pfilm->numreels) >= (uint32)((TinselVersion >= 2) ? T2_HOPEDFORREELS : T1_HOPEDFORREELS)); // not as many reels as expected
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
	pfilm = (const FILM *)_vm->_handle->LockMem(hf);
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
	int startx = 100;
	int starty = 100;
	if (TinselVersion == 3) {
		MinWidth = 3;
		MinHeight = 2;
		StartWidth = 3;
		StartHeight = 2;
		MaxWidth = 3;
		MaxHeight = 2;
		startx = 0;
		starty = 50;
	}
	idec_inv(INV_2, text, MaxContents, MinWidth, MinHeight,
			 StartWidth, StartHeight, MaxWidth, MaxHeight,
			 startx, starty, true);
}

/**
 * Called from Glitter functions: dec_invMain
 * - Declare inventories 1,3 and 4 and hail the loadingscreen(?) scene.
 */
void Dialogs::idec_invMain(SCNHANDLE text, int MaxContents) {
	idec_inv(INV_1, text, MaxContents,3, 2, 3, 2, 3, 2, 39,
			72, false);
	idec_inv(INV_MENU, 0, 3, 2, 2, 2, 1, 3, 1, 100, 100,
			false);
	idec_inv(INV_4, text, MaxContents,3, 2, 3, 2, 3, 2, 39,
			72, false);

	const char *fileName = _vm->getSceneFile(TextLanguage());
	SCNHANDLE sceneHandle = _vm->_handle->FindLanguageSceneHandle(fileName);
	DoHailScene(sceneHandle);
}

/**
 * Called from Glitter function 'GetInvLimit()'
 */
int Dialogs::invGetLimit(int invno) {
	assert(invno == INV_1 || invno == INV_2); // only INV_1 and INV_2 supported

	return _invD[invno].MaxInvObj;
}

/**
 * Called from Glitter function 'SetInvLimit()'
 */
void Dialogs::invSetLimit(int invno, int MaxContents) {
	assert(invno == INV_1 || invno == INV_2);       // only INV_1 and INV_2 supported
	assert(MaxContents >= _invD[invno].NoofItems); // can't reduce maximum contents below current contents

	if (MaxContents > MAX_ININV)
		MaxContents = MAX_ININV; // Max contents

	_invD[invno].MaxInvObj = MaxContents;
}

/**
 * Called from Glitter function 'SetInvSize()'
 */
void Dialogs::invSetSize(int invno, int MinWidth, int MinHeight,
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

bool Dialogs::isTopWindow() {
	return (_inventoryState == BOGUS_INV);
}

bool Dialogs::menuActive() {
	return (_inventoryState == ACTIVE_INV && _activeInv == INV_CONF);
}

bool Dialogs::isConvWindow() {
	return (_inventoryState == ACTIVE_INV && _activeInv == INV_CONV);
}

void Dialogs::callFunction(BFUNC boxFunc) {

	Common::Keymapper *keymapper = _vm->getEventManager()->getKeymapper();
	if (boxFunc == CLOSEWIN || boxFunc == SAVEGAME || boxFunc == LOADGAME) {
		keymapper->getKeymap("game-shortcuts")->setEnabled(true);
		keymapper->getKeymap("saveload-shortcuts")->setEnabled(false);
	}

	switch (boxFunc) {
	case SAVEGAME:
		killInventory();
		invSaveGame();
		break;
	case LOADGAME:
		killInventory();
		invLoadGame();
		break;
	case IQUITGAME:
		_vm->quitGame();
		break;
	case CLOSEWIN:
		killInventory();
		if ((cd.box == hopperBox1) || (cd.box == hopperBox2))
			freeSceneHopper();
		break;
	case OPENLOAD:
		killInventory();
		openMenu(LOAD_MENU);
		break;
	case OPENSAVE:
		killInventory();
		openMenu(SAVE_MENU);
		break;
	case OPENREST:
		killInventory();
		openMenu(RESTART_MENU);
		break;
	case OPENSOUND:
		killInventory();
		openMenu(SOUND_MENU);
		break;
	case OPENCONT:
		killInventory();
		openMenu(CONTROLS_MENU);
		break;
#ifndef JAPAN
	case OPENSUBT:
		killInventory();
		openMenu(SUBTITLES_MENU);
		break;
#endif
	case OPENQUIT:
		killInventory();
		openMenu(QUIT_MENU);
		break;
	case INITGAME:
		killInventory();
		FnRestartGame();
		break;
	case CLANG:
		if (!languageChange())
			killInventory();
		break;
	case RLANG:
		killInventory();
		break;
	case HOPPER2:
		_vm->_dialogs->killInventory();
		_vm->_dialogs->openMenu(HOPPER_MENU2);
		break;
	case BF_CHANGESCENE:
		_vm->_dialogs->killInventory();
		_vm->_dialogs->hopAction();
		_vm->_dialogs->freeSceneHopper();
		break;
	default:
		break;
	}
}

const FILM *Dialogs::getWindowData() {
	return (const FILM *)_vm->_handle->LockMem(_hWinParts);
}

void Dialogs::redraw() {
	if (displayObjectsActive()) {
		if (_ItemsChanged && !configurationIsActive() && !inventoryIsHidden()) {
			fillInInventory();

			// Needed when clicking on scroll bar.
			int curX, curY;
			_vm->_cursor->GetCursorXY(&curX, &curY, false);
			invCursor(IC_AREA, curX, curY);

			_ItemsChanged = false;
		}
		if (!configurationIsActive()) {
			for (int i = 0; i < MAX_ICONS; i++) {
				if (_iconArray[i] != NULL)
					StepAnimScript(&_iconAnims[i]);
			}
		}
		if (isMixingDeskControl()) {
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
				slideMSlider(0, (cd.selBox & IS_RIGHT) ? S_TIMEUP : S_TIMEDN);
			}
		}
	}
	if (TinselVersion == 3) {
		_vm->_notebook->stepAnimScripts();
	}
}

// Noir
bool Dialogs::isConvAndNotMove() {
	// TODO: Ensure that the used global is correct
	// If this is the right mapping, the variable is reversed in Noir
	return isConvWindow() && !_bMoveOnUnHide;
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

	_vm->_dialogs->invPutDown(*pindex);

	CORO_END_CODE;
}

static void ButtonPress(CORO_PARAM, CONFBOX *box) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;

	assert(box->boxType == AAGBUT || box->boxType == ARSGBUT);

	// Replace highlight image with normal image
	pfilm = _vm->_dialogs->getWindowData();
	MultiDeleteObjectIfExists(FIELD_STATUS, &_vm->_dialogs->_iconArray[HL1]);
	pfilm = _vm->_dialogs->getWindowData();
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->addObject(&pfilm->reels[box->bi + NORMGRAPH], -1);
	MultiSetAniXYZ(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->currentInventoryX() + box->xpos,
				   _vm->_dialogs->currentInventoryY() + box->ypos,
	               Z_INV_ICONS + 1);

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// Replace normal image with depresses image
	pfilm = _vm->_dialogs->getWindowData();
	MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->addObject(&pfilm->reels[box->bi + DOWNGRAPH], -1);
	MultiSetAniXYZ(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->currentInventoryX() + box->xpos,
				   _vm->_dialogs->currentInventoryY() + box->ypos,
	               Z_INV_ICONS + 1);

	// Hold depressed image for 2 frames
	CORO_SLEEP(2);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// Replace depressed image with normal image
	pfilm = _vm->_dialogs->getWindowData();
	MultiDeleteObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->addObject(&pfilm->reels[box->bi + NORMGRAPH], -1);
	MultiSetAniXYZ(_vm->_dialogs->_iconArray[HL1], _vm->_dialogs->currentInventoryX() + box->xpos,
				   _vm->_dialogs->currentInventoryY() + box->ypos,
	               Z_INV_ICONS + 1);

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
	MultiDeleteObjectIfExists(FIELD_STATUS, &_vm->_dialogs->_iconArray[HL1]);

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (!_vm->_dialogs->inventoryIsActive())
		return;

	// Add depressed image
	pfilm = _vm->_dialogs->getWindowData();
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->addObject(&pfilm->reels[box->bi + DOWNGRAPH], -1);
	MultiSetAniXYZ(_vm->_dialogs->_iconArray[HL1],
				   _vm->_dialogs->currentInventoryX() + box->xpos,
				   _vm->_dialogs->currentInventoryY() + box->ypos,
	               Z_INV_ICONS + 1);

	// Hold depressed image for 1 frame
	CORO_SLEEP(1);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// Toggle state
	(*box->ival) = *(box->ival) ^ 1; // XOR with true
	box->bi = *(box->ival) ? IX_TICK1 : IX_CROSS1;
	_vm->_dialogs->addBoxes(false);
	// Keep highlight (e.g. flag)
	if (cd.selBox != NOBOX)
		_vm->_dialogs->select(cd.selBox, true);

	// New state, depressed image
	pfilm = _vm->_dialogs->getWindowData();
	MultiDeleteObjectIfExists(FIELD_STATUS, &_vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->addObject(&pfilm->reels[box->bi + DOWNGRAPH], -1);
	MultiSetAniXYZ(_vm->_dialogs->_iconArray[HL1],
				   _vm->_dialogs->currentInventoryX() + box->xpos,
				   _vm->_dialogs->currentInventoryY() + box->ypos,
	               Z_INV_ICONS + 1);

	// Hold new depressed image for 1 frame
	CORO_SLEEP(1);
	if (_vm->_dialogs->_iconArray[HL1] == NULL)
		return;

	// New state, normal
	MultiDeleteObjectIfExists(FIELD_STATUS, &_vm->_dialogs->_iconArray[HL1]);

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (!_vm->_dialogs->inventoryIsActive())
		return;

	// New state, highlighted
	pfilm = _vm->_dialogs->getWindowData();
	MultiDeleteObjectIfExists(FIELD_STATUS, &_vm->_dialogs->_iconArray[HL1]);
	_vm->_dialogs->_iconArray[HL1] = _vm->_dialogs->addObject(&pfilm->reels[box->bi + HIGRAPH], -1);
	MultiSetAniXYZ(_vm->_dialogs->_iconArray[HL1],
				   _vm->_dialogs->currentInventoryX() + box->xpos,
				   _vm->_dialogs->currentInventoryY() + box->ypos,
	               Z_INV_ICONS + 1);

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

		_vm->_dialogs->redraw();

		if (_vm->_dialogs->_buttonEffect.bButAnim) {
			assert(_vm->_dialogs->_buttonEffect.box);
			if (_vm->_dialogs->_buttonEffect.press) {
				if (_vm->_dialogs->_buttonEffect.box->boxType == AAGBUT || _vm->_dialogs->_buttonEffect.box->boxType == ARSGBUT)
					CORO_INVOKE_1(ButtonPress, _vm->_dialogs->_buttonEffect.box);
				_vm->_dialogs->callFunction(_vm->_dialogs->_buttonEffect.box->boxFunc);
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
	const InventoryObject *pinvo;
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

	if (TinselVersion <= 1)
		CORO_INVOKE_1(AllowDclick, to->bev);

	_ctx->pic = InitInterpretContext(GS_INVENTORY, to->pinvo->getScript(), to->event, NOPOLY, 0, to->pinvo,
	                                 to->myEscape);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	if (to->event == POINTED) {
		_ctx->ThisPointedWait = ++_vm->_dialogs->_pointedWaitCount;
		while (1) {
			CORO_SLEEP(1);
			int x, y;
			_vm->_cursor->GetCursorXY(&x, &y, false);
			if (_vm->_dialogs->invItemId(x, y) != to->pinvo->getId())
				break;

			// Fix the 'repeated pressing bug'
			if (_ctx->ThisPointedWait != _vm->_dialogs->_pointedWaitCount)
				CORO_KILL_SELF();
		}

		_ctx->pic = InitInterpretContext(GS_INVENTORY, to->pinvo->getScript(), UNPOINT, NOPOLY, 0, to->pinvo);
		CORO_INVOKE_1(Interpret, _ctx->pic);
	}

	CORO_END_CODE;
}

/**
 * Run inventory item's Glitter code
 */
static void InvTinselEvent(const InventoryObject *pinvo, TINSEL_EVENT event, PLR_EVENT be, int index) {
	OP_INIT to = {pinvo, event, be, 0};

	if (_vm->_dialogs->inventoryIsHidden() || ((TinselVersion >= 2) && !pinvo->getScript()))
		return;

	_vm->_dialogs->_glitterIndex = index;
	CoroScheduler.createProcess(PID_TCODE, ObjectProcess, &to, sizeof(to));
}

extern void ObjectEvent(CORO_PARAM, int objId, TINSEL_EVENT event, bool bWait, int myEscape, bool *result) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	Common::PROCESS *pProc;
	const InventoryObject *pInvo;
	OP_INIT op;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (result)
		*result = false;
	_ctx->pInvo = _vm->_dialogs->getInvObject(objId);
	if (!_ctx->pInvo->getScript())
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
