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
	void run(byte *scriptVariables);

private:
	int *allocs(int param_1, int param_2);
	void penteSub02Frees(penteTable *param_1);
	void penteSub05BuildLookupTable(penteTable *table);
	penteTable *penteSub01Init(byte width, byte height, byte length);
	void penteSub03Scoring(penteTable *table, byte move_y, byte move_x, bool whose_turn);
	void penteSub07RevertScore(penteTable *table_1, byte y, byte x);
	byte penteScoreCaptureSingle(penteTable *table, byte x, byte y, int slopeX, int slopeY);
	uint penteSub04ScoreCapture(penteTable *table, byte y, byte x);
	void penteSub08MaybeAnimateCapture(short param_1, byte *param_2, short *param_3, short *param_4);
	void penteSub11RevertCapture(penteTable *table, byte y, byte x, byte y2);
	int penteSub10AiRecurse(penteTable *table_1, char depth, int parent_score);
	uint penteSub09Ai(uint y_1, int param_2, int param_3, penteTable *table_4, byte depth);
	void penteOp(byte *vars);
	void test();
	void testGame(uint32 seed, Common::Array<int> moves, bool playerWin);

	Common::RandomSource _random;

	byte globalY;
	byte globalX;
	char global2;
	short globalPlayerMove;
	short global1;
	penteTable *game_state_table;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_PENTE_H
