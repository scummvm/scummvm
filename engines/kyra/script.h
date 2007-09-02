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

#ifndef KYRA_SCRIPT_H
#define KYRA_SCRIPT_H

#include "kyra/kyra.h"
#include "kyra/util.h"

#include "common/file.h"

namespace Kyra {

struct ScriptData {
	byte *text;
	uint16 *data;
	uint16 *ordr;
	uint16 dataSize;

	const Common::Array<const Opcode*> *opcodes;
};

struct ScriptState {
	uint16 *ip;
	const ScriptData *dataPtr;
	int16 retValue;
	uint16 bp;
	uint16 sp;
	int16 regs[30];		// VM registers
	int16 stack[61];	// VM stack
};

#define stackPos(x) script->stack[script->sp+x]
#define stackPosString(x) (const char*)&script->dataPtr->text[READ_BE_UINT16(&((uint16 *)script->dataPtr->text)[stackPos(x)])]

class ScriptFileParser {
public:
	ScriptFileParser() : _scriptFile(), _startOffset(0), _endOffset(0) {}
	ScriptFileParser(const char *filename, Resource *res) : _scriptFile(), _startOffset(0), _endOffset(0) { setFile(filename, res); } 
	~ScriptFileParser() { destroy(); }
	
	// 'script' must be allocated with new!
	void setFile(const char *filename, Resource *res);
	
	operator bool() const { return (_startOffset != _endOffset) && _scriptFile.isOpen(); }

	uint32 getFORMBlockSize();
	uint32 getIFFBlockSize(const uint32 chunk);
	bool loadIFFBlock(const uint32 chunk, void *loadTo, uint32 ptrSize);
private:
	void destroy();

	Common::File _scriptFile;
	uint32 _startOffset;
	uint32 _endOffset;
};

class ScriptHelper {
public:
	ScriptHelper(KyraEngine *vm);
	
	bool loadScript(const char *filename, ScriptData *data, const Common::Array<const Opcode*> *opcodes);
	void unloadScript(ScriptData *data);
	
	void initScript(ScriptState *scriptState, const ScriptData *data);
	bool startScript(ScriptState *script, int function);
	
	bool validScript(ScriptState *script);
	
	bool runScript(ScriptState *script);
protected:
	KyraEngine *_vm;
	int16 _parameter;
	bool _continue;

	typedef void (ScriptHelper::*CommandProc)(ScriptState*);
	struct CommandEntry {
		CommandProc proc;
		const char *desc;
	};
	
	const CommandEntry *_commands;
private:
	void cmd_jmpTo(ScriptState*);
	void cmd_setRetValue(ScriptState*);
	void cmd_pushRetOrPos(ScriptState*);
	void cmd_push(ScriptState*);
	void cmd_pushReg(ScriptState*);
	void cmd_pushBPNeg(ScriptState*);
	void cmd_pushBPAdd(ScriptState*);
	void cmd_popRetOrPos(ScriptState*);
	void cmd_popReg(ScriptState*);
	void cmd_popBPNeg(ScriptState*);
	void cmd_popBPAdd(ScriptState*);
	void cmd_addSP(ScriptState*);
	void cmd_subSP(ScriptState*);
	void cmd_execOpcode(ScriptState*);
	void cmd_ifNotJmp(ScriptState*);
	void cmd_negate(ScriptState*);
	void cmd_eval(ScriptState*);
	void cmd_setRetAndJmp(ScriptState*);
};
} // end of namespace Kyra

#endif

