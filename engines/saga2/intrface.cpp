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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/blitters.h"
#include "saga2/objects.h"
#include "saga2/button.h"
#include "saga2/contain.h"
#include "saga2/intrface.h"
#include "saga2/player.h"
#include "saga2/hresmgr.h"
#include "saga2/grequest.h"
#include "saga2/assign.h"
#include "saga2/grabinfo.h"
#include "saga2/mouseimg.h"
#include "saga2/uidialog.h"
#include "saga2/savefile.h"
#include "saga2/motion.h"
#include "saga2/enchant.h"
#include "saga2/display.h"
#include "saga2/localize.h"
#include "saga2/imagcach.h"

namespace Saga2 {

/* ===================================================================== *
   External delarations
 * ===================================================================== */
extern ReadyContainerView   *TrioCviews[kNumViews];
extern ReadyContainerView   *indivCviewTop, *indivCviewBot;
extern PlayerActor          playerList[];   //  Master list of all PlayerActors
extern gPanelList           *trioControls, *indivControls;
extern gPanelList           *playControls;
extern const uint32         imageGroupID;
extern gPanelList           *tileControls;
extern BackWindow           *mainWindow;
extern uint8                fixedColors[16];


/* ===================================================================== *
   Classes
 * ===================================================================== */

//	Private subclass of gCompImage for armor display

class gArmorIndicator : public gCompImage {
public:
	ArmorAttributes  attr;
	void drawClipped(gPort &,
	                 const   Point16 &,
	                 const   Rect16 &);

	void setValue(PlayerActorID pID);

	gArmorIndicator(gPanelList &list, const Rect16 &box, void *img, uint16 ident, AppFunc *cmd = nullptr)
		: gCompImage(list, box, img, ident, cmd) {
		attr.damageAbsorbtion = 0;
		attr.damageDivider = 1;
		attr.defenseBonus = 0;
	}
};

//	Private subclass of gControl for enchantment display
//	REM: Some of this should probably be moved to a header file, even though no-one
//	is ever going to need it other than this file.

enum enchantmentIcons {
	iconInvisible = 0,
	iconDetectPoison,
	iconSoulSight,
	iconPoisoned,
	iconParalysed,
	iconClumsy,
	iconFrozen,
	iconConstrained,
	iconAfraid,
	iconSeawalk,
	iconHaste,
	iconFirewalk,
	iconSurestrike,
	iconAdrenalFervor,
	iconInnerBalance,
	iconShadowWalk,
	iconBattleFever,
	iconSunWard,
	iconNetherWard,
	iconSpellBarrier,
	iconIronskin,
	iconNumbscent,
	iconProtectEvil,
	iconProtectUndead,
	iconProtectGhosts,
	iconCushionAir,
	iconResistImpact,
	iconResistSlash,
	iconResistProjectile,
	iconResistFire,
	iconResistAcid,
	iconResistHeat,
	iconResistCold,
	iconResistLightning,
	iconResistPoison,
	iconResistPsionic,
	iconResistDirectMagic,
	iconImmuneFire,
	iconImmuneAcid,
	iconImmuneHeat,
	iconImmuneCold,
	iconImmuneLightning,
	iconImmunePoison,
	iconImmunePsionic,

	iconCount
};

char *enchantmentNames[] = {
	"Invisible",
	"Detect Poison",
	"Soul Sight",
	"Poisoned",
	"Paralysed",
	"Clumsy",
	"Frozen",
	"Constrained",
	"Afraid",
	"Seawalk",
	"Haste",
	"Fire Walk",
	"Surestrike",
	"Adrenal Fervor",
	"Inner Balance",
	"Shadow Walk",
	"Battle Fever",
	"Sun Ward",
	"Nether Ward",
	"Spell Barrier",
	"Ironskin",
	"Numbscent",
	"Protection from Evil",
	"Protection from Undead",
	"Protection from Ghosts",
	"Cushion of Air",
	"Resist impact damage",
	"Resist slashing damage",
	"Resist projectile damage",
	"Resist Fire / Yellow Magic",
	"Resist Acid / Violet Magic",
	"Resist Heat / Red Magic",
	"Resist Cold / Blue Magic",
	"Resist Lightning / Orange Magic",
	"Resist Poison / Green Magic",
	"Resist Mental Damage",
	"Resist Direct Magic",
	"Immunity to Fire / Yellow Magic",
	"Immunity to Acid / Violet Magic",
	"Immunity to Heat / Red Magic",
	"Immunity to Cold / Blue Magic",
	"Immunity to Lightning / Orange Magic",
	"Immunity to Poison / Green Magic",
	"Immunity to Mental Damage",

	nullptr
};

class gEnchantmentDisplay : public gControl {

	uint8       iconFlags[iconCount];

	void drawClipped(gPort &, const Point16 &, const Rect16 &);
	void pointerMove(gPanelMessage &msg);
public:
	void setValue(PlayerActorID pID);

	gEnchantmentDisplay(gPanelList &list, uint16 ident, AppFunc *cmd = nullptr)
		: gControl(list, Rect16(0, 0, 630, 18), nullptr, ident, cmd) {
		memset(iconFlags, 0, sizeof iconFlags);
	}
};

/* ===================================================================== *
   Imports
 * ===================================================================== */

#ifndef FTAASM_H
extern void compositePixels(
    gPixelMap       *compMap,
    gPixelMap       *sprMap,
    int32           xpos,
    int32           ypos,
    uint8           *lookup);

extern void unpackImage(gPixelMap *map,
                                  int32 width,
                                  int32 rowCount,
                                  int8 *srcData);
#endif

// this is a redeclare of the struct in playmode.cpp
typedef struct {
	Point16     size;
	int16       compress;
	int8        data[2];
} ImageHeader;


// external appfuncs
APPFUNC(cmdBrain);

/* ===================================================================== *
   User interface application functions
 * ===================================================================== */
APPFUNC(cmdPortrait);
APPFUNC(cmdAggressive);
//APPFUNC( cmdJump );
APPFUNC(cmdArmor);
APPFUNC(cmdCenter);
APPFUNC(cmdBand);
APPFUNC(cmdOptions);
APPFUNC(cmdBroChange);
APPFUNC(cmdHealthStar);
APPFUNC(cmdMassInd);
APPFUNC(cmdBulkInd);
APPFUNC(cmdManaInd);

/* ===================================================================== *
   Globals
 * ===================================================================== */
static bool indivControlsFlag = FALSE;

bool userControlsSetup = FALSE;


/* ===================================================================== *
   User control metrics
 * ===================================================================== */

// position arrays for all buttons on the individual panels
Rect16  topBox[numButtons] =  {
	/* portrait          */             Rect16(489, 22 + (yContOffset * 0), 65, 72),
	/* agress            */             Rect16(559, 86 + (yContOffset * 0), 28, 27),
	/* jump              */             Rect16(592, 86 + (yContOffset * 0), 28, 27),
	/* center            */             Rect16(559, 56 + (yContOffset * 0), 28, 27),
	/* banding           */             Rect16(592, 56 + (yContOffset * 0), 28, 27),
	/* namePlates        */             Rect16(488, 94 + (yFaceOffset * 0), 65, 15),
	/* namePlateFrames   */             Rect16(487, 20 + (yFaceOffset * 0), 69, 92)
};


Rect16  midBox[numButtons] =  {
	Rect16(489, 22 + (yContOffset * 1), 65, 72),
	Rect16(559, 86 + (yContOffset * 1), 28, 27),
	Rect16(592, 86 + (yContOffset * 1), 28, 27),
	Rect16(559, 56 + (yContOffset * 1), 28, 27),
	Rect16(592, 56 + (yContOffset * 1), 28, 27),
	Rect16(488, 94 + (yFaceOffset * 1), 65, 15),
	Rect16(487, 20 + (yFaceOffset * 1), 69, 92)
};



Rect16  botBox[numButtons] =  {
	Rect16(489, 22 + (yContOffset * 2), 65, 72),
	Rect16(559, 86 + (yContOffset * 2), 28, 27),
	Rect16(592, 86 + (yContOffset * 2), 28, 27),
	Rect16(559, 56 + (yContOffset * 2), 28, 27),
	Rect16(592, 56 + (yContOffset * 2), 28, 27),
	Rect16(488, 94 + (yFaceOffset * 2), 65, 15),
	Rect16(487, 20 + (yFaceOffset * 2), 69, 92)
};



// options button
gCompButton         *optBtn;
gEnchantmentDisplay *enchDisp;

// brother buttons
gOwnerSelCompButton *julBtn;
gOwnerSelCompButton *phiBtn;
gOwnerSelCompButton *kevBtn;
gCompImage          *broBtnFrame;


// trio controls
gMultCompButton     *portBtns[kNumViews];
gOwnerSelCompButton *aggressBtns[kNumViews];
//gCompButton           *jumpBtns[kNumViews];
gOwnerSelCompButton *centerBtns[kNumViews];
gOwnerSelCompButton *bandingBtns[kNumViews];
gCompImage          *namePlates[kNumViews];
gCompImage          *namePlateFrames[kNumViews];
gArmorIndicator     *armorInd[kNumViews];

// individual
gMultCompButton     *indivPortBtn;
gOwnerSelCompButton *indivAggressBtn;
//gCompButton           *indivJumpBtn;
gOwnerSelCompButton *indivCenterBtn;
gOwnerSelCompButton *indivBandingBtn;
gCompImage          *indivNamePlate;
gCompImage          *indivNamePlateFrame;
gArmorIndicator     *indivArmorInd;

// mental button/indicators
gCompButton         *menConBtn;


// [brother panels] compressed image non-allocated pointer arrays
void **portImag[kNumViews];
void **aggressImag;
//void **jumpImag;
void **centerImag;
void **bandingImag;
void **menConBtnImag;
void **optBtnImag;
void **julBtnImag;
void **phiBtnImag;
void **kevBtnImag;

// portrait name plate things
void *namePlateImages[kNumViews];
void *namePlateFrameImag;
void *armorImag;

// brother selection button frame
void *broBtnFrameImag;

// name plate resource indexes
int16   namePlateResNum[] = { 8, 9, 10 };

// number of plate images
int16   numNamePlateRes[] = { 1, 1, 1 };

// brother resource names
ResName broNames[] = { { 'J', 'U', 'L', 0 },
	{ 'P', 'H', 'I', 0 },
	{ 'K', 'E', 'V', 0 }
};

const uint32 containerGroupID = MKTAG('C', 'O', 'N', 'T');


// button position views
// topBox, midBox, and botBox are defined in uimetrcs.h
Rect16  *views[] = { topBox, midBox, botBox };

// individual indicators/buttons
Rect16  menConBtnRect   = Rect16(485, 265, 44, 43);

// options button
Rect16  optBtnRect      = Rect16(20, 445, 26, 15);

//  enchantment display
Rect16  enchDispRect    = Rect16(0, 0, 630, 18);

// brother buttons and frame
Rect16  broBtnRect      = Rect16(481, 450, 144, 11);
Rect16  julBtnRect      = Rect16(482, 451, 44, 9);
Rect16  phiBtnRect      = Rect16(531, 451, 44, 9);
Rect16  kevBtnRect      = Rect16(580, 451, 44, 9);


textPallete genericTextPal(9 + 15, 20, 14, 11, 23, 17);
/*  uint8   dlPen;
    uint8   urPen;
    uint8   inPen;
    uint8   dlHilitePen;
    uint8   urHilitePen;
    uint8   inHilitePen; */

//  [Added here by Talin to fix problem in Win32. I don't know if it's the right place]
class CPortrait *Portrait;

/* ===================================================================== *
   ui controls
 * ===================================================================== */

// which brother is set in the individual controls
uint16  indivBrother;

bool isBrotherDead(PlayerActorID brotherID);

/* ===================================================================== *
   ui indicators
 * ===================================================================== */

// status line rect
Rect16      statusLineArea(49, 445, 407, 15);


/* ===================================================================== *
   Global class declarations
 * ===================================================================== */

// these objhets have to be initialized after resource are initialized
CStatusLine             *StatusLine          = nullptr;
CMassWeightIndicator    *MassWeightIndicator = nullptr;
CHealthIndicator        *HealthIndicator     = nullptr;
CManaIndicator          *ManaIndicator       = nullptr;

/* ===================================================================== *
   Clicks
 * ===================================================================== */

const int clickSounds = 3;

int32 maxClicks = clickSounds;
int32 clickSizes[clickSounds];
uint8 *clickData[clickSounds];

void playMemSound(uint32 s); // play click # s

/* ===================================================================== *
   Plaq writing class
 * ===================================================================== */

CPlaqText::CPlaqText(gPanelList     &list,
                     const Rect16  &box,
                     const char    *msg,
                     gFont         *font,
                     int16         textPos,
                     textPallete   &pal,
                     int16         ident,
                     AppFunc       *cmd)
	: gControl(list, box, msg, ident, cmd) {
	if (strlen(msg) <= bufSize) {
		strcpy(lineBuf, msg);
	} else {
		*lineBuf = '\0';
	}

	textFacePal     = pal;
	buttonFont      = font;
	textRect        = box;
	textPosition    = textPos;
}

void CPlaqText::enable(bool abled) {
	gPanel::enable(abled);
}

void CPlaqText::invalidate(void) {
	window.update(extent);
}

void CPlaqText::draw(void) {
	gPort           &port = window.windowPort;
	Rect16          rect = window.getExtent();


	// save pen color, etc.
	SAVE_GPORT_STATE(port);
	oldFont = port.font;

	// setup the port
	port.setMode(drawModeMatte);
	port.setFont(buttonFont);

	pointer.hide(port, extent);              // hide mouse pointer
	drawClipped(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
	pointer.show(port, extent);              // show mouse pointer

	// reset the old font
	port.setFont(oldFont);
}

void CPlaqText::drawClipped(gPort &port,
                            const Point16 &offset,
                            const Rect16 &r) {
	if (extent.overlap(r)) {
		if (*lineBuf) {
			textRect = extent;
			textRect.x -= offset.x;
			textRect.y -= offset.y;


			writePlaqText(port, textRect, buttonFont, textPosition, textFacePal, selected,  lineBuf);
		}
	}
}

/* ===================================================================== *
    Portrait control class
 * ===================================================================== */

CPortrait::CPortrait(gMultCompButton **portraits,
                     gMultCompButton *indivPort,
                     const uint16 numPorts,
                     uint16 numBrothers) {     // numBrothers = post 1
	// do some checking
	assert(portraits);
	assert(indivPort);

	for (uint16 i = 0; i < numBrothers; i++) {
		assert(portraits[i]);
	};

	buttons     = portraits;    // set the pointer for class
	indivButton = indivPort;    // set the individual portrait
	numButtons  = numPorts;     // number of buttons per pointer
	_numViews    = numBrothers;  // number of pointers for whole array

	// start off in a normal facial state
	for (uint16 i = 0; i < _numViews + 1; i++) {
		currentState[i] = normal;
	}
}

void CPortrait::setPortrait(uint16 brotherID) {
	assert(brotherID < _numViews + 1);

	// tell button to select and display new image
	if (brotherID == uiIndiv) {
		WriteStatusF(4, " Brother id %d", brotherID);

		indivButton->setCurrent(currentState[brotherID]);
		indivButton->invalidate();
	} else {
		buttons[brotherID]->setCurrent(currentState[brotherID]);
		buttons[brotherID]->invalidate();
	}
}

void CPortrait::set(uint16 brotherID, PortraitType type) {
	currentState[brotherID] = type;

	setPortrait(brotherID);
}

void CPortrait::ORset(uint16 brotherID, PortraitType type) { // brotherID = post 0
	assert(brotherID < _numViews + 1);

	if (type == currentState[brotherID]) {
		currentState[brotherID] = normal;
	} else {
		currentState[brotherID] = type;
	}

	// set this button to the new state
	setPortrait(brotherID);
}

size_t appendToStr(char *dst, const char *src, size_t srcLen, size_t maxCpyLen) {
	size_t      cpyLen;

	cpyLen = MIN(srcLen, maxCpyLen);
	memcpy(dst, src, cpyLen);
	dst[cpyLen] = '\0';

	return cpyLen;
}

void CPortrait::getStateString(char buf[], int8 size, uint16 brotherID) {
	static char asleepStr[]         = ASLEEP_STATE;
	static char paralysedStr[]      = PARALY_STATE;
	static char blindStr[]          = BLIND_STATE ;
	static char afraidStr[]         = AFRAID_STATE;
	static char angryStr[]          = ANGRY_STATE ;
	static char badlyWoundedStr[]   = BADWND_STATE;
	static char hurtStr[]           = HURT_STATE  ;
	static char poisonedStr[]       = POISON_STATE;
	static char diseasedStr[]       = DISEAS_STATE;
	static char normalStr[]         = NORMAL_STATE;
	static char commaStr[]          = ", ";

	PlayerActor     *pa = getPlayerActorAddress(brotherID);
	Actor           *a = pa->getActor();
	ActorAttributes &stats = pa->getBaseStats();
	size_t          length = 0;

	buf[size - 1] = '\0';

	if (a->isDead()) {
		strncpy(buf, DEAD_STATE, size - 1);
		return;
	}

	buf[0] = '\0';

	if (a->enchantmentFlags & (1 << actorAsleep)) {
		length +=   appendToStr(
		                &buf[length],
		                asleepStr,
		                elementsof(asleepStr) - 1,
		                size - length - 1);
	} else if (a->enchantmentFlags & (1 << actorParalyzed)) {
		length +=   appendToStr(
		                &buf[length],
		                paralysedStr,
		                elementsof(paralysedStr) - 1,
		                size - length - 1);
	} else if (a->enchantmentFlags & (1 << actorBlind)) {
		length +=   appendToStr(
		                &buf[length],
		                blindStr,
		                elementsof(blindStr) - 1,
		                size - length - 1);
	} else if (a->enchantmentFlags & (1 << actorFear)) {
		length +=   appendToStr(
		                &buf[length],
		                afraidStr,
		                elementsof(afraidStr) - 1,
		                size - length - 1);
	} else if (pa->isAggressive()) {
		length +=   appendToStr(
		                &buf[length],
		                angryStr,
		                elementsof(angryStr) - 1,
		                size - length - 1);
	}

	if (stats.vitality >= a->effectiveStats.vitality * 3) {
		if (length != 0)
			length +=   appendToStr(
			                &buf[length],
			                commaStr,
			                elementsof(commaStr) - 1,
			                size - length - 1);
		length +=   appendToStr(
		                &buf[length],
		                badlyWoundedStr,
		                elementsof(badlyWoundedStr) - 1,
		                size - length - 1);
	} else if (stats.vitality * 2 > a->effectiveStats.vitality * 3) {
		if (length != 0)
			length +=   appendToStr(
			                &buf[length],
			                commaStr,
			                elementsof(commaStr) - 1,
			                size - length - 1);
		length +=   appendToStr(
		                &buf[length],
		                hurtStr,
		                elementsof(hurtStr) - 1,
		                size - length - 1);
	}

	if (a->enchantmentFlags & (1 << actorPoisoned)) {
		if (length != 0)
			length +=   appendToStr(
			                &buf[length],
			                commaStr,
			                elementsof(commaStr) - 1,
			                size - length - 1);
		length +=   appendToStr(
		                &buf[length],
		                poisonedStr,
		                elementsof(poisonedStr) - 1,
		                size - length - 1);
	} else if (a->enchantmentFlags & (1 << actorDiseased)) {
		if (length != 0)
			length +=   appendToStr(
			                &buf[length],
			                commaStr,
			                elementsof(commaStr) - 1,
			                size - length - 1);
		length +=   appendToStr(
		                &buf[length],
		                diseasedStr,
		                elementsof(diseasedStr) - 1,
		                size - length - 1);
	}

	if (length == 0) {
		length +=   appendToStr(
		                &buf[length],
		                normalStr,
		                elementsof(normalStr) - 1,
		                size - length - 1);
	}
}

/* ===================================================================== *
   status line gadget
 * ===================================================================== */

// status line class
CStatusLine::CStatusLine(gPanelList         &list,
                         const Rect16    &box,
                         char            *msg,
                         gFont           *font,
                         int16           textPos,
                         textPallete     &pal,
                         int32           /*frameTime*/,
                         int16           ident,
                         AppFunc         *cmd) :
	CPlaqText(list, box, msg, font, textPos, pal, ident, cmd)

{
	int         i;

	lineDisplayed = FALSE;
	queueHead = queueTail = 0;

	for (i = 0; i < elementsof(lineQueue); i++)
		lineQueue[i].text = nullptr;
}

CStatusLine::~CStatusLine(void) {
	while (queueTail != queueHead) {
		assert(lineQueue[queueTail].text != nullptr);

		delete[] lineQueue[queueTail].text;
		queueTail = bump(queueTail);
	}
}

void CStatusLine::setLine(char *msg, uint32 frameTime) { // frametime def
	uint8       newHead = bump(queueHead);

	if (newHead != queueTail) {
		size_t      msgLen = strlen(msg);

		if ((lineQueue[queueHead].text = new char[msgLen + 1]()) !=  nullptr) {
			strcpy(lineQueue[queueHead].text, msg);
			lineQueue[queueHead].frameTime = frameTime;
			queueHead = newHead;
		}
	}
}

void CStatusLine::experationCheck(void) {
	if (lineDisplayed
	        && (waitAlarm.check()
	            || (queueTail != queueHead && minWaitAlarm.check()))) {
		enable(FALSE);
		window.update(extent);

		lineDisplayed = FALSE;
	}

	if (!lineDisplayed && queueTail != queueHead) {
		// enable the control
		enable(TRUE);

		// set up the time for this message
		waitAlarm.set(lineQueue[queueTail].frameTime);
		minWaitAlarm.set(lineQueue[queueTail].frameTime / 5);

		// copy upto the buffer's size in chars
		strncpy(lineBuf, lineQueue[queueTail].text, bufSize - 1);
		lineBuf[bufSize - 1] = '\0';

		//  free the queue text buffer
		delete[] lineQueue[queueTail].text;
		lineQueue[queueTail].text = nullptr;

		//  bump the queue tail
		queueTail = bump(queueTail);

		// draw the new textline
		window.update(extent);

		lineDisplayed = TRUE;
	}
}

void CStatusLine::clear(void) {
	enable(FALSE);
	window.update(extent);
	lineDisplayed = FALSE;

	queueHead = queueTail = 0;
}

/* ===================================================================== *
    CMassWeightInterface: Static list of indicators
 * ===================================================================== */
DList CMassWeightIndicator::indList;

/* ===================================================================== *
    CMassWeightInterface: mass and weight allowence indicators
 * ===================================================================== */

bool CMassWeightIndicator::bRedraw;

CMassWeightIndicator::CMassWeightIndicator(gPanelList *panel, const Point16 &pos, uint16 type, bool death) {

	// set up the position of this indicator
	backImagePos    = pos;
	massPiePos      = backImagePos;
	bulkPiePos      = backImagePos;

	massPiePos.x    += massPieXOffset;
	massPiePos.y    += massPieYOffset;
	bulkPiePos.x    += bulkPieXOffset;
	bulkPiePos.y    += bulkPieYOffset;

	bRedraw         = TRUE; // this MUST be true or the indicators will not draw the first time

	// attach the resource context
	containerRes = resFile->newContext(containerGroupID, "container context");

	// setup mass/bulk indicator imagery
	if (death) {
		checkAlloc(massBulkImag     = ImageCache.requestImage(containerRes, MKTAG('D', 'J', 'B', massBulkResNum)));

		checkAlloc(pieIndImag       = loadImageRes(containerRes, pieIndResNum, numPieIndImages, 'D', 'A', 'J'));
	} else {

		checkAlloc(massBulkImag     = ImageCache.requestImage(containerRes, MKTAG('G', 'J', 'B', massBulkResNum)));

		checkAlloc(pieIndImag       = loadImageRes(containerRes, pieIndResNum, numPieIndImages, 'G', 'A', 'J'));
	}

	// attach controls to the indivControls panel
	// these butttons will get deactivated along with the panel
	checkAlloc(pieMass = new gCompImage(*panel,
	                                       Rect16(massPiePos.x, massPiePos.y, pieXSize, pieYSize),
	                                       pieIndImag,
	                                       numPieIndImages,
	                                       0,
	                                       type,
	                                       cmdMassInd));

	checkAlloc(pieBulk = new gCompImage(*panel,
	                                       Rect16(bulkPiePos.x, bulkPiePos.y, pieXSize, pieYSize),
	                                       pieIndImag,
	                                       numPieIndImages,
	                                       0,
	                                       type,
	                                       cmdBulkInd));

	// mass/bulk back image
	checkAlloc(new gCompImage(*panel,
	                             Rect16(backImagePos.x, backImagePos.y, backImageXSize, backImageYSize),
	                             massBulkImag,
	                             uiIndiv,
	                             nullptr));

	// release resource context
	if (containerRes) {
		resFile->disposeContext(containerRes);
		containerRes = nullptr;
	}

	currentMass = 0;
	currentBulk = 0;

	// if this is something other then the ready containers
	if (type > 1) {
		containerObject = (GameObject *)panel->userData;
	} else {
		containerObject = nullptr;
	}

	indList.addHead(*this);
}

CMassWeightIndicator::~CMassWeightIndicator(void) {
	remove();

	unloadImageRes(pieIndImag, numPieIndImages);
	ImageCache.releaseImage(massBulkImag);
}

/*****************************************************************************
** Method: recalculate()
** Description: This will recalculate the values to be displayed for the
**              mass and bulk of the current ( single mode ) player
**/
void CMassWeightIndicator::recalculate(void) {
	assert(pieMass);
	assert(pieBulk);

	uint16 mass = getMassPieDiv();
	uint16 bulk = getBulkPieDiv();
	uint16 retMass, retBulk;


	if (containerObject) {
		setMassPie(retMass = getWeightRatio(containerObject, mass, FALSE));
		setBulkPie(retBulk = getBulkRatio(containerObject, bulk, FALSE));
	} else {
		setMassPie(retMass = getWeightRatio(playerList[getCenterActorPlayerID()].getActor(), mass, FALSE));
		setBulkPie(retBulk = getBulkRatio(playerList[getCenterActorPlayerID()].getActor(), bulk, FALSE));
	}
}

/*****************************************************************************
** Method: update()
** Description: This will call recalculate and then invalidate the entire
**              weight/bulk control ( so it refreshes )
**/
void CMassWeightIndicator::update(void) {
	CMassWeightIndicator *indNode = nullptr;

	if (bRedraw == TRUE) {
		for (indNode = (CMassWeightIndicator *)indList.first();
		        indNode;
		        indNode = (CMassWeightIndicator *)indNode->next()) {
			indNode->recalculate();
			indNode->invalidate();
		}

		bRedraw = FALSE;
	}
}

/* ===================================================================== *
    CManaIndicator: magic stuff indicator
 * ===================================================================== */

// setup the mana color tables
static uint8 manaColorMap[CManaIndicator::numManaTypes][CManaIndicator::numManaColors + 9] = {
	// each row has 9 leading zero's to acount for windows colors
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x74, 0x73, 0x72, 0x80, 0x84, 0x83, 0x82, 0x82, 0x81, 0x81, 0xF4, 0xE9, // Red

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x74, 0x73, 0x72, 0x78, 0x77, 0x76, 0x75, 0x6B, 0x6A, 0x69, 0x3A, 0x39, // Orange

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x64, 0x63, 0x62, 0x61, 0x60, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59, // Yellow

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xE8, 0xE7, 0xE6, 0xE5, 0xE4, 0xE3, 0xE2, 0xE1, 0xE0, 0xDF, 0xDE, 0xC9, // Green

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xA0, 0x9F, 0x9E, 0xB2, 0xB1, 0xB0, 0xAF, 0xAE, 0xAD, 0xAC, 0xAA, 0xA9, // Blue

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xA0, 0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x93, 0x92, 0x92, 0x91, 0x0C, // Violet
};


CManaIndicator::CManaIndicator(gPanelList &list) : gCompImage(list,
	        Rect16(x, y, xSize, ySize),
	        nullptr,
	        0,
	        cmdManaInd) {
	assert(resFile);

	// init the resource handle with the mana resource group
	resContext  = resFile->newContext(MKTAG('M', 'A', 'N', 'A'),
	                                  "mana context");

	// load star images
	checkAlloc(starImages = loadImageRes(resContext,
	                                     starResNum,
	                                     numStars,
	                                     'S', 'T', 'A'));

	// load in the ring images
	checkAlloc(ringImages = loadImageRes(resContext,
	                                     ringResNum,
	                                     numRings,
	                                     'R', 'N', 'G'));

	checkAlloc(backImage = ImageCache.requestImage(resContext, MKTAG('B', 'A', 'C', 'K')));

	checkAlloc(wellImage = ImageCache.requestImage(resContext, MKTAG('W', 'E', 'L', 'L')));

	// hmm this could be cleaner...
	starRingEndPos[0] = Point16(redEndX,    redEndY);
	starRingEndPos[1] = Point16(orangeEndX, orangeEndY);
	starRingEndPos[2] = Point16(yellowEndX, yellowEndY);
	starRingEndPos[3] = Point16(greenEndX,  greenEndY);
	starRingEndPos[4] = Point16(blueEndX,   blueEndY);
	starRingEndPos[5] = Point16(violetEndX, violetEndY);

	starSizes[0] = Point16(star1XSize, star1YSize);
	starSizes[1] = Point16(star2XSize, star2YSize);
	starSizes[2] = Point16(star3XSize, star3YSize);
	starSizes[3] = Point16(star4XSize, star4YSize);
	starSizes[4] = Point16(star5XSize, star5YSize);
	starSizes[5] = Point16(star6XSize, star6YSize);
	starSizes[6] = Point16(star7XSize, star7YSize);

	ringSizes[0] = Point16(ring1XSize, ring1YSize);
	ringSizes[1] = Point16(ring2XSize, ring2YSize);
	ringSizes[2] = Point16(ring3XSize, ring3YSize);
	ringSizes[3] = Point16(ring4XSize, ring4YSize);
	ringSizes[4] = Point16(ring5XSize, ring5YSize);
	ringSizes[5] = Point16(ring6XSize, ring6YSize);
	ringSizes[6] = Point16(ring7XSize, ring7YSize);


	// get rid of resource context
	resFile->disposeContext(resContext);
	resContext = nullptr;

	// set update checks to nominal values
	for (uint16 i = 0; i < numManaTypes; i++) {
		currentMana[i]        = -1;
		currentBaseMana[i]    = -1;
	}

	// init the save map
	savedMap.size = Extent16(xSize, ySize);
	savedMap.data = new uint8[savedMap.bytes()];
	checkAlloc(savedMap.data);
}

CManaIndicator::~CManaIndicator(void) {
	// release images
	unloadImageRes(starImages, numStars);
	unloadImageRes(ringImages, numRings);

	// release back image
	ImageCache.releaseImage(backImage);
	ImageCache.releaseImage(wellImage);

	// release the saved map
	if (savedMap.data)
		delete[] savedMap.data;
}

// this method provides a rect for any of the six mana regions of the control
// region numbers:
//  -------
//  |0 1 2|
//  |3 4 5|
//  -------

Rect16 CManaIndicator::getManaRegionRect(int8 nRegion) {
	assert(nRegion >= 0 && nRegion < numManaRegions);

	int boxSizeX = xSize / 3;
	int boxSizeY = ySize / 2;

	static Rect16 manaRegionRects[numManaRegions] = {
		Rect16(x, y, boxSizeX, boxSizeY),
		Rect16(x + boxSizeX, y, boxSizeX, boxSizeY),
		Rect16(x + boxSizeX * 2, y, boxSizeX, boxSizeY),
		Rect16(x, y + boxSizeY, boxSizeX, boxSizeY),
		Rect16(x + boxSizeX, y + boxSizeY, boxSizeX, boxSizeY),
		Rect16(x + boxSizeX * 2, y + boxSizeY, boxSizeX, boxSizeY)
	};

	return manaRegionRects[nRegion];
}

void CManaIndicator::draw(void) {

	gPort           &port = window.windowPort;


	// save pen color, etc.
	SAVE_GPORT_STATE(port);

	// setup the port
	port.setMode(drawModeMatte);

	pointer.hide(port, extent);              // hide mouse pointer
	drawClipped(port, Point16(0, 0), Rect16(0, 0, xSize, ySize));
	pointer.show(port, extent);              // show mouse pointer


}


void CManaIndicator::drawClipped(gPort &port,
                                 const Point16 &offset,
                                 const Rect16 &clipRect) {
	bool calcDraw;

	// Do an update to the mana star info if needed,
	// if not, do not draw stuff
	calcDraw = update(&playerList[getCenterActorPlayerID()]);

	if (!calcDraw) {
		if (!extent.overlap(clipRect)) return;

		// draw the saved image to the port
		port.setMode(drawModeMatte);
		port.bltPixels(savedMap, 0, 0,
		               extent.x - offset.x, extent.y - offset.y,
		               xSize, ySize);

		// draw the frame
		drawCompressedImage(port, Point16(extent.x - offset.x, extent.y - offset.y), backImage);

		// and finish
		return;
	}

	// otherwise continue with the update
	pointer.hide();

	// create a temporary gPort to blit stuff to
	gPort       tempPort;
	gPixelMap   ringMap, starMap, mixMap, tempMap;


	if (NewTempPort(tempPort, xSize, ySize) != errOK) return;

	// set the blit surface to a flat black
	memset(tempPort.map->data, 24, tempPort.map->bytes());

	// draw the well
	drawCompressedImage(tempPort, Point16(wellX, wellY), wellImage);

	// make a mixing plane and blank it
	mixMap.size = Extent16(xSize, ySize);
	mixMap.data = new uint8[mixMap.bytes()]();
	// make a temp plane and blank it
	tempMap.size = Extent16(xSize, ySize);
	tempMap.data = new uint8[tempMap.bytes()]();

	// clear out the blit surfaces
	memset(mixMap.data, 0, mixMap.bytes());
	memset(tempMap.data, 0, tempMap.bytes());

	// draw as glyph
	tempPort.setMode(drawModeMatte);

	// draw each star and ring with color remap
	for (uint16 i = 0; i < numManaTypes; i++) {
		// get the header for the image pointer passed
		ImageHeader *starHdr = (ImageHeader *)starImages[manaLines[i].starImageIndex];
		ImageHeader *ringHdr = (ImageHeader *)ringImages[manaLines[i].ringImageIndex];

		// set the buffer blit area to the image size
		starMap.size = starHdr->size;
		ringMap.size = ringHdr->size;

		// see if it's compressed
		if (starHdr->compress) {
			// allocation of the temp buffer
			starMap.data = new uint8[starMap.bytes()]();
			checkAlloc(starMap.data);

			// if it is then upack it to spec'ed coords.
			unpackImage(&starMap, starMap.size.x, starMap.size.y, starHdr->data);
		} else starMap.data = (uint8 *)starHdr->data;

		// see if it's compressed
		if (ringHdr->compress) {
			// allocation of the temp buffer
			ringMap.data = new uint8[ringMap.bytes()]();
			checkAlloc(ringMap.data);

			// if it is then upack it to spec'ed coords.
			unpackImage(&ringMap, ringMap.size.x, ringMap.size.y, ringHdr->data);
		} else ringMap.data = (uint8 *)ringHdr->data;

		// now blit the rings to the mixing surface
		TBlit(&mixMap, &ringMap, manaLines[i].ringPos.x, manaLines[i].ringPos.y);
		TBlit(&tempMap, &starMap, manaLines[i].starPos.x, manaLines[i].starPos.y);

		// now do a peusdo-log additive thing to the images
		uint8   *dst    = (uint8 *)mixMap.data;
		uint8   *src    = (uint8 *)tempMap.data;

		// get the least common dinominator for size ( should be equal )
		uint16  bufferSize  = MIN(mixMap.bytes(), tempMap.bytes());

		for (uint16 j = 0; j < bufferSize; j++) {
			// image bug fix/kludge
			if (dst[j] > 21) dst[j] = 10;
			if (src[j] > 21) src[j] = 10;

			// if the tempMap pixel is greater then zero
			if (src[j] != 0 && src[j] > dst[j]) {
				dst[j] += src[j] - dst[j];
			}

		}

		// for each color index possible, match correct color value
		// at dest buffer
		compositePixels(
		    tempPort.map,
		    &mixMap,
		    0,
		    0,
		    manaColorMap[i]);

		// clear out the mixing surfaces
		memset(mixMap.data, 0, mixMap.bytes());
		memset(tempMap.data, 0, tempMap.bytes());

		// dispose the temporary gPixelMap
		if (starHdr->compress)
			delete[] starMap.data;
		if (ringHdr->compress)
			delete[] ringMap.data;
	}

	// save this frame
	TBlit(&savedMap, tempPort.map, 0, 0);

	//  Blit the pixelmap to the main screen
	port.setMode(drawModeMatte);
	port.bltPixels(*tempPort.map, 0, 0,
	               extent.x - offset.x, extent.y - offset.y,
	               xSize, ySize);

	// now blit the frame on top of it all.
	drawCompressedImage(port, Point16(extent.x - offset.x, extent.y - offset.y), backImage);

	// dispose of temporary pixelmap
	DisposeTempPort(tempPort);
	if (mixMap.data)
		delete[] mixMap.data;
	if (tempMap.data)
		delete[] tempMap.data;

	pointer.show();
}

bool CManaIndicator::needUpdate(PlayerActor *player) {
	assert(player);

	// get the ability scores for this character
	ActorAttributes *stats          = player->getEffStats();
	ActorAttributes baseStatsRef    = player->getBaseStats();
	int16           manaAmount;
	int16           baseManaAmount;
	uint16          i;
	bool            newData = FALSE;

	// this could do more array checking, but
	// the number of mana type should remain stable
	for (i = 0; i < numManaTypes; i++) {
		manaAmount      = stats->mana(i);
		baseManaAmount  = baseStatsRef.mana(i);

		// check for new data
		if (manaAmount != currentMana[i] || baseManaAmount != currentBaseMana[i]) {
			return TRUE;
		}
	}

	return FALSE;
}


bool CManaIndicator::update(PlayerActor *player) {
	assert(player);

	// get the ability scores for this character
	ActorAttributes *stats          = player->getEffStats();
	ActorAttributes baseStatsRef    = player->getBaseStats();
	int16           manaAmount;
	int16           baseManaAmount;
	uint16          i;
	bool            newData = FALSE;

	// this could do more array checking, but
	// the number of mana type should remain stable
	for (i = 0; i < numManaTypes; i++) {
		manaAmount      = stats->mana(i);
		baseManaAmount  = baseStatsRef.mana(i);

		// check for new data
		if (manaAmount != currentMana[i] || baseManaAmount != currentBaseMana[i]) {
			newData = TRUE;

			currentMana[i]        = manaAmount;
			currentBaseMana[i]    = baseManaAmount;
		}

		// get manaLine info ( which star/ring image, and position on screen )
		// from getStarInfo which takes the mana type index ( i ),
		// current mana total, and the player base mana
		if (newData == TRUE) {
			getManaLineInfo(i, manaAmount, baseManaAmount, &manaLines[i]);
		}
	}

	// return the state of data change
	if (newData  == FALSE) {
		return FALSE;
	} else {
		return TRUE;
	}
}

//  A generalized interpolation template
template<class T> inline T LERP(T p1, T p2, int32 steps, int32 stepNum) {
	return p1 + (((p2 - p1) * stepNum) / steps);
}

void CManaIndicator::getManaLineInfo(uint16 index,
                                     int16 manaAmount,
                                     int16 baseManaAmount,
                                     manaLineInfo *info) {
	Point16 basePos = Point16(xSize / 2, ySize / 2);

	// div zero prevention
	if (manaAmount == 0) manaAmount = 1;
	if (baseManaAmount == 0) baseManaAmount = 1;

	manaLineInfo manaInfo;

	//  Calculate the positions of the mana stars, and which images to use.
	manaInfo.starPos        = LERP(basePos,
	                               starRingEndPos[index],
	                               (int32)maxLevel,
	                               (int32)manaAmount);

	manaInfo.ringPos        = LERP(basePos,
	                               starRingEndPos[index],
	                               (int32)maxLevel,
	                               (int32)baseManaAmount);

	manaInfo.starImageIndex = clamp(0,     manaAmount * numStars / maxLevel, numStars - 1);
	manaInfo.ringImageIndex = clamp(0, baseManaAmount * numStars / maxLevel, numRings - 1);

	// now do centering correct for images
	manaInfo.starPos.x -= starSizes[manaInfo.starImageIndex].x / 2;
	manaInfo.starPos.y -= starSizes[manaInfo.starImageIndex].y / 2;
	manaInfo.ringPos.x -= ringSizes[manaInfo.ringImageIndex].x / 2;
	manaInfo.ringPos.y -= ringSizes[manaInfo.ringImageIndex].y / 2;

	// return the manaLineInfo struct info about mana star ring
	*info = manaInfo;
}

/* ===================================================================== *
    CHealthIndicator: Health star indicator
 * ===================================================================== */

CHealthIndicator::CHealthIndicator(AppFunc *cmd) {
	uint16 i;

	// init the resource handle with the image group context
	healthRes = resFile->newContext(imageGroupID, "health imagery context");

	// load in health star imagery
	checkAlloc(starImag = loadButtonRes(healthRes, starStart, starNum, 'S', 'T', 'A'));

	// load in the health star border
	checkAlloc(starFrameImag    = ImageCache.requestImage(healthRes, MKTAG('B', 'T', 'N', starFrameResNum)));

	// set the image indexes to nominal startup values
	for (i = 0; i < numControls + 1; i++) {
		imageIndexMemory[i] = -1;
	}

	// setup the id's for each of the stars
	starIDs[0] = uiJulian;
	starIDs[1] = uiPhillip;
	starIDs[2] = uiKevin;


	// health controls for the trio view
	// deallocated with panel
	for (i = 0; i < numControls; i++) {
		checkAlloc(starBtns[i] = new gCompImage(*trioControls,
		                           Rect16(starXPos,
		                                  starYPos + starYOffset * i,
		                                  starXSize,
		                                  starYSize),
		                           starImag,
		                           starNum,
		                           starInitial,
		                           starIDs[i],
		                           cmd));


		// image control for the star border/frame trio mode
		checkAlloc(new gCompImage(*trioControls,
		                             Rect16(frameXPos,
		                                    frameYPos + starYOffset * i,
		                                    frameXSize,
		                                    frameYSize),
		                             starFrameImag,
		                             0,
		                             nullptr));


	}
	// health control for individual mode
	// deallocated with panel
	checkAlloc(indivStarBtn = new gCompImage(*indivControls,
	                          Rect16(starXPos,
	                                 starYPos,
	                                 starXSize,
	                                 starYSize),
	                          starImag,
	                          starNum,
	                          starInitial,
	                          uiIndiv,
	                          cmd));

	// image control for the star border/frame indiv mode
	checkAlloc(new gCompImage(*indivControls,
	                             Rect16(frameXPos,
	                                    frameYPos,
	                                    frameXSize,
	                                    frameYSize),
	                             starFrameImag,
	                             0,
	                             nullptr));

	// release resource context
	if (healthRes) {
		resFile->disposeContext(healthRes);
		healthRes = nullptr;
	}
}


CHealthIndicator::~CHealthIndicator(void) {
	// release star imagery
	unloadImageRes(starImag, starNum);

	// release star frame imagery
	ImageCache.releaseImage(starFrameImag);
}

//  Recalculate and update the health star for a particular brother

void CHealthIndicator::updateStar(gCompImage *starCtl, int32 bro, int32 baseVitality, int32 curVitality) {
	assert(baseVitality >= 0);

	int16 maxStar, imageIndex;

	// prevent div zero
	if (baseVitality == 0) baseVitality = 1;

	maxStar = clamp(0, baseVitality / 10 + 14, starLevels - 1);
//	imageIndex = (int16)( sqrt( sqrt( (double)curVitality ) ) * maxStar) / sqrt( sqrt( (double)baseVitality ) );
	imageIndex = (int16)(sqrt((double)MAX(0, curVitality)) * maxStar) / sqrt((double)baseVitality);

	// prevent needless draws
	if (imageIndexMemory[bro] != imageIndex) {
		starCtl->setCurrent(imageIndex);
		starCtl->invalidate();

		imageIndexMemory[bro] = imageIndex;
	}
}

void CHealthIndicator::update(void) {
	if (indivControlsFlag) {
		// get the stats for the selected brother
		int16 baseVitality  = playerList[translatePanID(uiIndiv)].getBaseStats().vitality;
		int16 currVitality  = playerList[translatePanID(uiIndiv)].getEffStats()->vitality;

		updateStar(indivStarBtn, uiIndiv, baseVitality, currVitality);
	} else {

		for (uint16 i = 0; i < numControls; i++) {
			// get the stats for the selected brother
			int16 baseVitality  = playerList[i].getBaseStats().vitality;
			int16 currVitality  = playerList[i].getEffStats()->vitality;

			updateStar(starBtns[i], i, baseVitality, currVitality);
		}
	}
}
/* ===================================================================== *
   Plaq style text writing function
 * ===================================================================== */

void writePlaqText(gPort            &port,
                   const Rect16    &r,
                   gFont           *font,
                   int16           textPos,
                   textPallete     &pal,
                   bool            hiLite,
                   const char      *msg, ...) {
	char            lineBuf[128];
	va_list         argptr;
	int16           cnt;
	Rect16          workRect;
	gFont           *oldFont = port.font;

	va_start(argptr, msg);
	cnt = vsprintf(lineBuf, msg, argptr);
	va_end(argptr);

	SAVE_GPORT_STATE(port);
	port.setMode(drawModeMatte);
	port.setFont(font);

	workRect = r;

	port.setColor(hiLite ? pal.dlHilitePen : pal.dlPen);
	workRect.x--;
	port.drawTextInBox(lineBuf, cnt, workRect, textPos, Point16(0,  0));

	workRect.y++;
	port.drawTextInBox(lineBuf, cnt, workRect, textPos, Point16(0, 0));

	workRect.x++;
	port.drawTextInBox(lineBuf, cnt, workRect, textPos, Point16(0, 0));

	port.setColor(hiLite ? pal.urHilitePen : pal.urPen);
	workRect.x++;
	workRect.y--;
	port.drawTextInBox(lineBuf, cnt, workRect, textPos, Point16(0,  0));

	workRect.y--;
	port.drawTextInBox(lineBuf, cnt, workRect, textPos, Point16(0,  0));

	workRect.x--;
	port.drawTextInBox(lineBuf, cnt, workRect, textPos, Point16(0,  0));


	port.setColor(hiLite ? pal.inHilitePen : pal.inPen);
	workRect.y++;
	port.drawTextInBox(lineBuf, cnt, workRect, textPos, Point16(0,  0));

	port.setFont(oldFont);
}

void writePlaqTextPos(gPort         &port,
                      const Point16    &pos,
                      gFont            *font,
                      int16            /*textPos*/,
                      textPallete      &pal,
                      bool             hiLite,
                      const char       *msg, ...) {
	char            lineBuf[128];
	va_list         argptr;
	int16           cnt;
	Point16         drawPos;
	gFont           *oldFont = port.font;

	va_start(argptr, msg);
	cnt = vsprintf(lineBuf, msg, argptr);
	va_end(argptr);

	SAVE_GPORT_STATE(port);
	port.setMode(drawModeMatte);
	port.setFont(font);

	drawPos = pos;

	port.setColor(hiLite ? pal.dlHilitePen : pal.dlPen);
	drawPos.x--;
	port.moveTo(drawPos);

	port.drawText(lineBuf, -1);

	drawPos.y++;
	port.moveTo(drawPos);

	port.drawText(lineBuf, -1);

	drawPos.x++;
	port.moveTo(drawPos);

	port.drawText(lineBuf, -1);

	port.setColor(hiLite ? pal.urHilitePen : pal.urPen);
	drawPos.x++;
	drawPos.y--;
	port.moveTo(drawPos);

	port.drawText(lineBuf, -1);

	drawPos.y--;
	port.moveTo(drawPos);

	port.drawText(lineBuf, -1);

	drawPos.x--;
	port.moveTo(drawPos);

	port.drawText(lineBuf, -1);


	port.setColor(hiLite ? pal.inHilitePen : pal.inPen);
	drawPos.y++;
	port.moveTo(drawPos);

	port.drawText(lineBuf, -1);

	port.setFont(oldFont);
}



///////////////////////////////////////////////////////////////////
/* functions for loading and unloading sets of compressed images */


// creates an array of button images
// passed a resource context, resource ref ID and the number of images
// to sequentially load in
void **loadButtonRes(hResContext *con, int16 resID, int16 numRes) {
	int16   i, k;

	void **images = (void **) TALLOC(sizeof(void *)*numRes, memInterface);

	for (i = 0, k = resID; i < numRes; i++, k++) {
		// get an image from the image cache
		images[i] = ImageCache.requestImage(con, MKTAG('B', 'T', 'N', k));
	}

	return images;
}

// creates an array of images
// passed a resource context, resource ref ID, the number of images
// to sequentially load in, and the context id's
void **loadButtonRes(hResContext *con, int16 resID, int16 numRes, char a, char b, char c) {
	int16   i, k;


	void **images = (void **) TALLOC(sizeof(void *)*numRes, memInterface);

	for (i = 0, k = resID; i < numRes; i++, k++) {
		images[i] = ImageCache.requestImage(con, MKTAG(a, b, c, k));
	}

	return images;
}

// see LoadButtonRes
void **loadImageRes(hResContext *con, int16 resID, int16 numRes, char a, char b, char c) {
	return loadButtonRes(con, resID, numRes, a, b, c);
}

// correctly deletes any image arrays allocated with the LoadButtonRes or
// LoadImageRes sets
void unloadImageRes(void **images, int16 numRes) {
	int16   i;

	if (images) {
		for (i = 0; i < numRes; i++) {
			ImageCache.releaseImage(images[i]);
		}

		delete images;
	}
}

// defined for setup off all button based user controls
void SetupUserControls(void) {
	// index variables
	uint16  n;
	uint8   index = 0;

	// resource handle
	hResContext *imageRes;

	// brother panel id's
	uint16  brotherIDs[kNumViews]   =  { uiJulian, uiPhillip, uiKevin };

	// portrait resource indexes
	int16   portResNum[]    = { 0, 0, 0 };


	// image setup

	// init the resource handle with the image group context
	imageRes        = resFile->newContext(imageGroupID, "image context");


	// set up the control button images >>> move checkAlloc to *(new) check
	checkAlloc(aggressImag      = loadButtonRes(imageRes, aggressResNum, numBtnImages));
//	checkAlloc( jumpImag     = loadButtonRes( imageRes, jumpResNum, numBtnImages ) );
	checkAlloc(centerImag       = loadButtonRes(imageRes, centerResNum, numBtnImages));
	checkAlloc(bandingImag      = loadButtonRes(imageRes, bandingResNum, numBtnImages));
	checkAlloc(menConBtnImag    = loadButtonRes(imageRes, menConBtnResNum, numBtnImages));

	// setup the options button imagery
	checkAlloc(optBtnImag       = loadButtonRes(imageRes, optBtnResNum, numBtnImages));

	// setup the brother selector button imagery and button frame
	checkAlloc(julBtnImag       = loadButtonRes(imageRes, julBtnResNum, numBtnImages));
	checkAlloc(phiBtnImag       = loadButtonRes(imageRes, phiBtnResNum, numBtnImages));
	checkAlloc(kevBtnImag       = loadButtonRes(imageRes, kevBtnResNum, numBtnImages));
	checkAlloc(broBtnFrameImag  = ImageCache.requestImage(imageRes, MKTAG('F', 'R', 'A', 'M')));


	// set up the portrait name plates
	for (n = 0; n < kNumViews; n++) {
		checkAlloc(namePlateImages[n] = ImageCache.requestImage(imageRes, MKTAG('B', 'T', 'N', namePlateResNum[n])));
	}

	// get the frame image
	checkAlloc(namePlateFrameImag = ImageCache.requestImage(imageRes, MKTAG('B', 'T', 'N', 15)));
	checkAlloc(armorImag = ImageCache.requestImage(imageRes, MKTAG('B', 'T', 'N', 34)));

	// clean out the old context
	if (imageRes) resFile->disposeContext(imageRes);
	imageRes = nullptr;

	// init the resource handle with the face group context
	imageRes = resFile->newContext(faceGroupID, "face resources");

	// set up the portrait button images
	for (n = 0; n < kNumViews; n++) {
		checkAlloc(portImag[n]    = loadButtonRes(imageRes, portResNum[n], numPortImages, broNames[n].a, broNames[n].b, broNames[n].c));
	}

	// setup stand alone controls

	checkAlloc(optBtn = new gCompButton(*playControls, optBtnRect, optBtnImag,
	                                       numBtnImages, 0, cmdOptions));

	checkAlloc(enchDisp = new gEnchantmentDisplay(*playControls, 0));

	// setup the trio user cntl buttons
	for (n = 0; n < kNumViews; n++) {
		// portrait button
		checkAlloc(portBtns[n]        = new gMultCompButton(*trioControls, views[n][index++],
		                                  portImag[n], numPortImages, 0, FALSE, brotherIDs[n], cmdPortrait));

		portBtns[n]->setMousePoll(TRUE);

		// aggressive button
		checkAlloc(aggressBtns[n]     = new gOwnerSelCompButton(*trioControls, views[n][index++],
		                                  aggressImag, numBtnImages, brotherIDs[n], cmdAggressive));

		// jump  button
//		checkAlloc( jumpBtns[n]       = new gCompButton( *trioControls, views[n][index++],
//							               jumpImag, numBtnImages, brotherIDs[n], cmdJump ) );

		// name plates that go under the portraits
		checkAlloc(armorInd[n]        = new gArmorIndicator(*trioControls, views[n][index++],
		                                  armorImag, brotherIDs[n], cmdArmor));

		// center on brother
		checkAlloc(centerBtns[n]      = new gOwnerSelCompButton(*trioControls, views[n][index++],
		                                  centerImag, numBtnImages, brotherIDs[n], cmdCenter));

		// banding
		checkAlloc(bandingBtns[n] = new gOwnerSelCompButton(*trioControls, views[n][index++],
		                              bandingImag, numBtnImages, brotherIDs[n], cmdBand));

		// name plates that go under the portraits
		checkAlloc(namePlates[n]  = new gCompImage(*trioControls, views[n][index++],
		                              namePlateImages[n], 0, nullptr));

		// the frames for the name plates
		checkAlloc(namePlateFrames[n] = new gCompImage(*trioControls, views[n][index++],
		                                  namePlateFrameImag, 0, nullptr));

		index = 0;
	}

	// individual control buttons

	// portrait button
	checkAlloc(indivPortBtn = new gMultCompButton(*indivControls, views[0][index++],
	                          portImag[0], numPortImages, 0, FALSE, uiIndiv, cmdPortrait));
	indivPortBtn->setMousePoll(TRUE);

	// aggressive button
	checkAlloc(indivAggressBtn  = new gOwnerSelCompButton(*indivControls, views[0][index++],
	                              aggressImag, numBtnImages, uiIndiv, cmdAggressive));

	// jump button
//	checkAlloc( indivJumpBtn = new gCompButton( *indivControls, views[0][index++],
//						            jumpImag, numBtnImages, uiIndiv, cmdJump ) );

	checkAlloc(indivArmorInd    = new gArmorIndicator(*indivControls, views[0][index++],
	                              armorImag, uiIndiv, cmdArmor));
	// center on brother
	checkAlloc(indivCenterBtn   = new gOwnerSelCompButton(*indivControls, views[0][index++],
	                              centerImag, numBtnImages, uiIndiv, cmdCenter));

	// banding
	checkAlloc(indivBandingBtn  = new gOwnerSelCompButton(*indivControls, views[0][index++],
	                              bandingImag, numBtnImages, uiIndiv, cmdBand));

	// name plates that go under the portraits
	checkAlloc(indivNamePlate  = new gCompImage(*indivControls, views[0][index++],
	                             namePlateImages[0], 0, nullptr));

	// the frames for the name plates
	checkAlloc(indivNamePlateFrame = new gCompImage(*indivControls, views[0][index++],
	                                 namePlateFrameImag, 0, nullptr));

	// setup the portrait object
	checkAlloc(Portrait = new CPortrait(portBtns,      // portrait buttons
	                                       indivPortBtn,
	                                       numPortImages,// num of images per button
	                                       kNumViews));   // number of brothers


	// mental container button
	checkAlloc(menConBtn    = new gCompButton(*indivControls, menConBtnRect,
	                          menConBtnImag, numBtnImages, uiIndiv, cmdBrain));

	// brother selection buttons >>> need to replace these with sticky buttons
	checkAlloc(julBtn   = new gOwnerSelCompButton(*indivControls, julBtnRect,
	                      julBtnImag, numBtnImages, uiJulian, cmdBroChange));

	checkAlloc(phiBtn   = new gOwnerSelCompButton(*indivControls, phiBtnRect,
	                      phiBtnImag, numBtnImages, uiPhillip, cmdBroChange));

	checkAlloc(kevBtn   = new gOwnerSelCompButton(*indivControls, kevBtnRect,
	                      kevBtnImag, numBtnImages, uiKevin, cmdBroChange));

	// frame for brother buttons
	checkAlloc(broBtnFrame  = new gCompImage(*indivControls, broBtnRect,
	                          broBtnFrameImag, uiIndiv, nullptr));

	// make the mana indicator
	checkAlloc(ManaIndicator = new CManaIndicator(*indivControls));
	ManaIndicator->setMousePoll(TRUE);

	// get rid of the resource contexts
	if (imageRes) {
		resFile->disposeContext(imageRes);
		imageRes = nullptr;
	}

	//The controls need to be enabled but undrawn at startup
	//if ( displayEnabled() )
	//  userControlsSetup = TRUE;
	updateAllUserControls();
}

void enableUserControls(void) {
	userControlsSetup = TRUE;
}

void disableUserControls(void) {
	userControlsSetup = FALSE;
}

// defines the cleanup for ALL user interface controls
void CleanupUserControls(void) {
	userControlsSetup = FALSE;
	CleanupButtonImages();
}

// defines the cleaup for the global button image array
void CleanupButtonImages(void) {
	int16 i;

	// deallocates the images in the array and the arrays themselves
	unloadImageRes(aggressImag, numBtnImages);
//	unloadImageRes( jumpImag   , numBtnImages );
	unloadImageRes(centerImag, numBtnImages);
	unloadImageRes(bandingImag, numBtnImages);
	unloadImageRes(menConBtnImag, numBtnImages);


	// dealloc the imag for the option button
	unloadImageRes(optBtnImag, numBtnImages);

	// dealloc brother's indiv mode buttons
	unloadImageRes(julBtnImag, numBtnImages);
	unloadImageRes(phiBtnImag, numBtnImages);
	unloadImageRes(kevBtnImag, numBtnImages);


	// portraits
	for (i = 0; i < kNumViews; i++) {
		unloadImageRes(portImag[i], numPortImages);
	}

	// name plate frame
	ImageCache.releaseImage(namePlateFrameImag);
	ImageCache.releaseImage(armorImag);

	// release name frames
	ImageCache.releaseImage(broBtnFrameImag);

	// name plates
	for (i = 0; i < kNumViews; i++) {
		ImageCache.releaseImage(namePlateImages[i]);
	}
}

void updateIndicators(void) {
	HealthIndicator->update();
	MassWeightIndicator->update();

	// mana indicator update check
	if (isIndivMode()) {
		if (ManaIndicator->needUpdate(&playerList[getCenterActorPlayerID()])) {
			// redraw the region that is not covered by any other window
			ManaIndicator->invalidate();
		}
	}
}


//>>>
/* ===================================================================== *
   These should become methods of the playerActor class
 * ===================================================================== */

template< class T >
inline T GetRatio(T curUnits, T maxUnits, T ratio) {
	uint16 delt;

	if (maxUnits < ratio) {
		delt = ratio / maxUnits;

		curUnits *= delt;
	} else {
		delt = maxUnits / ratio;

		curUnits /= delt;
	}

	return clamp(0, curUnits, ratio);
}

uint16 getWeightRatio(GameObject *obj, uint16 &maxRatio, bool bReturnMaxRatio = TRUE) {
	assert(isObject(obj) || isActor(obj));

	uint16 weight;
	uint16 maxWeight;

	// get the mass capacity for this container
	maxWeight = obj->massCapacity();;

	// get the total mass this container is holding
	weight = obj->totalContainedMass();


	if (bReturnMaxRatio) {
		maxRatio = maxWeight;
		return weight;
	} else {
		return  maxWeight != unlimitedCapacity
		        ?   GetRatio(weight, maxWeight, maxRatio)
		        :   0;
	}
}


uint16 getBulkRatio(GameObject *obj, uint16 &maxRatio, bool bReturnMaxRatio = TRUE) {
	assert(isObject(obj) || isActor(obj));

	uint16 maxBulk;
	uint16 bulk;

	//  get the bulk capacity for this container
	maxBulk = obj->bulkCapacity();

	// get the total bulk this container is holding
	bulk = obj->totalContainedBulk();

	if (bReturnMaxRatio) {
		maxRatio = maxBulk;
		return bulk;
	} else {
		return  maxBulk != unlimitedCapacity
		        ?   GetRatio(bulk, maxBulk, maxRatio)
		        :   0;
	}
}

/* ===================================================================== *
   Application functions
 * ===================================================================== */

void updateReadyContainers(void) {
	// if in individual mode
	if (indivControlsFlag) {
		indivCviewTop->invalidate();
		indivCviewBot->invalidate();
	} else if (TrioCviews[getCenterActorPlayerID()]) {
		TrioCviews[getCenterActorPlayerID()]->invalidate();
	}
}

void setEnchantmentDisplay(void) {
	if (enchDisp) enchDisp->setValue(getCenterActorPlayerID());
}

// sets the individual brother control state buttons
void setIndivBtns(uint16 brotherID) {    // top = 0, mid = 1, bot = 2
	indivControlsFlag = TRUE;

	// set the indiv bro
	indivBrother = brotherID;

	// set all the individual brother buttons to the correct states
	indivCenterBtn->select(centerBtns[brotherID]->isSelected());
	indivCenterBtn->ghost(centerBtns[brotherID]->isGhosted());
	//indivStarBtn->setCurrent( ( uint16 )starBtns[brotherID]->getCurrent() );
	indivPortBtn->setImages(portImag[brotherID]);
	indivNamePlate->setImage(namePlateImages[brotherID]);
	Portrait->set(uiIndiv, Portrait->getCurrentState(brotherID));
	indivBandingBtn->select(bandingBtns[brotherID]->isSelected());
	indivBandingBtn->ghost(bandingBtns[brotherID]->isGhosted());
	indivAggressBtn->select(aggressBtns[brotherID]->isSelected());
	indivAggressBtn->ghost(aggressBtns[brotherID]->isGhosted());
	indivArmorInd->setValue(brotherID);
	indivArmorInd->ghost(armorInd[brotherID]->isGhosted());
	setEnchantmentDisplay();

	// point the read containers to the correct brother
	if (brotherID > playerActors) brotherID = playerActors;

	indivCviewTop->setContainer(GameObject::objectAddress(ActorBaseID + brotherID));
	indivCviewTop->ghost(TrioCviews[brotherID]->isGhosted());
	indivCviewBot->setContainer(GameObject::objectAddress(ActorBaseID + brotherID));
	indivCviewBot->ghost(TrioCviews[brotherID]->isGhosted());

	// now set the indicators for mass and bulk
	uint16 pieWeightRatio   = MassWeightIndicator->getMassPieDiv();
	uint16 pieBulkRatio     = MassWeightIndicator->getBulkPieDiv();
	PlayerActor *brother    = &playerList[brotherID];

	MassWeightIndicator->setMassPie(getWeightRatio(brother->getActor(), pieWeightRatio, FALSE));
	MassWeightIndicator->setBulkPie(getBulkRatio(brother->getActor(), pieBulkRatio, FALSE));
}

// sets the trio brothers control state buttons
void setTrioBtns(void) {
	indivControlsFlag = FALSE;

	// reset any value that might have changed in idividual mode
	centerBtns[indivBrother]->select(indivCenterBtn->isSelected());
	bandingBtns[indivBrother]->select(indivBandingBtn->isSelected());
	Portrait->set(indivBrother, Portrait->getCurrentState(uiIndiv));
	aggressBtns[indivBrother]->select(indivAggressBtn->isSelected());

	armorInd[indivBrother]->setValue(indivBrother);
	setEnchantmentDisplay();
}

void setControlPanelsToIndividualMode(uint16 brotherID) {
	// copy the button/indicator states to the indiv buttons
	setIndivBtns(brotherID);

	// set the mode controls
	trioControls->show(FALSE, FALSE);
	indivControls->show(TRUE, TRUE);
	trioControls->show(FALSE, TRUE);
}

void setControlPanelsToTrioMode(void) {
	setTrioBtns();
	indivControls->show(FALSE, FALSE);
	trioControls->show(TRUE, TRUE);
	indivControls->show(FALSE, TRUE);
}

void toggleIndivMode(void) {
	if (indivControlsFlag) setControlPanelsToTrioMode();
	else setControlPanelsToIndividualMode(getCenterActorPlayerID());
}

void setCenterBrother(uint16 whichBrother) {
	//  If we picked up anything, then put it back.
	mouseInfo.replaceObject();

	// set the new center actor
	setCenterActor(&playerList[whichBrother]);
}

uint16 translatePanID(uint16 panID) {
	// individual mode brother id translation
	if (panID == uiIndiv) {
		panID = indivBrother;
	}

	return panID;
}

void updateBrotherPortrait(uint16 brotherID, int16 pType) {
	if (userControlsSetup) {
		Portrait->set(brotherID, (PortraitType)pType);

		if (brotherID == indivBrother)
			Portrait->set(uiIndiv, (PortraitType)pType);
	}
}

void updateBrotherAggressionButton(uint16 brotherID, bool aggressive) {
	if (userControlsSetup) {
		aggressBtns[brotherID]->select(aggressive);
		aggressBtns[brotherID]->ghost(isBrotherDead(brotherID));

		if (brotherID == indivBrother) {
			indivAggressBtn->select(aggressive);
			indivAggressBtn->ghost(isBrotherDead(brotherID));
		}

		//  possibly change portrait type
		recalcPortraitType(brotherID);
	}
}

void updateBrotherBandingButton(uint16 brotherID, bool banded) {
	if (userControlsSetup) {
		bandingBtns[brotherID]->select(banded);
		bandingBtns[brotherID]->ghost(isBrotherDead(brotherID));

		if (brotherID == indivBrother) {
			indivBandingBtn->select(banded);
			indivBandingBtn->ghost(isBrotherDead(brotherID));
		}
	}
}

void updateBrotherRadioButtons(uint16 brotherID) {
	if (userControlsSetup) {
		bool    jul = (uiJulian == brotherID);
		bool    phi = (uiPhillip == brotherID);
		bool    kev = (uiKevin == brotherID);

		// set the selection buttons to the correct states
		julBtn->select(jul);
		phiBtn->select(phi);
		kevBtn->select(kev);

		julBtn->ghost(isBrotherDead(uiJulian));
		phiBtn->ghost(isBrotherDead(uiPhillip));
		kevBtn->ghost(isBrotherDead(uiKevin));

		// set the center brother buttons
		centerBtns[uiJulian]->select(jul);
		centerBtns[uiPhillip]->select(phi);
		centerBtns[uiKevin]->select(kev);

		centerBtns[uiJulian]->ghost(isBrotherDead(uiJulian));
		centerBtns[uiPhillip]->ghost(isBrotherDead(uiPhillip));
		centerBtns[uiKevin]->ghost(isBrotherDead(uiKevin));

		if (brotherID == indivBrother) {
			indivCenterBtn->select(TRUE);
			indivCenterBtn->ghost(isBrotherDead(brotherID));
		}

		if (indivControlsFlag)
			setControlPanelsToIndividualMode(brotherID);
	}
}

void updateBrotherArmor(uint16 brotherID) {
	if (userControlsSetup) {
		armorInd[brotherID]->setValue(brotherID);
		armorInd[brotherID]->ghost(isBrotherDead(brotherID));

		if (brotherID == indivBrother) {
			indivArmorInd->setValue(brotherID);
			indivArmorInd->ghost(isBrotherDead(brotherID));
		}
	}
}

void updateAllUserControls(void) {
	if (displayEnabled()) {
		if (userControlsSetup) {
			uint16      centerBrotherID = getCenterActorPlayerID(),
			            brotherID;

			if (indivControlsFlag)
				setControlPanelsToIndividualMode(indivBrother);
			else
				setControlPanelsToTrioMode();

			updateBrotherRadioButtons(centerBrotherID);
			for (brotherID = 0; brotherID < kNumViews; brotherID++) {
				bool    dead = isBrotherDead(brotherID);

				updateBrotherBandingButton(brotherID, isBanded(brotherID));
				updateBrotherAggressionButton(brotherID, isAggressive(brotherID));
				updateBrotherPortrait(brotherID, getPortraitType(brotherID));
				updateBrotherArmor(brotherID);

				// if in individual mode, ghost containers if he's dead
				if (brotherID == indivBrother) {
					indivCviewTop->ghost(dead);
					indivCviewBot->ghost(dead);
				}

				//  Ghost the ready container if he's dead.
				TrioCviews[brotherID]->ghost(dead);
			}
		}
	} else {
		reDrawScreen();
	}
}

void updateBrotherControls(PlayerActorID brotherID) {
	if (userControlsSetup) {
		bool    dead = isBrotherDead(brotherID);

		updateBrotherRadioButtons(getCenterActorPlayerID());
		updateBrotherBandingButton(brotherID, isBanded(brotherID));
		updateBrotherAggressionButton(brotherID, isAggressive(brotherID));
		updateBrotherPortrait(brotherID, getPortraitType(brotherID));
		updateBrotherArmor(brotherID);

		// if in individual mode, ghost containers if he's dead
		if (brotherID == indivBrother) {
			indivCviewTop->ghost(dead);
			indivCviewBot->ghost(dead);
		}

		//  Ghost the ready container if he's dead.
		TrioCviews[brotherID]->ghost(dead);
	}
}

// button call backs
APPFUNC(cmdPortrait) {
	const int bufSize = 80;
	const int stateBufSize = 60;

	uint16  panID = ev.panel->id;
	GameObject      *mouseObject = mouseInfo.getObject();   // object being dragged

	switch (ev.eventType) {

	case gEventNewValue:

		if (mouseObject != nullptr) {
			PlayerActor     *pa = getPlayerActorAddress(translatePanID(panID));
			Actor           *centerActorPtr = getCenterActor();

			//  we dropped the object onto another object
			if (mouseInfo.getDoable()) {
				int16   intent = mouseInfo.getIntent();

				mouseInfo.replaceObject();
				if (intent == GrabInfo::Use) {
					//  If we are using an intangible object (spell) then consider
					//  the owner of the spell to be the center actor for the rest
					//  of this action.
					if (mouseObject->proto()->containmentSet() & ProtoObj::isIntangible) {
						ObjectID    possessor = mouseObject->possessor();

						if (possessor != Nothing) {
							centerActorPtr = (Actor *)GameObject::objectAddress(possessor);
						}
					}

					MotionTask::useObjectOnObject(
					    *centerActorPtr,
					    *mouseObject,
					    *pa->getActor());
				} else if (intent == GrabInfo::Drop) {
					MotionTask::dropObjectOnObject(
					    *centerActorPtr,
					    *mouseObject,
					    *pa->getActor(),
					    mouseInfo.getMoveCount());
				}

//				( ( gGenericControl * )ev.panel )->disableDblClick();
				//  clickActionDone = TRUE;
			} else if (mouseInfo.getIntent() == GrabInfo::Use) {
				mouseInfo.replaceObject();
//				clickActionDone = TRUE;
			}
		} else if (panID != uiIndiv) {
			if (!isBrotherDead(panID)) {
				setCenterBrother(panID);
				setControlPanelsToIndividualMode(panID);
			}
		} else if (panID == uiIndiv) {
			setControlPanelsToTrioMode();
		}
		break;

	case gEventMouseMove:

		if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
			mouseInfo.setDoable(TRUE);
			break;
		}

//		if (ev.value == gCompImage::enter)
		{
			if (mouseObject != nullptr) {
				PlayerActor     *pa = getPlayerActorAddress(translatePanID(panID));
				Actor           *targetActor = pa->getActor(),
				                 *enactor = getCenterActor();

				mouseInfo.setText(nullptr);

				if ((enactor->getLocation() - targetActor->getLocation()).quickHDistance() > 96) {
					mouseInfo.setDoable(FALSE);
				} else {
					mouseInfo.setDoable(TRUE);
				}
			} else {

				// working buffer
				char buf[bufSize];
				char state[stateBufSize];
				uint16 brotherID = translatePanID(panID);

				Portrait->getStateString(state, stateBufSize, brotherID);

				switch (brotherID) {
				case uiJulian:
					sprintf(buf, "%s %s", JULIAN_BROSTATE, state);
					break;
				case uiPhillip:
					sprintf(buf, "%s %s", PHILLIP_BROSTATE, state);
					break;
				case uiKevin:
					sprintf(buf, "%s %s", KEVIN_BROSTATE, state);
					break;
				}
				// set the text in the cursor
				mouseInfo.setText(buf);
			}
		}
		break;
	}
}

void toggleAgression(PlayerActorID bro, bool all) {
	int16   wasAggressive = isAggressive(bro);

	if (all) {
		for (int i = 0; i < playerActors; i++)
			setAggression(i, !wasAggressive);
	} else setAggression(bro, !wasAggressive);
}

APPFUNC(cmdAggressive) {
	uint16 transBroID = translatePanID(ev.panel->id);

	// check for message update stuff
	// and aggression update
	if (ev.eventType == gEventNewValue) {
		toggleAgression(transBroID, rightButtonState());
//		int16   wasAggressive = isAggressive( transBroID );

//		if (rightButtonState())
//		{
//			for (int i = 0; i < playerActors; i++)
//				setAggression( i, !wasAggressive );
//		}
//		else setAggression( transBroID, !wasAggressive );
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter) {
			// set the text in the cursor
			warning("STUB: APPFUNC(cmdAggressive)");
			//mouseInfo.setText(isAggressive(transBroID)
			//                  ? ON_AGRESS
			//                  : OFF_AGRESS);
		} else if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
		}
	}
}

/*
APPFUNC( cmdJump )
{
    if( ev.eventType == gEventNewValue )
    {
        jump = !jump;

        if( jump )
        {
            setMouseImage( CloseBx1Image, 0, 0 );
        }
        else
        {
            setMouseImage( ArrowImage, 0, 0 );
        }
    }
}
*/

APPFUNC(cmdArmor) {
	if (ev.eventType == gEventMouseMove) {

		if (ev.value == gCompImage::enter) {
			gArmorIndicator *gai = (gArmorIndicator *)ev.panel;
			char    buf[128];

			if (gai->attr.damageAbsorbtion == 0
			        &&  gai->attr.defenseBonus == 0) {
				mouseInfo.setText(NO_ARMOR);
			} else {
				sprintf(buf,
				        DESC_ARMOR,
				        gai->attr.damageAbsorbtion,
				        gai->attr.damageDivider,
				        gai->attr.defenseBonus);

				// set the text in the cursor
				mouseInfo.setText(buf);
			}
		} else if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
		}
	}
}

APPFUNC(cmdCenter) {
	uint16 transBroID = translatePanID(ev.panel->id);

	if (ev.eventType == gEventNewValue) {
		if (rightButtonState())
			setCenterBrother((transBroID + 1) % playerActors);
		else setCenterBrother(transBroID);
	}
	if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter) {
			// set the text in the cursor
			warning("STUB: APPFUNC(cmdCenter)");
			//mouseInfo.setText(getCenterActorPlayerID() == transBroID
			//                  ? ON_CENTER
			//                  : OFF_CENTER);
		} else if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
		}
	}
}

void toggleBanding(PlayerActorID bro, bool all) {
	int16   wasBanded = isBanded(bro);

	if (all) {
		for (int i = 0; i < playerActors; i++)
			setBanded(i, !wasBanded);
	} else setBanded(bro, !wasBanded);
}

APPFUNC(cmdBand) {
	uint16 transBroID = translatePanID(ev.panel->id);

	if (ev.eventType == gEventNewValue) {
		toggleBanding(transBroID, rightButtonState());
//		int16   wasBanded = isBanded( transBroID );
//
//		if (rightButtonState())
//		{
//			for (int i = 0; i < playerActors; i++)
//				setBanded( i, !wasBanded );
//		}
//		else setBanded( transBroID, !wasBanded );
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter) {
			// set the text in the cursor
			warning("STUB: APPFUNC(cmdBand)");
			//mouseInfo.setText(isBanded(transBroID)
			//                  ? ON_BANDED
			//                  : OFF_BANDED);
		} else if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
		}
	}
}

APPFUNC(cmdOptions) {
	if (ev.eventType == gEventNewValue) {
		OptionsDialog();
		//openOptionsPanel();
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter)        mouseInfo.setText(OPTIONS_PANEL);
		else if (ev.value == gCompImage::leave) mouseInfo.setText(nullptr);
	}
}

APPFUNC(cmdBroChange) {
	if (ev.eventType == gEventNewValue) {
		if (!isBrotherDead(ev.panel->id)) {
			setCenterBrother(ev.panel->id);
			// this sets up the buttons in trio mode to the correct
			// state ( must be called before indiv mode switchtes )
			setTrioBtns();
			setControlPanelsToIndividualMode(ev.panel->id);
		}
	} else if (ev.eventType == gEventMouseMove) {
		const int bufSize = 80;
		const int stateBufSize = 60;

		uint16  panID = ev.panel->id;

		if (ev.value == gCompImage::enter) {
			// working buffer
			char buf[bufSize];
			char state[stateBufSize];
			uint16 brotherID = translatePanID(panID);

			Portrait->getStateString(state, stateBufSize, brotherID);

			switch (brotherID) {
			case uiJulian:
				sprintf(buf, "%s %s", JULIAN_BROSTATE, state);
				break;
			case uiPhillip:
				sprintf(buf, "%s %s", PHILLIP_BROSTATE, state);
				break;
			case uiKevin:
				sprintf(buf, "%s %s", KEVIN_BROSTATE, state);
				break;
			}
			// set the text in the cursor
			mouseInfo.setText(buf);
		} else if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
		}
	}
}

APPFUNC(cmdHealthStar) {
	uint16 transBroID = translatePanID(ev.panel->id);

	if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
			return;
		}

		if (ev.value == gCompImage::enter) {
			ev.panel->setMousePoll(TRUE);
		}

		// get the stats for the selected brother
		int16 baseVitality = playerList[transBroID].getBaseStats().vitality;
		int16 currVitality = playerList[transBroID].getEffStats()->vitality;

		char buf[40];

		sprintf(buf, "%s %d/%d", HEALTH_HINT, currVitality, baseVitality);
		mouseInfo.setText(buf);
	}
}

APPFUNC(cmdMassInd) {
	gWindow         *win = nullptr;
	GameObject      *containerObject = nullptr;

	if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter) {
			const   int bufSize     = 40;
			int     curWeight;
			uint16  baseWeight;
			char    buf[bufSize];

			win = ev.panel->getWindow();        // get the window pointer

			assert(win);

			// is it something other than the brother's indicators?
			if (ev.panel->id > 1) {
				containerObject = (GameObject *)win->userData;
			} else {
				containerObject = (GameObject *)playerList[getCenterActorPlayerID()].getActor();
			}

			assert(containerObject);

			curWeight = getWeightRatio(containerObject, baseWeight);

			if (baseWeight != unlimitedCapacity) {
				sprintf(buf, "%s %d/%d", WEIGHT_HINT, curWeight, baseWeight);
				mouseInfo.setText(buf);
			} else
				mouseInfo.setText(UNK_WEIGHT_HINT);
		} else if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
		}
	}
}

APPFUNC(cmdBulkInd) {
	gWindow         *win = nullptr;
	GameObject      *containerObject = nullptr;


	if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter) {
			const   int bufSize     = 40;
			uint16  baseBulk    = 100;
			char    buf[bufSize];
			int     curBulk;

			win = ev.panel->getWindow();        // get the window pointer

			assert(win);

			// is it something other than the brother's indicators?
			if (ev.panel->id > 1) {
				containerObject = (GameObject *)win->userData;
			} else {
				containerObject = (GameObject *)playerList[getCenterActorPlayerID()].getActor();
			}

			assert(containerObject);

			curBulk = getBulkRatio(containerObject, baseBulk);

			if (baseBulk != unlimitedCapacity) {
				sprintf(buf, "%s %d/%d", BULK_HINT, curBulk, baseBulk);
				mouseInfo.setText(buf);
			} else
				mouseInfo.setText(UNK_BULK_HINT);
		} else if (ev.value == gCompImage::leave) {
			mouseInfo.setText(nullptr);
		}
	}
}

APPFUNC(cmdManaInd) {
	if (ev.eventType == gEventMouseMove) {
		if (ev.value != gCompImage::leave) {
			const   int BUF_SIZE = 64;
			char    textBuffer[BUF_SIZE];
			int     manaType = -1;
			int     numManaRegions = ManaIndicator->getNumManaRegions();
			int     i;
			int     curMana, baseMana;
			PlayerActor *player             = &playerList[getCenterActorPlayerID()];
			ActorAttributes *stats          = player->getEffStats();
			ActorAttributes baseStatsRef    = player->getBaseStats();
			Point16 pos = ev.mouse;

			pos.x += ManaIndicator->getExtent().x;
			pos.y += ManaIndicator->getExtent().y;

			for (i = 0; i < numManaRegions; i++) {
				Rect16 regionRect = ManaIndicator->getManaRegionRect(i);

				if (regionRect.ptInside(pos)) {
					manaType = i;
					break;
				}
			}

			if (manaType != -1) {
				curMana     = stats->mana(clamp(0, manaType, numManaRegions));
				baseMana    = baseStatsRef.mana(clamp(0, manaType, numManaRegions));
			}

			switch (manaType) {
#define RED_MANA            "Red Mana:"
#define ORANGE_MANA         "Orange Mana:"



#define YELLOW_MANA         "Yellow Mana:"



#define GREEN_MANA          "Green Mana:"



#define BLUE_MANA           "Blue Mana:"



#define VIOLET_MANA         "Purple Mana:"
			case 0:
				sprintf(textBuffer, "%s %d/%d", RED_MANA, curMana, baseMana);
				break;

			case 1:
				sprintf(textBuffer, "%s %d/%d", ORANGE_MANA, curMana, baseMana);
				break;

			case 2:
				sprintf(textBuffer, "%s %d/%d", YELLOW_MANA, curMana, baseMana);
				break;

			case 3:
				sprintf(textBuffer, "%s %d/%d", GREEN_MANA, curMana, baseMana);
				break;

			case 4:
				sprintf(textBuffer, "%s %d/%d", BLUE_MANA, curMana, baseMana);
				break;

			case 5:
				sprintf(textBuffer, "%s %d/%d", VIOLET_MANA, curMana, baseMana);
				break;

			case -1:
				sprintf(textBuffer, "\0");
				break;

			default:
				assert(FALSE);   // should never get here
				break;
			}

			// set the text in the cursor
			mouseInfo.setText(textBuffer);
		} else
			mouseInfo.setText(nullptr);
	}
}

struct UIStateArchive {
	bool    indivControlsFlag;
	uint16  indivBrother;
};

bool isIndivMode(void) {
	return indivControlsFlag;
}

void initUIState(void) {
	indivControlsFlag = FALSE;
	indivBrother = 0;

	//updateAllUserControls();
}

void saveUIState(SaveFileConstructor &saveGame) {
	UIStateArchive      archive;

	archive.indivControlsFlag = indivControlsFlag;
	archive.indivBrother = indivBrother;

	saveGame.writeChunk(
	    MakeID('U', 'I', 'S', 'T'),
	    &archive,
	    sizeof(archive));
}

void loadUIState(SaveFileReader &saveGame) {
	UIStateArchive      archive;

	saveGame.read(&archive, sizeof(archive));

	indivControlsFlag = archive.indivControlsFlag;
	indivBrother = archive.indivBrother;

	updateAllUserControls();
}

void cleanupUIState(void) {
	if (StatusLine != nullptr)
		StatusLine->clear();
}

void gArmorIndicator::setValue(PlayerActorID brotherID) {
	Actor *bro = playerList[brotherID].getActor();
	bro->totalArmorAttributes(attr);
	invalidate();
}

// getCurrentCompImage() is virtual function that should return
// the current image to be displayed (to be used across all sub-classes)
void gArmorIndicator::drawClipped(gPort &port,
                                  const Point16 &offset,
                                  const Rect16 &r) {
	if (!extent.overlap(r))    return;

	SAVE_GPORT_STATE(port);

	// get the current image
	void *dispImage = getCurrentCompImage();

	// make sure the image is valid
	if (dispImage) {
		// will part of this be drawn on screen?
		if (extent.overlap(r)) {
			char buf[8];

			// offset the image?
			Point16 pos(extent.x - offset.x,
			            extent.y - offset.y
			           );
			// draw the compressed image
			if (isGhosted()) {
				drawCompressedImageGhosted(port, pos, dispImage);
				return;
			} else drawCompressedImage(port, pos, dispImage);

			// draw the armor numebrs
			port.setFont(&Helv11Font);
			port.setColor(11);                   // set color to white
			port.setStyle(textStyleThickOutline);
			port.setOutlineColor(24);                // set outline color to black
			port.setMode(drawModeMatte);

			if (attr.damageAbsorbtion == 0 && attr.defenseBonus == 0)
				sprintf(buf, "-");
			else if (attr.damageDivider > 1)
				sprintf(buf, "%d/%d", attr.damageAbsorbtion, attr.damageDivider);
			else sprintf(buf, "%d", attr.damageAbsorbtion);

			port.drawTextInBox(buf, -1, Rect16(pos.x, pos.y, extent.width, extent.height),
			                   textPosRight | textPosHigh, Point16(0,  2));

			if (attr.damageAbsorbtion == 0 && attr.defenseBonus == 0)
				sprintf(buf, "-");
			else sprintf(buf, "%d", attr.defenseBonus);
			port.drawTextInBox(buf, -1, Rect16(pos.x, pos.y, extent.width, extent.height),
			                   textPosRight | textPosLow, Point16(0,  2));
		}
	}
}

void gEnchantmentDisplay::drawClipped(gPort &port, const    Point16 &offset, const Rect16 &r) {
	Point16     pos(extent.x + extent.width - 10, extent.y + 1);

	pos += offset;

	if (!extent.overlap(r)) return;

	for (int i = 0; i < iconCount; i++) {
		if (iconFlags[i]) {
			Sprite      *sp = (*mentalSprites)->sprite(i + 162);

			pos.x -= sp->size.x + 2;
			DrawSprite(port, pos, sp);
		}
	}
}

void gEnchantmentDisplay::pointerMove(gPanelMessage &msg) {
	if (msg.pointerLeave) {
		mouseInfo.setText(nullptr);
	} else {
		int16       x = extent.width - 10;

		setMousePoll(TRUE);
		setValue(getCenterActorPlayerID());

		for (int i = 0; i < iconCount; i++) {
			if (iconFlags[i]) {
				Sprite      *sp = (*mentalSprites)->sprite(i + 162);

				x -= sp->size.x + 2;
				if (msg.pickPos.x >= x) {
					// set the text in the cursor
					char    buf[128];

					if (iconFlags[i] == 255)
						sprintf(buf, "%s", enchantmentNames[i]);
					else sprintf(buf, "%s : %d", enchantmentNames[i], iconFlags[i]);
					mouseInfo.setText(buf);
					return;
				}
			}
		}
	}
}

void gEnchantmentDisplay::setValue(PlayerActorID pID) {
	Actor           *a = playerList[pID].getActor();
	uint8           newIconFlags[iconCount];
	GameObject      *wornObj;
	EnchantmentIterator iter(a);
	ContainerIterator   cIter(a);

	GameObject          *obj;
	ObjectID            id;

	memset(newIconFlags, 0, sizeof newIconFlags);

	/*
	x   iconHaste,
	    iconFirewalk,
	x   iconAdrenalFervor,
	x   iconShadowWalk,
	    iconSunWard,
	    iconSpellBarrier,
	*/

	for (id = iter.first(&obj); id != Nothing; id = iter.next(&obj)) {
		uint16 enchantmentID = obj->getExtra();
		uint16 eType = getEnchantmentType(enchantmentID);
		uint16 eSubType = getEnchantmentSubType(enchantmentID);
		int16  eAmount = getEnchantmentAmount(enchantmentID);
		uint8   duration = obj->getHitPoints();             // get hitpoints of enchant

		switch (eType) {
		case effectAttrib:
			switch (eSubType) {
//			case skillIDArchery:
//			case skillIDShieldcraft:

			case skillIDBludgeon:
			case skillIDSwordcraft:
				if (eAmount > 0) newIconFlags[iconSurestrike] = duration;
				else newIconFlags[iconClumsy] = duration;
				break;

			case skillIDAgility:
				if (eAmount > 0) newIconFlags[iconInnerBalance] = duration;
				else newIconFlags[iconClumsy] = duration;
				break;

			case skillIDBrawn:
				newIconFlags[iconBattleFever] = duration;
				break;
			}
			break;

		case effectResist:

			switch (eSubType) {
			case damageImpact:
				newIconFlags[iconResistImpact] = duration;
				break;
			case damageSlash:
				newIconFlags[iconResistSlash] = duration;
				break;
			case damageProjectile:
				newIconFlags[iconResistProjectile] = duration;
				break;
			case damageFire:
				newIconFlags[iconResistFire] = duration;
				break;
			case damageAcid:
				newIconFlags[iconResistAcid] = duration;
				break;
			case damageHeat:
				newIconFlags[iconResistHeat] = duration;
				break;
			case damageCold:
				newIconFlags[iconResistCold] = duration;
				break;
			case damageLightning:
				newIconFlags[iconResistLightning] = duration;
				break;
			case damagePoison:
				newIconFlags[iconResistPoison] = duration;
				break;
			case damageMental:
				newIconFlags[iconResistPsionic] = duration;
				break;
			case damageDirMagic:
				newIconFlags[iconResistDirectMagic] = duration;
				break;
			}
			break;

		case effectImmune:
			switch (eSubType) {
			case damageImpact:
				newIconFlags[iconIronskin] = duration;
				break;
			case damageSlash:
				newIconFlags[iconIronskin] = duration;
				break;
			case damageFire:
				newIconFlags[iconImmuneFire] = duration;
				break;
			case damageAcid:
				newIconFlags[iconImmuneAcid] = duration;
				break;
			case damageHeat:
				newIconFlags[iconImmuneHeat] = duration;
				break;
			case damageCold:
				newIconFlags[iconImmuneCold] = duration;
				break;
			case damageLightning:
				newIconFlags[iconImmuneLightning] = duration;
				break;
			case damagePoison:
				newIconFlags[iconImmunePoison] = duration;
				break;
			case damageMental:
				newIconFlags[iconImmunePsionic] = duration;
				break;
			}
			break;

		case effectOthers:
			switch (eSubType) {
			case actorPoisoned:
			case actorDiseased:
				newIconFlags[iconPoisoned] = duration;
				break;

			case actorFear:
				newIconFlags[iconAfraid] = duration;
				break;
			case actorParalyzed:
				newIconFlags[iconParalysed] = duration;
				break; // iconFrozen ??
			case actorSlowFall:
				newIconFlags[iconCushionAir] = duration;
				break;
			case actorImmobile:
				newIconFlags[iconConstrained] = duration;
				break;
			case actorSeeInvis:
				newIconFlags[iconSoulSight] = duration;
				break;
			case actorInvisible:
				newIconFlags[iconInvisible] = duration;
				break;
			case actorUndetectable:
				newIconFlags[iconNumbscent] = duration;
				break;
			case actorDetPoison:
				newIconFlags[iconDetectPoison] = duration;
				break;
			case actorNoDrain:
				newIconFlags[iconNetherWard] = duration;
				break;
			case actorWaterBreathe:
				newIconFlags[iconSeawalk] = duration;
				break;
			case actorRepelEvil:
				newIconFlags[iconProtectEvil] = duration;
				break;
//			case actorRepelUndead: newIconFlags[iconProtectUndead] = duration; break;
//			case actorRepelGhosts: newIconFlags[iconProtectGhosts] = duration; break;
			}
		}
	}

	while (cIter.next(&obj)) {
		ProtoObj        *proto = obj->proto();
		uint16          cSet = proto->containmentSet();

		if ((cSet & (ProtoObj::isArmor | ProtoObj::isWeapon | ProtoObj::isWearable))
		        &&  proto->isObjectBeingUsed(obj)) {
			if (proto->immunity & (1 << resistImpact))            newIconFlags[iconResistImpact] = 255;
			else if (proto->resistance & (1 << resistImpact)) newIconFlags[iconResistImpact] = 255;

			if (proto->immunity & (1 << resistSlash))         newIconFlags[iconResistSlash] = 255;
			else if (proto->resistance & (1 << resistSlash))  newIconFlags[iconResistSlash] = 255;

			if (proto->immunity & (1 << resistProjectile))        newIconFlags[iconResistProjectile] = 255;
			else if (proto->resistance & (1 << resistProjectile)) newIconFlags[iconResistProjectile] = 255;

			if (proto->immunity & (1 << immuneFire))          newIconFlags[iconImmuneFire] = 255;
			else if (proto->resistance & (1 << resistFire))   newIconFlags[iconResistFire] = 255;

			if (proto->immunity & (1 << immuneAcid))          newIconFlags[iconImmuneAcid] = 255;
			else if (proto->resistance & (1 << resistAcid))   newIconFlags[iconResistAcid] = 255;

			if (proto->immunity & (1 << immuneHeat))          newIconFlags[iconImmuneHeat] = 255;
			else if (proto->resistance & (1 << resistHeat))   newIconFlags[iconResistHeat] = 255;

			if (proto->immunity & (1 << immuneCold))          newIconFlags[iconImmuneCold] = 255;
			else if (proto->resistance & (1 << resistCold))   newIconFlags[iconResistCold] = 255;

			if (proto->immunity & (1 << immuneLightning))     newIconFlags[iconImmuneLightning] = 255;
			else if (proto->resistance & (1 << resistLightning)) newIconFlags[iconResistLightning] = 255;

			if (proto->immunity & (1 << immunePoison))        newIconFlags[iconImmunePoison] = 255;
			else if (proto->resistance & (1 << resistPoison)) newIconFlags[iconResistPoison] = 255;

			if (proto->immunity & (1 << immuneMental))        newIconFlags[iconImmunePsionic] = 255;
			else if (proto->resistance & (1 << resistMental)) newIconFlags[iconResistPsionic] = 255;

			if (proto->immunity & (1 << resistDirMagic))      newIconFlags[iconResistDirectMagic] = 255;
			else if (proto->resistance & (1 << resistDirMagic))newIconFlags[iconResistDirectMagic] = 255;
		}
	}

	//  Compute icon flags for resistances and immunities

#if 0
	enum effectOthersTypes {
		// Movement flags
		o   actorNoncorporeal   = 1,    // The creature can walk through things
		x   actorWaterBreathe   = 2,    // death spell
		x   actorSlowFall       = 3,    // the creature is not harmed by falling (but falls none the less)
		-   actorLevitate       = 4,    // flying with no height control ?
		-   actorFlying         = 5,    // the creature flys
		// speed flags
		-   actorFastMove       = 6,    //
		-   actorFastAttack     = 7,    //
		actorSlowAttack     = 8,    // come... back... here... lit... tle... bun... ny...
		x   actorSlowMove       = 9,    // I thought I told you to leave the piano at home
		// ill effects
		-   actorAsleep         = 10,   // Zzzzzzzzzzz
		x   actorParalyzed      = 11,   // the creature can't move an inch
		x   actorFear           = 12,   // run away! run away
		x   actorDiseased       = 13,   // cannot heal
		x   actorPoisoned       = 14,   // death spell
		// perception & perceivability flags
		x   actorBlind          = 15,   // can't see
		x   actorSeeInvis       = 16,   // can see invisible
		-   actorClairvoyant    = 17,  // unknown effects
		x   actorInvisible      = 18,   // is invisible
		x   actorUndetectable   = 19,   // can't be seen, smelled
		x   actorDetPoison      = 20,   // poison things glow green
		// flags preventing changes to other flags
		actorNoEnchant      = 21,   // no bad enchantments
		x   actorNoDrain        = 22,   // no mana / food drains
		// flags that make things run away
		x   actorRepelEvil      = 23,   // death spell
		x   actorRepelGood      = 24,   // death spell
		x   actorRepelUndead    = 25,   // death spell
		// dead or moved flags
//		actorMapping        =15,    //
//		actorLandWalk       =0 ,    // someone else had this I have no idea what it is
//		actorFloat          =2 ,    // the creature can travel through malts shakes & sundaes
		actorWaterWalk,          // can walk on water (same as float ?)
//		actorPanic          =13,    // creature takes off randomly
//		actorSpotHidden     =17,    // can see hidden
//		actorDetTraps       =22,    // traps glow green
//		actorFlameAura      =23,    // has a flaming aura
//		actorDead           =25,    // death spell
	};
#endif

	//  If icon flags changed, then redraw the control.

	if (memcmp(iconFlags, newIconFlags, sizeof iconFlags)) {
		memcpy(iconFlags, newIconFlags, sizeof iconFlags);
		invalidate();
	}
}

bool isBrotherDead(PlayerActorID brotherID) {
	return (getPlayerActorAddress(brotherID)->getActor()->isDead());
}

void StatusMsg(char *msg, ...) { // frametime def
	va_list         argptr;
	char            buffer[128];

	if (StatusLine) {
		va_start(argptr, msg);
		vsprintf(buffer, msg, argptr);
		va_end(argptr);

		StatusLine->setLine(buffer, 500);
	}
}

} // end of namespace Saga2
