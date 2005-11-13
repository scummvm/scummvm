/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
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
	script->ip = &script->dataPtr->data[functionOffset*2+2];
	return true;
}

bool ScriptHelper::validScript(ScriptState *script) {
	if (!script->ip || !script->dataPtr)
		return false;
	return true;
}

bool ScriptHelper::runScript(ScriptState *script) {
	_curScript = script;
	_parameter = 0;
	_continue = true;
	
	if (!_curScript->ip) {
		return false;
	}
	
	int16 code = READ_BE_UINT16(_curScript->ip); _curScript->ip += 2;
	int16 opcode = (code >> 8) & 0x1F;

	if (code & 0x8000) {
		opcode = 0;
		_parameter = code & 0x7FFF;
	} else if (code & 0x4000) {
		_parameter = (int8)(code);
	} else if (code & 0x2000) {
		_parameter = READ_BE_UINT16(_curScript->ip); _curScript->ip += 2;
	} else {
		_parameter = 0;
	}
	
	if (opcode > 18) {
		error("Script unknown command: %d", opcode);
	} else {
		debug(5, "%s(%d)", _commands[opcode].desc, _parameter);
		(this->*(_commands[opcode].proc))();
	}
	
	_curScript = 0;
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
} // end of namespace Kyra
