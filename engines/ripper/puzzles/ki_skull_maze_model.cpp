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

#include "ripper/puzzles/ki_skull_maze_model.h"

#include "common/util.h"

namespace Ripper {

KiSkullMazeModel::KiSkullMazeModel() :
		_currentCell(0), _blockedCellCount(0), _randomSeed(0) {
	for (uint cell = 0; cell < kCellCount; ++cell)
		_cells[cell] = kCardinalClosed;
}

uint KiSkullMazeModel::nextRandom15() {
	// GenerateRandomInt15 at 0x49a2f uses the Microsoft-compatible LCG.
	_randomSeed = _randomSeed * 0x41c64e6dU + 0x3039U;
	return (_randomSeed >> 16) & 0x7fff;
}

void KiSkullMazeModel::reset(uint32 seed, uint startCell) {
	_randomSeed = seed;
	_blockedCellCount = 0;
	_currentCell = startCell < kCellCount ? startCell : 0;
	for (uint cell = 0; cell < kCellCount; ++cell)
		_cells[cell] = nextRandom15() % 4;
}

byte KiSkullMazeModel::cellState(uint cell) const {
	return cell < kCellCount ? _cells[cell] : kSkull;
}

void KiSkullMazeModel::setCellState(uint cell, byte state) {
	if (cell >= kCellCount || state > kSkull)
		return;
	if (_cells[cell] == kSkull && state != kSkull)
		--_blockedCellCount;
	else if (_cells[cell] != kSkull && state == kSkull)
		++_blockedCellCount;
	_cells[cell] = state;
}

void KiSkullMazeModel::setCurrentCell(uint cell) {
	if (cell < kCellCount)
		_currentCell = cell;
}

bool KiSkullMazeModel::isOpen(uint cell) const {
	if (cell >= kCellCount)
		return false;
	return _cells[cell] == kCardinalOpen || _cells[cell] == kDiagonalOpen;
}

bool KiSkullMazeModel::isAdjacent(uint cell) const {
	if (cell >= kCellCount)
		return false;
	const int row = cell / kBoardSize;
	const int column = cell % kBoardSize;
	const int currentRow = _currentCell / kBoardSize;
	const int currentColumn = _currentCell % kBoardSize;
	return ABS(row - currentRow) + ABS(column - currentColumn) == 1;
}

bool KiSkullMazeModel::canMoveTo(uint cell) const {
	return isAdjacent(cell) && isOpen(cell);
}

bool KiSkullMazeModel::reachedExit() const {
	return _currentCell / kBoardSize == kBoardSize - 1;
}

int KiSkullMazeModel::neighborCell(int rowDelta, int columnDelta) const {
	const int row = _currentCell / kBoardSize + rowDelta;
	const int column = _currentCell % kBoardSize + columnDelta;
	if (row < 0 || row >= (int)kBoardSize ||
			column < 0 || column >= (int)kBoardSize)
		return -1;
	return row * kBoardSize + column;
}

bool KiSkullMazeModel::isTrapped() const {
	return !isOpen(neighborCell(-1, 0)) &&
		!isOpen(neighborCell(1, 0)) &&
		!isOpen(neighborCell(0, -1)) &&
		!isOpen(neighborCell(0, 1));
}

int KiSkullMazeModel::toggleNeighbor(uint step) {
	if (step == 0 || step > 4)
		return -1;

	static const int cardinalDeltas[4][2] = {
		{-1, 0}, {0, -1}, {0, 1}, {1, 0}
	};
	static const int diagonalDeltas[4][2] = {
		{-1, -1}, {-1, 1}, {1, -1}, {1, 1}
	};
	const int (*deltas)[2] = _cells[_currentCell] == kCardinalOpen ?
		cardinalDeltas : diagonalDeltas;
	const int cell = neighborCell(deltas[step - 1][0], deltas[step - 1][1]);
	if (cell < 0 || _cells[cell] == kSkull)
		return -1;

	switch (_cells[cell]) {
	case kCardinalClosed:
		_cells[cell] = kCardinalOpen;
		break;
	case kCardinalOpen:
		_cells[cell] = kCardinalClosed;
		break;
	case kDiagonalClosed:
		_cells[cell] = kDiagonalOpen;
		break;
	case kDiagonalOpen:
		_cells[cell] = kDiagonalClosed;
		break;
	default:
		return -1;
	}
	return cell;
}

int KiSkullMazeModel::spawnSkull(uint protectedCell) {
	if (_blockedCellCount >= kCellCount - 1)
		return -1;

	uint cell = 0;
	do {
		cell = nextRandom15() % kCellCount;
	} while (_cells[cell] == kSkull || cell == protectedCell);
	setCellState(cell, kSkull);
	return cell;
}

Common::String KiSkullMazeModel::stateString() const {
	Common::String result;
	for (uint cell = 0; cell < kCellCount; ++cell) {
		if (cell != 0)
			result += ',';
		result += (char)('0' + _cells[cell]);
	}
	return result;
}

} // End of namespace Ripper
