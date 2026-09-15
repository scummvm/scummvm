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

#ifndef MOHAWK_ZOOMBINI_PAGES_GRID_TRAVERSAL_H
#define MOHAWK_ZOOMBINI_PAGES_GRID_TRAVERSAL_H

#include "common/scummsys.h"

namespace Mohawk {

/**
 * Canonical orthogonal directions for the shared grid traversal core.
 * Individual puzzles translate their authored direction encoding to this
 * enum before advancing a cell.
 */
enum ZmbGridDirection {
	/** One row toward the top of the grid. */
	kZmbGridUp = 0,
	/** One column toward the right of the grid. */
	kZmbGridRight,
	/** One row toward the bottom of the grid. */
	kZmbGridDown,
	/** One column toward the left of the grid. */
	kZmbGridLeft
};

/** A row/column coordinate in a page-owned logical grid. */
struct ZmbGridCell {
	/** Zero-based row coordinate. */
	int16 row;
	/** Zero-based column coordinate. */
	int16 col;

	/** Construct the origin cell. */
	ZmbGridCell() : row(0), col(0) {}
	/** Construct a cell from its row and column coordinates. */
	ZmbGridCell(int16 cellRow, int16 cellCol) : row(cellRow), col(cellCol) {}
};

/** Inclusive rectangular bounds for a logical grid. */
struct ZmbGridBounds {
	/** Smallest valid row. */
	int16 minRow;
	/** Largest valid row. */
	int16 maxRow;
	/** Smallest valid column. */
	int16 minCol;
	/** Largest valid column. */
	int16 maxCol;

	/** Construct inclusive row and column bounds. */
	ZmbGridBounds(int16 firstRow, int16 lastRow, int16 firstCol, int16 lastCol)
		: minRow(firstRow), maxRow(lastRow), minCol(firstCol), maxCol(lastCol) {}

	/** Return true when @p cell lies inside both inclusive ranges. */
	bool contains(const ZmbGridCell &cell) const {
		return minRow <= cell.row && cell.row <= maxRow &&
			   minCol <= cell.col && cell.col <= maxCol;
	}
};

/** Result of asking the shared traversal core for one grid step. */
struct ZmbGridStep {
	/** The adjacent cell, whether or not it is inside the bounds. */
	ZmbGridCell cell;
	/** True when @p cell is inside the supplied @ref ZmbGridBounds. */
	bool inBounds;

	/** Construct a step result. */
	ZmbGridStep(const ZmbGridCell &nextCell, bool valid)
		: cell(nextCell), inBounds(valid) {}
};

/**
 * Stateless orthogonal grid traversal helpers shared by grid puzzles.
 *
 * The class computes coordinates and bounds only. It does not decide what a
 * puzzle-specific boundary means, how a cell is rendered, or how a runner is
 * animated; those policies remain with the owning page.
 */
class ZmbGridTraversal {
public:
	/**
	 * Compute one orthogonal cell step without applying puzzle policy.
	 *
	 * The returned cell is the raw adjacent coordinate. A page decides what an
	 * out-of-bounds result means: Maze stays put, while Lilly may treat the far
	 * edge as an exit.
	 */
	static ZmbGridStep computeStep(const ZmbGridCell &source, ZmbGridDirection direction,
								   const ZmbGridBounds &bounds);

	/** Apply one in-bounds step. The source cell remains unchanged on a boundary. */
	static bool advanceCell(ZmbGridCell &cell, ZmbGridDirection direction,
							const ZmbGridBounds &bounds);

	/** Return the direction that reverses @p direction. */
	static ZmbGridDirection oppositeDirection(ZmbGridDirection direction);
};

} // End of namespace Mohawk

#endif
