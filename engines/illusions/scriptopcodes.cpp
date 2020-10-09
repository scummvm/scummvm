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
#include "illusions/threads/scriptthread.h"

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
	debug(2, "execOpcode([%08X] %d) %s", opCall._callerThreadId, opCall._op, _opcodeNames[opCall._op].c_str());
	(*_opcodes[opCall._op])(scriptThread, opCall);
}

} // End of namespace Illusions
