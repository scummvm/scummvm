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
 */

#include "twp/twp.h"
#include "twp/sqgame.h"
#include "twp/squtil.h"
#include "twp/object.h"
#include "twp/room.h"
#include "squirrel/squirrel.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqobject.h"
#include "squirrel/sqstring.h"
#include "squirrel/sqstate.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqstdstring.h"
#include "squirrel/sqstdmath.h"
#include "squirrel/sqstdio.h"
#include "squirrel/sqstdaux.h"
#include "squirrel/sqfuncproto.h"
#include "squirrel/sqclosure.h"

namespace Twp {

extern TwpEngine *g_engine;

// Creates a new, room local object using sheet as the sprite sheet and image as the image name.
// This object is deleted when the room exits.
// If sheet parameter not provided, use room's sprite sheet instead.
// If image is an array, then use that as a sequence of frames for animation.
// Objects created at runtime can be passed to all the object commands.
// They do not have verbs or local variables by default, but these can be added when the object is created so it can be used in the construction of sentences.
static SQInteger createObject(HSQUIRRELVM v) {
	SQInteger numArgs = sq_gettop(v);
	Common::String sheet;
	Common::Array<Common::String> frames;
	SQInteger framesIndex = 2;

	// get sheet parameter if any
	if (numArgs == 3) {
		if (SQ_FAILED(sqget(v, 2, sheet)))
			return sq_throwerror(v, "failed to get sheet");
		framesIndex = 3;
	}

	// get frames parameter if any
	if (numArgs >= 2) {
		switch (sq_gettype(v, framesIndex)) {
		case OT_STRING: {
			Common::String frame;
			sqget(v, framesIndex, frame);
			frames.push_back(frame);
		} break;
		case OT_ARRAY:
			sqgetarray(v, framesIndex, frames);
			break;
		default:
			return sq_throwerror(v, "Invalid parameter 2: expecting a string or an array");
		}
	}

	debug("Create object: %s, %u", sheet.c_str(), frames.size());
	Object *obj = g_engine->_room->createObject(sheet, frames);
	sq_pushobject(v, obj->_table);

	return 1;
}

static SQInteger createTextObject(HSQUIRRELVM v) {
	// Creates a text object of the given size.
	// TextObjects can be passed to all the object commands, but like objects created with createObject they don't have verbs or local variables by default.
	// If alignment specified, it should take the form of: verticalAlign| horizonalAlign [| horiztonalWidth ].
	// Valid values for verticalAlign are ALIGN_TOP|ALIGN_BOTTOM. Valid values for horizonalAlign are ALIGN_LEFT|ALIGN_CENTER|ALIGN_RIGHT.
	// If the optional horiztonalWidth parameter is present, it will wrap the text to that width.
	const SQChar *fontName;
	if (SQ_FAILED(sqget(v, 2, fontName)))
		return sq_throwerror(v, "failed to get fontName");
	const SQChar *text;
	if (SQ_FAILED(sqget(v, 3, text)))
		return sq_throwerror(v, "failed to get text");
	TextHAlignment thAlign = thCenter;
	TextVAlignment tvAlign = tvCenter;
	float maxWidth = 0.0f;
	if (sq_gettop(v) == 4) {
		int align;
		if (SQ_FAILED(sqget(v, 4, align)))
			return sq_throwerror(v, "failed to get align");
		uint64 hAlign = align & 0x0000000070000000;
		uint64 vAlign = align & 0xFFFFFFFFA1000000;
		maxWidth = (align & 0x00000000000FFFFF);
		switch (hAlign) {
		case 0x0000000010000000:
			thAlign = thLeft;
		case 0x0000000020000000:
			thAlign = thCenter;
		case 0x0000000040000000:
			thAlign = thRight;
		default:
			return sq_throwerror(v, "failed to get halign");
		}
		switch (vAlign) {
		case 0xFFFFFFFF80000000:
			tvAlign = tvTop;
		case 0x0000000001000000:
			tvAlign = tvBottom;
		default:
			tvAlign = tvTop;
		}
	}
	debug("Create text %d, %d, max=%f, text=%s", thAlign, tvAlign, maxWidth, text);
	Object *obj = g_engine->_room->createTextObject(fontName, text, thAlign, tvAlign, maxWidth);
	sqpush(v, obj->_table);
	return 1;
}

// Deletes object permanently from the game.
//
// .. code-block:: Squirrel
// local drip = createObject("drip")
// local time = 1.5
// objectAt(drip, 432, 125)
// objectOffsetTo(drip, 0, -103, time, SLOW_EASE_IN)
// breaktime(time)
// playObjectSound(randomfrom(soundDrip1, soundDrip2, soundDrip3), radioStudioBucket)
// deleteObject(drip)
static SQInteger deleteObject(HSQUIRRELVM v) {
	// TODO: deleteObject
	warning("deleteObject not implemented");
	return 0;
}

// Returns the object that is at the specified coordinates.
// If there is no object at those coordinates, it returns NULL.
// Used for determining what the player is clicking on now (e.g. for the phone).
//
// .. code-block:: Squirrel
// local button = findObjectAt(x,y)
// if (button == null)
//     return NO
// if (objectState(button) == OFF) {
//     if (button == Phone.phoneReceiver) {    ... }
// }
static SQInteger findObjectAt(HSQUIRRELVM v) {
	// TODO: findObjectAt
	warning("findObjectAt not implemented");
	return 0;
}

static SQInteger isInventoryOnScreen(HSQUIRRELVM v) {
	// TODO: isInventoryOnScreen
	warning("isInventoryOnScreen not implemented");
	return 0;
}

static SQInteger isObject(HSQUIRRELVM v) {
	// TODO: isObject
	warning("isObject not implemented");
	return 0;
}

static SQInteger jiggleInventory(HSQUIRRELVM v) {
	// TODO: jiggleInventory
	warning("jiggleInventory not implemented");
	return 0;
}

static SQInteger jiggleObject(HSQUIRRELVM v) {
	// TODO: jiggleObject
	warning("jiggleObject not implemented");
	return 0;
}

static SQInteger loopObjectState(HSQUIRRELVM v) {
	// TODO: loopObjectState
	warning("loopObjectState not implemented");
	return 0;
}

static SQInteger objectAlpha(HSQUIRRELVM v) {
	// TODO: objectAlpha
	warning("objectAlpha not implemented");
	return 0;
}

static SQInteger objectAlphaTo(HSQUIRRELVM v) {
	// TODO: objectAlphaTo
	warning("objectAlphaTo not implemented");
	return 0;
}

static SQInteger objectBumperCycle(HSQUIRRELVM v) {
	// TODO: objectBumperCycle
	warning("objectBumperCycle not implemented");
	return 0;
}

static SQInteger objectCenter(HSQUIRRELVM v) {
	// TODO: objectCenter
	warning("objectCenter not implemented");
	return 0;
}

static SQInteger objectColor(HSQUIRRELVM v) {
	// TODO: objectColor
	warning("objectColor not implemented");
	return 0;
}

static SQInteger objectDependentOn(HSQUIRRELVM v) {
	// TODO: objectDependentOn
	warning("objectDependentOn not implemented");
	return 0;
}

static SQInteger objectFPS(HSQUIRRELVM v) {
	// TODO: objectFPS
	warning("objectFPS not implemented");
	return 0;
}

static SQInteger objectHidden(HSQUIRRELVM v) {
	// TODO: objectHidden
	warning("objectHidden not implemented");
	return 0;
}

static SQInteger objectHotspot(HSQUIRRELVM v) {
	// TODO: objectHotspot
	warning("objectHotspot not implemented");
	return 0;
}

static SQInteger objectIcon(HSQUIRRELVM v) {
	// TODO: objectIcon
	warning("objectIcon not implemented");
	return 0;
}

static SQInteger objectLit(HSQUIRRELVM v) {
	// TODO: objectLit
	warning("objectLit not implemented");
	return 0;
}

static SQInteger objectMoveTo(HSQUIRRELVM v) {
	// TODO: objectMoveTo
	warning("objectMoveTo not implemented");
	return 0;
}

static SQInteger objectOffset(HSQUIRRELVM v) {
	// TODO: objectOffset
	warning("objectOffset not implemented");
	return 0;
}

static SQInteger objectOffsetTo(HSQUIRRELVM v) {
	// TODO: objectOffsetTo
	warning("objectOffsetTo not implemented");
	return 0;
}

static SQInteger objectOwner(HSQUIRRELVM v) {
	// TODO: objectOwner
	warning("objectOwner not implemented");
	return 0;
}

static SQInteger objectParallaxLayer(HSQUIRRELVM v) {
	// TODO: objectParallaxLayer
	warning("objectParallaxLayer not implemented");
	return 0;
}

static SQInteger objectParent(HSQUIRRELVM v) {
	// TODO: objectParent
	warning("objectParent not implemented");
	return 0;
}

static SQInteger objectPosX(HSQUIRRELVM v) {
	// TODO: objectPosX
	warning("objectPosX not implemented");
	return 0;
}

static SQInteger objectPosY(HSQUIRRELVM v) {
	// TODO: objectPosY
	warning("objectPosY not implemented");
	return 0;
}

static SQInteger objectRenderOffset(HSQUIRRELVM v) {
	// TODO: objectRenderOffset
	warning("objectRenderOffset not implemented");
	return 0;
}

static SQInteger objectRoom(HSQUIRRELVM v) {
	// TODO: objectRoom
	warning("objectRoom not implemented");
	return 0;
}

static SQInteger objectRotate(HSQUIRRELVM v) {
	// TODO: objectRotate
	warning("objectRotate not implemented");
	return 0;
}

static SQInteger objectRotateTo(HSQUIRRELVM v) {
	// TODO: objectRotateTo
	warning("objectRotateTo not implemented");
	return 0;
}

static SQInteger objectScale(HSQUIRRELVM v) {
	// TODO: objectScale
	warning("objectScale not implemented");
	return 0;
}

static SQInteger objectScaleTo(HSQUIRRELVM v) {
	// TODO: objectScaleTo
	warning("objectScaleTo not implemented");
	return 0;
}

static SQInteger objectScreenSpace(HSQUIRRELVM v) {
	// TODO: objectScreenSpace
	warning("objectScreenSpace not implemented");
	return 0;
}

static SQInteger objectShader(HSQUIRRELVM v) {
	// TODO: objectShader
	warning("objectShader not implemented");
	return 0;
}

static SQInteger objectState(HSQUIRRELVM v) {
	// TODO: objectState
	warning("objectState not implemented");
	return 0;
}

static SQInteger objectTouchable(HSQUIRRELVM v) {
	// TODO: objectTouchable
	warning("objectTouchable not implemented");
	return 0;
}

static SQInteger objectSort(HSQUIRRELVM v) {
	// TODO: objectSort
	warning("objectSort not implemented");
	return 0;
}

static SQInteger objectUsePos(HSQUIRRELVM v) {
	// TODO: objectUsePos
	warning("objectUsePos not implemented");
	return 0;
}

static SQInteger objectUsePosX(HSQUIRRELVM v) {
	// TODO: objectUsePosX
	warning("objectUsePosX not implemented");
	return 0;
}

static SQInteger objectUsePosY(HSQUIRRELVM v) {
	// TODO: objectUsePosY
	warning("objectUsePosY not implemented");
	return 0;
}

static SQInteger objectValidUsePos(HSQUIRRELVM v) {
	// TODO: objectValidUsePos
	warning("objectValidUsePos not implemented");
	return 0;
}

static SQInteger objectValidVerb(HSQUIRRELVM v) {
	// TODO: objectValidVerb
	warning("objectValidVerb not implemented");
	return 0;
}

static SQInteger pickupObject(HSQUIRRELVM v) {
	// TODO: pickupObject
	warning("pickupObject not implemented");
	return 0;
}

static SQInteger pickupReplacementObject(HSQUIRRELVM v) {
	// TODO: pickupReplacementObject
	warning("pickupReplacementObject not implemented");
	return 0;
}

static SQInteger playObjectState(HSQUIRRELVM v) {
	// TODO: playObjectState
	warning("playObjectState not implemented");
	return 0;
}

static SQInteger popInventory(HSQUIRRELVM v) {
	// TODO: popInventory
	warning("popInventory not implemented");
	return 0;
}

static SQInteger removeInventory(HSQUIRRELVM v) {
	// TODO: removeInventory
	warning("removeInventory not implemented");
	return 0;
}

static SQInteger setDefaultObject(HSQUIRRELVM v) {
	// TODO: setDefaultObject
	warning("setDefaultObject not implemented");
	return 0;
}

static SQInteger shakeObject(HSQUIRRELVM v) {
	// TODO: shakeObject
	warning("shakeObject not implemented");
	return 0;
}

static SQInteger stopObjectMotors(HSQUIRRELVM v) {
	// TODO: stopObjectMotors
	warning("stopObjectMotors not implemented");
	return 0;
}

static SQInteger objectAt(HSQUIRRELVM v) {
	HSQOBJECT o;
	sq_getstackobj(v, 2, &o);

	SQInteger id;
	sqgetf(o, "_id", id);

	Object **pObj = Common::find_if(g_engine->_objects.begin(), g_engine->_objects.end(), [&](Object *o) {
		SQObjectPtr id2;
		_table(o->_table)->Get(sqtoobj(v, "_id"), id2);
		return id == _integer(id2);
	});

	if (!pObj)
		return sq_throwerror(v, "failed to get object");

	// TODO:
	// Object* obj = *pObj;
	// SQInteger x, y;
	// if (SQ_FAILED(sq_getinteger(v, 3, &x)))
	// 	return sq_throwerror(v, "failed to get x");
	// if (SQ_FAILED(sq_getinteger(v, 4, &y)))
	// 	return sq_throwerror(v, "failed to get y");
	// obj->x = x;
	// obj->y = y;
	// debug("Object at: %lld, %lld", x, y);

	return 0;
}

void sqgame_register_objlib(HSQUIRRELVM v) {
	regFunc(v, createObject, _SC("createObject"));
	regFunc(v, createTextObject, _SC("createTextObject"));
	regFunc(v, deleteObject, _SC("deleteObject"));
	regFunc(v, findObjectAt, _SC("findObjectAt"));
	regFunc(v, isInventoryOnScreen, _SC("isInventoryOnScreen"));
	regFunc(v, isObject, _SC("is_object"));
	regFunc(v, isObject, _SC("isObject"));
	regFunc(v, jiggleInventory, _SC("jiggleInventory"));
	regFunc(v, jiggleObject, _SC("jiggleObject"));
	regFunc(v, loopObjectState, _SC("loopObjectState"));
	regFunc(v, objectAlpha, _SC("objectAlpha"));
	regFunc(v, objectAlphaTo, _SC("objectAlphaTo"));
	regFunc(v, objectAt, _SC("objectAt"));
	regFunc(v, objectBumperCycle, _SC("objectBumperCycle"));
	regFunc(v, objectCenter, _SC("objectCenter"));
	regFunc(v, objectColor, _SC("objectColor"));
	regFunc(v, objectDependentOn, _SC("objectDependentOn"));
	regFunc(v, objectFPS, _SC("objectFPS"));
	regFunc(v, objectHidden, _SC("objectHidden"));
	regFunc(v, objectHotspot, _SC("objectHotspot"));
	regFunc(v, objectIcon, _SC("objectIcon"));
	regFunc(v, objectLit, _SC("objectLit"));
	regFunc(v, objectMoveTo, _SC("objectMoveTo"));
	regFunc(v, objectOwner, _SC("objectOwner"));
	regFunc(v, objectOffset, _SC("objectOffset"));
	regFunc(v, objectOffsetTo, _SC("objectOffsetTo"));
	regFunc(v, objectParallaxLayer, _SC("objectParallaxLayer"));
	regFunc(v, objectParent, _SC("objectParent"));
	regFunc(v, objectPosX, _SC("objectPosX"));
	regFunc(v, objectPosY, _SC("objectPosY"));
	regFunc(v, objectRenderOffset, _SC("objectRenderOffset"));
	regFunc(v, objectRoom, _SC("objectRoom"));
	regFunc(v, objectRotate, _SC("objectRotate"));
	regFunc(v, objectRotateTo, _SC("objectRotateTo"));
	regFunc(v, objectScale, _SC("objectScale"));
	regFunc(v, objectScaleTo, _SC("objectScaleTo"));
	regFunc(v, objectScreenSpace, _SC("objectScreenSpace"));
	regFunc(v, objectShader, _SC("objectShader"));
	regFunc(v, objectSort, _SC("objectSort"));
	regFunc(v, objectState, _SC("objectState"));
	regFunc(v, objectTouchable, _SC("objectTouchable"));
	regFunc(v, objectUsePos, _SC("objectUsePos"));
	regFunc(v, objectUsePosX, _SC("objectUsePosX"));
	regFunc(v, objectUsePosY, _SC("objectUsePosY"));
	regFunc(v, objectValidUsePos, _SC("objectValidUsePos"));
	regFunc(v, objectValidVerb, _SC("objectValidVerb"));
	regFunc(v, pickupObject, _SC("pickupObject"));
	regFunc(v, pickupReplacementObject, _SC("pickupReplacementObject"));
	regFunc(v, playObjectState, _SC("playObjectState"));
	regFunc(v, popInventory, _SC("popInventory"));
	regFunc(v, removeInventory, _SC("removeInventory"));
	regFunc(v, setDefaultObject, _SC("setDefaultObject"));
	regFunc(v, shakeObject, _SC("shakeObject"));
	regFunc(v, stopObjectMotors, _SC("stopObjectMotors"));
	regFunc(v, objectAt, _SC("objectAt"));
}

} // namespace Twp
