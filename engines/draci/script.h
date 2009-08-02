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

#ifndef DRACI_SCRIPT_H
#define DRACI_SCRIPT_H

#include "common/str.h"
#include "common/stream.h"
#include "common/queue.h"

namespace Draci {

/** The maximum number of parameters for a GPL command */
const int kMaxParams = 3; 

class DraciEngine;
class Script;

enum {
	kNumCommands = 55
};

typedef void (Script::* GPLHandler)(Common::Queue<int> &);
typedef int  (Script::* GPLOperatorHandler)(int, int);
typedef int  (Script::* GPLFunctionHandler)(int);

/**
 *  Represents a single command in the GPL scripting language bytecode.
 *	Each command is represented in the bytecode by a command number and a 
 *	subnumber.
 */

struct GPL2Command { 
	byte _number; 
	byte _subNumber; 
	Common::String _name; 
	uint16 _numParams;
	int _paramTypes[kMaxParams];
	GPLHandler _handler;
};

struct GPL2Operator {
	Common::String _name;
	GPLOperatorHandler _handler;
};

struct GPL2Function {
	Common::String _name;
	GPLFunctionHandler _handler;
};

/** 
 *  A convenience data type that holds both the actual bytecode and the
 *  length of the bytecode. Passed to Script::run().
 */

struct GPL2Program {
	GPL2Program() : _bytecode(NULL), _length(0) {}
	
	byte *_bytecode;
	uint16 _length;
};

class Script {

public:
	Script(DraciEngine *vm) : _vm(vm), _jump(0) { setupCommandList(); };	

	int run(GPL2Program program, uint16 offset);

private:
	
	int _jump;

	/** List of all GPL commands. Initialised in the constructor. */
	const GPL2Command *_commandList;
	const GPL2Operator *_operatorList;
	const GPL2Function *_functionList;
 
	void c_If(Common::Queue<int> &params);
	void c_Goto(Common::Queue<int> &params);
	void c_Let(Common::Queue<int> &params);
	void load(Common::Queue<int> &params);
	void start(Common::Queue<int> &params);
	void mark(Common::Queue<int> &params);
	void release(Common::Queue<int> &params);
	void objStat(Common::Queue<int> &params);
	void objStatOn(Common::Queue<int> &params);
	void execInit(Common::Queue<int> &params);
	void execLook(Common::Queue<int> &params);
	void execUse(Common::Queue<int> &params);
	void walkOn(Common::Queue<int> &params);
	void play(Common::Queue<int> &params);
	void startPlay(Common::Queue<int> &params);
	void newRoom(Common::Queue<int> &params);
	void talk(Common::Queue<int> &params);

	int operAnd(int op1, int op2);
	int operOr(int op1, int op2);
	int operXor(int op1, int op2);
	int operSub(int op1, int op2);
	int operAdd(int op1, int op2);
	int operDiv(int op1, int op2);
	int operMul(int op1, int op2);
	int operEqual(int op1, int op2);
	int operNotEqual(int op1, int op2);
	int operGreater(int op1, int op2);
	int operLess(int op1, int op2);
	int operGreaterOrEqual(int op1, int op2);
	int operLessOrEqual(int op1, int op2);
	int operMod(int op1, int op2);

	int funcRandom(int n);
	int funcNot(int n);
	int funcIsIcoOn(int iconID);
	int funcIcoStat(int iconID);
	int funcIsObjOn(int objID);
	int funcIsObjOff(int objID);
	int funcIsObjAway(int objID);
	int funcActPhase(int objID);


	void setupCommandList();
	const GPL2Command *findCommand(byte num, byte subnum);
	int handleMathExpression(Common::MemoryReadStream &reader);

	DraciEngine *_vm;
};

}

#endif // DRACI_SCRIPT_H
