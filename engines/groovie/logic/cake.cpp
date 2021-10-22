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

#include <limits.h>
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
T11hCake::T11hCake(Common::RandomSource &rng) : _random(rng) {
	Restart();

	_map = {};
	int numLines = 0;

	// map all the lines with slope of (1, 0)
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				SetLineNum(x + z, y, numLines);
			}
			numLines++;
		}
	}

	// map all the lines with slope of (0, 1)
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y <= HEIGHT - GOAL_LEN; y++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				SetLineNum(x, y + z, numLines);
			}
			numLines++;
		}
	}

	// map all the lines with slope of (1,1)
	for (int y = 0; y <= HEIGHT - GOAL_LEN; y++) {
		for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				SetLineNum(x + z, y + z, numLines);
			}
			numLines++;
		}
	}

	// map all the lines with slope of (1,-1)
	for (int y = GOAL_LEN - 1; y < HEIGHT; y++) {
		for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
			for (int z = 0; z < GOAL_LEN; z++) {
				SetLineNum(x + z, y - z, numLines);
			}
			numLines++;
		}
	}
}

byte T11hCake::OpConnectFour(byte &lastMove) {
	if (lastMove == 8) {
		Restart();
		return 0;
	}

	if (lastMove == 9) {
		// samantha makes a move
		// TODO: fix graphical bug when samantha makes a move
		lastMove = AiGetBestMove(6);
		_hasCheated = true;
		return 0;
	}

	if (IsColumnFull(lastMove)) {
		warning("player tried to place a bon bon in a full column, last_move: %d", (int)lastMove);
		lastMove = 10;
		return 0;
	}

	PlaceBonBon(lastMove);
	byte winner = GetWinner();
	if (winner) {
		return winner;
	}

	lastMove = AiGetBestMove(4 + (_hasCheated == false));
	PlaceBonBon(lastMove);
	if (GameEnded())
		return STAUF;

	return 0;
}

void T11hCake::Restart() {
	_playerProgress = {};
	_staufProgress = {};
	memset(_boardState, 0, sizeof(_boardState));
	memset(_columnHeights, 0, sizeof(_columnHeights));
	_moveCount = 0;
	_hasCheated = false;

	_playerProgress._score = NUM_LINES;
	_staufProgress._score = NUM_LINES;
}

void T11hCake::SetLineNum(uint x, uint y, uint index) {
	assert(x < WIDTH);
	assert(y < HEIGHT);
	byte slot = _map.lengths[x][y]++;
	assert(slot < GOAL_LEN * GOAL_LEN);
	assert(index < NUM_LINES);
	_map.indecies[x][y][slot] = index;
}

bool T11hCake::IsColumnFull(byte column) {
	return _columnHeights[column] >= HEIGHT;
}

T11hCake::PlayerProgress &T11hCake::GetPlayerProgress(bool stauf) {
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
void T11hCake::UpdateScores(byte x, bool revert) {
	bool stauf = _moveCount % 2;
	PlayerProgress &pp = GetPlayerProgress(stauf);

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
			PlayerProgress &pp2 = GetPlayerProgress(!stauf);
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

void T11hCake::PlaceBonBon(byte x) {
	byte y = _columnHeights[x]++;
	if (_moveCount % 2)
		_boardState[x][y] = STAUF;
	else
		_boardState[x][y] = PLAYER;

	UpdateScores(x);

	_moveCount++;
}

void T11hCake::RevertMove(byte x) {
	// PlaceBonBon in reverse, this is used for the AI's recursion rollback
	_moveCount--;

	UpdateScores(x, true);

	byte y = --_columnHeights[x];
	_boardState[x][y] = 0;
}

byte T11hCake::GetWinner() {
	if (_playerProgress._score >= WIN_SCORE)
		return PLAYER;

	if (_staufProgress._score >= WIN_SCORE)
		return STAUF;

	return 0;
}

bool T11hCake::GameEnded() {
	if (GetWinner())
		return true;

	if (_moveCount >= WIDTH * HEIGHT)
		return true;

	return false;
}

int T11hCake::GetScoreDiff() {
	if (_moveCount % 2)
		return _staufProgress._score - _playerProgress._score;
	else
		return _playerProgress._score - _staufProgress._score;
}

int T11hCake::AiRecurse(int search_depth, int parent_score) {
	int best_score = 0x7fffffff;

	for (byte move = 0; move < WIDTH; move++) {
		if (IsColumnFull(move))
			continue;

		PlaceBonBon(move);
		int score = GetScoreDiff();
		if (search_depth > 1 && !GameEnded())
			score = AiRecurse(search_depth - 1, best_score);
		RevertMove(move);

		if (score < best_score)
			best_score = score;

		if (-parent_score != best_score && parent_score <= -best_score)
			break;
	}

	// we negate the score because from the perspective of our parent caller, this is his opponent's score
	return -best_score;
}

uint T11hCake::Rng() {
	return _random.getRandomNumber(UINT_MAX);
}

byte T11hCake::AiGetBestMove(int search_depth) {
	int best_move = 0xffff;
	uint counter = 1;

	for (int best_score = 0x7fffffff; best_score > 999999 && search_depth > 1; search_depth--) {
		for (byte move = 0; move < WIDTH; move++) {
			if (IsColumnFull(move))
				continue;

			PlaceBonBon(move);
			if (GetWinner()) {
				RevertMove(move);
				return move;
			}

			int score = AiRecurse(search_depth - 1, best_score);
			RevertMove(move);
			if (score < best_score) {
				counter = 1;
				best_move = move;
				best_score = score;
			} else if (best_score == score) {
				// rng is only used on moves with equal scores
				counter++;
				uint r = Rng() % 1000000;
				if (r * counter < 1000000) {
					best_move = move;
				}
			}
		}
	}

	return best_move;
}

} // End of Groovie namespace
