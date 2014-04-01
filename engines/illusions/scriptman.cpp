/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/scriptman.h"
#include "illusions/abortablethread.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/scriptthread.h"
#include "illusions/scriptopcodes.h"
#include "illusions/talkthread.h"
#include "illusions/timerthread.h"

namespace Illusions {

// ActiveScenes

ActiveScenes::ActiveScenes() {
	clear();
}

void ActiveScenes::clear() {
	_stack.clear();
}

void ActiveScenes::push(uint32 sceneId) {
	ActiveScene activeScene;
	activeScene._sceneId = sceneId;
	activeScene._pauseCtr = 0;
	_stack.push(activeScene);
}

void ActiveScenes::pop() {
	_stack.pop();
}

void ActiveScenes::pauseActiveScene() {
	++_stack.top()._pauseCtr;
}

void ActiveScenes::unpauseActiveScene() {
	--_stack.top()._pauseCtr;
}

uint ActiveScenes::getActiveScenesCount() {
	return _stack.size();
}

void ActiveScenes::getActiveSceneInfo(uint index, uint32 *sceneId, int *pauseCtr) {
	if (sceneId)
		*sceneId = _stack[index - 1]._sceneId;
	if (pauseCtr)
		*pauseCtr = _stack[index - 1]._pauseCtr;
}

uint32 ActiveScenes::getCurrentScene() {
	if (_stack.size() > 0)
		return _stack.top()._sceneId;
	return 0;
}

bool ActiveScenes::isSceneActive(uint32 sceneId) {
	for (uint i = 0; i < _stack.size(); ++i)
		if (_stack[i]._sceneId == sceneId && _stack[i]._pauseCtr <= 0)
			return true;
	return false;
}

// TriggerFunction

TriggerFunction::TriggerFunction(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback)
	: _sceneId(sceneId), _verbId(verbId), _objectId2(objectId2), _objectId(objectId), _callback(callback) {
}

TriggerFunction::~TriggerFunction() {
	delete _callback;
}

void TriggerFunction::run(uint32 callingThreadId) {
	(*_callback)(this, callingThreadId);
}

// TriggerFunctions

void TriggerFunctions::add(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, TriggerFunctionCallback *callback) {
	ItemsIterator it = findInternal(sceneId, verbId, objectId2, objectId);
	if (it != _triggerFunctions.end()) {
		delete *it;
		_triggerFunctions.erase(it);
	}
	_triggerFunctions.push_back(new TriggerFunction(sceneId, verbId, objectId2, objectId, callback));
}

TriggerFunction *TriggerFunctions::find(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	ItemsIterator it = findInternal(sceneId, verbId, objectId2, objectId);
	if (it != _triggerFunctions.end())
		return (*it);
	return 0;
}

void TriggerFunctions::removeBySceneId(uint32 sceneId) {
	ItemsIterator it = _triggerFunctions.begin();
	while (it != _triggerFunctions.end()) {
		if ((*it)->_sceneId == sceneId) {
			delete *it;
			it = _triggerFunctions.erase(it);
		} else
			++it;
	}
}

TriggerFunctions::ItemsIterator TriggerFunctions::findInternal(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	ItemsIterator it = _triggerFunctions.begin();
	for (; it != _triggerFunctions.end(); ++it) {
		TriggerFunction *triggerFunction = *it;
		if (triggerFunction->_sceneId == sceneId && triggerFunction->_verbId == verbId &&
			triggerFunction->_objectId2 == objectId2 && triggerFunction->_objectId == objectId)
			break;
	}
	return it;		
}

// ScriptStack

ScriptStack::ScriptStack() {
	clear();
}

void ScriptStack::clear() {
	for (uint i = 0; i < 256; ++i)
		_stack[i] = (int16)0xEEEE;
	_stackPos = 256;
}

void ScriptStack::push(int16 value) {
	--_stackPos;
	if (_stackPos > 0)
		_stack[_stackPos] = value;
}

int16 ScriptStack::pop() {
	int16 value = 0;
	if (_stackPos < 256) {
		value = _stack[_stackPos];
		_stack[_stackPos] = (int16)0xEEEE;
		++_stackPos;
	}
	return value;
}

int16 ScriptStack::peek() {
	int16 value = 0;
	if (_stackPos < 256)
		value = _stack[_stackPos];
	return value;
}

int16 *ScriptStack::topPtr() {
	return &_stack[_stackPos];
}

// ScriptMan

ScriptMan::ScriptMan(IllusionsEngine *vm)
	: _vm(vm), _pauseCtr(0), _doScriptThreadInit(false) {
	_threads = new ThreadList(vm);
	_scriptOpcodes = new ScriptOpcodes(vm);
	_field8 = 1;
	_fieldA = 0;
	_fieldE = 240;
}

ScriptMan::~ScriptMan() {
	delete _threads;
	delete _scriptOpcodes;
}

void ScriptMan::setSceneIdThreadId(uint32 theSceneId, uint32 theThreadId) {
	_theSceneId = theSceneId;
	_theThreadId = theThreadId;
}

void ScriptMan::startScriptThread(uint32 threadId, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	debug(2, "Starting script thread %08X", threadId);
	byte *scriptCodeIp = _scriptResource->getThreadCode(threadId);
	newScriptThread(threadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
}

void ScriptMan::startAnonScriptThread(int32 threadId, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	debug(2, "Starting anonymous script thread %08X", threadId);
	uint32 tempThreadId = newTempThreadId();
	byte *scriptCodeIp = _scriptResource->getThreadCode(threadId);
	scriptCodeIp = _scriptResource->getThreadCode(threadId);
	newScriptThread(tempThreadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
}

uint32 ScriptMan::startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	uint32 tempThreadId = newTempThreadId();
	debug(2, "Starting temp script thread %08X", tempThreadId);
	newScriptThread(tempThreadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
	return tempThreadId;
}

uint32 ScriptMan::startAbortableTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, true);
}

uint32 ScriptMan::startTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, false);
}

uint32 ScriptMan::startAbortableThread(byte *scriptCodeIp1, byte *scriptCodeIp2, uint32 callingThreadId) {
	uint32 tempThreadId = newTempThreadId();
	debug(2, "Starting abortable thread %08X", tempThreadId);
	uint32 scriptThreadId = startTempScriptThread(scriptCodeIp1, tempThreadId, 0, 0, 0);
	AbortableThread *abortableThread = new AbortableThread(_vm, tempThreadId, callingThreadId, 0,
		scriptThreadId, scriptCodeIp2);
	_threads->startThread(abortableThread);
	return tempThreadId;
}

uint32 ScriptMan::startTalkThread(int16 duration, uint32 objectId, uint32 talkId, uint32 sequenceId1,
	uint32 sequenceId2, uint32 namedPointId, uint32 callingThreadId) {
	debug(2, "Starting talk thread");
	uint32 tempThreadId = newTempThreadId();
	_threads->endTalkThreadsNoNotify();
	TalkThread *talkThread = new TalkThread(_vm, tempThreadId, callingThreadId, 0,
		duration, objectId, talkId, sequenceId1, sequenceId2, namedPointId);
	_threads->startThread(talkThread);
	return tempThreadId;
}

bool ScriptMan::findTriggerCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(sceneId & 0xFFFF);
	if (progInfo)
		return progInfo->findTriggerCause(verbId, objectId2, objectId, codeOffs);
	return false;
}

void ScriptMan::setCurrFontId(uint32 fontId) {
	_fontId = fontId;
}

bool ScriptMan::checkActiveTalkThreads() {
	return _threads->isActiveThread(kMsgQueryTalkThreadActive);
}

uint32 ScriptMan::clipTextDuration(uint32 duration) {
	switch (_field8) {
	case 2:
		if (duration == 0)
			duration = 240;
		break;
	case 3:
		if (duration < _fieldA)
			duration = _fieldA;
		break;
	case 4:
		if (duration > _fieldA)
			duration = _fieldA;
		break;
	}
	return duration;
}

void ScriptMan::reset() {
	_scriptResource->_blockCounters.clear();
	_scriptResource->_properties.clear();
	// TODO script_sub_417FF0(1, 0);
}

bool ScriptMan::enterScene(uint32 sceneId, uint32 threadId) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(sceneId & 0xFFFF);
	if (!progInfo) {
		// TODO dumpActiveScenes(_globalSceneId, threadId);
		sceneId = _theSceneId;
	}
	_activeScenes.push(sceneId);
	return progInfo != 0;
}

void ScriptMan::exitScene(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	// TODO krnfileDump(sceneId);
	// TODO UpdateFunctions_disableByTag__TODO_maybe(sceneId);
	_threads->terminateThreadsByTag(sceneId, threadId);
	_vm->_controls->destroyControlsByTag(sceneId);
	_vm->_triggerFunctions->removeBySceneId(sceneId);
	_vm->_resSys->unloadResourcesByTag(sceneId);
	_activeScenes.pop();
}

void ScriptMan::enterPause(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	_vm->_camera->pushCameraMode();
	_threads->suspendThreadsByTag(sceneId, threadId);
	_vm->_controls->pauseControlsByTag(sceneId);
	_vm->_actorItems->pauseByTag(sceneId);
	_vm->_backgroundItems->pauseByTag(sceneId);
	_activeScenes.pauseActiveScene();
}

void ScriptMan::leavePause(uint32 threadId) {
	uint32 sceneId = _activeScenes.getCurrentScene();
	_vm->_backgroundItems->unpauseByTag(sceneId);
	_vm->_actorItems->unpauseByTag(sceneId);
	_vm->_controls->unpauseControlsByTag(sceneId);
	_threads->notifyThreadsByTag(sceneId, threadId);
	_vm->_camera->popCameraMode();
	_activeScenes.unpauseActiveScene();
}

void ScriptMan::dumpActiveScenes(uint32 sceneId, uint32 threadId) {
	uint activeScenesCount = _activeScenes.getActiveScenesCount();
	while (activeScenesCount > 0) {
		uint32 activeSceneId;
		_activeScenes.getActiveSceneInfo(activeScenesCount, &activeSceneId, 0);
		if (activeSceneId == sceneId)
			break;
		exitScene(threadId);
		--activeScenesCount;
	}
	_vm->_camera->clearCameraModeStack();
}

void ScriptMan::newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10) {
	ScriptThread *scriptThread = new ScriptThread(_vm, threadId, callingThreadId, notifyFlags,
		scriptCodeIp, value8, valueC, value10);
	_threads->startThread(scriptThread);
	if (_pauseCtr > 0)
		scriptThread->pause();
	if (_doScriptThreadInit) {
		int updateResult = kTSRun;
		while (scriptThread->_pauseCtr <= 0 && updateResult != kTSTerminate && updateResult != kTSYield)
			updateResult = scriptThread->update();
	}
}

uint32 ScriptMan::newTimerThread(uint32 duration, uint32 callingThreadId, bool isAbortable) {
	uint32 tempThreadId = newTempThreadId();
	TimerThread *timerThread = new TimerThread(_vm, tempThreadId, callingThreadId, 0,
		duration, isAbortable);
	_threads->startThread(timerThread);
	return tempThreadId;
}

uint32 ScriptMan::newTempThreadId() {
	uint32 threadId = _nextTempThreadId + 2 * _scriptResource->_codeCount;
	if (threadId > 65535) {
		_nextTempThreadId = 0;
		threadId = 2 * _scriptResource->_codeCount;
	}
	++_nextTempThreadId;
	return 0x00020000 | threadId;
}

} // End of namespace Illusions
