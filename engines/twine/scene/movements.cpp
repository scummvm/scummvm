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

IVec3 Movements::getShadow(const IVec3 &pos) { // GetShadow
	IVec3 shadowCoord;
	const uint8 *ptr = _engine->_grid->getBlockBufferGround(pos, shadowCoord.y);
	shadowCoord.x = pos.x;
	shadowCoord.z = pos.z;

	ShapeType shadowCollisionType;
	const int32 blockIdx = *ptr;
	if (blockIdx) {
		const int32 brickIdx = *(ptr + 1);
		const BlockDataEntry *blockPtr = _engine->_grid->getAdrBlock(blockIdx, brickIdx);
		shadowCollisionType = (ShapeType)blockPtr->brickShape;
	} else {
		shadowCollisionType = ShapeType::kNone;
	}
	_engine->_collision->reajustPos(shadowCoord, shadowCollisionType);
	return shadowCoord;
}

void Movements::initRealAngle(int16 startAngle, int16 endAngle, int16 stepAngle, RealValue *movePtr) {
	movePtr->startValue = ClampAngle(startAngle);
	movePtr->endValue = ClampAngle(endAngle);
	movePtr->timeValue = ClampAngle(stepAngle);
	movePtr->memoTicks = _engine->timerRef;
}

void Movements::clearRealAngle(ActorStruct *actorPtr) {
	initRealAngle(actorPtr->_beta, actorPtr->_beta, LBAAngles::ANGLE_0, &actorPtr->realAngle);
}

void Movements::initRealValue(int16 startAngle, int16 endAngle, int16 stepAngle, RealValue *movePtr) {
	movePtr->startValue = startAngle;
	movePtr->endValue = endAngle;
	movePtr->timeValue = stepAngle;
	movePtr->memoTicks = _engine->timerRef;
}

int32 Movements::getAngle(int32 x0, int32 z0, int32 x1, int32 z1) {
#if 1
	int32 difZ = z1 - z0;
	const int32 newZ = difZ * difZ;

	int32 difX = x1 - x0;
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

	int32 startAngle = LBAAngles::ANGLE_0;
	//	stopAngle  = LBAAngles::ANGLE_90;
	const int16 *shadeAngleTab3(&sinTab[LBAAngles::ANGLE_135]);
	while (shadeAngleTab3[startAngle] > destAngle) {
		startAngle++;
	}

	if (shadeAngleTab3[startAngle] != destAngle) {
		if ((shadeAngleTab3[startAngle - 1] + shadeAngleTab3[startAngle]) / 2 <= destAngle) {
			startAngle--;
		}
	}

	int32 finalAngle = LBAAngles::ANGLE_45 + startAngle;

	if (difX <= 0) {
		finalAngle = -finalAngle;
	}

	if (flag) {
		finalAngle = -finalAngle + LBAAngles::ANGLE_90;
	}

	return ClampAngle(finalAngle);
#else
	z1 -= z0;
	x1 -= x0;
	const int32 x2 = x1 * x1;
	const int32 z2 = z1 * z1;

	_targetActorDistance = (int32)sqrt((float)(x2 + z2));
	if (!_targetActorDistance) {
		return 0;
	}

	if (z2 > x2) {
		const int32 tmpEx = z1;
		x1 = z1 | 1; // flag = 1
		z1 = tmpEx;
	} else {
		x1 &= -2; // flag = 0
	}

	const int32 tmp = (z1 * SCENE_SIZE_HALF) / _targetActorDistance;

	int32 start = LBAAngles::ANGLE_135;
	int32 end = LBAAngles::ANGLE_135 + LBAAngles::ANGLE_90;
	int32 diff = 0;

	while (start < (end - 1)) {
		int32 angle = (start + end) >> 1;
		diff = tmp - sinTab[angle];
		if (diff > 0) {
			end = angle;
		} else {
			start = angle;
			if (diff == 0) {
				break;
			}
		}
	}
	if (diff) {
		if (tmp <= ((sinTab[start] + sinTab[end]) >> 1)) {
			start = end;
		}
	}

	int32 angle = start - LBAAngles::ANGLE_90;
	if (x1 < 0) {
		angle = -angle;
	}

	if (x1 & 1) {
		angle = LBAAngles::ANGLE_90 - angle;
	}

	return ClampAngle(angle);
#endif
}

void Movements::initRealAngleConst(int32 start, int32 end, int32 duration, RealValue *movePtr) const { // ManualRealAngle
	const int16 cstart = ClampAngle(start);
	const int16 cend = ClampAngle(end);

	movePtr->startValue = cstart;
	movePtr->endValue = cend;

	const int16 numOfStep = (cstart - cend) * 64;
	int32 t = ABS(numOfStep);
	t /= 64;

	t *= duration;
	t /= 256;

	movePtr->timeValue = (int16)t;
	movePtr->memoTicks = _engine->timerRef;
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
		if (_engine->_actor->_combatAuto) {
			ActorStruct *actor = _engine->_scene->getActor(actorIdx);
			_lastJoyFlag = true;
			actor->_beta = actor->realAngle.getRealAngle(_engine->timerRef);
			// TODO: previousLoopActionKey must be handled properly
			if (!_previousLoopActionKey || actor->_genAnim == AnimationTypes::kStanding) {
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
		_engine->_animations->initAnim(AnimationTypes::kHide, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
		break;
	case HeroBehaviourType::kProtoPack:
	case HeroBehaviourType::kMax:
		break;
	}
	return executeAction;
}

bool Movements::processAttackExecution(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (!_engine->_gameState->_usingSabre) {
		// Use Magic Ball
		if (_engine->_gameState->hasItem(InventoryItems::kiMagicBall)) {
			if (_engine->_gameState->_magicBall == -1) {
				_engine->_animations->initAnim(AnimationTypes::kThrowBall, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);
			}

			actor->_beta = actor->realAngle.getRealAngle(_engine->timerRef);
			return true;
		}
	} else if (_engine->_gameState->hasItem(InventoryItems::kiUseSabre)) {
		if (actor->_genBody != BodyType::btSabre) {
			_engine->_actor->initBody(BodyType::btSabre, actorIdx);
		}

		_engine->_animations->initAnim(AnimationTypes::kSabreAttack, AnimType::kAnimationThen, AnimationTypes::kStanding, actorIdx);

		actor->_beta = actor->realAngle.getRealAngle(_engine->timerRef);
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
	if (!_changedCursorKeys || _actionNormal) {
		// if walking should get stopped
		if (!_engine->_input->isActionActive(TwinEActionType::MoveForward) && !_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			if (_lastJoyFlag && (_heroActionKey != _previousLoopActionKey || _changedCursorKeys != _previousChangedCursorKeys)) {
				_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
			}
		}

		_lastJoyFlag = false;

		if (_engine->_input->isActionActive(TwinEActionType::MoveForward)) {
			if (!_engine->_scene->_flagClimbing) {
				_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
			}
			_lastJoyFlag = true;
		} else if (_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
			_engine->_animations->initAnim(AnimationTypes::kBackward, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
			_lastJoyFlag = true;
		}

		if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
			if (actor->_genAnim == AnimationTypes::kStanding) {
				_engine->_animations->initAnim(AnimationTypes::kTurnLeft, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
			} else {
				if (!actor->_workFlags.bIsRotationByAnim) {
					actor->_beta = actor->realAngle.getRealAngle(_engine->timerRef);
				}
			}
			_lastJoyFlag = true;
		} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
			if (actor->_genAnim == AnimationTypes::kStanding) {
				_engine->_animations->initAnim(AnimationTypes::kTurnRight, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
			} else {
				if (!actor->_workFlags.bIsRotationByAnim) {
					actor->_beta = actor->realAngle.getRealAngle(_engine->timerRef);
				}
			}
			_lastJoyFlag = true;
		}
	}
}

void Movements::processManualRotationExecution(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (!_engine->_actor->_combatAuto && actor->isAttackAnimationActive()) {
		// it is allowed to rotate in auto aggressive mode - but not in manual mode.
		return;
	}
	if (actor->isJumpAnimationActive()) {
		return;
	}
	int16 tempAngle;
	if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
		tempAngle = LBAAngles::ANGLE_90;
	} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
		tempAngle = -LBAAngles::ANGLE_90;
	} else {
		tempAngle = LBAAngles::ANGLE_0;
	}

	initRealAngleConst(actor->_beta, actor->_beta + tempAngle, actor->_srot, &actor->realAngle);
}

void Movements::processManualAction(int actorIdx) {
	if (IS_HERO(actorIdx)) {
		_actionNormal = false;
		if (_engine->_input->isHeroActionActive()) {
			_actionNormal = processBehaviourExecution(actorIdx);
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
	int32 newAngle = getAngle(actor->posObj(), followedActor->posObj());
	if (actor->_flags.bSprite3D) {
		actor->_beta = newAngle;
	} else {
		initRealAngleConst(actor->_beta, newAngle, actor->_srot, &actor->realAngle);
	}
}

void Movements::processRandomAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_workFlags.bIsRotationByAnim) {
		return;
	}

	if (actor->brickCausesDamage()) {
		const int32 angle = ClampAngle(actor->_beta + (_engine->getRandomNumber() & (LBAAngles::ANGLE_180 - 1)) - LBAAngles::ANGLE_90 + LBAAngles::ANGLE_180);
		initRealAngleConst(actor->_beta, angle, actor->_srot, &actor->realAngle);
		actor->_delayInMillis = _engine->timerRef + _engine->getRandomNumber(_engine->toSeconds(6)) + _engine->toSeconds(6);
		_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
	}

	if (!actor->realAngle.timeValue) {
		_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
		if (_engine->timerRef > actor->_delayInMillis) {
			const int32 angle = ClampAngle(actor->_beta + (_engine->getRandomNumber() & (LBAAngles::ANGLE_180 - 1)) - LBAAngles::ANGLE_90);
			initRealAngleConst(actor->_beta, angle, actor->_srot, &actor->realAngle);
			actor->_delayInMillis = _engine->timerRef + _engine->getRandomNumber(_engine->toSeconds(6)) + _engine->toSeconds(6);
		}
	}
}

void Movements::processTrackAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_offsetTrack == -1) {
		actor->_offsetTrack = 0;
	}
}

void Movements::processSameXZAction(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const ActorStruct *followedActor = _engine->_scene->getActor(actor->_followedActor);
	actor->_posObj.x = followedActor->_posObj.x;
	actor->_posObj.z = followedActor->_posObj.z;
}

void Movements::manualRealAngle(ActorStruct *actor) {
	int16 tempAngle = LBAAngles::ANGLE_0;
	if (_engine->_input->isActionActive(TwinEActionType::TurnLeft)) {
		tempAngle = LBAAngles::ANGLE_90;
	} else if (_engine->_input->isActionActive(TwinEActionType::TurnRight)) {
		tempAngle = -LBAAngles::ANGLE_90;
	}

	initRealAngleConst(actor->_beta, actor->_beta + tempAngle, actor->_srot, &actor->realAngle);
}

void Movements::doDir(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_body == -1) {
		return;
	}

	if (actor->_workFlags.bIsFalling) {
		if (actor->_controlMode == ControlMode::kManual) {
			manualRealAngle(actor);
			// TODO: _lastJoyFlag = _joyFlag;
		}
		return;
	}
	if (!actor->_flags.bSprite3D && actor->_controlMode != ControlMode::kManual) {
		actor->_beta = actor->realAngle.getRealAngle(_engine->timerRef);
	}

	switch (actor->_controlMode) {
	case ControlMode::kManual:
		processManualAction(actorIdx);
		break;
	case ControlMode::kFollow:
		processFollowAction(actorIdx);
		break;
	case ControlMode::kRandom:
		processRandomAction(actorIdx);
		break;
	case ControlMode::kTrack:
		processTrackAction(actorIdx);
		break;
	case ControlMode::kSameXZ:
		// TODO: see lSET_DIRMODE and lSET_DIRMODE_OBJ opcodes
		processSameXZAction(actorIdx);
		break;
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
	default:
		warning("Unknown control mode %d", (int)actor->_controlMode);
		break;
	}
}

} // namespace TwinE
