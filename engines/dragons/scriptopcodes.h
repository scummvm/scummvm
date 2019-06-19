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

#define DRAGONS_NUM_SCRIPT_OPCODES 0x23
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
#define ARG_SKIP(x) scriptOpCall.skip(x);
#define ARG_BYTE(name) byte name = scriptOpCall.readByte(); debug(5, "ARG_BYTE(" #name " = %d)", name);
#define ARG_INT8(name) int8 name = scriptOpCall.readByte(); debug(5, "ARG_INT8(" #name " = %d)", name);
#define ARG_INT16(name) int16 name = scriptOpCall.readSint16(); debug(5, "ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = scriptOpCall.readUint32(); debug(5, "ARG_UINT32(" #name " = %08X)", name);

typedef Common::Functor1<ScriptOpCall&, void> ScriptOpcode;

class DragonFLG;
class SpecialOpcodes;

class ScriptOpcodes {
public:
	ScriptOpcodes(DragonsEngine *vm, DragonFLG *dragonFLG);
	~ScriptOpcodes();
	void runScript(ScriptOpCall &scriptOpCall);
	void runScript3(ScriptOpCall &scriptOpCall);
	bool runScript4(ScriptOpCall &scriptOpCall);
	void execOpcode(ScriptOpCall &scriptOpCall);
	void executeScriptLoop(ScriptOpCall &scriptOpCall);
	int16 FUN_800297d8(ScriptOpCall &scriptOpCall);
	int16 _data_80071f5c;
	int16 _data_800728c0;
protected:
	DragonsEngine *_vm;
	DragonFLG *_dragonFLG;
	SpecialOpcodes *_specialOpCodes;
	ScriptOpcode *_opcodes[DRAGONS_NUM_SCRIPT_OPCODES];
	Common::String _opcodeNames[DRAGONS_NUM_SCRIPT_OPCODES];

	void initOpcodes();
	void freeOpcodes();
	void updateReturn(ScriptOpCall &scriptOpCall, uint16 size);


	// Opcodes
	void opUnk1(ScriptOpCall &scriptOpCall);
	void opUnk2(ScriptOpCall &scriptOpCall);
	void opExecuteScript(ScriptOpCall &scriptOpCall); //op 4
	void opActorSetSequenceID2(ScriptOpCall &scriptOpCall); //op 5
	void opUnk6(ScriptOpCall &scriptOpCall);
	void opUnk7(ScriptOpCall &scriptOpCall);
	void opActorLoadSequence(ScriptOpCall &scriptOpCall);

	void opUnkA(ScriptOpCall &scriptOpCall);
	void opRunSpecialOpCode(ScriptOpCall &scriptOpCall); //op B
	void opUnkCSoundRelatedMaybe(ScriptOpCall &scriptOpCall);
	void opDelay(ScriptOpCall &scriptOpCall); //op D
	void opUnkE(ScriptOpCall &scriptOpCall);
	void opUnkF(ScriptOpCall &scriptOpCall);
	void opUnk10(ScriptOpCall &scriptOpCall);
	void opUnk11FlickerTalk(ScriptOpCall &scriptOpCall);
	void opUnk12LoadScene(ScriptOpCall &scriptOpCall);
	void opUnk13PropertiesRelated(ScriptOpCall &scriptOpCall);
	void opUnk14PropertiesRelated(ScriptOpCall &scriptOpCall);
	void opUnk15PropertiesRelated(ScriptOpCall &scriptOpCall);
	void opUnk16(ScriptOpCall &scriptOpCall);
	void opUnk17(ScriptOpCall &scriptOpCall);
	void opUnk18(ScriptOpCall &scriptOpCall);
	void opUnk1B(ScriptOpCall &scriptOpCall);
	void opSetActorFlag0x1000(ScriptOpCall &scriptOpCall);
	void opPlayMusic(ScriptOpCall &scriptOpCall);
	void opUnk20(ScriptOpCall &scriptOpCall);

	bool checkPropertyFlag(ScriptOpCall &scriptOpCall);
	void opCode_UnkA_setsProperty(ScriptOpCall &scriptOpCall);
	void opCode_Unk7(ScriptOpCall &scriptOpCall);

	void opCodeActorTalk(ScriptOpCall &scriptOpCall); // 0x22

	// misc
	uint16 getINIField(uint32 iniIndex, uint16 fieldOffset);
	void setINIField(uint32 iniIndex, uint16 fieldOffset, uint16 value);
};

} // End of namespace Dragons

#endif // DRAGONS_SCRIPTOPCODES_H
