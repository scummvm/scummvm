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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "common/queue.h"

#include "mediastation/mediascript/function.h"
#include "mediastation/mediastation.h"
#include "mediastation/minigames/maze.h"

namespace MediaStation {

// Interestingly, the titles that have these maze functions don't seem to have
// scripts that actually call them. Thus, for now only the maze functionality that
// is known to be actually used is reimplemented, namely the overloaded script_MazeSolve
// method and the methods it calls.
void FunctionManager::script_MazeGenerate(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// Doesn't seem to be actually called by scripts.
	warning("STUB: %s", __func__);
}

void FunctionManager::script_MazeApplyMoveMask(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// Doesn't seem to be actually called by scripts.
	warning("STUB: %s", __func__);
}

void FunctionManager::script_MazeSolve(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args.size() == 0) {
		// newMaze_clear
		delete _maze;
		_maze = nullptr;

	} else if (args.size() == 2) {
		// newMaze_init
		delete _maze;
		_maze = new MazeMinigame::Maze(args);

	} else if (args.size() == 4) {
		// newMaze_solve
		if (_maze != nullptr) {
			_maze->solve(args, returnValue);
		} else {
			error("%s: Maze has not been initialized", __func__);
		}

	} else if (args.size() == 5) {
		// Doesn't seem to be actually called by scripts.
		error("%s: This pathfinding code has not been implemented due to no known titles that use it", __func__);

	} else {
		error("%s: Got invalid number of args", __func__);
	}
}

MazeMinigame::Maze::Maze(const Common::Array<ScriptValue> &args) {
	Collection *gridDataCollection = args[0].asCollection();
	// args[1] is the precomputed routing table, which we won't
	// use since we are just doing BFS.

	const int16 rowCount = static_cast<int16>(gridDataCollection->operator[](gridDataCollection->size() - 2).asFloat());
	const int16 columnCount = static_cast<int16>(gridDataCollection->operator[](gridDataCollection->size() - 1).asFloat());
	_grid = new CellGrid(rowCount, columnCount);

	// Read the per-cell direction masks out of the padded script layout into
	// plain grid order (column outer, row inner).
	Common::Array<int16> directionMasks;
	directionMasks.reserve(rowCount * columnCount);
	for (int16 column = 0; column < columnCount; column++) {
		for (int16 row = 0; row < rowCount; row++) {
			const int32 dataIndex = _grid->paddedIndexForCoord(CellCoord(row, column));
			directionMasks.push_back(static_cast<int16>(gridDataCollection->operator[](dataIndex).asFloat()));
		}
	}

	_grid->setDirections(directionMasks);
}

MazeMinigame::Maze::~Maze() {
	delete _grid;
	_grid = nullptr;
}

MazeMinigame::CellGraphType MazeMinigame::Cell::getType() const {
	CellGraphType result = kObstacleCell;
	switch (_directionMask) {
	case 0x00:
		result = kObstacleCell;
		break;

	// These other cases are technically not needed because only the segment table generation
	// code used them. However, they are left here for completeness and later implementation
	// of the original logic if desired.
	case CellDirectionMask::kWest: // 0x01
	case CellDirectionMask::kSouth: // 0x02
	case CellDirectionMask::kEast: // 0x04
	case CellDirectionMask::kNorth: // 0x08
		result = kDeadEndNode;
		break;

	case (CellDirectionMask::kWest | CellDirectionMask::kSouth | CellDirectionMask::kEast): // 0x07
	case (CellDirectionMask::kWest | CellDirectionMask::kSouth | CellDirectionMask::kNorth): // 0x0B
	case (CellDirectionMask::kWest | CellDirectionMask::kEast | CellDirectionMask::kNorth): // 0x0D
	case (CellDirectionMask::kSouth | CellDirectionMask::kEast | CellDirectionMask::kNorth): // 0x0E
	case (CellDirectionMask::kWest | CellDirectionMask::kSouth | CellDirectionMask::kEast | CellDirectionMask::kNorth): // 0x0F
		result = kJunctionNode;
		break;

	case (CellDirectionMask::kWest | CellDirectionMask::kSouth): // 0x03
	case (CellDirectionMask::kWest | CellDirectionMask::kEast): // 0x05
	case (CellDirectionMask::kSouth | CellDirectionMask::kEast): // 0x06
	case (CellDirectionMask::kWest | CellDirectionMask::kNorth): // 0x09
	case (CellDirectionMask::kSouth | CellDirectionMask::kNorth): // 0x0A
	case (CellDirectionMask::kEast | CellDirectionMask::kNorth): // 0x0C
		result = kCorridorCell;
		break;

	default:
		warning("%s: Unexpected direction mask 0x%x", __func__, _directionMask);
		break;
  }

  return result;
}

void MazeMinigame::Maze::solve(const Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// Cruella and the puppy arrive as 1-based row/column pairs.
	const CellCoord cruellaCoord(
		static_cast<int16>(args[0].asFloat() - 1),
		static_cast<int16>(args[1].asFloat() - 1));
	const CellCoord puppyCoord(
		static_cast<int16>(args[2].asFloat() - 1),
		static_cast<int16>(args[3].asFloat() - 1));

	Collection *pathCollection = new Collection;
	bool cruellaHasCaughtPuppy = (cruellaCoord == puppyCoord);
	if (!cruellaHasCaughtPuppy) {
		const Common::Array<CellCoord> path = _grid->shortestPath(cruellaCoord, puppyCoord);
		if (path.empty()) {
			// The maze is always fully connected, so this should never happen.
			error("%s: No path found", __func__);
		}

		for (CellCoord coord : path) {
			// Scripts expect these padded indices.
			const int32 paddedGridIndex = _grid->paddedIndexForCoord(coord);
			ScriptValue paddedGridIndexValue;
			paddedGridIndexValue.setToFloat(paddedGridIndex);
			pathCollection->push_back(paddedGridIndexValue);
		}
	}

	// The return value owns this collection that holds the serialized
	// path to send back to scripts.
	returnValue.setToCollection(pathCollection);
}

int32 MazeMinigame::CellGrid::paddedIndexForCoord(CellCoord coord) {
	// Maze data received from or sent to scripts is stored
	// with a one-cell border of padding on each side.
	return (coord.column + 1) * (_rows + 1) + (coord.row + 1);
}

MazeMinigame::CellGrid::CellGrid(int16 rowCount, int16 columnCount) : _rows(rowCount), _columns(columnCount) {
	_cells.resize(_rows * _columns);
}

void MazeMinigame::CellGrid::setDirections(const Common::Array<int16> &directionMasks) {
	// The masks arrive in grid order, so each linear slot is one cell.
	for (uint cellIndex = 0; cellIndex < _cells.size(); cellIndex++) {
		const CellCoord coord = coordForIndex(static_cast<int16>(cellIndex));
		const int16 directionMask = directionMasks[cellIndex];
		Cell &cell = at(coord);
		cell = Cell(directionMask);

		// Record the reachable neighbor in each open direction.
		if (directionMask & CellDirectionMask::kWest) { // 0x01
			cell._validNeighbors.push_back(indexForCoord(CellCoord(coord.row, coord.column - 1)));
		}
		if (directionMask & CellDirectionMask::kSouth) { // 0x02
			cell._validNeighbors.push_back(indexForCoord(CellCoord(coord.row + 1, coord.column)));
		}
		if (directionMask & CellDirectionMask::kEast) { // 0x04
			cell._validNeighbors.push_back(indexForCoord(CellCoord(coord.row, coord.column + 1)));
		}
		if (directionMask & CellDirectionMask::kNorth) { // 0x08
			cell._validNeighbors.push_back(indexForCoord(CellCoord(coord.row - 1, coord.column)));
		}
	}
}

int16 MazeMinigame::CellGrid::indexForCoord(CellCoord coord) const {
	return coord.row + coord.column * _rows;
}

MazeMinigame::CellCoord MazeMinigame::CellGrid::coordForIndex(int16 index) const {
	return CellCoord(index % _rows, index / _rows);
}

MazeMinigame::Cell &MazeMinigame::CellGrid::at(CellCoord coord) {
	return _cells[indexForCoord(coord)];
}

Common::Array<MazeMinigame::CellCoord> MazeMinigame::CellGrid::shortestPath(CellCoord start, CellCoord end) const {
	Common::Array<CellCoord> path;
	const Common::Array<uint> pathIndices = buildShortestPath(indexForCoord(start), indexForCoord(end));
	for (uint cellIndex : pathIndices) {
		path.push_back(coordForIndex(cellIndex));
	}
	return path;
}

Common::Array<uint> MazeMinigame::CellGrid::buildShortestPath(uint startIndex, uint endIndex) const {
	const int32 NOT_VISITED = -1;
	Common::Array<int32> predecessor(_cells.size(), NOT_VISITED);
	predecessor[startIndex] = startIndex;

	Common::Queue<uint> cellsToVisit;
	cellsToVisit.push(startIndex);

	// Do a standard BFS.
	bool endWasReached = (predecessor[endIndex] != NOT_VISITED);
	bool moreCellsToVisit = (!cellsToVisit.empty() && !endWasReached);
	while (moreCellsToVisit) {
		const uint currentCellIndex = cellsToVisit.pop();
		const Cell &currentCell = _cells[currentCellIndex];

		for (int16 neighborCellIndex : currentCell._validNeighbors) {
			bool alreadyVisited = (predecessor[neighborCellIndex] != NOT_VISITED);
			if (alreadyVisited) {
				continue;
			}

			if (_cells[neighborCellIndex].getType() == kObstacleCell) {
				continue;
			}

			// We have not seen this cell yet but it is valid
			// to move there, so we need to check it out.
			predecessor[neighborCellIndex] = currentCellIndex;
			cellsToVisit.push(neighborCellIndex);
		}

		endWasReached = (predecessor[endIndex] != NOT_VISITED);
		moreCellsToVisit = (!cellsToVisit.empty() && !endWasReached);
	}

	Common::Array<uint> path;
	if (endWasReached) {
		// Trace the path from the end to the start, then reverse
		// it to get the ordering scripts expect.
		for (uint cellIndex = endIndex; cellIndex != startIndex; cellIndex = predecessor[cellIndex]) {
			path.push_back(cellIndex);
		}
		path.push_back(startIndex);
		Common::reverse(path.begin(), path.end());
	}

	return path;
};

} // End of namespace MediaStation
