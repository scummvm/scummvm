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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_INTRFACE_H
#define SAGA2_INTRFACE_H

#include "saga2/button.h"
#include "saga2/modal.h"
#include "saga2/player.h"

namespace Saga2 {

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

// notes: the load/unload resource functions are to be fazed out in lieu of
// the newer image cache object which will handle duplicate images without
// all the unwieldy calls

// the appfunc helper functions will eventually be merged into an object
// that will handle all of the trasitive portion of the UI.

void SetupUserControls();
void CleanupButtonImages();
void CleanupUserControls();
void SetupContainerViews(ContainerWindow *viewWindow);
void **loadButtonRes(hResContext *con, int16 resID, int16 numRes);
void **loadButtonRes(hResContext *con, int16 resID, int16 numRes, char a, char b, char c);
void **loadImageRes(hResContext *con, int16 resID, int16 numRes, char a, char b, char c);
void unloadImageRes(void **images, int16 numRes);
bool isIndivMode();

// temp >>>
uint16 getPlayerActorWeightRatio(PlayerActor *player, uint16 &maxRatio, bool bReturnMaxRatio);
uint16 getPlayerActorBulkRatio(PlayerActor *player, uint16 &maxRatio, bool bReturnMaxRatio);
template< class T > inline T GetRatio(T curUnits, T maxUnits, T ratio);
uint16 getBulkRatio(GameObject *obj, uint16 &maxRatio, bool bReturnMaxRatio);
uint16 getWeightRatio(GameObject *obj, uint16 &maxRatio, bool bReturnMaxRatio);
void GetTotalMassBulk(GameObject *obj, uint16 &totalMass, uint16 &totalBulk);

// appfunc helpers
void setIndivBtns(uint16 broNum);
void setTrioBtns();
void setCenterBtns(uint16 whichBrother);
void setControlPanelsToIndividualMode(uint16 whichBrother);
void setControlPanelsToTrioMode();
void setCenterBrother(uint16 whichBrother);
void toggleBrotherBanding(uint16 whichBrother);
uint16 translatePanID(uint16 ID);
void updateBrotherPortrait(uint16 brotherID, int16 pType);
void updateBrotherAggressionButton(uint16 brotherID, bool aggressive);
void updateBrotherBandingButton(uint16 brotherID, bool banded);
void updateBrotherRadioButtons(uint16 brotherID);
void updateReadyContainers();
void updateBrotherArmor(uint16 brotherID);

void updateAllUserControls();
void updateBrotherControls(PlayerActorID brotherID);
void enableUserControls();
void disableUserControls();

bool isBrotherDead(PlayerActorID brotherID);

// update functions
void updateIndicators();

void initUIState();
void saveUIState(Common::OutSaveFile *outS);
void loadUIState(Common::InSaveFile *in);
void cleanupUIState();

//  Varargs function to write to the status line.
void StatusMsg(const char *msg, ...);      // frametime def


const uint32    faceGroupID     = MKTAG('F', 'A', 'C', 'E');
const uint32    mentGroupID     = MKTAG('C', 'O', 'N', 'T');


/* ===================================================================== *
   Consts
 * ===================================================================== */


/* ===================================================================== *
   Exports
 * ===================================================================== */
extern uint16   indivBrother;

/* ===================================================================== *
   Class forword Declarations
 * ===================================================================== */

class CPlaqText;
class CTextWindow;
class gTextBox;
class CPortrait;

/* ===================================================================== *
   User controls
 * ===================================================================== */

// control button state structure
struct BtnCtrlState {
	bool    aggress;
	bool    jump;
	bool    center;
	bool    banding;
	int16   star;
};

// name resource structure
struct ResName {
	char    a;
	char    b;
	char    c;
	char    d;
};

enum uiBrotherName {
	kUiJulian = 0,
	kUiPhillip,
	kUiKevin,
	kUiIndiv
};

// compressed button resource indexes
const   int16   kAggressResNum       = 0;
const   int16   kJumpResNum          = 6;
const   int16   kCenterResNum        = 4;
const   int16   kBandingResNum       = 2;
const   int16   kMenConBtnResNum     = 18;
const   int16   kMassBulkResNum      = 0;
const   int16   kPieIndResNum        = 0;
const   int16   kJulBtnResNum        = 22;
const   int16   kPhiBtnResNum        = 24;
const   int16   kKevBtnResNum        = 26;
const   int16   kOptBtnResNum        = 20;



// standard number of images for push-buttons
const   int16   kNumBtnImages    = 2;

// standard number of images for portraits
const   uint16  kNumPortImages   = 8;

// number for pie indicators
const   uint16  kNumPieIndImages = 16;

// object pointers
extern CPortrait *Portrait;

/* ===================================================================== *
   Text Gadgets
 * ===================================================================== */

// plaq text writing class
class CPlaqText : public gControl {
protected:

	enum {
		kBufSize = 128
	};


	char            _lineBuf[kBufSize]; // text to render on button
	textPallete     _textFacePal;    // contains info about coloring for multi-depth text rendering
	Rect16          _textRect;       // rect for the text
	int16           _textPosition;
	gFont           *_buttonFont;    // pointer to font for this button
	gFont           *_oldFont;

public:

	CPlaqText(gPanelList &, const Rect16 &, const char *, gFont *,
	          int16, textPallete &, int16, AppFunc *cmd = NULL);

	void enable(bool);
	void invalidate(Rect16 *unused = nullptr);

	void draw();
	virtual void drawClipped(gPort &,
	                         const Point16 &,
	                         const Rect16 &);

};


/* ===================================================================== *
    CStatusLine: Status text object, child of CPlaqText
 * ===================================================================== */


// takes a string and displays it in the status for
// a length of time, and then disappears
class CStatusLine : public CPlaqText {
private:

	Alarm   _waitAlarm,
	        _minWaitAlarm;

	struct {
		char    *text;
		uint32  frameTime;
	} _lineQueue[12];

	uint8       _queueHead,
	            _queueTail;
	bool        _lineDisplayed;

	static uint8 bump(uint8 i) {
		return (i + 1) % 12;
	}

public:
	CStatusLine(gPanelList &, const Rect16 &, const char *, gFont *,
	            int16, textPallete, int32, int16, AppFunc *cmd = NULL);
	~CStatusLine();

	void setLine(char *, uint32 frameTime);
	void experationCheck();

	void clear();
};


/* ===================================================================== *
    Portrait control class
 * ===================================================================== */

enum PortraitType {
	kPortraitNormal,
	kPortraitAngry,
	kPortraitConfused,
	kPortraitOuch,
	kPortraitSick,
	kPortraitWounded,
	kPortraitAsleep,
	kPortraitDead
};

class CPortrait {
private:
	PortraitType    _currentState[kNumViews + 1];
	uint16          _numButtons;
	uint16          _numViews;
	GfxMultCompButton **_buttons;
	GfxMultCompButton *_indivButton;

	void setPortrait(uint16);

public:
	// button array, number of buttons per view, num views
	CPortrait(GfxMultCompButton *[], GfxMultCompButton *, const uint16, uint16);

	void ORset(uint16, PortraitType type);
	void set(uint16 brotherID, PortraitType type);
	PortraitType getCurrentState(uint16 brotherID) {
		return _currentState[brotherID];
	}
	void getStateString(char buf[], int8 size, uint16 brotherID);
};


/* ===================================================================== *
    CInterface: Class that handles indicators for the interface
 * ===================================================================== */

class CMassWeightIndicator {
private:
	GameObject *_containerObject;

public:
	static  bool _bRedraw;

private:
	enum {
		// background image coords
		kBackImageXSize  = 88,
		kBackImageYSize  = 43,

		// pie image coords
		kMassPieXOffset  = 8,
		kMassPieYOffset  = 9,
		kBulkPieXOffset  = 53,
		kBulkPieYOffset  = 9,
		kPieXSize        = 28,
		kPieYSize        = 26
	};

	// xy positions of this indicator
	Point16 _backImagePos;
	Point16 _massPiePos;
	Point16 _bulkPiePos;

	// memory for update
	uint16 _currentMass;
	uint16 _currentBulk;

	// resource context pointer
	hResContext *_containerRes;

	// indicator images
	void *_massBulkImag;

	// array of pointers to images
	void **_pieIndImag;

	// image control buttons
	GfxCompImage          *_pieMass;
	GfxCompImage          *_pieBulk;


public:
	void invalidate(Rect16 *unused = nullptr) {
		_pieMass->invalidate();
		_pieBulk->invalidate();
	}

	CMassWeightIndicator(gPanelList *, const Point16 &, uint16 type = 1, bool death = false);
	~CMassWeightIndicator();

	uint16 getMassPieDiv() {
		return _pieMass->getMax();
	}
	uint16 getBulkPieDiv() {
		return _pieBulk->getMax();
	}

	void setMassPie(uint16 val) {
		_pieMass->setCurrent(val);
	}
	void setBulkPie(uint16 val) {
		_pieBulk->setCurrent(val);
	}
	void recalculate();
	static void update();
};


class CManaIndicator : public GfxCompImage {
public:

	// sizes of the mana star images
	enum startSize {
		kStar1XSize = 10,
		kStar1YSize = 9,
		kStar2XSize = 16,
		kStar2YSize = 15,
		kStar3XSize = 20,
		kStar3YSize = 19,
		kStar4XSize = 28,
		kStar4YSize = 27,
		kStar5XSize = 32,
		kStar5YSize = 31,
		kStar6XSize = 36,
		kStar6YSize = 35,
		kStar7XSize = 46,
		kStar7YSize = 45
	};

	// sizes of the rings
	enum ringSize {
		kRing1XSize = 8,
		kRing1YSize = 7,
		kRing2XSize = 12,
		kRing2YSize = 11,
		kRing3XSize = 16,
		kRing3YSize = 15,
		kRing4XSize = 22,
		kRing4YSize = 21,
		kRing5XSize = 26,
		kRing5YSize = 25,
		kRing6XSize = 32,
		kRing6YSize = 31,
		kRing7XSize = 40,
		kRing7YSize = 39
	};

	// area of control
	enum area {
		kAreaX       = 475,
		kAreaY       = 315,
		kAreaXSize   = 152,
		kAreaYSize   = 135
	};

	// coordinates
	enum {
		kCenterX     = kAreaX + kAreaXSize / 2,
		kCenterY     = kAreaY + kAreaYSize / 2,
		kWellXSize   = 108,
		kWellYSize   = 123,
		kWellX       = (kAreaXSize / 2 - kWellXSize / 2) + 1,
		kWellY       = kAreaYSize / 2 - kWellYSize / 2
	};

	// manas end points

	enum manaEndCoordsBase { // based on quadrants
		kManaXOffset = 9,
		kManaYOffset = -10,

		kManaUpperLeftX  = 16    + kManaXOffset,
		kManaUpperLeftY  = 45    + kManaYOffset,
		kManaUpperMidX   = 70    + kManaXOffset,
		kManaUpperMidY   = 13    + kManaYOffset,
		kManaUpperRightX = 122   + kManaXOffset,
		kManaUpperRightY = 45    + kManaYOffset,
		kManaLowerLeftX  = 16    + kManaXOffset,
		kManaLowerLeftY  = 106   + kManaYOffset,
		kManaLowerMidX   = 70    + kManaXOffset,
		kManaLowerMidY   = 135   + kManaYOffset,
		kManaLowerRightX = 122   + kManaXOffset,
		kManaLowerRightY = 106   + kManaYOffset
	};

	enum manaEndCoords {
		kManaRedEndX         = kManaUpperLeftX,   //a // these are just dup tags
		kManaRedEndY         = kManaUpperLeftY,   //b
		kManaOrangeEndX      = kManaUpperMidX,    //c
		kManaOrangeEndY      = kManaUpperMidY,
		kManaYellowEndX      = kManaUpperRightX,  //d
		kManaYellowEndY      = kManaUpperRightY,  //b
		kManaGreenEndX       = kManaLowerLeftX,   //a
		kManaGreenEndY       = kManaLowerLeftY,   //e
		kManaBlueEndX        = kManaLowerMidX,    //c
		kManaBlueEndY        = kManaLowerMidY,
		kManaVioletEndX      = kManaLowerRightX,  //d
		kManaVioletEndY      = kManaLowerRightY,  //e
		kManaNumXYCoords     = 12,
		kManaNumManaTypes    = kManaNumXYCoords / 2
	};

	// mana star display offset from well center
	enum startOffset {
		kManaStartOffset = 12
	};

	// mana information
	enum manainfo {
		kManaMaxLevel = 200,
		kManaNumManaRegions = kManaNumManaTypes
	};

	// resource data
	enum resourceInfo {
		kResNumStars        = 7,
		kResNumRings        = 7,
		kResStarResNum      = 0,
		kResRingResNum      = 0,
		kResNumManaColors   = 13
	};

	// this describes a star
	// ... via image number and position on screen
	struct manaLineInfo {
		Point16 starPos;
		Point16 ringPos;
		uint8   starImageIndex;
		uint8   ringImageIndex;
	};

private:

	// resource handle
	hResContext     *_resContext;

	// array of image pointers
	void            **_starImages;
	void            **_ringImages;

	// background image pointer
	void            *_backImage;
	void            *_wellImage;

	// image maps
	gPixelMap   _savedMap;

	// array of manaLine infos for blitting
	manaLineInfo _manaLines[kManaNumManaTypes];

	// array of ring and star end positions
	// this is initialized via constructor
	Point16 _starRingEndPos[kManaNumManaTypes];

	Point16 _starSizes[kResNumStars];
	Point16 _ringSizes[kResNumRings];

	// these are checks against redundent updates
	int32   _currentMana[kManaNumManaTypes], _currentBaseMana[kManaNumManaTypes];
protected:

	// these do line and position calculations
	int16   bresLine(Point16 from, Point16 to);
	Point16 bresLine(Point16 from, Point16 to, int16 steps);

	// this gets the star/ring positions and image indexes
	void getManaLineInfo(uint16 index, int16 manaAmount, int16 baseManaAmount, manaLineInfo *info);

public:
	CManaIndicator(gPanelList &list);
	~CManaIndicator();

	// this updates the star and ring position info
	bool update(PlayerActor *player);

	// checks to see if the mana indicator needs to draw
	bool needUpdate(PlayerActor *player);

	// this method provides a rect for any of the six mana regions of the control
	// region numbers:
	//  -------
	//  |0 1 2|
	//  |3 4 5|
	//  -------
	Rect16  getManaRegionRect(int8 region);
	int     getNumManaRegions() {
		return kManaNumManaRegions;
	}

	// drawing routines
	void draw();
	virtual void drawClipped(gPort &, const Point16 &, const Rect16 &);
};

/* ===================================================================== *
    CHealthIndicator: Health star indicator
 * ===================================================================== */

class CHealthIndicator {
private:

	static const char *_hintText;

	enum {
		kHealthStarFrameResNum     = 14,
		kHealthStarFrameNum        = 1,
		kHealthStarStart           = 0,
		kHealthStarNum             = 23,
		kHealthStarInitial         = 0,
		kHealthStarLevels          = 24,
		kHealthNumControls         = kNumViews
	};

	enum {
		kHealthStarXPos    = 572,
		kHealthStarYPos    = 21,
		kHealthStarXSize   = 32,
		kHealthStarYSize   = 32,
		kHealthStarYOffset = 150,
		kHealthFrameXPos   = 571,
		kHealthFrameYPos   = 20,
		kHealthFrameXSize  = 32,
		kHealthFrameYSize  = 32
	};

	// resource handle
	hResContext *_healthRes;

	// buttons
	GfxCompImage          *_starBtns[kHealthNumControls];
	GfxCompImage          *_indivStarBtn;

	// array of pointer to the star imagery
	void **_starImag;

	// health star frame imagery
	void *_starFrameImag;

	void updateStar(GfxCompImage *starCtl, int32 bro, int32 baseVitality, int32 curVitality);

public:
	uint16  _starIDs[3];
	int16   _imageIndexMemory[4];

public:

	CHealthIndicator(AppFunc *cmd);
	~CHealthIndicator();


	void update();
};


/* ===================================================================== *
    Global classes
 * ===================================================================== */

extern CStatusLine              *StatusLine;
extern CMassWeightIndicator     *MassWeightIndicator;
extern CHealthIndicator         *HealthIndicator;
extern CManaIndicator           *ManaIndicator;

} // end of namespace Saga2

#endif
