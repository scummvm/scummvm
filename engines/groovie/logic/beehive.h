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

#ifndef GROOVIE_LOGIC_BEEHIVE_H
#define GROOVIE_LOGIC_BEEHIVE_H

#include "common/system.h"

namespace Groovie {

/*
 * Beehive (Blood and Honey) puzzle (hs.grv)
 *
 * An infection-style game in which the player must cover more
 * territory than the computer. It's similar to the microscope puzzle
 * in the 7th Guest. The playfield is a honeycomb made of 61
 * hexagons. The hexagons are numbered starting from the top-left
 * corner, with a direction from bottom left to top right.
 */
class BeehiveGame {
public:
	BeehiveGame(bool easierAi) {
#if 0
		_easierAi = false;
		tests();
#endif
		_easierAi = easierAi;
	}
	~BeehiveGame() {}

	void run(byte *scriptVariables);

private:
	void sub02(int8 *a1, int8 *a2);
	void sub04(int8 a1, int8 a2, int8 *scriptVariables);
	void calcSamanthaMove(int8 *a1, int8 *a2, int8 *a3, int8 *a4, int8 *a5, int8 *a6);
	void calcStaufMove(int8 *a1, int8 *a2, int8 *a3, int8 *a4, int8 *a5, int8 *a6);
	int8 sub11(int8 *beehiveState, int8 *a2, int8 *a3, int8 *a4, int8 a5, int8 a6, int8 *a7);
	int8 sub12(int8 *beehiveState, int8 a2, int8 a3, int8 a4);
	int8 sub13(int8 *beehiveState, int8 a2, int8 a3);
	void sub15(int8 *beehiveState, int8 a2, int8 *a3);
	void sub16(int8 a1, int8 a2, int8 *a3, int8 *a4, int8 *a5);
	void sub17(int8 *beehiveState, int8 a2, int8 *a3, int8 *a4, int8 *a5);
	void selectSourceHexagon(int8 a1, int8 *a2, int8 *a3);
	int8 sub19(int8 a1, int8 a2);
	int8 getHexDifference();
	int8 getTotal(int8 *hexagons);
	int8 calcMove(int8 *beehiveState, int8 a2, int8 a3, int8 depth, int a5, int8 *a6);
	int8 findCell(int8 *beehiveState, int8 *pos, int8 key);
	void testGame(Common::Array<int> moves, bool playerWin);
	void tests();
	void overrideClick(byte *vars);
	void overrideMove(byte *vars);

	#define HEXCOUNT 61
	int8 _beehiveState[HEXCOUNT];

	Common::Array<int> overrideMoves;
	uint overrideIndex;
	bool _easierAi;
	int8 _maxDepth;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_BEEHIVE_H
