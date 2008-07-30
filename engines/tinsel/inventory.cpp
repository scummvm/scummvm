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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Handles the inventory and conversation windows.
 *
 * And the save/load game windows. Some of this will be platform
 * specific - I'll try to separate this ASAP.
 *
 * And there's still a bit of tidying and commenting to do yet.
 */

//#define USE_3FLAGS 1

#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/inventory.h"
#include "tinsel/multiobj.h"
#include "tinsel/music.h"
#include "tinsel/polygons.h"
#include "tinsel/savescn.h"
#include "tinsel/sched.h"
#include "tinsel/serializer.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "tinsel/text.h"
#include "tinsel/timers.h"		// For ONE_SECOND constant
#include "tinsel/tinsel.h"		// For engine access
#include "tinsel/token.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"

namespace Tinsel {

//----------------- EXTERNAL GLOBAL DATA --------------------

// In DOS_DW.C
extern bool bRestart;		// restart flag - set to restart the game

#ifdef MAC_OPTIONS
// In MAC_SOUND.C
extern int volMaster;
#endif


//----------------- EXTERNAL FUNCTIONS ---------------------

// Tag functions in PDISPLAY.C
extern void EnableTags(void);
extern void DisableTags(void);


//----------------- LOCAL DEFINES --------------------

//#define ALL_CURSORS

#define INV_PICKUP	BE_SLEFT		// Local names
#define INV_LOOK	BE_SRIGHT		//	for button events
#define INV_ACTION	BE_DLEFT		//


// For SlideSlider() and similar
enum SSFN {
	S_START, S_SLIDE, S_END, S_TIMEUP, S_TIMEDN
};

/** attribute values - may become bit field if further attributes are added */
enum {
 IO_ONLYINV1	= 0x01,
 IO_ONLYINV2	= 0x02,
 IO_DROPCODE	= 0x04
};

//-----------------------
// Moveable window translucent rectangle position limits
enum {
	MAXLEFT		= 315,		//
	MINRIGHT	= 3,		// These values keep 2 pixcells
	MINTOP		= -13,		// of header on the screen.
	MAXTOP		= 195		//
};

//-----------------------
// Indices into winPartsf's reels
#define IX_SLIDE 0		// Slider
#define IX_V26	1
#define IX_V52	2
#define IX_V78	3
#define IX_V104	4
#define IX_V130	5
#define IX_H26	6
#define IX_H52	7
#define IX_H78	8
#define IX_H104	9
#define IX_H130	10
#define IX_H156	11
#define IX_H182	12
#define IX_H208	13
#define IX_H234	14
#define IX_TL	15		// Top left corner
#define IX_TR	16		// Top right corner
#define IX_BL	17		// Bottom left corner
#define IX_BR	18		// Bottom right corner
#define IX_H25	19
#define IX_V11	20
#define IX_RTL	21		// Re-sizing top left corner
#define IX_RTR	22		// Re-sizing top right corner
#define IX_RBR	23		// Re-sizing bottom right corner
#define IX_CURLR	24		// }
#define IX_CURUD	25		// }
#define IX_CURDU	26		// } Custom cursors
#define IX_CURDD	27		// }
#define IX_CURUP	28		// }
#define IX_CURDOWN	29		// }
#define IX_MDGROOVE	30	// 'Mixing desk' slider background
#define IX_MDSLIDER	34	// 'Mixing desk' slider

#define IX_BLANK1	35		//
#define IX_BLANK2	36		//
#define IX_BLANK3	37		//
#define IX_CIRCLE1	38	//
#define IX_CIRCLE2	39	//
#define IX_CROSS1	40		//
#define IX_CROSS2	41		//
#define IX_CROSS3	42		//
#define IX_QUIT1	43	//
#define IX_QUIT2	44	//
#define IX_QUIT3	45	//
#define IX_TICK1	46		//
#define IX_TICK2	47		//
#define IX_TICK3	48		//
#define IX_NTR		49		// New top right corner
#define HOPEDFORREELS	50

#define NORMGRAPH	0
#define DOWNGRAPH	1
#define HIGRAPH		2
//-----------------------
#define FIX_UK		0
#define FIX_FR		1
#define FIX_GR		2
#define FIX_IT		3
#define FIX_SP		4
#define FIX_USA		5
#define HOPEDFORFREELS	6	// Expected flag reels
//-----------------------

#define MAX_ININV	150	// Max in an inventory
#define MAX_CONVBASIC	10	// Max permanent conversation icons

#define MAXHICONS	10	// Max dimensions of
#define MAXVICONS	6	// an inventory window

#define ITEM_WIDTH	25	// Dimensions of an icon
#define ITEM_HEIGHT	25	//

// Number of objects that makes up an empty window
#define MAX_WCOMP	21		// 4 corners + (3+3) sides + (2+2) extra sides
					// + Bground + title + slider
					// + more Needed for save game window

#define MAX_ICONS	MAXHICONS*MAXVICONS



//----------------- LOCAL GLOBAL DATA --------------------

//----- Permanent data (compiled in) -----

// Save game name editing cursor

#define CURSOR_CHAR	'_'
char sCursor[2]	= { CURSOR_CHAR, 0 };
static const int hFillers[MAXHICONS] = {
	IX_H26,			// 2 icons wide
	IX_H52,			// 3
	IX_H78,			// 4
	IX_H104,		// 5
	IX_H130,		// 6
	IX_H156,		// 7
	IX_H182,		// 8
	IX_H208,		// 9
	IX_H234			// 10 icons wide
};
static const int vFillers[MAXVICONS] = {
	IX_V26,			// 2 icons high
	IX_V52,			// 3
	IX_V78,			// 4
	IX_V104,		// 5
	IX_V130			// 6 icons high
};


//----- Permanent data (set once) -----

static SCNHANDLE winPartsf = 0;	// Window members and cursors' graphic data
static SCNHANDLE flagFilm = 0;	// Window members and cursors' graphic data
static SCNHANDLE configStrings[20];

static INV_OBJECT *pio = 0;		// Inventory objects' data
static int numObjects = 0;		// Number of inventory objects


//----- Permanent data (updated, valid while inventory closed) -----

static enum {NO_INV, IDLE_INV, ACTIVE_INV, BOGUS_INV} InventoryState;

static int HeldItem = INV_NOICON;	// Current held item

struct INV_DEF {

	int MinHicons;		// }
	int MinVicons;		// } Dimension limits
	int MaxHicons;		// }
	int MaxVicons;		// }

	int NoofHicons;		// }
	int NoofVicons;		// } Current dimentsions

	int ItemOrder[MAX_ININV];	// Contained items
	int NoofItems;			// Current number of held items

	int FirstDisp;		// Index to first item currently displayed

	int inventoryX;		// } Display position
	int inventoryY;		// } 
	int otherX;		// } Display position
	int otherY;		// } 

	int MaxInvObj;		// Max. allowed contents

	SCNHANDLE hInvTitle;	// Window heading

	bool resizable;		// Re-sizable window?
	bool moveable;		// Moveable window?

	int sNoofHicons;	// }
	int sNoofVicons;	// } Current dimensions

	bool bMax;		// Maximised last time open?

};

static INV_DEF InvD[NUM_INV];		// Conversation + 2 inventories + ...


// Permanent contents of conversation inventory
static int Inv0Order[MAX_CONVBASIC];	// Basic items i.e. permanent contents
static int Num0Order = 0;			// - copy to conv. inventory at pop-up time



//----- Data pertinant to current active inventory -----

static int ino = 0;		// Which inventory is currently active

static bool InventoryHidden = false;
static bool InventoryMaximised = false;

static enum {	ID_NONE, ID_MOVE, ID_SLIDE,
		ID_BOTTOM, ID_TOP, ID_LEFT, ID_RIGHT,
		ID_TLEFT, ID_TRIGHT, ID_BLEFT, ID_BRIGHT,
		ID_CSLIDE, ID_MDCONT } InvDragging;

static int SuppH = 0;		// 'Linear' element of
static int SuppV = 0;		// dimensions during re-sizing

static int Ychange = 0;		//
static int Ycompensate = 0;		// All to do with re-sizing.
static int Xchange = 0;		//
static int Xcompensate = 0;		//

static bool ItemsChanged = 0;	// When set, causes items to be re-drawn

static bool bOpenConf = 0;

static int TL = 0, TR = 0, BL = 0, BR = 0;	// Used during window construction
static int TLwidth = 0, TLheight = 0;	//
static int TRwidth = 0;		//
static int BLheight = 0;		//



static OBJECT	*objArray[MAX_WCOMP];	// Current display objects (window)
static OBJECT	*iconArray[MAX_ICONS];	// Current display objects (icons)
static ANIM		iconAnims[MAX_ICONS];
static OBJECT	*DobjArray[MAX_WCOMP];	// Current display objects (re-sizing window)

static OBJECT *RectObject = 0, *SlideObject = 0;	// Current display objects, for reference
					// objects are in objArray.

static int slideY = 0;			// For positioning the slider
static int slideYmax = 0, slideYmin = 0;	// 

// Also to do with the slider
static struct { int n; int y; } slideStuff[MAX_ININV+1];

#define MAXSLIDES 4
struct MDSLIDES {
	int	num;
	OBJECT	*obj;
	int	min, max;
};
static MDSLIDES mdSlides[MAXSLIDES];
static int numMdSlides = 0;

static int GlitterIndex = 0;

static HPOLYGON thisConvPoly = 0;			// Conversation code is in a polygon code block
static int thisConvIcon = 0;				// Passed to polygon code via convIcon()
static int pointedIcon = INV_NOICON;		// used by InvLabels - icon pointed to on last call
static volatile int PointedWaitCount = 0;	// used by InvTinselProcess - fix the 'repeated pressing bug'
static int sX = 0;							// used by SlideMSlider() - current x-coordinate
static int lX = 0;							// used by SlideMSlider() - last x-coordinate

//----- Data pertinant to configure (incl. load/save game) -----

#define COL_MAINBOX	TBLUE1		// Base blue colour
#define COL_BOX		TBLUE1
#define COL_HILIGHT	TBLUE4

#ifdef JAPAN
#define BOX_HEIGHT	17
#define EDIT_BOX1_WIDTH	149
#else
#define BOX_HEIGHT	13
#define EDIT_BOX1_WIDTH	145
#endif
#define EDIT_BOX2_WIDTH	166

// RGROUP	Radio button group - 1 is selectable at a time. Action on double click
// ARSBUT	Action if a radio button is selected
// AABUT	Action always
// AATBUT	Action always, text box
// AAGBUT	Action always, graphic button
// SLIDER	Not a button at all
enum BTYPE {
	RGROUP, ARSBUT, AABUT, AATBUT, ARSGBUT, AAGBUT, SLIDER,
	TOGGLE, DCTEST, FLIP, FRGROUP, NOTHING
};

enum BFUNC {
	NOFUNC, SAVEGAME, LOADGAME, IQUITGAME, CLOSEWIN,
	OPENLOAD, OPENSAVE, OPENREST,
	OPENSOUND, OPENCONT,
#ifndef JAPAN
	OPENSUBT,
#endif
	OPENQUIT,
	INITGAME, MIDIVOL,
	CLANG, RLANG
#ifdef MAC_OPTIONS
	, MASTERVOL, SAMPVOL 
#endif
};

struct CONFBOX {
	BTYPE	boxType;
	BFUNC	boxFunc;
	char	*boxText;
	int	ixText;
	int	xpos;
	int	ypos;
	int	w;		// Doubles as max value for SLIDERs
	int	h;		// Doubles as iteration size for SLIDERs
	int	*ival;
	int	bi;		// Base index for AAGBUTs
};


#define NO_HEADING		(-1)
#define USE_POINTER		(-1)
#define SIX_LOAD_OPTION		0
#define SIX_SAVE_OPTION		1
#define SIX_RESTART_OPTION	2
#define SIX_SOUND_OPTION	3
#define SIX_CONTROL_OPTION	4
#ifndef JAPAN
#define SIX_SUBTITLES_OPTION	5
#endif
#define SIX_QUIT_OPTION		6
#define SIX_RESUME_OPTION	7
#define SIX_LOAD_HEADING	8
#define SIX_SAVE_HEADING	9
#define SIX_RESTART_HEADING	10
#define SIX_MVOL_SLIDER		11
#define SIX_SVOL_SLIDER		12
#define SIX_VVOL_SLIDER		13
#define SIX_DCLICK_SLIDER	14
#define SIX_DCLICK_TEST		15
#define SIX_SWAP_TOGGLE		16
#define SIX_TSPEED_SLIDER	17
#define SIX_STITLE_TOGGLE	18
#define SIX_QUIT_HEADING	19


/*-------------------------------------------------------------*\
| This is the main menu (that comes up when you hit F1 on a PC)	|
\*-------------------------------------------------------------*/

#ifdef JAPAN
#define FBY	11	// y-offset of first button
#define FBX	13	// x-offset of first button
#else
#define FBY	20	// y-offset of first button
#define FBX	15	// x-offset of first button
#endif

CONFBOX optionBox[] = {

 { AATBUT, OPENLOAD, NULL, SIX_LOAD_OPTION,	FBX, FBY,			EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
 { AATBUT, OPENSAVE, NULL, SIX_SAVE_OPTION,	FBX, FBY + (BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
 { AATBUT, OPENREST, NULL, SIX_RESTART_OPTION,	FBX, FBY + 2*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
 { AATBUT, OPENSOUND, NULL, SIX_SOUND_OPTION,	FBX, FBY + 3*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
 { AATBUT, OPENCONT, NULL, SIX_CONTROL_OPTION,	FBX, FBY + 4*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
#ifdef JAPAN
// TODO: If in JAPAN mode, simply disable the subtitles button?
 { AATBUT, OPENQUIT, NULL, SIX_QUIT_OPTION,	FBX, FBY + 5*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
 { AATBUT, CLOSEWIN, NULL, SIX_RESUME_OPTION,	FBX, FBY + 6*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 }
#else
 { AATBUT, OPENSUBT, NULL, SIX_SUBTITLES_OPTION,FBX, FBY + 5*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
 { AATBUT, OPENQUIT, NULL, SIX_QUIT_OPTION,	FBX, FBY + 6*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 },
 { AATBUT, CLOSEWIN, NULL, SIX_RESUME_OPTION,	FBX, FBY + 7*(BOX_HEIGHT + 2),	EDIT_BOX1_WIDTH, BOX_HEIGHT, NULL, 0 }
#endif

};

/*-------------------------------------------------------------*\
| These are the load and save game menus.			|
\*-------------------------------------------------------------*/

#ifdef JAPAN
#define NUM_SL_RGROUP	7	// number of visible slots
#define SY		32	// y-position of first slot
#else
#define NUM_SL_RGROUP	9	// number of visible slots
#define SY		31	// y-position of first slot
#endif

CONFBOX loadBox[NUM_SL_RGROUP+2] = {

 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY,				EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + (BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + 2*(BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + 3*(BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + 4*(BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + 5*(BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + 6*(BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
#ifndef JAPAN
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + 7*(BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, LOADGAME, NULL, USE_POINTER, 28, SY + 8*(BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
#endif
 { ARSGBUT, LOADGAME, NULL, USE_POINTER, 230, 44,	23, 19, NULL, IX_TICK1 },
 { AAGBUT, CLOSEWIN, NULL, USE_POINTER, 230, 44+47,	23, 19, NULL, IX_CROSS1 }

};

CONFBOX saveBox[NUM_SL_RGROUP+2] = {

 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY,			EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + (BOX_HEIGHT + 2),	EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + 2*(BOX_HEIGHT + 2),EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + 3*(BOX_HEIGHT + 2),EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + 4*(BOX_HEIGHT + 2),EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + 5*(BOX_HEIGHT + 2),EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + 6*(BOX_HEIGHT + 2),EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
#ifndef JAPAN
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + 7*(BOX_HEIGHT + 2),EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
 { RGROUP, SAVEGAME, NULL, USE_POINTER, 28,	SY + 8*(BOX_HEIGHT + 2),EDIT_BOX2_WIDTH, BOX_HEIGHT, NULL, 0 },
#endif
 { ARSGBUT, SAVEGAME, NULL,USE_POINTER, 230, 44,	23, 19, NULL, IX_TICK1 },
 { AAGBUT, CLOSEWIN, NULL, USE_POINTER, 230, 44+47,	23, 19, NULL, IX_CROSS1 }

};


/*-------------------------------------------------------------*\
| This is the restart confirmation 'menu'.			|
\*-------------------------------------------------------------*/

CONFBOX restartBox[] = {

#ifdef JAPAN
 { AAGBUT, INITGAME, NULL, USE_POINTER, 96, 44,	23, 19, NULL, IX_TICK1 },
 { AAGBUT, CLOSEWIN, NULL, USE_POINTER, 56, 44,	23, 19, NULL, IX_CROSS1 }
#else
 { AAGBUT, INITGAME, NULL, USE_POINTER, 70, 28,	23, 19, NULL, IX_TICK1 },
 { AAGBUT, CLOSEWIN, NULL, USE_POINTER, 30, 28,	23, 19, NULL, IX_CROSS1 }
#endif

};


/*-------------------------------------------------------------*\
| This is the sound control 'menu'.				|
\*-------------------------------------------------------------*/

#ifdef MAC_OPTIONS
	CONFBOX soundBox[] = {
		{ SLIDER, MASTERVOL, NULL, SIX_MVOL_SLIDER,	142, 20, 100, 2, &volMaster, 0 },	
 		{ SLIDER, MIDIVOL, NULL, SIX_MVOL_SLIDER,	142, 20+40,	100, 2, &volMidi, 0 },
 		{ SLIDER, SAMPVOL, NULL, SIX_SVOL_SLIDER,	142, 20+2*40,	100, 2, &volSound, 0 },
 		{ SLIDER, SAMPVOL, NULL, SIX_VVOL_SLIDER,	142, 20+3*40,	100, 2, &volVoice, 0 }
	};
#else
CONFBOX soundBox[] = {
	{ SLIDER, MIDIVOL, NULL, SIX_MVOL_SLIDER,	142, 25,	MAXMIDIVOL, 2, &volMidi, 0 },
	{ SLIDER, NOFUNC, NULL, SIX_SVOL_SLIDER,	142, 25+40,	MAXSAMPVOL, 2, &volSound, 0 },
	{ SLIDER, NOFUNC, NULL, SIX_VVOL_SLIDER,	142, 25+2*40,	MAXSAMPVOL, 2, &volVoice, 0 }
};
#endif


/*-------------------------------------------------------------*\
| This is the (mouse) control 'menu'.				|
\*-------------------------------------------------------------*/

int bFlipped;	// looks like this is just so the code has something to alter!


#ifdef MAC_OPTIONS
CONFBOX controlBox[] = {

 { SLIDER, NOFUNC, NULL, SIX_DCLICK_SLIDER,	142, 25,	3*DOUBLE_CLICK_TIME, 1, &dclickSpeed, 0 },
 { FLIP, NOFUNC, NULL, SIX_DCLICK_TEST,		142, 25+30,	23, 19, &bFlipped, IX_CIRCLE1 }

};
#else
CONFBOX controlBox[] = {

 { SLIDER, NOFUNC, NULL, SIX_DCLICK_SLIDER,	142, 25,	3*DOUBLE_CLICK_TIME, 1, &dclickSpeed, 0 },
 { FLIP, NOFUNC, NULL, SIX_DCLICK_TEST,		142, 25+30,	23, 19, &bFlipped, IX_CIRCLE1 },
#ifdef JAPAN
 { TOGGLE, NOFUNC, NULL, SIX_SWAP_TOGGLE,	205, 25+70,	23, 19, &bSwapButtons, 0 }
#else
 { TOGGLE, NOFUNC, NULL, SIX_SWAP_TOGGLE,	155, 25+70,	23, 19, &bSwapButtons, 0 }
#endif

};
#endif


/*-------------------------------------------------------------*\
| This is the subtitles 'menu'.					|
\*-------------------------------------------------------------*/

#ifndef JAPAN
CONFBOX subtitlesBox[] = {

#ifdef USE_5FLAGS
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	15, 100,	56, 32, NULL, FIX_UK },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	85, 100,	56, 32, NULL, FIX_FR },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	155, 100,	56, 32, NULL, FIX_GR },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	50, 137,	56, 32, NULL, FIX_IT },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	120, 137,	56, 32, NULL, FIX_SP },
#endif
#ifdef USE_4FLAGS
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	20, 100,	56, 32, NULL, FIX_FR },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	108, 100,	56, 32, NULL, FIX_GR },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	64, 137,	56, 32, NULL, FIX_IT },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	152, 137,	56, 32, NULL, FIX_SP },
#endif
#ifdef USE_3FLAGS
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	15, 118,	56, 32, NULL, FIX_FR },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	85, 118,	56, 32, NULL, FIX_GR },
 { FRGROUP, NOFUNC, NULL, USE_POINTER,	155, 118,	56, 32, NULL, FIX_SP },
#endif

 { SLIDER, NOFUNC, NULL, SIX_TSPEED_SLIDER,	142, 20,	100, 2, &speedText, 0 },
 { TOGGLE, NOFUNC, NULL, SIX_STITLE_TOGGLE,	142, 20+40,	23, 19, &bSubtitles, 0 },

#if defined(USE_3FLAGS) || defined(USE_4FLAGS) || defined(USE_5FLAGS)
 { ARSGBUT, CLANG, NULL, USE_POINTER,	230, 110,	23, 19, NULL, IX_TICK1 },
 { AAGBUT, RLANG, NULL, USE_POINTER,	230, 140,	23, 19, NULL, IX_CROSS1 }
#endif

};
#endif


/*-------------------------------------------------------------*\
| This is the quit confirmation 'menu'.				|
\*-------------------------------------------------------------*/

CONFBOX quitBox[] = {
#ifdef JAPAN
 { AAGBUT, IQUITGAME, NULL, USE_POINTER,70, 44,	23, 19, NULL, IX_TICK1 },
 { AAGBUT, CLOSEWIN, NULL, USE_POINTER,	30, 44,	23, 19, NULL, IX_CROSS1 }
#else
 { AAGBUT, IQUITGAME, NULL, USE_POINTER,70, 28,	23, 19, NULL, IX_TICK1 },
 { AAGBUT, CLOSEWIN, NULL, USE_POINTER,	30, 28,	23, 19, NULL, IX_CROSS1 }
#endif
};


CONFBOX topwinBox[] = {
 { NOTHING, NOFUNC, NULL, USE_POINTER, 0, 0, 0, 0, NULL, 0 }
};



struct CONFINIT {
	int	h;
	int	v;
	int	x;
	int	y;
	bool bExtraWin;
	CONFBOX *Box;
	int	NumBoxes;
	int	ixHeading;
};

CONFINIT ciOption	= { 6, 5, 72, 23, false, optionBox,	ARRAYSIZE(optionBox),	NO_HEADING };

CONFINIT ciLoad		= { 10, 6, 20, 16, true, loadBox,	ARRAYSIZE(loadBox),	SIX_LOAD_HEADING };
CONFINIT ciSave		= { 10, 6, 20, 16, true, saveBox,	ARRAYSIZE(saveBox),	SIX_SAVE_HEADING };
#ifdef JAPAN
CONFINIT ciRestart	= { 6, 2, 72, 53, false, restartBox,	ARRAYSIZE(restartBox),	SIX_RESTART_HEADING };
#else
CONFINIT ciRestart	= { 4, 2, 98, 53, false, restartBox,	ARRAYSIZE(restartBox),	SIX_RESTART_HEADING };
#endif
CONFINIT ciSound	= { 10, 5, 20, 16, false, soundBox,	ARRAYSIZE(soundBox),	NO_HEADING };
#ifdef MAC_OPTIONS
	CONFINIT ciControl	= { 10, 3, 20, 40, false, controlBox,	ARRAYSIZE(controlBox),	NO_HEADING };
#else
	CONFINIT ciControl	= { 10, 5, 20, 16, false, controlBox,	ARRAYSIZE(controlBox),	NO_HEADING };
#endif
#ifndef JAPAN
#if defined(USE_3FLAGS) || defined(USE_4FLAGS) || defined(USE_5FLAGS)
CONFINIT ciSubtitles	= { 10, 6, 20, 16, false, subtitlesBox,	ARRAYSIZE(subtitlesBox),	NO_HEADING };
#else
CONFINIT ciSubtitles	= { 10, 3, 20, 16, false, subtitlesBox,	ARRAYSIZE(subtitlesBox),	NO_HEADING };
#endif
#endif
CONFINIT ciQuit		= { 4, 2, 98, 53, false, quitBox,	ARRAYSIZE(quitBox),	SIX_QUIT_HEADING };

CONFINIT ciTopWin	= { 6, 5, 72, 23, false, topwinBox,	0,					NO_HEADING };

#define NOBOX (-1)

// Conf window globals
static struct {
	CONFBOX *Box;
	int	NumBoxes;
	bool bExtraWin;
	int	ixHeading;
	bool editableRgroup;

	int	selBox;
	int	pointBox;	// Box pointed to on last call
	int	saveModifier;
	int	fileBase;
	int	numSaved;
} cd = {
	NULL, 0, false, 0, false,
	NOBOX, NOBOX, 0, 0, 0
};

// For editing save game names
char sedit[SG_DESC_LEN+2];

#define HL1	0	// Hilight that moves with the cursor
#define HL2	1	// Hilight on selected RGROUP box
#define HL3	2	// Text on selected RGROUP box
#define NUMHL	3


// Data for button press/toggle effects
static struct {
	bool bButAnim;
	CONFBOX *box;
	bool press;		// true = button press; false = button toggle
} g_buttonEffect = { false, 0, false };


//----- LOCAL FORWARD REFERENCES -----

enum {
	IB_NONE			= -1,	//
	IB_UP			= -2,	// negative numbers returned
	IB_DOWN			= -3,	// by WhichInvBox()
	IB_SLIDE		= -4,	//
	IB_SLIDE_UP		= -5,	//
	IB_SLIDE_DOWN	= -6	//
};

enum {
	HI_BIT		= ((uint)MIN_INT >> 1),	// The next to top bit
	IS_LEFT		= HI_BIT,
	IS_SLIDER	= (IS_LEFT >> 1),
	IS_RIGHT	= (IS_SLIDER >> 1),
	IS_MASK		= (IS_LEFT | IS_SLIDER | IS_RIGHT)
};

static int WhichInvBox(int curX, int curY, bool bSlides);
static void SlideMSlider(int x, SSFN fn);
static OBJECT *AddObject(const FREEL *pfreel, int num);
static void AddBoxes(bool posnSlide);

static void ConfActionSpecial(int i);


/*-------------------------------------------------------------------------*/
/***	Magic numbers	***/

#define M_SW	5	// Side width
#define M_TH	5	// Top height
#ifdef JAPAN
#define M_TOFF	6	// Title text Y offset from top
#define M_TBB	20	// Title box bottom Y offset
#else
#define M_TOFF	4	// Title text Y offset from top
#define M_TBB	14	// Title box bottom Y offset
#endif
#define M_SBL	26	// Scroll bar left X offset
#define M_SH	5	// Slider height (*)
#define M_SW	5	// Slider width (*)
#define M_SXOFF	9	// Slider X offset from right-hand side
#ifdef JAPAN
#define M_IUT	22	// Y offset of top of up arrow
#define M_IUB	30	// Y offset of bottom of up arrow
#else
#define M_IUT	16	// Y offset of top of up arrow
#define M_IUB	24	// Y offset of bottom of up arrow
#endif
#define M_IDT	10	// Y offset (from bottom) of top of down arrow
#define M_IDB	3	// Y offset (from bottom) of bottom of down arrow
#define M_IAL	12	// X offset (from right) of left of scroll arrows
#define M_IAR	3	// X offset (from right) of right of scroll arrows

#define START_ICONX	(M_SW+1)	// } Relative offset of first icon
#define START_ICONY	(M_TBB+M_TH+1)	// } within the inventory window

/*-------------------------------------------------------------------------*/



#ifndef JAPAN
bool LanguageChange(void) {
	LANGUAGE nLang;

#ifdef USE_3FLAGS
	// VERY quick dodgy bodge
	if (cd.selBox == 0)
		nLang = TXT_FRENCH;
	else if (cd.selBox == 1)
		nLang = TXT_GERMAN;
	else
		nLang = TXT_SPANISH;
	if (nLang != language) {
#elif defined(USE_4FLAGS)
	nLang = (LANGUAGE)(cd.selBox + 1);
	if (nLang != language) {
#else
	if (cd.selBox != language) {
		nLang = (LANGUAGE)cd.selBox;
#endif
		KillInventory();
		ChangeLanguage(nLang);
		language = nLang;
		return true;
	}
	else
		return false;
}
#endif

/**************************************************************************/
/******************** Some miscellaneous functions ************************/
/**************************************************************************/

/*---------------------------------------------------------------------*\
|	DumpIconArray()/DumpDobjArray()/DumpObjArray()			|
|-----------------------------------------------------------------------|
| Delete all the objects in iconArray[]/DobjArray[]/objArray[]		|
\*---------------------------------------------------------------------*/
static void DumpIconArray(void){
	for (int i = 0; i < MAX_ICONS; i++) {
		if (iconArray[i] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[i]);
			iconArray[i] = NULL;
		}
	}
}

/**
 * Delete all the objects in DobjArray[]
 */

static void DumpDobjArray(void) {
	for (int i = 0; i < MAX_WCOMP; i++) {
		if (DobjArray[i] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), DobjArray[i]);
			DobjArray[i] = NULL;
		}
	}
}

/**
 * Delete all the objects in objArray[]
 */

static void DumpObjArray(void) {
	for (int i = 0; i < MAX_WCOMP; i++) {
		if (objArray[i] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), objArray[i]);
			objArray[i] = NULL;
		}
	}
}

/**
 * Convert item ID number to pointer to item's compiled data
 * i.e. Image data and Glitter code.
 */
INV_OBJECT *findInvObject(int num) {
	INV_OBJECT *retval = pio;

	for (int i = 0; i < numObjects; i++, retval++) {
		if (retval->id == num)
			return retval;
	}

	error("Trying to manipulate undefined inventory icon");
}

/**
 * Returns position of an item in one of the inventories.
 * The actual position is not important for the uses that this is put to.
 */

int InventoryPos(int num) {
	int	i;

	for (i = 0; i < InvD[INV_1].NoofItems; i++)	// First inventory
		if (InvD[INV_1].ItemOrder[i] == num)
			return i;

	for (i = 0; i < InvD[INV_2].NoofItems; i++)	// Second inventory
		if (InvD[INV_2].ItemOrder[i] == num)
			return i;

	if (HeldItem == num)
		return INV_HELDNOTIN;	// Held, but not in either inventory

	return INV_NOICON;		// Not held, not in either inventory
}

bool IsInInventory(int object, int invnum) {
	assert(invnum == INV_1 || invnum == INV_2);

	for (int i = 0; i < InvD[invnum].NoofItems; i++)	// First inventory
		if (InvD[invnum].ItemOrder[i] == object)
			return true;

	return false;
}

/**
 * Returns which item is held (INV_NOICON (-1) if none)
 */

int WhichItemHeld(void) {
	return HeldItem;
}

/**
 * Called from the cursor module when it re-initialises (at the start of
 * a new scene). For if we are holding something at scene-change time.
 */

void InventoryIconCursor(void) {
	INV_OBJECT *invObj;

	if (HeldItem != INV_NOICON) {
		invObj = findInvObject(HeldItem);
		SetAuxCursor(invObj->hFilm);
	}
}

/**
 * Returns TRUE if the inventory is active.
 */

bool InventoryActive(void) {
	return (InventoryState == ACTIVE_INV);
}

int WhichInventoryOpen(void) {
	if (InventoryState != ACTIVE_INV)
		return 0;
	else
		return ino;
}


/**************************************************************************/
/************** Running inventory item's Glitter code *********************/
/**************************************************************************/

struct ITP_INIT {
	INV_OBJECT *pinvo;
	USER_EVENT	event;
	BUTEVENT	bev;
};

/**
 * Run inventory item's Glitter code
 */
static void InvTinselProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
		int	ThisPointedWait;			//	Fix the 'repeated pressing bug'
	CORO_END_CONTEXT(_ctx);

	// get the stuff copied to process when it was created
	ITP_INIT *to = (ITP_INIT *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(AllowDclick, to->bev);

	_ctx->pic = InitInterpretContext(GS_INVENTORY, to->pinvo->hScript, to->event, NOPOLY, 0, to->pinvo);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	if (to->event == POINTED) {
		_ctx->ThisPointedWait = ++PointedWaitCount;
		while (1) {
			CORO_SLEEP(1);
			int	x, y;
			GetCursorXY(&x, &y, false);
			if (InvItemId(x, y) != to->pinvo->id)
				break;

			// Fix the 'repeated pressing bug'
			if (_ctx->ThisPointedWait != PointedWaitCount)
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
void RunInvTinselCode(INV_OBJECT *pinvo, USER_EVENT event, BUTEVENT be, int index) {
	ITP_INIT to = { pinvo, event, be };
	
	if (InventoryHidden)
		return;

	GlitterIndex = index;
	g_scheduler->createProcess(PID_TCODE, InvTinselProcess, &to, sizeof(to));
}

/**************************************************************************/
/****************** Load/Save game specific functions *********************/
/**************************************************************************/

/**
 * Set first load/save file entry displayed.
 * Point Box[] text pointers to appropriate file descriptions.
 */

void firstFile(int first) {
	int	i, j;

	i = getList();

	cd.numSaved = i;

	if (first < 0)
		first = 0;
	else if (first > MAX_SFILES-NUM_SL_RGROUP)
		first = MAX_SFILES-NUM_SL_RGROUP;

	if (first == 0 && i < MAX_SFILES && cd.Box == saveBox) {
		// Blank first entry for new save
		cd.Box[0].boxText = NULL;
		cd.saveModifier = j = 1;
	} else {
		cd.saveModifier = j = 0;
	}

	for (i = first; j < NUM_SL_RGROUP; j++, i++) {
		cd.Box[j].boxText = ListEntry(i, LE_DESC);
	}

	cd.fileBase = first;
}

/**
 * Save the game using filename from selected slot & current description.
 */

void InvSaveGame(void) {
	if (cd.selBox != NOBOX) {
#ifndef JAPAN
		sedit[strlen(sedit)-1] = 0;	// Don't include the cursor!
#endif
		SaveGame(ListEntry(cd.selBox-cd.saveModifier+cd.fileBase, LE_NAME), sedit);
	}
}

/**
 * Load the selected saved game.
 */
void InvLoadGame(void) {
	int	rGame;

	if (cd.selBox != NOBOX && (cd.selBox+cd.fileBase < cd.numSaved)) {
		rGame = cd.selBox;
		cd.selBox = NOBOX;
		if (iconArray[HL3] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL3]);
			iconArray[HL3] = NULL;
		}
		if (iconArray[HL2] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL2]);
			iconArray[HL2] = NULL;
		}
		if (iconArray[HL1] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
			iconArray[HL1] = NULL;
		}	
		RestoreGame(rGame+cd.fileBase);
	}
}

/**
 * Edit the string in sedit[]
 * Returns TRUE if the string was altered.
 */
#ifndef JAPAN
bool UpdateString(const Common::KeyState &kbd) {
	int	cpos;

	if (!cd.editableRgroup)
		return false;

	cpos = strlen(sedit)-1;

	if (kbd.keycode == Common::KEYCODE_BACKSPACE) {
		if (!cpos)
			return false;
		sedit[cpos] = 0;
		cpos--;
		sedit[cpos] = CURSOR_CHAR;
		return true;
//	} else if (isalnum(c) || c == ',' || c == '.' || c == '\'' || (c == ' ' && cpos != 0)) {
	} else if (IsCharImage(hTagFontHandle(), kbd.ascii) || (kbd.ascii == ' ' && cpos != 0)) {
		if (cpos == SG_DESC_LEN)
			return false;
		sedit[cpos] = kbd.ascii;
		cpos++;
		sedit[cpos] = CURSOR_CHAR;
		sedit[cpos+1] = 0;
		return true;
	}
	return false;
}
#endif

/**
 * Keystrokes get sent here when load/save screen is up.
 */
bool InvKeyIn(const Common::KeyState &kbd) {
	if (kbd.keycode == Common::KEYCODE_PAGEUP ||
	    kbd.keycode == Common::KEYCODE_PAGEDOWN ||
	    kbd.keycode == Common::KEYCODE_HOME ||
	    kbd.keycode == Common::KEYCODE_END)
		return true;	// Key needs processing

	if (kbd.keycode == 0 && kbd.ascii == 0) {
		;
	} else if (kbd.keycode == Common::KEYCODE_RETURN) {
		return true;	// Key needs processing
	} else if (kbd.keycode == Common::KEYCODE_ESCAPE) {
		return true;	// Key needs processing
	} else {
#ifndef JAPAN
		if (UpdateString(kbd)) {
			/*
			* Delete display of text currently being edited,
			* and replace it with freshly edited text.
			*/
			if (iconArray[HL3] != NULL) {
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL3]);
				iconArray[HL3] = NULL;
			}
			iconArray[HL3] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), sedit, 0,
				InvD[ino].inventoryX + cd.Box[cd.selBox].xpos + 2,
				InvD[ino].inventoryY + cd.Box[cd.selBox].ypos,
				hTagFontHandle(), 0);
			if (MultiRightmost(iconArray[HL3]) > 213) {
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL3]);
				UpdateString(Common::KeyState(Common::KEYCODE_BACKSPACE));
				iconArray[HL3] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), sedit, 0,
					InvD[ino].inventoryX + cd.Box[cd.selBox].xpos + 2,
					InvD[ino].inventoryY + cd.Box[cd.selBox].ypos,
					hTagFontHandle(), 0);
			}
			MultiSetZPosition(iconArray[HL3], Z_INV_ITEXT + 2);
		}
#endif
	}
	return false;
}

/*---------------------------------------------------------------------*\
|	Select()							|
|-----------------------------------------------------------------------|
| Highlights selected box.						|
| If it's editable (save game), copy existing description and add a	|
| cursor.								|
\*---------------------------------------------------------------------*/
void Select(int i, bool force) {
#ifdef JAPAN
	time_t		secs_now;
	struct tm	*time_now;
#endif

	i &= ~IS_MASK;

	if (cd.selBox == i && !force)
		return;

	cd.selBox = i;

	// Clear previous selected highlight and text
	if (iconArray[HL2] != NULL) {
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL2]);
		iconArray[HL2] = NULL;
	}
	if (iconArray[HL3] != NULL) {
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL3]);
		iconArray[HL3] = NULL;
	}

	// New highlight box
	switch (cd.Box[i].boxType) {
	case RGROUP:
		iconArray[HL2] = RectangleObject(BackPal(), COL_HILIGHT, cd.Box[i].w, cd.Box[i].h);
		MultiInsertObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL2]);
		MultiSetAniXY(iconArray[HL2],
		InvD[ino].inventoryX + cd.Box[i].xpos,
		InvD[ino].inventoryY + cd.Box[i].ypos);

		// Z-position of box, and add edit text if appropriate
		if (cd.editableRgroup) {
			MultiSetZPosition(iconArray[HL2], Z_INV_ITEXT+1);

			assert(cd.Box[i].ixText == USE_POINTER);
#ifdef JAPAN
			// Current date and time
			time(&secs_now);
			time_now = localtime(&secs_now);
			strftime(sedit, SG_DESC_LEN, "%D %H:%M", time_now);
#else
			// Current description with cursor appended
			if (cd.Box[i].boxText != NULL) {
				strcpy(sedit, cd.Box[i].boxText);
				strcat(sedit, sCursor);
			} else {
				strcpy(sedit, sCursor);
			}
#endif
			iconArray[HL3] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), sedit, 0,
				InvD[ino].inventoryX + cd.Box[i].xpos + 2,
#ifdef JAPAN
				InvD[ino].inventoryY + cd.Box[i].ypos + 2,
#else
				InvD[ino].inventoryY + cd.Box[i].ypos,
#endif
				hTagFontHandle(), 0);
			MultiSetZPosition(iconArray[HL3], Z_INV_ITEXT + 2);
		} else {
			MultiSetZPosition(iconArray[HL2], Z_INV_ICONS + 1);
		}

		_vm->divertKeyInput(InvKeyIn);

		break;

#if defined(USE_3FLAGS) || defined(USE_4FLAGS) || defined(USE_5FLAGS)
	case FRGROUP:
		iconArray[HL2] = RectangleObject(BackPal(), COL_HILIGHT, cd.Box[i].w+6, cd.Box[i].h+6);
		MultiInsertObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL2]);
		MultiSetAniXY(iconArray[HL2],
		InvD[ino].inventoryX + cd.Box[i].xpos - 2,
		InvD[ino].inventoryY + cd.Box[i].ypos - 2);
		MultiSetZPosition(iconArray[HL2], Z_INV_BRECT+1);

		break;
#endif
	default:
		break;
	}
}


/**************************************************************************/
/***/
/**************************************************************************/

/**
 * If the item is not already held, hold it.
 */

void HoldItem(int item) {
	INV_OBJECT *invObj;

	if (HeldItem != item) {
		if (item == INV_NOICON && HeldItem != INV_NOICON)
			DelAuxCursor();			// no longer aux cursor

		if (item != INV_NOICON) {
			invObj = findInvObject(item);
			SetAuxCursor(invObj->hFilm);	// and is aux. cursor
		}

		HeldItem = item;			// Item held
	}

	// Redraw contents - held item not displayed as a content.
	ItemsChanged = true;
}

/**
 * Stop holding an item.
 */

void DropItem(int item) {
	if (HeldItem == item) {
		HeldItem = INV_NOICON;		// Item not held
		DelAuxCursor();			// no longer aux cursor
	}

	// Redraw contents - held item was not displayed as a content.
	ItemsChanged = true;
}

/**
 * Stick the item into an inventory list (ItemOrder[]), and hold the
 * item if requested.
 */

void AddToInventory(int invno, int icon, bool hold) {
	int	i;
	bool	bOpen;
#ifdef DEBUG
	INV_OBJECT *invObj;
#endif

	assert((invno == INV_1 || invno == INV_2 || invno == INV_CONV || invno == INV_OPEN)); // Trying to add to illegal inventory

	if (invno == INV_OPEN) {
		assert(InventoryState == ACTIVE_INV && (ino == INV_1 || ino == INV_2)); // addopeninv() with inventry not open
		invno = ino;
		bOpen = true;

		// Make sure it doesn't get in both!
		RemFromInventory(ino == INV_1 ? INV_2 : INV_1, icon);
	} else
		bOpen = false;

#ifdef DEBUG
	invObj = findInvObject(icon);
	if ((invObj->attribute & IO_ONLYINV1 && invno != INV_1)
	|| (invObj->attribute & IO_ONLYINV2 && invno != INV_2))
		error("Trying to add resticted object to wrong inventory");
#endif

	if (invno == INV_1)
		RemFromInventory(INV_2, icon);
	else if (invno == INV_2)
		RemFromInventory(INV_1, icon);

	// See if it's already there
	for (i = 0; i < InvD[invno].NoofItems; i++) {
		if (InvD[invno].ItemOrder[i] == icon)
			break;
	}

	// Add it if it isn't already there
	if (i == InvD[invno].NoofItems) {
		if (!bOpen) {
			if (invno == INV_CONV) {
				// For conversation, insert before last icon
				// which will always be the goodbye icon
				InvD[invno].ItemOrder[InvD[invno].NoofItems] = InvD[invno].ItemOrder[InvD[invno].NoofItems-1];
				InvD[invno].ItemOrder[InvD[invno].NoofItems-1] = icon;
				InvD[invno].NoofItems++;
			} else {
				InvD[invno].ItemOrder[InvD[invno].NoofItems++] = icon;
			}
			ItemsChanged = true;
		} else {
			// It could be that the index is beyond what you'd expect
			// as delinv may well have been called
			if (GlitterIndex < InvD[invno].NoofItems) {
				memmove(&InvD[invno].ItemOrder[GlitterIndex + 1],
					&InvD[invno].ItemOrder[GlitterIndex],
					(InvD[invno].NoofItems-GlitterIndex)*sizeof(int));
				InvD[invno].ItemOrder[GlitterIndex] = icon;
			} else {
				InvD[invno].ItemOrder[InvD[invno].NoofItems] = icon;
			}
			InvD[invno].NoofItems++;
		}
	}

	// Hold it if requested
	if (hold)
		HoldItem(icon);
}

/**
 * Take the item from the inventory list (ItemOrder[]).
 * Return FALSE if item wasn't present, true if it was.
 */

bool RemFromInventory(int invno, int icon) {
	int i;

	assert(invno == INV_1 || invno == INV_2 || invno == INV_CONV); // Trying to delete from illegal inventory

	// See if it's there
	for (i = 0; i < InvD[invno].NoofItems; i++) {
		if (InvD[invno].ItemOrder[i] == icon)
			break;
	}

	if (i == InvD[invno].NoofItems)
		return false;			// Item wasn't there
	else {
		memmove(&InvD[invno].ItemOrder[i], &InvD[invno].ItemOrder[i+1], (InvD[invno].NoofItems-i)*sizeof(int));
		InvD[invno].NoofItems--;
		ItemsChanged = true;
		return true;			// Item removed
	}
}


/**************************************************************************/
/***/
/**************************************************************************/

/*---------------------------------------------------------------------*\
|	InvArea()							|
|-----------------------------------------------------------------------|
| Work out which area of the inventory window the cursor is in.		|
|-----------------------------------------------------------------------|
| This used to be worked out with appropriately defined magic numbers.	|
| Then the graphic changed and I got it right again. Then the graphic	|
| changed and I got fed up of faffing about. It's probably easier just	|
| to rework all this.							|
\*---------------------------------------------------------------------*/
enum {	I_NOTIN, I_MOVE, I_BODY,
	I_TLEFT, I_TRIGHT, I_BLEFT, I_BRIGHT,
	I_TOP, I_BOTTOM, I_LEFT, I_RIGHT,
	I_UP, I_SLIDE_UP, I_SLIDE, I_SLIDE_DOWN, I_DOWN,
	I_ENDCHANGE
};

#define EXTRA	1	// This was introduced when we decided to increase
			// the active area of the borders for re-sizing.

/*---------------------------------*/
#define LeftX	InvD[ino].inventoryX
#define TopY	InvD[ino].inventoryY
/*---------------------------------*/

int InvArea(int x, int y) {
	int RightX = MultiRightmost(RectObject) + 1;
	int BottomY = MultiLowest(RectObject) + 1;

// Outside the whole rectangle?
	if (x <= LeftX - EXTRA || x > RightX + EXTRA
	|| y <= TopY - EXTRA || y > BottomY + EXTRA)
		return I_NOTIN;

// The bottom line
	if (y > BottomY - 2 - EXTRA) {		// Below top of bottom line?
		if (x <= LeftX + 2 + EXTRA)
			return I_BLEFT;		// Bottom left corner
		else if (x > RightX - 2 - EXTRA)
			return I_BRIGHT;	// Bottom right corner
		else
			return I_BOTTOM;	// Just plain bottom
	}

// The top line
	if (y <= TopY + 2 + EXTRA) {		// Above bottom of top line?
		if (x <= LeftX + 2 + EXTRA)
			return I_TLEFT;		// Top left corner
		else if (x > RightX - 2 - EXTRA)
			return I_TRIGHT;	// Top right corner
		else
			return I_TOP;		// Just plain top
	}

// Sides
	if (x <= LeftX + 2 + EXTRA)		// Left of right of left side?
		return I_LEFT;
	else if (x > RightX - 2 - EXTRA)		// Right of left of right side?
		return I_RIGHT;

// From here down still needs fixing up properly
/*
* In the move area?
*/
	if (ino != INV_CONF
	&& x >= LeftX + M_SW - 2 && x <= RightX - M_SW + 3 &&
	   y >= TopY + M_TH - 2  && y < TopY + M_TBB + 2)
		return I_MOVE;

/*
* Scroll bits
*/
	if (ino == INV_CONF && cd.bExtraWin) {
	} else {
		if (x > RightX - M_IAL + 3 && x <= RightX - M_IAR + 1) {
			if (y > TopY + M_IUT + 1 && y < TopY + M_IUB - 1)
				return I_UP;
			if (y > BottomY - M_IDT + 4 && y <= BottomY - M_IDB + 1)
				return I_DOWN;

			if (y >= TopY + slideYmin && y < TopY + slideYmax + M_SH) {
				if (y < TopY + slideY)
					return I_SLIDE_UP;
				if (y < TopY + slideY + M_SH)
					return I_SLIDE;
				else
					return I_SLIDE_DOWN;
			}
		}
	}

	return I_BODY;
}

/**
 * Returns the id of the icon displayed under the given position.
 * Also return co-ordinates of items tag display position, if requested.
 */

int InvItem(int *x, int *y, bool update) {
	int itop, ileft;
	int row, col;
	int item;
	int IconsX;

	itop = InvD[ino].inventoryY + START_ICONY;

	IconsX = InvD[ino].inventoryX + START_ICONX;

	for (item = InvD[ino].FirstDisp, row = 0; row < InvD[ino].NoofVicons; row++) {
		ileft = IconsX;

		for (col = 0; col < InvD[ino].NoofHicons; col++, item++) {
			if (*x >= ileft && *x < ileft + ITEM_WIDTH &&
			   *y >= itop  && *y < itop + ITEM_HEIGHT) {
				if (update) {
					*x = ileft + ITEM_WIDTH/2;
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

/**
 * Returns the id of the icon displayed under the given position.
 */

int InvItemId(int x, int y) {
	int itop, ileft;
	int row, col;
	int item;

	if (InventoryHidden || InventoryState == IDLE_INV)
		return INV_NOICON;

	itop = InvD[ino].inventoryY + START_ICONY;

	int IconsX = InvD[ino].inventoryX + START_ICONX;

	for (item = InvD[ino].FirstDisp, row = 0; row < InvD[ino].NoofVicons; row++) {
		ileft = IconsX;

		for (col = 0; col < InvD[ino].NoofHicons; col++, item++) {
			if (x >= ileft && x < ileft + ITEM_WIDTH &&
			   y >= itop  && y < itop + ITEM_HEIGHT) {
				return InvD[ino].ItemOrder[item];
			}

			ileft += ITEM_WIDTH + 1;
		}
		itop += ITEM_HEIGHT + 1;
	}
	return INV_NOICON;
}

/*---------------------------------------------------------------------*\
|	WhichInvBox()							|
|-----------------------------------------------------------------------|
| Finds which box the cursor is in.					|
\*---------------------------------------------------------------------*/
#define MD_YSLIDTOP	7
#define MD_YSLIDBOT	18
#define MD_YBUTTOP	9
#define MD_YBUTBOT	16
#define MD_XLBUTL	1
#define MD_XLBUTR	10
#define MD_XRBUTL	105
#define MD_XRBUTR	114

static int WhichInvBox(int curX, int curY, bool bSlides) {
	if (bSlides) {
		for (int i = 0; i < numMdSlides; i++) {
			if (curY > MultiHighest(mdSlides[i].obj) && curY < MultiLowest(mdSlides[i].obj)
			&& curX > MultiLeftmost(mdSlides[i].obj) && curX < MultiRightmost(mdSlides[i].obj))
				return mdSlides[i].num | IS_SLIDER;
		}
	}

	curX -= InvD[ino].inventoryX;
	curY -= InvD[ino].inventoryY;

	for (int i = 0; i < cd.NumBoxes; i++) {
		switch (cd.Box[i].boxType) {
		case SLIDER:
			if (bSlides) {
				if (curY >= cd.Box[i].ypos+MD_YBUTTOP && curY < cd.Box[i].ypos+MD_YBUTBOT) {
					if (curX >= cd.Box[i].xpos+MD_XLBUTL && curX < cd.Box[i].xpos+MD_XLBUTR)
						return i | IS_LEFT;
					if (curX >= cd.Box[i].xpos+MD_XRBUTL && curX < cd.Box[i].xpos+MD_XRBUTR)
						return i | IS_RIGHT;
				}
			}
			break;

		case AAGBUT:
		case ARSGBUT:
		case TOGGLE:
		case FLIP:
			if (curY > cd.Box[i].ypos && curY < cd.Box[i].ypos + cd.Box[i].h
			&& curX > cd.Box[i].xpos && curX < cd.Box[i].xpos + cd.Box[i].w)
				return i;
			break;

		default:
			// 'Normal' box
			if (curY >= cd.Box[i].ypos && curY < cd.Box[i].ypos + cd.Box[i].h
			&& curX >= cd.Box[i].xpos && curX < cd.Box[i].xpos + cd.Box[i].w)
				return i;
			break;
		}
	}

	if (cd.bExtraWin) {
		if (curX > 20 + 181 && curX < 20 + 181 + 8 &&
			curY > 24 + 2   && curY < 24 + 139 + 5) {

			if (curY < 24 + 2 + 5) {
				return IB_UP;
			} else if (curY > 24 + 139) {
				return IB_DOWN;
			} else if (curY+InvD[ino].inventoryY >= slideY && curY+InvD[ino].inventoryY < slideY + 5) {
				return IB_SLIDE;
			} else if (curY+InvD[ino].inventoryY < slideY) {
				return IB_SLIDE_UP;
			} else if (curY+InvD[ino].inventoryY >= slideY + 5) {
				return IB_SLIDE_DOWN;
			}
		}
	}

	return IB_NONE;
}

/**************************************************************************/
/***/
/**************************************************************************/

/**
 * InBoxes
 */
void InvBoxes(bool InBody, int curX, int curY) {
	int	index;			// Box pointed to on this call
	const FILM *pfilm;

	// Find out which icon is currently pointed to
	if (!InBody)
		index = -1;
	else {
		index = WhichInvBox(curX, curY, false);
	}

	// If no icon pointed to, or points to (logical position of)
	// currently held icon, then no icon is pointed to!
	if (index < 0) {
		// unhigh-light box (if one was)
		cd.pointBox = NOBOX;
		if (iconArray[HL1] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
			iconArray[HL1] = NULL;
		}	
	} else if (index != cd.pointBox) {
		cd.pointBox = index;
		// A new box is pointed to - high-light it
		if (iconArray[HL1] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
			iconArray[HL1] = NULL;
		}
		if ((cd.Box[cd.pointBox].boxType == ARSBUT && cd.selBox != NOBOX) ||
///* I don't agree */ cd.Box[cd.pointBox].boxType == RGROUP ||
		    cd.Box[cd.pointBox].boxType == AATBUT ||
		    cd.Box[cd.pointBox].boxType == AABUT) {
			iconArray[HL1] = RectangleObject(BackPal(), COL_HILIGHT, cd.Box[cd.pointBox].w, cd.Box[cd.pointBox].h);
			MultiInsertObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
			MultiSetAniXY(iconArray[HL1],
				InvD[ino].inventoryX + cd.Box[cd.pointBox].xpos,
				InvD[ino].inventoryY + cd.Box[cd.pointBox].ypos);
			MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);
		}
		else if (cd.Box[cd.pointBox].boxType == AAGBUT ||
				cd.Box[cd.pointBox].boxType == ARSGBUT ||
				cd.Box[cd.pointBox].boxType == TOGGLE) {
			pfilm = (const FILM *)LockMem(winPartsf);

			iconArray[HL1] = AddObject(&pfilm->reels[cd.Box[cd.pointBox].bi+HIGRAPH], -1);
			MultiSetAniXY(iconArray[HL1],
				InvD[ino].inventoryX + cd.Box[cd.pointBox].xpos,
				InvD[ino].inventoryY + cd.Box[cd.pointBox].ypos);
			MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);
		}
	}
}

static void ButtonPress(CORO_PARAM, CONFBOX *box) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;

	assert(box->boxType == AAGBUT || box->boxType == ARSGBUT);

	// Replace highlight image with normal image
	pfilm = (const FILM *)LockMem(winPartsf);
	if (iconArray[HL1] != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
	pfilm = (const FILM *)LockMem(winPartsf);
	iconArray[HL1] = AddObject(&pfilm->reels[box->bi+NORMGRAPH], -1);
	MultiSetAniXY(iconArray[HL1], InvD[ino].inventoryX + box->xpos, InvD[ino].inventoryY + box->ypos);
	MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (iconArray[HL1] == NULL)
		return;

	// Replace normal image with depresses image
	pfilm = (const FILM *)LockMem(winPartsf);
	MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
	iconArray[HL1] = AddObject(&pfilm->reels[box->bi+DOWNGRAPH], -1);
	MultiSetAniXY(iconArray[HL1], InvD[ino].inventoryX + box->xpos, InvD[ino].inventoryY + box->ypos);
	MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);

	// Hold depressed image for 2 frames
	CORO_SLEEP(2);
	if (iconArray[HL1] == NULL)
		return;

	// Replace depressed image with normal image
	pfilm = (const FILM *)LockMem(winPartsf);
	MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
	iconArray[HL1] = AddObject(&pfilm->reels[box->bi+NORMGRAPH], -1);
	MultiSetAniXY(iconArray[HL1], InvD[ino].inventoryX + box->xpos, InvD[ino].inventoryY + box->ypos);
	MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);

	CORO_SLEEP(1);

	CORO_END_CODE;
}

static void ButtonToggle(CORO_PARAM, CONFBOX *box) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;

	assert(box->boxType == TOGGLE);

	// Remove hilight image
	if (iconArray[HL1] != NULL) {
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
		iconArray[HL1] = NULL;
	}

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (InventoryState != ACTIVE_INV)
		return;

	// Add depressed image
	pfilm = (const FILM *)LockMem(winPartsf);
	iconArray[HL1] = AddObject(&pfilm->reels[box->bi+DOWNGRAPH], -1);
	MultiSetAniXY(iconArray[HL1], InvD[ino].inventoryX + box->xpos, InvD[ino].inventoryY + box->ypos);
	MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);

	// Hold depressed image for 1 frame
	CORO_SLEEP(1);
	if (iconArray[HL1] == NULL)
		return;

	// Toggle state
	(*box->ival) = *(box->ival) ^ 1;	// XOR with true
	box->bi = *(box->ival) ? IX_TICK1 : IX_CROSS1;
	AddBoxes(false);
	// Keep highlight (e.g. flag)
	if (cd.selBox != NOBOX)
		Select(cd.selBox, true);

	// New state, depressed image
	pfilm = (const FILM *)LockMem(winPartsf);
	if (iconArray[HL1] != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
	iconArray[HL1] = AddObject(&pfilm->reels[box->bi+DOWNGRAPH], -1);
	MultiSetAniXY(iconArray[HL1], InvD[ino].inventoryX + box->xpos, InvD[ino].inventoryY + box->ypos);
	MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);

	// Hold new depressed image for 1 frame
	CORO_SLEEP(1);
	if (iconArray[HL1] == NULL)
		return;

	// New state, normal
	MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
	iconArray[HL1] = NULL;

	// Hold normal image for 1 frame
	CORO_SLEEP(1);
	if (InventoryState != ACTIVE_INV)
		return;

	// New state, highlighted
	pfilm = (const FILM *)LockMem(winPartsf);
	if (iconArray[HL1] != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), iconArray[HL1]);
	iconArray[HL1] = AddObject(&pfilm->reels[box->bi+HIGRAPH], -1);
	MultiSetAniXY(iconArray[HL1], InvD[ino].inventoryX + box->xpos, InvD[ino].inventoryY + box->ypos);
	MultiSetZPosition(iconArray[HL1], Z_INV_ICONS+1);

	CORO_END_CODE;
}

/**
 * Monitors for POINTED event for inventory icons.
 */

void InvLabels(bool InBody, int aniX, int aniY) {
	int	index;				// Icon pointed to on this call
	INV_OBJECT *invObj;

	// Find out which icon is currently pointed to
	if (!InBody)
		index = INV_NOICON;
	else {
		index = InvItem(&aniX, &aniY, false);
		if (index != INV_NOICON) {
			if (index >= InvD[ino].NoofItems)
				index = INV_NOICON;
			else
				index = InvD[ino].ItemOrder[index];
		}
	}

	// If no icon pointed to, or points to (logical position of)
	// currently held icon, then no icon is pointed to!
	if (index == INV_NOICON || index == HeldItem) {
		pointedIcon = INV_NOICON;
	} else if (index != pointedIcon) {
		// A new icon is pointed to - run its script with POINTED event
		invObj = findInvObject(index);
		if (invObj->hScript)
			RunInvTinselCode(invObj, POINTED, BE_NONE, index);
		pointedIcon = index;
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

void AdjustTop(void) {
	int tMissing, bMissing, nMissing;
	int nslideY;
	int rowsWanted;
	int slideRange;
	int n, i;

	// Only do this if there's a slider
	if (!SlideObject)
		return;

	rowsWanted = (InvD[ino].NoofItems - InvD[ino].FirstDisp + InvD[ino].NoofHicons-1) / InvD[ino].NoofHicons;

	while (rowsWanted < InvD[ino].NoofVicons) {
		if (InvD[ino].FirstDisp) {
			InvD[ino].FirstDisp -= InvD[ino].NoofHicons;
			if (InvD[ino].FirstDisp < 0)
				InvD[ino].FirstDisp = 0;
			rowsWanted++;
		} else
			break;
	}
	tMissing = InvD[ino].FirstDisp ? (InvD[ino].FirstDisp + InvD[ino].NoofHicons-1)/InvD[ino].NoofHicons : 0;
	bMissing = (rowsWanted > InvD[ino].NoofVicons) ? rowsWanted - InvD[ino].NoofVicons : 0;

	nMissing = tMissing + bMissing;
	slideRange = slideYmax - slideYmin;

	if (!tMissing)
		nslideY = slideYmin;
	else if (!bMissing)
		nslideY = slideYmax;
	else {
		nslideY = tMissing*slideRange/nMissing;
		nslideY += slideYmin;
	}

	if (nMissing) {
		n = InvD[ino].FirstDisp - tMissing*InvD[ino].NoofHicons;
		for (i = 0; i <= nMissing; i++, n += InvD[ino].NoofHicons) {
			slideStuff[i].n = n;
			slideStuff[i].y = (i*slideRange/nMissing) + slideYmin;
		}
		if (slideStuff[0].n < 0)
			slideStuff[0].n = 0;
		assert(i < MAX_ININV + 1);
		slideStuff[i].n = -1;
	} else {
		slideStuff[0].n = 0;
		slideStuff[0].y = slideYmin;
		slideStuff[1].n = -1;
	}

	if (nslideY != slideY) {
		MultiMoveRelXY(SlideObject, 0, nslideY - slideY);
		slideY = nslideY;
	}
}

/**
 * Insert an inventory icon object onto the display list.
 */

OBJECT *AddInvObject(int num, const FREEL **pfreel, const FILM **pfilm) {
	INV_OBJECT *invObj;		// Icon data
	const MULTI_INIT *pmi;		// Its INIT structure - from the reel
	IMAGE *pim;		// ... you get the picture
	OBJECT *pPlayObj;	// The object we insert

	invObj = findInvObject(num);

	// Get pointer to image
	pim = GetImageFromFilm(invObj->hFilm, 0, pfreel, &pmi, pfilm);

	// Poke in the background palette
	pim->hImgPal = TO_LE_32(BackPal());

	// Set up the multi-object
	pPlayObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), pPlayObj);

	return pPlayObj;
}

/**
 * Create display objects for the displayed icons in an inventory window.
 */

void FillInInventory(void) {
	int	Index;		// Index into ItemOrder[]
	int	n = 0;		// index into iconArray[]
	int	xpos, ypos;
	int	row, col;
	const FREEL *pfr;
	const FILM *pfilm;

	DumpIconArray();

	if (InvDragging != ID_SLIDE)
		AdjustTop();		// Set up slideStuff[]

	Index = InvD[ino].FirstDisp;	// Start from first displayed object
	n = 0;
	ypos = START_ICONY;		// Y-offset of first display row

	for (row = 0; row < InvD[ino].NoofVicons; row++,	ypos += ITEM_HEIGHT + 1) {
		xpos = START_ICONX;		// X-offset of first display column

		for (col = 0; col < InvD[ino].NoofHicons; col++) {
			if (Index >= InvD[ino].NoofItems)
				break;
			else if (InvD[ino].ItemOrder[Index] != HeldItem) {
				// Create a display object and position it
				iconArray[n] = AddInvObject(InvD[ino].ItemOrder[Index], &pfr, &pfilm);
				MultiSetAniXY(iconArray[n], InvD[ino].inventoryX + xpos , InvD[ino].inventoryY + ypos);
				MultiSetZPosition(iconArray[n], Z_INV_ICONS);

				InitStepAnimScript(&iconAnims[n], iconArray[n], FROM_LE_32(pfr->script), ONE_SECOND / FROM_LE_32(pfilm->frate));

				n++;
			}
			Index++;
			xpos += ITEM_WIDTH + 1;	// X-offset of next display column
		}
	}
}

/**
 * Set up a rectangle as the background to the inventory window.
 *  Additionally, sticks the window title up.
 */

enum {FROM_HANDLE, FROM_STRING};

void AddBackground(OBJECT **rect, OBJECT **title, int extraH, int extraV, int textFrom) {
	// Why not 2 ????
	int width = TLwidth + extraH + TRwidth - 3;
	int height = TLheight + extraV + BLheight - 3;

	// Create a rectangle object
	RectObject = *rect = TranslucentObject(width, height);

	// add it to display list and position it
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), *rect);
	MultiSetAniXY(*rect, InvD[ino].inventoryX + 1, InvD[ino].inventoryY + 1);
	MultiSetZPosition(*rect, Z_INV_BRECT);

	// Create text object using title string
	if (textFrom == FROM_HANDLE) {
		LoadStringRes(InvD[ino].hInvTitle, tBufferAddr(), TBUFSZ);
		*title = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(), 0,
					InvD[ino].inventoryX + width/2, InvD[ino].inventoryY + M_TOFF,
					hTagFontHandle(), TXT_CENTRE);
		assert(*title); // Inventory title string produced NULL text
		MultiSetZPosition(*title, Z_INV_HTEXT);
	} else if (textFrom == FROM_STRING && cd.ixHeading != NO_HEADING) {
		LoadStringRes(configStrings[cd.ixHeading], tBufferAddr(), TBUFSZ);
		*title = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(), 0,
					InvD[ino].inventoryX + width/2, InvD[ino].inventoryY + M_TOFF,
					hTagFontHandle(), TXT_CENTRE);
		assert(*title); // Inventory title string produced NULL text
		MultiSetZPosition(*title, Z_INV_HTEXT);
	}
}

/**
 * Insert a part of the inventory window frame onto the display list.
 */

static OBJECT *AddObject(const FREEL *pfreel, int num) {
	const MULTI_INIT *pmi;	// Get the MULTI_INIT structure
	IMAGE *pim;
	OBJECT *pPlayObj;

	// Get pointer to image
	pim = GetImageFromReel(pfreel, &pmi);

	// Poke in the background palette
	pim->hImgPal = TO_LE_32(BackPal());

	// Horrible bodge involving global variables to save
	// width and/or height of some window frame components
	if (num == TL) {
		TLwidth = FROM_LE_16(pim->imgWidth);
		TLheight = FROM_LE_16(pim->imgHeight);
	} else if (num == TR) {
		TRwidth = FROM_LE_16(pim->imgWidth);
	} else if (num == BL) {
		BLheight = FROM_LE_16(pim->imgHeight);
	}

	// Set up and insert the multi-object
	pPlayObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), pPlayObj);

	return pPlayObj;
}

/**
 * Display the scroll bar slider.
 */

void AddSlider(OBJECT **slide, const FILM *pfilm) {
	SlideObject = *slide = AddObject(&pfilm->reels[IX_SLIDE], -1);
	MultiSetAniXY(*slide, MultiRightmost(RectObject)-M_SXOFF+2, InvD[ino].inventoryY + slideY);
	MultiSetZPosition(*slide, Z_INV_MFRAME);
}

enum {
	SLIDE_RANGE	= 81,
	SLIDE_MINX	= 8,
	SLIDE_MAXX	= 8+SLIDE_RANGE,

	MDTEXT_YOFF	= 6,
	MDTEXT_XOFF	= -4
};

/**
 * Display a box with some text in it.
 */

void AddBox(int *pi, int i) {
	int x	= InvD[ino].inventoryX + cd.Box[i].xpos;
	int y	= InvD[ino].inventoryY + cd.Box[i].ypos;
	int *pival = cd.Box[i].ival;
	int	xdisp;
	const FILM *pfilm;

	switch (cd.Box[i].boxType) {
	default:
		// Give us a box
		iconArray[*pi] = RectangleObject(BackPal(), COL_BOX, cd.Box[i].w, cd.Box[i].h);
		MultiInsertObject(GetPlayfieldList(FIELD_STATUS), iconArray[*pi]);
		MultiSetAniXY(iconArray[*pi], x, y);
		MultiSetZPosition(iconArray[*pi], Z_INV_BRECT+1);
		*pi += 1;

		// Stick in the text
		if (cd.Box[i].ixText == USE_POINTER) {
			if (cd.Box[i].boxText != NULL) {
				if (cd.Box[i].boxType == RGROUP) {
					iconArray[*pi] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), cd.Box[i].boxText, 0,
#ifdef JAPAN
							x+2, y+2, hTagFontHandle(), 0);
#else
							x+2, y, hTagFontHandle(), 0);
#endif
				} else {
					iconArray[*pi] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), cd.Box[i].boxText, 0,
#ifdef JAPAN
// Note: it never seems to go here!
							x + cd.Box[i].w/2, y+2, hTagFontHandle(), TXT_CENTRE);
#else
							x + cd.Box[i].w/2, y, hTagFontHandle(), TXT_CENTRE);
#endif
				}
				MultiSetZPosition(iconArray[*pi], Z_INV_ITEXT);
				*pi += 1;
			}
		} else {
			LoadStringRes(configStrings[cd.Box[i].ixText], tBufferAddr(), TBUFSZ);
			assert(cd.Box[i].boxType != RGROUP); // You'll need to add some code!
			iconArray[*pi] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(), 0,
#ifdef JAPAN
					x + cd.Box[i].w/2, y+2, hTagFontHandle(), TXT_CENTRE);
#else
					x + cd.Box[i].w/2, y, hTagFontHandle(), TXT_CENTRE);
#endif
			MultiSetZPosition(iconArray[*pi], Z_INV_ITEXT);
			*pi += 1;
		}
		break;

	case AAGBUT:
	case ARSGBUT:
		pfilm = (const FILM *)LockMem(winPartsf);

		iconArray[*pi] = AddObject(&pfilm->reels[cd.Box[i].bi+NORMGRAPH], -1);
		MultiSetAniXY(iconArray[*pi], x, y);
		MultiSetZPosition(iconArray[*pi], Z_INV_BRECT+1);
		*pi += 1;

		break;

#if defined(USE_3FLAGS) || defined(USE_4FLAGS) || defined(USE_5FLAGS)
	case FRGROUP:
		assert(flagFilm != 0); // Language flags not declared!

		pfilm = (const FILM *)LockMem(flagFilm);

		if (bAmerica && cd.Box[i].bi == FIX_UK)
			cd.Box[i].bi = FIX_USA;

		iconArray[*pi] = AddObject(&pfilm->reels[cd.Box[i].bi], -1);
		MultiSetAniXY(iconArray[*pi], x, y);
		MultiSetZPosition(iconArray[*pi], Z_INV_BRECT+2);
		*pi += 1;

		break;
#endif
	case FLIP:
		pfilm = (const FILM *)LockMem(winPartsf);

		if (*(cd.Box[i].ival))
			iconArray[*pi] = AddObject(&pfilm->reels[cd.Box[i].bi], -1);
		else
			iconArray[*pi] = AddObject(&pfilm->reels[cd.Box[i].bi+1], -1);
		MultiSetAniXY(iconArray[*pi], x, y);
		MultiSetZPosition(iconArray[*pi], Z_INV_BRECT+1);
		*pi += 1;

		// Stick in the text
		assert(cd.Box[i].ixText != USE_POINTER);
		LoadStringRes(configStrings[cd.Box[i].ixText], tBufferAddr(), TBUFSZ);
		iconArray[*pi] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(), 0,
				x+MDTEXT_XOFF, y+MDTEXT_YOFF, hTagFontHandle(), TXT_RIGHT);
		MultiSetZPosition(iconArray[*pi], Z_INV_ITEXT);
		*pi += 1;
		break;

	case TOGGLE:
		pfilm = (const FILM *)LockMem(winPartsf);

		cd.Box[i].bi = *(cd.Box[i].ival) ? IX_TICK1 : IX_CROSS1;
		iconArray[*pi] = AddObject(&pfilm->reels[cd.Box[i].bi+NORMGRAPH], -1);
		MultiSetAniXY(iconArray[*pi], x, y);
		MultiSetZPosition(iconArray[*pi], Z_INV_BRECT+1);
		*pi += 1;

		// Stick in the text
		assert(cd.Box[i].ixText != USE_POINTER);
		LoadStringRes(configStrings[cd.Box[i].ixText], tBufferAddr(), TBUFSZ);
		iconArray[*pi] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(), 0,
				x+MDTEXT_XOFF, y+MDTEXT_YOFF, hTagFontHandle(), TXT_RIGHT);
		MultiSetZPosition(iconArray[*pi], Z_INV_ITEXT);
		*pi += 1;
		break;

	case SLIDER:
		pfilm = (const FILM *)LockMem(winPartsf);
		xdisp = SLIDE_RANGE*(*pival)/cd.Box[i].w;

		iconArray[*pi] = AddObject(&pfilm->reels[IX_MDGROOVE], -1);
		MultiSetAniXY(iconArray[*pi], x, y);
		MultiSetZPosition(iconArray[*pi], Z_MDGROOVE);
		*pi += 1;
		iconArray[*pi] = AddObject(&pfilm->reels[IX_MDSLIDER], -1);
		MultiSetAniXY(iconArray[*pi], x+SLIDE_MINX+xdisp, y);
		MultiSetZPosition(iconArray[*pi], Z_MDSLIDER);
		assert(numMdSlides < MAXSLIDES);
		mdSlides[numMdSlides].num = i;
		mdSlides[numMdSlides].min = x+SLIDE_MINX;
		mdSlides[numMdSlides].max = x+SLIDE_MAXX;
		mdSlides[numMdSlides++].obj = iconArray[*pi];
		*pi += 1;

		// Stick in the text
		assert(cd.Box[i].ixText != USE_POINTER);
		LoadStringRes(configStrings[cd.Box[i].ixText], tBufferAddr(), TBUFSZ);
		iconArray[*pi] = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), tBufferAddr(), 0,
				x+MDTEXT_XOFF, y+MDTEXT_YOFF, hTagFontHandle(), TXT_RIGHT);
		MultiSetZPosition(iconArray[*pi], Z_INV_ITEXT);
		*pi += 1;
		break;
	}
}

/**
 * Display some boxes.
 */
static void AddBoxes(bool posnSlide) {
	int	oCount = NUMHL;	// Object count - allow for HL1, HL2 etc.

	DumpIconArray();
	numMdSlides = 0;

	for (int i = 0; i < cd.NumBoxes; i++) {
		AddBox(&oCount, i);
	}

	if (cd.bExtraWin) {
		if (posnSlide)
			slideY = slideYmin + (cd.fileBase*(slideYmax-slideYmin))/(MAX_SFILES-NUM_SL_RGROUP);
		MultiSetAniXY(SlideObject, InvD[ino].inventoryX + 24 + 179, slideY);
	}

	assert(oCount < MAX_ICONS); // added too many icons
}

/**
 * Display the scroll bar slider.
 */

void AddEWSlider(OBJECT **slide, const FILM *pfilm) {
	SlideObject = *slide = AddObject(&pfilm->reels[IX_SLIDE], -1);
	MultiSetAniXY(*slide, InvD[ino].inventoryX + 24 + 127, slideY);
	MultiSetZPosition(*slide, Z_INV_MFRAME);
}

/**
 * AddExtraWindow
 */

int AddExtraWindow(int x, int y, OBJECT **retObj) {
	int	n = 0;
	const FILM *pfilm;

	// Get the frame's data
	pfilm = (const FILM *)LockMem(winPartsf);

	x += 20;
	y += 24;

// Draw the four corners
	retObj[n] = AddObject(&pfilm->reels[IX_RTL], -1);	// Top left
	MultiSetAniXY(retObj[n], x, y);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_NTR], -1);	// Top right
	MultiSetAniXY(retObj[n], x + 152, y);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_BL], -1);	// Bottom left
	MultiSetAniXY(retObj[n], x, y + 124);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_BR], -1);	// Bottom right
	MultiSetAniXY(retObj[n], x + 152, y + 124);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;

// Draw the edges
	retObj[n] = AddObject(&pfilm->reels[IX_H156], -1);	// Top
	MultiSetAniXY(retObj[n], x + 6, y);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_H156], -1);	// Bottom
	MultiSetAniXY(retObj[n], x + 6, y + 143);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_V104], -1);	// Left
	MultiSetAniXY(retObj[n], x, y + 20);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_V104], -1);	// Right 1
	MultiSetAniXY(retObj[n], x + 179, y + 20);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;
	retObj[n] = AddObject(&pfilm->reels[IX_V104], -1);	// Right 2
	MultiSetAniXY(retObj[n], x + 188, y + 20);
	MultiSetZPosition(retObj[n], Z_INV_MFRAME);
	n++;

	slideY = slideYmin = y + 9;
	slideYmax = y + 134;
	AddEWSlider(&retObj[n++], pfilm);

	return n;
}


enum InventoryType { EMPTY, FULL, CONF };

/**
 * Construct an inventory window - either a standard one, with
 * background, slider and icons, or a re-sizing window.
 */
void ConstructInventory(InventoryType filling) {
	int	eH, eV;		// Extra width and height
	int	n = 0;		// Index into object array
	int	zpos;		// Z-position of frame
	int	invX = InvD[ino].inventoryX;
	int	invY = InvD[ino].inventoryY;
	OBJECT **retObj;
	const FILM *pfilm;

	extern bool RePosition(void);	// Forward reference
	// Select the object array to use
	if (filling == FULL || filling == CONF) {
		retObj = objArray;		// Standard window
		zpos = Z_INV_MFRAME;
	} else {
		retObj = DobjArray;		// Re-sizing window
		zpos = Z_INV_RFRAME;
	}

	// Dispose of anything it may be replacing
	for (int i = 0; i < MAX_WCOMP; i++) {
		if (retObj[i] != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), retObj[i]);
			retObj[i] = NULL;
		}
	}

	// Get the frame's data
	pfilm = (const FILM *)LockMem(winPartsf);

	// Standard window is of granular dimensions
	if (filling == FULL) {
		// Round-up/down to nearest number of icons
		if (SuppH > ITEM_WIDTH / 2)
			InvD[ino].NoofHicons++;
		if (SuppV > ITEM_HEIGHT / 2)
			InvD[ino].NoofVicons++;
		SuppH = SuppV = 0;
	}

	// Extra width and height
	eH = (InvD[ino].NoofHicons - 1) * (ITEM_WIDTH+1) + SuppH;
	eV = (InvD[ino].NoofVicons - 1) * (ITEM_HEIGHT+1) + SuppV;

	// Which window frame corners to use
	if (filling == FULL && ino != INV_CONV) {
		TL = IX_TL;
		TR = IX_TR;
		BL = IX_BL;
		BR = IX_BR;
	} else {
		TL = IX_RTL;
		TR = IX_RTR;
		BL = IX_BL;
		BR = IX_RBR;
	}

// Draw the four corners
	retObj[n] = AddObject(&pfilm->reels[TL], TL);
	MultiSetAniXY(retObj[n], invX, invY);
	MultiSetZPosition(retObj[n], zpos);
	n++;
	retObj[n] = AddObject(&pfilm->reels[TR], TR);
	MultiSetAniXY(retObj[n], invX + TLwidth + eH, invY);
	MultiSetZPosition(retObj[n], zpos);
	n++;
	retObj[n] = AddObject(&pfilm->reels[BL], BL);
	MultiSetAniXY(retObj[n], invX, invY + TLheight + eV);
	MultiSetZPosition(retObj[n], zpos);
	n++;
	retObj[n] = AddObject(&pfilm->reels[BR], BR);
	MultiSetAniXY(retObj[n], invX + TLwidth + eH, invY + TLheight + eV);
	MultiSetZPosition(retObj[n], zpos);
	n++;

// Draw extra Top and bottom parts
	if (InvD[ino].NoofHicons > 1) {
		// Top side
		retObj[n] = AddObject(&pfilm->reels[hFillers[InvD[ino].NoofHicons-2]], -1);
		MultiSetAniXY(retObj[n], invX + TLwidth, invY);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Bottom of header box
		if (filling == FULL) {
			retObj[n] = AddObject(&pfilm->reels[hFillers[InvD[ino].NoofHicons-2]], -1);
			MultiSetAniXY(retObj[n], invX + TLwidth, invY + M_TBB + 1);
			MultiSetZPosition(retObj[n], zpos);
			n++;

			// Extra bits for conversation - hopefully temporary
			if (ino == INV_CONV) {
				retObj[n] = AddObject(&pfilm->reels[IX_H26], -1);
				MultiSetAniXY(retObj[n], invX + TLwidth - 2, invY + M_TBB + 1);
				MultiSetZPosition(retObj[n], zpos);
				n++;

				retObj[n] = AddObject(&pfilm->reels[IX_H52], -1);
				MultiSetAniXY(retObj[n], invX + eH - 10, invY + M_TBB + 1);
				MultiSetZPosition(retObj[n], zpos);
				n++;
			}
		}

		// Bottom side
		retObj[n] = AddObject(&pfilm->reels[hFillers[InvD[ino].NoofHicons-2]], -1);
		MultiSetAniXY(retObj[n], invX + TLwidth, invY + TLheight + eV + BLheight - M_TH + 1);
		MultiSetZPosition(retObj[n], zpos);
		n++;
	}
	if (SuppH) {
		int offx = TLwidth + eH - 26; 
		if (offx < TLwidth)	// Not too far!
			offx = TLwidth;

		// Top side extra
		retObj[n] = AddObject(&pfilm->reels[IX_H26], -1);
		MultiSetAniXY(retObj[n], invX + offx, invY);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Bottom side extra
		retObj[n] = AddObject(&pfilm->reels[IX_H26], -1);
		MultiSetAniXY(retObj[n], invX + offx, invY + TLheight + eV + BLheight - M_TH + 1);
		MultiSetZPosition(retObj[n], zpos);
		n++;
	}

// Draw extra side parts
	if (InvD[ino].NoofVicons > 1) {
		// Left side
		retObj[n] = AddObject(&pfilm->reels[vFillers[InvD[ino].NoofVicons-2]], -1);
		MultiSetAniXY(retObj[n], invX, invY + TLheight);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Left side of scroll bar
		if (filling == FULL && ino != INV_CONV) {
			retObj[n] = AddObject(&pfilm->reels[vFillers[InvD[ino].NoofVicons-2]], -1);
			MultiSetAniXY(retObj[n], invX + TLwidth + eH + M_SBL + 1, invY + TLheight);
			MultiSetZPosition(retObj[n], zpos);
			n++;
		}

		// Right side
		retObj[n] = AddObject(&pfilm->reels[vFillers[InvD[ino].NoofVicons-2]], -1);
		MultiSetAniXY(retObj[n], invX + TLwidth + eH + TRwidth - M_SW + 1, invY + TLheight);
		MultiSetZPosition(retObj[n], zpos);
		n++;
	}
	if (SuppV) {
		int offy = TLheight + eV - 26;
		if (offy < 5)
			offy = 5;

		// Left side extra
		retObj[n] = AddObject(&pfilm->reels[IX_V26], -1);
		MultiSetAniXY(retObj[n], invX, invY + offy);
		MultiSetZPosition(retObj[n], zpos);
		n++;

		// Right side extra
		retObj[n] = AddObject(&pfilm->reels[IX_V26], -1);
		MultiSetAniXY(retObj[n], invX + TLwidth + eH + TRwidth - M_SW + 1, invY + offy);
		MultiSetZPosition(retObj[n], zpos);
		n++;
	}

	OBJECT **rect, **title;

// Draw background, slider and icons
	if (filling == FULL) {
		rect = &retObj[n++];
		title = &retObj[n++];

		AddBackground(rect, title, eH, eV, FROM_HANDLE);

		if (ino == INV_CONV)
			SlideObject = NULL;
		else if (InvD[ino].NoofItems > InvD[ino].NoofHicons*InvD[ino].NoofVicons) {
			slideYmin = TLheight - 2;
			slideYmax = TLheight + eV + 10;
			AddSlider(&retObj[n++], pfilm);
		}

		FillInInventory();
	}
	else if (filling == CONF) {
		rect = &retObj[n++];
		title = &retObj[n++];

		AddBackground(rect, title, eH, eV, FROM_STRING);
		if (cd.bExtraWin)
			n += AddExtraWindow(invX, invY, &retObj[n]);
		AddBoxes(true);
	}

	assert(n < MAX_WCOMP); // added more parts than we can handle!

	// Reposition returns TRUE if needs to move
	if (InvD[ino].moveable && filling == FULL && RePosition()) {
		ConstructInventory(FULL);
	}
}


/**
 * Call this when drawing a 'FULL', movable inventory. Checks that the
 * position of the Translucent object is within limits. If it isn't,
 * adjusts the x/y position of the current inventory and returns TRUE.
 */
bool RePosition(void) {
	int	p;
	bool	bMoveitMoveit = false;

	assert(RectObject); // no recangle object!

	// Test for off-screen horizontally
	p = MultiLeftmost(RectObject);
	if (p > MAXLEFT) {
		// Too far to the right
		InvD[ino].inventoryX += MAXLEFT - p;
		bMoveitMoveit = true;			// I like to....
	} else {
		// Too far to the left?
		p = MultiRightmost(RectObject);
		if (p < MINRIGHT) {
			InvD[ino].inventoryX += MINRIGHT - p;
			bMoveitMoveit = true;		// I like to....
		}
	}

	// Test for off-screen vertically
	p = MultiHighest(RectObject);
	if (p < MINTOP) {
		// Too high
		InvD[ino].inventoryY += MINTOP - p;
		bMoveitMoveit = true;			// I like to....
	} else if (p > MAXTOP) {
		// Too low
		InvD[ino].inventoryY += MAXTOP - p;
		bMoveitMoveit = true;			// I like to....
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
void AlterCursor(int num) {
	const FREEL *pfreel;
	IMAGE *pim;

	// Get pointer to image
	pim = GetImageFromFilm(winPartsf, num, &pfreel);

	// Poke in the background palette
	pim->hImgPal = TO_LE_32(BackPal());

	SetTempCursor(FROM_LE_32(pfreel->script));
}

enum InvCursorFN {IC_AREA, IC_DROP};

/**
 * InvCursor
 */
void InvCursor(InvCursorFN fn, int CurX, int CurY) {
	static enum { IC_NORMAL, IC_DR, IC_UR, IC_TB, IC_LR,
		IC_INV, IC_UP, IC_DN } ICursor = IC_NORMAL;	// FIXME: local static var

	int	area;		// The part of the window the cursor is over
	bool	restoreMain = false;

	// If currently dragging, don't be messing about with the cursor shape
	if (InvDragging != ID_NONE)
		return;

	switch (fn) {
	case IC_DROP:
		ICursor = IC_NORMAL;
		InvCursor(IC_AREA, CurX, CurY);
		break;

	case IC_AREA:
		area = InvArea(CurX, CurY);

		// Check for POINTED events
		if (ino == INV_CONF)
			InvBoxes(area == I_BODY, CurX, CurY);
		else
			InvLabels(area == I_BODY, CurX, CurY);

		// No cursor trails while within inventory window
		if (area == I_NOTIN)
			UnHideCursorTrails();
		else
			HideCursorTrails();

		switch (area) {
		case I_NOTIN:
			restoreMain = true;
			break;

		case I_TLEFT:
		case I_BRIGHT:
			if (!InvD[ino].resizable)
				restoreMain = true;
			else if (ICursor != IC_DR) {
				AlterCursor(IX_CURDD);
				ICursor = IC_DR;
			}
			break;

		case I_TRIGHT:
		case I_BLEFT:
			if (!InvD[ino].resizable)
				restoreMain = true;
			else if (ICursor != IC_UR) {
				AlterCursor(IX_CURDU);
				ICursor = IC_UR;
			}
			break;

		case I_TOP:
		case I_BOTTOM:
			if (!InvD[ino].resizable) {
				restoreMain = true;
				break;
			}
			if (ICursor != IC_TB) {
				AlterCursor(IX_CURUD);
				ICursor = IC_TB;
			}
			break;

		case I_LEFT:
		case I_RIGHT:
			if (!InvD[ino].resizable)
				restoreMain = true;
			else if (ICursor != IC_LR) {
				AlterCursor(IX_CURLR);
				ICursor = IC_LR;
			}
			break;

		case I_UP:
		case I_SLIDE_UP:
		case I_DOWN:
		case I_SLIDE_DOWN:
		case I_SLIDE:
		case I_MOVE:
		case I_BODY:
			restoreMain = true;
			break;
		}
		break;
	}

	if (restoreMain && ICursor != IC_NORMAL) {
		RestoreMainCursor();
		ICursor = IC_NORMAL;
	}
}




/*-------------------------------------------------------------------------*/


/**************************************************************************/
/******************** Conversation specific functions *********************/
/**************************************************************************/


void ConvAction(int index) {
	assert(ino == INV_CONV); // not conv. window!

	switch (index) {
	case INV_NOICON:
		return;

	case INV_CLOSEICON:
		thisConvIcon = -1;	// Postamble
		break;

	case INV_OPENICON:
		thisConvIcon = -2;	// Preamble
		break;

	default:
		thisConvIcon = InvD[ino].ItemOrder[index];
		break;
	}

	RunPolyTinselCode(thisConvPoly, CONVERSE, BE_NONE, true);
}
/*-------------------------------------------------------------------------*/

void AddIconToPermanentDefaultList(int icon) {
	int i;

	// See if it's already there
	for (i = 0; i < Num0Order; i++) {
		if (Inv0Order[i] == icon)
			break;
	}

	// Add it if it isn't already there
	if (i == Num0Order) {
		Inv0Order[Num0Order++] = icon;
	}
}

/*-------------------------------------------------------------------------*/

void convPos(int fn) {
	if (fn == CONV_DEF)
		InvD[INV_CONV].inventoryY = 8;
	else if (fn == CONV_BOTTOM)
		InvD[INV_CONV].inventoryY = 150;
}

void ConvPoly(HPOLYGON hPoly) {
	thisConvPoly = hPoly;
}

int convIcon(void) {
	return thisConvIcon;
}

void CloseDownConv(void) {
	if (InventoryState == ACTIVE_INV && ino == INV_CONV) {
		KillInventory();
	}
}

void convHide(bool hide) {
	int aniX, aniY;
	int i;

	if (InventoryState == ACTIVE_INV && ino == INV_CONV) {
		if (hide) {
			for (i = 0; objArray[i] && i < MAX_WCOMP; i++) {
				MultiAdjustXY(objArray[i], 2*SCREEN_WIDTH, 0);
			}
			for (i = 0; iconArray[i] && i < MAX_ICONS; i++) {
				MultiAdjustXY(iconArray[i], 2*SCREEN_WIDTH, 0);
			}
			InventoryHidden = true;

			InvLabels(false, 0, 0);
		} else {
			InventoryHidden = false;

			for (i = 0; objArray[i] && i < MAX_WCOMP; i++) {
				MultiAdjustXY(objArray[i], -2*SCREEN_WIDTH, 0);
			}

			// Don't flash if items changed. If they have, will be redrawn anyway.
			if (!ItemsChanged) {
				for (i = 0; iconArray[i] && i < MAX_ICONS; i++) {
					MultiAdjustXY(iconArray[i], -2*SCREEN_WIDTH, 0);
				}
			}

			GetCursorXY(&aniX, &aniY, false);
			InvLabels(true, aniX, aniY);
		}
	}
}

bool convHid(void) {
	return InventoryHidden;
}


/**************************************************************************/
/******************* Open and closing functions ***************************/
/**************************************************************************/

/**
 * Start up an inventory window.
 */

void PopUpInventory(int invno) {
	assert((invno == INV_1 || invno == INV_2 || invno == INV_CONV || invno == INV_CONF)); // Trying to open illegal inventory

	if (InventoryState == IDLE_INV) {
		bOpenConf = false;	// Better safe than sorry...

		DisableTags();		// Tags disabled during inventory

		if (invno == INV_CONV) {	// Conversation window?
			// Start conversation with permanent contents
			memset(InvD[INV_CONV].ItemOrder, 0, MAX_ININV*sizeof(int));
			memcpy(InvD[INV_CONV].ItemOrder, Inv0Order, Num0Order*sizeof(int));
			InvD[INV_CONV].NoofItems = Num0Order;
			thisConvIcon = 0;
		} else if (invno == INV_CONF) {	// Configuration window?
			cd.selBox = NOBOX;
			cd.pointBox = NOBOX;
		}

		ino = invno;			// The open inventory

		ItemsChanged = false;		// Nothing changed
		InvDragging = ID_NONE;		// Not dragging
		InventoryState = ACTIVE_INV;	// Inventory actiive
		InventoryHidden = false;	// Not hidden
		InventoryMaximised = InvD[ino].bMax;
		if (invno != INV_CONF)	// Configuration window?
			ConstructInventory(FULL);	// Draw it up
		else {
			ConstructInventory(CONF);	// Draw it up
		}
	}
}

void SetConfGlobals(CONFINIT *ci) {
	InvD[INV_CONF].MinHicons = InvD[INV_CONF].MaxHicons = InvD[INV_CONF].NoofHicons = ci->h;
	InvD[INV_CONF].MaxVicons = InvD[INV_CONF].MinVicons = InvD[INV_CONF].NoofVicons = ci->v;
	InvD[INV_CONF].inventoryX = ci->x;
	InvD[INV_CONF].inventoryY = ci->y;
	cd.bExtraWin = ci->bExtraWin;
	cd.Box = ci->Box;
	cd.NumBoxes = ci->NumBoxes;
	cd.ixHeading = ci->ixHeading;
}

/**
 * PopupConf
 */

void PopUpConf(CONFTYPE type) {
	int curX, curY;

	if (InventoryState != IDLE_INV)
		return;

	InvD[INV_CONF].resizable = false;
	InvD[INV_CONF].moveable = false;

	switch (type) {
	case SAVE:
	case LOAD:
		if (type == SAVE) {
			SetCursorScreenXY(262, 91);
			SetConfGlobals(&ciSave);
			cd.editableRgroup = true;
		} else {
			SetConfGlobals(&ciLoad);
			cd.editableRgroup = false;
		}
		firstFile(0);
		break;

	case QUIT:
#ifdef JAPAN
		SetCursorScreenXY(180, 106);
#else
		SetCursorScreenXY(180, 90);
#endif
		SetConfGlobals(&ciQuit);
		break;

	case RESTART:
#ifdef JAPAN
		SetCursorScreenXY(180, 106);
#else
		SetCursorScreenXY(180, 90);
#endif
		SetConfGlobals(&ciRestart);
		break;

	case OPTION:
		SetConfGlobals(&ciOption);
		break;

	case CONTROLS:
		SetConfGlobals(&ciControl);
		break;

	case SOUND:
		SetConfGlobals(&ciSound);
		break;

#ifndef JAPAN
	case SUBT:
		SetConfGlobals(&ciSubtitles);
		break;
#endif

	case TOPWIN:
		SetConfGlobals(&ciTopWin);
		ino = INV_CONF;
		ConstructInventory(CONF);	// Draw it up
		InventoryState = BOGUS_INV;
		return;

	default:
		return;
	}

	if (HeldItem != INV_NOICON)
		DelAuxCursor();			// no longer aux cursor

	PopUpInventory(INV_CONF);

	if (type == SAVE || type == LOAD)
		Select(0, false);
#ifndef JAPAN
#if !defined(USE_3FLAGS) || !defined(USE_4FLAGS) || !defined(USE_5FLAGS)
	else if (type == SUBT) {
#ifdef USE_3FLAGS
		// VERY quick dirty bodges
		if (language == TXT_FRENCH)
			Select(0, false);
		else if (language == TXT_GERMAN)
			Select(1, false);
		else
			Select(2, false);
#elif defined(USE_4FLAGS)
		Select(language-1, false);
#else
		Select(language, false);
#endif
	}
#endif
#endif // JAPAN

	GetCursorXY(&curX, &curY, false);
	InvCursor(IC_AREA, curX, curY);
}

/**
 * Close down an inventory window.
 */

void KillInventory(void) {
	if (objArray[0] != NULL) {
		DumpObjArray();
		DumpDobjArray();
		DumpIconArray();
	}

	if (InventoryState == ACTIVE_INV) {
		EnableTags();

		InvD[ino].bMax = InventoryMaximised;

		UnHideCursorTrails();
		_vm->divertKeyInput(NULL);
	}

	InventoryState = IDLE_INV;

	if (bOpenConf) {
		bOpenConf = false;
		PopUpConf(OPTION);
	} else if (ino == INV_CONF)
		InventoryIconCursor();
}

void CloseInventory(void) {
	// If not active, ignore this
	if (InventoryState != ACTIVE_INV)
		return;

	// If hidden, a conversation action is still underway - ignore this
	if (InventoryHidden)
		return;

	// If conversation, this is a closeing event
	if (ino == INV_CONV)
		ConvAction(INV_CLOSEICON);

	KillInventory();

	RestoreMainCursor();
}



/**************************************************************************/
/************************ The inventory process ***************************/
/**************************************************************************/

/**
 * Redraws the icons if appropriate. Also handle button press/toggle effects
 */
void InventoryProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (1) {
		CORO_SLEEP(1);		// allow scheduling

		if (objArray[0] != NULL) {
			if (ItemsChanged && ino != INV_CONF && !InventoryHidden) {
				FillInInventory();

				// Needed when clicking on scroll bar.
				int	curX, curY;
				GetCursorXY(&curX, &curY, false);
				InvCursor(IC_AREA, curX, curY);

				ItemsChanged = false;
			}
			if (ino != INV_CONF) {
				for (int i = 0; i < MAX_ICONS; i++) {
					if (iconArray[i] != NULL)
						StepAnimScript(&iconAnims[i]);
				}
			}
			if (InvDragging == ID_MDCONT) {
				// Mixing desk control
				int sval, index, *pival;

				index = cd.selBox & ~IS_MASK;
				pival = cd.Box[index].ival;
				sval = *pival;

				if (cd.selBox & IS_LEFT) {
					*pival -= cd.Box[index].h;
					if (*pival < 0)
						*pival = 0;
				} else if (cd.selBox & IS_RIGHT) {
					*pival += cd.Box[index].h;
					if (*pival > cd.Box[index].w)
						*pival = cd.Box[index].w;
				}

				if (sval != *pival) {
					SlideMSlider(0, (cd.selBox & IS_RIGHT) ? S_TIMEUP : S_TIMEDN);
				}
			}
		}

		if (g_buttonEffect.bButAnim) {
			assert(g_buttonEffect.box);
			if (g_buttonEffect.press) {
				if (g_buttonEffect.box->boxType == AAGBUT || g_buttonEffect.box->boxType == ARSGBUT)
					CORO_INVOKE_1(ButtonPress, g_buttonEffect.box);
				switch (g_buttonEffect.box->boxFunc) {
				case SAVEGAME:
					KillInventory();
					InvSaveGame();
					break;
				case LOADGAME:
					KillInventory();
					InvLoadGame();
					break;
				case IQUITGAME:
					_vm->quitFlag = true;
					break;
				case CLOSEWIN:
					KillInventory();
					break;
				case OPENLOAD:
					KillInventory();
					PopUpConf(LOAD);
					break;
				case OPENSAVE:
					KillInventory();
					PopUpConf(SAVE);
					break;
				case OPENREST:
					KillInventory();
					PopUpConf(RESTART);
					break;
				case OPENSOUND:
					KillInventory();
					PopUpConf(SOUND);
					break;
				case OPENCONT:
					KillInventory();
					PopUpConf(CONTROLS);
					break;
	#ifndef JAPAN
				case OPENSUBT:
					KillInventory();
					PopUpConf(SUBT);
					break;
	#endif
				case OPENQUIT:
					KillInventory();
					PopUpConf(QUIT);
					break;
				case INITGAME:
					KillInventory();
					bRestart = true;
					break;
	#if defined(USE_3FLAGS) || defined(USE_4FLAGS) || defined(USE_5FLAGS)
				case CLANG:
					if (!LanguageChange())
						KillInventory();
					break;
				case RLANG:
					KillInventory();
					break;
	#endif
				default:
					break;
				}
			} else
				CORO_INVOKE_1(ButtonToggle, g_buttonEffect.box);
		
			g_buttonEffect.bButAnim = false;
		}

	}
	CORO_END_CODE;
}

/**************************************************************************/
/*************** Drag stuff - Resizing and moving window ******************/
/**************************************************************************/

/**
 * Appears to find the nearest entry in slideStuff[] to the supplied
 * y-coordinate.
 */
int NearestSlideY(int fity) {
	int nearDist = 1000;
	int thisDist;
	int nearI = 0;	// Index of nearest fit
	int i = 0;

	do {
		thisDist = ABS(slideStuff[i].y - fity);
		if (thisDist < nearDist) {
			nearDist = thisDist;
			nearI = i;
		}
	} while (slideStuff[++i].n != -1);
	return nearI;
}

/**
 * Gets called at the start and end of a drag on the slider, and upon
 * y-movement during such a drag.
 */
void SlideSlider(int y, SSFN fn) {
	static int newY = 0, lasti = 0;	// FIXME: local static var
	int gotoY, ati;

	// Only do this if there's a slider
	if (!SlideObject)
		return;

	switch (fn) {
	case S_START:			// Start of a drag on the slider
		newY = slideY;
		lasti = NearestSlideY(slideY);
		break;

	case S_SLIDE:			// Y-movement during drag
		newY = newY + y;		// New y-position

		if (newY < slideYmin)
			gotoY = slideYmin;	// Above top limit
		else if (newY > slideYmax)
			gotoY = slideYmax;	// Below bottom limit
		else
			gotoY = newY;		// Hunky-Dory

		// Move slider to new position
		MultiMoveRelXY(SlideObject, 0, gotoY - slideY);
		slideY = gotoY;

		// Re-draw icons if necessary
		ati = NearestSlideY(slideY);
		if (ati != lasti) {
			InvD[ino].FirstDisp = slideStuff[ati].n;
			assert(InvD[ino].FirstDisp >= 0); // negative first displayed
			ItemsChanged = true;
			lasti = ati;
		}
		break;

	case S_END:			// End of a drag on the slider
		// Draw icons from new start icon
		ati = NearestSlideY(slideY);
		InvD[ino].FirstDisp = slideStuff[ati].n;
		ItemsChanged = true;
		break;

	default:
		break;
	}
}

/**
 * Gets called at the start and end of a drag on the slider, and upon
 * y-movement during such a drag.
 */

void SlideCSlider(int y, SSFN fn) {
	static int newY = 0;	// FIXME: local static var
	int	gotoY;
	int	fc;

	// Only do this if there's a slider
	if (!SlideObject)
		return;

	switch (fn) {
	case S_START:			// Start of a drag on the slider
		newY = slideY;
		break;

	case S_SLIDE:			// Y-movement during drag
		newY = newY + y;		// New y-position

		if (newY < slideYmin)
			gotoY = slideYmin;	// Above top limit
		else if (newY > slideYmax)
			gotoY = slideYmax;	// Below bottom limit
		else
			gotoY = newY;		// Hunky-Dory

		slideY = gotoY;

		fc = cd.fileBase;
		firstFile((slideY-slideYmin)*(MAX_SFILES-NUM_SL_RGROUP)/(slideYmax-slideYmin));
		if (fc != cd.fileBase) {
			AddBoxes(false);
				fc -= cd.fileBase;
			cd.selBox += fc;
			if (cd.selBox < 0)
				cd.selBox = 0;
			else if (cd.selBox >= NUM_SL_RGROUP)
				cd.selBox = NUM_SL_RGROUP-1;
			Select(cd.selBox, true);
		}
		break;

	case S_END:			// End of a drag on the slider
		break;

	default:
		break;
	}
}

/**
 * Gets called at the start and end of a drag on a mixing desk slider,
 * and upon x-movement during such a drag.
 */

static void SlideMSlider(int x, SSFN fn) {
	static int newX = 0;	// FIXME: local static var
	int gotoX;
	int index, i;

	if (fn == S_END || fn == S_TIMEUP || fn == S_TIMEDN)
		;
	else if (!(cd.selBox & IS_SLIDER))
		return;

	// Work out the indices
	index = cd.selBox & ~IS_MASK;
	for (i = 0; i < numMdSlides; i++)
		if (mdSlides[i].num == index)
			break;
	assert(i < numMdSlides);

	switch (fn) {
	case S_START:			// Start of a drag on the slider
		// can use index as a throw-away value
		GetAniPosition(mdSlides[i].obj, &newX, &index);
		lX = sX = newX;
		break;

	case S_SLIDE:			// X-movement during drag
		if (x == 0)
			return;

		newX = newX + x;	// New x-position

		if (newX < mdSlides[i].min)
			gotoX = mdSlides[i].min;	// Below bottom limit
		else if (newX > mdSlides[i].max)
			gotoX = mdSlides[i].max;	// Above top limit
		else
			gotoX = newX;		// Hunky-Dory

		// Move slider to new position
		MultiMoveRelXY(mdSlides[i].obj, gotoX - sX, 0);
		sX = gotoX;

		if (lX != sX) {
			*cd.Box[index].ival = (sX - mdSlides[i].min)*cd.Box[index].w/SLIDE_RANGE;
			if (cd.Box[index].boxFunc == MIDIVOL)
				SetMidiVolume(*cd.Box[index].ival);
#ifdef MAC_OPTIONS
			if (cd.Box[index].boxFunc == MASTERVOL)
				SetSystemVolume(*cd.Box[index].ival);

			if (cd.Box[index].boxFunc == SAMPVOL)
				SetSampleVolume(*cd.Box[index].ival);
#endif
			lX = sX;
		}
		break;

	case S_TIMEUP:
	case S_TIMEDN:
		gotoX = SLIDE_RANGE*(*cd.Box[index].ival)/cd.Box[index].w;
		MultiSetAniX(mdSlides[i].obj, mdSlides[i].min+gotoX);

		if (cd.Box[index].boxFunc == MIDIVOL)
			SetMidiVolume(*cd.Box[index].ival);
#ifdef MAC_OPTIONS
			if (cd.Box[index].boxFunc == MASTERVOL)
				SetSystemVolume(*cd.Box[index].ival);

			if (cd.Box[index].boxFunc == SAMPVOL)
				SetSampleVolume(*cd.Box[index].ival);
#endif
		break;

	case S_END:			// End of a drag on the slider
		AddBoxes(false);	// Might change position slightly
#ifndef JAPAN
		if (ino == INV_CONF && cd.Box == subtitlesBox)
			Select(language, false);
#endif
		break;
	}
}

/**
 * Called from ChangeingSize() during re-sizing.
 */

void GettingTaller(void) {
	if (SuppV) {
		Ychange += SuppV;
		if (Ycompensate == 'T')
			InvD[ino].inventoryY += SuppV;
		SuppV = 0;
	}
	while (Ychange > (ITEM_HEIGHT+1) && InvD[ino].NoofVicons < InvD[ino].MaxVicons) {
		Ychange -= (ITEM_HEIGHT+1);
		InvD[ino].NoofVicons++;
		if (Ycompensate == 'T')
			InvD[ino].inventoryY -= (ITEM_HEIGHT+1);
	}
	if (InvD[ino].NoofVicons < InvD[ino].MaxVicons) {
		SuppV = Ychange;
		Ychange = 0;
		if (Ycompensate == 'T')
			InvD[ino].inventoryY -= SuppV;
	}
}

/**
 * Called from ChangeingSize() during re-sizing.
 */

void GettingShorter(void) {
	int StartNvi = InvD[ino].NoofVicons;
	int StartUv = SuppV;

	if (SuppV) {
		Ychange += (SuppV - (ITEM_HEIGHT+1));
		InvD[ino].NoofVicons++;
		SuppV = 0;
	}
	while (Ychange < -(ITEM_HEIGHT+1) && InvD[ino].NoofVicons > InvD[ino].MinVicons) {
		Ychange += (ITEM_HEIGHT+1);
		InvD[ino].NoofVicons--;
	}
	if (InvD[ino].NoofVicons > InvD[ino].MinVicons && Ychange) {
		SuppV = (ITEM_HEIGHT+1) + Ychange;
		InvD[ino].NoofVicons--;
		Ychange = 0;
	}
	if (Ycompensate == 'T')
		InvD[ino].inventoryY += (ITEM_HEIGHT+1)*(StartNvi - InvD[ino].NoofVicons) - (SuppV - StartUv);
}

/**
 * Called from ChangeingSize() during re-sizing.
 */

void GettingWider(void) {
	int StartNhi = InvD[ino].NoofHicons;
	int StartUh = SuppH;

	if (SuppH) {
		Xchange += SuppH;
		SuppH = 0;
	}
	while (Xchange > (ITEM_WIDTH+1) && InvD[ino].NoofHicons < InvD[ino].MaxHicons) {
		Xchange -= (ITEM_WIDTH+1);
		InvD[ino].NoofHicons++;
	}
	if (InvD[ino].NoofHicons < InvD[ino].MaxHicons) {
		SuppH = Xchange;
		Xchange = 0;
	}
	if (Xcompensate == 'L')
		InvD[ino].inventoryX += (ITEM_WIDTH+1)*(StartNhi - InvD[ino].NoofHicons) - (SuppH - StartUh);
}

/**
 * Called from ChangeingSize() during re-sizing.
 */

void GettingNarrower(void) {
	int StartNhi = InvD[ino].NoofHicons;
	int StartUh = SuppH;

	if (SuppH) {
		Xchange += (SuppH - (ITEM_WIDTH+1));
		InvD[ino].NoofHicons++;
		SuppH = 0;
	}
	while (Xchange < -(ITEM_WIDTH+1) && InvD[ino].NoofHicons > InvD[ino].MinHicons) {
		Xchange += (ITEM_WIDTH+1);
		InvD[ino].NoofHicons--;
	}
	if (InvD[ino].NoofHicons > InvD[ino].MinHicons && Xchange) {
		SuppH = (ITEM_WIDTH+1) + Xchange;
		InvD[ino].NoofHicons--;
		Xchange = 0;
	}
	if (Xcompensate == 'L')
		InvD[ino].inventoryX += (ITEM_WIDTH+1)*(StartNhi - InvD[ino].NoofHicons) - (SuppH - StartUh);
}


/**
 * Called from Xmovement()/Ymovement() during re-sizing.
 */

void ChangeingSize(void) {
	/* Make it taller or shorter if necessary. */
	if (Ychange > 0)
		GettingTaller();
	else if (Ychange < 0)
		GettingShorter();

	/* Make it wider or narrower if necessary. */
	if (Xchange > 0)
		GettingWider();
	else if (Xchange < 0)
		GettingNarrower();

	ConstructInventory(EMPTY);
}

/**
 * Called from cursor module when cursor moves while inventory is up.
 */

void Xmovement(int x) {
	int aniX, aniY;
	int i;

	if (x && objArray[0] != NULL) {
		switch (InvDragging) {
		case ID_MOVE:
			GetAniPosition(objArray[0], &InvD[ino].inventoryX, &aniY);
			InvD[ino].inventoryX +=x;
			MultiSetAniX(objArray[0], InvD[ino].inventoryX);
			for (i = 1; objArray[i] && i < MAX_WCOMP; i++)
				MultiMoveRelXY(objArray[i], x, 0);
			for (i = 0; iconArray[i] && i < MAX_ICONS; i++)
				MultiMoveRelXY(iconArray[i], x, 0);
			break;

		case ID_LEFT:
		case ID_TLEFT:
		case ID_BLEFT:
			Xchange -= x;
			ChangeingSize();
			break;

		case ID_RIGHT:
		case ID_TRIGHT:
		case ID_BRIGHT:
			Xchange += x;
			ChangeingSize();
			break;

		case ID_NONE:
			GetCursorXY(&aniX, &aniY, false);
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

void Ymovement(int y) {
	int aniX, aniY;
	int i;

	if (y && objArray[0] != NULL) {
		switch (InvDragging) {
		case ID_MOVE:
			GetAniPosition(objArray[0], &aniX, &InvD[ino].inventoryY);
			InvD[ino].inventoryY +=y;
			MultiSetAniY(objArray[0], InvD[ino].inventoryY);
			for (i = 1; objArray[i] && i < MAX_WCOMP; i++)
				MultiMoveRelXY(objArray[i], 0, y);
			for (i = 0; iconArray[i] && i < MAX_ICONS; i++)
				MultiMoveRelXY(iconArray[i], 0, y);
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
			Ychange += y;
			ChangeingSize();
			break;

		case ID_TOP:
		case ID_TLEFT:
		case ID_TRIGHT:
			Ychange -= y;
			ChangeingSize();
			break;

		case ID_NONE:
			GetCursorXY(&aniX, &aniY, false);
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

void InvDragStart(void) {
	int curX, curY;		// cursor's animation position

	GetCursorXY(&curX, &curY, false);

/*
* Do something different for Save/Restore screens
*/
	if (ino == INV_CONF) {
		int	whichbox;

		whichbox = WhichInvBox(curX, curY, true);

		if (whichbox == IB_SLIDE) {
			InvDragging = ID_CSLIDE;
			SlideCSlider(0, S_START);
		} else if (whichbox > 0 && (whichbox & IS_MASK)) {
			InvDragging = ID_MDCONT;	// Mixing desk control
			cd.selBox = whichbox;
			SlideMSlider(0, S_START);
		}
		return;
	}

/*
* Normal operation
*/
	switch (InvArea(curX, curY)) {
	case I_MOVE:
		if (InvD[ino].moveable) {
			InvDragging = ID_MOVE;
		}
		break;

	case I_SLIDE:
		InvDragging = ID_SLIDE;
		SlideSlider(0, S_START);
		break;

	case I_BOTTOM:
		if (InvD[ino].resizable) {
			Ychange = 0;
			InvDragging = ID_BOTTOM;
			Ycompensate = 'B';
		}
		break;

	case I_TOP:
		if (InvD[ino].resizable) {
			Ychange = 0;
			InvDragging = ID_TOP;
			Ycompensate = 'T';
		}
		break;

	case I_LEFT:
		if (InvD[ino].resizable) {
			Xchange = 0;
			InvDragging = ID_LEFT;
			Xcompensate = 'L';
		}
		break;

	case I_RIGHT:
		if (InvD[ino].resizable) {
			Xchange = 0;
			InvDragging = ID_RIGHT;
			Xcompensate = 'R';
		}
		break;

	case I_TLEFT:
		if (InvD[ino].resizable) {
			Ychange = 0;
			Ycompensate = 'T';
			Xchange = 0;
			Xcompensate = 'L';
			InvDragging = ID_TLEFT;
		}
		break;

	case I_TRIGHT:
		if (InvD[ino].resizable) {
			Ychange = 0;
			Ycompensate = 'T';
			Xchange = 0;
			Xcompensate = 'R';
			InvDragging = ID_TRIGHT;
		}
		break;

	case I_BLEFT:
		if (InvD[ino].resizable) {
			Ychange = 0;
			Ycompensate = 'B';
			Xchange = 0;
			Xcompensate = 'L';
			InvDragging = ID_BLEFT;
		}
		break;

	case I_BRIGHT:
		if (InvD[ino].resizable) {
			Ychange = 0;
			Ycompensate = 'B';
			Xchange = 0;
			Xcompensate = 'R';
			InvDragging = ID_BRIGHT;
		}
		break;
	}
}

/**
 * Called when a drag is over.
 */

void InvDragEnd(void) {
	int curX, curY;		// cursor's animation position

	GetCursorXY(&curX, &curY, false);

	if (InvDragging != ID_NONE) {
		if (InvDragging == ID_SLIDE) {
			SlideSlider(0, S_END);
		} else if (InvDragging == ID_CSLIDE) {
			;	// No action
		} else if (InvDragging == ID_MDCONT) {
			SlideMSlider(0, S_END);
		} else if (InvDragging == ID_MOVE) {
			;	// No action
		} else {
			// Were re-sizing. Redraw the whole thing.
			DumpDobjArray();
			DumpObjArray();
			ConstructInventory(FULL);

			// If this was the maximised, it no longer is!
			if (InventoryMaximised) {
				InventoryMaximised = false;
				InvD[ino].otherX = InvD[ino].inventoryX;
				InvD[ino].otherY = InvD[ino].inventoryY;
			}
		}
		InvDragging = ID_NONE;
	}

	// Cursor could well now be inappropriate
	InvCursor(IC_AREA, curX, curY);

	Xchange = Ychange = 0;		// Probably no need, but does no harm!
}


/**************************************************************************/
/************** Incoming events - further processing **********************/
/**************************************************************************/

/**
 * ConfAction
 */
void ConfAction(int i, bool dbl) {

	if (i >= 0) {
		switch (cd.Box[i].boxType) {
		case FLIP:
			if (dbl) {
				*(cd.Box[i].ival) ^= 1;	// XOR with true
				AddBoxes(false);
			}
			break;

		case TOGGLE:
			if (!g_buttonEffect.bButAnim) {
				g_buttonEffect.bButAnim = true;
				g_buttonEffect.box = &cd.Box[i];
				g_buttonEffect.press = false;
			}
			break;

		case RGROUP:
			if (dbl) {
				// Already highlighted
				switch (cd.Box[i].boxFunc) {
				case SAVEGAME:
					KillInventory();
					InvSaveGame();
					break;
				case LOADGAME:
					KillInventory();
					InvLoadGame();
					break;
				default:
					break;
				}
			} else {
				Select(i, false);
			}
			break;

#if defined(USE_3FLAGS) || defined(USE_4FLAGS) || defined(USE_5FLAGS)
		case FRGROUP:
			if (dbl) {
				Select(i, false);
				LanguageChange();
			} else {
				Select(i, false);
			}
			break;
#endif

		case AAGBUT:
		case ARSGBUT:
		case ARSBUT:
		case AABUT:
		case AATBUT:
			if (g_buttonEffect.bButAnim)
				break;

			g_buttonEffect.bButAnim = true;
			g_buttonEffect.box = &cd.Box[i];
			g_buttonEffect.press = true;
			break;
		default:
			break;
		}
	} else {
		ConfActionSpecial(i);
	}
}

static void ConfActionSpecial(int i) {
	switch (i) {
	case IB_NONE:
		break;
	case IB_UP:	// Scroll up
		if (cd.fileBase > 0) {
			firstFile(cd.fileBase-1);
			AddBoxes(true);
			if (cd.selBox < NUM_SL_RGROUP-1)
				cd.selBox += 1;
			Select(cd.selBox, true);
		}
		break;
	case IB_DOWN:	// Scroll down
		if (cd.fileBase < MAX_SFILES-NUM_SL_RGROUP) {
			firstFile(cd.fileBase+1);
			AddBoxes(true);
			if (cd.selBox)
				cd.selBox -= 1;
			Select(cd.selBox, true);
		}
		break;
	case IB_SLIDE_UP:
		if (cd.fileBase > 0) {
			firstFile(cd.fileBase-(NUM_SL_RGROUP-1));
			AddBoxes(true);
			cd.selBox = 0;
			Select(cd.selBox, true);
		}
		break;
	case IB_SLIDE_DOWN:	// Scroll down
		if (cd.fileBase < MAX_SFILES-NUM_SL_RGROUP) {
			firstFile(cd.fileBase+(NUM_SL_RGROUP-1));
			AddBoxes(true);
			cd.selBox = NUM_SL_RGROUP-1;
			Select(cd.selBox, true);
		}
		break;
	}
}
// SLIDE_UP and SLIDE_DOWN on d click??????

void InvPutDown(int index) {
	int aniX, aniY;
			// index is the drop position
	int hiIndex;	// Current position of held item (if in)

	// Find where the held item is positioned in this inventory (if it is)
	for (hiIndex = 0; hiIndex < InvD[ino].NoofItems; hiIndex++)
		if (InvD[ino].ItemOrder[hiIndex] == HeldItem)
			break;

	// If drop position would leave a gap, move it up
	if (index >= InvD[ino].NoofItems) {
		if (hiIndex == InvD[ino].NoofItems)	// Not in, add it
			index = InvD[ino].NoofItems;
		else
			index = InvD[ino].NoofItems - 1;
	}

	if (hiIndex == InvD[ino].NoofItems) {	// Not in, add it
		if (InvD[ino].NoofItems < InvD[ino].MaxInvObj) {
			InvD[ino].NoofItems++;

			// Don't leave it in the other inventory!
			if (InventoryPos(HeldItem) != INV_HELDNOTIN)
				RemFromInventory(ino == INV_1 ? INV_2 : INV_1, HeldItem);
		} else {
			// No room at the inn!
			return;
		}
	}

	// Position it in the inventory
	if (index < hiIndex) {
		memmove(&InvD[ino].ItemOrder[index + 1], &InvD[ino].ItemOrder[index], (hiIndex-index)*sizeof(int));
		InvD[ino].ItemOrder[index] = HeldItem;
	} else if (index > hiIndex) {
		memmove(&InvD[ino].ItemOrder[hiIndex], &InvD[ino].ItemOrder[hiIndex+1], (index-hiIndex)*sizeof(int));
		InvD[ino].ItemOrder[index] = HeldItem;
	} else {
		InvD[ino].ItemOrder[index] = HeldItem;
	}

	HeldItem = INV_NOICON;
	ItemsChanged = true;
	DelAuxCursor();
	RestoreMainCursor();
	GetCursorXY(&aniX, &aniY, false);
	InvCursor(IC_DROP, aniX, aniY);
}

void InvPdProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GetToken(TOKEN_LEFT_BUT);
	CORO_SLEEP(dclickSpeed+1);
	FreeToken(TOKEN_LEFT_BUT);

	// get the stuff copied to process when it was created
	int	*pindex = (int *)param;

	InvPutDown(*pindex);

	CORO_END_CODE;
}

void InvPickup(int index) {
	INV_OBJECT *invObj;

	if (index != INV_NOICON) {
		if (HeldItem == INV_NOICON && InvD[ino].ItemOrder[index] && InvD[ino].ItemOrder[index] != HeldItem) {
			// Pick-up
			invObj = findInvObject(InvD[ino].ItemOrder[index]);
			if (invObj->hScript)
				RunInvTinselCode(invObj, WALKTO, INV_PICKUP, index);
		} else if (HeldItem != INV_NOICON) {			// Put icon down
			// Put-down
			invObj = findInvObject(HeldItem);

			if (invObj->attribute & IO_DROPCODE && invObj->hScript)
				RunInvTinselCode(invObj, PUTDOWN, INV_PICKUP, index);

			else if (!(invObj->attribute & IO_ONLYINV1 && ino !=INV_1)
			     && !(invObj->attribute & IO_ONLYINV2 && ino !=INV_2))
				g_scheduler->createProcess(PID_TCODE, InvPdProcess, &index, sizeof(index));
		}
	}
}

/**
 * Pick up/put down icon
 */
void InvSLClick(void) {
	int i;
	int aniX, aniY;		// Cursor's animation position

	GetCursorXY(&aniX, &aniY, false);

	switch (InvArea(aniX, aniY)) {
	case I_NOTIN:
		if (ino == INV_CONV)
			ConvAction(INV_CLOSEICON);
		KillInventory();
		break;

	case I_SLIDE_UP:
		if (InvD[ino].NoofVicons == 1)
			InvD[ino].FirstDisp -= InvD[ino].NoofHicons;
		for (i = 1; i < InvD[ino].NoofVicons; i++)
			InvD[ino].FirstDisp -= InvD[ino].NoofHicons;
		if (InvD[ino].FirstDisp < 0)
			InvD[ino].FirstDisp = 0;
		ItemsChanged = true;
		break;

	case I_UP:
		InvD[ino].FirstDisp -= InvD[ino].NoofHicons;
		if (InvD[ino].FirstDisp < 0)
			InvD[ino].FirstDisp = 0;
		ItemsChanged = true;
		break;

	case I_SLIDE_DOWN:
		if (InvD[ino].NoofVicons == 1)
			if (InvD[ino].FirstDisp + InvD[ino].NoofHicons*InvD[ino].NoofVicons < InvD[ino].NoofItems)
				InvD[ino].FirstDisp += InvD[ino].NoofHicons;
		for (i = 1; i < InvD[ino].NoofVicons; i++) {
			if (InvD[ino].FirstDisp + InvD[ino].NoofHicons*InvD[ino].NoofVicons < InvD[ino].NoofItems)
				InvD[ino].FirstDisp += InvD[ino].NoofHicons;
		}
		ItemsChanged = true;
		break;

	case I_DOWN:
		if (InvD[ino].FirstDisp + InvD[ino].NoofHicons*InvD[ino].NoofVicons < InvD[ino].NoofItems) {
			InvD[ino].FirstDisp += InvD[ino].NoofHicons;
			ItemsChanged = true;
		}
		break;

	case I_BODY:
		if (ino == INV_CONF) {
			if (!InventoryHidden)
				ConfAction(WhichInvBox(aniX, aniY, false), false);
		} else {
			i = InvItem(&aniX, &aniY, false);

			// Special bodge for David, to
			// cater for drop in dead space between icons
			if (i == INV_NOICON && HeldItem != INV_NOICON && (ino == INV_1 || ino == INV_2)) {
				aniX += 1;				// 1 to the right
				i = InvItem(&aniX, &aniY, false);
				if (i == INV_NOICON) {
					aniX -= 1;			// 1 down
					aniY += 1;
					i = InvItem(&aniX, &aniY, false);
					if (i == INV_NOICON) {
						aniX += 1;		// 1 down-right
						i = InvItem(&aniX, &aniY, false);
					}
				}
			}

			if (ino == INV_CONV) {
				ConvAction(i);
			} else 
				InvPickup(i);
		}
		break;
	}
}

void InvAction(void) {
	int index;
	INV_OBJECT *invObj;
	int aniX, aniY;
	int i;

	GetCursorXY(&aniX, &aniY, false);

	switch (InvArea(aniX, aniY)) {
	case I_BODY:
		if (ino == INV_CONF) {
			if (!InventoryHidden)
				ConfAction(WhichInvBox(aniX, aniY, false), true);
		} else if (ino == INV_CONV) {
			index = InvItem(&aniX, &aniY, false);
			ConvAction(index);
		} else {
			index = InvItem(&aniX, &aniY, false);
			if (index != INV_NOICON) {
				if (InvD[ino].ItemOrder[index] && InvD[ino].ItemOrder[index] != HeldItem) {
					invObj = findInvObject(InvD[ino].ItemOrder[index]);
					if (invObj->hScript)
						RunInvTinselCode(invObj, ACTION, INV_ACTION, index);
				}
			}
		}
		break;

	case I_MOVE:	// Maximise/unmaximise inventory
		if (!InvD[ino].resizable)
			break;

		if (!InventoryMaximised) {
			InvD[ino].sNoofHicons = InvD[ino].NoofHicons;
			InvD[ino].sNoofVicons = InvD[ino].NoofVicons;
			InvD[ino].NoofHicons = InvD[ino].MaxHicons;
			InvD[ino].NoofVicons = InvD[ino].MaxVicons;
			InventoryMaximised = true;

			i = InvD[ino].inventoryX;
			InvD[ino].inventoryX = InvD[ino].otherX;
			InvD[ino].otherX = i;
			i = InvD[ino].inventoryY;
			InvD[ino].inventoryY = InvD[ino].otherY;
			InvD[ino].otherY = i;
		} else {
			InvD[ino].NoofHicons = InvD[ino].sNoofHicons;
			InvD[ino].NoofVicons = InvD[ino].sNoofVicons;
			InventoryMaximised = false;

			i = InvD[ino].inventoryX;
			InvD[ino].inventoryX = InvD[ino].otherX;
			InvD[ino].otherX = i;
			i = InvD[ino].inventoryY;
			InvD[ino].inventoryY = InvD[ino].otherY;
			InvD[ino].otherY = i;
		}

		// Delete current, and re-draw
		DumpDobjArray();
		DumpObjArray();
		ConstructInventory(FULL);
		break;

	case I_UP:
		InvD[ino].FirstDisp -= InvD[ino].NoofHicons;
		if (InvD[ino].FirstDisp < 0)
			InvD[ino].FirstDisp = 0;
		ItemsChanged = true;
		break;
	case I_DOWN:
		if (InvD[ino].FirstDisp + InvD[ino].NoofHicons*InvD[ino].NoofVicons < InvD[ino].NoofItems) {
			InvD[ino].FirstDisp += InvD[ino].NoofHicons;
			ItemsChanged = true;
		}
		break;
	}

}


void InvLook(void) {
	int index;
	INV_OBJECT *invObj;
	int aniX, aniY;

	GetCursorXY(&aniX, &aniY, false);

	switch (InvArea(aniX, aniY)) {
	case I_BODY:
		index = InvItem(&aniX, &aniY, false);
		if (index != INV_NOICON) {
			if (InvD[ino].ItemOrder[index] && InvD[ino].ItemOrder[index] != HeldItem) {
				invObj = findInvObject(InvD[ino].ItemOrder[index]);
				if (invObj->hScript)
					RunInvTinselCode(invObj, LOOK, INV_LOOK, index);
			}
		}
		break;

	case I_NOTIN:
		if (ino == INV_CONV)
			ConvAction(INV_CLOSEICON);
		KillInventory();
		break;
	}
}


/**************************************************************************/
/********************* Incoming events ************************************/
/**************************************************************************/


void ButtonToInventory(BUTEVENT be) {
	if (InventoryHidden)
		return;

	switch (be) {
	case INV_PICKUP:		// BE_SLEFT
		InvSLClick();
		break;

	case INV_LOOK:			// BE_SRIGHT
		if (IsConfWindow())
			InvSLClick();
		else
			InvLook();
		break;

	case INV_ACTION:		// BE_DLEFT
		if (InvDragging != ID_MDCONT)
			InvDragEnd();
		InvAction();
		break;

	case BE_LDSTART:		// Left drag start
		InvDragStart();
		break;

	case BE_LDEND:		// Left drag end
		InvDragEnd();
		break;

//	case BE_DLEFT:		// Double click left (also ends left drag)
//		ButtonToInventory(LDEND);
//		break;

	case BE_RDSTART:
	case BE_RDEND:
	case BE_UNKNOWN:
		break;
	default:
		break;
	}
}

void KeyToInventory(KEYEVENT ke) {
	int i;

	switch (ke) {
	case ESC_KEY:
		if (InventoryState == ACTIVE_INV && ino == INV_CONF && cd.Box != optionBox)
			bOpenConf = true;
		CloseInventory();
		break;

	case PGDN_KEY:
		if (ino == INV_CONF) {
			// Only act if load or save screen
			if (cd.Box != loadBox && cd.Box != saveBox)
				break;

			ConfActionSpecial(IB_SLIDE_DOWN);
		} else {
			// This code is a copy of SLClick on IB_SLIDE_DOWN
			// TODO: So share this duplicate code
			if (InvD[ino].NoofVicons == 1)
				if (InvD[ino].FirstDisp + InvD[ino].NoofHicons*InvD[ino].NoofVicons < InvD[ino].NoofItems)
					InvD[ino].FirstDisp += InvD[ino].NoofHicons;
			for (i = 1; i < InvD[ino].NoofVicons; i++) {
				if (InvD[ino].FirstDisp + InvD[ino].NoofHicons*InvD[ino].NoofVicons < InvD[ino].NoofItems)
					InvD[ino].FirstDisp += InvD[ino].NoofHicons;
			}
			ItemsChanged = true;
		}
		break;

	case PGUP_KEY:
		if (ino == INV_CONF) {
			// Only act if load or save screen
			if (cd.Box != loadBox && cd.Box != saveBox)
				break;

			ConfActionSpecial(IB_SLIDE_UP);
		} else {
			// This code is a copy of SLClick on I_SLIDE_UP
			// TODO: So share this duplicate code
			if (InvD[ino].NoofVicons == 1)
				InvD[ino].FirstDisp -= InvD[ino].NoofHicons;
			for (i = 1; i < InvD[ino].NoofVicons; i++)
				InvD[ino].FirstDisp -= InvD[ino].NoofHicons;
			if (InvD[ino].FirstDisp < 0)
				InvD[ino].FirstDisp = 0;
			ItemsChanged = true;
		}
		break;

	case HOME_KEY:
		if (ino == INV_CONF) {
			// Only act if load or save screen
			if (cd.Box != loadBox && cd.Box != saveBox)
				break;

			firstFile(0);
			AddBoxes(true);
			cd.selBox = 0;
			Select(cd.selBox, true);
		} else {
			InvD[ino].FirstDisp = 0;
			ItemsChanged = true;
		}
		break;

	case END_KEY:
		if (ino == INV_CONF) {
			// Only act if load or save screen
			if (cd.Box != loadBox && cd.Box != saveBox)
				break;

			firstFile(MAX_SFILES);	// Will get reduced to appropriate value
			AddBoxes(true);
			cd.selBox = 0;
			Select(cd.selBox, true);
		} else {
			InvD[ino].FirstDisp = InvD[ino].NoofItems - InvD[ino].NoofHicons*InvD[ino].NoofVicons;
			if (InvD[ino].FirstDisp < 0)
				InvD[ino].FirstDisp = 0;
			ItemsChanged = true;
		}
		break;

	default:
		error("We're at KeyToInventory(), with default");
	}
}

/**************************************************************************/
/************************* Odds and Ends **********************************/
/**************************************************************************/

/**
 * Called from Glitter function invdepict()
 * Changes (permanently) the animation film for that object.
 */

void invObjectFilm(int object, SCNHANDLE hFilm) {
	INV_OBJECT *invObj;

	invObj = findInvObject(object);
	invObj->hFilm = hFilm;

	if (HeldItem != object)
		ItemsChanged = true;
}

/**
 * (Un)serialize the inventory data for save/restore game.
 */

void syncInvInfo(Serializer &s) {
	for (int i = 0; i < NUM_INV; i++) {
		s.syncAsSint32LE(InvD[i].MinHicons);
		s.syncAsSint32LE(InvD[i].MinVicons);
		s.syncAsSint32LE(InvD[i].MaxHicons);
		s.syncAsSint32LE(InvD[i].MaxVicons);
		s.syncAsSint32LE(InvD[i].NoofHicons);
		s.syncAsSint32LE(InvD[i].NoofVicons);
		for (int j = 0; j < MAX_ININV; j++) {
			s.syncAsSint32LE(InvD[i].ItemOrder[j]);
		}
		s.syncAsSint32LE(InvD[i].NoofItems);
		s.syncAsSint32LE(InvD[i].FirstDisp);
		s.syncAsSint32LE(InvD[i].inventoryX);
		s.syncAsSint32LE(InvD[i].inventoryY);
		s.syncAsSint32LE(InvD[i].otherX);
		s.syncAsSint32LE(InvD[i].otherY);
		s.syncAsSint32LE(InvD[i].MaxInvObj);
		s.syncAsSint32LE(InvD[i].hInvTitle);
		s.syncAsSint32LE(InvD[i].resizable);
		s.syncAsSint32LE(InvD[i].moveable);
		s.syncAsSint32LE(InvD[i].sNoofHicons);
		s.syncAsSint32LE(InvD[i].sNoofVicons);
		s.syncAsSint32LE(InvD[i].bMax);
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
void RegisterIcons(void *cptr, int num) {
	numObjects = num;
	pio = (INV_OBJECT *) cptr;
}

/**
 * Called from Glitter function 'dec_invw()' - Declare the bits that the
 * inventory windows are constructed from, and special cursors.
 */

void setInvWinParts(SCNHANDLE hf) {
#ifdef DEBUG
	const FILM *pfilm;
#endif

	winPartsf = hf;

#ifdef DEBUG
	pfilm = (const FILM *)LockMem(hf);
	assert(FROM_LE_32(pfilm->numreels) >= HOPEDFORREELS); // not as many reels as expected
#endif
}

/**
 * Called from Glitter function 'dec_flags()' - Declare the language
 * flag films
 */

void setFlagFilms(SCNHANDLE hf) {
#ifdef DEBUG
	const FILM *pfilm;
#endif

	flagFilm = hf;

#ifdef DEBUG
	pfilm = (const FILM *)LockMem(hf);
	assert(FROM_LE_32(pfilm->numreels) >= HOPEDFORFREELS); // not as many reels as expected
#endif
}

void setConfigStrings(SCNHANDLE *tp) {
	memcpy(configStrings, tp, sizeof(configStrings));
}

/**
 * Called from Glitter functions: dec_convw()/dec_inv1()/dec_inv2()
 * - Declare the heading text and dimensions etc.
 */

void idec_inv(int num, SCNHANDLE text, int MaxContents,
		int MinWidth, int MinHeight,
		int StartWidth, int StartHeight,
		int MaxWidth, int MaxHeight,
		int startx, int starty, bool moveable) {
	if (MaxWidth > MAXHICONS)
		MaxWidth = MAXHICONS;		// Max window width
	if (MaxHeight > MAXVICONS)
		MaxHeight = MAXVICONS;		// Max window height
	if (MaxContents > MAX_ININV)
		MaxContents = MAX_ININV;	// Max contents

	if (StartWidth > MaxWidth)
		StartWidth = MaxWidth;
	if (StartHeight > MaxHeight)
		StartHeight = MaxHeight;

	InventoryState = IDLE_INV;

	InvD[num].MaxHicons = MaxWidth;
	InvD[num].MinHicons = MinWidth;
	InvD[num].MaxVicons = MaxHeight;
	InvD[num].MinVicons = MinHeight;

	InvD[num].NoofHicons = StartWidth;
	InvD[num].NoofVicons = StartHeight;

	memset(InvD[num].ItemOrder, 0, sizeof(InvD[num].ItemOrder));
	InvD[num].NoofItems = 0;

	InvD[num].FirstDisp = 0;

	InvD[num].inventoryX = startx;
	InvD[num].inventoryY = starty;
	InvD[num].otherX = 21;
	InvD[num].otherY = 15;

	InvD[num].MaxInvObj = MaxContents;

	InvD[num].hInvTitle = text;

	if (MaxWidth != MinWidth && MaxHeight != MinHeight)
		InvD[num].resizable = true;

	InvD[num].moveable = moveable;

	InvD[num].bMax = false;
}

/**
 * Called from Glitter functions: dec_convw()/dec_inv1()/dec_inv2()
 * - Declare the heading text and dimensions etc.
 */

void idec_convw(SCNHANDLE text, int MaxContents,
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

void idec_inv1(SCNHANDLE text, int MaxContents,
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

void idec_inv2(SCNHANDLE text, int MaxContents,
		int MinWidth, int MinHeight,
		int StartWidth, int StartHeight,
		int MaxWidth, int MaxHeight) {
	idec_inv(INV_2, text, MaxContents, MinWidth, MinHeight,
			StartWidth, StartHeight, MaxWidth, MaxHeight,
			100, 100, true);
}

int InvGetLimit(int invno) {
	assert(invno == INV_1 || invno == INV_2); // only INV_1 and INV_2 supported

	return InvD[invno].MaxInvObj;
}

void InvSetLimit(int invno, int MaxContents) {
	assert(invno == INV_1 || invno == INV_2); // only INV_1 and INV_2 supported
	assert(MaxContents >= InvD[invno].NoofItems); // can't reduce maximum contents below current contents

	if (MaxContents > MAX_ININV)
		MaxContents = MAX_ININV;	// Max contents

	InvD[invno].MaxInvObj = MaxContents;
}

void InvSetSize(int invno, int MinWidth, int MinHeight,
		int StartWidth, int StartHeight, int MaxWidth, int MaxHeight) {
	assert(invno == INV_1 || invno == INV_2); // only INV_1 and INV_2 supported

	if (StartWidth > MaxWidth)
		StartWidth = MaxWidth;
	if (StartHeight > MaxHeight)
		StartHeight = MaxHeight;

	InvD[invno].MaxHicons = MaxWidth;
	InvD[invno].MinHicons = MinWidth;
	InvD[invno].MaxVicons = MaxHeight;
	InvD[invno].MinVicons = MinHeight;

	InvD[invno].NoofHicons = StartWidth;
	InvD[invno].NoofVicons = StartHeight;

	if (MaxWidth != MinWidth && MaxHeight != MinHeight)
		InvD[invno].resizable = true;
	else
		InvD[invno].resizable = false;

	InvD[invno].bMax = false;
}

/**************************************************************************/

bool IsTopWindow(void) {
	return (InventoryState == BOGUS_INV);
}


bool IsConfWindow(void) {
	return (InventoryState == ACTIVE_INV && ino == INV_CONF);
}


bool IsConvWindow(void) {
	return (InventoryState == ACTIVE_INV && ino == INV_CONV);
}

} // end of namespace Tinsel
