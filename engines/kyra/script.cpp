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


#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/system.h"
#include "kyra/kyra_v1.h"
#include "kyra/resource.h"
#include "kyra/script.h"

namespace Kyra {
EMCInterpreter::EMCInterpreter(KyraEngine_v1 *vm) : _vm(vm) {
#define COMMAND(x) { &EMCInterpreter::x, #x }
	static const CommandEntry commandProcs[] = {
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

bool EMCInterpreter::load(const char *filename, EMCData *scriptData, const Common::Array<const Opcode*> *opcodes) {
	ScriptFileParser file(filename, _vm->resource());
	if (!file) {
		error("Couldn't open script file '%s'", filename);
		return false;
	}

	memset(scriptData, 0, sizeof(EMCData));

	uint32 formBlockSize = file.getFORMBlockSize();
	if (formBlockSize == (uint32)-1) {
		error("No FORM chunk found in file: '%s'", filename);
		return false;
	}

	uint32 chunkSize = file.getIFFBlockSize(TEXT_CHUNK);
	if (chunkSize != (uint32)-1) {
		scriptData->text = new byte[chunkSize];

		if (!file.loadIFFBlock(TEXT_CHUNK, scriptData->text, chunkSize)) {
			unload(scriptData);
			error("Couldn't load TEXT chunk from file: '%s'", filename);
			return false;
		}
	}

	chunkSize = file.getIFFBlockSize(ORDR_CHUNK);
	if (chunkSize == (uint32)-1) {
		unload(scriptData);
		error("No ORDR chunk found in file: '%s'", filename);
		return false;
	}
	chunkSize >>= 1;

	scriptData->ordr = new uint16[chunkSize];

	if (!file.loadIFFBlock(ORDR_CHUNK, scriptData->ordr, chunkSize << 1)) {
		unload(scriptData);
		error("Couldn't load ORDR chunk from file: '%s'", filename);
		return false;
	}

	while (chunkSize--)
		scriptData->ordr[chunkSize] = READ_BE_UINT16(&scriptData->ordr[chunkSize]);

	chunkSize = file.getIFFBlockSize(DATA_CHUNK);
	if (chunkSize == (uint32)-1) {
		unload(scriptData);
		error("No DATA chunk found in file: '%s'", filename);
		return false;
	}
	chunkSize >>= 1;

	scriptData->data = new uint16[chunkSize];

	if (!file.loadIFFBlock(DATA_CHUNK, scriptData->data, chunkSize << 1)) {
		unload(scriptData);
		error("Couldn't load DATA chunk from file: '%s'", filename);
		return false;
	}
	scriptData->dataSize = chunkSize;

	while (chunkSize--)
		scriptData->data[chunkSize] = READ_BE_UINT16(&scriptData->data[chunkSize]);

	scriptData->opcodes = opcodes;

	strncpy(scriptData->filename, filename, 13);

	return true;
}

void EMCInterpreter::unload(EMCData *data) {
	if (!data)
		return;

	delete[] data->text;
	delete[] data->ordr;
	delete[] data->data;

	data->text = 0;
	data->ordr = data->data = 0;
}

void EMCInterpreter::init(EMCState *scriptStat, const EMCData *data) {
	scriptStat->dataPtr = data;
	scriptStat->ip = 0;
	scriptStat->stack[60] = 0;
	scriptStat->bp = 62;
	scriptStat->sp = 60;
}

bool EMCInterpreter::start(EMCState *script, int function) {
	if (!script->dataPtr)
		return false;

	uint16 functionOffset = script->dataPtr->ordr[function];
	if (functionOffset == 0xFFFF)
		return false;

	if (_vm->game() == GI_KYRA1) {
		if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
			script->ip = &script->dataPtr->data[functionOffset+1];
		else
			script->ip = &script->dataPtr->data[functionOffset];
	} else {
		script->ip = &script->dataPtr->data[functionOffset+1];
	}

	return true;
}

bool EMCInterpreter::isValid(EMCState *script) {
	if (!script->ip || !script->dataPtr || _vm->quit())
		return false;
	return true;
}

bool EMCInterpreter::run(EMCState *script) {
	_parameter = 0;

	if (!script->ip)
		return false;

	// Should be no Problem at all to cast to uint32 here, since that's the biggest ptrdiff the original
	// would allow, of course that's not realistic to happen to be somewhere near the limit of uint32 anyway.
	const uint32 instOffset = (uint32)((const byte*)script->ip - (const byte*)script->dataPtr->data);
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
		error("Script unknown command: %d in file '%s' at offset 0x%.08X", opcode, script->dataPtr->filename, instOffset);
	} else {
		debugC(5, kDebugLevelScript, "[0x%.08X] EMCInterpreter::%s([%d/%u])", instOffset, _commands[opcode].desc, _parameter, (uint)_parameter);
		(this->*(_commands[opcode].proc))(script);
	}

	return (script->ip != 0);
}

#pragma mark -
#pragma mark - ScriptFileParser implementation
#pragma mark -

void ScriptFileParser::setFile(const char *filename, Resource *res) {
	destroy();

	res->exists(filename, true);
	_stream = res->getFileStream(filename);
	assert(_stream);
	_startOffset = 0;
	_endOffset = _stream->size();
}

void ScriptFileParser::destroy() {
	delete _stream;
	_stream = 0;
	_startOffset = _endOffset = 0;
}

uint32 ScriptFileParser::getFORMBlockSize() {
	uint32 oldOffset = _stream->pos();

	uint32 data = _stream->readUint32LE();

	if (data != FORM_CHUNK) {
		_stream->seek(oldOffset);
		return (uint32)-1;
	}

	data = _stream->readUint32BE();
	return data;
}

uint32 ScriptFileParser::getIFFBlockSize(const uint32 chunkName) {
	uint32 size = (uint32)-1;

	_stream->seek(_startOffset + 0x0C);

	while (_stream->pos() < _endOffset) {
		uint32 chunk = _stream->readUint32LE();
		uint32 size_temp = _stream->readUint32BE();

		if (chunk != chunkName) {
			_stream->seek((size_temp + 1) & (~1), SEEK_CUR);
			assert(_stream->pos() <= _endOffset);
		} else {
			size = size_temp;
			break;
		}
	}

	return size;
}

bool ScriptFileParser::loadIFFBlock(const uint32 chunkName, void *loadTo, uint32 ptrSize) {
	_stream->seek(_startOffset + 0x0C);

	while (_stream->pos() < _endOffset) {
		uint32 chunk = _stream->readUint32LE();
		uint32 chunkSize = _stream->readUint32BE();

		if (chunk != chunkName) {
			_stream->seek((chunkSize + 1) & (~1), SEEK_CUR);
			assert(_stream->pos() <= _endOffset);
		} else {
			uint32 loadSize = 0;

			loadSize = MIN(ptrSize, chunkSize);
			_stream->read(loadTo, loadSize);
			return true;
		}
	}

	return false;
}

#pragma mark -
#pragma mark - Command implementations
#pragma mark -

void EMCInterpreter::cmd_jmpTo(EMCState* script) {
	script->ip = script->dataPtr->data + _parameter;
}

void EMCInterpreter::cmd_setRetValue(EMCState* script) {
	script->retValue = _parameter;
}

void EMCInterpreter::cmd_pushRetOrPos(EMCState* script) {
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
		script->ip = 0;
		break;
	}
}

void EMCInterpreter::cmd_push(EMCState* script) {
	script->stack[--script->sp] = _parameter;
}

void EMCInterpreter::cmd_pushReg(EMCState* script) {
	script->stack[--script->sp] = script->regs[_parameter];
}

void EMCInterpreter::cmd_pushBPNeg(EMCState* script) {
	script->stack[--script->sp] = script->stack[(-(int32)(_parameter + 2)) + script->bp];
}

void EMCInterpreter::cmd_pushBPAdd(EMCState* script) {
	script->stack[--script->sp] = script->stack[(_parameter - 1) + script->bp];
}

void EMCInterpreter::cmd_popRetOrPos(EMCState* script) {
	switch (_parameter) {
	case 0:
		script->retValue = script->stack[script->sp++];
		break;

	case 1:
		if (script->sp >= 60) {
			script->ip = 0;
		} else {
			script->bp = script->stack[script->sp++];
			script->ip = script->dataPtr->data + script->stack[script->sp++];
		}
		break;

	default:
		script->ip = 0;
		break;
	}
}

void EMCInterpreter::cmd_popReg(EMCState* script) {
	script->regs[_parameter] = script->stack[script->sp++];
}

void EMCInterpreter::cmd_popBPNeg(EMCState* script) {
	script->stack[(-(int32)(_parameter + 2)) + script->bp] = script->stack[script->sp++];
}

void EMCInterpreter::cmd_popBPAdd(EMCState* script) {
	script->stack[(_parameter - 1) + script->bp] = script->stack[script->sp++];
}

void EMCInterpreter::cmd_addSP(EMCState* script) {
	script->sp += _parameter;
}

void EMCInterpreter::cmd_subSP(EMCState* script) {
	script->sp -= _parameter;
}

void EMCInterpreter::cmd_execOpcode(EMCState* script) {
	uint8 opcode = _parameter;

	assert(script->dataPtr->opcodes);
	assert(opcode < script->dataPtr->opcodes->size());

	if ((*script->dataPtr->opcodes)[opcode] && ((*script->dataPtr->opcodes)[opcode])->isValid()) {
		script->retValue = (*(*script->dataPtr->opcodes)[opcode])(script);
	} else {
		script->retValue = 0;
		warning("Calling unimplemented opcode(0x%.02X/%d) from file '%s'", opcode, opcode, script->dataPtr->filename);
	}
}

void EMCInterpreter::cmd_ifNotJmp(EMCState* script) {
	if (!script->stack[script->sp++]) {
		_parameter &= 0x7FFF;
		script->ip = script->dataPtr->data + _parameter;
	}
}

void EMCInterpreter::cmd_negate(EMCState* script) {
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
		warning("Unknown negation func: %d", _parameter);
		script->ip = 0;
		break;
	}
}

void EMCInterpreter::cmd_eval(EMCState* script) {
	int16 ret = 0;
	bool error = false;

	int16 val1 = script->stack[script->sp++];
	int16 val2 = script->stack[script->sp++];

	switch (_parameter) {
	case 0:
		ret = (val2 && val1) ? 1 : 0;
		break;

	case 1:
		ret = (val2 || val1) ? 1 : 0;
		break;

	case 2:
		ret = (val1 == val2) ? 1 : 0;
		break;

	case 3:
		ret = (val1 != val2) ? 1 : 0;
		break;

	case 4:
		ret = (val1 > val2) ? 1 : 0;
		break;

	case 5:
		ret = (val1 >= val2) ? 1 : 0;
		break;

	case 6:
		ret = (val1 < val2) ? 1 : 0;
		break;

	case 7:
		ret = (val1 <= val2) ? 1 : 0;
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

	if (error)
		script->ip = 0;
	else
		script->stack[--script->sp] = ret;
}

void EMCInterpreter::cmd_setRetAndJmp(EMCState* script) {
	if (script->sp >= 60) {
		script->ip = 0;
	} else {
		script->retValue = script->stack[script->sp++];
		uint16 temp = script->stack[script->sp++];
		script->stack[60] = 0;
		script->ip = &script->dataPtr->data[temp];
	}
}
} // end of namespace Kyra

