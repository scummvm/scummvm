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
#include "twp/thread.h"

namespace Twp{

Thread::Thread() : paused(false), waitTime(0), numFrames(0), stopRequest(false) {
}

Thread::~Thread() {
	HSQUIRRELVM v = g_engine->getVm();
	for (int i = 0; i < args.size(); i++) {
		sq_release(v, &args[i]);
	}
	sq_release(v, &threadObj);
	sq_release(v, &envObj);
	sq_release(v, &closureObj);
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

}
