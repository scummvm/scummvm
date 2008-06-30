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

#ifndef MADE_SCRIPT_H
#define MADE_SCRIPT_H

#include "common/util.h"
#include "common/file.h"
#include "common/stream.h"

namespace Made {

// Define this to dump all game scripts and a usage statistic of all
// opcodes/extended functions instead of running the actual game.
// Then run ScummVM with debuglevel 1.
//#define DUMP_SCRIPTS

class MadeEngine;
class ScriptFunctions;

const int kScriptStackSize = 1000;
const int kScriptStackLimit = kScriptStackSize + 1;

class ScriptStack {
public:
	ScriptStack();
	~ScriptStack();
	int16 top();
	int16 pop();
	void push(int16 value = 0);
	void setTop(int16 value);
	int16 peek(int16 index);
	void poke(int16 index, int16 value);
	void alloc(int16 count);
	void free(int16 count);
	int16 getStackPos() const { return _stackPos; }
	void setStackPos(int16 stackPtr);
	int16 *getStackPtr();
protected:
	int16 _stack[kScriptStackSize];
	int16 _stackPos;
};

class ScriptInterpreter {
public:
	ScriptInterpreter(MadeEngine *vm);
	~ScriptInterpreter();
	void runScript(int16 scriptObjectIndex);
	void dumpScript(int16 objectIndex, int *opcodeStats, int *externStats);
	void dumpAllScripts();
protected:
	MadeEngine *_vm;

	ScriptStack _stack;
	int16 _localStackPos;
	int16 _runningScriptObjectIndex;
	byte *_codeBase, *_codeIp;
	bool _terminated;

	ScriptFunctions *_functions;

	byte readByte();
	int16 readInt16();

	typedef void (ScriptInterpreter::*CommandProc)();
	struct CommandEntry {
		CommandProc proc;
		const char *desc;
#ifdef DUMP_SCRIPTS
		const char *sig;
#endif
	};

	const CommandEntry *_commands;
	int16 _commandsMax;

	void cmd_branchTrue();
	void cmd_branchFalse();
	void cmd_branch();
	void cmd_true();
	void cmd_false();
	void cmd_push();
	void cmd_not();
	void cmd_add();
	void cmd_sub();
	void cmd_mul();
	void cmd_div();
	void cmd_mod();
	void cmd_band();
	void cmd_bor();
	void cmd_bnot();
	void cmd_lt();
	void cmd_eq();
	void cmd_gt();
	void cmd_loadConstant();
	void cmd_loadVariable();
	void cmd_getObjectProperty();
	void cmd_setObjectProperty();
	void cmd_set();
	void cmd_print();
	void cmd_terpri();
	void cmd_printNumber();
	void cmd_vref();
	void cmd_vset();
	void cmd_vsize();
	void cmd_exit();
	void cmd_return();
	void cmd_call();
	void cmd_svar();
	void cmd_sset();
	void cmd_split();
	void cmd_snlit();
	void cmd_yorn();
	void cmd_save();
	void cmd_restore();
	void cmd_arg();
	void cmd_aset();
	void cmd_tmp();
	void cmd_tset();
	void cmd_tspace();
	void cmd_class();
	void cmd_objectp();
	void cmd_vectorp();
	void cmd_restart();
	void cmd_rand();
	void cmd_randomize();
	void cmd_send();
	void cmd_extend();
	void cmd_catch();
	void cmd_cdone();
	void cmd_throw();
	void cmd_functionp();
	void cmd_le();
	void cmd_ge();
	void cmd_varx();
	void cmd_setx();

};

} // End of namespace Made

#endif /* MADE_H */
