/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/system.h"
#include "kyra/kyra.h"
#include "kyra/resource.h"
#include "kyra/script.h"

#define FORM_CHUNK 0x4D524F46
#define TEXT_CHUNK 0x54584554
#define DATA_CHUNK 0x41544144
#define ORDR_CHUNK 0x5244524F

namespace Kyra {
ScriptHelper::ScriptHelper(KyraEngine *vm) : _vm(vm) {
#define COMMAND(x) { &ScriptHelper::x, #x }
	// now we create a list of all Command/Opcode procs and so
	static CommandEntry commandProcs[] = {
		// 0x00
		COMMAND(c1_jmpTo),
		COMMAND(c1_setRetValue),
		COMMAND(c1_pushRetOrPos),
		COMMAND(c1_push),
		// 0x04
		COMMAND(c1_push),
		COMMAND(c1_pushVar),
		COMMAND(c1_pushBPNeg),
		COMMAND(c1_pushBPAdd),
		// 0x08
		COMMAND(c1_popRetOrPos),
		COMMAND(c1_popVar),
		COMMAND(c1_popBPNeg),
		COMMAND(c1_popBPAdd),
		// 0x0C
		COMMAND(c1_addSP),
		COMMAND(c1_subSP),
		COMMAND(c1_execOpcode),
		COMMAND(c1_ifNotJmp),
		// 0x10
		COMMAND(c1_negate),
		COMMAND(c1_eval),
		COMMAND(c1_setRetAndJmp)
	};
	_commands = commandProcs;
#undef COMMAND
}

ScriptHelper::~ScriptHelper() {
}

bool ScriptHelper::loadScript(const char *filename, ScriptData *scriptData, KyraEngine::OpcodeProc *opcodes, int opcodeSize, byte *specialPtr) {
	uint32 size = 0;
	uint8 *data = _vm->resource()->fileData(filename, &size);	
	byte *curData = data;
	
	uint32 formBlockSize = getFORMBlockSize(curData);
	if (formBlockSize == (uint32)-1) {
		delete [] data;
		error("No FORM chunk found in file: '%s'", filename);
		return false;
	}
	
	uint32 chunkSize = getIFFBlockSize(data, curData, size, TEXT_CHUNK);
	if (chunkSize != (uint32)-1) {
		if (specialPtr) {
			scriptData->mustBeFreed = 0;
			scriptData->text = specialPtr;
			specialPtr += chunkSize;
		} else {
			scriptData->mustBeFreed = 1;
			scriptData->text = new byte[chunkSize];
		}
		if (!loadIFFBlock(data, curData, size, TEXT_CHUNK, scriptData->text, chunkSize)) {
			delete [] data;
			unloadScript(scriptData);
			error("Couldn't load TEXT chunk from file: '%s'", filename);
			return false;
		}
	}
	
	chunkSize = getIFFBlockSize(data, curData, size, ORDR_CHUNK);
	if (chunkSize == (uint32)-1) {
		delete [] data;
		unloadScript(scriptData);
		error("No ORDR chunk found in file: '%s'", filename);
		return false;
	}
	if (specialPtr) {
		scriptData->mustBeFreed = 0;
		scriptData->ordr = specialPtr;
		specialPtr += chunkSize;
	} else {
		scriptData->mustBeFreed = 1;
		scriptData->ordr = new byte[chunkSize];
	}
	if (!loadIFFBlock(data, curData, size, ORDR_CHUNK, scriptData->ordr, chunkSize)) {
		delete [] data;
		unloadScript(scriptData);
		error("Couldn't load ORDR chunk from file: '%s'", filename);
		return false;
	}
	chunkSize = chunkSize / 2;
	while (chunkSize--) {
		((uint16*)scriptData->ordr)[chunkSize] = READ_BE_UINT16(&((uint16*)scriptData->ordr)[chunkSize]);
	}
	
	chunkSize = getIFFBlockSize(data, curData, size, DATA_CHUNK);
	if (chunkSize == (uint32)-1) {
		delete [] data;
		unloadScript(scriptData);
		error("No DATA chunk found in file: '%s'", filename);
		return false;
	}
	if (specialPtr) {
		scriptData->mustBeFreed = 0;
		scriptData->data = specialPtr;
		specialPtr += chunkSize;
	} else {
		scriptData->mustBeFreed = 1;
		scriptData->data = new byte[chunkSize];
	}
	if (!loadIFFBlock(data, curData, size, DATA_CHUNK, scriptData->data, chunkSize)) {
		delete [] data;
		unloadScript(scriptData);
		error("Couldn't load DATA chunk from file: '%s'", filename);
		return false;
	}
	scriptData->dataSize = chunkSize / 2;
	scriptData->opcodes = opcodes;
	scriptData->opcodeSize = opcodeSize;
	
	delete [] data;
	return true;
}

void ScriptHelper::unloadScript(ScriptData *data) {
	if (data->mustBeFreed) {
		delete [] data->text;
		delete [] data->ordr;
		delete [] data->data;
	}
	
	data->mustBeFreed = 0;
	data->text = data->ordr = data->data = 0;
}

void ScriptHelper::initScript(ScriptState *scriptStat, ScriptData *data) {
	scriptStat->dataPtr = data;
	scriptStat->ip = 0;
	scriptStat->stack[60] = 0;
	scriptStat->bp = 62;
	scriptStat->sp = 60;
}

bool ScriptHelper::startScript(ScriptState *script, int function) {
	if (!script->dataPtr) {
		return false;
	}
	uint16 functionOffset = ((uint16*)script->dataPtr->ordr)[function];
	if (functionOffset == (uint16)-1) {
		return false;
	}
	script->ip = &script->dataPtr->data[functionOffset*2];
	return true;
}

bool ScriptHelper::validScript(ScriptState *script) {
	if (!script->ip || !script->dataPtr)
		return false;
	return true;
}

bool ScriptHelper::runScript(ScriptState *script) {
	_parameter = 0;
	_continue = true;
	
	if (!script->ip) {
		return false;
	}
	
	int16 code = READ_BE_UINT16(script->ip); script->ip += 2;
	int16 opcode = (code >> 8) & 0x1F;

	if (code & 0x8000) {
		opcode = 0;
		_parameter = code & 0x7FFF;
	} else if (code & 0x4000) {
		_parameter = (int8)(code);
	} else if (code & 0x2000) {
		_parameter = READ_BE_UINT16(script->ip); script->ip += 2;
	} else {
		_parameter = 0;
	}
	
	if (opcode > 18) {
		error("Script unknown command: %d", opcode);
	} else {
		debugC(5, kDebugLevelScript, "%s([%d/%u])", _commands[opcode].desc, _parameter, (uint)_parameter);
		(this->*(_commands[opcode].proc))(script);
	}
	
	return _continue;
}

uint32 ScriptHelper::getFORMBlockSize(byte *&data) const {
	static const uint32 chunkName = FORM_CHUNK;
	if (READ_LE_UINT32(data) != chunkName) {
		return (uint32)-1;
	}
	data += 4;
	uint32 retValue = READ_BE_UINT32(data); data += 4;
	return retValue;
}

uint32 ScriptHelper::getIFFBlockSize(byte *start, byte *&data, uint32 maxSize, const uint32 chunkName) const {
	uint32 size = (uint32)-1;
	bool special = false;
	
	if (data == (start + maxSize)) {
		data = start + 0x0C;
	}
	while (data < (start + maxSize)) {
		uint32 chunk = READ_LE_UINT32(data); data += 4;
		uint32 size_temp = READ_BE_UINT32(data); data += 4;
		if (chunk != chunkName) {
			if (special) {
				data += (size_temp + 1) & 0xFFFFFFFE;
			} else {
				data = start + 0x0C;
				special = true;
			}
		} else {
			// kill our data
			data = start;
			size = size_temp;
			break;
		}
	}
	return size;
}

bool ScriptHelper::loadIFFBlock(byte *start, byte *&data, uint32 maxSize, const uint32 chunkName, byte *loadTo, uint32 ptrSize) const {
	bool special = false;
	
	if (data == (start + maxSize)) {
		data = start + 0x0C;
	}
	while (data < (start + maxSize)) {
		uint32 chunk = READ_LE_UINT32(data); data += 4;
		uint32 chunkSize = READ_BE_UINT32(data); data += 4;
		if (chunk != chunkName) {
			if (special) {
				data += (chunkSize + 1) & 0xFFFFFFFE;
			} else {
				data = start + 0x0C;
				special = true;
			}
		} else {
			uint32 loadSize = 0;
			if (chunkSize < ptrSize)
				loadSize = chunkSize;
			else
				loadSize = ptrSize;
			memcpy(loadTo, data, loadSize);
			chunkSize = (chunkSize + 1) & 0xFFFFFFFE;
			if (chunkSize > loadSize) {
				data += (chunkSize - loadSize);
			}
			return true;
		}
	}
	return false;
}

#pragma mark -
#pragma mark - Command implementations
#pragma mark -

void ScriptHelper::c1_jmpTo(ScriptState* script) {
	script->ip = script->dataPtr->data + (_parameter << 1);
}

void ScriptHelper::c1_setRetValue(ScriptState* script) {
	script->retValue = _parameter;
}

void ScriptHelper::c1_pushRetOrPos(ScriptState* script) {
	switch (_parameter) {
		case 0:
			script->stack[--script->sp] = script->retValue;
		break;
		
		case 1:
			script->stack[--script->sp] = (script->ip - script->dataPtr->data) / 2 + 1;
			script->stack[--script->sp] = script->bp;
			script->bp = script->sp + 2;
		break;
		
		default:
			_continue = false;
			script->ip = 0;
		break;
	}
}

void ScriptHelper::c1_push(ScriptState* script) {
	script->stack[--script->sp] = _parameter;
}

void ScriptHelper::c1_pushVar(ScriptState* script) {
	script->stack[--script->sp] = script->variables[_parameter];
}

void ScriptHelper::c1_pushBPNeg(ScriptState* script) {
	script->stack[--script->sp] = script->stack[(-(int32)(_parameter + 2)) + script->bp];
}

void ScriptHelper::c1_pushBPAdd(ScriptState* script) {
	script->stack[--script->sp] = script->stack[(_parameter - 1) + script->bp];
}

void ScriptHelper::c1_popRetOrPos(ScriptState* script) {
	switch (_parameter) {
		case 0:
			script->retValue = script->stack[script->sp++];
		break;
		
		case 1:
			if (script->sp >= 60) {
				_continue = false;
				script->ip = 0;
			} else {
				script->bp = script->stack[script->sp++];
				script->ip = script->dataPtr->data + (script->stack[script->sp++] << 1);
			}
		break;
		
		default:
			_continue = false;
			script->ip = 0;
		break;
	}
}

void ScriptHelper::c1_popVar(ScriptState* script) {
	script->variables[_parameter] = script->stack[script->sp++];
}

void ScriptHelper::c1_popBPNeg(ScriptState* script) {
	script->stack[(-(int32)(_parameter + 2)) + script->bp] = script->stack[script->sp++];
}

void ScriptHelper::c1_popBPAdd(ScriptState* script) {
	script->stack[(_parameter - 1) + script->bp] = script->stack[script->sp++];
}

void ScriptHelper::c1_addSP(ScriptState* script) {
	script->sp += _parameter;
}

void ScriptHelper::c1_subSP(ScriptState* script) {
	script->sp -= _parameter;
}

void ScriptHelper::c1_execOpcode(ScriptState* script) {
	assert((uint8)_parameter < script->dataPtr->opcodeSize);
	if (script->dataPtr->opcodes[(uint8)_parameter] == &KyraEngine::cmd_dummy)
		warning("calling unimplemented opcode(0x%.02X)", (uint8)_parameter);
	int val = (_vm->*script->dataPtr->opcodes[(uint8)_parameter])(script);
	assert(script);
	script->retValue = val;
}

void ScriptHelper::c1_ifNotJmp(ScriptState* script) {
	if (!script->stack[script->sp++]) {
		_parameter &= 0x7FFF;
		script->ip = script->dataPtr->data + (_parameter << 1);
	}
}

void ScriptHelper::c1_negate(ScriptState* script) {
	int16 value = script->stack[script->sp];
	switch (_parameter) {
		case 0:
			if (!value) {
				script->stack[script->sp] = 1;
			} else {
				script->stack[script->sp] = 0;
			}
		break;
		
		case 1:
			script->stack[script->sp] = -value;
		break;
		
		case 2:
			script->stack[script->sp] = ~value;
		break;
		
		default:
			_continue = false;
		break;
	}
}

void ScriptHelper::c1_eval(ScriptState* script) {
	int16 ret = 0;
	bool error = false;
	
	int16 val1 = script->stack[script->sp++];
	int16 val2 = script->stack[script->sp++];
	
	switch (_parameter) {
		case 0:
			if (!val2 || !val1) {
				ret = 0;
			} else {
				ret = 1;
			}
		break;
		
		case 1:
			if (val2 || val1) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 2:
			if (val1 == val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 3:
			if (val1 != val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 4:
			if (val1 > val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 5:
			if (val1 >= val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 6:
			if (val1 < val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 7:
			if (val1 <= val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 8:
			ret = val1 + val2;
		break;
		
		case 9:
			ret = val2 - val1;
		break;
		
		case 10:
			ret = val1 * val2;
		break;
		
		case 11:
			ret = val2 / val1;
		break;
		
		case 12:
			ret = val2 >> val1;
		break;
		
		case 13:
			ret = val2 << val1;
		break;
		
		case 14:
			ret = val1 & val2;
		break;
		
		case 15:
			ret = val1 | val2;
		break;
		
		case 16:
			ret = val2 % val1;
		break;
		
		case 17:
			ret = val1 ^ val2;
		break;
		
		default:
			warning("Unknown evaluate func: %d", _parameter);
			error = true;
		break;
	}
	
	if (error) {
		script->ip = 0;
		_continue = false;
	} else {
		script->stack[--script->sp] = ret;
	}
}

void ScriptHelper::c1_setRetAndJmp(ScriptState* script) {
	if (script->sp >= 60) {
		_continue = false;
		script->ip = 0;
	} else {
		script->retValue = script->stack[script->sp++];
		uint16 temp = script->stack[script->sp++];
		script->stack[60] = 0;
		script->ip = &script->dataPtr->data[temp*2];
	}
}
} // end of namespace Kyra
