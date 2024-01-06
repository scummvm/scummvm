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
#include "twp/camera.h"
#include "twp/motor.h"
#include "twp/room.h"
#include "twp/scenegraph.h"
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
			break;
		case 0x0000000020000000:
			thAlign = thCenter;
			break;
		case 0x0000000040000000:
			thAlign = thRight;
			break;
		default:
			return sq_throwerror(v, "failed to get halign");
		}
		switch (vAlign) {
		case 0xFFFFFFFF80000000:
			tvAlign = tvTop;
			break;
		case 0x0000000001000000:
			tvAlign = tvBottom;
			break;
		default:
			tvAlign = tvTop;
			break;
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
	Object *obj = sqobj(v, 2);
	if (obj) {
		delete obj;
	}
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
	int x, y;
	if (SQ_FAILED(sqget(v, 2, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 3, y)))
		return sq_throwerror(v, "failed to get y");
	Object *obj = g_engine->objAt(Math::Vector2d(x, y));
	if (!obj)
		sq_pushnull(v);
	else
		sqpush(v, obj->_table);
	return 1;
}

static SQInteger isInventoryOnScreen(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	if (!obj->_owner || (obj->_owner != g_engine->_actor)) {
		debug("Is '%s(%s)' in inventory: no", obj->_name.c_str(), obj->_key.c_str());
		sqpush(v, false);
		return 1;
	}
	int offset = obj->_owner->_inventoryOffset;
	int index = find(obj->_owner->_inventory, obj);
	int res = index >= offset * 4 && index < (offset * 4 + 8);
	debug("Is '%s(%s)' in inventory: {%d}", obj->_name.c_str(), obj->_key.c_str(), res);
	sqpush(v, res);
	return 1;
}

// Returns true if the object is actually an object and not something else.
//
// .. code-block:: Squirrel
// if (isObject(obj) && objectValidUsePos(obj) && objectTouchable(obj)) {
static SQInteger isObject(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	sqpush(v, obj && isObject(obj->getId()));
	return 1;
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

// Works exactly the same as playObjectState, but plays the animation as a continuous loop, playing the specified animation.
//
// .. code-block:: Squirrel
// loopObjectState(aStreetFire, 0)
// loopObjectState(flies, 3)
static SQInteger loopObjectState(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	if (sq_gettype(v, 3) == OT_INTEGER) {
		int index;
		if (SQ_FAILED(sqget(v, 3, index)))
			return sq_throwerror(v, "failed to get state");
		obj->play(index, true);
	} else if (sq_gettype(v, 3) == OT_STRING) {
		const SQChar *state;
		if (SQ_FAILED(sqget(v, 3, state)))
			return sq_throwerror(v, "failed to get state (string)");
		obj->play(state, true);
	} else {
		return sq_throwerror(v, "failed to get state");
	}
	return 0;
}

// Sets an object's alpha (transparency) in the range of 0.0 to 1.0.
// Setting an object's color will set it's alpha back to 1.0, ie completely opaque.
//
// .. code-block:: Squirrel
// objectAlpha(cloud, 0.5)
static SQInteger objectAlpha(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		float alpha = 0.0f;
		if (SQ_FAILED(sq_getfloat(v, 3, &alpha)))
			return sq_throwerror(v, "failed to get alpha");
		obj->setAlphaTo(nullptr);
		obj->_node->setAlpha(alpha);
	}
	return 0;
}

// Changes an object's alpha from its current state to the specified alpha over the time period specified by time.
//
// If an interpolationMethod is used, the change will follow the rules of the easing method, e.g. LINEAR, EASE_INOUT.
// See also stopObjectMotors.
static SQInteger objectAlphaTo(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) != OT_NULL) {
		Object *obj = sqobj(v, 2);
		if (!obj)
			return sq_throwerror(v, "failed to get object");
		float alpha = 0.0f;
		if (SQ_FAILED(sqget(v, 3, alpha)))
			return sq_throwerror(v, "failed to get alpha");
		alpha = clamp(alpha, 0.0f, 1.0f);
		float t = 0.0f;
		if (SQ_FAILED(sqget(v, 4, t)))
			return sq_throwerror(v, "failed to get time");
		int interpolation = 0;
		if ((sq_gettop(v) >= 5) && (SQ_FAILED(sqget(v, 5, interpolation))))
			interpolation = 0;
		obj->setAlphaTo(new AlphaTo(t, obj, alpha, intToInterpolationMethod(interpolation)));
	}
	return 0;
}

// Places the specified object at the given x and y coordinates in the current room.
//
// .. code-block:: Squirrel
// objectAt(text, 160,90)
// objectAt(obj, leftMargin, topLinePos)
static SQInteger objectAt(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	if (sq_gettop(v) == 3) {
		Object *spot = sqobj(v, 3);
		if (!spot)
			return sq_throwerror(v, "failed to get spot");
		obj->_node->setPos(spot->getUsePos());
		return 0;
	}
	if (sq_gettop(v) == 4) {
		SQInteger x, y;
		if (SQ_FAILED(sq_getinteger(v, 3, &x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sq_getinteger(v, 4, &y)))
			return sq_throwerror(v, "failed to get y");
		obj->_node->setPos(Math::Vector2d(x, y));
		return 0;
	}
	return sq_throwerror(v, "invalid number of arguments");
}

static SQInteger objectBumperCycle(HSQUIRRELVM v) {
	// TODO: objectBumperCycle
	warning("objectBumperCycle not implemented");
	return 0;
}

static SQInteger objectCenter(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	Math::Vector2d pos = obj->_node->getPos() + obj->_usePos;
	sqpush(v, pos);
	return 1;
}

// Sets an object's color. The color is an int in the form of 0xRRGGBB
//
// .. code-block:: Squirrel
// objectColor(warningSign, 0x808000)
static SQInteger objectColor(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		int color = 0;
		if (SQ_FAILED(sqget(v, 3, color)))
			return sq_throwerror(v, "failed to get color");
		obj->_node->setColor(Color::fromRgba(color));
	}
	return 0;
}

static SQInteger objectDependentOn(HSQUIRRELVM v) {
	Object *child = sqobj(v, 2);
	if (!child)
		return sq_throwerror(v, "failed to get child object");
	Object *parent = sqobj(v, 3);
	if (!parent)
		return sq_throwerror(v, "failed to get parent object");
	int state = 0;
	if (SQ_FAILED(sqget(v, 4, state)))
		return sq_throwerror(v, "failed to get state");
	child->dependentOn(parent, state);
	return 0;
}

// Sets how many frames per second (fpsRate) the object will animate at.
//
// .. code-block:: Squirrel
// objectFPS(pigeon1, 15)
static SQInteger objectFPS(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		float fps = 0.0f;
		if (SQ_FAILED(sqget(v, 3, fps)))
			return sq_throwerror(v, "failed to get fps");
		obj->_fps = fps;
	}
	return 0;
}

// Sets if an object is hidden or not. If the object is hidden, it is no longer displayed or touchable.
//
// .. code-block:: Squirrel
// objectHidden(oldRags, YES)
static SQInteger objectHidden(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		int hidden = 0;
		sqget(v, 3, hidden);
		debug("Sets object visible %s/%s to %s", obj->_name.c_str(), obj->_key.c_str(), hidden == 0 ? "true" : "false");
		obj->_node->setVisible(hidden == 0);
	}
	return 0;
}

// Sets the touchable area of an actor or object.
// This is a rectangle enclosed by the specified coordinates.
// We also use this on the postalworker to enlarge his touchable area to make it easier to click on him while he's sorting mail.
//
// .. code-block:: Squirrel
// objectHotspot(willie, 14, 0, 14, 62)         // Willie standing up
// objectHotspot(willie, -28, 0, 28, 50)        // Willie lying down drunk
static SQInteger objectHotspot(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object or actor");
	if (sq_gettop(v) == 2) {
		Math::Vector2d pos = obj->_node->getAbsPos();
		sqpush(v, Rectf::fromPosAndSize(Math::Vector2d(obj->_hotspot.left + pos.getX(), obj->_hotspot.bottom + pos.getY()), Math::Vector2d(obj->_hotspot.width(), obj->_hotspot.height())));
		return 1;
	}
	int left = 0;
	int top = 0;
	int right = 0;
	int bottom = 0;
	if (SQ_FAILED(sqget(v, 3, left)))
		return sq_throwerror(v, "failed to get left");
	if (SQ_FAILED(sqget(v, 4, top)))
		return sq_throwerror(v, "failed to get top");
	if (SQ_FAILED(sqget(v, 5, right)))
		return sq_throwerror(v, "failed to get right");
	if (SQ_FAILED(sqget(v, 6, bottom)))
		return sq_throwerror(v, "failed to get bottom");
	if (bottom < top)
		SWAP(bottom, top);
	obj->_hotspot = Common::Rect(left, top, right, bottom);
	return 0;
}

// Used for inventory object, it changes the object's icon to be the new one specified.
//
// .. code-block:: Squirrel
// objectIcon(obj, "glowing_spell_book")
// objectIcon(obj, "spell_book")
static SQInteger objectIcon(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	switch (sq_gettype(v, 3)) {
	case OT_STRING: {
		Common::String icon;
		if (SQ_FAILED(sqget(v, 3, icon)))
			return sq_throwerror(v, "failed to get icon");
		obj->setIcon(icon);
		return 0;
	}
	case OT_ARRAY: {
		Common::String icon;
		Common::StringArray icons;
		int fps;
		sq_push(v, 3);
		sq_pushnull(v); // null iterator
		if (SQ_SUCCEEDED(sq_next(v, -2)))
			sqget(v, -1, fps);
		sq_pop(v, 2);
		while (SQ_SUCCEEDED(sq_next(v, -2))) {
			sqget(v, -1, icon);
			icons.push_back(icon);
			sq_pop(v, 2);
		}
		sq_pop(v, 2); // pops the null iterator and object
		obj->setIcon(fps, icons);
		return 0;
	}
	default:
		return sq_throwerror(v, "invalid argument type");
	}
}

// Specifies whether the object is affected by lighting elements.
// Note: this is currently used for actor objects, but can also be used for room objects.
// Lighting background flat art would be hard and probably look odd.
static SQInteger objectLit(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object or actor");
	bool lit = false;
	if (SQ_FAILED(sqget(v, 3, lit)))
		return sq_throwerror(v, "failed to get lit");
	obj->_lit = lit;
	return 0;
}

// Moves the object to the specified location over the time period specified.
//
// If an interpolation method is used for the transition, it will use that.
// Unlike `objectOffsetTo`, `objectMoveTo` moves the item to a x, y on the screen, not relative to the object's starting position.
// If you want to move the object back again, you need to store where the object started.
//
// .. code-block:: Squirrel
// objectMoveTo(this, 10, 20, 2.0)
//
// See also:
// - `stopObjectMotors method <#stopObjectMotors.e>`_
// - `objectOffsetTo method <#objectOffsetTo.e>`_
static SQInteger objectMoveTo(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		int x = 0;
		int y = 0;
		if (SQ_FAILED(sqget(v, 3, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 4, y)))
			return sq_throwerror(v, "failed to get y");
		float duration = 0.0f;
		if (SQ_FAILED(sqget(v, 5, duration)))
			return sq_throwerror(v, "failed to get duration");
		int interpolation = 0;
		if ((sq_gettop(v) >= 6) && SQ_FAILED(sqget(v, 6, interpolation)))
			interpolation = 0;
		Math::Vector2d destPos = Math::Vector2d(x, y);
		obj->setMoveTo(new MoveTo(duration, obj, destPos, intToInterpolationMethod(interpolation)));
	}
	return 0;
}

// Instantly offsets the object (image, use position, hotspot) with respect to the origin of the object.
//
// .. code-block:: Squirrel
// objectOffset(coroner, 0, 0)
// objectOffset(SewerManhole.sewerManholeDime, 0, 96)
static SQInteger objectOffset(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		int x = 0;
		int y = 0;
		if (SQ_FAILED(sqget(v, 3, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 4, y)))
			return sq_throwerror(v, "failed to get y");
		obj->setMoveTo(nullptr);
		obj->_node->setOffset(Math::Vector2d(x, y));
	}
	return 0;
}

static SQInteger objectOffsetTo(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		int x = 0;
		int y = 0;
		if (SQ_FAILED(sqget(v, 3, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 4, y)))
			return sq_throwerror(v, "failed to get y");
		float duration = 0.5f;
		if (sq_gettop(v) >= 5)
			if (SQ_FAILED(sqget(v, 5, duration)))
				return sq_throwerror(v, "failed to get duration");
		SQInteger interpolation = 0;
		if ((sq_gettop(v) >= 6) && (SQ_FAILED(sq_getinteger(v, 6, &interpolation))))
			interpolation = 0;
		Math::Vector2d destPos(x, y);
		obj->setMoveTo(new OffsetTo(duration, obj, destPos, intToInterpolationMethod(interpolation)));
	}
	return 0;
}

// Returns the actor who owns the specified object/inventory item.
// If there is no owner, returns false.
//
// .. code-block:: Squirrel
// objectOwner(dime) == currentActor
// !objectOwner(countyMap1)
static SQInteger objectOwner(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	if (!obj->_owner)
		sq_pushnull(v);
	else
		sq_pushobject(v, obj->_owner->_table);
	return 1;
}

// Changes the object's layer.
static SQInteger objectParallaxLayer(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	int layer = 0;
	if (SQ_FAILED(sqget(v, 3, layer)))
		return sq_throwerror(v, "failed to get parallax layer");
	g_engine->_room->objectParallaxLayer(obj, layer);
	return 0;
}

static SQInteger objectParent(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get child");
	Object *parent = sqobj(v, 3);
	if (!parent)
		return sq_throwerror(v, "failed to get parent");
	obj->_parent = parent->_key;
	parent->_node->addChild(obj->_node);
	return 0;
}

// Returns the x-coordinate of the given object or actor.
static SQInteger objectPosX(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	float x = obj->getUsePos().getX() + obj->_hotspot.left + obj->_hotspot.width() / 2.0f;
	sqpush(v, (int)x);
	return 1;
}

// Returns the y-coordinate of the given object or actor.
static SQInteger objectPosY(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	float y = obj->getUsePos().getY() + obj->_hotspot.top + obj->_hotspot.height() / 2.0f;
	sqpush(v, (int)y);
	return 1;
}

// Sets the rendering offset of the actor to x and y.
//
// A rendering offset of 0,0 would cause them to be rendered from the middle of their image.
// Actor's are typically adjusted so they are rendered from the middle of the bottom of their feet.
// To maintain sanity, it is best if all actors have the same image size and are all adjust the same, but this is not a requirement.
static SQInteger objectRenderOffset(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	SQInteger x, y;
	if (SQ_FAILED(sq_getinteger(v, 3, &x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sq_getinteger(v, 4, &y)))
		return sq_throwerror(v, "failed to get y");
	obj->_node->setRenderOffset(Math::Vector2d(x, y));
	return 0;
}

// Returns the room of a given object or actor.
static SQInteger objectRoom(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	if (!obj->_room)
		sq_pushnull(v);
	else
		sqpush(v, obj->_room->_table);
	return 1;
}

// Sets the rotation of object to the specified amount instantly.
//
// .. code-block:: Squirrel
// objectRotate(pigeonVanBackWheel, 0)
static SQInteger objectRotate(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		float rotation = 0.0f;
		if (SQ_FAILED(sqget(v, 3, rotation)))
			return sq_throwerror(v, "failed to get rotation");
		obj->setRotateTo(nullptr);
		obj->_node->setRotation(rotation);
	}
	return 0;
}

// Rotates the object from its current rotation to the desired rotation over duration time period.
// The interpolationMethod specifies how the animation is played.
// if `LOOPING` is used, it will continue to rotate as long as the rotation parameter is 360 or -360.
//
// .. code-block:: Squirrel
// objectRotateTo(bridgeGrateTree, 45, 3.7, SLOW_EASE_IN)
// objectRotateTo(AStreet.aStreetPhoneBook, 6, 2.0, SWING)
// objectRotateTo(firefly, direction, 12, LOOPING)
static SQInteger objectRotateTo(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (obj) {
		float rotation = 0.0f;
		if (SQ_FAILED(sqget(v, 3, rotation)))
			return sq_throwerror(v, "failed to get rotation");
		float duration = 0.0f;
		if (SQ_FAILED(sqget(v, 4, duration)))
			return sq_throwerror(v, "failed to get duration");
		int interpolation = 0;
		if ((sq_gettop(v) >= 5) && SQ_FAILED(sqget(v, 5, interpolation)))
			interpolation = 0;
		obj->setRotateTo(new RotateTo(duration, obj->_node, rotation, intToInterpolationMethod(interpolation)));
	}
	return 0;
}

// Sets how scaled the object's image will appear on screen. 1 is no scaling.
static SQInteger objectScale(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	float scale = 0.0f;
	if (SQ_FAILED(sqget(v, 3, scale)))
		return sq_throwerror(v, "failed to get scale");
	obj->_node->setScale(Math::Vector2d(scale, scale));
	return 0;
}

static SQInteger objectScaleTo(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj) {
		float scale = 0.0f;
		if (SQ_FAILED(sqget(v, 3, scale)))
			return sq_throwerror(v, "failed to get scale");
		float duration = 0.0f;
		if (SQ_FAILED(sqget(v, 4, duration)))
			return sq_throwerror(v, "failed to get duration");
		int interpolation = 0;
		if ((sq_gettop(v) >= 5) && SQ_FAILED(sqget(v, 5, interpolation)))
			interpolation = 0;
		obj->setRotateTo(new ScaleTo(duration, obj->_node, scale, intToInterpolationMethod(interpolation)));
	}
	return 0;
}

// Sets the object in the screen space.
// It means that its position is relative to the screen, not to the room.
static SQInteger objectScreenSpace(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	g_engine->_screenScene.addChild(obj->_node);
	return 0;
}

static SQInteger objectShader(HSQUIRRELVM v) {
	// TODO: objectShader
	warning("objectShader not implemented");
	return 0;
}

// Changes the state of an object, although this can just be a internal state,
//
// it is typically used to change the object's image as it moves from it's current state to another.
// Behind the scenes, states as just simple ints. State0, State1, etc.
// Symbols like CLOSED and OPEN and just pre-defined to be 0 or 1.
// State 0 is assumed to be the natural state of the object, which is why OPEN is 1 and CLOSED is 0 and not the other way around.
// This can be a little confusing at first.
// If the state of an object has multiple frames, then the animation is played when changing state, such has opening the clock.
// GONE is a unique in that setting an object to GONE both sets its graphical state to 1, and makes it untouchable. Once an object is set to GONE, if you want to make it visible and touchable again, you have to set both:
//
// .. code-block:: Squirrel
// objectState(coin, HERE)
// objectTouchable(coin, YES)
static SQInteger objectState(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) == OT_NULL)
		return 0;
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	SQInteger nArgs = sq_gettop(v);
	if (nArgs == 2) {
		sqpush(v, obj->_state);
		return 1;
	}
	if (nArgs == 3) {
		int state;
		if (SQ_FAILED(sqget(v, 3, state)))
			return sq_throwerror(v, "failed to get state");
		obj->setState(state);
		return 0;
	}
	return sq_throwerror(v, "invalid number of arguments");
}

// Gets or sets if an object is player touchable.
static SQInteger objectTouchable(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	SQInteger nArgs = sq_gettop(v);
	if (nArgs == 2) {
		sqpush(v, obj->isTouchable());
		return 1;
	}
	if (nArgs == 3) {
		bool touchable;
		if (SQ_FAILED(sqget(v, 3, touchable)))
			return sq_throwerror(v, "failed to get touchable");
		obj->setTouchable(touchable);
		return 0;
	}
	return sq_throwerror(v, "objectTouchable: invalid argument");
}

//  Sets the zsort order of an object, essentially the order in which an object is drawn on the screen.
// A sort order of 0 is the bottom of the screen.
// Actors typically have a sort order of their Y position.
//
// .. code-block:: Squirrel
// objectSort(censorBox, 0)   // Will be on top of everything.
static SQInteger objectSort(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	int zsort;
	if (SQ_FAILED(sqget(v, 3, zsort)))
		return sq_throwerror(v, "failed to get zsort");
	obj->_node->setZSort(zsort);
	return 0;
}

// Sets the location an actor will stand at when interacting with this object.
// Directions are: FACE_FRONT, FACE_BACK, FACE_LEFT, FACE_RIGHT
//
// .. code-block:: Squirrel
// objectUsePos(popcornObject, -13, 0, FACE_RIGHT)
static SQInteger objectUsePos(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	int x, y, dir;
	if (SQ_FAILED(sqget(v, 3, x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sqget(v, 4, y)))
		return sq_throwerror(v, "failed to get y");
	if (SQ_FAILED(sqget(v, 5, dir)))
		return sq_throwerror(v, "failed to get direction");
	obj->_usePos = Math::Vector2d(x, y);
	obj->_useDir = (Direction)dir;
	return 0;
}

// Returns the x of the object's use position.
//
// .. code-block:: Squirrel
// objectUsePosX(dimeLoc)
static SQInteger objectUsePosX(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	sqpush(v, obj->getUsePos().getX());
	return 1;
}

// Returns the y of the object's use position.
//
// .. code-block:: Squirrel
// objectUsePosY(dimeLoc)
static SQInteger objectUsePosY(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	sqpush(v, obj->getUsePos().getY());
	return 1;
}

// Returns true if the object's use position has been set (ie is not 0,0).
static SQInteger objectValidUsePos(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	sqpush(v, obj->_usePos != Math::Vector2d());
	return 1;
}

// Returns true if this object has a verb function for the specified verb.
// Mostly used for testing when trying to check interactions.
// Verb options are: VERB_WALKTO, VERB_LOOKAT, VERB_PICKUP, VERB_OPEN, VERB_CLOSE, VERB_PUSH, VERB_PULL, VERB_TALKTO.
// Cannot use DEFAULT_VERB because that is not a real verb to the system.
//
// .. code-block:: Squirrel
// if (objectValidVerb(obj, VERB_PICKUP)) {
//    logAction("PickUp", obj)
//    pushSentence(VERB_PICKUP, obj)
//    tries = 0
//}
static SQInteger objectValidVerb(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object or actor");
	int verb;
	if (SQ_FAILED(sqget(v, 3, verb)))
		return sq_throwerror(v, "failed to get verb");

	// int verbId = verb;
	if (!g_engine->_actor) {
		ActorSlot *slot = g_engine->_hud.actorSlot(g_engine->_actor);
		for (int i = 0; i < 22; i++) {
			Verb *vb = &slot->verbs[i];
			if (vb->id.id == verb) {
				if (sqrawexists(obj->_table, vb->fun)) {
					sqpush(v, true);
					return 1;
				}
			}
		}
	}
	sqpush(v, false);
	return 1;
}

static SQInteger pickupObject(HSQUIRRELVM v) {
	// Picks up an object and adds it to the selected actor's inventory.
	// The object that appears in the room is not the object you pick up, this is due to the code often needing to be very different when it's held in your inventory, plus inventory objects need icons.
	//
	// .. code-block:: Squirrel
	// pickupObject(Dime)
	Object *obj = sqobj(v, 2);
	if (!obj) {
		HSQOBJECT o;
		sq_getstackobj(v, 2, &o);
		Common::String name;
		sqgetf(o, "name", name);
		return sq_throwerror(v, Common::String::format("failed to get object %x, %s", o._type, g_engine->_textDb.getText(name).c_str()).c_str());
	}
	Object *actor = nullptr;
	if (sq_gettop(v) >= 3) {
		actor = sqactor(v, 3);
		if (!actor)
			return sq_throwerror(v, "failed to get actor");
	}
	if (!actor)
		actor = g_engine->_actor;
	actor->pickupObject(obj);
	return 0;
}

static SQInteger pickupReplacementObject(HSQUIRRELVM v) {
	Object *obj1 = sqobj(v, 2);
	if (!obj1)
		return sq_throwerror(v, "failed to get object 1");
	Object *obj2 = sqobj(v, 3);
	if (!obj2)
		return sq_throwerror(v, "failed to get object 2");

	// remove obj1 from inventory's owner
	if (obj1->_owner) {
		int index = find(obj1->_owner->_inventory, obj1);
		obj1->_owner->_inventory.remove_at(index);
		obj1->_owner = nullptr;
	}

	// replace obj2 by obj1
	Object *owner = obj2->_owner;
	int index = find(obj2->_owner->_inventory, obj2);
	owner->_inventory[index] = obj1;
	obj1->_owner = owner;
	obj2->_owner = nullptr;
	return 0;
}

// The only difference between objectState and playObjectState is if they are called during the enter code.
// objectState will set the image to the last frame of the state's animation, where as, playObjectState will play the full animation.
//
// .. code-block:: Squirrel
// playObjectState(Mansion.windowShutters, OPEN)
static SQInteger playObjectState(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return 0;
	if (sq_gettype(v, 3) == OT_INTEGER) {
		SQInteger index;
		if (SQ_FAILED(sq_getinteger(v, 3, &index)))
			return sq_throwerror(v, "failed to get state");
		obj->play(index);
	} else if (sq_gettype(v, 3) == OT_STRING) {
		Common::String state;
		if (SQ_FAILED(sqget(v, 3, state)))
			return sq_throwerror(v, "failed to get state");
		obj->play(state);
	} else {
		return sq_throwerror(v, "failed to get state");
	}
	return 0;
}

static SQInteger popInventory(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	int count;
	if (SQ_FAILED(sqget(v, 3, count)))
		return sq_throwerror(v, "failed to get count");
	obj->setPop(count);
	return 0;
}

// Removes an object from the current actor's inventory.
// If the object is not in the current actor's inventory, the command silently fails.
static SQInteger removeInventory(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	obj->removeInventory();
	return 0;
}

// Globally sets a default object.
// When the player executes the sentence open painting and the painting object has no verbOpen function defined,
// it will call the default object's verbOpen as a fallback, allowing for common failure phrase like "I can't open that.".
// The default object can be changed at anytime, so different selectable characters can have different default responses.
static SQInteger setDefaultObject(HSQUIRRELVM v) {
	HSQUIRRELVM vm = g_engine->getVm();
	if (g_engine->_defaultObj._type != OT_NULL)
		sq_release(vm, &g_engine->_defaultObj);
	if (SQ_FAILED(sq_getstackobj(v, 2, &g_engine->_defaultObj)))
		return sq_throwerror(v, "failed to get default object");
	sq_addref(vm, &g_engine->_defaultObj);
	return 0;
}

static SQInteger shakeObject(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	float amount;
	if (SQ_FAILED(sqget(v, 3, amount)))
		return sq_throwerror(v, "failed to get amount");
	obj->setShakeTo(new Shake(obj->_node, amount));
	return 0;
}

static SQInteger stopObjectMotors(HSQUIRRELVM v) {
	Object *obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object");
	obj->stopObjectMotors();
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
}

} // namespace Twp
