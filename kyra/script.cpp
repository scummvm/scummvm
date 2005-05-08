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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "kyra.h"
#include "script.h"
#include "resource.h"

#include "common/stream.h"
#include "common/util.h"

#define COMMAND(x) { &VMContext::x, #x }
#define OPCODE(x) { &VMContext::x, #x }

namespace Kyra {
VMContext::VMContext(KyraEngine* engine) {
	_engine = engine;
	_error = false;
		
	// now we create a list of all Command/Opcode procs and so
	static CommandEntry commandProcs[] = {
		// 0x00
		COMMAND(c1_goToLine),
		COMMAND(c1_setReturn),
		COMMAND(c1_pushRetRec),
		COMMAND(c1_push),
		// 0x04
		COMMAND(c1_push),			
		COMMAND(c1_pushVar),
		COMMAND(c1_pushFrameNeg),
		COMMAND(c1_pushFramePos),
		// 0x08
		COMMAND(c1_popRetRec),
		COMMAND(c1_popVar),
		COMMAND(c1_popFrameNeg),
		COMMAND(c1_popFramePos),
		// 0x0C
		COMMAND(c1_addToSP),
		COMMAND(c1_subFromSP),
		COMMAND(c1_execOpcode),
		COMMAND(c1_ifNotGoTo),
		// 0x10
		COMMAND(c1_negate),
		COMMAND(c1_evaluate)
	};
	_numCommands = ARRAYSIZE(commandProcs);
	_commands = commandProcs;
		
	static OpcodeEntry opcodeProcs[] = {
		// 0x00
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x04
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x08
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x0C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x10
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x14
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x18
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x1C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x20
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x24
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x28
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x2C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x30
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x34
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x38
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x3C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x40
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x44
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x48
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x4C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x50
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x54
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x58
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x5C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x60
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x64
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x68
		OPCODE(o1_0x68),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x6C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x70
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x74
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x78
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x7C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x80
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x84
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x88
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x8C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x90
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x94
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x98
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0x9C
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xA0
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xA4
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xA8
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xAC
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xB0
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xB4
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xB8
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xBC
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xC0
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xC4
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xC8
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xCC
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xD0
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xD4
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xD8
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xDC
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xE0
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xE4
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xE8
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xEC
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xF0
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xF4
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xF8
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		// 0xFC
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode),
		OPCODE(o1_unknownOpcode)
	};
	_numOpcodes = ARRAYSIZE(opcodeProcs);
	_opcodes = opcodeProcs;

	_scriptFile = NULL;
	_scriptFileSize = 0;
}
	
void VMContext::loadScript(const char* file) {
	if (_scriptFile) {
		delete [] _scriptFile;
		_scriptFileSize = 0;
	}
		
	memset(_stack, 0, sizeof(int32) * ARRAYSIZE(_stack));

	// loads the new file
	_scriptFile = _engine->resManager()->fileData(file, &_scriptFileSize);
	
	if (!_scriptFileSize || !_scriptFile) {
		error("couldn't load script file '%s'", file);
	}
	
	Common::MemoryReadStream script(_scriptFile, _scriptFileSize);
	memset(_chunks, 0, sizeof(ScriptChunk) * kCountChunkTypes);
	uint8 chunkName[sizeof("EMC2ORDR") + 1];
	
	// so lets look for our chunks :)
	while (!script.eos()) {
		// lets read only the first 4 chars
		script.read(chunkName, sizeof(uint8) * 4);
		chunkName[4] = '\0';
			
		// check name of chunk
		if (!scumm_stricmp((const char *)chunkName, "FORM")) {			
			// FreeKyra swaps the size I only read it in BigEndian :)
			_chunks[kForm]._size = script.readUint32BE();
		} else if (!scumm_stricmp((const char *)chunkName, "TEXT")) {
			uint32 text_size = script.readUint32BE();
			text_size += text_size % 2 != 0 ? 1 : 0;
			
			_chunks[kText]._data = _scriptFile + script.pos();
			_chunks[kText]._size = READ_BE_UINT16(_chunks[kText]._data) >> 1;
			_chunks[kText]._additional = _chunks[kText]._data + (_chunks[kText]._size << 1);
			script.seek(script.pos() + text_size);
		} else if (!scumm_stricmp((const char *)chunkName, "DATA")) {
			_chunks[kData]._size = script.readUint32BE();
			_chunks[kData]._data = _scriptFile + script.pos();
			// mostly it will be the end of the file because all files should end with a 'DATA' chunk
			script.seek(script.pos() + _chunks[kData]._size);
		} else {
			// read next 4 chars
			script.read(&chunkName[4], sizeof(uint8) * 4);
			chunkName[8] = '\0';
			
			if (!scumm_stricmp((const char *)chunkName, "EMC2ORDR")) {
				_chunks[kEmc2Ordr]._size = script.readUint32BE() >> 1;
				_chunks[kEmc2Ordr]._data = _scriptFile + script.pos();
				script.seek(script.pos() + _chunks[kEmc2Ordr]._size * 2);
			} else {
				// any unkown chunk or problems with seeking through the file
				error("unknown chunk(%s)", chunkName);
			}
		}
	}
}
	
int32 VMContext::param(int32 index) {
	if (_stackPos - index - 1 >= ARRAYSIZE(_stack) || _stackPos - index - 1 < 0)
		return -0xFFFF;
	return _stack[_stackPos - index - 1];
}
	
const char* VMContext::stringAtIndex(int32 index) {
	if (index < 0 || (uint32)index >= _chunks[kText]._size)
		return 0;
	
	return (const char *)(_chunks[kText]._additional + _chunks[kText]._data[index]);
}
	
bool VMContext::startScript(int32 func) {
	if ((uint32)func >= _chunks[kEmc2Ordr]._size || func < 0) {
		debug("script doesn't support function %d", func);
		return false;
	}
			
	_instructionPos = READ_BE_UINT16(&_chunks[kEmc2Ordr]._data[func]) << 1;
	_stackPos = 0;
	_tempPos = 0;
	_delay = 0;
	_error = false;
	_scriptState = kScriptRunning;
	
	uint32 pos = 0xFFFFFFFE;
		
	// get start of next script
	for (uint32 tmp = 0; tmp < _chunks[kEmc2Ordr]._size; ++tmp) {
		if ((uint32)((READ_BE_UINT16(&_chunks[kEmc2Ordr]._data[tmp]) << 1)) > (uint32)_instructionPos &&
			(uint32)((READ_BE_UINT16(&_chunks[kEmc2Ordr]._data[tmp]) << 1)) < pos) {
			pos = ((READ_BE_UINT16(&_chunks[kEmc2Ordr]._data[tmp]) << 1));
		}
	}
		
	if (pos > _scriptFileSize) {
		pos = _scriptFileSize;
	}
		
	_nextScriptPos = pos;

	return true;
}
	
uint32 VMContext::contScript(void) {
	uint8* script_start = _chunks[kData]._data;
	assert(script_start);
	
	uint32 scriptStateAtStart = _scriptState;
		
	// runs the script
	while (true) {
		if ((uint32)_instructionPos > _chunks[kData]._size) {
			debug("_instructionPos( = %d) > _chunks[kData]._size( = %d)", _instructionPos, _chunks[kData]._size);
			_error = true;
			break;
		} else if (_instructionPos >= _nextScriptPos) {
			_scriptState = kScriptStopped;
			break;
		}
			
		_currentCommand = *(script_start + _instructionPos++);
		
		// gets out 
		if (_currentCommand & 0x80) {
			_argument = ((_currentCommand & 0x0F) << 8) | *(script_start + _instructionPos++);
			_currentCommand &= 0xF0;
		} else if (_currentCommand & 0x40) {
			_argument = *(script_start + _instructionPos++);
		} else if (_currentCommand & 0x20) {
			_instructionPos++;
			
			uint16 tmp = *(uint16*)(script_start + _instructionPos);
			tmp &= 0xFF7F;
			
			_argument = READ_BE_UINT16(&tmp);
			_instructionPos += 2;
		} else {
			debug("unknown way of getting the command (0x%X)", _currentCommand);
			// next thing
			continue;
		}
		
		_currentCommand &= 0x1f;
			
		if (_currentCommand < _numCommands) {
			CommandProc currentProc = _commands[_currentCommand].proc;
			(this->*currentProc)();
		} else {
			c1_unknownCommand();
		}
		
		if (_error) {
			_scriptState = kScriptError;
			break;
		}
			
		if (scriptStateAtStart != _scriptState) {
			break;
		}
	}

	return _scriptState;
}
} // end of namespace Kyra
