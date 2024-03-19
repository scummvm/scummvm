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
#include "common/random.h"

namespace Agi {

int AgiEngine::checkStep(int delta, int step) {
	return (-step >= delta) ? 0 : (step <= delta) ? 2 : 1;
}

bool AgiEngine::checkBlock(int16 x, int16 y) {
	if (x <= _game.block.x1 || x >= _game.block.x2)
		return false;

	if (y <= _game.block.y1 || y >= _game.block.y2)
		return false;

	return true;
}

void AgiEngine::changePos(ScreenObjEntry *screenObj) {
	const int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
	const int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };

	int16 x = screenObj->xPos;
	int16 y = screenObj->yPos;
	bool insideBlock = checkBlock(x, y);

	x += screenObj->stepSize * dx[screenObj->direction];
	y += screenObj->stepSize * dy[screenObj->direction];

	if (checkBlock(x, y) == insideBlock) {
		screenObj->flags &= ~fMotion;
	} else {
		screenObj->flags |= fMotion;
		screenObj->direction = 0;
		if (isEgoView(screenObj))
			setVar(VM_VAR_EGO_DIRECTION, 0);
	}
}

// WORKAROUND:
// Overwrite cycler state with motion data as original AGI did, almost.
//
// The original AGI interpreter stored motion and cycler data in the same four
// bytes of the screen object struct. If a script activated a motion while a
// cycler is active, or the opposite, then the previous action's state was
// overwritten. Some game scripts rely on this behavior, although probably
// unintentionally. We store motion and cycler data in separate fields, so
// in order to produce the original behavior that games depend on, we implement
// the overwrite behavior.
//
// However, we make an exception: when cycler data is overwritten, the original
// would set an unintended game flag when it completed. It doesn't seem like
// anything good can come from setting an unintended flag, so we do not set any
// flag when an overwritten cycler completes.
//
// This affects at least:
// - KQ1: when the eagle grabs ego (room 22), Bug #7046
// - BC:  when the witches disappear at the end of the game (room 12, screen object 13)
// - DDP: introduction when the ducks jump, Bug #14170
// - KQ2: happened somewhere in the game, LordHoto couldn't remember exactly where
void AgiEngine::motionActivated(ScreenObjEntry *screenObj) {
	if (screenObj->flags & fCycling) { // Is a cycler active?
		switch (screenObj->cycle) {
		case kCycleEndOfLoop: // "end.of.loop"
		case kCycleRevLoop:   // "reverse.loop"
			// This would have overwritten the cycler's flag with wander_count
			// or follow_stepSize or move_x, and that would cause the cycler
			// to set an unintended flag if it completes.
			// We skip setting the unintended flag with ignoreLoopFlag.
			// Jumping at the eagle in KQ1 room 22 depends on the overwritten
			// flag not being set. Bug #7046
			screenObj->ignoreLoopFlag = true;
			warning("Motion activated for screen object %d while cycler is active", screenObj->objectNr);
			warning("Original AGI would overwrite flag %d, we skip setting it", screenObj->loop_flag);
			break;
		default:
			break;
		}
	}
}

// WORKAROUND:
// Overwrite motion state with cycler data as original AGI did.
// This is necessary because we use a different data structure than the
// original, but games relied on undefined behavior when activating a
// cycler while a motion was in progress.
// See comment for motionActivated()
void AgiEngine::cyclerActivated(ScreenObjEntry *screenObj) {
	const char *fieldName;
	uint8 previousValue;
	switch (screenObj->motionType) {
	case kMotionWander:
		// Overwrite wander count with the cycler's flag.
		fieldName = "wander_count";
		previousValue = screenObj->wander_count;
		screenObj->wander_count = screenObj->loop_flag;
		break;
	case kMotionFollowEgo:
		// Overwrite follow step size with the cycler's flag.
		fieldName = "follow_stepSize";
		previousValue = screenObj->follow_stepSize;
		screenObj->follow_stepSize = screenObj->loop_flag;
		break;
	case kMotionMoveObj:
		// Overwrite move_x with the cycler's flag.
		// Required for witches to disappear at the end of Black Cauldron, room 12.
		fieldName = "move_x";
		previousValue = screenObj->move_x;
		screenObj->move_x = screenObj->loop_flag;
		break;
	default:
		return;
	}
	warning("Cycler activated for screen object %d while motion is active", screenObj->objectNr);
	warning("Overwriting %s: %d with flag number %d, as original AGI did", fieldName, previousValue, screenObj->loop_flag);
}

void AgiEngine::motionWander(ScreenObjEntry *screenObj) {
	uint8 originalWanderCount = screenObj->wander_count;

	screenObj->wander_count--;
	if ((originalWanderCount == 0) || (screenObj->flags & fDidntMove)) {
		screenObj->direction = _rnd->getRandomNumber(8);

		if (isEgoView(screenObj)) {
			setVar(VM_VAR_EGO_DIRECTION, screenObj->direction);
		}

		while (screenObj->wander_count < 6) {
			screenObj->wander_count = _rnd->getRandomNumber(50);    // huh?
		}
	}
}

void AgiEngine::motionFollowEgo(ScreenObjEntry *screenObj) {
	ScreenObjEntry *screenObjEgo = &_game.screenObjTable[SCREENOBJECTS_EGO_ENTRY];

	int egoX = screenObjEgo->xPos + screenObjEgo->xSize / 2;
	int egoY = screenObjEgo->yPos;

	int objX = screenObj->xPos + screenObj->xSize / 2;
	int objY = screenObj->yPos;

	// Get direction to reach ego
	int dir = getDirection(objX, objY, egoX, egoY, screenObj->follow_stepSize);

	// Already at ego coordinates
	if (dir == 0) {
		screenObj->direction = 0;
		screenObj->motionType = kMotionNormal;
		setFlag(screenObj->follow_flag, true);
		return;
	}

	if (screenObj->follow_count == 0xff) {
		screenObj->follow_count = 0;
	} else if (screenObj->flags & fDidntMove) {
		int d;

		while ((screenObj->direction = _rnd->getRandomNumber(8)) == 0) {
		}

		d = (ABS(egoY - objY) + ABS(egoX - objX)) / 2;

		if (d < screenObj->stepSize) {
			screenObj->follow_count = screenObj->stepSize;
			return;
		}

		while ((screenObj->follow_count = _rnd->getRandomNumber(d)) < screenObj->stepSize) {
		}
		return;
	}

	if (screenObj->follow_count != 0) {
		// DF: this is ugly and I dont know why this works, but
		// other line does not! (watcom complained about lvalue)
		//
		// if (((int8)v->parm3 -= v->step_size) < 0)
		//      v->parm3 = 0;

		int k = screenObj->follow_count;
		k -= screenObj->stepSize;
		screenObj->follow_count = k;

		if ((int8) screenObj->follow_count < 0)
			screenObj->follow_count = 0;
	} else {
		screenObj->direction = dir;
	}
}

void AgiEngine::motionMoveObj(ScreenObjEntry *screenObj) {
	screenObj->direction = getDirection(screenObj->xPos, screenObj->yPos, screenObj->move_x, screenObj->move_y, screenObj->stepSize);

	// Update V6 if ego
	if (isEgoView(screenObj))
		setVar(VM_VAR_EGO_DIRECTION, screenObj->direction);

	if (screenObj->direction == 0)
		motionMoveObjStop(screenObj);
}

void AgiEngine::checkMotion(ScreenObjEntry *screenObj) {
	switch (screenObj->motionType) {
	case kMotionNormal:
		break;
	case kMotionWander:
		motionWander(screenObj);
		break;
	case kMotionFollowEgo:
		motionFollowEgo(screenObj);
		break;
	case kMotionEgo:
	case kMotionMoveObj:
		motionMoveObj(screenObj);
		break;
	default:
		break;
	}

	if ((_game.block.active && (~screenObj->flags & fIgnoreBlocks)) && screenObj->direction)
		changePos(screenObj);
}

/*
 * Public functions
 */

void AgiEngine::checkAllMotions() {
	ScreenObjEntry *screenObj;

	for (screenObj = _game.screenObjTable; screenObj < &_game.screenObjTable[SCREENOBJECTS_MAX]; screenObj++) {
		if ((screenObj->flags & (fAnimated | fUpdate | fDrawn)) == (fAnimated | fUpdate | fDrawn)
		        && screenObj->stepTimeCount == 1) {
			checkMotion(screenObj);
		}
	}
}

/**
 * Check if given entry is at destination point.
 * This function is used to updated the flags of an object with move.obj
 * type motion that * has reached its final destination coordinates.
 * @param  v  Pointer to view table entry
 */
void AgiEngine::inDestination(ScreenObjEntry *screenObj) {
	if (screenObj->motionType == kMotionMoveObj) {
		screenObj->stepSize = screenObj->move_stepSize;
		setFlag(screenObj->move_flag, true);
	}
	screenObj->motionType = kMotionNormal;
	if (isEgoView(screenObj))
		_game.playerControl = true;
}

void AgiEngine::motionMoveObjStop(ScreenObjEntry *screenObj) {
	screenObj->stepSize = screenObj->move_stepSize;

	// This check for motionType was only done in AGI3.
	// But we use this motion type for mouse movement, so we need to check in any case, otherwise it will cause glitches.
	if (screenObj->motionType != kMotionEgo) {
		setFlag(screenObj->move_flag, true);
	}

	screenObj->motionType = kMotionNormal;
	if (isEgoView(screenObj)) {
		_game.playerControl = true;
		setVar(VM_VAR_EGO_DIRECTION, 0);
	}
}

/**
 * Wrapper for static function motion_moveobj().
 * This function is used by cmd_move_object() in the first motion cycle
 * after setting the motion mode to kMotionMoveObj.
 * @param  v  Pointer to view table entry
 */
void AgiEngine::moveObj(ScreenObjEntry *screenObj) {
	motionMoveObj(screenObj);
}

/**
 * Get direction from motion coordinates
 * This function gets the motion direction from the current and previous
 * object coordinates and the step size.
 * @param  x0  Original x coordinate of the object
 * @param  y0  Original y coordinate of the object
 * @param  x   x coordinate of the object
 * @param  y   y coordinate of the object
 * @param  s   step size
 */
int AgiEngine::getDirection(int16 objX, int16 objY, int16 destX, int16 destY, int16 stepSize) {
	const int dirTable[9] = { 8, 1, 2, 7, 0, 3, 6, 5, 4 };
	return dirTable[checkStep(destX - objX, stepSize) + 3 * checkStep(destY - objY, stepSize)];
}

} // End of namespace Agi
