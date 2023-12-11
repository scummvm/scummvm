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
		if (SQ_FAILED(get(v, 2, sheet)))
			return sq_throwerror(v, "failed to get sheet");
		framesIndex = 3;
	}

	// get frames parameter if any
	if (numArgs >= 2) {
		switch (sq_gettype(v, framesIndex)) {
		case OT_STRING: {
			Common::String frame;
			get(v, framesIndex, frame);
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

	sq_pushobject(v, obj->table);

	return 1;
}

static SQInteger objectAt(HSQUIRRELVM v) {
	HSQOBJECT o;
	sq_getstackobj(v, 2, &o);

	SQInteger id;
	getf(o, "_id", id);

	Object **pObj = Common::find_if(g_engine->_objects.begin(), g_engine->_objects.end(), [&](Object *o) {
		SQObjectPtr id2;
		_table(o->table)->Get(sqobj(v, "_id"), id2);
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
	regFunc(v, createObject, _SC("createObject"), 0, nullptr);
	regFunc(v, objectAt, _SC("objectAt"), 0, nullptr);
}

} // namespace Twp
