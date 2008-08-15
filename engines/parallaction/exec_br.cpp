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

namespace Parallaction {

#define INST_ON			1
#define INST_OFF		2
#define INST_X			3
#define INST_Y			4
#define INST_Z			5
#define INST_F			6
#define INST_LOOP		7
#define INST_ENDLOOP	8
#define INST_SHOW		9
#define INST_INC		10
#define INST_DEC		11
#define INST_SET		12
#define INST_PUT		13
#define INST_CALL		14
#define INST_WAIT		15
#define INST_START		16
#define INST_PROCESS	17
#define INST_MOVE		18
#define INST_COLOR		19
#define INST_SOUND		20
#define INST_MASK		21
#define INST_PRINT		22
#define INST_TEXT		23
#define INST_MUL		24
#define INST_DIV		25
#define INST_IFEQ		26
#define INST_IFLT		27
#define INST_IFGT		28
#define INST_ENDIF		29
#define INST_STOP		30
#define INST_ENDSCRIPT	31

#define SetOpcodeTable(x) table = &x;

typedef Common::Functor0Mem<void, CommandExec_br> OpcodeV1;
#define COMMAND_OPCODE(op) table->push_back(new OpcodeV1(this, &CommandExec_br::cmdOp_##op))
#define DECLARE_COMMAND_OPCODE(op) void CommandExec_br::cmdOp_##op()

typedef Common::Functor0Mem<void, ProgramExec_br> OpcodeV2;
#define INSTRUCTION_OPCODE(op) table->push_back(new OpcodeV2(this, &ProgramExec_br::instOp_##op))
#define DECLARE_INSTRUCTION_OPCODE(op) void ProgramExec_br::instOp_##op()

extern const char *_instructionNamesRes_br[];

void Parallaction_br::setupSubtitles(char *s, char *s2, int y) {
	debugC(5, kDebugExec, "setupSubtitles(%s, %s, %i)", s, s2, y);

	clearSubtitles();

	if (!scumm_stricmp("clear", s)) {
		return;
	}

	if (y != -1) {
		_subtitleY = y;
	}

	// FIXME: render subtitles using the right color (10 instead of 0).
	// The original game features a nasty hack, having the font rendering routine
	// replacing color 12 of font RUSSIA with 10 when preparing subtitles.
	_subtitle[0] = _gfx->createLabel(_labelFont, s, 0);
	_gfx->showLabel(_subtitle[0], CENTER_LABEL_HORIZONTAL, _subtitleY);
	if (s2) {
		_subtitle[1] = _gfx->createLabel(_labelFont, s2, 0);
		_gfx->showLabel(_subtitle[1], CENTER_LABEL_HORIZONTAL, _subtitleY + 5 + _labelFont->height());
	} else {
		_subtitle[1] = -1;
	}
#if 0	// disabled because no references to lip sync has been found in the scripts
	_subtitleLipSync = 0;
#endif
}

void Parallaction_br::clearSubtitles() {
	if (_subtitle[0] != -1) {
		_gfx->hideLabel(_subtitle[0]);
	}

	if (_subtitle[1] != -1) {
		_gfx->hideLabel(_subtitle[1]);
	}
}


DECLARE_COMMAND_OPCODE(location) {
	warning("Parallaction_br::cmdOp_location command not yet implemented");

	// TODO: handle startPos and startPos2
	_vm->scheduleLocationSwitch(_ctxt.cmd->u._string);
}


DECLARE_COMMAND_OPCODE(open) {
	warning("Parallaction_br::cmdOp_open command not yet implemented");
	_vm->updateDoor(_ctxt.cmd->u._zone, false);
}


DECLARE_COMMAND_OPCODE(close) {
	warning("Parallaction_br::cmdOp_close not yet implemented");
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


DECLARE_COMMAND_OPCODE(drop) {
	_vm->dropItem(_ctxt.cmd->u._object);
}


DECLARE_COMMAND_OPCODE(move) {
	_vm->_char.scheduleWalk(_ctxt.cmd->u._move.x, _ctxt.cmd->u._move.y);
	_ctxt.suspend = true;
}

DECLARE_COMMAND_OPCODE(start) {
	_ctxt.cmd->u._zone->_flags |= kFlagsActing;
}

DECLARE_COMMAND_OPCODE(stop) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsActing;
}


DECLARE_COMMAND_OPCODE(character) {
	debugC(9, kDebugExec, "Parallaction_br::cmdOp_character(%s)", _ctxt.cmd->u._string);
	_vm->changeCharacter(_ctxt.cmd->u._string);
}


DECLARE_COMMAND_OPCODE(followme) {
	warning("Parallaction_br::cmdOp_followme not yet implemented");
}


DECLARE_COMMAND_OPCODE(onmouse) {
	_vm->_input->setMouseState(MOUSE_ENABLED_SHOW);
}


DECLARE_COMMAND_OPCODE(offmouse) {
	_vm->_input->setMouseState(MOUSE_DISABLED);
}


DECLARE_COMMAND_OPCODE(add) {
	_vm->addInventoryItem(_ctxt.cmd->u._object);
}


DECLARE_COMMAND_OPCODE(leave) {
	warning("Parallaction_br::cmdOp_leave not yet implemented");
}


DECLARE_COMMAND_OPCODE(inc) {
	_vm->_counters[_ctxt.cmd->u._lvalue] += _ctxt.cmd->u._rvalue;
}


DECLARE_COMMAND_OPCODE(dec) {
	_vm->_counters[_ctxt.cmd->u._lvalue] -= _ctxt.cmd->u._rvalue;
}


DECLARE_COMMAND_OPCODE(ifeq) {
	if (_vm->_counters[_ctxt.cmd->u._lvalue] == _ctxt.cmd->u._rvalue) {
		_vm->setLocationFlags(kFlagsTestTrue);
	} else {
		_vm->clearLocationFlags(kFlagsTestTrue);
	}
}

DECLARE_COMMAND_OPCODE(iflt) {
	if (_vm->_counters[_ctxt.cmd->u._lvalue] < _ctxt.cmd->u._rvalue) {
		_vm->setLocationFlags(kFlagsTestTrue);
	} else {
		_vm->clearLocationFlags(kFlagsTestTrue);
	}
}

DECLARE_COMMAND_OPCODE(ifgt) {
	if (_vm->_counters[_ctxt.cmd->u._lvalue] > _ctxt.cmd->u._rvalue) {
		_vm->setLocationFlags(kFlagsTestTrue);
	} else {
		_vm->clearLocationFlags(kFlagsTestTrue);
	}
}


DECLARE_COMMAND_OPCODE(let) {
	_vm->_counters[_ctxt.cmd->u._lvalue] = _ctxt.cmd->u._rvalue;
}


DECLARE_COMMAND_OPCODE(music) {
	warning("Parallaction_br::cmdOp_music not yet implemented");
}


DECLARE_COMMAND_OPCODE(fix) {
	_ctxt.cmd->u._zone->_flags |= kFlagsFixed;
}


DECLARE_COMMAND_OPCODE(unfix) {
	_ctxt.cmd->u._zone->_flags &= ~kFlagsFixed;
}


DECLARE_COMMAND_OPCODE(zeta) {
	_vm->_location._zeta0 = _ctxt.cmd->u._zeta0;
	_vm->_location._zeta1 = _ctxt.cmd->u._zeta1;
	_vm->_location._zeta2 = _ctxt.cmd->u._zeta2;
}


DECLARE_COMMAND_OPCODE(scroll) {
	warning("Parallaction_br::cmdOp_scroll not yet implemented");
	_vm->_gfx->setVar("scroll_x", _ctxt.cmd->u._rvalue );
}


DECLARE_COMMAND_OPCODE(swap) {
	warning("Parallaction_br::cmdOp_swap not yet implemented");
}


DECLARE_COMMAND_OPCODE(give) {
	warning("Parallaction_br::cmdOp_give not yet implemented");
}


DECLARE_COMMAND_OPCODE(text) {
	CommandData *data = &_ctxt.cmd->u;
	_vm->setupSubtitles(data->_string, data->_string2, data->_zeta0);
}


DECLARE_COMMAND_OPCODE(part) {
	warning("Parallaction_br::cmdOp_part not yet implemented");
}


DECLARE_COMMAND_OPCODE(testsfx) {
	warning("Parallaction_br::cmdOp_testsfx not completely implemented");
	_vm->clearLocationFlags(kFlagsTestTrue);	// should test if sfx are enabled
}


DECLARE_COMMAND_OPCODE(ret) {
	_engineFlags |= kEngineReturn;
}


DECLARE_COMMAND_OPCODE(onsave) {
	warning("Parallaction_br::cmdOp_onsave not yet implemented");
}


DECLARE_COMMAND_OPCODE(offsave) {
	warning("Parallaction_br::cmdOp_offsave not yet implemented");
}




DECLARE_INSTRUCTION_OPCODE(on) {
	_vm->showZone((*_ctxt.inst)->_z, true);
}


DECLARE_INSTRUCTION_OPCODE(off) {
	_vm->showZone((*_ctxt.inst)->_z, false);
}


DECLARE_INSTRUCTION_OPCODE(set) {
	InstructionPtr inst = *_ctxt.inst;
	inst->_opA.setValue(inst->_opB.getValue());
}



DECLARE_INSTRUCTION_OPCODE(inc) {
	InstructionPtr inst = *_ctxt.inst;

	int16 rvalue = inst->_opB.getValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (rvalue > 0 ? rvalue : -rvalue);
		if (_ctxt.modCounter % _bx != 0) return;

		rvalue = (rvalue > 0 ?  1 : -1);
	}

	int16 lvalue = inst->_opA.getValue();

	switch (inst->_index) {
	case INST_INC:
		lvalue += rvalue;
		break;

	case INST_DEC:
		lvalue -= rvalue;
		break;

	case INST_MUL:
		lvalue *= rvalue;
		break;

	case INST_DIV:
		lvalue /= rvalue;
		break;

	default:
		error("This should never happen. Report immediately");
	}

	inst->_opA.setValue(lvalue);

}


DECLARE_INSTRUCTION_OPCODE(put) {
	warning("Parallaction_br::instOp_put not yet implemented");
}



DECLARE_INSTRUCTION_OPCODE(wait) {
	warning("Parallaction_br::instOp_wait not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(start) {
	(*_ctxt.inst)->_z->_flags |= kFlagsActing;
}


DECLARE_INSTRUCTION_OPCODE(process) {
	_vm->_activeZone2 = (*_ctxt.inst)->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	warning("Parallaction_br::instOp_move not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(color) {
	InstructionPtr inst = *_ctxt.inst;
	_vm->_gfx->_palette.setEntry(inst->_opB.getValue(), inst->_colors[0], inst->_colors[1], inst->_colors[2]);
}


DECLARE_INSTRUCTION_OPCODE(mask) {
#if 0
	Instruction *inst = *_ctxt.inst;
	_gfx->_bgLayers[0] = inst->_opA.getRValue();
	_gfx->_bgLayers[1] = inst->_opB.getRValue();
	_gfx->_bgLayers[2] = inst->_opC.getRValue();
#endif
}


DECLARE_INSTRUCTION_OPCODE(print) {
	warning("Parallaction_br::instOp_print not yet implemented");
}

DECLARE_INSTRUCTION_OPCODE(text) {
	InstructionPtr inst = (*_ctxt.inst);
	_vm->setupSubtitles(inst->_text, inst->_text2, inst->_y);
}


DECLARE_INSTRUCTION_OPCODE(ifeq) {
	warning("Parallaction_br::instOp_ifeq not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(iflt) {
	warning("Parallaction_br::instOp_iflt not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(ifgt) {
	warning("Parallaction_br::instOp_ifgt not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(endif) {
	warning("Parallaction_br::instOp_endif not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(stop) {
	warning("Parallaction_br::instOp_stop not yet implemented");
}


void CommandExec_br::init() {
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
	COMMAND_OPCODE(character);
	COMMAND_OPCODE(followme);
	COMMAND_OPCODE(onmouse);
	COMMAND_OPCODE(offmouse);
	COMMAND_OPCODE(add);
	COMMAND_OPCODE(leave);
	COMMAND_OPCODE(inc);
	COMMAND_OPCODE(dec);
	COMMAND_OPCODE(ifeq);
	COMMAND_OPCODE(iflt);
	COMMAND_OPCODE(ifgt);
	COMMAND_OPCODE(let);
	COMMAND_OPCODE(music);
	COMMAND_OPCODE(fix);
	COMMAND_OPCODE(unfix);
	COMMAND_OPCODE(zeta);
	COMMAND_OPCODE(scroll);
	COMMAND_OPCODE(swap);
	COMMAND_OPCODE(give);
	COMMAND_OPCODE(text);
	COMMAND_OPCODE(part);
	COMMAND_OPCODE(testsfx);
	COMMAND_OPCODE(ret);
	COMMAND_OPCODE(onsave);
	COMMAND_OPCODE(offsave);
}

CommandExec_br::CommandExec_br(Parallaction_br* vm) : CommandExec_ns(vm), _vm(vm) {

}

CommandExec_br::~CommandExec_br() {

}

void ProgramExec_br::init() {

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
	INSTRUCTION_OPCODE(show);		// show
	INSTRUCTION_OPCODE(inc);
	INSTRUCTION_OPCODE(inc);		// dec
	INSTRUCTION_OPCODE(set);
	INSTRUCTION_OPCODE(put);
	INSTRUCTION_OPCODE(call);
	INSTRUCTION_OPCODE(wait);
	INSTRUCTION_OPCODE(start);
	INSTRUCTION_OPCODE(process);
	INSTRUCTION_OPCODE(move);
	INSTRUCTION_OPCODE(color);
	INSTRUCTION_OPCODE(process);	// sound
	INSTRUCTION_OPCODE(mask);
	INSTRUCTION_OPCODE(print);
	INSTRUCTION_OPCODE(text);
	INSTRUCTION_OPCODE(inc);		// mul
	INSTRUCTION_OPCODE(inc);		// div
	INSTRUCTION_OPCODE(ifeq);
	INSTRUCTION_OPCODE(iflt);
	INSTRUCTION_OPCODE(ifgt);
	INSTRUCTION_OPCODE(endif);
	INSTRUCTION_OPCODE(stop);
	INSTRUCTION_OPCODE(endscript);
}

ProgramExec_br::ProgramExec_br(Parallaction_br *vm) : ProgramExec_ns(vm), _vm(vm) {
	_instructionNames = _instructionNamesRes_br;
}

ProgramExec_br::~ProgramExec_br() {
}

#if 0
void Parallaction_br::jobWaitRemoveLabelJob(void *parm, Job *job) {

}

void Parallaction_br::jobPauseSfx(void *parm, Job *job) {

}


void Parallaction_br::jobStopFollower(void *parm, Job *job) {

}


void Parallaction_br::jobScroll(void *parm, Job *job) {

}
#endif




} // namespace Parallaction
