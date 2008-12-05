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
#include "gob/inter.h"

namespace Gob {

Parse_v2::Parse_v2(GobEngine *vm) : Parse_v1(vm) {
}

int16 Parse_v2::parseVarIndex(uint16 *arg_0, uint16 *arg_4) {
	int16 temp2;
	byte *arrDesc;
	int16 dim;
	int16 dimCount;
	int16 operation;
	int16 temp;
	int16 offset;
	int16 val;
	uint32 varPos = 0;

	operation = *_vm->_global->_inter_execPtr++;

	while ((operation == 14) || (operation == 15)) {
		if (operation == 14) {
			uint16 n = _vm->_inter->load16();
			varPos += n * 4;

			if (arg_0)
				*arg_0 = READ_LE_UINT16(_vm->_global->_inter_execPtr);
			if (arg_4)
				*arg_4 = 14;

			_vm->_global->_inter_execPtr += 2;

			debugC(2, kDebugParser, "parseVarIndex: Prefix %d (%d)", varPos, operation);

			if (*_vm->_global->_inter_execPtr != 97)
				return varPos;

			_vm->_global->_inter_execPtr++;

		} else if (operation == 15) {
			uint16 n = _vm->_inter->load16();
			varPos += n * 4;

			uint16 var_0C = _vm->_inter->load16();
			if (arg_0)
				*arg_0 = var_0C;
			if (arg_4)
				*arg_4 = 15;

			uint8 var_A = *_vm->_global->_inter_execPtr++;

			byte *var_12 = _vm->_global->_inter_execPtr;
			_vm->_global->_inter_execPtr += var_A;

			uint16 var_6 = 0;

			for (int i = 0; i < var_A; i++) {
				temp2 = parseValExpr(12);

				int16 ax = sub_12063(temp2, var_12[i], varPos, 0, 0);

				var_6 = var_6 * var_12[i] + ax;
			}

			varPos += var_6 * var_0C * 4;

			debugC(2, kDebugParser, "parseVarIndex: Prefix %d (%d)", varPos, operation);

			if (*_vm->_global->_inter_execPtr != 97)
				return varPos;

			_vm->_global->_inter_execPtr++;
		}

		operation = *_vm->_global->_inter_execPtr++;
	}

	if (arg_0)
		*arg_0 = 0;
	if (arg_4)
		*arg_4 = operation;

	debugC(5, kDebugParser, "var parse = %d", operation);
	switch (operation) {
	case 16:
	case 26:
	case 27:
	case 28:
		temp = _vm->_inter->load16();
		dimCount = *_vm->_global->_inter_execPtr++;
		arrDesc = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr += dimCount;
		offset = 0;
		for (dim = 0; dim < dimCount; dim++) {
			temp2 = parseValExpr(12);
			offset = arrDesc[dim] * offset + temp2;
		}
		if (operation == 16)
			return varPos + temp + offset;
		if (operation == 26)
			return varPos + (temp + offset) * 4;
		if (operation == 27)
			return varPos + (temp + offset) * 2;
		temp *= 4;
		offset *= 4;
		if (*_vm->_global->_inter_execPtr == 13) {
			_vm->_global->_inter_execPtr++;
			temp += parseValExpr(12);
		}
		return varPos + offset * _vm->_global->_inter_animDataSize + temp;

	case 17:
		return varPos + _vm->_inter->load16() * 2;

	case 18:
		return varPos + _vm->_inter->load16();

	case 23:
	case 24:
	case 25:
		temp = _vm->_inter->load16() * 4;
		debugC(5, kDebugParser, "oper = %d",
				(int16) *_vm->_global->_inter_execPtr);
		if ((operation == 25) && (*_vm->_global->_inter_execPtr == 13)) {
			_vm->_global->_inter_execPtr++;
			val = parseValExpr(12);
			temp += val;
			debugC(5, kDebugParser, "parse subscript = %d", val);
		}
		return varPos + temp;

	default:
		return 0;
	}
}

int16 Parse_v2::parseValExpr(byte stopToken) {
	int16 values[20];
	byte operStack[20];
	int16 *valPtr;
	byte *operPtr;
	byte *arrDesc;
	byte operation;
	int16 temp2;
	int16 dim;
	int16 dimCount;
	int16 temp;
	int16 offset;
	int16 stkPos;
	int16 brackPos;
	static int16 flag = 0;
	int16 oldflag;
	uint32 varPos = 0;

	memset(values, 0, 20 * sizeof(int16));

	oldflag = flag;
	if (flag == 0) {
		flag = 1;
		printExpr(stopToken);
	}

	stkPos = -1;
	operPtr = operStack - 1;
	valPtr = values - 1;

	while (1) {
		operation = *_vm->_global->_inter_execPtr++;

		while ((operation == 14) || (operation == 15)) {
			if (operation == 14) {
				uint16 n = _vm->_inter->load16();
				varPos += n * 4;

				_vm->_global->_inter_execPtr += 3;
			} else if (operation == 15) {
				uint16 n = _vm->_inter->load16();
				varPos += n * 4;

				uint16 var_0C = _vm->_inter->load16();
				uint8 var_A = *_vm->_global->_inter_execPtr++;

				byte *var_12 = _vm->_global->_inter_execPtr;
				_vm->_global->_inter_execPtr += var_A;

				uint16 var_6 = 0;

				for (int i = 0; i < var_A; i++) {
					temp2 = parseValExpr(12);

					int16 ax = sub_12063(temp2, var_12[i], varPos, 0, 0);

					var_6 = var_6 * var_12[i] + ax;
				}

				varPos += var_6 * var_0C * 4;

				_vm->_global->_inter_execPtr++;
			}

			debugC(2, kDebugParser, "parseValExpr: Prefix %d (%d)", varPos, operation);

			operation = *_vm->_global->_inter_execPtr++;
		}

		stkPos++;
		operPtr++;
		valPtr++;

		if ((operation >= 16) && (operation <= 29)) {
			*operPtr = 20;
			switch (operation) {
			case 16:
			case 26:
			case 27:
			case 28:
				temp = _vm->_inter->load16();
				dimCount = *_vm->_global->_inter_execPtr++;
				arrDesc = _vm->_global->_inter_execPtr;
				_vm->_global->_inter_execPtr += dimCount;
				offset = 0;
				for (dim = 0; dim < dimCount; dim++) {
					temp2 = parseValExpr(12);
					offset = arrDesc[dim] * offset + temp2;
				}
				if (operation == 16)
					*valPtr = (int8) READ_VARO_UINT8(varPos + temp + offset);
				else if (operation == 26)
					*valPtr = (uint16) READ_VARO_UINT32(varPos + temp * 4 + offset * 4);
				else if (operation == 27)
					*valPtr = READ_VARO_UINT16(varPos + temp * 2 + offset * 2);
				else if (operation == 28) {
					_vm->_global->_inter_execPtr++;
					temp2 = parseValExpr(12);
					*valPtr = READ_VARO_UINT8(varPos + temp * 4 +
							offset * 4 * _vm->_global->_inter_animDataSize + temp2);
				}
				break;

			case 17:
				*valPtr = READ_VARO_UINT16(varPos + _vm->_inter->load16() * 2);
				break;

			case 18:
				*valPtr = (int8) READ_VARO_UINT8(varPos + _vm->_inter->load16());
				break;

			case 19:
				*valPtr = (uint16) READ_LE_UINT32(varPos + _vm->_global->_inter_execPtr);
				_vm->_global->_inter_execPtr += 4;
				break;

			case 20:
				*valPtr = _vm->_inter->load16();
				break;

			case 21:
				*valPtr = (int8) *_vm->_global->_inter_execPtr++;
				break;

			case 23:
				*valPtr = (uint16) READ_VARO_UINT32(varPos + _vm->_inter->load16() * 4);
				break;

			case 24:
				*valPtr = READ_VARO_UINT16(varPos + _vm->_inter->load16() * 4);
				break;

			case 25:
				temp = _vm->_inter->load16() * 4;
				_vm->_global->_inter_execPtr++;
				temp += parseValExpr(12);
				*valPtr = READ_VARO_UINT8(varPos + temp);
				break;

			case 29:
				operation = *_vm->_global->_inter_execPtr++;
				parseExpr(10, 0);

				if (operation == 5) {
					_vm->_global->_inter_resVal =
						_vm->_global->_inter_resVal * _vm->_global->_inter_resVal;
				} else if (operation == 7) {
					if (_vm->_global->_inter_resVal < 0)
						_vm->_global->_inter_resVal = -_vm->_global->_inter_resVal;
				} else if (operation == 10) {
					_vm->_global->_inter_resVal =
						_vm->_util->getRandom(_vm->_global->_inter_resVal);
				}
				*valPtr = _vm->_global->_inter_resVal;
				break;

			}	// switch
			if ((stkPos > 0) && (operPtr[-1] == 1)) {
				stkPos--;
				operPtr--;
				valPtr--;
				operPtr[0] = 20;
				valPtr[0] = -valPtr[1];
			}

			if ((stkPos > 0) && (operPtr[-1] > 4) && (operPtr[-1] < 9)) {
				stkPos -= 2;
				operPtr -= 2;
				valPtr -= 2;

				switch (operPtr[1]) {
				case 5:
					valPtr[0] *= valPtr[2];
					break;

				case 6:
					valPtr[0] /= valPtr[2];
					break;

				case 7:
					valPtr[0] %= valPtr[2];
					break;

				case 8:
					valPtr[0] &= valPtr[2];
					break;
				}
			}	// if ((stkPos > 0) && (cmdPtr[-1] > 4) && (cmdPtr[-1] < 9))
			varPos = 0;
			continue;
		}

		if ((operation >= 1) && (operation <= 9)) {
			*operPtr = operation;
			continue;
		}

		while (stkPos >= 2) {
			if (operPtr[-2] == 9) {
				stkPos--;
				operPtr--;
				valPtr--;

				operPtr[-1] = operPtr[0];
				valPtr[-1] = valPtr[0];
				if ((stkPos > 1) && (operPtr[-2] == 1)) {
					operPtr[-2] = 20;
					valPtr[-2] = -valPtr[-1];

					stkPos--;
					operPtr--;
					valPtr--;
				}

				if ((stkPos > 2) && (operPtr[-2] > 4) && (operPtr[-2] < 9)) {
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					switch (operPtr[0]) {
					case 5:
						valPtr[-1] *= valPtr[1];
						break;

					case 6:
						valPtr[-1] /= valPtr[1];
						break;

					case 7:
						valPtr[-1] %= valPtr[1];
						break;

					case 8:
						valPtr[-1] &= valPtr[1];
						break;
					}
				}
				if (operation == 10)
					break;
			}	// operPtr[-2] == 9

			for (brackPos = (stkPos - 2); (brackPos > 0) &&
			    (operStack[brackPos] < 30) && (operStack[brackPos] != 9);
					brackPos--);

			if ((operStack[brackPos] >= 30) || (operStack[brackPos] == 9))
				brackPos++;

			if ((operPtr[-2] < 2) || (operPtr[-2] > 8))
				break;

			stkPos -= 2;
			operPtr -= 2;
			valPtr -= 2;
			switch (operPtr[0]) {
			case 2:
				values[brackPos] += valPtr[1];
				continue;
			case 3:
				values[brackPos] -= valPtr[1];
				continue;
			case 4:
				values[brackPos] |= valPtr[1];
				continue;
			case 5:
				valPtr[-1] *= valPtr[1];
				continue;
			case 6:
				valPtr[-1] /= valPtr[1];
				continue;
			case 7:
				valPtr[-1] %= valPtr[1];
				continue;
			case 8:
				valPtr[-1] &= valPtr[1];
				continue;
			}
		}

		if (operation != 10) {
			if (operation != stopToken) {
				debugC(5, kDebugParser, "stoptoken error: %d != %d",
						operation, stopToken);
			}
			flag = oldflag;
			return values[0];
		}

		stkPos--;
		operPtr--;
		valPtr--;
	}
}

int16 Parse_v2::parseExpr(byte stopToken, byte *arg_2) {
	int32 values[20];
	byte operStack[20];
	int32 prevPrevVal;
	int32 prevVal;
	int32 curVal;
	int32 *valPtr;
	byte *operPtr;
	byte *arrDescPtr;
	byte var_C;
	byte operation;
	int16 dimCount;
	int16 temp;
	int16 temp2;
	int16 offset;
	int16 dim;
	bool var_1A;
	int16 stkPos;
	int16 brackStart;
	uint32 varPos = 0;

	memset(operStack, 0, 20);

	stkPos = -1;
	operPtr = operStack - 1;
	valPtr = values - 1;

	while (1) {
		operation = *_vm->_global->_inter_execPtr++;

		while ((operation == 14) || (operation == 15)) {
			if (operation == 14) {
				uint16 n = _vm->_inter->load16();
				varPos += n * 4;

				_vm->_global->_inter_execPtr += 2;
				if (*_vm->_global->_inter_execPtr == 97)
					_vm->_global->_inter_execPtr++;
			} else if (operation == 15) {
				uint16 n = _vm->_inter->load16();
				varPos += n * 4;

				uint16 var_0C = _vm->_inter->load16();
				uint8 var_A = *_vm->_global->_inter_execPtr++;

				byte *var_12 = _vm->_global->_inter_execPtr;
				_vm->_global->_inter_execPtr += var_A;

				uint16 var_6 = 0;

				for (int i = 0; i < var_A; i++) {
					temp2 = parseValExpr(12);

					int16 ax = sub_12063(temp2, var_12[i], varPos, 0, 0);

					var_6 = var_6 * var_12[i] + ax;
				}

				varPos += var_6 * var_0C * 4;

				if (*_vm->_global->_inter_execPtr == 97)
					_vm->_global->_inter_execPtr++;
			}

			debugC(2, kDebugParser, "parseExpr: Prefix %d (%d)", varPos, operation);

			operation = *_vm->_global->_inter_execPtr++;
		}

		stkPos++;
		operPtr++;
		valPtr++;

		if ((operation >= 16) && (operation <= 29)) {
			switch (operation) {
			case 16:
			case 26:
			case 27:
			case 28:
				*operPtr = (operation == 28) ? 22 : 20;
				temp = _vm->_inter->load16();
				dimCount = *_vm->_global->_inter_execPtr++;
				arrDescPtr = _vm->_global->_inter_execPtr;
				_vm->_global->_inter_execPtr += dimCount;
				offset = 0;
				for (dim = 0; dim < dimCount; dim++) {
					temp2 = parseValExpr(12);
					offset = offset * arrDescPtr[dim] + temp2;
				}
				if (operation == 16)
					*valPtr = (int8) READ_VARO_UINT8(varPos + temp + offset);
				else if (operation == 26)
					*valPtr = READ_VARO_UINT32(varPos + temp * 4 + offset * 4);
				else if (operation == 27)
					*valPtr = (int16) READ_VARO_UINT16(varPos + temp * 2 + offset * 2);
				else if (operation == 28) {
					*valPtr = encodePtr(_vm->_inter->_variables->getAddressOff8(
								varPos + temp * 4 + offset * _vm->_global->_inter_animDataSize * 4, 0),
							kInterVar);
					if (*_vm->_global->_inter_execPtr == 13) {
						_vm->_global->_inter_execPtr++;
						temp2 = parseValExpr(12);
						*operPtr = 20;
						*valPtr = READ_VARO_UINT8(varPos + temp * 4 +
								offset * 4 * _vm->_global->_inter_animDataSize + temp2);
					}
				}
				break;

			case 17:
				*operPtr = 20;
				*valPtr = (int16) READ_VARO_UINT16(varPos + _vm->_inter->load16() * 2);
				break;

			case 18:
				*operPtr = 20;
				*valPtr = (int8) READ_VARO_UINT8(varPos + _vm->_inter->load16());
				break;

			case 19:
				*operPtr = 20;
				*valPtr = READ_LE_UINT32(varPos + _vm->_global->_inter_execPtr);
				_vm->_global->_inter_execPtr += 4;
				break;

			case 20:
				*operPtr = 20;
				*valPtr = _vm->_inter->load16();
				break;

			case 21:
				*operPtr = 20;
				*valPtr = (int8) *_vm->_global->_inter_execPtr++;
				break;

			case 22:
				*operPtr = 22;
				*valPtr = encodePtr(_vm->_global->_inter_execPtr, kExecPtr);
				_vm->_global->_inter_execPtr +=
					strlen((char *) _vm->_global->_inter_execPtr) + 1;
				break;

			case 23:
				*operPtr = 20;
				*valPtr = READ_VARO_UINT32(varPos + _vm->_inter->load16() * 4);
				break;

			case 24:
				*operPtr = 20;
				*valPtr = (int16) READ_VARO_UINT16(varPos + _vm->_inter->load16() * 4);
				break;

			case 25:
				*operPtr = 22;
				temp = _vm->_inter->load16() * 4;
				*valPtr = encodePtr(_vm->_inter->_variables->getAddressOff8(varPos + temp, 0), kInterVar);
				if (*_vm->_global->_inter_execPtr == 13) {
					_vm->_global->_inter_execPtr++;
					temp += parseValExpr(12);
					*operPtr = 20;
					*valPtr = READ_VARO_UINT8(varPos + temp);
				}
				break;

			case 29:
				operation = *_vm->_global->_inter_execPtr++;
				parseExpr(10, 0);

				switch (operation) {
				case 0:
				case 1:
				case 6:
					curVal = 1;
					prevVal = 1;

					do {
						prevPrevVal = prevVal;
						prevVal = curVal;
						curVal = (curVal + _vm->_global->_inter_resVal / curVal) / 2;
					} while ((curVal != prevVal) && (curVal != prevPrevVal));
					_vm->_global->_inter_resVal = curVal;
					break;

				case 5:
					_vm->_global->_inter_resVal =
						_vm->_global->_inter_resVal * _vm->_global->_inter_resVal;
					break;

				case 7:
					if (_vm->_global->_inter_resVal < 0)
						_vm->_global->_inter_resVal = -_vm->_global->_inter_resVal;
					break;

				case 10:
					_vm->_global->_inter_resVal =
						_vm->_util->getRandom(_vm->_global->_inter_resVal);
					break;
				}

				*operPtr = 20;
				*valPtr = _vm->_global->_inter_resVal;
				break;
			}

			if ((stkPos > 0) && ((operPtr[-1] == 1) || (operPtr[-1] == 11))) {
				stkPos--;
				operPtr--;
				valPtr--;

				if (*operPtr == 1) {
					*operPtr = 20;
					valPtr[0] = -valPtr[1];
				} else
					*operPtr = (operPtr[1] == 23) ? 24 : 23;
			}

			if (stkPos <= 0) {
				varPos = 0;
				continue;
			}

			switch (operPtr[-1]) {
			case 2:
				if (operPtr[-2] == 22) {
					if ((char *) decodePtr(valPtr[-2]) != _vm->_global->_inter_resStr) {
						strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-2]));
						valPtr[-2] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
					}
					strcat(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[0]));
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
				}
				break;

			case 5:
				valPtr[-2] *= valPtr[0];
				stkPos -= 2;
				operPtr -= 2;
				valPtr -= 2;
				break;

			case 6:
				valPtr[-2] /= valPtr[0];
				stkPos -= 2;
				operPtr -= 2;
				valPtr -= 2;
				break;

			case 7:
				valPtr[-2] %= valPtr[0];
				stkPos -= 2;
				operPtr -= 2;
				valPtr -= 2;
				break;

			case 8:
				valPtr[-2] &= valPtr[0];
				stkPos -= 2;
				operPtr -= 2;
				valPtr -= 2;
				break;
			}
			varPos = 0;
			continue;
		} // (op >= 16) && (op <= 29)

		if ((operation == stopToken) || (operation == 30) ||
				(operation == 31) || (operation == 10)) {
			while (stkPos >= 2) {
				var_1A = false;
				if ((operPtr[-2] == 9) &&
						((operation == 10) || (operation == stopToken))) {
					operPtr[-2] = operPtr[-1];
					if ((operPtr[-2] == 20) || (operPtr[-2] == 22))
						valPtr[-2] = valPtr[-1];

					stkPos--;
					operPtr--;
					valPtr--;

					if (stkPos > 1) {
						if (operPtr[-2] == 1) {
							operPtr[-2] = 20;
							valPtr[-2] = -valPtr[-1];
							stkPos--;
							operPtr--;
							valPtr--;
						} else if (operPtr[-2] == 11) {
							operPtr[-2] = (operPtr[-1] == 23) ? 24 : 23;
							stkPos--;
							operPtr--;
							valPtr--;
						}
					}	// stkPos > 1

					if (stkPos > 2) {
						switch (operPtr[-2]) {
						case 5:
							valPtr[-3] *= valPtr[-1];
							stkPos -= 2;
							operPtr -= 2;
							valPtr -= 2;
							break;

						case 6:
							valPtr[-3] /= valPtr[-1];
							stkPos -= 2;
							operPtr -= 2;
							valPtr -= 2;
							break;

						case 7:
							valPtr[-3] %= valPtr[-1];
							stkPos -= 2;
							operPtr -= 2;
							valPtr -= 2;
							break;

						case 8:
							valPtr[-3] &= valPtr[-1];
							stkPos -= 2;
							operPtr -= 2;
							valPtr -= 2;
							break;
						}	// switch
					}	// stkPos > 2

					if (operation != stopToken)
						break;
				}	// if ((operPtr[-2] == 9) && ...)

				for (brackStart = (stkPos - 2); (brackStart > 0) &&
				    (operStack[brackStart] < 30) && (operStack[brackStart] != 9);
						brackStart--);

				if ((operStack[brackStart] >= 30) || (operStack[brackStart] == 9))
					brackStart++;

				switch (operPtr[-2]) {
				case 2:
					if (operStack[brackStart] == 20) {
						values[brackStart] += valPtr[-1];
					} else if (operStack[brackStart] == 22) {
						if ((char *) decodePtr(values[brackStart]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(values[brackStart]));
							values[brackStart] =
								encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						strcat(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-1]));
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					continue;

				case 3:
					values[brackStart] -= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					continue;

				case 4:
					values[brackStart] |= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					continue;

				case 5:
					valPtr[-3] *= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 6:
					valPtr[-3] /= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 7:
					valPtr[-3] %= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 8:
					valPtr[-3] &= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 30:
					if (operPtr[-3] == 23)
						operPtr[-3] = operPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 31:
					if (operPtr[-3] == 24)
						operPtr[-3] = operPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 32:
					var_C = operPtr[-3];
					operPtr[-3] = 23;
					if (var_C == 20) {
						if (valPtr[-3] < valPtr[-1])
							operPtr[-3] = 24;
					} else if (var_C == 22) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						if (strcmp(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-1])) < 0)
							operPtr[-3] = 24;
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 33:
					var_C = operPtr[-3];
					operPtr[-3] = 23;
					if (var_C == 20) {
						if (valPtr[-3] <= valPtr[-1])
							operPtr[-3] = 24;
					} else if (var_C == 22) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						if (strcmp(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-1])) <= 0)
							operPtr[-3] = 24;
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 34:
					var_C = operPtr[-3];
					operPtr[-3] = 23;
					if (var_C == 20) {
						if (valPtr[-3] > valPtr[-1])
							operPtr[-3] = 24;
					} else if (var_C == 22) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						if (strcmp(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-1])) > 0)
							operPtr[-3] = 24;
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 35:
					var_C = operPtr[-3];
					operPtr[-3] = 23;
					if (var_C == 20) {
						if (valPtr[-3] >= valPtr[-1])
							operPtr[-3] = 24;
					} else if (var_C == 22) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						if (strcmp(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-1])) >= 0)
							operPtr[-3] = 24;
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 36:
					var_C = operPtr[-3];
					operPtr[-3] = 23;
					if (var_C == 20) {
						if (valPtr[-3] == valPtr[-1])
							operPtr[-3] = 24;
					} else if (var_C == 22) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						if (strcmp(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-1])) == 0)
							operPtr[-3] = 24;
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				case 37:
					var_C = operPtr[-3];
					operPtr[-3] = 23;
					if (var_C == 20) {
						if (valPtr[-3] != valPtr[-1])
							operPtr[-3] = 24;
					} else if (var_C == 22) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						if (scumm_stricmp(_vm->_global->_inter_resStr,
									(char *) decodePtr(valPtr[-1])) != 0)
							operPtr[-3] = 24;
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				default:
					var_1A = true;
					break;
				}	// switch

				if (var_1A)
					break;
			}	// while (stkPos >= 2)

			if ((operation == 30) || (operation == 31)) {
				if (operPtr[-1] == 20) {
					if (valPtr[-1] != 0)
						operPtr[-1] = 24;
					else
						operPtr[-1] = 23;
				}

				if (((operation == 30) && (operPtr[-1] == 24)) ||
				    ((operation == 31) && (operPtr[-1] == 23))) {
					if ((stkPos > 1) && (operPtr[-2] == 9)) {
						skipExpr(10);
						operPtr[-2] = operPtr[-1];
						stkPos -= 2;
						operPtr -= 2;
						valPtr -= 2;
					} else {
						skipExpr(stopToken);
					}
					operation = _vm->_global->_inter_execPtr[-1];
					if ((stkPos > 0) && (operPtr[-1] == 11)) {
						if (operPtr[0] == 23)
							operPtr[-1] = 24;
						else
							operPtr[-1] = 23;

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

			if (arg_2 != 0)
				*arg_2 = operStack[0];

			switch (operStack[0]) {
			case 11:
				if (arg_2 != 0)
					*arg_2 ^= 1;
				break;

			case 20:
				_vm->_global->_inter_resVal = values[0];
				break;

			case 22:
				if ((char *) decodePtr(values[0]) != _vm->_global->_inter_resStr)
					strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(values[0]));
				break;

			case 23:
			case 24:
				break;

			default:
				_vm->_global->_inter_resVal = 0;
				if (arg_2 != 0)
					*arg_2 = 20;
				break;
			}
			return 0;
		}		// (operation == stopToken) || (operation == 30) || (operation == 31) || (operation == 10)

		if ((operation < 1) || (operation > 11)) {
			if ((operation < 32) || (operation > 37))
				continue;

			if (stkPos > 2) {
				if (operPtr[-2] == 2) {
					if (operPtr[-3] == 20) {
						valPtr[-3] += valPtr[-1];
					} else if (operPtr[-3] == 22) {
						if ((char *) decodePtr(valPtr[-3]) != _vm->_global->_inter_resStr) {
							strcpy(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr((byte *) _vm->_global->_inter_resStr, kResStr);
						}
						strcat(_vm->_global->_inter_resStr, (char *) decodePtr(valPtr[-1]));
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;

				} else if (operPtr[-2] == 3) {
					valPtr[-3] -= valPtr[-1];
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
				} else if (operPtr[-2] == 4) {
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

int16 Parse_v2::sub_12063(int16 arg_0, byte arg_2, uint32 arg_3, uint16 arg_7, uint16 arg_9) {
	if (arg_0 < 0)
		return 0;

	if (arg_2 > arg_0)
		return arg_0;

	return arg_2 - 1;
}

} // End of namespace Gob
