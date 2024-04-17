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
#include "twp/detection.h"
#include "twp/ids.h"
#include "twp/resmanager.h"
#include "twp/squtil.h"
#include "twp/thread.h"

namespace Twp {

Thread::Thread(const Common::String &name, bool global, HSQOBJECT threadObj, HSQOBJECT envObj, HSQOBJECT closureObj, const Common::Array<HSQOBJECT> args) {
	_id = g_twp->_resManager->newThreadId();
	_name = name;
	_global = global;
	_threadObj = threadObj;
	_envObj = envObj;
	_closureObj = closureObj;
	_args = args;
	_pauseable = true;

	HSQUIRRELVM v = g_twp->getVm();
	for (auto &_arg : _args) {
		sq_addref(v, &_arg);
	}
	sq_addref(v, &_threadObj);
	sq_addref(v, &_envObj);
	sq_addref(v, &_closureObj);
}

Thread::~Thread() {
	debugC(kDebugGame, "delete thread %d, %s, global: %s", _id, _name.c_str(), _global ? "yes" : "no");
	HSQUIRRELVM v = g_twp->getVm();
	for (auto &_arg : _args) {
		sq_release(v, &_arg);
	}
	sq_release(v, &_threadObj);
	sq_release(v, &_envObj);
	sq_release(v, &_closureObj);
}

void Thread::pause() {
	if (_pauseable) {
		_paused = true;
		suspend();
	}
}

void Thread::unpause() {
	_paused = false;
	resume();
}

bool Thread::isDead() const {
	SQInteger state = sq_getvmstate(getThread());
	return _stopRequest || state == SQ_VMSTATE_IDLE;
}

bool Thread::isSuspended() const {
	SQInteger state = sq_getvmstate(getThread());
	return state != SQ_VMSTATE_RUNNING;
}

void Thread::suspend() {
	if (_pauseable && !isSuspended()) {
		sq_suspendvm(getThread());
	}
}

void Thread::resume() {
	if (!isDead() && isSuspended()) {
		sq_wakeupvm(getThread(), SQFalse, SQFalse, SQTrue, SQFalse);
	}
}

bool Thread::call() {
	HSQUIRRELVM v = _threadObj._unVal.pThread;
	// call the closure in the thread
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, _closureObj);
	sq_pushobject(v, _envObj);
	for (auto _arg : _args) {
		sq_pushobject(v, _arg);
	}
	if (SQ_FAILED(sq_call(v, 1 + _args.size(), SQFalse, SQTrue))) {
		sq_settop(v, top);
		return false;
	}
	return true;
}

bool Thread::update(float elapsed) {
	uint32 startTime = g_system->getMillis();
	if (_paused) {
	} else if (_waitTime > 0) {
		_waitTime -= elapsed;
		if (_waitTime <= 0) {
			_waitTime = 0;
			resume();
		}
	} else if (_numFrames > 0) {
		_numFrames--;
		if (_numFrames <= 0) {
			_numFrames = 0;
			resume();
		}
	}
	_lastUpdateTime = g_system->getMillis() - startTime;
	return isDead();
}

void Thread::stop() {
	_stopRequest = true;
	suspend();
}

} // namespace Twp
