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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_PUZZLES_TICTACTOE_H
#define ASYLUM_PUZZLES_TICTACTOE_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleTicTacToe : public Puzzle {
public:
	PuzzleTicTacToe(AsylumEngine *engine);
	~PuzzleTicTacToe();

private:
	enum GameStatus {
		kStatus0,
		kStatusFree,
		kStatusNeedBlocking
	};

	uint32 _ticker;
	uint32 _frameIndex;
	uint32 _frameCount;
	int32  _currentPos;
	bool   _gameOver;
	int32  _winLine;

	uint32 _solveDelay;
	uint32 _brokenLines;

	char   _board[9];
	uint32 _moveList[40];
	uint32 _numberOfPossibleMoves;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	void updateScreen();
	bool key(const AsylumEvent &evt) { return keyExit(evt); }
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Init & update
	//////////////////////////////////////////////////////////////////////////
	void clearBoard();
	void drawField();
	void getTwoEmpty(uint32 field1, uint32 field2, uint32 field3);

	//////////////////////////////////////////////////////////////////////////
	// Game
	//////////////////////////////////////////////////////////////////////////
	bool computerThinks();
	GameStatus returnLineData(uint32 field1, uint32 field2, uint32 field3, char mark, uint32 *counterX, uint32 *counterO) const;
	bool expandLine();
	bool tryNewLine();
	uint32 returnEmptySlot(uint32 position1, uint32 position2, uint position3) const;
	bool checkWin();
	int32 lookForAWinner();
	bool strategy(char mark);
	bool tryToWin()     { return strategy('X'); }
	bool tryNotToLose() { return strategy('O'); }
	bool arbitraryPlacement();
	void computerMoves();
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_TICTACTOE_H
