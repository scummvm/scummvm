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
#include "illusions/actorresource.h"
#include "illusions/scriptopcodes.h"

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
	debug("execOpcode(%d)", opCall._op);
	(*_opcodes[opCall._op])(control, opCall);
}

typedef Common::Functor2Mem<Control*, OpCall&, void, SequenceOpcodes> SequenceOpcodeI;
#define OPCODE(op, func) _opcodes[op] = new SequenceOpcodeI(this, &SequenceOpcodes::func);

void SequenceOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < 256; ++i)
		_opcodes[i] = 0;
	// Register opcodes
	OPCODE(2, opSetFrameIndex);
	OPCODE(3, opEndSequence);
	OPCODE(5, opSetRandomFrameDelay);
	OPCODE(6, opSetFrameSpeed);
	OPCODE(7, opJump);
	OPCODE(9, opGotoSequence);
	OPCODE(11, opBeginLoop);
	OPCODE(12, opNextLoop);
	OPCODE(15, opJumpIfNotFacing);
	OPCODE(28, opNotifyThreadId1);
	OPCODE(29, opSetPathCtrY);
	OPCODE(33, opSetPathWalkPoints);
	OPCODE(36, opSetScaleLayer);
	OPCODE(38, opSetPathWalkRects);
	OPCODE(39, opSetPriority);
	OPCODE(40, opSetPriorityLayer);
	OPCODE(50, opPlaySound);
	OPCODE(51, opStopSound);
}

#undef OPCODE

void SequenceOpcodes::freeOpcodes() {
	for (uint i = 0; i < 256; ++i)
		delete _opcodes[i];
}

// Opcodes

// Convenience macros
#define	ARG_SKIP(x) opCall.skip(x); 
#define ARG_INT16(name) int16 name = opCall.readSint16(); debug("ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = opCall.readUint32(); debug("ARG_UINT32(" #name " = %08X)", name);

void SequenceOpcodes::opSetFrameIndex(Control *control, OpCall &opCall) {
	ARG_INT16(frameIndex);
	if (control->_actor->_flags & 0x80) {
		debug("opSetFrameIndex TODO");
		/* TODO
		v9 = actor->field30;
		if (*(_WORD *)v9) {
			LOWORD(flag) = *(_WORD *)v9;
			v6 = v6 + flag - 1;
			actor->field30 = v9 + 2;
		} else {
			actor->flags &= 0xFF7Fu;
			v10 = actor->notifyThreadId1;
			actor->field30 = 0;
			actor->notifyThreadId2 = 0;
			if (v10) {
				actor->notifyThreadId1 = 0;
				ThreadList_notifyId__(v10);
			}
			actorDead = 1;
			breakInner = 1;
		}
		*/
	}
	control->_actor->_flags &= ~0x0100;
	if (control->_actor->_flags & 0x8000) {
		control->appearActor();
		control->_actor->_flags &= ~0x800;
	}
	control->_actor->_newFrameIndex = frameIndex;
}

void SequenceOpcodes::opEndSequence(Control *control, OpCall &opCall) {
	control->_actor->_seqCodeIp = 0;
	if (control->_actor->_flags & 0x0800) {
		control->_actor->_flags &= ~0x0800;
		control->_actor->_frames = 0;
		control->_actor->_frameIndex = 0;
		control->_actor->_newFrameIndex = 0;
		// TODO _vm->_resSys->unloadResourceById(control->_actor->_sequenceId);
	}
	_vm->notifyThreadId(control->_actor->_notifyThreadId1);
	opCall._result = 1;
}

void SequenceOpcodes::opSetRandomFrameDelay(Control *control, OpCall &opCall) {
	ARG_INT16(minFrameDelay);
	ARG_INT16(maxFrameDelay);
	control->_actor->_seqCodeValue3 += 0;//DEBUG minFrameDelay + _vm->getRandom(maxFrameDelay);
}

void SequenceOpcodes::opSetFrameSpeed(Control *control, OpCall &opCall) {
	ARG_INT16(frameSpeed);
	control->_actor->_seqCodeValue2 = frameSpeed;
}

void SequenceOpcodes::opJump(Control *control, OpCall &opCall) {
	ARG_INT16(jumpOffs);
	opCall._deltaOfs += jumpOffs;
}

void SequenceOpcodes::opGotoSequence(Control *control, OpCall &opCall) {
	ARG_SKIP(2);
	ARG_UINT32(nextSequenceId);
	uint32 notifyThreadId1 = control->_actor->_notifyThreadId1;
	control->clearNotifyThreadId1();
	if (false/*TODO control->_actor->_pathNode*/) {
		control->startSequenceActor(nextSequenceId, 1, notifyThreadId1);
	} else {
		control->startSequenceActor(nextSequenceId, 2, notifyThreadId1);
	}
	opCall._deltaOfs = 0;
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

void SequenceOpcodes::opJumpIfNotFacing(Control *control, OpCall &opCall) {
	ARG_INT16(facing);
	ARG_INT16(jumpOffs);
	if (!(control->_actor->_facing & facing))
		opCall._deltaOfs += jumpOffs;
}

void SequenceOpcodes::opNotifyThreadId1(Control *control, OpCall &opCall) {
	_vm->notifyThreadId(control->_actor->_notifyThreadId1);
}

void SequenceOpcodes::opSetPathCtrY(Control *control, OpCall &opCall) {
	ARG_INT16(pathCtrY);
	control->_actor->_pathCtrY = pathCtrY;
}

void SequenceOpcodes::opSetPathWalkPoints(Control *control, OpCall &opCall) {
	ARG_INT16(pathWalkPointsIndex);
	BackgroundResource *bgRes = _vm->_backgroundItems->getActiveBgResource();
	control->_actor->_flags |= 2;
	// TODO control->_actor->_pathWalkPoints = bgRes->getPathWalkPoints(pathWalkPointsIndex - 1);
}

void SequenceOpcodes::opSetScaleLayer(Control *control, OpCall &opCall) {
	ARG_INT16(scaleLayerIndex);
	BackgroundResource *bgRes = _vm->_backgroundItems->getActiveBgResource();
	control->_actor->_flags |= 4;
	control->_actor->_scaleLayer = bgRes->getScaleLayer(scaleLayerIndex - 1);
	int scale = control->_actor->_scaleLayer->getScale(control->_actor->_position);
	control->setActorScale(scale);
}

void SequenceOpcodes::opSetPathWalkRects(Control *control, OpCall &opCall) {
	ARG_INT16(pathWalkRectsIndex);
	BackgroundResource *bgRes = _vm->_backgroundItems->getActiveBgResource();
	control->_actor->_flags |= 0x10;
	// TODO control->_actor->_pathWalkRects = bgRes->getPathWalkRects(pathWalkRectsIndex - 1);
}

void SequenceOpcodes::opSetPriority(Control *control, OpCall &opCall) {
	ARG_INT16(priority);
	control->_actor->_flags &= ~8;
	control->setPriority(priority);
}

void SequenceOpcodes::opSetPriorityLayer(Control *control, OpCall &opCall) {
	ARG_INT16(priorityLayerIndex);
	BackgroundResource *bgRes = _vm->_backgroundItems->getActiveBgResource();
	control->_actor->_flags |= 8;
	control->_actor->_priorityLayer = bgRes->getPriorityLayer(priorityLayerIndex - 1);
	int priority = control->_actor->_priorityLayer->getPriority(control->_actor->_position);
	control->setPriority(priority);
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
	// TODO _vm->startSound(soundEffectId, volume, pan);
}

void SequenceOpcodes::opStopSound(Control *control, OpCall &opCall) {
	ARG_UINT32(soundEffectId);
	// TODO _vm->stopSound(soundEffectId);
}

} // End of namespace Illusions
