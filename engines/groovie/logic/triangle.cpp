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
#include "groovie/logic/triangle.h"

namespace Groovie {

namespace {
extern const int8 triangleLookup1[12];
extern const int8 triangleLookup2[12];
extern const int8 triangleLookup3[12];
extern const int8 triangleLogicTable[924];
}

TriangleGame::TriangleGame() : _random("TriangleGame") {
	init();
#if 0
	test();
#endif
}

void TriangleGame::run(byte *scriptVariables) {
	byte op = scriptVariables[3];
	uint8 move;

	switch (op) {
	case 3:
		init();
		scriptVariables[3] = 0;
		return;

	case 4:
		// Samantha AI
		move = sub03(2);
		break;

	case 5:
		// Stauf AI (only called after Samantha)
		move = sub03(1);
		break;

	default:
		// Player and then Stauf
		debugC(kDebugLogic, "player chose spot %d", (int)(scriptVariables[1]) + (10 * (int)scriptVariables[0]));
		setCell(scriptVariables[1] + 10 * scriptVariables[0], 2);
		scriptVariables[3] = sub02();

		if (scriptVariables[3] == 0) {
			move = sub03(1);
		} else {
			debugC(kDebugLogic, "winner: %d", (int)scriptVariables[3]);
			return;
		}
	}

	scriptVariables[0] = move / 10;
	scriptVariables[1] = move % 10;
	scriptVariables[3] = sub02();
	debugC(kDebugLogic, "stauf chose spot %d, winner: %d", (int)move, (int)scriptVariables[3]);
}

void TriangleGame::init() {
	debugC(kDebugLogic, "TriangleGame::init(), seed: %u", _random.getSeed());
	_triangleCellCount = 0;
	memset(_triangleCells, 0, 66);
}

int8 TriangleGame::sub02() {
	int8 v6[132];
	int8 v7[68];

	sub05(_triangleCells, v6, v7);

	for (int i = 0; v6[i] != 66; i++) {
		bool v1 = false;
		bool v2 = false;
		bool pl = false;

		// There could be several sections, each one
		// ends with 66. And the overall list ends with 66 too
		// Hence, two loops
		for (; v6[i] != 66; i++) {
			if (!triangleLogicTable[14 * v6[i] + 6])
				pl = true;
			if (!triangleLogicTable[14 * v6[i] + 7])
				v2 = true;
			if (!triangleLogicTable[14 * v6[i] + 8])
				v1 = true;
		}

		if (pl && v2 && v1)
			return _triangleCells[v6[i - 1]];
	}

	return 0;
}

int8 TriangleGame::sub03(int8 player) {
	int8 pickedMoves[4];
	int8 tempTriangle1[68];
	int8 tempTriangle2[68];
	int8 a6a[132];
	int8 tempTriangle3[68];
	int8 tempMoves[132];
	int8 pos;

	if (_triangleCellCount >= 2) {
		sub05(_triangleCells, tempMoves, tempTriangle3);
		sub07(tempMoves, _triangleCells, tempTriangle3, tempTriangle2, tempTriangle1, a6a);

		// Find move until valid one
		(pos = sub09(player, tempTriangle2, tempTriangle1, a6a, _triangleCells)) != 66 ||
		(pos = sub10(player, tempTriangle1, _triangleCells)) != 66 ||
		(pos = sub12(player, a6a, _triangleCells, tempTriangle1)) != 66 ||
		(pos = sub09(3 - player, tempTriangle2, tempTriangle1, a6a, _triangleCells));

		if (pos == 66) {
			pos = _random.getRandomNumber(65);

			int8 oldPos = pos;
			while (_triangleCells[pos]) {
				if (++pos > 65)
					pos = 0;
				if (oldPos == pos) {
					pos = 66;
					break;
				}
			}
		}
	} else {
		int8 max = 0;
		if (!_triangleCells[24]) {
			pickedMoves[0] = 24;
			max = 1;
		}

		if (!_triangleCells[31])
			pickedMoves[max++] = 31;
		if (!_triangleCells[32])
			pickedMoves[max++] = 32;
		if (max)
			pos = pickedMoves[_random.getRandomNumber(max - 1)];
		else {
			warning("TriangleGame: Undefined behaviour");
			pos = tempMoves[0]; // This is uninitalized in this branch
		}
	}

	if (pos != 66)
		setCell(pos, player);

	return pos;
}

void TriangleGame::sub05(int8 *triangleCells, int8 *tempMoves, int8 *tempTriangle) {
	int8 dest[8];

	for (int i = 0; i < 66; i++)
		tempTriangle[i] = triangleCells[i];

	int v16 = 3;

	for (int j = 0; j < 66; ++j) {
		if (triangleCells[j]) {
			bool flag = false;

			copyLogicRow(j, triangleCells[j], dest);
			for (int8 *k = dest; *k != 66; k++) {
				if (j > *k) {
					if (flag) {
						if (tempTriangle[j] != tempTriangle[*k])
							replaceCells(tempTriangle, j, tempTriangle[j], tempTriangle[*k]);
					} else {
						flag = true;
						tempTriangle[j] = tempTriangle[*k];
					}
				}
			}
			if (!flag)
				tempTriangle[j] = v16++;
		}
	}

	int v11 = 0;

	if (v16 > 3) {
		for (int v12 = 3; v12 < v16; v12++) {
			int v14 = v11;

			for (int m = 0; m < 66; m++) {
				if (tempTriangle[m] == v12)
					tempMoves[v11++] = m;
			}

			if (v11 != v14)
				tempMoves[v11++] = 66;
		}
	}

	tempMoves[v11] = 66;
}

void TriangleGame::sub07(int8 *tempMoves, int8 *triangleCells, int8 *tempTriangle3, int8 *tempTriangle2, int8 *tempTriangle1, int8 *tempMoves2) {
	int8 singleRow[8];
	int8 tempTriangle[68];
	int8 routes[4356];

	for (int i = 0; i < 66; i++) {
		tempTriangle2[i] = tempTriangle3[i];
		tempTriangle1[i] = 0;
	}

	if (*tempMoves != 66) {
		for (int8 *ptr1 = tempMoves; *ptr1 != 66; ptr1++) {
			int8 *v45 = &routes[66 * tempTriangle3[*ptr1]];
			*v45 = 66;

			while (*ptr1 != 66) {
				copyLogicRow(*ptr1++, 0, singleRow);
				collapseLoops(v45, singleRow);
			}
		}

		for (int8 *ptr2 = tempMoves; *ptr2 != 66; ptr2++) {
			int8 *j;

			for (j = ptr2; *j != 66; j++)
				;
			for (int8 *k = j + 1; *k != 66; k++) {
				if (triangleCells[*k] == triangleCells[*ptr2]) {
					int8 val1 = tempTriangle2[*k];
					int8 val2 = tempTriangle2[*ptr2];

					if (val1 != val2) {
						int8 lookupLen = copyLookup(&routes[66 * val2], &routes[66 * val1], tempTriangle);
						if (lookupLen == 1) {
							if (triangleCells[*ptr2] == 1) {
								tempTriangle1[tempTriangle[0]] |= 1u;
							} else if (triangleCells[*ptr2] == 2) {
								tempTriangle1[tempTriangle[0]] |= 0x40u;
							}
						} else if (lookupLen > 1) {
							int8 v16 = lookupLen - 1;
							do {
								if (triangleCells[*ptr2] == 1) {
									tempTriangle1[tempTriangle[v16]] |= 0x10u;
								} else if (triangleCells[*ptr2] == 2) {
									tempTriangle1[tempTriangle[v16]] |= 0x20u;
								}
							} while (v16--);

							collapseLoops(&routes[66 * tempTriangle2[*k]], &routes[66 * tempTriangle2[*ptr2]]);
							int8 from = tempTriangle2[*ptr2];
							int8 to = tempTriangle2[*k];
							for (int m = 0; m < 66; m++) {
								if (tempTriangle2[m] == from)
									tempTriangle2[m] = to;
							}
						}
					}
				}
				for (; *k != 66; k++)
					;
			}
			for (; *ptr2 != 66; ptr2++)
				;
		}
	}

	int8 maxVal = 0;
	for (int i = 0; i < 66; i++)  {
		if (tempTriangle2[i] > maxVal)
			maxVal = tempTriangle2[i];
	}

	int8 len2 = 0;

	for (int i = 3; i <= maxVal; i++) {
		int8 prevLen2 = len2;
		for (int j = 0; j < 66; j++) {
			if (tempTriangle2[j] == i)
				tempMoves2[len2++] = j;
		}

		if (prevLen2 != len2)
			tempMoves2[len2++] = 66;
	}

	tempMoves2[len2] = 66;

	for (int8 *ptr3 = tempMoves2; *ptr3 != 66; ptr3++) {
		bool flag1 = false, flag2 = false, flag3 = false;
		int8 row = tempTriangle2[*ptr3];
		byte mask1 = 0, mask2 = 0;

		for (int i = 0; i < 66; i++) {
			if (tempTriangle2[i] == row && triangleCells[i]) {
				if (triangleCells[i] == 1) {
					mask1 = 1;
					mask2 = 16;
				} else {
					mask1 = 64;
					mask2 = 32;
				}
				break;
			}
		}

		while (*ptr3 != 66) {
			if (!triangleLogicTable[14 * *ptr3 + 6])
				flag1 = 1;
			if (!triangleLogicTable[14 * *ptr3 + 7])
				flag2 = 1;
			if (!triangleLogicTable[14 * *ptr3 + 8])
				flag3 = 1;
			++ptr3;
		}

		if (!flag1) {
			int8 lookup1 = copyLookup(triangleLookup1, &routes[66 * row], tempTriangle);
			if (lookup1 == 1) {
				tempTriangle1[tempTriangle[0]] |= mask1;
			} else if (lookup1 > 1) {
				int k = lookup1 - 1;
				do
					tempTriangle1[tempTriangle[k]] |= mask2;
				while (k--);
				flag1 = 1;
			}
		}
		if (!flag2) {
			int8 lookup2 = copyLookup(triangleLookup2, &routes[66 * row], tempTriangle);
			if (lookup2 == 1) {
				tempTriangle1[tempTriangle[0]] |= mask1;
			} else if (lookup2 > 1) {
				int k = lookup2 - 1;
				do
					tempTriangle1[tempTriangle[k]] |= mask2;
				while (k--);
				flag2 = 1;
			}
		}
		if (!flag3) {
			int8 lookup3 = copyLookup(triangleLookup3, &routes[66 * row], tempTriangle);
			if (lookup3 == 1) {
				tempTriangle1[tempTriangle[0]] |= mask1;
			} else if (lookup3 > 1) {
				int k = lookup3 - 1;
				do
					tempTriangle1[tempTriangle[k]] |= mask2;
				while (k--);
				flag3 = 1;
			}
		}

		byte mask3 = 0;

		if (flag1)
			mask3 = 4;
		if (flag2)
			mask3 |= 8u;
		if (flag3)
			mask3 |= 2u;

		for (int i = 0; i < 66; i++) {
			if (tempTriangle2[i] == row)
			tempTriangle1[i] |= mask3;
		}
	}
}

int8 TriangleGame::sub09(int8 player, int8 *tempTriangle2, int8 *tempTriangle1, int8 *a4, int8 *triangleCells) {
	int8 movesTable[280];
	
	int numDir1 = 0;
	int numDir2 = 0;
	int numDir3 = 0;
	int numDir4 = 0;
	int row = 0;

	for (const int8 *tPtr = &triangleLogicTable[6]; tPtr < &triangleLogicTable[924]; tPtr += 14, row++) {
		if (!triangleCells[row] && (tempTriangle1[row] & (player == 1 ? 1 : 64)) != 0) {
			int c1 = 0, c2 = 0, c3 = 0;
			int mask = 0;

			copyLogicRow(row, player, movesTable);

			for (int8 *movPtr = movesTable; *movPtr != 66; ++movPtr) {
				int row2 = 0;

				mask |= tempTriangle1[*movPtr];

				for (const int8 *tPtr2 = &triangleLogicTable[6]; tPtr2 < &triangleLogicTable[924]; tPtr2 += 14, row2++) {
					if (tempTriangle2[row2] == tempTriangle2[*movPtr]) {
						c1 += (tPtr2[0] == 0) ? 1 : 0;
						c2 += (tPtr2[1] == 0) ? 1 : 0;
						c3 += (tPtr2[2] == 0) ? 1 : 0;
					}
				}
			}

			if (c1)
				mask &= ~4u;
			if (c2)
				mask &= ~8u;
			if (c3)
				mask &= ~2u;

			if (tPtr[0] || c1) {
				if (tPtr[1] || c2) {
					if (tPtr[2] || c3) {
						if (mask) {
							if (mask == 0xe) {
								movesTable[numDir2 + 76] = row;
								numDir2++;
							} else if (mask == 2 || mask == 8 || mask == 4) {
								movesTable[numDir4 + 212] = row;
								numDir4++;
							} else {
								movesTable[numDir3 + 144] = row;
								numDir3++;
							}
						}
					} else {
						movesTable[numDir1 + 8] = row;
						numDir1++;
					}
				} else {
					movesTable[numDir1 + 8] = row;
					numDir1++;
				}
			} else {
				movesTable[numDir1 + 8] = row;
				numDir1++;
			}
		}
	}

	if (numDir1)
		return movesTable[_random.getRandomNumber(numDir1 - 1) + 8];
	if (numDir2)
		return movesTable[_random.getRandomNumber(numDir2 - 1) + 76];
	if (numDir3)
		return movesTable[_random.getRandomNumber(numDir3 - 1) + 144];
	if (numDir4)
		return movesTable[_random.getRandomNumber(numDir4 - 1) + 212];

	return 66;
}

int8 TriangleGame::sub10(int8 player, int8 *a2, int8 *triangleCells) {
	int8 *destPtr;
	byte mask;
	int counter;
	int8 dest1[8];
	int8 dest2[68];

	mask = 0;
	counter = 0;

	if (player == 1)
		mask = 16;
	else if (player == 2)
		mask = 32;

	for (int i = 0; i < 66; ++i) {
		if (!triangleCells[i] && (mask & (byte)a2[i]) != 0) {
			copyLogicRow(i, player, dest1);

			destPtr = dest1;

			while (*destPtr != 66) {
				if ((a2[*destPtr] & 0xE) == 0xE) {
					dest2[counter] = i;
					counter++;
					break;
				}

				destPtr++;
			}
		}
	}

	if (counter)
		return dest2[_random.getRandomNumber(counter - 1)];

	return 66;
}

int8 TriangleGame::sub12(int8 player, int8 *tempMoves, int8 *triangleCells, int8 *tempTriangle) {
	int8 moves[8];
	int8 tempTriangle1[68];
	int8 tempTriangle2[68];
	int8 tempTriangle3[68];
	int8 tempTriangle4[68];

	int8 result = 66;
	int maxlen = -1;
	int8 *startPtr = triangleCells;

	for (int8 *ptr = tempMoves; *ptr != 66; ptr++) {
		int len = 0;
		int8 *beg = ptr;
		int8 p0 = *ptr;

		for (; *ptr != 66; ++ptr)
			++len;

		if (len > maxlen && triangleCells[p0] == player) {
			maxlen = len;
			startPtr = beg;
		}
	}

	tempTriangle4[0] = 66;

	for (int8 *ptr = startPtr; *ptr != 66; ++ptr) {
		if (sub13(*ptr, triangleCells, moves))
			collapseLoops(tempTriangle4, moves);
	}

	int len1 = 0, len2 = 0, len3 = 0;

	tempTriangle1[0] = 66;
	tempTriangle2[0] = 66;
	tempTriangle3[0] = 66;

	for (int8 *ptr = tempTriangle4; *ptr != 66; ++ptr) {
		int8 v13 = 100;

		int8 v15 = triangleLogicTable[14 * *ptr + 11];
		if (v15 < 100)
			v13 = triangleLogicTable[14 * *ptr + 11];

		int8 v16 = triangleLogicTable[14 * *ptr + 13];
		if (v13 > v16)
			v13 = triangleLogicTable[14 * *ptr + 13];

		int8 v17 = triangleLogicTable[14 * *ptr + 12];
		if (v13 > v17)
			v13 = triangleLogicTable[14 * *ptr + 12];

		if (v13 == v15) {
			tempTriangle1[len1++] = *ptr;
		} else if (v13 == v17) {
			tempTriangle2[len2++] = *ptr;
		} else if (v13 == v16) {
			tempTriangle3[len3++] = *ptr;
		}
	}

	bool flag1 = false, flag2 = false, flag3 = false;
	tempTriangle3[len3] = 66;
	tempTriangle2[len2] = 66;
	tempTriangle1[len1] = 66;

	int8 startVal = tempTriangle[*startPtr];

	switch (startVal) {
	case 8:
		flag3 = true;
		break;
	case 4:
		flag2 = true;
		break;
	case 2:
		flag1 = true;
		break;
	case 12:
		flag2 = true;
		flag3 = flag2;
		break;
	case 6:
		flag1 = true;
		flag2 = true;
		break;
	case 10:
		flag1 = true;
		flag3 = true;
		break;
	case 14:
		flag2 = false;
		flag1 = false;
		flag3 = flag2;
		break;
	default:
		flag2 = true;
		flag1 = true;
		flag3 = flag2;
		break;
	}

	int minLen = 101;
	if (flag1) {
		for (int8 *ptr = tempTriangle1; *ptr != 66; ++ptr) {
			int8 part1 = 0;
			if ((startVal & 8) == 0)
				part1 = triangleLogicTable[14 * *ptr + 7];
			int8 part2 = 0;
			if ((startVal & 4) == 0)
				part2 = triangleLogicTable[14 * *ptr + 6];
			if (minLen > part1 + part2) {
				minLen = part1 + part2;
				result = *ptr;
			}
		}
	}
	if (flag2) {
		for (int8 *ptr = tempTriangle2; *ptr != 66; ++ptr) {
			int8 part1 = 0;
			if ((startVal & 8) == 0)
				part1 = triangleLogicTable[14 * *ptr + 7];
			int8 part2 = 0;
			if ((startVal & 2) == 0)
				part2 = triangleLogicTable[14 * *ptr + 8];
			if (minLen > part1 + part2) {
				minLen = part1 + part2;
				result = *ptr;
			}
		}
	}
	if (flag3) {
		for (int8 *ptr = tempTriangle3; *ptr != 66; ++ptr) {
			int8 part1 = 0;
			if ((startVal & 2) == 0)
				part1 = triangleLogicTable[14 * *ptr + 8];
			int8 part2 = 0;
			if ((startVal & 4) == 0)
				part2 = triangleLogicTable[14 * *ptr + 6];
			if (minLen > part1 + part2) {
				minLen = part1 + part2;
				result = *ptr;
			}
		}
	}

	return result;
}

int TriangleGame::sub13(int8 row, int8 *triangleCells, int8 *moves) {
	int pos = 0;

	for (int i = 0; i < 6; i++) {
		int8 v6 = triangleLogicTable[14 * row + i];

		if (v6 != -1 && !triangleCells[v6]) {
			int v7 = (i + 1) % 6;
			int8 v8 = triangleLogicTable[14 * row + v7];

			if (v8 != -1 && !triangleCells[v8]) {
				int8 v9 = triangleLogicTable[14 * v6 + v7];

				if (v9 != -1 && !triangleCells[v9])
					moves[pos++] = v9;
			}
		}
	}

	moves[pos] = 66;

	return pos;
}

void TriangleGame::setCell(int8 cellnum, int8 val) {
	assert(cellnum >= 0);
	assert(cellnum < 66);
	if (cellnum >= 0 && cellnum < 66) {
		++_triangleCellCount;
		assert(_triangleCells[cellnum] == 0);
		_triangleCells[cellnum] = val;
	}
}

void TriangleGame::copyLogicRow(int row, int8 key, int8 *dest) {
	int pos = 0;

	for (int i = 0; i < 6; i++) {
		int8 val = triangleLogicTable[14 * row + i];
		if (val != -1 && _triangleCells[val] == key)
			dest[pos++] = val;
	}

	dest[pos] = 66;
}

void TriangleGame::replaceCells(int8 *tempTriangle, int limit, int8 from, int8 to) {
	for (int i = 0; i <= limit; ++i) {
		if (tempTriangle[i] == from)
			tempTriangle[i] = to;
	}
}

int TriangleGame::copyLookup(const int8 *lookup, int8 *start, int8 *dest){
	int counter = 0;

	if (*lookup == 66) {
		*dest = 66;
		return counter;
	}

	for (; *lookup != 66; lookup++) {
		for (int8 *ptr = start; *ptr != 66; ptr++) {
			if (*ptr == *lookup)
				dest[counter++] = *lookup;
		}
	}

	dest[counter] = 66;

	return counter;
}

void TriangleGame::collapseLoops(int8 *route, int8 *singleRow) {
	int len = 0;
	for (int8 *i = route; *i != 66; i++)
		len++;

	int origlen = len;


	for (int8 *i = singleRow; *i != 66; i++) {
		int j;
		for (j = 0; j < len; j++) {
			if (route[j] == *i)
				break;
		}
		if (j == len)
			route[len++] = *i;
	}

	if (len != origlen)
		route[len] = 66;
}

bool TriangleGame::testGame(uint32 seed, const Common::Array<uint8> moves, bool playerWin) {
	byte vars[1024];
	byte &op = vars[3];
	byte &move10s = vars[0];
	byte &move1s = vars[1];
	byte &winner = vars[3];

	memset(vars, 0, sizeof(vars));

	op = 3;
	run(vars);

	warning("starting TriangleGame::testGame(%u, %u, %d)", seed, moves.size(), (int)playerWin);
	_random.setSeed(seed);

	for (uint i = 0; i < moves.size(); i++) {
		if (i % 2) {
			// check Stauf's move
			uint8 move = ((uint)move10s * 10) + (uint)move1s;
			if (move != moves[i]) {
				error("%u: bad Stauf move: %d", (int)i, (int)move);
				// return false here is useful for finding the right seed to test
				return false;
			}
			continue;
		}

		// else, input player's move
		if (winner != 0)
			error("%u: early winner: %d", (int)i, (int)winner);

		uint8 move = moves[i];
		move10s = move / 10;
		move1s = move % 10;
		op = 0;
		run(vars);
	}

	if (playerWin && winner != 2)
		error("player didn't win, winner: %d", (int)winner);
	if (playerWin == false && winner != 1)
		error("Stauf didn't win, winner: %d", (int)winner);

	warning("finished TriangleGame::testGame(%u, %u, %d)", seed, moves.size(), (int)playerWin);
	return true;
}

void TriangleGame::ensureSamanthaWin(uint32 seed) {
	byte vars[1024];
	byte &op = vars[3];
	byte &winner = vars[3];

	op = 3;
	run(vars);

	warning("starting TriangleGame::ensureSamanthaWin(%u)", seed);
	_random.setSeed(seed);

	for (int i = 0; i < 100; i++) {
		// Samantha
		op = 4;
		run(vars);
		if (winner)
			break;

		// Stauf
		op = 5;
		run(vars);
		if (winner)
			break;
	}

	if (winner != 2)
		error("Samantha didn't win, winner: %d", (int)winner);

	warning("finished TriangleGame::ensureSamanthaWin(%u)", seed);
}

void TriangleGame::testPlayRandomly(uint32 seed) {
	byte vars[1024];
	byte &op = vars[3];
	byte &move10s = vars[0];
	byte &move1s = vars[1];
	byte &winner = vars[3];

	op = 3;
	run(vars);

	warning("starting TriangleGame::testPlayRandomly(%u)", seed);
	_random.setSeed(seed);

	for (int i = 0; i < 100; i++) {
		// Player make random move
		uint8 move = 0;
		do {
			move = _random.getRandomNumber(65);
		} while (_triangleCells[move]);

		move10s = move / 10;
		move1s = move % 10;
		op = 0;
		run(vars);
		if (winner)
			break;

		// Stauf
		op = 5;
		run(vars);
		if (winner)
			break;
	}

	if (winner != 1)
		error("Stauf didn't win, winner: %d", (int)winner);

	warning("finished TriangleGame::testPlayRandomly(%u)", seed);
}

void TriangleGame::test() {
	warning("starting TriangleGame::test");
	uint32 oldSeed = _random.getSeed();

	// Samantha appears to not always win, but she usually does, and she wins these seeds
	// haven't verified if she always wins in the original game
	for (uint32 i = 100; i < 105; i++)
		ensureSamanthaWin(i);

	// Similar thing here, technically there might be a seed where the player wins, but Stauf should win the vast majority of them
	for (uint32 i = 200; i < 205; i++)
		testPlayRandomly(i);

	testGame(1, {24, 32, 30, 42, 37, 53, 45, 39, 19, 47, 20, 56, 55, 59, 36, 49, 29, 46, 23, 38, 18}, true);
	testGame(1, {24, 32, 30, 42, 37, 53, 19, 39, 45, 47, 46, 59, 56, 49, 38, 48, 31, 40, 25, 50, 20}, true);
	testGame(2, {24, 31, 33, 38, 43, 46, 16, 41, 54, 52, 64, 61, 53, 37, 42, 51, 32, 40, 23, 60, 15}, true);
	testGame(2, {24, 31, 33, 38, 43, 46, 16, 41, 53, 52, 64, 61, 54, 37, 34, 50, 25, 36, 17, 0, 10}, true);
	testGame(40680, {0, 24, 1, 12, 2, 4, 3, 5, 6, 30, 7, 9, 8, 29, 10, 13, 11, 47, 14, 18, 20, 37, 19, 36, 27, 57, 26, 31}, false);

	_random.setSeed(oldSeed);
	warning("finished TriangleGame::test");
}

namespace {

const int8 triangleLookup1[12] = {
	0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66
};

const int8 triangleLookup2[12] = {
	0, 2, 5, 9, 14, 20, 27, 35, 44, 54, 65, 66
};

const int8 triangleLookup3[12] = {
	55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66
};

const int8 triangleLogicTable[924] = {
	-1, -1,  2,  1, -1, -1,  0,  0, 10, 10,  6,  0, 10, 10,
	 0,  2,  4,  3, -1, -1,  0,  1,  9,  9,  5,  1,  9,  9,
	-1, -1,  5,  4,  1,  0,  1,  0,  9, 11,  5,  1,  9,  9,
	 1,  4,  7,  6, -1, -1,  0,  2,  8,  8,  4,  2,  8,  8,
	 2,  5,  8,  7,  3,  1,  1,  1,  8, 10,  4,  1,  8,  8,
	-1, -1,  9,  8,  4,  2,  2,  0,  8, 12,  4,  2,  8,  8,
	 3,  7, 11, 10, -1, -1,  0,  3,  7,  7,  3,  3,  7,  7,
	 4,  8, 12, 11,  6,  3,  1,  2,  7,  9,  3,  2,  7,  7,
	 5,  9, 13, 12,  7,  4,  2,  1,  7, 11,  3,  2,  7,  7,
	-1, -1, 14, 13,  8,  5,  3,  0,  7, 13,  3,  3,  7,  7,
	 6, 11, 16, 15, -1, -1,  0,  4,  6,  6,  3,  4,  6,  6,
	 7, 12, 17, 16, 10,  6,  1,  3,  6,  8,  2,  3,  6,  6,
	 8, 13, 18, 17, 11,  7,  2,  2,  6, 10,  2,  2,  6,  6,
	 9, 14, 19, 18, 12,  8,  3,  1,  6, 12,  2,  3,  6,  6,
	-1, -1, 20, 19, 13,  9,  4,  0,  6, 14,  3,  4,  6,  6,
	10, 16, 22, 21, -1, -1,  0,  5,  5,  5,  3,  5,  5,  5,
	11, 17, 23, 22, 15, 10,  1,  4,  5,  7,  2,  4,  5,  5,
	12, 18, 24, 23, 16, 11,  2,  3,  5,  9,  1,  3,  5,  5,
	13, 19, 25, 24, 17, 12,  3,  2,  5, 11,  1,  3,  5,  5,
	14, 20, 26, 25, 18, 13,  4,  1,  5, 13,  2,  4,  5,  5,
	-1, -1, 27, 26, 19, 14,  5,  0,  5, 15,  3,  5,  5,  5,
	15, 22, 29, 28, -1, -1,  0,  6,  4,  4,  3,  6,  6,  4,
	16, 23, 30, 29, 21, 15,  1,  5,  4,  6,  2,  5,  5,  4,
	17, 24, 31, 30, 22, 16,  2,  4,  4,  8,  1,  4,  4,  4,
	18, 25, 32, 31, 23, 17,  3,  3,  4, 10,  0,  3,  4,  4,
	19, 26, 33, 32, 24, 18,  4,  2,  4, 12,  1,  4,  4,  4,
	20, 27, 34, 33, 25, 19,  5,  1,  4, 14,  2,  5,  4,  5,
	-1, -1, 35, 34, 26, 20,  6,  0,  4, 16,  3,  6,  4,  6,
	21, 29, 37, 36, -1, -1,  0,  7,  3,  3,  3,  7,  7,  3,
	22, 30, 38, 37, 28, 21,  1,  6,  3,  5,  2,  6,  6,  3,
	23, 31, 39, 38, 29, 22,  2,  5,  3,  7,  1,  5,  5,  3,
	24, 32, 40, 39, 30, 23,  3,  4,  3,  9,  0,  4,  4,  3,
	25, 33, 41, 40, 31, 24,  4,  3,  3, 11,  0,  4,  3,  4,
	26, 34, 42, 41, 32, 25,  5,  2,  3, 13,  1,  5,  3,  5,
	27, 35, 43, 42, 33, 26,  6,  1,  3, 15,  2,  6,  3,  6,
	-1, -1, 44, 43, 34, 27,  7,  0,  3, 17,  3,  7,  3,  7,
	28, 37, 46, 45, -1, -1,  0,  8,  2,  2,  4,  8,  8,  2,
	29, 38, 47, 46, 36, 28,  1,  7,  2,  4,  3,  7,  7,  2,
	30, 39, 48, 47, 37, 29,  2,  6,  2,  6,  2,  6,  6,  2,
	31, 40, 49, 48, 38, 30,  3,  5,  2,  8,  1,  5,  5,  3,
	32, 41, 50, 49, 39, 31,  4,  4,  2, 10,  1,  4,  4,  4,
	33, 42, 51, 50, 40, 32,  5,  3,  2, 12,  1,  5,  3,  5,
	34, 43, 52, 51, 41, 33,  6,  2,  2, 14,  2,  6,  2,  6,
	35, 44, 53, 52, 42, 34,  7,  1,  2, 16,  3,  7,  2,  7,
	-1, -1, 54, 53, 43, 35,  8,  0,  2, 18,  4,  8,  2,  8,
	36, 46, 56, 55, -1, -1,  0,  9,  1,  1,  5,  9,  9,  1,
	37, 47, 57, 56, 45, 36,  1,  8,  1,  3,  4,  8,  8,  1,
	38, 48, 58, 57, 46, 37,  2,  7,  1,  5,  3,  7,  7,  2,
	39, 49, 59, 58, 47, 38,  3,  6,  1,  7,  2,  6,  6,  3,
	40, 50, 60, 59, 48, 39,  4,  5,  1,  9,  2,  5,  5,  4,
	41, 51, 61, 60, 49, 40,  5,  4,  1, 11,  2,  5,  4,  5,
	42, 52, 62, 61, 50, 41,  6,  3,  1, 13,  2,  6,  3,  6,
	43, 53, 63, 62, 51, 42,  7,  2,  1, 15,  3,  7,  2,  7,
	44, 54, 64, 63, 52, 43,  8,  1,  1, 17,  4,  8,  1,  8,
	-1, -1, 65, 64, 53, 44,  9,  0,  1, 19,  5,  9,  1,  9,
	45, 56, -1, -1, -1, -1,  0, 10,  0,  0,  6, 10, 10,  0,
	46, 57, -1, -1, 55, 45,  1,  9,  0,  2,  5,  9,  9,  1,
	47, 58, -1, -1, 56, 46,  2,  8,  0,  4,  4,  8,  8,  2,
	48, 59, -1, -1, 57, 47,  3,  7,  0,  6,  3,  7,  7,  3,
	49, 60, -1, -1, 58, 48,  4,  6,  0,  8,  3,  6,  6,  4,
	50, 61, -1, -1, 59, 49,  5,  5,  0, 10,  3,  5,  5,  5,
	51, 62, -1, -1, 60, 50,  6,  4,  0, 12,  3,  6,  4,  6,
	52, 63, -1, -1, 61, 51,  7,  3,  0, 14,  3,  7,  3,  7,
	53, 64, -1, -1, 62, 52,  8,  2,  0, 16,  4,  8,  2,  8,
	54, 65, -1, -1, 63, 53,  9,  1,  0, 18,  5,  9,  1,  9,
	-1, -1, -1, -1, 64, 54, 10,  0,  0, 20,  6, 10,  0, 10
};

} // End of anonymous namespace


} // End of Groovie namespace
