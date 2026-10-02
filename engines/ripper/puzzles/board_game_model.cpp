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

#include "ripper/puzzles/board_game_model.h"

namespace Ripper {

namespace {

struct BoardOffset {
	int row;
	int column;
};

// g_boardMoveOffsetPointerTable at 0x40d98 selects these four tables.
static const BoardOffset kKingOffsets[] = {
	{-1, 0}, {0, -1}, {0, 1}, {1, 0}
};

static const BoardOffset kAdjacentOffsets[] = {
	{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
	{0, 1}, {1, -1}, {1, 0}, {1, 1}
};

static const BoardOffset kLeapOffsets[] = {
	{-2, -2}, {-2, 0}, {-2, 2}, {0, -2},
	{0, 2}, {2, -2}, {2, 0}, {2, 2}
};

// InitializeBoardPuzzleState at 0x40fa1 copies these 42 signed bytes from
// 0x40c32. Positive pieces move first.
static const int8 kInitialPosition[BoardGameModel::kCellCount] = {
	-1,  0, -4,  0,  0,  0,
	-2, -4,  0, -2, -2,  0,
	 0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,
	 0,  2,  2,  0,  4,  2,
	 0,  0,  0,  4,  0,  1
};

static bool containsDestination(const Common::Array<int> &destinations,
		int destination) {
	for (uint i = 0; i < destinations.size(); ++i) {
		if (destinations[i] == destination)
			return true;
	}
	return false;
}

} // End of anonymous namespace

BoardGameModel::BoardGameModel() : _sideToMove(1), _result(0) {
	reset();
}

void BoardGameModel::reset() {
	setPosition(kInitialPosition, 1);
}

void BoardGameModel::setPosition(const int8 *cells, int sideToMove) {
	memcpy(_cells, cells, sizeof(_cells));
	_sideToMove = sideToMove < 0 ? -1 : 1;
	_result = 0;
}

int BoardGameModel::pieceAt(int cell) const {
	return cell >= 0 && cell < kCellCount ? _cells[cell] : 0;
}

void BoardGameModel::appendDestination(Common::Array<int> &destinations,
		int destination) const {
	if (!containsDestination(destinations, destination))
		destinations.push_back(destination);
}

void BoardGameModel::legalDestinations(int source,
		Common::Array<int> &destinations) const {
	destinations.clear();
	if (_result != 0 || source < 0 || source >= kCellCount)
		return;

	const int piece = _cells[source];
	if (piece == 0 || (piece < 0 ? -1 : 1) != _sideToMove)
		return;
	const int type = ABS(piece);
	const int sourceRow = source / kColumnCount;
	const int sourceColumn = source % kColumnCount;
	const BoardOffset *offsets = nullptr;
	uint offsetCount = 0;
	if (type == 1) {
		offsets = kKingOffsets;
		offsetCount = ARRAYSIZE(kKingOffsets);
	} else if (type == 2 || type == 3) {
		offsets = kAdjacentOffsets;
		offsetCount = ARRAYSIZE(kAdjacentOffsets);
	} else if (type == 4) {
		offsets = kLeapOffsets;
		offsetCount = ARRAYSIZE(kLeapOffsets);
	}

	for (uint offset = 0; offset < offsetCount; ++offset) {
		const int row = sourceRow + offsets[offset].row;
		const int column = sourceColumn + offsets[offset].column;
		if (row < 0 || row >= kRowCount || column < 0 ||
				column >= kColumnCount) {
			// TryGetLegalBoardMoveDestination at 0x41c1e maps a runner
			// crossing a side edge inside its far three rows to the shared
			// off-board destination 42. Vertical exits are never legal.
			if ((type == 2 || type == 3) && row == sourceRow &&
					(column < 0 || column >= kColumnCount) &&
					((piece < 0 && sourceRow >= 4) ||
					 (piece > 0 && sourceRow <= 2)))
				appendDestination(destinations, kOffBoardDestination);
			continue;
		}

		if (type == 1 && ((piece < 0 && row > 1) ||
				(piece > 0 && row < kRowCount - 2)))
			continue;
		const int destination = row * kColumnCount + column;
		const int target = _cells[destination];
		if (target != 0 && (target < 0) == (piece < 0))
			continue;
		if (type == 4 && target == 0)
			continue;
		if (type == 2 && target != 0 && ABS(target) >= 4)
			continue;
		appendDestination(destinations, destination);
	}

	if (type == 5) {
		for (uint group = 0; group < 2; ++group) {
			const BoardOffset *groupOffsets = group == 0 ?
				kAdjacentOffsets : kLeapOffsets;
			const uint groupCount = group == 0 ?
				ARRAYSIZE(kAdjacentOffsets) : ARRAYSIZE(kLeapOffsets);
			for (uint offset = 0; offset < groupCount; ++offset) {
				const int row = sourceRow + groupOffsets[offset].row;
				const int column = sourceColumn + groupOffsets[offset].column;
				if (row < 0 || row >= kRowCount || column < 0 ||
						column >= kColumnCount)
					continue;
				const int destination = row * kColumnCount + column;
				const int target = _cells[destination];
				if (target == 0 || (target < 0) != (piece < 0))
					appendDestination(destinations, destination);
			}
		}
	}
}

bool BoardGameModel::isLegalMove(const Move &move) const {
	Common::Array<int> destinations;
	legalDestinations(move.source, destinations);
	return containsDestination(destinations, move.destination);
}

void BoardGameModel::legalMoves(Common::Array<Move> &moves,
		bool reverseSources) const {
	moves.clear();
	for (int index = 0; index < kCellCount; ++index) {
		const int source = reverseSources ? kCellCount - index - 1 : index;
		Common::Array<int> destinations;
		legalDestinations(source, destinations);
		for (uint destination = 0; destination < destinations.size(); ++destination)
			moves.push_back(Move(source, destinations[destination]));
	}
}

bool BoardGameModel::hasRunner(int side) const {
	for (uint cell = 0; cell < kCellCount; ++cell) {
		if ((_cells[cell] == side * 2) || (_cells[cell] == side * 3))
			return true;
	}
	return false;
}

void BoardGameModel::upgradeSpecialPieces(int side) {
	if (hasRunner(side))
		return;
	for (uint cell = 0; cell < kCellCount; ++cell) {
		if (_cells[cell] == side * 4)
			_cells[cell] = side * 5;
	}
}

bool BoardGameModel::applyMove(const Move &move) {
	if (!isLegalMove(move))
		return false;

	const int movingPiece = _cells[move.source];
	const int movingSide = movingPiece < 0 ? -1 : 1;
	int capturedPiece = 0;
	_cells[move.source] = 0;
	if (move.destination != kOffBoardDestination) {
		capturedPiece = _cells[move.destination];
		_cells[move.destination] = movingPiece;
		if (ABS(movingPiece) == 2 &&
				((movingSide < 0 && move.destination / kColumnCount == kRowCount - 1) ||
				 (movingSide > 0 && move.destination / kColumnCount == 0) ||
				 ABS(capturedPiece) == 2 || ABS(capturedPiece) == 3))
			_cells[move.destination] = movingSide * 3;
	}

	if (ABS(capturedPiece) == 1)
		_result = movingSide;
	if (ABS(movingPiece) == 2 || ABS(movingPiece) == 3)
		upgradeSpecialPieces(movingSide);
	if (ABS(capturedPiece) == 2 || ABS(capturedPiece) == 3)
		upgradeSpecialPieces(-movingSide);
	_sideToMove = -_sideToMove;
	return true;
}

void BoardGameModel::passTurn() {
	if (_result == 0)
		_sideToMove = -_sideToMove;
}

void BoardGameModel::setResult(int result) {
	_result = result < 0 ? -1 : result > 0 ? 1 : 0;
}

} // End of namespace Ripper
