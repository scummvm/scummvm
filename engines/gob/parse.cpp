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
	_resultStr[0] = 0;
	_resultInt = 0;
}

int32 Parse::encodePtr(byte *ptr, int type) {
	int32 offset = 0;

	switch (type) {
	case kExecPtr:
		offset = ptr - _vm->_game->_totFileData;
		break;
	case kInterVar:
		offset = ptr - ((byte *) _vm->_inter->_variables->getAddressOff8(0));
		break;
	case kResStr:
		offset = ptr - ((byte *) _vm->_parse->_resultStr);
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
		ptr = (byte *) _vm->_inter->_variables->getAddressOff8(0);
		break;
	case kResStr:
		ptr = (byte *) _vm->_parse->_resultStr;
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
			case OP_LOAD_VAR_INT32:
			case OP_LOAD_VAR_INT32_AS_INT16:
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

			case OP_ARRAY_INT8:
			case OP_ARRAY_INT32:
			case OP_ARRAY_INT16:
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
		} // if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC))

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

		if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC)) {
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

			case OP_LOAD_VAR_INT32:
			case OP_LOAD_VAR_INT32_AS_INT16:
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

			case OP_ARRAY_INT8:  // int8 array access
			case OP_ARRAY_INT32: // int32 array access
			case OP_ARRAY_INT16: // int16 array access
			case OP_ARRAY_STR:   // string array access
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
		}		// if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC))

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
	case OP_LOAD_VAR_INT32:
	case OP_LOAD_VAR_STR:
		temp = _vm->_inter->load16() * 4;
		debugN(5, "&var_%d", temp);
		if ((operation == OP_LOAD_VAR_STR) && (*_vm->_global->_inter_execPtr == 13)) {
			_vm->_global->_inter_execPtr++;
			debugN(5, "+");
			printExpr(OP_END_MARKER);
		}
		break;

	case OP_ARRAY_INT32:
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
	byte type = operPtr[-3];
	int cmpTemp = 0;

	if (type == OP_LOAD_IMM_INT16) {
		cmpTemp = (int)valPtr[-3] - (int)valPtr[-1];
	} else if (type == OP_LOAD_IMM_STR) {
		if ((char *)decodePtr(valPtr[-3]) != _vm->_parse->_resultStr) {
			strcpy(_vm->_parse->_resultStr, (char *)decodePtr(valPtr[-3]));
			valPtr[-3] = encodePtr((byte *) _vm->_parse->_resultStr, kResStr);
		}
		cmpTemp = strcmp(_vm->_parse->_resultStr, (char *)decodePtr(valPtr[-1]));
	}

	return cmpTemp;
}

bool Parse::getVarBase(uint32 &varBase, bool mindStop,
		uint16 *size, uint16 *type) {

	varBase = 0;

	byte operation = *_vm->_global->_inter_execPtr;
	while ((operation == 14) || (operation == 15)) {
		_vm->_global->_inter_execPtr++;

		if (operation == 14) {
			// Add a direct offset

			varBase += _vm->_inter->load16() * 4;

			if (size)
				*size = READ_LE_UINT16(_vm->_global->_inter_execPtr);
			if (type)
				*type = 14;

			_vm->_global->_inter_execPtr += 2;

			debugC(2, kDebugParser, "varBase: %d, by %d", varBase, operation);

			if (*_vm->_global->_inter_execPtr != 97) {
				if (mindStop)
					return true;
			} else
				_vm->_global->_inter_execPtr++;

		} else if (operation == 15) {
			// Add an offset from an array

			varBase += _vm->_inter->load16() * 4;

			uint16 offset1 = _vm->_inter->load16();

			if (size)
				*size = offset1;
			if (type)
				*type = 15;

			uint8 dimCount = *_vm->_global->_inter_execPtr++;
			byte *dimArray = _vm->_global->_inter_execPtr;

			_vm->_global->_inter_execPtr += dimCount;

			uint16 offset2 = 0;
			for (int i = 0; i < dimCount; i++) {
				int16 dim = CLIP<int>(parseValExpr(OP_END_MARKER), 0, dimArray[i] - 1);

				offset2 = offset2 * dimArray[i] + dim;
			}

			varBase += offset2 * offset1 * 4;

			debugC(2, kDebugParser, "varBase: %d, by %d", varBase, operation);

			if (*_vm->_global->_inter_execPtr != 97) {
				if (mindStop)
					return true;
			} else
				_vm->_global->_inter_execPtr++;
		}

		operation = *_vm->_global->_inter_execPtr;
	}

	return false;
}

int16 Parse::parseVarIndex(uint16 *size, uint16 *type) {
	int16 temp2;
	byte *arrDesc;
	int16 dim;
	int16 dimCount;
	int16 operation;
	int16 temp;
	int16 offset;
	int16 val;
	uint32 varBase;

	if (getVarBase(varBase, true, size, type))
		return varBase;

	operation = *_vm->_global->_inter_execPtr++;

	if (size)
		*size = 0;
	if (type)
		*type = operation;

	debugC(5, kDebugParser, "var parse = %d", operation);
	switch (operation) {
	case OP_ARRAY_INT8:
	case OP_ARRAY_INT32:
	case OP_ARRAY_INT16:
	case OP_ARRAY_STR:
		temp = _vm->_inter->load16();
		dimCount = *_vm->_global->_inter_execPtr++;
		arrDesc = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr += dimCount;
		offset = 0;
		for (dim = 0; dim < dimCount; dim++) {
			temp2 = parseValExpr(OP_END_MARKER);
			offset = arrDesc[dim] * offset + temp2;
		}
		if (operation == OP_ARRAY_INT8)
			return varBase + temp + offset;
		if (operation == OP_ARRAY_INT32)
			return varBase + (temp + offset) * 4;
		if (operation == OP_ARRAY_INT16)
			return varBase + (temp + offset) * 2;
		temp *= 4;
		offset *= 4;
		if (*_vm->_global->_inter_execPtr == 13) {
			_vm->_global->_inter_execPtr++;
			temp += parseValExpr(OP_END_MARKER);
		}
		return varBase + offset * _vm->_global->_inter_animDataSize + temp;

	case OP_LOAD_VAR_INT16:
		return varBase + _vm->_inter->load16() * 2;

	case OP_LOAD_VAR_INT8:
		return varBase + _vm->_inter->load16();

	case OP_LOAD_VAR_INT32:
	case OP_LOAD_VAR_INT32_AS_INT16:
	case OP_LOAD_VAR_STR:
		temp = _vm->_inter->load16() * 4;
		debugC(5, kDebugParser, "oper = %d",
				(int16) *_vm->_global->_inter_execPtr);
		if ((operation == OP_LOAD_VAR_STR) && (*_vm->_global->_inter_execPtr == 13)) {
			_vm->_global->_inter_execPtr++;
			val = parseValExpr(OP_END_MARKER);
			temp += val;
			debugC(5, kDebugParser, "parse subscript = %d", val);
		}
		return varBase + temp;

	default:
		return 0;
	}
}

int16 Parse::parseValExpr(byte stopToken) {
	parseExpr(stopToken, 0);

	return _vm->_parse->_resultInt;
}

// Load a value according to the operation
void Parse::loadValue(byte operation, uint32 varBase, byte *operPtr, int32 *valPtr) {
	int16 dimCount;
	int16 temp;
	int16 temp2;
	int16 offset;
	int16 dim;
	byte *arrDescPtr;
	int32 prevPrevVal;
	int32 prevVal;
	int32 curVal;

	switch (operation) {
	case OP_ARRAY_INT8:
	case OP_ARRAY_INT32:
	case OP_ARRAY_INT16:
	case OP_ARRAY_STR:
		*operPtr = (operation == OP_ARRAY_STR) ? OP_LOAD_IMM_STR : OP_LOAD_IMM_INT16;
		temp = _vm->_inter->load16();
		dimCount = *_vm->_global->_inter_execPtr++;
		arrDescPtr = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr += dimCount;
		offset = 0;
		for (dim = 0; dim < dimCount; dim++) {
			temp2 = parseValExpr(OP_END_MARKER);
			offset = offset * arrDescPtr[dim] + temp2;
		}
		if (operation == OP_ARRAY_INT8)
			*valPtr = (int8) READ_VARO_UINT8(varBase + temp + offset);
		else if (operation == OP_ARRAY_INT32)
			*valPtr = READ_VARO_UINT32(varBase + temp * 4 + offset * 4);
		else if (operation == OP_ARRAY_INT16)
			*valPtr = (int16) READ_VARO_UINT16(varBase + temp * 2 + offset * 2);
		else if (operation == OP_ARRAY_STR) {
			*valPtr = encodePtr(_vm->_inter->_variables->getAddressOff8(
						varBase + temp * 4 + offset * _vm->_global->_inter_animDataSize * 4),
					kInterVar);
			if (*_vm->_global->_inter_execPtr == 13) {
				_vm->_global->_inter_execPtr++;
				temp2 = parseValExpr(OP_END_MARKER);
				*operPtr = OP_LOAD_IMM_INT16;
				*valPtr = READ_VARO_UINT8(varBase + temp * 4 +
						offset * 4 * _vm->_global->_inter_animDataSize + temp2);
			}
		}
		break;

	case OP_LOAD_VAR_INT16:
		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = (int16) READ_VARO_UINT16(varBase + _vm->_inter->load16() * 2);
		break;

	case OP_LOAD_VAR_INT8:
		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = (int8) READ_VARO_UINT8(varBase + _vm->_inter->load16());
		break;

	case OP_LOAD_IMM_INT32:
		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = READ_LE_UINT32(varBase + _vm->_global->_inter_execPtr);
		_vm->_global->_inter_execPtr += 4;
		break;

	case OP_LOAD_IMM_INT16:
		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = _vm->_inter->load16();
		break;

	case OP_LOAD_IMM_INT8:
		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = (int8) *_vm->_global->_inter_execPtr++;
		break;

	case OP_LOAD_IMM_STR:
		*operPtr = OP_LOAD_IMM_STR;
		*valPtr = encodePtr(_vm->_global->_inter_execPtr, kExecPtr);
		_vm->_global->_inter_execPtr +=
			strlen((char *) _vm->_global->_inter_execPtr) + 1;
		break;

	case OP_LOAD_VAR_INT32:
		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = READ_VARO_UINT32(varBase + _vm->_inter->load16() * 4);
		break;

	case OP_LOAD_VAR_INT32_AS_INT16:
		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = (int16) READ_VARO_UINT16(varBase + _vm->_inter->load16() * 4);
		break;

	case OP_LOAD_VAR_STR:
		*operPtr = OP_LOAD_IMM_STR;
		temp = _vm->_inter->load16() * 4;
		*valPtr = encodePtr(_vm->_inter->_variables->getAddressOff8(varBase + temp), kInterVar);
		if (*_vm->_global->_inter_execPtr == 13) {
			_vm->_global->_inter_execPtr++;
			temp += parseValExpr(OP_END_MARKER);
			*operPtr = OP_LOAD_IMM_INT16;
			*valPtr = READ_VARO_UINT8(varBase + temp);
		}
		break;

	case OP_FUNC:
		operation = *_vm->_global->_inter_execPtr++;
		parseExpr(OP_END_EXPR, 0);

		switch (operation) {
		case FUNC_SQRT1:
		case FUNC_SQRT2:
		case FUNC_SQRT3:
			curVal = 1;
			prevVal = 1;

			do {
				prevPrevVal = prevVal;
				prevVal = curVal;
				curVal = (curVal + _vm->_parse->_resultInt / curVal) / 2;
			} while ((curVal != prevVal) && (curVal != prevPrevVal));
			_vm->_parse->_resultInt = curVal;
			break;

		case FUNC_SQR:
			_vm->_parse->_resultInt =
				_vm->_parse->_resultInt * _vm->_parse->_resultInt;
			break;

		case FUNC_ABS:
			if (_vm->_parse->_resultInt < 0)
				_vm->_parse->_resultInt = -_vm->_parse->_resultInt;
			break;

		case FUNC_RAND:
			_vm->_parse->_resultInt =
				_vm->_util->getRandom(_vm->_parse->_resultInt);
			break;
		}

		*operPtr = OP_LOAD_IMM_INT16;
		*valPtr = _vm->_parse->_resultInt;
		break;
	}
}

void Parse::simpleArithmetic1(byte *&operPtr, int32 *&valPtr, int16 &stkPos) {
	switch (operPtr[-1]) {
	case OP_ADD:
		if (operPtr[-2] == OP_LOAD_IMM_STR) {
			if ((char *) decodePtr(valPtr[-2]) != _vm->_parse->_resultStr) {
				strcpy(_vm->_parse->_resultStr, (char *) decodePtr(valPtr[-2]));
				valPtr[-2] = encodePtr((byte *) _vm->_parse->_resultStr, kResStr);
			}
			strcat(_vm->_parse->_resultStr, (char *) decodePtr(valPtr[0]));
			stkPos -= 2;
			operPtr -= 2;
			valPtr -= 2;
		}
		break;

	case OP_MUL:
		valPtr[-2] *= valPtr[0];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_DIV:
		valPtr[-2] /= valPtr[0];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_MOD:
		valPtr[-2] %= valPtr[0];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_BITAND:
		valPtr[-2] &= valPtr[0];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;
	}
}

void Parse::simpleArithmetic2(byte *&operPtr, int32 *&valPtr, int16 &stkPos) {
	if (stkPos > 1) {
		if (operPtr[-2] == OP_NEG) {
			operPtr[-2] = OP_LOAD_IMM_INT16;
			valPtr[-2] = -valPtr[-1];
			stkPos--;
			operPtr--;
			valPtr--;
		} else if (operPtr[-2] == OP_NOT) {
			operPtr[-2] = (operPtr[-1] == GOB_FALSE) ? GOB_TRUE : GOB_FALSE;
			stkPos--;
			operPtr--;
			valPtr--;
		}
	}

	if (stkPos > 2) {
		switch (operPtr[-2]) {
		case OP_MUL:
			valPtr[-3] *= valPtr[-1];
			stkPos -= 2;
			operPtr -= 2;
			valPtr -= 2;
			break;

		case OP_DIV:
			valPtr[-3] /= valPtr[-1];
			stkPos -= 2;
			operPtr -= 2;
			valPtr -= 2;
			break;

		case OP_MOD:
			valPtr[-3] %= valPtr[-1];
			stkPos -= 2;
			operPtr -= 2;
			valPtr -= 2;
			break;

		case OP_BITAND:
			valPtr[-3] &= valPtr[-1];
			stkPos -= 2;
			operPtr -= 2;
			valPtr -= 2;
			break;
		}
	}

}

// Complex arithmetics with brackets
bool Parse::complexArithmetic(byte *&operPtr, int32 *&valPtr, int16 &stkPos,
		byte *operStack, int32 *values, int16 brackStart) {

	switch (operPtr[-2]) {
	case OP_ADD:
		if (operStack[brackStart] == OP_LOAD_IMM_INT16) {
			values[brackStart] += valPtr[-1];
		} else if (operStack[brackStart] == OP_LOAD_IMM_STR) {
			if ((char *) decodePtr(values[brackStart]) != _vm->_parse->_resultStr) {
				strcpy(_vm->_parse->_resultStr, (char *) decodePtr(values[brackStart]));
				values[brackStart] =
					encodePtr((byte *) _vm->_parse->_resultStr, kResStr);
			}
			strcat(_vm->_parse->_resultStr, (char *) decodePtr(valPtr[-1]));
		}
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_SUB:
		values[brackStart] -= valPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_BITOR:
		values[brackStart] |= valPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_MUL:
		valPtr[-3] *= valPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_DIV:
		valPtr[-3] /= valPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_MOD:
		valPtr[-3] %= valPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_BITAND:
		valPtr[-3] &= valPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_OR:
		// (x OR false) == x
		// (x OR true) == true
		if (operPtr[-3] == GOB_FALSE)
			operPtr[-3] = operPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_AND:
		// (x AND false) == false
		// (x AND true) == x
		if (operPtr[-3] == GOB_TRUE)
			operPtr[-3] = operPtr[-1];
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_LESS:
		operPtr[-3] = (cmpHelper(operPtr, valPtr) < 0) ? GOB_TRUE : GOB_FALSE;
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_LEQ:
		operPtr[-3] = (cmpHelper(operPtr, valPtr) <= 0) ? GOB_TRUE : GOB_FALSE;
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_GREATER:
		operPtr[-3] = (cmpHelper(operPtr, valPtr) > 0) ? GOB_TRUE : GOB_FALSE;
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_GEQ:
		operPtr[-3] = (cmpHelper(operPtr, valPtr) >= 0) ? GOB_TRUE : GOB_FALSE;
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_EQ:
		operPtr[-3] = (cmpHelper(operPtr, valPtr) == 0) ? GOB_TRUE : GOB_FALSE;
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	case OP_NEQ:
		operPtr[-3] = (cmpHelper(operPtr, valPtr) != 0) ? GOB_TRUE : GOB_FALSE;
		stkPos -= 2;
		operPtr -= 2;
		valPtr -= 2;
		break;

	default:
		return true;
	}

	return false;
}

// Assign the result to the appropriate _result variable
void Parse::getResult(byte operation, int32 value, byte *type) {
	if (type != 0)
		*type = operation;

	switch (operation) {
	case OP_NOT:
		if (type != 0)
			*type ^= 1;
		break;

	case OP_LOAD_IMM_INT16:
		_vm->_parse->_resultInt = value;
		break;

	case OP_LOAD_IMM_STR:
		if ((char *) decodePtr(value) != _vm->_parse->_resultStr)
			strcpy(_vm->_parse->_resultStr, (char *) decodePtr(value));
		break;

	case OP_LOAD_VAR_INT32:
	case OP_LOAD_VAR_INT32_AS_INT16:
		break;

	default:
		_vm->_parse->_resultInt = 0;
		if (type != 0)
			*type = OP_LOAD_IMM_INT16;
		break;
	}
}

int16 Parse::parseExpr(byte stopToken, byte *type) {
	int32 values[20];
	byte operStack[20];
	int32 *valPtr;
	byte *operPtr;
	byte operation;
	bool escape;
	int16 stkPos;
	int16 brackStart;
	uint32 varBase;

	memset(operStack, 0, 20);

	stkPos = -1;
	operPtr = operStack - 1;
	valPtr = values - 1;

	while (true) {
		getVarBase(varBase);

		stkPos++;
		operPtr++;
		valPtr++;

		operation = *_vm->_global->_inter_execPtr++;
		if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC)) {

			loadValue(operation, varBase, operPtr, valPtr);

			if ((stkPos > 0) && ((operPtr[-1] == OP_NEG) || (operPtr[-1] == OP_NOT))) {
				stkPos--;
				operPtr--;
				valPtr--;

				if (*operPtr == OP_NEG) {
					*operPtr = OP_LOAD_IMM_INT16;
					valPtr[0] = -valPtr[1];
				} else
					*operPtr = (operPtr[1] == GOB_FALSE) ? GOB_TRUE : GOB_FALSE;
			}

			if (stkPos <= 0)
				continue;

			simpleArithmetic1(operPtr, valPtr, stkPos);

			continue;
		} // (op >= OP_ARRAY_INT8) && (op <= OP_FUNC)

		if ((operation == stopToken) || (operation == OP_OR) ||
				(operation == OP_AND) || (operation == OP_END_EXPR)) {
			while (stkPos >= 2) {
				escape = false;
				if ((operPtr[-2] == OP_BEGIN_EXPR) &&
						((operation == OP_END_EXPR) || (operation == stopToken))) {
					operPtr[-2] = operPtr[-1];
					if ((operPtr[-2] == OP_LOAD_IMM_INT16) || (operPtr[-2] == OP_LOAD_IMM_STR))
						valPtr[-2] = valPtr[-1];

					stkPos--;
					operPtr--;
					valPtr--;

					simpleArithmetic2(operPtr, valPtr, stkPos);

					if (operation != stopToken)
						break;
				}	// if ((operPtr[-2] == OP_BEGIN_EXPR) && ...)

				for (brackStart = (stkPos - 2); (brackStart > 0) &&
				    (operStack[brackStart] < OP_OR) && (operStack[brackStart] != OP_BEGIN_EXPR);
						brackStart--)
					;

				if ((operStack[brackStart] >= OP_OR) || (operStack[brackStart] == OP_BEGIN_EXPR))
					brackStart++;

				if (complexArithmetic(operPtr, valPtr, stkPos, operStack, values, brackStart))
					break;

			}	// while (stkPos >= 2)

			if ((operation == OP_OR) || (operation == OP_AND)) {
				if (operPtr[-1] == OP_LOAD_IMM_INT16) {
					if (valPtr[-1] != 0)
						operPtr[-1] = GOB_TRUE;
					else
						operPtr[-1] = GOB_FALSE;
				}

				if (((operation == OP_OR) && (operPtr[-1] == GOB_TRUE)) ||
				    ((operation == OP_AND) && (operPtr[-1] == GOB_FALSE))) {
					if ((stkPos > 1) && (operPtr[-2] == OP_BEGIN_EXPR)) {
						skipExpr(OP_END_EXPR);
						operPtr[-2] = operPtr[-1];
						stkPos -= 2;
						operPtr -= 2;
						valPtr -= 2;
					} else {
						skipExpr(stopToken);
					}
					operation = _vm->_global->_inter_execPtr[-1];
					if ((stkPos > 0) && (operPtr[-1] == OP_NOT)) {
						if (operPtr[0] == GOB_FALSE)
							operPtr[-1] = GOB_TRUE;
						else
							operPtr[-1] = GOB_FALSE;

						stkPos--;
						operPtr--;
						valPtr--;
					}
				} else
					operPtr[0] = operation;
			} else {
				stkPos--;
				operPtr--;
				valPtr--;
			}

			if (operation != stopToken)
				continue;

			getResult(operStack[0], values[0], type);

			return 0;
		}		// (operation == stopToken) || (operation == OP_OR) || (operation == OP_AND) || (operation == OP_END_EXPR)

		if ((operation < OP_NEG) || (operation > OP_NOT)) {
			if ((operation < OP_LESS) || (operation > OP_NEQ))
				continue;

			if (stkPos > 2) {
				if (operPtr[-2] == OP_ADD) {
					if (operPtr[-3] == OP_LOAD_IMM_INT16) {
						valPtr[-3] += valPtr[-1];
					} else if (operPtr[-3] == OP_LOAD_IMM_STR) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_parse->_resultStr) {
							strcpy(_vm->_parse->_resultStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_parse->_resultStr, kResStr);
						}
						strcat(_vm->_parse->_resultStr, (char *) decodePtr(valPtr[-1]));
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;

				} else if (operPtr[-2] == OP_SUB) {
					valPtr[-3] -= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
				} else if (operPtr[-2] == OP_BITOR) {
					valPtr[-3] |= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
				}
			}
		}
		*operPtr = operation;
	}
}

} // End of namespace Gob
