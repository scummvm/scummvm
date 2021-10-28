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

#include "groovie/logic/pente.h"
#include "common/stack.h"
#include "groovie/groovie.h"
#include <limits.h>

namespace Groovie {

const uint WIN_SCORE = 100000000;
const uint CAPTURE_SCORE = 1000000;

struct pentePlayerTable {
	Common::FixedStack<int, 813> lines;
};

struct penteTable {
	pentePlayerTable player;
	pentePlayerTable stauf;
	uint playerScore;
	uint staufScore;
	byte playerLines;
	byte staufLines;
	byte width;
	byte height;
	uint16 boardSize;
	byte lineLength;
	uint16 moveCounter;
	byte boardState[20][15];
	uint16 linesCounter;
	uint16 linesTable[20][15][21];
	byte numAdjacentPieces[20][15];
	byte calcTouchingPieces; // the deepest level of AI recursion sets this to 0, and then sets it back to 1 when returning
};

void PenteGame::penteSub02Frees(penteTable *table) {
	delete table;
}

void PenteGame::penteSub05BuildLookupTable(penteTable *table) {
	uint16 *puVar1;
	uint _width;
	uint uVar4;
	uint uVar5;
	byte bVar6;
	byte local_14;
	byte local_13;
	uint16 lines_counter;
	byte height;
	byte line_length;
	byte width;

	width = table->width;
	height = table->height;
	bVar6 = 0;
	lines_counter = 0;
	line_length = table->lineLength;
	_width = (uint)width;
	
	auto &lines_table = table->linesTable;
	local_14 = 0;
	if (height != 0) {
		do {
			local_13 = 0;
			if (-1 < (int)(_width - line_length)) {
				do {
					bVar6 = 0;
					if (line_length != 0) {
						do {
							uVar4 = (uint)bVar6;
							bVar6 += 1;
							puVar1 = lines_table[uVar4 + local_13][local_14];
							*puVar1 = *puVar1 + 1;
							puVar1 = lines_table[uVar4 + local_13][local_14];
							puVar1[*puVar1] = lines_counter;
						} while (bVar6 < line_length);
					}
					lines_counter += 1;
					local_13 += 1;
				} while ((int)(uint)local_13 <= (int)(_width - line_length));
			}
			local_14 += 1;
		} while (local_14 < height);
	}
	local_13 = 0;
	if (width != 0) {
		do {
			local_14 = 0;
			if (-1 < (int)((uint)height - (uint)line_length)) {
				do {
					if (line_length != 0) {
						uVar4 = 0;
						do {
							bVar6 = (char)uVar4 + 1;
							puVar1 = lines_table[local_13][uVar4 + local_14];
							*puVar1 = *puVar1 + 1;
							puVar1 = lines_table[local_13][uVar4 + local_14];
							puVar1[*puVar1] = lines_counter;
							uVar4 = (uint)bVar6;
						} while (bVar6 < line_length);
					}
					lines_counter += 1;
					local_14 += 1;
				} while ((int)(uint)local_14 <= (int)((uint)height - (uint)line_length));
			}
			local_13 += 1;
		} while (local_13 < width);
	}
	local_14 = 0;
	uVar4 = (uint)line_length;
	if (-1 < (int)(height - uVar4)) {
		do {
			local_13 = 0;
			if (-1 < (int)(_width - uVar4)) {
				do {
					if (line_length != 0) {
						uVar5 = 0;
						do {
							width = (char)uVar5 + 1;
							puVar1 = lines_table[local_13 + uVar5][uVar5 + local_14];
							*puVar1 = *puVar1 + 1;
							puVar1 = lines_table[local_13 + uVar5][uVar5 + local_14];
							puVar1[*puVar1] = lines_counter;
							uVar5 = (uint)width;
						} while (width < line_length);
					}
					lines_counter += 1;
					local_13 += 1;
				} while ((int)(uint)local_13 <= (int)(_width - uVar4));
			}
			local_14 += 1;
		} while ((int)(uint)local_14 <= (int)(height - uVar4));
	}
	local_14 = line_length - 1;
	if (local_14 < height) {
		do {
			local_13 = 0;
			if (-1 < (int)(_width - uVar4)) {
				do {
					if (line_length != 0) {
						uVar5 = 0;
						do {
							width = (char)uVar5 + 1;
							puVar1 = lines_table[local_13 + uVar5][local_14 - uVar5];
							*puVar1 = *puVar1 + 1;
							puVar1 = lines_table[local_13 + uVar5][local_14 - uVar5];
							puVar1[*puVar1] = lines_counter;
							uVar5 = (uint)width;
						} while (width < line_length);
					}
					lines_counter += 1;
					local_13 += 1;
				} while ((int)(uint)local_13 <= (int)(_width - uVar4));
			}
			local_14 += 1;
		} while (local_14 < height);
	}
	table->linesCounter = lines_counter;
}

penteTable *PenteGame::penteSub01Init(byte width, byte height, byte length) {
	penteTable *table;
	byte bVar5;
	uint16 _height;

	table = new penteTable();
	table->width = width;
	table->height = height;
	_height = (uint16)height;
	table->boardSize = _height * width;
	bVar5 = 0;
	table->lineLength = length;
	memset(table->boardState, 0, sizeof(table->boardState));
	
	penteSub05BuildLookupTable(table);
	assert(table->linesCounter == 812);
	
	bVar5 = 0;
	table->staufScore = (uint)table->linesCounter;
	table->playerScore = (uint)table->linesCounter;
	memset(table->numAdjacentPieces, 0, sizeof(table->numAdjacentPieces));
	
	table->calcTouchingPieces = 1;
	return table;
}

uint &getPlayerTable(penteTable *table, bool staufTurn, pentePlayerTable *&pt) {
	pt = staufTurn ? &table->stauf : &table->player;
	return staufTurn ? table->staufScore : table->playerScore;
}

void penteScoringLine(penteTable *table, uint16 lineIndex, bool stauf_turn, bool revert) {
	pentePlayerTable *playerTable;
	uint &score = getPlayerTable(table, stauf_turn, playerTable);

	int lineLength, mult;
	if (revert) {
		lineLength = --playerTable->lines[lineIndex];
		mult = -1;
	} else {
		lineLength = playerTable->lines[lineIndex]++;
		mult = 1;
	}

	if (table->lineLength - lineLength == 1) {
		score = (int)score + (int)WIN_SCORE * mult;
	} else {
		pentePlayerTable *opponentTable;
		uint &opponentScore = getPlayerTable(table, !stauf_turn, opponentTable);
		int opponentLineLength = opponentTable->lines[lineIndex];
		if (lineLength == 0) {
			opponentScore += (-(1 << ((byte)opponentLineLength & 0x1f))) * mult;
			if (table->lineLength - opponentLineLength == 1) {
				if (stauf_turn)
					table->playerLines -= mult;
				else
					table->staufLines -= mult;
			}
		}
		if (opponentLineLength == 0) {
			score += (1 << ((byte)lineLength & 0x1f)) * mult;
			if (table->lineLength - lineLength == 2) {
				byte b;
				if (stauf_turn)
					b = (table->staufLines += mult);
				else
					b = (table->playerLines += mult);

				if (revert)
					b -= mult;

				if (1 < b) {
					score = (int)score + (int)CAPTURE_SCORE * mult;
				}
			}
		}
	}
}

void penteTouchingPieces(penteTable *table, byte moveX, byte moveY, bool revert) {
	byte endX, endY;

	if (moveX + 1 < table->width) {
		endX = moveX + 1;
	} else {
		endX = table->width - 1;
	}

	if (moveY + 1 < table->height) {
		endY = moveY + 1;
	} else {
		endY = table->height - 1;
	}

	byte x = 0;
	if (1 < moveX) {
		x = moveX - 1;
	}

	for (; x <= endX; x++) {
		byte y = 0;
		if (1 < moveY) {
			y = moveY - 1;
		}

		for (; y <= endY; y++) {
			if (revert)
				table->numAdjacentPieces[x][y]--;
			else
				table->numAdjacentPieces[x][y]++;
		}
	}
}

void PenteGame::penteSub03Scoring(penteTable *table, byte move_y, byte move_x, bool stauf_turn) {
	table->boardState[move_x][move_y] = stauf_turn ? 88 : 79;
	uint16 lines = table->linesTable[move_x][move_y][0];

	for (int i = 1; i <= lines; i++) {
		uint16 lineIndex = table->linesTable[move_x][move_y][i];
		penteScoringLine(table, lineIndex, stauf_turn, false);
	}

	if (table->calcTouchingPieces != 0) {
		penteTouchingPieces(table, move_x, move_y, false);
	}

	table->moveCounter++;
}

void PenteGame::penteSub07RevertScore(penteTable *table, byte y, byte x) {
	bool stauf_turn = table->boardState[x][y] == 88;
	table->boardState[x][y] = 0;
	table->moveCounter--;
	uint lines = table->linesTable[x][y][0];

	for (uint i = 1; i <= lines; i++) {
		uint16 lineIndex = table->linesTable[x][y][i];
		penteScoringLine(table, lineIndex, stauf_turn, true);
	}

	if (table->calcTouchingPieces != 0) {
		penteTouchingPieces(table, x, y, true);
	}
}

byte PenteGame::penteScoreCaptureSingle(penteTable *table, byte x, byte y, int slopeX, int slopeY) {
	byte x1 = x + slopeX;
	byte y1 = y + slopeY;
	byte x2 = x + slopeX * 2;
	byte y2 = y + slopeY * 2;
	byte endX = x + slopeX * 3;
	byte endY = y + slopeY * 3;

	// we don't need to check for below 0 when we have unsigned types
	if (x >= table->width || y >= table->height)
		return 0;
	if (endX >= table->width || endY >= table->height)
		return 0;

	auto &boardState = table->boardState;
	byte captor = boardState[x][y];
	byte captive = captor == 88 ? 79 : 88;

	// make sure the captor is at the start and end of the line
	if (boardState[endX][endY] != captor)
		return 0;

	// check that the captive is both of the middle pieces
	if (boardState[x1][y1] != captive || boardState[x2][y2] != captive)
		return 0;

	penteSub07RevertScore(table, y1, x1);
	penteSub07RevertScore(table, y2, x2);
	return 1;
}

struct Slope {
	int x, y;
};
// the order of these is important because we return the bitMask
Slope slopes[] = {{1, 0},
				  {1, 1},
				  {0, 1},
				  {-1, 1},
				  {-1, 0},
				  {-1, -1},
				  {0, -1},
				  {1, -1}};

uint PenteGame::penteSub04ScoreCapture(penteTable *table, byte y, byte x) {
	byte bitMask = 0;
	bool isStauf = table->boardState[x][y] == 88;

	for (const Slope &slope : slopes) {
		bitMask <<= 1;
		bitMask |= penteScoreCaptureSingle(table, x, y, slope.x, slope.y);
	}

	for (int i = bitMask; i; i >>= 1) {
		if ((i & 1) == 0)
			continue;
		pentePlayerTable *playerTable;
		uint &score = getPlayerTable(table, isStauf, playerTable);

		int lineLength = ++playerTable->lines[table->linesCounter];
		if (table->lineLength == lineLength) {
			score += WIN_SCORE;
		} else {
			score += 1 << (lineLength - 1U & 0x1f);
		}
	}
	return bitMask;
}

void PenteGame::penteSub08MaybeAnimateCapture(short move, byte *bitMaskG, short *param_3, short *param_4)

{
	byte x;
	byte y;

	x = (byte)((int)move / 0xf);
	y = 0xe - (char)((int)move % 0xf);

	byte &bitMask = *bitMaskG;
	byte bVar3 = 0;
	for (bVar3 = 0; bVar3 < 8; bVar3++) {
		if (bitMask >> bVar3 & 1) {
			bitMask = '\x01' << (bVar3 & 0x1f) ^ bitMask;
			break;
		}
	}

	short sVar4;
	switch (bVar3) {
	case 0:
		*param_3 = (x + 2) * 0xf - (uint16)y;
		*param_4 = ((uint16)x * 0xf - (uint16)y) + 0x2e;
		return;
	case 1:
		*param_3 = (x + 1) * 0xf - (uint16)y;
		*param_4 = ((uint16)x * 0xf - (uint16)y) + 0x10;
		return;
	case 2:
		sVar4 = (uint16)x * 0xf - (uint16)y;
		*param_3 = sVar4;
		*param_4 = sVar4 + -0xe;
		return;
	case 3:
		sVar4 = (uint16)x * 0xf - (uint16)y;
		*param_3 = sVar4 + -1;
		*param_4 = sVar4 + -0x10;
		return;
	case 4:
		sVar4 = (uint16)x * 0xf - (uint16)y;
		*param_3 = sVar4 + -2;
		*param_4 = sVar4 + -0x12;
		return;
	case 5:
		sVar4 = (uint16)x * 0xf - (uint16)y;
		*param_3 = sVar4 + 0xd;
		*param_4 = sVar4 + 0xc;
		return;
	case 6:
		sVar4 = (uint16)x * 0xf - (uint16)y;
		*param_3 = sVar4 + 0x1c;
		*param_4 = sVar4 + 0x2a;
		return;
	case 7:
		sVar4 = (uint16)x * 0xf - (uint16)y;
		*param_3 = sVar4 + 0x1d;
		*param_4 = sVar4 + 0x2c;
	}
	return;
}

void PenteGame::penteSub11RevertCapture(penteTable *table, byte y, byte x, byte bitMask) {
	bool isPlayer = table->boardState[x][y] == 79;
	for (int i = bitMask; i; i >>= 1) {
		if ((i & 1) == 0)
			continue;

		pentePlayerTable *playerTable;
		uint &score = getPlayerTable(table, !isPlayer, playerTable);

		int linesCounter = --playerTable->lines[table->linesCounter];

		if (table->lineLength - linesCounter == 1) {
			score -= WIN_SCORE;
		} else {
			score -= (1 << ((char)linesCounter & 0x1f));
		}
	}

	for (int i = 0; i < 8; i++) {
		if ((bitMask >> i & 1) == 0)
			continue;

		Slope &slope = slopes[7 - i];
		penteSub03Scoring(table, y + slope.y * 2, x + slope.x * 2, isPlayer);
		penteSub03Scoring(table, y + slope.y, x + slope.x, isPlayer);
	}
}

int PenteGame::penteSub10AiRecurse(penteTable *table_1, char depth, int parent_score) {
	int iVar2;
	int iVar3;
	int iVar4;
	bool bVar5;
	uint uVar7;
	int iVar8;
	uint uVar9;
	short sVar10;
	int iVar12;
	uint16 local_970[2];
	int best_score;
	Common::FixedStack<int, 600> local_95c;

	best_score = 0x7fffffff;
	if (depth == 1) {
		table_1->calcTouchingPieces = 0;
		for (byte bVar1 = 0; bVar1 < table_1->width; bVar1++) {
			for (byte bVar11 = 0; bVar11 < table_1->height; bVar11++) {
				if ((table_1->boardState[bVar1][bVar11] != 0) ||
					(table_1->numAdjacentPieces[bVar1][bVar11] == 0)) {
					continue;
				}

				penteSub03Scoring(table_1, bVar11, bVar1, (bool)((byte)table_1->moveCounter & 1));
				uVar7 = penteSub04ScoreCapture(table_1, bVar11, bVar1);
				if ((*(byte *)&table_1->moveCounter & 1) == 0) {
					iVar12 = table_1->playerScore - table_1->staufScore;
				} else {
					iVar12 = table_1->staufScore - table_1->playerScore;
				}
				if ((byte)uVar7 != 0) {
					penteSub11RevertCapture(table_1, bVar11, bVar1, (byte)uVar7);
				}
				penteSub07RevertScore(table_1, bVar11, bVar1);
				if (iVar12 < best_score) {
					best_score = iVar12;
				}
				if (-parent_score != best_score && parent_score <= -best_score) {
					table_1->calcTouchingPieces = 1;
					return -best_score;
				}
			}
		}
		table_1->calcTouchingPieces = 1;
	} else {
		*(uint *)local_970 = 0;
		for (byte bVar1 = 0; bVar1 < table_1->width; bVar1++) {
			for (byte bVar11 = 0; bVar11 < table_1->height; bVar11++) {
				if ((table_1->boardState[bVar1][bVar11] != 0) ||
					(table_1->numAdjacentPieces[bVar1][bVar11] == 0)) {
					continue;
				}

				penteSub03Scoring(table_1, bVar11, bVar1, (bool)((byte)table_1->moveCounter & 1));
				uVar7 = penteSub04ScoreCapture(table_1, bVar11, bVar1);
				if ((*(byte *)&table_1->moveCounter & 1) == 0) {
					iVar12 = table_1->playerScore - table_1->staufScore;
				} else {
					iVar12 = table_1->staufScore - table_1->playerScore;
				}
				if ((byte)uVar7 != 0) {
					penteSub11RevertCapture(table_1, bVar11, bVar1, (byte)uVar7);
				}
				penteSub07RevertScore(table_1, bVar11, bVar1);
				iVar8 = (int)(short)local_970[1];
				*(uint *)local_970 = (uint)(uint16)(local_970[1] + 1) << 0x10;
				local_95c[iVar8 * 2 + 1] = iVar12;
				*(byte *)&local_95c[iVar8 * 2] = bVar11;
				*((byte *)&local_95c[iVar8 * 2] + 1) = bVar1;
			}
		}

		uint16 uVar6;
		for (uVar6 = 1; uVar6 < local_970[1]; uVar6 = uVar6 * 3 + 1) {
		}

		while (2 < (short)uVar6) {
			uVar6 = (short)uVar6 / 3;
			uVar7 = *(uint *)local_970 & 0xffff0000;
			*(uint *)local_970 = uVar7 | uVar6;
			local_970[1] = (uint16)(uVar7 >> 0x10);
			if ((short)uVar6 < (short)local_970[1]) {
				do {
					bVar5 = false;
					sVar10 = local_970[0] - uVar6;
					while ((-1 < sVar10 && (!bVar5))) {
						iVar8 = (int)sVar10;
						iVar12 = (short)uVar6 + iVar8;
						if (local_95c[iVar12 * 2 + 1] < local_95c[iVar8 * 2 + 1]) {
							sVar10 -= uVar6;
							iVar2 = local_95c[iVar12 * 2 + 1];
							iVar3 = local_95c[iVar12 * 2];
							iVar4 = local_95c[iVar8 * 2 + 1];
							local_95c[iVar12 * 2] = local_95c[iVar8 * 2];
							local_95c[iVar12 * 2 + 1] = iVar4;
							local_95c[iVar8 * 2] = iVar3;
							local_95c[iVar8 * 2 + 1] = iVar2;
						} else {
							bVar5 = true;
						}
					}
					uVar7 = *(uint *)local_970 & 0xffff0000;
					*(uint *)local_970 = uVar7 | (uint16)(local_970[0] + 1);
					local_970[1] = (uint16)(uVar7 >> 0x10);
				} while ((short)(local_970[0] + 1) < (short)local_970[1]);
			}
		}

		local_970[1] = (uint16)(*(uint *)local_970 >> 0x10);
		for (sVar10 = 0; sVar10 < local_970[1]; sVar10++) {
			byte bVar1 = *(byte *)&local_95c[sVar10 * 2];
			byte bVar11 = *((byte *)&local_95c[sVar10 * 2] + 1);
			*(uint *)local_970 &= 0xffffff00;
			penteSub03Scoring(table_1, bVar1, bVar11, (bool)((byte)table_1->moveCounter & 1));
			uVar9 = penteSub04ScoreCapture(table_1, bVar1, bVar11);
			uVar7 = table_1->playerScore;
			if ((((int)uVar7 < WIN_SCORE) && ((int)table_1->staufScore < WIN_SCORE)) &&
				(table_1->boardSize != table_1->moveCounter)) {
				iVar12 = penteSub10AiRecurse(table_1, depth + -1, best_score);
			} else {
				if ((*(byte *)&table_1->moveCounter & 1) == 0) {
					iVar12 = uVar7 - table_1->staufScore;
				} else {
					iVar12 = table_1->staufScore - uVar7;
				}
			}
			if ((byte)uVar9 != 0) {
				penteSub11RevertCapture(table_1, bVar1, bVar11, (byte)uVar9);
			}
			penteSub07RevertScore(table_1, bVar1, bVar11);
			if (iVar12 < best_score) {
				best_score = iVar12;
			}
			if (-parent_score != best_score && parent_score <= -best_score)
				break;
			local_970[1] = (uint16)(*(uint *)local_970 >> 0x10);
		}
	}
	return -best_score;
}

uint PenteGame::penteSub09Ai(uint y_1, int param_2, int param_3, penteTable *table_4, byte depth) {
	bool bVar1;
	uint uVar2;
	int iVar3;
	byte _y;
	int best_score;
	uint16 uVar5;
	byte bStack18;
	byte _x;
	uint16 local_c;
	short local_4;
	uint _y2;

	bStack18 = 1;
	_x = 0;
	uVar5 = 0xffff;
	best_score = 0x7fffffff;
	if (table_4->width != 0) {
		do {
			_y = 0;
			if (table_4->height != 0) {
				do {
					y_1 = (uint)_y;
					if ((table_4->boardState[_x][y_1] == 0) && (table_4->numAdjacentPieces[_x][y_1] != 0)) {
						penteSub03Scoring(table_4, _y, _x, (bool)((byte)table_4->moveCounter & 1));
						_y2 = penteSub04ScoreCapture(table_4, _y, _x);
						if (((int)table_4->playerScore < WIN_SCORE) &&
							((int)table_4->staufScore < WIN_SCORE)) {
							bVar1 = false;
						} else {
							bVar1 = true;
						}
						if ((byte)_y2 != 0) {
							penteSub11RevertCapture(table_4, _y, _x, (byte)_y2);
						}
						penteSub07RevertScore(table_4, _y, _x);
						if (bVar1) {
							return y_1 & 0xffff0000 | (uint)(uint16)((uint16)_y + (uint16)_x * 100);
						}
					}
					_y += 1;
				} while (_y <= table_4->height && table_4->height != _y);
			}
			_x += 1;
			y_1 &= 0xffffff00;
		} while (_x <= table_4->width && table_4->width != _x);
	}
	do {
		_x = 0;
		if (table_4->width != 0) {
			do {
				_y2 = 0;
				if (table_4->height != 0) {
					do {
						_y = (byte)_y2;
						y_1 = 0;
						if ((table_4->boardState[_x][_y2] == 0) && (table_4->numAdjacentPieces[_x][_y2] != 0)) {
							penteSub03Scoring(table_4, _y, _x, (bool)((byte)table_4->moveCounter & 1));
							uVar2 = penteSub04ScoreCapture(table_4, _y, _x);
							iVar3 = penteSub10AiRecurse(table_4, depth - 1, best_score);
							if ((byte)uVar2 != 0) {
								penteSub11RevertCapture(table_4, _y, _x, (byte)uVar2);
							}
							penteSub07RevertScore(table_4, _y, _x);
							local_c = (uint16)_x;
							local_4 = (short)_y2;
							if (iVar3 < best_score) {
								bStack18 = 1;
								uVar5 = local_c * 100 + local_4;
								best_score = iVar3;
							} else {
								if (iVar3 == best_score) {
									bStack18 += 1;
									_y2 = _random.getRandomNumber(UINT_MAX);
									y_1 = (uint)bStack18;
									if ((_y2 % CAPTURE_SCORE) * y_1 < CAPTURE_SCORE) {
										uVar5 = local_c * 100 + local_4;
									}
								}
							}
						}
						_y += 1;
						_y2 = (uint)_y;
					} while (_y <= table_4->height && table_4->height != _y);
				}
				_x += 1;
				y_1 &= 0xffffff00;
			} while (_x <= table_4->width && table_4->width != _x);
		}
	} while ((99999999 < best_score) && (depth -= 1, 1 < depth));
	return y_1 & 0xffff0000 | (uint)uVar5;
}

int varsMoveToXY(byte var0, byte var1, byte var2, byte &x, byte &y) {
	int move = ((char)var0 * 10 + (short)(char)var1) * 10 + (short)(char)var2;
	x = (byte)(move / 15);
	y = 0xe - (char)((int)move % 15);
	return move;
}

void aiMoveToXY(int move, byte &x, byte &y) {
	x = move / 100;
	y = move % 100;
}

void moveToVars(int move, byte &var0, byte &var1, byte &var2) {
	var0 = (byte)(move / 100);
	var1 = (byte)((move % 100) / 10);
	var2 = (byte)(move % 10);
}

void moveXYToVars(uint x, uint y, byte &var0, byte &var1, byte &var2) {
	int move = (x * 0xf - y) + 0xe;
	moveToVars(move, var0, var1, var2);
}

void PenteGame::penteOp(byte *vars) {
	uint16 uVar1;
	int iVar2;
	uint uVar3;
	byte ai_depth;
	short local_2;

	if ((game_state_table == (penteTable *)0x0) && (vars[4] != 0)) {
		game_state_table = penteSub01Init(0x14, 0xf, 5);
	}
	uVar3 = vars[4];
	debugC(kDebugLogic, "penteOp vars[4]: %d", (int)vars[4]);

	switch (uVar3) {
	case 0:
		if (game_state_table != (penteTable *)0x0) {
			penteSub02Frees(game_state_table);
		}
		game_state_table = (penteTable *)0x0;
		return;
	case 1:
		globalPlayerMove = varsMoveToXY(vars[0], vars[1], vars[2], globalX, globalY);
		debugC(kDebugLogic, "player moved to %d, %d", (int)globalX, (int)globalY);
		penteSub03Scoring(game_state_table, globalY, globalX,
										 (bool)((byte)game_state_table->moveCounter & 1));
		uVar3 = penteSub04ScoreCapture(game_state_table, globalY, globalX);
		global2 = (char)uVar3;
		return;
	case 2:
	case 4:
		if (global2 != '\0') {
			if (global1 < 0) {
				penteSub08MaybeAnimateCapture(globalPlayerMove, (byte *)&global2, &local_2, &global1);
				vars[5] = 1;
				moveToVars(local_2, vars[0], vars[1], vars[2]);
				return;
			}
		LAB_00412da4:
			vars[0] = (byte)((int)global1 / 100);
			vars[1] = (byte)((int)(global1 % 100) / 10);
			iVar2 = (int)global1;
			vars[2] = (byte)(iVar2 % 10);
			global1 = -1;
			vars[5] = 1;
			return;
		}
		if (-1 < global1)
			goto LAB_00412da4;
		if ((int)game_state_table->playerScore < WIN_SCORE) {
			if (((int)game_state_table->staufScore < WIN_SCORE) &&
				(uVar1 = game_state_table->moveCounter)) {
				vars[5] = 0;
				return;
			}
			if ((int)game_state_table->playerScore < WIN_SCORE) {
				uVar3 = game_state_table->staufScore;
				vars[5] = 2; // Stauf wins
				if ((int)uVar3 < WIN_SCORE) {
					vars[5] = 4; // player wins because the board is full?
				}
				goto DEALLOC;
			}
		}
		vars[5] = 3; // player wins
	DEALLOC:
		penteSub02Frees(game_state_table);
		game_state_table = (penteTable *)0x0;
		return;
	case 3:
		break;
	case 5:
		// asking Samantha to make a move? this does a bunch of queries to check if pieces belong to stauf or the player?
		byte x, y;
		varsMoveToXY(vars[0], vars[1], vars[2], x, y);
		ai_depth = game_state_table->boardState[x][y];
		if (ai_depth == 0) {
			vars[3] = 0;
			return;
		}
		if (ai_depth == 0x4f) {
			vars[3] = 2;
			return;
		}
		if (ai_depth != 0x58) {
			return;
		}
		vars[3] = 1;
	default:
		return;
	}
	ai_depth = vars[6];
	if (ai_depth == 0) {
		ai_depth = 3;
	} else {
		if (ai_depth == 1) {
			ai_depth = 4;
		} else {
			if (ai_depth != 2)
				goto LAB_00412e85;
			ai_depth = 5;
		}
	}
	globalPlayerMove = penteSub09Ai(0, 0, 0, game_state_table, ai_depth);
LAB_00412e85:
	aiMoveToXY(globalPlayerMove, globalX, globalY);
	debugC(kDebugLogic, "Stauf moved to %d, %d", (int)globalX, (int)globalY);
	penteSub03Scoring(game_state_table, globalY, globalX,
									 (bool)((byte)game_state_table->moveCounter & 1));
	uVar3 = penteSub04ScoreCapture(game_state_table, globalY, globalX);
	global2 = (char)uVar3;
	globalPlayerMove = ((uint16)globalX * 0xf - (uint16)globalY) + 0xe;
	moveXYToVars(globalX, globalY, vars[0], vars[1], vars[2]);
}

PenteGame::PenteGame() : _random("PenteGame") {
	global1 = -1;
	game_state_table = NULL;
	globalY = 0;
	globalX = 0;
	global2 = 0;
	globalPlayerMove = 0;
#if 0
	test();
#endif
}

void PenteGame::run(byte *scriptVariables) {
	// TODO: don't need to copy these variables once this is cleaned up
	byte tvars[1024];
	memcpy(tvars, scriptVariables, sizeof(tvars));
	penteOp(tvars);
	for (int i = 0; i < sizeof(tvars); i++) {
		if (tvars[i] != scriptVariables[i]) {
			debugC(kDebugLogic, "PenteGame::run var %d changed from %d to %d", i, (int)scriptVariables[i], (int)tvars[i]);
		}
	}
	memcpy(scriptVariables, tvars, sizeof(tvars));
}

void PenteGame::test() {
	warning("starting PenteGame::test()");
	uint32 oldSeed = _random.getSeed();

	// 6 moves per line
	testGame(3,
		{
			/*x=*/10, /*y=*/6, /*x=*/9, /*y=*/6, /*x=*/10, /*y=*/7, /*x=*/10, /*y=*/5, /*x=*/10, /*y=*/8, /*x=*/9, /*y=*/9,
			/*x=*/10, /*y=*/9, /*x=*/10, /*y=*/10, /*x=*/9, /*y=*/8, /*x=*/8, /*y=*/7, /*x=*/8, /*y=*/8, /*x=*/7, /*y=*/8,
			/*x=*/6, /*y=*/9, /*x=*/11, /*y=*/4, 
		}, false);

	testGame(10,
		{
			/*x=*/10, /*y=*/6, /*x=*/11, /*y=*/7, /*x=*/10, /*y=*/5, /*x=*/10, /*y=*/7, /*x=*/9, /*y=*/7, /*x=*/12, /*y=*/7,
			/*x=*/10, /*y=*/4, /*x=*/8, /*y=*/8, /*x=*/10, /*y=*/3, /*x=*/11, /*y=*/5, /*x=*/10, /*y=*/2, /*x=*/9, /*y=*/7,
			/*x=*/10, /*y=*/6, 
		}, true);

	// test bottom left corner
	testGame(1993,
		{
			/*x=*/0, /*y=*/0, /*x=*/1, /*y=*/1, /*x=*/1, /*y=*/0, /*x=*/2, /*y=*/0, /*x=*/0, /*y=*/1, /*x=*/0, /*y=*/2,
			/*x=*/2, /*y=*/1, /*x=*/3, /*y=*/2, /*x=*/1, /*y=*/2, /*x=*/2, /*y=*/3, /*x=*/4, /*y=*/1, /*x=*/1, /*y=*/4,
			/*x=*/5, /*y=*/1, /*x=*/6, /*y=*/1, /*x=*/3, /*y=*/0, /*x=*/5, /*y=*/2, /*x=*/4, /*y=*/3, /*x=*/3, /*y=*/1,
			/*x=*/3, /*y=*/3, /*x=*/5, /*y=*/3, /*x=*/4, /*y=*/1, /*x=*/4, /*y=*/3, /*x=*/3, /*y=*/3, /*x=*/3, /*y=*/4,
			/*x=*/2, /*y=*/5, /*x=*/7, /*y=*/0
		}, false);

	// test bottom right corner
	testGame(1995,
		{
			/*x=*/19, /*y=*/0, /*x=*/18, /*y=*/1, /*x=*/19, /*y=*/1, /*x=*/18, /*y=*/2, /*x=*/18, /*y=*/0, /*x=*/18, /*y=*/3,
			/*x=*/18, /*y=*/4, /*x=*/17, /*y=*/5, /*x=*/17, /*y=*/0, /*x=*/16, /*y=*/5, /*x=*/17, /*y=*/4, /*x=*/16, /*y=*/4,
			/*x=*/18, /*y=*/5, /*x=*/18, /*y=*/6, /*x=*/18, /*y=*/5, /*x=*/15, /*y=*/3, /*x=*/18, /*y=*/4, /*x=*/14, /*y=*/2, 
		}, false);

	// test top left corner
	testGame(1996,
		{
			/*x=*/0, /*y=*/14, /*x=*/1, /*y=*/13, /*x=*/1, /*y=*/14, /*x=*/2, /*y=*/14, /*x=*/0, /*y=*/13, /*x=*/0, /*y=*/12,
			/*x=*/1, /*y=*/12, /*x=*/2, /*y=*/11, /*x=*/2, /*y=*/12, /*x=*/3, /*y=*/12, /*x=*/4, /*y=*/13, /*x=*/1, /*y=*/10,
			/*x=*/0, /*y=*/9, /*x=*/3, /*y=*/10, /*x=*/1, /*y=*/12, /*x=*/4, /*y=*/9, /*x=*/5, /*y=*/8, /*x=*/6, /*y=*/9,
			/*x=*/3, /*y=*/11, /*x=*/6, /*y=*/10, /*x=*/6, /*y=*/11, /*x=*/4, /*y=*/8, /*x=*/3, /*y=*/9, /*x=*/4, /*y=*/10,
			/*x=*/4, /*y=*/11, /*x=*/2, /*y=*/10, /*x=*/0, /*y=*/10, /*x=*/5, /*y=*/10
		}, false);

	// test top right corner
	testGame(2019,
		{
			/*x=*/19, /*y=*/14, /*x=*/18, /*y=*/13, /*x=*/19, /*y=*/12, /*x=*/18, /*y=*/12, /*x=*/18, /*y=*/11, /*x=*/17, /*y=*/10,
			/*x=*/18, /*y=*/14, /*x=*/16, /*y=*/11, /*x=*/18, /*y=*/9, /*x=*/15, /*y=*/12, /*x=*/14, /*y=*/13, /*x=*/15, /*y=*/10,
			/*x=*/15, /*y=*/11, /*x=*/14, /*y=*/10, /*x=*/17, /*y=*/12, /*x=*/16, /*y=*/10, /*x=*/13, /*y=*/10, /*x=*/18, /*y=*/10
		}, false);

	_random.setSeed(oldSeed);
	warning("finished PenteGame::test()");
}

void PenteGame::testGame(uint32 seed, Common::Array<int> moves, bool playerWin) {
	byte vars[1024];
	byte &winner = vars[5];
	byte &op = vars[4];

	warning("starting PenteGame::testGame(%u, %u, %d)", seed, moves.size(), (int)playerWin);
	memset(vars, 0, sizeof(vars));
	_random.setSeed(seed);

	op = 0;
	penteOp(vars);

	for (uint i = 0; i < moves.size(); i += 2) {
		if (winner)
			error("%u: early winner: %d", i, (int)winner);

		int x = moves[i];
		int y = moves[i + 1];

		if (i % 4) {
			// check Stauf's move
			op = 3;
			penteOp(vars);

			byte sX, sY;
			varsMoveToXY(vars[0], vars[1], vars[2], sX, sY);

			if (sX != x || sY != y)
				error("%u: Stauf, expected (%d, %d), got (%d, %d)", i, (int)x, (int)y, (int)sX, (int)sY);

			do {
				op = 4;
				penteOp(vars);
			} while (winner == 1);
			continue;
		}

		moveXYToVars(x, y, vars[0], vars[1], vars[2]);
		op = 1;
		penteOp(vars);

		do {
			op = 2;
			penteOp(vars);
		} while (winner == 1);
	}

	if (playerWin && winner != 3)
		error("player didn't win, winner: %d", (int)winner);
	else if (playerWin == false && winner != 2)
		error("Stauf didn't win, winner: %d", (int)winner);

	warning("finished PenteGame::testGame(%u, %u, %d)", seed, moves.size(), (int)playerWin);
}

} // namespace Groovie
