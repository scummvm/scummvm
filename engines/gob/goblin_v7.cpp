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

static void sub_4309D3(Draw *draw, Mult::Mult_Object &obj) {
	if (obj.field_38 != nullptr) {
		Mult::Mult_AnimData animData = *obj.pAnimData;
		draw->freeSprite(50 - animData.animation - 1);
		// sub_473261()

		if (obj.field_50)
			obj.field_3C = nullptr;
		else
			obj.field_38[10] = 0;

		if (obj.field_38[40] != 0) {

		}
	}

	obj.field_38 = nullptr;
	obj.animName[0] = '\0';
}

void Goblin_v7::changeDirection(Mult::Mult_Object *obj, int16 animState) {
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
		if (animState <= 0 || animState > obj->animVariables[2]) {
			obj->pAnimData->animType = 11;
			return;
		} else {
			int16 *animVariablesForState = obj->animVariables + animState * obj->animVariables[1];
			if (animVariablesForState[0] == 0) {
				newXCorrection = animVariablesForState[1];
				newYCorrection = animVariablesForState[2];
			} else {
				if (animVariablesForState[0] == -2) {
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
						animState -= 10;
						break;
					default: // 3, 7, 9-30, > 36
						obj->pAnimData->animType = 11;
						return;
					}

					obj->pAnimData->layer |= 0x80;
					newXCorrection = animVariablesForState[1];
					newYCorrection = animVariablesForState[2];
				} else if (animVariablesForState[0] == -1) {
					obj->pAnimData->animType = 11;
					return;
				} else {
					animState = animVariablesForState[0];
					continue;
				}
			}
		}

		break;
	}

	int32 var_A4 = 0;
	if (obj->pAnimData->stateType == 1) {
		var_A4 = animState;
		if (var_A4 == 22)
			animState = 27;
		else if (var_A4 == 25)
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
	case 22:
		warning("Goblin_v7::changeDirection: animState == 22 not implemented");
		break;
	case 23:
		Common::strlcat(str, "EXP", 128);
		break;
	case 24:
		Common::strlcat(str, "FRA", 128);
		break;
	case 25:
		warning("Goblin_v7::changeDirection: animState == 25 not implemented");
		break;
	case 26:
		warning("Goblin_v7::changeDirection: animState == 26 not implemented");
		break;
	case 27:
		warning("Goblin_v7::changeDirection: animState == 27 not implemented");
		break;

	default:
		Common::strlcat(str, Common::String::format("%02d", animState).c_str(), 128);
	}

	if (strcmp(str, obj->animName) != 0) {
		Common::strlcpy(obj->animName, str, 16);
	}

	int32 newX = 0;
	int32 newY = 0;
	if (_vm->_map->_mapUnknownBool) {
		// Weird coordinates mapping
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
		sub_4309D3(_vm->_draw, *obj);

		VideoPlayer::Properties props;
		props.x          = -1;
		props.y          = -1;
		props.startFrame = 0;
		props.lastFrame  = 0;
		props.breakKey   = 0;
		props.flags      = 0x1201;
		props.palStart   = 0;
		props.palEnd     = 0;
		props.sprite = -obj->pAnimData->animation - 1;

		Common::strlcpy(obj->animName, str, 16);
		if (obj->videoSlot > 0)
			_vm->_vidPlayer->closeVideo(obj->videoSlot - 1);

		int slot = _vm->_vidPlayer->openVideo(false, str, props);
		obj->videoSlot = slot + 1;
	} else {
		if (obj->field_38 == nullptr ||
			strcmp(obj->animName, str) != 0 ||
			obj->field_38[4] & 0x800) {

			VideoPlayer::Properties props;
			props.x          = -1;
			props.y          = -1;
			props.startFrame = 0;
			props.lastFrame  = 0;
			props.breakKey   = 0;
			props.flags      = 0x1601;
			props.palStart   = 0;
			props.palEnd     = 0;
			props.sprite = -obj->pAnimData->animation - 1;

			if (obj->videoSlot > 0)
				_vm->_vidPlayer->closeVideo(obj->videoSlot - 1);

			int slot = _vm->_vidPlayer->openVideo(false, str, props);
			debugC(1, kDebugVideo, "Playing video \"%s\" (change directions 2) @ %d+%d, frames %d - %d, "
								   "paletteCmd %d (%d - %d), flags %X, slot = %d", str,
								   props.x, props.y, props.startFrame, props.lastFrame,
								   props.palCmd, props.palStart, props.palEnd, props.flags, slot);
			obj->videoSlot = slot + 1;
		}
	}
}

/*
 * 0: (0, 0)
 * 1: (-1, -1)
 * 2: (0, -1)
 * 3: (1, -1)
 * 4: (1, 0)
 * 5: (1, 1)
 * 6: (0, 1)
 * 7: (-1, 1)
 * 8: (-1, 0)
 * 9: (0, 0)
 */

static int8 deltaXFromDirection[10] = {0, -1, 0, 1, 1, 1, 0, -1, -1, 0};
static int8 deltaYFromDirection[10] = {0, -1, -1, -1, 0, 1, 1, 1, 0, 0};

static void updateGobDest(Map *map, Mult::Mult_Object &obj) {
	if (map->getPass(obj.gobDestX, obj.gobDestY, map->getMapWidth())) {
		int8 newGobDestX = 0;
		int32 var_8 = 1000;
		int8 newGobDestY = 0;
		int32 direction = 2;
		for (;direction <= 8; direction += 2) {
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

				if (!map->getPass(tempGobDestX, tempGobDestY, map->getMapWidth())) {
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
	int8 newX = x + deltaXFromDirection[direction];
	int8 newY = y + deltaYFromDirection[direction];
	if (newX >= 0 &&
		newX < _vm->_map->getMapWidth() &&
		newY >= 0 &&
		newY < _vm->_map->getMapHeight()) {
		return _vm->_map->getPass(x, y, direction);
	}
	else
		return false;
}

int32 Goblin_v7::directionFromOriginAndDest(int8 x, int8 y, int8 destX, int8 destY) {
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
	int8 newX = x;
	int8 newY = y;
	int8 previousDir = -1;
	int8 var_8 = 0;
	int8 var_18 = 0;
	int8 var_1C = 0;

	while (true)
	{
		int8 direction = directionFromOriginAndDest(newX, newY , destX, destY);
		if (direction == 0)
			return 0;

		if (direction >= 0) {
			if (var_8 == 1) {
				var_8 = 2;
				var_1C = findPath(x, y, newX, newY);
				if (var_1C > 0)
					var_18 = var_1C;
			}
		}
		else {
			direction = -direction;
			if (var_8 == 0)
				var_8 = 1;
		}

		if (previousDir > 0) {
			previousDir += 4;
			if (previousDir > 8)
				previousDir -= 8;
		}

		if (direction == previousDir) {
			direction += 4;
			if (direction > 8)
				direction -= 8;

			if (!directionWalkable(newX, newY, direction))
				return 0;
		}

		if (var_18 == 0)
			var_18 = direction;

		previousDir = direction;
		newX += deltaXFromDirection[direction];
		newY += deltaYFromDirection[direction];

		if (newX != destX || newY != destY)
			continue;
		else
			return var_18;
	}
}

int32 Goblin_v7::computeObjNextDirection(Mult::Mult_Object &obj) {
	Mult::Mult_AnimData animData = *obj.pAnimData;
	if (animData.stateType == 1) {
		warning("Adibou2 computeObjNextDirection animData.stateType == 1 stub");
		return 0;
	}
	else {
		updateGobDest(_vm->_map, obj);
		int32 direction = findPath(obj.goblinX, obj.goblinY, obj.gobDestX, obj.gobDestY);
		if (direction == 0) {
			direction = directionFromOriginAndDest(obj.goblinX, obj.goblinY, obj.gobDestX, obj.gobDestY);
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
		changeDirection(obj, animState);
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
					changeDirection(obj, 105);
					obj->pAnimData->pathExistence = 3;
				}
			} else {
				changeDirection(obj, 101);
				obj->pAnimData->pathExistence = 3;
			}
		} else {
			changeDirection(obj, obj->pAnimData->curLookDir + 100);
		}
	}
}

} // End of namespace Gob
