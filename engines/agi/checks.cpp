/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	int16 curX;
	int16 curY;
	int16 celX;
	byte screenPriority = 0;

	if (!(screenObj->flags & fFixedPriority)) {
		// Priority bands
		screenObj->priority = _gfx->priorityFromY(screenObj->yPos);
	}

	if (screenObj->priority != 0x0f) {

		touchedWater = true;

		curX = screenObj->xPos;
		curY = screenObj->yPos;

		for (celX = 0; celX < screenObj->xSize; celX++, curX++) {
			screenPriority = _gfx->getPriority(curX, curY);

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
		setFlag(VM_FLAG_EGO_TOUCHED_P2, touchedTrigger ? true : false);
		setFlag(VM_FLAG_EGO_WATER, touchedWater ? true : false);
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
	ScreenObjEntry *screenObj;
	int x, y, oldX, oldY, border;

	setVar(VM_VAR_BORDER_CODE, 0);
	setVar(VM_VAR_BORDER_TOUCH_EGO, 0);
	setVar(VM_VAR_BORDER_TOUCH_OBJECT, 0);

	for (screenObj = _game.screenObjTable; screenObj < &_game.screenObjTable[SCREENOBJECTS_MAX]; screenObj++) {
		if ((screenObj->flags & (fAnimated | fUpdate | fDrawn)) != (fAnimated | fUpdate | fDrawn)) {
			continue;
		}

		if (screenObj->stepTimeCount > 1) {
			screenObj->stepTimeCount--;
			continue;
		}

		screenObj->stepTimeCount = screenObj->stepTime;

		x = oldX = screenObj->xPos;
		y = oldY = screenObj->yPos;

		// If object has moved, update its position
		if (!(screenObj->flags & fUpdatePos)) {
			int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
			int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };
			x += screenObj->stepSize * dx[screenObj->direction];
			y += screenObj->stepSize * dy[screenObj->direction];
		}

		// Now check if it touched the borders
		border = 0;

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
	int count, dir, size;

	debugC(4, kDebugLevelSprites, "adjusting view table entry #%d (%d,%d)", screenObj->objectNr, screenObj->xPos, screenObj->yPos);

	// test horizon
	if ((!(screenObj->flags & fIgnoreHorizon)) && screenObj->yPos <= _game.horizon)
		screenObj->yPos = _game.horizon + 1;

	dir = 0;
	count = size = 1;

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

} // End of namespace Agi
