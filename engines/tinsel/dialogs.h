
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
 * Inventory related functions
 */

#ifndef TINSEL_INVENTORY_H	// prevent multiple includes
#define TINSEL_INVENTORY_H

#include "tinsel/dw.h"
#include "tinsel/events.h"	// for PLR_EVENT, PLR_EVENT
#include "tinsel/object.h"
#include "tinsel/rince.h"

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
	INV_OPEN	= -1,	// DW1 only
	INV_CONV	= 0,
	INV_1		= 1,
	INV_2		= 2,
	INV_CONF	= 3,
	INV_MENU	= 3,	// DW2 constant
	NUM_INV		= 4,

	// Discworld 2 constants
	DW2_INV_OPEN = 5,
	INV_DEFAULT = 6
};

enum {
	NOOBJECT = -1,
	INV_NOICON = -1,
	INV_CLOSEICON = -2,
	INV_OPENICON = -3,
	INV_HELDNOTIN = -4
};

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

// attribute values - not a bit bit field to prevent portability problems
#define DROPCODE 0x01
#define ONLYINV1 0x02
#define ONLYINV2 0x04
#define DEFINV1 0x08
#define DEFINV2 0x10
#define PERMACONV 0x20
#define CONVENDITEM 0x40
#define sliderRange (_sliderYmax - _sliderYmin)
#define MAXSLIDES 4
#define MAX_PERMICONS 10 // Max permanent conversation icons
#define MAXHICONS 10     // Max dimensions of
#define MAXVICONS 6      // an inventory window
#define SG_DESC_LEN 40   // Max. saved game description length

// Number of objects that makes up an empty window
#define MAX_WCOMP 21 // 4 corners + (3+3) sides + (2+2) extra sides
	                 // + Bground + title + slider
	                 // + more Needed for save game window

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

/** structure of each inventory object */
struct INV_OBJECT {
	int32 id;		// inventory objects id
	SCNHANDLE hIconFilm;	// inventory objects animation film
	SCNHANDLE hScript;	// inventory objects event handling script
	int32 attribute;		// inventory object's attribute
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

#include "common/pack-start.h"	// START STRUCT PACKING

struct HOPPER {
	uint32		hScene;
	SCNHANDLE	hSceneDesc;
	uint32		numEntries;
	uint32		entryIndex;
} PACKED_STRUCT;
typedef HOPPER *PHOPPER;

struct HOPENTRY {
	uint32	eNumber;	// entrance number
	SCNHANDLE hDesc;	// handle to entrance description
	uint32	flags;
} PACKED_STRUCT;
typedef HOPENTRY *PHOPENTRY;

#include "common/pack-end.h"	// END STRUCT PACKING

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
	      TM_NONE };

// For SlideSlider() and similar
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

class Dialogs {
public:
	Dialogs();
	virtual ~Dialogs();

	void PopUpInventory(int invno);
	void OpenMenu(CONFTYPE type);

	void Xmovement(int x);
	void Ymovement(int y);

	void EventToInventory(PLR_EVENT pEvent, const Common::Point &coOrds);

	int WhichItemHeld();

	void HoldItem(int item, bool bKeepFilm = false);
	void DropItem(int item);
	void ClearInventory(int invno);
	void AddToInventory(int invno, int icon, bool hold = false);
	bool RemFromInventory(int invno, int icon);

	void RegisterIcons(void *cptr, int num);

	void idec_convw(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
	                int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
	void idec_inv1(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
	               int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
	void idec_inv2(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
	               int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

	bool InventoryActive();

	void PermaConvIcon(int icon, bool bEnd = false);

	void convPos(int bpos);
	void ConvPoly(HPOLYGON hp);
	int GetIcon();
	void CloseDownConv();
	void HideConversation(bool hide);
	bool ConvIsHidden();

	void ConvAction(int index);
	void SetConvDetails(CONV_PARAM fn, HPOLYGON hPoly, int ano);
	void InventoryIconCursor(bool bNewItem);

	void setInvWinParts(SCNHANDLE hf);
	void setFlagFilms(SCNHANDLE hf);
	void setConfigStrings(SCNHANDLE *tp);

	int InvItem(Common::Point &coOrds, bool update);
	int InvItem(int *x, int *y, bool update);
	int InvItemId(int x, int y);

	int InventoryPos(int num);

	bool IsInInventory(int object, int invnum);

	void KillInventory();

	void syncInvInfo(Common::Serializer &s);

	int InvGetLimit(int invno);
	void InvSetLimit(int invno, int n);
	void InvSetSize(int invno, int MinWidth, int MinHeight,
	                int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

	bool GetIsInvObject(int id);

	int WhichInventoryOpen();

	bool IsTopWindow();
	bool MenuActive();
	bool IsConvWindow();

	void SetObjectFilm(int object, SCNHANDLE hFilm);
	void CallFunction(BFUNC boxFunc);

	OBJECT *AddObject(const FREEL *pfreel, int num);
	void InvPutDown(int index);
	void SlideMSlider(int x, SSFN fn);
	void AddBoxes(bool posnSlide);
	void Select(int i, bool force);
	void FillInInventory();
	void InvCursor(InvCursorFN fn, int CurX, int CurY);
	INV_OBJECT *GetInvObject(int id);
	bool UpdateString(const Common::KeyState &kbd);
	bool InventoryIsActive() { return _inventoryState == ACTIVE_INV; }
	bool IsMixingDeskControl() { return _invDragging == ID_MDCONT; }
	int CurrentInventoryX() { return _invD[_activeInv].inventoryX; }
	int CurrentInventoryY() { return _invD[_activeInv].inventoryY; }
	bool ConfigurationIsActive() { return _activeInv == INV_CONF; }
	bool DisplayObjectsActive() { return _objArray[0] != NULL; }
	bool InventoryIsHidden() { return _InventoryHidden; }
	const FILM *GetWindowData();
	void Redraw();

	bool _noLanguage;
	int _glitterIndex;
	volatile int _pointedWaitCount; // used by ObjectProcess - fix the 'repeated pressing bug'
	// For editing save game names
	char _saveGameDesc[SG_DESC_LEN + 2];

	OBJECT *_iconArray[MAX_ICONS]; // Current display objects (icons)
	BUTTONEFFECT _buttonEffect;

private:
	int WhichMenuBox(int curX, int curY, bool bSlides);
	void ConfActionSpecial(int i);
	bool RePosition();
	bool LanguageChange();
	void PrimeSceneHopper();
	void FreeSceneHopper();
	void RememberChosenScene();
	void SetChosenScene();
	void FirstEntry(int first);
	void HopAction();
	void DumpIconArray();
	void DumpDobjArray();
	void DumpObjArray();
	void FirstScene(int first);
	void FirstFile(int first);
	int GetObjectIndex(int id);
	void InvSaveGame();
	void InvLoadGame();
	int InvArea(int x, int y);
	void InvBoxes(bool InBody, int curX, int curY);
	void InvLabels(bool InBody, int aniX, int aniY);
	void AdjustTop();
	OBJECT *AddInvObject(int num, const FREEL **pfreel, const FILM **pfilm);
	void AddBackground(OBJECT **rect, OBJECT **title, int extraH, int extraV, int textFrom);
	void AddBackground(OBJECT **rect, int extraH, int extraV);
	void AddTitle(POBJECT *title, int extraH);
	void AddSlider(OBJECT **slide, const FILM *pfilm);
	void AddBox(int *pi, const int i);
	void AddEWSlider(OBJECT **slide, const FILM *pfilm);
	int AddExtraWindow(int x, int y, OBJECT **retObj);
	void ConstructInventory(InventoryType filling);
	void AlterCursor(int num);
	void SetMenuGlobals(CONFINIT *ci);
	void CloseInventory();
	int NearestSlideY(int fity);
	void SlideSlider(int y, SSFN fn);
	void SlideCSlider(int y, SSFN fn);
	void GettingTaller();
	void GettingShorter();
	void GettingWider();
	void GettingNarrower();
	void ChangeingSize();
	void InvDragStart();
	void InvDragEnd();
	bool MenuDown(int lines);
	bool MenuUp(int lines);
	void MenuRollDown();
	void MenuRollUp();
	void MenuPageDown();
	void MenuPageUp();
	void InventoryDown();
	void InventoryUp();
	void MenuAction(int i, bool dbl);
	void InvPickup(int index);
	void InvWalkTo(const Common::Point &coOrds);
	void InvAction();
	void InvLook(const Common::Point &coOrds);
	void idec_inv(int num, SCNHANDLE text, int MaxContents,
	              int MinWidth, int MinHeight,
	              int StartWidth, int StartHeight,
	              int MaxWidth, int MaxHeight,
	              int startx, int starty, bool moveable);

	//----- Permanent data (set once) -----
	SCNHANDLE _flagFilm;  // Window members and cursors' graphic data
	SCNHANDLE _configStrings[20];

	INV_DEF _invD[NUM_INV];        // Conversation + 2 inventories + ...
	int _activeInv;                      // Which inventory is currently active
	INV_OBJECT *_invObjects; // Inventory objects' data
	int _numObjects;               // Number of inventory objects
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

	OBJECT *_objArray[MAX_WCOMP];  // Current display objects (window)
	OBJECT *_dispObjArray[MAX_WCOMP]; // Current display objects (re-sizing window)
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
	int _pointedIcon;      // used by InvLabels - icon pointed to on last call
	int _sX;                        // used by SlideMSlider() - current x-coordinate
	int _lX;                        // used by SlideMSlider() - last x-coordinate

	bool _bMoveOnUnHide; // Set before start of conversation
	    // - causes conversation to be started in a sensible place

	PHOPPER _pHopper;
	PHOPENTRY _pEntries;
	int _numScenes;

	int _numEntries;

	PHOPPER _pChosenScene;

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
