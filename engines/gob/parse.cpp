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
	while (1) {
		operation = *_vm->_global->_inter_execPtr++;

		if ((operation >= 16) && (operation <= 29)) {
			switch (operation) {
			case 17:
			case 18:
			case 20:
			case 23:
			case 24:
				_vm->_global->_inter_execPtr += 2;
				break;

			case 19:
				_vm->_global->_inter_execPtr += 4;
				break;

			case 21:
				_vm->_global->_inter_execPtr += 1;
				break;

			case 22:
				_vm->_global->_inter_execPtr +=
					strlen((char *) _vm->_global->_inter_execPtr) + 1;
				break;

			case 25:
				_vm->_global->_inter_execPtr += 2;
				if (*_vm->_global->_inter_execPtr == 13) {
					_vm->_global->_inter_execPtr++;
					skipExpr(12);
				}
				break;

			case 16:
			case 26:
			case 27:
			case 28:
				dimCount = _vm->_global->_inter_execPtr[2];
				// skip header and dimensions
				_vm->_global->_inter_execPtr += 3 + dimCount;
				// skip indices
				for (dim = 0; dim < dimCount; dim++)
					skipExpr(12);

				if ((operation == 28) && (*_vm->_global->_inter_execPtr == 13)) {
					_vm->_global->_inter_execPtr++;
					skipExpr(12);
				}
				break;

			case 29:
				_vm->_global->_inter_execPtr++;
				skipExpr(10);
			}
			continue;
		} // if ((operation >= 16) && (operation <= 29))

		if (operation == 9) {
			num++;
			continue;
		}

		if ((operation == 11) || ((operation >= 1) && (operation <= 8)))
			continue;

		if ((operation >= 30) && (operation <= 37))
			continue;

		if (operation == 10)
			num--;

		if (operation != stopToken)
			continue;

		if ((stopToken != 10) || (num < 0))
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
	while (1) {
		operation = *_vm->_global->_inter_execPtr++;

		if ((operation >= 16) && (operation <= 29)) {
			// operands

			switch (operation) {
			case 17: // uint16 variable load
				debugN(5, "var16_%d", _vm->_inter->load16());
				break;

			case 18: // uint8 variable load:
				debugN(5, "var8_%d", _vm->_inter->load16());
				break;

			case 19: // int32/uint32 immediate
				debugN(5, "%d", READ_LE_UINT32(_vm->_global->_inter_execPtr));
				_vm->_global->_inter_execPtr += 4;
				break;

			case 20: // int16 immediate
				debugN(5, "%d", _vm->_inter->load16());
				break;

			case 21: // int8 immediate
				debugN(5, "%d", (int8) *_vm->_global->_inter_execPtr++);
				break;

			case 22: // string immediate
				debugN(5, "\42%s\42", _vm->_global->_inter_execPtr);
				_vm->_global->_inter_execPtr +=
					strlen((char *) _vm->_global->_inter_execPtr) + 1;
				break;

			case 23: // uint32 variable load
			case 24: // uint32 variable load as uint16
				debugN(5, "var_%d", _vm->_inter->load16());
				break;

			case 25: // string variable load
				debugN(5, "(&var_%d)", _vm->_inter->load16());
				if (*_vm->_global->_inter_execPtr == 13) {
					_vm->_global->_inter_execPtr++;
					debugN(5, "{");
					printExpr_internal(12); // this also prints the closing }
				}
				break;

			case 16: // uint8 array access
			case 26: // uint32 array access
			case 27: // uint16 array access
			case 28: // string array access
				debugN(5, "\n");
				if (operation == 28)
					debugN(5, "(&");

				debugN(5, "var_%d[", _vm->_inter->load16());
				dimCount = *_vm->_global->_inter_execPtr++;
				arrDesc = _vm->_global->_inter_execPtr;
				_vm->_global->_inter_execPtr += dimCount;
				for (dim = 0; dim < dimCount; dim++) {
					printExpr_internal(12);
					debugN(5, " of %d", (int16) arrDesc[dim]);
					if (dim != dimCount - 1)
						debugN(5, ",");
				}
				debugN(5, "]");
				if (operation == 28)
					debugN(5, ")");

				if ((operation == 28) && (*_vm->_global->_inter_execPtr == 13)) {
					_vm->_global->_inter_execPtr++;
					debugN(5, "{");
					printExpr_internal(12); // this also prints the closing }
				}
				break;

			case 29: // function
				func = *_vm->_global->_inter_execPtr++;
				if (func == 5)
					debugN(5, "sqr(");
				else if (func == 10)
					debugN(5, "rand(");
				else if (func == 7)
					debugN(5, "abs(");
				else if ((func == 0) || (func == 1) || (func == 6))
					debugN(5, "sqrt(");
				else
					debugN(5, "id(");
				printExpr_internal(10);
				break;
			}
			continue;
		}		// if ((operation >= 16) && (operation <= 29))

		// operators
		switch (operation) {
		case 9:
			debugN(5, "(");
			break;

		case 11:
			debugN(5, "!");
			break;

		case 10:
			debugN(5, ")");
			break;

		case 1:
			debugN(5, "-");
			break;

		case 2:
			debugN(5, "+");
			break;

		case 3:
			debugN(5, "-");
			break;

		case 4:
			debugN(5, "|");
			break;

		case 5:
			debugN(5, "*");
			break;

		case 6:
			debugN(5, "/");
			break;

		case 7:
			debugN(5, "%%");
			break;

		case 8:
			debugN(5, "&");
			break;

		case 30:
			debugN(5, "||");
			break;

		case 31:
			debugN(5, "&&");
			break;

		case 32:
			debugN(5, "<");
			break;

		case 33:
			debugN(5, "<=");
			break;

		case 34:
			debugN(5, ">");
			break;

		case 35:
			debugN(5, ">=");
			break;

		case 36:
			debugN(5, "==");
			break;

		case 37:
			debugN(5, "!=");
			break;

		case 99:
			debugN(5, "\n");
			break;

		case 12:
			debugN(5, "}");
			if (stopToken != 12) {
				debugN(5, "Closing paren without opening?");
			}
			break;

		default:
			debugN(5, "<%d>", (int16) operation);
			error("Parse::printExpr(): invalid operator in expression");
			break;
		}

		if (operation == 9) {
			num++;
			continue;
		}

		if ((operation == 11) || ((operation >= 1) && (operation <= 8)))
			continue;

		if ((operation >= 30) && (operation <= 37))
			continue;

		if (operation == 10)
			num--;

		if (operation == stopToken) {
			if ((stopToken != 10) || (num < 0)) {
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
	case 25:
		temp = _vm->_inter->load16() * 4;
		debugN(5, "&var_%d", temp);
		if ((operation == 25) && (*_vm->_global->_inter_execPtr == 13)) {
			_vm->_global->_inter_execPtr++;
			debugN(5, "+");
			printExpr(12);
		}
		break;

	case 26:
	case 28:
		debugN(5, "&var_%d[", _vm->_inter->load16());
		dimCount = *_vm->_global->_inter_execPtr++;
		arrDesc = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr += dimCount;
		for (dim = 0; dim < dimCount; dim++) {
			printExpr(12);
			debugN(5, " of %d", (int16) arrDesc[dim]);
			if (dim != dimCount - 1)
				debugN(5, ",");
		}
		debugN(5, "]");

		if ((operation == 28) && (*_vm->_global->_inter_execPtr == 13)) {
			_vm->_global->_inter_execPtr++;
			debugN(5, "+");
			printExpr(12);
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

} // End of namespace Gob
