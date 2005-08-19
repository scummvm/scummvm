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

#include "common/stdafx.h"
#include "kyra/kyra.h"
#include "kyra/script.h"

namespace Kyra {
// Command procs

void VMContext::c1_unknownCommand(void) {
	debug("unknown command '0x%x'.", _currentCommand);
	debug("\targument: '0x%x'", _argument);

	_error = true;
}

void VMContext::c1_goToLine(void) {
	_instructionPos = _argument << 1;
}

void VMContext::c1_setReturn(void) {
	_returnValue = _argument;
}

void VMContext::c1_pushRetRec(void) {
	if (!_argument) {
		pushStack(_returnValue);
	} else {
		int32 rec = ((int16)_tempPos << 16) | ((_instructionPos >> 1) + 1);
		pushStack(rec);
		_tempPos = _instructionPos;
	}
}

void VMContext::c1_push(void) {
	pushStack(_argument);
}

void VMContext::c1_pushVar(void) {
	pushStack(_registers[_argument]);
}

void VMContext::c1_pushFrameNeg(void) {
	pushStack(_stack[_tempPos + _argument]);
}

void VMContext::c1_pushFramePos(void) {
	pushStack(_stack[_tempPos - _argument]);
}

void VMContext::c1_popRetRec(void) {
	if (!_argument) {
		_returnValue = popStack();
	} else {
		if (_stackPos <= 0) {
			_scriptState = kScriptStopped;
		}
		int32 rec = popStack();

		_tempPos = (int16)((rec & 0xFFFF0000) >> 16);
		_instructionPos = (rec & 0x0000FFFF) * 2;
	}
}

void VMContext::c1_popVar(void) {
	_registers[_argument] = popStack();
}

void VMContext::c1_popFrameNeg(void) {
	_stack[_tempPos + _argument] = popStack();
}

void VMContext::c1_popFramePos(void) {
	_stack[_tempPos - _argument] = popStack();
}

void VMContext::c1_addToSP(void) {
	_stackPos -= _argument;
}

void VMContext::c1_subFromSP(void) {
	_stackPos += _argument;
}

void VMContext::c1_execOpcode(void) {
	if (_argument < _numOpcodes) {
		OpcodeProc proc = _opcodes[_argument].proc;
		(this->*proc)();
	} else {
		error("Invalid opcode 0x%X", _argument);
	}
}

void VMContext::c1_ifNotGoTo(void) {
	if (!popStack()) {
		_instructionPos = _argument << 1;
	}
}

void VMContext::c1_negate(void) {
	switch(_argument) {
	case 0:
		topStack() = !topStack();
		break;

	case 1:
		topStack() = -topStack();
		break;

	case 2:
		topStack() = ~topStack();
		break;

	default:
		debug("unkown negate instruction %d", _argument);
		_error = true;
		break;
	};
}

void VMContext::c1_evaluate(void) {
	int32 x, y;
	int32 res = false;

	x = popStack();
	y = popStack();

	switch(_argument) {
	case 0:
		res = x && y;
		break;

	case 1:
		res = x || y;
		break;

	case 3:
		res = x != y;
		break;

	case 4:
		res = x < y;
		break;

	case 5:
		res = x <= y;
		break;

	case 6:
		res = x > y;
		break;

	case 7:
		res = x >= y;
		break;

	case 8:
		res = x + y;
		break;

	case 9:
		res = x - y;
		break;

	case 10:
		res = x * y;
		break;

	case 11:
		res = x / y;
		break;

	case 12:
		res = x >> y;
		break;

	case 13:
		res = x << y;
		break;

	case 14:
		res = x & y;
		break;

	case 15:
		res = x | y;
		break;

	case 16:
		res = x % y;
		break;

	case 17:
		res = x ^ y;
		break;

	default:
		debug("unknown evaluate command");
		break;
	};

	pushStack(res);
}

// opcode procs
void VMContext::o1_unknownOpcode(void) {
	_error = true;

	debug("unknown opcode '0x%x'.", _argument);
	debug("parameters:\n"
		"Param0: %d\nParam1: %d\nParam2: %d\nParam3: %d\nParam4: %d\nParam5: %d\n"
		"Param0 as a string: %s\nParam1 as a string: %s\nParam2 as a string: %s\n"
		"Param3 as a string: %s\nParam4 as a string: %s\nParam5 as a string: %s\n",
		param(0), param(1), param(2), param(3), param(5), param(5),
		paramString(0), paramString(1), paramString(2), paramString(3),
		paramString(4), paramString(5));
}

void VMContext::o1_0x68(void) {
	debug("o1_0x68 was called with param0: '0x%x'", param(0));
	_error = true;
}
} // end of namespace Kyra
