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

#include <cxxtest/TestSuite.h>

class RipperBoardGameTestSuite : public CxxTest::TestSuite {
public:
	void testRetailInitialPositionAndSideToMove() {
		Ripper::BoardGameModel model;

		TS_ASSERT_EQUALS(model.sideToMove(), 1);
		TS_ASSERT_EQUALS(model.pieceAt(0), -1);
		TS_ASSERT_EQUALS(model.pieceAt(2), -4);
		TS_ASSERT_EQUALS(model.pieceAt(6), -2);
		TS_ASSERT_EQUALS(model.pieceAt(31), 2);
		TS_ASSERT_EQUALS(model.pieceAt(34), 4);
		TS_ASSERT_EQUALS(model.pieceAt(41), 1);
	}

	void testKingRemainsInItsTwoHomeRows() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[41] = 1;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);
		Common::Array<int> destinations;

		model.legalDestinations(41, destinations);

		TS_ASSERT_EQUALS(destinations.size(), 2U);
		TS_ASSERT(contains(destinations, 35));
		TS_ASSERT(contains(destinations, 40));
	}

	void testTypeFourLeapsOnlyToCaptures() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[20] = 4;
		cells[6] = -2;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);
		Common::Array<int> destinations;

		model.legalDestinations(20, destinations);

		TS_ASSERT_EQUALS(destinations.size(), 1U);
		TS_ASSERT_EQUALS(destinations[0], 6);
	}

	void testTypeTwoCannotCaptureTypeFourOrFive() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[20] = 2;
		cells[14] = -4;
		cells[15] = -3;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);
		Common::Array<int> destinations;

		model.legalDestinations(20, destinations);

		TS_ASSERT(!contains(destinations, 14));
		TS_ASSERT(contains(destinations, 15));
	}

	void testRunnerPromotesAtFarRow() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[6] = 2;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);

		TS_ASSERT(model.applyMove(Ripper::BoardGameModel::Move(6, 0)));
		TS_ASSERT_EQUALS(model.pieceAt(0), 3);
		TS_ASSERT_EQUALS(model.sideToMove(), -1);
	}

	void testLastRunnerCaptureUpgradesTypeFour() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[14] = 3;
		cells[8] = -2;
		cells[0] = -4;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);

		TS_ASSERT(model.applyMove(Ripper::BoardGameModel::Move(14, 8)));
		TS_ASSERT_EQUALS(model.pieceAt(0), -5);
	}

	void testRunnerCanLeaveSideEdgeInFarRowsThroughDestination42() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[6] = 2;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);

		TS_ASSERT(model.isLegalMove(Ripper::BoardGameModel::Move(
			6, Ripper::BoardGameModel::kOffBoardDestination)));
		TS_ASSERT(model.applyMove(Ripper::BoardGameModel::Move(
			6, Ripper::BoardGameModel::kOffBoardDestination)));
		TS_ASSERT_EQUALS(model.pieceAt(6), 0);

		int8 negativeCells[Ripper::BoardGameModel::kCellCount] = {};
		negativeCells[35] = -2;
		model.setPosition(negativeCells, -1);
		TS_ASSERT(model.isLegalMove(Ripper::BoardGameModel::Move(
			35, Ripper::BoardGameModel::kOffBoardDestination)));
	}

	void testRunnerCannotLeaveThroughRowEdgeOrNearSideEdge() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[1] = 2;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);

		TS_ASSERT(!model.isLegalMove(Ripper::BoardGameModel::Move(
			1, Ripper::BoardGameModel::kOffBoardDestination)));

		int8 interiorCells[Ripper::BoardGameModel::kCellCount] = {};
		interiorCells[40] = -2;
		model.setPosition(interiorCells, -1);
		TS_ASSERT(!model.isLegalMove(Ripper::BoardGameModel::Move(
			40, Ripper::BoardGameModel::kOffBoardDestination)));

		int8 nearEdgeCells[Ripper::BoardGameModel::kCellCount] = {};
		nearEdgeCells[18] = -2;
		model.setPosition(nearEdgeCells, -1);
		TS_ASSERT(!model.isLegalMove(Ripper::BoardGameModel::Move(
			18, Ripper::BoardGameModel::kOffBoardDestination)));
	}

	void testKingCaptureSetsMovingSideResult() {
		int8 cells[Ripper::BoardGameModel::kCellCount] = {};
		cells[6] = 3;
		cells[0] = -1;
		Ripper::BoardGameModel model;
		model.setPosition(cells, 1);

		TS_ASSERT(model.applyMove(Ripper::BoardGameModel::Move(6, 0)));
		TS_ASSERT_EQUALS(model.result(), 1);
	}

private:
	bool contains(const Common::Array<int> &values, int value) const {
		for (uint index = 0; index < values.size(); ++index) {
			if (values[index] == value)
				return true;
		}
		return false;
	}
};
