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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/parse.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/inter.h"

namespace Gob {

Parse::Parse(GobEngine *vm) : _vm(vm) {
}

int32 Parse::encodePtr(byte *ptr, int type) {
	int32 offset = 0;

	switch (type) {
	case kExecPtr:
		offset = ptr - _vm->_game->_totFileData;
		break;
	case kInterVar:
		offset = ptr - ((byte *) _vm->_inter->_variables->getAddressOff8(0, 0));
		break;
	case kResStr:
		offset = ptr - ((byte *) _vm->_global->_inter_resStr);
		break;
	default:
		error("Parse::encodePtr(): Unknown pointer type");
	}
	assert((offset & 0xF0000000) == 0);
	return (type << 28) | offset;
}

byte *Parse::decodePtr(int32 n) {
	byte *ptr;

	switch (n >> 28) {
	case kExecPtr:
		ptr = _vm->_game->_totFileData;
		break;
	case kInterVar:
		ptr = (byte *) _vm->_inter->_variables->getAddressOff8(0, 0);
		break;
	case kResStr:
		ptr = (byte *) _vm->_global->_inter_resStr;
		break;
	default:
		error("Parse::decodePtr(): Unknown pointer type");
	}
	return ptr + (n & 0x0FFFFFFF);
}

void Parse::skipExpr(char stopToken) {
	int16 dimCount;
	byte operation;
	int16 num;
	int16 dim;

	num = 0;
	while (true) {
		operation = *_vm->_global->_inter_execPtr++;

		if ((operation >= 14) && (operation <= OP_FUNC)) {
			switch (operation) {
			case 14:
				_vm->_global->_inter_execPtr += 4;
				if (*_vm->_global->_inter_execPtr == 97)
					_vm->_global->_inter_execPtr++;
				break;

			case OP_LOAD_VAR_INT16:
			case OP_LOAD_VAR_INT8:
			case OP_LOAD_IMM_INT16:
			case 23:
			case 24:
				_vm->_global->_inter_execPtr += 2;
				break;

			case OP_LOAD_IMM_INT32:
				_vm->_global->_inter_execPtr += 4;
				break;

			case OP_LOAD_IMM_INT8:
				_vm->_global->_inter_execPtr += 1;
				break;

			case OP_LOAD_IMM_STR:
				_vm->_global->_inter_execPtr +=
					strlen((char *) _vm->_global->_inter_execPtr) + 1;
				break;

			case OP_LOAD_VAR_STR:
				_vm->_global->_inter_execPtr += 2;
				if (*_vm->_global->_inter_execPtr == 13) {
					_vm->_global->_inter_execPtr++;
					skipExpr(OP_END_MARKER);
				}
				break;

			case 15:
				_vm->_global->_inter_execPtr += 2;

			case OP_ARRAY_UINT8:
			case OP_ARRAY_UINT32:
			case OP_ARRAY_UINT16:
			case OP_ARRAY_STR:
				dimCount = _vm->_global->_inter_execPtr[2];
				// skip header and dimensions
				_vm->_global->_inter_execPtr += 3 + dimCount;
				// skip indices
				for (dim = 0; dim < dimCount; dim++)
					skipExpr(OP_END_MARKER);

				if ((operation == OP_ARRAY_STR) && (*_vm->_global->_inter_execPtr == 13)) {
					_vm->_global->_inter_execPtr++;
					skipExpr(OP_END_MARKER);
				}
				break;

			case OP_FUNC:
				_vm->_global->_inter_execPtr++;
				skipExpr(OP_END_EXPR);
			}
			continue;
		} // if ((operation >= OP_ARRAY_UINT8) && (operation <= OP_FUNC))

		if (operation == OP_BEGIN_EXPR) {
			num++;
			continue;
		}

		if ((operation == OP_NOT) || ((operation >= OP_NEG) && (operation <= 8)))
			continue;

		if ((operation >= OP_OR) && (operation <= OP_NEQ))
			continue;

		if (operation == OP_END_EXPR)
			num--;

		if (operation != stopToken)
			continue;

		if ((stopToken != OP_END_EXPR) || (num < 0))
			return;
	}
}

void Parse::printExpr(char stopToken) {
	// Expression printing disabled by default
	return;

	byte *savedPos = _vm->_global->_inter_execPtr;
	printExpr_internal(stopToken);

	// restore IP to start of expression
	_vm->_global->_inter_execPtr = savedPos;
}

void Parse::printExpr_internal(char stopToken) {
	int16 dimCount;
	byte operation;
	int16 num;
	int16 dim;
	byte *arrDesc;
	byte func;

	num = 0;
	while (true) {
		operation = *_vm->_global->_inter_execPtr++;

		if ((operation >= OP_ARRAY_UINT8) && (operation <= OP_FUNC)) {
			// operands

			switch (operation) {
			case OP_LOAD_VAR_INT16: // int16 variable load
				debugN(5, "var16_%d", _vm->_inter->load16());
				break;

			case OP_LOAD_VAR_INT8: // int8 variable load:
				debugN(5, "var8_%d", _vm->_inter->load16());
				break;

			case OP_LOAD_IMM_INT32: // int32/uint32 immediate
				debugN(5, "%d", READ_LE_UINT32(_vm->_global->_inter_execPtr));
				_vm->_global->_inter_execPtr += 4;
				break;

			case OP_LOAD_IMM_INT16: // int16 immediate
				debugN(5, "%d", _vm->_inter->load16());
				break;

			case OP_LOAD_IMM_INT8: // int8 immediate
				debugN(5, "%d", (int8) *_vm->_global->_inter_execPtr++);
				break;

			case OP_LOAD_IMM_STR: // string immediate
				debugN(5, "\42%s\42", _vm->_global->_inter_execPtr);
				_vm->_global->_inter_execPtr +=
					strlen((char *) _vm->_global->_inter_execPtr) + 1;
				break;

			case 23: // uint32 variable load
			case 24: // uint32 variable load as uint16
				debugN(5, "var_%d", _vm->_inter->load16());
				break;

			case OP_LOAD_VAR_STR: // string variable load
				debugN(5, "(&var_%d)", _vm->_inter->load16());
				if (*_vm->_global->_inter_execPtr == 13) {
					_vm->_global->_inter_execPtr++;
					debugN(5, "{");
					printExpr_internal(OP_END_MARKER); // this also prints the closing }
				}
				break;

			case OP_ARRAY_UINT8: // uint8 array access
			case OP_ARRAY_UINT32: // uint32 array access
			case OP_ARRAY_UINT16: // uint16 array access
			case OP_ARRAY_STR: // string array access
				debugN(5, "\n");
				if (operation == OP_ARRAY_STR)
					debugN(5, "(&");

				debugN(5, "var_%d[", _vm->_inter->load16());
				dimCount = *_vm->_global->_inter_execPtr++;
				arrDesc = _vm->_global->_inter_execPtr;
				_vm->_global->_inter_execPtr += dimCount;
				for (dim = 0; dim < dimCount; dim++) {
					printExpr_internal(OP_END_MARKER);
					debugN(5, " of %d", (int16) arrDesc[dim]);
					if (dim != dimCount - 1)
						debugN(5, ",");
				}
				debugN(5, "]");
				if (operation == OP_ARRAY_STR)
					debugN(5, ")");

				if ((operation == OP_ARRAY_STR) && (*_vm->_global->_inter_execPtr == 13)) {
					_vm->_global->_inter_execPtr++;
					debugN(5, "{");
					printExpr_internal(OP_END_MARKER); // this also prints the closing }
				}
				break;

			case OP_FUNC: // function
				func = *_vm->_global->_inter_execPtr++;
				if (func == FUNC_SQR)
					debugN(5, "sqr(");
				else if (func == FUNC_RAND)
					debugN(5, "rand(");
				else if (func == FUNC_ABS)
					debugN(5, "abs(");
				else if ((func == FUNC_SQRT1) || (func == FUNC_SQRT2) || (func == FUNC_SQRT3))
					debugN(5, "sqrt(");
				else
					debugN(5, "id(");
				printExpr_internal(OP_END_EXPR);
				break;
			}
			continue;
		}		// if ((operation >= OP_ARRAY_UINT8) && (operation <= OP_FUNC))

		// operators
		switch (operation) {
		case OP_BEGIN_EXPR:
			debugN(5, "(");
			break;

		case OP_NOT:
			debugN(5, "!");
			break;

		case OP_END_EXPR:
			debugN(5, ")");
			break;

		case OP_NEG:
			debugN(5, "-");
			break;

		case OP_ADD:
			debugN(5, "+");
			break;

		case OP_SUB:
			debugN(5, "-");
			break;

		case OP_BITOR:
			debugN(5, "|");
			break;

		case OP_MUL:
			debugN(5, "*");
			break;

		case OP_DIV:
			debugN(5, "/");
			break;

		case OP_MOD:
			debugN(5, "%%");
			break;

		case OP_BITAND:
			debugN(5, "&");
			break;

		case OP_OR:
			debugN(5, "||");
			break;

		case 31:
			debugN(5, "&&");
			break;

		case OP_LESS:
			debugN(5, "<");
			break;

		case OP_LEQ:
			debugN(5, "<=");
			break;

		case OP_GREATER:
			debugN(5, ">");
			break;

		case OP_GEQ:
			debugN(5, ">=");
			break;

		case OP_EQ:
			debugN(5, "==");
			break;

		case OP_NEQ:
			debugN(5, "!=");
			break;

		case 99:
			debugN(5, "\n");
			break;

		case OP_END_MARKER:
			debugN(5, "}");
			if (stopToken != OP_END_MARKER) {
				debugN(5, "Closing paren without opening?");
			}
			break;

		default:
			debugN(5, "<%d>", (int16) operation);
			error("Parse::printExpr(): invalid operator in expression");
			break;
		}

		if (operation == OP_BEGIN_EXPR) {
			num++;
			continue;
		}

		if ((operation == OP_NOT) || ((operation >= OP_NEG) && (operation <= 8)))
			continue;

		if ((operation >= OP_OR) && (operation <= OP_NEQ))
			continue;

		if (operation == OP_END_EXPR)
			num--;

		if (operation == stopToken) {
			if ((stopToken != OP_END_EXPR) || (num < 0)) {
				return;
			}
		}
	}
}


void Parse::printVarIndex() {
	byte *arrDesc;
	int16 dim;
	int16 dimCount;
	int16 operation;
	int16 temp;

	byte *pos = _vm->_global->_inter_execPtr;

	operation = *_vm->_global->_inter_execPtr++;
	switch (operation) {
	case 23:
	case OP_LOAD_VAR_STR:
		temp = _vm->_inter->load16() * 4;
		debugN(5, "&var_%d", temp);
		if ((operation == OP_LOAD_VAR_STR) && (*_vm->_global->_inter_execPtr == 13)) {
			_vm->_global->_inter_execPtr++;
			debugN(5, "+");
			printExpr(OP_END_MARKER);
		}
		break;

	case OP_ARRAY_UINT32:
	case OP_ARRAY_STR:
		debugN(5, "&var_%d[", _vm->_inter->load16());
		dimCount = *_vm->_global->_inter_execPtr++;
		arrDesc = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr += dimCount;
		for (dim = 0; dim < dimCount; dim++) {
			printExpr(OP_END_MARKER);
			debugN(5, " of %d", (int16) arrDesc[dim]);
			if (dim != dimCount - 1)
				debugN(5, ",");
		}
		debugN(5, "]");

		if ((operation == OP_ARRAY_STR) && (*_vm->_global->_inter_execPtr == 13)) {
			_vm->_global->_inter_execPtr++;
			debugN(5, "+");
			printExpr(OP_END_MARKER);
		}
		break;

	default:
		debugN(5, "var_0");
		break;
	}
	debugN(5, "\n");
	_vm->_global->_inter_execPtr = pos;
	return;
}

int Parse::cmpHelper(byte *operPtr, int32 *valPtr) {
	byte var_C = operPtr[-3];
	int cmpTemp;
	if (var_C == OP_LOAD_IMM_INT16) {
		cmpTemp = (int)valPtr[-3] - (int)valPtr[-1];
	} else if (var_C == OP_LOAD_IMM_STR) {
		if ((char *)decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
			strcpy(_vm->_global->_inter_resStr, (char *)decodePtr(valPtr[-3]));
			valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
		}
		cmpTemp = strcmp(_vm->_global->_inter_resStr, (char *)decodePtr(valPtr[-1]));
	}

	return cmpTemp;
}


} // End of namespace Gob
