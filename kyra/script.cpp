/* ScummVM - Kyrandia Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#define COMMAND(x) { &VMContext::x, #x }
#define OPCODE(x) { &VMContext::x, #x }

namespace Kyra {
	VMContext::VMContext(KyraEngine* engine) {
		_engine = engine;
		
		// now we create a list of all Command/Opcode procs and so
		static CommandEntry commandProcs[] = {
			// 0
			COMMAND(c1_goToLine),
			COMMAND(c1_setReturn),
			COMMAND(c1_pushRetRec),
			COMMAND(c1_push),
			COMMAND(c1_push),			
			COMMAND(c1_pushVar),
			COMMAND(c1_pushFrameNeg),
			COMMAND(c1_pushFramePos),
			COMMAND(c1_popRetRec),
			COMMAND(c1_popVar),
			
			// 10
			COMMAND(c1_popFrameNeg),
			COMMAND(c1_popFramePos),
			COMMAND(c1_addToSP),
			COMMAND(c1_subFromSP),
			COMMAND(c1_execOpcode),			
			COMMAND(c1_ifNotGoTo),
			COMMAND(c1_negate),
			COMMAND(c1_evaluate),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 20
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 30
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 40
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 50
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 60
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 70
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 80
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 90
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 100
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 110
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 120
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 130
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 140
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 150
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 160
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 170
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 180
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 190
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 200
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 210
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 220
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 230
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 240
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),			
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			
			// 250
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			COMMAND(c1_unknownCommand),
			{ 0, 0 }
		};
		_commands = commandProcs;
		static OpcodeEntry opcodeProcs[] = {
			// 0
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 10
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 20
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 30
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 40
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 50
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 60
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 70
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 80
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 90
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 100
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_0x68),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 110
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 120
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 130
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 140
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 150
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 160
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 170
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 180
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 190
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 200
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 210
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 220
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 230
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 240
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),			
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			
			// 250
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			COMMAND(o1_unknownOpcode),
			{ 0, 0 }
		};
		_opcodes = opcodeProcs;

		_scriptFile = NULL;
		_scriptFileSize = 0;
	}
	
	void VMContext::loadScript(const char* file) {
		if (_scriptFile) {
			delete [] _scriptFile;
			_scriptFileSize = 0;
		}
		
		debug("--------------");
		
		// loads the new file
		_scriptFile = _engine->resManager()->fileData(file, &_scriptFileSize);
		
		if (!_scriptFileSize || !_scriptFile) {
			error("couldn't load script file '%s'", file);
		}
		
		Common::MemoryReadStream script(_scriptFile, _scriptFileSize);
		memset(_chunks, 0, sizeof(ScriptChunk) * kCountChunkTypes);
		uint8 chunkName[sizeof("EMC2ORDR") + 1];
		
		// so lets look for our chunks :)
		while(true) {
			if (script.eof()) {
				break;
			}		
			// lets read only the first 4 chars
			script.read(chunkName, sizeof(uint8) * 4);
			chunkName[4] = '\0';
			debug("chunk name(4 chars): '%s'", chunkName);
			
			// check name of chunk
			if (!scumm_stricmp((char*)chunkName, "FORM")) {			
				// FreeKyra swaps the size I only read it in BigEndian :)
				_chunks[kForm]._size = script.readUint32BE();	
				debug("_chunks[kForm]._size = %d", _chunks[kForm]._size);				
			} else if (!scumm_stricmp((char*)chunkName, "TEXT")) {
				uint32 text_size = script.readUint32BE();
				text_size += text_size % 2 != 0 ? 1 : 0;
				
				_chunks[kText]._data = _scriptFile + script.pos();
				_chunks[kText]._size = READ_BE_UINT16(_chunks[kText]._data) >> 1;
				_chunks[kText]._additional = _chunks[kText]._data + (_chunks[kText]._size << 1);				
				debug("_chunks[kText]._size = %d, real chunk size = %d", _chunks[kText]._size, text_size);	
				
				script.seek(script.pos() + text_size);
			} else if (!scumm_stricmp((char*)chunkName, "DATA")) {
				_chunks[kData]._size = script.readUint32BE();
				_chunks[kData]._data = _scriptFile + script.pos();				
				debug("_chunks[kData]._size = %d", _chunks[kData]._size);
				
				// mostly it will be the end of the file because all files should end with a 'DATA' chunk
				script.seek(script.pos() + _chunks[kData]._size);
			} else {
				// read next 4 chars
				script.read(&chunkName[4], sizeof(uint8) * 4);
				chunkName[8] = '\0';
				debug("chunk name(8 chars): '%s'", chunkName);
				
				if (!scumm_stricmp((char*)chunkName, "EMC2ORDR")) {
					_chunks[kEmc2Ordr]._size = script.readUint32BE() >> 1;
					_chunks[kEmc2Ordr]._data = _scriptFile + script.pos();					
					debug("_chunks[kEmc2Ordr]._size = %d, real chunk size = %d", _chunks[kEmc2Ordr]._size, _chunks[kEmc2Ordr]._size * 2);
					
					script.seek(script.pos() + _chunks[kEmc2Ordr]._size * 2);
				} else {
					// any unkown chunk or problems with seeking through the file
					error("unknown chunk");
				}
			}
		}
		
		// so file loaded
		debug("--------------");
	}
	
	int32 VMContext::param(int32 index) {
		if (_stackPos - index + 1 >= 16 || _stackPos - index + 1 < 0)
			return -0xFFFF;
		return _stack[_stackPos - index + 1];
	}
	
	const char* VMContext::stringAtIndex(int32 index) {
		if (index < 0 || (uint32)index >= _chunks[kText]._size)
			return 0;
		
		return (char*)(_chunks[kText]._additional + _chunks[kText]._data[index]);
	}
	
	bool VMContext::startScript(int32 func) {
		if ((uint32)func >= _chunks[kEmc2Ordr]._size || func < 0) {
			debug("script doesn't support function %d", func);
			return false;
		}
			
		_instructionPos = (READ_BE_UINT16(&_chunks[kEmc2Ordr]._data[func]) << 1) + 2;
		_stackPos = 0;
		_tempPos = 0;
		_delay = 0;
		_scriptState = kScriptRunning;
		return true;
	}
	
	uint32 VMContext::contScript(void) {
		uint8* script_start = _chunks[kData]._data;
		assert(script_start);
		
		uint32 scriptStateAtStart = _scriptState;
		_error = false;

		// runs the script
		while(true) {
			if ((uint32)_instructionPos > _chunks[kData]._size) {
				debug("_instructionPos( = %d) > _chunks[kData]._size( = %d)", _instructionPos, _chunks[kData]._size);
				_error = true;
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
				debug("unknown way of getting the command");
			}
			
			_currentCommand &= 0x1f;
		
			CommandProc currentProc = _commands[_currentCommand].proc;
			(this->*currentProc)();
			
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
