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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#include "gob/global.h"
#include "gob/gob.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/scenery.h"
#include "gob/videoplayer.h"

namespace Gob {

Goblin_v7::Goblin_v7(GobEngine *vm) : Goblin_v4(vm) {
}

void Goblin_v7::setGoblinState(Mult::Mult_Object *obj, int16 animState) {
	char str[128];
	obj->pAnimData->layer &= 3;
	int32 var_4 = 0;
	Common::strlcpy(str, obj->animName, 128);
	if (obj->pAnimData->curLookDir >= 10) {
		if (obj->pAnimData->curLookDir == 26 ||
			obj->pAnimData->curLookDir == 36) {
			if (str[strlen(str) - 1] == 'U') {
				str[strlen(str) - 3] = '\0';
			} else
				str[strlen(str) - 2] = '\0';
		} else {
			str[strlen(str) - 3] = '\0';
		}
	} else {
		str[strlen(str) - 2] = '\0';
	}

	if (animState < 100)
		var_4 = 1;
	else
		var_4 = 0;

	animState %= 100;
	obj->pAnimData->curLookDir = animState;

	int16 newXCorrection = 0;
	int16 newYCorrection = 0;
	while (true) {
		// obj->animVariables[1]: number of fields per state
		// obj->animVariables[2]: max number of states
		if (animState <= 0 || animState > (int16) obj->animVariables->at(2)) {
			obj->pAnimData->animType = 11;
			return;
		}

		int32 newOffset = animState * (int16) obj->animVariables->at(1);
		VariableReferenceArray animVariablesForState = obj->animVariables->arrayAt(newOffset);
		if (animVariablesForState.at(0) == 0) {
			newXCorrection = (int16) animVariablesForState.at(1);
			newYCorrection = (int16) animVariablesForState.at(2);
			break;
		}

		if ((int16) animVariablesForState.at(0) == -2) {
			// Reflexion relative to Y axis:
			// Some videos exist only for "west" directions (W, NW, SW, N S),
			// "east" directions (E, NE, SE) are then obtained by symmetry
			switch (animState) {
			case 1:
				animState = 5;
				break;
			case 2:
				animState = 4;
				break;
			case 4:
				animState = 2;
				break;
			case 5:
				animState = 1;
				break;
			case 6:
				animState = 8;
				break;
			case 8:
				animState = 6;
				break;
			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
				animState -= 10;
				break;
			default: // 3, 7, 9-30, > 36
				obj->pAnimData->animType = 11;
				return;
			}

			obj->pAnimData->layer |= 0x80;
			newXCorrection = (int16) animVariablesForState.at(1);
			newYCorrection = (int16) animVariablesForState.at(2);
			break;
		}

		if ((int16) animVariablesForState.at(0) == -1) {
			obj->pAnimData->animType = 11;
			return;
		}

		animState = (int16) animVariablesForState.at(0);
	}

	if (obj->pAnimData->stateType == 1) {
		if (animState == 22)
			animState = 27;
		else if (animState == 25)
			animState = 26;
	}

	switch (animState) {
	case 1:
		Common::strlcat(str, "GG", 128);
		break;
	case 2:
		Common::strlcat(str, "GH", 128);
		break;
	case 3:
		Common::strlcat(str, "HH", 128);
		break;
	case 4:
		Common::strlcat(str, "DH", 128);
		break;
	case 5:
		Common::strlcat(str, "DD", 128);
		break;
	case 6:
		Common::strlcat(str, "DB", 128);
		break;
	case 7:
		Common::strlcat(str, "BB", 128);
		break;
	case 8:
		Common::strlcat(str, "GB", 128);
		break;
	case 21:
		Common::strlcat(str, "COG", 128);
		break;
	case 22: {
		uint animIndex = _vm->_rnd.getRandomNumber(1);
		switch (animIndex) {
		case 0:
			Common::strlcat(str, "EFR", 128); // scared (EFFRAYE)
			break;
		case 1:
			Common::strlcat(str, "EF1", 128);
			break;
		default:
			break;
		}

		break;
	}
	case 23:
		Common::strlcat(str, "EXP", 128);
		break;
	case 24:
		Common::strlcat(str, "FRA", 128);
		break;
	case 25: {
		uint animIndex = _vm->_rnd.getRandomNumber(3);
		switch (animIndex) {
		case 0:
			Common::strlcat(str, "PAR", 128); // talking (PARLE)
			break;
		case 1:
			Common::strlcat(str, "PA1", 128);
			break;
		case 2:
			Common::strlcat(str, "PA2", 128);
			break;
		case 3:
			Common::strlcat(str, "PA3", 128);
			break;
		default:
			break;
		}

		break;
	}


	case 26: {
		uint animIndex = _vm->_rnd.getRandomNumber(3);
		switch (animIndex) {
		case 0:
			Common::strlcat(str, "PAU", 128); // taking a break (PAUSE)
			break;
		case 1:
			Common::strlcat(str, "P1", 128);
			break;
		case 2:
			Common::strlcat(str, "P2", 128);
			break;
		case 3:
			Common::strlcat(str, "P3", 128);
			break;
		default:
			break;
		}

		break;
	}
	case 27: {
		uint animIndex = _vm->_rnd.getRandomNumber(1);
		switch (animIndex) {
		case 0:
			Common::strlcat(str, "RIR", 128); // laughing (RIRE)
			break;
		case 1:
			Common::strlcat(str, "RI1", 128);
			break;
		default:
			break;
		}

		break;
	}

	default:
		Common::strlcat(str, Common::String::format("%02d", animState).c_str(), 128);
	}

	if (strcmp(str, obj->animName) != 0) {
		_vm->_mult->closeObjVideo(*obj);
		Common::strlcpy(obj->animName, str, 16);
	}

	int32 newX = 0;
	int32 newY = 0;
	if (_vm->_map->_usesObliqueCoordinates) {
		// Oblique coordinates to screen coordinates mapping
		newX = (_vm->_map->getTilesWidth() / 2) * obj->pAnimData->destX +
				 (_vm->_map->getTilesWidth() / 2) * obj->pAnimData->destY -
				 (_vm->_map->getTilesWidth() * 39) / 2;

		newY = (_vm->_map->getTilesHeight() / 2) * obj->pAnimData->destY -
				 (_vm->_map->getTilesHeight() / 2) * obj->pAnimData->destX +
				 (_vm->_map->getTilesHeight() * 20);

		if (animState > 10) {
			obj->destX = obj->pAnimData->destX;
			obj->goblinX = obj->destX;
			obj->destY = obj->pAnimData->destY;
			obj->goblinY = obj->destY;
		}
	} else {
		newY = obj->pAnimData->destY * _vm->_map->getTilesHeight();
		newX = obj->pAnimData->destX * _vm->_map->getTilesWidth();
	}

	*obj->pPosX = newX + newXCorrection;
	*obj->pPosY = newY + newYCorrection;
	obj->pAnimData->frame = 0;
	if (var_4 == 0) {
		_vm->_mult->closeObjVideo(*obj);

		VideoPlayer::Properties props;
		props.x          = -1;
		props.y          = -1;
		props.startFrame = 0;
		props.lastFrame  = 0;
		props.breakKey   = 0;
		props.flags      = 0x1601;
		props.palStart   = 0;
		props.palEnd     = 0;
		props.sprite     = 50 - obj->pAnimData->animation - 1;

		_vm->_mult->openObjVideo(str, props, obj->pAnimData->animation);
	} else {
		if (obj->videoSlot == 0 ||
			strcmp(obj->animName, str) != 0 ||
			(_vm->_vidPlayer->getFlags(obj->videoSlot - 1) & 0x800)) {

			VideoPlayer::Properties props;
			props.x          = -1;
			props.y          = -1;
			props.startFrame = 0;
			props.lastFrame  = 0;
			props.breakKey   = 0;
			props.flags      = 0x1601;
			props.palStart   = 0;
			props.palEnd     = 0;
			props.sprite     = 50 - obj->pAnimData->animation - 1;

			_vm->_mult->openObjVideo(str, props, obj->pAnimData->animation);
		}
	}
}

/*
 * NOTE: conversion between direction index, coordinates in Map with _map->_usesObliqueCoordinates
 * and screen coordinates
 * -----------------------------------------------------
 * | dir. index | Map obl.   | screen                  |
 * | 0          | ( 0,  0)   | ( 0,  0)                |
 * | 1          | (-1, -1)   | (-1,  0) -> left        |
 * | 2          | ( 0, -1)   | (-1, -1) -> up-left     |
 * | 3          | ( 1, -1)   | ( 0, -1) -> up          |
 * | 4          | ( 1,  0)   | ( 1, -1) -> up-right    |
 * | 5          | ( 1,  1)   | ( 1,  0) -> right       |
 * | 6          | ( 0,  1)   | ( 1,  1) -> down-right  |
 * | 7          | (-1,  1)   | ( 0,  1) -> down        |
 * | 8          | (-1,  0)   | (-1,  1) -> down-left   |
 * | 9          | ( 0,  0)   | ( 0,  0)                |
 * -----------------------------------------------------
 */

static int8 deltaXFromDirection[10] = {0, -1, 0, 1, 1, 1, 0, -1, -1, 0};
static int8 deltaYFromDirection[10] = {0, -1, -1, -1, 0, 1, 1, 1, 0, 0};

static bool positionWalkable(Map *map, int8 x, int8 y) {
	return !map->getPass(x, y, map->getMapWidth());
}

static void updateGobDest(Map *map, Mult::Mult_Object &obj) {
	if (!positionWalkable(map, obj.gobDestX, obj.gobDestY)) {
		int8 newGobDestX = 0;
		int32 var_8 = 1000;
		int8 newGobDestY = 0;
		for (int32 direction = 2; direction <= 8; direction += 2) {
			int32 nbrOfStepsDir = 0;
			int8 tempGobDestX = obj.gobDestX;
			int8 tempGobDestY = obj.gobDestY;

			while (true) {
				tempGobDestX += deltaXFromDirection[direction];
				tempGobDestY += deltaYFromDirection[direction];
				nbrOfStepsDir++;
				if (tempGobDestX < 0
					|| tempGobDestX >= map->getMapWidth()
					|| tempGobDestY < 0
					|| tempGobDestY >= map->getMapHeight()) {
					break;
				}

				if (positionWalkable(map, tempGobDestX, tempGobDestY)) {
					if (nbrOfStepsDir < var_8) {
						newGobDestX = tempGobDestX;
						newGobDestY = tempGobDestY;
						var_8 = nbrOfStepsDir;
						break;
					}
				}
			}
		}

		if (var_8 != 1000) {
			obj.gobDestX = newGobDestX;
			obj.gobDestY = newGobDestY;
		}
	}
}

int8 directionFromDeltaXY(int8 deltaX, int8 deltaY) {
	for (int8 direction = 1; direction <= 8; ++direction) {
		if (deltaXFromDirection[direction] == deltaX &&
			deltaYFromDirection[direction] == deltaY)
			return direction;
	}
	return 0;
}

bool Goblin_v7::directionWalkable(int8 x, int8 y, int8 direction) {
	int8 nextX = x + deltaXFromDirection[direction];
	int8 nextY = y + deltaYFromDirection[direction];
	if (nextX >= 0 &&
		nextX < _vm->_map->getMapWidth() &&
		nextY >= 0 &&
		nextY < _vm->_map->getMapHeight()) {
		return positionWalkable(_vm->_map, nextX, nextY);
	} else
		return false;
}

int32 Goblin_v7::bestWalkableDirectionFromOriginAndDest(int8 x, int8 y, int8 destX, int8 destY) {
	int8 deltaX = 0;
	int8 deltaY = 0;

	if (destX < x)
		deltaX = -1;
	else if (destX > x)
		deltaX = 1;

	if (destY < y)
		deltaY = -1;
	else if (destY > y)
		deltaY = 1;

	int8 direction = directionFromDeltaXY(deltaX, deltaY);
	if (directionWalkable(x, y, direction))
		return direction;

	// Look for another walkable direction
	direction -= 1;
	if (direction <= 0)
		direction += 8;
	if (directionWalkable(x, y, direction))
		return direction;

	direction += 2;
	if (direction > 8)
		direction -= 8;
	if (directionWalkable(x, y, direction))
		return direction;

	direction -= 3;
	if (direction <= 0)
		direction += 8;
	if (directionWalkable(x, y, direction))
		return -direction;

	direction += 4;
	if (direction > 8)
		direction -= 8;
	if (directionWalkable(x, y, direction))
		return -direction;

	return 0;
}


int32 Goblin_v7::findPath(int8 x, int8 y, int8 destX, int8 destY) {
	int8 currentX = x;
	int8 currentY = y;
	int8 returnToPreviousStepDir = -1;
	int8 var_8 = 0;
	int8 firstDirection = 0;
	int8 var_1C = 0;

	while (true) {
		int8 currentDirection = bestWalkableDirectionFromOriginAndDest(currentX, currentY, destX, destY);
		if (currentDirection == 0)
			return 0;

		if (currentDirection >= 0) {
			if (var_8 == 1) {
				var_8 = 2;
				var_1C = findPath(x, y, currentX, currentY);
				if (var_1C > 0)
					firstDirection = var_1C;
			}
		} else {
			currentDirection = -currentDirection;
			if (var_8 == 0)
				var_8 = 1;
		}

		if (returnToPreviousStepDir > 0) {
			returnToPreviousStepDir += 4;
			if (returnToPreviousStepDir > 8)
				returnToPreviousStepDir -= 8;
		}

		if (currentDirection == returnToPreviousStepDir) {
			currentDirection += 4;
			if (currentDirection > 8)
				currentDirection -= 8;

			if (!directionWalkable(currentX, currentY, currentDirection))
				return 0;
		}

		if (firstDirection == 0)
			firstDirection = currentDirection;

		returnToPreviousStepDir = currentDirection; // Will be inverted later
		currentX += deltaXFromDirection[currentDirection];
		currentY += deltaYFromDirection[currentDirection];

		if (currentX != destX || currentY != destY)
			continue;
		else
			return firstDirection;
	}
}

int32 Goblin_v7::computeObjNextDirection(Mult::Mult_Object &obj) {
	Mult::Mult_AnimData animData = *obj.pAnimData;
	if (animData.stateType == 1) {
		warning("STUB: Goblin_v7::computeObjNextDirection animData.stateType == 1");
		return 0;
	} else {
		updateGobDest(_vm->_map, obj);
		int32 direction = findPath(obj.goblinX, obj.goblinY, obj.gobDestX, obj.gobDestY);
		if (direction == 0) {
			direction = bestWalkableDirectionFromOriginAndDest(obj.goblinX, obj.goblinY, obj.gobDestX, obj.gobDestY);
			if (direction < 0)
				direction = -direction;
		}

		if (animData.newState > 0) {
			int32 newState = animData.newState + 4;
			if (newState > 8) {
				newState -= 8;
			}

			if (direction == newState) {
				direction = animData.newState;
				if (!directionWalkable(obj.goblinX, obj.goblinY, direction))
					return 0;
			}
		}

		if (direction < 0)
			return 0;

		obj.destX = obj.goblinX + deltaXFromDirection[direction];
		obj.destY = obj.goblinY + deltaYFromDirection[direction];
		return direction;
	}
}

void Goblin_v7::initiateMove(Mult::Mult_Object *obj) {
	int32 animState = 0;
	if (obj->goblinX != obj->gobDestX || obj->goblinY != obj->gobDestY) {
		debugC(5, kDebugGameFlow, "Computing Obj %s new state (obj->goblinX = %d, obj->gobDestX = %d, obj->goblinY = %d, obj->gobDestY = %d)",
			   obj->animName, obj->goblinX , obj->gobDestX , obj->goblinY ,obj->gobDestY);

		animState = computeObjNextDirection(*obj);

		debugC(5, kDebugGameFlow, "Obj %s new state = %d (obj->goblinX = %d, obj->gobDestX = %d, obj->goblinY = %d, obj->gobDestY = %d)",
			   obj->animName, animState, obj->goblinX , obj->gobDestX , obj->goblinY ,obj->gobDestY);
	}

	debugC(5, kDebugGameFlow, "Obj %s initiateMove (lookDir=%d, obj->posX=%d, obj->posY=%d, obj->goblinX = %d, obj->gobDestX = %d, obj->goblinY = %d, obj->gobDestY = %d)",
		   obj->animName, obj->pAnimData->curLookDir, (int16) *obj->pPosX, (int16) *obj->pPosY,  obj->goblinX , obj->gobDestX , obj->goblinY , obj->gobDestY);

	if (animState != 0) {
		obj->pAnimData->newState = animState;
		setGoblinState(obj, animState);
	} else {
		if (obj->pAnimData->destX != obj->pAnimData->gobDestX_maybe || obj->pAnimData->destY != obj->pAnimData->gobDestY_maybe)
			obj->pAnimData->pathExistence = 2;
		else
			obj->pAnimData->pathExistence = 1;

		obj->pAnimData->animType = 12;
		if (obj->pAnimData->curLookDir >= 20) {
			if (obj->pAnimData->curLookDir >= 30) {
				if (obj->pAnimData->curLookDir >= 40)
					return;
				else {
					setGoblinState(obj, 105);
					obj->pAnimData->pathExistence = 3;
				}
			} else {
				setGoblinState(obj, 101);
				obj->pAnimData->pathExistence = 3;
			}
		} else {
			setGoblinState(obj, obj->pAnimData->curLookDir + 100);
		}
	}
}

} // End of namespace Gob
