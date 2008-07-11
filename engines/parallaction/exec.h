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


#ifndef PARALLACTION_EXEC_H
#define PARALLACTION_EXEC_H

#include "common/util.h"
#include "parallaction/objects.h"


namespace Parallaction {

typedef Common::Functor0<void> Opcode;
typedef Common::Array<const Opcode*>	OpcodeSet;

#define DECLARE_UNQUALIFIED_COMMAND_OPCODE(op) void cmdOp_##op()
#define DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(op) void instOp_##op()

class Parallaction_ns;
class Parallaction_br;

class CommandExec {
protected:
	struct ParallactionStruct1 {
		CommandPtr cmd;
		ZonePtr	z;
	} _cmdRunCtxt;

	OpcodeSet	_opcodes;

public:
	virtual void init() = 0;
	virtual void run(CommandList &list, ZonePtr z = nullZonePtr);
	CommandExec() {
	}
	virtual ~CommandExec() {
		for (Common::Array<const Opcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
			delete *i;
		_opcodes.clear();
	}
};

class CommandExec_ns : public CommandExec {

	Parallaction_ns	*_vm;

protected:
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(invalid);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(set);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(clear);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(start);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(speak);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(get);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(location);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(open);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(close);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(on);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(off);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(call);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(toggle);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(drop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(quit);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(move);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(stop);

public:
	void init();

	CommandExec_ns(Parallaction_ns* vm);
	~CommandExec_ns();
};

class CommandExec_br : public CommandExec_ns {

protected:
	Parallaction_br	*_vm;

	DECLARE_UNQUALIFIED_COMMAND_OPCODE(location);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(open);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(close);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(on);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(off);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(call);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(drop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(move);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(start);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(stop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(character);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(followme);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(onmouse);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(offmouse);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(add);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(leave);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(inc);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(dec);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ifeq);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(iflt);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ifgt);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(let);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(music);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(fix);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(unfix);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(zeta);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(scroll);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(swap);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(give);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(text);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(part);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(testsfx);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ret);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(onsave);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(offsave);

public:
	void		init();

	CommandExec_br(Parallaction_br* vm);
	~CommandExec_br();
};





class ProgramExec {
protected:
	struct ParallactionStruct2 {
		AnimationPtr	anim;
		ProgramPtr		program;
		InstructionList::iterator inst;
		uint16		modCounter;
		bool		suspend;
	} _instRunCtxt;

	OpcodeSet	_opcodes;

public:
	virtual void init() = 0;
	virtual void runScripts(ProgramList::iterator first, ProgramList::iterator last);
	ProgramExec() {
	}
	virtual ~ProgramExec() {
		for (Common::Array<const Opcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
			delete *i;
		_opcodes.clear();
	}
};

class ProgramExec_ns : public ProgramExec {

	Parallaction_ns *_vm;

protected:
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(invalid);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(on);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(off);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endloop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(null);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(call);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(wait);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(start);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(sound);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endscript);

public:
	void init();

	ProgramExec_ns(Parallaction_ns *vm);
	~ProgramExec_ns();
};

class ProgramExec_br : public ProgramExec_ns {

	Parallaction_br *_vm;

protected:
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(on);
 	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(off);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(dec);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(wait);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(start);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(process);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(color);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(mask);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(print);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(text);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(mul);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(div);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(ifeq);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(iflt);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(ifgt);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endif);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(stop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endscript);

public:
	void init();
	ProgramExec_br(Parallaction_br *vm);
	~ProgramExec_br();
};

} // namespace Parallaction

#endif
