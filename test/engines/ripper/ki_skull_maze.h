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

#include <cxxtest/TestSuite.h>

class RipperKiSkullMazeTestSuite : public CxxTest::TestSuite {
public:
	void testRetailRandomizerProducesStableInitialCells() {
		Ripper::KiSkullMazeModel model;
		model.reset(4, 3);
		const byte expected[16] = {
			1, 2, 3, 3, 3, 0, 2, 1,
			2, 3, 1, 1, 1, 3, 3, 1
		};
		for (uint cell = 0; cell < ARRAYSIZE(expected); ++cell)
			TS_ASSERT_EQUALS(model.cellState(cell), expected[cell]);
		TS_ASSERT_EQUALS(model.currentCell(), 3U);
		TS_ASSERT_EQUALS(model.blockedCellCount(), 0U);
	}

	void testCardinalOpenCellTogglesCardinalNeighbors() {
		Ripper::KiSkullMazeModel model;
		model.reset(4, 27);
		model.setCellState(27, Ripper::KiSkullMazeModel::kCardinalOpen);
		model.setCellState(19, Ripper::KiSkullMazeModel::kCardinalClosed);
		model.setCellState(26, Ripper::KiSkullMazeModel::kCardinalOpen);
		model.setCellState(28, Ripper::KiSkullMazeModel::kDiagonalClosed);
		model.setCellState(35, Ripper::KiSkullMazeModel::kDiagonalOpen);

		TS_ASSERT_EQUALS(model.toggleNeighbor(1), 19);
		TS_ASSERT_EQUALS(model.toggleNeighbor(2), 26);
		TS_ASSERT_EQUALS(model.toggleNeighbor(3), 28);
		TS_ASSERT_EQUALS(model.toggleNeighbor(4), 35);
		TS_ASSERT_EQUALS(model.cellState(19),
			Ripper::KiSkullMazeModel::kCardinalOpen);
		TS_ASSERT_EQUALS(model.cellState(26),
			Ripper::KiSkullMazeModel::kCardinalClosed);
		TS_ASSERT_EQUALS(model.cellState(28),
			Ripper::KiSkullMazeModel::kDiagonalOpen);
		TS_ASSERT_EQUALS(model.cellState(35),
			Ripper::KiSkullMazeModel::kDiagonalClosed);
	}

	void testDiagonalOpenCellTogglesDiagonalNeighbors() {
		Ripper::KiSkullMazeModel model;
		model.reset(4, 27);
		model.setCellState(27, Ripper::KiSkullMazeModel::kDiagonalOpen);
		model.setCellState(18, Ripper::KiSkullMazeModel::kCardinalClosed);
		model.setCellState(20, Ripper::KiSkullMazeModel::kCardinalClosed);
		model.setCellState(34, Ripper::KiSkullMazeModel::kDiagonalClosed);
		model.setCellState(36, Ripper::KiSkullMazeModel::kDiagonalClosed);

		TS_ASSERT_EQUALS(model.toggleNeighbor(1), 18);
		TS_ASSERT_EQUALS(model.toggleNeighbor(2), 20);
		TS_ASSERT_EQUALS(model.toggleNeighbor(3), 34);
		TS_ASSERT_EQUALS(model.toggleNeighbor(4), 36);
		TS_ASSERT_EQUALS(model.cellState(18),
			Ripper::KiSkullMazeModel::kCardinalOpen);
		TS_ASSERT_EQUALS(model.cellState(20),
			Ripper::KiSkullMazeModel::kCardinalOpen);
		TS_ASSERT_EQUALS(model.cellState(34),
			Ripper::KiSkullMazeModel::kDiagonalOpen);
		TS_ASSERT_EQUALS(model.cellState(36),
			Ripper::KiSkullMazeModel::kDiagonalOpen);
	}

	void testMovementExitTrapAndSkullProtection() {
		Ripper::KiSkullMazeModel model;
		model.reset(4, 9);
		for (uint cell = 0; cell < Ripper::KiSkullMazeModel::kCellCount; ++cell)
			model.setCellState(cell, Ripper::KiSkullMazeModel::kSkull);
		model.setCellState(9, Ripper::KiSkullMazeModel::kCardinalOpen);
		model.setCellState(10, Ripper::KiSkullMazeModel::kDiagonalOpen);
		TS_ASSERT(model.canMoveTo(10));
		TS_ASSERT(!model.isTrapped());
		model.setCellState(10, Ripper::KiSkullMazeModel::kCardinalClosed);
		TS_ASSERT(model.isTrapped());
		TS_ASSERT_EQUALS(model.spawnSkull(9), 10);
		TS_ASSERT_EQUALS(model.cellState(9),
			Ripper::KiSkullMazeModel::kCardinalOpen);

		model.setCellState(63, Ripper::KiSkullMazeModel::kCardinalOpen);
		model.setCurrentCell(63);
		TS_ASSERT(model.reachedExit());
	}
};
