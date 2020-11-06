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

#include "twine/movements.h"
#include "common/textconsole.h"
#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/collision.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/input.h"
#include "twine/renderer.h"
#include "twine/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Movements::Movements(TwinEEngine *engine) : _engine(engine) {}

void Movements::getShadowPosition(int32 x, int32 y, int32 z) {
	const uint8 *ptr = _engine->_grid->getBlockBufferGround(x, y, z, processActorY);
	processActorX = x;
	processActorZ = z;

	if (*ptr) {
		const uint8 *blockPtr = _engine->_grid->getBlockLibrary(*ptr - 1) + 3 + *(ptr + 1) * 4;
		const uint8 brickShape = *((const uint8 *)(blockPtr));
		_engine->_actor->shadowCollisionType = brickShape;
	} else {
		_engine->_actor->shadowCollisionType = 0;
	}
	_engine->_collision->reajustActorPosition(_engine->_actor->shadowCollisionType);

	_engine->_actor->shadowX = processActorX;
	_engine->_actor->shadowY = processActorY;
	_engine->_actor->shadowZ = processActorZ;
}

void Movements::setActorAngleSafe(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr) {
	movePtr->from = startAngle & 0x3FF;
	movePtr->to = endAngle & 0x3FF;
	movePtr->numOfStep = stepAngle & 0x3FF;
	movePtr->timeOfChange = _engine->lbaTime;
}

void Movements::clearRealAngle(ActorStruct *actorPtr) {
	setActorAngleSafe(actorPtr->angle, actorPtr->angle, 0, &actorPtr->move);
}

void Movements::setActorAngle(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr) {
	movePtr->from = startAngle;
	movePtr->to = endAngle;
	movePtr->numOfStep = stepAngle;
	movePtr->timeOfChange = _engine->lbaTime;
}

int32 Movements::getAngleAndSetTargetActorDistance(int32 x1, int32 z1, int32 x2, int32 z2) {
	/*
	//Pythagoras
    targetActorDistance = (int32)sqrt((int64)(((z2 - z1)*(z2 - z1) + (x2 - x1)*(x2 - x1))));

	if (targetActorDistance == 0)
        return 0;

    //given two points, we calculate its arc-tangent in radians
    //Then we convert from radians (360 degrees == 2*M_PI) to a 10bit value (360 degrees == 1024) and invert the rotation direction
    //Then we add an offset of 90 degrees (256) and limit it to the 10bit value range.
    return (256 + ((int32)floor((-1024 * atan2((int64)(z2-z1), (int32)(x2-x1))) / (2*M_PI)))) % 1024;
	*/

	int32 difZ = z2 - z1;
	const int32 newZ = difZ * difZ;

	int32 difX = x2 - x1;
	const int32 newX = difX * difX;

	int32 flag;
	// Exchange X and Z
	if (newX < newZ) {
		const int32 tmpEx = difX;
		difX = difZ;
		difZ = tmpEx;

		flag = 1;
	} else {
		flag = 0;
	}

	targetActorDistance = (int32)sqrt((int64)newX + (int64)newZ);

	if (!targetActorDistance) {
		return 0;
	}

	const int32 destAngle = (difZ << 14) / targetActorDistance;

	int32 startAngle = 0;
	//	stopAngle  = 0x100;

	while (_engine->_renderer->shadeAngleTab3[startAngle] > destAngle) {
		startAngle++;
	}

	if (_engine->_renderer->shadeAngleTab3[startAngle] != destAngle) {
		if ((_engine->_renderer->shadeAngleTab3[startAngle - 1] + _engine->_renderer->shadeAngleTab3[startAngle]) / 2 <= destAngle) {
			startAngle--;
		}
	}

	int32 finalAngle = 128 + startAngle;

	if (difX <= 0) {
		finalAngle = -finalAngle;
	}

	if (flag == 1) {
		finalAngle = -finalAngle + 0x100;
	}

	return finalAngle & 0x3FF;
}

int32 Movements::getRealAngle(ActorMoveStruct *movePtr) {
	if (movePtr->numOfStep) {
		const int32 timePassed = _engine->lbaTime - movePtr->timeOfChange;

		if (timePassed >= movePtr->numOfStep) { // rotation is finished
			movePtr->numOfStep = 0;
			return movePtr->to;
		}

		int32 remainingAngle = movePtr->to - movePtr->from;

		if (remainingAngle < -0x200) {
			remainingAngle += 0x400;
		} else if (remainingAngle > 0x200) {
			remainingAngle -= 0x400;
		}

		remainingAngle *= timePassed;
		remainingAngle /= movePtr->numOfStep;
		remainingAngle += movePtr->from;

		return remainingAngle;
	}

	return movePtr->to;
}

int32 Movements::getRealValue(ActorMoveStruct *movePtr) {
	if (!movePtr->numOfStep) {
		return movePtr->to;
	}

	if (!(_engine->lbaTime - movePtr->timeOfChange < movePtr->numOfStep)) {
		movePtr->numOfStep = 0;
		return movePtr->to;
	}

	int32 tempStep = movePtr->to - movePtr->from;
	tempStep *= _engine->lbaTime - movePtr->timeOfChange;
	tempStep /= movePtr->numOfStep;

	return tempStep + movePtr->from;
}

void Movements::rotateActor(int32 x, int32 z, int32 angle) {
	const double radians = 2 * M_PI * angle / 0x400;
	_engine->_renderer->destX = (int32)(x * cos(radians) + z * sin(radians));
	_engine->_renderer->destZ = (int32)(-x * sin(radians) + z * cos(radians));
}

int32 Movements::getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2) {
	return (int32)sqrt(((int64)(x2 - x1) * (int64)(x2 - x1) + (int64)(z2 - z1) * (int64)(z2 - z1)));
}

int32 Movements::getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2) {
	return (int32)sqrt(((int64)(x2 - x1) * (int64)(x2 - x1) + (int64)(y2 - y1) * (int64)(y2 - y1) + (int64)(z2 - z1) * (int64)(z2 - z1)));
}

void Movements::moveActor(int32 angleFrom, int32 angleTo, int32 speed, ActorMoveStruct *movePtr) { // ManualRealAngle
	const int16 from = angleFrom & 0x3FF;
	const int16 to = angleTo & 0x3FF;

	movePtr->from = from;
	movePtr->to = to;

	const int16 numOfStep = (from - to) << 6;

	int32 numOfStepInt;
	if (numOfStep < 0) {
		numOfStepInt = -numOfStep;
	} else {
		numOfStepInt = numOfStep;
	}

	numOfStepInt >>= 6;

	numOfStepInt *= speed;
	numOfStepInt >>= 8;

	movePtr->numOfStep = (int16)numOfStepInt;
	movePtr->timeOfChange = _engine->lbaTime;
}

void Movements::update() {
	previousLoopActionKey = heroActionKey;
	heroActionKey = _engine->_input->isHeroActionActive();
	loopCursorKeys = _engine->_input->cursorKeys;
}

void Movements::processManualAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (IS_HERO(actorIdx)) {
		heroAction = false;

		// If press W for action
		if (_engine->_input->toggleActionIfActive(TwinEActionType::SpecialAction)) {
			heroAction = true;
		}

		// Process hero actions
		if (_engine->_input->isActionActive(TwinEActionType::ExecuteBehaviourAction)) {
			switch (_engine->_actor->heroBehaviour) {
			case kNormal:
				heroAction = true;
				break;
			case kAthletic:
				_engine->_animations->initAnim(kJump, 1, 0, actorIdx);
				break;
			case kAggressive:
				if (_engine->_actor->autoAgressive) {
					heroMoved = true;
					actor->angle = getRealAngle(&actor->move);
					// TODO: previousLoopActionKey must be handled properly
					if (!previousLoopActionKey || actor->anim == kAnimNone) {
						const int32 aggresiveMode = _engine->getRandomNumber(3);

						switch (aggresiveMode) {
						case 0:
							_engine->_animations->initAnim(kKick, 1, 0, actorIdx);
							break;
						case 1:
							_engine->_animations->initAnim(kRightPunch, 1, 0, actorIdx);
							break;
						case 2:
							_engine->_animations->initAnim(kLeftPunch, 1, 0, actorIdx);
							break;
						}
					}
				} else {
					if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
						_engine->_animations->initAnim(kLeftPunch, 1, 0, actorIdx);
					} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
						_engine->_animations->initAnim(kRightPunch, 1, 0, actorIdx);
					}

					if (_engine->_input->toggleActionIfActive(TwinEActionType::MoveForward)) {
						_engine->_animations->initAnim(kKick, 1, 0, actorIdx);
					}
				}
				break;
			case kDiscrete:
					_engine->_animations->initAnim(kHide, 0, 255, actorIdx);
				break;
			case kProtoPack:
				break;
			}
		}
	}

	if (_engine->_input->isActionActive(TwinEActionType::ThrowMagicBall) && !_engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED]) {
		if (!_engine->_gameState->usingSabre) { // Use Magic Ball
			if (_engine->_gameState->gameFlags[InventoryItems::kiMagicBall]) {
				if (_engine->_gameState->magicBallIdx == -1) {
					_engine->_animations->initAnim(kThrowBall, 1, 0, actorIdx);
				}

				heroMoved = true;
				actor->angle = getRealAngle(&actor->move);
			}
		} else if (_engine->_gameState->gameFlags[InventoryItems::kiUseSabre]) {
			if (actor->body != InventoryItems::kiUseSabre) {
				_engine->_actor->initModelActor(InventoryItems::kiUseSabre, actorIdx);
			}

			_engine->_animations->initAnim(kSabreAttack, 1, 0, actorIdx);

			heroMoved = true;
			actor->angle = getRealAngle(&actor->move);
		}
	}

	// TODO: remove loopCursorKeys here
	if (!loopCursorKeys || heroAction) {
		// if continue walking
		if (_engine->_input->isActionActive(TwinEActionType::MoveForward) || _engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			heroMoved = false; // don't break animation
		}

		if (heroActionKey != heroPressedKey || loopCursorKeys != heroPressedKey2) {
			if (heroMoved) {
				_engine->_animations->initAnim(kStanding, 0, 255, actorIdx);
			}
		}

		heroMoved = false;

		if (_engine->_input->isActionActive(TwinEActionType::MoveForward)) {
			if (!_engine->_scene->currentActorInZone) {
				_engine->_animations->initAnim(kForward, 0, 255, actorIdx);
			}
			heroMoved = true;
		} else if (_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			_engine->_animations->initAnim(kBackward, 0, 255, actorIdx);
			heroMoved = true;
		}

		if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
			heroMoved = true;
			if (actor->anim == 0) {
				_engine->_animations->initAnim(kTurnLeft, 0, 255, actorIdx);
			} else {
				if (!actor->dynamicFlags.bIsRotationByAnim) {
					actor->angle = getRealAngle(&actor->move);
				}
			}
		} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
			heroMoved = true;
			if (actor->anim == 0) {
				_engine->_animations->initAnim(kTurnRight, 0, 255, actorIdx);
			} else {
				if (!actor->dynamicFlags.bIsRotationByAnim) {
					actor->angle = getRealAngle(&actor->move);
				}
			}
		}
	}

	int16 tempAngle;
	if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
		tempAngle = 0x100;
	} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
		tempAngle = -0x100;
	} else {
		tempAngle = 0;
	}

	moveActor(actor->angle, actor->angle + tempAngle, actor->speed, &actor->move);

	heroPressedKey = heroActionKey;
	heroPressedKey2 = loopCursorKeys;
}

void Movements::processFollowAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const ActorStruct* followedActor = _engine->_scene->getActor(actor->followedActor);
	int32 newAngle = getAngleAndSetTargetActorDistance(actor->x, actor->z, followedActor->x, followedActor->z);
	if (actor->staticFlags.bIsSpriteActor) {
		actor->angle = newAngle;
	} else {
		moveActor(actor->angle, newAngle, actor->speed, &actor->move);
	}
}

void Movements::processRandomAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->dynamicFlags.bIsRotationByAnim) {
		return;
	}

	if (actor->brickShape & 0x80) {
		moveActor(actor->angle, (((_engine->getRandomNumber() & 0x100) + (actor->angle - 0x100)) & 0x3FF), actor->speed, &actor->move);
		actor->info0 = _engine->getRandomNumber(300) + _engine->lbaTime + 300;
		_engine->_animations->initAnim(kStanding, 0, 255, actorIdx);
	}

	if (!actor->move.numOfStep) {
		_engine->_animations->initAnim(kForward, 0, 255, actorIdx);
		if (_engine->lbaTime > actor->info0) {
			moveActor(actor->angle, (((_engine->getRandomNumber() & 0x100) + (actor->angle - 0x100)) & 0x3FF), actor->speed, &actor->move);
			actor->info0 = _engine->getRandomNumber(300) + _engine->lbaTime + 300;
		}
	}
}

void Movements::processTrackAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->positionInMoveScript == -1) {
		actor->positionInMoveScript = 0;
	}
}

void Movements::processSameXZAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const ActorStruct* followedActor = _engine->_scene->getActor(actor->followedActor);
	actor->x = followedActor->x;
	actor->z = followedActor->z;
}

void Movements::processActorMovements(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->entity == -1) {
		return;
	}

	if (actor->dynamicFlags.bIsFalling) {
		if (actor->controlMode != ControlMode::kManual) {
			return;
		}

		int16 tempAngle = 0;
		if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
			tempAngle = 0x100;
		} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
			tempAngle = -0x100;
		}

		moveActor(actor->angle, actor->angle + tempAngle, actor->speed, &actor->move);

		heroPressedKey = heroActionKey;
		return;
	}
	if (!actor->staticFlags.bIsSpriteActor) {
		if (actor->controlMode != ControlMode::kManual) {
			actor->angle = getRealAngle(&actor->move);
		}
	}

	switch (actor->controlMode) {
	/**
	 * The Actor's Track Script is stopped. Track Script execution may be started with Life Script of
	 * the Actor or other Actors (with SET_TRACK(_OBJ) command). This mode does not mean the Actor
	 * will literally not move, but rather that it's Track Script (also called Move Script) is
	 * initially stopped. The Actor may move if it is assigned a moving animation.
	 */
	case kNoMove:
	case kFollow2:     // unused
	case kTrackAttack: // unused
		break;
	case kManual:
		processManualAction(actorIdx);
		break;
	case kFollow:
		processFollowAction(actorIdx);
		break;
	case kTrack:
		processTrackAction(actorIdx);
		break;
	case kSameXZ:
		processSameXZAction(actorIdx);
		break;
	case kRandom:
		processRandomAction(actorIdx);
		break;
	default:
		warning("Unknown Control mode %d\n", actor->controlMode);
		break;
	}
}

} // namespace TwinE
