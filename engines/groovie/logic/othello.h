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

#ifndef GROOVIE_LOGIC_OTHELLO_H
#define GROOVIE_LOGIC_OTHELLO_H

#include "common/random.h"
#include "common/system.h"

namespace Groovie {

/*
 * Othello/Reversi Cursed Coins puzzle in Clandestiny and UHP.
 */
struct Freeboard {
	int _score;
	byte _boardstate[8][8]; // 0 is empty, 1 is player, 2 is AI

	// for sorting an array of pointers
	friend bool operator<(const Freeboard &a, const Freeboard &b) {
		return a._score > b._score;
	}
};

class OthelloGame {
public:
	OthelloGame(bool easierAi);
	void run(byte *scriptVariables);

private:
	int scoreEdge(byte (&board)[8][8], int x, int y, int slopeX, int slopeY);
	int scoreEarlyGame(Freeboard *freeboard);
	int scoreLateGame(Freeboard *freeboard);
	int scoreBoard(Freeboard *board);
	void restart(void);
	void writeBoardToVars(Freeboard *board, byte *vars);
	void readBoardStateFromVars(byte *vars);
	Freeboard getPossibleMove(Freeboard *freeboard, int moveSpot);
	void checkPossibleMove(Freeboard *board, Freeboard (&boards)[30], int8 **lineSpot, int &numPossibleMoves, int moveSpot, byte player, byte opponent);
	int getAllPossibleMoves(Freeboard *board, Freeboard (&boards)[30]);
	int aiRecurse(Freeboard *board, int depth, int parentScore, int opponentBestScore);
	byte aiDoBestMove(Freeboard *pBoard);
	void initLines(void);
	uint makeMove(Freeboard *freeboard, uint8 x, uint8 y);
	byte getLeader(Freeboard *f);
	void opInit(byte *vars);
	void tickBoard();
	void opPlayerMove(byte *vars);
	void op3(byte *vars);
	void opAiMove(byte *vars);
	void op5(byte *vars);

	void test();
	void testMatch(Common::Array<int> moves, bool playerWin);

	Common::RandomSource _random;
	byte _flag1;
	int8 _flag2;
	const int _depths[60];
	int _counter;
	const int _movesLateGame;    // this is 52, seems to be a marker of when to change the function pointer to an aleternate scoring algorithm for the late game
	bool _isLateGame;      // used to choose the scoring function, true means scoreLateGame
	const int8 _lookupPlayer[3]; // used to convert from internal values that represent piece colors to what the script uses in vars, {21, 40, 31}
	const int8 _scores[3][4];
	const int8 _edgesScores[112];
	const int _cornersScores[105];
	int _isAiTurn;
	int8 **_lines[64];
	int8 *_linesStorage[484];
	int8 _lineStorage[2016];
	Freeboard _board;
	bool _easierAi;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_OTHELLO_H
