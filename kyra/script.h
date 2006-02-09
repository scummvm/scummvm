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

#ifndef KYRASCRIPT_H
#define KYRASCRIPT_H

#include "kyra/kyra.h"

namespace Kyra {
struct ScriptData {
	byte *text;
	byte *data;
	byte *ordr;
	uint16 dataSize;
	KyraEngine::OpcodeProc *opcodes;
	int opcodeSize;
	uint16 mustBeFreed;
};

struct ScriptState {
	byte *ip;
	ScriptData *dataPtr;
	int16 retValue;
	uint16 bp;
	uint16 sp;
	int16 variables[30];
	int16 stack[61];
};

enum {
	SCRIPT_INIT = 0
};

class ScriptHelper {
public:
	ScriptHelper(KyraEngine *vm);
	virtual ~ScriptHelper();
	
	bool loadScript(const char *filename, ScriptData *data, KyraEngine::OpcodeProc *opcodes, int opcodeSize, byte *specialPtr = 0);
	void unloadScript(ScriptData *data);
	
	void initScript(ScriptState *scriptState, ScriptData *data);
	bool startScript(ScriptState *script, int function);
	
	bool validScript(ScriptState *script);
	
	bool runScript(ScriptState *script);
protected:
	uint32 getFORMBlockSize(byte *&data) const;
	uint32 getIFFBlockSize(byte *start, byte *&data, uint32 maxSize, const uint32 chunk) const;
	bool loadIFFBlock(byte *start, byte *&data, uint32 maxSize, const uint32 chunk, byte *loadTo, uint32 ptrSize) const;
	
	KyraEngine *_vm;
	int16 _parameter;
	bool _continue;

	typedef void (ScriptHelper::*CommandProc)(ScriptState*);
	struct CommandEntry {
		CommandProc proc;
		const char* desc;
	};
	
	const CommandEntry *_commands;
private:
	void c1_jmpTo(ScriptState*);
	void c1_setRetValue(ScriptState*);
	void c1_pushRetOrPos(ScriptState*);
	void c1_push(ScriptState*);
	//void c1_push(); same as 03
	void c1_pushVar(ScriptState*);
	void c1_pushBPNeg(ScriptState*);
	void c1_pushBPAdd(ScriptState*);
	void c1_popRetOrPos(ScriptState*);
	void c1_popVar(ScriptState*);
	void c1_popBPNeg(ScriptState*);
	void c1_popBPAdd(ScriptState*);
	void c1_addSP(ScriptState*);
	void c1_subSP(ScriptState*);
	void c1_execOpcode(ScriptState*);
	void c1_ifNotJmp(ScriptState*);
	void c1_negate(ScriptState*);
	void c1_eval(ScriptState*);
	void c1_setRetAndJmp(ScriptState*);
};
} // end of namespace Kyra

#endif
