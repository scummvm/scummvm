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

#include "agi/agi.h"
#include "agi/graphics.h"

namespace Agi {

bool AgiEngine::checkPosition(ScreenObjEntry *screenObj) {
	bool result = true; // position is fine
	debugC(4, kDebugLevelSprites, "check position @ %d, %d", screenObj->xPos, screenObj->yPos);

	if (screenObj->xPos < 0) {
		result = false;
	} else {
		if (screenObj->xPos + screenObj->xSize > SCRIPT_WIDTH) {
			result = false;
		} else {
			if (screenObj->yPos - screenObj->ySize < -1) {
				result = false;
			} else {
				if (screenObj->yPos >= SCRIPT_HEIGHT) {
					result = false;
				} else {
					if (((!(screenObj->flags & fIgnoreHorizon)) && screenObj->yPos <= _game.horizon)) {
						result = false;
					}
				}
			}
		}
	}

	// MH1 needs this, but it breaks LSL1
// TODO: *NOT* in disassembly of AGI3 .149, why was this needed?
//	if (getVersion() >= 0x3000) {
//		if (screenObj->yPos < screenObj->ySize)
//			result = false;
//	}

	if (!result) {
		debugC(4, kDebugLevelSprites, "check position failed: x=%d, y=%d, h=%d, w=%d",
		       screenObj->xPos, screenObj->yPos, screenObj->xSize, screenObj->ySize);
	}
	return result;
}

/**
 * Check if there's another object on the way
 */
bool AgiEngine::checkCollision(ScreenObjEntry *screenObj) {
	ScreenObjEntry *checkObj;

	if (screenObj->flags & fIgnoreObjects)
		return false;

	for (checkObj = _game.screenObjTable; checkObj < &_game.screenObjTable[SCREENOBJECTS_MAX]; checkObj++) {
		if ((checkObj->flags & (fAnimated | fDrawn)) != (fAnimated | fDrawn))
			continue;

		if (checkObj->flags & fIgnoreObjects)
			continue;

		// Same object, check next
		if (screenObj->objectNr == checkObj->objectNr)
			continue;

		// No horizontal overlap, check next
		if (screenObj->xPos + screenObj->xSize < checkObj->xPos || screenObj->xPos > checkObj->xPos + checkObj->xSize)
			continue;

		// Same y, return error!
		if (screenObj->yPos == checkObj->yPos) {
			debugC(4, kDebugLevelSprites, "check returns 1 (object %d)", screenObj->objectNr);
			return true;
		}

		// Crossed the baseline, return error!
		if ((screenObj->yPos > checkObj->yPos && screenObj->yPos_prev < checkObj->yPos_prev) ||
		        (screenObj->yPos < checkObj->yPos && screenObj->yPos_prev > checkObj->yPos_prev)) {
			debugC(4, kDebugLevelSprites, "check returns 1 (object %d)", screenObj->objectNr);
			return true;
		}
	}

	return false;
}

bool AgiEngine::checkPriority(ScreenObjEntry *screenObj) {
	bool touchedWater = false;
	bool touchedTrigger = false;
	bool touchedControl = true;

	if (!(screenObj->flags & fFixedPriority)) {
		// Priority bands
		screenObj->priority = _gfx->priorityFromY(screenObj->yPos);
	}

	if (screenObj->priority != 0x0f) {
		touchedWater = true;

		int16 curX = screenObj->xPos;
		int16 curY = screenObj->yPos;

		for (int16 celX = 0; celX < screenObj->xSize; celX++, curX++) {
			byte screenPriority = _gfx->getPriority(curX, curY);

			if (screenPriority == 0) {  // unconditional black. no go at all!
				touchedControl = false;
				break;
			}

			if (screenPriority != 3) {  // not water surface
				touchedWater = false;

				if (screenPriority == 1) {  // conditional blue
					if (!(screenObj->flags & fIgnoreBlocks)) {
						debugC(4, kDebugLevelSprites, "Blocks observed!");
						touchedControl = false;
						break;
					}
				} else if (screenPriority == 2) {
					debugC(4, kDebugLevelSprites, "stepped on trigger");
					if (!_debug.ignoretriggers)
						touchedTrigger = true;
				}
			}
		}

		if (touchedControl) {
			if (!touchedWater) {
				if (screenObj->flags & fOnWater)
					touchedControl = false;
			} else {
				if (screenObj->flags & fOnLand)
					touchedControl = false;
			}
		}
	}

	// Check ego
	if (screenObj->objectNr == 0) {
		setFlag(VM_FLAG_EGO_TOUCHED_P2, touchedTrigger);
		setFlag(VM_FLAG_EGO_WATER, touchedWater);

		// WORKAROUND: KQ3 infinite falling, bug #13379
		// Falling off of the ladder in room 22 or the stairs in room 64 can
		// cause ego to fall forever in place. In both rooms, and possibly
		// others, an unrelated black priority line overlaps with fall paths.
		// This also occurs in the original. Ignore these lines when falling.
		if (!touchedControl && getGameID() == GID_KQ3 && screenObj->currentViewNr == 11) {
			touchedControl = true;
		}
	}

	return touchedControl;
}

/*
 * Public functions
 */

/**
 * Update position of objects
 * This function updates the position of all animated, updating view
 * table entries according to its motion type, step size, etc. The
 * new position must be valid according to the sprite positioning
 * rules, otherwise the previous position will be kept.
 */
void AgiEngine::updatePosition() {
	setVar(VM_VAR_BORDER_CODE, 0);
	setVar(VM_VAR_BORDER_TOUCH_EGO, 0);
	setVar(VM_VAR_BORDER_TOUCH_OBJECT, 0);

	ScreenObjEntry *screenObj;
	for (screenObj = _game.screenObjTable; screenObj < &_game.screenObjTable[SCREENOBJECTS_MAX]; screenObj++) {
		if ((screenObj->flags & (fAnimated | fUpdate | fDrawn)) != (fAnimated | fUpdate | fDrawn)) {
			continue;
		}

		if (screenObj->stepTimeCount > 1) {
			screenObj->stepTimeCount--;
			continue;
		}

		screenObj->stepTimeCount = screenObj->stepTime;

		int x = screenObj->xPos;
		int oldX = x;
		int y = screenObj->yPos;
		int oldY = y;

		// If object has moved, update its position
		if (!(screenObj->flags & fUpdatePos)) {
			const int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
			const int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };
			x += screenObj->stepSize * dx[screenObj->direction];
			y += screenObj->stepSize * dy[screenObj->direction];
		}

		// Now check if it touched the borders
		int border = 0;

		// Check left/right borders
		if (getVersion() == 0x3086) {
			// KQ4 interpreter does a different comparison on x
			// The interpreter before (2.917) and after that (3.098) don't do them that way
			// This difference is required for at least Sarien bug #192
			// KQ4: room 135, hen moves from the center of the screen to the left border,
			// but it doesn't disappear.
			if (x <= 0) {
				x = 0;
				border = 4;
			}
		} else {
			// regular comparison
			if (x < 0) {
				x = 0;
				border = 4;
			}
		}

//		} else if (v->entry == 0 && x == 0 && v->flags & fAdjEgoXY) { // should not be required
//			// Extra test to walk west clicking the mouse
//			x = 0;
//			border = 4;

		if (!border) {
			if (x + screenObj->xSize > SCRIPT_WIDTH) {
				x = SCRIPT_WIDTH - screenObj->xSize;
				border = 2;
			}
		}

		// Check top/bottom borders.
		if (y - screenObj->ySize < -1) {
			y = screenObj->ySize - 1;
			border = 1;
		} else if (y > SCRIPT_HEIGHT - 1) {
			y = SCRIPT_HEIGHT - 1;
			border = 3;
		} else if ((!(screenObj->flags & fIgnoreHorizon)) && y <= _game.horizon) {
			debugC(4, kDebugLevelSprites, "y = %d, horizon = %d", y, _game.horizon);
			y = _game.horizon + 1;
			border = 1;
		}

		// Test new position. rollback if test fails
		screenObj->xPos = x;
		screenObj->yPos = y;
		if (checkCollision(screenObj) || !checkPriority(screenObj)) {
			screenObj->xPos = oldX;
			screenObj->yPos = oldY;
			border = 0;
			fixPosition(screenObj->objectNr);
		}

		if (border) {
			if (isEgoView(screenObj)) {
				setVar(VM_VAR_BORDER_TOUCH_EGO, border);
			} else {
				setVar(VM_VAR_BORDER_CODE, screenObj->objectNr);
				setVar(VM_VAR_BORDER_TOUCH_OBJECT, border);
			}
			if (screenObj->motionType == kMotionMoveObj) {
				motionMoveObjStop(screenObj);
			}
		}

		screenObj->flags &= ~fUpdatePos;
	}
}

/**
 * Adjust position of a sprite
 * This function adjusts the position of a sprite moving it until
 * certain criteria is matched. According to priority and control line
 * data, a sprite may not always appear at the location we specified.
 * This behavior is also known as the "Budin-Sonneveld effect".
 *
 * @param n view table entry number
 */
void AgiEngine::fixPosition(int16 screenObjNr) {
	ScreenObjEntry *screenObj = &_game.screenObjTable[screenObjNr];
	fixPosition(screenObj);
}

void AgiEngine::fixPosition(ScreenObjEntry *screenObj) {
	debugC(4, kDebugLevelSprites, "adjusting view table entry #%d (%d,%d)", screenObj->objectNr, screenObj->xPos, screenObj->yPos);

	// test horizon
	if ((!(screenObj->flags & fIgnoreHorizon)) && screenObj->yPos <= _game.horizon)
		screenObj->yPos = _game.horizon + 1;

	int dir = 0;
	int count = 1;
	int size = 1;

	while (!checkPosition(screenObj) || checkCollision(screenObj) || !checkPriority(screenObj)) {
		switch (dir) {
		case 0: // west
			screenObj->xPos--;
			if (--count)
				continue;
			dir = 1;
			break;
		case 1: // south
			screenObj->yPos++;
			if (--count)
				continue;
			dir = 2;
			size++;
			break;
		case 2: // east
			screenObj->xPos++;
			if (--count)
				continue;
			dir = 3;
			break;
		case 3: // north
			screenObj->yPos--;
			if (--count)
				continue;
			dir = 0;
			size++;
			break;
		default:
			break;
		}

		count = size;
	}

	debugC(4, kDebugLevelSprites, "view table entry #%d position adjusted to (%d,%d)", screenObj->objectNr, screenObj->xPos, screenObj->yPos);
}

/**
 * Tests if ego is facing nearby water without obstacles. Used by opcode 5F in
 * Black Cauldron AGIv1 to test if the water flask can be filled. Removed from
 * the interpreter in AGIv2 and replaced with position tests in game scripts.
 *
 * Returns distance to water or 250 if water is not found or is blocked.
 */
byte AgiEngine::egoNearWater(byte limit) {
	ScreenObjEntry &ego = _game.screenObjTable[SCREENOBJECTS_EGO_ENTRY];
	int16 x1 = ego.xPos;
	int16 x2 = 0;
	byte direction;

	switch (ego.currentLoopNr) {
	case 0: // right
		direction = 3;
		x1 += ego.xSize;
		break;
	case 1: // left
		direction = 7;
		break;
	case 2: // down
		direction = 5;
		x1 += (ego.xSize / 2);
		break;
	case 3: // up
		direction = 1;
		x2 = x1 + ego.xSize;
		x1--;
		break;
	default: // unhandled in original
		return 250; // no water
	}

	int16 distance = -1; // uninitialized in original
	while (x1 != 0) {
		distance = nearWater(ego, direction, x1, ego.yPos, limit);
		if (distance != -1) {
			break;
		}
		x1 = x2;
		x2 = 0;
	}

	if (distance == -1) {
		return 250; // no water
	}

	// adjust ego positions for collision check
	int16 prevPrevX = ego.xPos_prev;
	int16 prevPrevY = ego.yPos_prev;
	ego.xPos_prev = ego.xPos;
	ego.yPos_prev = ego.yPos;
	switch (direction) {
	case 1: // up
		ego.yPos -= distance;
		break;
	case 3: // right
		ego.xPos += (x1 - ego.xSize);
		break;
	case 5: // down
		ego.yPos += distance;
		break;
	case 7: // left
		ego.xPos -= distance;
		break;
	default:
		break;
	}

	if (!checkCollision(&ego)) {
		if (_game.block.active) {
			if (!(ego.flags & fIgnoreBlocks)) {
				if (checkBlock(ego.xPos, ego.yPos)) {
					distance = 250; // no water
				}
			}
		}
	} else {
		distance = 250; // no water
	}

	// restore ego positions
	ego.xPos = ego.xPos_prev;
	ego.yPos = ego.yPos_prev;
	ego.xPos_prev = prevPrevX;
	ego.yPos_prev = prevPrevY;

	return distance;
}

/**
 * Tests if a screen object is near water in a given direction.
 *
 * Returns the distance to water or -1 if water is not found or is blocked.
 *
 * Note that the original contains a bug that scans left when facing right.
 * We do not implement this bug. In Black Cauldron AGIv1, it prevents filling
 * the flask when facing right unless ego is on or facing away from water.
 */
int16 AgiEngine::nearWater(ScreenObjEntry &screenObj, byte direction, int16 x, int16 y, byte limit) {
	int16 dx = 0;
	int16 dy = 0;
	switch (direction) {
	case 1: dy = -1; break;
	case 3: dx =  1; break;
	case 5: dy =  1; break;
	case 7: dx = -1; break;
	default: break;
	}

	for (int16 i = 0; i <= limit; i++) {
		if (!(0 <= x && x < SCRIPT_WIDTH && 0 <= y && y < SCRIPT_HEIGHT)) {
			break;
		}

		byte priority = _gfx->getPriority(x, y);
		x += dx;
		y += dy;

		// water found?
		if (priority == 3) {
			return i;
		}

		if (screenObj.priority != 15) {
			// is blocked by priority 0?
			if (priority == 0) {
				break;
			}

			// is blocked by priority 1?
			if (priority == 1 && !(screenObj.flags & fIgnoreBlocks)) {
				break;
			}
		}
	}

	return -1; // water not found
}

} // End of namespace Agi
