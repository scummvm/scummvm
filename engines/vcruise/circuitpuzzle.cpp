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

#include "vcruise/circuitpuzzle.h"

namespace VCruise {

namespace CircuitPuzzleTables {

// These are hard-coded into the Schizm executable (they're 32-bit in it), figured these out by hand
// from tracing screen captures and feeding through a boundary finder script.

static const int16 g_barriersHorizontal1[100] = {
	166, 6, 191, 62,
	244, 7, 267, 62,
	320, 7, 341, 62,
	394, 8, 416, 63,
	469, 8, 492, 62,
	164, 80, 191, 134,
	244, 80, 265, 130,
	320, 80, 336, 131,
	394, 80, 415, 130,
	468, 80, 492, 130,
	163, 152, 191, 199,
	245, 152, 263, 199,
	320, 152, 340, 199,
	395, 152, 415, 199,
	469, 152, 493, 199,
	166, 221, 187, 270,
	245, 221, 266, 270,
	320, 220, 341, 271,
	394, 221, 414, 271,
	469, 221, 492, 271,
	166, 290, 187, 343,
	244, 290, 263, 343,
	320, 290, 341, 343,
	395, 289, 415, 343,
	469, 291, 494, 346,
};

static const int16 g_barriersVertical1[64] = {
	187, 64, 240, 84,
	266, 63, 316, 84,
	340, 64, 392, 83,
	416, 64, 470, 84,
	186, 135, 240, 151,
	266, 135, 315, 152,
	341, 134, 393, 151,
	416, 135, 479, 152,
	187, 204, 239, 221,
	266, 203, 314, 221,
	341, 204, 391, 220,
	416, 204, 470, 220,
	187, 273, 239, 293,
	266, 271, 314, 292,
	342, 271, 391, 292,
	416, 272, 471, 292,
};

static const int16 g_barriersHorizontal2[100] = {
	160, 8, 185, 62,
	239, 7, 260, 63,
	309, 8, 332, 63,
	388, 8, 407, 63,
	455, 8, 485, 63,
	162, 79, 184, 130,
	241, 80, 260, 131,
	313, 80, 332, 131,
	387, 82, 408, 131,
	460, 82, 484, 131,
	162, 153, 184, 200,
	238, 153, 258, 200,
	312, 153, 333, 201,
	386, 153, 408, 201,
	459, 153, 485, 200,
	161, 220, 183, 270,
	237, 220, 259, 270,
	313, 221, 332, 270,
	384, 220, 408, 269,
	459, 219, 485, 267,
	161, 289, 183, 343,
	238, 288, 259, 350,
	312, 289, 332, 341,
	383, 291, 408, 342,
	460, 290, 484, 341,
};

static const int16 g_barriersVertical2[64] = {
	186, 63, 239, 84,
	263, 64, 313, 84,
	337, 64, 389, 84,
	411, 64, 464, 85,
	184, 135, 238, 151,
	259, 136, 311, 151,
	336, 135, 388, 152,
	410, 136, 465, 152,
	184, 203, 238, 220,
	263, 204, 311, 220,
	338, 203, 388, 219,
	408, 203, 464, 219,
	185, 272, 237, 291,
	260, 272, 313, 291,
	337, 271, 388, 290,
	411, 271, 463, 289,
};

static const int16 g_linksHorizontal1[100] = {
	136, 24, 206, 38,
	216, 24, 284, 38,
	294, 24, 363, 38,
	374, 24, 442, 38,
	452, 24, 520, 38,
	136, 97, 205, 110,
	215, 97, 284, 110,
	294, 98, 363, 110,
	373, 98, 442, 110,
	451, 98, 520, 110,
	137, 170, 206, 182,
	216, 170, 284, 182,
	294, 170, 363, 182,
	373, 170, 442, 182,
	452, 170, 520, 182,
	137, 242, 204, 255,
	216, 242, 284, 255,
	295, 242, 363, 254,
	374, 242, 441, 254,
	452, 242, 520, 254,
	137, 315, 204, 328,
	216, 315, 284, 328,
	295, 315, 362, 327,
	374, 315, 441, 327,
	452, 315, 520, 327,
};

static const int16 g_linksVertical1[64] = {
	205, 36, 217, 98,
	284, 36, 295, 99,
	362, 36, 374, 99,
	441, 37, 452, 99,
	205, 109, 217, 171,
	284, 109, 295, 171,
	363, 109, 374, 171,
	441, 109, 452, 170,
	205, 181, 217, 244,
	284, 181, 295, 243,
	362, 181, 374, 243,
	441, 181, 452, 243,
	205, 254, 217, 316,
	284, 254, 295, 316,
	362, 254, 374, 316,
	441, 254, 453, 316,
};

static const int16 g_linksHorizontal2[100] = {
	135, 26, 206, 39,
	214, 27, 284, 39,
	292, 27, 362, 40,
	370, 28, 439, 41,
	447, 29, 515, 40,
	135, 98, 206, 111,
	214, 99, 284, 111,
	292, 99, 362, 111,
	370, 99, 439, 112,
	447, 100, 515, 112,
	135, 170, 206, 182,
	214, 170, 284, 182,
	293, 170, 362, 182,
	370, 170, 438, 182,
	447, 170, 514, 182,
	135, 241, 206, 255,
	214, 241, 285, 254,
	293, 241, 362, 254,
	370, 241, 438, 253,
	447, 241, 515, 253,
	135, 314, 206, 327,
	214, 314, 284, 326,
	292, 313, 362, 325,
	370, 313, 438, 325,
	447, 312, 515, 324,
};

static const int16 g_linksVertical2[64] = {
	204, 37, 216, 100,
	283, 38, 294, 100,
	360, 39, 372, 100,
	437, 39, 449, 102,
	204, 109, 216, 171,
	283, 110, 294, 171,
	360, 110, 371, 171,
	437, 111, 448, 171,
	204, 181, 216, 242,
	283, 181, 294, 242,
	360, 181, 371, 242,
	436, 181, 448, 242,
	204, 253, 216, 315,
	283, 252, 294, 315,
	360, 252, 372, 314,
	436, 251, 448, 314,
};

} // End of namespace CircuitPuzzleTables

struct CircuitPuzzleAIEvaluator {
	CircuitPuzzleAIEvaluator();

	static const uint kMaxMovesToReach = CircuitPuzzle::kBoardWidth * CircuitPuzzle::kBoardHeight * 2;

	uint stepsToReach[CircuitPuzzle::kBoardWidth][CircuitPuzzle::kBoardHeight];
};

class CircuitPuzzleVisitedSet {
public:
	CircuitPuzzleVisitedSet();

	void set(const Common::Point &coord);
	bool get(const Common::Point &coord) const;
	void clear();

private:
	uint32 _bits;
};

CircuitPuzzleVisitedSet::CircuitPuzzleVisitedSet() : _bits(0) {
}

void CircuitPuzzleVisitedSet::set(const Common::Point &coord) {
	int bit = coord.y * static_cast<int>(CircuitPuzzle::kBoardWidth) + coord.x;
	_bits |= (1u << bit);
}

bool CircuitPuzzleVisitedSet::get(const Common::Point &coord) const {
	int bit = coord.y * static_cast<int>(CircuitPuzzle::kBoardWidth) + coord.x;
	return (_bits & (1u << bit)) != 0;
}

void CircuitPuzzleVisitedSet::clear() {
	_bits = 0;
}

CircuitPuzzleAIEvaluator::CircuitPuzzleAIEvaluator() {
	for (uint x = 0; x < CircuitPuzzle::kBoardWidth; x++)
		for (uint y = 0; y < CircuitPuzzle::kBoardHeight; y++)
			stepsToReach[x][y] = kMaxMovesToReach;
}

CircuitPuzzle::Action::Action() : _direction(kCellDirectionDown) {
}

CircuitPuzzle::CircuitPuzzle(int layout) : _havePreviousAction(false) {
	_startPoint = Common::Point(0, 0);
	_goalPoint = Common::Point(kBoardWidth - 1, 0);

	const int16 *linksHoriz = nullptr;
	const int16 *linksVert = nullptr;
	const int16 *barriersHoriz = nullptr;
	const int16 *barriersVert = nullptr;

	if (layout == 1) {
		linksHoriz = CircuitPuzzleTables::g_linksHorizontal1;
		linksVert = CircuitPuzzleTables::g_linksVertical1;
		barriersHoriz = CircuitPuzzleTables::g_barriersHorizontal1;
		barriersVert = CircuitPuzzleTables::g_barriersVertical1;
	} else if (layout == 2) {
		linksHoriz = CircuitPuzzleTables::g_linksHorizontal2;
		linksVert = CircuitPuzzleTables::g_linksVertical2;
		barriersHoriz = CircuitPuzzleTables::g_barriersHorizontal2;
		barriersVert = CircuitPuzzleTables::g_barriersVertical2;
	} else
		error("Unknown circuit screen layout");

	// Pre-connect the side rails
	for (uint i = 0; i < (kBoardHeight - 1u); i++) {
		*getConnectionState(Common::Point(0, i), KDirectionDown) = kLinkStateConnected;
		*getConnectionState(Common::Point(kBoardWidth - 1, i), KDirectionDown) = kLinkStateConnected;
	}

	// Block edge points
	for (uint i = 0; i < kBoardWidth; i++)
		_cells[i][kBoardHeight - 1]._downLink = kLinkStateBlocked;
	for (uint i = 0; i < kBoardHeight; i++)
		_cells[kBoardWidth - 1][i]._rightLink = kLinkStateBlocked;

	// Barriers are traced from pixel matches, but links are traced from the highlight boxes.
	// Since the highlight boxes are (1,1) larger than the clipping box of the animation, and because
	// the coordinates are inclusive, we need to add (1,1) to barrier sizes, but not link sizes, since the
	// inclusive (+1,+1) cancels out from the oversize (-1,-1) adjustment.

	// Resolve horizontal links and barriers
	for (uint y = 0; y < kBoardHeight; y++) {
		for (uint x = 0; x < (kBoardWidth - 1u); x++) {
			uint rectDataOffset = (x + y * (kBoardWidth - 1u)) * 4u;

			CellRectSpec &rectSpec = _cellRectSpecs[x][y];
			rectSpec._rightBarrierRect = Common::Rect(barriersHoriz[rectDataOffset + 0], barriersHoriz[rectDataOffset + 1], barriersHoriz[rectDataOffset + 2] + 1, barriersHoriz[rectDataOffset + 3] + 1);
			rectSpec._rightLinkRect = Common::Rect(linksHoriz[rectDataOffset + 0], linksHoriz[rectDataOffset + 1], linksHoriz[rectDataOffset + 2], linksHoriz[rectDataOffset + 3]);
		}
	}

	// Resolve vertical links and barriers.  Skip the first and last column.
	for (uint y = 0; y < (kBoardHeight - 1u); y++) {
		for (uint x = 1; x < (kBoardWidth - 1u); x++) {
			uint rectDataOffset = ((x - 1) + y * (kBoardWidth - 2u)) * 4u;

			CellRectSpec &rectSpec = _cellRectSpecs[x][y];
			rectSpec._downBarrierRect = Common::Rect(barriersVert[rectDataOffset + 0], barriersVert[rectDataOffset + 1], barriersVert[rectDataOffset + 2] + 1, barriersVert[rectDataOffset + 3] + 1);
			rectSpec._downLinkRect = Common::Rect(linksVert[rectDataOffset + 0], linksVert[rectDataOffset + 1], linksVert[rectDataOffset + 2], linksVert[rectDataOffset + 3]);
		}
	}
}

bool CircuitPuzzle::executeAIAction(Common::RandomSource &randomSource, Common::Point &outCoord, CellDirection &outBlockDirection) {
	// Don't know exactly what algorithm Schizm uses, we use something that approximates the original
	// pretty well most of the time:
	// - Identify all connection paths that are tied for the fewest number of new connections required to win.
	// - Enumerate all open connections on those paths.
	// - If the previous move blocked a horizontal connection (i.e. with a vertical barrier), prioritize
	//   connections on the same X coordinate as that block.
	// - Block a random connection from the candidates.
	//
	// There seem to be times that Schizm doesn't do this.  In particular, Schizm will (rarely) fail to block
	// a connection even if it's the only connection that will immediately win the puzzle for the player.
	//
	// It also doesn't prioritize making moves that will immediately win for the AI.
	CircuitPuzzleAIEvaluator evaluator;

	computeStepsToReach(evaluator);

	uint stepsToReachGoal = evaluator.stepsToReach[_goalPoint.x][_goalPoint.y];

	if (stepsToReachGoal == 0 || stepsToReachGoal == CircuitPuzzleAIEvaluator::kMaxMovesToReach)
		return false;

	const uint kMaxLinks = kBoardWidth * kBoardHeight * 2;

	Action potentialBlocks[kMaxLinks];
	uint numPotentialBlocks = 0;

	Common::Point pointsList1[kMaxLinks];
	Common::Point pointsList2[kMaxLinks];

	Common::Point *pointsToFloodFill = pointsList1;
	Common::Point *pointsToProspect = pointsList2;

	uint numPointsToFloodFill = 1;
	uint numPointsToProspect = 0;

	pointsToFloodFill[0] = _goalPoint;

	CircuitPuzzleVisitedSet visitedSet;

	uint prospectLevel = stepsToReachGoal;

	while (prospectLevel > 0) {
		floodFillLinks(pointsToFloodFill, numPointsToFloodFill, visitedSet);

		for (uint i = 0; i < numPointsToFloodFill; i++) {
			const Common::Point &pt = pointsToFloodFill[i];

			for (uint dir = 0; dir < kDirectionCount; dir++) {
				const LinkState *linkState = getConnectionState(pt, static_cast<Direction>(dir));
				if (linkState && (*linkState) == kLinkStateOpen) {
					Common::Point connectedPoint = getConnectedPoint(pt, static_cast<Direction>(dir));

					if (!visitedSet.get(connectedPoint)) {
						visitedSet.set(connectedPoint);

						if (evaluator.stepsToReach[connectedPoint.x][connectedPoint.y] + 1u == prospectLevel) {
							// This point is on the shortest path
							Action action;

							switch (dir) {
							case kDirectionUp:
								action._point = connectedPoint;
								action._direction = kCellDirectionDown;
								break;
							case KDirectionDown:
								action._point = pt;
								action._direction = kCellDirectionDown;
								break;
							case kDirectionLeft:
								action._point = connectedPoint;
								action._direction = kCellDirectionRight;
								break;
							case kDirectionRight:
								action._point = pt;
								action._direction = kCellDirectionRight;
								break;
							default:
								error("Internal error: Bad direction");
								return false;
							}

							potentialBlocks[numPotentialBlocks] = action;
							numPotentialBlocks++;

							pointsToProspect[numPointsToProspect] = connectedPoint;
							numPointsToProspect++;
						}
					}
				}
			}
		}

		Common::Point *tempList = pointsToFloodFill;
		pointsToFloodFill = pointsToProspect;
		pointsToProspect = tempList;

		numPointsToFloodFill = numPointsToProspect;
		numPointsToProspect = 0;

		prospectLevel--;
	}

	if (numPotentialBlocks == 0)
		return false;

	// All potential blocks are now on the shortest path.
	// Try to mimic some of the AI behavior of Schizm to form wall advances.
	// The highest-priority move is one that runs parallel to the previous move.
	uint selectedBlock = 0;
	if (numPotentialBlocks > 1) {
		uint blockQualities[kMaxLinks];
		for (uint i = 0; i < numPotentialBlocks; i++)
			blockQualities[i] = 0;

		uint highestQuality = 0;
		if (_havePreviousAction) {
			for (uint i = 0; i < numPotentialBlocks; i++) {
				uint quality = 0;

				const Action &pblock = potentialBlocks[i];

				// We don't want to favor horizontal walls because otherwise that triggers are degenerate behavior where the player can run a wall
				// directly across and the AI will keeps inserting horizontal walls parallel to the player action.
				bool isWallBlock = false;
				if (_previousAction._direction == kCellDirectionRight && pblock._direction == kCellDirectionRight && _previousAction._point.x == pblock._point.x)
					isWallBlock = true;
#if 0
				else if (_previousAction._direction == kCellDirectionDown && pblock._direction == kCellDirectionDown && _previousAction._point.y == pblock._point.y)
					isWallBlock = true;
#endif

				// If this forms a vertical wall, it's quality 2
				if (isWallBlock)
					quality = 2;
				else {
					// If this forms a corner, it's quality 1 (disabled, this seems less accurate)
#if 0
					if (_previousAction._direction != pblock._direction) {
						Common::Point prevAdjacent = _previousAction._point;
						if (_previousAction._direction == kCellDirectionRight)
							prevAdjacent.x++;
						else if (_previousAction._direction == kCellDirectionDown)
							prevAdjacent.y++;

						Common::Point pblockAdjacent = pblock._point;
						if (pblock._direction == kCellDirectionRight)
							pblockAdjacent.x++;
						else if (pblock._direction == kCellDirectionDown)
							pblockAdjacent.y++;

						if (prevAdjacent == pblock._point || prevAdjacent == pblockAdjacent || _previousAction._point == pblock._point || _previousAction._point == pblockAdjacent)
							quality = 1;
					}
#endif
				}

				blockQualities[i] = quality;
				if (quality > highestQuality)
					highestQuality = quality;
			}
		}

		uint blocksInHighestQuality[kMaxLinks];
		uint numBlocksInHighestQuality = 0;

		for (uint i = 0; i < numPotentialBlocks; i++) {
			if (blockQualities[i] == highestQuality) {
				blocksInHighestQuality[numBlocksInHighestQuality] = i;
				numBlocksInHighestQuality++;
			}
		}

		if (numBlocksInHighestQuality == 1)
			selectedBlock = blocksInHighestQuality[0];
		else {
			assert(numBlocksInHighestQuality > 1);
			selectedBlock = blocksInHighestQuality[randomSource.getRandomNumber(numBlocksInHighestQuality - 1)];
		}
	}

	const Action &pblock = potentialBlocks[selectedBlock];

	outCoord = pblock._point;
	outBlockDirection = pblock._direction;

	if (pblock._direction == kCellDirectionDown)
		_cells[pblock._point.x][pblock._point.y]._downLink = kLinkStateBlocked;
	if (pblock._direction == kCellDirectionRight)
		_cells[pblock._point.x][pblock._point.y]._rightLink = kLinkStateBlocked;

	_havePreviousAction = true;
	_previousAction = pblock;

	return true;
}

void CircuitPuzzle::addLink(const Common::Point &coord, CellDirection direction) {
	validateCoord(coord);

	CellState &cell = _cells[coord.x][coord.y];

	LinkState *linkState = nullptr;
	if (direction == kCellDirectionDown)
		linkState = &cell._downLink;
	else if (direction == kCellDirectionRight)
		linkState = &cell._rightLink;

	if (linkState == nullptr || (*linkState) != kLinkStateOpen)
		error("Internal error: Circuit link state was invalid");

	*linkState = kLinkStateConnected;
}

CircuitPuzzle::Conclusion CircuitPuzzle::checkConclusion() const {
	CircuitPuzzleAIEvaluator evaluator;

	computeStepsToReach(evaluator);

	uint stepsToReachGoal = evaluator.stepsToReach[_goalPoint.x][_goalPoint.y];

	if (stepsToReachGoal == 0)
		return kConclusionPlayerWon;

	if (stepsToReachGoal == CircuitPuzzleAIEvaluator::kMaxMovesToReach)
		return kConclusionPlayerLost;

	return kConclusionNone;
}

const CircuitPuzzle::CellRectSpec *CircuitPuzzle::getCellRectSpec(const Common::Point &coord) const {
	validateCoord(coord);

	return &_cellRectSpecs[coord.x][coord.y];
}

bool CircuitPuzzle::isCellDownLinkOpen(const Common::Point &coord) const {
	validateCoord(coord);

	return _cells[coord.x][coord.y]._downLink == kLinkStateOpen;
}

bool CircuitPuzzle::isCellRightLinkOpen(const Common::Point &coord) const {
	validateCoord(coord);

	return _cells[coord.x][coord.y]._rightLink == kLinkStateOpen;
}

CircuitPuzzle::CellState::CellState() : _downLink(kLinkStateOpen), _rightLink(kLinkStateOpen) {
}

Common::Point CircuitPuzzle::getConnectedPoint(const Common::Point &coord, Direction direction) {
	switch (direction) {
	case kDirectionUp:
		return Common::Point(coord.x, coord.y - 1);
	case KDirectionDown:
		return Common::Point(coord.x, coord.y + 1);
	case kDirectionLeft:
		return Common::Point(coord.x - 1, coord.y);
	case kDirectionRight:
		return Common::Point(coord.x + 1, coord.y);
	default:
		return coord;
	};
}

CircuitPuzzle::LinkState *CircuitPuzzle::getConnectionState(const Common::Point &coord, Direction direction) {
	if (!isPositionValid(coord))
		return nullptr;

	switch (direction) {
	case kDirectionUp:
		if (coord.y == 0)
			return nullptr;
		return &_cells[coord.x][coord.y - 1]._downLink;
	case KDirectionDown:
		if (coord.y == static_cast<int>(kBoardHeight - 1))
			return nullptr;

		return &_cells[coord.x][coord.y]._downLink;
	case kDirectionLeft:
		if (coord.x <= 0)
			return nullptr;
		return &_cells[coord.x - 1][coord.y]._rightLink;
	case kDirectionRight:
		if (coord.x == static_cast<int>(kBoardWidth - 1))
			return nullptr;

		return &_cells[coord.x][coord.y]._rightLink;
	default:
		return nullptr;
	};
}

const CircuitPuzzle::LinkState *CircuitPuzzle::getConnectionState(const Common::Point &coord, Direction direction) const {
	return const_cast<CircuitPuzzle *>(this)->getConnectionState(coord, direction);
}

bool CircuitPuzzle::isPositionValid(const Common::Point &coord) {
	if (coord.x < 0 || coord.y < 0 || coord.x >= static_cast<int>(kBoardWidth) || coord.y >= static_cast<int>(kBoardHeight))
		return false;

	return true;
}

void CircuitPuzzle::computeStepsToReach(CircuitPuzzleAIEvaluator &evaluator) const {
	const uint kMaxLinks = kBoardWidth * kBoardHeight * 2;

	Common::Point pointsList1[kMaxLinks];
	Common::Point pointsList2[kMaxLinks];

	Common::Point *pointsToFloodFill = pointsList1;
	Common::Point *pointsToProspect = pointsList2;

	uint numPointsToFloodFill = 1;
	uint numPointsToProspect = 0;

	uint floodFillValue = 0;
	pointsToFloodFill[0] = _startPoint;

	for (uint x = 0; x < kBoardWidth; x++)
		for (uint y = 0; y < kBoardHeight; y++)
			evaluator.stepsToReach[x][y] = CircuitPuzzleAIEvaluator::kMaxMovesToReach;

	CircuitPuzzleVisitedSet visitedSet;
	while (numPointsToFloodFill > 0) {
		floodFillLinks(pointsToFloodFill, numPointsToFloodFill, visitedSet);

		for (uint i = 0; i < numPointsToFloodFill; i++) {
			const Common::Point &pt = pointsToFloodFill[i];

			evaluator.stepsToReach[pt.x][pt.y] = floodFillValue;

			for (uint dir = 0; dir < kDirectionCount; dir++) {
				const LinkState *linkState = getConnectionState(pt, static_cast<Direction>(dir));
				if (linkState && (*linkState) == kLinkStateOpen) {
					Common::Point connectedPoint = getConnectedPoint(pt, static_cast<Direction>(dir));

					if (!visitedSet.get(connectedPoint)) {
						visitedSet.set(connectedPoint);

						pointsToProspect[numPointsToProspect] = connectedPoint;
						numPointsToProspect++;
					}
				}
			}
		}

		Common::Point *tempList = pointsToFloodFill;
		pointsToFloodFill = pointsToProspect;
		pointsToProspect = tempList;

		numPointsToFloodFill = numPointsToProspect;
		numPointsToProspect = 0;

		floodFillValue++;
	}
}

void CircuitPuzzle::floodFillLinks(Common::Point *pointsList, uint &listSize, CircuitPuzzleVisitedSet &visitedSet) const {
	for (uint i = 0; i < listSize; i++) {
		const Common::Point &pt = pointsList[i];

		visitedSet.set(pt);
		for (uint dir = 0; dir < kDirectionCount; dir++) {
			const LinkState *linkState = getConnectionState(pt, static_cast<Direction>(dir));
			if (linkState && (*linkState) == kLinkStateConnected) {
				Common::Point connectedPoint = getConnectedPoint(pt, static_cast<Direction>(dir));

				if (!visitedSet.get(connectedPoint)) {
					pointsList[listSize] = connectedPoint;
					listSize++;
				}
			}
		}
	}
}


void CircuitPuzzle::validateCoord(const Common::Point &coord) {
	assert(coord.x >= 0 && coord.y >= 0 && coord.x < static_cast<int>(kBoardWidth) && coord.y < static_cast<int>(kBoardHeight));
}


} // End of namespace VCruise
