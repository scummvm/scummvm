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

#include "twp/sqgame.h"
#include "twp/twp.h"
#include "twp/room.h"
#include "twp/object.h"
#include "twp/scenegraph.h"
#include "twp/squtil.h"
#include "twp/squirrel/squirrel.h"
#include "twp/squirrel/sqvm.h"
#include "twp/squirrel/sqobject.h"
#include "twp/squirrel/sqstring.h"
#include "twp/squirrel/sqstate.h"
#include "twp/squirrel/sqtable.h"
#include "twp/squirrel/sqstdstring.h"
#include "twp/squirrel/sqstdmath.h"
#include "twp/squirrel/sqstdio.h"
#include "twp/squirrel/sqstdaux.h"
#include "twp/squirrel/sqfuncproto.h"
#include "twp/squirrel/sqclosure.h"

namespace Twp {

struct GetArray {
	GetArray(Common::Array<HSQOBJECT> &objs) : _objs(objs) {}

	void operator()(HSQOBJECT &o) {
		_objs.push_back(o);
	}

private:
	Common::Array<HSQOBJECT> &_objs;
};

template<typename T>
static void shuffle(Common::Array<T> &array) {
	if (array.size() > 1) {
		Common::RandomSource &rnd = g_engine->getRandomSource();
		for (size_t i = 0; i < array.size() - 1; i++) {
			size_t j = i + rnd.getRandomNumber(RAND_MAX) / (RAND_MAX / (array.size() - i) + 1);
			T &t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

static SQInteger activeVerb(HSQUIRRELVM v) {
	// TODO: activeVerb
	// push(v, gEngine.hud.verb.id);
	// return 1;
	warning("activeVerb not implemented");
	return 0;
}

static SQInteger adhocalytics(HSQUIRRELVM v) {
	warning("adhocalytics not implemented");
	return 0;
}

static SQInteger arrayShuffle(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) != OT_ARRAY)
		return sq_throwerror(v, "An array is expected");
	HSQOBJECT obj;
	sq_getstackobj(v, 2, &obj);
	Common::Array<HSQOBJECT> arr;
	GetArray g(arr);
	sqgetitems(obj, g);
	shuffle(arr);

	sq_newarray(v, 0);
	for (auto it = arr.begin(); it != arr.end(); it++) {
		sqpush(v, *it);
		sq_arrayappend(v, -2);
	}
	return 1;
}

// Returns TRUE if the specified entry exists in the assets.
static SQInteger assetExists(HSQUIRRELVM v) {
	const SQChar *filename;
	if (SQ_FAILED(sq_getstring(v, 2, &filename)))
		return sq_throwerror(v, "failed to get filename");
	sqpush(v, g_engine->_pack.assetExists(filename));
	return 1;
}

// Moves the camera to the specified x, y location.
// If a spot is specified, will move to the x, y specified by that spot.
// .. code-block:: Squirrel
// cameraAt(450, 128)
//
// enterRoomFromDoor(Bridge.startRight)
// actorAt(ray, Bridge.startLeft)
// actorAt(reyes, Bridge.startRight)
// cameraAt(Bridge.bridgeBody)
static SQInteger cameraAt(HSQUIRRELVM v) {
	// TODO: cameraAt
	warning("cameraAt not implemented");
	return 0;
}

// Sets how far the camera can pan.
static SQInteger cameraBounds(HSQUIRRELVM v) {
	// TODO: cameraBounds
	warning("cameraBounds not implemented");
	return 0;
}

static SQInteger cameraFollow(HSQUIRRELVM v) {
	// TODO: cameraFollow
	warning("cameraFollow not implemented");
	return 0;
}

// Moves the camera to the specified room.
//
// Does not move any of the actors.
//
// .. code-block:: Squirrel
// aStreetPhoneBook =
// {
//     name = "phone book"
//     verbLookAt = function()
//     {
//         cameraInRoom(PhoneBook)
//      }
// }
static SQInteger cameraInRoom(HSQUIRRELVM v) {
	Room *room = sqroom(v, 2);
	if (room) {
		g_engine->setRoom(room);
	} else {
		Object *obj = sqobj(v, 2);
		if (!obj || !obj->_room) {
			return sq_throwerror(v, "failed to get room");
		}
		g_engine->setRoom(obj->_room);
	}
	return 0;
}

// Pans the camera to the specified x, y location over the duration using the transition method.
// Transition methods are: EASE_IN, EASE_INOUT, EASE_OUT, LINEAR, SLOW_EASE_IN, SLOW_EASE_OUT.
//
// .. code-block:: Squirrel
// cameraPanTo(450, 128, pan_time, EASE_INOUT)
// inputOff()
// actorWalkTo(currentActor, Highway.detectiveSpot1)
// breakwhilewalking(currentActor)
// cameraPanTo(currentActor, 2.0)
static SQInteger cameraPanTo(HSQUIRRELVM v) {
	SQInteger numArgs = sq_gettop(v);
	Math::Vector2d pos;
	float duration = 0.f;
	InterpolationKind interpolation = IK_LINEAR;
	if (numArgs == 3) {
		Object *obj = sqobj(v, 2);
		if (!obj)
			return sq_throwerror(v, "failed to get object/actor");
		pos = obj->getUsePos();
		if (SQ_FAILED(sqget(v, 3, duration)))
			return sq_throwerror(v, "failed to get duration");
	} else if (numArgs == 4) {
		if (sq_gettype(v, 2) == OT_INTEGER) {
			int x;
			if (SQ_FAILED(sqget(v, 2, x)))
				return sq_throwerror(v, "failed to get x");
			if (SQ_FAILED(sqget(v, 3, duration)))
				return sq_throwerror(v, "failed to get duration");
			int im;
			if (SQ_FAILED(sqget(v, 4, im)))
				return sq_throwerror(v, "failed to get interpolation method");
			pos = Math::Vector2d(x, g_engine->getGfx().cameraPos().getY());
			interpolation = (InterpolationKind)im;
		} else {
			Object *obj = sqobj(v, 2);
			if (SQ_FAILED(sqget(v, 3, duration)))
				return sq_throwerror(v, "failed to get duration");
			int im;
			if (SQ_FAILED(sqget(v, 4, im)))
				return sq_throwerror(v, "failed to get interpolation method");
			pos = obj->_node->getAbsPos();
			interpolation = (InterpolationKind)im;
		}
	} else if (numArgs == 5) {
		int x, y;
		if (SQ_FAILED(sqget(v, 2, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 3, y)))
			return sq_throwerror(v, "failed to get y");
		if (SQ_FAILED(sqget(v, 4, duration)))
			return sq_throwerror(v, "failed to get duration");
		int im;
		if (SQ_FAILED (sqget(v, 5, im)))
      return sq_throwerror(v, "failed to get interpolation method");
		pos = Math::Vector2d(x, y);
		interpolation = (InterpolationKind)im;
	} else {
		return sq_throwerror(v, Common::String::format("invalid argument number: %lld", numArgs).c_str());
	}
	Math::Vector2d halfScreen(g_engine->_room->getScreenSize() / 2.f);
	debug("cameraPanTo: {pos}, dur={duration}, method={interpolation}");
	g_engine->follow(nullptr);
	g_engine->_camera.panTo(pos - Math::Vector2d(0.f, halfScreen.getY()), duration, interpolation);
	return 0;
}

// Returns the current camera position x, y.
static SQInteger cameraPos(HSQUIRRELVM v) {
	// TODO: cameraPos
	warning("cameraPos not implemented");
	return 0;
	// push(v, g_engine->cameraPos())
	// return 1;
}

// Converts an integer to a char.
static SQInteger sqChr(HSQUIRRELVM v) {
	int value;
	sqget(v, 2, value);
	Common::String s;
	s += char(value);
	sqpush(v, s);
	return 1;
}

// Returns x coordinates of the mouse in screen coordinates.
static SQInteger cursorPosX(HSQUIRRELVM v) {
	// TODO: cursorPosX
	warning("cursorPosX not implemented");
	return 0;

	//   let scrPos = winToScreen(mousePos())
	//   push(v, scrPos.x)
	//   return 1;
}

// Returns y coordinates of the mouse in screen coordinates.
static SQInteger cursorPosY(HSQUIRRELVM v) {
	// TODO: cursorPosY
	warning("cursorPosY not implemented");
	return 0;
	//   let scrPos = winToScreen(mousePos())
	//   push(v, scrPos.y)
	//   return 1;
}

static SQInteger distance(HSQUIRRELVM v) {
	// TODO: distance
	warning("distance not implemented");
	return 0;
}

static SQInteger findScreenPosition(HSQUIRRELVM v) {
	// TODO: findScreenPosition
	warning("findScreenPosition not implemented");
	return 0;
}

static SQInteger frameCounter(HSQUIRRELVM v) {
	// TODO: frameCounter
	warning("frameCounter not implemented");
	return 0;
}

static SQInteger getUserPref(HSQUIRRELVM v) {
	// TODO: getUserPref
	warning("getUserPref not implemented");
	return 0;
}

static SQInteger getPrivatePref(HSQUIRRELVM v) {
	// TODO: getPrivatePref
	warning("getPrivatePref not implemented");
	return 0;
}

static SQInteger incutscene(HSQUIRRELVM v) {
	// TODO: incutscene
	warning("incutscene not implemented");
	return 0;
}

static SQInteger indialog(HSQUIRRELVM v) {
	// TODO: indialog
	warning("indialog not implemented");
	return 0;
}

static SQInteger integer(HSQUIRRELVM v) {
	float f = 0.f;
	if (SQ_FAILED(sqget(v, 2, f)))
		return sq_throwerror(v, "failed to get float value");
	sqpush(v, static_cast<int>(f));
	return 1;
}

static SQInteger is_oftype(HSQUIRRELVM v, bool pred(SQObjectType)) {
	sqpush(v, pred(sq_gettype(v, 2)));
	return 1;
}

static SQInteger in_array(HSQUIRRELVM v) {
	HSQOBJECT obj;
	sq_resetobject(&obj);
	if (SQ_FAILED(sqget(v, 2, obj)))
		return sq_throwerror(v, "Failed to get object");
	HSQOBJECT arr;
	sq_resetobject(&arr);
	if (SQ_FAILED(sqget(v, 3, arr)))
		return sq_throwerror(v, "Failed to get array");

	Common::Array<HSQOBJECT> objs;
	sq_pushobject(v, arr);
	sq_pushnull(v); // null iterator
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		HSQOBJECT tmp;
		sq_getstackobj(v, -1, &tmp);
		objs.push_back(tmp);
		sq_pop(v, 2); // pops key and val before the nex iteration
	}
	sq_pop(v, 1); // pops the null iterator

	for (auto it = objs.begin(); it != objs.end(); it++) {
		sq_pushobject(v, obj);
		sq_pushobject(v, *it);
		if (sq_cmp(v) == 0) {
			sq_pop(v, 2);
			sqpush(v, 1);
			return 1;
		}
		sq_pop(v, 2);
	}

	sq_pushinteger(v, 0);
	return 1;
}

static SQInteger is_array(HSQUIRRELVM v) {
	return is_oftype(v, [](SQObjectType type) { return type == OT_ARRAY; });
}

static SQInteger is_function(HSQUIRRELVM v) {
	return is_oftype(v, [](SQObjectType type) { return (type == OT_CLOSURE) || (type == OT_NATIVECLOSURE); });
}

static SQInteger is_string(HSQUIRRELVM v) {
	return is_oftype(v, [](SQObjectType type) { return type == OT_STRING; });
}

static SQInteger is_table(HSQUIRRELVM v) {
	return is_oftype(v, [](SQObjectType type) { return type == OT_TABLE; });
}

// Returns a random number from from to to inclusively.
// The number is a pseudo-random number and the game will produce the same sequence of numbers unless primed using randomSeed(seed).
//
// .. code-block:: Squirrel
// wait_time = random(0.5, 2.0)
static SQInteger sqrandom(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) == OT_FLOAT || sq_gettype(v, 3) == OT_FLOAT) {
		SQFloat min, max;
		sq_getfloat(v, 2, &min);
		sq_getfloat(v, 3, &max);
		if (min > max)
			SWAP(min, max);
		float scale = g_engine->getRandomSource().getRandomNumber(RAND_MAX) / (float)RAND_MAX;
		SQFloat value = min + scale * (max - min);
		sq_pushfloat(v, value);
	} else {
		SQInteger min, max;
		sq_getinteger(v, 2, &min);
		sq_getinteger(v, 3, &max);
		if (min > max)
			SWAP(min, max);
		SQInteger value = g_engine->getRandomSource().getRandomNumberRngSigned(min, max);
		sq_pushinteger(v, value);
	}
	return 1;
}

static SQInteger loadArray(HSQUIRRELVM v) {
	// TODO: loadArray
	warning("loadArray not implemented");
	return 0;
}

static SQInteger markAchievement(HSQUIRRELVM v) {
	// TODO: markAchievement
	warning("markAchievement not implemented");
	return 0;
}

static SQInteger markProgress(HSQUIRRELVM v) {
	// TODO: markProgress
	warning("markProgress not implemented");
	return 0;
}

static SQInteger markStat(HSQUIRRELVM v) {
	// TODO: markStat
	warning("markStat not implemented");
	return 0;
}

static SQInteger ord(HSQUIRRELVM v) {
	// TODO: ord
	warning("ord not implemented");
	return 0;
}

static SQInteger pushSentence(HSQUIRRELVM v) {
	// TODO: pushSentence
	warning("pushSentence not implemented");
	return 0;
}

static SQInteger randomFrom(HSQUIRRELVM v) {
	// TODO: randomFrom
	warning("randomFrom not implemented");
	return 0;
}

static SQInteger randomOdds(HSQUIRRELVM v) {
	// TODO: randomOdds
	warning("randomOdds not implemented");
	return 0;
}

static SQInteger randomseed(HSQUIRRELVM v) {
	// TODO: randomseed
	warning("randomseed not implemented");
	return 0;
}

static SQInteger refreshUI(HSQUIRRELVM v) {
	// TODO: refreshUI
	warning("refreshUI not implemented");
	return 0;
}

static SQInteger screenSize(HSQUIRRELVM v) {
	// TODO: screenSize
	warning("screenSize not implemented");
	return 0;
}

static SQInteger setDebugger(HSQUIRRELVM v) {
	// TODO: setDebugger
	warning("setDebugger not implemented");
	return 0;
}

static SQInteger setPrivatePref(HSQUIRRELVM v) {
	// TODO: setPrivatePref
	warning("setPrivatePref not implemented");
	return 0;
}

static SQInteger setUserPref(HSQUIRRELVM v) {
	// TODO: setUserPref
	warning("setUserPref not implemented");
	return 0;
}

static SQInteger setVerb(HSQUIRRELVM v) {
	// TODO: setVerb
	warning("setVerb not implemented");
	return 0;
}

static SQInteger startDialog(HSQUIRRELVM v) {
	// TODO: startDialog
	warning("startDialog not implemented");
	return 0;
}

static SQInteger stopSentence(HSQUIRRELVM v) {
	// TODO: stopSentence
	warning("stopSentence not implemented");
	return 0;
}

static SQInteger strcount(HSQUIRRELVM v) {
	// TODO: strcount
	warning("strcount not implemented");
	return 0;
}

static SQInteger strcrc(HSQUIRRELVM v) {
	// TODO: strcrc
	warning("strcrc not implemented");
	return 0;
}

static SQInteger strfind(HSQUIRRELVM v) {
	// TODO: strfind
	warning("strfind not implemented");
	return 0;
}

static SQInteger strfirst(HSQUIRRELVM v) {
	// TODO: strfirst
	warning("strfirst not implemented");
	return 0;
}

static SQInteger strlast(HSQUIRRELVM v) {
	// TODO: strlast
	warning("strlast not implemented");
	return 0;
}

static SQInteger strlines(HSQUIRRELVM v) {
	// TODO: strlines
	warning("strlines not implemented");
	return 0;
}

static SQInteger strreplace(HSQUIRRELVM v) {
	// TODO: strreplace
	warning("strreplace not implemented");
	return 0;
}

static SQInteger strsplit(HSQUIRRELVM v) {
	// TODO: strsplit
	warning("strsplit not implemented");
	return 0;
}

static SQInteger translate(HSQUIRRELVM v) {
	// TODO: translate
	warning("translate not implemented");
	return 0;
}

void sqgame_register_genlib(HSQUIRRELVM v) {
	regFunc(v, activeVerb, _SC("activeVerb"));
	regFunc(v, adhocalytics, _SC("adhocalytics"));
	regFunc(v, arrayShuffle, _SC("arrayShuffle"));
	regFunc(v, assetExists, _SC("assetExists"));
	regFunc(v, cameraAt, _SC("cameraAt"));
	regFunc(v, cameraBounds, _SC("cameraBounds"));
	regFunc(v, cameraFollow, _SC("cameraFollow"));
	regFunc(v, cameraInRoom, _SC("cameraInRoom"));
	regFunc(v, cameraPanTo, _SC("cameraPanTo"));
	regFunc(v, cameraPos, _SC("cameraPos"));
	regFunc(v, sqChr, _SC("chr"));
	regFunc(v, cursorPosX, _SC("cursorPosX"));
	regFunc(v, cursorPosY, _SC("cursorPosY"));
	regFunc(v, distance, _SC("distance"));
	regFunc(v, findScreenPosition, _SC("findScreenPosition"));
	regFunc(v, frameCounter, _SC("frameCounter"));
	regFunc(v, getUserPref, _SC("getUserPref"));
	regFunc(v, getPrivatePref, _SC("getPrivatePref"));
	regFunc(v, incutscene, _SC("incutscene"));
	regFunc(v, indialog, _SC("indialog"));
	regFunc(v, integer, _SC("integer"));
	regFunc(v, in_array, _SC("in_array"));
	regFunc(v, is_array, _SC("is_array"));
	regFunc(v, is_function, _SC("is_function"));
	regFunc(v, is_string, _SC("is_string"));
	regFunc(v, is_table, _SC("is_table"));
	regFunc(v, sqrandom, _SC("random"));
	regFunc(v, loadArray, _SC("loadArray"));
	regFunc(v, markAchievement, _SC("markAchievement"));
	regFunc(v, markProgress, _SC("markProgress"));
	regFunc(v, markStat, _SC("markStat"));
	regFunc(v, ord, _SC("ord"));
	regFunc(v, pushSentence, _SC("pushSentence"));
	regFunc(v, randomFrom, _SC("randomFrom"));
	regFunc(v, randomOdds, _SC("randomOdds"));
	regFunc(v, randomseed, _SC("randomseed"));
	regFunc(v, refreshUI, _SC("refreshUI"));
	regFunc(v, screenSize, _SC("screenSize"));
	regFunc(v, setDebugger, _SC("setDebugger"));
	regFunc(v, setPrivatePref, _SC("setPrivatePref"));
	regFunc(v, setUserPref, _SC("setUserPref"));
	regFunc(v, setVerb, _SC("setVerb"));
	regFunc(v, startDialog, _SC("startDialog"));
	regFunc(v, stopSentence, _SC("stopSentence"));
	regFunc(v, strcount, _SC("strcount"));
	regFunc(v, strcrc, _SC("strcrc"));
	regFunc(v, strfind, _SC("strfind"));
	regFunc(v, strfirst, _SC("strfirst"));
	regFunc(v, strlast, _SC("strlast"));
	regFunc(v, strlines, _SC("strlines"));
	regFunc(v, strreplace, _SC("strreplace"));
	regFunc(v, strsplit, _SC("strsplit"));
	regFunc(v, translate, _SC("translate"));
}

} // namespace Twp
