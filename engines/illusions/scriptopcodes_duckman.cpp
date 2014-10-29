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

#include "illusions/illusions_duckman.h"
#include "illusions/scriptopcodes_duckman.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/screen.h"
#include "illusions/scriptman.h"
#include "illusions/scriptresource.h"
#include "illusions/scriptthread.h"
#include "illusions/specialcode.h"
#include "illusions/talkresource.h"

namespace Illusions {

// ScriptOpcodes_Duckman

ScriptOpcodes_Duckman::ScriptOpcodes_Duckman(IllusionsEngine_Duckman *vm)
	: ScriptOpcodes(vm), _vm(vm) {
	initOpcodes();
}

ScriptOpcodes_Duckman::~ScriptOpcodes_Duckman() {
	freeOpcodes();
}

typedef Common::Functor2Mem<ScriptThread*, OpCall&, void, ScriptOpcodes_Duckman> ScriptOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new ScriptOpcodeI(this, &ScriptOpcodes_Duckman::func); \
	_opcodeNames[op] = #func;

void ScriptOpcodes_Duckman::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < 256; ++i)
		_opcodes[i] = 0;
	OPCODE(2, opSuspend);
	OPCODE(3, opYield);
	OPCODE(4, opTerminate);
	OPCODE(5, opJump);
	OPCODE(6, opStartScriptThread);
	OPCODE(7, opStartTimerThread);
	OPCODE(9, opNotifyThread);
	OPCODE(10, opSuspendThread);
	OPCODE(16, opLoadResource);
	OPCODE(17, opUnloadResource);
	OPCODE(18, opEnterScene18);
	OPCODE(20, opChangeScene);
	OPCODE(22, opStartModalScene);
	OPCODE(23, opExitModalScene);
	OPCODE(24, opEnterScene24);
	OPCODE(25, opLeaveScene24);
	OPCODE(33, opPanTrackObject);
	OPCODE(34, opPanToObject);
	OPCODE(36, opPanToPoint);
	OPCODE(37, opPanStop);
	OPCODE(38, opStartFade);
	OPCODE(39, opSetDisplay);
	OPCODE(40, opSetCameraBounds);
	OPCODE(48, opSetProperty);
	OPCODE(49, opPlaceActor);
	OPCODE(50, opFaceActor);
	OPCODE(51, opFaceActorToObject);
	OPCODE(52, opStartSequenceActor);
	OPCODE(53, opStartSequenceActorAtPosition);
	OPCODE(54, opStartMoveActor);
	OPCODE(55, opStartMoveActorToObject);
	OPCODE(56, opStartTalkThread);
	OPCODE(57, opAppearActor);
	OPCODE(58, opDisappearActor);
	OPCODE(59, opActivateObject);
	OPCODE(60, opDeactivateObject);
	OPCODE(61, opSetDefaultSequence);
	OPCODE(64, opStopCursorHoldingObject);
	OPCODE(65, opStartCursorHoldingObject);
	OPCODE(66, opPlayVideo);
	OPCODE(69, opRunSpecialCode);
	OPCODE(72, opStartSound);
	OPCODE(75, opStopSound);
	OPCODE(76, opStartMidiMusic);
	OPCODE(77, opStopMidiMusic);
	OPCODE(78, opFadeMidiMusic);
	OPCODE(80, opAddMenuChoice);
	OPCODE(81, opDisplayMenu);
	OPCODE(82, opSwitchMenuChoice);
	OPCODE(84, opResetGame);
	OPCODE(87, opDeactivateButton);
	OPCODE(88, opActivateButton);
	OPCODE(96, opIncBlockCounter);
	OPCODE(97, opClearBlockCounter);
	OPCODE(104, opJumpIf);
	OPCODE(105, opIsPrevSceneId);
	OPCODE(106, opNot);
	OPCODE(107, opAnd);
	OPCODE(108, opOr);
	OPCODE(109, opGetProperty);
	OPCODE(110, opCompareBlockCounter);
	OPCODE(112, opAddDialogItem);
	OPCODE(113, opStartDialog);
	OPCODE(114, opJumpToDialogChoice);
	OPCODE(115, opSetBlockCounter115);
	OPCODE(116, opSetBlockCounter116);
	OPCODE(117, opSetBlockCounter117);
	OPCODE(118, opSetBlockCounter118);
	OPCODE(126, opDebug126);
	OPCODE(127, opDebug127);
#if 0		
	// Register opcodes
	OPCODE(8, opStartTempScriptThread);
	OPCODE(14, opSetThreadSceneId);
	OPCODE(15, opEndTalkThreads);
	OPCODE(20, opEnterScene);
	OPCODE(30, opEnterCloseUpScene);
	OPCODE(31, opExitCloseUpScene);
	OPCODE(32, opPanCenterObject);
	OPCODE(35, opPanToNamedPoint);
	OPCODE(53, opSetActorToNamedPoint);
	OPCODE(63, opSetSelectSfx);
	OPCODE(64, opSetMoveSfx);
	OPCODE(65, opSetDenySfx);
	OPCODE(66, opSetAdjustUpSfx);
	OPCODE(67, opSetAdjustDnSfx);
	OPCODE(78, opStackPushRandom);
	OPCODE(79, opIfLte);
	OPCODE(105, opIsCurrentSceneId);
	OPCODE(106, opIsActiveSceneId);
	OPCODE(146, opStackPop);
	OPCODE(147, opStackDup);
	OPCODE(148, opLoadSpecialCodeModule);
	OPCODE(160, opStopActor);
	OPCODE(161, opSetActorUsePan);
	OPCODE(168, opStartAbortableThread);
	OPCODE(169, opKillThread);
	OPCODE(175, opSetSceneIdThreadId);
	OPCODE(176, opStackPush0);
	OPCODE(177, opSetFontId);
	OPCODE(178, opAddMenuKey);
	OPCODE(179, opChangeSceneAll);
#endif	
}

#undef OPCODE

void ScriptOpcodes_Duckman::freeOpcodes() {
	for (uint i = 0; i < 256; ++i)
		delete _opcodes[i];
}

// Opcodes

void ScriptOpcodes_Duckman::opSuspend(ScriptThread *scriptThread, OpCall &opCall) {
	opCall._result = kTSSuspend;
}

void ScriptOpcodes_Duckman::opYield(ScriptThread *scriptThread, OpCall &opCall) {
	opCall._result = kTSYield;
}

void ScriptOpcodes_Duckman::opTerminate(ScriptThread *scriptThread, OpCall &opCall) {
	opCall._result = kTSTerminate;
}

void ScriptOpcodes_Duckman::opJump(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(jumpOffs);
	opCall._deltaOfs += jumpOffs;
}

void ScriptOpcodes_Duckman::opStartScriptThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->startScriptThread(threadId, opCall._threadId);
}

void ScriptOpcodes_Duckman::opStartTimerThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(isAbortable);
	ARG_INT16(duration);
	ARG_INT16(maxDuration);
	if (maxDuration)
		duration += _vm->getRandom(maxDuration);
		
//duration = 1;//DEBUG Speeds up things
duration = 5;		
		
	if (isAbortable)
		_vm->startAbortableTimerThread(duration, opCall._threadId);
	else
		_vm->startTimerThread(duration, opCall._threadId);
}

void ScriptOpcodes_Duckman::opNotifyThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->_threads->notifyId(threadId);
	_vm->_threads->notifyTimerThreads(threadId);
}

void ScriptOpcodes_Duckman::opSuspendThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->_threads->suspendId(threadId);
	_vm->_threads->suspendTimerThreads(threadId);
}

void ScriptOpcodes_Duckman::opLoadResource(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(resourceId);
	uint32 sceneId = _vm->getCurrentScene();
	_vm->_resSys->loadResource(resourceId, sceneId, opCall._threadId);
	_vm->notifyThreadId(opCall._threadId);
}

void ScriptOpcodes_Duckman::opUnloadResource(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(resourceId);
	_vm->_resSys->unloadResourceById(resourceId);
}

void ScriptOpcodes_Duckman::opEnterScene18(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->enterScene(sceneId, 0);
}

//static uint dsceneId = 0, dthreadId = 0;
//static uint dsceneId = 0x00010008, dthreadId = 0x00020029;//Beginning in Jac
//static uint dsceneId = 0x00010012, dthreadId = 0x0002009D;//Paramount
//static uint dsceneId = 0x00010039, dthreadId = 0x00020089;//Map
//static uint dsceneId = 0x00010033, dthreadId = 0x000201A4;//Chinese
//static uint dsceneId = 0x00010020, dthreadId = 0x00020112;//Xmas
//static uint dsceneId = 0x00010039, dthreadId = 0x00020089;//Pizza
//static uint dsceneId = 0x0001002D, dthreadId = 0x00020141;
//static uint dsceneId = 0x0001004B, dthreadId = 0x0002029B;
//static uint dsceneId = 0x00010021, dthreadId = 0x00020113;
static uint dsceneId = 0x0001000A, dthreadId = 0x00020043;

void ScriptOpcodes_Duckman::opChangeScene(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	ARG_UINT32(threadId);
	_vm->_input->discardButtons(0xFFFF);
	
	debug("changeScene(%08X, %08X)", sceneId, threadId);
	
	//DEBUG
	if (dsceneId) {
		sceneId = dsceneId;
		threadId = dthreadId;
		dsceneId = 0;
	}
	
	if (_vm->_scriptResource->_properties.get(31)) {
		_vm->changeScene(0x10002, 0x20001, opCall._callerThreadId);
	} else {
		_vm->changeScene(sceneId, threadId, opCall._callerThreadId);
	}
}

void ScriptOpcodes_Duckman::opStartModalScene(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	ARG_UINT32(threadId);
	_vm->_input->discardButtons(0xFFFF);
	_vm->enterPause(_vm->getCurrentScene(), opCall._callerThreadId);
	_vm->_talkItems->pauseByTag(_vm->getCurrentScene());
	_vm->enterScene(sceneId, threadId);
	opCall._result = kTSSuspend;
}

void ScriptOpcodes_Duckman::opExitModalScene(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_input->discardButtons(0xFFFF);
	if (_vm->_scriptResource->_properties.get(0x000E0027)) {
		// TODO _vm->startScriptThread2(0x10002, 0x20001, 0);
		opCall._result = kTSTerminate;
	} else {
		_vm->dumpCurrSceneFiles(_vm->getCurrentScene(), opCall._callerThreadId);
		_vm->exitScene();
		_vm->leavePause(_vm->getCurrentScene(), opCall._callerThreadId);
		_vm->_talkItems->unpauseByTag(_vm->getCurrentScene());
	}
}

void ScriptOpcodes_Duckman::opEnterScene24(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->_input->discardButtons(0xFFFF);
	_vm->enterPause(_vm->getCurrentScene(), opCall._callerThreadId);
	_vm->enterScene(sceneId, 0);
}

void ScriptOpcodes_Duckman::opLeaveScene24(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_input->discardButtons(0xFFFF);
	_vm->dumpCurrSceneFiles(_vm->getCurrentScene(), opCall._callerThreadId);
	_vm->exitScene();
	_vm->leavePause(_vm->getCurrentScene(), opCall._callerThreadId);
}

void ScriptOpcodes_Duckman::opPanTrackObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	_vm->_camera->panTrackObject(objectId);
}

void ScriptOpcodes_Duckman::opPanToObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(speed);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	Common::Point pos = control->getActorPosition();
	_vm->_camera->panToPoint(pos, speed, opCall._threadId);
}

void ScriptOpcodes_Duckman::opPanToPoint(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(speed);
	ARG_INT16(x);
	ARG_INT16(y);
	_vm->_camera->panToPoint(Common::Point(x, y), speed, opCall._threadId);
}

void ScriptOpcodes_Duckman::opPanStop(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_camera->stopPan();
}

void ScriptOpcodes_Duckman::opStartFade(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(arg1);
	ARG_INT16(arg2);
	ARG_INT16(arg3);
	ARG_INT16(arg4);
	ARG_INT16(arg5);
	// TODO

	//DEBUG Resume calling thread, later done when the fading is finished
	_vm->notifyThreadId(opCall._threadId);
}

void ScriptOpcodes_Duckman::opSetDisplay(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(flag);
	_vm->_screen->setDisplayOn(flag != 0);
}

void ScriptOpcodes_Duckman::opSetCameraBounds(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(x1);
	ARG_INT16(y1);
	ARG_INT16(x2);
	ARG_INT16(y2);
	_vm->_camera->setBounds(Common::Point(x1, y1), Common::Point(x2, y2));
}

void ScriptOpcodes_Duckman::opSetProperty(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(value);
	ARG_UINT32(propertyId);
	_vm->_scriptResource->_properties.set(propertyId, value != 0);
}

void ScriptOpcodes_Duckman::opPlaceActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	ARG_UINT32(namedPointId);
	Common::Point pos = _vm->getNamedPointPosition(namedPointId);
	uint32 actorTypeId = _vm->getObjectActorTypeId(objectId);
	_vm->_controls->placeActor(actorTypeId, pos, sequenceId, objectId, opCall._threadId);
}

void ScriptOpcodes_Duckman::opFaceActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(facing);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->faceActor(facing);
}

void ScriptOpcodes_Duckman::opFaceActorToObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId2);
	ARG_UINT32(objectId1);
	Control *control1 = _vm->_dict->getObjectControl(objectId1);
	Control *control2 = _vm->_dict->getObjectControl(objectId2);
	Common::Point pos1 = control1->getActorPosition();
	Common::Point pos2 = control2->getActorPosition();
	uint facing;
	if (_vm->calcPointDirection(pos1, pos2, facing))
		control1->faceActor(facing);
}

void ScriptOpcodes_Duckman::opStartSequenceActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->startSequenceActor(sequenceId, 2, opCall._threadId);
}

void ScriptOpcodes_Duckman::opStartSequenceActorAtPosition(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	ARG_UINT32(namedPointId);
	Common::Point pos = _vm->getNamedPointPosition(namedPointId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->setActorPosition(pos);
	control->startSequenceActor(sequenceId, 2, opCall._threadId);
}

void ScriptOpcodes_Duckman::opStartMoveActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	ARG_UINT32(namedPointId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	Common::Point pos = _vm->getNamedPointPosition(namedPointId);
	control->startMoveActor(sequenceId, pos, opCall._callerThreadId, opCall._threadId);
}

void ScriptOpcodes_Duckman::opStartMoveActorToObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId1);
	ARG_UINT32(objectId2);
	ARG_UINT32(sequenceId);
	Control *control1 = _vm->_dict->getObjectControl(objectId1);
	Common::Point pos;
	if (objectId2 == 0x40003) {
		pos = _vm->_cursor._position;
	} else {
		Control *control2 = _vm->_dict->getObjectControl(objectId2);
		pos = control2->_feetPt;
		if (control2->_actor) {
			pos.x += control2->_actor->_position.x;
			pos.y += control2->_actor->_position.y;
		}
	}
	control1->startMoveActor(sequenceId, pos, opCall._callerThreadId, opCall._threadId);
}

void ScriptOpcodes_Duckman::opStartTalkThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(talkId);
	ARG_UINT32(sequenceId1);
	ARG_UINT32(sequenceId2);
	_vm->startTalkThread(objectId, talkId, sequenceId1, sequenceId2, opCall._threadId);
}

void ScriptOpcodes_Duckman::opAppearActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	if (!control) {
		Common::Point pos = _vm->getNamedPointPosition(0x70001);		
		_vm->_controls->placeActor(0x50001, pos, 0x60001, objectId, 0);
		control = _vm->_dict->getObjectControl(objectId);
	}
	control->appearActor();
}

void ScriptOpcodes_Duckman::opDisappearActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->disappearActor();
}

void ScriptOpcodes_Duckman::opActivateObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	if (control)
		control->activateObject();
}

void ScriptOpcodes_Duckman::opDeactivateObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->deactivateObject();
}

void ScriptOpcodes_Duckman::opSetDefaultSequence(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(defaultSequenceId);
	ARG_UINT32(sequenceId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->_actor->_defaultSequences.set(sequenceId, defaultSequenceId);
}

void ScriptOpcodes_Duckman::opStopCursorHoldingObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(flags);
	_vm->stopCursorHoldingObject();
	if (!(flags & 1))
		_vm->playSoundEffect(7);
}

void ScriptOpcodes_Duckman::opStartCursorHoldingObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(flags);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	_vm->startCursorHoldingObject(objectId, sequenceId);
	if (!(flags & 1))
		_vm->playSoundEffect(6);
}

void ScriptOpcodes_Duckman::opPlayVideo(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	// NOTE This has no attached objectId or priority
	// TODO _vm->playVideo(videoId, objectId, value, opCall._threadId);
	
	//DEBUG Resume calling thread, later done by the video player
	_vm->notifyThreadId(opCall._threadId);
	
}

void ScriptOpcodes_Duckman::opRunSpecialCode(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(specialCodeId);
	_vm->runSpecialCode(specialCodeId, opCall);
	//DEBUG Resume calling thread, later done by the special code
	_vm->notifyThreadId(opCall._threadId);
}

void ScriptOpcodes_Duckman::opStartSound(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(volume);
	ARG_UINT32(soundEffectId);
	// TODO _vm->startSound(soundEffectId, volume, pan);
}

void ScriptOpcodes_Duckman::opStopSound(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->stopSound(soundEffectId);
}

void ScriptOpcodes_Duckman::opStartMidiMusic(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(musicId);
	// TODO _vm->playMidiMusic(musicId);
}

void ScriptOpcodes_Duckman::opStopMidiMusic(ScriptThread *scriptThread, OpCall &opCall) {
	// TODO _vm->stopMidiMusic();
}

void ScriptOpcodes_Duckman::opFadeMidiMusic(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(duration);
	ARG_INT16(finalVolume);
	// TODO _vm->fadeMidiMusic(2, finalVolume, duration, opCall._threadId);
}

void ScriptOpcodes_Duckman::opAddMenuChoice(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(jumpOffs);
	ARG_INT16(endMarker);
	_vm->_stack->push(endMarker);
	_vm->_stack->push(jumpOffs);
}

void ScriptOpcodes_Duckman::opDisplayMenu(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(unk1);
	ARG_UINT32(menuId);
	ARG_UINT32(unk2);
	// TODO _vm->_shellMgr->displayMenu(_vm->_stack->topPtr(), &_vm->_menuChoiceOfs, menuId, unk1, unk2, opCall._callerThreadId);
	// Remove menu choices from the stack
	do {
		_vm->_stack->pop();
	} while (_vm->_stack->pop() == 0);

	//DEBUG Resume calling thread, later done by the video player
	_vm->notifyThreadId(opCall._callerThreadId);

}

void ScriptOpcodes_Duckman::opSwitchMenuChoice(ScriptThread *scriptThread, OpCall &opCall) {
_vm->_menuChoiceOfs = 156; // DEBUG Chose "Start game"

	opCall._deltaOfs += _vm->_menuChoiceOfs;
}

void ScriptOpcodes_Duckman::opResetGame(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->reset();
	_vm->_input->activateButton(0xFFFF);
	// TODO _vm->stopMusic();
	// TODO _vm->_gameStates->clear();
}

void ScriptOpcodes_Duckman::opDeactivateButton(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(button)
	_vm->_input->deactivateButton(button);
}

void ScriptOpcodes_Duckman::opActivateButton(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(button)
	_vm->_input->activateButton(button);
}

void ScriptOpcodes_Duckman::opIncBlockCounter(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);	
	byte value = _vm->_scriptResource->_blockCounters.get(index) + 1;
	if (value <= 63)
		_vm->_scriptResource->_blockCounters.set(index, value);
}

void ScriptOpcodes_Duckman::opClearBlockCounter(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);
	_vm->_scriptResource->_blockCounters.set(index, 0);
}

void ScriptOpcodes_Duckman::opJumpIf(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(jumpOffs);
	int16 value = _vm->_stack->pop();
	if (value == 0)
		opCall._deltaOfs += jumpOffs;
}

void ScriptOpcodes_Duckman::opIsPrevSceneId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->_stack->push(_vm->_prevSceneId == sceneId ? 1 : 0);
}

void ScriptOpcodes_Duckman::opNot(ScriptThread *scriptThread, OpCall &opCall) {
	int16 value = _vm->_stack->pop();
	_vm->_stack->push(value != 0 ? 0 : 1);
}

void ScriptOpcodes_Duckman::opAnd(ScriptThread *scriptThread, OpCall &opCall) {
	int16 value1 = _vm->_stack->pop();
	int16 value2 = _vm->_stack->pop();
	_vm->_stack->push(value1 & value2);
}

void ScriptOpcodes_Duckman::opOr(ScriptThread *scriptThread, OpCall &opCall) {
	int16 value1 = _vm->_stack->pop();
	int16 value2 = _vm->_stack->pop();
	_vm->_stack->push(value1 | value2);
}

void ScriptOpcodes_Duckman::opGetProperty(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(propertyId)
	bool value = _vm->_scriptResource->_properties.get(propertyId);
	_vm->_stack->push(value ? 1 : 0);
}

void ScriptOpcodes_Duckman::opCompareBlockCounter(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);	
	ARG_INT16(compareOp);	
	ARG_INT16(rvalue);
	int16 lvalue = _vm->_scriptResource->_blockCounters.get(index);
	bool compareResult = false;
	switch (compareOp) {
	case 1:
		compareResult = lvalue == rvalue;
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
	_vm->_stack->push(compareResult ? 1 : 0);
}

void ScriptOpcodes_Duckman::opAddDialogItem(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(index);
	ARG_INT16(choiceJumpOffs);
	ARG_UINT32(sequenceId);
	if (index && (_vm->_scriptResource->_blockCounters.getC0(index) & 0x40))
		_vm->addDialogItem(choiceJumpOffs, sequenceId);
}

void ScriptOpcodes_Duckman::opStartDialog(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(actorTypeId);
	_vm->startDialog(&_vm->_menuChoiceOfs, actorTypeId, opCall._callerThreadId);
}

void ScriptOpcodes_Duckman::opJumpToDialogChoice(ScriptThread *scriptThread, OpCall &opCall) {
	opCall._deltaOfs += _vm->_menuChoiceOfs;
}

void ScriptOpcodes_Duckman::opSetBlockCounter115(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);
	if (_vm->_scriptResource->_blockCounters.getC0(index) & 0x80)
		_vm->_scriptResource->_blockCounters.set(index, 0);
	_vm->_scriptResource->_blockCounters.setC0(index, 0x40);
}

void ScriptOpcodes_Duckman::opSetBlockCounter116(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);
	if (!(_vm->_scriptResource->_blockCounters.getC0(index) & 0x80))
		_vm->_scriptResource->_blockCounters.setC0(index, 0x40);
}

void ScriptOpcodes_Duckman::opSetBlockCounter117(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);
	_vm->_scriptResource->_blockCounters.setC0(index, 0);
}

void ScriptOpcodes_Duckman::opSetBlockCounter118(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(index);
	_vm->_scriptResource->_blockCounters.setC0(index, 0x40);
}

void ScriptOpcodes_Duckman::opDebug126(ScriptThread *scriptThread, OpCall &opCall) {
	// NOTE Prints some debug text
	debug(1, "[DBG126] %s", (char*)opCall._code);
}

void ScriptOpcodes_Duckman::opDebug127(ScriptThread *scriptThread, OpCall &opCall) {
	// NOTE Prints some debug text
	debug(1, "[DBG127] %s", (char*)opCall._code);
}

#if 0

void ScriptOpcodes_Duckman::opStartTempScriptThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(codeOffs);
	_vm->startTempScriptThread(opCall._code + codeOffs,
		opCall._threadId, scriptThread->_value8, scriptThread->_valueC, scriptThread->_value10);
}

void ScriptOpcodes_Duckman::opSetThreadSceneId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->_threads->setThreadSceneId(opCall._callerThreadId, sceneId);
}

void ScriptOpcodes_Duckman::opEndTalkThreads(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_threads->endTalkThreads();
}

void ScriptOpcodes_Duckman::opEnterScene(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	uint scenesCount = _vm->_activeScenes.getActiveScenesCount();
	if (scenesCount > 0) {
		uint32 currSceneId;
		_vm->_activeScenes.getActiveSceneInfo(scenesCount, &currSceneId, 0);
		// TODO krnfileDump(currSceneId);
	}
	if (!_vm->enterScene(sceneId, opCall._callerThreadId))
		opCall._result = kTSTerminate;
}

void ScriptOpcodes_Duckman::opEnterCloseUpScene(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->_input->discardButtons(0xFFFF);
	_vm->enterPause(opCall._callerThreadId);
	_vm->enterScene(sceneId, opCall._callerThreadId);
}

void ScriptOpcodes_Duckman::opExitCloseUpScene(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->exitScene(opCall._callerThreadId);
	_vm->leavePause(opCall._callerThreadId);
	opCall._result = kTSYield;
}

void ScriptOpcodes_Duckman::opPanCenterObject(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(speed);	
	ARG_UINT32(objectId);
	_vm->_camera->panCenterObject(objectId, speed);
}

void ScriptOpcodes_Duckman::opPanToNamedPoint(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(speed);	
	ARG_UINT32(namedPointId);
	Common::Point pos = _vm->getNamedPointPosition(namedPointId);
	_vm->_camera->panToPoint(pos, speed, opCall._threadId);
}

void ScriptOpcodes_Duckman::opSetActorToNamedPoint(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	ARG_UINT32(namedPointId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	Common::Point pos = _vm->getNamedPointPosition(namedPointId);
	control->stopActor();
	control->setActorPosition(pos);
}

void ScriptOpcodes_Duckman::opSetSelectSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setSelectSfx(soundEffectId);
}

void ScriptOpcodes_Duckman::opSetMoveSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setMoveSfx(soundEffectId);
}

void ScriptOpcodes_Duckman::opSetDenySfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setDenySfx(soundEffectId);
}

void ScriptOpcodes_Duckman::opSetAdjustUpSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setAdjustUpSfx(soundEffectId);
}

void ScriptOpcodes_Duckman::opSetAdjustDnSfx(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(soundEffectId);
	// TODO _vm->setAdjustDnSfx(soundEffectId);
}

void ScriptOpcodes_Duckman::opStackPushRandom(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(maxValue);
	_vm->_stack->push(_vm->getRandom(maxValue) + 1);
}

void ScriptOpcodes_Duckman::opIfLte(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(rvalue);
	ARG_INT16(elseJumpOffs);
	int16 lvalue = _vm->_stack->pop();
	if (!(lvalue <= rvalue))
		opCall._deltaOfs += elseJumpOffs;
}

void ScriptOpcodes_Duckman::opIsCurrentSceneId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->_stack->push(_vm->getCurrentScene() == sceneId ? 1 : 0);
}

void ScriptOpcodes_Duckman::opIsActiveSceneId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	_vm->_stack->push(_vm->_activeScenes.isSceneActive(sceneId) ? 1 : 0);
}

void ScriptOpcodes_Duckman::opStackPop(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_stack->pop(); 
}

void ScriptOpcodes_Duckman::opStackDup(ScriptThread *scriptThread, OpCall &opCall) {
	int16 value = _vm->_stack->peek();
	_vm->_stack->push(value); 
}

void ScriptOpcodes_Duckman::opLoadSpecialCodeModule(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(specialCodeModuleId);
	_vm->_resSys->loadResource(specialCodeModuleId, 0, 0);
}

void ScriptOpcodes_Duckman::opStopActor(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->stopActor();
}

void ScriptOpcodes_Duckman::opSetActorUsePan(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_INT16(usePan)
	ARG_UINT32(objectId);
	Control *control = _vm->_dict->getObjectControl(objectId);
	control->setActorUsePan(usePan);
}

void ScriptOpcodes_Duckman::opStartAbortableThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(codeOffs);
	ARG_INT16(skipOffs);
	_vm->startAbortableThread(opCall._code + codeOffs,
		opCall._code + skipOffs, opCall._threadId);
}

void ScriptOpcodes_Duckman::opKillThread(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->_threads->killThread(threadId);
}

void ScriptOpcodes_Duckman::opSetSceneIdThreadId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	ARG_UINT32(threadId);
	_vm->setSceneIdThreadId(sceneId, threadId);
}

void ScriptOpcodes_Duckman::opStackPush0(ScriptThread *scriptThread, OpCall &opCall) {
	_vm->_stack->push(0);
}

void ScriptOpcodes_Duckman::opSetFontId(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(fontId);
	_vm->setCurrFontId(fontId);
}

void ScriptOpcodes_Duckman::opAddMenuKey(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(key);
	ARG_UINT32(threadId);
	// TODO _vm->addMenuKey(key, threadId);
}

void ScriptOpcodes_Duckman::opChangeSceneAll(ScriptThread *scriptThread, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(sceneId);
	ARG_UINT32(threadId);
	_vm->_input->discardButtons(0xFFFF);
	_vm->_prevSceneId = _vm->getCurrentScene();
	_vm->dumpActiveScenes(_vm->_globalSceneId, opCall._callerThreadId);
	_vm->enterScene(sceneId, opCall._callerThreadId);
	// TODO _vm->_gameStates->writeStates(_vm->_prevSceneId, sceneId, threadId);
	_vm->startAnonScriptThread(threadId, 0,
		scriptThread->_value8, scriptThread->_valueC, scriptThread->_value10);
}

#endif

} // End of namespace Illusions
