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
#include "illusions/scriptthread.h"
#include "illusions/scriptopcodes.h"
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

int ActiveScenes::getActiveScenesCount() {
	return _stack.size();
}

void ActiveScenes::getActiveSceneInfo(uint index, uint32 *sceneId, int *pauseCtr) {
	if (sceneId)
		*sceneId = _stack[index]._sceneId;
	if (pauseCtr)
		*pauseCtr = _stack[index]._pauseCtr;
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

// ScriptMan

ScriptMan::ScriptMan(IllusionsEngine *vm)
	: _vm(vm), _pauseCtr(0), _doScriptThreadInit(false) {
	_threads = new ThreadList(vm);
	_scriptOpcodes = new ScriptOpcodes(vm);
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
	debug("Starting script thread %08X", threadId);
	byte *scriptCodeIp = _scriptResource->getThreadCode(threadId);
	newScriptThread(threadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
}

void ScriptMan::startAnonScriptThread(int32 threadId, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	debug("Starting anonymous script thread %08X", threadId);
	uint32 tempThreadId = newTempThreadId();
	byte *scriptCodeIp = _scriptResource->getThreadCode(threadId);
	scriptCodeIp = _scriptResource->getThreadCode(threadId);
	newScriptThread(tempThreadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
}

uint32 ScriptMan::startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
	uint32 value8, uint32 valueC, uint32 value10) {
	debug("Starting temp script thread");
	uint32 tempThreadId = newTempThreadId();
	newScriptThread(tempThreadId, callingThreadId, 0, scriptCodeIp, value8, valueC, value10);
	return tempThreadId;
}

uint32 ScriptMan::startAbortableTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, true);
}

uint32 ScriptMan::startTimerThread(uint32 duration, uint32 threadId) {
	return newTimerThread(duration, threadId, false);
}

void ScriptMan::setCurrFontId(uint32 fontId) {
	_fontId = fontId;
}

bool ScriptMan::enterScene(uint32 sceneId, uint32 threadId) {
	ProgInfo *progInfo = _scriptResource->getProgInfo(sceneId & 0xFFFF);
	if (!progInfo) {
		// TODO dumpActiveScenes(_someSceneId2, threadId);
		sceneId = _theSceneId;
	}
	_activeScenes.push(sceneId);
	return progInfo != 0;
}

void ScriptMan::newScriptThread(uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10) {
	ScriptThread *scriptThread = new ScriptThread(_vm, threadId, callingThreadId, notifyFlags,
		scriptCodeIp, value8, valueC, value10);
	_threads->startThread(scriptThread);
	if (_pauseCtr > 0)
		scriptThread->pause();
	if (_doScriptThreadInit) {
		int updateResult = 4;
		while (scriptThread->_pauseCtr <= 0 && updateResult != 1 && updateResult != 2)
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
