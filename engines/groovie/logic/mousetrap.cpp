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
	_mouseTrapCounter = _mouseTrapCounter1 = 0;
	_mouseTrapX = _mouseTrapY = 0;
	memset(_mouseTrapRoute, 0, 75);
	memset(_mouseTrapRouteCopy, 0, 76);
	_mouseTrapPosX = _mouseTrapPosY = 0;
	memset(_mouseTrapCells, 0, 31);
	_mouseTrapNumSteps = 0;
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

static const int8 mouseTrapLookup[] = {
	1, 0, 3, 0, 0, 1, 0, 3, 1, 4, 3, 4, 4, 1, 4, 3
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

void MouseTrapGame::copyRoute(int8 x, int8 y) {
	int i;

	for (i = 0; i < _mouseTrapCounter > i; i++) {
		if (_mouseTrapRoute[3 * i] == x && _mouseTrapRoute[3 * i + 1] == y )
			break;
	}

	_mouseTrapCounter1 = 0;

	do {
		_mouseTrapRouteCopy[3 * _mouseTrapCounter1 + 0] = _mouseTrapRoute[3 * i + 0];
		_mouseTrapRouteCopy[3 * _mouseTrapCounter1 + 1] = _mouseTrapRoute[3 * i + 1];
		_mouseTrapRouteCopy[3 * _mouseTrapCounter1 + 2] = _mouseTrapRoute[3 * i + 2];

		_mouseTrapCounter1++;

		i = _mouseTrapRoute[3 * i + 2];
	} while (i);
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
	int8 pos = _mouseTrapPosY + 5 * _mouseTrapPosX;	// coordinates swapped?
	int8 val = _mouseTrapCells[pos + 5];

	return ((val & 1) != 0 && _mouseTrapPosX && (_mouseTrapCells[pos] & 4) != 0)
		|| ((val & 4) != 0 && _mouseTrapPosX < 4 && (_mouseTrapCells[pos + 10] & 1) != 0)
		|| ((val & 8) != 0 && _mouseTrapPosY < 4 && (_mouseTrapCells[pos + 6] & 2) != 0)
		|| ((val & 2) != 0 && _mouseTrapPosY && (_mouseTrapCells[pos + 4] & 8) != 0);
}

bool MouseTrapGame::havePosInRoute(int8 x, int8 y) {
	for (int i = 0; i < _mouseTrapCounter; i++) {
		if (_mouseTrapRoute[3 * i] == x && _mouseTrapRoute[3 * i + 1] == y)
			return true;
	}

	return false;
}

void MouseTrapGame::addToRoute(int8 x, int8 y, int8 num) {
	if (!havePosInRoute(x, y)) {
		_mouseTrapRoute[3 * _mouseTrapCounter] = x;
		_mouseTrapRoute[3 * _mouseTrapCounter + 1] = y;
		_mouseTrapRoute[3 * _mouseTrapCounter + 2] = num;

		_mouseTrapCounter++;
	}
}

void MouseTrapGame::updateRoute() {
	_mouseTrapCounter = 0;

	addToRoute(_mouseTrapPosX, _mouseTrapPosY, 0);

	int prevCounter = 0;

	do {
		prevCounter = _mouseTrapCounter;

		for (int i = prevCounter; i < _mouseTrapCounter; i++) {
			int8 y1 = _mouseTrapRoute[3 * i + 1];
			int8 x1 = _mouseTrapRoute[3 * i];
			int8 pos = 5 * x1 + y1;
			int8 mask = _mouseTrapCells[pos + 5];

			if ((mask & 1) != 0 && x1 && (_mouseTrapCells[pos] & 4) != 0)
				addToRoute(x1 - 1, y1, i);
			if ((mask & 4) != 0 && x1 < 4 && (_mouseTrapCells[pos + 10] & 1) != 0)
				addToRoute(x1 + 1, y1, i);
			if ((mask & 8) != 0 && y1 < 4 && (_mouseTrapCells[pos + 6] & 2) != 0)
				addToRoute(x1, y1 + 1, i);
			if ((mask & 2) != 0 && y1 && (_mouseTrapCells[pos + 4] & 8) != 0)
				addToRoute(x1, y1 - 1, i);
		}
	} while (_mouseTrapCounter != prevCounter);
}

void MouseTrapGame::popLastStep(int8 *x, int8 *y) {
	_mouseTrapCounter1--;

	*x = _mouseTrapRouteCopy[3 * _mouseTrapCounter1];
	*y = _mouseTrapRouteCopy[3 * _mouseTrapCounter1 + 1];
}

void MouseTrapGame::goFarthest(int8 *x, int8 *y) {
	int8 origX = _mouseTrapX;
	int8 origY = _mouseTrapY;
	int8 maxVal = 0;
	int8 maxX = 0, maxY = 0;

	if (_mouseTrapNumSteps)
		--_mouseTrapNumSteps;

	for (int8 i = 0; i < 8; i++) {
		int8 x1 = mouseTrapLookup[2 * i];
		int8 y1 = mouseTrapLookup[2 * i + 1];
		if (x1 != origX || y1 != origY) {
			flipField(x1, y1);

			int8 dist = calcDistanceToExit();

			if (_mouseTrapNumSteps && _random.getRandomNumber(1) != 0 )
				dist += 3;

			if (dist >= maxVal) {
				maxVal = dist;
				maxX = x1;
				maxY = y1;
			}

			flipField(mouseTrapLookup[2 * ((i + 4) & 7)], mouseTrapLookup[2 * ((i + 4) & 7) + 1]);
		}
	}

	*x = maxX;
	*y = maxY;
}

void MouseTrapGame::findMinPointInRoute(int8 *x, int8 *y) {
	int8 maxVal = 0;
	int8 x1 = _mouseTrapPosX;
	int8 y1 = _mouseTrapPosY;
	for (int i = 0; i < _mouseTrapCounter > i; i++) {
		if (8 - _mouseTrapRoute[3 * i + 1] - _mouseTrapRoute[3 * i] > maxVal) {
			maxVal = 8 - _mouseTrapRoute[3 * i + 1] - _mouseTrapRoute[3 * i];
			x1 = _mouseTrapRoute[3 * i];
			y1 = _mouseTrapRoute[3 * i + 1];
		}
	}
	*x = x1;
	*y = y1;
}

int8 MouseTrapGame::calcDistanceToExit() {
	int8 maxDist = 0;

	updateRoute();
	if (havePosInRoute(4, 4))
		return 0;

	for (int i = 0; i < _mouseTrapCounter > i; i++) {
		if (8 - _mouseTrapRoute[3 * i + 1] - _mouseTrapRoute[3 * i] > maxDist)
			maxDist = 8 - _mouseTrapRoute[3 * i + 1] - _mouseTrapRoute[3 * i];
	}

	return maxDist;
}

void MouseTrapGame::getBestDirection(int8 *x, int8 *y) {
	int8 maxVal = 0;
	int8 origX = _mouseTrapX;
	int8 origY = _mouseTrapY;
	_mouseTrapNumSteps = 8;
	int8 maxX = 0, maxY = 0;

	for (int i = 0; i < 8; i++) {
		int x1 = mouseTrapLookup[2 * i];
		int y1 = mouseTrapLookup[2 * i + 1];

		if (origX != x1 || origY != y1) {
			flipField(x1, y1);

			int8 maxInRoute = findMaxInRoute();
			if (maxInRoute >= maxVal) {
				maxVal = maxInRoute;
				maxX = x1;
				maxY = y1;
			}

			flipField(mouseTrapLookup[2 * ((i + 4) & 7)], mouseTrapLookup[2 * ((i + 4) & 7) + 1]);
		}
	}

	*x = maxX;
	*y = maxY;
}

void MouseTrapGame::findMaxPointInRoute(int8 *x, int8 *y) {
	int8 maxVal = 0;
	int8 x1 = _mouseTrapPosX;
	int8 y1 = _mouseTrapPosY;

	updateRoute();

	for (int i = 0; i < _mouseTrapCounter; i++) {
		if (_mouseTrapRoute[3 * i] + _mouseTrapRoute[3 * i + 1] > maxVal) {
			maxVal = _mouseTrapRoute[3 * i] + _mouseTrapRoute[3 * i + 1];
			x1 = _mouseTrapRoute[3 * i];
			y1 = _mouseTrapRoute[3 * i + 1];
		}
	}

	*x = x1;
	*y = y1;
}

int8 MouseTrapGame::findMaxInRoute() {
	updateRoute();

	if (havePosInRoute(0, 0))
		return 0;

	int8 maxCoords = 0;

	for (int i = 0; i < _mouseTrapCounter; i++) {
		if (_mouseTrapRoute[3 * i] + _mouseTrapRoute[3 * i + 1] > maxCoords )
			maxCoords = _mouseTrapRoute[3 * i] + _mouseTrapRoute[3 * i + 1];
		}

	return maxCoords;
}

} // End of Groovie namespace
