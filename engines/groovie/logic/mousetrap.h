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

#ifndef GROOVIE_LOGIC_MOUSETRAP_H
#define GROOVIE_LOGIC_MOUSETRAP_H

#include "common/random.h"
#include "common/system.h"

namespace Groovie {

/*
 * Mouse Trap puzzle in the Lab.
 *
 * Stauf's Goal is space 1, counting up as you go north east
 * towards the north corner which is space 5 and the moveable
 * space to the left of that is space 4.
 * South east from Stauf's goal is the next line starting with
 * space 6, counting up as you go north east where the moveable
 * space to the right of the north corner is space 10
 *
 * Next line is 11 (unmovable) to 15 (unmoveable), this line
 * contains the center space which is space 13
 * Next line is 16 (moveable) to 20 (moveable)
 * Next line is 21 (unmovable) to 25 (unmovable), with 25 being
 * the player's goal door
 *
 * Space -2 is the next piece, outside of the box
 */
class MouseTrapGame {
public:
	MouseTrapGame(bool easierAi);
	void run(byte *scriptVariables);

private:
	void init();
	void sub01(byte *scriptVariables);
	void sub03(byte *scriptVariables);
	void sub05(byte *scriptVariables);
	void sub06(byte *scriptVariables);
	void sub07(byte *scriptVariables);
	void sub08(byte *scriptVariables);
	void sub09(byte *scriptVariables);
	void copyRoute(int8 x, int8 y);
	int8 xyToPos(int8 x, int8 y);
	void posToXY(int8 pos, int8 *x, int8 *y);
	void copyStateToVars(byte *scriptVariables);
	int8 findState(int8 val);
	void flipField(int8 x, int8 y);
	bool calcSolution();
	bool havePosInRoute(int8 y, int8 x);
	void addToRoute(int8 y, int8 x, int8 num);
	void updateRoute();
	void popLastStep(int8 *x, int8 *y);
	void goFarthest(int8 *x, int8 *y);
	void findMinPointInRoute(int8 *y, int8 *x);
	int8 calcDistanceToExit();
	void getBestDirection(int8 *x, int8 *y);
	void findMaxPointInRoute(int8 *x, int8 *y);
	int8 findMaxInRoute();


private:
	Common::RandomSource _random;
	int8 _mouseTrapX, _mouseTrapY;
	int8 _mouseTrapCounter, _mouseTrapCounter1;
	int8 _mouseTrapRoute[75];
	int8 _mouseTrapRouteCopy[76];
	int8 _mouseTrapCells[31];
	int8 _mouseTrapPosX, _mouseTrapPosY;
	int8 _mouseTrapNumSteps;
	bool _easierAi;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_MOUSETRAP_H
