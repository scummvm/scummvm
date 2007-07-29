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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
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
	static CommandEntry commandProcs[] = {
		// 0x00
		COMMAND(cmd_jmpTo),
		COMMAND(cmd_setRetValue),
		COMMAND(cmd_pushRetOrPos),
		COMMAND(cmd_push),
		// 0x04
		COMMAND(cmd_push),
		COMMAND(cmd_pushReg),
		COMMAND(cmd_pushBPNeg),
		COMMAND(cmd_pushBPAdd),
		// 0x08
		COMMAND(cmd_popRetOrPos),
		COMMAND(cmd_popReg),
		COMMAND(cmd_popBPNeg),
		COMMAND(cmd_popBPAdd),
		// 0x0C
		COMMAND(cmd_addSP),
		COMMAND(cmd_subSP),
		COMMAND(cmd_execOpcode),
		COMMAND(cmd_ifNotJmp),
		// 0x10
		COMMAND(cmd_negate),
		COMMAND(cmd_eval),
		COMMAND(cmd_setRetAndJmp)
	};
	_commands = commandProcs;
#undef COMMAND
}

bool ScriptHelper::loadScript(const char *filename, ScriptData *scriptData, const Common::Array<const Opcode*> *opcodes) {
	ScriptFileParser file(filename, _vm->resource());
	if (!file) {
		error("Couldn't open script file '%s'", filename);
		return false;
	}
	
	uint32 formBlockSize = file.getFORMBlockSize();
	if (formBlockSize == (uint32)-1) {
		error("No FORM chunk found in file: '%s'", filename);
		return false;
	}
	
	uint32 chunkSize = file.getIFFBlockSize(TEXT_CHUNK);
	if (chunkSize != (uint32)-1) {
		scriptData->text = new byte[chunkSize];

		if (!file.loadIFFBlock(TEXT_CHUNK, scriptData->text, chunkSize)) {
			unloadScript(scriptData);
			error("Couldn't load TEXT chunk from file: '%s'", filename);
			return false;
		}
	}
	
	chunkSize = file.getIFFBlockSize(ORDR_CHUNK);
	if (chunkSize == (uint32)-1) {
		unloadScript(scriptData);
		error("No ORDR chunk found in file: '%s'", filename);
		return false;
	}
	chunkSize >>= 1;

	scriptData->ordr = new uint16[chunkSize];

	if (!file.loadIFFBlock(ORDR_CHUNK, scriptData->ordr, chunkSize << 1)) {
		unloadScript(scriptData);
		error("Couldn't load ORDR chunk from file: '%s'", filename);
		return false;
	}

	while (chunkSize--)
		scriptData->ordr[chunkSize] = READ_BE_UINT16(&scriptData->ordr[chunkSize]);
	
	chunkSize = file.getIFFBlockSize(DATA_CHUNK);
	if (chunkSize == (uint32)-1) {
		unloadScript(scriptData);
		error("No DATA chunk found in file: '%s'", filename);
		return false;
	}
	chunkSize >>= 1;

	scriptData->data = new uint16[chunkSize];

	if (!file.loadIFFBlock(DATA_CHUNK, scriptData->data, chunkSize << 1)) {
		unloadScript(scriptData);
		error("Couldn't load DATA chunk from file: '%s'", filename);
		return false;
	}
	scriptData->dataSize = chunkSize;

	while (chunkSize--)
		scriptData->data[chunkSize] = READ_BE_UINT16(&scriptData->data[chunkSize]);

	scriptData->opcodes = opcodes;

	return true;
}

void ScriptHelper::unloadScript(ScriptData *data) {
	if (!data)
		return;

	delete [] data->text;
	delete [] data->ordr;
	delete [] data->data;
	
	data->text = 0; 
	data->ordr = data->data = 0;
}

void ScriptHelper::initScript(ScriptState *scriptStat, const ScriptData *data) {
	scriptStat->dataPtr = data;
	scriptStat->ip = 0;
	scriptStat->stack[60] = 0;
	scriptStat->bp = 62;
	scriptStat->sp = 60;
}

bool ScriptHelper::startScript(ScriptState *script, int function) {
	if (!script->dataPtr)
		return false;

	uint16 functionOffset = script->dataPtr->ordr[function];
	if (functionOffset == 0xFFFF)
		return false;

	if (_vm->game() == GI_KYRA1) {
		if (_vm->gameFlags().platform == Common::kPlatformFMTowns)
			script->ip = &script->dataPtr->data[functionOffset+1];
		else
			script->ip = &script->dataPtr->data[functionOffset];
	} else {
		script->ip = &script->dataPtr->data[functionOffset+1];
	}

	return true;
}

bool ScriptHelper::validScript(ScriptState *script) {
	if (!script->ip || !script->dataPtr || _vm->quit())
		return false;
	return true;
}

bool ScriptHelper::runScript(ScriptState *script) {
	_parameter = 0;
	_continue = true;
	
	if (!script->ip)
		return false;

	int16 code = *script->ip++;
	int16 opcode = (code >> 8) & 0x1F;

	if (code & 0x8000) {
		opcode = 0;
		_parameter = code & 0x7FFF;
	} else if (code & 0x4000) {
		_parameter = (int8)(code);
	} else if (code & 0x2000) {
		_parameter = *script->ip++; 
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

#pragma mark -
#pragma mark - ScriptFileParser implementation
#pragma mark -

void ScriptFileParser::setFile(const char *filename, Resource *res) {
	destroy();
	
	if (!res->getFileHandle(filename, &_endOffset, _scriptFile))
		return;
	_startOffset = _scriptFile.pos();
	_endOffset += _startOffset;
}

void ScriptFileParser::destroy() {
	_scriptFile.close();
	_startOffset = _endOffset = 0;
}

uint32 ScriptFileParser::getFORMBlockSize() {
	uint32 oldOffset = _scriptFile.pos();
	
	uint32 data = _scriptFile.readUint32LE();

	if (data != FORM_CHUNK) {
		_scriptFile.seek(oldOffset);
		return (uint32)-1;
	}

	data = _scriptFile.readUint32BE();
	return data;
}

uint32 ScriptFileParser::getIFFBlockSize(const uint32 chunkName) {
	uint32 size = (uint32)-1;
	
	_scriptFile.seek(_startOffset + 0x0C);

	while (_scriptFile.pos() < _endOffset) {
		uint32 chunk = _scriptFile.readUint32LE();
		uint32 size_temp = _scriptFile.readUint32BE();
	
		if (chunk != chunkName) {
			_scriptFile.seek((size_temp + 1) & (~1), SEEK_CUR);
			assert(_scriptFile.pos() <= _endOffset);
		} else {
			size = size_temp;
			break;
		}
	}

	return size;
}

bool ScriptFileParser::loadIFFBlock(const uint32 chunkName, void *loadTo, uint32 ptrSize) {
	_scriptFile.seek(_startOffset + 0x0C);

	while (_scriptFile.pos() < _endOffset) {
		uint32 chunk = _scriptFile.readUint32LE();
		uint32 chunkSize = _scriptFile.readUint32BE();

		if (chunk != chunkName) {
			_scriptFile.seek((chunkSize + 1) & (~1), SEEK_CUR);
			assert(_scriptFile.pos() <= _endOffset);
		} else {
			uint32 loadSize = 0;

			loadSize = MIN(ptrSize, chunkSize);
			_scriptFile.read(loadTo, loadSize);
			return true;
		}
	}

	return false;
}

#pragma mark -
#pragma mark - Command implementations
#pragma mark -

void ScriptHelper::cmd_jmpTo(ScriptState* script) {
	script->ip = script->dataPtr->data + _parameter;
}

void ScriptHelper::cmd_setRetValue(ScriptState* script) {
	script->retValue = _parameter;
}

void ScriptHelper::cmd_pushRetOrPos(ScriptState* script) {
	switch (_parameter) {
	case 0:
		script->stack[--script->sp] = script->retValue;
		break;

	case 1:
		script->stack[--script->sp] = script->ip - script->dataPtr->data + 1;
		script->stack[--script->sp] = script->bp;
		script->bp = script->sp + 2;
		break;

	default:
		_continue = false;
		script->ip = 0;
		break;
	}
}

void ScriptHelper::cmd_push(ScriptState* script) {
	script->stack[--script->sp] = _parameter;
}

void ScriptHelper::cmd_pushReg(ScriptState* script) {
	script->stack[--script->sp] = script->regs[_parameter];
}

void ScriptHelper::cmd_pushBPNeg(ScriptState* script) {
	script->stack[--script->sp] = script->stack[(-(int32)(_parameter + 2)) + script->bp];
}

void ScriptHelper::cmd_pushBPAdd(ScriptState* script) {
	script->stack[--script->sp] = script->stack[(_parameter - 1) + script->bp];
}

void ScriptHelper::cmd_popRetOrPos(ScriptState* script) {
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
			script->ip = script->dataPtr->data + script->stack[script->sp++];
		}
		break;

	default:
		_continue = false;
		script->ip = 0;
		break;
	}
}

void ScriptHelper::cmd_popReg(ScriptState* script) {
	script->regs[_parameter] = script->stack[script->sp++];
}

void ScriptHelper::cmd_popBPNeg(ScriptState* script) {
	script->stack[(-(int32)(_parameter + 2)) + script->bp] = script->stack[script->sp++];
}

void ScriptHelper::cmd_popBPAdd(ScriptState* script) {
	script->stack[(_parameter - 1) + script->bp] = script->stack[script->sp++];
}

void ScriptHelper::cmd_addSP(ScriptState* script) {
	script->sp += _parameter;
}

void ScriptHelper::cmd_subSP(ScriptState* script) {
	script->sp -= _parameter;
}

void ScriptHelper::cmd_execOpcode(ScriptState* script) {
	uint8 opcode = _parameter;

	assert(script->dataPtr->opcodes);
	assert(opcode < script->dataPtr->opcodes->size());

	if ((*script->dataPtr->opcodes)[opcode] && *(*script->dataPtr->opcodes)[opcode]) {
		script->retValue = (*(*script->dataPtr->opcodes)[opcode])(script);
	} else {
		script->retValue = 0;
		warning("calling unimplemented opcode(0x%.02X/%d)", opcode, opcode);
	}
}

void ScriptHelper::cmd_ifNotJmp(ScriptState* script) {
	if (!script->stack[script->sp++]) {
		_parameter &= 0x7FFF;
		script->ip = script->dataPtr->data + _parameter;
	}
}

void ScriptHelper::cmd_negate(ScriptState* script) {
	int16 value = script->stack[script->sp];
	switch (_parameter) {
	case 0:
		if (!value)
			script->stack[script->sp] = 1;
		else
			script->stack[script->sp] = 0;
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

void ScriptHelper::cmd_eval(ScriptState* script) {
	int16 ret = 0;
	bool error = false;
	
	int16 val1 = script->stack[script->sp++];
	int16 val2 = script->stack[script->sp++];
	
	switch (_parameter) {
	case 0:
		if (!val2 || !val1)
			ret = 0;
		else
			ret = 1;
		break;

	case 1:
		if (val2 || val1)
			ret = 1;
		else
			ret = 0;
		break;

	case 2:
		if (val1 == val2)
			ret = 1;
		else
			ret = 0;
		break;

	case 3:
		if (val1 != val2)
			ret = 1;
		else
			ret = 0;
		break;

	case 4:
		if (val1 > val2)
			ret = 1;
		else
			ret = 0;
		break;

	case 5:
		if (val1 >= val2)
			ret = 1;
		else
			ret = 0;
		break;

	case 6:
		if (val1 < val2)
			ret = 1;
		else
			ret = 0;
		break;

	case 7:
		if (val1 <= val2)
			ret = 1;
		else
			ret = 0;
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

void ScriptHelper::cmd_setRetAndJmp(ScriptState* script) {
	if (script->sp >= 60) {
		_continue = false;
		script->ip = 0;
	} else {
		script->retValue = script->stack[script->sp++];
		uint16 temp = script->stack[script->sp++];
		script->stack[60] = 0;
		script->ip = &script->dataPtr->data[temp];
	}
}
} // end of namespace Kyra

