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

int8 MouseTrapGame::xyToPos(int8 x, int8 y) {
	return 5 * y + x + 1;
}



} // End of Groovie namespace
