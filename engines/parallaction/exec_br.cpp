
#include "common/stdafx.h"
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
#define INST_IF			26
#define INST_IFEQ		27
#define INST_IFLT		28
#define INST_IFGT		29
#define INST_ENDIF		30
#define INST_STOP		31


typedef OpcodeImpl<Parallaction_br> OpcodeV2;
#define COMMAND_OPCODE(op) OpcodeV2(this, &Parallaction_br::cmdOp_##op)
#define DECLARE_COMMAND_OPCODE(op) void Parallaction_br::cmdOp_##op()

#define INSTRUCTION_OPCODE(op) OpcodeV2(this, &Parallaction_br::instOp_##op)
#define DECLARE_INSTRUCTION_OPCODE(op) void Parallaction_br::instOp_##op()


DECLARE_COMMAND_OPCODE(location) {
	warning("Parallaction_br::cmdOp_location command not yet implemented");
}


DECLARE_COMMAND_OPCODE(open) {
	warning("Parallaction_br::cmdOp_open command not yet implemented");
}


DECLARE_COMMAND_OPCODE(close) {
	warning("Parallaction_br::cmdOp_close not yet implemented");
}


DECLARE_COMMAND_OPCODE(on) {
	warning("Parallaction_br::cmdOp_on not yet implemented");
}


DECLARE_COMMAND_OPCODE(off) {
	warning("Parallaction_br::cmdOp_off not yet implemented");
}


DECLARE_COMMAND_OPCODE(call) {
	callFunction(_cmdRunCtxt.cmd->u._callable, _cmdRunCtxt.z);
}


DECLARE_COMMAND_OPCODE(drop) {
	warning("Parallaction_br::cmdOp_drop not yet implemented");
}


DECLARE_COMMAND_OPCODE(move) {
	warning("Parallaction_br::cmdOp_move not yet implemented");
}

DECLARE_COMMAND_OPCODE(start) {
	warning("Parallaction_br::cmdOp_start not yet implemented");
}

DECLARE_COMMAND_OPCODE(stop) {
	warning("Parallaction_br::cmdOp_stop not yet implemented");
}


DECLARE_COMMAND_OPCODE(character) {
	warning("Parallaction_br::cmdOp_character not yet implemented");
}


DECLARE_COMMAND_OPCODE(followme) {
	warning("Parallaction_br::cmdOp_followme not yet implemented");
}


DECLARE_COMMAND_OPCODE(onmouse) {
	showCursor(true);
}


DECLARE_COMMAND_OPCODE(offmouse) {
	showCursor(false);
}


DECLARE_COMMAND_OPCODE(add) {
	warning("Parallaction_br::cmdOp_add not yet implemented");
}


DECLARE_COMMAND_OPCODE(leave) {
	warning("Parallaction_br::cmdOp_leave not yet implemented");
}


DECLARE_COMMAND_OPCODE(inc) {
	_counters[_cmdRunCtxt.cmd->u._lvalue] += _cmdRunCtxt.cmd->u._rvalue;
}


DECLARE_COMMAND_OPCODE(dec) {
	_counters[_cmdRunCtxt.cmd->u._lvalue] -= _cmdRunCtxt.cmd->u._rvalue;
}


DECLARE_COMMAND_OPCODE(ifeq) {
	if (_counters[_cmdRunCtxt.cmd->u._lvalue] == _cmdRunCtxt.cmd->u._rvalue) {
		_localFlags[_currentLocationIndex] |= kFlagsTestTrue;
	} else {
		_localFlags[_currentLocationIndex] &= ~kFlagsTestTrue;
	}
}

DECLARE_COMMAND_OPCODE(iflt) {
	if (_counters[_cmdRunCtxt.cmd->u._lvalue] < _cmdRunCtxt.cmd->u._rvalue) {
		_localFlags[_currentLocationIndex] |= kFlagsTestTrue;
	} else {
		_localFlags[_currentLocationIndex] &= ~kFlagsTestTrue;
	}
}

DECLARE_COMMAND_OPCODE(ifgt) {
	if (_counters[_cmdRunCtxt.cmd->u._lvalue] > _cmdRunCtxt.cmd->u._rvalue) {
		_localFlags[_currentLocationIndex] |= kFlagsTestTrue;
	} else {
		_localFlags[_currentLocationIndex] &= ~kFlagsTestTrue;
	}
}


DECLARE_COMMAND_OPCODE(let) {
	_counters[_cmdRunCtxt.cmd->u._lvalue] = _cmdRunCtxt.cmd->u._rvalue;
}


DECLARE_COMMAND_OPCODE(music) {
	warning("Parallaction_br::cmdOp_music not yet implemented");
}


DECLARE_COMMAND_OPCODE(fix) {
	_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsFixed;
}


DECLARE_COMMAND_OPCODE(unfix) {
	_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsFixed;
}


DECLARE_COMMAND_OPCODE(zeta) {
	_zeta0 = _cmdRunCtxt.cmd->u._zeta0;
	_zeta1 = _cmdRunCtxt.cmd->u._zeta1;
	_zeta2 = _cmdRunCtxt.cmd->u._zeta2;
}


DECLARE_COMMAND_OPCODE(scroll) {
	warning("Parallaction_br::cmdOp_scroll not yet implemented");
}


DECLARE_COMMAND_OPCODE(swap) {
	warning("Parallaction_br::cmdOp_swap not yet implemented");
}


DECLARE_COMMAND_OPCODE(give) {
	warning("Parallaction_br::cmdOp_give not yet implemented");
}


DECLARE_COMMAND_OPCODE(text) {
	warning("Parallaction_br::cmdOp_text not yet implemented");
}


DECLARE_COMMAND_OPCODE(part) {
	warning("Parallaction_br::cmdOp_part not yet implemented");
}


DECLARE_COMMAND_OPCODE(testsfx) {
	warning("Parallaction_br::cmdOp_testsfx not completely implemented");
	_localFlags[_currentLocationIndex] &= ~kFlagsTestTrue;	// should test if sfx are enabled
}


DECLARE_COMMAND_OPCODE(ret) {
	warning("Parallaction_br::cmdOp_ret not yet implemented");
}


DECLARE_COMMAND_OPCODE(onsave) {
	warning("Parallaction_br::cmdOp_onsave not yet implemented");
}


DECLARE_COMMAND_OPCODE(offsave) {
	warning("Parallaction_br::cmdOp_offsave not yet implemented");
}




DECLARE_INSTRUCTION_OPCODE(on) {
	warning("Parallaction_br::instOp_on not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(off) {
	warning("Parallaction_br::instOp_off not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(set) {
	Instruction *inst = *_instRunCtxt.inst;

	int16 rvalue = inst->_opB.getRValue();
	int16* lvalue = inst->_opA.getLValue();

	*lvalue = rvalue;

}


DECLARE_INSTRUCTION_OPCODE(loop) {
	Instruction *inst = *_instRunCtxt.inst;

	_instRunCtxt.a->_program->_loopCounter = inst->_opB.getRValue();
	_instRunCtxt.a->_program->_loopStart = _instRunCtxt.inst;
}


DECLARE_INSTRUCTION_OPCODE(inc) {
	Instruction *inst = *_instRunCtxt.inst;

	int16 rvalue = inst->_opB.getRValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (rvalue > 0 ? rvalue : -rvalue);
		if (_instRunCtxt.modCounter % _bx != 0) return;

		rvalue = (rvalue > 0 ?  1 : -1);
	}

	int16 *lvalue = inst->_opA.getLValue();

	switch (inst->_index) {
	case INST_INC:
		*lvalue += rvalue;
		break;

	case INST_DEC:
		*lvalue -= rvalue;
		break;

	case INST_MUL:
		*lvalue *= rvalue;
		break;

	case INST_DIV:
		*lvalue /= rvalue;
		break;

	default:
		error("This should never happen. Report immediately");;
	}

	if (inst->_opA._flags & kParaLocal) {
		wrapLocalVar(inst->_opA._local);
	}

}


DECLARE_INSTRUCTION_OPCODE(put) {
	warning("Parallaction_br::instOp_put not yet implemented");
}



DECLARE_INSTRUCTION_OPCODE(wait) {
	warning("Parallaction_br::instOp_wait not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(start) {
	(*_instRunCtxt.inst)->_z->_flags |= kFlagsActing;
}


DECLARE_INSTRUCTION_OPCODE(process) {
	_activeZone2 = (*_instRunCtxt.inst)->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	warning("Parallaction_br::instOp_move not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(color) {
	Instruction *inst = *_instRunCtxt.inst;

	int16 entry = inst->_opB.getRValue();

	_gfx->_palette.setEntry(entry, inst->_colors[0], inst->_colors[1], inst->_colors[2]);

}


DECLARE_INSTRUCTION_OPCODE(mask) {
	Instruction *inst = *_instRunCtxt.inst;

	_gfx->_bgLayers[0] = inst->_opA.getRValue();
	_gfx->_bgLayers[1] = inst->_opB.getRValue();
	_gfx->_bgLayers[2] = inst->_opC.getRValue();
}


DECLARE_INSTRUCTION_OPCODE(print) {
	warning("Parallaction_br::instOp_print not yet implemented");
}


DECLARE_INSTRUCTION_OPCODE(text) {
	warning("Parallaction_br::instOp_text not yet implemented");
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

DECLARE_INSTRUCTION_OPCODE(endscript) {
	warning("Parallaction_br::instOp_endscript not yet implemented");
}

void Parallaction_br::initOpcodes() {

	static const OpcodeV2 op1[] = {
		COMMAND_OPCODE(invalid),
		COMMAND_OPCODE(set),
		COMMAND_OPCODE(clear),
		COMMAND_OPCODE(start),
		COMMAND_OPCODE(speak),
		COMMAND_OPCODE(get),
		COMMAND_OPCODE(location),
		COMMAND_OPCODE(open),
		COMMAND_OPCODE(close),
		COMMAND_OPCODE(on),
		COMMAND_OPCODE(off),
		COMMAND_OPCODE(call),
		COMMAND_OPCODE(toggle),
		COMMAND_OPCODE(drop),
		COMMAND_OPCODE(quit),
		COMMAND_OPCODE(move),
		COMMAND_OPCODE(stop),
		COMMAND_OPCODE(character),
		COMMAND_OPCODE(followme),
		COMMAND_OPCODE(onmouse),
		COMMAND_OPCODE(offmouse),
		COMMAND_OPCODE(add),
		COMMAND_OPCODE(leave),
		COMMAND_OPCODE(inc),
		COMMAND_OPCODE(dec),
		COMMAND_OPCODE(ifeq),
		COMMAND_OPCODE(iflt),
		COMMAND_OPCODE(ifgt),
		COMMAND_OPCODE(let),
		COMMAND_OPCODE(music),
		COMMAND_OPCODE(fix),
		COMMAND_OPCODE(unfix),
		COMMAND_OPCODE(zeta),
		COMMAND_OPCODE(scroll),
		COMMAND_OPCODE(swap),
		COMMAND_OPCODE(give),
		COMMAND_OPCODE(text),
		COMMAND_OPCODE(part),
		COMMAND_OPCODE(testsfx),
		COMMAND_OPCODE(ret),
		COMMAND_OPCODE(onsave),
		COMMAND_OPCODE(offsave)
	};

	uint i;
	for (i = 0; i < ARRAYSIZE(op1); i++)
		_commandOpcodes.push_back(&op1[i]);


	static const OpcodeV2 op2[] = {
		INSTRUCTION_OPCODE(invalid),
		INSTRUCTION_OPCODE(on),
		INSTRUCTION_OPCODE(off),
		INSTRUCTION_OPCODE(set),		// x
		INSTRUCTION_OPCODE(set),		// y
		INSTRUCTION_OPCODE(set),		// z
		INSTRUCTION_OPCODE(set),		// f
		INSTRUCTION_OPCODE(loop),
		INSTRUCTION_OPCODE(endloop),
		INSTRUCTION_OPCODE(null),		// show
		INSTRUCTION_OPCODE(inc),
		INSTRUCTION_OPCODE(inc),		// dec
		INSTRUCTION_OPCODE(set),
		INSTRUCTION_OPCODE(put),
		INSTRUCTION_OPCODE(call),
		INSTRUCTION_OPCODE(wait),
		INSTRUCTION_OPCODE(start),
		INSTRUCTION_OPCODE(process),
		INSTRUCTION_OPCODE(move),
		INSTRUCTION_OPCODE(color),
		INSTRUCTION_OPCODE(process),	// sound
		INSTRUCTION_OPCODE(mask),
		INSTRUCTION_OPCODE(print),
		INSTRUCTION_OPCODE(text),
		INSTRUCTION_OPCODE(inc),		// mul
		INSTRUCTION_OPCODE(inc),		// div
		INSTRUCTION_OPCODE(ifeq),
		INSTRUCTION_OPCODE(iflt),
		INSTRUCTION_OPCODE(ifgt),
		INSTRUCTION_OPCODE(endif),
		INSTRUCTION_OPCODE(stop),
		INSTRUCTION_OPCODE(endscript)
	};

	for (i = 0; i < ARRAYSIZE(op2); i++)
		_instructionOpcodes.push_back(&op2[i]);


}


} // namespace Parallaction
