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

namespace Twp {

Thread::Thread() {}

Thread::~Thread() {
	HSQUIRRELVM v = g_engine->getVm();
	for (int i = 0; i < _args.size(); i++) {
		sq_release(v, &_args[i]);
	}
	sq_release(v, &_threadObj);
	sq_release(v, &_envObj);
	sq_release(v, &_closureObj);
}

bool Thread::call() {
	HSQUIRRELVM v = _threadObj._unVal.pThread;
	// call the closure in the thread
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, _closureObj);
	sq_pushobject(v, _envObj);
	for (int i = 0; i < _args.size(); i++) {
		sq_pushobject(v, _args[i]);
	}
	if (SQ_FAILED(sq_call(v, 1 + _args.size(), SQFalse, SQTrue))) {
		sq_settop(v, top);
		return false;
	}
	return true;
}

bool Thread::isDead() {
	SQInteger state = sq_getvmstate(_threadObj._unVal.pThread);
	return _stopRequest || state == 0;
}

bool Thread::isSuspended() {
	SQInteger state = sq_getvmstate(_threadObj._unVal.pThread);
	return state != 1;
}

void Thread::suspend() {
	// TODO: pauseable
	if (!isSuspended()) {
		sq_suspendvm(_threadObj._unVal.pThread);
	}
}

void Thread::resume() {
	if (!isDead() && isSuspended()) {
		sq_wakeupvm(_threadObj._unVal.pThread, SQFalse, SQFalse, SQTrue, SQFalse);
	}
}

bool Thread::update(float elapsed) {
	if (_paused) {
	} else if (_waitTime > 0) {
		_waitTime -= elapsed;
		if (_waitTime <= 0) {
			_waitTime = 0;
			resume();
		}
	} else if (_numFrames > 0) {
		_numFrames -= 1;
		_numFrames = 0;
		resume();
	}
	return isDead();
}

void Thread::stop() {
	_stopRequest = true;
	suspend();
}

} // namespace Twp
