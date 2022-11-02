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

#ifndef GROOVIE_LOGIC_PENTE_H
#define GROOVIE_LOGIC_PENTE_H

#include "common/random.h"
#include "common/system.h"

namespace Groovie {

/*
 * Pente puzzle at the end of the game.
 */

struct pentePlayerTable;
struct penteTable;

class PenteGame {
public:
	PenteGame(bool easierAi);
	void run(byte *vars);

private:
	void animateCapturesCheckWinner(byte *vars);
	void opQueryPiece(byte *vars);

	void addLine(int x, int y, int linesCounter);
	void buildLookupTable();
	void penteDeInit();
	void penteInit(uint width, uint height, uint length);
	int &getPlayerTable(bool staufTurn, pentePlayerTable *&pt);
	void scoreLine(uint16 lineIndex, bool isStaufTurn, bool revert);
	void calcTouchingPieces(byte moveX, byte moveY, bool revert);
	void updateScore(byte x, byte y, bool whose_turn);
	void revertScore(byte x, byte y);
	byte scoreCaptureSingle(byte x, byte y, int slopeX, int slopeY);
	uint scoreCapture(byte x, byte y);
	void animateCapture(short move, byte *bitMaskG, short *outCapture1, short *outCapture2);
	void revertCapture(byte x, byte y, byte y2);
	int scoreMoveAndRevert(byte x, byte y, char depth, int parentScore, bool &gameOver);
	int scoreMoveAndRevert(byte x, byte y, char depth, int parentScore);
	int aiRecurseTail(int parentScore);
	int aiRecurse(char depth, int parentScore);
	uint16 aiGetBestMove(byte depth);
	void test();
	bool testGame(uint32 seed, Common::Array<int> moves, bool playerWin);
	void testRandomGame(uint32 seed);

	Common::RandomSource _random;

	byte _animateCapturesBitMask;
	short _previousMove;
	short _nextCapturedSpot;
	penteTable *_table;
	bool _easierAi;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_PENTE_H
