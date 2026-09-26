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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_PUZZLES_BOARD_GAME_MODEL_H
#define RIPPER_PUZZLES_BOARD_GAME_MODEL_H

#include "common/array.h"

namespace Ripper {

class BoardGameModel {
public:
	enum {
		kRowCount = 7,
		kColumnCount = 6,
		kCellCount = kRowCount * kColumnCount,
		kOffBoardDestination = kCellCount
	};

	struct Move {
		int source;
		int destination;

		Move() : source(-1), destination(-1) {}
		Move(int source_, int destination_) :
			source(source_), destination(destination_) {}
	};

	BoardGameModel();

	void reset();
	void setPosition(const int8 *cells, int sideToMove);
	int pieceAt(int cell) const;
	int sideToMove() const { return _sideToMove; }
	int result() const { return _result; }
	bool isLegalMove(const Move &move) const;
	void legalDestinations(int source, Common::Array<int> &destinations) const;
	void legalMoves(Common::Array<Move> &moves, bool reverseSources = false) const;
	bool applyMove(const Move &move);
	void passTurn();
	void setResult(int result);

private:
	void appendDestination(Common::Array<int> &destinations, int destination) const;
	void upgradeSpecialPieces(int side);
	bool hasRunner(int side) const;

	int8 _cells[kCellCount];
	int8 _sideToMove;
	int8 _result;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_BOARD_GAME_MODEL_H
