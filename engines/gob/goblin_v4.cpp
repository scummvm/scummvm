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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "gob/gob.h"
#include "gob/goblin.h"
#include "gob/global.h"
#include "gob/mult.h"
#include "gob/map.h"
#include "gob/scenery.h"
#include "gob/inter.h"

namespace Gob {

Goblin_v4::Goblin_v4(GobEngine *vm) : Goblin_v3(vm) {
}

void Goblin_v4::movePathFind(Mult::Mult_Object *obj, Gob_Object *gobDesc, int16 nextAct) {
	Mult::Mult_AnimData *animData;
	int16 framesCount;
	int16 gobX;
	int16 gobY;
	int16 gobDestX;
	int16 gobDestY;
	int16 destX;
	int16 destY;
	int16 dir;

	dir = 0;
	animData = obj->pAnimData;
	framesCount = _vm->_scenery->getAnimLayer(animData->animation, animData->layer)->framesCount;
	animData->newCycle = framesCount;
	gobX = obj->goblinX;
	gobY = obj->goblinY;
	animData->order = gobY;
	gobDestX = obj->gobDestX;
	gobDestY = obj->gobDestY;
	animData->destX = gobDestX;
	animData->destY = gobDestY;
	destX = obj->destX;
	destY = obj->destY;

	if (animData->pathExistence == 1) {
		dir = _vm->_map->getDirection(gobX, gobY, destX, destY);
		if (dir == 0)
			animData->pathExistence = 0;
		if ((gobX == destX) && (gobY == destY))
			animData->pathExistence = 4;
	} else if (animData->pathExistence == 3) {
		if ((gobX == gobDestX) && (gobY == gobDestY)) {
			animData->pathExistence = 4;
			destX = gobDestX;
			destY = gobDestY;
		} else {
			if (_vm->_map->checkDirectPath(obj, gobX, gobY, gobDestX, gobDestY) != 1) {
				if ((gobX == destX) && (gobY == destY)) {
					if (obj->nearestWayPoint > obj->nearestDest) {
						_vm->_map->optimizePoints(obj, gobX, gobY);
						destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
						destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
						if (_vm->_map->checkDirectPath(obj, gobX, gobY, destX, destY) == 3) {
							WRITE_VAR(56, 1);
							animData->pathExistence = 0;
						}
						if (obj->nearestWayPoint > obj->nearestDest)
							obj->nearestWayPoint--;
					} else if (obj->nearestWayPoint < obj->nearestDest) {
						_vm->_map->optimizePoints(obj, gobX, gobY);
						destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
						destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
						if (_vm->_map->checkDirectPath(obj, gobX, gobY, destX, destY) == 3) {
							WRITE_VAR(56, 1);
							animData->pathExistence = 0;
						}
						if (obj->nearestWayPoint < obj->nearestDest)
							obj->nearestWayPoint++;
					} else {
						if ((_vm->_map->checkDirectPath(obj, gobX, gobY, gobDestX, gobDestY) == 3) &&
								(_vm->_map->getPass(gobDestX, gobDestY) != 0)) {
							destX = _vm->_map->_wayPoints[obj->nearestWayPoint].x;
							destY = _vm->_map->_wayPoints[obj->nearestWayPoint].y;
							WRITE_VAR(56, 1);
						} else {
							animData->pathExistence = 1;
							destX = gobDestX;
							destY = gobDestY;
						}
					}
				}
			} else {
				destX = gobDestX;
				destY = gobDestY;
			}
			dir = _vm->_map->getDirection(gobX, gobY, destX, destY);
		}
	}

	obj->goblinX = gobX;
	obj->goblinY = gobY;
	obj->gobDestX = gobDestX;
	obj->gobDestY = gobDestY;
	obj->destX = destX;
	obj->destY = destY;

	if (_vm->_map->_widthByte == 4) {
		switch (dir) {
		case Map::kDirNW:
			animData->nextState = sub_20430(animData->state, Map::kDirNW);
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) &&
					(animData->nextState == 1))
				animData->nextState = 40;
			if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) != 10)
				animData->nextState = sub_20430(animData->state, Map::kDirNW);
			break;

		case Map::kDirN:
			animData->nextState =
				(animData->curLookDir == 2) ? 2 : sub_20430(animData->state, Map::kDirN);
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) {
				if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) != 10) {
					if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) == 10)
						animData->nextState = 42;
					else
						animData->nextState = 2;
				} else
					animData->nextState = 40;
			}
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20) &&
			    (animData->nextState == 2))
				animData->nextState = 38;
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19) &&
			   (animData->nextState == 2))
				animData->nextState = 26;
			break;

		case Map::kDirNE:
			animData->nextState = sub_20430(animData->state, Map::kDirNE);
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) &&
			    (animData->nextState == 3))
				animData->nextState = 42;
			if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) != 10)
				animData->nextState = sub_20430(animData->state, Map::kDirNE);
			break;

		case Map::kDirW:
			animData->nextState = sub_20430(animData->state, Map::kDirW);
			break;

		case Map::kDirE:
			animData->nextState = sub_20430(animData->state, Map::kDirE);
			break;

		case Map::kDirSW:
			animData->nextState = sub_20430(animData->state, Map::kDirSW);
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) &&
			    (animData->nextState == 7))
				animData->nextState = 41;
			if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY) != 10)
				animData->nextState = sub_20430(animData->state, Map::kDirSW);
			break;

		case Map::kDirS:
			animData->nextState =
				(animData->curLookDir == 6) ? 6 : sub_20430(animData->state, Map::kDirS);
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) {
				if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY + 2) != 10) {
					if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY + 2) == 10)
						animData->nextState = 43;
					else
						animData->nextState = 6;
				} else
					animData->nextState = 41;
			}
			// loc_20AAD
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20) &&
			    (animData->nextState == 6))
				animData->nextState = 39;
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19) &&
			   (animData->nextState == 6))
				animData->nextState = 27;
			break;

		case Map::kDirSE:
			animData->nextState = sub_20430(animData->state, Map::kDirSE);
			if ((_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) &&
			    (animData->nextState == 5))
				animData->nextState = 43;
			if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY) != 10)
				animData->nextState = sub_20430(animData->state, Map::kDirSE);
			break;

		default:
			// loc_20D18
			switch (animData->state) {
			case 0:
			case 8:
				// loc_21134
				animData->nextState = 8;
				break;

			case 1:
			case 10:
			case 40:
				// loc_21152
				animData->nextState = 10;
				break;

			case 2:
			case 29:
				// loc_2113E
				animData->nextState = 29;
				break;

			case 3:
			case 11:
			case 42:
				// loc_2115C
				animData->nextState = 11;
				break;

			case 4:
			case 9:
				// loc_2112A
				animData->nextState = 9;
				break;

			case 5:
			case 30:
			case 43:
				// loc_21166
				animData->nextState = 30;
				break;

			case 6:
			case 28:
				// loc_21148
				animData->nextState = 28;
				break;

			case 7:
			case 31:
			case 41:
				// loc_21170
				animData->nextState = 31;
				break;
			}
			break;
		}
	} else {
		switch (dir) {
		case Map::kDirNW:
			animData->nextState = 1;
			if (_vm->_map->_screenWidth == 640) {
				if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
					animData->nextState = 40;
				if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) != 10)
					animData->nextState = 1;
			}
			break;

		case Map::kDirN:
			animData->nextState =
				(animData->curLookDir == 2) ? 2 : rotateState(animData->curLookDir, 2);
			if (_vm->_map->_screenWidth == 640) {
				if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) {
					if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) != 10) {
						if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) == 10)
							animData->nextState = 42;
						else
							animData->nextState = 2;
					} else
						animData->nextState = 40;
				} else if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20)
					animData->nextState = 38;
				else if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19)
					animData->nextState = 26;
			}
			break;

		case Map::kDirNE:
			animData->nextState =	3;
			if (_vm->_map->_screenWidth == 640) {
				if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
					animData->nextState = 42;
				if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) != 10)
					animData->nextState = 3;
			}
			break;

		case Map::kDirW:
			animData->nextState = rotateState(animData->curLookDir, 0);
			break;

		case Map::kDirE:
			animData->nextState = rotateState(animData->curLookDir, 4);
			break;

		case Map::kDirSW:
			animData->nextState = 7;
			if (_vm->_map->_screenWidth == 640) {
				if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
					animData->nextState = 41;
				if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY + 2) != 10)
					animData->nextState = 7;
			}
			break;

		case Map::kDirS:
			animData->nextState =
				(animData->curLookDir == 6) ? 6 : rotateState(animData->curLookDir, 6);
			if (_vm->_map->_screenWidth == 640) {
				if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20)
					animData->nextState = 39;
				else if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19)
					animData->nextState = 27;
			}
			break;

		case Map::kDirSE:
			animData->nextState = 5;
			if (_vm->_map->_screenWidth == 640) {
				if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
					animData->nextState = 43;
				if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY + 2) != 10)
					animData->nextState = 5;
			}
			break;

		default:
			switch (animData->curLookDir) {
			case 0:
				animData->nextState = 8;
				break;
			case 1:
				animData->nextState = 10;
				break;
			case 2:
				animData->nextState = 29;
				break;
			case 3:
				animData->nextState = 11;
				break;
			case 4:
				animData->nextState = 9;
				break;
			case 5:
				animData->nextState = 30;
				break;
			case 6:
				animData->nextState = 28;
				break;
			case 7:
				animData->nextState = 31;
				break;
			}
			break;
		}
	}
}

void Goblin_v4::moveAdvance(Mult::Mult_Object *obj, Gob_Object *gobDesc,
		int16 nextAct, int16 framesCount) {
	Mult::Mult_AnimData *animData;
	int16 gobX;
	int16 gobY;
	int16 animation;
	int16 state;
	int16 layer;

	if (!obj->goblinStates)
		return;

	movePathFind(obj, 0, 0);
	playSounds(obj);

	animData = obj->pAnimData;

	framesCount = _vm->_scenery->getAnimLayer(animData->animation, animData->layer)->framesCount;

	if (animData->isPaused == 0)
		animData->frame++;

	switch (animData->stateType) {
	case 0:
	case 1:
		animData->isPaused = 0;
		break;

	case 4:
		if (animData->frame == 0)
			animData->isPaused = 1;
		break;

	case 6:
		if (animData->frame >= framesCount)
			animData->isPaused = 1;
		break;
	}

	switch (animData->state) {
	case 0:
	case 1:
	case 7:
	case 13:
	case 16:
	case 23:
	case 40:
	case 41:
		animData->curLookDir = 0;
		break;

	case 2:
	case 15:
	case 18:
	case 21:
	case 26:
	case 38:
		animData->curLookDir = 2;
		break;

	case 3:
	case 4:
	case 5:
	case 12:
	case 19:
	case 22:
	case 42:
	case 43:
		animData->curLookDir = 4;
		break;

	case 6:
	case 14:
	case 17:
	case 20:
	case 27:
	case 39:
		animData->curLookDir = 6;
		break;

	case 8:
	case 9:
	case 10:
	case 11:
	case 28:
	case 29:
	case 30:
	case 31:
		if (animData->pathExistence == 4)
			animData->pathExistence = 5;
		break;
	}

	if ((animData->newState != -1) && (animData->frame == framesCount) &&
			(animData->newState != animData->state)) {
		animData->nextState = animData->newState;
		animData->newState = -1;
		animData->state = animData->nextState;

		Scenery::AnimLayer *animLayer =
			_vm->_scenery->getAnimLayer(animData->animation, animData->layer);
		*obj->pPosX += animLayer->animDeltaX;
		*obj->pPosY += animLayer->animDeltaY;

		animation = obj->goblinStates[animData->nextState][0].animation;
		layer = obj->goblinStates[animData->nextState][0].layer;
		animData->layer = layer;
		animData->animation = animation;
		animData->frame = 0;
	} else {
		if (isMovement(animData->state)) {
			state = animData->nextState;
			if (animData->frame == ((framesCount + 1) / 2)) {
				gobX = obj->goblinX;
				gobY = obj->goblinY;

				advMovement(obj, state);

				if (animData->state != state) {
					animation = obj->goblinStates[state][0].animation;
					layer = obj->goblinStates[state][0].layer;
					animData->layer = layer;
					animData->animation = animation;
					animData->frame = 0;
					animData->state = state;
					_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
					if (_vm->_map->_bigTiles)
						*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
							(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) - (gobY + 1) / 2;
					else
						*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
							(_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
					*obj->pPosX = gobX * _vm->_map->_tilesWidth;
				}
			}
		}

		if (animData->frame >= framesCount) {
			state = animData->nextState;
			animation = obj->goblinStates[state][0].animation;
			layer = obj->goblinStates[state][0].layer;
			animData->layer = layer;
			animData->animation = animation;
			animData->frame = 0;
			animData->state = state;
			gobX = obj->goblinX;
			gobY = obj->goblinY;

			advMovement(obj, state);

			_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
			if (_vm->_map->_bigTiles)
				*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
					(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) - (gobY + 1) / 2;
			else
				*obj->pPosY = ((gobY + 1) * _vm->_map->_tilesHeight) -
					(_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			*obj->pPosX = gobX * _vm->_map->_tilesWidth;
		}
	}
}

int16 Goblin_v4::sub_20430(int16 state, uint16 dir) {
	static const int16 word_3F25E[8][8] = {
		{ 0,  1, 10, 10, 10, 31, 31,  7},
		{ 0,  1,  2, 29, 29, 29,  8,  8},
		{10,  1,  2,  3, 11, 11, 11, 10},
		{29, 29,  2,  3,  4,  9,  9,  9},
		{30, 11, 11,  3,  4,  5, 30, 30},
		{28, 28,  9,  9,  4,  5,  6, 28},
		{31, 31, 31, 30, 30,  5,  6,  7},
		{ 0,  8,  8,  8, 28, 28,  6,  7}
	};
	int16 dx = state, cx = 0;

	switch (state) {
	case 0:
	case 8:
		// loc_20447
		dx = 0;
		break;

	case 1:
	case 10:
	case 40:
		// loc_2044B
		dx = 1;
		break;

	case 3:
	case 11:
	case 42:
		// loc_20455
		dx = 3;
		break;

	case 5:
	case 30:
	case 43:
		// loc_2045F
		dx = 5;
		break;

	case 7:
	case 31:
	case 41:
		// loc_20469
		dx = 7;
		break;

	case 9:
		// loc_2045A
		dx = 4;
		break;

	case 28:
		// loc_20464
		dx = 6;
		break;

	case 29:
		// loc_20450
		dx = 2;
		break;
	}

	// loc_2046C

	switch (dir) {
	case Map::kDirNW:
		cx = 1;
		break;

	case Map::kDirN:
		cx = 2;
		break;
	
	case Map::kDirNE:
		cx = 3;
		break;
	
	case Map::kDirW:
		cx = 0;
		break;
	
	case Map::kDirE:
		cx = 4;
		break;
	
	case Map::kDirSW:
		cx = 7;
		break;

	case Map::kDirS:
		cx = 6;
		break;

	case Map::kDirSE:
		cx = 5;
		break;
	}

	return word_3F25E[dx][cx];
}

} // End of namespace Gob
