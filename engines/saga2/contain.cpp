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

#include "saga2/saga2.h"
#include "saga2/detection.h"
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

#include "saga2/pclass.r"

namespace Saga2 {

enum {
	kMaxOpenDistance = 32
};

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern ReadyContainerView   *TrioCviews[kNumViews];
extern ReadyContainerView   *indivCviewTop, *indivCviewBot;
extern SpriteSet    *objectSprites;        // object sprites
extern Alarm        containerObjTextAlarm;
extern bool         gameSetupComplete;

extern hResContext  *imageRes;              // image resource handle
hResContext         *containerRes;          // image resource handle

extern APPFUNC(cmdWindowFunc);

//  Temporary...
void grabObject(ObjectID pickedObject);      // turn object into mouse ptr
void releaseObject();                    // restore mouse pointer

/* Reference Types
ProtoObj::kIsTangible
ProtoObj::kIsContainer
ProtoObj::kIsBottle
ProtoObj::kIsFood
ProtoObj::kIsWearable
ProtoObj::kIsWeapon
ProtoObj::kIsDocument
ProtoObj::kIsIntangible
ProtoObj::kIsConcept
ProtoObj::kIsMemory
ProtoObj::kIsPsych
ProtoObj::kIsSpell
ProtoObj::kIsEnchantment
*/

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

static const StaticWindow brassDecorations[] = {
	{{0,  0, 268,  86},   nullptr, 3},
	{{13, 86, 242, 109},  nullptr, 4},
	{{13, 195, 242, 121}, nullptr, 5}
};

static const StaticWindow clothDecorations[] = {
	{{0,  0, 268,  86},   nullptr, 6},
	{{13, 86, 242, 109},  nullptr, 7},
	{{13, 195, 242, 121}, nullptr, 8}
};

static const StaticWindow steelDecorations[] = {
	{{0,  0, 268,  86},   nullptr, 9},
	{{13, 86, 242, 109},  nullptr, 10},
	{{13, 195, 242, 121}, nullptr, 11}
};

static const StaticWindow woodDecorations[] = {
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
static const StaticWindow deathDecorations[] = {
	{{0,  0, 206,  250}, nullptr, 15}
};

//-----------------------------------------------------------------------
//	ReadyContainer appearance

static const ContainerAppearanceDef readyContainerAppearance = {
	{0, 0, 0, 0},
	{476, 105, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{ 0, 0 },
	{ 0, 0 },
	{kIconOriginX - 1, kIconOriginY - 1 - 8},
	{kIconSpacingX, kIconSpacingY},
	1, 3,
	3
};

//-----------------------------------------------------------------------
//	Mental Container appearance

static const ContainerAppearanceDef mentalContainerAppearance = {
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

static const StaticWindow mentalDecorations[] = {
	{{0,  0, 158,  215}, nullptr, 0}      //  Bottom decoration panel
};
//-----------------------------------------------------------------------
//	Enchantment container appearance

static const ContainerAppearanceDef enchantmentContainerAppearance = {
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

ContainerView::ContainerView(
    gPanelList      &list,
    const Rect16    &rect,
    ContainerNode   &nd,
    const ContainerAppearanceDef &app,
    AppFunc         *cmd)
	: gControl(list, rect, nullptr, 0, cmd),
	  _iconOrigin(app.iconOrigin),
	  _iconSpacing(app.iconSpacing),
	  _visibleRows(app.rows),
	  _visibleCols(app.cols),
	  _node(nd) {
	_containerObject = GameObject::objectAddress(nd._object);
	_scrollPosition  = 0;
	_totalRows       = app.totRows;
	setMousePoll(true);
	_totalMass = 0;
	_totalBulk = 0;
	_numObjects = 0;
}

//  Destructor
ContainerView::~ContainerView() {
}

//  returns true if the object is visible for this type of
//  container.
bool ContainerView::isVisible(GameObject *item) {
	ProtoObj *proto = item->proto();

	if (proto->containmentSet() & ProtoObj::kIsEnchantment)
		return false;

	//  If Intangible Container then don't show it.
	if ((proto->containmentSet() & (ProtoObj::kIsContainer | ProtoObj::kIsIntangible)) == (ProtoObj::kIsContainer | ProtoObj::kIsIntangible))
		return true;

	return true;
}

//  total the mass, bulk, and number of all objects in container.
void ContainerView::totalObjects() {
	ObjectID objID;
	GameObject *item = nullptr;

	_totalMass   = 0;
	_totalBulk   = 0;
	_numObjects  = 0;

	if (_containerObject == nullptr) return;

	RecursiveContainerIterator  iter(_containerObject);

	//  See if he has enough currency
	for (objID = iter.first(&item); objID != Nothing; objID = iter.next(&item)) {
		//  If the object is visible, then add to total mass and
		//  bulk.
		if (isVisible(item)) {
			uint16  numItems;

			ProtoObj *proto = item->proto();

			_numObjects++;

			// if it's mergeable calc using the getExtra method of
			// quantity calculation
			// if not, then use the objLoc.z method
			if (proto->flags & ResourceObjectPrototype::kObjPropMergeable)
				numItems = item->getExtra();
			else numItems = 1;

			_totalMass += proto->mass * numItems;
			_totalBulk += proto->bulk * numItems;
		}
	}
}

//  Return the Nth visible object from this container.
ObjectID ContainerView::getObject(int16 slotNum) {
	ObjectID        objID;
	GameObject      *item;

	if (_containerObject == nullptr) return Nothing;

	ContainerIterator   iter(_containerObject);

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
	int16           originX = _extent.x - offset.x + _iconOrigin.x,
	                originY = _extent.y - offset.y + _iconOrigin.y;

	ObjectID        objID;
	GameObject      *item;

	//  Iterator class for the container.
	ContainerIterator   iter(_containerObject);

	//  Color set to draw the object.
	ColorTable      objColors;

	//  If there's no overlap between extent and clip, then return.
	if (!_extent.overlap(r)) return;

	//  Iterate through each item within the container.
	while ((objID = iter.next(&item)) != Nothing) {
		TilePoint   objLoc;
		ProtoObj    *objProto = item->proto();

		objLoc = item->getLocation();

		if (objLoc.z == 0) continue;

		//  Draw object only if visible and in a visible row & col.
		if (objLoc.u >= _scrollPosition
		        &&  objLoc.u < _scrollPosition + _visibleRows
		        &&  objLoc.v < _visibleCols
		        &&  isVisible(item)) {
			Sprite      *spr;
			ProtoObj    *proto = item->proto();
			Point16     sprPos;

			y =     originY
			        + (objLoc.u - _scrollPosition)
			        * (_iconSpacing.y + kIconHeight);
			x =     originX
			        +       objLoc.v
			        * (_iconSpacing.x + kIconWidth);

			//  Get the address of the sprite.
			spr = proto->getSprite(item, ProtoObj::kObjInContainerView).sp;

			sprPos.x = x + ((kIconWidth - spr->size.x) >> 1)
			           - spr->offset.x;
			sprPos.y = y + ((kIconHeight - spr->size.y) >> 1)
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
			if (g_vm->_cnm->_objToGet == item) {
				Point16 selectorPos = Point16(x + ((kIconWidth - kSelectorX) >> 1),
				                              y + ((kIconHeight - kSelectorY) >> 1));

				// draw the selector thingy
				drawSelector(port, selectorPos);

				// set the position of the inc center
				g_vm->_cnm->_amountIndY = y - (kSelectorY >> 1) - 12;
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
	drawCompressedImage(port, pos, g_vm->_cnm->_selImage);

	// draw the number of items selected thus far
	num = Common::sprintf_s(buf, " %d ", g_vm->_cnm->_numPicked);

	port.moveTo(Point16(pos.x - ((3 * (num - 3)) + 1),  pos.y + 7));
	port.setFont(&Helv11Font);
	port.setColor(11);                   // set color to white
	port.setStyle(kTextStyleThickOutline);
	port.setOutlineColor(24);                // set outline color to black
	port.setMode(kDrawModeMatte);

	port.drawText(buf);
}

void ContainerView::drawQuantity(
    gPort           &port,
    GameObject      *item,
    ProtoObj        *objProto,
    int16           x,
    int16           y) {
	int16       quantity;

	quantity = (objProto->flags & ResourceObjectPrototype::kObjPropMergeable)
	           ? item->getExtra()
	           : item->getLocation().z;

	if (quantity > 1) {
		SAVE_GPORT_STATE(port);
		char buf[8];

		// draw the number of items selected thus far
		Common::sprintf_s(buf, "%d", quantity);

		port.moveTo(x - 1,  y + 22);
		port.setFont(&Helv11Font);
		port.setColor(11);                   // set color to white
		port.setStyle(kTextStyleThickOutline);
		port.setOutlineColor(24);                // set outline color to black
		port.setMode(kDrawModeMatte);

		port.drawText(buf);
	}
}

void ContainerView::setContainer(GameObject *containerObj) {
	_containerObject = containerObj;
	totalObjects();
}


//	Get the slot that the point is over
TilePoint ContainerView::pickObjectSlot(const Point16 &pickPos) {
	TilePoint   slot;
	Point16     temp;

	//  Compute the u/v of the slot that they clicked on.
	temp   = pickPos + _iconSpacing / 2 - _iconOrigin;
	slot.v = clamp(0, temp.x / (kIconWidth  + _iconSpacing.x), _visibleCols - 1);
	slot.u = clamp(0, temp.y / (kIconHeight + _iconSpacing.y), _visibleRows - 1) + _scrollPosition;
	slot.z = 1;
	return slot;
}

//	Get the object that the pointer is over
GameObject *ContainerView::getObject(const TilePoint &slot) {
	GameObject *item;
	TilePoint   loc;

	item = _containerObject->child();

	while (item != nullptr) {
		//  Skip objects that are stacked behind other objects
		if (item->getLocation().z != 0) {
			ProtoObj *proto = item->proto();

			loc = item->getLocation();

			if (
			    (loc.u == slot.u) &&
			    (loc.v == slot.v) &&
			    //Skip The Enchantments
			    (!(proto->containmentSet() & ProtoObj::kIsEnchantment))
			) {
				return item;
			}
		}

		item = item->next();
	}

	return nullptr;

}

//	Get the object ID that the point is over
ObjectID ContainerView::pickObjectID(const Point16 &pickPos) {
	TilePoint       slot;
	GameObject      *item;

	slot = pickObjectSlot(pickPos);
	item = getObject(slot);

	if (item != nullptr) {
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

void ContainerView::deactivate() {
}

void ContainerView::pointerMove(gPanelMessage &msg) {
	if (msg._pointerLeave) {
		g_vm->_cnm->_lastPickedObjectID = Nothing;
		g_vm->_cnm->_lastPickedObjectQuantity = -1;
		g_vm->_mouseInfo->setText(nullptr);
		g_vm->_cnm->_mouseText[0] = 0;

		// static bool that tells if the mouse cursor
		// is in a panel
		g_vm->_cnm->_mouseInView = false;
		g_vm->_mouseInfo->setDoable(true);
	} else {
//		if( msg.pointerEnter )
		{
			// static bool that tells if the mouse cursor
			// is in a panel
			g_vm->_cnm->_mouseInView = true;

			GameObject *mouseObject;
			mouseObject = g_vm->_mouseInfo->getObject();

			if (!_node.isAccessable(getCenterActorID())) {
				g_vm->_mouseInfo->setDoable(false);
			} else if (mouseObject == nullptr) {
				g_vm->_mouseInfo->setDoable(true);
			} else {
				g_vm->_mouseInfo->setDoable(_containerObject->canContain(mouseObject->thisID()));
			}
		}

		//  Determine if mouse is pointing at a new object
		updateMouseText(msg._pickPos);
	}
}

bool ContainerView::pointerHit(gPanelMessage &msg) {
	GameObject  *mouseObject;
	GameObject  *slotObject;
	uint16       mouseSet;

	slotObject  = pickObject(msg._pickPos);
	mouseObject = g_vm->_mouseInfo->getObject();
	mouseSet    = mouseObject ? mouseObject->containmentSet() : 0;

	if (!g_vm->_mouseInfo->getDoable()) return false;

	if (msg._doubleClick && !g_vm->_cnm->_alreadyDone) {
		dblClick(mouseObject, slotObject, msg);
	} else { // single click
		if (mouseObject != nullptr) {
			g_vm->_cnm->_alreadyDone = true;    // if object then no doubleClick

			if (g_vm->_mouseInfo->getIntent() == GrabInfo::kIntDrop) {
				if (mouseSet & ProtoObj::kIsTangible) {
					dropPhysical(msg, mouseObject, slotObject, g_vm->_mouseInfo->getMoveCount());
				}

				//  intangibles are used by dropping them
				else if ((mouseSet & ProtoObj::kIsConcept) ||
				         (mouseSet & ProtoObj::kIsPsych) ||
				         (mouseSet & ProtoObj::kIsSpell) ||
				         (mouseSet & ProtoObj::kIsSkill)) {
					useConcept(msg, mouseObject, slotObject);
				} else {
					// !!!! bad state, reset cursor
					g_vm->_mouseInfo->replaceObject();
				}
			} else if (g_vm->_mouseInfo->getIntent() == GrabInfo::kIntUse) {
				if (mouseSet & ProtoObj::kIsTangible) {
					usePhysical(msg, mouseObject, slotObject);
				} else if ((mouseSet & ProtoObj::kIsSpell) ||
				           (mouseSet & ProtoObj::kIsSkill)) {
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
			g_vm->_cnm->_alreadyDone = false;
			clickOn(msg, mouseObject, slotObject);
		}
	}

	// total the mass and bulk of all the objects in this container
	totalObjects();
	_window.update(_extent);

	return activate(kEventMouseDown);
}

void ContainerView::pointerRelease(gPanelMessage &) {
	// see if in multi-item get mode
	if (g_vm->_cnm->_objToGet) {
		g_vm->_cnm->_objToGet->take(getCenterActorID(), g_vm->_cnm->_numPicked);

		// reset the flags and pointer dealing with merged object movement
		g_vm->_cnm->_objToGet = nullptr;
		g_vm->_cnm->_numPicked = 1;
		g_vm->_cnm->_amountIndY = -1;
	}

	gPanel::deactivate();
}

void ContainerView::timerTick(gPanelMessage &msg) {
	// validate objToGet and make sure that the number selected for move
	// is less then or equal to the number of items present in the merged object
	if (g_vm->_cnm->_objToGet && g_vm->_cnm->_amountIndY != -1) {
		int32   rate = (g_vm->_cnm->_amountIndY - msg._pickAbsPos.y);

		rate = rate * ((rate > 0) ? rate : -rate);

		//  Add to the amount accumulator based on the mouse position
		g_vm->_cnm->_amountAccumulator += rate / 4;

		//  Take the top bits of the amount accumulator and add to
		//  the mergeable amount.
		g_vm->_cnm->_numPicked = clamp(1,
		                  g_vm->_cnm->_numPicked + (g_vm->_cnm->_amountAccumulator >> 8),
		                  g_vm->_cnm->_objToGet->getExtra());

		//  Now remove the bits that we added to the grab amount
		//  keep the remaining bits to accumulate for next time
		g_vm->_cnm->_amountAccumulator &= 0x00ff;
	}
}

void ContainerView::dblClick(GameObject *mouseObject, GameObject *slotObject, gPanelMessage &msg) {
	g_vm->_cnm->_alreadyDone = true;

	// double click stuff
	dblClickOn(msg, mouseObject, slotObject);
}

// activate the click function
void ContainerView::clickOn(
    gPanelMessage &,
    GameObject *,
    GameObject *cObj) {
	if (cObj != nullptr) {
		if (cObj->proto()->flags & ResourceObjectPrototype::kObjPropMergeable) {
			if (!rightButtonState()) {
				//  just get the object into the cursor
				cObj->take(getCenterActorID(), cObj->getExtra());
			} else {
				// activate multi-object get interface if a mergeable object
				getMerged(cObj);
				g_vm->_mouseInfo->setText(nullptr);
				g_vm->_cnm->_mouseText[0] = 0;
			}
		} else {
			//  just get the object into the cursor
			cObj->take(getCenterActorID(), g_vm->_cnm->_numPicked);
		}
	}
}

void ContainerView::getMerged(GameObject *obj) {
	// reset the number picked.
	g_vm->_cnm->_numPicked = 1;

	// set the object to be gotten
	g_vm->_cnm->_objToGet = obj;
}

// Activate the double click function
void ContainerView::dblClickOn(
    gPanelMessage &,
    GameObject *mObj,
    GameObject *) {
	if (mObj != nullptr) {
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
	assert(mObj->containmentSet() & ProtoObj::kIsTangible);

	//  Place object back where it came from, temporarily
	g_vm->_mouseInfo->replaceObject();

	//  test to check if item is accepted by container
	if (_containerObject->canContain(mObj->thisID())) {
		Actor       *centerActor = getCenterActor();
		Location    loc(pickObjectSlot(msg._pickPos),
		                _containerObject->thisID());

		//  check if no object in the current slot
		if (cObj == nullptr) {
			MotionTask::dropObject(*centerActor, *mObj, loc, num);

			WriteStatusF(6, "No object state");
		} else {
			//  Try dropping this object on the object in the container
			MotionTask::dropObjectOnObject(*centerActor, *mObj, *cObj, num);
		}

		g_vm->_cnm->_alreadyDone = true;
	}
}

// use a physical object
void ContainerView::usePhysical(
    gPanelMessage   &msg,
    GameObject      *mObj,
    GameObject      *cObj) {
	assert(g_vm->_mouseInfo->getObject() == mObj);
	assert(mObj->containmentSet() & ProtoObj::kIsTangible);

	if (cObj == nullptr) {
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
	assert(mObj->containmentSet() & ProtoObj::kIsIntangible);

	g_vm->_mouseInfo->replaceObject();

	//  Determine if this object can go into this container
	if (_containerObject->canContain(mObj->thisID())) {
		ObjectID    centerActorID = getCenterActorID();

		if (cObj == nullptr) {
			//  If there is no object already in this slot drop the
			//  mouse object here

			Location    loc(pickObjectSlot(msg._pickPos),
			                _containerObject->thisID());

			mObj->drop(centerActorID, loc);
		} else
			//  If there is an object here drop the mouse object onto it
			mObj->dropOn(centerActorID, cObj->thisID());

		g_vm->_cnm->_alreadyDone = true;
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
		g_vm->_mouseInfo->setText(nullptr);
		g_vm->_cnm->_mouseText[0] = 0;

		// reset the last picked thingy
		g_vm->_cnm->_lastPickedObjectID          = Nothing;
		g_vm->_cnm->_lastPickedObjectQuantity    = -1;

		// set the display alarm to false
		g_vm->_cnm->_objTextAlarm = false;

		return;
	}

	// get handles to the object in question
	GameObject  *slotObject = GameObject::objectAddress(slotID);

	if (slotID == g_vm->_cnm->_lastPickedObjectID && slotObject->getExtra() == g_vm->_cnm->_lastPickedObjectQuantity) {
		return; // same object, bug out
	} else {
		// was not same, but is now.
		g_vm->_cnm->_lastPickedObjectID          = slotID;
		g_vm->_cnm->_lastPickedObjectQuantity    = slotObject->getExtra();

		// clear out the mouse text
		g_vm->_mouseInfo->setText(nullptr);
		g_vm->_cnm->_mouseText[0] = 0;

		// reset the alarm flag
		g_vm->_cnm->_objTextAlarm = false;

		// set the hint alarm
		containerObjTextAlarm.set(kTicksPerSecond / 2);

		// put the normalized text into mouseText
		slotObject->objCursorText(g_vm->_cnm->_mouseText, ContainerManager::kBufSize);
	}
}

/* ===================================================================== *
    EnchantmentContainerView member functions
 * ===================================================================== */

EnchantmentContainerView::EnchantmentContainerView(
    gPanelList      &list,
    ContainerNode   &nd,
    const ContainerAppearanceDef &app,
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
	_visibleRows = numRows;
	_visibleCols = numCols;
	_totalRows   = totRows;

	if (backgrounds) {
		_backImages  = backgrounds;
		_numIm       = numRes;
	} else {
		_backImages  = nullptr;
		_numIm       = 0;
	}
}

void ReadyContainerView::setScrollOffset(int8 num) {
	if (num > 0) {
		_scrollPosition = num;
	}
}

void ReadyContainerView::timerTick(gPanelMessage &msg) {
	// validate objToGet and make sure that the number selected for move
	// is less then or equal to the number of items present in the merged object
	if (g_vm->_cnm->_objToGet && g_vm->_cnm->_amountIndY != -1) {
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
	int16           originX = _extent.x - offset.x + _iconOrigin.x,
	                originY = _extent.y - offset.y + _iconOrigin.y;

	//  Row and column number of the inventory slot.
	int16           col,
	                row;

	ObjectID        objID;
	GameObject      *item;

	//  Iterator class for the container.
	ContainerIterator   iter(_containerObject);

	//  Color set to draw the object.
	ColorTable      objColors;

	//  If there's no overlap between extent and clip, then return.
	if (!_extent.overlap(r)) return;

	//  Draw the boxes for visible rows and cols.

	if (_backImages) {
		int16   i;
		Point16 drawOrg(_extent.x - offset.x + kBackOriginX,
		                _extent.y - offset.y + kBackOriginY);

		for (y = drawOrg.y, row = 0;
		        row < _visibleRows;
		        row++, y += _iconSpacing.y + kIconHeight) {
			// reset y for background image stuff
			//y = drawOrg.y;

			for (i = 0, x = drawOrg.x, col = 0;
			        col < _visibleCols;
			        i++, col++, x += _iconSpacing.x + kIconWidth) {
				Point16 pos(x, y);

				if (isGhosted()) drawCompressedImageGhosted(port, pos, _backImages[i % _numIm]);
				else drawCompressedImage(port, pos, _backImages[i % _numIm]);
			}

		}
	} else {
		for (y = originY, row = 0;
		        row < _visibleRows;
		        row++, y += _iconSpacing.y + kIconHeight) {

			for (x = originX, col = 0;
			        col < _visibleCols;
			        col++, x += _iconSpacing.x + kIconWidth) {
				//  REM: We need to come up with some way of
				//  indicating how to render the pattern data which
				//  is behind the object...
				port.setColor(14);
				port.fillRect(x, y, kIconWidth, kIconHeight);

			}

		}

	}

	//  Iterate through each item within the container.
	while ((objID = iter.next(&item)) != Nothing) {

		TilePoint   objLoc;
		ProtoObj    *objProto = item->proto();

		//  If Intangible Container then don't show it.
		if ((objProto->containmentSet() & (ProtoObj::kIsContainer | ProtoObj::kIsIntangible)) == (ProtoObj::kIsContainer | ProtoObj::kIsIntangible))
			continue;

		objLoc = item->getLocation();

		if (objLoc.z == 0) continue;

		//  Draw object only if visible and in a visible row & col.
		if (objLoc.u >= _scrollPosition &&
		        objLoc.u < _scrollPosition + _visibleRows &&
		        objLoc.v < _visibleCols &&
		        isVisible(item)) {
			Sprite      *spr;
			ProtoObj    *proto = item->proto();
			Point16     sprPos;

			y = originY + (objLoc.u - _scrollPosition) * (_iconSpacing.y + kIconHeight);
			x = originX + objLoc.v * (_iconSpacing.x + kIconWidth);

			//  Get the address of the sprite.
			spr = proto->getSprite(item, ProtoObj::kObjInContainerView).sp;

			sprPos.x = x + ((kIconWidth - spr->size.x) >> 1)
			           - spr->offset.x;
			sprPos.y = y + ((kIconHeight - spr->size.y) >> 1)
			           - spr->offset.y;

			if (isGhosted()) return;

			//  Draw the "in use" indicator.
			if (_backImages && proto->isObjectBeingUsed(item)) {
				drawCompressedImage(port,
				                    Point16(x - 4, y - 4), _backImages[3]);
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
			if (g_vm->_cnm->_objToGet == item) {
				Point16 selectorPos = Point16(x + ((kIconWidth - kSelectorX) >> 1),
				                              y + ((kIconHeight - kSelectorY) >> 1));

				// draw the selector thingy
				drawSelector(port, selectorPos);

				// set the position of the inc center
				g_vm->_cnm->_amountIndY = y - (kSelectorY >> 1) + 28;   // extent.y;
			} else drawQuantity(port, item, objProto, x, y);
		}
	}
}

/* ===================================================================== *
    ContainerWindow member functions
 * ===================================================================== */

//  ContainerWindow class

ContainerWindow::ContainerWindow(ContainerNode &nd,
                                 const ContainerAppearanceDef &app,
                                 const char saveas[])
	: FloatingWindow(nd._position, 0, saveas, cmdWindowFunc) {
	//  Initialize view to NULL.
	_view = nullptr;

	// create the close button for this window
	_closeCompButton = new GfxCompButton(
	                      *this,
	                      app.closeRect,              // rect for button
	                      containerRes,               // resource context
	                      app.closeResID[0],
	                      app.closeResID[1],
	                      0,
	                      cmdCloseButtonFunc);        // mind app func
}

//  Virtual destructor (base does nothing)
ContainerWindow::~ContainerWindow() {}

ContainerView &ContainerWindow::getView() {
	return *_view;
}

/* ===================================================================== *
    ScrollableContainerWindow member functions
 * ===================================================================== */

ScrollableContainerWindow::ScrollableContainerWindow(
    ContainerNode &nd, const ContainerAppearanceDef &app, const char saveas[])
	: ContainerWindow(nd, app, saveas) {
	_view = new ContainerView(*this, app.viewRect, nd, app);

	// make the button connected to this window
	_scrollCompButton = new GfxCompButton(
	                       *this,
	                       app.scrollRect,                 // rect for button
	                       containerRes,                   // resource context
	                       app.scrollResID[0],           // resource handle name
	                       app.scrollResID[1],
	                       0,
	                       cmdScrollFunc);                 // mind app func

	assert(_view != nullptr);
	assert(_scrollCompButton != nullptr);
}

/* ===================================================================== *
    TangibleContainerWindow member functions
 * ===================================================================== */

TangibleContainerWindow::TangibleContainerWindow(
    ContainerNode &nd, const ContainerAppearanceDef &app)
	: ScrollableContainerWindow(nd, app, "ObjectWindow") {

	const int weightIndicatorType = 2;
	_objRect = app.iconRect;
	_deathFlag = nd.getType() == ContainerNode::kDeadType;
	_containerSpriteImg = nullptr;

	// setup the mass and weight indicator
	if (_deathFlag) {
		// set the decorations for this window
		setDecorations(deathDecorations,
		               ARRAYSIZE(deathDecorations),
		               containerRes, 'F', 'R', 'M');
		_massWeightIndicator = nullptr;
	} else {
		const StaticWindow *winDecs[] =  {
			brassDecorations,
		    clothDecorations,
		    steelDecorations,
		    woodDecorations
		};
		uint16      bgndType = _view->_containerObject->proto()->appearanceType;

		assert(bgndType < 4);

		setContainerSprite();               // show at the top of the box

		// set the decorations for this window
		setDecorations(winDecs[bgndType],
		               ARRAYSIZE(brassDecorations),    // brass was arb, all should have same
		               containerRes, 'F', 'R', 'M');

		// set the userdata such that we can extract the container object later
		// through an appfunc.
		this->_userData = _view->_containerObject;

		_massWeightIndicator = new CMassWeightIndicator(
		                          this,
		                          Point16(app.massRect.x, app.massRect.y),
		                          weightIndicatorType,
		                          _deathFlag);
	}
}

TangibleContainerWindow::~TangibleContainerWindow() {
	if (_massWeightIndicator)    delete _massWeightIndicator;
	if (_containerSpriteImg)     delete _containerSpriteImg;
}

void TangibleContainerWindow::setContainerSprite() {
	// pointer to sprite data that will be drawn
	Sprite              *spr;
	ProtoObj            *proto = _view->_containerObject->proto();
	Point16             sprPos;
	char                dummy = '\0';

	//  Get the address of the sprite.
	spr = proto->getSprite(_view->_containerObject, ProtoObj::kObjInContainerView).sp;

	sprPos.x = _objRect.x - (spr->size.x >> 1);  //_objRect.x + ( spr->size.x >> 1 );
	sprPos.y = _objRect.y - (spr->size.y >> 1);

	_containerSpriteImg = new GfxSpriteImage(
	                         *this,
	                         Rect16(sprPos.x,
	                                sprPos.y,
	                                _objRect.height,
	                                _objRect.width),
	                         _view->_containerObject,
	                         dummy,
	                         0,
	                         nullptr);
}

void TangibleContainerWindow::massBulkUpdate() {
	if (_massWeightIndicator) {      //  Death container doesn't have MW indicator
		// set the indicators to the correct mass and bulk
		_massWeightIndicator->setMassPie(_view->_totalMass);
		_massWeightIndicator->setBulkPie(_view->_totalBulk);
	}
}

void TangibleContainerWindow::drawClipped(
    gPort &port,
    const Point16 &offset,
    const Rect16 &clip) {
	if (!_extent.overlap(clip)) return;

	// draw the decorations
	ScrollableContainerWindow::drawClipped(port, offset, clip);
}

/* ===================================================================== *
    IntangibleContainerWindow member functions
 * ===================================================================== */

IntangibleContainerWindow::IntangibleContainerWindow(
    ContainerNode &nd, const ContainerAppearanceDef &app)
	: ScrollableContainerWindow(nd, app, "MentalWindow") {
	// make the button connected to this window
	_mindSelectorCompButton = new GfxMultCompButton(
	                             *this,
	                             Rect16(49, 15 - 13, 52, 67),
	                             containerRes,
	                             'H', 'E', 'D', 1, 3, 1,
	                             0,
	                             cmdMindContainerFunc);          // mind app func

	assert(_mindSelectorCompButton != nullptr);

	_mindSelectorCompButton->setResponse(false);

	// set the decorations for this window
	setDecorations(mentalDecorations,
	               ARRAYSIZE(mentalDecorations),
	               containerRes, 'F', 'R', 'M');

	setMindContainer(nd._mindType, *this);
}

/* ===================================================================== *
    EnchantmentContainerWindow member functions
 * ===================================================================== */

EnchantmentContainerWindow::EnchantmentContainerWindow(
    ContainerNode &nd, const ContainerAppearanceDef &app)
	: ContainerWindow(nd, app, "EnchantmentWindow") {
	_view = new EnchantmentContainerView(*this, nd, app);

	// make the button connected to this window
	_scrollCompButton = new GfxCompButton(
	                       *this,
	                       app.scrollRect,                 // rect for button
	                       containerRes,                   // resource context
	                       app.scrollResID[0],           // resource handle name
	                       app.scrollResID[1],
	                       0,
	                       cmdScrollFunc);                 // mind app func

	assert(_view != nullptr);
	assert(_scrollCompButton != nullptr);
}

/* ===================================================================== *
   ContainerNode functions
 * ===================================================================== */

ContainerNode::ContainerNode(ContainerManager &cl, ObjectID id, int typ) {
	GameObject      *obj = GameObject::objectAddress(id);
	PlayerActorID   ownerID;

	//  Convert the possessor() of the object to a player actor ID,
	//  if it is indeed a player actor; Else set to "nobody".
	if (isActor(id)) {
		if (actorIDToPlayerID(id, ownerID) == false)
			ownerID = ContainerNode::kNobody;
	} else {
		ObjectID        possessor = obj->possessor();

		if (possessor == Nothing || actorIDToPlayerID(possessor, ownerID) == false)
			ownerID = ContainerNode::kNobody;
	}

	//  Compute the initial position of the container window
	switch (typ) {
	case kReadyType:
		break;

	case kDeadType:
		_position = deathContainerAppearance.defaultWindowPos;
		break;

	case kMentalType:
		_mindType = 0; //protoClassIdeaContainer;
		_position = mentalContainerAppearance.defaultWindowPos;
		break;

	case kPhysicalType:
		_position = physicalContainerAppearance.defaultWindowPos;
		break;

	case kEnchantType:
		_position = enchantmentContainerAppearance.defaultWindowPos;
		break;
	}

	//  Fill in the initial values.
	_window      = nullptr;
	_type        = typ;
	_object      = id;
	_owner       = ownerID;
	_action      = 0;
	_mindType    = 0;

	//  Add to container list.
	cl.add(this);
}

//  Return the container window for a container node, if it is visible
ContainerWindow *ContainerNode::getWindow() {
	return _window;
}

//  Return the container view for a container node, if it is visible
ContainerView   *ContainerNode::getView() {
	return _window ? &_window->getView() : nullptr;
}

//  Destructor
ContainerNode::~ContainerNode() {
	//  Close the container window.
	hide();

	//  Remove from container list
	g_vm->_cnm->remove(this);
}

void ContainerNode::read(Common::InSaveFile *in) {
	//  Restore fields
	_object = in->readUint16LE();
	_type = in->readByte();
	_owner = in->readByte();
	_position.read(in);
	_mindType = in->readByte();
	_window = nullptr;
	_action = 0;

	bool shown = in->readUint16LE();

	//  If this container was shown, re-show it
	if (shown)
		markForShow();

	debugC(4, kDebugSaveload, "... object = %d", _object);
	debugC(4, kDebugSaveload, "... type = %d", _type);
	debugC(4, kDebugSaveload, "... owner = %d", _owner);
	debugC(4, kDebugSaveload, "... position = (%d, %d, %d, %d)", _position.x, _position.y, _position.width, _position.height);
	debugC(4, kDebugSaveload, "... _mindType = %d", _mindType);
	debugC(4, kDebugSaveload, "... shown = %d", shown);
}

void ContainerNode::write(Common::MemoryWriteStreamDynamic *out) {
	//  Store fields
	out->writeUint16LE(_object);
	out->writeByte(_type);
	out->writeByte(_owner);
	_position.write(out);
	out->writeByte(_mindType);
	out->writeUint16LE(_window != nullptr);

	debugC(4, kDebugSaveload, "... object = %d", _object);
	debugC(4, kDebugSaveload, "... type = %d", _type);
	debugC(4, kDebugSaveload, "... owner = %d", _owner);
	debugC(4, kDebugSaveload, "... position = (%d, %d, %d, %d)", _position.x, _position.y, _position.width, _position.height);
	debugC(4, kDebugSaveload, "... _mindType = %d", _mindType);
	debugC(4, kDebugSaveload, "... shown = %d", _window != nullptr);
}

//  Close the container window, but leave the node.
void ContainerNode::hide() {
	//  close the window, but don't close the object.
	if (_type != kReadyType && _window != nullptr) {
		_position = _window->getExtent();     //  Save old window position
		_window->close();
		delete _window;
		_window = nullptr;
	}
}

//  Open the cotainer window, given the node info.
void ContainerNode::show() {
	ProtoObj        *proto = GameObject::protoAddress(_object);

	assert(proto);

	//  open the window; Object should already be "open"
	if (_window == nullptr) {
		switch (_type) {
		case kPhysicalType:
			physicalContainerAppearance.rows    = proto->getViewableRows();
			physicalContainerAppearance.cols    = proto->getViewableCols();
			physicalContainerAppearance.totRows = proto->getMaxRows();
			_window = new TangibleContainerWindow(*this, physicalContainerAppearance);
			break;

		case kDeadType:
			deathContainerAppearance.rows       = proto->getViewableRows();
			deathContainerAppearance.cols       = proto->getViewableCols();
			deathContainerAppearance.totRows    = proto->getMaxRows();
			_window = new TangibleContainerWindow(*this, deathContainerAppearance);
			break;

		case kMentalType:
			_window = new IntangibleContainerWindow(*this, mentalContainerAppearance);
			break;

		case kEnchantType:
			_window = new EnchantmentContainerWindow(*this, enchantmentContainerAppearance);
			break;

		case kReadyType:
		default:
			return;
		}
	}

	_window->open();
}

void ContainerNode::update() {
	if (_type == kReadyType) {
		//  Update ready containers if they are enabled
		if (TrioCviews[_owner]->getEnabled())  TrioCviews[_owner]->invalidate();
		if (indivCviewTop->getEnabled())        indivCviewTop->invalidate();
		if (indivCviewBot->getEnabled())        indivCviewBot->invalidate();

		//  If the container to update is the center brother's ready container.
		if (isIndivMode() && _owner == getCenterActorPlayerID()) {
			//  Update player's mass & weight indicator...
			MassWeightIndicator->update();
		}
	} else if (_window) {
		getView()->invalidate();
		_window->massBulkUpdate();
	}
}

//  Find a container node, given a specific object
ContainerNode *ContainerManager::find(ObjectID id) {
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it)
		if ((*it)->_object == id)
			return *it;

	return nullptr;
}

ContainerNode *ContainerManager::find(ObjectID id, int16 type) {
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it)
		if ((*it)->_object == id && (*it)->_type == type)
			return *it;

	return nullptr;
}

//  returns true if the object represented by the container can be
//  accessed by the player.
bool ContainerNode::isAccessable(ObjectID enactor) {
	Actor       *a = (Actor *)GameObject::objectAddress(enactor);
	ObjectID    holder;
	GameObject  *obj = GameObject::objectAddress(_object);
	int32       dist;

	//  REM: We really ought to do a line-of-sight test here.

	//  Calculate distance between actor and container.
	dist = (a->getLocation() - obj->getWorldLocation()).quickHDistance();

	//  If the container object is too far away we can't access any containers.
	//  Note: Actors are not considered to be in possession of themselves...
	holder = obj->possessor();
	if (holder != Nothing || isActor(_object)) {
		//  "Reach" for other players is further than for other objects
		if (holder != a->thisID() && dist > 96)
			return false;
	} else if (dist > kMaxOpenDistance)
		return false;

	return true;
}

//  Change the owner of a ready container (for indiv mode)
void ContainerNode::changeOwner(int16 newOwner) {
	_owner = newOwner;
	_object = getPlayerActorAddress(newOwner)->getActorID();
}

/* ===================================================================== *
   ContainerManager functions
 * ===================================================================== */

void ContainerManager::setPlayerNum(PlayerActorID playerNum) {
	//  Close all containers which are not on the ground and not owned
	//  by the current protagonist.
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->_owner != ContainerNode::kNobody && n->_owner != playerNum)
			n->hide();
	}

	//  Open any containers which belong to the new protagonist.
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->_owner == playerNum)
			n->markForShow();
	}
}

void ContainerManager::doDeferredActions() {
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
		if (n->_owner == ContainerNode::kNobody) {
			//  If the object is in a different world, or too far away
			//  from the protagonist, then quietly close the object.
			GameObject  *obj = GameObject::objectAddress(n->_object);
			if (obj->world() != world
			        || (obj->getWorldLocation() - tp).quickHDistance() > kMaxOpenDistance) {
				//  Close object image and window (silently)
				obj->setFlags(0, kObjectOpen);
				delete n;
				continue;
			}
		}

		if (n->_action & ContainerNode::kActionDelete) {
			delete n;
			continue;
		}

		if (n->_action & ContainerNode::kActionHide) {
			n->hide();
		} else {
			if (n->_action & ContainerNode::kActionShow) n->show();
			if (n->_action & ContainerNode::kActionUpdate) n->update();
		}

		n->_action = 0;
	}
}

void ContainerManager::setUpdate(ObjectID id) {
	//  Close all containers which are not on the ground and not owned
	//  by the current protagonist.
	for (Common::List<ContainerNode *>::iterator it = _list.begin(); it != _list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->_object == id)
			n->update();
		else if (n->_type == ContainerNode::kMentalType    //  Special case for mind containers
		         &&  n->_object == GameObject::objectAddress(id)->IDParent())
			n->update();
	}
}

extern int16 openMindType;

//  General function to create a container "node". This determines what
//  kind of container is appropriate, and also if a container of that
//  type is already open.
ContainerNode *CreateContainerNode(ObjectID id, bool open, int16) {
	ContainerNode   *cn = nullptr;
	GameObject      *obj = GameObject::objectAddress(id);
	PlayerActorID   owner;

	if (isActor(id)) {
		if (actorIDToPlayerID(id, owner) == false)
			owner = ContainerNode::kNobody;

		if (((Actor *)obj)->isDead()) {
			//  Open dead container for dead actor
			if (!(cn = g_vm->_cnm->find(owner, ContainerNode::kDeadType)))
				cn = new ContainerNode(*g_vm->_cnm, id, ContainerNode::kDeadType);
		} else if (owner != ContainerNode::kNobody) {
			return OpenMindContainer(owner, open, /*mType*/ openMindType);
		} else {
			error("Attempt to open non-dead actor as a container");
		}
	} else {
		if (actorIDToPlayerID(obj->possessor(), owner) == false)
			owner = ContainerNode::kNobody;

		if (!(cn = g_vm->_cnm->find(id, ContainerNode::kPhysicalType)))
			cn = new ContainerNode(*g_vm->_cnm, id, ContainerNode::kPhysicalType);
	}

	//  If node was successfully created, and we wanted it open, and the owner
	//  is the center actor or no-actor then make the container window visible.
	if (cn != nullptr
	        &&  open
	        && (owner == getCenterActorID() || owner == ContainerNode::kNobody)) {
		cn->show();
	}

	return cn;
}

ContainerNode *CreateReadyContainerNode(PlayerActorID player) {
	return new ContainerNode(*g_vm->_cnm,
	                            getPlayerActorAddress(player)->getActorID(),
	                            ContainerNode::kReadyType);
}

ContainerNode *OpenMindContainer(PlayerActorID player, int16 open, int16 type) {
	ContainerNode   *cn;
	ObjectID        id = getPlayerActorAddress(player)->getActorID();

	if (!(cn = g_vm->_cnm->find(id, ContainerNode::kMentalType))) {
		cn = new ContainerNode(*g_vm->_cnm, id, ContainerNode::kMentalType);
		cn->_mindType = type;

		//  If node was successfully created, and we wanted it open, and the owner
		//  is the center actor or no-actor then make the container window visible.
		if (open && id == getCenterActorID()) {
			cn->show();
		}
	} else {
		IntangibleContainerWindow   *cw = (IntangibleContainerWindow *)cn->getWindow();

		if (cw && (type != cn->_mindType)) {
			cn->_mindType = type;
			setMindContainer(cn->_mindType, *cw);
			cw->update(cw->getView().getExtent());
		}
	}
	return cn;
}

/* ===================================================================== *
    Misc. functions
 * ===================================================================== */

void initContainers() {
	if (g_vm->getGameId() == GID_DINO) {
		warning("TODO: initContainers() for Dino");
		return;
	}

	if (containerRes == nullptr)
		containerRes = resFile->newContext(MKTAG('C', 'O', 'N', 'T'), "cont.resources");

	g_vm->_cnm->_selImage = g_vm->_imageCache->requestImage(imageRes, MKTAG('A', 'M', 'N', 'T'));
}

void cleanupContainers() {
	if (g_vm->_cnm->_selImage)
		g_vm->_imageCache->releaseImage(g_vm->_cnm->_selImage);
	if (containerRes)
		resFile->disposeContext(containerRes);

	g_vm->_cnm->_selImage = nullptr;
	containerRes = nullptr;
}

void initContainerNodes() {
	//  Verify the globalContainerList only has ready ContainerNodes

	Common::List<ContainerNode *>::iterator it;

	for (it = g_vm->_cnm->_list.begin(); it != g_vm->_cnm->_list.end(); ++it) {
		if ((*it)->getType() != ContainerNode::kReadyType) {
			error("initContainerNodes: ContainerNode type not readyType (%d != %d)", (*it)->getType(), ContainerNode::kReadyType);
		}
	}
}

void saveContainerNodes(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Container Nodes");

	int i = 0;
	int16 numNodes = 0;

	//  Make sure there are no pending container view actions
	g_vm->_cnm->doDeferredActions();

	//  Count the number of nodes to save
	for (Common::List<ContainerNode *>::iterator it = g_vm->_cnm->_list.begin(); it != g_vm->_cnm->_list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->getType() != ContainerNode::kReadyType)
			numNodes++;
	}

	outS->write("CONT", 4);
	CHUNK_BEGIN;
	//  Store the number of nodes to save
	out->writeSint16LE(numNodes);

	debugC(3, kDebugSaveload, "... numNodes = %d", numNodes);

	//  Store the nodes
	for (Common::List<ContainerNode *>::iterator it = g_vm->_cnm->_list.begin(); it != g_vm->_cnm->_list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->getType() != ContainerNode::kReadyType) {
			debugC(3, kDebugSaveload, "Saving ContainerNode %d", i++);
			n->write(out);
		}
	}
	CHUNK_END;
}

void loadContainerNodes(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading Container Nodes");

	ContainerNode *node;
	Common::List<ContainerNode *> tempList;
	int16 numNodes;

	//  Read in the number of container nodes to restore
	numNodes = in->readSint16LE();
	debugC(3, kDebugSaveload, "... numNodes = %d", numNodes);

	for (int i = 0; i < numNodes; i++) {
		debugC(3, kDebugSaveload, "Loading ContainerNode %d", i);

		node = new ContainerNode;

		//  Restore the state of the node
		node->read(in);

		//  Add it back to the container list
		g_vm->_cnm->add(node);
	}

	assert(tempList.empty());
}

void cleanupContainerNodes() {
	if (g_vm->_cnm == nullptr)
		return;

	Common::Array<ContainerNode *> deletionArray;

	for (Common::List<ContainerNode *>::iterator it = g_vm->_cnm->_list.begin(); it != g_vm->_cnm->_list.end(); ++it) {
		ContainerNode *n = *it;

		if (n->getType() != ContainerNode::kReadyType)
			deletionArray.push_back(*it);
	}

	for (uint i = 0; i < deletionArray.size(); ++i)
		delete deletionArray[i];
}

void updateContainerWindows() {
	// TODO: updateContainerWindows() for Dino
	if (g_vm->getGameId() == GID_DINO)
		return;

	g_vm->_cnm->doDeferredActions();
}

void setMindContainer(int index, IntangibleContainerWindow &cw) {
	const int classTable[] = {
		protoClassIdeaContainer,
		protoClassSkillContainer,
		protoClassMemoryContainer,
		protoClassPsychContainer    // Not used anymore
	};

	ObjectID        ownerID = cw.getView()._node.getObject();
	GameObject      *object = GameObject::objectAddress(ownerID);
	ContainerIterator iter(object);
	GameObject      *item;
	ObjectID        id;

	assert(index >= 0);
	assert(index < ARRAYSIZE(classTable));

	int             containerClass = classTable[index];

	cw._mindSelectorCompButton->setCurrent(index);
	cw._mindSelectorCompButton->invalidate();

	while ((id = iter.next(&item)) != Nothing) {
		if (item->proto()->classType == containerClass) {
			cw._view->setContainer(item);
			return;
		}
	}
}

APPFUNC(cmdMindContainerFunc) {
	if (ev.panel && ev.eventType == kEventNewValue /* && ev.value */) {
		IntangibleContainerWindow   *cw = (IntangibleContainerWindow *)ev.window;
		ContainerNode   &nd = cw->getView()._node;
		int             newMindType = nd._mindType;

		const Rect16 idea(0, 0, 22, 67),      // idea button click area
		             skill(22, 0, 11, 67),    // skill area
		             memory(33, 0,  9, 67),   // memory area
		             psych(42, 0, 10, 67);    // psych(ic?) area

		if (idea.ptInside(ev.mouse))    newMindType = 0; //protoClassIdeaContainer;
		if (skill.ptInside(ev.mouse))   newMindType = 1; //protoClassSkillContainer;
		if (memory.ptInside(ev.mouse))  newMindType = 2; //protoClassMemoryContainer;
//		if (psych.ptInside(ev.mouse))   newMindType = protoClassPsychContainer;

		if (newMindType != nd._mindType) {
			nd._mindType = newMindType;
			setMindContainer(nd._mindType, *cw);
			cw->update(cw->getView().getExtent());
		}
	} else if (ev.eventType == kEventMouseMove) {
		//if (ev.value == gCompImage::enter)
		{
			const Rect16 idea(0, 0, 22, 67),      // idea button click area
			             skill(22, 0, 11, 67),    // skill area
			             memory(33, 0,  9, 67);   // memory area


			const int BUF_SIZE = 64;
			char    textBuffer[BUF_SIZE];
			int     _mindType = -1;


			if (idea.ptInside(ev.mouse))       _mindType = 0;    //protoClassIdeaContainer;
			if (skill.ptInside(ev.mouse))  _mindType = 1;    //protoClassSkillContainer;
			if (memory.ptInside(ev.mouse)) _mindType = 2;    //protoClassMemoryContainer;

			switch (_mindType) {
			case 0:
				Common::sprintf_s(textBuffer, IDEAS_MENTAL);
				break;

			case 1:
				Common::sprintf_s(textBuffer, SPELL_MENTAL);
				break;

			case 2:
				Common::sprintf_s(textBuffer, SKILL_MENTAL);
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

		if (ev.value == GfxCompImage::kLeave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

APPFUNC(cmdCloseButtonFunc) {
	if (ev.eventType == kEventNewValue && ev.value == 1) {
		ContainerWindow     *win = (ContainerWindow *)ev.window;

		if (win->getView()._node.getType() == ContainerNode::kMentalType) {
			win->getView()._node.markForDelete();
		} else {
			win->containerObject()->close(getCenterActorID());
		}
		updateContainerWindows();

		// make sure the hint text goes away
		if (g_vm->_mouseInfo->getObject() == nullptr) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	} else if (ev.eventType == kEventMouseMove) {
		if (ev.value == GfxCompImage::kEnter) {
			g_vm->_mouseInfo->setText(CLOSE_MOUSE);
		} else if (ev.value == GfxCompImage::kLeave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

APPFUNC(cmdScrollFunc) {
	if (ev.panel && ev.eventType == kEventNewValue && ev.value) {
		ScrollableContainerWindow *cw;
		const Rect16 upArea(0, 0, 44, 22);

		cw = (ScrollableContainerWindow *)ev.window;
		if (upArea.ptInside(ev.mouse))
			cw->scrollUp();
		else
			cw->scrollDown();
		ev.window->update(cw->getView().getExtent());
	} else if (ev.eventType == kEventMouseMove) {
		if (ev.value == GfxCompImage::kEnter) {
			g_vm->_mouseInfo->setText(SCROLL_MOUSE);
		} else if (ev.value == GfxCompImage::kLeave) {
			g_vm->_mouseInfo->setText(nullptr);
		}
	}
}

} // end of namespace Saga2
