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

#ifndef GROOVIE_LOGIC_TRIANGLE_H
#define GROOVIE_LOGIC_TRIANGLE_H

#include "common/random.h"
#include "common/system.h"

namespace Groovie {

class TriangleGame {
public:
	TriangleGame();
	void run(byte *scriptVariables);

private:
	void init();
	int8 sub02();
	int8 sub03(int8 player);
	void sub05(int8 *triangleCells, int8 *a2, int8 *a3);
	void sub07(int8 *tempMoves, int8 *triangleCells, int8 *tempTriangle3, int8 *tempTriangle2, int8 *tempTriangle1, int8 *tempMoves2);
	int8 sub09(int8 key, int8 *a2, int8 *a3, int8 *a4, int8 *triangleCells);
	int8 sub10(int8 key, int8 *a2, int8 *triangleCells);
	int8 sub12(int8 a1, int8 *a2, int8 *triangleCells, int8 *a4);
	int sub13(int8 row, int8 *triangleCells, int8 *moves);
	void setCell(int8 cellnum, int8 val);
	void copyLogicRow(int row, int8 key, int8 *dest);
	void replaceCells(int8 *tempTriangle, int limit, int8 from, int8 to);
	int copyLookup(const int8 *lookup, int8 *start, int8 *dest);
	void collapseLoops(int8 *route, int8 *singleRow);

	bool testGame(uint32 seed, Common::Array<uint8> moves, bool player_win);
	void ensureSamanthaWin(uint32 seed);
	void testPlayRandomly(uint32 seed);
	void test();

private:
	int _triangleCellCount;
	int8 _triangleCells[66];

	Common::RandomSource _random;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_TRIANGLE_H
