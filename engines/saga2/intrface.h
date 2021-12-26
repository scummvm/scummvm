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
// all the unwieldly calls

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
	uiJulian = 0,
	uiPhillip,
	uiKevin,
	uiIndiv
};

// compressed button resource indexes
const   int16   aggressResNum       = 0;
const   int16   jumpResNum          = 6;
const   int16   centerResNum        = 4;
const   int16   bandingResNum       = 2;
const   int16   menConBtnResNum     = 18;
const   int16   massBulkResNum      = 0;
const   int16   pieIndResNum        = 0;
const   int16   julBtnResNum        = 22;
const   int16   phiBtnResNum        = 24;
const   int16   kevBtnResNum        = 26;
const   int16   optBtnResNum        = 20;



// standard number of images for push-buttons
const   int16   numBtnImages    = 2;

// standard number of images for portraits
const   uint16  numPortImages   = 8;

// number for pie indicators
const   uint16  numPieIndImages = 16;

// object pointers
extern CPortrait *Portrait;

/* ===================================================================== *
   Text Gadgets
 * ===================================================================== */

// plaq text writing class
class CPlaqText : public gControl {
protected:

	enum {
		bufSize = 128
	};


	char            lineBuf[bufSize]; // text to render on button
	textPallete     textFacePal;    // contains info about coloring for multi-depth text rendering
	Rect16          textRect;       // rect for the text
	int16           textPosition;
	gFont           *buttonFont;    // pointer to font for this button
	gFont           *oldFont;

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

	Alarm   waitAlarm,
	        minWaitAlarm;

	struct {
		char    *text;
		uint32  frameTime;
	} lineQueue[12];

	uint8       queueHead,
	            queueTail;
	bool        lineDisplayed;

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
	PortraitType    currentState[kNumViews + 1];
	uint16          numButtons;
	uint16          _numViews;
	GfxMultCompButton **buttons;
	GfxMultCompButton *indivButton;

	void setPortrait(uint16);

public:
	// button array, number of buttons per view, num views
	CPortrait(GfxMultCompButton *[], GfxMultCompButton *, const uint16, uint16);

	void ORset(uint16, PortraitType type);
	void set(uint16 brotherID, PortraitType type);
	PortraitType getCurrentState(uint16 brotherID) {
		return currentState[brotherID];
	}
	void getStateString(char buf[], int8 size, uint16 brotherID);
};


/* ===================================================================== *
    CInterface: Class that handles indicators for the interface
 * ===================================================================== */

class CMassWeightIndicator {
private:
	GameObject *containerObject;

public:
	static  bool bRedraw;

private:
	enum {
		// background image coords
		backImageXSize  = 88,
		backImageYSize  = 43,

		// pie image coords
		massPieXOffset  = 8,
		massPieYOffset  = 9,
		bulkPieXOffset  = 53,
		bulkPieYOffset  = 9,
		pieXSize        = 28,
		pieYSize        = 26
	};

	// xy positions of this indicator
	Point16 backImagePos;
	Point16 massPiePos;
	Point16 bulkPiePos;

	// memory for update
	uint16 currentMass;
	uint16 currentBulk;

	// resource context pointer
	hResContext *containerRes;

	// indicator images
	void *massBulkImag;

	// array of pointers to images
	void **pieIndImag;

	// image control buttons
	GfxCompImage          *pieMass;
	GfxCompImage          *pieBulk;


public:
	void invalidate(Rect16 *unused = nullptr) {
		pieMass->invalidate();
		pieBulk->invalidate();
	}

	CMassWeightIndicator(gPanelList *, const Point16 &, uint16 type = 1, bool death = false);
	~CMassWeightIndicator();

	uint16 getMassPieDiv() {
		return pieMass->getMax();
	}
	uint16 getBulkPieDiv() {
		return pieBulk->getMax();
	}

	void setMassPie(uint16 val) {
		pieMass->setCurrent(val);
	}
	void setBulkPie(uint16 val) {
		pieBulk->setCurrent(val);
	}
	void recalculate();
	static void update();
};


class CManaIndicator : public GfxCompImage {
public:

	// sizes of the mana star images
	enum startSize {
		star1XSize = 10,
		star1YSize = 9,
		star2XSize = 16,
		star2YSize = 15,
		star3XSize = 20,
		star3YSize = 19,
		star4XSize = 28,
		star4YSize = 27,
		star5XSize = 32,
		star5YSize = 31,
		star6XSize = 36,
		star6YSize = 35,
		star7XSize = 46,
		star7YSize = 45
	};

	// sizes of the rings
	enum ringSize {
		ring1XSize = 8,
		ring1YSize = 7,
		ring2XSize = 12,
		ring2YSize = 11,
		ring3XSize = 16,
		ring3YSize = 15,
		ring4XSize = 22,
		ring4YSize = 21,
		ring5XSize = 26,
		ring5YSize = 25,
		ring6XSize = 32,
		ring6YSize = 31,
		ring7XSize = 40,
		ring7YSize = 39
	};

	// area of control
	enum area {
		x       = 475,
		y       = 315,
		xSize   = 152,
		ySize   = 135
	};

	// coordinates
	enum {
		centerX     = x + xSize / 2,
		centerY     = y + ySize / 2,
		wellXSize   = 108,
		wellYSize   = 123,
		wellX       = (xSize / 2 - wellXSize / 2) + 1,
		wellY       = ySize / 2 - wellYSize / 2
	};

	// manas end points

	enum manaEndCoordsBase { // based on quadrants
		xOffset = 9,
		yOffset = -10,

		upperLeftX  = 16    + xOffset,
		upperLeftY  = 45    + yOffset,
		upperMidX   = 70    + xOffset,
		upperMidY   = 13    + yOffset,
		upperRightX = 122   + xOffset,
		upperRightY = 45    + yOffset,
		lowerLeftX  = 16    + xOffset,
		lowerLeftY  = 106   + yOffset,
		lowerMidX   = 70    + xOffset,
		lowerMidY   = 135   + yOffset,
		lowerRightX = 122   + xOffset,
		lowerRightY = 106   + yOffset
	};

	enum manaEndCoords {
		redEndX         = upperLeftX,   //a // these are just dup tags
		redEndY         = upperLeftY,   //b
		orangeEndX      = upperMidX,    //c
		orangeEndY      = upperMidY,
		yellowEndX      = upperRightX,  //d
		yellowEndY      = upperRightY,  //b
		greenEndX       = lowerLeftX,   //a
		greenEndY       = lowerLeftY,   //e
		blueEndX        = lowerMidX,    //c
		blueEndY        = lowerMidY,
		violetEndX      = lowerRightX,  //d
		violetEndY      = lowerRightY,  //e
		numXYCoords     = 12,
		numManaTypes    = numXYCoords / 2
	};

	// mana star display offset from well center
	enum startOffset {
		startOffset = 12
	};

	// mana information
	enum manainfo {
		maxLevel = 200,
		numManaRegions = numManaTypes
	};

	// resource data
	enum resourceInfo {
		numStars        = 7,
		numRings        = 7,
		starResNum      = 0,
		ringResNum      = 0,
		numManaColors   = 13
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
	hResContext     *resContext;

	// array of image pointers
	void            **starImages;
	void            **ringImages;

	// background image pointer
	void            *backImage;
	void            *wellImage;

	// image maps
	gPixelMap   savedMap;

	// array of manaLine infos for blitting
	manaLineInfo manaLines[numManaTypes];

	// array of ring and star end positions
	// this is initialized via constructor
	Point16 starRingEndPos[numManaTypes];

	Point16 starSizes[numStars];
	Point16 ringSizes[numRings];

	// these are checks against redundent updates
	int32   currentMana[numManaTypes], currentBaseMana[numManaTypes];
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
		return numManaRegions;
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

	static const char *hintText;

	enum {
		starFrameResNum     = 14,
		starFrameNum        = 1,
		starStart           = 0,
		starNum             = 23,
		starInitial         = 0,
		starLevels          = 24,
		numControls         = kNumViews
	};

	enum {
		starXPos    = 572,
		starYPos    = 21,
		starXSize   = 32,
		starYSize   = 32,
		starYOffset = 150,
		frameXPos   = 571,
		frameYPos   = 20,
		frameXSize  = 32,
		frameYSize  = 32
	};

	// resource handle
	hResContext *healthRes;

	// buttons
	GfxCompImage          *starBtns[numControls];
	GfxCompImage          *indivStarBtn;

	// array of pointer to the star imagery
	void **starImag;

	// health star frame imagery
	void *starFrameImag;

	void updateStar(GfxCompImage *starCtl, int32 bro, int32 baseVitality, int32 curVitality);

public:
	uint16  starIDs[3];
	int16   imageIndexMemory[4];

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
