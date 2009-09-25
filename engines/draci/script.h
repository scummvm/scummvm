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

// TODO(spalek): shouldn't modify params passed by reference.  Either make it const or copy the parameter.
typedef void (Script::* GPLHandler)(Common::Queue<int> &);
typedef int  (Script::* GPLOperatorHandler)(int, int) const;
typedef int  (Script::* GPLFunctionHandler)(int) const;

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

	int run(const GPL2Program &program, uint16 offset);
	bool testExpression(const GPL2Program &program, uint16 offset) const;
	void endCurrentProgram();

private:
	
	int _jump;
	bool _endProgram;

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
	void icoStat(Common::Queue<int> &params);	
	void objStat(Common::Queue<int> &params);
	void objStatOn(Common::Queue<int> &params);
	void execInit(Common::Queue<int> &params);
	void execLook(Common::Queue<int> &params);
	void execUse(Common::Queue<int> &params);
	void walkOn(Common::Queue<int> &params);
	void walkOnPlay(Common::Queue<int> &params);
	void play(Common::Queue<int> &params);
	void startPlay(Common::Queue<int> &params);
	void newRoom(Common::Queue<int> &params);
	void talk(Common::Queue<int> &params);
	void loadMap(Common::Queue<int> &params);
	void roomMap(Common::Queue<int> &params);
	void dialogue(Common::Queue<int> &params);
	void exitDialogue(Common::Queue<int> &params);
	void resetDialogue(Common::Queue<int> &params);
	void resetDialogueFrom(Common::Queue<int> &params);
	void resetBlock(Common::Queue<int> &params);
	void setPalette(Common::Queue<int> &params);
	void blackPalette(Common::Queue<int> &params);
	void loadPalette(Common::Queue<int> &params);

	int operAnd(int op1, int op2) const;
	int operOr(int op1, int op2) const;
	int operXor(int op1, int op2) const;
	int operSub(int op1, int op2) const;
	int operAdd(int op1, int op2) const;
	int operDiv(int op1, int op2) const;
	int operMul(int op1, int op2) const;
	int operEqual(int op1, int op2) const;
	int operNotEqual(int op1, int op2) const;
	int operGreater(int op1, int op2) const;
	int operLess(int op1, int op2) const;
	int operGreaterOrEqual(int op1, int op2) const;
	int operLessOrEqual(int op1, int op2) const;
	int operMod(int op1, int op2) const;

	int funcRandom(int n) const;
	int funcNot(int n) const;
	int funcIsIcoOn(int iconID) const;
	int funcIcoStat(int iconID) const;
	int funcActIco(int iconID) const;
	int funcIsIcoAct(int iconID) const;
	int funcIsObjOn(int objID) const;
	int funcIsObjOff(int objID) const;
	int funcIsObjAway(int objID) const;
	int funcActPhase(int objID) const;
	int funcObjStat(int objID) const;
	int funcLastBlock(int blockID) const;
	int funcAtBegin(int yesno) const;
	int funcBlockVar(int blockID) const;
	int funcHasBeen(int blockID) const;
	int funcMaxLine(int lines) const;

	void setupCommandList();
	const GPL2Command *findCommand(byte num, byte subnum) const;
	int handleMathExpression(Common::MemoryReadStream &reader) const;

	DraciEngine *_vm;
};

}

#endif // DRACI_SCRIPT_H
