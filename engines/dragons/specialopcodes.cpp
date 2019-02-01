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
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/specialopcodes.h"
#include "dragons/actor.h"
#include "specialopcodes.h"


namespace Dragons {

// SpecialOpcodes

SpecialOpcodes::SpecialOpcodes(DragonsEngine *vm)
	: _vm(vm) {
	initOpcodes();
}

SpecialOpcodes::~SpecialOpcodes() {
	freeOpcodes();
}

void SpecialOpcodes::run(int16 op) {
	if (!_opcodes[op])
		error("SpecialOpcodes::execOpcode() Unimplemented opcode %d (0x%X)", op, op);
	debug(3, "run(%d) %s", op, _opcodeNames[op].c_str());
	(*_opcodes[op])();
}

typedef Common::Functor0Mem<void, SpecialOpcodes> SpecialOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new SpecialOpcodeI(this, &SpecialOpcodes::func); \
	_opcodeNames[op] = #func;

void SpecialOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < DRAGONS_NUM_SPECIAL_OPCODES; ++i) {
		_opcodes[i] = 0;
	}
	// Register opcodes
	// OPCODE(1, opUnk1);
	OPCODE(3, spcClearEngineFlag10);
	OPCODE(9, spcUnk9);
	OPCODE(20, spcClearEngineFlag8);

	OPCODE(0x54, spcSetEngineFlag0x4000000);
	OPCODE(0x55, spcSetCursorSequenceIdToZero);
}

#undef OPCODE

void SpecialOpcodes::freeOpcodes() {
	for (uint i = 0; i < DRAGONS_NUM_SPECIAL_OPCODES; ++i) {
		delete _opcodes[i];
	}
}

// Opcodes

void SpecialOpcodes::spcClearEngineFlag10() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_10);
}

void SpecialOpcodes::spcUnk9() {
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	assert(flicker);
	flicker->field_1a_flags_maybe |= 0x20;
	assert(flicker->actor);
	flicker->actor->flags |= Dragons::ACTOR_FLAG_100;
	flicker->actor->priorityLayer = 0;
	_vm->getINI(1)->field_1a_flags_maybe |= 0x20;
}

void SpecialOpcodes::spcClearEngineFlag8() {
	_vm->clearFlags(Dragons::ENGINE_FLAG_8);
}

void SpecialOpcodes::spcSetEngineFlag0x4000000() {
	_vm->setFlags(Dragons::ENGINE_FLAG_4000000);
}

void SpecialOpcodes::spcSetCursorSequenceIdToZero() {
	_vm->_cursorSequenceID = 0;
}


} // End of namespace Dragons
