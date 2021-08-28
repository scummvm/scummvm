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

#include "twine/scene/animations.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/util.h"
#include "twine/audio/sound.h"
#include "twine/parser/anim.h"
#include "twine/parser/entity.h"
#include "twine/renderer/renderer.h"
#include "twine/resources/resources.h"
#include "twine/scene/collision.h"
#include "twine/scene/extra.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"

namespace TwinE {

static const int32 magicLevelStrengthOfHit[] = {
	MagicballStrengthType::kNoBallStrength,
	MagicballStrengthType::kYellowBallStrength,
	MagicballStrengthType::kGreenBallStrength,
	MagicballStrengthType::kRedBallStrength,
	MagicballStrengthType::kFireBallStrength,
	0};

Animations::Animations(TwinEEngine *engine) : _engine(engine) {
}

int32 Animations::getBodyAnimIndex(AnimationTypes animIdx, int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const int32 bodyAnimIndex = actor->_entityDataPtr->getAnimIndex(animIdx);
	if (bodyAnimIndex != -1) {
		_currentActorAnimExtraPtr = animIdx;
	}
	return bodyAnimIndex;
}

int16 Animations::applyAnimStepRotation(int32 deltaTime, int32 keyFrameLength, int16 newAngle1, int16 lastAngle1) const {
	const int16 lastAngle = ClampAngle(lastAngle1);
	const int16 newAngle = ClampAngle(newAngle1);

	int16 angleDiff = newAngle - lastAngle;

	int16 computedAngle;
	if (angleDiff) {
		if (angleDiff < -ANGLE_180) {
			angleDiff += ANGLE_360;
		} else if (angleDiff > ANGLE_180) {
			angleDiff -= ANGLE_360;
		}

		computedAngle = lastAngle + (angleDiff * deltaTime) / keyFrameLength;
	} else {
		computedAngle = lastAngle;
	}

	return ClampAngle(computedAngle);
}

int16 Animations::applyAnimStepTranslation(int32 deltaTime, int32 keyFrameLength, int16 newPos, int16 lastPos) const {
	int16 distance = newPos - lastPos;

	int16 computedPos;
	if (distance) {
		computedPos = lastPos + (distance * deltaTime) / keyFrameLength;
	} else {
		computedPos = lastPos;
	}

	return computedPos;
}

bool Animations::setModelAnimation(int32 keyframeIdx, const AnimData &animData, BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr) {
	if (!bodyData.isAnimated()) {
		return false;
	}
	const KeyFrame *keyFrame = animData.getKeyframe(keyframeIdx);

	_currentStep.x = keyFrame->x;
	_currentStep.y = keyFrame->y;
	_currentStep.z = keyFrame->z;

	_processRotationByAnim = keyFrame->boneframes[0].type;
	_processLastRotationAngle = ToAngle(keyFrame->boneframes[0].y);

	const int16 numBones = bodyData.getNumBones();

	int32 numOfBonesInAnim = animData.getNumBoneframes();
	if (numOfBonesInAnim > numBones) {
		numOfBonesInAnim = numBones;
	}
	const int32 keyFrameLength = keyFrame->length;

	const KeyFrame *lastKeyFramePtr = animTimerDataPtr->ptr;
	int32 remainingFrameTime = animTimerDataPtr->time;
	if (lastKeyFramePtr == nullptr) {
		lastKeyFramePtr = keyFrame;
		remainingFrameTime = keyFrameLength;
	}
	const int32 deltaTime = _engine->_lbaTime - remainingFrameTime;
	if (deltaTime >= keyFrameLength) {
		copyKeyFrameToState(keyFrame, bodyData, numOfBonesInAnim);
		animTimerDataPtr->ptr = keyFrame;
		animTimerDataPtr->time = _engine->_lbaTime;
		return true;
	}

	_processLastRotationAngle = (_processLastRotationAngle * deltaTime) / keyFrameLength;

	if (numOfBonesInAnim <= 1) {
		return false;
	}

	int16 boneIdx = 1;
	int16 tmpNumOfPoints = MIN<int16>(lastKeyFramePtr->boneframes.size() - 1, numOfBonesInAnim - 1);
	do {
		BoneFrame *boneState = bodyData.getBoneState(boneIdx);
		const BoneFrame &boneFrame = keyFrame->boneframes[boneIdx];
		const BoneFrame &lastBoneFrame = lastKeyFramePtr->boneframes[boneIdx];

		boneState->type = boneFrame.type;
		switch (boneFrame.type) {
		case 0:
			boneState->x = applyAnimStepRotation(deltaTime, keyFrameLength, boneFrame.x, lastBoneFrame.x);
			boneState->y = applyAnimStepRotation(deltaTime, keyFrameLength, boneFrame.y, lastBoneFrame.y);
			boneState->z = applyAnimStepRotation(deltaTime, keyFrameLength, boneFrame.z, lastBoneFrame.z);
			break;
		case 1:
		case 2:
			boneState->x = applyAnimStepTranslation(deltaTime, keyFrameLength, boneFrame.x, lastBoneFrame.x);
			boneState->y = applyAnimStepTranslation(deltaTime, keyFrameLength, boneFrame.y, lastBoneFrame.y);
			boneState->z = applyAnimStepTranslation(deltaTime, keyFrameLength, boneFrame.z, lastBoneFrame.z);
			break;
		default:
			error("Unsupported animation rotation mode %d", boneFrame.type);
		}

		++boneIdx;
	} while (--tmpNumOfPoints);

	return false;
}

void Animations::setAnimAtKeyframe(int32 keyframeIdx, const AnimData &animData, BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr) {
	if (!bodyData.isAnimated()) {
		return;
	}

	const int32 numOfKeyframeInAnim = animData.getKeyframes().size();
	if (keyframeIdx < 0 || keyframeIdx >= numOfKeyframeInAnim) {
		return;
	}

	const KeyFrame *keyFrame = animData.getKeyframe(keyframeIdx);

	_currentStep.x = keyFrame->x;
	_currentStep.y = keyFrame->y;
	_currentStep.z = keyFrame->z;

	_processRotationByAnim = keyFrame->boneframes[0].type;
	_processLastRotationAngle = ToAngle(keyFrame->boneframes[0].y);

	animTimerDataPtr->ptr = animData.getKeyframe(keyframeIdx);
	animTimerDataPtr->time = _engine->_lbaTime;

	const int16 numBones = bodyData.getNumBones();

	int16 numOfBonesInAnim = animData.getNumBoneframes();
	if (numOfBonesInAnim > numBones) {
		numOfBonesInAnim = numBones;
	}

	copyKeyFrameToState(keyFrame, bodyData, numOfBonesInAnim);
}

void Animations::stockAnimation(const BodyData &bodyData, AnimTimerDataStruct *animTimerDataPtr) {
	if (!bodyData.isAnimated()) {
		return;
	}

	if (_animKeyframeBufIdx >= ARRAYSIZE(_animKeyframeBuf)) {
		_animKeyframeBufIdx = 0;
	}
	animTimerDataPtr->time = _engine->_lbaTime;
	KeyFrame *keyframe = &_animKeyframeBuf[_animKeyframeBufIdx++];
	animTimerDataPtr->ptr = keyframe;
	copyStateToKeyFrame(keyframe, bodyData);
}

void Animations::copyStateToKeyFrame(KeyFrame *keyframe, const BodyData &bodyData) const {
	const int32 numBones = bodyData.getNumBones();
	keyframe->boneframes.clear();
	keyframe->boneframes.reserve(numBones);
	for (int32 i = 0; i < numBones; ++i) {
		const BoneFrame *boneState = bodyData.getBoneState(i);
		keyframe->boneframes.push_back(*boneState);
	}
}

void Animations::copyKeyFrameToState(const KeyFrame *keyframe, BodyData &bodyData, int32 numBones) const {
	for (int32 i = 0; i < numBones; ++i) {
		BoneFrame *boneState = bodyData.getBoneState(i);
		*boneState = keyframe->boneframes[i];
	}
}

bool Animations::verifyAnimAtKeyframe(int32 keyframeIdx, const AnimData &animData, AnimTimerDataStruct *animTimerDataPtr) {
	const KeyFrame *keyFrame = animData.getKeyframe(keyframeIdx);
	const int32 keyFrameLength = keyFrame->length;

	int32 remainingFrameTime = animTimerDataPtr->time;
	if (animTimerDataPtr->ptr == nullptr) {
		remainingFrameTime = keyFrameLength;
	}

	const int32 deltaTime = _engine->_lbaTime - remainingFrameTime;

	_currentStep.x = keyFrame->x;
	_currentStep.y = keyFrame->y;
	_currentStep.z = keyFrame->z;

	const BoneFrame &boneFrame = keyFrame->boneframes[0];
	_processRotationByAnim = boneFrame.type;
	_processLastRotationAngle = ToAngle(boneFrame.y);

	if (deltaTime >= keyFrameLength) {
		animTimerDataPtr->ptr = animData.getKeyframe(keyframeIdx);
		animTimerDataPtr->time = _engine->_lbaTime;
		return true;
	}

	_processLastRotationAngle = (_processLastRotationAngle * deltaTime) / keyFrameLength;
	_currentStep.x = (_currentStep.x * deltaTime) / keyFrameLength;
	_currentStep.y = (_currentStep.y * deltaTime) / keyFrameLength;
	_currentStep.z = (_currentStep.z * deltaTime) / keyFrameLength;

	return false;
}

void Animations::processAnimActions(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_entityDataPtr == nullptr || actor->_animExtraPtr == AnimationTypes::kAnimNone) {
		return;
	}

	const Common::Array<EntityAnim::Action> *actions = actor->_entityDataPtr->getActions(actor->_animExtraPtr);
	if (actions == nullptr) {
		return;
	}
	for (const EntityAnim::Action &action : *actions) {
		switch (action.type) {
		case ActionType::ACTION_HITTING:
			if (action.animFrame - 1 == actor->_animPosition) {
				actor->_strengthOfHit = action.strength;
				actor->_dynamicFlags.bIsHitting = 1;
			}
			break;
		case ActionType::ACTION_SAMPLE:
		case ActionType::ACTION_SAMPLE_FREQ:
			if (action.animFrame == actor->_animPosition) {
				_engine->_sound->playSample(action.sampleIndex, 1, actor->pos(), actorIdx);
			}
			break;
		case ActionType::ACTION_THROW_EXTRA_BONUS:
			if (action.animFrame == actor->_animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->_pos.x, actor->_pos.y + action.yHeight, actor->_pos.z, action.spriteIndex, action.xAngle, action.yAngle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_MAGIC_BALL:
			if (_engine->_gameState->_magicBallIdx == -1 && action.animFrame == actor->_animPosition) {
				_engine->_extra->addExtraThrowMagicball(actor->_pos.x, actor->_pos.y + action.yHeight, actor->_pos.z, action.xAngle, actor->_angle + action.yAngle, action.xRotPoint, action.extraAngle);
			}
			break;
		case ActionType::ACTION_SAMPLE_REPEAT:
			if (action.animFrame == actor->_animPosition) {
				_engine->_sound->playSample(action.sampleIndex, action.repeat, actor->pos(), actorIdx);
			}
			break;
		case ActionType::ACTION_THROW_SEARCH:
			if (action.animFrame == actor->_animPosition) {
				_engine->_extra->addExtraAiming(actorIdx, actor->_pos.x, actor->_pos.y + action.yHeight, actor->_pos.z, action.spriteIndex, action.targetActor, action.finalAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_ALPHA:
			if (action.animFrame == actor->_animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->_pos.x, actor->_pos.y + action.yHeight, actor->_pos.z, action.spriteIndex, action.xAngle, actor->_angle + action.yAngle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_SAMPLE_STOP:
			if (action.animFrame == actor->_animPosition) {
				_engine->_sound->stopSample(action.sampleIndex);
			}
			break;
		case ActionType::ACTION_LEFT_STEP:
		case ActionType::ACTION_RIGHT_STEP:
			if (action.animFrame == actor->_animPosition && (actor->_brickSound & 0xF0U) != 0xF0U) {
				const int16 sampleIdx = (actor->_brickSound & 0x0FU) + Samples::WalkFloorBegin;
				_engine->_sound->playSample(sampleIdx, 1, actor->pos(), actorIdx);
			}
			break;
		case ActionType::ACTION_HERO_HITTING:
			if (action.animFrame - 1 == actor->_animPosition) {
				actor->_strengthOfHit = magicLevelStrengthOfHit[_engine->_gameState->_magicLevelIdx];
				actor->_dynamicFlags.bIsHitting = 1;
			}
			break;
		case ActionType::ACTION_THROW_3D:
			if (action.animFrame == actor->_animPosition) {
				const IVec3 &destPos = _engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->_angle);

				const int32 throwX = destPos.x + actor->_pos.x;
				const int32 throwY = action.distanceY + actor->_pos.y;
				const int32 throwZ = destPos.z + actor->_pos.z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, action.spriteIndex,
				                               action.xAngle, action.yAngle + actor->_angle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_3D_ALPHA:
			if (action.animFrame == actor->_animPosition) {
				const int32 distance = getDistance2D(actor->pos(), _engine->_scene->_sceneHero->pos());
				const int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->_pos.y, 0, _engine->_scene->_sceneHero->_pos.y, distance);

				const IVec3 &destPos = _engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->_angle);

				const int32 throwX = destPos.x + actor->_pos.x;
				const int32 throwY = action.distanceY + actor->_pos.y;
				const int32 throwZ = destPos.z + actor->_pos.z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, action.spriteIndex,
				                               action.xAngle + newAngle, action.yAngle + actor->_angle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_3D_SEARCH:
			if (action.animFrame == actor->_animPosition) {
				const IVec3 &destPos = _engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->_angle);
				const int32 x = actor->_pos.x + destPos.x;
				const int32 y = actor->_pos.y + action.distanceY;
				const int32 z = actor->_pos.z + destPos.z;
				_engine->_extra->addExtraAiming(actorIdx, x, y, z, action.spriteIndex,
				                                action.targetActor, action.finalAngle, action.strength);
			}
			break;
		case ActionType::ACTION_UNKNOWN_21:
			if (_engine->_gameState->_magicBallIdx == -1 && action.animFrame == actor->_animPosition) {
				const IVec3 &destPos = _engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->_angle);
				const int32 x = actor->_pos.x + destPos.x;
				const int32 y = actor->_pos.y + action.distanceY;
				const int32 z = actor->_pos.z + destPos.z;
				_engine->_extra->addExtraThrowMagicball(x, y, z, action.xAngle, actor->_angle, action.yAngle, action.finalAngle);
			}
			break;
		case ActionType::ACTION_ZV:
		default:
			break;
		}
	}
}

bool Animations::initAnim(AnimationTypes newAnim, AnimType animType, AnimationTypes animExtra, int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_entity == -1) {
		return false;
	}

	if (actor->_staticFlags.bIsSpriteActor) {
		return false;
	}

	if (newAnim == actor->_anim && actor->_previousAnimIdx != -1) {
		return true;
	}

	if (animExtra == AnimationTypes::kAnimInvalid && actor->_animType != AnimType::kAnimationType_2) {
		animExtra = actor->_anim;
	}

	int32 animIndex = getBodyAnimIndex(newAnim, actorIdx);

	if (animIndex == -1) {
		animIndex = getBodyAnimIndex(AnimationTypes::kStanding, actorIdx);
	}

	if (animType != AnimType::kAnimationType_4 && actor->_animType == AnimType::kAnimationType_2) {
		actor->_animExtra = newAnim;
		return false;
	}

	if (animType == AnimType::kAnimationType_3) {
		animType = AnimType::kAnimationType_2;

		animExtra = actor->_anim;

		if (animExtra == AnimationTypes::kThrowBall || animExtra == AnimationTypes::kFall || animExtra == AnimationTypes::kLanding || animExtra == AnimationTypes::kLandingHit) {
			animExtra = AnimationTypes::kStanding;
		}
	}

	if (animType == AnimType::kAnimationType_4) {
		animType = AnimType::kAnimationType_2;
	}

	if (actor->_previousAnimIdx == -1) {
		// if no previous animation
		setAnimAtKeyframe(0, _engine->_resources->_animData[animIndex], _engine->_resources->_bodyData[actor->_entity], &actor->_animTimerData);
	} else {
		// interpolation between animations
		stockAnimation(_engine->_resources->_bodyData[actor->_entity], &actor->_animTimerData);
	}

	actor->_previousAnimIdx = animIndex;
	actor->_anim = newAnim;
	actor->_animExtra = animExtra;
	actor->_animExtraPtr = _currentActorAnimExtraPtr;
	actor->_animType = animType;
	actor->_animPosition = 0;
	actor->_dynamicFlags.bIsHitting = 0;
	actor->_dynamicFlags.bAnimEnded = 0;
	actor->_dynamicFlags.bAnimFrameReached = 1;

	processAnimActions(actorIdx);

	actor->_lastRotationAngle = ANGLE_0;
	actor->_lastPos = IVec3();

	return true;
}

void Animations::processActorAnimations(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	_currentlyProcessedActorIdx = actorIdx;
	_engine->_actor->_processActorPtr = actor;

	if (actor->_entity == -1) {
		return;
	}

	IVec3 &previousActor = _engine->_movements->_previousActor;
	previousActor = actor->_collisionPos;

	IVec3 &processActor = _engine->_movements->_processActor;
	if (actor->_staticFlags.bIsSpriteActor) {
		if (actor->_strengthOfHit) {
			actor->_dynamicFlags.bIsHitting = 1;
		}

		processActor = actor->pos();

		if (!actor->_dynamicFlags.bIsFalling) {
			if (actor->_speed) {
				int32 xAxisRotation = actor->_move.getRealValue(_engine->_lbaTime);
				if (!xAxisRotation) {
					if (actor->_move.to > 0) {
						xAxisRotation = 1;
					} else {
						xAxisRotation = -1;
					}
				}

				IVec3 destPos = _engine->_movements->rotateActor(xAxisRotation, 0, actor->_spriteActorRotation);

				processActor.y = actor->_pos.y - destPos.z;

				destPos = _engine->_movements->rotateActor(0, destPos.x, actor->_angle);

				processActor.x = actor->_pos.x + destPos.x;
				processActor.z = actor->_pos.z + destPos.z;

				_engine->_movements->setActorAngle(ANGLE_0, actor->_speed, ANGLE_17, &actor->_move);

				if (actor->_dynamicFlags.bIsSpriteMoving) {
					if (actor->_doorStatus) { // open door
						if (getDistance2D(processActor.x, processActor.z, actor->_lastPos.x, actor->_lastPos.z) >= actor->_doorStatus) {
							if (actor->_angle == ANGLE_0) {
								processActor.z = actor->_lastPos.z + actor->_doorStatus;
							} else if (actor->_angle == ANGLE_90) {
								processActor.x = actor->_lastPos.x + actor->_doorStatus;
							} else if (actor->_angle == ANGLE_180) {
								processActor.z = actor->_lastPos.z - actor->_doorStatus;
							} else if (actor->_angle == ANGLE_270) {
								processActor.x = actor->_lastPos.x - actor->_doorStatus;
							}

							actor->_dynamicFlags.bIsSpriteMoving = 0;
							actor->_speed = 0;
						}
					} else { // close door
						bool updatePos = false;

						if (actor->_angle == ANGLE_0) {
							if (processActor.z <= actor->_lastPos.z) {
								updatePos = true;
							}
						} else if (actor->_angle == ANGLE_90) {
							if (processActor.x <= actor->_lastPos.x) {
								updatePos = true;
							}
						} else if (actor->_angle == ANGLE_180) {
							if (processActor.z >= actor->_lastPos.z) {
								updatePos = true;
							}
						} else if (actor->_angle == ANGLE_270) {
							if (processActor.x >= actor->_lastPos.x) {
								updatePos = true;
							}
						}

						if (updatePos) {
							processActor = actor->_lastPos;

							actor->_dynamicFlags.bIsSpriteMoving = 0;
							actor->_speed = 0;
						}
					}
				}
			}

			if (actor->_staticFlags.bCanBePushed) {
				processActor += actor->_lastPos;

				if (actor->_staticFlags.bUseMiniZv) {
					processActor.x = ((processActor.x / 128) * 128);
					processActor.z = ((processActor.z / 128) * 128);
				}

				actor->_lastPos = IVec3();
			}
		}
	} else { // 3D actor
		if (actor->_previousAnimIdx != -1) {
			const AnimData &animData = _engine->_resources->_animData[actor->_previousAnimIdx];

			bool keyFramePassed = false;
			if (_engine->_resources->_bodyData[actor->_entity].isAnimated()) {
				keyFramePassed = verifyAnimAtKeyframe(actor->_animPosition, animData, &actor->_animTimerData);
			}

			if (_processRotationByAnim) {
				actor->_dynamicFlags.bIsRotationByAnim = 1;
			} else {
				actor->_dynamicFlags.bIsRotationByAnim = 0;
			}

			actor->_angle = ClampAngle(actor->_angle + _processLastRotationAngle - actor->_lastRotationAngle);
			actor->_lastRotationAngle = _processLastRotationAngle;

			const IVec3 &destPos = _engine->_movements->rotateActor(_currentStep.x, _currentStep.z, actor->_angle);

			_currentStep.x = destPos.x;
			_currentStep.z = destPos.z;

			processActor = actor->pos() + _currentStep - actor->_lastPos;

			actor->_lastPos = _currentStep;

			actor->_dynamicFlags.bAnimEnded = 0;
			actor->_dynamicFlags.bAnimFrameReached = 0;

			if (keyFramePassed) {
				actor->_animPosition++;

				// if actor have animation actions to process
				processAnimActions(actorIdx);

				int16 numKeyframe = actor->_animPosition;
				if (numKeyframe == (int16)animData.getNumKeyframes()) {
					actor->_dynamicFlags.bIsHitting = 0;

					if (actor->_animType == AnimType::kAnimationTypeLoop) {
						actor->_animPosition = animData.getLoopFrame();
					} else {
						actor->_anim = actor->_animExtra;
						actor->_previousAnimIdx = getBodyAnimIndex(actor->_anim, actorIdx);

						if (actor->_previousAnimIdx == -1) {
							actor->_previousAnimIdx = getBodyAnimIndex(AnimationTypes::kStanding, actorIdx);
							actor->_anim = AnimationTypes::kStanding;
						}

						actor->_animExtraPtr = _currentActorAnimExtraPtr;

						actor->_animType = AnimType::kAnimationTypeLoop;
						actor->_animPosition = 0;
						actor->_strengthOfHit = 0;
					}

					processAnimActions(actorIdx);

					actor->_dynamicFlags.bAnimEnded = 1;
				}

				actor->_lastRotationAngle = ANGLE_0;

				actor->_lastPos = IVec3();
			}
		}
	}

	// actor standing on another actor
	if (actor->_standOn != -1) {
		const ActorStruct *standOnActor = _engine->_scene->getActor(actor->_standOn);
		processActor -= standOnActor->_collisionPos;
		processActor += standOnActor->pos();

		if (!_engine->_collision->standingOnActor(actorIdx, actor->_standOn)) {
			actor->_standOn = -1; // no longer standing on other actor
		}
	}

	// actor falling Y speed
	if (actor->_dynamicFlags.bIsFalling) {
		processActor = previousActor;
		processActor.y += _engine->_loopActorStep; // add step to fall
	}

	// actor collisions with bricks
	if (actor->_staticFlags.bComputeCollisionWithBricks) {
		_engine->_collision->_collision.y = 0;

		ShapeType brickShape = _engine->_grid->getBrickShape(previousActor);

		if (brickShape != ShapeType::kNone) {
			if (brickShape != ShapeType::kSolid) {
				_engine->_collision->reajustActorPosition(brickShape);
			} /*else { // this shouldn't happen (collision should avoid it)
				actor->y = processActor.y = (processActor.y / BRICK_HEIGHT) * BRICK_HEIGHT + BRICK_HEIGHT; // go upper
			}*/
		}

		if (actor->_staticFlags.bComputeCollisionWithObj) {
			_engine->_collision->checkCollisionWithActors(actorIdx);
		}

		if (actor->_standOn != -1 && actor->_dynamicFlags.bIsFalling) {
			_engine->_collision->stopFalling();
		}

		_engine->_collision->_causeActorDamage = 0;

		_engine->_collision->_processCollision = processActor;

		if (IS_HERO(actorIdx) && !actor->_staticFlags.bComputeLowCollision) {
			// check hero collisions with bricks
			_engine->_collision->checkHeroCollisionWithBricks(actor->_boudingBox.mins.x, actor->_boudingBox.mins.y, actor->_boudingBox.mins.z, 1);
			_engine->_collision->checkHeroCollisionWithBricks(actor->_boudingBox.maxs.x, actor->_boudingBox.mins.y, actor->_boudingBox.mins.z, 2);
			_engine->_collision->checkHeroCollisionWithBricks(actor->_boudingBox.maxs.x, actor->_boudingBox.mins.y, actor->_boudingBox.maxs.z, 4);
			_engine->_collision->checkHeroCollisionWithBricks(actor->_boudingBox.mins.x, actor->_boudingBox.mins.y, actor->_boudingBox.maxs.z, 8);
		} else {
			// check other actors collisions with bricks
			_engine->_collision->checkActorCollisionWithBricks(actor->_boudingBox.mins.x, actor->_boudingBox.mins.y, actor->_boudingBox.mins.z, 1);
			_engine->_collision->checkActorCollisionWithBricks(actor->_boudingBox.maxs.x, actor->_boudingBox.mins.y, actor->_boudingBox.mins.z, 2);
			_engine->_collision->checkActorCollisionWithBricks(actor->_boudingBox.maxs.x, actor->_boudingBox.mins.y, actor->_boudingBox.maxs.z, 4);
			_engine->_collision->checkActorCollisionWithBricks(actor->_boudingBox.mins.x, actor->_boudingBox.mins.y, actor->_boudingBox.maxs.z, 8);
		}

		// process wall hit while running
		if (_engine->_collision->_causeActorDamage && !actor->_dynamicFlags.bIsFalling && !_currentlyProcessedActorIdx && _engine->_actor->_heroBehaviour == HeroBehaviourType::kAthletic && actor->_anim == AnimationTypes::kForward) {
			IVec3 destPos = _engine->_movements->rotateActor(actor->_boudingBox.mins.x, actor->_boudingBox.mins.z, actor->_angle + ANGLE_360 + ANGLE_135);

			destPos.x += processActor.x;
			destPos.z += processActor.z;

			if (destPos.x >= 0 && destPos.z >= 0 && destPos.x <= 0x7E00 && destPos.z <= 0x7E00) { // SCENE_SIZE_MAX
				if (_engine->_grid->getBrickShape(destPos.x, processActor.y + BRICK_HEIGHT, destPos.z) != ShapeType::kNone && _engine->_cfgfile.WallCollision) { // avoid wall hit damage
					_engine->_extra->addExtraSpecial(actor->_pos.x, actor->_pos.y + 1000, actor->_pos.z, ExtraSpecialType::kHitStars);
					initAnim(AnimationTypes::kBigHit, AnimType::kAnimationType_2, AnimationTypes::kStanding, _currentlyProcessedActorIdx);

					if (IS_HERO(_currentlyProcessedActorIdx)) {
						_engine->_movements->_heroMoved = true;
					}

					actor->addLife(-1);
				}
			}
		}

		brickShape = _engine->_grid->getBrickShape(processActor);
		actor->setBrickShape(brickShape);

		if (brickShape != ShapeType::kNone) {
			if (brickShape == ShapeType::kSolid) {
				if (actor->_dynamicFlags.bIsFalling) {
					_engine->_collision->stopFalling();
					processActor.y = (_engine->_collision->_collision.y * BRICK_HEIGHT) + BRICK_HEIGHT;
				} else {
					if (IS_HERO(actorIdx) && _engine->_actor->_heroBehaviour == HeroBehaviourType::kAthletic && actor->_anim == AnimationTypes::kForward && _engine->_cfgfile.WallCollision) { // avoid wall hit damage
						_engine->_extra->addExtraSpecial(actor->_pos.x, actor->_pos.y + 1000, actor->_pos.z, ExtraSpecialType::kHitStars);
						initAnim(AnimationTypes::kBigHit, AnimType::kAnimationType_2, AnimationTypes::kStanding, _currentlyProcessedActorIdx);
						_engine->_movements->_heroMoved = true;
						actor->addLife(-1);
					}

					// no Z coordinate issue
					if (_engine->_grid->getBrickShape(processActor.x, processActor.y, previousActor.z) == ShapeType::kNone) {
						processActor.z = previousActor.z;
					}

					// no X coordinate issue
					if (_engine->_grid->getBrickShape(previousActor.x, processActor.y, processActor.z) == ShapeType::kNone) {
						processActor.x = previousActor.x;
					}

					// X and Z with issue, no move
					if (_engine->_grid->getBrickShape(processActor.x, processActor.y, previousActor.z) != ShapeType::kNone &&
					    _engine->_grid->getBrickShape(previousActor.x, processActor.y, processActor.z) != ShapeType::kNone) {
						return;
					}
				}
			} else {
				if (actor->_dynamicFlags.bIsFalling) {
					_engine->_collision->stopFalling();
				}

				_engine->_collision->reajustActorPosition(brickShape);
			}

			actor->_dynamicFlags.bIsFalling = 0;
		} else {
			if (actor->_staticFlags.bCanFall && actor->_standOn == -1) {
				brickShape = _engine->_grid->getBrickShape(processActor.x, processActor.y - 1, processActor.z);

				if (brickShape != ShapeType::kNone) {
					if (actor->_dynamicFlags.bIsFalling) {
						_engine->_collision->stopFalling();
					}

					_engine->_collision->reajustActorPosition(brickShape);
				} else {
					if (!actor->_dynamicFlags.bIsRotationByAnim) {
						actor->_dynamicFlags.bIsFalling = 1;

						if (IS_HERO(actorIdx) && _engine->_scene->_heroYBeforeFall == 0) {
							_engine->_scene->_heroYBeforeFall = processActor.y;
						}

						initAnim(AnimationTypes::kFall, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
					}
				}
			}
		}

		// if under the map, than die
		if (_engine->_collision->_collision.y == -1) {
			actor->setLife(0);
		}
	} else {
		if (actor->_staticFlags.bComputeCollisionWithObj) {
			_engine->_collision->checkCollisionWithActors(actorIdx);
		}
	}

	if (_engine->_collision->_causeActorDamage) {
		actor->setBrickCausesDamage();
	}

	// check and fix actor bounding position
	if (processActor.x < 0) {
		processActor.x = 0;
	}

	if (processActor.y < 0) {
		processActor.y = 0;
	}

	if (processActor.z < 0) {
		processActor.z = 0;
	}

	if (processActor.x > 0x7E00) { // SCENE_SIZE_MAX
		processActor.x = 0x7E00;
	}

	if (processActor.z > 0x7E00) { // SCENE_SIZE_MAX
		processActor.z = 0x7E00;
	}

	actor->_pos = processActor;
}

} // namespace TwinE
