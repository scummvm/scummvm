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
#include "common/debug.h"
#include "common/scummsys.h"

#include "pelrock/pathfinding.h"
#include "pelrock/types.h"
#include "pelrock/util.h"

namespace Pelrock {

bool findPath(int sourceX, int sourceY, int targetX, int targetY, Common::Array<WalkBox> &walkboxes, PathContext *context, HotSpot *hotspot) {

	if (context->pathBuffer == nullptr) {
		context->pathBuffer = (byte *)malloc(kMaxPathLength);
	}
	if (context->movementBuffer == nullptr) {
		context->movementBuffer = (MovementStep *)malloc(kMaxMovementSteps * sizeof(MovementStep));
	}

	int startX = sourceX;
	int startY = sourceY;
	Common::Point target = calculateWalkTarget(walkboxes, targetX, targetY, nullptr);
	targetX = target.x;
	targetY = target.y;

	byte startBox = findWalkboxForPoint(walkboxes, startX, startY);
	byte destBox = findWalkboxForPoint(walkboxes, targetX, targetY);

	// Check if both points are in valid walkboxes
	if (startBox == 0xFF || destBox == 0xFF) {
		debug("Error: Start or destination not in any walkbox\n");
		return false;
	}
	// Special case: same walkbox
	if (startBox == destBox) {
		// Generate direct movement
		MovementStep direct_step;
		direct_step.flags = 0;
		if (startX < targetX) {
			direct_step.distanceX = targetX - startX;
			direct_step.flags |= kMoveRight;
		} else {
			direct_step.distanceX = startX - targetX;
			direct_step.flags |= kMoveLeft;
		}

		if (startY < targetY) {
			direct_step.distanceY = targetY - startY;
			direct_step.flags |= kMoveDown;
		} else {
			direct_step.distanceY = startY - targetY;
			direct_step.flags |= kMoveUp;
		}

		context->movementBuffer[0] = direct_step;
		context->movementCount = 1;
	} else {
		// Build walkbox path
		context->pathLength = buildWalkboxPath(walkboxes, startBox, destBox, context->pathBuffer);
		if (context->pathLength == 0) {
			debug("Error: No path found\n");
			return false;
		}

		// Generate movement steps
		context->movementCount = generateMovementSteps(walkboxes, context->pathBuffer, context->pathLength, startX, startY, targetX, targetY, context->movementBuffer);
	}
	return true;
}

Common::Point calculateWalkTarget(Common::Array<WalkBox> &walkboxes, int sourceX, int sourceY, HotSpot *hotspot) {

	if (hotspot != nullptr) {
		// if there is a hotspot then the source is the center of the hotspot.
		sourceX = hotspot->x + hotspot->w / 2;
		sourceY = hotspot->y + hotspot->h / 2;
	}

	// Find nearest walkbox
	uint32 minDistance = 0xFFFF;
	int bestXDistance = 0;
	int bestYDistance = 0;
	int bestXDirection = 0; // 0 = left/subtract, 1 = right/add
	int bestYDirection = 0; // 0 = up/subtract, 1 = down/add

	for (size_t i = 0; i < walkboxes.size(); i++) {
		int xDistance = 0;
		int xDirection = 0;
		int yDistance = 0;
		int yDirection = 0;

		// Calculate X distance with direction
		if (sourceX < walkboxes[i].x) {
			xDistance = walkboxes[i].x - sourceX;
			xDirection = 1; // RIGHT
		} else if (sourceX > walkboxes[i].x + walkboxes[i].w) {
			// KEY: subtract 1 from right edge
			xDistance = sourceX - (walkboxes[i].x + walkboxes[i].w - 1);
			xDirection = 0; // LEFT
		}

		// Calculate Y distance with direction
		if (sourceY < walkboxes[i].y) {
			yDistance = walkboxes[i].y - sourceY;
			yDirection = 1; // DOWN
		} else if (sourceY > walkboxes[i].y + walkboxes[i].h) {
			// KEY: subtract 1 from bottom edge
			yDistance = sourceY - (walkboxes[i].y + walkboxes[i].h - 1);
			yDirection = 0; // UP
		}

		uint32 totalDistance = xDistance + yDistance;

		if (totalDistance < minDistance) {
			minDistance = totalDistance;
			bestXDistance = xDistance;
			bestYDistance = yDistance;
			bestXDirection = xDirection;
			bestYDirection = yDirection;
		}
	}

	// Calculate final target point
	Common::Point target;

	if (bestXDirection == 1) {
		target.x = sourceX + bestXDistance;
	} else {
		target.x = sourceX - bestXDistance;
	}

	if (bestYDirection == 1) {
		target.y = sourceY + bestYDistance;
	} else {
		target.y = sourceY - bestYDistance;
	}

	return target;
}

byte findWalkboxForPoint(Common::Array<WalkBox> &walkboxes, uint16 x, uint16 y) {
	for (byte i = 0; i < walkboxes.size(); i++) {
		if (isPointInWalkbox(&walkboxes[i], x, y)) {
			return i;
		}
	}
	return 0xFF; // Not found
}

bool isPointInWalkbox(WalkBox *box, uint16 x, uint16 y) {
	return (x >= box->x &&
			x <= box->x + box->w &&
			y >= box->y &&
			y <= box->y + box->h);
}

/**
 * Check if two walkboxes overlap or touch (are adjacent)
 */
bool areWalkboxesAdjacent(WalkBox *box1, WalkBox *box2) {
	uint16 box1_x_max = box1->x + box1->w;
	uint16 box1_y_max = box1->y + box1->h;
	uint16 box2_x_max = box2->x + box2->w;
	uint16 box2_y_max = box2->y + box2->h;

	// Check if X ranges overlap
	bool xOverlap = (box1->x <= box2_x_max) && (box2->x <= box1_x_max);

	// Check if Y ranges overlap
	bool yOverlap = (box1->y <= box2_y_max) && (box2->y <= box1_y_max);

	return xOverlap && yOverlap;
}

byte getAdjacentWalkbox(Common::Array<WalkBox> &walkboxes, byte currentBoxIndex) {
	WalkBox *currentBox = &walkboxes[currentBoxIndex];

	// Mark current walkbox as visited
	currentBox->flags = 0x01;

	// Search for adjacent unvisited walkbox
	for (byte i = 0; i < walkboxes.size(); i++) {
		// Skip current walkbox
		if (i == currentBoxIndex) {
			continue;
		}

		// Skip already visited walkboxes
		if (walkboxes[i].flags == 0x01) {
			continue;
		}

		// Check if walkboxes are adjacent
		if (areWalkboxesAdjacent(currentBox, &walkboxes[i])) {
			return i;
		}
	}

	return 0xFF; // No adjacent walkbox found
}

void clearVisitedFlags(Common::Array<WalkBox> &walkboxes) {
	for (int i = 0; i < walkboxes.size(); i++) {
		walkboxes[i].flags = 0;
	}
}

uint16 buildWalkboxPath(Common::Array<WalkBox> &walkboxes, byte startBox, byte destBox, byte *pathBuffer) {

	uint16 pathIndex = 0;
	byte currentBox = startBox;

	// Initialize path with start walkbox
	pathBuffer[pathIndex++] = startBox;

	// Clear visited flags
	clearVisitedFlags(walkboxes);

	// Breadth-first search through walkboxes
	while (currentBox != destBox && pathIndex < kMaxPathLength - 1) {
		byte nextBox = getAdjacentWalkbox(walkboxes, currentBox);

		if (nextBox == 0xFF) {
			// Dead end - backtrack
			if (pathIndex > 1) {
				pathIndex--;
				currentBox = pathBuffer[pathIndex - 1];
			} else {
				// No path exists
				return 0;
			}
		} else if (nextBox == destBox) {
			// Found destination
			pathBuffer[pathIndex++] = destBox;
			break;
		} else {
			// Continue searching
			pathBuffer[pathIndex++] = nextBox;
			currentBox = nextBox;
		}
	}

	// Terminate path
	pathBuffer[pathIndex] = kPathEnd;
	debug("Built walkbox path of length %d", pathIndex);
	return pathIndex;
}

/**
 * Calculate movement needed to reach a target within a walkbox
 */
void calculateMovementToTarget(uint16 currentX, uint16 currentY, uint16 targetX, uint16 targetY, WalkBox *box, MovementStep *step) {
	step->flags = 0;
	step->distanceX = 0;
	step->distanceY = 0;

	// Calculate horizontal movement
	if (currentX < box->x) {
		// Need to move right to enter walkbox
		step->distanceX = box->x - currentX;
		step->flags |= kMoveRight;
	} else if (currentX > box->x + box->w) {
		// Need to move left to enter walkbox
		step->distanceX = currentX - (box->x + box->w);
		step->flags |= kMoveLeft;
	}

	// Calculate vertical movement
	if (currentY < box->y) {
		// Need to move down to enter walkbox
		step->distanceY = box->y - currentY;
		step->flags |= kMoveDown;
	} else if (currentY > box->y + box->h) {
		// Need to move up to enter walkbox
		step->distanceY = currentY - (box->y + box->h);
		step->flags |= kMoveUp;
	}
}

/**
 * Generate movement steps from walkbox path
 * Returns: number of movement steps generated
 */
uint16 generateMovementSteps(Common::Array<WalkBox> &walkboxes,
							 byte *pathBuffer,
							 uint16 pathLength,
							 uint16 startX, uint16 startY,
							 uint16 destX, uint16 destY,
							 MovementStep *movementBuffer) {
	uint16 currentX = startX;
	uint16 currentY = startY;
	uint16 movementIndex = 0;

	// Generate movements for each walkbox in path
	for (uint16 i = 0; i < pathLength && pathBuffer[i] != kPathEnd; i++) {
		byte boxIndex = pathBuffer[i];
		WalkBox *box = &walkboxes[boxIndex];

		MovementStep step;
		calculateMovementToTarget(currentX, currentY, destX, destY, box, &step);

		if (step.distanceX > 0 || step.distanceY > 0) {
			movementBuffer[movementIndex++] = step;

			// Update current position
			if (step.flags & kMoveRight) {
				currentX = box->x;
			} else if (step.flags & kMoveLeft) {
				currentX = box->x + box->w;
			}

			if (step.flags & kMoveDown) {
				currentY = box->y;
			} else if (step.flags & kMoveUp) {
				currentY = box->y + box->h;
			}
		}
	}

	// Final movement to exact destination
	MovementStep final_step;
	final_step.flags = 0;

	if (currentX < destX) {
		final_step.distanceX = destX - currentX;
		final_step.flags |= kMoveRight;
	} else if (currentX > destX) {
		final_step.distanceX = currentX - destX;
		final_step.flags |= kMoveLeft;
	} else {
		final_step.distanceX = 0;
	}

	if (currentY < destY) {
		final_step.distanceY = destY - currentY;
		final_step.flags |= kMoveDown;
	} else if (currentY > destY) {
		final_step.distanceY = currentY - destY;
		final_step.flags |= kMoveUp;
	} else {
		final_step.distanceY = 0;
	}

	if (final_step.distanceX > 0 || final_step.distanceY > 0) {
		movementBuffer[movementIndex++] = final_step;
	}

	return movementIndex;
}

} // End of namespace Pelrock
