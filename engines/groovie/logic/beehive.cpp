/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is dual-licensed.
 * In addition to the GPLv2 license mentioned above, MojoTouch has exclusively licensed
 * this code on November 10th, 2021, to be use in closed-source products.
 * Therefore, any contributions (commits) to it will also be dual-licensed.
 *
 */

#include "groovie/groovie.h"
#include "groovie/logic/beehive.h"

namespace Groovie {

namespace {
extern const int8 beehiveLogicTable1[368];
extern const int8 beehiveLogicTable2[800];
}

void BeehiveGame::overrideClick(byte *vars) {
	if (overrideIndex >= overrideMoves.size())
		return;

	int move = overrideMoves[overrideIndex];
	vars[0] = move / 10;
	vars[1] = move % 10;
}

void BeehiveGame::overrideMove(byte *vars) {
	if (overrideIndex >= overrideMoves.size())
		return;

	int from = overrideMoves[overrideIndex++];
	int to = overrideMoves[overrideIndex++];
	vars[0] = from / 10;
	vars[1] = from % 10;

	vars[2] = to / 10;
	vars[3] = to % 10;
}

void BeehiveGame::run(byte *scriptVariables) {
	int8 *hexagons = (int8 *)scriptVariables + 25;
	int8 *hexDifference = (int8 *)scriptVariables + 13;
	byte op = scriptVariables[14] - 1;

	enum kBeehiveColor {
		kBeehiveColorYellow = -1,
		kBeehiveColorRed = 1
	};

	debugC(1, kDebugLogic, "Beehive subop %d", op);

	int8 v21, v22, v24;
	int8 tempState[64];

	// init hexDifference on every iteration
	*hexDifference = 4;

	switch (op) {
	case 0:	// Init board's hexagons
		_maxDepth = 4;
		memset(_beehiveState, 0, HEXCOUNT);
		_beehiveState[0] = kBeehiveColorYellow;
		_beehiveState[4] = kBeehiveColorRed;
		_beehiveState[34] = kBeehiveColorYellow;
		_beehiveState[60] = kBeehiveColorRed;
		_beehiveState[56] = kBeehiveColorYellow;
		_beehiveState[26] = kBeehiveColorRed;
		return;

	case 1:
		memset(hexagons, 0, HEXCOUNT);
		scriptVariables[85] = 0;
		sub02(&v22, tempState);
		if (v22) {
			for (int i = 0; i < v22; i++)
				scriptVariables[tempState[i] + 25] = kBeehiveColorRed;
		} else {
			*hexDifference = getHexDifference();
		}
		return;

	case 2:	// Player clicks on a honey-filled (source) hexagon
		memset(hexagons, 0, HEXCOUNT);
		scriptVariables[85] = 0;
		//overrideClick(scriptVariables);
		v24 = 10 * scriptVariables[0] + scriptVariables[1];
		debugC(2, kDebugLogic, "Beehive player clicked %d", (int)v24);
		selectSourceHexagon(v24, &v22, tempState);
		for (int j = 0; j < v22; j++)
			scriptVariables[tempState[j] + 25] = kBeehiveColorRed;
		scriptVariables[v24 + 25] = kBeehiveColorRed;
		return;

	case 3:	// Player moves into an empty (destination) hexagon
		scriptVariables[24] = 1;
		scriptVariables[4] = 2;
		overrideMove(scriptVariables);
		v24 = 10 * scriptVariables[0] + scriptVariables[1];
		v22 = 10 * scriptVariables[2] + scriptVariables[3];
		debugC(1, kDebugLogic, "Beehive player moved from %d, to %d", (int)v24, (int)v22);
		sub16(v24, v22, hexDifference, (int8 *)scriptVariables + 16, (int8 *)scriptVariables + 17);
		scriptVariables[15] = scriptVariables[16];
		sub04(v24, v22, (int8 *)scriptVariables);
		return;

	case 4:	// Stauf plays
		scriptVariables[24] = 1;
		scriptVariables[4] = 1;
		calcStaufMove(&v24, &v22, hexDifference, &v21, (int8 *)scriptVariables + 16, (int8 *)scriptVariables + 17);
		// Execute method tail
		break;

	case 5:	// Calculate board state after every move
		if (scriptVariables[24] == 1) {
			scriptVariables[0] = scriptVariables[2];
			scriptVariables[1] = scriptVariables[3];
			scriptVariables[24] = 0;
		}

		if (scriptVariables[16]) {
			int8 v16 = scriptVariables[16] - 1;
			*hexDifference = 1;
			scriptVariables[16] = v16;
			v24 = 10 * scriptVariables[0] + scriptVariables[1];
			int8 v23 = scriptVariables[v16 + 17];
			scriptVariables[2] = v23 / 10;
			scriptVariables[3] = v23 % 10;
			sub04(v24, v23, (int8 *)scriptVariables);
		} else {
			*hexDifference = 4 - (scriptVariables[4] == 2 ? 1 : 0);
		}
		return;

	case 6:
		scriptVariables[24] = 1;
		scriptVariables[4] = 2;
		calcSamanthaMove(&v24, &v22, hexDifference, &v21, (int8 *)scriptVariables + 16, (int8 *)scriptVariables + 17);
		// Execute method tail
		break;

	default:
		return;
	}

	if (v24 == -1) {
		*hexDifference = getHexDifference();
	} else {
		scriptVariables[0] = v24 / 10;
		scriptVariables[1] = v24 % 10;
		scriptVariables[2] = v22 / 10;
		scriptVariables[3] = v22 % 10;
		sub04(v24, v22, (int8 *)scriptVariables);
	}
}

void BeehiveGame::sub02(int8 *a1, int8 *a2) {
	int8 v9 = -1;

	*a1 = 0;

	while (findCell(_beehiveState, &v9, -1)) {
		bool v3 = false;

		for (int i = 0; i < 6; i++) {
			if (v3)
				break;

			int8 move = beehiveLogicTable1[6 * v9 + i];
			if (move != -1 && !_beehiveState[move]) {
				a2[*a1] = v9;
				v3 = true;
				++*a1;
			}
		}

		for (int i = 0; i < 12; i++) {
			if (v3)
				break;

			int8 move = beehiveLogicTable2[12 * v9 + i];
			if (move != -1 && !_beehiveState[move]) {
				a2[*a1] = v9;
				v3 = true;
				++*a1;
			}
		}
	}

	if (!*a1) {
		for (int i = 0; i < HEXCOUNT; ++i)
			if (!_beehiveState[i])
				_beehiveState[i] = 1;
	}
}

void BeehiveGame::sub04(int8 a1, int8 a2, int8 *scriptVariables) {
	int v3 = 0;
	if (scriptVariables[13] == 1) {
		if (beehiveLogicTable1[6 * a1] != a2) {
			for (; v3 < 5; v3++) {
				if (beehiveLogicTable1[6 * a1 + v3] == a2)
					break;
			}
		}

		int v7 = v3 + 12;
		scriptVariables[5] = v7 / 10;
		scriptVariables[6] = v7 % 10;

		return;
	}

	scriptVariables[10] = 0;
	scriptVariables[7] = 0;
	if (beehiveLogicTable2[12 * a1] != a2) {
		for (; v3 < 11; v3++) {
			if (beehiveLogicTable2[12 * a1 + v3] == a2)
				break;
		}
	}

	scriptVariables[5] = v3 / 10;
	int8 v5 = -1;
	int8 v6 = -1;
	scriptVariables[6] = v3 % 10;

	switch (v3) {
	case 0:
		v6 = beehiveLogicTable1[6 * a1];
		break;
	case 1:
		v5 = beehiveLogicTable1[6 * a1];
		// fall through
	case 2:
		v6 = beehiveLogicTable1[6 * a1 + 1];
		break;
	case 3:
		v5 = beehiveLogicTable1[6 * a1 + 1];
		// fall through
	case 4:
		v6 = beehiveLogicTable1[6 * a1 + 2];
		break;
	case 5:
		v5 = beehiveLogicTable1[6 * a1 + 2];
		// fall through
	case 6:
		v6 = beehiveLogicTable1[6 * a1 + 3];
		break;
	case 7:
		v5 = beehiveLogicTable1[6 * a1 + 3];
		// fall through
	case 8:
		v6 = beehiveLogicTable1[6 * a1 + 4];
		break;
	case 9:
		v5 = beehiveLogicTable1[6 * a1 + 4];
		// fall through
	case 10:
		v6 = beehiveLogicTable1[6 * a1 + 5];
		break;
	case 11:
		v6 = beehiveLogicTable1[6 * a1 + 5];
		v5 = beehiveLogicTable1[6 * a1];
		break;
	default:
		v6 = 0;
		break;
	}

	int8 v4 = 0;

	if (v5 != -1)
		v4 = _beehiveState[v5];

	if (_beehiveState[v6]) {
		scriptVariables[8] = v6 / 10;
		scriptVariables[9] = v6 % 10;
		scriptVariables[7] = 2 - (_beehiveState[v6] == 1 ? 1 : 0);
	}

	if (v4) {
		scriptVariables[11] = v5 / 10;
		scriptVariables[12] = v5 % 10;
		scriptVariables[10] = 2 - (v4 == 1 ? 1 : 0);
	}
}

void BeehiveGame::calcSamanthaMove(int8 *a1, int8 *a2, int8 *a3, int8 *a4, int8 *a5, int8 *a6) {
	int8 params[4];

	*a4 = 0;
	_maxDepth = 5;// in the original game Samantha did 4 like Stauf

	if (calcMove(_beehiveState, -125, -1, _maxDepth, 0, params) == 125
			&& (*a4 = 1, calcMove(_beehiveState, -125, -1, _maxDepth, 1, params) == 125)) {
		*a1 = -1;
		*a2 = -1;
		for (int i = 0; i < HEXCOUNT; ++i) {
			if (!_beehiveState[i])
				_beehiveState[i] = 1;
		}
	} else {
		*a1 = params[1];
		*a2 = params[2];
		*a3 = params[0];
		sub17(_beehiveState, -1, params, a5, a6);
	}
}

void BeehiveGame::calcStaufMove(int8 *a1, int8 *a2, int8 *a3, int8 *a4, int8 *a5, int8 *a6) {
	int8 params[4];

	*a4 = 0;

	_maxDepth = 4;
	if (_easierAi) {
		int numPieces = 0;
		for (int i = 0; i < HEXCOUNT; ++i)
			numPieces += _beehiveState[i] != 0;

		if (numPieces < 16)
			_maxDepth = 3;
		else
			_maxDepth = 1;
	}

	if (calcMove(_beehiveState, 125, 1, _maxDepth, 0, params) == -125
			&& (*a4 = 1, calcMove(_beehiveState, 125, 1, _maxDepth, 1, params) == -125)) {
		*a1 = -1;
		*a2 = -1;
		for (int i = 0; i < HEXCOUNT; ++i) {
			if (!_beehiveState[i])
				_beehiveState[i] = -1;
		}
	} else {
		*a1 = params[1];
		*a2 = params[2];
		*a3 = params[0];
		sub17(_beehiveState, 1, params, a5, a6);
	}
}

int8 BeehiveGame::sub11(int8 *beehiveState, int8 *a2, int8 *a3, int8 *a4, int8 a5, int8 a6, int8 *a7) {
	if (*a2 == -1) {
		if (!findCell(beehiveState, a2, a5))
			return 0;
	}

	int8 v16 = 0;

	while (1) {
		while (1) {
			if (v16)
				return 1;

			for (; *a3 < 6; (*a3)++) {
				if (v16)
					break;

				int8 v9 = beehiveLogicTable1[6 * *a2 + *a3];

				if (v9 != -1 && !beehiveState[v9] && *a2 < sub12(beehiveState, a5, v9, *a2)) {
					v16 = 1;
					*a7 = 1;
					a7[1] = *a2;
					a7[2] = beehiveLogicTable1[6 * *a2 + *a3];
				}
			}

			if (*a4 >= 12)
				break;

			while (!v16) {
				int8 v11 = beehiveLogicTable2[12 * *a2 + *a4];

				if (v11 != -1
						&& !beehiveState[v11]
						&& !sub13(beehiveState, v11, a5)
						&& sub13(beehiveState, beehiveLogicTable2[12 * *a2 + *a4], -a5)) {
					int8 v12 = sub13(beehiveState, *a2, -a5);
					int8 v13 = *a4 >> 1;
					int8 v14 = ~(1 << v13) & v12;

					if ((*a4 & 1) != 0) {
						if (v13 == 5)
							v14 &= ~1u;
						else
							v14 &= ~(1 << (v13 + 1));
					}

					if (!v14 || !sub13(beehiveState, *a2, a5) || a6) {
						v16 = 1;
						*a7 = 2;
						a7[1] = *a2;
						a7[2] = beehiveLogicTable2[12 * *a2 + *a4];
					}
				}

				(*a4)++;
				if (*a4 >= 12)
					break;
			}

			if (*a4 >= 12)
				break;
		}

		if (v16)
			return 1;

		if (!findCell(beehiveState, a2, a5))
			return 0;

		*a3 = 0;
		*a4 = 0;
	}
}

int8 BeehiveGame::sub12(int8 *beehiveState, int8 a2, int8 a3, int8 a4) {
	int8 result = 125;

	for (int i = 0; i < 6; i++) {
		int8 v7 = beehiveLogicTable1[i + 6 * a3];

		if (v7 != -1 && beehiveState[v7] == a2 && a4 != v7 && result > v7)
			result = beehiveLogicTable1[i + 6 * a3];
	}

	return result;
}

int8 BeehiveGame::sub13(int8 *beehiveState, int8 a2, int8 a3) {
	int result = 0;

	for (int i = 0; i < 6; i++) {
		int8 v5 = beehiveLogicTable1[6 * a2 + i];

		if (v5 != -1 && beehiveState[v5] == a3)
			result |= 1 << i;
	}

	return result;
}

void BeehiveGame::sub15(int8 *beehiveState, int8 a2, int8 *a3) {
	beehiveState[a3[2]] = a2;

	if (*a3 == 2)
		beehiveState[a3[1]] = 0;

	for (int i = 0; i < 6; ++i) {
		int8 v4 = beehiveLogicTable1[6 * a3[2] + i];
		if (v4 != -1) {
			if (!(a2 + beehiveState[v4]))
				beehiveState[v4] = a2;
		}
	}
}

void BeehiveGame::sub16(int8 a1, int8 a2, int8 *a3, int8 *a4, int8 *a5) {
	int8 params[4];

	params[0] = sub19(a1, a2);
	params[1] = a1;
	params[2] = a2;
	*a3 = params[0];
	sub17(_beehiveState, -1, params, a4, a5);
}

void BeehiveGame::sub17(int8 *beehiveState, int8 a2, int8 *a3, int8 *a4, int8 *a5) {
	beehiveState[a3[2]] = a2;

	if (*a3 == 2)
		beehiveState[a3[1]] = 0;

	*a4 = 0;

	for (int i = 0; i < 6; i++) {
		int8 v6 = beehiveLogicTable1[6 * a3[2] + i];
		if (v6 != -1) {
			if (!(a2 + beehiveState[v6])) {
				beehiveState[v6] = a2;
				a5[(*a4)++] = beehiveLogicTable1[6 * a3[2] + i];
			}
		}
	}
}

void BeehiveGame::selectSourceHexagon(int8 a1, int8 *a2, int8 *a3) {
	*a2 = 0;

	for (int i = 0; i < 6; i++) {
		int8 val = beehiveLogicTable1[6 * a1 + i];
		if (val != -1 && !_beehiveState[val])
			a3[(*a2)++] = val;
	}

	for (int i = 0; i < 12; i++) {
		int val = beehiveLogicTable2[12 * a1 + i];
		if (val != -1 && !_beehiveState[val])
			a3[(*a2)++] = val;
	}
}

int8 BeehiveGame::sub19(int8 a1, int8 a2) {
	for (int i = 0; i < 6; i++)
		if (beehiveLogicTable1[6 * a1 + i] == a2)
			return 1;

	return 2;
}

int8 BeehiveGame::calcMove(int8 *beehiveState, int8 a2, int8 a3, int8 depth, int a5, int8 *params) {
	int8 paramsloc[4];
	int8 params2[3];
	int8 state[64];

	if (!depth)
		return getTotal(beehiveState);

	int8 v7 = -125 * a3;
	int8 v14 = 0;
	int8 v13 = 0;
	int8 v15 = -1;

	if (sub11(beehiveState, &v15, &v14, &v13, a3, a5, params2)) {
		do {
			for (int i = 0; i < HEXCOUNT; i++)
				state[i] = beehiveState[i];

			sub15(state, a3, params2);
			int8 v8 = calcMove(state, v7, -a3, depth - 1, a5, paramsloc);

			if (a3 <= 0) {
				if (v8 < v7) {
					params[0] = params2[0];
					params[1] = params2[1];
					params[2] = params2[2];
					v7 = v8;
				}
				if (a2 >= v7)
					return v7;
			} else {
				if (v8 > v7) {
					params[0] = params2[0];
					params[1] = params2[1];
					params[2] = params2[2];
					v7 = v8;
				}
				if (a2 <= v7)
					return v7;
			}
		} while (sub11(beehiveState, &v15, &v14, &v13, a3, a5, params2));
	}

	if (depth < _maxDepth && -125 * a3 == v7)
		return getTotal(beehiveState);
	else
		return v7;
}

int8 BeehiveGame::getHexDifference() {
	return (getTotal(_beehiveState) >= 0 ? 1 : 0) + 5;
}

int8 BeehiveGame::getTotal(int8 *hexagons) {
	int8 result = 0;

	for (int i = 0; i < HEXCOUNT; i++)
		result += hexagons[i];

	return result;
}

int8 BeehiveGame::findCell(int8 *beehiveState, int8 *pos, int8 key) {
	for (int i = *pos + 1; i < HEXCOUNT; i++) {
		if (beehiveState[i] == key) {
			*pos = i;
			return 1;
		}
	}

	return 0;
}

namespace {

const int8 beehiveLogicTable1[368] = {
	-1,  5,  6,  1, -1, -1,
	 0,  6,  7,  2, -1, -1,
	 1,  7,  8,  3, -1, -1,
	 2,  8,  9,  4, -1, -1,
	 3,  9, 10, -1, -1, -1,
	-1, 11, 12,  6,  0, -1,
	 5, 12, 13,  7,  1,  0,
	 6, 13, 14,  8,  2,  1,
	 7, 14, 15,  9,  3,  2,
	 8, 15, 16, 10,  4,  3,
	 9, 16, 17, -1, -1,  4,
	-1, 18, 19, 12,  5, -1,
	11, 19, 20, 13,  6,  5,
	12, 20, 21, 14,  7,  6,
	13, 21, 22, 15,  8,  7,
	14, 22, 23, 16,  9,  8,
	15, 23, 24, 17, 10,  9,
	16, 24, 25, -1, -1, 10,
	-1, 26, 27, 19, 11, -1,
	18, 27, 28, 20, 12, 11,
	19, 28, 29, 21, 13, 12,
	20, 29, 30, 22, 14, 13,
	21, 30, 31, 23, 15, 14,
	22, 31, 32, 24, 16, 15,
	23, 32, 33, 25, 17, 16,
	24, 33, 34, -1, -1, 17,
	-1, -1, 35, 27, 18, -1,
	26, 35, 36, 28, 19, 18,
	27, 36, 37, 29, 20, 19,
	28, 37, 38, 30, 21, 20,
	29, 38, 39, 31, 22, 21,
	30, 39, 40, 32, 23, 22,
	31, 40, 41, 33, 24, 23,
	32, 41, 42, 34, 25, 24,
	33, 42, -1, -1, -1, 25,
	-1, -1, 43, 36, 27, 26,
	35, 43, 44, 37, 28, 27,
	36, 44, 45, 38, 29, 28,
	37, 45, 46, 39, 30, 29,
	38, 46, 47, 40, 31, 30,
	39, 47, 48, 41, 32, 31,
	40, 48, 49, 42, 33, 32,
	41, 49, -1, -1, 34, 33,
	-1, -1, 50, 44, 36, 35,
	43, 50, 51, 45, 37, 36,
	44, 51, 52, 46, 38, 37,
	45, 52, 53, 47, 39, 38,
	46, 53, 54, 48, 40, 39,
	47, 54, 55, 49, 41, 40,
	48, 55, -1, -1, 42, 41,
	-1, -1, 56, 51, 44, 43,
	50, 56, 57, 52, 45, 44,
	51, 57, 58, 53, 46, 45,
	52, 58, 59, 54, 47, 46,
	53, 59, 60, 55, 48, 47,
	54, 60, -1, -1, 49, 48,
	-1, -1, -1, 57, 51, 50,
	56, -1, -1, 58, 52, 51,
	57, -1, -1, 59, 53, 52,
	58, -1, -1, 60, 54, 53,
	59, -1, -1, -1, 55, 54,
	 0,  0
};

const int8 beehiveLogicTable2[800] = {
	-1, -1, 11, 12, 13,  7,  2, -1, -1, -1, -1, -1,
	-1,  5, 12, 13, 14,  8,  3, -1, -1, -1, -1, -1,
	 0,  6, 13, 14, 15,  9,  4, -1, -1, -1, -1, -1,
	 1,  7, 14, 15, 16, 10, -1, -1, -1, -1, -1, -1,
	 2,  8, 15, 16, 17, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, 18, 19, 20, 13,  7,  1, -1, -1, -1, -1,
	-1, 11, 19, 20, 21, 14,  8,  2, -1, -1, -1, -1,
	 5, 12, 20, 21, 22, 15,  9,  3, -1, -1, -1,  0,
	 6, 13, 21, 22, 23, 16, 10,  4, -1, -1, -1,  1,
	 7, 14, 22, 23, 24, 17, -1, -1, -1, -1, -1,  2,
	 8, 15, 23, 24, 25, -1, -1, -1, -1, -1, -1,  3,
	-1, -1, 26, 27, 28, 20, 13,  6,  0, -1, -1, -1,
	-1, 18, 27, 28, 29, 21, 14,  7,  1,  0, -1, -1,
	11, 19, 28, 29, 30, 22, 15,  8,  2,  1,  0,  5,
	12, 20, 29, 30, 31, 23, 16,  9,  3,  2,  1,  6,
	13, 21, 30, 31, 32, 24, 17, 10,  4,  3,  2,  7,
	14, 22, 31, 32, 33, 25, -1, -1, -1,  4,  3,  8,
	15, 23, 32, 33, 34, -1, -1, -1, -1, -1,  4,  9,
	-1, -1, -1, 35, 36, 28, 20, 12,  5, -1, -1, -1,
	-1, 26, 35, 36, 37, 29, 21, 13,  6,  5, -1, -1,
	18, 27, 36, 37, 38, 30, 22, 14,  7,  6,  5, 11,
	19, 28, 37, 38, 39, 31, 23, 15,  8,  7,  6, 12,
	20, 29, 38, 39, 40, 32, 24, 16,  9,  8,  7, 13,
	21, 30, 39, 40, 41, 33, 25, 17, 10,  9,  8, 14,
	22, 31, 40, 41, 42, 34, -1, -1, -1, 10,  9, 15,
	23, 32, 41, 42, -1, -1, -1, -1, -1, -1, 10, 16,
	-1, -1, -1, -1, 43, 36, 28, 19, 11, -1, -1, -1,
	-1, -1, -1, 43, 44, 37, 29, 20, 12, 11, -1, -1,
	26, 35, 43, 44, 45, 38, 30, 21, 13, 12, 11, 18,
	27, 36, 44, 45, 46, 39, 31, 22, 14, 13, 12, 19,
	28, 37, 45, 46, 47, 40, 32, 23, 15, 14, 13, 20,
	29, 38, 46, 47, 48, 41, 33, 24, 16, 15, 14, 21,
	30, 39, 47, 48, 49, 42, 34, 25, 17, 16, 15, 22,
	31, 40, 48, 49, -1, -1, -1, -1, -1, 17, 16, 23,
	32, 41, 49, -1, -1, -1, -1, -1, -1, -1, 17, 24,
	-1, -1, -1, -1, 50, 44, 37, 28, 19, 18, -1, -1,
	-1, -1, -1, 50, 51, 45, 38, 29, 20, 19, 18, 26,
	35, 43, 50, 51, 52, 46, 39, 30, 21, 20, 19, 27,
	36, 44, 51, 52, 53, 47, 40, 31, 22, 21, 20, 28,
	37, 45, 52, 53, 54, 48, 41, 32, 23, 22, 21, 29,
	38, 46, 53, 54, 55, 49, 42, 33, 24, 23, 22, 30,
	39, 47, 54, 55, -1, -1, -1, 34, 25, 24, 23, 31,
	40, 48, 55, -1, -1, -1, -1, -1, -1, 25, 24, 32,
	-1, -1, -1, -1, 56, 51, 45, 37, 28, 27, 26, -1,
	-1, -1, -1, 56, 57, 52, 46, 38, 29, 28, 27, 35,
	43, 50, 56, 57, 58, 53, 47, 39, 30, 29, 28, 36,
	44, 51, 57, 58, 59, 54, 48, 40, 31, 30, 29, 37,
	45, 52, 58, 59, 60, 55, 49, 41, 32, 31, 30, 38,
	46, 53, 59, 60, -1, -1, -1, 42, 33, 32, 31, 39,
	47, 54, 60, -1, -1, -1, -1, -1, 34, 33, 32, 40,
	-1, -1, -1, -1, -1, 57, 52, 45, 37, 36, 35, -1,
	-1, -1, -1, -1, -1, 58, 53, 46, 38, 37, 36, 43,
	50, 56, -1, -1, -1, 59, 54, 47, 39, 38, 37, 44,
	51, 57, -1, -1, -1, 60, 55, 48, 40, 39, 38, 45,
	52, 58, -1, -1, -1, -1, -1, 49, 41, 40, 39, 46,
	53, 59, -1, -1, -1, -1, -1, -1, 42, 41, 40, 47,
	-1, -1, -1, -1, -1, -1, 58, 52, 45, 44, 43, -1,
	-1, -1, -1, -1, -1, -1, 59, 53, 46, 45, 44, 50,
	56, -1, -1, -1, -1, -1, 60, 54, 47, 46, 45, 51,
	57, -1, -1, -1, -1, -1, -1, 55, 48, 47, 46, 52,
	58, -1, -1, -1, -1, -1, -1, -1, 49, 48, 47, 53,
	 0,  0,  0,  0, 26, 18, 35, 11, 27, 43,  5, 19,
	36, 50,  0, 12, 28, 44, 56,  6, 20, 37, 51,  1,
	13, 29, 45, 57,  7, 21, 38, 52,  2, 14, 30, 46,
	58,  8, 22, 39, 53,  3, 15, 31, 47, 59,  9, 23,
	40, 54,  4, 16, 32, 48, 60, 10, 24, 41, 55, 17,
	33, 49, 25, 42, 34,  0,  0,  0
};

} // End of anonymous namespace

void BeehiveGame::testGame(Common::Array<int> moves, bool playerWin) {
	byte vars[1024];
	memset(vars, 0, sizeof(vars));

	int8 &hexDifference = ((int8 *)vars)[13];
	byte &op = vars[14];// can't do the -1 with a reference
	byte &counter = vars[16];

	op = 1;
	run(vars);
	op = 2;
	run(vars);

	for (uint i = 0; i < moves.size(); i += 2) {
		int from = moves[i];
		int to = moves[i + 1];

		op = 3;
		vars[0] = from / 10;
		vars[1] = from % 10;
		run(vars);

		op = 4;
		vars[0] = from / 10;
		vars[1] = from % 10;
		vars[2] = to / 10;
		vars[3] = to % 10;
		run(vars);

		while (counter) {
			op = 6;
			run(vars);
		}
		op = 6;
		run(vars);

		if (i + 2 < moves.size() && hexDifference == 6) {
			error("early Stauf win");
		} else if (i + 2 < moves.size() && hexDifference == 5) {
			error("early player win");
		}

		op = 5;
		run(vars);

		while (counter) {
			op = 6;
			run(vars);
		}
		op = 6;
		run(vars);

		op = 2;
		run(vars);

		if (i + 2 < moves.size() && hexDifference == 6) {
			error("early Stauf win");
		} else if (i + 2 < moves.size() && hexDifference == 5) {
			error("early player win");
		}
	}

	if (playerWin && hexDifference != 5)
		error("player didn't win");
	if (playerWin == false && hexDifference != 6)
		error("Stauf didn't win");
}

void BeehiveGame::tests() {
	warning("starting BeehiveGame::tests()");
	// 8 moves per line, in from and to pairs
	
	// speedrun strat
	testGame({
		/**/ 34, 42, /**/ 56, 50, /**/ 50, 35, /**/ 42, 55, /**/ 34, 42, /**/ 42, 49, /**/ 35, 43, /**/ 43, 50,
		/**/ 50, 51, /**/ 51, 52, /**/ 52, 53, /**/ 53, 54, /**/ 52, 57, /**/ 52, 46, /**/ 34, 25, /**/ 34, 24,
		/**/ 25, 23, /**/ 46, 31, /**/ 31, 30, /**/ 52, 38, /**/ 29, 12, /**/ 31, 39, /**/ 35, 28, /**/ 49, 32,
		/**/ 31, 40, /**/ 39, 47, /**/ 20, 19, /**/ 29, 37, /**/ 57, 58, /**/ 53, 46, /**/ 53, 52
	}, true);

	// losing game
	testGame({
		/**/ 34, 25, /**/ 25, 10, /**/ 34, 17, /**/ 0, 2, /**/ 56, 57, /**/ 57, 51, /**/ 51, 50, /**/ 51, 52,
		/**/ 51, 44, /**/ 50, 43, /**/ 50, 35, /**/ 36, 38, /**/ 35, 37, /**/ 38, 39, /**/ 38, 29, /**/ 45, 58,
		/**/ 58, 59, /**/ 57, 45, /**/ 44, 35, /**/ 35, 26, /**/ 46, 54, /**/ 59, 60, /**/ 59, 55, /**/ 55, 40,
		/**/ 39, 23
	}, false);

	// copy the moveset from one of the tests to play it out yourself
	overrideMoves = {};
	overrideIndex = 0;

	warning("finished BeehiveGame::tests()");
}

} // End of Groovie namespace
