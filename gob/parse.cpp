/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/util.h"
#include "gob/inter.h"

namespace Gob {

enum PointerType {
	kExecPtr = 0,
	kInterVar = 1,
	kResStr = 2
};

int32 encodePtr(char *ptr, int type) {
	int32 offset;

	switch (type) {
	case kExecPtr:
		offset = ptr - game_totFileData;
		break;
	case kInterVar:
		offset = ptr - inter_variables;
		break;
	case kResStr:
		offset = ptr - inter_resStr;
		break;
	default:
		error("encodePtr: Unknown pointer type");
	}
	assert((offset & 0xF0000000) == 0);
	return (type << 28) | offset;
}

char *decodePtr(int32 n) {
	char *ptr;

	switch (n >> 28) {
	case kExecPtr:
		ptr = game_totFileData;
		break;
	case kInterVar:
		ptr = inter_variables;
		break;
	case kResStr:
		ptr = inter_resStr;
		break;
	default:
		error("decodePtr: Unknown pointer type");
	}
	return ptr + (n & 0x0FFFFFFF);
}

int16 parse_parseExpr(char arg_0, byte *arg_2) {
	int32 values[20];
	byte operStack[20];
	int32 prevPrevVal;
	int32 prevVal;
	int32 curVal;
	int32 *valPtr;
	char *operPtr;
	byte *arrDescPtr;
	char var_C;
	byte operation;
	int16 dimCount;
	int16 temp;
	int16 temp2;
	uint16 offset;
	int16 dim;
	char var_1A;
	int16 stkPos;
	int16 brackStart;

	stkPos = -1;
	operPtr = (char *)(operStack - 1);
	valPtr = values - 1;

	while (1) {
		stkPos++;
		operPtr++;
		valPtr++;
		operation = *inter_execPtr++;
		if (operation >= 19 && operation <= 29) {
			switch (operation) {
			case 19:
				*operPtr = 20;
				*valPtr = READ_LE_UINT32(inter_execPtr);
				inter_execPtr += 4;
				break;

			case 20:
				*operPtr = 20;
				*valPtr = inter_load16();
				break;

			case 22:
				*operPtr = 22;
				*valPtr = encodePtr(inter_execPtr, kExecPtr);
				inter_execPtr += strlen(inter_execPtr) + 1;
				break;

			case 23:
				*operPtr = 20;
				*valPtr = VAR(inter_load16());
				break;

			case 25:
				*operPtr = 22;
				temp = inter_load16() * 4;
				*valPtr = encodePtr(inter_variables + temp, kInterVar);
				if (*inter_execPtr == 13) {
					inter_execPtr++;
					temp += parse_parseValExpr();
					*operPtr = 20;
					*valPtr = (uint8)*(inter_variables + temp);
				}
				break;

			case 26:
			case 28:
				*operPtr = operation - 6;
				temp = inter_load16();
				dimCount = *inter_execPtr++;
				arrDescPtr = (byte *)inter_execPtr;
				inter_execPtr += dimCount;
				offset = 0;
				dim = 0;
				for (dim = 0; dim < dimCount; dim++) {
					temp2 = parse_parseValExpr();
					offset = offset * arrDescPtr[dim] + temp2;
				}

				if (operation == 26) {
					*valPtr = VAR(temp + offset);
					break;
				}
				*valPtr = encodePtr(inter_variables + temp * 4 + offset * inter_animDataSize * 4, kInterVar);
				if (*inter_execPtr == 13) {
					inter_execPtr++;
					temp2 = parse_parseValExpr();
					*operPtr = 20;
					*valPtr = (uint8)*(inter_variables + temp * 4 + offset * 4 * inter_animDataSize + temp2);
				}
				break;

			case 29:
				operation = *inter_execPtr++;
				parse_parseExpr(10, 0);

				switch (operation) {
				case 5:
					inter_resVal = inter_resVal * inter_resVal;
					break;

				case 0:
				case 1:
				case 6:
					curVal = 1;
					prevVal = 1;

					do {
						prevPrevVal = prevVal;
						prevVal = curVal;
						curVal = (curVal + inter_resVal / curVal) / 2;
					} while (curVal != prevVal && curVal != prevPrevVal);
					inter_resVal = curVal;
					break;

				case 10:
					inter_resVal = util_getRandom(inter_resVal);
					break;

				case 7:
					if (inter_resVal < 0)
						inter_resVal = -inter_resVal;
					break;
				}
				*operPtr = 20;
				*valPtr = inter_resVal;
			}

			if (stkPos > 0 && (operPtr[-1] == 1 || operPtr[-1] == 11)) {
				stkPos--;
				operPtr--;
				valPtr--;

				if (*operPtr == 1) {
					*operPtr = 20;
					valPtr[0] = -valPtr[1];
				} else if (*operPtr == 11) {
					if (operPtr[1] == 23)
						*operPtr = 24;
					else
						*operPtr = 23;
				}
			}

			if (stkPos <= 0)
				continue;

			switch (operPtr[-1]) {
			case 2:
				if (operPtr[-2] == 22) {
					if (decodePtr(valPtr[-2]) != inter_resStr) {
						strcpy(inter_resStr, decodePtr(valPtr[-2]));
						valPtr[-2] = encodePtr(inter_resStr, kResStr);
					}
					strcat(inter_resStr, decodePtr(valPtr[0]));
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
			continue;
		}		// op>= 19 && op <= 29

		if (operation == arg_0 || operation == 30 || operation == 31 || operation == 10) {
			while (stkPos >= 2) {
				var_1A = 0;
				if (operPtr[-2] == 9 && (operation == 10 || operation == arg_0)) {
					operPtr[-2] = operPtr[-1];
					if (operPtr[-2] == 20 || operPtr[-2] == 22)
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
							if (operPtr[-1] == 23)
								operPtr[-2] = 24;
							else
								operPtr[-2] = 23;

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

					if (operation != arg_0)
						break;
				}	// if (operPtr[-2] == 9 && ...)

				for (brackStart = stkPos - 2; brackStart > 0 &&
				    operStack[brackStart] < 30 &&
				    operStack[brackStart] != 9; brackStart--);

				if (operStack[brackStart] >= 30 || operStack[brackStart] == 9)
					brackStart++;

				switch (operPtr[-2]) {
				case 2:
					if (operStack[brackStart] == 20) {
						values[brackStart] += valPtr[-1];
					} else if (operStack[brackStart] == 22) {
						if (decodePtr(values[brackStart]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(values[brackStart]));
							values[brackStart] = encodePtr(inter_resStr, kResStr);
						}
						strcat(inter_resStr, decodePtr(valPtr[-1]));
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
						if (decodePtr(valPtr[-3]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr(inter_resStr, kResStr);
						}
						if (strcmp(inter_resStr, decodePtr(valPtr[-1])) < 0)
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
						if (decodePtr(valPtr[-3]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr(inter_resStr, kResStr);
						}
						if (strcmp(inter_resStr, decodePtr(valPtr[-1])) <= 0)
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
						if (decodePtr(valPtr[-3]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr(inter_resStr, kResStr);
						}
						if (strcmp(inter_resStr, decodePtr(valPtr[-1])) > 0)
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
						if (decodePtr(valPtr[-3]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr(inter_resStr, kResStr);
						}
						if (strcmp(inter_resStr, decodePtr(valPtr[-1])) >= 0)
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
						if (decodePtr(valPtr[-3]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr(inter_resStr, kResStr);
						}
						if (strcmp(inter_resStr, decodePtr(valPtr[-1])) == 0)
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
						if (decodePtr(valPtr[-3]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr(inter_resStr, kResStr);
						}
						if (strcmp(inter_resStr, decodePtr(valPtr[-1])) != 0)
							operPtr[-3] = 24;
					}
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					break;

				default:
					var_1A = 1;
					break;
				}	// switch

				if (var_1A != 0)
					break;
			}	// while (stkPos >= 2)

			if (operation == 30 || operation == 31) {
				if (operPtr[-1] == 20) {
					if (valPtr[-1] != 0)
						operPtr[-1] = 24;
					else
						operPtr[-1] = 23;
				}

				if ((operation == 30 && operPtr[-1] == 24) ||
				    (operation == 31 && operPtr[-1] == 23)) {
					if (stkPos > 1 && operPtr[-2] == 9) {
						parse_skipExpr(10);
						operPtr[-2] = operPtr[-1];
						stkPos -= 2;
						operPtr -= 2;
						valPtr -= 2;
					} else {
						parse_skipExpr(arg_0);
					}
					operation = inter_execPtr[-1];
					if (stkPos > 0 && operPtr[-1] == 11) {
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

			if (operation != arg_0)
				continue;

			if (arg_2 != 0)
				*arg_2 = operStack[0];

			switch (operStack[0]) {
			case 20:
				inter_resVal = values[0];
				break;

			case 22:
				if (decodePtr(values[0]) != inter_resStr)
					strcpy(inter_resStr, decodePtr(values[0]));
				break;

			case 11:
				if (arg_2 != 0)
					*arg_2 ^= 1;
				break;

			case 23:
			case 24:
				break;

			default:
				inter_resVal = 0;
				if (arg_2 != 0)
					*arg_2 = 20;
				break;
			}
			return 0;
		}		// operation == arg_0 || operation == 30 || operation == 31 || operation == 10

		if (operation < 1 || operation > 11) {
			if (operation < 32 || operation > 37)
				continue;

			if (stkPos > 2) {
				if (operPtr[-2] == 2) {
					if (operPtr[-3] == 20) {
						valPtr[-3] += valPtr[-1];
					} else if (operPtr[-3] == 22) {
						if (decodePtr(valPtr[-3]) != inter_resStr) {
							strcpy(inter_resStr, decodePtr(valPtr[-3]));
							valPtr[-3] = encodePtr(inter_resStr, kResStr);
						}
						strcat(inter_resStr, decodePtr(valPtr[-1]));
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

void parse_skipExpr(char arg_0) {
	int16 dimCount;
	char operation;
	int16 num;
	int16 dim;

	num = 0;
	while (1) {
		operation = *inter_execPtr++;

		if (operation >= 19 && operation <= 29) {
			switch (operation) {
			case 20:
			case 23:
				inter_execPtr += 2;
				break;

			case 19:
				inter_execPtr += 4;
				break;

			case 22:
				inter_execPtr += strlen(inter_execPtr) + 1;
				break;

			case 25:
				inter_execPtr += 2;
				if (*inter_execPtr == 13) {
					inter_execPtr++;
					parse_skipExpr(12);
				}
				break;

			case 26:
			case 28:
				dimCount = inter_execPtr[2];
				inter_execPtr += 3 + dimCount;	// ???
				for (dim = 0; dim < dimCount; dim++)
					parse_skipExpr(12);

				if (operation == 28 && *inter_execPtr == 13) {
					inter_execPtr++;
					parse_skipExpr(12);
				}
				break;

			case 29:
				inter_execPtr++;
				parse_skipExpr(10);
			}
			continue;
		}		// if (operation >= 19 && operation <= 29)

		if (operation == 9) {
			num++;
			continue;
		}

		if (operation == 11 || (operation >= 1 && operation <= 8))
			continue;

		if (operation >= 30 && operation <= 37)
			continue;

		if (operation == 10)
			num--;

		if (operation != arg_0)
			continue;

		if (arg_0 != 10 || num < 0)
			return;
	}
}

int16 parse_parseValExpr() {
	int16 values[20];
	byte operStack[20];
	int16 *valPtr;
	byte *operPtr;
	byte *arrDesc;
	unsigned operation;
	int16 temp2;
	int16 dim;
	int16 dimCount;
	int16 temp;
	int16 offset;
	int16 stkPos;
	int16 brackPos;
	static int16 flag = 0;
	int16 oldflag;

	oldflag = flag;
	if (flag == 0) {
		flag = 1;
		parse_printExpr(99);
	}

	stkPos = -1;
	operPtr = operStack - 1;
	valPtr = values - 1;

	while (1) {
		stkPos++;
		operPtr++;
		valPtr++;

		operation = *inter_execPtr++;
		if (operation >= 19 && operation <= 29) {
			*operPtr = 20;
			switch (operation) {
			case 19:
				*valPtr = READ_LE_UINT32(inter_execPtr);
				inter_execPtr += 4;
				break;

			case 20:
				*valPtr = inter_load16();
				break;

			case 23:
				*valPtr = (uint16)VAR(inter_load16());
				break;

			case 25:
				temp = inter_load16() * 4;
				inter_execPtr++;
				temp += parse_parseValExpr();
				*valPtr = (uint8)*(inter_variables + temp);
				break;

			case 26:
			case 28:
				temp = inter_load16();
				dimCount = *inter_execPtr++;
				arrDesc = (byte*)inter_execPtr;
				inter_execPtr += dimCount;
				offset = 0;
				for (dim = 0; dim < dimCount; dim++) {
					temp2 = parse_parseValExpr();
					offset = arrDesc[dim] * offset + temp2;
				}
				if (operation == 26) {
					*valPtr = (uint16)VAR(temp + offset);
				} else {
					inter_execPtr++;
					temp2 = parse_parseValExpr();
					*valPtr = (uint8)*(inter_variables + temp * 4 + offset * 4 * inter_animDataSize + temp2);
				}
				break;

			case 29:
				operation = *inter_execPtr++;
				parse_parseExpr(10, 0);

				if (operation == 5) {
					inter_resVal = inter_resVal * inter_resVal;
				} else if (operation == 7) {
					if (inter_resVal < 0)
						inter_resVal = -inter_resVal;
				} else if (operation == 10) {
					inter_resVal = util_getRandom(inter_resVal);
				}
				*valPtr = inter_resVal;
				break;

			}	// switch
			if (stkPos > 0 && operPtr[-1] == 1) {
				stkPos--;
				operPtr--;
				valPtr--;
				operPtr[0] = 20;
				valPtr[0] = -valPtr[1];
			}

			if (stkPos > 0 && operPtr[-1] > 4 && operPtr[-1] < 9) {
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
			}	// if (stkPos > 0 && cmdPtr[-1] > 4 && cmdPtr[-1] < 9)
			continue;
		}

		if (operation >= 1 && operation <= 9) {
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
				if (stkPos > 1 && operPtr[-2] == 1) {
					valPtr[-2] = 20;
					valPtr[-2] = -valPtr[-1];

					stkPos--;
					operPtr--;
					valPtr--;
				}

				if (stkPos > 2 && operPtr[-2] > 4
				    && operPtr[-2] < 9) {
					stkPos -= 2;
					operPtr -= 2;
					valPtr -= 2;
					switch (operPtr[0]) {
					case 5:
						operPtr[-1] *= operPtr[1];
						break;

					case 6:
						operPtr[-1] /= operPtr[1];
						break;

					case 7:
						operPtr[-1] %= operPtr[1];
						break;

					case 8:
						operPtr[-1] &= operPtr[1];
						break;
					}
				}
				if (operation == 10)
					break;
			}	// operPtr[-2] == 9

			for (brackPos = stkPos - 2; brackPos > 0 &&
			    operStack[brackPos] < 30
			    && operStack[brackPos] != 9; brackPos--);

			if (operStack[brackPos] >= 30
			    || operStack[brackPos] == 9)
				brackPos++;

			if (operPtr[-2] < 2 || operPtr[-2] > 8)
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
			flag = oldflag;
			return values[0];
		}

		stkPos--;
		operPtr--;
		valPtr--;
	}
}

int16 parse_parseVarIndex() {
	int16 temp2;
	char *arrDesc;
	int16 dim;
	int16 dimCount;
	int16 operation;
	int16 temp;
	int16 offset;
	int16 val;

	operation = *inter_execPtr++;
	debug(5, "var parse = %d", operation);
	switch (operation) {
	case 23:
	case 25:
		temp = inter_load16() * 4;
		debug(5, "oper = %d", (int16)*inter_execPtr);
		if (operation == 25 && *inter_execPtr == 13) {
			inter_execPtr++;
			val = parse_parseValExpr();
			temp += val;
			debug(5, "parse subscript = %d", val);
		}
		return temp;

	case 26:
	case 28:
		temp = inter_load16() * 4;
		dimCount = *inter_execPtr++;
		arrDesc = inter_execPtr;
		inter_execPtr += dimCount;
		offset = 0;
		for (dim = 0; dim < dimCount; dim++) {
			temp2 = parse_parseValExpr();
			offset = arrDesc[dim] * offset + temp2;
		}
		offset *= 4;
		if (operation != 28)
			return temp + offset;

		if (*inter_execPtr == 13) {
			inter_execPtr++;
			temp += parse_parseValExpr();
		}
		return offset * inter_animDataSize + temp;

	default:
		return 0;
	}
}

void parse_printExpr(char arg_0) {
	int16 dimCount;
	char operation;
	int16 num;
	int16 dim;
	char *arrDesc;
	char func;
	char saved = 0;
	static char *savedPos = 0;

	// printExpr() is not safe function. It suffers from unability to process
	// stopTokens. So enable it only temporary when you need debugging.
	return;

	if (savedPos == 0) {
		savedPos = inter_execPtr;
		saved = 1;
	}

	num = 0;
	while (1) {
		operation = *inter_execPtr++;

		if (operation >= 19 && operation <= 29) {
			switch (operation) {
			case 19:
				debug(5, "%l", READ_LE_UINT32(inter_execPtr));
				inter_execPtr += 4;
				break;

			case 20:
				debug(5, "%d", inter_load16());
				break;

			case 22:
				debug(5, "\42%s\42", inter_execPtr);
				inter_execPtr += strlen(inter_execPtr) + 1;
				break;

			case 23:
				debug(5, "var_%d", inter_load16());
				break;

			case 25:
				debug(5, "(&var_%d)", inter_load16());
				if (*inter_execPtr == 13) {
					inter_execPtr++;
					debug(5, "{");
					parse_printExpr(12);
//                  debug(5, "}");
				}
				break;

			case 26:
			case 28:
				if (operation == 28)
					debug(5, "(&");

				debug(5, "var_%d[", inter_load16());
				dimCount = *inter_execPtr++;
				arrDesc = inter_execPtr;
				inter_execPtr += dimCount;
				for (dim = 0; dim < dimCount; dim++) {
					parse_printExpr(12);
					debug(5, " of %d", (int16)arrDesc[dim]);
					if (dim != dimCount - 1)
						debug(5, ",");
				}
				debug(5, "]");
				if (operation == 28)
					debug(5, ")");

				if (operation == 28 && *inter_execPtr == 13) {
					inter_execPtr++;
					debug(5, "{");
					parse_printExpr(12);
//                  debug(5, "}");
				}
				break;

			case 29:
				func = *inter_execPtr++;
				if (func == 5)
					debug(5, "sqr(");
				else if (func == 10)
					debug(5, "rand(");
				else if (func == 7)
					debug(5, "abs(");
				else if (func == 0 || func == 1 || func == 6)
					debug(5, "sqrt(");
				else
					debug(5, "id(");
				parse_printExpr(10);
				break;

			case 12:
				debug(5, "}");
				break;

			default:
				debug(5, "<%d>", (int16)operation);
				break;
			}
			continue;
		}		// if (operation >= 19 && operation <= 29)
		switch (operation) {
		case 9:
			debug(5, "(");
			break;

		case 11:
			debug(5, "!");
			break;

		case 10:
			debug(5, ")");
			break;

		case 1:
			debug(5, "-");
			break;

		case 2:
			debug(5, "+");
			break;

		case 3:
			debug(5, "-");
			break;

		case 4:
			debug(5, "|");
			break;

		case 5:
			debug(5, "*");
			break;

		case 6:
			debug(5, "/");
			break;

		case 7:
			debug(5, "%");
			break;

		case 8:
			debug(5, "&");
			break;

		case 30:
			debug(5, "||");
			break;

		case 31:
			debug(5, "&&");
			break;

		case 32:
			debug(5, "<");
			break;

		case 33:
			debug(5, "<=");
			break;

		case 34:
			debug(5, ">");
			break;

		case 35:
			debug(5, ">=");
			break;

		case 36:
			debug(5, "==");
			break;

		case 37:
			debug(5, "!=");
			break;

		case 99:
			debug(5, "\n");
			break;

		case 12:
			debug(5, "}");
			break;

		default:
			debug(5, "<%d>", (int16)operation);
			break;
		}

		if (operation == 9) {
			num++;
			continue;
		}

		if (operation == 11 || (operation >= 1 && operation <= 8))
			continue;

		if (operation >= 30 && operation <= 37)
			continue;

		if (operation == 10)
			num--;

		if (operation == arg_0) {
			if (arg_0 != 10 || num < 0) {

				if (saved != 0) {
					inter_execPtr = savedPos;
					savedPos = 0;
				}
				return;
			}
		}
	}
}

void parse_printVarIndex() {
	char *arrDesc;
	int16 dim;
	int16 dimCount;
	int16 operation;
	int16 temp;

	char *pos = inter_execPtr;

	operation = *inter_execPtr++;
	switch (operation) {
	case 23:
	case 25:
		temp = inter_load16() * 4;
		debug(5, "&var_%d", temp);
		if (operation == 25 && *inter_execPtr == 13) {
			inter_execPtr++;
			debug(5, "+");
			parse_printExpr(99);
		}
		break;

	case 26:
	case 28:
		debug(5, "&var_%d[", inter_load16());
		dimCount = *inter_execPtr++;
		arrDesc = inter_execPtr;
		inter_execPtr += dimCount;
		for (dim = 0; dim < dimCount; dim++) {
			parse_printExpr(12);
			debug(5, " of %d", (int16)arrDesc[dim]);
			if (dim != dimCount - 1)
				debug(5, ",");
		}
		debug(5, "]");

		if (operation == 28 && *inter_execPtr == 13) {
			inter_execPtr++;
			debug(5, "+");
			parse_printExpr(99);
		}
		break;

	default:
		debug(5, "var_0");
		break;
	}
	debug(5, "\n");
	inter_execPtr = pos;
	return;
}

} // End of namespace Gob
