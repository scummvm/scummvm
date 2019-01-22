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
#include "dragons/scriptopcodes.h"
#include "dragons/actor.h"
#include "scriptopcodes.h"


namespace Dragons {
// ScriptOpCall

void ScriptOpCall::skip(uint size) {
	_code += size;
}

byte ScriptOpCall::readByte() {
	return *_code++;
}

int16 ScriptOpCall::readSint16() {
	int16 value = READ_LE_UINT16(_code);
	_code += 2;
	return value;
}

uint32 ScriptOpCall::readUint32() {
	uint32 value = READ_LE_UINT32(_code);
	_code += 4;
	return value;
}

// ScriptOpcodes

ScriptOpcodes::ScriptOpcodes(DragonsEngine *vm)
	: _vm(vm), _data_80071f5c(0) {
	initOpcodes();
}

ScriptOpcodes::~ScriptOpcodes() {
	freeOpcodes();
}

void ScriptOpcodes::execOpcode(ScriptOpCall &scriptOpCall) {
	if (!_opcodes[scriptOpCall._op])
		error("ScriptOpcodes::execOpcode() Unimplemented opcode %d (0x%X)", scriptOpCall._op, scriptOpCall._op);
	debug(3, "execScriptOpcode(%d) %s", scriptOpCall._op, _opcodeNames[scriptOpCall._op].c_str());
	(*_opcodes[scriptOpCall._op])(scriptOpCall);
}

typedef Common::Functor1Mem<ScriptOpCall&, void, ScriptOpcodes> ScriptOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new ScriptOpcodeI(this, &ScriptOpcodes::func); \
	_opcodeNames[op] = #func;

void ScriptOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < DRAGONS_NUM_SCRIPT_OPCODES; ++i) {
		_opcodes[i] = 0;
	}
	// Register opcodes
	OPCODE(1, opUnk1);
	OPCODE(19, opUnk13PropertiesRelated);
	OPCODE(31, opPlayMusic);

}

#undef OPCODE

void ScriptOpcodes::freeOpcodes() {
	for (uint i = 0; i < DRAGONS_NUM_SCRIPT_OPCODES; ++i) {
		delete _opcodes[i];
	}
}

void ScriptOpcodes::updateReturn(ScriptOpCall &scriptOpCall, uint16 size) {
	// scriptOpCall._deltaOfs = size * 2 + 2;
}


void ScriptOpcodes::runScript(ScriptOpCall &scriptOpCall) {
	scriptOpCall._field8 = 0;
	scriptOpCall._result = 0;
	_data_80071f5c = 0;
	executeScriptLoop(scriptOpCall);
}

void ScriptOpcodes::executeScriptLoop(ScriptOpCall &scriptOpCall) {

	if (scriptOpCall._code >= scriptOpCall._codeEnd || scriptOpCall._result & 1) {
		return;
	}

	if (Dragons::getEngine()->isFlagSet(Dragons::ENGINE_FLAG_100000)) {
		return;
	}

	if (Dragons::getEngine()->isFlagSet(Dragons::ENGINE_FLAG_80000)) {
		//TODO
//		if (IsPressedStart(0)) {
//			Dragons::getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
//		}
	}

	uint16 opcode = READ_LE_INT16(scriptOpCall._code);

	scriptOpCall._op = (byte) opcode;
	if (opcode < DRAGONS_NUM_SCRIPT_OPCODES) {
		execOpcode(scriptOpCall);
	}

	while (scriptOpCall._code < scriptOpCall._codeEnd && !(scriptOpCall._result & 1) && _data_80071f5c == 0) {

		if (Dragons::getEngine()->isFlagSet(Dragons::ENGINE_FLAG_100000)) {
			return;
		}

		if (Dragons::getEngine()->isFlagSet(Dragons::ENGINE_FLAG_80000)) {
			//TODO
//		if (IsPressedStart(0)) {
//			Dragons::getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
//		}
		}

		uint16 opcode = READ_LE_INT16(scriptOpCall._code);

		if (opcode >= DRAGONS_NUM_SCRIPT_OPCODES) {
			return; //TODO should continue here.
		}
		scriptOpCall._op = (byte) opcode;
		execOpcode(scriptOpCall);
	}
}

// Opcodes

void ScriptOpcodes::opUnk1(ScriptOpCall &scriptOpCall) {
//	ARG_INT16(framePointer);
//	debug(3, "set frame pointer %X", framePointer);
//	actor->loadFrame((uint16)framePointer);
//	actor->flags |= Dragons::ACTOR_FLAG_2;
//	actor->sequenceTimer = actor->field_c;
//	updateReturn(scriptOpCall, 1);
}

void ScriptOpcodes::opPlayMusic(ScriptOpCall &scriptOpCall) {
	byte *code = scriptOpCall._code;
	scriptOpCall._code += 4;
	if (scriptOpCall._field8 == 0) {
		//TODO play music here.
	}
}

void ScriptOpcodes::opUnk13PropertiesRelated(ScriptOpCall &scriptOpCall) {
	//TODO sub_8002f02c()
}

} // End of namespace Dragons
