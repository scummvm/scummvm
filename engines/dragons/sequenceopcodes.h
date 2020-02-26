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

#ifndef DRAGONS_SEQUENCEOPCODES_H
#define DRAGONS_SEQUENCEOPCODES_H

#include "common/func.h"
#include "common/str.h"


namespace Dragons {

#define DRAGONS_NUM_SEQ_OPCODES 22
class DragonsEngine;
class Actor;

struct OpCall {
	byte _op;
	byte _opSize;
	int32 _deltaOfs;
	byte *_code;
	int _result;

	void skip(uint size);
	byte readByte();
	int16 readSint16();
	uint32 readUint32();
};

// Convenience macros
#define ARG_SKIP(x) opCall.skip(x);
#define ARG_BYTE(name) byte name = opCall.readByte(); debug(5, "ARG_BYTE(" #name " = %d)", name);
#define ARG_INT8(name) int8 name = opCall.readByte(); debug(5, "ARG_INT8(" #name " = %d)", name);
#define ARG_INT16(name) int16 name = opCall.readSint16(); debug(5, "ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = opCall.readUint32(); debug(5, "ARG_UINT32(" #name " = %08X)", name);

typedef Common::Functor2<Actor*, OpCall&, void> SequenceOpcode;

class SequenceOpcodes {
public:
	SequenceOpcodes(DragonsEngine *vm);
	~SequenceOpcodes();
	void execOpcode(Actor *actor, OpCall &opCall);

protected:
	DragonsEngine *_vm;
	SequenceOpcode *_opcodes[DRAGONS_NUM_SEQ_OPCODES];
	Common::String _opcodeNames[DRAGONS_NUM_SEQ_OPCODES];
	void initOpcodes();
	void freeOpcodes();
	void updateReturn(OpCall &opCall, uint16 size);

	// Opcodes
	void opSetFramePointer(Actor *actor, OpCall &opCall);
	void opSetFramePointerAndStop(Actor *actor, OpCall &opCall);
	void opJmp(Actor *actor, OpCall &opCall);
	void opSetSequenceTimerStartValue(Actor *actor, OpCall &opCall);
	void opSetSequenceTimer(Actor *actor, OpCall &opCall);
	void opUpdateXYResetSeqTimer(Actor *actor, OpCall &opCall);
	void opUpdateXYResetSeqTimerAndStop(Actor *actor, OpCall &opCall);

	void opSetActorFlag4AndStop(Actor *actor, OpCall &opCall);
	void opSetActorFlags404(Actor *actor, OpCall &opCall);
	void opClearActorFlag400(Actor *actor, OpCall &opCall);
	void opChangeSequence(Actor *actor, OpCall &opCall);

	void opSetField7a(Actor *actor, OpCall &opCall);
	void opUpdateFlags(Actor *actor, OpCall &opCall);
	void opPlaySound(Actor *actor, OpCall &opCall);
	void opSetXY(Actor *actor, OpCall &opCall);
	void opSetXYAndStop(Actor *actor, OpCall &opCall);

};

} // End of namespace Dragons

#endif // DRAGONS_SEQUENCEOPCODES_H
