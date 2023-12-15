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
#include "common/debug.h"

namespace Twp {

static Thread *thread(HSQUIRRELVM v) {
	return *Common::find_if(g_engine->_threads.begin(), g_engine->_threads.end(), [&](Thread *t) {
		return t->_threadObj._unVal.pThread == v;
	});
}

static SQInteger _startthread(HSQUIRRELVM v, bool global) {
	HSQUIRRELVM vm = g_engine->getVm();
	SQInteger size = sq_gettop(v);

	Thread *t = new Thread();
	t->_global = global;

	static uint64 gThreadId = 300000;
	sq_newtable(v);
	sq_pushstring(v, _SC("_id"), -1);
	sq_pushinteger(v, gThreadId++);
	sq_newslot(v, -3, SQFalse);
	sq_getstackobj(v, -1, &t->_obj);
	sq_addref(vm, &t->_obj);
	sq_pop(v, 1);

	sq_resetobject(&t->_envObj);
	if (SQ_FAILED(sq_getstackobj(v, 1, &t->_envObj)))
		return sq_throwerror(v, "Couldn't get environment from stack");
	sq_addref(vm, &t->_envObj);

	// create thread and store it on the stack
	sq_newthread(vm, 1024);
	sq_resetobject(&t->_threadObj);
	if (SQ_FAILED(sq_getstackobj(vm, -1, &t->_threadObj)))
		return sq_throwerror(v, "Couldn't get coroutine thread from stack");
	sq_addref(vm, &t->_threadObj);

	for (int i = 0; i < size - 2; i++) {
		HSQOBJECT arg;
		sq_resetobject(&arg);
		if (SQ_FAILED(sq_getstackobj(v, 3 + i, &arg)))
			return sq_throwerror(v, "Couldn't get coroutine args from stack");
		t->_args.push_back(arg);
		sq_addref(vm, &arg);
	}

	// get the closure
	sq_resetobject(&t->_closureObj);
	if (SQ_FAILED(sq_getstackobj(v, 2, &t->_closureObj)))
		return sq_throwerror(v, "Couldn't get coroutine thread from stack");
	sq_addref(vm, &t->_closureObj);

	const SQChar *name = nullptr;
	if (SQ_SUCCEEDED(sq_getclosurename(v, 2)))
		sq_getstring(v, -1, &name);

	t->_name = Common::String::format("%s %s (%lld)", name == nullptr ? "<anonymous>" : name, _stringval(_closure(t->_closureObj)->_function->_sourcename), _closure(t->_closureObj)->_function->_lineinfos->_line);
	sq_pop(vm, 1);
	if (name)
		sq_pop(v, 1); // pop name
	sq_pop(v, 1);     // pop closure

	g_engine->_threads.push_back(t);

	debug("create thread %s", t->_name.c_str());

	// call the closure in the thread
	if (!t->call())
		return sq_throwerror(v, "call failed");

	sq_pushobject(v, t->_obj);
	return 1;
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

static SQInteger activeController(HSQUIRRELVM v) {
	warning("TODO: activeController: not implemented");
	// TODO: change harcoded mouse
	sq_pushinteger(v, 1);
	return 1;
}

static SQInteger addCallback(HSQUIRRELVM v) {
	warning("TODO: addCallback: not implemented");
	return 0;
}

// Registers a folder that assets can appear in.
//
// Only used for development builds where the assets are not bundled up.
// Use in the Boot.nut process.
// Not necessary for release.
static SQInteger addFolder(HSQUIRRELVM v) {
	return 0;
}

// When called in a function started with startthread, execution is suspended for count frames.
// It is an error to call breakhere in a function that was not started with startthread.
// Particularly useful instead of breaktime if you just want to wait 1 frame, since not all machines run at the same speed.
// . code-block:: Squirrel
// while(isSoundPlaying(soundPhoneBusy)) {
//   breakhere(5)
//}
static SQInteger breakhere(HSQUIRRELVM v) {
	warning("TODO: breakhere: not implemented");
	return 0;
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
		return breakfunc(v, [](Thread &t) { t._numFrames = 1; });
	else
		return breakfunc(v, [&](Thread &t) { t._waitTime = time; });
}

static SQInteger breakwhileanimating(HSQUIRRELVM v) {
	warning("TODO: breakwhileanimating: not implemented");
	return 0;
}

static SQInteger breakwhilecamera(HSQUIRRELVM v) {
	warning("TODO: breakwhilecamera: not implemented");
	return 0;
}

static SQInteger breakwhilecutscene(HSQUIRRELVM v) {
	warning("TODO: breakwhilecutscene: not implemented");
	return 0;
}

static SQInteger breakwhiledialog(HSQUIRRELVM v) {
	warning("TODO: breakwhiledialog: not implemented");
	return 0;
}

static SQInteger breakwhileinputoff(HSQUIRRELVM v) {
	warning("TODO: breakwhileinputoff: not implemented");
	return 0;
}
static SQInteger breakwhilerunning(HSQUIRRELVM v) {
	warning("TODO: breakwhilerunning: not implemented");
	return 0;
}
static SQInteger breakwhiletalking(HSQUIRRELVM v) {
	warning("TODO: breakwhiletalking: not implemented");
	return 0;
}
static SQInteger breakwhilewalking(HSQUIRRELVM v) {
	warning("TODO: breakwhilewalking: not implemented");
	return 0;
}
static SQInteger breakwhilesound(HSQUIRRELVM v) {
	warning("TODO: breakwhilesound: not implemented");
	return 0;
}

static SQInteger cutscene(HSQUIRRELVM v) {
	warning("TODO: cutscene: not implemented");
	return 0;
}

static SQInteger cutsceneOverride(HSQUIRRELVM v) {
	warning("TODO: cutsceneOverride: not implemented");
	return 0;
}

static SQInteger dumpvar(HSQUIRRELVM v) {
	warning("TODO: dumpvar: not implemented");
	return 0;
}

static SQInteger exCommand(HSQUIRRELVM v) {
	warning("TODO: exCommand: not implemented");
	return 0;
}

static SQInteger gameTime(HSQUIRRELVM v) {
	warning("TODO: gameTime: not implemented");
	return 0;
}

static SQInteger sysInclude(HSQUIRRELVM v) {
	warning("TODO: sysInclude: not implemented");
	return 0;
}

static SQInteger inputController(HSQUIRRELVM v) {
	warning("TODO: inputController: not implemented");
	return 0;
}

static SQInteger inputHUD(HSQUIRRELVM v) {
	warning("TODO: inputHUD: not implemented");
	return 0;
}

static SQInteger inputOff(HSQUIRRELVM v) {
	warning("TODO: inputOff: not implemented");
	return 0;
}

static SQInteger inputOn(HSQUIRRELVM v) {
	warning("TODO: inputOn: not implemented");
	return 0;
}

static SQInteger inputSilentOff(HSQUIRRELVM v) {
	warning("TODO: inputSilentOff: not implemented");
	return 0;
}

static SQInteger sysInputState(HSQUIRRELVM v) {
	warning("TODO: sysInputState: not implemented");
	return 0;
}

static SQInteger inputVerbs(HSQUIRRELVM v) {
	warning("TODO: inputVerbs: not implemented");
	return 0;
}

static SQInteger isInputOn(HSQUIRRELVM v) {
	warning("TODO: isInputOn: not implemented");
	return 0;
}

static SQInteger logEvent(HSQUIRRELVM v) {
	warning("TODO: logEvent: not implemented");
	return 0;
}

static SQInteger logInfo(HSQUIRRELVM v) {
	warning("TODO: logInfo: not implemented");
	return 0;
}

static SQInteger logWarning(HSQUIRRELVM v) {
	warning("TODO: logWarning: not implemented");
	return 0;
}

static SQInteger microTime(HSQUIRRELVM v) {
	warning("TODO: microTime: not implemented");
	return 0;
}

static SQInteger moveCursorTo(HSQUIRRELVM v) {
	warning("TODO: moveCursorTo: not implemented");
	return 0;
}

static SQInteger removeCallback(HSQUIRRELVM v) {
	warning("TODO: removeCallback: not implemented");
	return 0;
}

static SQInteger setAmbientLight(HSQUIRRELVM v) {
	warning("TODO: setAmbientLight: not implemented");
	return 0;
}

static SQInteger startthread(HSQUIRRELVM v) {
	return _startthread(v, false);
}

static SQInteger startglobalthread(HSQUIRRELVM v) {
	return _startthread(v, true);
}

static SQInteger stopthread(HSQUIRRELVM v) {
	warning("TODO: stopthread: not implemented");
	return 0;
}

static SQInteger threadid(HSQUIRRELVM v) {
	warning("TODO: threadid: not implemented");
	return 0;
}

static SQInteger threadpauseable(HSQUIRRELVM v) {
	warning("TODO: threadpauseable: not implemented");
	return 0;
}

void sqgame_register_syslib(HSQUIRRELVM v) {
	regFunc(v, startthread, _SC("startthread"));
	regFunc(v, startglobalthread, _SC("startglobalthread"));
	regFunc(v, breaktime, _SC("breaktime"));
	regFunc(v, activeController, _SC("activeController"));
	regFunc(v, addCallback, _SC("addCallback"));
	regFunc(v, addFolder, _SC("addFolder"));
	regFunc(v, breakhere, _SC("breakhere"));
	regFunc(v, breaktime, _SC("breaktime"));
	regFunc(v, breakwhileanimating, _SC("breakwhileanimating"));
	regFunc(v, breakwhilecamera, _SC("breakwhilecamera"));
	regFunc(v, breakwhilecutscene, _SC("breakwhilecutscene"));
	regFunc(v, breakwhiledialog, _SC("breakwhiledialog"));
	regFunc(v, breakwhileinputoff, _SC("breakwhileinputoff"));
	regFunc(v, breakwhilerunning, _SC("breakwhilerunning"));
	regFunc(v, breakwhilesound, _SC("breakwhilesound"));
	regFunc(v, breakwhiletalking, _SC("breakwhiletalking"));
	regFunc(v, breakwhilewalking, _SC("breakwhilewalking"));
	regFunc(v, cutscene, _SC("cutscene"));
	regFunc(v, cutsceneOverride, _SC("cutsceneOverride"));
	regFunc(v, dumpvar, _SC("dumpvar"));
	regFunc(v, exCommand, _SC("exCommand"));
	regFunc(v, gameTime, _SC("gameTime"));
	regFunc(v, sysInclude, _SC("include"));
	regFunc(v, inputController, _SC("inputController"));
	regFunc(v, inputHUD, _SC("inputHUD"));
	regFunc(v, inputOff, _SC("inputOff"));
	regFunc(v, inputOn, _SC("inputOn"));
	regFunc(v, inputSilentOff, _SC("inputSilentOff"));
	regFunc(v, sysInputState, _SC("inputState"));
	regFunc(v, inputVerbs, _SC("inputVerbs"));
	regFunc(v, isInputOn, _SC("isInputOn"));
	regFunc(v, logEvent, _SC("logEvent"));
	regFunc(v, logInfo, _SC("logInfo"));
	regFunc(v, logWarning, _SC("logWarning"));
	regFunc(v, microTime, _SC("microTime"));
	regFunc(v, moveCursorTo, _SC("moveCursorTo"));
	regFunc(v, removeCallback, _SC("removeCallback"));
	regFunc(v, setAmbientLight, _SC("setAmbientLight"));
	regFunc(v, startglobalthread, _SC("startglobalthread"));
	regFunc(v, startthread, _SC("startthread"));
	regFunc(v, stopthread, _SC("stopthread"));
	regFunc(v, threadid, _SC("threadid"));
	regFunc(v, threadpauseable, _SC("threadpauseable"));
}

} // namespace Twp
