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

#include "common/config-manager.h"
#include "common/crc.h"
#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/dialog.h"
#include "twp/hud.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/sqgame.h"
#include "twp/squtil.h"
#include "twp/squirrel/sqvm.h"
#include "twp/squirrel/sqstring.h"
#include "twp/squirrel/sqstate.h"
#include "twp/squirrel/sqtable.h"
#include "twp/squirrel/sqfuncproto.h"
#include "twp/squirrel/sqclosure.h"
#include "twp/tsv.h"

namespace Twp {

struct GetArray {
	explicit GetArray(Common::Array<HSQOBJECT> &objs) : _objs(objs) {}

	void operator()(HSQOBJECT &o) {
		_objs.push_back(o);
	}

private:
	Common::Array<HSQOBJECT> &_objs;
};

template<typename T>
static void shuffle(Common::Array<T> &array) {
	if (array.size() > 1) {
		for (size_t i = 0; i < array.size(); i++) {
			size_t j = g_twp->getRandomSource().getRandomNumberRng(0, array.size() - 1);
			SWAP(array[j], array[i]);
		}
	}
}

static SQInteger activeVerb(HSQUIRRELVM v) {
	sqpush(v, g_twp->_hud->_verb.id.id);
	return 1;
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
	for (auto &it : arr) {
		sqpush(v, it);
		sq_arrayappend(v, -2);
	}
	return 1;
}

// Returns TRUE if the specified entry exists in the assets.
static SQInteger assetExists(HSQUIRRELVM v) {
	const SQChar *filename;
	if (SQ_FAILED(sq_getstring(v, 2, &filename)))
		return sq_throwerror(v, "failed to get filename");
	sqpush(v, g_twp->_pack->assetExists(filename));
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
	SQInteger numArgs = sq_gettop(v);
	Math::Vector2d pos;
	if (numArgs == 3) {
		SQInteger x, y;
		if (SQ_FAILED(sqget(v, 2, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 3, y)))
			return sq_throwerror(v, "failed to get y");
		pos = Math::Vector2d(x, y);
	} else if (numArgs == 2) {
		Common::SharedPtr<Object> obj = sqobj(v, 2);
		if (!obj)
			return sq_throwerror(v, "failed to get spot or actor");
		g_twp->follow(nullptr);
		g_twp->setRoom(obj->_room);
		pos = obj->getUsePos();
	} else {
		return sq_throwerror(v, Common::String::format("invalid argument number: %lld", numArgs).c_str());
	}
	g_twp->follow(nullptr);
	g_twp->cameraAt(pos);
	return 0;
}

// Sets how far the camera can pan.
static SQInteger cameraBounds(HSQUIRRELVM v) {
	SQInteger xMin, xMax, yMin, yMax;
	if (SQ_FAILED(sqget(v, 2, xMin)))
		return sq_throwerror(v, "failed to get xMin");
	if (SQ_FAILED(sqget(v, 3, xMax)))
		return sq_throwerror(v, "failed to get xMax");
	if (SQ_FAILED(sqget(v, 4, yMin)))
		return sq_throwerror(v, "failed to get yMin");
	if (SQ_FAILED(sqget(v, 5, yMax)))
		return sq_throwerror(v, "failed to get yMax");
	g_twp->_camera->setBounds(Rectf::fromMinMax(Math::Vector2d(xMin, yMin), Math::Vector2d(xMax, yMax)));
	return 0;
}

static SQInteger cameraFollow(HSQUIRRELVM v) {
	Common::SharedPtr<Object> actor = sqactor(v, 2);
	g_twp->follow(actor);
	Math::Vector2d pos = actor->_node->getPos();
	Common::SharedPtr<Room> oldRoom = g_twp->_room;
	if (actor->_room)
		g_twp->setRoom(actor->_room);
	if (oldRoom != actor->_room)
		g_twp->cameraAt(pos);
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
	Common::SharedPtr<Room> room = sqroom(v, 2);
	if (room) {
		g_twp->setRoom(room);
	} else {
		Common::SharedPtr<Object> obj = sqobj(v, 2);
		if (!obj || !obj->_room) {
			return sq_throwerror(v, "failed to get room");
		}
		g_twp->setRoom(obj->_room);
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
		Common::SharedPtr<Object> obj = sqobj(v, 2);
		if (!obj)
			return sq_throwerror(v, "failed to get object/actor");
		pos = obj->getUsePos();
		if (SQ_FAILED(sqget(v, 3, duration)))
			return sq_throwerror(v, "failed to get duration");
	} else if (numArgs == 4) {
		if (sq_gettype(v, 2) == OT_INTEGER) {
			SQInteger x;
			if (SQ_FAILED(sqget(v, 2, x)))
				return sq_throwerror(v, "failed to get x");
			if (SQ_FAILED(sqget(v, 3, duration)))
				return sq_throwerror(v, "failed to get duration");
			SQInteger im;
			if (SQ_FAILED(sqget(v, 4, im)))
				return sq_throwerror(v, "failed to get interpolation method");
			pos = Math::Vector2d(x, g_twp->getGfx().cameraPos().getY());
			interpolation = (InterpolationKind)im;
		} else {
			Common::SharedPtr<Object> obj = sqobj(v, 2);
			if (SQ_FAILED(sqget(v, 3, duration)))
				return sq_throwerror(v, "failed to get duration");
			SQInteger im;
			if (SQ_FAILED(sqget(v, 4, im)))
				return sq_throwerror(v, "failed to get interpolation method");
			pos = obj->_node->getAbsPos();
			interpolation = (InterpolationKind)im;
		}
	} else if (numArgs == 5) {
		SQInteger x, y;
		if (SQ_FAILED(sqget(v, 2, x)))
			return sq_throwerror(v, "failed to get x");
		if (SQ_FAILED(sqget(v, 3, y)))
			return sq_throwerror(v, "failed to get y");
		if (SQ_FAILED(sqget(v, 4, duration)))
			return sq_throwerror(v, "failed to get duration");
		SQInteger im;
		if (SQ_FAILED(sqget(v, 5, im)))
			return sq_throwerror(v, "failed to get interpolation method");
		pos = Math::Vector2d(x, y);
		interpolation = (InterpolationKind)im;
	} else {
		return sq_throwerror(v, Common::String::format("invalid argument number: %lld", numArgs).c_str());
	}
	debugC(kDebugGenScript, "cameraPanTo: (%f,%f), dur=%f, method=%d", pos.getX(), pos.getY(), duration, interpolation);
	g_twp->follow(nullptr);
	g_twp->_camera->panTo(pos, duration, interpolation);
	return 0;
}

// Returns the current camera position x, y.
static SQInteger cameraPos(HSQUIRRELVM v) {
	sqpush(v, g_twp->cameraPos());
	return 1;
}

// Converts an integer to a char.
static SQInteger sqChr(HSQUIRRELVM v) {
	SQInteger value;
	if (SQ_FAILED(sqget(v, 2, value)))
		return sq_throwerror(v, "failed to get value");
	Common::String s;
	s += char(value);
	sqpush(v, Common::move(s));
	return 1;
}

// Returns x coordinates of the mouse in screen coordinates.
static SQInteger cursorPosX(HSQUIRRELVM v) {
	sqpush(v, g_twp->_cursor.pos.getX());
	return 1;
}

// Returns y coordinates of the mouse in screen coordinates.
static SQInteger cursorPosY(HSQUIRRELVM v) {
	sqpush(v, g_twp->_cursor.pos.getY());
	return 1;
}

static SQInteger distance(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) == OT_INTEGER) {
		SQInteger num1;
		if (SQ_FAILED(sqget(v, 2, num1)))
			return sq_throwerror(v, "failed to get num1");
		SQInteger num2;
		if (SQ_FAILED(sqget(v, 3, num2)))
			return sq_throwerror(v, "failed to get num2");
		float d = abs(num1 - num2);
		sqpush(v, d);
		return 1;
	}

	Common::SharedPtr<Object> obj1 = sqobj(v, 2);
	if (!obj1)
		return sq_throwerror(v, "failed to get object1 or actor1");
	Common::SharedPtr<Object> obj2 = sqobj(v, 3);
	if (!obj2)
		return sq_throwerror(v, "failed to get object2 or actor2");
	Math::Vector2d d = obj1->_node->getAbsPos() - obj2->_node->getAbsPos();
	sqpush(v, sqrt(d.getX() * d.getX() + d.getY() * d.getY()));
	return 1;
}

static SQInteger findScreenPosition(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) == OT_INTEGER) {
		SQInteger verb;
		if (SQ_FAILED(sqget(v, 2, verb)))
			return sq_throwerror(v, "failed to get verb");
		ActorSlot *actorSlot = g_twp->_hud->actorSlot(g_twp->_actor);
		if (!actorSlot)
			return 0;
		for (int i = 1; i < MAX_VERBS; i++) {
			const Verb &vb = actorSlot->verbSlots[i]._verb;
			if (vb.id.id == verb) {
				SpriteSheet *verbSheet = g_twp->_resManager->spriteSheet("VerbSheet");
				const SpriteSheetFrame *verbFrame = &verbSheet->getFrame(Common::String::format("%s_en", vb.image.c_str()));
				Math::Vector2d pos(verbFrame->spriteSourceSize.left + verbFrame->frame.width() / 2.f, verbFrame->sourceSize.getY() - verbFrame->spriteSourceSize.top - verbFrame->spriteSourceSize.height() + verbFrame->frame.height() / 2.f);
				debugC(kDebugGenScript, "findScreenPosition(%lld) => %f,%f", verb, pos.getX(), pos.getY());
				sqpush(v, Common::move(pos));
				return 1;
			}
		}
		return sq_throwerror(v, "failed to find verb");
	}
	Common::SharedPtr<Object> obj = sqobj(v, 2);
	if (!obj)
		return sq_throwerror(v, "failed to get object or actor");
	if (obj->inInventory()) {
		sqpush(v, g_twp->_uiInv.getPos(obj));
		return 1;
	}

	Math::Vector2d rPos = g_twp->roomToScreen(obj->_node->getAbsPos());
	Math::Vector2d pos(rPos.getX() + obj->_node->getSize().getX() / 2.f, rPos.getY() + obj->_node->getSize().getY() / 2.f);
	debugC(kDebugGenScript, "findScreenPosition(%s) => (%f,%f)", obj->_name.c_str(), pos.getX(), pos.getY());
	sqpush(v, Common::move(pos));
	return 1;
}

static SQInteger frameCounter(HSQUIRRELVM v) {
	return sqpush(v, g_twp->_frameCounter);
}

static Common::String toPlatform(const Common::String &value) {
	const char *platformsSrc[] = {"xbox", "switch", "playstation", "playstation2"};
	const char *platformsDst[] = {"XBOX", "SWITCH", "PS4", "PS4"};
	for (int i = 0; i < ARRAYSIZE(platformsSrc); i++) {
		if (value == platformsSrc[i]) {
			return platformsDst[i];
		}
	}
	return "0";
}

static SQInteger getUserPref(HSQUIRRELVM v) {
	Common::String key;
	if (SQ_FAILED(sqget(v, 2, key))) {
		return sq_throwerror(v, _SC("failed to get key"));
	}

	if (key == "platform" && ConfMan.hasKey(key)) {
		sqpush(v, toPlatform(ConfMan.get(key)));
		return 1;
	}

	SQInteger numArgs = sq_gettop(v) - 1;
	// is there a fault value as argument ?
	if (numArgs > 1) {
		SQObjectType type = sq_gettype(v, 3);
		if (type == SQObjectType::OT_STRING) {
			Common::String str;
			if (SQ_FAILED(sqget(v, 3, str))) {
				return sq_throwerror(v, _SC("failed to get string"));
			}
			sqpush(v, ConfMan.hasKey(key) ? ConfMan.get(key) : str);
			return 1;
		}
		if (type == SQObjectType::OT_INTEGER) {
			SQInteger integer;
			if (SQ_FAILED(sqget(v, 3, integer))) {
				return sq_throwerror(v, _SC("failed to get integer"));
			}
			if (ConfMan.hasKey(key)) {
				Common::String value = ConfMan.get(key);
				bool bValue = false;
				if (parseBool(value, bValue)) {
					sqpush(v, bValue);
					return 1;
				}
				sqpush(v, ConfMan.getInt(key));
				return 1;
			}

			sqpush(v, integer);
			return 1;
		}
		if (type == SQObjectType::OT_FLOAT) {
			SQFloat fl;
			if (SQ_FAILED(sqget(v, 3, fl))) {
				return sq_throwerror(v, _SC("failed to get float"));
			}
			sqpush(v, ConfMan.hasKey(key) ? atof(ConfMan.get(key).c_str()) : fl);
			return 1;
		}
		return sq_throwerror(v, _SC("failed to get user preferences"));
	}

	if (ConfMan.hasKey(key)) {
		Common::String value = ConfMan.get(key);
		bool isNumber = !value.empty();
		bool isFloat = false;
		for (size_t i = 0; i < value.size(); i++) {
			if (!isFloat && value[i] == '.') {
				isFloat = true;
				continue;
			}
			if (!Common::isDigit(value[i])) {
				isNumber = false;
				break;
			}
		}
		if (!isNumber) {
			sqpush(v, value);
			return 1;
		}
		if (isFloat) {
			sqpush(v, atof(value.c_str()));
			return 1;
		}
		sqpush(v, (SQInteger)atol(value.c_str()));
		return 1;
	}

	sq_pushnull(v);
	return 1;
}

static SQInteger getPrivatePref(HSQUIRRELVM v) {
	Common::String key;
	if (SQ_FAILED(sqget(v, 2, key))) {
		return sq_throwerror(v, "failed to get key");
	} else if (ConfMan.hasKey(key)) {
		Common::String value = ConfMan.get(key);
		char *errpos;
		int ivalue = (int)strtol(value.c_str(), &errpos, 0);
		if (value.c_str() == errpos) {
			return sqpush(v, ConfMan.get(key));
		}
		return sqpush(v, ivalue);
	} else if (sq_gettop(v) == 3) {
		HSQOBJECT obj;
		sq_getstackobj(v, 3, &obj);
		sqpush(v, obj);
		return 1;
	}
	sq_pushnull(v);
	return 1;
}

static SQInteger incutscene(HSQUIRRELVM v) {
	sqpush(v, g_twp->_cutscene != nullptr);
	return 1;
}

static SQInteger indialog(HSQUIRRELVM v) {
	sqpush(v, (int)g_twp->_dialog->getState());
	return 1;
}

static SQInteger integer(HSQUIRRELVM v) {
	float f = 0.f;
	if (SQ_FAILED(sqget(v, 2, f)))
		return sq_throwerror(v, "failed to get float value");
	sqpush(v, static_cast<int>(f));
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

	for (auto &it : objs) {
		sq_pushobject(v, obj);
		sq_pushobject(v, it);
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
	return sqpush(v, sq_gettype(v, 2) == OT_ARRAY);
}

static SQInteger is_function(HSQUIRRELVM v) {
	SQObjectType type = sq_gettype(v, 2);
	return sqpush(v, type == OT_CLOSURE || type == OT_NATIVECLOSURE);
}

static SQInteger is_string(HSQUIRRELVM v) {
	return sqpush(v, sq_gettype(v, 2) == OT_STRING);
}

static SQInteger is_table(HSQUIRRELVM v) {
	return sqpush(v, sq_gettype(v, 2) == OT_TABLE);
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
		SQFloat value = g_twp->getRandom(min, max);
		sq_pushfloat(v, value);
	} else {
		SQInteger min, max;
		sq_getinteger(v, 2, &min);
		sq_getinteger(v, 3, &max);
		if (min > max)
			SWAP(min, max);
		SQInteger value = g_twp->getRandomSource().getRandomNumberRngSigned(min, max);
		sq_pushinteger(v, value);
	}
	return 1;
}

// Returns an array of all the lines of the given `filename`.
static SQInteger loadArray(HSQUIRRELVM v) {
	const SQChar *orgFilename = nullptr;
	if (SQ_FAILED(sqget(v, 2, orgFilename)))
		return sq_throwerror(v, "failed to get filename");
	debugC(kDebugGenScript, "loadArray: %s", orgFilename);
	Common::String filename = ResManager::getKey(orgFilename);
	GGPackEntryReader entry;
	entry.open(*g_twp->_pack, g_twp->_pack->assetExists(filename.c_str()) ? Common::move(filename) : orgFilename);
	sq_newarray(v, 0);
	while (!entry.eos()) {
		Common::String line = entry.readLine();
		sq_pushstring(v, line.c_str(), -1);
		sq_arrayappend(v, -2);
	}
	return 1;
}

static SQInteger markAchievement(HSQUIRRELVM v) {
	Common::String id;
	if (SQ_FAILED(sqget(v, 2, id)))
		return sq_throwerror(v, "failed to get id");

	SQInteger numArgs = sq_gettop(v);
	switch (numArgs) {
	case 2:
		AchMan.setAchievement(id);
		break;
	case 4: {
		SQInteger count, total;
		if (SQ_FAILED(sqget(v, 3, count)))
			return sq_throwerror(v, "failed to get count");
		if (SQ_FAILED(sqget(v, 4, total)))
			return sq_throwerror(v, "failed to get total");
		AchMan.setStatInt(Common::String::format("ST%s", id.substr(3).c_str()), count);
		if (count == total) {
			AchMan.setAchievement(id);
		}
	} break;
	default:
		error("TODO: markAchievement not implemented");
		break;
	}
	return 0;
}

static SQInteger markProgress(HSQUIRRELVM v) {
	warning("TODO: markProgress not implemented");
	return 0;
}

static SQInteger markStat(HSQUIRRELVM v) {
	warning("TODO: markStat not implemented");
	return 0;
}

// Returns the internal int value of x
static SQInteger ord(HSQUIRRELVM v) {
	Common::String letter;
	if (SQ_FAILED(sqget(v, 2, letter)))
		return sq_throwerror(v, "Failed to get letter");
	if (letter.size() > 0) {
		sq_pushinteger(v, (int)(letter[0]));
	} else {
		sq_pushinteger(v, 0);
	}
	return 1;
}

// Executes a verb sentence as though the player had inputted/constructed it themselves.
// You can push several sentences one after the other.
// They will execute in reverse order (it's a stack).
static SQInteger pushSentence(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	SQInteger id;
	if (SQ_FAILED(sqget(v, 2, id)))
		return sq_throwerror(v, "Failed to get verb id");

	if (id == VERB_DIALOG) {
		SQInteger choice;
		if (SQ_FAILED(sqget(v, 3, choice)))
			return sq_throwerror(v, "Failed to get choice");
		// use pushSentence with VERB_DIALOG
		g_twp->_dialog->choose(choice);
		return 0;
	}

	Common::SharedPtr<Object> obj1;
	Common::SharedPtr<Object> obj2;
	if (nArgs >= 3) {
		obj1 = sqobj(v, 3);
		if (!obj1)
			return sq_throwerror(v, "Failed to get obj1");
	}
	if (nArgs == 4) {
		obj2 = sqobj(v, 4);
		if (!obj2)
			return sq_throwerror(v, "Failed to get obj2");
	}
	VerbId verb;
	verb.id = id;
	g_twp->execSentence(nullptr, verb, obj1, obj2);
	return 0;
}

// Selects an item randomly from the given array or listed options.
//
// .. code-block:: Squirrel
// local line = randomfrom(lines)
// breakwhiletalking(willie)
// mumbleLine(willie, line)
//
// local snd = randomfrom(soundBeep1, soundBeep2, soundBeep3, soundBeep4, soundBeep5, soundBeep6)
// playObjectSound(snd, Highway.pigeonVan)
static SQInteger randomFrom(HSQUIRRELVM v) {
	if (sq_gettype(v, 2) == OT_ARRAY) {
		HSQOBJECT obj;
		sq_resetobject(&obj);
		SQInteger size = sq_getsize(v, 2);
		assert(size > 0);
		int index = g_twp->getRandomSource().getRandomNumber(size - 1);
		assert(index >= 0);
		int i = 0;
		sq_push(v, 2);  // array
		sq_pushnull(v); // null iterator
		while (SQ_SUCCEEDED(sq_next(v, -2))) {
			sq_getstackobj(v, -1, &obj);
			sq_pop(v, 2); // pops key and val before the nex iteration
			if (index == i) {
				sq_pop(v, 2); // pops the null iterator and array
				sq_pushobject(v, obj);
				return 1;
			}
			i++;
		}
		sq_pop(v, 2); // pops the null iterator and array
		sq_pushobject(v, obj);
	} else {
		SQInteger size = sq_gettop(v);
		int index = g_twp->getRandomSource().getRandomNumber(size - 2);
		assert(index >= 0);
		sq_push(v, 2 + index);
	}
	return 1;
}

// Returns TRUE or FALSE based on the percent, which needs to be from 0.0 to 1.0.
//
// A percent of 0.0 will always return FALSE and 1.0 will always return TRUE.
// `randomOdds(0.3333)` will return TRUE about one third of the time.
//
// .. code-block:: Squirrel
// if (randomOdds(0.5) { ... }
static SQInteger randomOdds(HSQUIRRELVM v) {
	float value = 0.0f;
	if (SQ_FAILED(sqget(v, 2, value)))
		return sq_throwerror(v, "failed to get value");
	float rnd = g_twp->getRandom();
	bool res = rnd <= value;
	sq_pushbool(v, res);
	return 1;
}

// Initializes a new Rand state using the given seed.
// Providing a specific seed will produce the same results for that seed each time.
// The resulting state is independent of the default RNG's state.
static SQInteger randomseed(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	switch (nArgs) {
	case 1: {
		sqpush(v, (int)g_twp->getRandomSource().getSeed());
		return 1;
	}
	case 2: {
		SQInteger seed = 0;
		if (sq_gettype(v, 2) == OT_NULL) {
			g_twp->getRandomSource().setSeed(g_twp->getRandomSource().generateNewSeed());
			return 0;
		}
		if (SQ_FAILED(sqget(v, 2, seed))) {
			return sq_throwerror(v, "failed to get seed");
		}
		g_twp->getRandomSource().setSeed(seed);
		return 0;
	}
	}
	return sq_throwerror(v, "invalid number of parameters for randomseed");
}

static SQInteger refreshUI(HSQUIRRELVM v) {
	// TODO: refreshUI
	warning("refreshUI not implemented");
	return 0;
}

// Returns the x and y dimensions of the current screen/window.
//
// .. code-block:: Squirrel
// function clickedAt(x,y) {
//     local screenHeight = screenSize().y
//     local exitButtonB = screenHeight - (exitButtonPadding + 16)
//     if (y > exitButtonB) { ... }
// }
static SQInteger screenSize(HSQUIRRELVM v) {
	Math::Vector2d screen = g_twp->_room->getScreenSize();
	sqpush(v, Common::move(screen));
	return 1;
}

static SQInteger setDebugger(HSQUIRRELVM v) {
	// TODO: setDebugger
	warning("setDebugger not implemented");
	return 0;
}

static SQInteger setPrivatePref(HSQUIRRELVM v) {
	Common::String key;
	if (SQ_FAILED(sqget(v, 2, key))) {
		return sq_throwerror(v, _SC("failed to get key"));
	}
	SQObjectType type = sq_gettype(v, 3);
	switch (type) {
	case SQObjectType::OT_STRING: {
		Common::String str;
		if (SQ_FAILED(sqget(v, 3, str))) {
			return sq_throwerror(v, _SC("failed to get str"));
		}
		ConfMan.set(key, str);
		return 0;
	}
	case SQObjectType::OT_INTEGER:
		SQInteger integer;
		if (SQ_FAILED(sqget(v, 3, integer))) {
			return sq_throwerror(v, _SC("failed to get integer"));
		}
		ConfMan.setInt(key, (int)integer);
		return 0;
	default:
		break;
	}
	warning("setPrivatePref not implemented");
	return 0;
}

static SQInteger setUserPref(HSQUIRRELVM v) {
	Common::String key;
	if (SQ_FAILED(sqget(v, 2, key))) {
		return sq_throwerror(v, _SC("failed to get key"));
	}
	SQObjectType type = sq_gettype(v, 3);
	switch (type) {
	case SQObjectType::OT_STRING: {
		Common::String str;
		if (SQ_FAILED(sqget(v, 3, str))) {
			return sq_throwerror(v, _SC("failed to get str"));
		}
		ConfMan.set(key, str);
		return 0;
	}
	case SQObjectType::OT_INTEGER:
		SQInteger integer;
		if (SQ_FAILED(sqget(v, 3, integer))) {
			return sq_throwerror(v, _SC("failed to get integer"));
		}
		ConfMan.setInt(key, (int)integer);
		return 0;
	default:
		break;
	}
	warning("setUserPref not implemented");
	return 0;
}

static SQInteger setVerb(HSQUIRRELVM v) {
	SQInteger actorSlot;
	if (SQ_FAILED(sqget(v, 2, actorSlot)))
		return sq_throwerror(v, "failed to get actor slot");
	SQInteger verbSlot;
	if (SQ_FAILED(sqget(v, 3, verbSlot)))
		return sq_throwerror(v, "failed to get verb slot");
	HSQOBJECT table;
	if (SQ_FAILED(sqget(v, 4, table)))
		return sq_throwerror(v, "failed to get verb definitionTable");
	if (!sq_istable(table))
		return sq_throwerror(v, "verb definitionTable is not a table");
	SQInteger id = 0;
	Common::String image;
	Common::String text;
	Common::String fun;
	Common::String key;
	SQInteger flags = 0;
	if (SQ_FAILED(sqgetf(table, "verb", id)))
		return sq_throwerror(v, "failed to get verb");
	if (SQ_FAILED(sqgetf(table, "text", text)))
		return sq_throwerror(v, "failed to get verb text");
	if (sqrawexists(table, "image") && SQ_FAILED(sqgetf(table, "image", image)))
		return sq_throwerror(v, "failed to get verb image");
	if (sqrawexists(table, "func") && SQ_FAILED(sqgetf(table, "func", fun)))
		return sq_throwerror(v, "failed to get verb func");
	if (sqrawexists(table, "key") && SQ_FAILED(sqgetf(table, "key", key)))
		return sq_throwerror(v, "failed to get verb key");
	if (sqrawexists(table, "flags") && SQ_FAILED(sqgetf(table, "flags", flags)))
		return sq_throwerror(v, "failed to get verb flags");
	debugC(kDebugGenScript, "setVerb %lld, %lld, %lld, %s", actorSlot, verbSlot, id, text.c_str());
	VerbId verbId;
	verbId.id = id;
	g_twp->_hud->_actorSlots[actorSlot - 1].verbSlots[verbSlot]._verb = Verb(verbId, image, fun, text, key, flags);
	return 0;
}

static SQInteger startDialog(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	Common::String dialog;
	if (SQ_FAILED(sqget(v, 2, dialog)))
		return sq_throwerror(v, "failed to get dialog");

	Common::String node = "start";
	if (nArgs == 3) {
		if (SQ_FAILED(sqget(v, 3, node))) {
			return sq_throwerror(v, "failed to get node");
		}
	}
	Common::String actor = g_twp->_actor ? g_twp->_actor->_key : "";
	g_twp->_dialog->start(actor, dialog, node);
	return 0;
}

static SQInteger stopSentence(HSQUIRRELVM v) {
	SQInteger nArgs = sq_gettop(v);
	switch (nArgs) {
	case 1: {
		for (auto layer : g_twp->_room->_layers) {
			for (auto obj : layer->_objects) {
				obj->_exec.enabled = false;
			}
		}
		break;
	case 2: {
		Common::SharedPtr<Object> obj = sqobj(v, 2);
		obj->_exec.enabled = false;
		break;
	}
	default:
		warning("stopSentence not implemented with %lld arguments", nArgs);
		break;
	}
	}
	return 0;
}

// Counts the occurrences of a substring sub in the string `str`.
static SQInteger strcount(HSQUIRRELVM v) {
	const char *str;
	const char *sub;
	if (SQ_FAILED(sqget(v, 2, str)))
		return sq_throwerror(v, "Failed to get str");
	if (SQ_FAILED(sqget(v, 3, sub)))
		return sq_throwerror(v, "Failed to get sub");
	int count = 0;
	while ((str = strstr(str, sub))) {
		str += strlen(sub);
		++count;
	}
	sq_pushinteger(v, count);
	return 1;
}

static SQInteger strcrc(HSQUIRRELVM v) {
	Common::CRC32 crc;
	const SQChar *str;
	if (SQ_FAILED(sq_getstring(v, 2, &str))) {
		return sq_throwerror(v, _SC("failed to get string"));
	}
	uint32 u = crc.crcFast((const byte *)str, strlen(str));
	sq_pushinteger(v, (SQInteger)u);
	return 1;
}

static SQInteger strfind(HSQUIRRELVM v) {
	const SQChar *str1;
	if (SQ_FAILED(sq_getstring(v, 2, &str1))) {
		return sq_throwerror(v, _SC("failed to get string1"));
	}
	const SQChar *str2;
	if (SQ_FAILED(sq_getstring(v, 3, &str2))) {
		return sq_throwerror(v, _SC("failed to get string1"));
	}
	const char *p = strstr(str1, str2);
	if (p == nullptr) {
		sq_pushinteger(v, -1);
	} else {
		sq_pushinteger(v, p - str1);
	}
	return 1;
}

static SQInteger strfirst(HSQUIRRELVM v) {
	const SQChar *str;
	if (SQ_FAILED(sq_getstring(v, 2, &str))) {
		return sq_throwerror(v, _SC("failed to get string"));
	}
	if (strlen(str) > 0) {
		const SQChar s[2]{str[0], '\0'};
		sq_pushstring(v, s, 1);
		return 1;
	}
	sq_pushnull(v);
	return 1;
}

static SQInteger strlast(HSQUIRRELVM v) {
	const SQChar *str;
	if (SQ_FAILED(sq_getstring(v, 2, &str))) {
		return sq_throwerror(v, _SC("failed to get string"));
	}
	auto len = strlen(str);
	if (len > 0) {
		const SQChar s[2]{str[len - 1], '\0'};
		sq_pushstring(v, s, 1);
		return 1;
	}
	sq_pushnull(v);
	return 1;
}

static SQInteger strlines(HSQUIRRELVM v) {
	Common::String text;
	if (SQ_FAILED(sqget(v, 2, text))) {
		return sq_throwerror(v, _SC("failed to get text"));
	}
	Common::String line;
	Common::MemoryReadStream ms((const byte *)text.c_str(), text.size());
	sq_newarray(v, 0);
	while (!ms.eos()) {
		line = ms.readLine();
		sq_pushstring(v, line.c_str(), line.size());
		sq_arrayappend(v, -2);
	}
	return 1;
}

static SQInteger strreplace(HSQUIRRELVM v) {
	const SQChar *input;
	const SQChar *search;
	const SQChar *replace;
	if (SQ_FAILED(sq_getstring(v, 2, &input))) {
		return sq_throwerror(v, _SC("failed to get input"));
	}
	if (SQ_FAILED(sq_getstring(v, 3, &search))) {
		return sq_throwerror(v, _SC("failed to get search"));
	}
	if (SQ_FAILED(sq_getstring(v, 4, &replace))) {
		return sq_throwerror(v, _SC("failed to get replace"));
	}
	Common::String strInput(input);
	Common::String strSearch(search);
	Common::String strReplace(replace);
	strInput = replaceAll(strInput, strSearch, strReplace);
	sq_pushstring(v, strInput.c_str(), strInput.size());
	return 1;
}

// Splits the string `str` into substrings using a string separator.
static SQInteger strsplit(HSQUIRRELVM v) {
	Common::String str;
	const SQChar *delimiter;
	if (SQ_FAILED(sqget(v, 2, str)))
		return sq_throwerror(v, "Failed to get str");
	if (SQ_FAILED(sqget(v, 3, delimiter)))
		return sq_throwerror(v, "Failed to get delimiter");
	sq_newarray(v, 0);
	size_t delLen = strlen(delimiter);
	if (delLen == 0)
		return 1;

	size_t len = str.size();
	char *s = str.begin();
	size_t result = strcspn(s, delimiter);
	while (result != len) {
		sq_pushstring(v, s, result);
		sq_arrayappend(v, -2);
		s += (result + delLen);
		len -= (result + delLen);
		result = strcspn(s, delimiter);
	}
	sq_pushstring(v, s, result);
	sq_arrayappend(v, -2);
	return 1;
}

static SQInteger translate(HSQUIRRELVM v) {
	const SQChar *text;
	if (SQ_FAILED(sqget(v, 2, text)))
		return sq_throwerror(v, "Failed to get text");
	Common::String newText = g_twp->getTextDb().getText(text);
	debugC(kDebugGenScript, "translate(%s): %s", text, newText.c_str());
	sqpush(v, Common::move(newText));
	return 1;
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
	regFunc(v, integer, _SC("int"));
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
	regFunc(v, randomFrom, _SC("randomfrom"));
	regFunc(v, randomOdds, _SC("randomOdds"));
	regFunc(v, randomOdds, _SC("randomodds"));
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
