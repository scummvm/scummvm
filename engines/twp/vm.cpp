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
#include "twp/vm.h"
#include "common/array.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/random.h"
#include "image/png.h"
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

static HSQUIRRELVM gVm = nullptr;

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

static SQRESULT get(HSQUIRRELVM v, int i, Common::String &value) {
	const SQChar *val;
	SQRESULT result = sq_getstring(v, i, &val);
	value = val;
	return result;
}

static void getArray(HSQUIRRELVM v, HSQOBJECT o, Common::Array<Common::String> &arr) {
	sq_pushobject(v, o);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		const SQChar *str;
		sq_getstring(v, -1, &str);
		arr.push_back(str);
		sq_pop(v, 2);
	}
	sq_pop(v, 1);
}

static SQRESULT getArray(HSQUIRRELVM v, int i, Common::Array<Common::String> &arr) {
	HSQOBJECT obj;
	SQRESULT result = sq_getstackobj(v, i, &obj);
	getArray(v, obj, arr);
	return result;
}

static Thread *thread(HSQUIRRELVM v) {
	return *Common::find_if(g_engine->threads.begin(), g_engine->threads.end(), [&](Thread *t) {
		return t->threadObj._unVal.pThread == v;
	});
}

template<typename F>
static SQInteger breakfunc(HSQUIRRELVM v, const F &func) {
	Thread *t = thread(v);
	if (!t)
		return sq_throwerror(v, "failed to get thread");
	t->suspend();
	func(*t);
	return -666;
}

// When called in a function started with startthread, execution is suspended for time seconds.
// It is an error to call breaktime in a function that was not started with startthread.
//
// . code-block:: Squirrel
// for (local x = 1; x < 4; x += 1) {
//   playSound(soundPhoneRinging)
//   breaktime(5.0)
// }
static SQInteger breaktime(HSQUIRRELVM v) {
	SQFloat time;
	if (SQ_FAILED(sq_getfloat(v, 2, &time)))
		return sq_throwerror(v, "failed to get time");
	if (time == 0.f)
		return breakfunc(v, [](Thread &t) { t.numFrames = 1; });
	else
		return breakfunc(v, [&](Thread &t) { t.waitTime = time; });
}

// Returns a random number from from to to inclusively.
// The number is a pseudo-random number and the game will produce the same sequence of numbers unless primed using randomSeed(seed).
//
// .. code-block:: Squirrel
// wait_time = random(0.5, 2.0)
static SQInteger sq_random(HSQUIRRELVM v) {
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
			getArray(v, framesIndex, frames);
			break;
		default:
			return sq_throwerror(v, "Invalid parameter 2: expecting a string or an array");
		}
	}

	debug("Create object: %s, %u", sheet.c_str(), frames.size());

	// load sheet json
	SpriteSheet* spritesheet = g_engine->resManager.spriteSheet(sheet);

	// TODO: create an entity
	Entity e;
	e.texture = g_engine->resManager.texture(sheet + ".png");
	e.rect = spritesheet->frameTable[frames[0]].frame;

	static int gId = 3000;
	sq_newtable(v);
	sq_pushstring(v, _SC("_id"), -1);
	sq_pushinteger(v, gId++);
	sq_newslot(v, -3, SQFalse);
	sq_getstackobj(v, -1, &e.obj);
	sq_addref(gVm, &e.obj);
	sq_pop(v, 1);

	g_engine->entities.push_back(e);

	sq_pushobject(v, e.obj);

	return 1;
}

static SQInteger _startthread(HSQUIRRELVM v, bool global) {
	SQInteger size = sq_gettop(v);

	Thread *t = new Thread();
	t->global = global;

	static uint64 gThreadId = 300000;
	sq_newtable(v);
	sq_pushstring(v, _SC("_id"), -1);
	sq_pushinteger(v, gThreadId++);
	sq_newslot(v, -3, SQFalse);
	sq_getstackobj(v, -1, &t->obj);
	sq_addref(gVm, &t->obj);
	sq_pop(v, 1);

	sq_resetobject(&t->envObj);
	if (SQ_FAILED(sq_getstackobj(v, 1, &t->envObj)))
		return sq_throwerror(v, "Couldn't get environment from stack");
	sq_addref(gVm, &t->envObj);

	// create thread and store it on the stack
	sq_newthread(gVm, 1024);
	sq_resetobject(&t->threadObj);
	if (SQ_FAILED(sq_getstackobj(gVm, -1, &t->threadObj)))
		return sq_throwerror(v, "Couldn't get coroutine thread from stack");
	sq_addref(gVm, &t->threadObj);

	for (int i = 0; i < size - 2; i++) {
		HSQOBJECT arg;
		sq_resetobject(&arg);
		if (SQ_FAILED(sq_getstackobj(v, 3 + i, &arg)))
			return sq_throwerror(v, "Couldn't get coroutine args from stack");
		t->args.push_back(arg);
		sq_addref(gVm, &arg);
	}

	// get the closure
	sq_resetobject(&t->closureObj);
	if (SQ_FAILED(sq_getstackobj(v, 2, &t->closureObj)))
		return sq_throwerror(v, "Couldn't get coroutine thread from stack");
	sq_addref(gVm, &t->closureObj);

	const SQChar *name = nullptr;
	if (SQ_SUCCEEDED(sq_getclosurename(v, 2)))
		sq_getstring(v, -1, &name);

	t->name = Common::String::format("%s %s (%lld)", name == nullptr ? "<anonymous>" : name, _stringval(_closure(t->closureObj)->_function->_sourcename), _closure(t->closureObj)->_function->_lineinfos->_line);
	sq_pop(gVm, 1);
	if (name)
		sq_pop(v, 1); // pop name
	sq_pop(v, 1);     // pop closure

	g_engine->threads.push_back(t);

	debug("create thread %s", t->name.c_str());

	// call the closure in the thread
	if (!t->call())
		return sq_throwerror(v, "call failed");

	sq_pushobject(v, t->obj);
	return 1;
}

static SQInteger startthread(HSQUIRRELVM v) {
	return _startthread(v, false);
}

static SQInteger startglobalthread(HSQUIRRELVM v) {
	return _startthread(v, true);
}

static SQInteger objectAt(HSQUIRRELVM v) {
	HSQOBJECT o;
	sq_getstackobj(v, 2, &o);

	SQObjectPtr id;
	_table(o)->Get(sqObj(v, "_id"), id);

	Entity *ett = Common::find_if(g_engine->entities.begin(), g_engine->entities.end(), [&](Entity &e) {
		SQObjectPtr id2;
		_table(e.obj)->Get(sqObj(v, "_id"), id2);
		return _integer(id) == _integer(id2);
	});

	if (!ett)
		return sq_throwerror(v, "failed to get object");

	SQInteger x, y;
	if (SQ_FAILED(sq_getinteger(v, 3, &x)))
		return sq_throwerror(v, "failed to get x");
	if (SQ_FAILED(sq_getinteger(v, 4, &y)))
		return sq_throwerror(v, "failed to get y");
	ett->x = x;
	ett->y = y;
	debug("Object at: %lld, %lld", x, y);

	return 0;
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
	gVm = v = sq_open(1024);
	sq_setcompilererrorhandler(v, errorHandler);
	sq_newclosure(v, aux_printerror, 0);
	sq_seterrorhandler(v);
	sq_setprintfunc(v, printfunc, printfunc);

	sq_pushroottable(v);
	sqstd_register_stringlib(v);
	sqstd_register_mathlib(v);
	sqstd_register_iolib(v);
	sq_pop(v, 1);

	regFunc(v, sq_random, _SC("random"), 0, nullptr);
	regFunc(v, createObject, _SC("createObject"), 0, nullptr);
	regFunc(v, startthread, _SC("startthread"), 0, nullptr);
	regFunc(v, startglobalthread, _SC("startglobalthread"), 0, nullptr);
	regFunc(v, objectAt, _SC("objectAt"), 0, nullptr);
	regFunc(v, breaktime, _SC("breaktime"), 0, nullptr);

	SQObject platform = sqObj(v, 666);
	_table(v->_roottable)->NewSlot(sqObj(v, _SC("PLATFORM")), SQObjectPtr(platform));
}

Vm::~Vm() {
	for (int i = 0; i < g_engine->threads.size(); i++) {
		delete g_engine->threads[i];
	}
	sq_close(v);
}

void Vm::exec(const SQChar *code) {
	sqExec(v, code);
}

Thread::Thread() : paused(false), waitTime(0), numFrames(0), stopRequest(false) {
}

Thread::~Thread() {
	for (int i = 0; i < args.size(); i++) {
		sq_release(gVm, &args[i]);
	}
	sq_release(gVm, &threadObj);
	sq_release(gVm, &envObj);
	sq_release(gVm, &closureObj);
}

bool Thread::call() {
	HSQUIRRELVM v = threadObj._unVal.pThread;
	// call the closure in the thread
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, closureObj);
	sq_pushobject(v, envObj);
	for (int i = 0; i < args.size(); i++) {
		sq_pushobject(v, args[i]);
	}
	if (SQ_FAILED(sq_call(v, 1 + args.size(), SQFalse, SQTrue))) {
		sq_settop(v, top);
		return false;
	}
	return true;
}

bool Thread::isDead() {
	SQInteger state = sq_getvmstate(threadObj._unVal.pThread);
	return stopRequest || state == 0;
}

bool Thread::isSuspended() {
	SQInteger state = sq_getvmstate(threadObj._unVal.pThread);
	return state != 1;
}

void Thread::suspend() {
	// TODO: pauseable
	if (!isSuspended()) {
		sq_suspendvm(threadObj._unVal.pThread);
	}
}

void Thread::resume() {
	if (!isDead() && isSuspended()) {
		sq_wakeupvm(threadObj._unVal.pThread, SQFalse, SQFalse, SQTrue, SQFalse);
	}
}

bool Thread::update(float elapsed) {
	if (paused) {
	} else if (waitTime > 0) {
		waitTime -= elapsed;
		if (waitTime <= 0) {
			waitTime = 0;
			resume();
		}
	} else if (numFrames > 0) {
		numFrames -= 1;
		numFrames = 0;
		resume();
	}
	return isDead();
}

} // namespace Twp
