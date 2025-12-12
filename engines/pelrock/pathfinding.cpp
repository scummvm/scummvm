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
#include "pelrock/util.h"

namespace Pelrock {

Common::String printMovementFlags(uint8_t flags) {
	Common::String result;
	if (flags & MOVE_HORIZ) {
		result += "HORIZ ";
	}
	if (flags & MOVE_VERT) {
		result += "VERT ";
	}
	if (flags & MOVE_DOWN) {
		result += "DOWN ";
	}
	if (flags & MOVE_LEFT) {
		result += "LEFT ";
	}
	if (flags & MOVE_UP) {
		result += "UP ";
	}
	if (flags & MOVE_RIGHT) {
		result += "RIGHT ";
	}
	return result;
}

bool findPath(int sourceX, int sourceY, int targetX, int targetY, Common::Array<WalkBox> &walkboxes, PathContext *context) {

	if (context->pathBuffer == NULL) {
		context->pathBuffer = (uint8_t *)malloc(MAX_PATH_LENGTH);
	}
	if (context->movementBuffer == NULL) {
		context->movementBuffer = (MovementStep *)malloc(MAX_MOVEMENT_STEPS * sizeof(MovementStep));
	}

	int startX = sourceX;
	int startY = sourceY;
	Common::Point target = calculateWalkTarget(walkboxes, targetX, targetY);
	targetX = target.x;
	targetY = target.y;
	debug("Startx= %d, starty= %d, destx= %d, desty= %d", startX, startY, targetX, targetY);

	uint8_t startBox = findWalkboxForPoint(walkboxes, startX, startY);
	uint8_t destBox = findWalkboxForPoint(walkboxes, targetX, targetY);

	debug("Pathfinding from (%d, %d) in box %d to (%d, %d) in box %d\n", startX, startY, startBox, targetX, targetY, destBox);
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
			direct_step.flags |= MOVE_RIGHT;
		} else {
			direct_step.distanceX = startX - targetX;
			direct_step.flags |= MOVE_LEFT;
		}

		if (startY < targetY) {
			direct_step.distanceY = targetY - startY;
			direct_step.flags |= MOVE_DOWN;
		} else {
			direct_step.distanceY = startY - targetY;
			direct_step.flags |= MOVE_UP;
		}

		context->movementBuffer[0] = direct_step;
		context->movementCount = 1;
	} else {
		// Build walkbox path
		context->pathLength = buildWalkboxPath(walkboxes, startBox, destBox, context->pathBuffer);
		debug("Walkbox path to point");
		for (int i = 0; i < context->pathLength; i++) {
			debug("Walkbox %d: %d", i, context->pathBuffer[i]);
		}
		if (context->pathLength == 0) {
			debug("Error: No path found\n");
			return false;
		}

		// Generate movement steps
		context->movementCount = generateMovementSteps(walkboxes, context->pathBuffer, context->pathLength, startX, startY, targetX, targetY, context->movementBuffer);
		// for (int i = 0; i < context->movementCount; i++) {
		// 	debug("Movement step %d: flags=\"%s\", dx=%d, dy=%d", i, printMovementFlags(context->movementBuffer[i].flags).c_str(), context->movementBuffer[i].distanceX, context->movementBuffer[i].distanceY);
		// }
	}
	return true;
}

Common::Point calculateWalkTarget(Common::Array<WalkBox> &walkboxes, int x, int y) {
	// Starting point for pathfinding
	int sourceX = x;
	int sourceY = y;

	// TODO: If hovering over a sprite/hotspot, adjust source point to sprite center
	// For now, just use mouse position

	// Find nearest walkable point in walkboxes
	uint32 minDistance = 0xFFFFFFFF;
	Common::Point bestTarget(sourceX, sourceY);

	for (size_t i = 0; i < walkboxes.size(); i++) {

		// Calculate distance from source point to this walkbox (Manhattan distance)
		int dx = 0;
		int dy = 0;

		// Calculate horizontal distance
		if (sourceX < walkboxes[i].x) {
			dx = walkboxes[i].x - sourceX;
		} else if (sourceX > walkboxes[i].x + walkboxes[i].w) {
			dx = sourceX - (walkboxes[i].x + walkboxes[i].w);
		}
		// else: sourceX is inside walkbox horizontally, dx = 0

		// Calculate vertical distance
		if (sourceY < walkboxes[i].y) {
			dy = walkboxes[i].y - sourceY;
		} else if (sourceY > walkboxes[i].y + walkboxes[i].h) {
			dy = sourceY - (walkboxes[i].y + walkboxes[i].h);
		}
		// else: sourceY is inside walkbox vertically, dy = 0

		uint32 distance = dx + dy;

		if (distance < minDistance) {
			minDistance = distance;

			// Calculate target point (nearest point on walkbox to source)
			int targetX = sourceX;
			int targetY = sourceY;

			if (sourceX < walkboxes[i].x) {
				targetX = walkboxes[i].x;
			} else if (sourceX > walkboxes[i].x + walkboxes[i].w) {
				targetX = walkboxes[i].x + walkboxes[i].w;
			}

			if (sourceY < walkboxes[i].y) {
				targetY = walkboxes[i].y;
			} else if (sourceY > walkboxes[i].y + walkboxes[i].h) {
				targetY = walkboxes[i].y + walkboxes[i].h;
			}

			bestTarget.x = targetX;
			bestTarget.y = targetY;
		}
	}

	return bestTarget;
}

uint8_t findWalkboxForPoint(Common::Array<WalkBox> &walkboxes, uint16_t x, uint16_t y) {
	for (uint8_t i = 0; i < walkboxes.size(); i++) {
		if (isPointInWalkbox(&walkboxes[i], x, y)) {
			return i;
		}
	}
	return 0xFF; // Not found
}

bool isPointInWalkbox(WalkBox *box, uint16_t x, uint16_t y) {
	return (x >= box->x &&
			x <= box->x + box->w &&
			y >= box->y &&
			y <= box->y + box->h);
}

/**
 * Check if two walkboxes overlap or touch (are adjacent)
 */
bool areWalkboxesAdjacent(WalkBox *box1, WalkBox *box2) {
	uint16_t box1_x_max = box1->x + box1->w;
	uint16_t box1_y_max = box1->y + box1->h;
	uint16_t box2_x_max = box2->x + box2->w;
	uint16_t box2_y_max = box2->y + box2->h;

	// Check if X ranges overlap
	bool xOverlap = (box1->x <= box2_x_max) && (box2->x <= box1_x_max);

	// Check if Y ranges overlap
	bool yOverlap = (box1->y <= box2_y_max) && (box2->y <= box1_y_max);

	return xOverlap && yOverlap;
}

uint8_t getAdjacentWalkbox(Common::Array<WalkBox> &walkboxes, uint8_t currentBoxIndex) {
	WalkBox *currentBox = &walkboxes[currentBoxIndex];

	// Mark current walkbox as visited
	currentBox->flags = 0x01;

	// Search for adjacent unvisited walkbox
	for (uint8_t i = 0; i < walkboxes.size(); i++) {
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

uint16_t buildWalkboxPath(Common::Array<WalkBox> &walkboxes, uint8_t startBox, uint8_t destBox, uint8_t *pathBuffer) {

	uint16_t pathIndex = 0;
	uint8_t currentBox = startBox;

	// Initialize path with start walkbox
	pathBuffer[pathIndex++] = startBox;

	// Clear visited flags
	clearVisitedFlags(walkboxes);

	// Breadth-first search through walkboxes
	while (currentBox != destBox && pathIndex < MAX_PATH_LENGTH - 1) {
		uint8_t nextBox = getAdjacentWalkbox(walkboxes, currentBox);

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
	pathBuffer[pathIndex] = PATH_END;
	debug("Built walkbox path of length %d", pathIndex);
	return pathIndex;
}

/**
 * Calculate movement needed to reach a target within a walkbox
 */
void calculateMovementToTarget(uint16_t currentX, uint16_t currentY, uint16_t targetX, uint16_t targetY, WalkBox *box, MovementStep *step) {
	step->flags = 0;
	step->distanceX = 0;
	step->distanceY = 0;

	// Calculate horizontal movement
	if (currentX < box->x) {
		// Need to move right to enter walkbox
		step->distanceX = box->x - currentX;
		step->flags |= MOVE_RIGHT;
	} else if (currentX > box->x + box->w) {
		// Need to move left to enter walkbox
		step->distanceX = currentX - (box->x + box->w);
		step->flags |= MOVE_LEFT;
	}

	// Calculate vertical movement
	if (currentY < box->y) {
		// Need to move down to enter walkbox
		step->distanceY = box->y - currentY;
		step->flags |= MOVE_DOWN;
	} else if (currentY > box->y + box->h) {
		// Need to move up to enter walkbox
		step->distanceY = currentY - (box->y + box->h);
		step->flags |= MOVE_UP;
	}
}

/**
 * Generate movement steps from walkbox path
 * Returns: number of movement steps generated
 */
uint16_t generateMovementSteps(Common::Array<WalkBox> &walkboxes,
							   uint8_t *pathBuffer,
							   uint16_t pathLength,
							   uint16_t startX, uint16_t startY,
							   uint16_t destX, uint16_t destY,
							   MovementStep *movementBuffer) {
	uint16_t currentX = startX;
	uint16_t currentY = startY;
	uint16_t movementIndex = 0;

	// Generate movements for each walkbox in path
	for (uint16_t i = 0; i < pathLength && pathBuffer[i] != PATH_END; i++) {
		uint8_t boxIndex = pathBuffer[i];
		WalkBox *box = &walkboxes[boxIndex];

		MovementStep step;
		calculateMovementToTarget(currentX, currentY, destX, destY, box, &step);

		if (step.distanceX > 0 || step.distanceY > 0) {
			movementBuffer[movementIndex++] = step;

			// Update current position
			if (step.flags & MOVE_RIGHT) {
				currentX = box->x;
			} else if (step.flags & MOVE_LEFT) {
				currentX = box->x + box->w;
			}

			if (step.flags & MOVE_DOWN) {
				currentY = box->y;
			} else if (step.flags & MOVE_UP) {
				currentY = box->y + box->h;
			}
		}
	}

	// Final movement to exact destination
	MovementStep final_step;
	final_step.flags = 0;

	if (currentX < destX) {
		final_step.distanceX = destX - currentX;
		final_step.flags |= MOVE_RIGHT;
	} else if (currentX > destX) {
		final_step.distanceX = currentX - destX;
		final_step.flags |= MOVE_LEFT;
	} else {
		final_step.distanceX = 0;
	}

	if (currentY < destY) {
		final_step.distanceY = destY - currentY;
		final_step.flags |= MOVE_DOWN;
	} else if (currentY > destY) {
		final_step.distanceY = currentY - destY;
		final_step.flags |= MOVE_UP;
	} else {
		final_step.distanceY = 0;
	}

	if (final_step.distanceX > 0 || final_step.distanceY > 0) {
		movementBuffer[movementIndex++] = final_step;
	}

	return movementIndex;
}

} // End of namespace Pelrock
