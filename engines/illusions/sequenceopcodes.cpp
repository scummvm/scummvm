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

typedef Common::Functor2Mem<ScriptThread*, OpCall&, void, SequenceOpcodes> SequenceOpcodeI;
#define OPCODE(op, func) _opcodes[op] = new SequenceOpcodeI(this, &SequenceOpcodes::func);

void SequenceOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < 256; ++i)
		_opcodes[i] = 0;
	// Register opcodes
	//OPCODE(42, opIncBlockCounter);
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
#define ARG_UINT32(name) uint32 name = opCall.readUint32(); debug("ARG_UINT32(" #name " = %d)", name);

} // End of namespace Illusions
