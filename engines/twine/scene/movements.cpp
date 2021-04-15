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

#include "twine/scene/movements.h"
#include "common/textconsole.h"
#include "twine/input.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/shadeangletab.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/collision.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Movements::Movements(TwinEEngine *engine) : _engine(engine) {}

void Movements::getShadowPosition(int32 x, int32 y, int32 z) {
	const uint8 *ptr = _engine->_grid->getBlockBufferGround(x, y, z, processActor.y);
	processActor.x = x;
	processActor.z = z;

	ShapeType shadowCollisionType;
	if (*ptr) {
		const uint8 *blockPtr = _engine->_grid->getBlockLibrary(*ptr - 1) + 3 + *(ptr + 1) * 4;
		const ShapeType brickShape = (ShapeType) * ((const uint8 *)(blockPtr));
		shadowCollisionType = brickShape;
	} else {
		shadowCollisionType = ShapeType::kNone;
	}
	_engine->_collision->reajustActorPosition(shadowCollisionType);

	_engine->_actor->shadowCoord = processActor;
}

void Movements::setActorAngleSafe(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr) {
	movePtr->from = ClampAngle(startAngle);
	movePtr->to = ClampAngle(endAngle);
	movePtr->numOfStep = ClampAngle(stepAngle);
	movePtr->timeOfChange = _engine->lbaTime;
}

void Movements::clearRealAngle(ActorStruct *actorPtr) {
	setActorAngleSafe(actorPtr->angle, actorPtr->angle, ANGLE_0, &actorPtr->move);
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
	targetActorDistance = (int32)sqrt((float)(((z2 - z1)*(z2 - z1) + (x2 - x1)*(x2 - x1))));

	if (targetActorDistance == 0)
		return 0;

	//given two points, we calculate its arc-tangent in radians
	//Then we convert from radians (360 degrees == 2*M_PI) to a 10bit value (360 degrees == 1024) and invert the rotation direction
	//Then we add an offset of 90 degrees (256) and limit it to the 10bit value range.
	return (256 + ((int32)floor((-1024 * atan2((float)(z2-z1), (int32)(x2-x1))) / (2*M_PI)))) % 1024;
	*/

	int32 difZ = z2 - z1;
	const int32 newZ = difZ * difZ;

	int32 difX = x2 - x1;
	const int32 newX = difX * difX;

	bool flag;
	// Exchange X and Z
	if (newX < newZ) {
		const int32 tmpEx = difX;
		difX = difZ;
		difZ = tmpEx;

		flag = true;
	} else {
		flag = false;
	}

	targetActorDistance = (int32)sqrt((float)(newX + newZ));

	if (!targetActorDistance) {
		return 0;
	}

	const int32 destAngle = (difZ * SCENE_SIZE_HALF) / targetActorDistance;

	int32 startAngle = ANGLE_0;
	//	stopAngle  = ANGLE_90;
	const int16 *shadeAngleTab3(&shadeAngleTable[ANGLE_135]);
	while (shadeAngleTab3[startAngle] > destAngle) {
		startAngle++;
	}

	if (shadeAngleTab3[startAngle] != destAngle) {
		if ((shadeAngleTab3[startAngle - 1] + shadeAngleTab3[startAngle]) / 2 <= destAngle) {
			startAngle--;
		}
	}

	int32 finalAngle = ANGLE_45 + startAngle;

	if (difX <= 0) {
		finalAngle = -finalAngle;
	}

	if (flag) {
		finalAngle = -finalAngle + ANGLE_90;
	}

	return ClampAngle(finalAngle);
}

void Movements::rotateActor(int32 x, int32 z, int32 angle) {
	const double radians = AngleToRadians(angle);
	_engine->_renderer->destPos.x = (int32)(x * cos(radians) + z * sin(radians));
	_engine->_renderer->destPos.z = (int32)(-x * sin(radians) + z * cos(radians));
}

int32 Movements::getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2) const {
	return (int32)sqrt((float)((x2 - x1) * (x2 - x1) + (z2 - z1) * (z2 - z1)));
}

int32 Movements::getDistance2D(const IVec3 &v1, const IVec3 &v2) const {
	return (int32)sqrt((float)((v2.x - v1.x) * (v2.x - v1.x) + (v2.z - v1.z) * (v2.z - v1.z)));
}

int32 Movements::getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2) const {
	return (int32)sqrt((float)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1)));
}

int32 Movements::getDistance3D(const IVec3 &v1, const IVec3 &v2) const {
	return (int32)sqrt((float)((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y) + (v2.z - v1.z) * (v2.z - v1.z)));
}

void Movements::moveActor(int32 angleFrom, int32 angleTo, int32 speed, ActorMoveStruct *movePtr) const { // ManualRealAngle
	const int16 from = ClampAngle(angleFrom);
	const int16 to = ClampAngle(angleTo);

	movePtr->from = from;
	movePtr->to = to;

	const int16 numOfStep = (from - to) * 64;
	int32 numOfStepInt = ABS(numOfStep);
	numOfStepInt /= 64;

	numOfStepInt *= speed;
	numOfStepInt /= 256;

	movePtr->numOfStep = (int16)numOfStepInt;
	movePtr->timeOfChange = _engine->lbaTime;
}

void Movements::ChangedCursorKeys::update(TwinEEngine *engine) {
	if (engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
		leftChange = leftDown == 0;
		leftDown = 1;
	} else {
		leftChange = leftDown;
		leftDown = 0;
	}

	if (engine->_input->isActionActive(TwinEActionType::TurnRight)) {
		rightChange = rightDown == 0;
		rightDown = 1;
	} else {
		rightChange = rightDown;
		rightDown = 0;
	}

	if (engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
		backwardChange = backwardDown == 0;
		backwardDown = 1;
	} else {
		backwardChange = backwardDown;
		backwardDown = 0;
	}

	if (engine->_input->isActionActive(TwinEActionType::MoveForward)) {
		forwardChange = forwardDown == 0;
		forwardDown = 1;
	} else {
		forwardChange = forwardDown;
		forwardDown = 0;
	}
}

void Movements::update() {
	_previousChangedCursorKeys = _changedCursorKeys;
	_previousLoopActionKey = _heroActionKey;

	_heroActionKey = _engine->_input->isHeroActionActive();
	_changedCursorKeys.update(_engine);
}

bool Movements::processBehaviourExecution(int actorIdx) {
	bool executeAction = false;
	if (_engine->_input->toggleActionIfActive(TwinEActionType::SpecialAction)) {
		executeAction = true;
	}
	switch (_engine->_actor->heroBehaviour) {
	case HeroBehaviourType::kNormal:
		executeAction = true;
		break;
	case HeroBehaviourType::kAthletic:
		_engine->_animations->initAnim(AnimationTypes::kJump, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
		break;
	case HeroBehaviourType::kAggressive:
		if (_engine->_actor->autoAggressive) {
			ActorStruct *actor = _engine->_scene->getActor(actorIdx);
			heroMoved = true;
			actor->angle = actor->move.getRealAngle(_engine->lbaTime);
			// TODO: previousLoopActionKey must be handled properly
			if (!_previousLoopActionKey || actor->anim == AnimationTypes::kStanding) {
				const int32 aggresiveMode = _engine->getRandomNumber(3);

				switch (aggresiveMode) {
				case 0:
					_engine->_animations->initAnim(AnimationTypes::kKick, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
					break;
				case 1:
					_engine->_animations->initAnim(AnimationTypes::kRightPunch, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
					break;
				case 2:
					_engine->_animations->initAnim(AnimationTypes::kLeftPunch, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
					break;
				}
			}
		} else {
			if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
				_engine->_animations->initAnim(AnimationTypes::kLeftPunch, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
				heroMoved = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
				_engine->_animations->initAnim(AnimationTypes::kRightPunch, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
				heroMoved = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::MoveForward)) {
				_engine->_animations->initAnim(AnimationTypes::kKick, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
				heroMoved = true;
			}
		}
		break;
	case HeroBehaviourType::kDiscrete:
		_engine->_animations->initAnim(AnimationTypes::kHide, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
		break;
	case HeroBehaviourType::kProtoPack:
		break;
	}
	return executeAction;
}

bool Movements::processAttackExecution(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (!_engine->_gameState->usingSabre) {
		// Use Magic Ball
		if (_engine->_gameState->hasItem(InventoryItems::kiMagicBall)) {
			if (_engine->_gameState->magicBallIdx == -1) {
				_engine->_animations->initAnim(AnimationTypes::kThrowBall, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);
			}

			actor->angle = actor->move.getRealAngle(_engine->lbaTime);
			return true;
		}
	} else if (_engine->_gameState->hasItem(InventoryItems::kiUseSabre)) {
		if (actor->body != BodyType::btSabre) {
			_engine->_actor->initModelActor(BodyType::btSabre, actorIdx);
		}

		_engine->_animations->initAnim(AnimationTypes::kSabreAttack, AnimType::kAnimationType_1, AnimationTypes::kStanding, actorIdx);

		actor->angle = actor->move.getRealAngle(_engine->lbaTime);
		return true;
	}
	return false;
}

void Movements::processManualMovementExecution(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->isAttackAnimationActive()) {
		return;
	}
	if (actor->isJumpAnimationActive()) {
		return;
	}
	if (actor->isAttackWeaponAnimationActive()) {
		return;
	}
	if (!_changedCursorKeys || heroAction) {
		// if walking should get stopped
		if (!_engine->_input->isActionActive(TwinEActionType::MoveForward) && !_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			if (heroMoved && (_heroActionKey != _previousLoopActionKey || _changedCursorKeys != _previousChangedCursorKeys)) {
				_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			}
		}

		heroMoved = false;

		if (_engine->_input->isActionActive(TwinEActionType::MoveForward)) {
			if (!_engine->_scene->currentActorInZone) {
				_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			}
			heroMoved = true;
		} else if (_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			_engine->_animations->initAnim(AnimationTypes::kBackward, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			heroMoved = true;
		}

		if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
			if (actor->anim == AnimationTypes::kStanding) {
				_engine->_animations->initAnim(AnimationTypes::kTurnLeft, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			} else {
				if (!actor->dynamicFlags.bIsRotationByAnim) {
					actor->angle = actor->move.getRealAngle(_engine->lbaTime);
				}
			}
			heroMoved = true;
		} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
			if (actor->anim == AnimationTypes::kStanding) {
				_engine->_animations->initAnim(AnimationTypes::kTurnRight, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			} else {
				if (!actor->dynamicFlags.bIsRotationByAnim) {
					actor->angle = actor->move.getRealAngle(_engine->lbaTime);
				}
			}
			heroMoved = true;
		}
	}
}

void Movements::processManualRotationExecution(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (!_engine->_actor->autoAggressive && actor->isAttackAnimationActive()) {
		// it is allowed to rotate in auto aggressive mode - but not in manual mode.
		return;
	}
	if (actor->isJumpAnimationActive()) {
		return;
	}
	int16 tempAngle;
	if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
		tempAngle = ANGLE_90;
	} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
		tempAngle = -ANGLE_90;
	} else {
		tempAngle = ANGLE_0;
	}

	moveActor(actor->angle, actor->angle + tempAngle, actor->speed, &actor->move);
}

void Movements::processManualAction(int actorIdx) {
	if (IS_HERO(actorIdx)) {
		heroAction = false;
		if (_engine->_input->isHeroActionActive()) {
			heroAction = processBehaviourExecution(actorIdx);
		}
	}

	if (_engine->_input->isActionActive(TwinEActionType::ThrowMagicBall) && !_engine->_gameState->inventoryDisabled()) {
		if (processAttackExecution(actorIdx)) {
			heroMoved = true;
		}
	}

	processManualMovementExecution(actorIdx);
	processManualRotationExecution(actorIdx);
}

void Movements::processFollowAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const ActorStruct *followedActor = _engine->_scene->getActor(actor->followedActor);
	int32 newAngle = getAngleAndSetTargetActorDistance(actor->pos, followedActor->pos);
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

	if (actor->brickCausesDamage()) {
		moveActor(actor->angle, ClampAngle((_engine->getRandomNumber() & ANGLE_90) + (actor->angle - ANGLE_90)), actor->speed, &actor->move);
		actor->delayInMillis = _engine->getRandomNumber(300) + _engine->lbaTime + 300;
		_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
	}

	if (!actor->move.numOfStep) {
		_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
		if (_engine->lbaTime > actor->delayInMillis) {
			moveActor(actor->angle, ClampAngle((_engine->getRandomNumber() & ANGLE_90) + (actor->angle - ANGLE_90)), actor->speed, &actor->move);
			actor->delayInMillis = _engine->getRandomNumber(300) + _engine->lbaTime + 300;
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
	const ActorStruct *followedActor = _engine->_scene->getActor(actor->followedActor);
	actor->pos.x = followedActor->pos.x;
	actor->pos.z = followedActor->pos.z;
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

		int16 tempAngle = ANGLE_0;
		if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
			tempAngle = ANGLE_90;
		} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
			tempAngle = -ANGLE_90;
		}

		moveActor(actor->angle, actor->angle + tempAngle, actor->speed, &actor->move);
		return;
	}
	if (!actor->staticFlags.bIsSpriteActor) {
		if (actor->controlMode != ControlMode::kManual) {
			actor->angle = actor->move.getRealAngle(_engine->lbaTime);
		}
	}

	switch (actor->controlMode) {
	/**
	 * The Actor's Track Script is stopped. Track Script execution may be started with Life Script of
	 * the Actor or other Actors (with SET_TRACK(_OBJ) command). This mode does not mean the Actor
	 * will literally not move, but rather that it's Track Script (also called Move Script) is
	 * initially stopped. The Actor may move if it is assigned a moving animation.
	 */
	case ControlMode::kNoMove:
	case ControlMode::kFollow2:     // unused
	case ControlMode::kTrackAttack: // unused
		break;
	case ControlMode::kManual:
		processManualAction(actorIdx);
		break;
	case ControlMode::kFollow:
		processFollowAction(actorIdx);
		break;
	case ControlMode::kTrack:
		processTrackAction(actorIdx);
		break;
	case ControlMode::kSameXZ:
		processSameXZAction(actorIdx);
		break;
	case ControlMode::kRandom:
		processRandomAction(actorIdx);
		break;
	default:
		warning("Unknown control mode %d", (int)actor->controlMode);
		break;
	}
}

} // namespace TwinE
