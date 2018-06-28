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

#ifndef ILLUSIONS_SCRIPTOPCODES_H
#define ILLUSIONS_SCRIPTOPCODES_H

#include "common/func.h"

namespace Illusions {

class IllusionsEngine;
class ScriptThread;

struct OpCall {
	byte _op;
	byte _opSize;
	uint32 _threadId;
	uint32 _callerThreadId;
	int16 _deltaOfs;
	byte *_code;
	int _result;
	void skip(uint size);
	byte readByte();
	int16 readSint16();
	uint32 readUint32();
};

typedef Common::Functor2<ScriptThread*, OpCall&, void> ScriptOpcode;

class ScriptOpcodes {
public:
	ScriptOpcodes(IllusionsEngine *vm);
	virtual ~ScriptOpcodes();
	void execOpcode(ScriptThread *scriptThread, OpCall &opCall);
protected:
	IllusionsEngine *_vm;
	ScriptOpcode *_opcodes[256];
	Common::String _opcodeNames[256];
	virtual void initOpcodes() {}
	virtual void freeOpcodes() {}
};

// Convenience macros
#define ARG_SKIP(x) opCall.skip(x);
#define ARG_BYTE(name) byte name = opCall.readByte(); debug(5, "ARG_BYTE(" #name " = %d)", name);
#define ARG_INT16(name) int16 name = opCall.readSint16(); debug(5, "ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = opCall.readUint32(); debug(5, "ARG_UINT32(" #name " = %08X)", name);

} // End of namespace Illusions

#endif // ILLUSIONS_SCRIPTOPCODES_H
