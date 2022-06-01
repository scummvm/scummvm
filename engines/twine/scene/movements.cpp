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

void Movements::getShadowPosition(const IVec3 &pos) {
	const uint8 *ptr = _engine->_grid->getBlockBufferGround(pos, _processActor.y);
	_processActor.x = pos.x;
	_processActor.z = pos.z;

	ShapeType shadowCollisionType;
	const int32 blockIdx = *ptr;
	if (blockIdx) {
		const int32 brickIdx = *(ptr + 1);
		const BlockDataEntry *blockPtr = _engine->_grid->getBlockPointer(blockIdx, brickIdx);
		shadowCollisionType = (ShapeType)blockPtr->brickShape;
	} else {
		shadowCollisionType = ShapeType::kNone;
	}
	_engine->_collision->reajustActorPosition(shadowCollisionType);

	_engine->_actor->_shadowCoord = _processActor;
}

void Movements::setActorAngleSafe(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr) {
	movePtr->from = ClampAngle(startAngle);
	movePtr->to = ClampAngle(endAngle);
	movePtr->numOfStep = ClampAngle(stepAngle);
	movePtr->timeOfChange = _engine->_lbaTime;
}

void Movements::clearRealAngle(ActorStruct *actorPtr) {
	setActorAngleSafe(actorPtr->_angle, actorPtr->_angle, ANGLE_0, &actorPtr->_move);
}

void Movements::setActorAngle(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr) {
	movePtr->from = startAngle;
	movePtr->to = endAngle;
	movePtr->numOfStep = stepAngle;
	movePtr->timeOfChange = _engine->_lbaTime;
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

	_targetActorDistance = (int32)sqrt((float)(newX + newZ));

	if (!_targetActorDistance) {
		return 0;
	}

	const int32 destAngle = (difZ * SCENE_SIZE_HALF) / _targetActorDistance;

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

IVec3 Movements::rotateActor(int32 x, int32 z, int32 angle) {
	const double radians = AngleToRadians(angle);
	const int32 vx = (int32)(x * cos(radians) + z * sin(radians));
	const int32 vz = (int32)(-x * sin(radians) + z * cos(radians));
	return IVec3(vx, 0, vz);
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
	movePtr->timeOfChange = _engine->_lbaTime;
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
	switch (_engine->_actor->_heroBehaviour) {
	case HeroBehaviourType::kNormal:
		executeAction = true;
		break;
	case HeroBehaviourType::kAthletic:
		_engine->_animations->initAnim(AnimationTypes::kJump, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
		break;
	case HeroBehaviourType::kAggressive:
		if (_engine->_actor->_autoAggressive) {
			ActorStruct *actor = _engine->_scene->getActor(actorIdx);
			_lastJoyFlag = true;
			actor->_angle = actor->_move.getRealAngle(_engine->_lbaTime);
			// TODO: previousLoopActionKey must be handled properly
			if (!_previousLoopActionKey || actor->_anim == AnimationTypes::kStanding) {
				const int32 aggresiveMode = _engine->getRandomNumber(3);

				switch (aggresiveMode) {
				case 0:
					_engine->_animations->initAnim(AnimationTypes::kKick, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
					break;
				case 1:
					_engine->_animations->initAnim(AnimationTypes::kRightPunch, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
					break;
				case 2:
					_engine->_animations->initAnim(AnimationTypes::kLeftPunch, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
					break;
				}
			}
		} else {
			if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
				_engine->_animations->initAnim(AnimationTypes::kLeftPunch, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
				_lastJoyFlag = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
				_engine->_animations->initAnim(AnimationTypes::kRightPunch, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
				_lastJoyFlag = true;
			} else if (_engine->_input->isActionActive(TwinEActionType::MoveForward)) {
				_engine->_animations->initAnim(AnimationTypes::kKick, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
				_lastJoyFlag = true;
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
	if (!_engine->_gameState->_usingSabre) {
		// Use Magic Ball
		if (_engine->_gameState->hasItem(InventoryItems::kiMagicBall)) {
			if (_engine->_gameState->_magicBallIdx == -1) {
				_engine->_animations->initAnim(AnimationTypes::kThrowBall, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
			}

			actor->_angle = actor->_move.getRealAngle(_engine->_lbaTime);
			return true;
		}
	} else if (_engine->_gameState->hasItem(InventoryItems::kiUseSabre)) {
		if (actor->_genBody != BodyType::btSabre) {
			_engine->_actor->initModelActor(BodyType::btSabre, actorIdx);
		}

		_engine->_animations->initAnim(AnimationTypes::kSabreAttack, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);

		actor->_angle = actor->_move.getRealAngle(_engine->_lbaTime);
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
	if (!_changedCursorKeys || _heroAction) {
		// if walking should get stopped
		if (!_engine->_input->isActionActive(TwinEActionType::MoveForward) && !_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			if (_lastJoyFlag && (_heroActionKey != _previousLoopActionKey || _changedCursorKeys != _previousChangedCursorKeys)) {
				_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			}
		}

		_lastJoyFlag = false;

		if (_engine->_input->isActionActive(TwinEActionType::MoveForward)) {
			if (!_engine->_scene->_currentActorInZone) {
				_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			}
			_lastJoyFlag = true;
		} else if (_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			_engine->_animations->initAnim(AnimationTypes::kBackward, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			_lastJoyFlag = true;
		}

		if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
			if (actor->_anim == AnimationTypes::kStanding) {
				_engine->_animations->initAnim(AnimationTypes::kTurnLeft, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			} else {
				if (!actor->_dynamicFlags.bIsRotationByAnim) {
					actor->_angle = actor->_move.getRealAngle(_engine->_lbaTime);
				}
			}
			_lastJoyFlag = true;
		} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
			if (actor->_anim == AnimationTypes::kStanding) {
				_engine->_animations->initAnim(AnimationTypes::kTurnRight, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
			} else {
				if (!actor->_dynamicFlags.bIsRotationByAnim) {
					actor->_angle = actor->_move.getRealAngle(_engine->_lbaTime);
				}
			}
			_lastJoyFlag = true;
		}
	}
}

void Movements::processManualRotationExecution(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (!_engine->_actor->_autoAggressive && actor->isAttackAnimationActive()) {
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

	moveActor(actor->_angle, actor->_angle + tempAngle, actor->_speed, &actor->_move);
}

void Movements::processManualAction(int actorIdx) {
	if (IS_HERO(actorIdx)) {
		_heroAction = false;
		if (_engine->_input->isHeroActionActive()) {
			_heroAction = processBehaviourExecution(actorIdx);
		}
	}

	if (_engine->_input->isActionActive(TwinEActionType::ThrowMagicBall) && !_engine->_gameState->inventoryDisabled()) {
		if (processAttackExecution(actorIdx)) {
			_lastJoyFlag = true;
		}
	}

	processManualMovementExecution(actorIdx);
	processManualRotationExecution(actorIdx);
}

void Movements::processFollowAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const ActorStruct *followedActor = _engine->_scene->getActor(actor->_followedActor);
	int32 newAngle = getAngleAndSetTargetActorDistance(actor->pos(), followedActor->pos());
	if (actor->_staticFlags.bIsSpriteActor) {
		actor->_angle = newAngle;
	} else {
		moveActor(actor->_angle, newAngle, actor->_speed, &actor->_move);
	}
}

void Movements::processRandomAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_dynamicFlags.bIsRotationByAnim) {
		return;
	}

	if (actor->brickCausesDamage()) {
		const int32 angle = ClampAngle(actor->_angle + (_engine->getRandomNumber() & (ANGLE_180 - 1)) - ANGLE_90 + ANGLE_180);
		moveActor(actor->_angle, angle, actor->_speed, &actor->_move);
		actor->_delayInMillis = _engine->getRandomNumber(300) + _engine->_lbaTime + 300;
		_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
	}

	if (!actor->_move.numOfStep) {
		_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
		if (_engine->_lbaTime > actor->_delayInMillis) {
			const int32 angle = ClampAngle(actor->_angle + (_engine->getRandomNumber() & (ANGLE_180 - 1)) - ANGLE_90);
			moveActor(actor->_angle, angle, actor->_speed, &actor->_move);
			actor->_delayInMillis = _engine->getRandomNumber(300) + _engine->_lbaTime + 300;
		}
	}
}

void Movements::processTrackAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_positionInMoveScript == -1) {
		actor->_positionInMoveScript = 0;
	}
}

void Movements::processSameXZAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const ActorStruct *followedActor = _engine->_scene->getActor(actor->_followedActor);
	actor->_pos.x = followedActor->_pos.x;
	actor->_pos.z = followedActor->_pos.z;
}

void Movements::processActorMovements(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_entity == -1) {
		return;
	}

	if (actor->_dynamicFlags.bIsFalling) {
		if (actor->_controlMode != ControlMode::kManual) {
			return;
		}

		int16 tempAngle = ANGLE_0;
		if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
			tempAngle = ANGLE_90;
		} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
			tempAngle = -ANGLE_90;
		}

		moveActor(actor->_angle, actor->_angle + tempAngle, actor->_speed, &actor->_move);
		return;
	}
	if (!actor->_staticFlags.bIsSpriteActor) {
		if (actor->_controlMode != ControlMode::kManual) {
			actor->_angle = actor->_move.getRealAngle(_engine->_lbaTime);
		}
	}

	switch (actor->_controlMode) {
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
		// TODO: see lSET_DIRMODE and lSET_DIRMODE_OBJ opcodes
		processSameXZAction(actorIdx);
		break;
	case ControlMode::kRandom:
		processRandomAction(actorIdx);
		break;
	default:
		warning("Unknown control mode %d", (int)actor->_controlMode);
		break;
	}
}

} // namespace TwinE
