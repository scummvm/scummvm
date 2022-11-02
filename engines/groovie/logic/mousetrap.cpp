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

#include "groovie/groovie.h"
#include "groovie/logic/mousetrap.h"

namespace Groovie {

MouseTrapGame::MouseTrapGame(bool easierAi) : _random("MouseTrapGame") {
	_mouseTrapCounter = _mouseTrapCounter1 = 0;
	_mouseTrapX = _mouseTrapY = 0;
	memset(_mouseTrapRoute, 0, 75);
	memset(_mouseTrapRouteCopy, 0, 76);
	_mouseTrapPosX = _mouseTrapPosY = 0;
	memset(_mouseTrapCells, 0, 31);
	_mouseTrapNumSteps = 0;
	_easierAi = easierAi;
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
	6, 12, 9, 3
};

static const int8 mouseTrapLookup[] = {
	1, 0, 3, 0, 0, 1, 0, 3, 1, 4, 3, 4, 4, 1, 4, 3
};

void MouseTrapGame::init() {
	int8 initState[8], initX[8], initY[8];

	initState[0] = 0;
	initState[1] = 1;
	initState[3] = 3;
	initState[2] = 2;
	initState[4] = 4;
	initState[5] = 5;
	initState[6] = 6;
	initState[7] = 7;
	initY[0] = 1;
	initY[1] = 3;
	initY[2] = 0;
	initY[3] = 4;
	initY[4] = 0;
	initY[5] = 4;
	initY[6] = 1;
	initY[7] = 3;
	initX[0] = 0;
	initX[1] = 0;
	initX[2] = 1;
	initX[3] = 1;
	initX[4] = 3;
	initX[5] = 3;
	initX[6] = 4;
	initX[7] = 4;

	for (int i = 7; i >= 0; i--) {
		int8 j = _random.getRandomNumber(i);

		_mouseTrapCells[5 * initY[i] + 5 + initX[i]] = mouseTrapStates[initState[j] >> 1];

		for (; j < i; j++) {
			initState[j] = initState[j + 1];
		}
	}

	_mouseTrapCells[11] = mouseTrapStates[3];
	_mouseTrapCells[16] = mouseTrapStates[0];
	_mouseTrapCells[5] = 12;
	_mouseTrapCells[21] = mouseTrapStates[0];
	_mouseTrapCells[12] = mouseTrapStates[3];
	_mouseTrapCells[15] = 13;
	_mouseTrapCells[25] = 9;
	_mouseTrapCells[22] = mouseTrapStates[1];
	_mouseTrapCells[13] = mouseTrapStates[2];
	_mouseTrapCells[18] = mouseTrapStates[2];
	_mouseTrapCells[23] = mouseTrapStates[1];
	_mouseTrapCells[7] = 14;
	_mouseTrapCells[17] = 15;
	_mouseTrapCells[27] = 11;
	_mouseTrapCells[9] = 6;
	_mouseTrapCells[19] = 7;
	_mouseTrapCells[29] = 3;
	_mouseTrapCells[30] = mouseTrapStates[_random.getRandomNumber(3)];

	_mouseTrapPosY = 2;
	_mouseTrapPosX = 2;
	_mouseTrapY = 0;
	_mouseTrapX = 0;
	_mouseTrapNumSteps = 0;
	_mouseTrapCounter = 0;
}

void MouseTrapGame::sub01(byte *scriptVariables) {
	int8 x, y;

	findMaxPointInRoute(&x, &y);
	scriptVariables[5] = (_mouseTrapPosX == x && _mouseTrapPosY == y) ? 1 : 0;
	if (havePosInRoute(4, 4)) {
		copyRoute(4, 4);
		scriptVariables[22] = 1;
	} else if (havePosInRoute(0, 0)) {
		copyRoute(0, 0);
		scriptVariables[22] = 2;
	} else {
		scriptVariables[22] = 0;
		if (!scriptVariables[5])
			copyRoute(x, y);
	}
}

void MouseTrapGame::sub03(byte *scriptVariables) {
	int cnt = 1;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			scriptVariables[cnt + 25] = findState(_mouseTrapCells[5 * j + 5 + i]);
			cnt++;
		}
	}
	scriptVariables[23] = findState(_mouseTrapCells[30]);
}

void MouseTrapGame::sub05(byte *scriptVariables) {
	int8 x, y;

	posToXY(scriptVariables[1] + 10 * scriptVariables[0], &x, &y);
	flipField(x, y);

	if (calcSolution()) {
		scriptVariables[5] = 0;
		updateRoute();

		if (havePosInRoute(4, 4)) {
			copyRoute(4, 4);
			scriptVariables[22] = 1;
		} else if (havePosInRoute(0, 0)) {
			copyRoute(0, 0);
			scriptVariables[22] = 2;
		} else {
			copyStateToVars(scriptVariables);
			scriptVariables[22] = 0;
		}
	} else {
		scriptVariables[5] = 1;
		scriptVariables[22] = 0;
	}
}

void MouseTrapGame::sub06(byte *scriptVariables) {
	int8 x, y;

	posToXY(10 * scriptVariables[0] + scriptVariables[1], &x, &y);
	copyRoute(x, y);
}

void MouseTrapGame::sub07(byte *scriptVariables) {
	int8 x1, y1, x2, y2;

	goFarthest(&x1, &y1);
	flipField(x1, y1);

	if (!calcSolution()) {
		scriptVariables[5] = 1;
		scriptVariables[22] = 0;
	} else {
		scriptVariables[5] = 0;
		updateRoute();

		if (!havePosInRoute(0, 0)) {
			if (havePosInRoute(4, 4)) {
				copyRoute(4, 4);
				scriptVariables[22] = 1;
			} else {
				findMinPointInRoute(&x2, &y2);

				if (_mouseTrapPosX != x2 || _mouseTrapPosY != y2) {
					copyRoute(x2, y2);
					scriptVariables[22] = 0;
				} else {
					scriptVariables[5] = 1;
					scriptVariables[22] = 0;
				}
			}
		} else {
			copyRoute(0, 0);
			scriptVariables[22] = 2;
		}
	}

	int8 pos = xyToPos(x1, y1);
	scriptVariables[0] = pos / 10;
	scriptVariables[1] = pos % 10;
}

void MouseTrapGame::sub08(byte *scriptVariables) {
	int8 x1, y1, x, y;

	popLastStep(&x1, &y1);
	int8 pos = xyToPos(x1, y1);

	_mouseTrapPosX = x1;
	_mouseTrapPosY = y1;

	scriptVariables[0] = scriptVariables[11];
	scriptVariables[1] = scriptVariables[12];
	scriptVariables[11] = pos / 10;
	scriptVariables[12] = pos % 10;
	posToXY(scriptVariables[1] + 10 * scriptVariables[0], &x, &y);

	if (y > y1) {
		scriptVariables[15] = 0;
	} else if (y < y1) {
		scriptVariables[15] = 2;
	} else if (x > x1) {
		scriptVariables[15] = 3;
	} else if (x < x1) {
		scriptVariables[15] = 1;
	}

	if (!_mouseTrapCounter1)
		scriptVariables[2] = 0;
}

void MouseTrapGame::sub09(byte *scriptVariables) {
	int8 x1, y1, x2, y2;

	getBestDirection(&x1, &y1);
	flipField(x1, y1);

	if (!calcSolution()) {
		scriptVariables[5] = 1;
		scriptVariables[22] = 0;
	} else {
		scriptVariables[5] = 0;
		updateRoute();

		if (!havePosInRoute(4, 4)) {
			if (havePosInRoute(0, 0)) {
				copyRoute(0, 0);
				scriptVariables[22] = 2;
			} else {
				findMaxPointInRoute(&x2, &y2);
				copyRoute(x2, y2);
				scriptVariables[22] = 0;
			}
		} else {
			copyRoute(4, 4);
			scriptVariables[22] = 1;
		}
	}

	int8 pos = xyToPos(x1, y1);
	scriptVariables[0] = pos / 10;
	scriptVariables[1] = pos % 10;
}

void MouseTrapGame::copyRoute(int8 x, int8 y) {
	int i;

	for (i = 0; i < _mouseTrapCounter; i++) {
		if (_mouseTrapRoute[3 * i] == x && _mouseTrapRoute[3 * i + 1] == y)
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
	int8 tmp;

	if (y) {
		if (y == 4) {
			if (x == 1) {
				tmp = _mouseTrapCells[10];
				_mouseTrapCells[10] = _mouseTrapCells[11];
				_mouseTrapCells[11] = _mouseTrapCells[12];
				_mouseTrapCells[12] = _mouseTrapCells[13];
				_mouseTrapCells[13] = _mouseTrapCells[14];
				_mouseTrapCells[14] = _mouseTrapCells[30];
				_mouseTrapCells[30] = tmp;
				_mouseTrapX = 1;
				_mouseTrapY = 0;
			} else if (x == 3) {
				tmp = _mouseTrapCells[20];
				_mouseTrapCells[20] = _mouseTrapCells[21];
				_mouseTrapCells[21] = _mouseTrapCells[22];
				_mouseTrapCells[22] = _mouseTrapCells[23];
				_mouseTrapCells[23] = _mouseTrapCells[24];
				_mouseTrapCells[24] = _mouseTrapCells[30];
				_mouseTrapCells[30] = tmp;
				_mouseTrapX = 3;
				_mouseTrapY = 0;
			}
		} else if (x) {
			if (x == 4) {
				if (y == 1) {
					tmp = _mouseTrapCells[6];
					_mouseTrapCells[6] = _mouseTrapCells[11];
					_mouseTrapCells[11] = _mouseTrapCells[16];
					_mouseTrapCells[16] = _mouseTrapCells[21];
					_mouseTrapCells[21] = _mouseTrapCells[26];
					_mouseTrapCells[26] = _mouseTrapCells[30];
					_mouseTrapCells[30] = tmp;
					_mouseTrapX = 0;
					_mouseTrapY = 1;
				} else if (y == 3) {
					tmp = _mouseTrapCells[8];
					_mouseTrapCells[8] = _mouseTrapCells[13];
					_mouseTrapCells[13] = _mouseTrapCells[18];
					_mouseTrapCells[18] = _mouseTrapCells[23];
					_mouseTrapCells[23] = _mouseTrapCells[28];
					_mouseTrapCells[28] = _mouseTrapCells[30];
					_mouseTrapCells[30] = tmp;
					_mouseTrapX = 0;
					_mouseTrapY = 3;
				}
			}
		} else if (y == 1) {
			tmp = _mouseTrapCells[26];
			_mouseTrapCells[26] = _mouseTrapCells[21];
			_mouseTrapCells[21] = _mouseTrapCells[16];
			_mouseTrapCells[16] = _mouseTrapCells[11];
			_mouseTrapCells[11] = _mouseTrapCells[6];
			_mouseTrapCells[6] = _mouseTrapCells[30];
			_mouseTrapCells[30] = tmp;
			_mouseTrapX = 4;
			_mouseTrapY = 1;
		} else if (y == 3) {
			tmp = _mouseTrapCells[28];
			_mouseTrapCells[28] = _mouseTrapCells[23];
			_mouseTrapCells[23] = _mouseTrapCells[18];
			_mouseTrapCells[18] = _mouseTrapCells[13];
			_mouseTrapCells[13] = _mouseTrapCells[8];
			_mouseTrapCells[8] = _mouseTrapCells[30];
			_mouseTrapCells[30] = tmp;
			_mouseTrapX = 4;
			_mouseTrapY = 3;
		}
	} else if (x == 1) {
		tmp = _mouseTrapCells[14];
		_mouseTrapCells[14] = _mouseTrapCells[13];
		_mouseTrapCells[13] = _mouseTrapCells[12];
		_mouseTrapCells[12] = _mouseTrapCells[11];
		_mouseTrapCells[11] = _mouseTrapCells[10];
		_mouseTrapCells[10] = _mouseTrapCells[30];
		_mouseTrapCells[30] = tmp;
		_mouseTrapX = 1;
		_mouseTrapY = 4;
	} else if (x == 3) {
		tmp = _mouseTrapCells[24];
		_mouseTrapCells[24] = _mouseTrapCells[23];
		_mouseTrapCells[23] = _mouseTrapCells[22];
		_mouseTrapCells[22] = _mouseTrapCells[21];
		_mouseTrapCells[21] = _mouseTrapCells[20];
		_mouseTrapCells[20] = _mouseTrapCells[30];
		_mouseTrapCells[30] = tmp;
		_mouseTrapX = 3;
		_mouseTrapY = 4;
	}
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

		prevCounter = _mouseTrapCounter;
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

			if (_easierAi)
				dist += _random.getRandomNumber(2);

			if (_mouseTrapNumSteps && _random.getRandomNumber(1) != 0)
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
	for (int i = 0; i < _mouseTrapCounter; i++) {
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

	for (int i = 0; i < _mouseTrapCounter; i++) {
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
		if (_mouseTrapRoute[3 * i] + _mouseTrapRoute[3 * i + 1] > maxCoords)
			maxCoords = _mouseTrapRoute[3 * i] + _mouseTrapRoute[3 * i + 1];
		}

	return maxCoords;
}

} // End of Groovie namespace
