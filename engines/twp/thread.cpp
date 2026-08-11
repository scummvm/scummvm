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

bool ThreadBase::isDead() {
	SQInteger state = sq_getvmstate(getThread());
	return _stopRequest || state == SQ_VMSTATE_IDLE;
}

bool ThreadBase::isSuspended() {
	SQInteger state = sq_getvmstate(getThread());
	return state != SQ_VMSTATE_RUNNING;
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

Cutscene::Cutscene(const Common::String &name, int parentThreadId, HSQOBJECT threadObj, HSQOBJECT closure, HSQOBJECT closureOverride, HSQOBJECT envObj)
	: _parentThreadId(parentThreadId),
	  _threadObj(threadObj),
	  _closure(closure),
	  _closureOverride(closureOverride),
	  _envObj(envObj) {

	_name = name;
	_id = g_twp->_resManager->newThreadId();
	_inputState = g_twp->_inputState.getState();
	_actor = g_twp->_followActor;
	_showCursor = g_twp->_inputState.getShowCursor();
	_state = csStart;
	debugC(kDebugGame, "Create cutscene %d with input: 0x%X from parent thread: %d", _id, _inputState, _parentThreadId);
	g_twp->_inputState.setInputActive(false);
	g_twp->_inputState.setShowCursor(false);
	for (auto thread : g_twp->_threads) {
		if (thread->isGlobal())
			thread->pause();
	}
	HSQUIRRELVM vm = g_twp->getVm();
	sq_addref(vm, &_threadObj);
	sq_addref(vm, &_closure);
	sq_addref(vm, &_closureOverride);
	sq_addref(vm, &_envObj);
}

Cutscene::~Cutscene() {
	debugC(kDebugGame, "destroy cutscene %d", _id);
	HSQUIRRELVM vm = g_twp->getVm();
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
	debugC(kDebugGame, "End cutscene: %d", getId());
	g_twp->_inputState.setState(_inputState);
	g_twp->_inputState.setShowCursor(_showCursor);
	if (_showCursor)
		g_twp->_inputState.setInputActive(true);
	debugC(kDebugGame, "Restore cutscene input: %X", _inputState);
	g_twp->follow(g_twp->_actor);
	Common::Array<Common::SharedPtr<ThreadBase> > threads(g_twp->_threads);
	for (auto thread : threads) {
		if (thread->isGlobal())
			thread->unpause();
	}
	sqcall("onCutsceneEnded");

	Common::SharedPtr<ThreadBase> t = sqthread(_parentThreadId);
	if (t && t->getId())
		t->unpause();
	HSQUIRRELVM thread = getThread();
	if (thread)
		sq_suspendvm(thread);
}

HSQUIRRELVM Cutscene::getThread() {
	if (_threadObj._type != OT_THREAD)
		return nullptr;
	return _threadObj._unVal.pThread;
}

void Cutscene::checkEndCutsceneOverride() {
	if (isStopped()) {
		_state = csEnd;
		debugC(kDebugGame, "end checkEndCutsceneOverride");
	}
}

bool Cutscene::update(float elapsed) {
	if (_waitTime > 0) {
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

	switch (_state) {
	case csStart:
		debugC(kDebugGame, "startCutscene");
		start();
		return false;
	case csCheckEnd:
		checkEndCutscene();
		return false;
	case csOverride:
		debugC(kDebugGame, "doCutsceneOverride");
		doCutsceneOverride();
		return false;
	case csCheckOverride:
		debugC(kDebugGame, "checkEndCutsceneOverride");
		checkEndCutsceneOverride();
		return false;
	case csEnd:
		stop();
		return false;
	case csQuit:
		return true;
	}

	return false;
}

bool Cutscene::hasOverride() const {
	return !sq_isnull(_closureOverride);
}

void Cutscene::doCutsceneOverride() {
	if (hasOverride()) {
		// first quit current thread
		HSQUIRRELVM vm = g_twp->getVm();
		sq_release(vm, &_threadObj);

		// create thread and store it on the stack
		sq_newthread(vm, 1024);
		sq_resetobject(&_threadObj);
		if (SQ_FAILED(sq_getstackobj(vm, -1, &_threadObj))) {
			error("failed to get coroutine thread from stack");
		}
		sq_addref(vm, &_threadObj);

		_state = csCheckOverride;
		debugC(kDebugGame, "start cutsceneOverride");
		sq_pushobject(getThread(), _closureOverride);
		sq_pushobject(getThread(), _envObj);
		if (SQ_FAILED(sq_call(getThread(), 1, SQFalse, SQTrue))) {
			error("Couldn't call cutsceneOverride");
		}
		return;
	}
	_state = csEnd;
}

void Cutscene::checkEndCutscene() {
	if (isStopped()) {
		_state = csEnd;
	}
}

bool Cutscene::isStopped() {
	if (_stopped || (_state == csQuit))
		return true;
	return sq_getvmstate(getThread()) == 0;
}

void Cutscene::cutsceneOverride() {
	if (_state == csCheckEnd) {
		_state = csOverride;
	}
}

// CutsceneOverride::CutsceneOverride(const Common::String &name, int parentThreadId, HSQOBJECT threadObj, HSQOBJECT closureOverride, HSQOBJECT envObj)
// 	: _parentThreadId(parentThreadId),
// 	  _threadObj(threadObj),
// 	//   _closure(closure),
// 	  _closureOverride(closureOverride),
// 	  _envObj(envObj) {

// 	_name = name;
// 	_id = g_twp->_resManager->newThreadId();
// 	_inputState = g_twp->_inputState.getState();
// 	_actor = g_twp->_followActor;
// 	_showCursor = g_twp->_inputState.getShowCursor();
// 	_state = csStart;
// 	debugC(kDebugGame, "Create cutscene %d with input: 0x%X from parent thread: %d", _id, _inputState, _parentThreadId);
// 	g_twp->_inputState.setInputActive(false);
// 	g_twp->_inputState.setShowCursor(false);
// 	for (auto thread : g_twp->_threads) {
// 		if (thread->isGlobal())
// 			thread->pause();
// 	}
// 	HSQUIRRELVM vm = g_twp->getVm();
// 	sq_addref(vm, &_threadObj);
// 	sq_addref(vm, &_closure);
// 	sq_addref(vm, &_closureOverride);
// 	sq_addref(vm, &_envObj);
// }

// CutsceneOverride::~CutsceneOverride() {
// 	debugC(kDebugGame, "destroy cutscene override %d", _id);
// 	HSQUIRRELVM vm = g_twp->getVm();
// 	sq_release(vm, &_threadObj);
// 	sq_release(vm, &_closure);
// 	sq_release(vm, &_closureOverride);
// 	sq_release(vm, &_envObj);
// }

// bool CutsceneOverride::update(float elapsed) {
// 	switch (_state) {
// 	case csStart:
// 		debugC(kDebugGame, "start cutsceneOverride");
// 		sq_pushobject(getThread(), _closureOverride);
// 		sq_pushobject(getThread(), _envObj);
// 		if (SQ_FAILED(sq_call(getThread(), 1, SQFalse, SQTrue)))
// 			error("Couldn't call cutsceneOverride");
// 			break;
// 	case csCheckOverride:
// 		debugC(kDebugGame, "checkEndCutsceneOverride");
// 		checkEndCutsceneOverride();
// 		return false;
// 	case csEnd:
// 		stop();
// 		return false;
// 	case csQuit:
// 		return true;
// 	}
// }

// void CutsceneOverride::checkEndCutsceneOverride() {
// 	if (isStopped()) {
// 		_state = csEnd;
// 		debugC(kDebugGame, "end checkEndCutsceneOverride");
// 	}
// }

} // namespace Twp
