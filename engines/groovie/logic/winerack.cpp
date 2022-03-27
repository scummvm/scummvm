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
#include "groovie/logic/winerack.h"

namespace Groovie {

enum WineBottle {
	kWineBottleOpponent = 1,
	kWineBottlePlayer = 2
};

namespace {
extern const int8 wineRackLogicTable[1200];
}

WineRackGame::WineRackGame() : _random("WineRackGame"), _totalBottles(0) {
	memset(_wineRackGrid, 0, 100);

#if 0
	runTests();
#endif
}

void WineRackGame::run(byte *scriptVariables) {
	char op = scriptVariables[3];
	byte pos = 0;

	/* positions on the board
	*	north = 9 (0, 9)
		south = 90 (9, 0)
		east = 99 (9, 9)
		west = 0 (0, 0)
	*/

	switch (op) {
	case 3:
		debugC(kDebugLogic, "WineRackGame initGrid, seed: %u", _random.getSeed());
		initGrid(scriptVariables[4]);
		break;
	case 4:	// Player's move
		pos = calculateNextMove(kWineBottlePlayer);
		placeBottle(pos, kWineBottlePlayer);
		scriptVariables[0] = pos / 10;
		scriptVariables[1] = pos % 10;
		scriptVariables[3] = didPlayerWin();
		break;
	case 5:	// Opponent's move
		scriptVariables[3] = 0;
		pos = calculateNextMove(kWineBottleOpponent);
		placeBottle(pos, kWineBottleOpponent);
		scriptVariables[0] = pos / 10;
		scriptVariables[1] = pos % 10;
		scriptVariables[3] = didAiWin() != 0 ? 1 : 0;
		break;
	default:
		scriptVariables[3] = 0;
		placeBottle(scriptVariables[0] * 10 + scriptVariables[1], 2);

		if (didPlayerWin()) {
			scriptVariables[3] = 2;
		} else {
			pos = calculateNextMove(kWineBottleOpponent);
			placeBottle(pos, kWineBottleOpponent);
			scriptVariables[0] = pos / 10;
			scriptVariables[1] = pos % 10;
			scriptVariables[3] = didAiWin() != 0 ? 1 : 0;
		}
		break;
	}
}

void WineRackGame::initGrid(byte difficulty) {
	memset(_wineRackGrid, 0, 100);

	switch (difficulty) {
	case 0:
		_totalBottles = 24;

		_wineRackGrid[15] = kWineBottlePlayer;
		_wineRackGrid[18] = kWineBottleOpponent;
		_wineRackGrid[19] = kWineBottleOpponent;
		_wineRackGrid[20] = kWineBottleOpponent;
		_wineRackGrid[21] = kWineBottleOpponent;
		_wineRackGrid[22] = kWineBottleOpponent;
		_wineRackGrid[23] = kWineBottleOpponent;
		_wineRackGrid[25] = kWineBottlePlayer;
		_wineRackGrid[26] = kWineBottleOpponent;
		_wineRackGrid[27] = kWineBottleOpponent;
		_wineRackGrid[28] = kWineBottleOpponent;
		_wineRackGrid[33] = kWineBottleOpponent;
		_wineRackGrid[34] = kWineBottleOpponent;
		_wineRackGrid[35] = kWineBottlePlayer;
		_wineRackGrid[36] = kWineBottleOpponent;
		_wineRackGrid[44] = kWineBottlePlayer;
		_wineRackGrid[45] = kWineBottlePlayer;
		_wineRackGrid[54] = kWineBottlePlayer;
		_wineRackGrid[62] = kWineBottlePlayer;
		_wineRackGrid[63] = kWineBottlePlayer;
		_wineRackGrid[64] = kWineBottlePlayer;
		_wineRackGrid[72] = kWineBottlePlayer;
		_wineRackGrid[82] = kWineBottlePlayer;
		_wineRackGrid[91] = kWineBottlePlayer;
		break;

	case 1:
		_totalBottles = 12;

		_wineRackGrid[75] = kWineBottlePlayer;
		_wineRackGrid[56] = kWineBottlePlayer;
		_wineRackGrid[45] = kWineBottlePlayer;
		_wineRackGrid[27] = kWineBottlePlayer;
		_wineRackGrid[24] = kWineBottlePlayer;
		_wineRackGrid[15] = kWineBottlePlayer;
		_wineRackGrid[64] = kWineBottleOpponent;
		_wineRackGrid[34] = kWineBottleOpponent;
		_wineRackGrid[33] = kWineBottleOpponent;
		_wineRackGrid[18] = kWineBottleOpponent;
		_wineRackGrid[16] = kWineBottleOpponent;
		_wineRackGrid[14] = kWineBottleOpponent;
		break;

	default:
		_totalBottles = 0;
		break;
	}
}

void WineRackGame::placeBottle(byte pos, byte val) {
	debugC(kDebugLogic, "placeBottle(%d, %d)", (int)pos, (int)val);
	_totalBottles++;
	assert(_wineRackGrid[pos] == 0);
	_wineRackGrid[pos] = val;
}

int8 WineRackGame::calculateNextMove(byte player) {
	int8 moves1[24];
	int8 moves2[24];
	byte playerIndex = (player == 1) + 1;

	if (!_totalBottles)
		return randomMoveStart();
	if (_totalBottles == 1)
		return randomMoveStart2();

	sub05(player, moves1);
	sub05(playerIndex, moves2);

	int8 result = sub06(moves1, moves2);

	if (result == -1)
		return findEmptySpot();

	assert(_wineRackGrid[result] == 0);

	return result;
}

int8 WineRackGame::findEmptySpot() {
	int8 result = 0;

	while (_wineRackGrid[result]) {
		if (result == 99)
			return 100;
		else
			++result;
	}

	return result;
}

void WineRackGame::sub05(int8 player, int8 *moves) {
	int8 moves1[23];
	int8 moves2[23];
	int8 to, delta, playerIndex;

	if (player == 1) {
		to = 90;
		delta = 10;
		playerIndex = 2;
	} else {
		to = 9;
		delta = 1;
		playerIndex = 3;
	}

	memset(moves, 0, 23);

	for (int i = 0; i < to; i += delta) {
		if (!_wineRackGrid[i] || _wineRackGrid[i] == player) {
			memset(moves1, 0, sizeof(moves1));
			memset(moves2, 0, sizeof(moves2));

			sub13(i, playerIndex, moves1, moves2);

			if (moves[0] < moves1[0]) {
				memcpy(moves, moves1, 23);
			}
		}
	}
}

int8 WineRackGame::sub06(int8 *moves1, int8 *moves2) {
	for (int i = 0; i < moves1[2]; i++) {

		int8 result = moves1[i + 3];

		if (!_wineRackGrid[result]) {
			for (int j = 0; j < moves2[2]; j++)
				if (moves2[j + 3] == result)
					return result;
		}
	}

	for (int i = 0; i < moves1[2]; i++) {
		if (!_wineRackGrid[moves1[i + 3]])
			return moves1[i + 3];
	}

	return -1;
}

uint32 WineRackGame::didPlayerWin() {
	memset(_wineRackGrid2, 0, 100);

	for (int i = 0; i < 10; i++) {
		if (_wineRackGrid[i] == kWineBottlePlayer) {
			int var = 0;
			sub10(100, i, 2, 3, &var);
			if (var == 1)
				return 1;
		}
	}

	return 0;
}

void WineRackGame::sub10(int8 endPos, int8 pos, int unused, int player, int *val) {
	int8 candidates[8];

	if (*val)
		return;

	if (wineRackLogicTable[12 * pos + player] == -1) {
		*val = 1;

		return;
	}

	sub11(pos, candidates);

	for (int i = 0; candidates[i] != 100; i++) {
		int8 nextPos = candidates[i];

		if (endPos != nextPos)
			sub10(pos, nextPos, unused, player, val);
	}
}

void WineRackGame::sub11(int8 pos, int8 *candidates) {
	int cnt = 0;

	_wineRackGrid2[pos] = 1;

	for (int i = 0; i < 6; i++) {
		int8 val = wineRackLogicTable[12 * pos + i];

		if (!_wineRackGrid2[val] && _wineRackGrid[pos] == _wineRackGrid[val])
			candidates[cnt++] = val;
	}

	candidates[cnt] = 100;
}

uint32 WineRackGame::didAiWin() {
	memset(_wineRackGrid2, 0, 100);

	for (int i = 0; i < 100; i += 10) {
		if (_wineRackGrid[i] == kWineBottleOpponent) {
			int var = 0;
			sub10(100, i, 1, 2, &var);
			if (var == 1)
				return 1;
		}
	}

	return 0;
}

void WineRackGame::sub13(int8 cell, int8 player, int8 *moves1, int8 *moves2) {
	int8 candidates[4];

	if (cell == -1)
		return;

	moves2[moves2[2] + 3] = cell;

	if (wineRackLogicTable[12 * cell + player] < 0) {
		++moves2[2];

		moves2[0] = countEmtpy(moves2);

		if (moves2[0] > moves1[0])
			memcpy(moves1, moves2, 23);

		--moves2[2];
	} else {
		++moves2[2];

		if (player == 2)
			sub15(cell, candidates);
		else
			sub16(cell, candidates);

		for (int i = 0; candidates[i] != -1; i++)
			sub13(candidates[i], player, moves1, moves2);

		--moves2[2];
	}
}

void WineRackGame::sub15(int8 cell, int8 *candidates) {
	int8 depth = 0;
	int8 pos2 = wineRackLogicTable[12 * cell + 2];
	int8 pos1 = wineRackLogicTable[12 * cell + 1];

	if (_wineRackGrid[pos2] == 2) {
		if (pos1 < 0 || _wineRackGrid[pos1] == 2) {
			if (cell >= 20) {
				int8 val1 = _wineRackGrid[cell - 10];

				if (val1 != 2) {
					int8 val2 = _wineRackGrid[cell - 10];

					if (val2 != 2 && (val1 == 1 || val2 == 1)) {
						depth = 1;
						candidates[0] = cell - 10;
					}
				}
			}
			if (cell < 80) {
				int8 val1 = _wineRackGrid[cell + 10];

				if (val1 != 2) {
					int8 val2 = _wineRackGrid[cell + 11];

					if (val2 != 2 && (val1 == 1 || val2 == 1)) {
						candidates[depth] = cell + 10;
						depth++;
					}
				}
			}
		} else if (_wineRackGrid[cell] == 1 || _wineRackGrid[pos1] == 1) {
			depth = 1;
			candidates[0] = pos1;
		}
	} else if (pos1 < 0 || _wineRackGrid[pos1] == 2) {
		if (_wineRackGrid[cell] == 1 || _wineRackGrid[pos2] == 1) {
			depth = 1;
			candidates[0] = pos2;
		}
	} else {
		depth = 2;
		candidates[0] = pos2;
		candidates[1] = pos1;
	}

	candidates[depth] = -1;
}

void WineRackGame::sub16(int8 cell, int8 *candidates) {
	int8 depth = 0;
	int8 pos3 = wineRackLogicTable[12 * cell + 3];
	int8 pos4 = wineRackLogicTable[12 * cell + 4];

	if (_wineRackGrid[pos3] == 1) {
		if (pos4 < 0 || _wineRackGrid[pos4] == 1) {
			if (cell % 10 >= 2) {
				int8 val1 = _wineRackGrid[cell - 1];

				if (val1 != 1) {
					int8 val2 = _wineRackGrid[cell + 8];

					if (val2 != 1 && (val1 == 2 || val2 == 2)) {
						depth = 1;
						candidates[0] = cell - 1;
					}
				}
			}
			if (cell < 80 && _wineRackGrid[cell + 1] != 1) {
				int8 val1 = _wineRackGrid[cell + 11];

				if (val1 != 1 && (_wineRackGrid[cell + 1] == 2 || val1 == 2)) {
					candidates[depth] = cell + 1;
					depth++;
				}
			}
		} else if (_wineRackGrid[cell] == 2 || _wineRackGrid[pos4] == 2) {
			depth = 1;
			candidates[0] = pos4;
		}
	} else if (pos4 < 0 || _wineRackGrid[pos4] == 1) {
		if (_wineRackGrid[cell] == 2 || _wineRackGrid[pos3] == 2) {
			depth = 1;
			candidates[0] = pos3;
		}
	} else {
		depth = 2;
		candidates[0] = pos3;
		candidates[1] = pos4;
	}

	candidates[depth] = -1;
}

int8 WineRackGame::countEmtpy(int8 *moves) {
	int8 cnt = 0;

	for (int i = 0; i < moves[2]; i++) {
		if (!_wineRackGrid[moves[i + 3]])
			++cnt;
	}

	return 20 - cnt;
}

int8 WineRackGame::randomMoveStart() {
	const int8 moves[] = { 44, 45, 54, 55 };

	return moves[_random.getRandomNumber(3)];
}

int8 WineRackGame::randomMoveStart2() {
	const int8 moves[] = { 25, 26, 63, 64 };
	// the original game doesn't ensure the spot isn't taken
	int8 res = 0;
	do {
		res = moves[_random.getRandomNumber(3)];
	} while (_wineRackGrid[res] != 0);
	return res;
}

void WineRackGame::testWinCondition(byte player, int baseX, int baseY) {
	initGrid(2);

	int basePos = baseX * 10 + baseY;

	for (int i = 0; i < 10; i++) {
		if (player == kWineBottlePlayer)
			placeBottle(i * 10 + basePos, player);
		else
			placeBottle(i + basePos, player);
	}

	if (player == kWineBottlePlayer && !didPlayerWin()) {
		error("WineRackGame::testWinCondition(%d, %d, %d) failed", (int)player, baseX, baseY);
	} else if (player == kWineBottleOpponent && !didAiWin()) {
		error("WineRackGame::testWinCondition(%d, %d, %d) failed", (int)player, baseX, baseY);
	}
}

void WineRackGame::testGame(uint32 seed, Common::Array<int> moves, bool playerWin) {
	byte vars[1024] = {};
	byte &x = vars[0];
	byte &y = vars[1];
	byte &op = vars[3];
	byte &winner = vars[3];
	byte &difficulty = vars[4];
	_random.setSeed(seed);

	difficulty = 2;
	op = 3;
	run(vars);
	winner = 0;

	for (uint i = 0; i < moves.size(); i += 2) {
		if (winner)
			error("early winner");
		x = moves[i];
		y = moves[i + 1];
		op = 1;
		run(vars);
	}

	if (playerWin && winner != 2)
		error("WineRackGame::testGame(%u, %u, %d) player didn't win", seed, moves.size(), (int)playerWin);
	else if (playerWin == false && winner != 1)
		error("WineRackGame::testGame(%u, %u, %d) ai didn't win", seed, moves.size(), (int)playerWin);
}

void WineRackGame::runTests() {
	warning("WineRackGame::runTests() starting");
	uint32 oldSeed = _random.getSeed();

	for (int i = 0; i < 10; i++) {
		testWinCondition(kWineBottlePlayer, 0, i);
		testWinCondition(kWineBottleOpponent, i, 0);
	}

	// pairs of x,y for the player's moves
	testGame(1, {9,0, 9,1, 9,2, 9,3, 9,4, 9,5, 9,6, 9,7, 9,8, 9,9}, false);
	testGame(2, {5,5, 3,5, 7,4, 1,6, 9,3, 0,7, 2,6, 4,5, 6,5, 8,4}, true);
	// in the original game, the AI had a 25% chance to move to 2,6 as its first move, even if your first move was to 2,6 already
	testGame(147160395, {2,6, 3,6, 4,6, 5,6, 7,6, 8,6, 9,6, 2,7, 3,7, 4,7}, false);

	_random.setSeed(oldSeed);
	warning("WineRackGame::runTests() finished");
}

namespace {

const int8 wineRackLogicTable[1200] = {
	-1, -1,  1, 10, -1, -1, -1, -1, 11, -1, -1, -1,
	-1, -1,  2, 11, 10,  0, -1, -1, 12, 20, -1, -1,
	-1, -1,  3, 12, 11,  1, -1, -1, 13, 21, 10, -1,
	-1, -1,  4, 13, 12,  2, -1, -1, 14, 22, 11, -1,
	-1, -1,  5, 14, 13,  3, -1, -1, 15, 23, 12, -1,
	-1, -1,  6, 15, 14,  4, -1, -1, 16, 24, 13, -1,
	-1, -1,  7, 16, 15,  5, -1, -1, 17, 25, 14, -1,
	-1, -1,  8, 17, 16,  6, -1, -1, 18, 26, 15, -1,
	-1, -1,  9, 18, 17,  7, -1, -1, 19, 27, 16, -1,
	-1, -1, -1, 19, 18,  8, -1, -1, -1, 28, 17, -1,
	 0,  1, 11, 20, -1, -1, -1,  2, 21, -1, -1, -1,
	 1,  2, 12, 21, 20, 10, -1,  3, 22, 30, -1,  0,
	 2,  3, 13, 22, 21, 11, -1,  4, 23, 31, 20,  1,
	 3,  4, 14, 23, 22, 12, -1,  5, 24, 32, 21,  2,
	 4,  5, 15, 24, 23, 13, -1,  6, 25, 33, 22,  3,
	 5,  6, 16, 25, 24, 14, -1,  7, 26, 34, 23,  4,
	 6,  7, 17, 26, 25, 15, -1,  8, 27, 35, 24,  5,
	 7,  8, 18, 27, 26, 16, -1,  9, 28, 36, 25,  6,
	 8,  9, 19, 28, 27, 17, -1, -1, 29, 37, 26,  7,
	 9, -1, -1, 29, 28, 18, -1, -1, -1, 38, 27,  8,
	10, 11, 21, 30, -1, -1,  1, 12, 31, -1, -1, -1,
	11, 12, 22, 31, 30, 20,  2, 13, 32, 40, -1, 10,
	12, 13, 23, 32, 31, 21,  3, 14, 33, 41, 30, 11,
	13, 14, 24, 33, 32, 22,  4, 15, 34, 42, 31, 12,
	14, 15, 25, 34, 33, 23,  5, 16, 35, 43, 32, 13,
	15, 16, 26, 35, 34, 24,  6, 17, 36, 44, 33, 14,
	16, 17, 27, 36, 35, 25,  7, 18, 37, 45, 34, 15,
	17, 18, 28, 37, 36, 26,  8, 19, 38, 46, 35, 16,
	18, 19, 29, 38, 37, 27,  9, -1, 39, 47, 36, 17,
	19, -1, -1, 39, 38, 28, -1, -1, -1, 48, 37, 18,
	20, 21, 31, 40, -1, -1, 11, 22, 41, -1, -1, -1,
	21, 22, 32, 41, 40, 30, 12, 23, 42, 50, -1, 20,
	22, 23, 33, 42, 41, 31, 13, 24, 43, 51, 40, 21,
	23, 24, 34, 43, 42, 32, 14, 25, 44, 52, 41, 22,
	24, 25, 35, 44, 43, 33, 15, 26, 45, 53, 42, 23,
	25, 26, 36, 45, 44, 34, 16, 27, 46, 54, 43, 24,
	26, 27, 37, 46, 45, 35, 17, 28, 47, 55, 44, 25,
	27, 28, 38, 47, 46, 36, 18, 29, 48, 56, 45, 26,
	28, 29, 39, 48, 47, 37, 19, -1, 49, 57, 46, 27,
	29, -1, -1, 49, 48, 38, -1, -1, -1, 58, 47, 28,
	30, 31, 41, 50, -1, -1, 21, 32, 51, -1, -1, -1,
	31, 32, 42, 51, 50, 40, 22, 33, 52, 60, -1, 30,
	32, 33, 43, 52, 51, 41, 23, 34, 53, 61, 50, 31,
	33, 34, 44, 53, 52, 42, 24, 35, 54, 62, 51, 32,
	34, 35, 45, 54, 53, 43, 25, 36, 55, 63, 52, 33,
	35, 36, 46, 55, 54, 44, 26, 37, 56, 64, 53, 34,
	36, 37, 47, 56, 55, 45, 27, 38, 57, 65, 54, 35,
	37, 38, 48, 57, 56, 46, 28, 39, 58, 66, 55, 36,
	38, 39, 49, 58, 57, 47, 29, -1, 59, 67, 56, 37,
	39, -1, -1, 59, 58, 48, -1, -1, -1, 68, 57, 38,
	40, 41, 51, 60, -1, -1, 31, 42, 61, -1, -1, -1,
	41, 42, 52, 61, 60, 50, 32, 43, 62, 70, -1, 40,
	42, 43, 53, 62, 61, 51, 33, 44, 63, 71, 60, 41,
	43, 44, 54, 63, 62, 52, 34, 45, 64, 72, 61, 42,
	44, 45, 55, 64, 63, 53, 35, 46, 65, 73, 62, 43,
	45, 46, 56, 65, 64, 54, 36, 47, 66, 74, 63, 44,
	46, 47, 57, 66, 65, 55, 37, 48, 67, 75, 64, 45,
	47, 48, 58, 67, 66, 56, 38, 49, 68, 76, 65, 46,
	48, 49, 59, 68, 67, 57, 39, -1, 69, 77, 66, 47,
	49, -1, -1, 69, 68, 58, -1, -1, -1, 78, 67, 48,
	50, 51, 61, 70, -1, -1, 41, 52, 71, -1, -1, -1,
	51, 52, 62, 71, 70, 60, 42, 53, 72, 80, -1, 50,
	52, 53, 63, 72, 71, 61, 43, 54, 73, 81, 70, 51,
	53, 54, 64, 73, 72, 62, 44, 55, 74, 82, 71, 52,
	54, 55, 65, 74, 73, 63, 45, 56, 75, 83, 72, 53,
	55, 56, 66, 75, 74, 64, 46, 57, 76, 84, 73, 54,
	56, 57, 67, 76, 75, 65, 47, 58, 77, 85, 74, 55,
	57, 58, 68, 77, 76, 66, 48, 59, 78, 86, 75, 56,
	58, 59, 69, 78, 77, 67, 49, -1, 79, 87, 76, 57,
	59, -1, -1, 79, 78, 68, -1, -1, -1, 88, 77, 58,
	60, 61, 71, 80, -1, -1, 51, 62, 81, -1, -1, -1,
	61, 62, 72, 81, 80, 70, 52, 63, 82, 90, -1, 60,
	62, 63, 73, 82, 81, 71, 53, 64, 83, 91, 80, 61,
	63, 64, 74, 83, 82, 72, 54, 65, 84, 92, 81, 62,
	64, 65, 75, 84, 83, 73, 55, 66, 85, 93, 82, 63,
	65, 66, 76, 85, 84, 74, 56, 67, 86, 94, 83, 64,
	66, 67, 77, 86, 85, 75, 57, 68, 87, 95, 84, 65,
	67, 68, 78, 87, 86, 76, 58, 69, 88, 96, 85, 66,
	68, 69, 79, 88, 87, 77, 59, -1, 89, 97, 86, 67,
	69, -1, -1, 89, 88, 78, -1, -1, -1, 98, 87, 68,
	70, 71, 81, 90, -1, -1, 61, 72, 91, -1, -1, -1,
	71, 72, 82, 91, 90, 80, 62, 73, 92, -1, -1, 70,
	72, 73, 83, 92, 91, 81, 63, 74, 93, -1, 90, 71,
	73, 74, 84, 93, 92, 82, 64, 75, 94, -1, 91, 72,
	74, 75, 85, 94, 93, 83, 65, 76, 95, -1, 92, 73,
	75, 76, 86, 95, 94, 84, 66, 77, 96, -1, 93, 74,
	76, 77, 87, 96, 95, 85, 67, 78, 97, -1, 94, 75,
	77, 78, 88, 97, 96, 86, 68, 79, 98, -1, 95, 76,
	78, 79, 89, 98, 97, 87, 69, -1, 99, -1, 96, 77,
	79, -1, -1, 99, 98, 88, -1, -1, -1, -1, 97, 78,
	80, 81, 91, -1, -1, -1, 71, 82, -1, -1, -1, -1,
	81, 82, 92, -1, -1, 90, 72, 83, -1, -1, -1, 80,
	82, 83, 93, -1, -1, 91, 73, 84, -1, -1, -1, 81,
	83, 84, 94, -1, -1, 92, 74, 85, -1, -1, -1, 82,
	84, 85, 95, -1, -1, 93, 75, 86, -1, -1, -1, 83,
	85, 86, 96, -1, -1, 94, 76, 87, -1, -1, -1, 84,
	86, 87, 97, -1, -1, 95, 77, 88, -1, -1, -1, 85,
	87, 88, 98, -1, -1, 96, 78, 89, -1, -1, -1, 86,
	88, 89, 99, -1, -1, 97, 79, -1, -1, -1, -1, 87,
	89, -1, -1, -1, -1, 98, -1, -1, -1, -1, -1, 88
};

} // End of anonymous namespace

} // End of Groovie namespace
