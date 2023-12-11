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
#include "twp/thread.h"
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

static Thread *thread(HSQUIRRELVM v) {
	return *Common::find_if(g_engine->_threads.begin(), g_engine->_threads.end(), [&](Thread *t) {
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

static SQInteger _startthread(HSQUIRRELVM v, bool global) {
	HSQUIRRELVM vm = g_engine->getVm();
	SQInteger size = sq_gettop(v);

	Thread *t = new Thread();
	t->global = global;

	static uint64 gThreadId = 300000;
	sq_newtable(v);
	sq_pushstring(v, _SC("_id"), -1);
	sq_pushinteger(v, gThreadId++);
	sq_newslot(v, -3, SQFalse);
	sq_getstackobj(v, -1, &t->obj);
	sq_addref(vm, &t->obj);
	sq_pop(v, 1);

	sq_resetobject(&t->envObj);
	if (SQ_FAILED(sq_getstackobj(v, 1, &t->envObj)))
		return sq_throwerror(v, "Couldn't get environment from stack");
	sq_addref(vm, &t->envObj);

	// create thread and store it on the stack
	sq_newthread(vm, 1024);
	sq_resetobject(&t->threadObj);
	if (SQ_FAILED(sq_getstackobj(vm, -1, &t->threadObj)))
		return sq_throwerror(v, "Couldn't get coroutine thread from stack");
	sq_addref(vm, &t->threadObj);

	for (int i = 0; i < size - 2; i++) {
		HSQOBJECT arg;
		sq_resetobject(&arg);
		if (SQ_FAILED(sq_getstackobj(v, 3 + i, &arg)))
			return sq_throwerror(v, "Couldn't get coroutine args from stack");
		t->args.push_back(arg);
		sq_addref(vm, &arg);
	}

	// get the closure
	sq_resetobject(&t->closureObj);
	if (SQ_FAILED(sq_getstackobj(v, 2, &t->closureObj)))
		return sq_throwerror(v, "Couldn't get coroutine thread from stack");
	sq_addref(vm, &t->closureObj);

	const SQChar *name = nullptr;
	if (SQ_SUCCEEDED(sq_getclosurename(v, 2)))
		sq_getstring(v, -1, &name);

	t->name = Common::String::format("%s %s (%lld)", name == nullptr ? "<anonymous>" : name, _stringval(_closure(t->closureObj)->_function->_sourcename), _closure(t->closureObj)->_function->_lineinfos->_line);
	sq_pop(vm, 1);
	if (name)
		sq_pop(v, 1); // pop name
	sq_pop(v, 1);     // pop closure

	g_engine->_threads.push_back(t);

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

void sqgame_register_syslib(HSQUIRRELVM v) {
	regFunc(v, startthread, _SC("startthread"), 0, nullptr);
	regFunc(v, startglobalthread, _SC("startglobalthread"), 0, nullptr);
	regFunc(v, breaktime, _SC("breaktime"), 0, nullptr);
}

}
