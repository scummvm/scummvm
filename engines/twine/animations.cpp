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

#include "twine/animations.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/actor.h"
#include "twine/collision.h"
#include "twine/parser/entity.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/movements.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/shared.h"
#include "twine/sound.h"
#include "twine/twine.h"

namespace TwinE {

static const int32 magicLevelStrengthOfHit[] = {
    MagicballStrengthType::kNoBallStrength,
    MagicballStrengthType::kYellowBallStrength,
    MagicballStrengthType::kGreenBallStrength,
    MagicballStrengthType::kRedBallStrength,
    MagicballStrengthType::kFireBallStrength,
    0};

Animations::Animations(TwinEEngine *engine) : _engine(engine), animBuffer((uint8 *)malloc(5000 * sizeof(uint8))) {
	animBufferPos = animBuffer;
}

Animations::~Animations() {
	free(animBuffer);
}

int32 Animations::getBodyAnimIndex(AnimationTypes animIdx, int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	EntityData entityData;
	entityData.loadFromBuffer(actor->entityDataPtr, actor->entityDataSize);
	const int32 bodyAnimIndex = entityData.getAnimIndex(animIdx);
	if (bodyAnimIndex != -1) {
		currentActorAnimExtraPtr = animIdx;
	}
	return bodyAnimIndex;
}

const uint8* Animations::getKeyFrameData(int32 frameIdx, const uint8 *animPtr) {
	const int16 numOfBonesInAnim = getNumBoneframes(animPtr);
	return (const uint8 *)((numOfBonesInAnim * 8 + 8) * frameIdx + animPtr + 8);
}

int16 Animations::getKeyFrameLength(int32 frameIdx, const uint8 *animPtr) {
	return READ_LE_INT16(getKeyFrameData(frameIdx, animPtr));
}

int16 Animations::getNumBoneframes(const uint8 *animPtr) {
	return READ_LE_INT16(animPtr + 2);
}

int16 Animations::getNumKeyframes(const uint8 *animPtr) {
	return READ_LE_INT16(animPtr + 0);
}

int16 Animations::getStartKeyframe(const uint8 *animPtr) {
	return READ_LE_INT16(animPtr + 4);
}

void Animations::applyAnimStepRotation(uint8 **ptr, int32 bp, int32 bx, const uint8 **keyFramePtr, const uint8 **lastKeyFramePtr) {
	const int16 lastAngle = ClampAngle(READ_LE_INT16(*lastKeyFramePtr));
	*lastKeyFramePtr += 2;

	const int16 newAngle = ClampAngle(READ_LE_INT16(*keyFramePtr));
	*keyFramePtr += 2;

	int16 angleDiff = newAngle - lastAngle;

	int16 computedAngle;
	if (angleDiff) {
		if (angleDiff < -ANGLE_180) {
			angleDiff += ANGLE_360;
		} else if (angleDiff > ANGLE_180) {
			angleDiff -= ANGLE_360;
		}

		computedAngle = lastAngle + (angleDiff * bp) / bx;
	} else {
		computedAngle = lastAngle;
	}

	int16 *dest = (int16 *)*(ptr);
	*dest = ClampAngle(computedAngle);
	*(ptr) = *(ptr) + 2;
}

void Animations::applyAnimStep(uint8 **ptr, int32 bp, int32 bx, const uint8 **keyFramePtr, const uint8 **lastKeyFramePtr) {
	int16 lastAngle = READ_LE_INT16(*lastKeyFramePtr);
	*lastKeyFramePtr += 2;

	int16 newAngle = READ_LE_INT16(*keyFramePtr);
	*keyFramePtr += 2;

	int16 angleDif = newAngle - lastAngle;

	int16 computedAngle;
	if (angleDif) {
		computedAngle = lastAngle + (angleDif * bp) / bx;
	} else {
		computedAngle = lastAngle;
	}

	int16 *dest = (int16 *)*(ptr);
	*dest = computedAngle;
	*(ptr) = *(ptr) + 2;
}

int32 Animations::getAnimMode(uint8 **ptr, const uint8 **keyFramePtr, const uint8 **lastKeyFramePtr) {
	int16 *lptr = (int16 *)*ptr;
	int16 opcode = READ_LE_INT16(*keyFramePtr);
	*(int16 *)(lptr) = opcode;

	*keyFramePtr += 2;
	*(ptr) = *(ptr) + 2;
	*lastKeyFramePtr += 2;

	return opcode;
}

bool Animations::setModelAnimation(int32 animState, const uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr) {
	if (!Model::isAnimated(bodyPtr)) {
		return false;
	}
	int32 numOfPointInAnim = getNumBoneframes(animPtr);
	const uint8 *keyFramePtr = getKeyFrameData(animState, animPtr);
	const int32 keyFrameLength = getKeyFrameLength(animState, animPtr);

	const uint8 *lastKeyFramePtr = animTimerDataPtr->ptr;
	int32 remainingFrameTime = animTimerDataPtr->time;

	if (!lastKeyFramePtr) {
		lastKeyFramePtr = keyFramePtr;
		remainingFrameTime = keyFrameLength;
	}

	uint8 *verticesBase = bodyPtr + 0x1A;

	const int16 numVertices = READ_LE_INT16(verticesBase);
	verticesBase += 2;
	uint8 *bonesBase = verticesBase + numVertices * 6;

	const int32 numBones = READ_LE_INT16(bonesBase);
	bonesBase += 2;

	if (numOfPointInAnim > numBones) {
		numOfPointInAnim = numBones;
	}

	const int32 deltaTime = _engine->lbaTime - remainingFrameTime;

	currentStepX = READ_LE_INT16(keyFramePtr + 2);
	currentStepY = READ_LE_INT16(keyFramePtr + 4);
	currentStepZ = READ_LE_INT16(keyFramePtr + 6);

	processRotationByAnim = READ_LE_INT16(keyFramePtr + 8);
	processLastRotationAngle = ToAngle(READ_LE_INT16(keyFramePtr + 12));

	uint8 *edi = bonesBase + 8;
	if (deltaTime >= keyFrameLength) {
		const int32 *sourcePtr = (const int32 *)(keyFramePtr + 8);
		int32 *destPtr = (int32 *)edi; // keyframe

		do {
			*(destPtr++) = *(sourcePtr++);
			*(destPtr++) = *(sourcePtr++);
			destPtr = (int32 *)(((int8 *)destPtr) + 30);
		} while (--numOfPointInAnim);

		animTimerDataPtr->ptr = keyFramePtr;
		animTimerDataPtr->time = _engine->lbaTime;

		return true;
	}

	processLastRotationAngle = (processLastRotationAngle * deltaTime) / keyFrameLength;

	lastKeyFramePtr += 16;
	keyFramePtr += 16;

	edi += 38;

	if (--numOfPointInAnim) {
		int16 tmpNumOfPoints = numOfPointInAnim;

		do {
			const int16 animOpcode = getAnimMode(&edi, &keyFramePtr, &lastKeyFramePtr);

			switch (animOpcode) {
			case 0: // allow global rotate
				applyAnimStepRotation(&edi, deltaTime, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStepRotation(&edi, deltaTime, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStepRotation(&edi, deltaTime, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				break;
			case 1: // dissallow global rotate
			case 2: // dissallow global rotate + hide
				applyAnimStep(&edi, deltaTime, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStep(&edi, deltaTime, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStep(&edi, deltaTime, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				break;
			default:
				error("Unsupported animation rotation mode %d", animOpcode);
			}

			edi += 30;
		} while (--tmpNumOfPoints);
	}

	return false;
}

int32 Animations::setAnimAtKeyframe(int32 keyframeIdx, const uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr) {
	const int16 numOfKeyframeInAnim = READ_LE_INT16(animPtr);
	if (keyframeIdx >= numOfKeyframeInAnim) {
		return numOfKeyframeInAnim;
	}

	if (!Model::isAnimated(bodyPtr)) {
		return 0;
	}

	int16 numOfBonesInAnim = getNumBoneframes(animPtr);
	const uint8 *ptrToData = getKeyFrameData(keyframeIdx, animPtr);

	animTimerDataPtr->ptr = ptrToData;
	animTimerDataPtr->time = _engine->lbaTime;

	uint8 *verticesBase = bodyPtr + 0x1A;
	int32 numVertices = READ_LE_INT16(verticesBase);

	uint8 *bonesBase = verticesBase + numVertices * 6 + 2;
	const int16 numBones = READ_LE_INT16(bonesBase);

	uint8 *bonesPtr = bonesBase + 2;
	bonesPtr += 8;

	if (numOfBonesInAnim > numBones) {
		numOfBonesInAnim = numBones;
	}

	ptrToData += 2;

	currentStepX = READ_LE_INT16(ptrToData + 0);
	currentStepY = READ_LE_INT16(ptrToData + 2);
	currentStepZ = READ_LE_INT16(ptrToData + 4);

	processRotationByAnim = READ_LE_INT16(ptrToData + 6);
	processLastRotationAngle = ToAngle(READ_LE_INT16(ptrToData + 10));

	ptrToData += 6;

	do {
		for (int32 i = 0; i < 8; i++) {
			*bonesPtr++ = *ptrToData++;
		}

		bonesPtr += 30;
	} while (--numOfBonesInAnim);


	return 1;
}

int32 Animations::stockAnimation(const uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr) {
	if (!Model::isAnimated(bodyPtr)) {
		return 0;
	}
	uint8 *animPtr = animBufferPos;

	const uint8 *verticesBase = bodyPtr + 0x1A;

	animTimerDataPtr->time = _engine->lbaTime;
	animTimerDataPtr->ptr = animPtr;

	int32 numVertices = READ_LE_INT16(verticesBase);
	const uint8 *ptr = verticesBase + numVertices * 6 + 2;

	int32 numBones = READ_LE_INT16(ptr);
	ptr += 2;

	int32 counter = numBones;
	// 8 = 4xint16 - firstpoint, numpoints, basepoint, baseelement - see elementEntry
	int32 var2 = (numBones * 8) + 8;

	int32 *edi = (int32 *)(animPtr + 8);
	const int32 *esi = (const int32 *)(ptr + 8);

	do {
		*(edi++) = *(esi++);
		*(edi++) = *(esi++);

		esi = (const int32 *)(((const int8 *)esi) + 30);
	} while (counter--);

	animBufferPos += var2;

	if (animBuffer + (560 * 8) + 8 < animBufferPos) {
		animBufferPos = animBuffer;
	}

	return var2;
}

bool Animations::verifyAnimAtKeyframe(int32 animIdx, const uint8 *animPtr, AnimTimerDataStruct *animTimerDataPtr) {
	const uint8 *keyFramePtr = getKeyFrameData(animIdx, animPtr);
	const int32 keyFrameLength = getKeyFrameLength(animIdx, animPtr);

	const uint8 *lastKeyFramePtr = animTimerDataPtr->ptr;
	int32 remainingFrameTime = animTimerDataPtr->time;

	if (!lastKeyFramePtr) {
		lastKeyFramePtr = keyFramePtr;
		remainingFrameTime = keyFrameLength;
	}

	const int32 deltaTime = _engine->lbaTime - remainingFrameTime;

	currentStepX = READ_LE_INT16(keyFramePtr + 2);
	currentStepY = READ_LE_INT16(keyFramePtr + 4);
	currentStepZ = READ_LE_INT16(keyFramePtr + 6);

	processRotationByAnim = READ_LE_INT16(keyFramePtr + 8);
	processLastRotationAngle = ToAngle(READ_LE_INT16(keyFramePtr + 12));

	if (deltaTime >= keyFrameLength) {
		animTimerDataPtr->ptr = keyFramePtr;
		animTimerDataPtr->time = _engine->lbaTime;
		return true;
	}

	processLastRotationAngle = (processLastRotationAngle * deltaTime) / keyFrameLength;
	currentStepX = (currentStepX * deltaTime) / keyFrameLength;
	currentStepY = (currentStepY * deltaTime) / keyFrameLength;
	currentStepZ = (currentStepZ * deltaTime) / keyFrameLength;

	return false;
}

void Animations::processAnimActions(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->entityDataPtr == nullptr || actor->animExtraPtr == AnimationTypes::kAnimNone) {
		return;
	}

	EntityData entityData;
	entityData.loadFromBuffer(actor->entityDataPtr, actor->entityDataSize);
	const Common::Array<EntityAnim::Action> *actions = entityData.getActions(actor->animExtraPtr);
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
				_engine->_sound->playSample(action.sampleIndex, 1, actor->x, actor->y, actor->z, actorIdx);
			}
			break;
		case ActionType::ACTION_THROW_EXTRA_BONUS:
			if (action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->x, actor->y + action.yHeight, actor->z, action.spriteIndex, action.xAngle, action.yAngle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_MAGIC_BALL:
			if (_engine->_gameState->magicBallIdx == -1 && action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraThrowMagicball(actor->x, actor->y + action.yHeight, actor->z, action.xAngle, actor->angle + action.yAngle, action.xRotPoint, action.extraAngle);
			}
			break;
		case ActionType::ACTION_SAMPLE_REPEAT:
			if (action.animFrame == actor->animPosition) {
				_engine->_sound->playSample(action.sampleIndex, action.repeat, actor->x, actor->y, actor->z, actorIdx);
			}
			break;
		case ActionType::ACTION_THROW_SEARCH:
			if (action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraAiming(actorIdx, actor->x, actor->y + action.yHeight, actor->z, action.spriteIndex, action.targetActor, action.finalAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_ALPHA:
			if (action.animFrame == actor->animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->x, actor->y + action.yHeight, actor->z, action.spriteIndex, action.xAngle, actor->angle + action.yAngle, action.xRotPoint, action.extraAngle, action.strength);
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
				_engine->_sound->playSample(sampleIdx, 1, actor->x, actor->y, actor->z, actorIdx);
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

				const int32 throwX = _engine->_renderer->destX + actor->x;
				const int32 throwY = action.distanceY + actor->y;
				const int32 throwZ = _engine->_renderer->destZ + actor->z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, action.spriteIndex,
				                               action.xAngle, action.yAngle + actor->angle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_3D_ALPHA:
			if (action.animFrame == actor->animPosition) {
				const int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->y, 0, _engine->_scene->sceneHero->y, _engine->_movements->getDistance2D(actor->x, actor->z, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->z));

				_engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->angle);

				const int32 throwX = _engine->_renderer->destX + actor->x;
				const int32 throwY = action.distanceY + actor->y;
				const int32 throwZ = _engine->_renderer->destZ + actor->z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, action.spriteIndex,
				                               action.xAngle + newAngle, action.yAngle + actor->angle, action.xRotPoint, action.extraAngle, action.strength);
			}
			break;
		case ActionType::ACTION_THROW_3D_SEARCH:
			if (action.animFrame == actor->animPosition) {
				_engine->_movements->rotateActor(action.distanceX, action.distanceZ, actor->angle);
				_engine->_extra->addExtraAiming(actorIdx, actor->x + _engine->_renderer->destX, actor->y + action.distanceY, actor->z + action.distanceZ, action.spriteIndex,
				                                action.targetActor, action.finalAngle, action.strength);
			}
			break;
		case ActionType::ACTION_ZV:
		default:
			break;
		}
	}
}

bool Animations::initAnim(AnimationTypes newAnim, int16 animType, AnimationTypes animExtra, int32 actorIdx) {
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

	if (animExtra == AnimationTypes::kAnimInvalid && actor->animType != 2) {
		animExtra = actor->anim;
	}

	int32 animIndex = getBodyAnimIndex(newAnim, actorIdx);

	if (animIndex == -1) {
		animIndex = getBodyAnimIndex(AnimationTypes::kStanding, actorIdx);
	}

	if (animType != 4 && actor->animType == 2) {
		actor->animExtra = newAnim;
		return false;
	}

	if (animType == 3) {
		animType = 2;

		animExtra = actor->anim;

		if (animExtra == AnimationTypes::kThrowBall || animExtra == AnimationTypes::kFall || animExtra == AnimationTypes::kLanding || animExtra == AnimationTypes::kLandingHit) {
			animExtra = AnimationTypes::kStanding;
		}
	}

	if (animType == 4) {
		animType = 2;
	}

	if (actor->previousAnimIdx == -1) {
		// if no previous animation
		setAnimAtKeyframe(0, _engine->_resources->animTable[animIndex], _engine->_actor->bodyTable[actor->entity], &actor->animTimerData);
	} else {
		// interpolation between animations
		animBufferPos += stockAnimation(_engine->_actor->bodyTable[actor->entity], &actor->animTimerData);
		if (animBuffer + (560 * 8) + 8 < animBufferPos) {
			animBufferPos = animBuffer;
		}
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

	actor->lastRotationAngle = 0;
	actor->lastX = 0;
	actor->lastY = 0;
	actor->lastZ = 0;

	return true;
}

void Animations::processActorAnimations(int32 actorIdx) { // DoAnim
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	currentlyProcessedActorIdx = actorIdx;
	_engine->_actor->processActorPtr = actor;

	if (actor->entity == -1) {
		return;
	}

	_engine->_movements->previousActorX = actor->collisionX;
	_engine->_movements->previousActorY = actor->collisionY;
	_engine->_movements->previousActorZ = actor->collisionZ;

	if (actor->staticFlags.bIsSpriteActor) { // is sprite actor
		if (actor->strengthOfHit) {
			actor->dynamicFlags.bIsHitting = 1;
		}

		_engine->_movements->processActorX = actor->x;
		_engine->_movements->processActorY = actor->y;
		_engine->_movements->processActorZ = actor->z;

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

				_engine->_movements->rotateActor(xAxisRotation, 0, actor->animType);

				_engine->_movements->processActorY = actor->y - _engine->_renderer->destZ;

				_engine->_movements->rotateActor(0, _engine->_renderer->destX, actor->angle);

				_engine->_movements->processActorX = actor->x + _engine->_renderer->destX;
				_engine->_movements->processActorZ = actor->z + _engine->_renderer->destZ;

				_engine->_movements->setActorAngle(ANGLE_0, actor->speed, 50, &actor->move);

				if (actor->dynamicFlags.bIsSpriteMoving) {
					if (actor->doorStatus) { // open door
						if (_engine->_movements->getDistance2D(_engine->_movements->processActorX, _engine->_movements->processActorZ, actor->lastX, actor->lastZ) >= actor->doorStatus) {
							if (actor->angle == ANGLE_0) {
								_engine->_movements->processActorZ = actor->lastZ + actor->doorStatus;
							} else if (actor->angle == ANGLE_90) {
								_engine->_movements->processActorX = actor->lastX + actor->doorStatus;
							} else if (actor->angle == ANGLE_180) {
								_engine->_movements->processActorZ = actor->lastZ - actor->doorStatus;
							} else if (actor->angle == ANGLE_270) {
								_engine->_movements->processActorX = actor->lastX - actor->doorStatus;
							}

							actor->dynamicFlags.bIsSpriteMoving = 0;
							actor->speed = 0;
						}
					} else { // close door
						bool updatePos = false;

						if (actor->angle == ANGLE_0) {
							if (_engine->_movements->processActorZ <= actor->lastZ) {
								updatePos = true;
							}
						} else if (actor->angle == ANGLE_90) {
							if (_engine->_movements->processActorX <= actor->lastX) {
								updatePos = true;
							}
						} else if (actor->angle == ANGLE_180) {
							if (_engine->_movements->processActorZ >= actor->lastZ) {
								updatePos = true;
							}
						} else if (actor->angle == ANGLE_270) {
							if (_engine->_movements->processActorX >= actor->lastX) {
								updatePos = true;
							}
						}

						if (updatePos) {
							_engine->_movements->processActorX = actor->lastX;
							_engine->_movements->processActorY = actor->lastY;
							_engine->_movements->processActorZ = actor->lastZ;

							actor->dynamicFlags.bIsSpriteMoving = 0;
							actor->speed = 0;
						}
					}
				}
			}

			if (actor->staticFlags.bCanBePushed) {
				_engine->_movements->processActorX += actor->lastX;
				_engine->_movements->processActorY += actor->lastY;
				_engine->_movements->processActorZ += actor->lastZ;

				if (actor->staticFlags.bUseMiniZv) {
					_engine->_movements->processActorX = ((_engine->_movements->processActorX / 128) * 128);
					_engine->_movements->processActorZ = ((_engine->_movements->processActorZ / 128) * 128);
				}

				actor->lastX = 0;
				actor->lastY = 0;
				actor->lastZ = 0;
			}
		}
	} else { // 3D actor
		if (actor->previousAnimIdx != -1) {
			const uint8 *animPtr = _engine->_resources->animTable[actor->previousAnimIdx];

			bool keyFramePassed = false;
			if (Model::isAnimated(_engine->_actor->bodyTable[actor->entity])) {
				keyFramePassed = verifyAnimAtKeyframe(actor->animPosition, animPtr, &actor->animTimerData);
			}

			if (processRotationByAnim) {
				actor->dynamicFlags.bIsRotationByAnim = 1;
			} else {
				actor->dynamicFlags.bIsRotationByAnim = 0;
			}

			actor->angle = ClampAngle(actor->angle + processLastRotationAngle - actor->lastRotationAngle);
			actor->lastRotationAngle = processLastRotationAngle;

			_engine->_movements->rotateActor(currentStepX, currentStepZ, actor->angle);

			currentStepX = _engine->_renderer->destX;
			currentStepZ = _engine->_renderer->destZ;

			_engine->_movements->processActorX = actor->x + currentStepX - actor->lastX;
			_engine->_movements->processActorY = actor->y + currentStepY - actor->lastY;
			_engine->_movements->processActorZ = actor->z + currentStepZ - actor->lastZ;

			actor->lastX = currentStepX;
			actor->lastY = currentStepY;
			actor->lastZ = currentStepZ;

			actor->dynamicFlags.bAnimEnded = 0;
			actor->dynamicFlags.bAnimFrameReached = 0;

			if (keyFramePassed) {
				actor->animPosition++;

				// if actor have animation actions to process
				processAnimActions(actorIdx);

				int16 numKeyframe = actor->animPosition;
				if (numKeyframe == getNumKeyframes(animPtr)) {
					actor->dynamicFlags.bIsHitting = 0;

					if (actor->animType == 0) {
						actor->animPosition = getStartKeyframe(animPtr);
					} else {
						actor->anim = (AnimationTypes)actor->animExtra;
						actor->previousAnimIdx = getBodyAnimIndex(actor->anim, actorIdx);

						if (actor->previousAnimIdx == -1) {
							actor->previousAnimIdx = getBodyAnimIndex(AnimationTypes::kStanding, actorIdx);
							actor->anim = AnimationTypes::kStanding;
						}

						actor->animExtraPtr = currentActorAnimExtraPtr;

						actor->animType = 0;
						actor->animPosition = 0;
						actor->strengthOfHit = 0;
					}

					processAnimActions(actorIdx);

					actor->dynamicFlags.bAnimEnded = 1;
				}

				actor->lastRotationAngle = 0;

				actor->lastX = 0;
				actor->lastY = 0;
				actor->lastZ = 0;
			}
		}
	}

	// actor standing on another actor
	if (actor->standOn != -1) {
		_engine->_movements->processActorX -= _engine->_scene->getActor(actor->standOn)->collisionX;
		_engine->_movements->processActorY -= _engine->_scene->getActor(actor->standOn)->collisionY;
		_engine->_movements->processActorZ -= _engine->_scene->getActor(actor->standOn)->collisionZ;

		_engine->_movements->processActorX += _engine->_scene->getActor(actor->standOn)->x;
		_engine->_movements->processActorY += _engine->_scene->getActor(actor->standOn)->y;
		_engine->_movements->processActorZ += _engine->_scene->getActor(actor->standOn)->z;

		if (!_engine->_collision->standingOnActor(actorIdx, actor->standOn)) {
			actor->standOn = -1; // no longer standing on other actor
		}
	}

	// actor falling Y speed
	if (actor->dynamicFlags.bIsFalling) {
		_engine->_movements->processActorX = _engine->_movements->previousActorX;
		_engine->_movements->processActorY = _engine->_movements->previousActorY + _engine->loopActorStep; // add step to fall
		_engine->_movements->processActorZ = _engine->_movements->previousActorZ;
	}

	// actor collisions with bricks
	if (actor->staticFlags.bComputeCollisionWithBricks) {
		_engine->_collision->collisionY = 0;

		ShapeType brickShape = _engine->_grid->getBrickShape(_engine->_movements->previousActorX, _engine->_movements->previousActorY, _engine->_movements->previousActorZ);

		if (brickShape != ShapeType::kNone) {
			if (brickShape != ShapeType::kSolid) {
				_engine->_collision->reajustActorPosition(brickShape);
			} /*else { // this shouldn't happen (collision should avoid it)
				actor->y = processActorY = (processActorY / 256) * 256 + 256; // go upper
			}*/
		}

		if (actor->staticFlags.bComputeCollisionWithObj) {
			_engine->_collision->checkCollisionWithActors(actorIdx);
		}

		if (actor->standOn != -1 && actor->dynamicFlags.bIsFalling) {
			_engine->_collision->stopFalling();
		}

		_engine->_collision->causeActorDamage = 0;

		_engine->_collision->processCollisionX = _engine->_movements->processActorX;
		_engine->_collision->processCollisionY = _engine->_movements->processActorY;
		_engine->_collision->processCollisionZ = _engine->_movements->processActorZ;

		if (IS_HERO(actorIdx) && !actor->staticFlags.bComputeLowCollision) {
			// check hero collisions with bricks
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.x.bottomLeft, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.bottomLeft, 1);
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.x.topRight, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.bottomLeft, 2);
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.x.topRight, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.topRight, 4);
			_engine->_collision->checkHeroCollisionWithBricks(actor->boudingBox.x.bottomLeft, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.topRight, 8);
		} else {
			// check other actors collisions with bricks
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.x.bottomLeft, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.bottomLeft, 1);
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.x.topRight, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.bottomLeft, 2);
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.x.topRight, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.topRight, 4);
			_engine->_collision->checkActorCollisionWithBricks(actor->boudingBox.x.bottomLeft, actor->boudingBox.y.bottomLeft, actor->boudingBox.z.topRight, 8);
		}

		// process wall hit while running
		if (_engine->_collision->causeActorDamage && !actor->dynamicFlags.bIsFalling && !currentlyProcessedActorIdx && _engine->_actor->heroBehaviour == HeroBehaviourType::kAthletic && actor->anim == AnimationTypes::kForward) {
			_engine->_movements->rotateActor(actor->boudingBox.x.bottomLeft, actor->boudingBox.z.bottomLeft, actor->angle + ANGLE_360 + ANGLE_135);

			_engine->_renderer->destX += _engine->_movements->processActorX;
			_engine->_renderer->destZ += _engine->_movements->processActorZ;

			if (_engine->_renderer->destX >= 0 && _engine->_renderer->destZ >= 0 && _engine->_renderer->destX <= 0x7E00 && _engine->_renderer->destZ <= 0x7E00) {
				if (_engine->_grid->getBrickShape(_engine->_renderer->destX, _engine->_movements->processActorY + 256, _engine->_renderer->destZ) != ShapeType::kNone && _engine->cfgfile.WallCollision) { // avoid wall hit damage
					_engine->_extra->addExtraSpecial(actor->x, actor->y + 1000, actor->z, ExtraSpecialType::kHitStars);
					initAnim(AnimationTypes::kBigHit, 2, AnimationTypes::kStanding, currentlyProcessedActorIdx);

					if (IS_HERO(currentlyProcessedActorIdx)) {
						_engine->_movements->heroMoved = true;
					}

					actor->life--;
				}
			}
		}

		brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);
		actor->setBrickShape(brickShape);

		if (brickShape != ShapeType::kNone) {
			if (brickShape == ShapeType::kSolid) {
				if (actor->dynamicFlags.bIsFalling) {
					_engine->_collision->stopFalling();
					_engine->_movements->processActorY = (_engine->_collision->collisionY << 8) + 0x100;
				} else {
					if (IS_HERO(actorIdx) && _engine->_actor->heroBehaviour == HeroBehaviourType::kAthletic && actor->anim == AnimationTypes::kForward && _engine->cfgfile.WallCollision) { // avoid wall hit damage
						_engine->_extra->addExtraSpecial(actor->x, actor->y + 1000, actor->z, ExtraSpecialType::kHitStars);
						initAnim(AnimationTypes::kBigHit, 2, AnimationTypes::kStanding, currentlyProcessedActorIdx);
						_engine->_movements->heroMoved = true;
						actor->life--;
					}

					// no Z coordinate issue
					if (_engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->previousActorZ) == ShapeType::kNone) {
						_engine->_movements->processActorZ = _engine->_movements->previousActorZ;
					}

					// no X coordinate issue
					if (_engine->_grid->getBrickShape(_engine->_movements->previousActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ) == ShapeType::kNone) {
						_engine->_movements->processActorX = _engine->_movements->previousActorX;
					}

					// X and Z with issue, no move
					if (_engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->previousActorZ) != ShapeType::kNone &&
					    _engine->_grid->getBrickShape(_engine->_movements->previousActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ) != ShapeType::kNone) {
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
				brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY - 1, _engine->_movements->processActorZ);

				if (brickShape != ShapeType::kNone) {
					if (actor->dynamicFlags.bIsFalling) {
						_engine->_collision->stopFalling();
					}

					_engine->_collision->reajustActorPosition(brickShape);
				} else {
					if (!actor->dynamicFlags.bIsRotationByAnim) {
						actor->dynamicFlags.bIsFalling = 1;

						if (IS_HERO(actorIdx) && _engine->_scene->heroYBeforeFall == 0) {
							_engine->_scene->heroYBeforeFall = _engine->_movements->processActorY;
						}

						initAnim(AnimationTypes::kFall, 0, AnimationTypes::kAnimInvalid, actorIdx);
					}
				}
			}
		}

		// if under the map, than die
		if (_engine->_collision->collisionY == -1) {
			actor->life = 0;
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
	if (_engine->_movements->processActorX < 0) {
		_engine->_movements->processActorX = 0;
	}

	if (_engine->_movements->processActorY < 0) {
		_engine->_movements->processActorY = 0;
	}

	if (_engine->_movements->processActorZ < 0) {
		_engine->_movements->processActorZ = 0;
	}

	if (_engine->_movements->processActorX > 0x7E00) {
		_engine->_movements->processActorX = 0x7E00;
	}

	if (_engine->_movements->processActorZ > 0x7E00) {
		_engine->_movements->processActorZ = 0x7E00;
	}

	actor->x = _engine->_movements->processActorX;
	actor->y = _engine->_movements->processActorY;
	actor->z = _engine->_movements->processActorZ;
}

} // namespace TwinE
