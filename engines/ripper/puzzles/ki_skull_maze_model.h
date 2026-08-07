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

#ifndef RIPPER_PUZZLES_KI_SKULL_MAZE_MODEL_H
#define RIPPER_PUZZLES_KI_SKULL_MAZE_MODEL_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Ripper {

class KiSkullMazeModel {
public:
	enum CellState {
		kCardinalClosed = 0,
		kCardinalOpen = 1,
		kDiagonalClosed = 2,
		kDiagonalOpen = 3,
		kSkull = 4
	};

	static const uint kBoardSize = 8;
	static const uint kCellCount = kBoardSize * kBoardSize;

	KiSkullMazeModel();

	void reset(uint32 seed, uint startCell);
	byte cellState(uint cell) const;
	void setCellState(uint cell, byte state);
	uint currentCell() const { return _currentCell; }
	void setCurrentCell(uint cell);
	uint blockedCellCount() const { return _blockedCellCount; }
	uint32 randomSeed() const { return _randomSeed; }

	bool isOpen(uint cell) const;
	bool isAdjacent(uint cell) const;
	bool canMoveTo(uint cell) const;
	bool reachedExit() const;
	bool isTrapped() const;
	int toggleNeighbor(uint step);
	int spawnSkull(uint protectedCell);
	Common::String stateString() const;

private:
	uint nextRandom15();
	int neighborCell(int rowDelta, int columnDelta) const;

	byte _cells[kCellCount];
	uint _currentCell;
	uint _blockedCellCount;
	uint32 _randomSeed;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KI_SKULL_MAZE_MODEL_H
