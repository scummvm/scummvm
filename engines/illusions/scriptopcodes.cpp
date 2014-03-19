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
#include "illusions/scriptopcodes.h"
#include "illusions/actor.h"
#include "illusions/input.h"
#include "illusions/screen.h"
#include "illusions/scriptman.h"
#include "illusions/scriptresource.h"
#include "illusions/scriptthread.h"

namespace Illusions {

// OpCall

void OpCall::skip(uint size) {
	_code += size;
}

byte OpCall::readByte() {
	return *_code++;
}

int16 OpCall::readSint16() {
	int16 value = READ_LE_UINT16(_code);
	_code += 2;
	return value;
}

uint32 OpCall::readUint32() {
	uint32 value = READ_LE_UINT32(_code);
	_code += 4;
	return value;
}

// ScriptOpcodes

ScriptOpcodes::ScriptOpcodes(IllusionsEngine *vm)
	: _vm(vm) {
	initOpcodes();
}

ScriptOpcodes::~ScriptOpcodes() {
	freeOpcodes();
}

void ScriptOpcodes::execOpcode(ScriptThread *scriptThread, OpCall &opCall) {
	if (!_opcodes[opCall._op])
		error("ScriptOpcodes::execOpcode() Unimplemented opcode %d", opCall._op);
	debug("execOpcode(%d)", opCall._op);
	(*_opcodes[opCall._op])(scriptThread, opCall);
}

typedef Common::Functor2Mem<ScriptThread*, OpCall&, void, ScriptOpcodes> ScriptOpcodeI;
#define OPCODE(op, func) _opcodes[op] = new ScriptOpcodeI(this, &ScriptOpcodes::func);

void ScriptOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < 256; ++i)
		_opcodes[i] = 0;
	// Register opcodes
	OPCODE(2, opSuspend);
	OPCODE(3, opYield);
	OPCODE(6, opStartScriptThread);
	OPCODE(9, opStartTimerThread);
	OPCODE(16, opLoadResource);
	OPCODE(20, opEnterScene);
	OPCODE(39, opSetDisplay);
	OPCODE(42, opIncBlockCounter);
	OPCODE(46, opPlaceActor);
	OPCODE(87, opDeactivateButton);
	OPCODE(88, opActivateButton);
	OPCODE(126, opDebug126);
	OPCODE(144, opPlayVideo);
	OPCODE(175, opSetSceneIdThreadId);
	OPCODE(177, opSetFontId);
}

#undef OPCODE

void ScriptOpcodes::freeOpcodes() {
	for (uint i = 0; i < 256; ++i)
		delete _opcodes[i];
}

// Opcodes

// Convenience macros
#define	ARG_SKIP(x) opCall.skip(x); 
#define ARG_INT16(name) int16 name = opCall.readSint16(); debug("ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = opCall.readUint32(); debug("ARG_UINT32(" #name " = %08X)", name);

void ScriptOpcodes::opSuspend(ScriptThread *scriptThread, OpCall &opCall) {
	opCall._result = kTSSuspend;
}

void ScriptOpcodes::opYield(ScriptThread *scriptThread, OpCall &opCall) {
	opCall._result = kTSYield;
}

void ScriptOpcodes::opStartScriptThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->_scriptMan->startScriptThread(threadId, opCall._threadId,
		scriptThread->_value8, scriptThread->_valueC, scriptThread->_value10);
}

void ScriptOpcodes::opStartTimerThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(isAbortable);
	ARG_INT16(duration);
	ARG_INT16(maxDuration);
	if (maxDuration)
		duration += _vm->getRandom(maxDuration);
	if (isAbortable)
		_vm->_scriptMan->startAbortableTimerThread(duration, opCall._threadId);
	else
		_vm->_scriptMan->startTimerThread(duration, opCall._threadId);
}

void ScriptOpcodes::opLoadResource(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(resourceId);
	// NOTE Skipped checking for stalled resources
	uint32 sceneId = _vm->_scriptMan->_activeScenes.getCurrentScene();
	_vm->_resSys->loadResource(resourceId, sceneId, opCall._threadId);
}

void ScriptOpcodes::opEnterScene(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	uint scenesCount = _vm->_scriptMan->_activeScenes.getActiveScenesCount();
	if (scenesCount > 0) {
		uint32 currSceneId;
		_vm->_scriptMan->_activeScenes.getActiveSceneInfo(scenesCount - 1, &currSceneId, 0);
		// TODO krnfileDump(currSceneId);
	}
	if (!_vm->_scriptMan->enterScene(sceneId, opCall._threadId))
		opCall._result = kTSTerminate;
}

void ScriptOpcodes::opSetDisplay(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(flag);
	_vm->_screen->setDisplayOn(flag != 0);
}

void ScriptOpcodes::opIncBlockCounter(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index)	
	byte value = _vm->_scriptMan->_scriptResource->_blockCounters.get(index + 1);
	if (value <= 63)
		_vm->_scriptMan->_scriptResource->_blockCounters.set(index + 1, value);
}

void ScriptOpcodes::opPlaceActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(actorTypeId);
	ARG_UINT32(sequenceId);
	ARG_UINT32(namedPointId);
	Common::Point pos = _vm->getNamedPointPosition(namedPointId);
	_vm->_controls->placeActor(actorTypeId, pos, sequenceId, objectId, opCall._threadId);
}

void ScriptOpcodes::opDeactivateButton(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(button)
	_vm->_input->deactivateButton(button);
}

void ScriptOpcodes::opActivateButton(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(button)
	_vm->_input->activateButton(button);
}

void ScriptOpcodes::opDebug126(ScriptThread *scriptThread, OpCall &opCall) {
	// NOTE Prints some debug text
}

void ScriptOpcodes::opPlayVideo(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(videoId);
	ARG_UINT32(priority);
	// TODO _vm->playVideo(videoId, objectId, value, opCall._threadId);
	
	//DEBUG Resume calling thread, later done by the video player
	_vm->notifyThreadId(opCall._threadId);
	
}

void ScriptOpcodes::opSetSceneIdThreadId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	ARG_UINT32(threadId);
	_vm->_scriptMan->setSceneIdThreadId(sceneId, threadId);
}

void ScriptOpcodes::opSetFontId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(fontId);
	_vm->_scriptMan->setCurrFontId(fontId);
}

} // End of namespace Illusions
