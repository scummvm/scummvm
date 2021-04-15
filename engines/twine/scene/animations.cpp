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
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/audio/sound.h"
#include "twine/parser/anim.h"
#include "twine/parser/entity.h"
#include "twine/renderer/renderer.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/collision.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/twine.h"

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
	const int32 bodyAnimIndex = actor->entityData->getAnimIndex(animIdx);
	if (bodyAnimIndex != -1) {
		currentActorAnimExtraPtr = animIdx;
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

	currentStep.x = keyFrame->x;
	currentStep.y = keyFrame->y;
	currentStep.z = keyFrame->z;

	processRotationByAnim = keyFrame->boneframes[0].type;
	processLastRotationAngle = ToAngle(keyFrame->boneframes[0].y);

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
	const int32 deltaTime = _engine->lbaTime - remainingFrameTime;
	if (deltaTime >= keyFrameLength) {
		copyKeyFrameToState(keyFrame, bodyData, numOfBonesInAnim);
		animTimerDataPtr->ptr = keyFrame;
		animTimerDataPtr->time = _engine->lbaTime;
		return true;
	}

	processLastRotationAngle = (processLastRotationAngle * deltaTime) / keyFrameLength;

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

	currentStep.x = keyFrame->x;
	currentStep.y = keyFrame->y;
	currentStep.z = keyFrame->z;

	processRotationByAnim = keyFrame->boneframes[0].type;
	processLastRotationAngle = ToAngle(keyFrame->boneframes[0].y);

	animTimerDataPtr->ptr = animData.getKeyframe(keyframeIdx);
	animTimerDataPtr->time = _engine->lbaTime;

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

	if (animKeyframeBufIdx >= ARRAYSIZE(animKeyframeBuf)) {
		animKeyframeBufIdx = 0;
	}
	animTimerDataPtr->time = _engine->lbaTime;
	KeyFrame *keyframe = &animKeyframeBuf[animKeyframeBufIdx++];
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

	const int32 deltaTime = _engine->lbaTime - remainingFrameTime;

	currentStep.x = keyFrame->x;
	currentStep.y = keyFrame->y;
	currentStep.z = keyFrame->z;

	const BoneFrame &boneFrame = keyFrame->boneframes[0];
	processRotationByAnim = boneFrame.type;
	processLastRotationAngle = ToAngle(boneFrame.y);

	if (deltaTime >= keyFrameLength) {
		animTimerDataPtr->ptr = animData.getKeyframe(keyframeIdx);
		animTimerDataPtr->time = _engine->lbaTime;
		return true;
	}

	processLastRotationAngle = (processLastRotationAngle * deltaTime) / keyFrameLength;
	currentStep.x = (currentStep.x * deltaTime) / keyFrameLength;
	currentStep.y = (currentStep.y * deltaTime) / keyFrameLength;
	currentStep.z = (currentStep.z * deltaTime) / keyFrameLength;

	return false;
}

void Animations::processAnimActions(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->entityData == nullptr || actor->animExtraPtr == AnimationTypes::kAnimNone) {
		return;
	}

	const Common::Array<EntityAnim::Action> *actions = actor->entityData->getActions(actor->animExtraPtr);
	if (actions == nullptr) {
		return;
	}
	for (const EntityAnim::Action &action : *actions) {
		switch (action.type) {
		case ActionType::ACTION_HITTING:
			if (action.animFrame - 1 == actor->animPosition) {
				actor->strengthOfHit = action.strength;
				actor->dynamicFlags.bIsHitting = 1;
			}
			break;
		case ActionType::ACTION_SAMPLE:
		case ActionType::ACTION_SAMPLE_FREQ:
			if (action.animFrame == actor->animPosition) {
				_engine->_sound->playSample(action.sampleIndex, 1, actor->pos, actorIdx);
			}
			break;
		case ActionType::ACTION_THROW_EXTRA_BONUS:
			if (action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->pos.x, actor->pos.y + action.yHeight, actor->pos.z, action.spriteIndex, action.xAngle, action.yAngle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_MAGIC_BALL:
			if (_engine->_gameState->magicBallIdx == -1 && action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraThrowMagicball(actor->pos.x, actor->pos.y + action.yHeight, actor->pos.z, action.xAngle, actor->angle + action.yAngle, action.xRotPoint, action.extraAngle);
			}
			break;
		case ActionType::ACTION_SAMPLE_REPEAT:
			if (action.animFrame == actor->animPosition) {
				_engine->_sound->playSample(action.sampleIndex, action.repeat, actor->pos, actorIdx);
			}
			break;
		case ActionType::ACTION_THROW_SEARCH:
			if (action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraAiming(actorIdx, actor->pos.x, actor->pos.y + action.yHeight, actor->pos.z, action.spriteIndex, action.targetActor, action.finalAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_ALPHA:
			if (action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->pos.x, actor->pos.y + action.yHeight, actor->pos.z, action.spriteIndex, action.xAngle, actor->angle + action.yAngle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_SAMPLE_STOP:
			if (action.animFrame == actor->animPosition) {
				_engine->_sound->stopSample(action.sampleIndex);
			}
			break;
		case ActionType::ACTION_LEFT_STEP:
		case ActionType::ACTION_RIGHT_STEP:
			if (action.animFrame == actor->animPosition && (actor->brickSound & 0x0F0) != 0x0F0) {
				const int16 sampleIdx = (actor->brickSound & 0x0F) + Samples::WalkFloorBegin;
				_engine->_sound->playSample(sampleIdx, 1, actor->pos, actorIdx);
			}
			break;
		case ActionType::ACTION_HERO_HITTING:
			if (action.animFrame - 1 == actor->animPosition) {
				actor->strengthOfHit = magicLevelStrengthOfHit[_engine->_gameState->magicLevelIdx];
				actor->dynamicFlags.bIsHitting = 1;
			}
			break;
		case ActionType::ACTION_THROW_3D:
			if (action.animFrame == actor->animPosition) {
				_engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->angle);

				const int32 throwX = _engine->_renderer->destPos.x + actor->pos.x;
				const int32 throwY = action.distanceY + actor->pos.y;
				const int32 throwZ = _engine->_renderer->destPos.z + actor->pos.z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, action.spriteIndex,
				                               action.xAngle, action.yAngle + actor->angle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_3D_ALPHA:
			if (action.animFrame == actor->animPosition) {
				const int32 distance = _engine->_movements->getDistance2D(actor->pos, _engine->_scene->sceneHero->pos);
				const int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->pos.y, 0, _engine->_scene->sceneHero->pos.y, distance);

				_engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->angle);

				const int32 throwX = _engine->_renderer->destPos.x + actor->pos.x;
				const int32 throwY = action.distanceY + actor->pos.y;
				const int32 throwZ = _engine->_renderer->destPos.z + actor->pos.z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, action.spriteIndex,
				                               action.xAngle + newAngle, action.yAngle + actor->angle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_3D_SEARCH:
			if (action.animFrame == actor->animPosition) {
				_engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->angle);
				const int32 x = actor->pos.x + _engine->_renderer->destPos.x;
				const int32 y = actor->pos.y + action.distanceY;
				const int32 z = actor->pos.z + _engine->_renderer->destPos.z;
				_engine->_extra->addExtraAiming(actorIdx, x, y, z, action.spriteIndex,
				                                action.targetActor, action.finalAngle, action.strength);
			}
			break;
		case ActionType::ACTION_UNKNOWN_21:
			if (_engine->_gameState->magicBallIdx == -1 && action.animFrame == actor->animPosition) {
				_engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->angle);
				const int32 x = actor->pos.x + _engine->_renderer->destPos.x;
				const int32 y = actor->pos.y + action.distanceY;
				const int32 z = actor->pos.z + _engine->_renderer->destPos.z;
				_engine->_extra->addExtraThrowMagicball(x, y, z, action.xAngle, actor->angle, action.yAngle, action.finalAngle);
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
	if (actor->entity == -1) {
		return false;
	}

	if (actor->staticFlags.bIsSpriteActor) {
		return false;
	}

	if (newAnim == actor->anim && actor->previousAnimIdx != -1) {
		return true;
	}

	if (animExtra == AnimationTypes::kAnimInvalid && actor->animType != AnimType::kAnimationType_2) {
		animExtra = actor->anim;
	}

	int32 animIndex = getBodyAnimIndex(newAnim, actorIdx);

	if (animIndex == -1) {
		animIndex = getBodyAnimIndex(AnimationTypes::kStanding, actorIdx);
	}

	if (animType != AnimType::kAnimationType_4 && actor->animType == AnimType::kAnimationType_2) {
		actor->animExtra = newAnim;
		return false;
	}

	if (animType == AnimType::kAnimationType_3) {
		animType = AnimType::kAnimationType_2;

		animExtra = actor->anim;

		if (animExtra == AnimationTypes::kThrowBall || animExtra == AnimationTypes::kFall || animExtra == AnimationTypes::kLanding || animExtra == AnimationTypes::kLandingHit) {
			animExtra = AnimationTypes::kStanding;
		}
	}

	if (animType == AnimType::kAnimationType_4) {
		animType = AnimType::kAnimationType_2;
	}

	if (actor->previousAnimIdx == -1) {
		// if no previous animation
		setAnimAtKeyframe(0, _engine->_resources->animData[animIndex], _engine->_resources->bodyData[actor->entity], &actor->animTimerData);
	} else {
		// interpolation between animations
		stockAnimation(_engine->_resources->bodyData[actor->entity], &actor->animTimerData);
	}

	actor->previousAnimIdx = animIndex;
	actor->anim = newAnim;
	actor->animExtra = animExtra;
	actor->animExtraPtr = currentActorAnimExtraPtr;
	actor->animType = animType;
	actor->animPosition = 0;
	actor->dynamicFlags.bIsHitting = 0;
	actor->dynamicFlags.bAnimEnded = 0;
	actor->dynamicFlags.bAnimFrameReached = 1;

	processAnimActions(actorIdx);

	actor->lastRotationAngle = ANGLE_0;
	actor->lastPos = IVec3();

	return true;
}

void Animations::processActorAnimations(int32 actorIdx) { // DoAnim
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	currentlyProcessedActorIdx = actorIdx;
	_engine->_actor->processActorPtr = actor;

	if (actor->entity == -1) {
		return;
	}

	_engine->_movements->previousActor = actor->collisionPos;

	if (actor->staticFlags.bIsSpriteActor) {
		if (actor->strengthOfHit) {
			actor->dynamicFlags.bIsHitting = 1;
		}

		_engine->_movements->processActor = actor->pos;

		if (!actor->dynamicFlags.bIsFalling) {
			if (actor->speed) {
				int32 xAxisRotation = actor->move.getRealValue(_engine->lbaTime);
				if (!xAxisRotation) {
					if (actor->move.to > 0) {
						xAxisRotation = 1;
					} else {
						xAxisRotation = -1;
					}
				}

				_engine->_movements->rotateActor(xAxisRotation, 0, actor->spriteActorRotation);

				_engine->_movements->processActor.y = actor->pos.y - _engine->_renderer->destPos.z;

				_engine->_movements->rotateActor(0, _engine->_renderer->destPos.x, actor->angle);

				_engine->_movements->processActor.x = actor->pos.x + _engine->_renderer->destPos.x;
				_engine->_movements->processActor.z = actor->pos.z + _engine->_renderer->destPos.z;

				_engine->_movements->setActorAngle(ANGLE_0, actor->speed, ANGLE_17, &actor->move);

				if (actor->dynamicFlags.bIsSpriteMoving) {
					if (actor->doorStatus) { // open door
						if (_engine->_movements->getDistance2D(_engine->_movements->processActor.x, _engine->_movements->processActor.z, actor->lastPos.x, actor->lastPos.z) >= actor->doorStatus) {
							if (actor->angle == ANGLE_0) {
								_engine->_movements->processActor.z = actor->lastPos.z + actor->doorStatus;
							} else if (actor->angle == ANGLE_90) {
								_engine->_movements->processActor.x = actor->lastPos.x + actor->doorStatus;
							} else if (actor->angle == ANGLE_180) {
								_engine->_movements->processActor.z = actor->lastPos.z - actor->doorStatus;
							} else if (actor->angle == ANGLE_270) {
								_engine->_movements->processActor.x = actor->lastPos.x - actor->doorStatus;
							}

							actor->dynamicFlags.bIsSpriteMoving = 0;
							actor->speed = 0;
						}
					} else { // close door
						bool updatePos = false;

						if (actor->angle == ANGLE_0) {
							if (_engine->_movements->processActor.z <= actor->lastPos.z) {
								updatePos = true;
							}
						} else if (actor->angle == ANGLE_90) {
							if (_engine->_movements->processActor.x <= actor->lastPos.x) {
								updatePos = true;
							}
						} else if (actor->angle == ANGLE_180) {
							if (_engine->_movements->processActor.z >= actor->lastPos.z) {
								updatePos = true;
							}
						} else if (actor->angle == ANGLE_270) {
							if (_engine->_movements->processActor.x >= actor->lastPos.x) {
								updatePos = true;
							}
						}

						if (updatePos) {
							_engine->_movements->processActor = actor->lastPos;

							actor->dynamicFlags.bIsSpriteMoving = 0;
							actor->speed = 0;
						}
					}
				}
			}

			if (actor->staticFlags.bCanBePushed) {
				_engine->_movements->processActor += actor->lastPos;

				if (actor->staticFlags.bUseMiniZv) {
					_engine->_movements->processActor.x = ((_engine->_movements->processActor.x / 128) * 128);
					_engine->_movements->processActor.z = ((_engine->_movements->processActor.z / 128) * 128);
				}

				actor->lastPos.x = 0;
				actor->lastPos.y = 0;
				actor->lastPos.z = 0;
			}
		}
	} else { // 3D actor
		if (actor->previousAnimIdx != -1) {
			const AnimData &animData = _engine->_resources->animData[actor->previousAnimIdx];

			bool keyFramePassed = false;
			if (_engine->_resources->bodyData[actor->entity].isAnimated()) {
				keyFramePassed = verifyAnimAtKeyframe(actor->animPosition, animData, &actor->animTimerData);
			}

			if (processRotationByAnim) {
				actor->dynamicFlags.bIsRotationByAnim = 1;
			} else {
				actor->dynamicFlags.bIsRotationByAnim = 0;
			}

			actor->angle = ClampAngle(actor->angle + processLastRotationAngle - actor->lastRotationAngle);
			actor->lastRotationAngle = processLastRotationAngle;

			_engine->_movements->rotateActor(currentStep.x, currentStep.z, actor->angle);

			currentStep.x = _engine->_renderer->destPos.x;
			currentStep.z = _engine->_renderer->destPos.z;

			_engine->_movements->processActor = actor->pos + currentStep - actor->lastPos;

			actor->lastPos = currentStep;

			actor->dynamicFlags.bAnimEnded = 0;
			actor->dynamicFlags.bAnimFrameReached = 0;

			if (keyFramePassed) {
				actor->animPosition++;

				// if actor have animation actions to process
				processAnimActions(actorIdx);

				int16 numKeyframe = actor->animPosition;
				if (numKeyframe == (int16)animData.getNumKeyframes()) {
					actor->dynamicFlags.bIsHitting = 0;

					if (actor->animType == AnimType::kAnimationTypeLoop) {
						actor->animPosition = animData.getLoopFrame();
					} else {
						actor->anim = actor->animExtra;
						actor->previousAnimIdx = getBodyAnimIndex(actor->anim, actorIdx);

						if (actor->previousAnimIdx == -1) {
							actor->previousAnimIdx = getBodyAnimIndex(AnimationTypes::kStanding, actorIdx);
							actor->anim = AnimationTypes::kStanding;
						}

						actor->animExtraPtr = currentActorAnimExtraPtr;

						actor->animType = AnimType::kAnimationTypeLoop;
						actor->animPosition = 0;
						actor->strengthOfHit = 0;
					}

					processAnimActions(actorIdx);

					actor->dynamicFlags.bAnimEnded = 1;
				}

				actor->lastRotationAngle = ANGLE_0;

				actor->lastPos.x = 0;
				actor->lastPos.y = 0;
				actor->lastPos.z = 0;
			}
		}
	}

	// actor standing on another actor
	if (actor->standOn != -1) {
		const ActorStruct *standOnActor = _engine->_scene->getActor(actor->standOn);
		_engine->_movements->processActor -= standOnActor->collisionPos;
		_engine->_movements->processActor += standOnActor->pos;

		if (!_engine->_collision->standingOnActor(actorIdx, actor->standOn)) {
			actor->standOn = -1; // no longer standing on other actor
		}
	}

	// actor falling Y speed
	if (actor->dynamicFlags.bIsFalling) {
		_engine->_movements->processActor.x = _engine->_movements->previousActor.x;
		_engine->_movements->processActor.y = _engine->_movements->previousActor.y + _engine->loopActorStep; // add step to fall
		_engine->_movements->processActor.z = _engine->_movements->previousActor.z;
	}

	// actor collisions with bricks
	if (actor->staticFlags.bComputeCollisionWithBricks) {
		_engine->_collision->collision.y = 0;

		ShapeType brickShape = _engine->_grid->getBrickShape(_engine->_movements->previousActor.x, _engine->_movements->previousActor.y, _engine->_movements->previousActor.z);

		if (brickShape != ShapeType::kNone) {
			if (brickShape != ShapeType::kSolid) {
				_engine->_collision->reajustActorPosition(brickShape);
			} /*else { // this shouldn't happen (collision should avoid it)
				actor->y = processActor.y = (processActor.y / BRICK_HEIGHT) * BRICK_HEIGHT + BRICK_HEIGHT; // go upper
			}*/
		}

		if (actor->staticFlags.bComputeCollisionWithObj) {
			_engine->_collision->checkCollisionWithActors(actorIdx);
		}

		if (actor->standOn != -1 && actor->dynamicFlags.bIsFalling) {
			_engine->_collision->stopFalling();
		}

		_engine->_collision->causeActorDamage = 0;

		_engine->_collision->processCollision = _engine->_movements->processActor;

		if (IS_HERO(actorIdx) && !actor->staticFlags.bComputeLowCollision) {
			// check hero collisions with bricks
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.mins.x, actor->boudingBox.mins.y, actor->boudingBox.mins.z, 1);
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.maxs.x, actor->boudingBox.mins.y, actor->boudingBox.mins.z, 2);
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.maxs.x, actor->boudingBox.mins.y, actor->boudingBox.maxs.z, 4);
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.mins.x, actor->boudingBox.mins.y, actor->boudingBox.maxs.z, 8);
		} else {
			// check other actors collisions with bricks
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.mins.x, actor->boudingBox.mins.y, actor->boudingBox.mins.z, 1);
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.maxs.x, actor->boudingBox.mins.y, actor->boudingBox.mins.z, 2);
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.maxs.x, actor->boudingBox.mins.y, actor->boudingBox.maxs.z, 4);
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.mins.x, actor->boudingBox.mins.y, actor->boudingBox.maxs.z, 8);
		}

		// process wall hit while running
		if (_engine->_collision->causeActorDamage && !actor->dynamicFlags.bIsFalling && !currentlyProcessedActorIdx && _engine->_actor->heroBehaviour == HeroBehaviourType::kAthletic && actor->anim == AnimationTypes::kForward) {
			_engine->_movements->rotateActor(actor->boudingBox.mins.x, actor->boudingBox.mins.z, actor->angle + ANGLE_360 + ANGLE_135);

			_engine->_renderer->destPos.x += _engine->_movements->processActor.x;
			_engine->_renderer->destPos.z += _engine->_movements->processActor.z;

			if (_engine->_renderer->destPos.x >= 0 && _engine->_renderer->destPos.z >= 0 && _engine->_renderer->destPos.x <= 0x7E00 && _engine->_renderer->destPos.z <= 0x7E00) {
				if (_engine->_grid->getBrickShape(_engine->_renderer->destPos.x, _engine->_movements->processActor.y + BRICK_HEIGHT, _engine->_renderer->destPos.z) != ShapeType::kNone && _engine->cfgfile.WallCollision) { // avoid wall hit damage
					_engine->_extra->addExtraSpecial(actor->pos.x, actor->pos.y + 1000, actor->pos.z, ExtraSpecialType::kHitStars);
					initAnim(AnimationTypes::kBigHit, AnimType::kAnimationType_2, AnimationTypes::kStanding, currentlyProcessedActorIdx);

					if (IS_HERO(currentlyProcessedActorIdx)) {
						_engine->_movements->heroMoved = true;
					}

					actor->addLife(-1);
				}
			}
		}

		brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z);
		actor->setBrickShape(brickShape);

		if (brickShape != ShapeType::kNone) {
			if (brickShape == ShapeType::kSolid) {
				if (actor->dynamicFlags.bIsFalling) {
					_engine->_collision->stopFalling();
					_engine->_movements->processActor.y = (_engine->_collision->collision.y * BRICK_HEIGHT) + BRICK_HEIGHT;
				} else {
					if (IS_HERO(actorIdx) && _engine->_actor->heroBehaviour == HeroBehaviourType::kAthletic && actor->anim == AnimationTypes::kForward && _engine->cfgfile.WallCollision) { // avoid wall hit damage
						_engine->_extra->addExtraSpecial(actor->pos.x, actor->pos.y + 1000, actor->pos.z, ExtraSpecialType::kHitStars);
						initAnim(AnimationTypes::kBigHit, AnimType::kAnimationType_2, AnimationTypes::kStanding, currentlyProcessedActorIdx);
						_engine->_movements->heroMoved = true;
						actor->addLife(-1);
					}

					// no Z coordinate issue
					if (_engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->previousActor.z) == ShapeType::kNone) {
						_engine->_movements->processActor.z = _engine->_movements->previousActor.z;
					}

					// no X coordinate issue
					if (_engine->_grid->getBrickShape(_engine->_movements->previousActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z) == ShapeType::kNone) {
						_engine->_movements->processActor.x = _engine->_movements->previousActor.x;
					}

					// X and Z with issue, no move
					if (_engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->previousActor.z) != ShapeType::kNone &&
					    _engine->_grid->getBrickShape(_engine->_movements->previousActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z) != ShapeType::kNone) {
						return;
					}
				}
			} else {
				if (actor->dynamicFlags.bIsFalling) {
					_engine->_collision->stopFalling();
				}

				_engine->_collision->reajustActorPosition(brickShape);
			}

			actor->dynamicFlags.bIsFalling = 0;
		} else {
			if (actor->staticFlags.bCanFall && actor->standOn == -1) {
				brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y - 1, _engine->_movements->processActor.z);

				if (brickShape != ShapeType::kNone) {
					if (actor->dynamicFlags.bIsFalling) {
						_engine->_collision->stopFalling();
					}

					_engine->_collision->reajustActorPosition(brickShape);
				} else {
					if (!actor->dynamicFlags.bIsRotationByAnim) {
						actor->dynamicFlags.bIsFalling = 1;

						if (IS_HERO(actorIdx) && _engine->_scene->heroYBeforeFall == 0) {
							_engine->_scene->heroYBeforeFall = _engine->_movements->processActor.y;
						}

						initAnim(AnimationTypes::kFall, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
					}
				}
			}
		}

		// if under the map, than die
		if (_engine->_collision->collision.y == -1) {
			actor->setLife(0);
		}
	} else {
		if (actor->staticFlags.bComputeCollisionWithObj) {
			_engine->_collision->checkCollisionWithActors(actorIdx);
		}
	}

	if (_engine->_collision->causeActorDamage) {
		actor->setBrickCausesDamage();
	}

	// check and fix actor bounding position
	if (_engine->_movements->processActor.x < 0) {
		_engine->_movements->processActor.x = 0;
	}

	if (_engine->_movements->processActor.y < 0) {
		_engine->_movements->processActor.y = 0;
	}

	if (_engine->_movements->processActor.z < 0) {
		_engine->_movements->processActor.z = 0;
	}

	if (_engine->_movements->processActor.x > 0x7E00) { // SCENE_SIZE_MAX
		_engine->_movements->processActor.x = 0x7E00;
	}

	if (_engine->_movements->processActor.z > 0x7E00) { // SCENE_SIZE_MAX
		_engine->_movements->processActor.z = 0x7E00;
	}

	actor->pos = _engine->_movements->processActor;
}

} // namespace TwinE
