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
 * $Header$
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
	/*command table ptr (uint32)*/
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
	
	bool loadScript(const char *filename, ScriptData *data, byte *specialPtr = 0);
	void unloadScript(ScriptData *data);
	
	void initScript(ScriptState *scriptStat, ScriptData *data);
	bool startScript(ScriptState *script, int function);
	
	bool validScript(ScriptState *script);
	
	bool runScript(ScriptState *script);
protected:
	uint32 getFORMBlockSize(byte *&data) const;
	uint32 getIFFBlockSize(byte *start, byte *&data, uint32 maxSize, const uint32 chunk) const;
	bool loadIFFBlock(byte *start, byte *&data, uint32 maxSize, const uint32 chunk, byte *loadTo, uint32 ptrSize) const;
	
	KyraEngine *_vm;
	ScriptState *_curScript;
	uint32 _parameter;
	bool _continue;

	typedef void (ScriptHelper::*CommandProc)();
	struct CommandEntry {
		CommandProc proc;
		const char* desc;
	};
	
	const CommandEntry *_commands;
private:
	void c1_jmpTo();
	void c1_setRetValue();
	void c1_pushRetOrPos();
	void c1_push();
	//void c1_push(); same as 03
	void c1_pushVar();
	void c1_pushBPNeg();
	void c1_pushBPAdd();
	void c1_popRetOrPos();
	void c1_popVar();
	void c1_popBPNeg();
	void c1_popBPAdd();
	void c1_addSP();
	void c1_subSP();
	void c1_execOpcode();
	void c1_ifNotJmp();
	void c1_negate();
	void c1_eval();
	void c1_setRetAndJmp();
};
} // end of namespace Kyra

#endif
