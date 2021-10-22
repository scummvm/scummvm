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

#include <limits.h>
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
	: _random("GroovieT11hGame"), _scriptVariables(scriptVariables), _cake(NULL) {
}

T11hGame::~T11hGame() {
}

void T11hGame::handleOp(uint8 op) {
	switch (op) {
	case 1:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Connect four in the dining room. (tb.grv)", op);
		opConnectFour();
		break;

	case 2:
		debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Beehive Puzzle in the top room (hs.grv)", op);
		// NOTE: Reused in UHP
		_beehive.run(_scriptVariables);
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

class T11hCake {
/*
* Connect Four puzzle, the cake in the dining room
*/
public:
	Common::RandomSource &_random;

	/*
	* T11hCake() constructor
	*	- Each spot on the board is part of multiple potential victory lines
	*	- The first x and y dimensions of the loops select the origin point of the line
	*	- The z is for the distance along that line
	*	- Then we push_back the id number of the line into the array at _map.indecies[x][y]
	*	- This is used in UpdateScores()
	*	.
	* @see UpdateScores()
	*/
	T11hCake(Common::RandomSource &rng) : _random(rng) {
		Restart();

		_map = {};
		int numLines = 0;

		// map all the lines with slope of (1, 0)
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
				for (int z = 0; z < GOAL_LEN; z++) {
					SetLineNum(x + z, y, numLines);
				}
				numLines++;
			}
		}

		// map all the lines with slope of (0, 1)
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y <= HEIGHT - GOAL_LEN; y++) {
				for (int z = 0; z < GOAL_LEN; z++) {
					SetLineNum(x, y + z, numLines);
				}
				numLines++;
			}
		}

		// map all the lines with slope of (1,1)
		for (int y = 0; y <= HEIGHT - GOAL_LEN; y++) {
			for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
				for (int z = 0; z < GOAL_LEN; z++) {
					SetLineNum(x + z, y + z, numLines);
				}
				numLines++;
			}
		}

		// map all the lines with slope of (1,-1)
		for (int y = GOAL_LEN - 1; y < HEIGHT; y++) {
			for (int x = 0; x <= WIDTH - GOAL_LEN; x++) {
				for (int z = 0; z < GOAL_LEN; z++) {
					SetLineNum(x + z, y - z, numLines);
				}
				numLines++;
			}
		}
	}

	byte OpConnectFour(byte &lastMove) {
		if (lastMove == 8) {
			Restart();
			return 0;
		}

		if (lastMove == 9) {
			// samantha makes a move
			// TODO: fix graphical bug when samantha makes a move
			lastMove = AiGetBestMove(6);
			_hasCheated = true;
			return 0;
		}

		if (IsColumnFull(lastMove)) {
			warning("player tried to place a bon bon in a full column, last_move: %d", (int)lastMove);
			lastMove = 10;
			return 0;
		}

		PlaceBonBon(lastMove);
		byte winner = GetWinner();
		if (winner) {
			return winner;
		}

		lastMove = AiGetBestMove(4 + (_hasCheated == false));
		PlaceBonBon(lastMove);
		if (GameEnded())
			return STAUF;

		return 0;
	}

private:
	static const int WIDTH = 8;
	static const int HEIGHT = 7;
	static const int GOAL_LEN = 4;
	static const int WIN_SCORE = 1000000;//!< the number of points added for a connect four
	static const byte STAUF = 1;
	static const byte PLAYER = 2;
	static const int NUM_LINES = 107;//!< how many potential victory lines there are


	//! ID numbers for all of the potential victory lines for each spot on the board
	struct LinesMappings {
		byte lengths[WIDTH][HEIGHT];
		byte indecies[WIDTH][HEIGHT][GOAL_LEN * GOAL_LEN];
	};

	//! how many points a player has, and their progress on potential victory lines
	struct PlayerProgress {
		int _score;
		int _linesCounters[NUM_LINES];//!< how many pieces are claimed in each potential victory, links to LineMappings, an entry of 4 means that's a victory
	};

	PlayerProgress _playerProgress;
	PlayerProgress _staufProgress;

	byte _boardState[WIDTH][HEIGHT];//!< (0, 0) is the bottom left of the board
	byte _columnHeights[WIDTH];

	int _moveCount;
	bool _hasCheated;

	LinesMappings _map;//!< ID numbers for all of the potential victory lines for each spot on the board

	void Restart() {
		_playerProgress = {};
		_staufProgress = {};
		memset(_boardState, 0, sizeof(_boardState));
		memset(_columnHeights, 0, sizeof(_columnHeights));
		_moveCount = 0;
		_hasCheated = false;

		_playerProgress._score = NUM_LINES;
		_staufProgress._score = NUM_LINES;
	}

	void SetLineNum(uint x, uint y, uint index) {
		assert(x < WIDTH);
		assert(y < HEIGHT);
		byte slot = _map.lengths[x][y]++;
		assert(slot < GOAL_LEN * GOAL_LEN);
		assert(index < NUM_LINES);
		_map.indecies[x][y][slot] = index;
	}

	bool IsColumnFull(byte column) {
		return _columnHeights[column] >= HEIGHT;
	}

	PlayerProgress &GetPlayerProgress(bool stauf) {
		if (stauf)
			return _staufProgress;
		else
			return _playerProgress;
	}

	/*
	* UpdateScores()
	*	- Each PlayerProgress has an array of ints, _linesCounters[], where each entry maps to the ID of a line
	*	- When a bon bon is added to the board, we look up _map.lengths[x][y] and then loop through all the indecies for that point
	*		- Increment the PlayerProgress._linesCounters[id]
	*		- Calculate the scores proportional to the PlayerProgress._linesCounters[id]
	*		.
	*	.
	*/
	void UpdateScores(byte x, bool revert=false) {
		bool stauf = _moveCount % 2;
		PlayerProgress &pp = GetPlayerProgress(stauf);

		byte y = _columnHeights[x] - 1;

		// get the number of potential victory lines that this spot exists in
		int num_lines = _map.lengths[x][y];

		for (int line = 0; line < num_lines; line++) {
			// get the ID for this potential victory line
			int index = _map.indecies[x][y][line];
			int len = pp._linesCounters[index];

			// add this new bon bon to the progress of this potential victory line, or remove in the case of revert
			int mult = 1;// mult is used for multiplying the score gains, depends on revert
			if (!revert)
				pp._linesCounters[index]++;
			else {
				len = --pp._linesCounters[index];
				mult = -1;
			}

			if (GOAL_LEN == len + 1) {
				// that's a bingo
				pp._score += WIN_SCORE * mult;
			}
			else {
				PlayerProgress &pp2 = GetPlayerProgress(!stauf);
				int len2 = pp2._linesCounters[index];
				if (len == 0) {
					// we started a new line, take away the points the opponent had from this line since we ruined it for them
					pp2._score -= (1 << (len2 & 31)) * mult;
				}
				if (len2 == 0) {
					// the opponent doesn't have any spots in this line, so we get points for it
					pp._score += (1 << (len & 31)) * mult;
				}
			}
		}
	}

	void PlaceBonBon(byte x) {
		byte y = _columnHeights[x]++;
		if (_moveCount % 2)
			_boardState[x][y] = STAUF;
		else
			_boardState[x][y] = PLAYER;

		UpdateScores(x);

		_moveCount++;
	}

	void RevertMove(byte x) {
		// PlaceBonBon in reverse, this is used for the AI's recursion rollback
		_moveCount--;

		UpdateScores(x, true);

		byte y = --_columnHeights[x];
		_boardState[x][y] = 0;
	}

	byte GetWinner() {
		if (_playerProgress._score >= WIN_SCORE)
			return PLAYER;

		if (_staufProgress._score >= WIN_SCORE)
			return STAUF;

		return 0;
	}

	bool GameEnded() {
		if (GetWinner())
			return true;

		if (_moveCount >= WIDTH * HEIGHT)
			return true;

		return false;
	}

	int GetScoreDiff() {
		if (_moveCount % 2)
			return _staufProgress._score - _playerProgress._score;
		else
			return _playerProgress._score - _staufProgress._score;
	}

	int AiRecurse(int search_depth, int parent_score) {
		int best_score = 0x7fffffff;

		for (byte move = 0; move < WIDTH; move++) {
			if (IsColumnFull(move))
				continue;

			PlaceBonBon(move);
			int score = GetScoreDiff();
			if (search_depth > 1 && !GameEnded())
				score = AiRecurse(search_depth - 1, best_score);
			RevertMove(move);

			if (score < best_score)
				best_score = score;

			if (-parent_score != best_score && parent_score <= -best_score)
				break;
		}

		// we negate the score because from the perspective of our parent caller, this is his opponent's score
		return -best_score;
	}

	uint Rng() {
		return _random.getRandomNumber(UINT_MAX);
	}

	byte AiGetBestMove(int search_depth) {
		int best_move = 0xffff;
		uint counter = 1;

		for (int best_score = 0x7fffffff; best_score > 999999 && search_depth > 1; search_depth--) {
			for (byte move = 0; move < WIDTH; move++) {
				if (IsColumnFull(move))
					continue;

				PlaceBonBon(move);
				if (GetWinner()) {
					RevertMove(move);
					return move;
				}

				int score = AiRecurse(search_depth - 1, best_score);
				RevertMove(move);
				if (score < best_score) {
					counter = 1;
					best_move = move;
					best_score = score;
				} else if (best_score == score) {
					// rng is only used on moves with equal scores
					counter++;
					uint r = Rng() % 1000000;
					if (r * counter < 1000000) {
						best_move = move;
					}
				}
			}
		}

		return best_move;
	}
};

void T11hGame::opConnectFour() {
	byte &last_move = _scriptVariables[1];
	byte &winner = _scriptVariables[3];
	winner = 0;

	if (_cake == NULL) {
		clearAIs();
		_cake = new T11hCake(_random);
	}

	winner = _cake->OpConnectFour(last_move);

	if (winner) {
		clearAIs();
	}
}

void T11hGame::clearAIs() {
	if (_cake != NULL) {
		delete _cake;
		_cake = NULL;
	}
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

const int kPieceCount = 21;
enum kGalleryPieceStatus {
	kPieceUnselected = 0,
	kPieceSelected = 1
};

void T11hGame::opGallery() {
	byte pieceStatus[kPieceCount];
	byte status1[kPieceCount];
	byte status2[kPieceCount];

	memcpy(pieceStatus, _scriptVariables + 26, kPieceCount);

	int selectedPieces = 0;
	for (int i = 0; i < kPieceCount; i++) {
		status1[i] = 0;
		if (pieceStatus[i] == kPieceSelected) {
			for (int j = 0; j < kPieceCount; j++)
				status2[j] = pieceStatus[j];

			byte curLink = kGalleryLinks[i][0];
			pieceStatus[i] = kPieceUnselected;
			status2[i] = 0;

			int linkedPiece = 1;
			while (curLink != 0) {
				linkedPiece++;
				status2[curLink - 1] = kPieceUnselected;
				curLink = kGalleryLinks[i][linkedPiece - 1];
			}
			status1[i] = opGalleryAI(status2, 1);
			if (status1[i] == kPieceSelected) {
				selectedPieces++;
			}
		}
	}

	if (selectedPieces == 0) {
		int esi = 0;
		for (int i = 0; i < kPieceCount; i++) {
			if (esi < status1[i]) {
				esi = status1[i];
			}
		}

		if (esi == 2) {
			esi = 1;
		} else {
			if (esi < kPieceCount) {
				esi = 2;
			} else {
				esi -= 12;
			}
		}

		for (int i = 0; i < kPieceCount; i++) {
			if (esi < status1[i]) {
				status1[i] = kPieceSelected;
				selectedPieces++;
			}
		}
	}

	int selectedPiece = 0;

	byte v12 = _scriptVariables[49] % selectedPieces;
	for (int i = 0; i < kPieceCount; i++) {
		if (status1[selectedPiece] == 1 && !v12--)
			break;

		selectedPiece++;
	}

	setScriptVar(47, (selectedPiece + 1) / 10);
	setScriptVar(48, (selectedPiece + 1) % 10);
}

byte T11hGame::opGalleryAI(byte *pieceStatus, int depth) {
	byte status1[kPieceCount];
	byte status2[kPieceCount];

	int selectedPieces = 0;

	for (int i = 0; i < kPieceCount; i++) {
		status1[i] = 0;
		if (pieceStatus[i] == kPieceSelected) {
			for (int j = 0; j < kPieceCount; j++)
				status2[j] = pieceStatus[j];

			byte curLink = kGalleryLinks[i][0];
			pieceStatus[i] = kPieceUnselected;
			status2[i] = 0;
			selectedPieces = 1;

			int linkedPiece = 1;
			while (curLink != 0) {
				linkedPiece++;
				status2[curLink - 1] = kPieceUnselected;
				curLink = kGalleryLinks[i][linkedPiece - 1];
			}
			status1[i] = opGalleryAI(status2, depth == 0 ? 1 : 0);
			if (!depth && status1[i] == kPieceSelected) {
				return 1;
			}
		}
	}

	if (selectedPieces) {
		byte v8 = 0;
		byte v9 = 0;
		byte v10 = 0;
		for (int j = 0; j < 21; ++j) {
			byte v12 = status2[j];
			if (v12) {
				++v10;
				if (v12 == 1)
					++v9;
				else
					v8 += v12;
			}
		}
		if (v9 == v10)
			return 1;
		else
			return (v8 + 102 * v9) / v10;
	}

	return depth == 0 ? 2 : 1;
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
