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
	uint16 code_store_offset = stream.readUint16();
	// unk2
	for (uint32 i = 0; i < 0x2e; i++) {
		stream.readByte();
	}
	uint16 globals_offset = stream.readUint16();
	uint16 globals_count = stream.readUint16();
	// unk3
	for (uint32 i = 0; i < 0x4; i++) {
		stream.readByte();
	}
	uint16 functions_count = stream.readUint16();
	stream.readUint16();
	uint16 functions_offset = stream.readUint16();
	uint16 consts_count = stream.readUint16();
	stream.readUint16();
	uint16 consts_offset = stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	stream.readUint16();
	uint16 consts_base = stream.readUint16();

	// preload all the constants!
	// these are stored as a reference table of 6 byte entries, followed by a storage area.

	// copy the storage area first.
	uint32 consts_store_offset = consts_offset+6*consts_count;
	uint32 consts_store_size = stream.size()-consts_store_offset;
	stream.seek(consts_store_offset);
	byte *const_store = (byte *)malloc(consts_store_size);
	stream.read(const_store, consts_store_size);

	Common::Array<Datum> const_data;

	// read each entry in the reference table.
	stream.seek(consts_offset);
	for (uint16 i=0; i<consts_count; i++) {
		Datum constant;
		uint16 const_type = stream.readUint16();
		uint32 value = stream.readUint32();
		switch (const_type) {
			case 1: { // String type
				constant.type = STRING;
				constant.u.s = new Common::String();
				uint32 pointer = value;
				while (pointer < consts_store_size) {
					if (const_store[pointer] == '\r') {
						constant.u.s += '\n';
					} else if (const_store[pointer] == '\0') {
						break;
					} else {
						constant.u.s += const_store[pointer];
					}
					pointer += 1;
				}
				if (pointer >= consts_store_size) {
					warning("Constant string has no null terminator");
					break;
				}
			}
			break;
			case 4: // Integer type
				constant.type = INT;
				constant.u.i = (int)value;
				break;
			case 9: { // Float type
				constant.type = FLOAT;
				if (value < consts_store_offset) {
					warning("Constant float start offset is out of bounds");
					break;
				} else if (value+4 > consts_store_size) {
					warning("Constant float end offset is out of bounds");
					break;
				}
				constant.u.f = *(float *)(const_store+value);
			}
			break;
			default:
				warning("Unknown constant type %d", type);
				break;
		}

		const_data.push_back(constant);
	}
	free(const_store);

	// parse each function!
	// these are stored as a code storage area, followed by a reference table of 42 byte entries.

	// copy the storage area first.
	uint32 code_store_size = functions_offset - code_store_offset;
	stream.seek(code_store_offset);
	byte *code_store = (byte *)malloc(code_store_size);
	stream.read(code_store, code_store_size);

	// read each entry in the function table.
	stream.seek(functions_offset);
	for (uint16 i=0; i<functions_count; i++) {
		_currentScriptFunction = i;
		_currentScriptContext->functions.push_back(new ScriptData);
		_currentScript = _currentScriptContext->functions[_currentScriptFunction];

		uint16 name_index = stream.readUint16();
		stream.readUint16();
		uint32 length = stream.readUint32();
		uint32 start_offset = stream.readUint32();
		uint16 arg_count = stream.readUint16();
		uint32 arg_offset = stream.readUint32();
		uint16 var_count = stream.readUint16();
		uint32 var_names_offset = stream.readUint32();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();
		stream.readUint16();

		if (start_offset < code_store_offset) {
			warning("Function %d start offset is out of bounds!", i);
			continue;
		} else if (start_offset + length >= code_store_offset + code_store_size) {
			warning("Function %d end offset is out of bounds", i);
			continue;
		}

		uint16 pointer = start_offset-code_store_offset;
		Common::Array<uint32> offset_list;
		while (pointer < start_offset+length-code_store_offset) {
			uint8 opcode = code_store[pointer];

			pointer += 1;
			if (_lingoV4.contains(opcode)) {
				offset_list.push_back(_currentScript->size());
				g_lingo->code1(_lingoV4[opcode]->func);
				size_t argc = strlen(_lingoV4[opcode]->proto);
				for (uint c=0; c<argc; c++) {
					switch (_lingoV4[opcode]->proto[c]) {
						case 'b':
							offset_list.push_back(_currentScript->size());
							g_lingo->codeInt((int8)code_store[pointer]);
							pointer += 1;
							break;
						case 'w':
							offset_list.push_back(_currentScript->size());
							offset_list.push_back(_currentScript->size());
							g_lingo->codeInt((int16)READ_UINT16(&code_store[pointer]));
							pointer += 2;
							break;
						default:
							break;
					}
				}

			} else {
				// exit condition
				if (opcode == 0x01) {
					offset_list.push_back(_currentScript->size());
					g_lingo->code1(STOP);
				// unimplemented instruction
				} else if (opcode < 0x40) { // 1 byte instruction
					offset_list.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk);
					g_lingo->codeInt(opcode);
				} else if (opcode < 0x80) { // 2 byte instruction
					offset_list.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk1);
					g_lingo->codeInt(opcode);
					offset_list.push_back(_currentScript->size());
					g_lingo->codeInt((uint)code_store[pointer]);
					pointer += 1;
				} else { // 3 byte instruction
					offset_list.push_back(_currentScript->size());
					g_lingo->code1(Lingo::c_unk2);
					g_lingo->codeInt(opcode);
					offset_list.push_back(_currentScript->size());
					g_lingo->codeInt((uint)code_store[pointer]);
					offset_list.push_back(_currentScript->size());
					g_lingo->codeInt((uint)code_store[pointer+1]);
					pointer += 2;
				}
			}
		}

	}
	free(code_store);
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
