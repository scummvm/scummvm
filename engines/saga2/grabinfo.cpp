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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/gdraw.h"
#include "saga2/images.h"
#include "saga2/sprite.h"
#include "saga2/objects.h"
#include "saga2/contain.h"
#include "saga2/mouseimg.h"
#include "saga2/grabinfo.h"
#include "saga2/methods.r"

namespace Saga2 {

GrabInfo mouseInfo;

/* ===================================================================== *
   GrabInfo members
 * ===================================================================== */


GrabInfo::GrabInfo() {
	pointerMap.size.x = 0;
	pointerMap.size.y = 0;
	pointerMap.data = NULL;

	// null out the "held" object
	grabId      = Nothing;
	grabObj     = NULL;
	intentDoable = TRUE;
	intention   = WalkTo;

	textBuf[ 0 ] = '\0';
	displayGauge = FALSE;
}

GrabInfo::~GrabInfo() {
	if (pointerMap.data != NULL)
		RDisposePtr(pointerMap.data);
}

// set the move count based on val and whether the object is
// mergeable or not.
void GrabInfo::setMoveCount(int16 val) {
	if (grabObj) {
		if (grabObj->proto()->flags & ResourceObjectPrototype::objPropMergeable) {
			moveCount = val;
		} else {
			moveCount = 1;
		}
	}
}

//  Make the object ID given into the mouse pointer
void GrabInfo::grabObject(ObjectID objid,  Intent in, int16 count) {
	GameObject  *obj = GameObject::objectAddress(objid);
	grabObject(obj, in, count);
}


//	Make the object pointer into the mouse pointer
void GrabInfo::grabObject(GameObject *obj,  Intent in, int16 count) {
	char        objText[ bufSize ];

	assert(!obj->isMoving());

	// set the number of items
	setMoveCount(count);

	//  Get address of object
	grabObj     = obj;
	grabId      = grabObj->thisID();

	// set the number of items
	setMoveCount(count);

	// get the original location
	from            = grabObj->getLocation();
	from.context    = grabObj->IDParent();
	// de-link the object
	grabObj->move(Location(Nowhere, Nothing));

	setIcon();
	setIntent(in);

	//  Display the name of the grabbed object under the mouse cursor
	grabObj->objCursorText(objText, bufSize, moveCount);
	setMouseText(objText);

	clearMouseGauge();
}

//  this allows the mouse pointer to reference the original object
//  without removing it from the container/land
//  it does not actually copy it
//	Make the object ID given into the mouse pointer
void GrabInfo::copyObject(ObjectID objid,  Intent in, int16 count) {
	GameObject      *obj = GameObject::objectAddress(objid);
	copyObject(obj, in, count);
}


//  this allows the mouse pointer to reference the original object
//  without removing it from the container/land
//  it does not actually copy it
//	Make the object pointer into the mouse pointer
void GrabInfo::copyObject(GameObject *obj,  Intent in, int16 count) {
	char        objText[ bufSize ];

	// set the number of items
	setMoveCount(count);

	//  Get address of object, and address of object prototype
	grabObj     = obj;
	grabId      = grabObj->thisID();

	// set the number of items
	setMoveCount(count);

	from            = Nowhere;
	from.context    = Nothing;

	setIcon();
	setIntent(in);

	//  Display the name of the grabbed object under the mouse cursor
	grabObj->objCursorText(objText, bufSize, moveCount);
	setMouseText(objText);

	clearMouseGauge();
}


//  Set a new intention.  All changes to intention must use this function.
uint8 GrabInfo::setIntent(uint8 in) {
	//  If intention isn't being changed, return immediately
	if (intention != (Intent)in) {
		//  Intention has changed to None
		if (in == (uint8)None && intention != None) pointer.hide();
		//  Intention has changed from None
		else if (in != (uint8)None && intention == None) pointer.show();

		intention = (Intent)in;
		//  Set new cursor
		setCursor();
	}

	return in;
}


//	Make the object given into the mouse pointer
void GrabInfo::setIcon(void) {
	assert(
	    pointerMap.size.x == 0
	    &&  pointerMap.size.y == 0
	    &&  pointerMap.data == NULL);

	assert(grabObj != NULL && isObject(grabObj));

	Sprite          *spr;
	ProtoObj        *proto;
	ColorTable      mainColors;             // colors for object
	uint8           *mapData;
	int32           mapBytes;

	//  Get address of object, and address of object prototype
	proto = grabObj->proto();

	//  Get address of sprite
	spr = proto->getSprite(grabObj, ProtoObj::objAsMousePtr, moveCount).sp;
	mapBytes = spr->size.x * spr->size.y;

	if ((mapData
	        = (uint8 *)RNewPtr(mapBytes, NULL, "Mouse icon data"))
	        !=  NULL) {
		//  Clear out the image data
		memset(mapData, 0, mapBytes);

		//  Build the current color table for the object
		grabObj->getColorTranslation(mainColors);

		pointerMap.size = spr->size;
		pointerMap.data = mapData;

		pointerOffset.x = - spr->size.x / 2;
		pointerOffset.y = - spr->size.y / 2;

		//  Render the sprite into the bitmap
		ExpandColorMappedSprite(pointerMap, spr, mainColors);
	} else
		error("Unable to allocate mouse image buffer");
}

void GrabInfo::clearIcon(void) {
	assert(grabObj == NULL);

	if (pointerMap.data != NULL) {
		RDisposePtr(pointerMap.data);
		pointerMap.size.x = 0;
		pointerMap.size.y = 0;
		pointerMap.data = NULL;
	}
}

//  Changes cursor image to reflect the current state of the cursor based
//  on the intention and intentDoable data members.
void GrabInfo::setCursor(void) {
	if (intentDoable) {
		switch (intention) {
		case None:
			//  If intention has been changed to none then the
			//  pointer has already been hidden.
			break;
		case WalkTo:
			setMouseImage(ArrowImage, 0, 0);
			break;
		case Open:
			setMouseImage(ArrowImage, 0, 0);
			break;
		case PickUp:
			setMouseImage(GrabPtrImage, -7, -7);
			break;
		case Drop:
			setMouseImage(pointerMap, pointerOffset.x, pointerOffset.y);
			break;
		case Use:
			setMouseImage(UsePtrImage, -7, -7);
			break;
		case Attack:
			setMouseImage(AttakPtrImage, -11, -11);
			break;
		case Cast:
			setMouseImage(AttakPtrImage, -11, -11);
			break;
		}
	} else {
		//  indicate current intention is not doable
		setMouseImage(XPointerImage, -7, -7);
	}
}

void GrabInfo::placeObject(const Location &loc) {
	grabObj->move(loc);

	//  Turn off state variables
	grabObj    = NULL;
	grabId     = Nothing;
	intentDoable = TRUE;
	setIntent(WalkTo);
	clearIcon();

	//  Display the saved text
	setMouseText(textBuf[ 0 ] != '\0' ? textBuf : NULL);

	//  Display the saved gauge data
	if (displayGauge)
		setMouseGauge(gaugeNumerator, gaugeDenominator);
	else
		clearMouseGauge();
}

// this should be use to return the object to the container
// and/or remove the object from the cursor.
void GrabInfo::replaceObject(void) {
	if (grabObj == NULL)
		return;

	// if actually attached to cursor, replace
	if (grabObj->IDParent() == Nothing) {
//		ContainerView *updateView;

		grabObj->move(from);

		//  Update the ContainerView from which the object came
//		updateView = ContainerView::findPane( grabObj->parent() );
//		if ( updateView != NULL )
//			( updateView->getWindow() )->update( updateView->getExtent() );
	}

	//  Turn off state variables
	grabObj    = NULL;
	grabId     = Nothing;
	intentDoable = TRUE;
	setIntent(WalkTo);
	clearIcon();

	//  Display the saved text
	setMouseText(textBuf[ 0 ] != '\0' ? textBuf : NULL);

	//  Display the saved gauge data
	if (displayGauge)
		setMouseGauge(gaugeNumerator, gaugeDenominator);
	else
		clearMouseGauge();
}


//  This function is called to request a new mouse text string.  If there
//  is currently no grabbed object the text will be displayed, else the
//  text pointer will simply be saved.
void GrabInfo::setText(const char *txt) {
	if ((txt != NULL) && strlen(txt)) {
		strncpy(textBuf, txt, bufSize);
		textBuf[ bufSize - 1 ] = '\0';
		if (grabObj == NULL)
			setMouseText(textBuf);
	} else {
		textBuf[ 0 ] = '\0';
		if (grabObj == NULL)
			setMouseText(NULL);
	}
}

//	request a change to the mouse gauge
void GrabInfo::setGauge(int16 numerator, int16 denominator) {
	displayGauge = TRUE;
	gaugeNumerator = numerator;
	gaugeDenominator = denominator;
	if (grabObj == NULL)
		setMouseGauge(gaugeNumerator, gaugeDenominator);
}

//	clear the mouse gauge
void GrabInfo::clearGauge(void) {
	displayGauge = FALSE;
	if (grabObj == NULL) clearMouseGauge();
}

#if CURSOR_CYCLING

extern bool eyeEnabled;
extern bool walkEnabled;

void cycleCursor() {

	if (! mouseInfo.cursorCyclingEnabled)
		return;

	uint8 curIntent;
	curIntent  = mouseInfo.getIntent();
	if (++curIntent == GrabInfo::IntentCounts)
		mouseInfo.setIntent(1); //Set Cursor First State Skip None
	else {
		if (!walkEnabled && curIntent == GrabInfo::WalkTo)
			++curIntent;
		if (curIntent == GrabInfo::PickUp && mouseInfo.getObject() != NULL)
			++curIntent;
		if (curIntent == GrabInfo::Drop && mouseInfo.getObject() == NULL)
			++curIntent;
		if (!eyeEnabled && curIntent == GrabInfo::LookAt)
			++curIntent;
		if (curIntent == GrabInfo::Navigate) {
			if (walkEnabled)
				curIntent = 1; //Set Cursor First State Skip None
			else {
				curIntent = 2;
				if (curIntent == GrabInfo::PickUp && mouseInfo.getObject() != NULL)
					++curIntent;
			}
		}
		mouseInfo.setIntent(curIntent); //Set Cursor To Next State
	}

}
#endif

} // end of namespace Saga2
