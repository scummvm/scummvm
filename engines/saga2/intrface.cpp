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

#include "saga2/saga2.h"
#include "saga2/blitters.h"
#include "saga2/objects.h"
#include "saga2/contain.h"
#include "saga2/intrface.h"
#include "saga2/grabinfo.h"
#include "saga2/uidialog.h"
#include "saga2/motion.h"
#include "saga2/enchant.h"
#include "saga2/display.h"
#include "saga2/localize.h"
#include "saga2/imagcach.h"
#include "saga2/fontlib.h"
#include "saga2/uimetrcs.h"

namespace Saga2 {

/* ===================================================================== *
   External delarations
 * ===================================================================== */
extern ReadyContainerView   *TrioCviews[kNumViews];
extern ReadyContainerView   *indivCviewTop, *indivCviewBot;
extern gPanelList           *trioControls, *indivControls;
extern gPanelList           *playControls;
extern const uint32         imageGroupID;
extern gPanelList           *tileControls;
extern BackWindow           *mainWindow;
extern uint8                fixedColors[16];


/* ===================================================================== *
   Classes
 * ===================================================================== */

//	Private subclass of GfxCompImage for armor display

class gArmorIndicator : public GfxCompImage {
public:
	ArmorAttributes  attr;
	void drawClipped(gPort &,
	                 const   Point16 &,
	                 const   Rect16 &);

	void setValue(PlayerActorID pID);

	gArmorIndicator(gPanelList &list, const Rect16 &box, void *img, uint16 ident, AppFunc *cmd = nullptr)
		: GfxCompImage(list, box, img, ident, cmd) {
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

const char *enchantmentNames[] = {
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
   User control metrics
 * ===================================================================== */

// position arrays for all buttons on the individual panels
static const StaticRect topBox[numButtons] = {
	/* portrait          */ { 489, 22 + (yContOffset * 0), 65, 72 },
	/* agress            */ { 559, 86 + (yContOffset * 0), 28, 27 },
	/* jump              */ { 592, 86 + (yContOffset * 0), 28, 27 },
	/* center            */ { 559, 56 + (yContOffset * 0), 28, 27 },
	/* banding           */ { 592, 56 + (yContOffset * 0), 28, 27 },
	/* namePlates        */ { 488, 94 + (yFaceOffset * 0), 65, 15 },
	/* namePlateFrames   */ { 487, 20 + (yFaceOffset * 0), 69, 92 }
};


static const StaticRect midBox[numButtons] = {
	{ 489, 22 + (yContOffset * 1), 65, 72 },
	{ 559, 86 + (yContOffset * 1), 28, 27 },
	{ 592, 86 + (yContOffset * 1), 28, 27 },
	{ 559, 56 + (yContOffset * 1), 28, 27 },
	{ 592, 56 + (yContOffset * 1), 28, 27 },
	{ 488, 94 + (yFaceOffset * 1), 65, 15 },
	{ 487, 20 + (yFaceOffset * 1), 69, 92 }
};



static const StaticRect botBox[numButtons] = {
	{ 489, 22 + (yContOffset * 2), 65, 72 },
	{ 559, 86 + (yContOffset * 2), 28, 27 },
	{ 592, 86 + (yContOffset * 2), 28, 27 },
	{ 559, 56 + (yContOffset * 2), 28, 27 },
	{ 592, 56 + (yContOffset * 2), 28, 27 },
	{ 488, 94 + (yFaceOffset * 2), 65, 15 },
	{ 487, 20 + (yFaceOffset * 2), 69, 92 }
};



// options button
GfxCompButton         *optBtn;
gEnchantmentDisplay *enchDisp;

// brother buttons
GfxOwnerSelCompButton *julBtn;
GfxOwnerSelCompButton *phiBtn;
GfxOwnerSelCompButton *kevBtn;
GfxCompImage          *broBtnFrame;


// trio controls
GfxMultCompButton     *portBtns[kNumViews];
GfxOwnerSelCompButton *aggressBtns[kNumViews];
//GfxCompButton           *jumpBtns[kNumViews];
GfxOwnerSelCompButton *centerBtns[kNumViews];
GfxOwnerSelCompButton *bandingBtns[kNumViews];
GfxCompImage          *namePlates[kNumViews];
GfxCompImage          *namePlateFrames[kNumViews];
gArmorIndicator     *armorInd[kNumViews];

// individual
GfxMultCompButton     *indivPortBtn;
GfxOwnerSelCompButton *indivAggressBtn;
//GfxCompButton           *indivJumpBtn;
GfxOwnerSelCompButton *indivCenterBtn;
GfxOwnerSelCompButton *indivBandingBtn;
GfxCompImage          *indivNamePlate;
GfxCompImage          *indivNamePlateFrame;
gArmorIndicator     *indivArmorInd;

// mental button/indicators
GfxCompButton         *menConBtn;


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
ResName broNames[] = {
	{ 'J', 'U', 'L', 0 },
	{ 'P', 'H', 'I', 0 },
	{ 'K', 'E', 'V', 0 }
};

const uint32 containerGroupID = MKTAG('C', 'O', 'N', 'T');


// button position views
// topBox, midBox, and botBox are defined in uimetrcs.h
static const StaticRect *views[] = {
	topBox,
	midBox,
	botBox
};

// individual indicators/buttons
static const StaticRect menConBtnRect = {485, 265, 44, 43};

// options button
static const StaticRect optBtnRect = {20, 445, 26, 15};

// brother buttons and frame
static const StaticRect broBtnRect = {481, 450, 144, 11};
static const StaticRect julBtnRect = {482, 451, 44, 9};
static const StaticRect phiBtnRect = {531, 451, 44, 9};
static const StaticRect kevBtnRect = {580, 451, 44, 9};


StaticTextPallete genericTextPal = {9 + 15, 20, 14, 11, 23, 17};
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
   Global class declarations
 * ===================================================================== */

// these objhets have to be initialized after resource are initialized
CStatusLine             *StatusLine          = nullptr;
CMassWeightIndicator    *MassWeightIndicator = nullptr;
CHealthIndicator        *HealthIndicator     = nullptr;
CManaIndicator          *ManaIndicator       = nullptr;

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
	oldFont         = nullptr;
}

void CPlaqText::enable(bool abled) {
	gPanel::enable(abled);
}

void CPlaqText::invalidate(Rect16 *) {
	window.update(_extent);
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

	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer
	drawClipped(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
	g_vm->_pointer->show(port, _extent);              // show mouse pointer

	// reset the old font
	port.setFont(oldFont);
}

void CPlaqText::drawClipped(gPort &port,
                            const Point16 &offset,
                            const Rect16 &r) {
	if (_extent.overlap(r)) {
		if (*lineBuf) {
			textRect = _extent;
			textRect.x -= offset.x;
			textRect.y -= offset.y;


			writePlaqText(port, textRect, buttonFont, textPosition, textFacePal, selected,  lineBuf);
		}
	}
}

/* ===================================================================== *
    Portrait control class
 * ===================================================================== */

CPortrait::CPortrait(GfxMultCompButton **portraits,
                     GfxMultCompButton *indivPort,
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
		currentState[i] = kPortraitNormal;
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
		currentState[brotherID] = kPortraitNormal;
	} else {
		currentState[brotherID] = type;
	}

	// set this button to the new state
	setPortrait(brotherID);
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

	buf[size - 1] = '\0';

	if (a->isDead()) {
		Common::strlcpy(buf, DEAD_STATE, size);
		return;
	}

	buf[0] = '\0';

	if (a->_enchantmentFlags & (1 << actorAsleep)) {
		Common::strlcat(buf, asleepStr, size);
	} else if (a->_enchantmentFlags & (1 << actorParalyzed)) {
		Common::strlcat(buf, paralysedStr, size);
	} else if (a->_enchantmentFlags & (1 << actorBlind)) {
		Common::strlcat(buf, blindStr, size);
	} else if (a->_enchantmentFlags & (1 << actorFear)) {
		Common::strlcat(buf, afraidStr, size);
	} else if (pa->isAggressive()) {
		Common::strlcat(buf, angryStr, size);
	}

	if (stats.vitality >= a->_effectiveStats.vitality * 3) {
		if (buf[0] != '\0')	// strlen(buf) > 0
			Common::strlcat(buf, commaStr, size);

		Common::strlcat(buf, badlyWoundedStr, size);
	} else if (stats.vitality * 2 > a->_effectiveStats.vitality * 3) {
		if (buf[0] != '\0')	// strlen(buf) > 0
			Common::strlcat(buf, commaStr, size);

		Common::strlcat(buf, hurtStr, size);
	}

	if (a->_enchantmentFlags & (1 << actorPoisoned)) {
		if (buf[0] != '\0')	// strlen(buf) > 0
			Common::strlcat(buf, commaStr, size);

		Common::strlcat(buf, poisonedStr, size);
	} else if (a->_enchantmentFlags & (1 << actorDiseased)) {
		if (buf[0] != '\0')	// strlen(buf) > 0
			Common::strlcat(buf, commaStr, size);

		Common::strlcat(buf, diseasedStr, size);
	}

	if (buf[0] == '\0')	// strlen(buf) == 0
		Common::strlcat(buf, normalStr, size);
}

/* ===================================================================== *
   status line gadget
 * ===================================================================== */

// status line class
CStatusLine::CStatusLine(gPanelList         &list,
                         const Rect16    &box,
                         const char            *msg,
                         gFont           *font,
                         int16           textPos,
                         textPallete     pal,
                         int32           /*frameTime*/,
                         int16           ident,
                         AppFunc         *cmd) :
	CPlaqText(list, box, msg, font, textPos, pal, ident, cmd) {

	lineDisplayed = false;
	queueHead = queueTail = 0;

	for (int i = 0; i < ARRAYSIZE(lineQueue); i++) {
		lineQueue[i].text = nullptr;
		lineQueue[i].frameTime = 0;
	}
	waitAlarm.basetime = waitAlarm.duration = 0;
	minWaitAlarm.basetime = minWaitAlarm.duration = 0;
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
		enable(false);
		window.update(_extent);

		lineDisplayed = false;
	}

	if (!lineDisplayed && queueTail != queueHead) {
		// enable the control
		enable(true);

		// set up the time for this message
		waitAlarm.set(lineQueue[queueTail].frameTime);
		minWaitAlarm.set(lineQueue[queueTail].frameTime / 5);

		// copy upto the buffer's size in chars
		Common::strlcpy(lineBuf, lineQueue[queueTail].text, bufSize);
		lineBuf[bufSize - 1] = '\0';

		//  free the queue text buffer
		delete[] lineQueue[queueTail].text;
		lineQueue[queueTail].text = nullptr;

		//  bump the queue tail
		queueTail = bump(queueTail);

		// draw the new textline
		window.update(_extent);

		lineDisplayed = true;
	}
}

void CStatusLine::clear(void) {
	enable(false);
	window.update(_extent);
	lineDisplayed = false;

	queueHead = queueTail = 0;
}

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

	bRedraw         = true; // this MUST be true or the indicators will not draw the first time

	// attach the resource context
	containerRes = resFile->newContext(containerGroupID, "container context");

	// setup mass/bulk indicator imagery
	if (death) {
		massBulkImag = g_vm->_imageCache->requestImage(containerRes, MKTAG('D', 'J', 'B', massBulkResNum));

		pieIndImag = loadImageRes(containerRes, pieIndResNum, numPieIndImages, 'D', 'A', 'J');
	} else {

		massBulkImag = g_vm->_imageCache->requestImage(containerRes, MKTAG('G', 'J', 'B', massBulkResNum));

		pieIndImag = loadImageRes(containerRes, pieIndResNum, numPieIndImages, 'G', 'A', 'J');
	}

	// attach controls to the indivControls panel
	// these butttons will get deactivated along with the panel
	pieMass = new GfxCompImage(*panel,
	                                       Rect16(massPiePos.x, massPiePos.y, pieXSize, pieYSize),
	                                       pieIndImag,
	                                       numPieIndImages,
	                                       0,
	                                       type,
	                                       cmdMassInd);

	pieBulk = new GfxCompImage(*panel,
	                                       Rect16(bulkPiePos.x, bulkPiePos.y, pieXSize, pieYSize),
	                                       pieIndImag,
	                                       numPieIndImages,
	                                       0,
	                                       type,
	                                       cmdBulkInd);

	// mass/bulk back image
	new GfxCompImage(*panel,
	                             Rect16(backImagePos.x, backImagePos.y, backImageXSize, backImageYSize),
	                             massBulkImag,
	                             uiIndiv,
	                             nullptr);

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

	g_vm->_indList.push_back(this);
}

CMassWeightIndicator::~CMassWeightIndicator(void) {
	g_vm->_indList.remove(this);

	unloadImageRes(pieIndImag, numPieIndImages);
	g_vm->_imageCache->releaseImage(massBulkImag);
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
		setMassPie(retMass = getWeightRatio(containerObject, mass, false));
		setBulkPie(retBulk = getBulkRatio(containerObject, bulk, false));
	} else {
		setMassPie(retMass = getWeightRatio(g_vm->_playerList[getCenterActorPlayerID()]->getActor(), mass, false));
		setBulkPie(retBulk = getBulkRatio(g_vm->_playerList[getCenterActorPlayerID()]->getActor(), bulk, false));
	}
}

/*****************************************************************************
** Method: update()
** Description: This will call recalculate and then invalidate the entire
**              weight/bulk control ( so it refreshes )
**/
void CMassWeightIndicator::update(void) {
	if (bRedraw == true) {
		for (Common::List<CMassWeightIndicator *>::iterator it = g_vm->_indList.begin(); it != g_vm->_indList.end(); ++it) {
			(*it)->recalculate();
			(*it)->invalidate();
		}

		bRedraw = false;
	}
}

/* ===================================================================== *
    CManaIndicator: magic stuff indicator
 * ===================================================================== */

// setup the mana color tables
static uint8 manaColorMap[CManaIndicator::numManaTypes][CManaIndicator::numManaColors + 9] = {
	// each row has 9 leading zero's to acount for windows colors
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x74, 0x73, 0x72, 0x80, 0x84, 0x83, 0x82, 0x82, 0x81, 0x81, 0xF4, 0xE9 }, // Red

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x74, 0x73, 0x72, 0x78, 0x77, 0x76, 0x75, 0x6B, 0x6A, 0x69, 0x3A, 0x39 }, // Orange

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x64, 0x63, 0x62, 0x61, 0x60, 0x5F, 0x5E, 0x5D, 0x5C, 0x5B, 0x5A, 0x59 }, // Yellow

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xE8, 0xE7, 0xE6, 0xE5, 0xE4, 0xE3, 0xE2, 0xE1, 0xE0, 0xDF, 0xDE, 0xC9 }, // Green

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xA0, 0x9F, 0x9E, 0xB2, 0xB1, 0xB0, 0xAF, 0xAE, 0xAD, 0xAC, 0xAA, 0xA9 }, // Blue

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xA0, 0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x93, 0x92, 0x92, 0x91, 0x0C }  // Violet
};


CManaIndicator::CManaIndicator(gPanelList &list) : GfxCompImage(list,
	        Rect16(x, y, xSize, ySize),
	        nullptr,
	        0,
	        cmdManaInd) {
	assert(resFile);

	// init the resource handle with the mana resource group
	resContext  = resFile->newContext(MKTAG('M', 'A', 'N', 'A'), "mana context");

	// load star images
	starImages = loadImageRes(resContext, starResNum, numStars, 'S', 'T', 'A');

	// load in the ring images
	ringImages = loadImageRes(resContext, ringResNum, numRings, 'R', 'N', 'G');

	backImage = g_vm->_imageCache->requestImage(resContext, MKTAG('B', 'A', 'C', 'K'));

	wellImage = g_vm->_imageCache->requestImage(resContext, MKTAG('W', 'E', 'L', 'L'));

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
}

CManaIndicator::~CManaIndicator(void) {
	// release images
	unloadImageRes(starImages, numStars);
	unloadImageRes(ringImages, numRings);

	// release back image
	g_vm->_imageCache->releaseImage(backImage);
	g_vm->_imageCache->releaseImage(wellImage);

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

	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer
	drawClipped(port, Point16(0, 0), Rect16(0, 0, xSize, ySize));
	g_vm->_pointer->show(port, _extent);              // show mouse pointer


}


void CManaIndicator::drawClipped(gPort &port,
                                 const Point16 &offset,
                                 const Rect16 &clipRect) {
	bool calcDraw;

	// Do an update to the mana star info if needed,
	// if not, do not draw stuff
	calcDraw = update(g_vm->_playerList[getCenterActorPlayerID()]);

	if (!calcDraw) {
		if (!_extent.overlap(clipRect)) return;

		// draw the saved image to the port
		port.setMode(drawModeMatte);
		port.bltPixels(savedMap, 0, 0,
		               _extent.x - offset.x, _extent.y - offset.y,
		               xSize, ySize);

		// draw the frame
		drawCompressedImage(port, Point16(_extent.x - offset.x, _extent.y - offset.y), backImage);

		// and finish
		return;
	}

	// otherwise continue with the update
	g_vm->_pointer->hide();

	// create a temporary gPort to blit stuff to
	gPort       tempPort;
	gPixelMap   ringMap, starMap, mixMap, tempMap;


	if (!NewTempPort(tempPort, xSize, ySize))
		return;

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

			// if it is then upack it to spec'ed coords.
			unpackImage(&starMap, starMap.size.x, starMap.size.y, starHdr->data);
		} else starMap.data = (uint8 *)starHdr->data;

		// see if it's compressed
		if (ringHdr->compress) {
			// allocation of the temp buffer
			ringMap.data = new uint8[ringMap.bytes()]();

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
	               _extent.x - offset.x, _extent.y - offset.y,
	               xSize, ySize);

	// now blit the frame on top of it all.
	drawCompressedImage(port, Point16(_extent.x - offset.x, _extent.y - offset.y), backImage);

	// dispose of temporary pixelmap
	DisposeTempPort(tempPort);
	if (mixMap.data)
		delete[] mixMap.data;
	if (tempMap.data)
		delete[] tempMap.data;

	g_vm->_pointer->show();
}

bool CManaIndicator::needUpdate(PlayerActor *player) {
	assert(player);

	// get the ability scores for this character
	ActorAttributes *stats          = player->getEffStats();
	ActorAttributes baseStatsRef    = player->getBaseStats();
	int16           manaAmount;
	int16           baseManaAmount;
	uint16          i;

	// this could do more array checking, but
	// the number of mana type should remain stable
	for (i = 0; i < numManaTypes; i++) {
		manaAmount      = stats->mana(i);
		baseManaAmount  = baseStatsRef.mana(i);

		// check for new data
		if (manaAmount != currentMana[i] || baseManaAmount != currentBaseMana[i]) {
			return true;
		}
	}

	return false;
}


bool CManaIndicator::update(PlayerActor *player) {
	assert(player);

	// get the ability scores for this character
	ActorAttributes *stats          = player->getEffStats();
	ActorAttributes baseStatsRef    = player->getBaseStats();
	int16           manaAmount;
	int16           baseManaAmount;
	uint16          i;
	bool            newData = false;

	// this could do more array checking, but
	// the number of mana type should remain stable
	for (i = 0; i < numManaTypes; i++) {
		manaAmount      = stats->mana(i);
		baseManaAmount  = baseStatsRef.mana(i);

		// check for new data
		if (manaAmount != currentMana[i] || baseManaAmount != currentBaseMana[i]) {
			newData = true;

			currentMana[i]        = manaAmount;
			currentBaseMana[i]    = baseManaAmount;
		}

		// get manaLine info ( which star/ring image, and position on screen )
		// from getStarInfo which takes the mana type index ( i ),
		// current mana total, and the player base mana
		if (newData == true) {
			getManaLineInfo(i, manaAmount, baseManaAmount, &manaLines[i]);
		}
	}

	// return the state of data change
	if (newData  == false) {
		return false;
	} else {
		return true;
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
	starImag = loadButtonRes(healthRes, starStart, starNum, 'S', 'T', 'A');

	// load in the health star border
	starFrameImag    = g_vm->_imageCache->requestImage(healthRes, MKTAG('B', 'T', 'N', starFrameResNum));

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
		starBtns[i] = new GfxCompImage(*trioControls,
		                           Rect16(starXPos,
		                                  starYPos + starYOffset * i,
		                                  starXSize,
		                                  starYSize),
		                           starImag,
		                           starNum,
		                           starInitial,
		                           starIDs[i],
		                           cmd);


		// image control for the star border/frame trio mode
		new GfxCompImage(*trioControls,
		                             Rect16(frameXPos,
		                                    frameYPos + starYOffset * i,
		                                    frameXSize,
		                                    frameYSize),
		                             starFrameImag,
		                             0,
		                             nullptr);


	}
	// health control for individual mode
	// deallocated with panel
	indivStarBtn = new GfxCompImage(*indivControls,
	                          Rect16(starXPos,
	                                 starYPos,
	                                 starXSize,
	                                 starYSize),
	                          starImag,
	                          starNum,
	                          starInitial,
	                          uiIndiv,
	                          cmd);

	// image control for the star border/frame indiv mode
	new GfxCompImage(*indivControls,
	                             Rect16(frameXPos,
	                                    frameYPos,
	                                    frameXSize,
	                                    frameYSize),
	                             starFrameImag,
	                             0,
	                             nullptr);

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
	g_vm->_imageCache->releaseImage(starFrameImag);
}

//  Recalculate and update the health star for a particular brother

void CHealthIndicator::updateStar(GfxCompImage *starCtl, int32 bro, int32 baseVitality, int32 curVitality) {
	assert(baseVitality >= 0);

	int16 maxStar, imageIndex;

	// prevent div zero
	if (baseVitality == 0) baseVitality = 1;

	maxStar = clamp(0, baseVitality / 10 + 14, starLevels - 1);
//	imageIndex = (int16)( sqrt( sqrt( (double)curVitality ) ) * maxStar) / sqrt( sqrt( (double)baseVitality ) );
	imageIndex = (int16)(sqrt((double)MAX((int32)0, curVitality)) * maxStar) / sqrt((double)baseVitality);

	// prevent needless draws
	if (imageIndexMemory[bro] != imageIndex) {
		starCtl->setCurrent(imageIndex);
		starCtl->invalidate();

		imageIndexMemory[bro] = imageIndex;
	}
}

void CHealthIndicator::update(void) {
	if (g_vm->_indivControlsFlag) {
		// get the stats for the selected brother
		int16 baseVitality  = g_vm->_playerList[translatePanID(uiIndiv)]->getBaseStats().vitality;
		int16 currVitality  = g_vm->_playerList[translatePanID(uiIndiv)]->getEffStats()->vitality;

		updateStar(indivStarBtn, uiIndiv, baseVitality, currVitality);
	} else {

		for (uint16 i = 0; i < numControls; i++) {
			// get the stats for the selected brother
			int16 baseVitality  = g_vm->_playerList[i]->getBaseStats().vitality;
			int16 currVitality  = g_vm->_playerList[i]->getEffStats()->vitality;

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
	Rect16          workRect;
	int16 cnt;
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
	Point16         drawPos;
	gFont           *oldFont = port.font;

	va_start(argptr, msg);
	vsprintf(lineBuf, msg, argptr);
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

	void **images = (void **)malloc(sizeof(void *)*numRes);

	for (i = 0, k = resID; i < numRes; i++, k++) {
		// get an image from the image cache
		images[i] = g_vm->_imageCache->requestImage(con, MKTAG('B', 'T', 'N', k));
	}

	return images;
}

// creates an array of images
// passed a resource context, resource ref ID, the number of images
// to sequentially load in, and the context id's
void **loadButtonRes(hResContext *con, int16 resID, int16 numRes, char a, char b, char c) {
	int16   i, k;


	void **images = (void **)malloc(sizeof(void *)*numRes);

	for (i = 0, k = resID; i < numRes; i++, k++) {
		images[i] = g_vm->_imageCache->requestImage(con, MKTAG(a, b, c, k));
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
			g_vm->_imageCache->releaseImage(images[i]);
		}

		free(images);
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


	// set up the control button images
	aggressImag      = loadButtonRes(imageRes, aggressResNum, numBtnImages);

	centerImag       = loadButtonRes(imageRes, centerResNum, numBtnImages);
	bandingImag      = loadButtonRes(imageRes, bandingResNum, numBtnImages);
	menConBtnImag    = loadButtonRes(imageRes, menConBtnResNum, numBtnImages);

	// setup the options button imagery
	optBtnImag       = loadButtonRes(imageRes, optBtnResNum, numBtnImages);

	// setup the brother selector button imagery and button frame
	julBtnImag       = loadButtonRes(imageRes, julBtnResNum, numBtnImages);
	phiBtnImag       = loadButtonRes(imageRes, phiBtnResNum, numBtnImages);
	kevBtnImag       = loadButtonRes(imageRes, kevBtnResNum, numBtnImages);
	broBtnFrameImag  = g_vm->_imageCache->requestImage(imageRes, MKTAG('F', 'R', 'A', 'M'));


	// set up the portrait name plates
	for (n = 0; n < kNumViews; n++) {
		namePlateImages[n] = g_vm->_imageCache->requestImage(imageRes, MKTAG('B', 'T', 'N', namePlateResNum[n]));
	}

	// get the frame image
	namePlateFrameImag = g_vm->_imageCache->requestImage(imageRes, MKTAG('B', 'T', 'N', 15));
	armorImag = g_vm->_imageCache->requestImage(imageRes, MKTAG('B', 'T', 'N', 34));

	// clean out the old context
	if (imageRes) resFile->disposeContext(imageRes);
	imageRes = nullptr;

	// init the resource handle with the face group context
	imageRes = resFile->newContext(faceGroupID, "face resources");

	// set up the portrait button images
	for (n = 0; n < kNumViews; n++) {
		portImag[n]    = loadButtonRes(imageRes, portResNum[n], numPortImages, broNames[n].a, broNames[n].b, broNames[n].c);
	}

	// setup stand alone controls

	optBtn = new GfxCompButton(*playControls, optBtnRect, optBtnImag, numBtnImages, 0, cmdOptions);

	enchDisp = new gEnchantmentDisplay(*playControls, 0);

	// setup the trio user cntl buttons
	for (n = 0; n < kNumViews; n++) {
		// portrait button
		portBtns[n]        = new GfxMultCompButton(*trioControls, views[n][index++],
		                                  portImag[n], numPortImages, 0, false, brotherIDs[n], cmdPortrait);

		portBtns[n]->setMousePoll(true);

		// aggressive button
		aggressBtns[n]     = new GfxOwnerSelCompButton(*trioControls, views[n][index++],
		                                  aggressImag, numBtnImages, brotherIDs[n], cmdAggressive);

		// name plates that go under the portraits
		armorInd[n]        = new gArmorIndicator(*trioControls, views[n][index++],
		                                  armorImag, brotherIDs[n], cmdArmor);

		// center on brother
		centerBtns[n]      = new GfxOwnerSelCompButton(*trioControls, views[n][index++],
		                                  centerImag, numBtnImages, brotherIDs[n], cmdCenter);

		// banding
		bandingBtns[n] = new GfxOwnerSelCompButton(*trioControls, views[n][index++],
		                              bandingImag, numBtnImages, brotherIDs[n], cmdBand);

		// name plates that go under the portraits
		namePlates[n]  = new GfxCompImage(*trioControls, views[n][index++],
		                              namePlateImages[n], 0, nullptr);

		// the frames for the name plates
		namePlateFrames[n] = new GfxCompImage(*trioControls, views[n][index++],
		                                  namePlateFrameImag, 0, nullptr);

		index = 0;
	}

	// individual control buttons

	// portrait button
	indivPortBtn = new GfxMultCompButton(*indivControls, views[0][index++],
	                          portImag[0], numPortImages, 0, false, uiIndiv, cmdPortrait);
	indivPortBtn->setMousePoll(true);

	// aggressive button
	indivAggressBtn  = new GfxOwnerSelCompButton(*indivControls, views[0][index++],
	                              aggressImag, numBtnImages, uiIndiv, cmdAggressive);

	indivArmorInd    = new gArmorIndicator(*indivControls, views[0][index++],
	                              armorImag, uiIndiv, cmdArmor);
	// center on brother
	indivCenterBtn   = new GfxOwnerSelCompButton(*indivControls, views[0][index++],
	                              centerImag, numBtnImages, uiIndiv, cmdCenter);

	// banding
	indivBandingBtn  = new GfxOwnerSelCompButton(*indivControls, views[0][index++],
	                              bandingImag, numBtnImages, uiIndiv, cmdBand);

	// name plates that go under the portraits
	indivNamePlate  = new GfxCompImage(*indivControls, views[0][index++],
	                             namePlateImages[0], 0, nullptr);

	// the frames for the name plates
	indivNamePlateFrame = new GfxCompImage(*indivControls, views[0][index++],
	                                 namePlateFrameImag, 0, nullptr);

	// setup the portrait object
	Portrait = new CPortrait(portBtns,      // portrait buttons
	                                       indivPortBtn,
	                                       numPortImages,// num of images per button
	                                       kNumViews);   // number of brothers


	// mental container button
	menConBtn = new GfxCompButton(*indivControls, menConBtnRect, menConBtnImag, numBtnImages, uiIndiv, cmdBrain);

	// brother selection buttons >>> need to replace these with sticky buttons
	julBtn = new GfxOwnerSelCompButton(*indivControls, julBtnRect, julBtnImag, numBtnImages, uiJulian, cmdBroChange);
	phiBtn = new GfxOwnerSelCompButton(*indivControls, phiBtnRect, phiBtnImag, numBtnImages, uiPhillip, cmdBroChange);
	kevBtn = new GfxOwnerSelCompButton(*indivControls, kevBtnRect, kevBtnImag, numBtnImages, uiKevin, cmdBroChange);

	// frame for brother buttons
	broBtnFrame = new GfxCompImage(*indivControls, broBtnRect, broBtnFrameImag, uiIndiv, nullptr);

	// make the mana indicator
	ManaIndicator = new CManaIndicator(*indivControls);
	ManaIndicator->setMousePoll(true);

	// get rid of the resource contexts
	if (imageRes) {
		resFile->disposeContext(imageRes);
		imageRes = nullptr;
	}

	//The controls need to be enabled but undrawn at startup
	//if ( displayEnabled() )
	//  g_vm->_userControlsSetup = true;
	updateAllUserControls();
}

void enableUserControls(void) {
	g_vm->_userControlsSetup = true;
}

void disableUserControls(void) {
	g_vm->_userControlsSetup = false;
}

// defines the cleanup for ALL user interface controls
void CleanupUserControls(void) {
	g_vm->_userControlsSetup = false;
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
	g_vm->_imageCache->releaseImage(namePlateFrameImag);
	g_vm->_imageCache->releaseImage(armorImag);

	// release name frames
	g_vm->_imageCache->releaseImage(broBtnFrameImag);

	// name plates
	for (i = 0; i < kNumViews; i++) {
		g_vm->_imageCache->releaseImage(namePlateImages[i]);
	}
}

void updateIndicators(void) {
	HealthIndicator->update();
	MassWeightIndicator->update();

	// mana indicator update check
	if (isIndivMode()) {
		if (ManaIndicator->needUpdate(g_vm->_playerList[getCenterActorPlayerID()])) {
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

uint16 getWeightRatio(GameObject *obj, uint16 &maxRatio, bool bReturnMaxRatio = true) {
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


uint16 getBulkRatio(GameObject *obj, uint16 &maxRatio, bool bReturnMaxRatio = true) {
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
	if (g_vm->_indivControlsFlag) {
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
	g_vm->_indivControlsFlag = true;

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
	if (brotherID >= kPlayerActors)
		brotherID = kPlayerActors - 1;

	indivCviewTop->setContainer(GameObject::objectAddress(ActorBaseID + brotherID));
	indivCviewTop->ghost(TrioCviews[brotherID]->isGhosted());
	indivCviewBot->setContainer(GameObject::objectAddress(ActorBaseID + brotherID));
	indivCviewBot->ghost(TrioCviews[brotherID]->isGhosted());

	// now set the indicators for mass and bulk
	uint16 pieWeightRatio   = MassWeightIndicator->getMassPieDiv();
	uint16 pieBulkRatio     = MassWeightIndicator->getBulkPieDiv();
	PlayerActor *brother    = g_vm->_playerList[brotherID];

	MassWeightIndicator->setMassPie(getWeightRatio(brother->getActor(), pieWeightRatio, false));
	MassWeightIndicator->setBulkPie(getBulkRatio(brother->getActor(), pieBulkRatio, false));
}

// sets the trio brothers control state buttons
void setTrioBtns(void) {
	g_vm->_indivControlsFlag = false;

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
	trioControls->show(false, false);
	indivControls->show(true, true);
	trioControls->show(false, true);
}

void setControlPanelsToTrioMode(void) {
	setTrioBtns();
	indivControls->show(false, false);
	trioControls->show(true, true);
	indivControls->show(false, true);
}

void toggleIndivMode(void) {
	if (g_vm->_indivControlsFlag) setControlPanelsToTrioMode();
	else setControlPanelsToIndividualMode(getCenterActorPlayerID());
}

void setCenterBrother(uint16 whichBrother) {
	//  If we picked up anything, then put it back.
	g_vm->_mouseInfo->replaceObject();

	// set the new center actor
	setCenterActor(g_vm->_playerList[whichBrother]);
}

uint16 translatePanID(uint16 panID) {
	// individual mode brother id translation
	if (panID == uiIndiv) {
		panID = indivBrother;
	}

	return panID;
}

void updateBrotherPortrait(uint16 brotherID, int16 pType) {
	if (g_vm->_userControlsSetup) {
		Portrait->set(brotherID, (PortraitType)pType);

		if (brotherID == indivBrother)
			Portrait->set(uiIndiv, (PortraitType)pType);
	}
}

void updateBrotherAggressionButton(uint16 brotherID, bool aggressive) {
	if (g_vm->_userControlsSetup) {
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
	if (g_vm->_userControlsSetup) {
		bandingBtns[brotherID]->select(banded);
		bandingBtns[brotherID]->ghost(isBrotherDead(brotherID));

		if (brotherID == indivBrother) {
			indivBandingBtn->select(banded);
			indivBandingBtn->ghost(isBrotherDead(brotherID));
		}
	}
}

void updateBrotherRadioButtons(uint16 brotherID) {
	if (g_vm->_userControlsSetup) {
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
			indivCenterBtn->select(true);
			indivCenterBtn->ghost(isBrotherDead(brotherID));
		}

		if (g_vm->_indivControlsFlag)
			setControlPanelsToIndividualMode(brotherID);
	}
}

void updateBrotherArmor(uint16 brotherID) {
	if (g_vm->_userControlsSetup) {
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
		if (g_vm->_userControlsSetup) {
			uint16      centerBrotherID = getCenterActorPlayerID(),
			            brotherID;

			if (g_vm->_indivControlsFlag)
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
	if (g_vm->_userControlsSetup) {
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
	GameObject      *mouseObject = g_vm->_mouseInfo->getObject();   // object being dragged

	switch (ev.eventType) {

	case gEventNewValue:

		if (mouseObject != nullptr) {
			PlayerActor     *pa = getPlayerActorAddress(translatePanID(panID));
			Actor           *centerActorPtr = getCenterActor();

			//  we dropped the object onto another object
			if (g_vm->_mouseInfo->getDoable()) {
				int16   intent = g_vm->_mouseInfo->getIntent();

				g_vm->_mouseInfo->replaceObject();
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
					    g_vm->_mouseInfo->getMoveCount());
				}

//				( ( gGenericControl * )ev.panel )->disableDblClick();
				//  clickActionDone = true;
			} else if (g_vm->_mouseInfo->getIntent() == GrabInfo::Use) {
				g_vm->_mouseInfo->replaceObject();
//				clickActionDone = true;
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

		if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
			g_vm->_mouseInfo->setDoable(true);
			break;
		}

//		if (ev.value == gCompImage::enter)
		{
			if (mouseObject != nullptr) {
				PlayerActor     *pa = getPlayerActorAddress(translatePanID(panID));
				Actor           *targetActor = pa->getActor(),
				                 *enactor = getCenterActor();

				g_vm->_mouseInfo->setText(nullptr);

				if ((enactor->getLocation() - targetActor->getLocation()).quickHDistance() > 96) {
					g_vm->_mouseInfo->setDoable(false);
				} else {
					g_vm->_mouseInfo->setDoable(true);
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
				g_vm->_mouseInfo->setText(buf);
			}
		}
		break;

	default:
		break;
	}
}

void toggleAgression(PlayerActorID bro, bool all) {
	int16   wasAggressive = isAggressive(bro);

	if (all) {
		for (int i = 0; i < kPlayerActors; i++)
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
//			for (int i = 0; i < kPlayerActors; i++)
//				setAggression( i, !wasAggressive );
//		}
//		else setAggression( transBroID, !wasAggressive );
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == GfxCompImage::enter) {
			// set the text in the cursor
			g_vm->_mouseInfo->setText(isAggressive(transBroID)
			                  ? ON_AGRESS
			                  : OFF_AGRESS);
		} else if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
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

		if (ev.value == GfxCompImage::enter) {
			gArmorIndicator *gai = (gArmorIndicator *)ev.panel;
			char    buf[128];

			if (gai->attr.damageAbsorbtion == 0
			        &&  gai->attr.defenseBonus == 0) {
				g_vm->_mouseInfo->setText(NO_ARMOR);
			} else {
				sprintf(buf,
				        DESC_ARMOR,
				        gai->attr.damageAbsorbtion,
				        gai->attr.damageDivider,
				        gai->attr.defenseBonus);

				// set the text in the cursor
				g_vm->_mouseInfo->setText(buf);
			}
		} else if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

APPFUNC(cmdCenter) {
	uint16 transBroID = translatePanID(ev.panel->id);

	if (ev.eventType == gEventNewValue) {
		if (rightButtonState())
			setCenterBrother((transBroID + 1) % kPlayerActors);
		else setCenterBrother(transBroID);
	}
	if (ev.eventType == gEventMouseMove) {
		if (ev.value == GfxCompImage::enter) {
			// set the text in the cursor
			g_vm->_mouseInfo->setText(getCenterActorPlayerID() == transBroID
			                  ? ON_CENTER
			                  : OFF_CENTER);
		} else if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

void toggleBanding(PlayerActorID bro, bool all) {
	int16   wasBanded = isBanded(bro);

	if (all) {
		for (int i = 0; i < kPlayerActors; i++)
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
//			for (int i = 0; i < kPlayerActors; i++)
//				setBanded( i, !wasBanded );
//		}
//		else setBanded( transBroID, !wasBanded );
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == GfxCompImage::enter) {
			// set the text in the cursor
			g_vm->_mouseInfo->setText(isBanded(transBroID)
			                  ? ON_BANDED
			                  : OFF_BANDED);
		} else if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

APPFUNC(cmdOptions) {
	if (ev.eventType == gEventNewValue) {
		OptionsDialog();
		//openOptionsPanel();
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == GfxCompImage::enter)        g_vm->_mouseInfo->setText(OPTIONS_PANEL);
		else if (ev.value == GfxCompImage::leave) g_vm->_mouseInfo->setText(nullptr);
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

		if (ev.value == GfxCompImage::enter) {
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
			g_vm->_mouseInfo->setText(buf);
		} else if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

APPFUNC(cmdHealthStar) {
	uint16 transBroID = translatePanID(ev.panel->id);

	if (ev.eventType == gEventMouseMove) {
		if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
			return;
		}

		if (ev.value == GfxCompImage::enter) {
			ev.panel->setMousePoll(true);
		}

		// get the stats for the selected brother
		int16 baseVitality = g_vm->_playerList[transBroID]->getBaseStats().vitality;
		int16 currVitality = g_vm->_playerList[transBroID]->getEffStats()->vitality;

		char buf[40];

		sprintf(buf, "%s %d/%d", HEALTH_HINT, currVitality, baseVitality);
		g_vm->_mouseInfo->setText(buf);
	}
}

APPFUNC(cmdMassInd) {
	gWindow         *win = nullptr;
	GameObject      *containerObject = nullptr;

	if (ev.eventType == gEventMouseMove) {
		if (ev.value == GfxCompImage::enter) {
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
				containerObject = (GameObject *)g_vm->_playerList[getCenterActorPlayerID()]->getActor();
			}

			assert(containerObject);

			curWeight = getWeightRatio(containerObject, baseWeight);

			if (baseWeight != unlimitedCapacity) {
				sprintf(buf, "%s %d/%d", WEIGHT_HINT, curWeight, baseWeight);
				g_vm->_mouseInfo->setText(buf);
			} else
				g_vm->_mouseInfo->setText(UNK_WEIGHT_HINT);
		} else if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

APPFUNC(cmdBulkInd) {
	gWindow         *win = nullptr;
	GameObject      *containerObject = nullptr;


	if (ev.eventType == gEventMouseMove) {
		if (ev.value == GfxCompImage::enter) {
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
				containerObject = (GameObject *)g_vm->_playerList[getCenterActorPlayerID()]->getActor();
			}

			assert(containerObject);

			curBulk = getBulkRatio(containerObject, baseBulk);

			if (baseBulk != unlimitedCapacity) {
				sprintf(buf, "%s %d/%d", BULK_HINT, curBulk, baseBulk);
				g_vm->_mouseInfo->setText(buf);
			} else
				g_vm->_mouseInfo->setText(UNK_BULK_HINT);
		} else if (ev.value == GfxCompImage::leave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

APPFUNC(cmdManaInd) {
	if (ev.eventType == gEventMouseMove) {
		if (ev.value != GfxCompImage::leave) {
			const   int BUF_SIZE = 64;
			char    textBuffer[BUF_SIZE];
			int     manaType = -1;
			int     numManaRegions = ManaIndicator->getNumManaRegions();
			int     i;
			int     curMana = 0, baseMana = 0;
			PlayerActor *player             = g_vm->_playerList[getCenterActorPlayerID()];
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
			case 0:
				sprintf(textBuffer, "%s %d/%d", "Red Mana:", curMana, baseMana);
				break;

			case 1:
				sprintf(textBuffer, "%s %d/%d", "Orange Mana:", curMana, baseMana);
				break;

			case 2:
				sprintf(textBuffer, "%s %d/%d", "Yellow Mana:", curMana, baseMana);
				break;

			case 3:
				sprintf(textBuffer, "%s %d/%d", "Green Mana:", curMana, baseMana);
				break;

			case 4:
				sprintf(textBuffer, "%s %d/%d", "Blue Mana:", curMana, baseMana);
				break;

			case 5:
				sprintf(textBuffer, "%s %d/%d", "Purple Mana:", curMana, baseMana);
				break;

			case -1:
				textBuffer[0] = 0;
				textBuffer[1] = 0;
				break;

			default:
				assert(false);   // should never get here
				break;
			}

			// set the text in the cursor
			g_vm->_mouseInfo->setText(textBuffer);
		} else
			g_vm->_mouseInfo->setText(nullptr);
	}
}

bool isIndivMode(void) {
	return g_vm->_indivControlsFlag;
}

void initUIState(void) {
	g_vm->_indivControlsFlag = false;
	indivBrother = 0;

	//updateAllUserControls();
}

void saveUIState(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving UIState");

	outS->write("UIST", 4);

	CHUNK_BEGIN;
	out->writeUint16LE(g_vm->_indivControlsFlag);
	out->writeUint16LE(indivBrother);
	CHUNK_END;

	debugC(3, kDebugSaveload, "..._indivControlsFlag = %d", g_vm->_indivControlsFlag);
	debugC(3, kDebugSaveload, "... indivBrother = %d", indivBrother);
}

void loadUIState(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading UIState");

	g_vm->_indivControlsFlag = in->readUint16LE();
	indivBrother = in->readUint16LE();

	debugC(3, kDebugSaveload, "... _indivControlsFlag = %d", g_vm->_indivControlsFlag);
	debugC(3, kDebugSaveload, "... indivBrother = %d", indivBrother);

	updateAllUserControls();
}

void cleanupUIState(void) {
	if (StatusLine != nullptr)
		StatusLine->clear();
}

void gArmorIndicator::setValue(PlayerActorID brotherID) {
	Actor *bro = g_vm->_playerList[brotherID]->getActor();
	bro->totalArmorAttributes(attr);
	invalidate();
}

// getCurrentCompImage() is virtual function that should return
// the current image to be displayed (to be used across all sub-classes)
void gArmorIndicator::drawClipped(gPort &port,
                                  const Point16 &offset,
                                  const Rect16 &r) {
	if (!_extent.overlap(r))    return;

	SAVE_GPORT_STATE(port);

	// get the current image
	void *dispImage = getCurrentCompImage();

	// make sure the image is valid
	if (dispImage) {
		// will part of this be drawn on screen?
		if (_extent.overlap(r)) {
			char buf[8];

			// offset the image?
			Point16 pos(_extent.x - offset.x,
			            _extent.y - offset.y
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

			port.drawTextInBox(buf, -1, Rect16(pos.x, pos.y, _extent.width, _extent.height),
			                   textPosRight | textPosHigh, Point16(0,  2));

			if (attr.damageAbsorbtion == 0 && attr.defenseBonus == 0)
				sprintf(buf, "-");
			else sprintf(buf, "%d", attr.defenseBonus);
			port.drawTextInBox(buf, -1, Rect16(pos.x, pos.y, _extent.width, _extent.height),
			                   textPosRight | textPosLow, Point16(0,  2));
		}
	}
}

void gEnchantmentDisplay::drawClipped(gPort &port, const    Point16 &offset, const Rect16 &r) {
	Point16     pos(_extent.x + _extent.width - 10, _extent.y + 1);

	pos += offset;

	if (!_extent.overlap(r)) return;

	for (int i = 0; i < iconCount; i++) {
		if (iconFlags[i]) {
			Sprite      *sp = mentalSprites->sprite(i + 162);

			pos.x -= sp->size.x + 2;
			DrawSprite(port, pos, sp);
		}
	}
}

void gEnchantmentDisplay::pointerMove(gPanelMessage &msg) {
	if (msg.pointerLeave) {
		g_vm->_mouseInfo->setText(nullptr);
	} else {
		int16       x = _extent.width - 10;

		setMousePoll(true);
		setValue(getCenterActorPlayerID());

		for (int i = 0; i < iconCount; i++) {
			if (iconFlags[i]) {
				Sprite      *sp = mentalSprites->sprite(i + 162);

				x -= sp->size.x + 2;
				if (msg.pickPos.x >= x) {
					// set the text in the cursor
					char    buf[128];

					if (iconFlags[i] == 255)
						sprintf(buf, "%s", enchantmentNames[i]);
					else sprintf(buf, "%s : %d", enchantmentNames[i], iconFlags[i]);
					g_vm->_mouseInfo->setText(buf);
					return;
				}
			}
		}
	}
}

void gEnchantmentDisplay::setValue(PlayerActorID pID) {
	Actor           *a = g_vm->_playerList[pID]->getActor();
	uint8           newIconFlags[iconCount];
	EnchantmentIterator iter(a);
	ContainerIterator   cIter(a);

	GameObject *obj = nullptr;

	memset(newIconFlags, 0, sizeof newIconFlags);

	/*
	x   iconHaste,
	    iconFirewalk,
	x   iconAdrenalFervor,
	x   iconShadowWalk,
	    iconSunWard,
	    iconSpellBarrier,
	*/

	for (ObjectID id1 = iter.first(&obj); id1 != Nothing; id1 = iter.next(&obj)) {
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
			case kDamageImpact:
				newIconFlags[iconResistImpact] = duration;
				break;
			case kDamageSlash:
				newIconFlags[iconResistSlash] = duration;
				break;
			case kDamageProjectile:
				newIconFlags[iconResistProjectile] = duration;
				break;
			case kDamageFire:
				newIconFlags[iconResistFire] = duration;
				break;
			case kDamageAcid:
				newIconFlags[iconResistAcid] = duration;
				break;
			case kDamageHeat:
				newIconFlags[iconResistHeat] = duration;
				break;
			case kDamageCold:
				newIconFlags[iconResistCold] = duration;
				break;
			case kDamageLightning:
				newIconFlags[iconResistLightning] = duration;
				break;
			case kDamagePoison:
				newIconFlags[iconResistPoison] = duration;
				break;
			case kDamageMental:
				newIconFlags[iconResistPsionic] = duration;
				break;
			case kDamageDirMagic:
				newIconFlags[iconResistDirectMagic] = duration;
				break;
			}
			break;

		case effectImmune:
			switch (eSubType) {
			case kDamageImpact:
				newIconFlags[iconIronskin] = duration;
				break;
			case kDamageSlash:
				newIconFlags[iconIronskin] = duration;
				break;
			case kDamageFire:
				newIconFlags[iconImmuneFire] = duration;
				break;
			case kDamageAcid:
				newIconFlags[iconImmuneAcid] = duration;
				break;
			case kDamageHeat:
				newIconFlags[iconImmuneHeat] = duration;
				break;
			case kDamageCold:
				newIconFlags[iconImmuneCold] = duration;
				break;
			case kDamageLightning:
				newIconFlags[iconImmuneLightning] = duration;
				break;
			case kDamagePoison:
				newIconFlags[iconImmunePoison] = duration;
				break;
			case kDamageMental:
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

void StatusMsg(const char *msg, ...) { // frametime def
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
