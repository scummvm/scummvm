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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "director/lingo/lingo.h"

namespace Director {

static struct LingoV4Bytecode {
	const uint8 opcode;
	const inst func;
	const char *proto;
} lingoV4[] = {
	{ 0x01, STOP, "" },
	{ 0x03, Lingo::c_voidpush, "" },
	{ 0x04, Lingo::c_mul, "" },
	{ 0x05, Lingo::c_add, "" },
	{ 0x06, Lingo::c_sub, "" },
	{ 0x07, Lingo::c_div, "" },
	{ 0x08, Lingo::c_mod, "" },
	{ 0x09, Lingo::c_negate, "" },
	{ 0x0a, Lingo::c_ampersand, "" },
	{ 0x0b, Lingo::c_concat, "" },
	{ 0x0c, Lingo::c_lt, "" },
	{ 0x0d, Lingo::c_le, "" },
	{ 0x0e, Lingo::c_neq, "" },
	{ 0x0f, Lingo::c_eq, "" },
	{ 0x10, Lingo::c_gt, "" },
	{ 0x11, Lingo::c_ge, "" },
	{ 0x12, Lingo::c_and, "" },
	{ 0x13, Lingo::c_or, "" },
	{ 0x14, Lingo::c_not, "" },
	{ 0x15, Lingo::c_contains, "" },
	{ 0x16, Lingo::c_starts, "" },
	{ 0x17, Lingo::c_of, "" },
	{ 0x18, Lingo::c_hilite, "" },
	{ 0x19, Lingo::c_intersects, "" },
	{ 0x1a, Lingo::c_within, "" },
	{ 0x1b, Lingo::c_field, "" },
	{ 0x1c, Lingo::c_tell, "" },
	{ 0x1d, Lingo::c_telldone, "" },
	{ 0x41, Lingo::c_intpush, "b" },
	{ 0, 0, 0 }
};

void Lingo::initBytecode() {
	for (LingoV4Bytecode *op = lingoV4; op->opcode; op++) {
		_lingoV4[op->opcode] = new Opcode( op->func, op->proto );
	}
}


void Lingo::addCodeV4(Common::SeekableSubReadStreamEndian &stream, ScriptType type, uint16 id) {
	debugC(1, kDebugLingoCompile, "Add V4 bytecode for type %s with id %d", scriptType2str(type), id);

	if (_scriptContexts[type].contains(id)) {
		for (size_t j = 0; j < _scriptContexts[type][id]->functions.size(); j++) {
			delete _scriptContexts[type][id]->functions[j];
		}
		delete _scriptContexts[type][id];
	}

	_currentScriptContext = new ScriptContext;
	_currentScriptType = type;
	_currentEntityId = id;
	_scriptContexts[type][id] = _currentScriptContext;

	// read the Lscr header!
	// unk1
	for (uint32 i = 0; i < 0x10; i++) {
		stream.readByte();
	}
	uint16 codeStoreOffset = stream.readUint16();
	// unk2
	for (uint32 i = 0; i < 0x2e; i++) {
		stream.readByte();
	}
	uint16 globalsOffset = stream.readUint16();
	uint16 globalsCount = stream.readUint16();
	// unk3
	for (uint32 i = 0; i < 0x4; i++) {
		stream.readByte();
	}
	uint16 functionsCount = stream.readUint16();
	stream.readUint16();
	uint16 functionsOffset = stream.readUint16();
	uint16 constsCount = stream.readUint16();
	stream.readUint16();
	uint16 constsOffset = stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	uint16 constsBase = stream.readUint16();

	// preload all the constants!
	// these are stored as a reference table of 6 byte entries, followed by a storage area.

	// copy the storage area first.
	uint32 constsStoreOffset = constsOffset + 6 * constsCount;
	uint32 constsStoreSize = stream.size() - constsStoreOffset;
	stream.seek(constsStoreOffset);
	byte *constsStore = (byte *)malloc(constsStoreSize);
	stream.read(constsStore, constsStoreSize);

	Common::Array<Datum> constsData;

	// read each entry in the reference table.
	stream.seek(constsOffset);
	for (uint16 i = 0; i < constsCount; i++) {
		Datum constant;
		uint32 constType = 0;
		if (_vm->getVersion() >= 5) {
			constType = stream.readUint32();
		} else {
			constType = (uint32)stream.readUint16();
		}
		uint32 value = stream.readUint32();
		switch (constType) {
		case 1: // String type
			{
				constant.type = STRING;
				constant.u.s = new Common::String();
				uint32 pointer = value;
				while (pointer < constsStoreSize) {
					if (constsStore[pointer] == '\r') {
						constant.u.s += '\n';
					} else if (constsStore[pointer] == '\0') {
						break;
					} else {
						constant.u.s += constsStore[pointer];
					}
					pointer += 1;
				}
				if (pointer >= constsStoreSize) {
					warning("Constant string has no null terminator");
					break;
				}
			}
			break;
		case 4: // Integer type
			constant.type = INT;
			constant.u.i = (int)value;
			break;
		case 9:  // Float type
			{
				constant.type = FLOAT;
				if (value < constsStoreOffset) {
					warning("Constant float start offset is out of bounds");
					break;
				} else if (value + 4 > constsStoreSize) {
					warning("Constant float end offset is out of bounds");
					break;
				}
				constant.u.f = *(float *)(constsStore + value);
			}
			break;
		default:
			warning("Unknown constant type %d", type);
			break;
		}

		constsData.push_back(constant);
	}
	free(constsStore);

	// parse each function!
	// these are stored as a code storage area, followed by a reference table of 42 byte entries.

	// copy the storage area first.
	uint32 codeStoreSize = functionsOffset - codeStoreOffset;
	stream.seek(codeStoreOffset);
	byte *codeStore = (byte *)malloc(codeStoreSize);
	stream.read(codeStore, codeStoreSize);

	// read each entry in the function table.
	stream.seek(functionsOffset);
	for (uint16 i = 0; i < functionsCount; i++) {
		_currentScriptFunction = i;
		_currentScriptContext->functions.push_back(new ScriptData);
		_currentScript = _currentScriptContext->functions[_currentScriptFunction];

		uint16 nameIndex = stream.readUint16();
		stream.readUint16();
		uint32 length = stream.readUint32();
		uint32 startOffset = stream.readUint32();
		uint16 argCount = stream.readUint16();
		uint32 argOffset = stream.readUint32();
		uint16 varCount = stream.readUint16();
		uint32 varNamesOffset = stream.readUint32();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();

		if (startOffset < codeStoreOffset) {
			warning("Function %d start offset is out of bounds!", i);
			continue;
		} else if (startOffset + length >= codeStoreOffset + codeStoreSize) {
			warning("Function %d end offset is out of bounds", i);
			continue;
		}

		uint16 pointer = startOffset - codeStoreOffset;
		Common::Array<uint32> offsetList;
		while (pointer < startOffset + length - codeStoreOffset) {
			uint8 opcode = codeStore[pointer];

			pointer += 1;
			if (_lingoV4.contains(opcode)) {
				offsetList.push_back(_currentScript->size());
				g_lingo->code1(_lingoV4[opcode]->func);
				size_t argc = strlen(_lingoV4[opcode]->proto);
				for (uint c = 0; c < argc; c++) {
					switch (_lingoV4[opcode]->proto[c]) {
					case 'b':
						offsetList.push_back(_currentScript->size());
						g_lingo->codeInt((int8)codeStore[pointer]);
						pointer += 1;
						break;
					case 'w':
						offsetList.push_back(_currentScript->size());
						offsetList.push_back(_currentScript->size());
						g_lingo->codeInt((int16)READ_UINT16(&codeStore[pointer]));
						pointer += 2;
						break;
					default:
						break;
					}
				}

			} else {
				// unimplemented instruction
				if (opcode < 0x40) { // 1 byte instruction
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk);
					g_lingo->codeInt(opcode);
				} else if (opcode < 0x80) { // 2 byte instruction
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk1);
					g_lingo->codeInt(opcode);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer]);
					pointer += 1;
				} else { // 3 byte instruction
					offsetList.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk2);
					g_lingo->codeInt(opcode);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer]);
					offsetList.push_back(_currentScript->size());
					g_lingo->codeInt((uint)codeStore[pointer+1]);
					pointer += 2;
				}
			}
		}

	}
	free(codeStore);
}


void Lingo::addNamesV4(Common::SeekableSubReadStreamEndian &stream) {
	debugC(1, kDebugLingoCompile, "Add V4 script name index");

	// read the Lnam header!
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	uint16 offset = stream.readUint16();
	uint16 count = stream.readUint16();

	stream.seek(offset);

	_namelist.clear();

	Common::Array<Common::String> names;
	for (uint32 i = 0; i < count; i++) {
		uint8 size = stream.readByte();
		Common::String name;
		for (uint8 j = 0; j < size; j++) {
			name += stream.readByte();
		}
		_namelist.push_back(name);
	}

}

}
