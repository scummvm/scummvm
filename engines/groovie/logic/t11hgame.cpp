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

T11hGame::T11hGame(byte *scriptVariables) :
	_random("GroovieT11hGame"), _scriptVariables(scriptVariables) {
}

T11hGame::~T11hGame() {
}

void T11hGame::handleOp(uint8 op) {
	switch (op) {
	case 1:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Connect four in the dining room. (tb.grv) TODO", op);
		opConnectFour();
		break;

	case 2:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Beehive Puzzle in the top room (hs.grv)", op);
		opBeehive();
		break;

	case 3:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Make last move on modern art picture in the gallery (bs.grv)", op);
		opGallery();
		break;

	case 4:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Triangle in the Chapel (tx.grv)", op);
		opTriangle();
		break;

	case 5:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Mouse Trap in the lab (al.grv)", op);
		opMouseTrap();
		break;

	case 6:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Pente (pt.grv)", op);
		opPente();
		break;

	case 8:	// used in UHP
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): UHP Othello", op);
		// TODO: Same as the Clandestiny Othello/Reversi puzzle (opOthello)
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

/*
* Connect Four puzzle, the cake in the dining room
*/
void T11hGame::opConnectFour() {
	byte &last_move = _scriptVariables[1];
	byte &winner = _scriptVariables[3];

	if (last_move == 8) {
		clearCake();
		return;
	}

	if (last_move == 9) {
		// samantha makes a move
		// TODO: fix graphical bug when samantha makes a move
		last_move = connectFourAI();
		return;
	}

	cakePlaceBonBon(last_move, CAKE_TEAM_PLAYER);
	winner = cakeGetWinner();
	if (winner) {
		return;
	}

	last_move = connectFourAI();
	cakePlaceBonBon(last_move, CAKE_TEAM_STAUF);
	winner = cakeGetWinner();
}

byte T11hGame::connectFourAI() {
	// TODO: copy the AI from the game
	// the cakeGetLineLen function returns the length of the line which should be the scoring function
	uint slot = 0;
	do {
		slot = _random.getRandomNumber(7);
	} while (cake_board[slot][CAKE_BOARD_HEIGHT - 1]);
	return slot;
}

bool T11hGame::isCakeFull() {
	return NULL == memchr(cake_board, 0, sizeof(cake_board));
}

byte T11hGame::cakeGetOpponent(byte team) {
	if (team == CAKE_TEAM_PLAYER)
		return CAKE_TEAM_STAUF;
	else if (team == CAKE_TEAM_STAUF)
		return CAKE_TEAM_PLAYER;
	return 0;
}

// also use the cakeGetLineLen function as a scoring function for the AI
int T11hGame::cakeGetLineLen(int start_x, int start_y, int slope_x, int slope_y, byte team) {
	byte opponent = cakeGetOpponent(team);

	// return 0 for worthless lines
	if (start_x + slope_x * CAKE_GOAL_LEN > CAKE_BOARD_WIDTH)
		return 0;
	if (start_x + slope_x * CAKE_GOAL_LEN < 0)
		return 0;
	if (start_y + slope_y * CAKE_GOAL_LEN > CAKE_BOARD_HEIGHT)
		return 0;
	if (start_y + slope_y * CAKE_GOAL_LEN < 0)
		return 0;

	// don't loop past CAKE_GOAL_LEN because more than 4 is useless to rules and the AI
	int x = start_x;
	int y = start_y;
	int len = 0;
	for (int i = 0; i < CAKE_GOAL_LEN; i++) {
		if (cake_board[x][y] == opponent)
			return 0; // return 0 for worthless lines
		if (cake_board[x][y] == team)
			len++;

		x += slope_x;
		y += slope_y;
	}
	return len;
}

byte T11hGame::cakeGetWinner() {
	// make sure to check if all columns are maxed then Stauf wins
	if (isCakeFull())
		return CAKE_TEAM_STAUF;

	// search for lines of 4, we search up, right, up-right, and down-right
	for (int x = 0; x < CAKE_BOARD_WIDTH; x++) {
		for (int y = 0; y < CAKE_BOARD_HEIGHT; y++) {
			byte team = cake_board[x][y];
			// if this spot is team 0 then we can move on to the next column
			if (team == 0)
				break;

			// if we find a line, then we return the team value stored in this spot
			int line = 0;
			line = MAX(cakeGetLineLen(x, y, 1, 0, team), line);
			line = MAX(cakeGetLineLen(x, y, 0, 1, team), line);
			line = MAX(cakeGetLineLen(x, y, 1, 1, team), line);
			line = MAX(cakeGetLineLen(x, y, 1, -1, team), line);

			if (line >= CAKE_GOAL_LEN)
				return team;
		}
	}

	return 0;
}

void T11hGame::clearCake() {
	memset(cake_board, 0, sizeof(cake_board));
}

void T11hGame::cakePlaceBonBon(int x, byte team) {
	for (int y = 0; y < CAKE_BOARD_HEIGHT; y++) {
		if (cake_board[x][y] == 0) {
			cake_board[x][y] = team;
			return;
		}
	}
}

/*
 * Beehive puzzle
 *
 * An infection-style game in which the player must cover more
 * territory than the computer. It's similar to the microscope puzzle
 * in the 7th Guest. The playfield is a honeycomb made of 61
 * hexagons. The hexagons are numbered starting from the top-left
 * corner, with a direction from bottom left to top right.
 */
void T11hGame::opBeehive() {
	// TODO: Finish the logic
	int8 *hexagons = (int8 *)_scriptVariables + 25;
	int8 *hexDifference = (int8 *)_scriptVariables + 13;
	byte op = _scriptVariables[14] - 1;

	enum kBeehiveColor {
		kBeehiveColorYellow = -1,
		kBeehiveColorRed = 1
	};

	warning("Beehive subop %d", op);

	//*hexDifference = 4;
	*hexDifference = 5; // DEBUG: set the difference to 5 to skip the game

	switch (op) {
	case 0:	// init board's hexagons
		memset(_beehiveHexagons, 0, 60);
		_beehiveHexagons[0] = kBeehiveColorYellow;
		_beehiveHexagons[4] = kBeehiveColorRed;
		_beehiveHexagons[34] = kBeehiveColorYellow;
		_beehiveHexagons[60] = kBeehiveColorRed;
		_beehiveHexagons[56] = kBeehiveColorYellow;
		_beehiveHexagons[26] = kBeehiveColorRed;
		break;
	case 1:
		memset(hexagons, 0, 60);
		_scriptVariables[85] = 0;
		//opBeehiveSub2();	// TODO
		// TODO: Check opBeehiveSub2()'s result
		//*hexDifference = opBeehiveGetHexDifference();
		break;
	case 2:
		memset(hexagons, 0, 60);
		_scriptVariables[85] = 0;
		//opBeehiveSub4();	// TODO
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	default:
		break;
	}
}

int8 T11hGame::opBeehiveGetHexDifference() {
	return (opBeehiveGetTotal(_beehiveHexagons) >= 0) + 5;
}

int8 T11hGame::opBeehiveGetTotal(int8 *hexagons) {
	int8 result = 0;

	for (int i = 0; i < 61; i++)
		result += hexagons[i];

	return result;
}

void T11hGame::opPente() {
	// FIXME: properly implement Pente game (the final puzzle)
	// for now just auto-solve the puzzle so the player can continue
	_scriptVariables[5] = 4;
}

/*
 * Puzzle in the Gallery.
 * The aim is to select the last piece of the image.
 * There are 18 pieces in total.
 * When selecting a piece, all surrounding pieces are also selected
 * 
 * +--------------------+--------------------------------+--------+
 * |         1/1A       |       2/1B                     |        |
 * |  +--------------+--+--------------------------+-----+        |
 * |  |              |                             |              |
 * +--+     4/1D     |            5/1E             |       3/1C   |
 * |                 |                             |              |
 * +-----+--------+--+--------+-----------------+--+--------+     |
 * |     |        |           |                 |           |     |
 * |     |        |           |                 |           |     |
 * |     |        |   8/21    |                 |           |     |
 * |     |        |           |     +-----------+           |     |
 * |     |        |           |     |           |           |     |
 * |     |        +-----------+     |   10/23   |   9/22    |     |
 * |     |                          |           |           |     |
 * |     |           7/20           +-----+-----+           +-----+
 * |     |                          |     |     |           |     |
 * |     +--------------------------+     |     |           |     |
 * |              6/1F                    |     |           |     |
 * +-----------+-----------+-----+--+     | 11  |           | 12  |
 * |   13/26   |           |     |  |     | /   |           | /   |
 * |     +-----+-----+     |     |  |     | 24  +-----------+ 25  |
 * |     |           |     |     |  |     |     |           |     |
 * +-----+   17/2A   |     |     |16|     |     |           |     |
 * |     |           |     |     |/ |     |     |           |     |
 * |     +-----+-----+     |     |29|     |     |           +-----+
 * |           |           |     |  |     |     |           |     |
 * |           |           |     |  |     +-----+   18/2B   |     |
 * |   19/2C   |   14/27   |     |  |           |           |     |
 * |           |           |     |  +-----------+           |     |
 * |           |           |     |  |           |           |     |
 * |           |           |     +--+   15/28   |           |     |
 * |           |           |                    |           |     |
 * |           +--------+--+--------------------+-----------+     |
 * |           | 20/2D  |              21/2E                      |
 * +-----------+--------+-----------------------------------------+
 */

// Links between the pieces in the Gallery challenge
// For example, the first row signifies that piece 1
// is connected to pieces 2, 4 and 5
const byte T11hGame::kGalleryLinks[21][10] = {
	{ 2,  4,  5,  0,  0,  0,  0,  0,  0,  0 },	//  1
	{ 1,  5,  3,  0,  0,  0,  0,  0,  0,  0 },	//  2
	{ 2,  5,  9, 12,  0,  0,  0,  0,  0,  0 },	//  3
	{ 1,  5,  6,  7,  8,  0,  0,  0,  0,  0 },	//  4
	{ 1,  2,  3,  4,  7,  8,  9,  0,  0,  0 },	//  5
	{ 4,  7, 10, 11, 13, 14, 15, 16, 18,  0 },	//  6
	{ 4,  5,  6,  8,  9, 10,  0,  0,  0,  0 },	//  7
	{ 4,  5,  7,  0,  0,  0,  0,  0,  0,  0 },	//  8
	{ 3,  5,  7, 10, 11, 12, 18,  0,  0,  0 },	//  9
	{ 6,  7,  9, 11,  0,  0,  0,  0,  0,  0 },	// 10
	{ 6,  9, 10, 18,  0,  0,  0,  0,  0,  0 },	// 11
	{ 3,  9, 18, 21,  0,  0,  0,  0,  0,  0 },	// 12
	{ 6, 14, 17, 19,  0,  0,  0,  0,  0,  0 },	// 13
	{ 6, 13, 15, 17, 19, 20, 21,  0,  0,  0 },	// 14
	{ 6, 14, 16, 18, 21,  0,  0,  0,  0,  0 },	// 15
	{ 6, 15,  0,  0,  0,  0,  0,  0,  0,  0 },	// 16
	{13, 14, 19,  0,  0,  0,  0,  0,  0,  0 },	// 17
	{ 6,  9, 11, 12, 15, 21,  0,  0,  0,  0 },	// 18
	{13, 14, 17, 20,  0,  0,  0,  0,  0,  0 },	// 19
	{14, 19, 21,  0,  0,  0,  0,  0,  0,  0 },	// 20
	{12, 14, 15, 18, 20,  0,  0,  0,  0,  0 }	// 21
};

void T11hGame::opGallery() {
	const int kPieceCount = 21;
	byte pieceStatus[kPieceCount];
	byte var_18[kPieceCount];
	int var_1c, linkedPiece;
	byte curLink = 0;

	enum kGalleryPieceStatus {
		kPieceUnselected = 0,
		kPieceSelected = 1
	};

	memcpy(pieceStatus, _scriptVariables + 26, kPieceCount);

	var_1c = 0;
	for (int curPiece = 0; curPiece < kPieceCount; curPiece++) {
		var_18[curPiece] = 0;
		if (pieceStatus[curPiece] == kPieceSelected) {
			curLink = kGalleryLinks[curPiece][0];
			linkedPiece = 1;
			pieceStatus[curPiece] = kPieceUnselected;
			while (curLink != 0) {
				linkedPiece++;
				pieceStatus[curLink - 1] = kPieceUnselected;
				curLink = kGalleryLinks[linkedPiece - 1][curPiece];
			}
			var_18[curPiece] = opGallerySub(1, pieceStatus);
			if (var_18[curPiece] == 1) {
				var_1c++;
			}
		}
	}

	if (var_1c == 0) {
		int esi = 0;
		for (int i = 0; i < kPieceCount; i++) {
			if (var_18[i] > esi) {
				esi = var_18[i];
			}
		}

		if (esi == 2) {
			esi = 1;
		} else {
			if (esi <= 20) {
				esi = 2;
			} else {
				esi -= 12;
			}
		}

		for (linkedPiece = 0; linkedPiece < kPieceCount; linkedPiece++) {
			if (var_18[linkedPiece] <= esi) {
				var_18[linkedPiece] = 1;
				var_1c++;
			}
		}
	}

	int selectedPart;

	// TODO: copy the AI from the game
	do {
		selectedPart = _random.getRandomNumber(20) + 1;
	} while (_scriptVariables[0x19 + selectedPart] != 1);

	setScriptVar(0x2F, selectedPart / 10);
	setScriptVar(0x30, selectedPart % 10);
}

byte T11hGame::opGallerySub(int one, byte* field) {
	// TODO
	warning("STUB: T11hGame::opGallerySub()");
	return 0;
}

void T11hGame::opTriangle() {
	// TODO
}

// This function is mainly for debugging purposes
void inline T11hGame::setScriptVar(uint16 var, byte value) {
	_scriptVariables[var] = value;
	debugC(5, kDebugTlcGame, "script variable[0x%03X] = %d (0x%04X)", var, value, value);
}

void inline T11hGame::setScriptVar16(uint16 var, uint16 value) {
	_scriptVariables[var] = value & 0xFF;
	_scriptVariables[var + 1] = (value >> 8) & 0xFF;
	debugC(5, kDebugTlcGame, "script variable[0x%03X, 0x%03X] = %d (0x%02X, 0x%02X)",
		var, var + 1, value, _scriptVariables[var], _scriptVariables[var + 1]);
}

uint16 inline T11hGame::getScriptVar16(uint16 var) {
	uint16 value;

	value = _scriptVariables[var];
	value += _scriptVariables[var + 1] << 8;

	return value;
}

} // End of Namespace Groovie
