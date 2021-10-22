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

#include "groovie/logic/cake.h"
#include "groovie/logic/t11hgame.h"
#include "groovie/groovie.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/translation.h"

namespace Groovie {

T11hGame::T11hGame(byte *scriptVariables)
	: _random("GroovieT11hGame"), _scriptVariables(scriptVariables) {
}

T11hGame::~T11hGame() {
}

void T11hGame::handleOp(uint8 op) {
	switch (op) {
	case 1:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Connect four in the dining room. (tb.grv)", op);
		_cake.run(_scriptVariables);
		break;

	case 2:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Beehive Puzzle in the top room (hs.grv)", op);
		// NOTE: Reused in UHP
		_beehive.run(_scriptVariables);
		break;

	case 3:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Make last move on modern art picture in the gallery (bs.grv)", op);
		_gallery.run(_scriptVariables);
		break;

	case 4:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Triangle in the Chapel (tx.grv)", op);
		opTriangle();
		break;

	case 5:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Mouse Trap in the lab (al.grv)", op);
		// NOTE: Reused in UHP
		opMouseTrap();
		break;

	case 6:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Pente (pt.grv)", op);
		opPente();
		break;

	default:
		debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): T11H Invalid -> NOP", op);
	}
}

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
void T11hGame::opMouseTrap() {
	// TODO: Finish the logic
	byte op = _scriptVariables[2];

	warning("Mousetrap subop %d", op);

	// variable 24 is the mouse?
	//_scriptVariables[24] = 2;

	// player wins: _scriptVariables[22] = 1;
	// stauf wins: _scriptVariables[22] = 2;
	// allows the player to click to place the mouse somewhere? _scriptVariables[5] = 0;

	switch (op) {
	case 0:
		break;
	case 1: // init board
		// value of 0 is V, 1 is <, 2 is ^, 3 is >
		// variable 23 is the outside piece
		_scriptVariables[23] = _random.getRandomNumber(3);
		// variable slot is the space number + 25, the left corner
		// (Stauf's goal) is space 1, above that is space 2, the
		// center is 13, and the right corner (goal) is space 25
		for (int i = 27; i <= 49; i++) {
			_scriptVariables[i] = _random.getRandomNumber(3);
		}
		break;
	case 2: // before player chooses the floor to move, set the banned move
	{
		int clicked = int(_scriptVariables[0]) * 5 + int(_scriptVariables[1]) + 1;
		_scriptVariables[clicked + 50] = 0;
		break;
	}
	case 3: // after player moving floor
		// a bunch of hardcoded conditionals to copy variables and
		// set the banned move
		// this probably also sets a variable to allow the player to
		// move the mouse, and checks for win/lose
		break;
	case 5: // maybe player moving mouse
		break;
	case 6: // Stauf moving floor?
		break;
	case 7: // maybe Stauf moving mouse
		break;
	case 8: // Samantha making a move
		break;

	default:
		warning("Unknown mousetrap op %d", op);
		break;
	}
}

void T11hGame::opPente() {
	// FIXME: properly implement Pente game (the final puzzle)
	// for now just auto-solve the puzzle so the player can continue
	_scriptVariables[5] = 4;
}

void T11hGame::opTriangle() {
	// TODO
}

} // End of Namespace Groovie
