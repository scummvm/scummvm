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

#include "saga2/saga2.h"
#include "saga2/objects.h"
#include "saga2/contain.h"
#include "saga2/grabinfo.h"
#include "saga2/motion.h"
#include "saga2/uimetrcs.h"
#include "saga2/localize.h"
#include "saga2/intrface.h"
#include "saga2/spellbuk.h"
#include "saga2/imagcach.h"
#include "saga2/hresmgr.h"
#include "saga2/fontlib.h"
#include "saga2/savefile.h"

#include "saga2/pclass.r"

namespace Saga2 {

// debug
uint8 weight = 0;
uint8 encum  = 0;

const int           maxOpenDistance = 32;
// selector image pointer
static void         *selImage;


/* ===================================================================== *
   Imports
 * ===================================================================== */

extern ReadyContainerView   *TrioCviews[kNumViews];
extern ReadyContainerView   *indivCviewTop, *indivCviewBot;
extern SpellStuff   spellBook[];
extern SpriteSet    *objectSprites;        // object sprites
extern PlayerActor  playerList[];   //  Master list of all PlayerActors
extern Alarm        containerObjTextAlarm;
extern bool         gameSetupComplete;

extern hResContext  *imageRes;              // image resource handle
hResContext         *containerRes;          // image resource handle

extern APPFUNC(cmdWindowFunc);

//  Temporary...
void grabObject(ObjectID pickedObject);      // turn object into mouse ptr
void releaseObject(void);                    // restore mouse pointer

/* Reference Types
ProtoObj::isTangible
ProtoObj::isContainer
ProtoObj::isBottle
ProtoObj::isFood
ProtoObj::isWearable
ProtoObj::isWeapon
ProtoObj::isDocument
ProtoObj::isIntangible
ProtoObj::isConcept
ProtoObj::isMemory
ProtoObj::isPsych
ProtoObj::isSpell
ProtoObj::isEnchantment
*/

// used to ignore doubleClick when doubleClick == singleClick
static bool alreadyDone;
//  ID of the last object that the mouse moved over

/* ===================================================================== *
   ContainerView member functions
 * ===================================================================== */

// static mouse info variables
ObjectID    ContainerView::lastPickedObjectID = Nothing;
int32       ContainerView::lastPickedObjectQuantity = - 1;
bool        ContainerView::objTextAlarm = false;
char        ContainerView::mouseText[ContainerView::bufSize] = { "" };
bool        ContainerView::mouseInView = false;
uint16      ContainerView::numPicked = 1;
GameObject  *ContainerView::objToGet;
int32       ContainerView::amountAccumulator = 0;
int16       ContainerView::amountIndY = -1;

ContainerList       globalContainerList;

//-----------------------------------------------------------------------
//	Physical container appearance

static ContainerAppearanceDef physicalContainerAppearance = {
	{250, 60, 268, 304 + 16},
	{17 + 4, 87, 268 - 2, 304 - 87},
	{13 + 8, 37, 44, 42},
	{13 + 8 + 44, 37, 44, 42},
	{13 + 118, 50, 36, 36},
	{13 + 139, 37, 88, 43},
	{ MKTAG('P', 'C', 'L', 0), MKTAG('P', 'C', 'L', 1) },
	{ MKTAG('P', 'S', 'L', 0), MKTAG('P', 'S', 'L', 1) },
	{13, 8},
	{22, 22},
	0, 0,
	0
};

static StaticWindow brassDecorations[] = {
	{{0,  0, 268,  86},   nullptr, 3},
	{{13, 86, 242, 109},  nullptr, 4},
	{{13, 195, 242, 121}, nullptr, 5}
};

static StaticWindow clothDecorations[] = {
	{{0,  0, 268,  86},   nullptr, 6},
	{{13, 86, 242, 109},  nullptr, 7},
	{{13, 195, 242, 121}, nullptr, 8}
};

static StaticWindow steelDecorations[] = {
	{{0,  0, 268,  86},   nullptr, 9},
	{{13, 86, 242, 109},  nullptr, 10},
	{{13, 195, 242, 121}, nullptr, 11}
};

static StaticWindow woodDecorations[] = {
	{{0,  0, 268,  86},   nullptr, 12},
	{{13, 86, 242, 109},  nullptr, 13},
	{{13, 195, 242, 121}, nullptr, 14}
};

//-----------------------------------------------------------------------
//	Death container appearance

static ContainerAppearanceDef deathContainerAppearance = {
	{260, 60, 206, 250},
	{2, 87, 206 - 22, 250 - 87 - 32},
	{16,  24, 44, 42},
	{120 + 18, 24, 44, 42},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{ MKTAG('D', 'C', 'L', 0), MKTAG('D', 'C', 'L', 1) },
	{ MKTAG('D', 'S', 'L', 0), MKTAG('D', 'S', 'L', 1) },
	{27, -4},
	{22, 22},
	0, 0,
	0
};

// physal dialog window decorations
static StaticWindow deathDecorations[] = {
	{{0,  0, 206,  250}, nullptr, 15}
};

//-----------------------------------------------------------------------
//	ReadyContainer appearance

static ContainerAppearanceDef readyContainerAppearance = {
	{0, 0, 0, 0},
	{476, 105, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{ 0, 0 },
	{ 0, 0 },
	{iconOriginX - 1, iconOriginY - 1 - 8},
	{iconSpacingX, iconSpacingY},
	1, 3,
	3
};

//-----------------------------------------------------------------------
//	Mental Container appearance

static ContainerAppearanceDef mentalContainerAppearance = {
	{478, 168 - 54, 158, 215},
	{2, 86 - 18 - 4, 158 - 2, 215 - 66},
	{2, 19, 44, 44},
	{103, 40 - 18 - 4, 44, 44},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{ MKTAG('C', 'L', 'S', 0), MKTAG('C', 'L', 'S', 1) },
	{ MKTAG('S', 'E', 'L', 0), MKTAG('S', 'E', 'L', 1) },
	{3, 0},
	{4, 4},
	4, 4,
	20
};

static StaticWindow mentalDecorations[] = {
	{{0,  0, 158,  215}, nullptr, 0}      //  Bottom decoration panel
};
//-----------------------------------------------------------------------
//	Enchantment container appearance

static ContainerAppearanceDef enchantmentContainerAppearance = {
	{262, 92, 116, 202},
	{2, 87, 116 - 2, 202 - 87},
	{7, 50, 44, 43},
	{57, 50, 44, 43},
	{38, 7, 32, 32},
	{0, 0, 0, 0},
	{ MKTAG('A', 'A', 'A', 0), MKTAG('A', 'A', 'A', 0) },
	{ MKTAG('A', 'A', 'A', 0), MKTAG('A', 'A', 'A', 0) },
	{12, 98},
	{16, 13},
	2, 2,
	2
};

//-----------------------------------------------------------------------
//	ContainerView class

//  Constructor
/* ContainerView::ContainerView(
    gPanelList      &list,
    const Rect16    &box,
    ContainerNode   &nd,
    Point16         org,
    Point16         space,
    int16           numRows,
    int16           numCols,
    int16           totRows,
    uint16          ident,
    AppFunc         *cmd ) */

ContainerView::ContainerView(
    gPanelList      &list,
    const Rect16    &rect,
    ContainerNode   &nd,
    ContainerAppearanceDef &app,
    AppFunc         *cmd)
	: gControl(list, rect, NULL, 0, cmd),
	  iconOrigin(app.iconOrigin),
	  iconSpacing(app.iconSpacing),
	  visibleRows(app.rows),
	  visibleCols(app.cols),
	  node(nd) {
	containerObject = GameObject::objectAddress(nd.object);
	scrollPosition  = 0;
	totalRows       = app.totRows;
	setMousePoll(true);
}

//  Destructor
ContainerView::~ContainerView() {
}

/****** contain.cpp/ContainerView::findPane ***********************
*
*   NAME
*             findPane - find a ContainerView control that is
*                        viewing the contents of the target object
*
*   SYNOPSIS
*             If pane is NULL, start search at first ContainerView
*             else, start search at pane->next.
*
*             Iterate through ContainerView list until a
*             ContainerView is found that is viewing the
*             target object (obj).
*
*   INPUTS
*             obj   Target Game Object
*             pane  Previously found pane, or NULL if search start
*
*   RESULT
*             Returns a pointer to a ContainterPanel if one is found
*             or NULL if no more found.
*
********************************************************************
*/

//  returns true if the object is visible for this type of
//  container.
bool ContainerView::isVisible(GameObject *item) {
	ProtoObj *proto = item->proto();

	if (proto->containmentSet() & ProtoObj::isEnchantment)
		return false;

	//  If Intangible Container then don't show it.
	if ((proto->containmentSet() & (ProtoObj::isContainer | ProtoObj::isIntangible)) == (ProtoObj::isContainer | ProtoObj::isIntangible))
		return true;

	return true;
}

//  total the mass, bulk, and number of all objects in container.
void ContainerView::totalObjects(void) {
	ObjectID        objID;
	GameObject      *item;

	totalMass   = 0;
	totalBulk   = 0;
	numObjects  = 0;

	if (containerObject == NULL) return;

	RecursiveContainerIterator  iter(containerObject);

	//  See if he has enough currency
	for (objID = iter.first(&item); objID != Nothing; objID = iter.next(&item)) {
		//  If the object is visible, then add to total mass and
		//  bulk.
		if (isVisible(item)) {
			uint16  numItems;

			ProtoObj *proto = item->proto();

			numObjects++;

			// if it's mergeable calc using the getExtra method of
			// quantity calculation
			// if not, then use the objLoc.z method
			if (proto->flags & ResourceObjectPrototype::objPropMergeable)
				numItems = item->getExtra();
			else numItems = 1;

			totalMass += proto->mass * numItems;
			totalBulk += proto->bulk * numItems;
		}
	}

//	ContainerView *viewToUpdate = ContainerView::findPane( containerObject );

//	viewToUpdate->getWindow()->update( viewToUpdate->getExtent() );
}

//  Return the Nth visible object from this container.
ObjectID ContainerView::getObject(int16 slotNum) {
	ObjectID        objID;
	GameObject      *item;

	if (containerObject == NULL) return Nothing;

	ContainerIterator   iter(containerObject);

	//  Iterate through all the objects in the container.
	while ((objID = iter.next(&item)) != Nothing) {
		//  If the object is visible, then check which # it is,
		//  and return the object if slotnum has been decremented
		//  to zero.
		if (isVisible(item)) {
			if (slotNum == 0) return objID;
			slotNum--;
		}
	}
	return Nothing;
}

//  REM: We need to handle the case of a NULL container here...
//  Draw the contents of the container.
void ContainerView::drawClipped(
    gPort           &port,
    const Point16   &offset,
    const Rect16    &r) {
	//  Coordinates to draw the inventory icon at.
	int16           x,
	                y;

	//  Coordinates for slot 0,0.
	int16           originX = extent.x - offset.x + iconOrigin.x,
	                originY = extent.y - offset.y + iconOrigin.y;

	ObjectID        objID;
	GameObject      *item;

	//  Iterator class for the container.
	ContainerIterator   iter(containerObject);

	//  Color set to draw the object.
	ColorTable      objColors;

	//  If there's no overlap between extent and clip, then return.
	if (!extent.overlap(r)) return;

	//  Iterate through each item within the container.
	while ((objID = iter.next(&item)) != Nothing) {
		TilePoint   objLoc;
		ProtoObj    *objProto = item->proto();

		objLoc = item->getLocation();

		if (objLoc.z == 0) continue;

		//  Draw object only if visible and in a visible row & col.
		if (objLoc.u >= scrollPosition
		        &&  objLoc.u < scrollPosition + visibleRows
		        &&  objLoc.v < visibleCols
		        &&  isVisible(item)) {
			Sprite      *spr;
			ProtoObj    *proto = item->proto();
			Point16     sprPos;

			y =     originY
			        + (objLoc.u - scrollPosition)
			        * (iconSpacing.y + iconHeight);
			x =     originX
			        +       objLoc.v
			        * (iconSpacing.x + iconWidth);

			//  Get the address of the sprite.
			spr = proto->getSprite(item, ProtoObj::objInContainerView).sp;

			sprPos.x = x + ((iconWidth - spr->size.x) >> 1)
			           - spr->offset.x;
			sprPos.y = y + ((iconHeight - spr->size.y) >> 1)
			           - spr->offset.y;

			//  Build the color table.
			item->getColorTranslation(objColors);
			//  Draw the sprite into the port

			DrawColorMappedSprite(
			    port,
			    sprPos,
			    spr,
			    objColors);

			// check to see if selecting amount for this objec
			if (objToGet == item) {
				Point16 selectorPos = Point16(x + ((iconWidth - selectorX) >> 1),
				                              y + ((iconHeight - selectorY) >> 1));

				// draw the selector thingy
				drawSelector(port, selectorPos);

				// set the position of the inc center
				amountIndY = y - (selectorY >> 1) - 12;
			} else drawQuantity(port, item, objProto, x, y);
		}
	}
}

// draws the mereged object multi-item selector
void ContainerView::drawSelector(gPort &port, Point16 &pos) {
	char buf[20];
	uint8   num;

	SAVE_GPORT_STATE(port);

	// draw the arrow images
	drawCompressedImage(port, pos, selImage);

	// draw the number of items selected thus far
	num = sprintf(buf, " %d ", numPicked);

	port.moveTo(Point16(pos.x - ((3 * (num - 3)) + 1),  pos.y + 7));
	port.setFont(&Helv11Font);
	port.setColor(11);                   // set color to white
	port.setStyle(textStyleThickOutline);
	port.setOutlineColor(24);                // set outline color to black
	port.setMode(drawModeMatte);

	port.drawText(buf);
}

void ContainerView::drawQuantity(
    gPort           &port,
    GameObject      *item,
    ProtoObj        *objProto,
    int16           x,
    int16           y) {
	int16       quantity;

	quantity = (objProto->flags & ResourceObjectPrototype::objPropMergeable)
	           ? item->getExtra()
	           : item->getLocation().z;

	if (quantity > 1) {
		SAVE_GPORT_STATE(port);
		char buf[8];

		// draw the number of items selected thus far
		sprintf(buf, "%d", quantity);

		port.moveTo(x - 1,  y + 22);
		port.setFont(&Helv11Font);
		port.setColor(11);                   // set color to white
		port.setStyle(textStyleThickOutline);
		port.setOutlineColor(24);                // set outline color to black
		port.setMode(drawModeMatte);

		port.drawText(buf);
	}
}

void ContainerView::setContainer(GameObject *containerObj) {
	containerObject = containerObj;
	totalObjects();
}


//	Get the slot that the point is over
TilePoint ContainerView::pickObjectSlot(const Point16 &pickPos) {
	TilePoint   slot;
	Point16     temp;

	//  Compute the u/v of the slot that they clicked on.
	temp   = pickPos + iconSpacing / 2 - iconOrigin;
	slot.v = clamp(0, temp.x / (iconWidth  + iconSpacing.x), visibleCols - 1);
	slot.u = clamp(0, temp.y / (iconHeight + iconSpacing.y), visibleRows - 1) + scrollPosition;
	slot.z = 1;
	return slot;
}

//	Get the object that the pointer is over
GameObject *ContainerView::getObject(const TilePoint &slot) {
	GameObject *item;
	TilePoint   loc;

	item = containerObject->child();

	while (item != NULL) {
		//  Skip objects that are stacked behind other objects
		if (item->getLocation().z != 0) {
			ProtoObj *proto = item->proto();

			loc = item->getLocation();

			if (
			    (loc.u == slot.u) &&
			    (loc.v == slot.v) &&
			    //Skip The Enchantments
			    (!(proto->containmentSet() & ProtoObj::isEnchantment))
			) {
				return item;
			}
		}

		item = item->next();
	}

	return NULL;

}

//	Get the object ID that the point is over
ObjectID ContainerView::pickObjectID(const Point16 &pickPos) {
	TilePoint       slot;
	GameObject      *item;

	slot = pickObjectSlot(pickPos);
	item = getObject(slot);

	if (item != NULL) {
		return item->thisID();
	} else {
		return 0;
	}
}

//	Get the object ID that the point is over
GameObject *ContainerView::pickObject(const Point16 &pickPos) {
	TilePoint       slot;
	GameObject      *item;

	slot = pickObjectSlot(pickPos);
	item = getObject(slot);

	return item;
}

bool ContainerView::activate(gEventType why) {
	gPanel::activate(why);

	return true;
}

void ContainerView::deactivate(void) {
}

void ContainerView::pointerMove(gPanelMessage &msg) {
	if (msg.pointerLeave) {
		lastPickedObjectID = Nothing;
		lastPickedObjectQuantity = -1;
		g_vm->_mouseInfo->setText(NULL);
		mouseText[0] = 0;

		// static bool that tells if the mouse cursor
		// is in a panel
		mouseInView = false;
		g_vm->_mouseInfo->setDoable(true);
	} else {
//		if( msg.pointerEnter )
		{
			// static bool that tells if the mouse cursor
			// is in a panel
			mouseInView = true;

			GameObject *mouseObject;
			mouseObject = g_vm->_mouseInfo->getObject();

			if (!node.isAccessable(getCenterActorID())) {
				g_vm->_mouseInfo->setDoable(false);
			} else if (mouseObject == NULL) {
				g_vm->_mouseInfo->setDoable(true);
			} else {
				g_vm->_mouseInfo->setDoable(containerObject->canContain(mouseObject->thisID()));
			}
		}

		//  Determine if mouse is pointing at a new object
		updateMouseText(msg.pickPos);
	}
}

bool ContainerView::pointerHit(gPanelMessage &msg) {
	GameObject  *mouseObject;
	GameObject  *slotObject;
	uint16       mouseSet;

	slotObject  = pickObject(msg.pickPos);
	mouseObject = g_vm->_mouseInfo->getObject();
	mouseSet    = mouseObject ? mouseObject->containmentSet() : 0;

	if (!g_vm->_mouseInfo->getDoable()) return false;

	if (msg.doubleClick && !alreadyDone) {
		dblClick(mouseObject, slotObject, msg);
	} else { // single click
		if (mouseObject != NULL) {
			alreadyDone = true;    // if object then no doubleClick

			if (g_vm->_mouseInfo->getIntent() == GrabInfo::Drop) {
				if (mouseSet & ProtoObj::isTangible) {
					dropPhysical(msg, mouseObject, slotObject, g_vm->_mouseInfo->getMoveCount());
				}

				//  intangibles are used by dropping them
				else if ((mouseSet & ProtoObj::isConcept) ||
				         (mouseSet & ProtoObj::isPsych) ||
				         (mouseSet & ProtoObj::isSpell) ||
				         (mouseSet & ProtoObj::isSkill)) {
					useConcept(msg, mouseObject, slotObject);
				} else {
					// !!!! bad state, reset cursor
					g_vm->_mouseInfo->replaceObject();
				}
			} else if (g_vm->_mouseInfo->getIntent() == GrabInfo::Use) {
				if (mouseSet & ProtoObj::isTangible) {
					usePhysical(msg, mouseObject, slotObject);
				} else if ((mouseSet & ProtoObj::isSpell) ||
				           (mouseSet & ProtoObj::isSkill)) {
					g_vm->_mouseInfo->replaceObject();
				} else {
					useConcept(msg, mouseObject, slotObject);
				}
			} else {
				// !!!! bad state, reset cursor
				g_vm->_mouseInfo->replaceObject();
			}
		} else {
			// default to doubleClick active
			alreadyDone = false;
			clickOn(msg, mouseObject, slotObject);
		}
	}

	// total the mass and bulk of all the objects in this container
	totalObjects();
	window.update(extent);

	return activate(gEventMouseDown);
}

void ContainerView::pointerRelease(gPanelMessage &) {
	// see if in multi-item get mode
	if (objToGet) {
		objToGet->take(getCenterActorID(), numPicked);

		// reset the flags and pointer dealing with merged object movement
		objToGet            = NULL;
		numPicked           = 1;
		amountIndY          = -1;
	}

	gPanel::deactivate();
}

void ContainerView::timerTick(gPanelMessage &msg) {
	// validate objToGet and make sure that the number selected for move
	// is less then or equal to the number of items present in the merged object
	if (objToGet && amountIndY != -1) {
		int32   rate = (amountIndY - msg.pickAbsPos.y);

		rate = rate * ((rate > 0) ? rate : -rate);

		//  Add to the amount accumulator based on the mouse position
		amountAccumulator += rate / 4 /* * accelSpeed */;

		//  Take the top bits of the amount accumulator and add to
		//  the mergeable amount.
		numPicked = clamp(1,
		                  numPicked + (amountAccumulator >> 8),
		                  objToGet->getExtra());

		//  Now remove the bits that we added to the grab amount
		//  keep the remaining bits to accumulate for next time
		amountAccumulator &= 0x00ff;
	}
}

void ContainerView::dblClick(GameObject *mouseObject, GameObject *slotObject, gPanelMessage &msg) {
	alreadyDone = true;

	// double click stuff
	dblClickOn(msg, mouseObject, slotObject);
}

// activate the click function
void ContainerView::clickOn(
    gPanelMessage &,
    GameObject *,
    GameObject *cObj) {
	if (cObj != NULL) {
		if (cObj->proto()->flags & ResourceObjectPrototype::objPropMergeable) {
			if (!rightButtonState()) {
				//  just get the object into the cursor
				cObj->take(getCenterActorID(), cObj->getExtra());
			} else {
				// activate multi-object get interface if a mergeable object
				getMerged(cObj);
				g_vm->_mouseInfo->setText(NULL);
				mouseText[0] = 0;
			}
		} else {
			//  just get the object into the cursor
			cObj->take(getCenterActorID(), numPicked);
		}
	}
}

void ContainerView::getMerged(GameObject *obj) {
	// reset the number picked.
	numPicked = 1;

	// set the object to be gotten
	objToGet = obj;
}

// Activate the double click function
void ContainerView::dblClickOn(
    gPanelMessage &,
    GameObject *mObj,
    GameObject *) {
	if (mObj != NULL) {
		ObjectID        possessor = mObj->possessor();
		ProtoObj        *proto = mObj->proto();
		PlayerActorID   pID;

		//  Only player actors can be possessors as far as the UI is concerned
		if (actorIDToPlayerID(possessor, pID) == false) possessor = Nothing;

		g_vm->_mouseInfo->replaceObject(); //Put Object Back
		if (!(proto->setUseCursor(mObj->thisID()))) {
			MotionTask::useObject(
			    possessor == Nothing ? *getCenterActor() : * (Actor *)GameObject::objectAddress(possessor),
			    *mObj);
		}
	}
}

// drop a physical object
void ContainerView::dropPhysical(
    gPanelMessage   &msg,
    GameObject      *mObj,
    GameObject      *cObj,
    int16           num) {
	assert(g_vm->_mouseInfo->getObject() == mObj);
	assert(mObj->containmentSet() & ProtoObj::isTangible);

	//  Place object back where it came from, temporarily
	g_vm->_mouseInfo->replaceObject();

	//  test to check if item is accepted by container
	if (containerObject->canContain(mObj->thisID())) {
		Actor       *centerActor = getCenterActor();
		Location    loc(pickObjectSlot(msg.pickPos),
		                containerObject->thisID());

		//  check if no object in the current slot
		if (cObj == NULL) {
			MotionTask::dropObject(*centerActor, *mObj, loc, num);

			WriteStatusF(6, "No object state");
		} else {
			//  Try dropping this object on the object in the container
			MotionTask::dropObjectOnObject(*centerActor, *mObj, *cObj, num);
		}

		alreadyDone = true;
	}
}

// use a physical object
void ContainerView::usePhysical(
    gPanelMessage   &msg,
    GameObject      *mObj,
    GameObject      *cObj) {
	assert(g_vm->_mouseInfo->getObject() == mObj);
	assert(mObj->containmentSet() & ProtoObj::isTangible);

	if (cObj == NULL) {
		dropPhysical(msg, mObj, cObj);
	} else {
		g_vm->_mouseInfo->replaceObject();
		//  Use mouse object on container object
		MotionTask::useObjectOnObject(*getCenterActor(), *mObj, *cObj);
	}
}

// Use Concept or Psycological state
void ContainerView::useConcept(
    gPanelMessage   &msg,
    GameObject      *mObj,
    GameObject      *cObj) {
	assert(g_vm->_mouseInfo->getObject() == mObj);
	assert(mObj->containmentSet() & ProtoObj::isIntangible);

	g_vm->_mouseInfo->replaceObject();

	//  Determine if this object can go into this container
	if (containerObject->canContain(mObj->thisID())) {
		ObjectID    centerActorID = getCenterActorID();

		if (cObj == NULL) {
			//  If there is no object already in this slot drop the
			//  mouse object here

			Location    loc(pickObjectSlot(msg.pickPos),
			                containerObject->thisID());

			mObj->drop(centerActorID, loc);
		} else
			//  If there is an object here drop the mouse object onto it
			mObj->dropOn(centerActorID, cObj->thisID());

		alreadyDone = true;
	}
}

// Use Spell or Skill
void ContainerView::useSpell(
    gPanelMessage   &msg,
    GameObject      *mObj,
    GameObject      *cObj) {
	useConcept(msg, mObj, cObj);
}

//  Determine if the mouse is pointing at a new object, and if so,
//  adjust the mouse text
void ContainerView::updateMouseText(Point16 &pickPos) {
	ObjectID slotID = pickObjectID(pickPos);

	// set the mouse text to null if there is no object to get hints about
	if (slotID == Nothing) {
		// clear out the mouse text
		g_vm->_mouseInfo->setText(NULL);
		mouseText[0] = 0;

		// reset the last picked thingy
		lastPickedObjectID          = Nothing;
		lastPickedObjectQuantity    = -1;

		// set the display alarm to false
		objTextAlarm = false;

		return;
	}

	// get handles to the object in question
	GameObject  *slotObject = GameObject::objectAddress(slotID);

	if (slotID == lastPickedObjectID && slotObject->getExtra() == lastPickedObjectQuantity) {
		return; // same object, bug out
	} else {
		// was not same, but is now.
		lastPickedObjectID          = slotID;
		lastPickedObjectQuantity    = slotObject->getExtra();

		// clear out the mouse text
		g_vm->_mouseInfo->setText(NULL);
		mouseText[0] = 0;

		// reset the alarm flag
		objTextAlarm = false;

		// set the hint alarm
		containerObjTextAlarm.set(ticksPerSecond / 2);

		// put the normalized text into mouseText
		slotObject->objCursorText(mouseText, bufSize);
	}
}

#if 0
//  Functions do not appear to be called

void ContainerView::setCursorText(GameObject *obj) {
	assert(obj);

	const   bufSize = 40;
	char cursorText[bufSize];

	// put the normalized text into cursorText
	obj->objCursorText(cursorText, bufSize);

	g_vm->_mouseInfo->setText(cursorText);
}

void ContainerView::setDelayedCursorText(GameObject *obj) {
	// clear out the mouse text
	g_vm->_mouseInfo->setText(NULL);
	mouseText[0] = 0;

	// reset the alarm flag
	objTextAlarm = false;

	// set the hint alarm
	containerObjTextAlarm.set(ticksPerSecond / 2);

	// put the normalized text into mouseText
	obj->objCursorText(mouseText, bufSize);
}
#endif

/* ===================================================================== *
    EnchantmentContainerView member functions
 * ===================================================================== */

EnchantmentContainerView::EnchantmentContainerView(
    gPanelList      &list,
    ContainerNode   &nd,
    ContainerAppearanceDef &app,
    AppFunc         *cmd)
	: ContainerView(list, app.viewRect, nd, app, cmd) {
}

//  Check If Sprite Should Be Shown
bool EnchantmentContainerView::pointerHit(gPanelMessage &) {
	return true;
}

void EnchantmentContainerView::pointerMove(gPanelMessage &) {}

/* ===================================================================== *
    ReadyContainerView member functions
 * ===================================================================== */

ReadyContainerView::ReadyContainerView(
    gPanelList      &list,
    const Rect16    &box,
    ContainerNode   &nd,
    void            **backgrounds,
    int16           numRes,
    int16           numRows,
    int16           numCols,
    int16           totRows,
    AppFunc         *cmd)
	: ContainerView(list, box, nd, readyContainerAppearance, cmd) {
	//  Over-ride row and column info in appearance record.
	visibleRows = numRows;
	visibleCols = numCols;
	totalRows   = totRows;

	if (backgrounds) {
		backImages  = backgrounds;
		numIm       = numRes;
	} else {
		backImages  = NULL;
		numIm       = 0;
	}
}

void ReadyContainerView::setScrollOffset(int8 num) {
	if (num > 0) {
		scrollPosition = num;
	}
}

void ReadyContainerView::timerTick(gPanelMessage &msg) {
	// validate objToGet and make sure that the number selected for move
	// is less then or equal to the number of items present in the merged object
	if (objToGet && amountIndY != -1) {
		ContainerView::timerTick(msg);

		// redraw the container to draw the amount indicator
		draw();
	}
}

void ReadyContainerView::drawClipped(
    gPort           &port,
    const Point16   &offset,
    const Rect16    &r) {
	//  Coordinates to draw the inventory icon at.
	int16           x,
	                y;

	//  Coordinates for slot 0,0.
	int16           originX = extent.x - offset.x + iconOrigin.x,
	                originY = extent.y - offset.y + iconOrigin.y;

	//  Row an column number of the inventory slot.
	int16           col,
	                row;

	ObjectID        objID;
	GameObject      *item;

	//  Iterator class for the container.
	ContainerIterator   iter(containerObject);

	//  Color set to draw the object.
	ColorTable      objColors;

	//  If there's no overlap between extent and clip, then return.
	if (!extent.overlap(r)) return;

	//  Draw the boxes for visible rows and cols.

	if (backImages) {
		int16   i;
		Point16 drawOrg(extent.x - offset.x + backOriginX,
		                extent.y - offset.y + backOriginY);

		for (y = drawOrg.y, row = 0;
		        row < visibleRows;
		        row++, y += iconSpacing.y + iconHeight) {
			// reset y for background image stuff
			//y = drawOrg.y;

			for (i = 0, x = drawOrg.x, col = 0;
			        col < visibleCols;
			        i++, col++, x += iconSpacing.x + iconWidth) {
				Point16 pos(x, y);

				if (isGhosted()) drawCompressedImageGhosted(port, pos, backImages[i % numIm]);
				else drawCompressedImage(port, pos, backImages[i % numIm]);
			}

		}
	} else {
		for (y = originY, row = 0;
		        row < visibleRows;
		        row++, y += iconSpacing.y + iconHeight) {

			for (x = originX, col = 0;
			        col < visibleCols;
			        col++, x += iconSpacing.x + iconWidth) {
				//  REM: We need to come up with some way of
				//  indicating how to render the pattern data which
				//  is behind the object...
				port.setColor(14);
				port.fillRect(x, y, iconWidth, iconHeight);

			}

		}

	}

	//  Iterate through each item within the container.
	while ((objID = iter.next(&item)) != Nothing) {

		TilePoint   objLoc;
		ProtoObj    *objProto = item->proto();

		//  If Intangible Container then don't show it.
		if ((objProto->containmentSet() & (ProtoObj::isContainer | ProtoObj::isIntangible)) == (ProtoObj::isContainer | ProtoObj::isIntangible))
			continue;

		objLoc = item->getLocation();

		if (objLoc.z == 0) continue;

		//  Draw object only if visible and in a visible row & col.
		if (objLoc.u >= scrollPosition &&
		        objLoc.u < scrollPosition + visibleRows &&
		        objLoc.v < visibleCols &&
		        isVisible(item)) {
			Sprite      *spr;
			ProtoObj    *proto = item->proto();
			Point16     sprPos;

			y = originY + (objLoc.u - scrollPosition) * (iconSpacing.y + iconHeight);
			x = originX + objLoc.v * (iconSpacing.x + iconWidth);

			//  Get the address of the sprite.
			spr = proto->getSprite(item, ProtoObj::objInContainerView).sp;

			sprPos.x = x + ((iconWidth - spr->size.x) >> 1)
			           - spr->offset.x;
			sprPos.y = y + ((iconHeight - spr->size.y) >> 1)
			           - spr->offset.y;

			if (isGhosted()) return;

			//  Draw the "in use" indicator.
			if (backImages && proto->isObjectBeingUsed(item)) {
				drawCompressedImage(port,
				                    Point16(x - 4, y - 4), backImages[3]);
			}

			//  Build the color table.
			item->getColorTranslation(objColors);

			//  Draw the sprite into the port
			DrawColorMappedSprite(
			    port,
			    sprPos,
			    spr,
			    objColors);

			// check to see if selecting amount for this objec
			if (objToGet == item) {
				Point16 selectorPos = Point16(x + ((iconWidth - selectorX) >> 1),
				                              y + ((iconHeight - selectorY) >> 1));

				// draw the selector thingy
				drawSelector(port, selectorPos);

				// set the position of the inc center
				amountIndY = y - (selectorY >> 1) + 28;   // extent.y;
			} else drawQuantity(port, item, objProto, x, y);
		}
	}
}

/* ===================================================================== *
    ContainerWindow member functions
 * ===================================================================== */

//  ContainerWindow class

ContainerWindow::ContainerWindow(ContainerNode &nd,
                                 ContainerAppearanceDef &app,
                                 const char saveas[])
	: FloatingWindow(nd.position, 0, saveas, cmdWindowFunc) {
	//  Initialize view to NULL.
	view = NULL;

	// create the close button for this window
	closeCompButton = new gCompButton(
	                      *this,
	                      app.closeRect,              // rect for button
	                      containerRes,               // resource context
	                      app.closeResID[0],
	                      app.closeResID[1],
	                      0,
	                      cmdCloseButtonFunc);        // mind app func
}

//  Virtual destructor (base does nothing)
ContainerWindow::~ContainerWindow(void) {}

ContainerView &ContainerWindow::getView(void) {
	return *view;
}

/* ===================================================================== *
    ScrollableContainerWindow member functions
 * ===================================================================== */

ScrollableContainerWindow::ScrollableContainerWindow(
    ContainerNode &nd, ContainerAppearanceDef &app, const char saveas[])
	: ContainerWindow(nd, app, saveas) {
	view = new ContainerView(*this, app.viewRect, nd, app);

	// make the button conected to this window
	scrollCompButton = new gCompButton(
	                       *this,
	                       app.scrollRect,                 // rect for button
	                       containerRes,                   // resource context
	                       app.scrollResID[0],           // resource handle name
	                       app.scrollResID[1],
	                       0,
	                       cmdScrollFunc);                 // mind app func

	assert(view != NULL);
	assert(scrollCompButton != NULL);
}

/* ===================================================================== *
    TangibleContainerWindow member functions
 * ===================================================================== */

TangibleContainerWindow::TangibleContainerWindow(
    ContainerNode &nd, ContainerAppearanceDef &app)
	: ScrollableContainerWindow(nd, app, "ObjectWindow") {
#if DEBUG
	assert(view->containerObject);
	assert(view->containerObject->proto());
#endif

	const int weightIndicatorType = 2;
	objRect = app.iconRect;
	deathFlag = nd.getType() == ContainerNode::deadType;
	containerSpriteImg = NULL;

	// setup the mass and weight indicator
	if (deathFlag) {
		// set the decorations for this window
		setDecorations(deathDecorations,
		               ARRAYSIZE(deathDecorations),
		               containerRes, 'F', 'R', 'M');
		massWeightIndicator = NULL;
	} else {
		static StaticWindow *winDecs[] =  {
			brassDecorations,
		    clothDecorations,
		    steelDecorations,
		    woodDecorations
		};
		uint16      bgndType = view->containerObject->proto()->appearanceType;

		assert(bgndType < 4);

		setContainerSprite();               // show at the top of the box

		// set the decorations for this window
		setDecorations(winDecs[bgndType],
		               ARRAYSIZE(brassDecorations),    // brass was arb, all should have same
		               containerRes, 'F', 'R', 'M');

		// set the userdata such that we can extract the container object later
		// through an appfunc.
		this->userData = view->containerObject;

		massWeightIndicator = new CMassWeightIndicator(
		                          this,
		                          Point16(app.massRect.x, app.massRect.y),
		                          weightIndicatorType,
		                          deathFlag);
	}
}

TangibleContainerWindow::~TangibleContainerWindow(void) {
	if (massWeightIndicator)    delete massWeightIndicator;
	if (containerSpriteImg)     delete containerSpriteImg;
}

void TangibleContainerWindow::setContainerSprite(void) {
	// pointer to sprite data that will be drawn
	Sprite              *spr;
	ProtoObj            *proto = view->containerObject->proto();
	Point16             sprPos;
	char                dummy = '\0';

	//  Get the address of the sprite.
	spr = proto->getSprite(view->containerObject, ProtoObj::objInContainerView).sp;

	sprPos.x = objRect.x - (spr->size.x >> 1);  //objRect.x + ( spr->size.x >> 1 );
	sprPos.y = objRect.y - (spr->size.y >> 1);

	containerSpriteImg = new gSpriteImage(
	                         *this,
	                         Rect16(sprPos.x,
	                                sprPos.y,
	                                objRect.height,
	                                objRect.width),
	                         view->containerObject,
	                         dummy,
	                         0,
	                         NULL);
}

void TangibleContainerWindow::massBulkUpdate() {
	if (massWeightIndicator) {      //  Death container doesn't have MW indicator
		// set the indicators to the correct mass and bulk
		massWeightIndicator->setMassPie(view->totalMass);
		massWeightIndicator->setBulkPie(view->totalBulk);
	}
}

void TangibleContainerWindow::drawClipped(
    gPort &port,
    const Point16 &offset,
    const Rect16 &clip) {
	if (!extent.overlap(clip)) return;

	// draw the decorations
	ScrollableContainerWindow::drawClipped(port, offset, clip);
}

/* ===================================================================== *
    IntangibleContainerWindow member functions
 * ===================================================================== */

IntangibleContainerWindow::IntangibleContainerWindow(
    ContainerNode &nd, ContainerAppearanceDef &app)
	: ScrollableContainerWindow(nd, app, "MentalWindow") {
	// make the button conected to this window
	mindSelectorCompButton = new gMultCompButton(
	                             *this,
	                             Rect16(49, 15 - 13, 52, 67),
	                             containerRes,
	                             'H', 'E', 'D', 1, 3, 1,
	                             0,
	                             cmdMindContainerFunc);          // mind app func

	assert(mindSelectorCompButton != NULL);

	mindSelectorCompButton->setResponse(false);

	// set the decorations for this window
	setDecorations(mentalDecorations,
	               ARRAYSIZE(mentalDecorations),
	               containerRes, 'F', 'R', 'M');

	setMindContainer(nd.mindType, *this);
}

/* ===================================================================== *
    EnchantmentContainerWindow member functions
 * ===================================================================== */

EnchantmentContainerWindow::EnchantmentContainerWindow(
    ContainerNode &nd, ContainerAppearanceDef &app)
	: ContainerWindow(nd, app, "EnchantmentWindow") {
	view = new EnchantmentContainerView(*this, nd, app);

	// make the button conected to this window
	scrollCompButton = new gCompButton(
	                       *this,
	                       app.scrollRect,                 // rect for button
	                       containerRes,                   // resource context
	                       app.scrollResID[0],           // resource handle name
	                       app.scrollResID[1],
	                       0,
	                       cmdScrollFunc);                 // mind app func

	assert(view != NULL);
	assert(scrollCompButton != NULL);
}

/* ===================================================================== *
   ContainerNode functions
 * ===================================================================== */

ContainerNode::ContainerNode(ContainerList &cl, ObjectID id, int typ) {
	GameObject      *obj = GameObject::objectAddress(id);
	PlayerActorID   ownerID;

	//  Convert the possessor() of the object to a player actor ID,
	//  if it is indeed a player actor; Else set to "nobody".
	if (isActor(id)) {
		if (actorIDToPlayerID(id, ownerID) == false)
			ownerID = ContainerNode::nobody;
	} else {
		ObjectID        possessor = obj->possessor();

		if (possessor == Nothing || actorIDToPlayerID(possessor, ownerID) == false)
			ownerID = ContainerNode::nobody;
	}

	//  Compute the initial position of the container window
	switch (typ) {
	case readyType:
		break;

	case deadType:
		position = deathContainerAppearance.defaultWindowPos;
		break;

	case mentalType:
		mindType = 0; //protoClassIdeaContainer;
		position = mentalContainerAppearance.defaultWindowPos;
		break;

	case physicalType:
		position = physicalContainerAppearance.defaultWindowPos;
		break;

	case enchantType:
		position = enchantmentContainerAppearance.defaultWindowPos;
		break;
	}

	//  Fill in the initial values.
	window      = NULL;
	type        = typ;
	object      = id;
	owner       = ownerID;
	action      = 0;

	//  Add to container list.
	cl.add(this);
}

//  Return the container window for a container node, if it is visible
ContainerWindow *ContainerNode::getWindow(void) {
	return window;
}

//  Return the container view for a container node, if it is visible
ContainerView   *ContainerNode::getView(void) {
	return window ? &window->getView() : NULL;
}

//  Destructor
ContainerNode::~ContainerNode() {
	//  Close the container window.
	hide();

	//  Remove from container list
	globalContainerList.remove(this);
}

//  Restore the state of this ContainerNode from archive buffer
void *ContainerNode::restore(void *buf) {
	Archive     *a = (Archive *)buf;

	//  Restore fields
	object      = a->object;
	type        = a->type;
	owner       = a->owner;
	position    = a->position;
	mindType    = a->mindType;
	window      = NULL;
	action      = 0;

	//  If this container was shown, re-show it
	if (a->shown) markForShow();

	return &a[1];
}

//  Store the state of this ContainerNode into archive buffer
void *ContainerNode::archive(void *buf) {
	Archive     *a = (Archive *)buf;

	//  Store fields
	a->object   = object;
	a->type     = type;
	a->owner    = owner;
	a->position = position;
	a->mindType = mindType;
	a->shown    = window != NULL;

	return &a[1];
}

//  Close the container window, but leave the node.
void ContainerNode::hide(void) {
	//  close the window, but don't close the object.
	if (type != readyType && window != NULL) {
		position = window->getExtent();     //  Save old window position
		window->close();
		delete window;
		window = NULL;
	}
}

//  Open the cotainer window, given the node info.
void ContainerNode::show(void) {
	ProtoObj        *proto = GameObject::protoAddress(object);

	assert(proto);

	//  open the window; Object should already be "open"
	if (window == NULL) {
		switch (type) {
		case readyType:
			return;

		case physicalType:
			physicalContainerAppearance.rows    = proto->getViewableRows();
			physicalContainerAppearance.cols    = proto->getViewableCols();
			physicalContainerAppearance.totRows = proto->getMaxRows();
			window = new TangibleContainerWindow(*this, physicalContainerAppearance);
			break;

		case deadType:
			deathContainerAppearance.rows       = proto->getViewableRows();
			deathContainerAppearance.cols       = proto->getViewableCols();
			deathContainerAppearance.totRows    = proto->getMaxRows();
			window = new TangibleContainerWindow(*this, deathContainerAppearance);
			break;

		case mentalType:
			window = new IntangibleContainerWindow(*this, mentalContainerAppearance);
			break;

		case enchantType:
			window = new EnchantmentContainerWindow(*this, enchantmentContainerAppearance);
			break;
		}
	}

	window->open();
}

void ContainerNode::update(void) {
	if (type == readyType) {
		//  Update ready containers if they are enabled
		if (TrioCviews[owner]->getEnabled())  TrioCviews[owner]->invalidate();
		if (indivCviewTop->getEnabled())        indivCviewTop->invalidate();
		if (indivCviewBot->getEnabled())        indivCviewBot->invalidate();

		//  If the container to update is the center brother's ready container.
		if (isIndivMode() && owner == getCenterActorPlayerID()) {
			//  Update player's mass & weight indicator...
			MassWeightIndicator->update();
		}
	} else if (window) {
		getView()->invalidate();
		window->massBulkUpdate();
	}
}

//  Find a container node, given a specific object
ContainerNode *ContainerList::find(ObjectID id) {
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it)
		if ((*it)->object == id)
			return *it;

	return NULL;
}

ContainerNode *ContainerList::find(ObjectID id, int16 type) {
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it)
		if ((*it)->object == id && (*it)->type == type)
			return *it;

	return NULL;
}

//  returns true if the object represented by the container can be
//  accessed by the player.
bool ContainerNode::isAccessable(ObjectID enactor) {
	Actor       *a = (Actor *)GameObject::objectAddress(enactor);
	ObjectID    holder;
	GameObject  *obj = GameObject::objectAddress(object);
	int32       dist;

	//  REM: We really ought to do a line-of-sight test here.

	//  Calculate distance between actor and container.
	dist = (a->getLocation() - obj->getWorldLocation()).quickHDistance();

	//  If the container object is too far away we can't access any containers.
	//  Note: Actors are not considered to be in possession of themselves...
	holder = obj->possessor();
	if (holder != Nothing || isActor(object)) {
		//  "Reach" for other players is further than for other objects
		if (holder != a->thisID() && dist > 96) return false;
	} else if (dist > maxOpenDistance) return false;

	return true;
}

//  Change the owner of a ready container (for indiv mode)
void ContainerNode::changeOwner(int16 newOwner) {
	owner = newOwner;
	object = getPlayerActorAddress(newOwner)->getActorID();
}

/* ===================================================================== *
   ContainerList functions
 * ===================================================================== */

void ContainerList::setPlayerNum(PlayerActorID playerNum) {
	//  Close all containers which are not on the ground and not owned
	//  by the current protagonist.
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->owner != ContainerNode::nobody && n->owner != playerNum)
			n->hide();
	}

	//  Open any containers which belong to the new protagonist.
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->owner == playerNum)
			n->markForShow();
	}
}

void ContainerList::doDeferredActions(void) {
	Common::List<ContainerNode *>::iterator nextIt;
	Actor           *a = getCenterActor();
	TilePoint       tp = a->getLocation();
	GameObject      *world = a->parent();

	//  Close all containers which are not on the ground and not owned
	//  by the current protagonist.
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); it = nextIt) {
		nextIt = it;
		nextIt++;
		ContainerNode *n = *it;

		//  If the object is not in a player inventory (i.e. on the ground)
		if (n->owner == ContainerNode::nobody) {
			//  If the object is in a different world, or too far away
			//  from the protagonist, then quietly close the object.
			GameObject  *obj = GameObject::objectAddress(n->object);
			if (obj->world() != world
			        || (obj->getWorldLocation() - tp).quickHDistance() > maxOpenDistance) {
				//  Close object image and window (silently)
				obj->setFlags(0, objectOpen);
				delete n;
				continue;
			}
		}

		if (n->action & ContainerNode::actionDelete) {
			delete n;
			continue;
		}

		if (n->action & ContainerNode::actionHide) {
			n->hide();
		} else {
			if (n->action & ContainerNode::actionShow) n->show();
			if (n->action & ContainerNode::actionUpdate) n->update();
		}

		n->action = 0;
	}
}

void ContainerList::setUpdate(ObjectID id) {
	//  Close all containers which are not on the ground and not owned
	//  by the current protagonist.
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->object == id)
			n->update();
		else if (n->type == ContainerNode::mentalType    //  Special case for mind containers
		         &&  n->object == GameObject::objectAddress(id)->IDParent())
			n->update();
	}
}

extern int16 openMindType;

//  General function to create a container "node". This determines what
//  kind of container is appropriate, and also if a container of that
//  type is already open.
ContainerNode *CreateContainerNode(ObjectID id, bool open, int16) {
	ContainerNode   *cn = NULL;
	GameObject      *obj = GameObject::objectAddress(id);
	PlayerActorID   owner;

	if (isActor(id)) {
		if (actorIDToPlayerID(id, owner) == false)
			owner = ContainerNode::nobody;

		if (((Actor *)obj)->isDead()) {
			//  Open dead container for dead actor
			if (!(cn = globalContainerList.find(owner, ContainerNode::deadType)))
				cn = new ContainerNode(globalContainerList, id, ContainerNode::deadType);
		} else if (owner != ContainerNode::nobody) {
			//  Open mental container for living player actor.
//			if (!(cn = globalContainerList.find( owner, ContainerNode::mentalType )))
//				cn = new ContainerNode( globalContainerList, id, ContainerNode::mentalType );
			return OpenMindContainer(owner, open, /*mType*/ openMindType);
		}
#if DEBUG
		else fatal("Attempt to open non-dead actor as a container.\n");
#endif
	} else {
		if (actorIDToPlayerID(obj->possessor(), owner) == false)
			owner = ContainerNode::nobody;

		if (!(cn = globalContainerList.find(id, ContainerNode::physicalType)))
			cn = new ContainerNode(globalContainerList, id, ContainerNode::physicalType);
	}

	//  If node was successfull created, and we wanted it open, and the owner
	//  is the center actor or no-actor then make the container window visible.
	if (cn != NULL
	        &&  open
	        && (owner == getCenterActorID() || owner == ContainerNode::nobody)) {
		cn->show();
	}

	return cn;
}

ContainerNode *CreateReadyContainerNode(PlayerActorID player) {
	return new ContainerNode(globalContainerList,
	                            getPlayerActorAddress(player)->getActorID(),
	                            ContainerNode::readyType);
}

ContainerNode *OpenMindContainer(PlayerActorID player, int16 open, int16 type) {
	ContainerNode   *cn;
	ObjectID        id = getPlayerActorAddress(player)->getActorID();

	if (!(cn = globalContainerList.find(id, ContainerNode::mentalType))) {
		cn = new ContainerNode(globalContainerList, id, ContainerNode::mentalType);
		cn->mindType = type;

		//  If node was successfull created, and we wanted it open, and the owner
		//  is the center actor or no-actor then make the container window visible.
		if (cn != NULL
		        &&  open
		        &&  id == getCenterActorID()) {
			cn->show();
		}
	} else {
		IntangibleContainerWindow   *cw = (IntangibleContainerWindow *)cn->getWindow();

		if (cw && (type != cn->mindType)) {
			cn->mindType = type;
			setMindContainer(cn->mindType, *cw);
			cw->update(cw->getView().getExtent());
		}
	}
	return cn;
}

/* ===================================================================== *
    Misc. functions
 * ===================================================================== */

void initContainers(void) {
	if (containerRes == NULL)
		containerRes = resFile->newContext(MKTAG('C', 'O', 'N', 'T'), "cont.resources");

	selImage = g_vm->_imageCache->requestImage(imageRes, MKTAG('A', 'M', 'N', 'T'));
}

void cleanupContainers(void) {
	if (selImage)       g_vm->_imageCache->releaseImage(selImage);
	if (containerRes)   resFile->disposeContext(containerRes);

	selImage = NULL;
	containerRes = NULL;
}

void initContainerNodes(void) {
#if DEBUG
	//  Verify the globalContainerList only has ready ContainerNodes

	ContainerNode   *node;
	bool            onlyReady = true;

	for (node = (ContainerNode *)globalContainerList.first();
	        node != NULL;
	        node = (ContainerNode *)node->next()) {
		if (node->getType() != ContainerNode::readyType) {
			onlyReady = false;
			break;
		}
	}

	assert(onlyReady);
#endif
}

void saveContainerNodes(SaveFileConstructor &saveGame) {
	int16               numNodes = 0;
	void                *archiveBuffer,
	                    *bufferPtr;
	int32               archiveBufSize;

	//  Make sure there are no pending container view actions
	globalContainerList.doDeferredActions();

	archiveBufSize = sizeof(numNodes);

	//  Count the number of nodes to save
	for (Common::List<ContainerNode *>::iterator it = globalContainerList._list.begin(); it != globalContainerList._list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->getType() != ContainerNode::readyType)
			numNodes++;
	}

	//  Compute size of archive buffer
	archiveBufSize += numNodes * ContainerNode::archiveSize();

	//  Allocate the archive buffer
	archiveBuffer = malloc(archiveBufSize);
	if (archiveBuffer == NULL)
		error("Unable to allocate ContainerNode archive buffer\n");

	bufferPtr = archiveBuffer;

	//  Store the number of nodes to save
	*((int16 *)bufferPtr) = numNodes;
	bufferPtr = (int16 *)bufferPtr + 1;

	//  Store the nodes
	for (Common::List<ContainerNode *>::iterator it = globalContainerList._list.begin(); it != globalContainerList._list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->getType() != ContainerNode::readyType)
			bufferPtr = n->archive(bufferPtr);
	}

	//  Write the archive data to the save file
	saveGame.writeChunk(
	    MakeID('C', 'O', 'N', 'T'),
	    archiveBuffer,
	    archiveBufSize);

	//  Free the archive buffer
	free(archiveBuffer);
}

void loadContainerNodes(SaveFileReader &saveGame) {
	ContainerNode       *node;
	Common::List<ContainerNode *> tempList;
	int16               i,
	                    numNodes;
	void                *archiveBuffer,
	                    *bufferPtr;
	int32               archiveBufSize;

	//  Read in the number of container nodes to restore
	saveGame.read(&numNodes, sizeof(numNodes));

	//  Allocate archive buffer
	archiveBufSize = saveGame.bytesLeftInChunk();
	archiveBuffer = malloc(archiveBufSize);
	if (archiveBuffer == NULL)
		error("Unable to allocate ContainerNode archive buffer\n");

	//  Read in node data
	saveGame.read(archiveBuffer, archiveBufSize);

	bufferPtr = archiveBuffer;

	for (i = 0; i < numNodes; i++) {
		node = new ContainerNode;

		//  Restore the state of the node
		bufferPtr = node->restore(bufferPtr);

		//  Add it back to the container list
		globalContainerList.add(node);
	}

	assert(tempList.empty());

	//  Free the archive buffer
	free(archiveBuffer);
}

void cleanupContainerNodes(void) {
	for (Common::List<ContainerNode *>::iterator it = globalContainerList._list.begin(); it != globalContainerList._list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->getType() != ContainerNode::readyType)
			delete n;
	}
}

void updateContainerWindows(void) {
	globalContainerList.doDeferredActions();
}

void setMindContainer(int index, IntangibleContainerWindow &cw) {
	static int classTable[] = { protoClassIdeaContainer,
	                            protoClassSkillContainer,
	                            protoClassMemoryContainer,
	                            protoClassPsychContainer    // Not used anymore
	                          };

	ObjectID        ownerID = cw.getView().node.getObject();
	GameObject      *object = GameObject::objectAddress(ownerID);
	ContainerIterator iter(object);
	GameObject      *item;
	ObjectID        id;

	assert(index >= 0);
	assert(index < ARRAYSIZE(classTable));

	int             containerClass = classTable[index];

	cw.mindSelectorCompButton->setCurrent(index);
	cw.mindSelectorCompButton->invalidate();

	while ((id = iter.next(&item)) != Nothing) {
		if (item->proto()->classType == containerClass) {
			cw.view->setContainer(item);
			return;
		}
	}
}

APPFUNC(cmdMindContainerFunc) {
	if (ev.panel && ev.eventType == gEventNewValue /* && ev.value */) {
		IntangibleContainerWindow   *cw = (IntangibleContainerWindow *)ev.window;
		ContainerNode   &nd = cw->getView().node;
		int             newMindType = nd.mindType;

		static Rect16   idea(0, 0, 22, 67),       // idea button click area
		       skill(22, 0, 11, 67),    // skill area
		       memory(33, 0,  9, 67),   // memory area
		       psych(42, 0, 10, 67);    // psych(ic?) area

		if (idea.ptInside(ev.mouse))    newMindType = 0; //protoClassIdeaContainer;
		if (skill.ptInside(ev.mouse))   newMindType = 1; //protoClassSkillContainer;
		if (memory.ptInside(ev.mouse))  newMindType = 2; //protoClassMemoryContainer;
//		if (psych.ptInside(ev.mouse))   newMindType = protoClassPsychContainer;

		if (newMindType != nd.mindType) {
			nd.mindType = newMindType;
			setMindContainer(nd.mindType, *cw);
			cw->update(cw->getView().getExtent());
		}
	} else if (ev.eventType == gEventMouseMove) {
		//if (ev.value == gCompImage::enter)
		{
			static Rect16   idea(0, 0, 22, 67),       // idea button click area
			       skill(22, 0, 11, 67),    // skill area
			       memory(33, 0,  9, 67);   // memory area


			const int BUF_SIZE = 64;
			char    textBuffer[BUF_SIZE];
			int     mindType = -1;


			if (idea.ptInside(ev.mouse))       mindType = 0;    //protoClassIdeaContainer;
			if (skill.ptInside(ev.mouse))  mindType = 1;    //protoClassSkillContainer;
			if (memory.ptInside(ev.mouse)) mindType = 2;    //protoClassMemoryContainer;

			switch (mindType) {
			case 0:
				sprintf(textBuffer, IDEAS_MENTAL);
				break;

			case 1:
				sprintf(textBuffer, SPELL_MENTAL);
				break;

			case 2:
				sprintf(textBuffer, SKILL_MENTAL);
				break;

			case -1:
				textBuffer[0] = 0;
				break;

			default:
				assert(false);   // should never get here
				break;
			}

			// set the text in the cursor
			g_vm->_mouseInfo->setText(textBuffer);
		}

		if (ev.value == gCompImage::leave) {
			g_vm->_mouseInfo->setText(NULL);
		}
	}
}

APPFUNC(cmdCloseButtonFunc) {
	if (ev.eventType == gEventNewValue && ev.value == 1) {
		ContainerWindow     *win = (ContainerWindow *)ev.window;

		if (win->getView().node.getType() == ContainerNode::mentalType) {
			win->getView().node.markForDelete();
		} else {
			win->containerObject()->close(getCenterActorID());
		}
		updateContainerWindows();

		// make sure the hint text goes away
		if (g_vm->_mouseInfo->getObject() == NULL) {
			g_vm->_mouseInfo->setText(NULL);
		}
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter) {
			g_vm->_mouseInfo->setText(CLOSE_MOUSE);
		} else if (ev.value == gCompImage::leave) {
			g_vm->_mouseInfo->setText(NULL);
		}
	}
}

APPFUNC(cmdScrollFunc) {
	if (ev.panel && ev.eventType == gEventNewValue && ev.value) {
		ScrollableContainerWindow       *cw;
		static Rect16                   upArea(0, 0, 44, 22);

		cw = (ScrollableContainerWindow *)ev.window;
		if (upArea.ptInside(ev.mouse))
			cw->scrollUp();
		else
			cw->scrollDown();
		ev.window->update(cw->getView().getExtent());
	} else if (ev.eventType == gEventMouseMove) {
		if (ev.value == gCompImage::enter) {
			g_vm->_mouseInfo->setText(SCROLL_MOUSE);
		} else if (ev.value == gCompImage::leave) {
			g_vm->_mouseInfo->setText(NULL);
		}
	}
}

} // end of namespace Saga2
