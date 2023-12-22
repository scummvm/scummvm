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

#include "twp/ids.h"
#include "twp/twp.h"
#include "twp/thread.h"
#include "twp/squtil.h"

namespace Twp {

bool ThreadBase::isDead() {
	SQInteger state = sq_getvmstate(getThread());
	return _stopRequest || state == 0;
}

bool ThreadBase::isSuspended() {
	SQInteger state = sq_getvmstate(getThread());
	return state != 1;
}

void ThreadBase::suspend() {
	if (_pauseable && !isSuspended()) {
		sq_suspendvm(getThread());
	}
}

void ThreadBase::resume() {
	if (!isDead() && isSuspended()) {
		sq_wakeupvm(getThread(), SQFalse, SQFalse, SQTrue, SQFalse);
	}
}

Thread::Thread(int id) {
	_id = id;
}

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

Cutscene::Cutscene(HSQUIRRELVM v, HSQOBJECT threadObj, HSQOBJECT closure, HSQOBJECT closureOverride, HSQOBJECT envObj)
	: _name("cutscene"),
	  _v(v),
	  _threadObj(threadObj),
	  _closure(closure),
	  _closureOverride(closureOverride),
	  _envObj(envObj) {

	_pauseable = false;
	_id = newThreadId();
	//_inputState = g_engine->inputState.getState();
	_actor = g_engine->_followActor;
	//_showCursor = g_engine->inputState.showCursor;
	_state = csStart;
	debug("Create cutscene %d with input: 0x%X", _id, _inputState);
	// TODO:
	//   g_engine->_inputState.inputActive = false;
	//   g_engine->_inputState.showCursor = false;
	for (int i = 0; i < g_engine->_threads.size(); i++) {
		ThreadBase *thread = g_engine->_threads[i];
		if (thread->isGlobal())
			thread->pause();
	}
	HSQUIRRELVM vm = g_engine->getVm();
	sq_addref(vm, &_threadObj);
	sq_addref(vm, &_closure);
	sq_addref(vm, &_closureOverride);
	sq_addref(vm, &_envObj);
}

Cutscene::~Cutscene() {
	debug("destroy cutscene %d", _id);
	HSQUIRRELVM vm = g_engine->getVm();
	sq_release(vm, &_threadObj);
	sq_release(vm, &_closure);
	sq_release(vm, &_closureOverride);
	sq_release(vm, &_envObj);
}

void Cutscene::start() {
	_state = csCheckEnd;
	HSQUIRRELVM thread = getThread();
	// call the closure in the thread
	SQInteger top = sq_gettop(thread);
	sq_pushobject(thread, _closure);
	sq_pushobject(thread, _envObj);
	if (SQ_FAILED(sq_call(thread, 1, SQFalse, SQTrue))) {
		sq_settop(thread, top);
		error("Couldn't call cutscene");
	}
}

void Cutscene::stop() {
	_state = csQuit;
	debug("End cutscene");
	// g_engine->inputState.setState(self.inputState);
	// g_engine->inputState.showCursor = self.showCursor;
	// if self.showCursor:
	// 	g_engine->inputState.inputActive = true
	debug("Restore cutscene input: %X", _inputState);
	g_engine->follow(g_engine->_actor);
	for (int i = 0; i < g_engine->_threads.size(); i++) {
		ThreadBase *thread = g_engine->_threads[i];
		if (thread->isGlobal())
			thread->unpause();
	}
	sqcall("onCutsceneEnded");
	sq_wakeupvm(_v, SQFalse, SQFalse, SQTrue, SQFalse);
	sq_suspendvm(getThread());
}

void Cutscene::checkEndCutsceneOverride() {
	if (isStopped()) {
		_state = csEnd;
		debug("end checkEndCutsceneOverride");
	}
}

bool Cutscene::update(float elapsed) {
	if (_waitTime > 0)
		_waitTime -= elapsed;
	if (_waitTime <= 0) {
		_waitTime = 0;
		resume();
	} else if (_numFrames > 0) {
		_numFrames -= 1;
		_numFrames = 0;
		resume();
	}

	switch (_state) {
	case csStart:
		debug("startCutscene");
		start();
		return false;
	case csCheckEnd:
		checkEndCutscene();
		return false;
	case csOverride:
		debug("doCutsceneOverride");
		doCutsceneOverride();
		return false;
	case csCheckOverride:
		debug("checkEndCutsceneOverride");
		checkEndCutsceneOverride();
		return false;
	case csEnd:
		debug("endCutscene");
		stop();
		return false;
	case csQuit:
		return true;
	}
}

bool Cutscene::hasOverride() const {
	return !sq_isnull(_closureOverride);
}

void Cutscene::doCutsceneOverride() {
	if (hasOverride()) {
		_state = csCheckOverride;
		debug("start cutsceneOverride");
		sq_pushobject(getThread(), _closureOverride);
		sq_pushobject(getThread(), _envObj);
		if (SQ_FAILED(sq_call(getThread(), 1, SQFalse, SQTrue)))
			error("Couldn't call cutsceneOverride");
		return;
	}
	_state = csEnd;
}

void Cutscene::checkEndCutscene() {
	if (isStopped()) {
		_state = csEnd;
		debug("end cutscene: %d", getId());
	}
}

bool Cutscene::isStopped() {
	if (_stopped || (_state == csQuit))
		return true;
	return sq_getvmstate(getThread()) == 0;
}

} // namespace Twp
