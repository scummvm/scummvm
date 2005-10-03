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

#ifndef KYRASCRIPT_H
#define KYRASCRIPT_H

namespace Kyra {
// TODO:
// find out more script functions
enum ScriptFunc {
	kSetupScene = 0,
	kClickEvent = 1, // _registers[1] and _registers[2] are mouse x, y _registers[4] is action
	kActorEvent = 2,
	kEnterEvent = 4,
	kExitEvent = 5,
	kLoadResources = 7
};

enum ScriptState {
	kScriptStopped = 0,
	kScriptRunning = 1,
	kScriptWaiting = 2,
	kScriptError = 3
};


class VMContext {

public:

	VMContext(KyraEngine* engine);
	~VMContext() { delete [] _scriptFile; }

	void loadScript(const char* file);

	const char* stringAtIndex(int32 index);

	// TODO: check for 'over'flow
	void pushStack(int32 value) { _stack[_stackPos++] = value; }
	void registerValue(int32 reg, int32 value) { _registers[reg] = value; }
	int32 checkReg(int32 reg) { return _registers[reg]; }

	uint32 state(void) { return _scriptState; }

	bool startScript(int32 func);
	uint32 contScript(void);

protected:
	KyraEngine* _engine;
	uint8* _scriptFile;
	uint32 _scriptFileSize;

	uint32 _scriptState;
	uint32 _delay;

	int32 _registers[32]; // registers of the interpreter
	int32 _stack[64]; // our stack

	// TODO: check for 'under'flow
	int32 popStack(void) { return _stack[--_stackPos]; }
	int32& topStack(void) { return _stack[_stackPos]; }

	uint32 _returnValue;

	int32 _nextScriptPos;
	int32 _instructionPos;
	int32 _stackPos;
	int32 _tempPos;

	// used by command & opcode procs
	uint16 _argument;
	uint8 _currentCommand;
	uint32 _currentOpcode;

	int32 param(int32 index);
	const char* paramString(int32 index) { return stringAtIndex(param(index)); }

	bool _error;	// used by all command- and opcodefuncs

	enum ScriptChunkTypes {
		kForm = 0,
		kEmc2Ordr = 1,
		kText = 2,
		kData = 3,
		kCountChunkTypes
	};

	struct ScriptChunk {
		uint32 _size;
		uint8* _data; // by TEXT used for count of texts, by EMC2ODRD it is used for a count of somewhat
		uint8* _additional; // currently only used for TEXT
	};

	ScriptChunk _chunks[kCountChunkTypes];

	typedef void (VMContext::*CommandProc)();
	struct CommandEntry {
		CommandProc proc;
		const char* desc;
	};

	typedef void (VMContext::*OpcodeProc)();
	struct OpcodeEntry {
		OpcodeProc proc;
		const char* desc;
	};

	uint16 _numCommands;
	const CommandEntry* _commands;
	uint16 _numOpcodes;
	const OpcodeEntry* _opcodes;

protected:
	// the command procs
	void c1_goToLine(void);			// 0x00
	void c1_setReturn(void);		// 0x01
	void c1_pushRetRec(void);		// 0x02
	void c1_push(void);			// 0x03 & 0x04
	void c1_pushVar(void);			// 0x05
	void c1_pushFrameNeg(void);		// 0x06
	void c1_pushFramePos(void);		// 0x07
	void c1_popRetRec(void);		// 0x08
	void c1_popVar(void);			// 0x09
	void c1_popFrameNeg(void);		// 0x0A
	void c1_popFramePos(void);		// 0x0B
	void c1_addToSP(void);			// 0x0C
	void c1_subFromSP(void);		// 0x0D
	void c1_execOpcode(void);		// 0x0E
	void c1_ifNotGoTo(void);		// 0x0F
	void c1_negate(void);			// 0x10
	void c1_evaluate(void);			// 0x11
	void c1_unknownCommand(void);

	// the opcode procs
	void o1_0x68(void);			// 0x68
	void o1_unknownOpcode(void);
};
} // end of namespace Kyra

#endif

