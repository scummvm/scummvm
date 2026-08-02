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

#include "mediastation/mediascript/collection.h"
#include "mediastation/mediascript/function.h"
#include "mediastation/mediastation.h"
#include "mediastation/minigames/barbiemaze.h"

namespace MediaStation {

enum BarbieMazeDirectionMask {
	kUp = 0x01,
	kRight = 0x02,
	kDown = 0x04,
	kLeft = 0x08
};

void FunctionManager::script_MazeGenerate(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args.size() < 2 || args.size() > 3) {
		warning("%s: expected 2 to 3 arguments, got %d", __func__, args.size());
		Collection *emptyCollection = new Collection();
		returnValue.setToCollection(emptyCollection);
		return;
	}

	// The original first creates a bitmap that shows where there is a wall,
	// then later this bitmap is masked, so do the same here.
	int16 width = static_cast<int16>(args[0].asFloat());
	int16 height = static_cast<int16>(args[1].asFloat());
	Common::Array<byte> mazeBitmap = BarbieMazeMinigame::doCreateMaze(width, height);

	// Convert the maze bitmap into a mask of allowed movement directions for each cell.
	// which is what scripts want to see.
	Common::Array<byte> maskData = BarbieMazeMinigame::doMaskMaze(width, height, mazeBitmap);
	Collection *result = new Collection();
	for (byte b : maskData) {
		ScriptValue value;
		value.setToFloat(static_cast<double>(b));
		result->push_back(value);
	}
	// Put the width and height at the end of the collection.
	ScriptValue widthValue;
	widthValue.setToFloat(static_cast<double>(width));
	result->push_back(widthValue);
	ScriptValue heightValue;
	heightValue.setToFloat(static_cast<double>(height));
	result->push_back(heightValue);

	// The return value takes ownership of the collection.
	returnValue.setToCollection(result);
}

void FunctionManager::script_MazeApplyMoveMask(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args.size() != 3) {
		warning("%s: Expected 3 arguments, got %d", __func__, args.size());
		returnValue.setToFloat(-1.0);
		return;
	}

	// A script told us where the user wants to move and gave us the maze direction mask
	// at the user's current spot. Determine if the user can move to the new spot.
	int16 xDirection = static_cast<int16>(args[0].asFloat());
	int16 yDirection = static_cast<int16>(args[1].asFloat());
	int16 possibleMoves = static_cast<int16>(args[2].asFloat());
	int16 requestedMove = 0;
	bool isHorizontalMoveRequested = (xDirection != 0);
	if (isHorizontalMoveRequested) {
		requestedMove |= 0x02;
	}
	bool isVerticalMoveRequested = (yDirection != 0);
	if (isVerticalMoveRequested) {
		requestedMove |= 0x04;
	}

	int16 allowedMoves = possibleMoves & requestedMove;
	switch (allowedMoves) {
	case 0x02:
		// Horizontal movement allowed.
		returnValue.setToFloat(4.0);
		break;

	case 0x04:
		// Vertical movement allowed.
		returnValue.setToFloat(2.0);
		break;

	case 0x06:
		// Both horizontal and vertical allowed (diagonal).
		returnValue.setToFloat(3.0);
		break;

	default:
		// Movement not allowed or no movement requested.
		returnValue.setToFloat(-1.0);
		break;
	}
}

Common::Array<byte> BarbieMazeMinigame::doCreateMaze(uint16 width, uint16 height) {
	// The maze bitmap dimensions are double the playable dimensions because each wall
	// must also be represented in the bitmap. 1 means wall, 0 means no wall.
	uint16 mazeWidth = width * 2 + 1;
	uint16 mazeHeight = height * 2 + 1;
	Common::Array<byte> mazeBitmap(mazeWidth * mazeHeight);

	// The outer border is always all wall.
	for (uint16 y = 0; y < mazeHeight; y++) {
		mazeBitmap[mazeWidth * y] = 1;
		mazeBitmap[mazeWidth * y + (mazeWidth - 1)] = 1;
	}
	for (uint16 x = 0; x < mazeWidth; x++) {
		mazeBitmap[x] = 1;
		mazeBitmap[mazeWidth * (mazeHeight - 1) + x] = 1;
	}

	uint16 totalCellsRemaining = (height - 1) * (width - 1);
	while (totalCellsRemaining > 0) {
		// Pick a random unvisited cell by index.
		uint16 targetCellIndex = g_engine->_randomSource.getRandomNumber(totalCellsRemaining - 1) + 1;

		uint16 totalCellsSeen = 1;
		for (uint16 x = 2; x < (mazeWidth - 1); x += 2) {
			for (uint16 y = 2; y < (mazeHeight - 1); y += 2) {
				bool isCellAlreadyVisited = (mazeBitmap[mazeWidth * y + x] != 0);
				if (isCellAlreadyVisited) {
					continue;
				}

				if (totalCellsSeen != targetCellIndex) {
					totalCellsSeen++;
					continue;
				}

				// Carve a random walk from this cell.
				uint16 currentX = x;
				uint16 currentY = y;
				bool reachedVisitedCell = false;
				while (!reachedVisitedCell) {
					mazeBitmap[mazeWidth * currentY + currentX] = 1;
					totalCellsRemaining--;

					uint direction = g_engine->_randomSource.getRandomNumber(3);
					switch (direction) {
					case 0: // Right.
						mazeBitmap[mazeWidth * currentY + currentX + 1] = 1;
						currentX += 2;
						break;

					case 1: // Left.
						mazeBitmap[mazeWidth * currentY + currentX - 1] = 1;
						currentX -= 2;
						break;

					case 2: // Down.
						mazeBitmap[mazeWidth * (currentY + 1) + currentX] = 1;
						currentY += 2;
						break;

					default: // Up.
						mazeBitmap[mazeWidth * (currentY - 1) + currentX] = 1;
						currentY -= 2;
						break;
					}

					reachedVisitedCell = (mazeBitmap[mazeWidth * currentY + currentX] != 0);
				}

				y = mazeHeight;
				x = mazeWidth;
			}
		}
	}
	return mazeBitmap;
}

Common::Array<byte> BarbieMazeMinigame::doMaskMaze(uint16 width, uint16 height, Common::Array<byte> &mazeBitmap) {
	uint16 maskSize = (width + 1) * (height + 1);
	Common::Array<byte> maskData(maskSize);
	uint16 mazeWidth = width * 2 + 1;

	maskData[0] = kUp | kRight | kDown | kLeft;
	for (uint16 y = 1; y <= height; y++) {
		maskData[(width + 1) * y] = kUp | kDown | kLeft;
	}
	for (uint16 x = 1; x <= width; x++) {
		maskData[x] = kRight | kDown | kLeft;
	}

	uint16 mazeX = 1;
	for (uint16 x = 1; x <= width; x++) {
		uint16 mazeY = 1;
		for (uint16 y = 1; y <= height; y++) {
			byte mask = 0;

			if (mazeBitmap[mazeWidth * (mazeY - 1) + mazeX] == 0) {
				mask |= kUp;
			}
			if (mazeBitmap[mazeWidth * mazeY + mazeX + 1] == 0) {
				mask |= kRight;
			}
			if (mazeBitmap[mazeWidth * (mazeY + 1) + mazeX] == 0) {
				mask |= kDown;
			}
			if (mazeBitmap[mazeWidth * mazeY + mazeX - 1] == 0) {
				mask |= kLeft;
			}

			maskData[(width + 1) * y + x] = mask;
			mazeY += 2;
		}
		mazeX += 2;
	}
	return maskData;
}

void BarbieMazeMinigame::solve(const Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	Collection *maskData = args[0].asCollection();
	Common::Point startPos(
		static_cast<int16>(args[1].asFloat()),
		static_cast<int16>(args[2].asFloat()));
	Common::Point endPos(
		static_cast<int16>(args[3].asFloat()),
		static_cast<int16>(args[4].asFloat()));
	Common::Point mazeDimensions(
		static_cast<int16>(maskData->operator[](maskData->size() - 2).asFloat() + 1),
		static_cast<int16>(maskData->operator[](maskData->size() - 1).asFloat() + 1));

	uint startIndex = startPos.y * mazeDimensions.x + startPos.x;
	uint endIndex = endPos.y * mazeDimensions.x + endPos.x;

	Collection *resultPath = new Collection;
	if (startIndex == endIndex) {
		ScriptValue endIndexValue;
		endIndexValue.setToFloat(endIndex);
		resultPath->push_back(endIndexValue);
	} else {
		Common::Array<uint> visitedPositions;
		findShortestPath(*maskData, mazeDimensions, startIndex, endIndex, *resultPath, visitedPositions);
	}

	// The return value takes ownership of the collection.
	returnValue.setToCollection(resultPath);
}

void BarbieMazeMinigame::findShortestPath(const Collection &maskData, Common::Point mazeDimensions,
	uint startIndex, uint endIndex,
	Collection &pathResult, Common::Array<uint> &visitedCells) {
	for (uint visitedCellIndex : visitedCells) {
		bool alreadyVisitedCell = (startIndex == visitedCellIndex);
		if (alreadyVisitedCell) {
			return;
		}
	}
	visitedCells.push_back(startIndex);

	bool hasReachedEnd = (startIndex == endIndex);
	if (hasReachedEnd) {
		bool isShorterThanBestPathSoFar = (pathResult.size() == 0 || visitedCells.size() < pathResult.size());
		if (isShorterThanBestPathSoFar) {
			pathResult.clear();
			for (uint visitedIndex : visitedCells) {
				ScriptValue nextPosIndexValue;
				nextPosIndexValue.setToFloat(visitedIndex);
				pathResult.push_back(nextPosIndexValue);
			}
		}

	} else {
		ScriptValue allowedDirectionsValue = maskData[startIndex];
		uint16 allowedDirections = static_cast<uint16>(allowedDirectionsValue.asFloat());

		if (allowedDirections & kLeft) {
			findShortestPath(maskData, mazeDimensions,
				startIndex - 1, endIndex,
				pathResult, visitedCells);
		}
		if (allowedDirections & kDown) {
			findShortestPath(maskData, mazeDimensions,
				startIndex + mazeDimensions.x, endIndex,
				pathResult, visitedCells);
		}
		if (allowedDirections & kRight) {
			findShortestPath(maskData, mazeDimensions,
				startIndex + 1, endIndex,
				pathResult, visitedCells);
		}
		if (allowedDirections & kUp) {
			findShortestPath(maskData, mazeDimensions,
				startIndex - mazeDimensions.x, endIndex,
				pathResult, visitedCells);
		}
	}

	if (visitedCells.size() > 0) {
		visitedCells.remove_at(visitedCells.size() - 1);
	}
}

} // End of namespace MediaStation
