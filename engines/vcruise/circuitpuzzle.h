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

#ifndef VCRUISE_CIRCUITPUZZLE_H
#define VCRUISE_CIRCUITPUZZLE_H

#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"

namespace Common {

class RandomSource;

} // End of namespace Common

namespace VCruise {

struct CircuitPuzzleAIEvaluator;
class CircuitPuzzleVisitedSet;

class CircuitPuzzle {
public:
	explicit CircuitPuzzle(int layout);

	static const uint kBoardWidth = 6;
	static const uint kBoardHeight = 5;

	enum CellDirection {
		kCellDirectionRight,
		kCellDirectionDown,
	};

	enum Conclusion {
		kConclusionNone,
		kConclusionPlayerWon,
		kConclusionPlayerLost,
	};

	struct CellRectSpec {
		Common::Rect _rightLinkRect;
		Common::Rect _downLinkRect;
		Common::Rect _rightBarrierRect;
		Common::Rect _downBarrierRect;
	};

	// Returns true if the AI can act, if it can then the actions are produced
	bool executeAIAction(Common::RandomSource &randomSource, Common::Point &outCoord, CellDirection &outBlockDirection);

	void addLink(const Common::Point &coord, CellDirection direction);

	Conclusion checkConclusion() const;

	const CellRectSpec *getCellRectSpec(const Common::Point &coord) const;
	bool isCellDownLinkOpen(const Common::Point &coord) const;
	bool isCellRightLinkOpen(const Common::Point &coord) const;

private:
	enum LinkState {
		kLinkStateOpen,
		kLinkStateConnected,
		kLinkStateBlocked,
	};

	enum Direction {
		kDirectionUp,
		KDirectionDown,
		kDirectionLeft,
		kDirectionRight,

		kDirectionCount,
	};

	struct CellState {
		CellState();

		LinkState _downLink;
		LinkState _rightLink;
	};

	struct Action {
		Action();

		Common::Point _point;
		CellDirection _direction;
	};

	static Common::Point getConnectedPoint(const Common::Point &coord, Direction direction);
	LinkState *getConnectionState(const Common::Point &coord, Direction direction);
	const LinkState *getConnectionState(const Common::Point &coord, Direction direction) const;
	static bool isPositionValid(const Common::Point &coord);

	void computeStepsToReach(CircuitPuzzleAIEvaluator &evaluator) const;
	void floodFillLinks(Common::Point *pointsList, uint &listSize, CircuitPuzzleVisitedSet &visitedSet) const;

	static void validateCoord(const Common::Point &coord);

	CellState _cells[kBoardWidth][kBoardHeight];
	CellRectSpec _cellRectSpecs[kBoardWidth][kBoardHeight];
	Common::Point _startPoint;
	Common::Point _goalPoint;

	bool _havePreviousAction;
	Action _previousAction;
};

} // End of namespace VCruise

#endif
