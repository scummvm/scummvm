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

#include "dragons/dragons.h"
#include "dragons/sequenceopcodes.h"
#include "dragons/actor.h"
#include "sequenceopcodes.h"


namespace Dragons {
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

// SequenceOpcodes

SequenceOpcodes::SequenceOpcodes(DragonsEngine *vm)
	: _vm(vm) {
	initOpcodes();
}

SequenceOpcodes::~SequenceOpcodes() {
	freeOpcodes();
}

void SequenceOpcodes::execOpcode(Actor *control, OpCall &opCall) {
	if (!_opcodes[opCall._op])
		error("SequenceOpcodes::execOpcode() Unimplemented opcode %d", opCall._op);
	debug(3, "execSequenceOpcode(%d) %s", opCall._op, _opcodeNames[opCall._op].c_str());
	(*_opcodes[opCall._op])(control, opCall);
}

typedef Common::Functor2Mem<Actor*, OpCall&, void, SequenceOpcodes> SequenceOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new SequenceOpcodeI(this, &SequenceOpcodes::func); \
	_opcodeNames[op] = #func;

void SequenceOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < DRAGONS_NUM_SEQ_OPCODES; ++i) {
		_opcodes[i] = 0;
	}
	// Register opcodes
	OPCODE(1, opSetFramePointer);
	OPCODE(2, opSetFramePointerAndStop);


	OPCODE(4, opSetFieldC);

	OPCODE(9, opSetActorFlag4AndStop);

	OPCODE(11, opSetActorFlags404);
	OPCODE(17, opPlaySound);

}

#undef OPCODE

void SequenceOpcodes::freeOpcodes() {
	for (uint i = 0; i < DRAGONS_NUM_SEQ_OPCODES; ++i) {
		delete _opcodes[i];
	}
}

void SequenceOpcodes::updateReturn(OpCall &opCall, uint16 size) {
	opCall._deltaOfs = size * 2 + 2;
}

// Opcodes

void SequenceOpcodes::opSetFramePointer(Actor *actor, OpCall &opCall) {
	ARG_INT16(framePointer);
	debug(3, "set frame pointer %X", framePointer);
	actor->loadFrame((uint16)framePointer);
	actor->flags |= Dragons::ACTOR_FLAG_2;
	actor->sequenceTimer = actor->field_c;
	updateReturn(opCall, 1);
}

void SequenceOpcodes::opSetFramePointerAndStop(Actor *actor, OpCall &opCall) {
	opSetFramePointer(actor, opCall);
	opCall._result = 0;
}

void SequenceOpcodes::opSetFieldC(Actor *actor, OpCall &opCall) {
	ARG_INT16(newFieldC);
	actor->field_c = (uint16)newFieldC;
	updateReturn(opCall, 1);
}

void SequenceOpcodes::opSetActorFlag4AndStop(Actor *actor, OpCall &opCall) {
	actor->flags |= Dragons::ACTOR_FLAG_4;
	opCall._deltaOfs = 0;
	opCall._result = 0;
	//updateReturn(opCall, 1);
}

void SequenceOpcodes::opSetActorFlags404(Actor *actor, OpCall &opCall) {
	actor->flags |= (Dragons::ACTOR_FLAG_4 | Dragons::ACTOR_FLAG_400 );
	updateReturn(opCall, 1);
}
void SequenceOpcodes::opPlaySound(Actor *actor, OpCall &opCall) {
	ARG_INT16(soundId);
	// TODO play sound here.
	updateReturn(opCall, 1);
}


//void SequenceOpcodes::opYield(Control *control, OpCall &opCall) {
//	opCall._result = 2;
//}
//
//void SequenceOpcodes::opStopSubSequence(Control *control, OpCall &opCall) {
//	ARG_INT16(linkIndex);
//	control->stopSubSequence(linkIndex);
//}

} // End of namespace Dragons
