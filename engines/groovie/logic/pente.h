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
	PenteGame();
	void run(byte *vars);

private:
	void animateCapturesCheckWinner(byte *vars);
	void opQueryPiece(byte *vars);

	void addLine(int x, int y, int linesCounter);
	void buildLookupTable();
	void penteDeInit();
	void penteInit(uint width, uint height, uint length);
	uint &getPlayerTable(bool staufTurn, pentePlayerTable *&pt);
	void scoreLine(uint16 lineIndex, bool stauf_turn, bool revert);
	void calcTouchingPieces(byte moveX, byte moveY, bool revert);
	void updateScore(byte move_y, byte move_x, bool whose_turn);
	void revertScore(byte y, byte x);
	byte scoreCaptureSingle(byte x, byte y, int slopeX, int slopeY);
	uint scoreCapture(byte y, byte x);
	void animateCapture(short param_1, byte *param_2, short * outMove, short *param_4);
	void revertCapture(byte y, byte x, byte y2);
	int scoreMoveAndRevert(byte x, byte y, char depth, int parentScore, bool &gameOver);
	int scoreMoveAndRevert(byte x, byte y, char depth, int parent_score);
	int aiRecurseTail(int parent_score);
	int aiRecurse(char depth, int parent_score);
	uint16 aiGetBestMove(uint y_1, int param_2, int param_3, byte depth);
	void test();
	void testGame(uint32 seed, Common::Array<int> moves, bool playerWin);

	Common::RandomSource _random;

	byte _animateCapturesBitMask;
	short _previousMove;
	short _nextCapturedSpot;
	penteTable *_table;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_PENTE_H
