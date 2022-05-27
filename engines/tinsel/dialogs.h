
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
 * Inventory related functions
 */

#ifndef TINSEL_INVENTORY_H	// prevent multiple includes
#define TINSEL_INVENTORY_H

#include "tinsel/dw.h"
#include "tinsel/events.h"	// for PLR_EVENT, PLR_EVENT
#include "tinsel/inv_objects.h"
#include "tinsel/object.h"
#include "tinsel/movers.h"

namespace Common {
class Serializer;
struct KeyState;
}

namespace Tinsel {

struct OBJECT;
struct FREEL;
struct FILM;
struct CONFINIT;

enum {
	INV_OPEN 		= -1, // DW1 only
	INV_CONV 		= 0,
	INV_1 			= 1,
	INV_2 			= 2,
	DW0_INV_CONF	= 3,
	INV_MENU 		= 3, // DW2 constant
	NUM_INV_V0 		= 4,

	// Discworld 2 constants
	DW2_INV_OPEN	= 5,
	INV_DEFAULT  	= 6,

	// Noir constants
	INV_3		 	= 3,
	INV_4 		 	= 4,
	NOIR_INV_CONF   = 5,
	NUM_INV_V3 	 	= 6,
	INV_7NOINV 	 	= 7,
	INV_8NOINV 	 	= 8,
	INV_NOTEBOOK 	= 9,

	MAX_NUM_INV 	= NUM_INV_V3 // For determination of _invD array size
};

#define INV_CONF ((TinselVersion == 3) ? NOIR_INV_CONF : DW0_INV_CONF)
#define NUM_INV ((TinselVersion == 3) ? NUM_INV_V3 : NUM_INV_V0)

enum {
	NOOBJECT = -1,
	INV_NOICON_V0 = -1,
	INV_CLOSEICON = -2,
	INV_OPENICON = -3,
	INV_HELDNOTIN_V0 = -4,
	// Noir discerns between NOOBJECT and INV_NOICON
	INV_NOICON_V3 = 0,
	INV_HELDNOTIN_V3 = 1,
	INV_HELDIN = 2,
};

#define INV_NOICON ((TinselVersion == 3) ? INV_NOICON_V3 : INV_NOICON_V0)
#define INV_HELDNOTIN ((TinselVersion == 3) ? INV_HELDNOTIN_V3 : INV_HELDNOTIN_V0)

enum CONV_PARAM {
	CONV_DEF,
	CONV_BOTTOM,
	CONV_END,
	CONV_TOP
};

enum InventoryType { EMPTY,
	                 FULL,
	                 CONF };

enum InvCursorFN { IC_AREA,
	               IC_DROP };

#define sliderRange (_sliderYmax - _sliderYmin)
#define MAXSLIDES 4
#define MAX_PERMICONS 10 // Max permanent conversation icons
#define MAXHICONS 10     // Max dimensions of
#define MAXVICONS 6      // an inventory window
#define SG_DESC_LEN 40   // Max. saved game description length

// Number of objects that makes up an empty window
#define MAX_WCOMP_T0 21 // 4 corners + (3+3) sides + (2+2) extra sides
						// + Bground + title + slider
	                    // + more Needed for save game window
#define MAX_WCOMP_T3 84
#define MAX_WCOMP (TinselVersion == 3 ? MAX_WCOMP_T3 : MAX_WCOMP_T0)

#define MAX_ICONS MAXHICONS *MAXVICONS
#define MAX_ININV_TOT 160

enum CONFTYPE {
	MAIN_MENU,
	SAVE_MENU,
	LOAD_MENU,
	QUIT_MENU,
	RESTART_MENU,
	SOUND_MENU,
	CONTROLS_MENU,
	SUBTITLES_MENU,
	HOPPER_MENU1,
	HOPPER_MENU2,
	TOP_WINDOW
};

struct INV_DEF {
	int MinHicons; // }
	int MinVicons; // } Dimension limits
	int MaxHicons; // }
	int MaxVicons; // }

	int NoofHicons; // }
	int NoofVicons; // } Current dimentsions

	int contents[160]; // Contained items
	int NoofItems;     // Current number of held items

	int FirstDisp; // Index to first item currently displayed

	int inventoryX; // } Display position
	int inventoryY; // }
	int otherX;     // } Display position
	int otherY;     // }

	int MaxInvObj; // Max. allowed contents

	SCNHANDLE hInvTitle; // Window heading

	bool resizable; // Re-sizable window?
	bool bMoveable; // Moveable window?

	int sNoofHicons; // }
	int sNoofVicons; // } Current dimensions

	bool bMax; // Maximised last time open?
};

//----- Data pertinant to scene hoppers ------------------------

struct HOPPER {
	uint32		hScene;
	SCNHANDLE	hSceneDesc;
	uint32		numEntries;
	uint32		entryIndex;
};

struct HOPENTRY {
	uint32	eNumber;	// entrance number
	SCNHANDLE hDesc;	// handle to entrance description
	uint32	flags;
};

enum BTYPE {
	RGROUP, ///< Radio button group - 1 is selectable at a time. Action on double click
	ARSBUT, ///< Action if a radio button is selected
	AABUT,  ///< Action always
	AATBUT, ///< Action always, text box
	ARSGBUT,
	AAGBUT,  ///< Action always, graphic button
	SLIDER,  ///< Not a button at all
	TOGGLE,  ///< Discworld 1 toggle
	TOGGLE1, ///< Discworld 2 toggle type 1
	TOGGLE2, ///< Discworld 2 toggle type 2
	DCTEST,
	FLIP,
	FRGROUP,
	ROTATE,
	NOTHING
};

enum BFUNC {
	NOFUNC,
	SAVEGAME,
	LOADGAME,
	IQUITGAME,
	CLOSEWIN,
	OPENLOAD,
	OPENSAVE,
	OPENREST,
	OPENSOUND,
	OPENCONT,
#ifndef JAPAN
	OPENSUBT,
#endif
	OPENQUIT,
	INITGAME,
	MUSICVOL,

	HOPPER2, // Call up Scene Hopper 2
	BF_CHANGESCENE,

	CLANG,
	RLANG
#ifdef MAC_OPTIONS
	,
	MASTERVOL,
	SAMPVOL
#endif
};

enum TM { TM_POINTER,
	      TM_INDEX,
	      TM_STRINGNUM,
	      TM_UNK4,
	      TM_NONE };

// For slideSlider() and similar
enum SSFN {
	S_START,
	S_SLIDE,
	S_END,
	S_TIMEUP,
	S_TIMEDN
};

struct CONFBOX {
	BTYPE boxType;
	BFUNC boxFunc;
	TM textMethod;

	char *boxText;
	int ixText;
	int xpos;
	int ypos;
	int w; // Doubles as max value for SLIDERs
	int h; // Doubles as iteration size for SLIDERs
	int *ival;
	int bi; // Base index for AAGBUTs
};

// Data for button press/toggle effects
struct BUTTONEFFECT {
	bool bButAnim;
	CONFBOX *box;
	bool press; // true = button press; false = button toggle
};

enum class SysReel;

class Dialogs {
public:
	Dialogs();
	virtual ~Dialogs();

	void popUpInventory(int invno, int menuId = -1);
	void openMenu(CONFTYPE menuType);

	void xMovement(int x);
	void yMovement(int y);

	void eventToInventory(PLR_EVENT pEvent, const Common::Point &coOrds);

	int whichItemHeld();

	void holdItem(int item, bool bKeepFilm = false);
	void dropItem(int item);
	void clearInventory(int invno);
	void addToInventory(int invno, int icon, bool hold = false);
	bool remFromInventory(int invno, int icon);

	void registerIcons(void *cptr, int num);

	void idec_convw(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
	                int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
	void idec_inv1(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
	               int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
	void idec_inv2(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
	               int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

	// Noir
	Common::Array<int> getAllNotebookClues() const;
	void idec_invMain(SCNHANDLE text, int MaxContents);

	bool inventoryActive();
	bool inventoryOrNotebookActive();

	void permaConvIcon(int icon, bool bEnd = false);

	void convPos(int bpos);
	void convPoly(HPOLYGON hPoly);
	int getIcon();
	void closeDownConv();
	void hideConversation(bool hide);
	bool convIsHidden();

	void convAction(int index);
	void setConvDetails(CONV_PARAM fn, HPOLYGON hPoly, int ano);
	void inventoryIconCursor(bool bNewItem);

	void setInvWinParts(SCNHANDLE hf);
	void setFlagFilms(SCNHANDLE hf);
	void setConfigStrings(SCNHANDLE *tp);

	int invItem(Common::Point &coOrds, bool update);
	int invItem(int *x, int *y, bool update);
	int invItemId(int x, int y);

	int inventoryPos(int num);

	bool isInInventory(int object, int invnum);

	void killInventory();

	void syncInvInfo(Common::Serializer &s);

	int invGetLimit(int invno);
	void invSetLimit(int invno, int MaxContents);
	void invSetSize(int invno, int MinWidth, int MinHeight,
	                int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

	bool getIsInvObject(int id);

	int whichInventoryOpen();

	bool isTopWindow();
	bool menuActive();
	bool isConvWindow();

	const FILM *getObjectFilm(int object) const;
	void setObjectFilm(int object, SCNHANDLE hFilm);
	void callFunction(BFUNC boxFunc);

	OBJECT *addObject(const FREEL *pfreel, int num);
	void invPutDown(int index);
	void slideMSlider(int x, SSFN fn);
	void addBoxes(bool posnSlide);
	void select(int i, bool force);
	void fillInInventory();
	void invCursor(InvCursorFN fn, int CurX, int CurY);
	const InventoryObject *getInvObject(int id);
	const InventoryObjectT3 *getInvObjectT3(int id);
	void invPointEvent(const InventoryObject *invObj, int index);
	bool updateString(const Common::KeyState &kbd);
	bool inventoryIsActive() { return _inventoryState == ACTIVE_INV; }
	bool isMixingDeskControl() { return _invDragging == ID_MDCONT; }
	int currentInventoryX() { return _invD[_activeInv].inventoryX; }
	int currentInventoryY() { return _invD[_activeInv].inventoryY; }
	bool configurationIsActive() { return _activeInv == INV_CONF; }
	bool displayObjectsActive() { return _objArray[0] != NULL; }
	bool inventoryIsHidden() { return _InventoryHidden; }
	const FILM *getWindowData();
	void redraw();

	// Noir
	bool IsConvAndNotMove();

	bool _noLanguage;
	int _glitterIndex;
	volatile int _pointedWaitCount; // used by ObjectProcess - fix the 'repeated pressing bug'
	// For editing save game names
	char _saveGameDesc[SG_DESC_LEN + 2];

	OBJECT *_iconArray[MAX_ICONS]; // Current display objects (icons)
	BUTTONEFFECT _buttonEffect;

private:
	int whichMenuBox(int curX, int curY, bool bSlides);
	void confActionSpecial(int i);
	bool rePosition();
	bool languageChange();
	void primeSceneHopper();
	void freeSceneHopper();
	void rememberChosenScene();
	void setChosenScene();
	void firstEntry(int first);
	void hopAction();
	void dumpIconArray();
	void dumpDobjArray();
	void dumpObjArray();
	void firstScene(int first);
	void firstFile(int first);
	int getObjectIndex(int id) const;
	void invSaveGame();
	void invLoadGame();
	int invArea(int x, int y);
	void invBoxes(bool InBody, int curX, int curY);
	void invLabels(bool InBody, int aniX, int aniY);
	void adjustTop();
	OBJECT *addInvObject(int num, const FREEL **pfreel, const FILM **pfilm);
	void addBackground(OBJECT **rect, const Common::Rect &bounds, OBJECT **title = nullptr, int textFrom = 0);
	void addTitle(OBJECT **title, const Common::Rect &rect);
	void addSlider(OBJECT **slide, const FILM *pfilm);
	void addBox(int *pi, const int i);
	void addEWSlider(OBJECT **slide, const FILM *pfilm);
	void positionInventory(OBJECT *pMultiObj, int xOffset, int yOffset, int zPosition);
	int addExtraWindow(int x, int y, OBJECT **retObj);
	void constructInventoryCommon(SysReel reel, bool hasTitle);
	void constructConversationInventory();
	void constructInventory(InventoryType filling);
	void constructOtherInventory(int menuId);
	void constructMainInventory();
	void alterCursor(int num);
	void setMenuGlobals(CONFINIT *ci);
	void closeInventory();
	int nearestSlideY(int fity);
	void slideSlider(int y, SSFN fn);
	void slideCSlider(int y, SSFN fn);
	void gettingTaller();
	void gettingShorter();
	void gettingWider();
	void gettingNarrower();
	void changeingSize();
	void invDragStart();
	void invDragEnd();
	bool menuDown(int lines);
	bool menuUp(int lines);
	void menuRollDown();
	void menuRollUp();
	void menuPageDown();
	void menuPageUp();
	void inventoryDown();
	void inventoryUp();
	void menuAction(int i, bool dbl);
	void invPickup(int index);
	void invWalkTo(const Common::Point &coOrds);
	void invAction();
	void invLook(const Common::Point &coOrds);
	void idec_inv(int num, SCNHANDLE text, int MaxContents,
	              int MinWidth, int MinHeight,
	              int StartWidth, int StartHeight,
	              int MaxWidth, int MaxHeight,
	              int startx, int starty, bool moveable);

	//----- Permanent data (set once) -----
	SCNHANDLE _flagFilm;  // Window members and cursors' graphic data
	SCNHANDLE _configStrings[20];

	INV_DEF _invD[MAX_NUM_INV];        // Conversation + 2 inventories + ...
	int _activeInv;                      // Which inventory is currently active
	InventoryObjects *_invObjects; // Inventory objects' data
	SCNHANDLE *_invFilms;
	DIRECTION _initialDirection;

	//----- Permanent data (updated, valid while inventory closed) -----
	int _heldItem; // Current held item

	SCNHANDLE _heldFilm;
	SCNHANDLE _hWinParts; // Window members and cursors' graphic data

	// Permanent contents of conversation inventory
	int _permIcons[MAX_PERMICONS]; // Basic items i.e. permanent contents
	int _numPermIcons;         // - copy to conv. inventory at pop-up time
	int _numEndIcons;

	//----- Data pertinant to current active inventory -----

	bool _InventoryHidden;
	bool _InventoryMaximised;
	bool _ItemsChanged; // When set, causes items to be re-drawn

	int _SuppH; // 'Linear' element of
	int _SuppV; // dimensions during re-sizing

	int _yChange;     //
	int _yCompensate; // All to do with re-sizing.
	int _xChange;     //
	int _xCompensate; //

	bool _reOpenMenu;

	int _TL, _TR, _BL, _BR; // Used during window construction
	int _TLwidth, _TLheight;          //
	int _TRwidth;                          //
	int _BLheight;                         //

	LANGUAGE _displayedLanguage;

	OBJECT *_objArray[MAX_WCOMP_T3];  // Current display objects (window)
	OBJECT *_dispObjArray[MAX_WCOMP_T3]; // Current display objects (re-sizing window)
	ANIM _iconAnims[MAX_ICONS];

	OBJECT *_rectObject, *_slideObject; // Current display objects, for reference
	                                                     // objects are in objArray.

	int _sliderYpos;                   // For positioning the slider
	int _sliderYmax, _sliderYmin; //

	// Also to do with the slider
	struct {
		int n;
		int y;
	} _slideStuff[MAX_ININV_TOT + 1];

	struct MDSLIDES {
		int num;
		OBJECT *obj;
		int min, max;
	};
	MDSLIDES _mdSlides[MAXSLIDES];
	int _numMdSlides;

	// Icon clicked on to cause an event
	// - Passed to conversation polygon or actor code via Topic()
	// - (sometimes) Passed to inventory icon code via OtherObject()
	int _thisIcon;

	CONV_PARAM _thisConvFn;             // Top, 'Middle' or Bottom
	HPOLYGON _thisConvPoly;         // Conversation code is in a polygon code block
	int _thisConvActor;                 // ...or an actor's code block.
	int _pointedIcon;      // used by invLabels - icon pointed to on last call
	int _sX;                        // used by slideMSlider() - current x-coordinate
	int _lX;                        // used by slideMSlider() - last x-coordinate

	bool _bMoveOnUnHide; // Set before start of conversation
	    // - causes conversation to be started in a sensible place

	HOPPER *_pHopper;
	HOPENTRY *_pEntries;
	int _numScenes;

	int _numEntries;

	HOPPER *_pChosenScene;

	int _lastChosenScene;
	bool _bRemember;
	enum { IC_NORMAL,
		          IC_DR,
		          IC_UR,
		          IC_TB,
		          IC_LR,
		          IC_INV,
		          IC_UP,
		          IC_DN } _invCursor;

	enum { NO_INV,
		   IDLE_INV,
		   ACTIVE_INV,
		   BOGUS_INV } _inventoryState;

	enum { ID_NONE,
		   ID_MOVE,
		   ID_SLIDE,
		   ID_BOTTOM,
		   ID_TOP,
		   ID_LEFT,
		   ID_RIGHT,
		   ID_TLEFT,
		   ID_TRIGHT,
		   ID_BLEFT,
		   ID_BRIGHT,
		   ID_CSLIDE,
		   ID_MDCONT } _invDragging;
};

void ObjectEvent(CORO_PARAM, int objId, TINSEL_EVENT event, bool bWait, int myEscape, bool *result = NULL);

} // End of namespace Tinsel

#endif /* TINSEL_INVENTRY_H */
