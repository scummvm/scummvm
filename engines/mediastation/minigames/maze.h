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

#ifndef MEDIASTATION_MINIGAMES_MAZE_H
#define MEDIASTATION_MINIGAMES_MAZE_H

#include "mediastation/actor.h"

namespace MediaStation {

namespace MazeMinigame {

enum CellDirectionMask {
	kWest = 0x01,
	kSouth = 0x02,
	kEast = 0x04,
	kNorth = 0x08
};

enum CellGraphType {
	kObstacleCell = 0,
	kDeadEndNode = 1,
	kJunctionNode = 2,
	kCorridorCell = 3,
};

// A row/column position within the maze grid.
struct CellCoord {
	int16 row = 0;
	int16 column = 0;

	CellCoord(int16 row_ = 0, int16 column_ = 0) : row(row_), column(column_) {}

	bool operator==(const CellCoord &other) const {
		return row == other.row && column == other.column;
	}
};

class Cell { // MazeElem
public:
	Cell(int16 directionMask = 0) : _directionMask(directionMask) {};

	CellGraphType getType() const;
	Common::Array<int16> _validNeighbors;

private:
	int16 _directionMask = 0;
};

// The original compressed the maze space down to "segments" (corridors connecting
// junction/dead-end nodes) and used a pre-computed routing table for each level.
// This seems way overkill for a 10x16 maze, so this reimplementation just does BFS.
//
// Another improvement from the original is that the linear cell indexing is purely
// an implementation detail here. Callers address cells by coordinate and receive paths
// back as coordinates.
class CellGrid {
public:
	CellGrid(int16 rowCount, int16 columnCount);
	void setDirections(const Common::Array<int16> &directionMasks);

	// The shortest path from start to end inclusive, as coordinates.
	// Empty if the two cells are not connected.
	Common::Array<CellCoord> shortestPath(CellCoord start, CellCoord end) const;
	int32 paddedIndexForCoord(CellCoord coord);

private:
	int16 indexForCoord(CellCoord coord) const;
	CellCoord coordForIndex(int16 index) const;
	Cell &at(CellCoord coord);
	Common::Array<uint> buildShortestPath(uint startIndex, uint endIndex) const;

	int16 _rows = 0;
	int16 _columns = 0;
	Common::Array<Cell> _cells;
};

// A simple maze with one player (puppy) and one enemy (Cruella). This translates the
// padded cell indexing that scripts use to and from actual grid coordinates.
class Maze {
public:
	Maze(const Common::Array<ScriptValue> &args); // newMaze_init
	~Maze();

	void solve(const Common::Array<ScriptValue> &args, ScriptValue &returnValue); // newMaze_solve

private:
	CellGrid *_grid = nullptr;
};

} // End of namespace MazeMinigame

} // End of namespace MediaStation

#endif
