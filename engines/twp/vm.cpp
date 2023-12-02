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

#include <vector>
#include <string>
#include "twp/vm.h"
#include "common/debug.h"
#include "common/random.h"
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

namespace Twp {

static SQRESULT get(HSQUIRRELVM v, int i, std::string &value) {
	const SQChar *val;
	SQRESULT result = sq_getstring(v, i, &val);
	value = val;
	return result;
}

static void getArray(HSQUIRRELVM v, HSQOBJECT o, std::vector<std::string> &arr) {
  sq_pushobject(v, o);
  sq_pushnull(v);
  while(SQ_SUCCEEDED(sq_next(v, -2))) {
    const SQChar* str;
    sq_getstring(v, -1, &str);
    arr.push_back(str);
    sq_pop(v, 2);
  }
  sq_pop(v, 1);
}

static SQRESULT getArray(HSQUIRRELVM v, int i, std::vector<std::string> &arr) {
  HSQOBJECT obj;
  SQRESULT result = sq_getstackobj(v, i, &obj);
  getArray(v, obj, arr);
  return result;
}

// Returns a random number from from to to inclusively.
// The number is a pseudo-random number and the game will produce the same sequence of numbers unless primed using randomSeed(seed).
//
// .. code-block:: Squirrel
// wait_time = random(0.5, 2.0)
static SQInteger sq_random(HSQUIRRELVM v) {
	Common::RandomSource rnd("twp");
	if (sq_gettype(v, 2) == OT_FLOAT || sq_gettype(v, 3) == OT_FLOAT) {
		SQFloat min, max;
		sq_getfloat(v, 2, &min);
		sq_getfloat(v, 3, &max);
		if (min > max)
			SWAP(min, max);
		float scale = rnd.getRandomNumber(RAND_MAX) / (float)RAND_MAX;
		SQFloat value = min + scale * (max - min);
		sq_pushfloat(v, value);
	} else {
		SQInteger min, max;
		sq_getinteger(v, 2, &min);
		sq_getinteger(v, 3, &max);
		if (min > max)
			SWAP(min, max);
		SQInteger value = rnd.getRandomNumberRngSigned(min, max);
		sq_pushinteger(v, value);
	}
	return 1;
}

// Creates a new, room local object using sheet as the sprite sheet and image as the image name.
// This object is deleted when the room exits.
// If sheet parameter not provided, use room's sprite sheet instead.
// If image is an array, then use that as a sequence of frames for animation.
// Objects created at runtime can be passed to all the object commands.
// They do not have verbs or local variables by default, but these can be added when the object is created so it can be used in the construction of sentences.
static SQInteger createObject(HSQUIRRELVM v) {
	SQInteger numArgs = sq_gettop(v);
	std::string sheet;
	std::vector<std::string> frames;
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
			std::string frame;
			get(v, framesIndex, frame);
			frames.push_back(frame);
		} break;
		case OT_ARRAY:
			getArray(v, framesIndex, frames);
			break;
		default:
			return sq_throwerror(v, "Invalid parameter 2: expecting a string or an array");
		}
	}

	debug("Create object: %s, %zu", sheet.c_str(), frames.size());
	// var obj = gEngine.room.createObject(sheet, frames)
	// push(v, obj.table)
	return 1;
}

static void regFunc(HSQUIRRELVM v, SQFUNCTION f, const SQChar *functionName, SQInteger nparamscheck, const SQChar *typemask) {
	sq_pushroottable(v);
	sq_pushstring(v, functionName, -1);
	sq_newclosure(v, f, 0); // create a new function
	sq_setparamscheck(v, nparamscheck, typemask);
	sq_setnativeclosurename(v, -1, functionName);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1); // pops the root table
}

static SQObjectPtr sqObj(HSQUIRRELVM v, const char *value) {
	SQObjectPtr string = SQString::Create(_ss(v), value);
	return string;
}

static inline SQObject sqObj(HSQUIRRELVM v, int value) {
	SQObject o;
	o._type = OT_INTEGER;
	o._unVal.nInteger = value;
	return o;
}

static void sqExec(HSQUIRRELVM v, const char *code) {
	SQInteger top = sq_gettop(v);
	if (SQ_FAILED(sq_compilebuffer(v, code, strlen(code), "twp", SQTrue))) {
		sqstd_printcallstack(v);
		return;
	}
	sq_pushroottable(v);
	if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue))) {
		sqstd_printcallstack(v);
		sq_pop(v, 1); // removes the closure
		return;
	}
	sq_settop(v, top);
}

static void errorHandler(HSQUIRRELVM v, const SQChar *desc, const SQChar *source, SQInteger line,
						 SQInteger column) {
	debug("TWP: desc %s, source: %s (%lld,%lld)", desc, source, line, column);
}

static SQInteger aux_printerror(HSQUIRRELVM v) {
	SQPRINTFUNCTION pf = sq_geterrorfunc(v);
	if (!pf)
		return 0;

	if (sq_gettop(v) < 1)
		return 0;

	const SQChar *error = nullptr;
	if (SQ_FAILED(sq_getstring(v, 2, &error))) {
		error = "unknown";
	}
	pf(v, _SC("\nAn error occured in the script: %s\n"), error);
	sqstd_printcallstack(v);

	return 0;
}

static void printfunc(HSQUIRRELVM v, const SQChar *s, ...) {
	char buf[1024 * 1024];
	va_list vl;
	va_start(vl, s);
	vsnprintf(buf, 1024 * 1024, s, vl);
	va_end(vl);

	debug("TWP: %s", buf);
}

Vm::Vm() {
	v = sq_open(1024);
	sq_setcompilererrorhandler(v, errorHandler);
	sq_newclosure(v, aux_printerror, 0);
	sq_seterrorhandler(v);
	sq_setprintfunc(v, printfunc, printfunc);
	sqstd_register_stringlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_iolib(v);
	regFunc(v, sq_random, _SC("random"), 0, nullptr);
	regFunc(v, createObject, _SC("createObject"), 0, nullptr);

	SQObject platform = sqObj(v, 666);
	_table(v->_roottable)->NewSlot(sqObj(v, _SC("PLATFORM")), SQObjectPtr(platform));
}

Vm::~Vm() {
	sq_close(v);
}

void Vm::exec(const SQChar *code) {
	sqExec(v, code);
}

} // namespace Twp
