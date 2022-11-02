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

#ifndef GROOVIE_LOGIC_CAKE_H
#define GROOVIE_LOGIC_CAKE_H

#include "common/random.h"
#include "common/system.h"

namespace Groovie {

/*
 * Cake (Connect Four) puzzle in the dining room (tb.grv)
 */
class CakeGame {
public:
	CakeGame(bool easierAi);
	void run(byte *scriptVariables);

private:
	static const int WIDTH = 8;
	static const int HEIGHT = 7;
	static const int GOAL_LEN = 4;
	static const int WIN_SCORE = 1000000;//!< the number of points added for a connect four
	static const byte STAUF = 1;
	static const byte PLAYER = 2;
	static const int NUM_LINES = 107;//!< how many potential victory lines there are

	Common::RandomSource _random;

	//! ID numbers for all of the potential victory lines for each spot on the board
	struct LinesMappings {
		byte lengths[WIDTH][HEIGHT];
		byte indecies[WIDTH][HEIGHT][GOAL_LEN * GOAL_LEN];
	};

	//! how many points a player has, and their progress on potential victory lines
	struct PlayerProgress {
		int _score;
		int _linesCounters[NUM_LINES];//!< how many pieces are claimed in each potential victory, links to LineMappings, an entry of 4 means that's a victory
	};

	PlayerProgress _playerProgress;
	PlayerProgress _staufProgress;

	byte _boardState[WIDTH][HEIGHT];//!< (0, 0) is the bottom left of the board
	byte _columnHeights[WIDTH];

	int _moveCount;
	bool _hasCheated;

	LinesMappings _map;//!< ID numbers for all of the potential victory lines for each spot on the board
	bool _easierAi;

	void restart();
	void setLineNum(uint x, uint y, uint index);
	bool isColumnFull(byte column);
	PlayerProgress &getPlayerProgress(bool stauf);
	void updateScores(byte x, bool revert = false);
	void placeBonBon(byte x);
	void revertMove(byte x);
	byte getWinner();
	bool gameEnded();
	int getScoreDiff();
	int aiRecurse(int search_depth, int parent_score);
	byte aiGetBestMove(int search_depth);
	void testCake();
	void runCakeTest(uint seed, const char *moves, bool player_win);
	void runCakeTestNoAi(const char *moves, bool player_win, bool draw);
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_CAKE_H
