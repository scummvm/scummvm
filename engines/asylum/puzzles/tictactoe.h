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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_TICTACTOE_H
#define ASYLUM_TICTACTOE_H

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
	int32 _lastMarkedField;
	bool _needToInitialize;
	int32 _strikeOutPosition;

	uint32 _counter;
	uint32 _counter2;

	char _gameField[9];
	uint32 _field[40];
	uint32 _emptyCount;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init();
	bool update();
	bool key(const AsylumEvent &evt);
	bool mouse(const AsylumEvent &evt);

	void mouseLeft();

	//////////////////////////////////////////////////////////////////////////
	// Init & update
	//////////////////////////////////////////////////////////////////////////
	void initField();
	void drawField();
	void updatePositions(uint32 field1, uint32 field2, uint32 field3);

	//////////////////////////////////////////////////////////////////////////
	// Game
	//////////////////////////////////////////////////////////////////////////
	bool check();
	GameStatus checkField(uint32 field1, uint32 field2, uint32 field3, char mark, uint32 *counterX, uint32 *counterO);
	bool checkFieldsUpdatePositions();
	bool checkFields();
	uint32 checkPosition(uint32 position1, uint32 position2, uint position3);
	bool checkWinner();
	int32 checkWinnerHelper();
	bool checkWinning(char mark);
	bool countEmptyFields();
	void placeOpponentMark();
};

} // End of namespace Asylum

#endif // ASYLUM_TICTACTOE_H
