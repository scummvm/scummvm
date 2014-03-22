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
#include "illusions/dictionary.h"
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
	debug("\nexecOpcode([%08X] %d) %s", opCall._callerThreadId, opCall._op, _opcodeNames[opCall._op].c_str());
	(*_opcodes[opCall._op])(scriptThread, opCall);
}

typedef Common::Functor2Mem<ScriptThread*, OpCall&, void, ScriptOpcodes> ScriptOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new ScriptOpcodeI(this, &ScriptOpcodes::func); \
	_opcodeNames[op] = #func;

void ScriptOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < 256; ++i)
		_opcodes[i] = 0;
	// Register opcodes
	OPCODE(2, opSuspend);
	OPCODE(3, opYield);
	OPCODE(4, opTerminate);
	OPCODE(5, opJump);
	OPCODE(6, opStartScriptThread);
	OPCODE(8, opStartTempScriptThread);
	OPCODE(9, opStartTimerThread);
	OPCODE(14, opSetThreadSceneId);
	OPCODE(15, opEndTalkThreads);
	OPCODE(16, opLoadResource);
	OPCODE(20, opEnterScene);
	OPCODE(25, opChangeScene);
	OPCODE(39, opSetDisplay);
	OPCODE(42, opIncBlockCounter);
	OPCODE(45, opSetProperty);
	OPCODE(46, opPlaceActor);	
	OPCODE(49, opStartSequenceActor);
	OPCODE(56, opStartTalkThread);
	OPCODE(57, opAppearActor);
	OPCODE(58, opDisappearActor);
	OPCODE(60, opActivateObject);
	OPCODE(61, opDeactivateObject);
	OPCODE(63, opSetSelectSfx);
	OPCODE(64, opSetMoveSfx);
	OPCODE(65, opSetDenySfx);
	OPCODE(66, opSetAdjustUpSfx);
	OPCODE(67, opSetAdjustDnSfx);
	OPCODE(75, opStartMusic);
	OPCODE(78, opStackPushRandom);
	OPCODE(79, opIfLte);
	OPCODE(80, opAddMenuChoice);
	OPCODE(81, opDisplayMenu);
	OPCODE(82, opSwitchMenuChoice);
	OPCODE(84, opResetGame);
	OPCODE(87, opDeactivateButton);
	OPCODE(88, opActivateButton);
	OPCODE(103, opJumpIf);
	OPCODE(107, opBoolNot);
	OPCODE(110, opGetProperty);
	OPCODE(111, opCompareBlockCounter);
	OPCODE(126, opDebug126);
	OPCODE(144, opPlayVideo);
	OPCODE(146, opStackPop);
	OPCODE(147, opStackDup);
	OPCODE(148, opLoadSpecialCodeModule);
	OPCODE(150, opRunSpecialCode);
	OPCODE(161, opSetActorUsePan);
	OPCODE(168, opStartAbortableThread);
	OPCODE(169, opKillThread);
	OPCODE(175, opSetSceneIdThreadId);
	OPCODE(176, opStackPush0);
	OPCODE(177, opSetFontId);
	OPCODE(178, opAddMenuKey);
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

void ScriptOpcodes::opTerminate(ScriptThread *scriptThread, OpCall &opCall) {
	opCall._result = kTSTerminate;
}

void ScriptOpcodes::opJump(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(jumpOffs);
	opCall._deltaOfs += jumpOffs;
}

void ScriptOpcodes::opStartScriptThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->_scriptMan->startScriptThread(threadId, opCall._threadId,
		scriptThread->_value8, scriptThread->_valueC, scriptThread->_value10);
}

void ScriptOpcodes::opStartTempScriptThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(codeOffs);
	_vm->_scriptMan->startTempScriptThread(opCall._code + codeOffs,
		opCall._threadId, scriptThread->_value8, scriptThread->_valueC, scriptThread->_value10);
}

void ScriptOpcodes::opStartTimerThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(isAbortable);
	ARG_INT16(duration);
	ARG_INT16(maxDuration);
	if (maxDuration)
		duration += _vm->getRandom(maxDuration);
		
duration = 5;//DEBUG Speeds up things		
		
	if (isAbortable)
		_vm->_scriptMan->startAbortableTimerThread(duration, opCall._threadId);
	else
		_vm->_scriptMan->startTimerThread(duration, opCall._threadId);
}

void ScriptOpcodes::opSetThreadSceneId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->_scriptMan->_threads->setThreadSceneId(opCall._callerThreadId, sceneId);
}

void ScriptOpcodes::opEndTalkThreads(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_scriptMan->_threads->endTalkThreads();
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
	if (!_vm->_scriptMan->enterScene(sceneId, opCall._callerThreadId))
		opCall._result = kTSTerminate;
}

void ScriptOpcodes::opChangeScene(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	ARG_UINT32(threadId);
	// NOTE Skipped checking for stalled resources
	_vm->_input->discardButtons(0xFFFF);
	_vm->_scriptMan->_prevSceneId = _vm->_scriptMan->_activeScenes.getCurrentScene();
	_vm->_scriptMan->exitScene(opCall._callerThreadId);
	_vm->_scriptMan->enterScene(sceneId, opCall._callerThreadId);
	// TODO _vm->_gameStates->writeStates(_vm->_scriptMan->_prevSceneId, sceneId, threadId);
	_vm->_scriptMan->startAnonScriptThread(threadId, 0,
		scriptThread->_value8, scriptThread->_valueC, scriptThread->_value10);
}

void ScriptOpcodes::opSetDisplay(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(flag);
	_vm->_screen->setDisplayOn(flag != 0);
}

void ScriptOpcodes::opIncBlockCounter(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);	
	byte value = _vm->_scriptMan->_scriptResource->_blockCounters.get(index) + 1;
	if (value <= 63)
		_vm->_scriptMan->_scriptResource->_blockCounters.set(index, value);
}

void ScriptOpcodes::opSetProperty(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(value);	
	ARG_UINT32(propertyId);	
	_vm->_scriptMan->_scriptResource->_properties.set(propertyId, value != 0);
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

void ScriptOpcodes::opStartSequenceActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	// NOTE Skipped checking for stalled sequence, not sure if needed
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->startSequenceActor(sequenceId, 2, opCall._threadId);
}

void ScriptOpcodes::opStartTalkThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(duration);	
	ARG_UINT32(objectId);
	ARG_UINT32(talkId);
	ARG_UINT32(sequenceId1);
	ARG_UINT32(sequenceId2);
	ARG_UINT32(namedPointId);
	// NOTE Skipped checking for stalled sequence, not sure if needed
	_vm->_scriptMan->startTalkThread(duration, objectId, talkId, sequenceId1, sequenceId2, namedPointId, opCall._callerThreadId);

	//DEBUG Resume calling thread, later done after talking is finished
	//_vm->notifyThreadId(opCall._threadId);
}

void ScriptOpcodes::opAppearActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	if (!control) {
		Common::Point pos = _vm->getNamedPointPosition(0x70023);
		_vm->_controls->placeActor(0x50001, pos, 0x60001, objectId, 0);
		control = _vm->_dict->getObjectControl(objectId);
		control->startSequenceActor(0x60001, 2, 0);
	}
	control->appearActor();
}

void ScriptOpcodes::opDisappearActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->disappearActor();
}

void ScriptOpcodes::opActivateObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	if (control)
		control->activateObject();
}

void ScriptOpcodes::opDeactivateObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->deactivateObject();
}

void ScriptOpcodes::opSetSelectSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setSelectSfx(soundEffectId);
}

void ScriptOpcodes::opSetMoveSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setMoveSfx(soundEffectId);
}

void ScriptOpcodes::opSetDenySfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setDenySfx(soundEffectId);
}

void ScriptOpcodes::opSetAdjustUpSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setAdjustUpSfx(soundEffectId);
}

void ScriptOpcodes::opSetAdjustDnSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setAdjustDnSfx(soundEffectId);
}
void ScriptOpcodes::opStartMusic(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(volume);
	ARG_INT16(pan);
	ARG_UINT32(musicId);
	ARG_UINT32(type);
	// TODO _vm->playMusic(musicId, type, volume, pan);
}

void ScriptOpcodes::opStackPushRandom(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(maxValue);
	_vm->_scriptMan->_stack.push(_vm->getRandom(maxValue) + 1);
}

void ScriptOpcodes::opIfLte(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(rvalue);
	ARG_INT16(elseJumpOffs);
	int16 lvalue = _vm->_scriptMan->_stack.pop();
	if (!(lvalue <= rvalue))
		opCall._deltaOfs += elseJumpOffs;
}

void ScriptOpcodes::opAddMenuChoice(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(jumpOffs);
	ARG_INT16(endMarker);
	_vm->_scriptMan->_stack.push(endMarker);
	_vm->_scriptMan->_stack.push(jumpOffs);
}

void ScriptOpcodes::opDisplayMenu(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(unk1);
	ARG_UINT32(menuId);
	ARG_UINT32(unk2);
	// TODO _vm->_shellMgr->displayMenu(_vm->_scriptMan->_stack.topPtr(), &_vm->_scriptMan->_menuChoiceOfs, menuId, unk1, unk2, opCall._callerThreadId);
	// Remove menu choices from the stack
	do {
		_vm->_scriptMan->_stack.pop();
	} while (_vm->_scriptMan->_stack.pop() == 0);

	//DEBUG Resume calling thread, later done by the video player
	_vm->notifyThreadId(opCall._callerThreadId);

}

void ScriptOpcodes::opSwitchMenuChoice(ScriptThread *scriptThread, OpCall &opCall) {
_vm->_scriptMan->_menuChoiceOfs = 88; // DEBUG Chose "Start game"

	opCall._deltaOfs += _vm->_scriptMan->_menuChoiceOfs;
debug("deltaOfs = %08X", opCall._deltaOfs);	
}

void ScriptOpcodes::opResetGame(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_scriptMan->_threads->terminateThreads(opCall._callerThreadId);
	_vm->_scriptMan->reset();
	_vm->_input->activateButton(0xFFFF);
	// TODO _vm->stopMusic();
	// TODO _vm->_gameStates->clear();
}

void ScriptOpcodes::opDeactivateButton(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(button)
	_vm->_input->deactivateButton(button);
}

void ScriptOpcodes::opActivateButton(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(button)
	_vm->_input->activateButton(button);
}

void ScriptOpcodes::opJumpIf(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(jumpOffs)
	int16 value = _vm->_scriptMan->_stack.pop();
	if (value == 0)
		opCall._deltaOfs += jumpOffs;
}

void ScriptOpcodes::opBoolNot(ScriptThread *scriptThread, OpCall &opCall) {
	int16 value = _vm->_scriptMan->_stack.pop();
	_vm->_scriptMan->_stack.push(value != 0 ? 0 : 1);
}

void ScriptOpcodes::opGetProperty(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(propertyId)
	bool value = _vm->_scriptMan->_scriptResource->_properties.get(propertyId);
	_vm->_scriptMan->_stack.push(value ? 1 : 0);
}

void ScriptOpcodes::opCompareBlockCounter(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);	
	ARG_INT16(compareOp);	
	ARG_INT16(rvalue);
	int16 lvalue = _vm->_scriptMan->_scriptResource->_blockCounters.get(index);
	debug("lvalue = %d", lvalue);
	bool compareResult = false;
	switch (compareOp) {
	case 1:
		compareResult = false;//lvalue == rvalue;
		break;
	case 2:
		compareResult = lvalue != rvalue;
		break;
	case 3:
		compareResult = lvalue < rvalue;
		break;
	case 4:
		compareResult = lvalue > rvalue;
		break;
	case 5:
		compareResult = lvalue >= rvalue;
		break;
	case 6:
		compareResult = lvalue <= rvalue;
		break;
	}
	debug("  compareResult -> %d", compareResult);
	_vm->_scriptMan->_stack.push(compareResult ? 1 : 0);
}

void ScriptOpcodes::opDebug126(ScriptThread *scriptThread, OpCall &opCall) {
	// NOTE Prints some debug text
	debug("[DBG] %s", (char*)opCall._code);
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

void ScriptOpcodes::opStackPop(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_scriptMan->_stack.pop(); 
}

void ScriptOpcodes::opStackDup(ScriptThread *scriptThread, OpCall &opCall) {
	int16 value = _vm->_scriptMan->_stack.peek();
	_vm->_scriptMan->_stack.push(value); 
}

void ScriptOpcodes::opLoadSpecialCodeModule(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(specialCodeModuleId);
	// TODO _vm->loadSpecialCodeModule(specialCodeModuleId);
}

void ScriptOpcodes::opRunSpecialCode(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(specialCodeId);
	_vm->_scriptMan->_callerThreadId = opCall._callerThreadId;
	// TODO _vm->runSpecialCode(specialCodeId, opCall._code + 8, opCall._threadId);
	_vm->_scriptMan->_callerThreadId = 0;

	//DEBUG Resume calling thread, later done by the special code
	_vm->notifyThreadId(opCall._callerThreadId);

}

void ScriptOpcodes::opSetActorUsePan(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(usePan)
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->setActorUsePan(usePan);
}

void ScriptOpcodes::opStartAbortableThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(codeOffs);
	ARG_INT16(skipOffs);
	_vm->_scriptMan->startAbortableThread(opCall._code + codeOffs,
		opCall._code + skipOffs, opCall._callerThreadId);
}

void ScriptOpcodes::opKillThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->_scriptMan->_threads->killThread(threadId);
}

void ScriptOpcodes::opSetSceneIdThreadId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	ARG_UINT32(threadId);
	_vm->_scriptMan->setSceneIdThreadId(sceneId, threadId);
}

void ScriptOpcodes::opStackPush0(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_scriptMan->_stack.push(0);
}

void ScriptOpcodes::opSetFontId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(fontId);
	_vm->_scriptMan->setCurrFontId(fontId);
}

void ScriptOpcodes::opAddMenuKey(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(key);
	ARG_UINT32(threadId);
	// TODO _vm->addMenuKey(key, threadId);
}

} // End of namespace Illusions
