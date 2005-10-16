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

void ScriptHelper::c1_jmpTo() {
	_curScript->ip = _curScript->dataPtr->data + (_parameter << 1);
}

void ScriptHelper::c1_setRetValue() {
	_curScript->retValue = _parameter;
}

void ScriptHelper::c1_pushRetOrPos() {
	switch (_parameter) {
		case 0:
			_curScript->stack[--_curScript->sp] = _curScript->retValue;
		break;
		
		case 1:
			_curScript->stack[--_curScript->sp] = (_curScript->ip - _curScript->dataPtr->data) / 2 + 1;
			_curScript->stack[--_curScript->sp] = _curScript->bp;
			_curScript->bp = _curScript->sp + 2;
		break;
		
		default:
			_continue = false;
			_curScript->ip = 0;
		break;
	}
}

void ScriptHelper::c1_push() {
	_curScript->stack[--_curScript->sp] = _parameter;
}

void ScriptHelper::c1_pushVar() {
	_curScript->stack[--_curScript->sp] = _curScript->variables[_parameter];
}

void ScriptHelper::c1_pushBPNeg() {
	_curScript->stack[--_curScript->sp] = _curScript->stack[(-(int32)(_parameter + 2)) + _curScript->bp];
}

void ScriptHelper::c1_pushBPAdd() {
	_curScript->stack[--_curScript->sp] = _curScript->stack[(_parameter - 1) + _curScript->bp];
}

void ScriptHelper::c1_popRetOrPos() {
	switch (_parameter) {
		case 0:
			_curScript->retValue = _curScript->stack[++_curScript->sp-1];
		break;
		
		case 1:
			if (_curScript->sp >= 60) {
				_continue = false;
				_curScript->ip = 0;
			} else {
				_curScript->bp = _curScript->stack[++_curScript->sp-1];
				_curScript->ip = _curScript->dataPtr->data + (_curScript->stack[++_curScript->sp-1] << 1);
			}
		break;
		
		default:
			_continue = false;
			_curScript->ip = 0;
		break;
	}
}

void ScriptHelper::c1_popVar() {
	_curScript->variables[_parameter] = _curScript->stack[++_curScript->sp-1];
}

void ScriptHelper::c1_popBPNeg() {
	_curScript->stack[(-(int32)(_parameter + 2)) + _curScript->bp] = _curScript->stack[++_curScript->sp-1];
}

void ScriptHelper::c1_popBPAdd() {
	_curScript->stack[(_parameter - 1) + _curScript->bp] = _curScript->stack[++_curScript->sp-1];
}

void ScriptHelper::c1_addSP() {
	_curScript->sp += _parameter;
}

void ScriptHelper::c1_subSP() {
	_curScript->sp -= _parameter;
}

void ScriptHelper::c1_execOpcode() {
	warning("c1_execOpcode STUB");
	// return 0 zero for now
	_curScript->retValue = 0;
}

void ScriptHelper::c1_ifNotJmp() {
	if (_curScript->stack[++_curScript->sp-1] != 0) {
		_parameter &= 0x7FFF;
		_curScript->ip = _curScript->dataPtr->data + (_parameter << 1);
	}
}

void ScriptHelper::c1_negate() {
	int16 value = _curScript->stack[_curScript->sp];
	switch (_parameter) {
		case 0:
			if (!value) {
				_curScript->stack[_curScript->sp] = 0;
			} else {
				_curScript->stack[_curScript->sp] = 1;
			}
		break;
		
		case 1:
			_curScript->stack[_curScript->sp] = -value;
		break;
		
		case 2:
			_curScript->stack[_curScript->sp] = ~value;
		break;
		
		default:
			_continue = false;
		break;
	}
}

void ScriptHelper::c1_eval() {
	int16 ret = 0;
	bool error = false;
	
	int16 val1 = _curScript->stack[++_curScript->sp-1];
	int16 val2 = _curScript->stack[++_curScript->sp-1];
	
	switch (_parameter) {
		case 0:
			if (!val2 || !val1) {
				ret = 0;
			} else {
				ret = 1;
			}
		break;
		
		case 1:
			if (val2 || val1) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 2:
			if (val1 == val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 3:
			if (val1 != val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 4:
			if (val1 > val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 5:
			if (val1 >= val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 6:
			if (val1 < val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 7:
			if (val1 <= val2) {
				ret = 1;
			} else {
				ret = 0;
			}
		break;
		
		case 8:
			ret = val1 + val2;
		break;
		
		case 9:
			ret = val2 - val1;
		break;
		
		case 10:
			ret = val1 * val2;
		break;
		
		case 11:
			ret = val2 / val1;
		break;
		
		case 12:
			ret = val2 >> val1;
		break;
		
		case 13:
			ret = val2 << val1;
		break;
		
		case 14:
			ret = val1 & val2;
		break;
		
		case 15:
			ret = val1 | val2;
		break;
		
		case 16:
			ret = val2 % val1;
		break;
		
		case 17:
			ret = val1 ^ val2;
		break;
		
		default:
			warning("Unknown evaluate func: %d", _parameter);
			error = true;
		break;
	}
	
	if (error) {
		_curScript->ip = 0;
		_continue = false;
	} else {
		_curScript->stack[--_curScript->sp] = ret;
	}
}

void ScriptHelper::c1_setRetAndJmp() {
	if (_curScript->sp >= 60) {
		_continue = false;
		_curScript->ip = 0;
	} else {
		_curScript->retValue = _curScript->stack[++_curScript->sp-1];
		uint16 temp = _curScript->stack[++_curScript->sp-1];
		_curScript->stack[60] = 0;
		_curScript->ip = &_curScript->dataPtr->data[temp*2];
	}
}

} // end of namespace Kyra
