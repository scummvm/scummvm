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
#include "illusions/sequenceopcodes.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/resources/actorresource.h"
#include "illusions/screen.h"
#include "illusions/scriptopcodes.h"
#include "illusions/sound.h"

namespace Illusions {

// SequenceOpcodes

SequenceOpcodes::SequenceOpcodes(IllusionsEngine *vm)
	: _vm(vm) {
	initOpcodes();
}

SequenceOpcodes::~SequenceOpcodes() {
	freeOpcodes();
}

void SequenceOpcodes::execOpcode(Control *control, OpCall &opCall) {
	if (!_opcodes[opCall._op])
		error("SequenceOpcodes::execOpcode() Unimplemented opcode %d", opCall._op);
	debug(3, "execSequenceOpcode(%d) %s objectID: %08X", opCall._op, _opcodeNames[opCall._op].c_str(), control->_objectId);
	(*_opcodes[opCall._op])(control, opCall);
}

typedef Common::Functor2Mem<Control*, OpCall&, void, SequenceOpcodes> SequenceOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new SequenceOpcodeI(this, &SequenceOpcodes::func); \
	_opcodeNames[op] = #func;

void SequenceOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < 256; ++i) {
		_opcodes[i] = 0;
	}
	// Register opcodes
	OPCODE(1, opYield);
	OPCODE(2, opSetFrameIndex);
	OPCODE(3, opEndSequence);
	OPCODE(4, opIncFrameDelay);
	OPCODE(5, opSetRandomFrameDelay);
	OPCODE(6, opSetFrameSpeed);
	OPCODE(7, opJump);
	OPCODE(8, opJumpRandom);
	OPCODE(9, opGotoSequence);
	OPCODE(10, opStartForeignSequence);
	OPCODE(11, opBeginLoop);
	OPCODE(12, opNextLoop);
	OPCODE(13, opSetActorIndex);
	OPCODE(14, opSwitchActorIndex);
	OPCODE(15, opSwitchFacing);
	OPCODE(16, opAppearActor);
	OPCODE(17, opDisappearActor);
	OPCODE(18, opAppearForeignActor);
	OPCODE(19, opDisappearForeignActor);
	OPCODE(20, opSetNamedPointPosition);
	OPCODE(21, opMoveDelta);
	// 22-24 unused in Duckman, CHECKME BBDOU
	OPCODE(25, opFaceActor);
	// 26-27 unused in Duckman, CHECKME BBDOU
	OPCODE(28, opNotifyThreadId1);
	OPCODE(29, opSetPathCtrY);
	// 30-31 unused in Duckman, CHECKME BBDOU
	OPCODE(32, opDisablePathWalkPoints);
	OPCODE(33, opSetPathWalkPoints);
	OPCODE(34, opDisableAutoScale);
	OPCODE(35, opSetScale);
	OPCODE(36, opSetScaleLayer);
	OPCODE(37, opDeactivatePathWalkRects);
	OPCODE(38, opSetPathWalkRects);
	OPCODE(39, opSetPriority);
	OPCODE(40, opSetPriorityLayer);
	OPCODE(41, opDisableAutoRegionLayer);
	OPCODE(42, opSetRegionLayer);
	// 43-47 unused in Duckman, CHECKME BBDOU
	OPCODE(48, opSetPalette);
	OPCODE(49, opShiftPalette);
	OPCODE(50, opPlaySound);
	OPCODE(51, opStopSound);
	OPCODE(52, opStartScriptThread);
	OPCODE(53, opPlaceSubActor);
	OPCODE(54, opStartSubSequence);
	OPCODE(55, opStopSubSequence);
}

#undef OPCODE

void SequenceOpcodes::freeOpcodes() {
	for (uint i = 0; i < 256; ++i) {
		delete _opcodes[i];
	}
}

// Opcodes

void SequenceOpcodes::opYield(Control *control, OpCall &opCall) {
	opCall._result = 2;
}

void SequenceOpcodes::opSetFrameIndex(Control *control, OpCall &opCall) {
	ARG_INT16(frameIndex);
	if (control->_actor->_flags & Illusions::ACTOR_FLAG_80) {
		int16 frameIncr = READ_LE_UINT16(control->_actor->_entryTblPtr);
		if (frameIncr) {
			frameIndex += frameIncr - 1;
			control->_actor->_entryTblPtr += 2;
		} else {
			control->_actor->_flags &= ~Illusions::ACTOR_FLAG_80;
			control->_actor->_entryTblPtr = 0;
			control->_actor->_notifyThreadId2 = 0;
			_vm->notifyThreadId(control->_actor->_notifyThreadId1);
			opCall._result = 1;
		}
	}
	control->_actor->_flags &= ~Illusions::ACTOR_FLAG_100;
	if (control->_actor->_flags & Illusions::ACTOR_FLAG_8000) {
		control->appearActor();
		control->_actor->_flags &= ~Illusions::ACTOR_FLAG_8000;
	}
	control->_actor->_newFrameIndex = frameIndex;
}

void SequenceOpcodes::opEndSequence(Control *control, OpCall &opCall) {
	control->_actor->_seqCodeIp = 0;
	if (control->_actor->_flags & Illusions::ACTOR_FLAG_800) {
		control->_actor->_flags &= ~Illusions::ACTOR_FLAG_800;
		control->_actor->_frames = 0;
		control->_actor->_frameIndex = 0;
		control->_actor->_newFrameIndex = 0;
		_vm->_resSys->unloadResourceById(control->_actor->_sequenceId);
	}
	_vm->notifyThreadId(control->_actor->_notifyThreadId1);
	opCall._result = 1;
}

void SequenceOpcodes::opIncFrameDelay(Control *control, OpCall &opCall) {
	ARG_INT16(frameDelayIncr);
	control->_actor->_seqCodeValue3 += frameDelayIncr;
	opCall._result = 2;
}

void SequenceOpcodes::opSetRandomFrameDelay(Control *control, OpCall &opCall) {
	ARG_INT16(minFrameDelay);
	ARG_INT16(maxFrameDelay);
	control->_actor->_seqCodeValue3 += minFrameDelay + _vm->getRandom(maxFrameDelay);
	opCall._result = 2;
}

void SequenceOpcodes::opSetFrameSpeed(Control *control, OpCall &opCall) {
	ARG_INT16(frameSpeed);
	control->_actor->_seqCodeValue2 = frameSpeed;
}

void SequenceOpcodes::opJump(Control *control, OpCall &opCall) {
	ARG_INT16(jumpOffs);
	opCall._deltaOfs += jumpOffs;
}

void SequenceOpcodes::opJumpRandom(Control *control, OpCall &opCall) {
	ARG_INT16(count);
	ARG_SKIP(_vm->getRandom(count) * 2);
	ARG_INT16(jumpOffs);
	opCall._deltaOfs += jumpOffs;
}

void SequenceOpcodes::opGotoSequence(Control *control, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(nextSequenceId);
	uint32 notifyThreadId1 = control->_actor->_notifyThreadId1;
	control->clearNotifyThreadId1();
	if (control->_actor->_pathNode) {
		control->startSequenceActor(nextSequenceId, 1, notifyThreadId1);
	} else {
		control->startSequenceActor(nextSequenceId, 2, notifyThreadId1);
	}
	opCall._deltaOfs = 0;
}

void SequenceOpcodes::opStartForeignSequence(Control *control, OpCall &opCall) {
	ARG_INT16(foreignObjectNum);
	ARG_UINT32(sequenceId);
	Control *foreignControl = _vm->_dict->getObjectControl(foreignObjectNum | 0x40000);
	foreignControl->startSequenceActor(sequenceId, 2, 0);
}

void SequenceOpcodes::opBeginLoop(Control *control, OpCall &opCall) {
	ARG_INT16(loopCount);
	control->_actor->pushSequenceStack(loopCount);
}

void SequenceOpcodes::opNextLoop(Control *control, OpCall &opCall) {
	ARG_INT16(jumpOffs);
	int16 currLoopCount = control->_actor->popSequenceStack();
	if (currLoopCount > 0) {
		control->_actor->pushSequenceStack(currLoopCount - 1);
		opCall._deltaOfs = -jumpOffs;
	}
}

void SequenceOpcodes::opSetActorIndex(Control *control, OpCall &opCall) {
	ARG_BYTE(actorIndex);
	control->setActorIndex(actorIndex);
}

void SequenceOpcodes::opSwitchActorIndex(Control *control, OpCall &opCall) {
	ARG_INT16(actorIndex);
	ARG_INT16(jumpOffs);
	if (control->_actor->_actorIndex != actorIndex)
		opCall._deltaOfs += jumpOffs;
}

void SequenceOpcodes::opSwitchFacing(Control *control, OpCall &opCall) {
	ARG_INT16(facing);
	ARG_INT16(jumpOffs);
	if (!(control->_actor->_facing & facing))
		opCall._deltaOfs += jumpOffs;
}

void SequenceOpcodes::opAppearActor(Control *control, OpCall &opCall) {
	control->appearActor();
}

void SequenceOpcodes::opDisappearActor(Control *control, OpCall &opCall) {
	control->disappearActor();
	control->_actor->_newFrameIndex = 0;
}

void SequenceOpcodes::opAppearForeignActor(Control *control, OpCall &opCall) {
	ARG_INT16(foreignObjectNum);
	Control *foreignControl = _vm->_dict->getObjectControl(foreignObjectNum | 0x40000);
	if (!foreignControl) {
		Common::Point pos = _vm->getNamedPointPosition(_vm->getGameId() == kGameIdDuckman ? 0x00070001 : 0x00070023);
		_vm->_controls->placeActor(0x00050001, pos, 0x00060001, foreignObjectNum | 0x40000, 0);
		foreignControl = _vm->_dict->getObjectControl(foreignObjectNum | 0x40000);
	}
	foreignControl->appearActor();
}

void SequenceOpcodes::opDisappearForeignActor(Control *control, OpCall &opCall) {
	ARG_INT16(foreignObjectNum);
	Control *foreignControl = _vm->_dict->getObjectControl(foreignObjectNum | 0x40000);
	foreignControl->disappearActor();
}

void SequenceOpcodes::opSetNamedPointPosition(Control *control, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(namedPointId);
	control->_actor->_position = _vm->getNamedPointPosition(namedPointId);
}

void SequenceOpcodes::opMoveDelta(Control *control, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_INT16(deltaX);
	ARG_INT16(deltaY);
	control->_actor->_position.x += deltaX;
	control->_actor->_position.y += deltaY;
}

void SequenceOpcodes::opFaceActor(Control *control, OpCall &opCall) {
	ARG_INT16(facing);
	control->_actor->_facing = facing;
}

void SequenceOpcodes::opNotifyThreadId1(Control *control, OpCall &opCall) {
	_vm->notifyThreadId(control->_actor->_notifyThreadId1);
}

void SequenceOpcodes::opSetPathCtrY(Control *control, OpCall &opCall) {
	ARG_INT16(pathCtrY);
	control->_actor->_pathCtrY = pathCtrY;
}

void SequenceOpcodes::opDisablePathWalkPoints(Control *control, OpCall &opCall) {
	control->_actor->_flags &= ~Illusions::ACTOR_FLAG_HAS_WALK_POINTS;
}

void SequenceOpcodes::opSetPathWalkPoints(Control *control, OpCall &opCall) {
	ARG_INT16(pathWalkPointsIndex);
	BackgroundResource *bgRes = _vm->_backgroundInstances->getActiveBgResource();
	control->_actor->_flags |= Illusions::ACTOR_FLAG_HAS_WALK_POINTS;
	control->_actor->_pathWalkPoints = bgRes->getPathWalkPoints(pathWalkPointsIndex - 1);
}

void SequenceOpcodes::opDisableAutoScale(Control *control, OpCall &opCall) {
	// Keep current scale but don't autoscale
	control->_actor->_flags &= ~Illusions::ACTOR_FLAG_SCALED;
}

void SequenceOpcodes::opSetScale(Control *control, OpCall &opCall) {
	ARG_INT16(scale);
	control->_actor->_flags &= ~Illusions::ACTOR_FLAG_SCALED;
	control->setActorScale(scale);
}

void SequenceOpcodes::opSetScaleLayer(Control *control, OpCall &opCall) {
	ARG_INT16(scaleLayerIndex);
	BackgroundResource *bgRes = _vm->_backgroundInstances->getActiveBgResource();
	control->_actor->_flags |= Illusions::ACTOR_FLAG_SCALED;
	control->_actor->_scaleLayer = bgRes->getScaleLayer(scaleLayerIndex - 1);
	int scale = control->_actor->_scaleLayer->getScale(control->_actor->_position);
	control->setActorScale(scale);
}

void SequenceOpcodes::opDeactivatePathWalkRects(Control *control, OpCall &opCall) {
	control->_actor->_flags &= ~Illusions::ACTOR_FLAG_HAS_WALK_RECTS;
}

void SequenceOpcodes::opSetPathWalkRects(Control *control, OpCall &opCall) {
	ARG_INT16(pathWalkRectsIndex);
	BackgroundResource *bgRes = _vm->_backgroundInstances->getActiveBgResource();
	control->_actor->_flags |= Illusions::ACTOR_FLAG_HAS_WALK_RECTS;
	control->_actor->_pathWalkRects = bgRes->getPathWalkRects(pathWalkRectsIndex - 1);
}

void SequenceOpcodes::opSetPriority(Control *control, OpCall &opCall) {
	ARG_INT16(priority);
	control->_actor->_flags &= ~Illusions::ACTOR_FLAG_PRIORITY;
	control->setPriority(priority);
}

void SequenceOpcodes::opSetPriorityLayer(Control *control, OpCall &opCall) {
	ARG_INT16(priorityLayerIndex);
	BackgroundResource *bgRes = _vm->_backgroundInstances->getActiveBgResource();
	control->_actor->_flags |= Illusions::ACTOR_FLAG_PRIORITY;
	control->_actor->_priorityLayer = bgRes->getPriorityLayer(priorityLayerIndex - 1);
	int priority = control->_actor->_priorityLayer->getPriority(control->_actor->_position);
	control->setPriority(priority);
}

void SequenceOpcodes::opDisableAutoRegionLayer(Control *control, OpCall &opCall) {
	control->_actor->_flags &= ~Illusions::ACTOR_FLAG_REGION;
}

void SequenceOpcodes::opSetRegionLayer(Control *control, OpCall &opCall) {
	ARG_INT16(regionLayerIndex);
	BackgroundResource *bgRes = _vm->_backgroundInstances->getActiveBgResource();
	control->_actor->_flags |= Illusions::ACTOR_FLAG_REGION;
	control->_actor->_regionLayer = bgRes->getRegionLayer(regionLayerIndex - 1);
}

void SequenceOpcodes::opSetPalette(Control *control, OpCall &opCall) {
	ARG_INT16(paletteIndex);
	ARG_BYTE(fromIndex);
	BackgroundResource *bgRes = _vm->_backgroundInstances->getActiveBgResource();
	Palette *palette = bgRes->getPalette(paletteIndex - 1);
	_vm->_screenPalette->setPalette(palette->_palette, fromIndex, palette->_count);
}

void SequenceOpcodes::opShiftPalette(Control *control, OpCall &opCall) {
	ARG_INT16(fromIndex);
	ARG_INT16(toIndex);
	_vm->_screenPalette->shiftPalette(fromIndex, toIndex);
}

void SequenceOpcodes::opPlaySound(Control *control, OpCall &opCall) {
	ARG_INT16(flags);
	ARG_INT16(volume);
	ARG_INT16(pan);
	ARG_UINT32(soundEffectId);
	if (!(flags & 1))
		volume = 255;
	if (!(flags & 2))
		pan = _vm->convertPanXCoord(control->_actor->_position.x);
	_vm->_soundMan->playSound(soundEffectId, volume, pan);
}

void SequenceOpcodes::opStopSound(Control *control, OpCall &opCall) {
	ARG_UINT32(soundEffectId);
	_vm->_soundMan->stopSound(soundEffectId);
}

void SequenceOpcodes::opStartScriptThread(Control *control, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(threadId);
	_vm->startScriptThreadSimple(threadId, 0);
}

void SequenceOpcodes::opPlaceSubActor(Control *control, OpCall &opCall) {
	ARG_INT16(linkIndex);
	ARG_UINT32(actorTypeId);
	ARG_UINT32(sequenceId);
	_vm->_controls->placeSubActor(control->_objectId, linkIndex, actorTypeId, sequenceId);
}

void SequenceOpcodes::opStartSubSequence(Control *control, OpCall &opCall) {
	ARG_INT16(linkIndex);
	ARG_UINT32(sequenceId);
	control->startSubSequence(linkIndex, sequenceId);
}

void SequenceOpcodes::opStopSubSequence(Control *control, OpCall &opCall) {
	ARG_INT16(linkIndex);
	control->stopSubSequence(linkIndex);
}

} // End of namespace Illusions
