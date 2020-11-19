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
#include "common/memstream.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/actor.h"
#include "twine/collision.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/movements.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/sound.h"
#include "twine/twine.h"

namespace TwinE {

static const int32 magicLevelStrengthOfHit[] = {
	kNoBallStrength,
	kYellowBallStrength,
	kGreenBallStrength,
	kRedBallStrength,
	kFireBallStrength,
	0
};

enum ActionType {
	ACTION_HITTING = 0,
	ACTION_SAMPLE = 1,
	ACTION_SAMPLE_FREQ = 2,
	ACTION_THROW_EXTRA_BONUS = 3,
	ACTION_THROW_MAGIC_BALL = 4,
	ACTION_SAMPLE_REPEAT = 5,
	ACTION_UNKNOWN_6 = 6,
	ACTION_UNKNOWN_7 = 7,
	ACTION_SAMPLE_STOP = 8,
	ACTION_UNKNOWN_9 = 9, // unused
	ACTION_SAMPLE_BRICK_1 = 10,
	ACTION_SAMPLE_BRICK_2 = 11,
	ACTION_HERO_HITTING = 12,
	ACTION_UNKNOWN_13 = 13,
	ACTION_UNKNOWN_14 = 14,
	ACTION_UNKNOWN_15 = 15,
	ACTION_LAST
};

Animations::Animations(TwinEEngine *engine) : _engine(engine) {
	animBuffer1 = animBuffer2 = (uint8 *)malloc(5000 * sizeof(uint8));
}

Animations::~Animations() {
	free(animBuffer1);
}

int32 Animations::setAnimAtKeyframe(int32 keyframeIdx, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr) {
	const int16 numOfKeyframeInAnim = READ_LE_INT16(animPtr);
	if (keyframeIdx >= numOfKeyframeInAnim) {
		return numOfKeyframeInAnim;
	}

	int16 numOfBonesInAnim = READ_LE_INT16(animPtr + 2);

	const uint8 *ptrToData = (const uint8 *)((numOfBonesInAnim * 8 + 8) * keyframeIdx + animPtr + 8);

	const int16 bodyHeader = READ_LE_INT16(bodyPtr);

	if (!(bodyHeader & 2)) {
		return 0;
	}

	uint8 *ptrToBodyData = bodyPtr + 14;

	animTimerDataPtr->ptr = ptrToData;
	animTimerDataPtr->time = _engine->lbaTime;

	ptrToBodyData = ptrToBodyData + READ_LE_INT16(ptrToBodyData) + 2;

	const int16 numOfElementInBody = READ_LE_INT16(ptrToBodyData);

	ptrToBodyData = ptrToBodyData + numOfElementInBody * 6 + 12;

	const int16 numOfPointInBody = READ_LE_INT16(ptrToBodyData - 10); // num elements

	if (numOfBonesInAnim > numOfPointInBody) {
		numOfBonesInAnim = numOfPointInBody;
	}

	const uint8 *ptrToDataBackup = ptrToData;

	ptrToData += 8;

	do {
		for (int32 i = 0; i < 8; i++) {
			*(ptrToBodyData++) = *(ptrToData++);
		}

		ptrToBodyData += 30;

	} while (--numOfBonesInAnim);

	ptrToData = ptrToDataBackup + 2;

	currentStepX = READ_LE_INT16(ptrToData);
	currentStepY = READ_LE_INT16(ptrToData + 2);
	currentStepZ = READ_LE_INT16(ptrToData + 4);

	processRotationByAnim = READ_LE_INT16(ptrToData + 6);
	processLastRotationAngle = ToAngle(READ_LE_INT16(ptrToData + 10));

	return 1;
}

int32 Animations::getNumKeyframes(const uint8 *animPtr) {
	return READ_LE_INT16(animPtr);
}

int32 Animations::getStartKeyframe(const uint8 *animPtr) {
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
	int32 numOfPointInAnim = READ_LE_INT16(animPtr + 2);

	const uint8* keyFramePtr = ((numOfPointInAnim * 8 + 8) * animState) + animPtr + 8;

	int32 keyFrameLength = READ_LE_INT16(keyFramePtr);

	int16 bodyHeader = READ_LE_INT16(bodyPtr);

	if (!(bodyHeader & 2)) {
		return false;
	}

	uint8 *edi = bodyPtr + 16;

	const uint8 *ebx = animTimerDataPtr->ptr;
	int32 ebp = animTimerDataPtr->time;

	if (!ebx) {
		ebx = keyFramePtr;
		ebp = keyFrameLength;
	}

	const uint8* lastKeyFramePtr = ebx;

	int32 eax = READ_LE_INT16(edi - 2);
	edi += eax;

	eax = READ_LE_INT16(edi);
	eax = eax + eax * 2;
	edi = edi + eax * 2 + 12;

	int32 numOfPointInBody = READ_LE_INT16(edi - 10);

	if (numOfPointInAnim > numOfPointInBody) {
		numOfPointInAnim = numOfPointInBody;
	}

	eax = _engine->lbaTime - ebp;

	if (eax >= keyFrameLength) {
		const int32 *sourcePtr = (const int32 *)(keyFramePtr + 8);
		int32 *destPtr = (int32 *)edi; // keyframe

		do {
			*(destPtr++) = *(sourcePtr++);
			*(destPtr++) = *(sourcePtr++);
			destPtr = (int32 *)(((int8 *)destPtr) + 30);
		} while (--numOfPointInAnim);

		animTimerDataPtr->ptr = keyFramePtr;
		animTimerDataPtr->time = _engine->lbaTime;

		currentStepX = READ_LE_INT16(keyFramePtr + 2);
		currentStepY = READ_LE_INT16(keyFramePtr + 4);
		currentStepZ = READ_LE_INT16(keyFramePtr + 6);

		processRotationByAnim = READ_LE_INT16(keyFramePtr + 8);
		processLastRotationAngle = ToAngle(READ_LE_INT16(keyFramePtr + 12));

		return true;
	}
	const uint8 *keyFramePtrOld = keyFramePtr;

	lastKeyFramePtr += 8;
	keyFramePtr += 8;

	processRotationByAnim = READ_LE_INT16(keyFramePtr);
	processLastRotationAngle = ToAngle((READ_LE_INT16(keyFramePtr + 4) * eax) / keyFrameLength);

	lastKeyFramePtr += 8;
	keyFramePtr += 8;

	edi += 38;

	if (--numOfPointInAnim) {
		int16 tmpNumOfPoints = numOfPointInAnim;

		do {
			int16 animOpcode = getAnimMode(&edi, &keyFramePtr, &lastKeyFramePtr);

			switch (animOpcode) {
			case 0:  // allow global rotate
				applyAnimStepRotation(&edi, eax, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStepRotation(&edi, eax, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStepRotation(&edi, eax, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				break;
			case 1:  // dissallow global rotate
			case 2:  // dissallow global rotate + hide
				applyAnimStep(&edi, eax, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStep(&edi, eax, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				applyAnimStep(&edi, eax, keyFrameLength, &keyFramePtr, &lastKeyFramePtr);
				break;
			default:
				error("Unsupported animation rotation mode %d", animOpcode);
			}

			edi += 30;
		} while (--tmpNumOfPoints);
	}

	currentStepX = (READ_LE_INT16(keyFramePtrOld + 2) * eax) / keyFrameLength;
	currentStepY = (READ_LE_INT16(keyFramePtrOld + 4) * eax) / keyFrameLength;
	currentStepZ = (READ_LE_INT16(keyFramePtrOld + 6) * eax) / keyFrameLength;

	return false;
}

int32 Animations::getBodyAnimIndex(AnimationTypes animIdx, int32 actorIdx) {
	uint8 *costumePtr = nullptr;

	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	uint8 *bodyPtr = actor->entityDataPtr;

	do {
		int8 type = *(bodyPtr++);

		if (type == -1) {
			currentActorAnimExtraPtr = nullptr;
			return -1;
		}

		uint8 *ptr = (bodyPtr + 1);

		if (type == 3) {
			if (animIdx == (AnimationTypes)*bodyPtr) {
				ptr++;
				uint16 realAnimIdx = READ_LE_INT16(ptr);
				ptr += 2;
				uint8 *ptr2 = ptr;
				ptr++;
				if (*ptr2 != 0) {
					costumePtr = ptr - 1;
				}
				currentActorAnimExtraPtr = costumePtr;
				return realAnimIdx;
			}
		}

		bodyPtr = *ptr + ptr;

	} while (1);

	return 0;
}

int32 Animations::stockAnimation(uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr) {
	uint8 *animPtr = animBuffer2;
	int32 playAnim = READ_LE_INT16(bodyPtr);

	if (!(playAnim & 2)) {
		return 0;
	}
	const uint8 *ptr = (bodyPtr + 0x10);

	animTimerDataPtr->time = _engine->lbaTime;
	animTimerDataPtr->ptr = animPtr;

	int32 var0 = READ_LE_INT16(ptr - 2);
	ptr = ptr + var0;

	int32 var1 = READ_LE_INT16(ptr);
	var1 = var1 + var1 * 2;

	ptr = ptr + var1 * 2 + 2;

	int32 var2 = READ_LE_INT16(ptr);
	int32 counter = var2;
	var2 = (var2 * 8) + 8;

	int32 *edi = (int32 *)(animPtr + 8);
	const int32 *esi = (const int32 *)(ptr + 10);

	do {
		*(edi++) = *(esi++);
		*(edi++) = *(esi++);

		esi = (const int32 *)(((const int8 *)esi) + 30);
	} while (counter--);

	animBuffer2 += var2;

	if (animBuffer1 + 4488 < animBuffer2) {
		animBuffer2 = animBuffer1;
	}

	return var2;
}

int32 Animations::verifyAnimAtKeyframe(int32 animIdx, uint8 *animPtr, uint8 *bodyPtr, AnimTimerDataStruct *animTimerDataPtr) {
	const int32 numOfPointInAnim = READ_LE_INT16(animPtr + 2);
	const uint8 *keyFramePtr = ((numOfPointInAnim * 8 + 8) * animIdx) + animPtr + 8;
	const int32 keyFrameLength = READ_LE_INT16(keyFramePtr);
	const int16 bodyHeader = READ_LE_INT16(bodyPtr);
	if (!(bodyHeader & 2)) {
		return 0;
	}

	const uint8 *ebx = animTimerDataPtr->ptr;
	int32 ebp = animTimerDataPtr->time;

	if (!ebx) {
		ebx = keyFramePtr;
		ebp = keyFrameLength;
	}

	const uint8* lastKeyFramePtr = ebx;

	const int32 eax = _engine->lbaTime - ebp;

	if (eax >= keyFrameLength) {
		animTimerDataPtr->ptr = keyFramePtr;
		animTimerDataPtr->time = _engine->lbaTime;

		currentStepX = READ_LE_INT16(keyFramePtr + 2);
		currentStepY = READ_LE_INT16(keyFramePtr + 4);
		currentStepZ = READ_LE_INT16(keyFramePtr + 6);

		processRotationByAnim = READ_LE_INT16(keyFramePtr + 8);
		processLastRotationAngle = ToAngle(READ_LE_INT16(keyFramePtr + 12));

		return 1;
	}
	const uint8 *keyFramePtrOld = keyFramePtr;

	lastKeyFramePtr += 8;
	keyFramePtr += 8;

	processRotationByAnim = READ_LE_INT16(keyFramePtr);
	processLastRotationAngle = ToAngle((READ_LE_INT16(keyFramePtr + 4) * eax) / keyFrameLength);

	lastKeyFramePtr += 8;
	keyFramePtr += 8;

	currentStepX = (READ_LE_INT16(keyFramePtrOld + 2) * eax) / keyFrameLength;
	currentStepY = (READ_LE_INT16(keyFramePtrOld + 4) * eax) / keyFrameLength;
	currentStepZ = (READ_LE_INT16(keyFramePtrOld + 6) * eax) / keyFrameLength;

	return 0;
}

void Animations::processAnimActions(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->animExtraPtr == nullptr) {
		return; // avoid null pointers
	}

	Common::MemoryReadStream stream(actor->animExtraPtr, 1000000);

	int32 index = 0;
	const int32 endAnimEntityIdx = stream.readByte();
	while (index++ < endAnimEntityIdx) {
		const int32 actionType = stream.readByte() - 5;
		if (actionType >= ACTION_LAST) {
			return;
		}

		switch (actionType) {
		case ACTION_HITTING: {
			const int32 animPos = stream.readByte() - 1;
			const int32 strength = stream.readByte();

			if (animPos == actor->animPosition) {
				actor->strengthOfHit = strength;
				actor->dynamicFlags.bIsHitting = 1;
			}
			break;
		}
		case ACTION_SAMPLE: {
			const int32 animPos = stream.readByte();
			const int16 sampleIdx = stream.readSint16LE();

			if (animPos == actor->animPosition) {
				_engine->_sound->playSample(sampleIdx, 4096, 1, actor->x, actor->y, actor->z, actorIdx);
			}
			break;
		}
		case ACTION_SAMPLE_FREQ: {
			const int32 animPos = stream.readByte();
			const int16 sampleIdx = stream.readSint16LE();
			int16 frequency = stream.readSint16LE();

			if (animPos == actor->animPosition) {
				frequency = _engine->getRandomNumber(frequency) + 4096 - (ABS(frequency) >> 1);
				_engine->_sound->playSample(sampleIdx, frequency, 1, actor->x, actor->y, actor->z, actorIdx);
			}
			break;
		}
		case ACTION_THROW_EXTRA_BONUS: {
			const int32 animPos = stream.readByte();
			const int32 yHeight = stream.readSint16LE();
			const int32 sprite = stream.readByte();
			const int32 xAngle = ToAngle(stream.readSint16LE());
			const int32 yAngle = actor->angle + ToAngle(stream.readSint16LE());
			const int32 xRotPoint = stream.readSint16LE();
			const int32 extraAngle = ToAngle(stream.readByte());
			const int32 strengthOfHit = stream.readByte();

			if (animPos == actor->animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->x, actor->y + yHeight, actor->z, sprite, xAngle, yAngle, xRotPoint, extraAngle, strengthOfHit);
			}
			break;
		}
		case ACTION_THROW_MAGIC_BALL: {
			const int32 animPos = stream.readByte();
			const int32 yOffset = stream.readSint16LE();
			const int32 xAngle = ToAngle(stream.readSint16LE());
			const int32 xRotPoint = stream.readSint16LE();
			const int32 extraAngle = stream.readByte();

			if (_engine->_gameState->magicBallIdx == -1 && animPos == actor->animPosition) {
				_engine->_extra->addExtraThrowMagicball(actor->x, actor->y + yOffset, actor->z, xAngle, actor->angle, xRotPoint, extraAngle);
			}
			break;
		}
		case ACTION_SAMPLE_REPEAT: {
			const int32 animPos = stream.readByte();
			const int16 sampleIdx = stream.readSint16LE();
			const int16 repeat = stream.readSint16LE();

			if (animPos == actor->animPosition) {
				_engine->_sound->playSample(sampleIdx, 0x1000, repeat, actor->x, actor->y, actor->z, actorIdx);
			}
			break;
		}
		case ACTION_UNKNOWN_6: {
			const int32 animPos = stream.readByte();
			if (animPos == actor->animPosition) {
				// TODO: The folowing fetches 7 bytes, but the else block skips only 6 bytes.
				// Please check if that's correct.
				const int32 yOffset = stream.readSint16LE();
				const int32 spriteIdx = stream.readByte();
				const int32 targetActorIdx = stream.readByte();
				const int32 maxSpeed = stream.readSint16LE();
				const int32 strengthOfHit = stream.readByte();

				_engine->_extra->addExtraAiming(actorIdx, actor->x, actor->y + yOffset, actor->z, spriteIdx, targetActorIdx, maxSpeed, strengthOfHit);
			} else {
				stream.skip(6);
			}
			break;
		}
		case ACTION_UNKNOWN_7: {
			const int32 animPos = stream.readByte();
			const int32 yHeight = stream.readSint16LE();
			const int32 spriteIdx = stream.readByte();
			const int32 xAngle = ToAngle(stream.readSint16LE());
			const int32 yAngle = actor->angle + ToAngle(stream.readSint16LE());
			const int32 xRotPoint = stream.readSint16LE();
			const int32 extraAngle = ToAngle(stream.readByte());
			const int32 strengthOfHit = stream.readByte();

			if (animPos == actor->animPosition) {
				_engine->_extra->addExtraThrow(actorIdx, actor->x, actor->y + yHeight, actor->z, spriteIdx, xAngle, yAngle, xRotPoint, extraAngle, strengthOfHit);
			}
			break;
		}
		case ACTION_SAMPLE_STOP: {
			const int32 animPos = stream.readByte();
			const int32 sampleIdx = stream.readByte(); //why is it reading a byte but saving it in a 32bit variable?
			stream.skip(1);               // TODO what is the meaning of this extra byte?

			if (animPos == actor->animPosition) {
				_engine->_sound->stopSample(sampleIdx);
			}
			break;
		}
		case ACTION_SAMPLE_BRICK_1: {
			const int32 animPos = stream.readByte();
			if (animPos == actor->animPosition && (actor->brickSound & 0x0F0) != 0x0F0) {
				const int16 sampleIdx = (actor->brickSound & 0x0F) + Samples::WalkFloorBegin;
				_engine->_sound->playSample(sampleIdx, _engine->getRandomNumber(1000) + 3596, 1, actor->x, actor->y, actor->z, actorIdx);
			}
			break;
		}
		case ACTION_SAMPLE_BRICK_2: {
			const int32 animPos = stream.readByte();
			if (animPos == actor->animPosition && (actor->brickSound & 0x0F0) != 0x0F0) {
				const int16 sampleIdx = (actor->brickSound & 0x0F) + Samples::WalkFloorBegin;
				_engine->_sound->playSample(sampleIdx, _engine->getRandomNumber(1000) + 3596, 1, actor->x, actor->y, actor->z, actorIdx);
			}
			break;
		}
		case ACTION_HERO_HITTING: {
			const int32 animPos = stream.readByte() - 1;
			if (animPos == actor->animPosition) {
				actor->strengthOfHit = magicLevelStrengthOfHit[_engine->_gameState->magicLevelIdx];
				actor->dynamicFlags.bIsHitting = 1;
			}
			break;
		}
		case ACTION_UNKNOWN_13: {
			const int32 animPos = stream.readByte();
			const int32 distanceX = stream.readSint16LE();
			const int32 distanceY = stream.readSint16LE();
			const int32 distanceZ = stream.readSint16LE();
			const int32 spriteIdx = stream.readByte();
			const int32 xAngle = ToAngle(stream.readSint16LE());
			const int32 yAngle = ToAngle(stream.readSint16LE());
			const int32 xRotPoint = stream.readSint16LE();
			const int32 extraAngle = ToAngle(stream.readByte());
			const int32 strength = stream.readByte();

			if (animPos == actor->animPosition) {
				_engine->_movements->rotateActor(distanceX, distanceZ, actor->angle);

				const int32 throwX = _engine->_renderer->destX + actor->x;
				const int32 throwY = distanceY + actor->y;
				const int32 throwZ = _engine->_renderer->destZ + actor->z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, spriteIdx,
				                               xAngle, yAngle + actor->angle, xRotPoint, extraAngle, strength);
			}
			break;
		}
		case ACTION_UNKNOWN_14: {
			const int32 animPos = stream.readByte();
			const int32 distanceX = stream.readSint16LE();
			const int32 distanceY = stream.readSint16LE();
			const int32 distanceZ = stream.readSint16LE();
			const int32 spriteIdx = stream.readByte();
			const int32 xAngle = ToAngle(stream.readSint16LE());
			const int32 yAngle = ToAngle(stream.readSint16LE());
			const int32 xRotPoint = stream.readSint16LE();
			const int32 extraAngle = ToAngle(stream.readByte());
			const int32 strength = stream.readByte();

			if (animPos == actor->animPosition) {
				const int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->y, 0, _engine->_scene->sceneHero->y, _engine->_movements->getDistance2D(actor->x, actor->z, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->z));

				_engine->_movements->rotateActor(distanceX, distanceZ, actor->angle);

				const int32 throwX = _engine->_renderer->destX + actor->x;
				const int32 throwY = distanceY + actor->y;
				const int32 throwZ = _engine->_renderer->destZ + actor->z;

				_engine->_extra->addExtraThrow(actorIdx, throwX, throwY, throwZ, spriteIdx,
				                               xAngle + newAngle, yAngle + actor->angle, xRotPoint, extraAngle, strength);
			}
			break;
		}
		case ACTION_UNKNOWN_15: {
			const int32 animPos = stream.readByte();
			const int32 distanceX = stream.readSint16LE();
			const int32 distanceY = stream.readSint16LE();
			const int32 distanceZ = stream.readSint16LE();
			const int32 spriteIdx = stream.readByte();
			const int32 targetActor = stream.readByte();
			const int32 finalAngle = ToAngle(stream.readSint16LE());
			const int32 strengthOfHit = stream.readByte();

			if (animPos == actor->animPosition) {
				_engine->_movements->rotateActor(distanceX, distanceZ, actor->angle);
				_engine->_extra->addExtraAiming(actorIdx, actor->x + _engine->_renderer->destX, actor->y + distanceY, actor->z + distanceZ, spriteIdx,
				                                targetActor, finalAngle, strengthOfHit);
			}
			break;
		}
		case ACTION_UNKNOWN_9:
			break;
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

	if (actor->previousAnimIdx == -1) { // if no previous animation
		setAnimAtKeyframe(0, _engine->_resources->animTable[animIndex], _engine->_actor->bodyTable[actor->entity], &actor->animTimerData);
	} else { // interpolation between animations
		animBuffer2 += stockAnimation(_engine->_actor->bodyTable[actor->entity], &actor->animTimerData);
		if (animBuffer1 + 4488 < animBuffer2) {
			animBuffer2 = animBuffer1;
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

	if (actor->animExtraPtr) {
		processAnimActions(actorIdx);
	}

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
				int32 xAxisRotation = _engine->_movements->getRealValue(&actor->move);
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
			uint8 *animPtr = _engine->_resources->animTable[actor->previousAnimIdx];

			int32 keyFramePassed = verifyAnimAtKeyframe(actor->animPosition, animPtr, _engine->_actor->bodyTable[actor->entity], &actor->animTimerData);

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
				if (actor->animExtraPtr) {
					processAnimActions(actorIdx);
				}

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

					if (actor->animExtraPtr) {
						processAnimActions(actorIdx);
					}

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
