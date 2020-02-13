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
	bool insideBlock;
	int16 x, y;
	int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
	int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };

	x = screenObj->xPos;
	y = screenObj->yPos;
	insideBlock = checkBlock(x, y);

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
// A motion was just activated, check if "end.of.loop"/"reverse.loop" is currently active for the same screen object
// If this is the case, it would result in some random flag getting overwritten in original AGI after the loop was
// completed, because in original AGI loop_flag + wander_count/follow_stepSize/move_X shared the same memory location.
// This is basically an implementation error in the original interpreter.
// Happens in at least:
// - BC: right at the end when the witches disappear at least on Apple IIgs (room 12, screen object 13, view 84)
// - KQ1: when grabbing the eagle (room 22).
// - KQ2: happened somewhere in the game, LordHoto couldn't remember exactly where
void AgiEngine::motionActivated(ScreenObjEntry *screenObj) {
	if (screenObj->flags & fCycling) {
		// Cycling active too
		switch (screenObj->cycle) {
		case kCycleEndOfLoop: // "end.of.loop"
		case kCycleRevLoop: // "reverse.loop"
			// Disable it
			screenObj->flags &= ~fCycling;
			screenObj->cycle = kCycleNormal;

			warning("Motion activated for screen object %d, but cycler also active", screenObj->objectNr);
			warning("This would have resulted in flag corruption in original AGI. Cycler disabled.");
			break;
		default:
			break;
		}
	}
}

// WORKAROUND:
// See comment for motionActivated()
// This way no flag would have been overwritten, but certain other variables of the motions.
void AgiEngine::cyclerActivated(ScreenObjEntry *screenObj) {
	switch (screenObj->motionType) {
	case kMotionWander:
		// this would have resulted in wander_count to get corrupted
		// We don't stop it.
		break;
	case kMotionFollowEgo:
		// this would have resulted in follow_stepSize to get corrupted
		// do not stop motion atm - screenObj->direction = 0;
		// do not stop motion atm - screenObj->motionType = kMotionNormal;
		break;
	case kMotionMoveObj:
		// this would have resulted in move_x to get corrupted
		// do not stop motion atm - motionMoveObjStop(screenObj);
		break;
	default:
		return;
		break;
	}
	warning("Cycler activated for screen object %d, but motion also active", screenObj->objectNr);
	warning("This would have resulted in corruption in original AGI. Motion disabled.");
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
	int egoX, egoY;
	int objX, objY;
	int dir;

	egoX = screenObjEgo->xPos + screenObjEgo->xSize / 2;
	egoY = screenObjEgo->yPos;

	objX = screenObj->xPos + screenObj->xSize / 2;
	objY = screenObj->yPos;

	// Get direction to reach ego
	dir = getDirection(objX, objY, egoX, egoY, screenObj->follow_stepSize);

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
		int k;

		// DF: this is ugly and I dont know why this works, but
		// other line does not! (watcom complained about lvalue)
		//
		// if (((int8)v->parm3 -= v->step_size) < 0)
		//      v->parm3 = 0;

		k = screenObj->follow_count;
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

/**
 *
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
	int dirTable[9] = { 8, 1, 2, 7, 0, 3, 6, 5, 4 };
	return dirTable[checkStep(destX - objX, stepSize) + 3 * checkStep(destY - objY, stepSize)];
}

} // End of namespace Agi
