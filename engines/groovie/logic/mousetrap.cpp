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

#include "groovie/groovie.h"
#include "groovie/logic/mousetrap.h"

namespace Groovie {

MouseTrapGame::MouseTrapGame() : _random("MouseTrapGame") {
	_mouseTrapCounter = 0;
	_mouseTrapX = _mouseTrapY = 0;
	memset(_mouseTrapRoute, 0, 75);
	_mouseTrapPos.x = _mouseTrapPos.y = 0;
	memset(_mouseTrapCells, 0, 31);
}

void MouseTrapGame::run(byte *scriptVariables) {
	byte op = scriptVariables[2];

	// variable 24 is the mouse?
	//scriptVariables[24] = 2;

	// player wins: scriptVariables[22] = 1;
	// stauf wins: scriptVariables[22] = 2;
	// allows the player to click to place the mouse somewhere? scriptVariables[5] = 0;

	switch (op) {
	case 0:
		sub01(scriptVariables);
		break;
	case 1: // init board
		// value of 0 is V, 1 is <, 2 is ^, 3 is >
		// variable 23 is the outside piece
		//scriptVariables[23] = _random.getRandomNumber(3);
		// variable slot is the space number + 25, the left corner
		// (Stauf's goal) is space 1, above that is space 2, the
		// center is 13, and the right corner (goal) is space 25
		init();
		sub03(scriptVariables);
		break;
	case 2: // before player chooses the floor to move, set the banned move
	{
		int clicked = xyToPos(_mouseTrapX, _mouseTrapY);
		scriptVariables[clicked + 50] = 0;
		break;
	}
	case 3: // after player moving floor
		// a bunch of hardcoded conditionals to copy variables and
		// set the banned move
		// this probably also sets a variable to allow the player to
		// move the mouse, and checks for win/lose
		sub05(scriptVariables);
		break;
	case 5: // maybe player moving mouse
		sub06(scriptVariables);
		break;
	case 6: // Stauf moving floor?
		sub07(scriptVariables);
		break;
	case 7: // maybe Stauf moving mouse
		sub08(scriptVariables);
		break;
	case 8: // Samantha making a move
		sub09(scriptVariables);
		break;

	default:
		warning("Unknown mousetrap op %d", op);
		break;
	}
}

static const int8 mouseTrapStates[] = {
	6, 12,  9,  3
};

void MouseTrapGame::init() {
}

void MouseTrapGame::sub01(byte *scriptVariables) {
}

void MouseTrapGame::sub03(byte *scriptVariables) {
}

void MouseTrapGame::sub05(byte *scriptVariables) {
}

void MouseTrapGame::sub06(byte *scriptVariables) {
}

void MouseTrapGame::sub07(byte *scriptVariables) {
}

void MouseTrapGame::sub08(byte *scriptVariables) {
}

void MouseTrapGame::sub09(byte *scriptVariables) {
}

void MouseTrapGame::sub11(int8 x, int8 y) {
}

int8 MouseTrapGame::xyToPos(int8 x, int8 y) {
	return 5 * y + x + 1;
}

void MouseTrapGame::posToXY(int8 pos, int8 *x, int8 *y) {
	*y = (pos - 1) / 5;
	*x = (pos - 1) % 5;
}

void MouseTrapGame::copyStateToVars(byte *scriptVariables) {
	memset(scriptVariables + 51, 0, 24);
	scriptVariables[75] = 0;

	for (int i = 0; i < _mouseTrapCounter; i++)
		scriptVariables[xyToPos(_mouseTrapRoute[3 * i], _mouseTrapRoute[3 * i + 1]) + 50] = 1;
}

int8 MouseTrapGame::findState(int8 val) {
  int8 result = 0;

  while (mouseTrapStates[result] != val) {
    if (++result >= 4)
      return -1;
  }
  return result;
}

void MouseTrapGame::flipField(int8 x, int8 y) {
}

bool MouseTrapGame::calcSolution() {
	int8 pos = _mouseTrapPos.x + 5 * _mouseTrapPos.y;
	int8 val = _mouseTrapCells[pos + 5];

	return ((val & 1) != 0 && _mouseTrapPos.y && (_mouseTrapCells[pos] & 4) != 0)
		|| ((val & 4) != 0 && _mouseTrapPos.y < 4 && (_mouseTrapCells[pos + 10] & 1) != 0)
		|| ((val & 8) != 0 && _mouseTrapPos.x < 4 && (_mouseTrapCells[pos + 6] & 2) != 0)
		|| ((val & 2) != 0 && _mouseTrapPos.x && (_mouseTrapCells[pos + 4] & 8) != 0);
}

bool MouseTrapGame::havePosInRoute(int8 y, int8 x) {
	return false;
}

void MouseTrapGame::addToRoute(int8 y, int8 x, int8 num) {
}

void MouseTrapGame::updateRoute() {
}

void MouseTrapGame::popLastStep(int8 *x, int8 *y) {
}

void MouseTrapGame::goFarthest(int8 *x, int8 *y) {
}

void MouseTrapGame::findMinPointInRoute(int8 *y, int8 *x) {
}

int8 MouseTrapGame::calcDistanceToExit() {
	return 0;
}

int8 MouseTrapGame::getBestDirection(int8 *x, int8 *y) {
	return 0;
}

int8 MouseTrapGame::findMaxPointInRoute(int8 *y, int8 *x) {
	return 0;
}

int8 MouseTrapGame::findMaxInRoute() {
	return 0;
}



} // End of Groovie namespace
