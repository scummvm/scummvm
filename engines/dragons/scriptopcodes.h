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

#ifndef DRAGONS_SCRIPTOPCODES_H
#define DRAGONS_SCRIPTOPCODES_H

#include "common/func.h"
#include "common/str.h"


namespace Dragons {

#define DRAGONS_NUM_SCRIPT_OPCODES 0x22
class DragonsEngine;

struct ScriptOpCall {
	byte _op;
	byte *_code;
	byte *_codeEnd;
	int _field8;
	int _result;
	void skip(uint size);
	byte readByte();
	int16 readSint16();
	uint32 readUint32();
};

// Convenience macros
#define ARG_SKIP(x) opCall.skip(x);
#define ARG_BYTE(name) byte name = scriptOpCall.readByte(); debug(5, "ARG_BYTE(" #name " = %d)", name);
#define ARG_INT8(name) int8 name = scriptOpCall.readByte(); debug(5, "ARG_INT8(" #name " = %d)", name);
#define ARG_INT16(name) int16 name = scriptOpCall.readSint16(); debug(5, "ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = scriptOpCall.readUint32(); debug(5, "ARG_UINT32(" #name " = %08X)", name);

typedef Common::Functor1<ScriptOpCall&, void> ScriptOpcode;

class ScriptOpcodes {
public:
	ScriptOpcodes(DragonsEngine *vm);
	~ScriptOpcodes();
	void runScript(ScriptOpCall &scriptOpCall);
	void execOpcode(ScriptOpCall &scriptOpCall);
protected:
	DragonsEngine *_vm;
	ScriptOpcode *_opcodes[DRAGONS_NUM_SCRIPT_OPCODES];
	Common::String _opcodeNames[DRAGONS_NUM_SCRIPT_OPCODES];
	int16 _data_80071f5c;

	void initOpcodes();
	void freeOpcodes();
	void updateReturn(ScriptOpCall &scriptOpCall, uint16 size);
	void executeScriptLoop(ScriptOpCall &scriptOpCall);


	// Opcodes
	void opUnk1(ScriptOpCall &scriptOpCall);

	void opUnk13PropertiesRelated(ScriptOpCall &scriptOpCall);
	void opPlayMusic(ScriptOpCall &scriptOpCall);

};

} // End of namespace Dragons

#endif // DRAGONS_SCRIPTOPCODES_H
