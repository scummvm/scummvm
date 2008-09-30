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

#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/sound.h"


namespace Parallaction {

#define INST_ON							1
#define INST_OFF						2
#define INST_X							3
#define INST_Y							4
#define INST_Z							5
#define INST_F							6
#define INST_LOOP						7
#define INST_ENDLOOP					8
#define INST_SHOW						9
#define INST_INC						10
#define INST_DEC						11
#define INST_SET						12
#define INST_PUT						13
#define INST_CALL						14
#define INST_WAIT						15
#define INST_START						16
#define INST_SOUND						17
#define INST_MOVE						18
#define INST_ENDSCRIPT					19

#define SetOpcodeTable(x) table = &x;

typedef Common::Functor0Mem<void, CommandExec_ns> OpcodeV1;
#define COMMAND_OPCODE(op) table->push_back(new OpcodeV1(this, &CommandExec_ns::cmdOp_##op))
#define DECLARE_COMMAND_OPCODE(op) void CommandExec_ns::cmdOp_##op()

typedef Common::Functor0Mem<void, ProgramExec_ns> OpcodeV2;
#define INSTRUCTION_OPCODE(op) table->push_back(new OpcodeV2(this, &ProgramExec_ns::instOp_##op))
#define DECLARE_INSTRUCTION_OPCODE(op) void ProgramExec_ns::instOp_##op()

extern const char *_instructionNamesRes_ns[];


DECLARE_INSTRUCTION_OPCODE(on) {
	InstructionPtr inst = *_ctxt.inst;

	inst->_a->_flags |= kFlagsActive;
	inst->_a->_flags &= ~kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(off) {
	(*_ctxt.inst)->_a->_flags |= kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(loop) {
	InstructionPtr inst = *_ctxt.inst;

	_ctxt.program->_loopCounter = inst->_opB.getValue();
	_ctxt.program->_loopStart = _ctxt.ip;
}


DECLARE_INSTRUCTION_OPCODE(endloop) {
	if (--_ctxt.program->_loopCounter > 0) {
		_ctxt.ip = _ctxt.program->_loopStart;
	}
}

DECLARE_INSTRUCTION_OPCODE(inc) {
	InstructionPtr inst = *_ctxt.inst;
	int16 _si = inst->_opB.getValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (_si > 0 ? _si : -_si);
		if (_ctxt.modCounter % _bx != 0) return;

		_si = (_si > 0 ?  1 : -1);
	}

	int16 lvalue = inst->_opA.getValue();

	if (inst->_index == INST_INC) {
		lvalue += _si;
	} else {
		lvalue -= _si;
	}

	inst->_opA.setValue(lvalue);

}


DECLARE_INSTRUCTION_OPCODE(set) {
	InstructionPtr inst = *_ctxt.inst;
	inst->_opA.setValue(inst->_opB.getValue());
}


DECLARE_INSTRUCTION_OPCODE(put) {
	InstructionPtr inst = *_ctxt.inst;
	Graphics::Surface v18;
	v18.w = inst->_a->width();
	v18.h = inst->_a->height();
	v18.pixels = inst->_a->getFrameData(inst->_a->getF());

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();
	bool mask = (inst->_flags & kInstMaskedPut) == kInstMaskedPut;

	_vm->_gfx->patchBackground(v18, x, y, mask);
}

DECLARE_INSTRUCTION_OPCODE(show) {
	_ctxt.suspend = true;
}

DECLARE_INSTRUCTION_OPCODE(invalid) {
	error("Can't execute invalid opcode %i", (*_ctxt.inst)->_index);
}

DECLARE_INSTRUCTION_OPCODE(call) {
	_vm->callFunction((*_ctxt.inst)->_immediate, 0);
}


DECLARE_INSTRUCTION_OPCODE(wait) {
	if (_engineFlags & kEngineWalking) {
		_ctxt.ip--;
		_ctxt.suspend = true;
	}
}


DECLARE_INSTRUCTION_OPCODE(start) {
	(*_ctxt.inst)->_a->_flags |= (kFlagsActing | kFlagsActive);
}


DECLARE_INSTRUCTION_OPCODE(sound) {
	_vm->_activeZone = (*_ctxt.inst)->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	InstructionPtr inst = (*_ctxt.inst);

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();

	_vm->_char.scheduleWalk(x, y);
}

DECLARE_INSTRUCTION_OPCODE(endscript) {
	if ((_ctxt.anim->_flags & kFlagsLooping) == 0) {
		_ctxt.anim->_flags &= ~kFlagsActing;
		_vm->_cmdExec->run(_ctxt.anim->_commands, _ctxt.anim);
		_ctxt.program->_status = kProgramDone;
	}

	_ctxt.ip = _ctxt.program->_instructions.begin();
	_ctxt.suspend = true;
}




DECLARE_COMMAND_OPCODE(invalid) {
	error("Can't execute invalid command '%i'", _ctxt.cmd->_id);
}

DECLARE_COMMAND_OPCODE(set) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_globalFlags |= _ctxt.cmd->u._flags;
	} else {
		_vm->setLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(clear) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_globalFlags &= ~_ctxt.cmd->u._flags;
	} else {
		_vm->clearLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(start) {
	_ctxt.cmd->u._zone->_flags |= kFlagsActing;
}


DECLARE_COMMAND_OPCODE(speak) {
	if ((_ctxt.cmd->u._zone->_type & 0xFFFF) == kZoneSpeak) {
		_vm->enterDialogueMode(_ctxt.cmd->u._zone);
	} else {
		_vm->_activeZone = _ctxt.cmd->u._zone;
	}
}


DECLARE_COMMAND_OPCODE(get) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsFixed;
	_vm->runZone(_ctxt.cmd->u._zone);
}


DECLARE_COMMAND_OPCODE(location) {
	_vm->scheduleLocationSwitch(_ctxt.cmd->u._string);
}


DECLARE_COMMAND_OPCODE(open) {
	_vm->updateDoor(_ctxt.cmd->u._zone, false);
}


DECLARE_COMMAND_OPCODE(close) {
	_vm->updateDoor(_ctxt.cmd->u._zone, true);
}

DECLARE_COMMAND_OPCODE(on) {
	_vm->showZone(_ctxt.cmd->u._zone, true);
}


DECLARE_COMMAND_OPCODE(off) {
	_vm->showZone(_ctxt.cmd->u._zone, false);
}


DECLARE_COMMAND_OPCODE(call) {
	_vm->callFunction(_ctxt.cmd->u._callable, &_ctxt.z);
}


DECLARE_COMMAND_OPCODE(toggle) {
	if (_ctxt.cmd->u._flags & kFlagsGlobal) {
		_ctxt.cmd->u._flags &= ~kFlagsGlobal;
		_globalFlags ^= _ctxt.cmd->u._flags;
	} else {
		_vm->toggleLocationFlags(_ctxt.cmd->u._flags);
	}
}


DECLARE_COMMAND_OPCODE(drop){
	_vm->dropItem( _ctxt.cmd->u._object );
}


DECLARE_COMMAND_OPCODE(quit) {
	_vm->_quit = true;
	_vm->quitGame();
}


DECLARE_COMMAND_OPCODE(move) {
	_vm->_char.scheduleWalk(_ctxt.cmd->u._move.x, _ctxt.cmd->u._move.y);
}


DECLARE_COMMAND_OPCODE(stop) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsActing;
}


void ProgramExec::runScript(ProgramPtr script, AnimationPtr a) {
	debugC(9, kDebugExec, "runScript(Animation = %s)", a->_name);

	_ctxt.ip = script->_ip;
	_ctxt.anim = a;
	_ctxt.program = script;
	_ctxt.suspend = false;
	_ctxt.modCounter = _modCounter;

	InstructionList::iterator inst;
	for ( ; (a->_flags & kFlagsActing) ; ) {

		inst = _ctxt.ip;
		_ctxt.inst = inst;
		_ctxt.ip++;

		debugC(9, kDebugExec, "inst [%02i] %s\n", (*inst)->_index, _instructionNames[(*inst)->_index - 1]);

		script->_status = kProgramRunning;

		(*_opcodes[(*inst)->_index])();

		if (_ctxt.suspend)
			break;

	}
	script->_ip = _ctxt.ip;

}

void ProgramExec::runScripts(ProgramList::iterator first, ProgramList::iterator last) {
	if (_engineFlags & kEnginePauseJobs) {
		return;
	}

	for (ProgramList::iterator it = first; it != last; it++) {

		AnimationPtr a = (*it)->_anim;

		if (a->_flags & kFlagsCharacter)
			a->setZ(a->getFrameY() + a->height());

		if ((a->_flags & kFlagsActing) == 0)
			continue;

		runScript(*it, a);

		if (a->_flags & kFlagsCharacter)
			a->setZ(a->getFrameY() + a->height());
	}

	_modCounter++;

	return;
}

void CommandExec::runList(CommandList::iterator first, CommandList::iterator last) {

	uint32 useFlags = 0;
	bool useLocalFlags;

	_ctxt.suspend = false;

	for ( ; first != last; first++) {
		if (_vm->shouldQuit())
			break;

		CommandPtr cmd = *first;

		if (cmd->_flagsOn & kFlagsGlobal) {
			useFlags = _globalFlags | kFlagsGlobal;
			useLocalFlags = false;
		} else {
			useFlags = _vm->getLocationFlags();
			useLocalFlags = true;
		}

		bool onMatch = (cmd->_flagsOn & useFlags) == cmd->_flagsOn;
		bool offMatch = (cmd->_flagsOff & ~useFlags) == cmd->_flagsOff;

		debugC(3, kDebugExec, "runCommands[%i] (on: %x, off: %x), (%s = %x)", cmd->_id,  cmd->_flagsOn, cmd->_flagsOff,
			useLocalFlags ? "LOCALFLAGS" : "GLOBALFLAGS", useFlags);

		if (!onMatch || !offMatch) continue;

		_ctxt.z = _execZone;
		_ctxt.cmd = cmd;

		(*_opcodes[cmd->_id])();

		if (_ctxt.suspend) {
			createSuspendList(++first, last);
			return;
		}
	}

}

void CommandExec::run(CommandList& list, ZonePtr z) {
	if (list.size() == 0) {
		debugC(3, kDebugExec, "runCommands: nothing to do");
		return;
	}

	_execZone = z;

	debugC(3, kDebugExec, "runCommands starting");
	runList(list.begin(), list.end());
	debugC(3, kDebugExec, "runCommands completed");
}

void CommandExec::createSuspendList(CommandList::iterator first, CommandList::iterator last) {
	if (first == last) {
		return;
	}

	debugC(3, kDebugExec, "CommandExec::createSuspendList()");

	_suspendedCtxt.valid = true;
	_suspendedCtxt.first = first;
	_suspendedCtxt.last = last;
	_suspendedCtxt.zone = _execZone;
}

void CommandExec::cleanSuspendedList() {
	debugC(3, kDebugExec, "CommandExec::cleanSuspended()");

	_suspendedCtxt.valid = false;
	_suspendedCtxt.first = _suspendedCtxt.last;
	_suspendedCtxt.zone = nullZonePtr;
}

void CommandExec::runSuspended() {
	if (_engineFlags & kEngineWalking) {
		return;
	}

	if (_suspendedCtxt.valid) {
		debugC(3, kDebugExec, "CommandExec::runSuspended()");

		_execZone = _suspendedCtxt.zone;
		runList(_suspendedCtxt.first, _suspendedCtxt.last);
		cleanSuspendedList();
	}
}

CommandExec_ns::CommandExec_ns(Parallaction_ns* vm) : _vm(vm) {

}

CommandExec_ns::~CommandExec_ns() {

}

void CommandExec_ns::init() {
	Common::Array<const Opcode*> *table = 0;

	SetOpcodeTable(_opcodes);
	COMMAND_OPCODE(invalid);
	COMMAND_OPCODE(set);
	COMMAND_OPCODE(clear);
	COMMAND_OPCODE(start);
	COMMAND_OPCODE(speak);
	COMMAND_OPCODE(get);
	COMMAND_OPCODE(location);
	COMMAND_OPCODE(open);
	COMMAND_OPCODE(close);
	COMMAND_OPCODE(on);
	COMMAND_OPCODE(off);
	COMMAND_OPCODE(call);
	COMMAND_OPCODE(toggle);
	COMMAND_OPCODE(drop);
	COMMAND_OPCODE(quit);
	COMMAND_OPCODE(move);
	COMMAND_OPCODE(stop);
}

void ProgramExec_ns::init() {

	Common::Array<const Opcode*> *table = 0;

	SetOpcodeTable(_opcodes);
	INSTRUCTION_OPCODE(invalid);
	INSTRUCTION_OPCODE(on);
	INSTRUCTION_OPCODE(off);
	INSTRUCTION_OPCODE(set);		// x
	INSTRUCTION_OPCODE(set);		// y
	INSTRUCTION_OPCODE(set);		// z
	INSTRUCTION_OPCODE(set);		// f
	INSTRUCTION_OPCODE(loop);
	INSTRUCTION_OPCODE(endloop);
	INSTRUCTION_OPCODE(show);
	INSTRUCTION_OPCODE(inc);
	INSTRUCTION_OPCODE(inc);		// dec
	INSTRUCTION_OPCODE(set);
	INSTRUCTION_OPCODE(put);
	INSTRUCTION_OPCODE(call);
	INSTRUCTION_OPCODE(wait);
	INSTRUCTION_OPCODE(start);
	INSTRUCTION_OPCODE(sound);
	INSTRUCTION_OPCODE(move);
	INSTRUCTION_OPCODE(endscript);

}

ProgramExec_ns::ProgramExec_ns(Parallaction_ns *vm) : _vm(vm) {
	_instructionNames = _instructionNamesRes_ns;
}

ProgramExec_ns::~ProgramExec_ns() {
}

}	// namespace Parallaction
