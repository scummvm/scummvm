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
#include "groovie/logic/cake.h"

namespace Groovie {

/*
* T11hCake() constructor
*	- Each spot on the board is part of multiple potential victory lines
*	- The first x and y dimensions of the loops select the origin point of the line
*	- The z is for the distance along that line
*	- Then we push_back the id number of the line into the array at _map.indecies[x][y]
*	- This is used in UpdateScores()
*	.
* @see UpdateScores()
*/
CakeGame::CakeGame(bool easierAi) : _random("CakeGame") {
	restart();

	_map = {};
	int numLines = 0;

	// map all the lines with slope of (1, 0)
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				setLineNum(x + z, y, numLines);
			}
			numLines++;
		}
	}

	// map all the lines with slope of (0, 1)
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y <= HEIGHT - GOAL_LEN; y++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				setLineNum(x, y + z, numLines);
			}
			numLines++;
		}
	}

	// map all the lines with slope of (1,1)
	for (int y = 0; y <= HEIGHT - GOAL_LEN; y++) {
		for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				setLineNum(x + z, y + z, numLines);
			}
			numLines++;
		}
	}

	// map all the lines with slope of (1,-1)
	for (int y = GOAL_LEN - 1; y < HEIGHT; y++) {
		for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				setLineNum(x + z, y - z, numLines);
			}
			numLines++;
		}
	}

#if 0
	_easierAi = false;
	testCake();
#endif

	_easierAi = easierAi;
}

void CakeGame::run(byte *scriptVariables) {
	byte &lastMove = scriptVariables[1];
	byte &winner = scriptVariables[3];
	winner = 0;

	if (lastMove == 8) {
		restart();
		return;
	}

	if (lastMove == 9) {
		// samantha makes a move
		lastMove = aiGetBestMove(6);
		_hasCheated = true;
		return;
	}

	if (isColumnFull(lastMove)) {
		warning("player tried to place a bon bon in a full column, last_move: %d", (int)lastMove);
		lastMove = 10;
		return;
	}

	placeBonBon(lastMove);
	winner = getWinner();
	if (winner) {
		return;
	}

	int depth = 4 + (_hasCheated == false);
	if (_easierAi && _moveCount > 8)
		depth = 3;
	else if (_easierAi)
		depth = 2;

	lastMove = aiGetBestMove(depth);
	placeBonBon(lastMove);
	if (gameEnded()) {
		winner = STAUF;
	}
}

void CakeGame::restart() {
	_playerProgress = {};
	_staufProgress = {};
	memset(_boardState, 0, sizeof(_boardState));
	memset(_columnHeights, 0, sizeof(_columnHeights));
	_moveCount = 0;
	_hasCheated = false;

	_playerProgress._score = NUM_LINES;
	_staufProgress._score = NUM_LINES;
}

void CakeGame::setLineNum(uint x, uint y, uint index) {
	assert(x < WIDTH);
	assert(y < HEIGHT);
	byte slot = _map.lengths[x][y]++;
	assert(slot < GOAL_LEN * GOAL_LEN);
	assert(index < NUM_LINES);
	_map.indecies[x][y][slot] = index;
}

bool CakeGame::isColumnFull(byte column) {
	return _columnHeights[column] >= HEIGHT;
}

CakeGame::PlayerProgress &CakeGame::getPlayerProgress(bool stauf) {
	if (stauf)
		return _staufProgress;
	else
		return _playerProgress;
}

/*
* UpdateScores()
*	- Each PlayerProgress has an array of ints, _linesCounters[], where each entry maps to the ID of a line
*	- When a bon bon is added to the board, we look up _map.lengths[x][y] and then loop through all the indecies for that point
*		- Increment the PlayerProgress._linesCounters[id]
*		- Calculate the scores proportional to the PlayerProgress._linesCounters[id]
*		.
*	.
*/
void CakeGame::updateScores(byte x, bool revert) {
	bool stauf = _moveCount % 2;
	PlayerProgress &pp = getPlayerProgress(stauf);

	byte y = _columnHeights[x] - 1;

	// get the number of potential victory lines that this spot exists in
	int num_lines = _map.lengths[x][y];

	for (int line = 0; line < num_lines; line++) {
		// get the ID for this potential victory line
		int index = _map.indecies[x][y][line];
		int len = pp._linesCounters[index];

		// add this new bon bon to the progress of this potential victory line, or remove in the case of revert
		int mult = 1;// mult is used for multiplying the score gains, depends on revert
		if (!revert)
			pp._linesCounters[index]++;
		else {
			len = --pp._linesCounters[index];
			mult = -1;
		}

		if (GOAL_LEN == len + 1) {
			// that's a bingo
			pp._score += WIN_SCORE * mult;
		}
		else {
			PlayerProgress &pp2 = getPlayerProgress(!stauf);
			int len2 = pp2._linesCounters[index];
			if (len == 0) {
				// we started a new line, take away the points the opponent had from this line since we ruined it for them
				pp2._score -= (1 << (len2 & 31)) * mult;
			}
			if (len2 == 0) {
				// the opponent doesn't have any spots in this line, so we get points for it
				pp._score += (1 << (len & 31)) * mult;
			}
		}
	}
}

void CakeGame::placeBonBon(byte x) {
	byte y = _columnHeights[x]++;
	if (_moveCount % 2)
		_boardState[x][y] = STAUF;
	else
		_boardState[x][y] = PLAYER;

	updateScores(x);

	_moveCount++;
}

void CakeGame::revertMove(byte x) {
	// PlaceBonBon in reverse, this is used for the AI's recursion rollback
	_moveCount--;

	updateScores(x, true);

	byte y = --_columnHeights[x];
	_boardState[x][y] = 0;
}

byte CakeGame::getWinner() {
	if (_playerProgress._score >= WIN_SCORE)
		return PLAYER;

	if (_staufProgress._score >= WIN_SCORE)
		return STAUF;

	return 0;
}

bool CakeGame::gameEnded() {
	if (getWinner())
		return true;

	if (_moveCount >= WIDTH * HEIGHT)
		return true;

	return false;
}

int CakeGame::getScoreDiff() {
	if (_moveCount % 2)
		return _staufProgress._score - _playerProgress._score;
	else
		return _playerProgress._score - _staufProgress._score;
}

int CakeGame::aiRecurse(int search_depth, int parent_score) {
	int best_score = 0x7fffffff;

	for (byte move = 0; move < WIDTH; move++) {
		if (isColumnFull(move))
			continue;

		placeBonBon(move);
		int score = getScoreDiff();
		if (search_depth > 1 && !gameEnded())
			score = aiRecurse(search_depth - 1, best_score);
		revertMove(move);

		if (score < best_score)
			best_score = score;

		if (-parent_score != best_score && parent_score <= -best_score)
			break;
	}

	// we negate the score because from the perspective of our parent caller, this is his opponent's score
	return -best_score;
}

byte CakeGame::aiGetBestMove(int search_depth) {
	int best_move = 0xffff;
	uint counter = 1;

	for (int best_score = 0x7fffffff; best_score > 999999 && search_depth > 1; search_depth--) {
		for (byte move = 0; move < WIDTH; move++) {
			if (isColumnFull(move))
				continue;

			placeBonBon(move);
			if (getWinner()) {
				revertMove(move);
				return move;
			}

			int score = aiRecurse(search_depth - 1, best_score);
			revertMove(move);
			if (score < best_score) {
				counter = 1;
				best_move = move;
				best_score = score;
			} else if (best_score == score) {
				// rng is only used on moves with equal scores
				counter++;
				uint r = _random.getRandomNumber(1000000 - 1);
				if (r * counter < 1000000) {
					best_move = move;
				}
			}
		}
	}

	return best_move;
}

void CakeGame::testCake() {
	warning("starting CakeGame::testCake()");
	uint32 oldSeed = _random.getSeed();
	// test the draw condition, grouped by column
	runCakeTestNoAi(/*move 1*/ "7777777" /*8*/ "6666666" /*15*/ "5555555" /*22*/ "34444444" /*30*/ "333333" /*36*/ "2222222" /*43*/ "01111111" /*51*/ "000000", false, true);

	runCakeTest(9, "24223233041", true);
	runCakeTest(1, "232232432445", false);
	runCakeTest(123, "4453766355133466", false);

	_random.setSeed(oldSeed);
	warning("finished CakeGame::testCake()");
}

void CakeGame::runCakeTestNoAi(const char *moves, bool playerWin, bool draw = false) {
	warning("starting runCakeTestNoAi(%s, %d)", moves, (int)playerWin);

	restart();

	for (int i = 0; moves[i]; i++) {
		byte win = getWinner();
		if (win) {
			error("early win at %d, winner: %d", i, (int)win);
		}
		if (gameEnded()) {
			error("early draw at %d", i);
		}
		byte move = moves[i] - '0';
		placeBonBon(move);
	}

	byte winner = getWinner();
	if (draw) {
		if (winner != 0 || !gameEnded())
			error("wasn't a draw! winner: %d, gameover: %d", (int)winner, (int)gameEnded());
	} else if (playerWin && winner != PLAYER) {
		error("player didn't win! winner: %d", (int)winner);
	} else if (playerWin == false && winner != STAUF) {
		error("Stauf didn't win! winner: %d", (int)winner);
	}

	warning("finished runCakeTestNoAi(%s, %d), winner: %d", moves, (int)playerWin, (int)winner);
}

void CakeGame::runCakeTest(uint seed, const char *moves, bool playerWin) {
	warning("starting runCakeTest(%u, %s, %d)", seed, moves, (int)playerWin);

	// first fill the board with the expected moves and test the win-detection function by itself without AI
	runCakeTestNoAi(moves, playerWin);

	restart();

	byte vars[1024];
	memset(vars, 0, sizeof(vars));
	byte &lastMove = vars[1];
	byte &winner = vars[3];
	winner = 0;
	lastMove = 8;
	run(vars);

	uint old_seed = _random.getSeed();
	_random.setSeed(seed);

	for (int i = 0; moves[i]; i += 2) {
		if (winner != 0) {
			error("early win at %d, winner: %d", i, (int)winner);
		}
		lastMove = moves[i] - '0';
		byte stauf_move = moves[i + 1] - '0';

		run(vars);

		if (stauf_move < 8) {
			if (winner == 2) {
				error("early player win at %d", i);
			}

			if (stauf_move != lastMove) {
				error("incorrect Stauf move, expected: %d, got: %d", (int)stauf_move, (int)lastMove);
			}
		} else if (winner != 2) {
			error("missing Stauf move, last_move: %d", (int)lastMove);
		} else
			break;
	}

	if (playerWin && winner != 2) {
		error("player didn't win! winner: %d", (int)winner);
	} else if (playerWin == false && winner != 1) {
		error("Stauf didn't win! winner: %d", (int)winner);
	}

	_random.setSeed(old_seed);

	warning("finished runCakeTest(%u, %s, %d)", seed, moves, (int)playerWin);
}

} // End of Groovie namespace
