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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/advsys/vm.h"

namespace Glk {
namespace AdvSys {

OpcodeMethod VM::_METHODS[0x34] = {
	&VM::opBRT,
	&VM::opBRF,
	&VM::opBR,
	&VM::opT,
	&VM::opNIL,
	&VM::opPUSH,
	&VM::opNOT,
	&VM::opADD,
	&VM::opSUB,
	&VM::opMUL,
	&VM::opDIV,
	&VM::opREM,
	&VM::opBAND,
	&VM::opBOR,
	&VM::opBNOT,
	&VM::opLT,
	&VM::opEQ,
	&VM::opGT,
	&VM::opLIT,
	&VM::opVAR,
	&VM::opGETP,
	&VM::opSETP,
	&VM::opSET,
	&VM::opPRINT,
	&VM::opTERPRI,
	&VM::opPNUMBER,
	&VM::opFINISH,
	&VM::opCHAIN,
	&VM::opABORT,
	&VM::opEXIT,
	&VM::opRETURN,
	&VM::opCALL,
	&VM::opSVAR,
	&VM::opSSET,
	&VM::opSPLIT,
	&VM::opSNLIT,
	&VM::opYORN,
	&VM::opSAVE,
	&VM::opRESTORE,
	&VM::opARG,
	&VM::opASET,
	&VM::opTMP,
	&VM::opTSET,
	&VM::opTSPACE,
	&VM::opCLASS,
	&VM::opMATCH,
	&VM::opPNOUN,
	&VM::opRESTART,
	&VM::opRAND,
	&VM::opRNDMIZE,
	&VM::opSEND,
	&VM::opVOWEL
};

VM::VM(OSystem *syst, const GlkGameDescription &gameDesc) : GlkInterface(syst, gameDesc), Game(),
		_pc(0), _status(IN_PROGRESS) {
}

ExecutionResult VM::execute(int offset) {
	// Set the code pointer
	_pc = offset;

	// Clear the stack
	_stack.clear();

	// Iterate through the script
	for (_status = IN_PROGRESS; !shouldQuit() && _status == IN_PROGRESS; )
		executeOpcode();

	return _status;
}

void VM::executeOpcode() {
	// Get next opcode
	uint opcode = getCodeByte(_pc);
	++_pc;

	if (opcode >= OP_BRT && opcode <= OP_VOWEL) {
		(this->*_METHODS[(int)opcode - 1])();
	} else if (opcode >= OP_XVAR && opcode < OP_XSET) {
		_stack.back() = getVariable((int)opcode - OP_XVAR);
	} else if (opcode >= OP_XSET && opcode < OP_XPLIT) {
		setVariable((int)opcode - OP_XSET, _stack.back());
	} else if (opcode >= OP_XPLIT && opcode < OP_XNLIT) {
		_stack.back() = (int)opcode - OP_XPLIT;
	} else if (opcode >= OP_XNLIT && (int)opcode < 256) {
		_stack.back() = OP_XNLIT - opcode;
	} else {
		error("Unknown opcode %x at offset %d", opcode, _pc);
	}
}

void VM::opBRT() {
}

void VM::opBRF() {
}

void VM::opBR() {
}

void VM::opT() {
}

void VM::opNIL() {
}

void VM::opPUSH() {
}

void VM::opNOT() {
}

void VM::opADD() {
}

void VM::opSUB() {
}

void VM::opMUL() {
}

void VM::opDIV() {
}

void VM::opREM() {
}

void VM::opBAND() {
}

void VM::opBOR() {
}

void VM::opBNOT() {
}

void VM::opLT() {
}

void VM::opEQ() {
}

void VM::opGT() {
}

void VM::opLIT() {
}

void VM::opVAR() {
}

void VM::opGETP() {
}

void VM::opSETP() {
}

void VM::opSET() {
}

void VM::opPRINT() {
}

void VM::opTERPRI() {
}

void VM::opPNUMBER() {
}

void VM::opFINISH() {
}

void VM::opCHAIN() {
}

void VM::opABORT() {
}

void VM::opEXIT() {
}

void VM::opRETURN() {
}

void VM::opCALL() {
}

void VM::opSVAR() {
}

void VM::opSSET() {
}

void VM::opSPLIT() {
}

void VM::opSNLIT() {
}

void VM::opYORN() {
}

void VM::opSAVE() {
}

void VM::opRESTORE() {
}

void VM::opARG() {
}

void VM::opASET() {
}

void VM::opTMP() {
}

void VM::opTSET() {
}

void VM::opTSPACE() {
}

void VM::opCLASS() {
}

void VM::opMATCH() {
}

void VM::opPNOUN() {
}

void VM::opRESTART() {
}

void VM::opRAND() {
}

void VM::opRNDMIZE() {
}

void VM::opSEND() {
}

void VM::opVOWEL() {
}

} // End of namespace AdvSys
} // End of namespace Glk
