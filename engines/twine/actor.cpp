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

#include "twine/actor.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "twine/animations.h"
#include "twine/extra.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/hqrdepack.h"
#include "twine/movements.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/twine.h"

namespace TwinE {

Actor::Actor(TwinEEngine *engine) : _engine(engine) {
}

void Actor::restartHeroScene() {
	_engine->_scene->sceneHero->controlMode = 1;
	memset(&_engine->_scene->sceneHero->dynamicFlags, 0, sizeof(_engine->_scene->sceneHero->dynamicFlags));
	memset(&_engine->_scene->sceneHero->staticFlags, 0, sizeof(_engine->_scene->sceneHero->staticFlags));

	_engine->_scene->sceneHero->staticFlags.bComputeCollisionWithObj = 1;
	_engine->_scene->sceneHero->staticFlags.bComputeCollisionWithBricks = 1;
	_engine->_scene->sceneHero->staticFlags.bIsZonable = 1;
	_engine->_scene->sceneHero->staticFlags.bCanDrown = 1;
	_engine->_scene->sceneHero->staticFlags.bCanFall = 1;

	_engine->_scene->sceneHero->armor = 1;
	_engine->_scene->sceneHero->positionInMoveScript = -1;
	_engine->_scene->sceneHero->labelIdx = -1;
	_engine->_scene->sceneHero->positionInLifeScript = 0;
	_engine->_scene->sceneHero->zone = -1;
	_engine->_scene->sceneHero->angle = previousHeroAngle;

	_engine->_movements->setActorAngleSafe(_engine->_scene->sceneHero->angle, _engine->_scene->sceneHero->angle, 0, &_engine->_scene->sceneHero->move);
	setBehaviour(previousHeroBehaviour);

	cropBottomScreen = 0;
}

void Actor::loadHeroEntities() {
	_engine->_hqrdepack->hqrGetallocEntry(&heroEntityATHLETIC, Resources::HQR_FILE3D_FILE, FILE3DHQR_HEROATHLETIC);
	_engine->_scene->sceneHero->entityDataPtr = heroEntityATHLETIC;
	heroAnimIdxATHLETIC = _engine->_animations->getBodyAnimIndex(0, 0);

	_engine->_hqrdepack->hqrGetallocEntry(&heroEntityAGGRESSIVE, Resources::HQR_FILE3D_FILE, FILE3DHQR_HEROAGGRESSIVE);
	_engine->_scene->sceneHero->entityDataPtr = heroEntityAGGRESSIVE;
	heroAnimIdxAGGRESSIVE = _engine->_animations->getBodyAnimIndex(0, 0);

	_engine->_hqrdepack->hqrGetallocEntry(&heroEntityDISCRETE, Resources::HQR_FILE3D_FILE, FILE3DHQR_HERODISCRETE);
	_engine->_scene->sceneHero->entityDataPtr = heroEntityDISCRETE;
	heroAnimIdxDISCRETE = _engine->_animations->getBodyAnimIndex(0, 0);

	_engine->_hqrdepack->hqrGetallocEntry(&heroEntityPROTOPACK, Resources::HQR_FILE3D_FILE, FILE3DHQR_HEROPROTOPACK);
	_engine->_scene->sceneHero->entityDataPtr = heroEntityPROTOPACK;
	heroAnimIdxPROTOPACK = _engine->_animations->getBodyAnimIndex(0, 0);

	_engine->_hqrdepack->hqrGetallocEntry(&heroEntityNORMAL, Resources::HQR_FILE3D_FILE, FILE3DHQR_HERONORMAL);
	_engine->_scene->sceneHero->entityDataPtr = heroEntityNORMAL;
	heroAnimIdxNORMAL = _engine->_animations->getBodyAnimIndex(0, 0);

	_engine->_scene->sceneHero->animExtraPtr = _engine->_animations->currentActorAnimExtraPtr;
}

void Actor::setBehaviour(int32 behaviour) {
	switch (behaviour) {
	case kNormal:
		heroBehaviour = kNormal;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityNORMAL;
		break;
	case kAthletic:
		heroBehaviour = kAthletic;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityATHLETIC;
		break;
	case kAggressive:
		heroBehaviour = kAggressive;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityAGGRESSIVE;
		break;
	case kDiscrete:
		heroBehaviour = kDiscrete;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityDISCRETE;
		break;
	case kProtoPack:
		heroBehaviour = kProtoPack;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityPROTOPACK;
		break;
	};

	const int32 bodyIdx = _engine->_scene->sceneHero->body;

	_engine->_scene->sceneHero->entity = -1;
	_engine->_scene->sceneHero->body = -1;

	initModelActor(bodyIdx, 0);

	_engine->_scene->sceneHero->anim = kAnimNone;
	_engine->_scene->sceneHero->animType = 0;

	_engine->_animations->initAnim(kStanding, 0, 255, 0);
}

void Actor::initSpriteActor(int32 actorIdx) {
	ActorStruct *localActor = &_engine->_scene->sceneActors[actorIdx];

	if (localActor->staticFlags.bIsSpriteActor && localActor->sprite != -1 && localActor->entity != localActor->sprite) {
		const int16 *ptr = (const int16 *)(_engine->_scene->spriteBoundingBoxPtr + localActor->sprite * 16 + 4);

		localActor->entity = localActor->sprite;
		localActor->boudingBox.x.bottomLeft = *(ptr++);
		localActor->boudingBox.x.topRight = *(ptr++);
		localActor->boudingBox.y.bottomLeft = *(ptr++);
		localActor->boudingBox.y.topRight = *(ptr++);
		localActor->boudingBox.z.bottomLeft = *(ptr++);
		localActor->boudingBox.z.topRight = *(ptr++);
	}
}

int32 Actor::initBody(int32 bodyIdx, int32 actorIdx) {
	ActorStruct *localActor = &_engine->_scene->sceneActors[actorIdx];
	uint8 *bodyPtr = localActor->entityDataPtr;

	do {
		uint8 var1 = *(bodyPtr++);

		if (var1 == 0xFF) {
			return -1;
		}

		uint8 *bodyPtr2 = bodyPtr + 1;

		if (var1 == 1) {
			uint8 var2 = *(bodyPtr);

			if (var2 == bodyIdx) {
				int32 index;
				uint8 *bodyPtr3 = bodyPtr2 + 1;
				int16 flag = *((uint16 *)bodyPtr3);

				if (!(flag & 0x8000)) {
					_engine->_hqrdepack->hqrGetallocEntry(&bodyTable[currentPositionInBodyPtrTab], Resources::HQR_BODY_FILE, flag & 0xFFFF);

					if (!bodyTable[currentPositionInBodyPtrTab]) {
						error("HQR ERROR: Loading body entities");
					}
					_engine->_renderer->prepareIsoModel(bodyTable[currentPositionInBodyPtrTab]);
					*((uint16 *)bodyPtr3) = currentPositionInBodyPtrTab + 0x8000;
					index = currentPositionInBodyPtrTab;
					currentPositionInBodyPtrTab++;
				} else {
					flag &= 0x7FFF;
					index = flag;
				}

				bodyPtr3 += 2;
				bottomLeftX = -32000;

				uint8 *bodyPtr4 = bodyPtr3;
				bodyPtr3++;

				if (!*bodyPtr4) {
					return index;
				}

				bodyPtr4 = bodyPtr3;
				bodyPtr3++;

				if (*bodyPtr4 != 14) {
					return index;
				}

				// bodyPtr5 = (int16 *) bodyPtr3;

				bottomLeftX = *((uint16 *)bodyPtr3);
				bodyPtr3 += 2;
				bottomLeftY = *((uint16 *)bodyPtr3);
				bodyPtr3 += 2;
				bottomLeftZ = *((uint16 *)bodyPtr3);
				bodyPtr3 += 2;

				topRightX = *((uint16 *)bodyPtr3);
				bodyPtr3 += 2;
				topRightY = *((uint16 *)bodyPtr3);
				bodyPtr3 += 2;
				topRightZ = *((uint16 *)bodyPtr3);
				bodyPtr3 += 2;

				return index;
			}
		}

		bodyPtr = *bodyPtr2 + bodyPtr2;
	} while (1);
}

void Actor::initModelActor(int32 bodyIdx, int16 actorIdx) {
	ActorStruct *localActor = &_engine->_scene->sceneActors[actorIdx];
	if (localActor->staticFlags.bIsSpriteActor) {
		return;
	}

	if (actorIdx == 0 && heroBehaviour == kProtoPack && localActor->armor != 0 && localActor->armor != 1) { // if hero
		setBehaviour(kNormal);
	}

	int32 entityIdx;
	if (bodyIdx != -1) {
		entityIdx = initBody(bodyIdx, actorIdx);
	} else {
		entityIdx = -1;
	}

	if (entityIdx != -1) {
		if (localActor->entity == entityIdx) {
			return;
		}

		localActor->entity = entityIdx;
		localActor->body = bodyIdx;
		int currentIndex = localActor->entity;

		if (bottomLeftX == -32000) {
			uint16 *ptr = (uint16 *)bodyTable[localActor->entity];
			ptr++;

			int16 var1 = *((int16 *)ptr++);
			int16 var2 = *((int16 *)ptr++);
			localActor->boudingBox.y.bottomLeft = *((int16 *)ptr++);
			localActor->boudingBox.y.topRight = *((int16 *)ptr++);
			int16 var3 = *((int16 *)ptr++);
			int16 var4 = *((int16 *)ptr++);

			int32 result = 0;
			if (localActor->staticFlags.bUseMiniZv) {
				int32 result1 = var2 - var1; // take smaller for bound
				int32 result2 = var4 - var3;

				result = MIN(result1, result2);

				result = ABS(result);
				result >>= 1;
			} else {
				int32 result1 = var2 - var1; // take average for bound
				int32 result2 = var4 - var3;

				result = result2 + result1;
				result = ABS(result);
				result >>= 2;
			}

			localActor->boudingBox.x.bottomLeft = -result;
			localActor->boudingBox.x.topRight = result;
			localActor->boudingBox.z.bottomLeft = -result;
			localActor->boudingBox.z.topRight = result;
		} else {
			localActor->boudingBox.x.bottomLeft = bottomLeftX;
			localActor->boudingBox.x.topRight = topRightX;
			localActor->boudingBox.y.bottomLeft = bottomLeftY;
			localActor->boudingBox.y.topRight = topRightY;
			localActor->boudingBox.z.bottomLeft = bottomLeftZ;
			localActor->boudingBox.z.topRight = topRightZ;
		}

		if (currentIndex == -1)
			return;

		if (localActor->previousAnimIdx == -1)
			return;

		_engine->_renderer->copyActorInternAnim(bodyTable[currentIndex], bodyTable[localActor->entity]);

		return;
	}

	localActor->body = -1;
	localActor->entity = -1;

	localActor->boudingBox.x.bottomLeft = 0;
	localActor->boudingBox.x.topRight = 0;
	localActor->boudingBox.y.bottomLeft = 0;
	localActor->boudingBox.y.topRight = 0;
	localActor->boudingBox.z.bottomLeft = 0;
	localActor->boudingBox.z.topRight = 0;
}

void Actor::initActor(int16 actorIdx) {
	ActorStruct *actor = &_engine->_scene->sceneActors[actorIdx];

	if (actor->staticFlags.bIsSpriteActor) { // if sprite actor
		if (actor->strengthOfHit != 0) {
			actor->dynamicFlags.bIsHitting = 1;
		}

		actor->entity = -1;

		initSpriteActor(actorIdx);

		_engine->_movements->setActorAngleSafe(0, 0, 0, &actor->move);

		if (actor->staticFlags.bUsesClipping) {
			actor->lastX = actor->x;
			actor->lastY = actor->y;
			actor->lastZ = actor->z;
		}
	} else {
		actor->entity = -1;

		initModelActor(actor->body, actorIdx);

		actor->previousAnimIdx = -1;
		actor->animType = 0;

		if (actor->entity != -1) {
			_engine->_animations->initAnim(actor->anim, 0, 255, actorIdx);
		}

		_engine->_movements->setActorAngleSafe(actor->angle, actor->angle, 0, &actor->move);
	}

	actor->positionInMoveScript = -1;
	actor->labelIdx = -1;
	actor->positionInLifeScript = 0;
}

void Actor::resetActor(int16 actorIdx) {
	ActorStruct *actor = &_engine->_scene->sceneActors[actorIdx];

	actor->body = 0;
	actor->anim = kStanding;
	actor->x = 0;
	actor->y = -1;
	actor->z = 0;

	actor->boudingBox.x.bottomLeft = 0;
	actor->boudingBox.x.topRight = 0;
	actor->boudingBox.y.bottomLeft = 0;
	actor->boudingBox.y.topRight = 0;
	actor->boudingBox.z.bottomLeft = 0;
	actor->boudingBox.z.topRight = 0;

	actor->angle = 0;
	actor->speed = 40;
	actor->controlMode = 0;

	actor->info0 = 0;
	actor->info1 = 0;
	actor->info2 = 0;
	actor->info3 = 0;

	actor->brickShape = 0;
	actor->collision = -1;
	actor->standOn = -1;
	actor->zone = -1;

	memset(&actor->staticFlags, 0, sizeof(StaticFlagsStruct));
	memset(&actor->dynamicFlags, 0, sizeof(DynamicFlagsStruct));

	actor->life = 50;
	actor->armor = 1;
	actor->hitBy = -1;
	actor->lastRotationAngle = 0;
	actor->lastX = 0;
	actor->lastY = 0;
	actor->lastZ = 0;
	actor->entity = -1;
	actor->previousAnimIdx = -1;
	actor->animType = 0;
	actor->animPosition = 0;

	_engine->_movements->setActorAngleSafe(0, 0, 0, &actor->move);

	actor->positionInMoveScript = -1;
	actor->positionInLifeScript = 0;
}

void Actor::hitActor(int32 actorIdx, int32 actorIdxAttacked, int32 strengthOfHit, int32 angle) {
	ActorStruct *actor = &_engine->_scene->sceneActors[actorIdxAttacked];

	if (actor->life <= 0) {
		return;
	}

	actor->hitBy = actorIdx;

	if (actor->armor <= strengthOfHit) {
		if (actor->anim == kBigHit || actor->anim == kHit2) {
			const int32 tmpAnimPos = actor->animPosition;
			if (actor->animExtra) {
				_engine->_animations->processAnimActions(actorIdxAttacked);
			}

			actor->animPosition = tmpAnimPos;
		} else {
			if (angle != -1) {
				_engine->_movements->setActorAngleSafe(angle, angle, 0, &actor->move);
			}

			if (_engine->getRandomNumber() & 1) {
				_engine->_animations->initAnim(kHit2, 3, 255, actorIdxAttacked);
			} else {
				_engine->_animations->initAnim(kBigHit, 3, 255, actorIdxAttacked);
			}
		}

		_engine->_extra->addExtraSpecial(actor->x, actor->y + 1000, actor->z, kHitStars);

		if (!actorIdxAttacked) {
			_engine->_movements->heroMoved = 1;
		}

		actor->life -= strengthOfHit;

		if (actor->life < 0) {
			actor->life = 0;
		}
	} else {
		_engine->_animations->initAnim(kHit, 3, 255, actorIdxAttacked);
	}
}

void Actor::processActorCarrier(int32 actorIdx) { // CheckCarrier
	ActorStruct *actor = &_engine->_scene->sceneActors[actorIdx];
	if (!actor->staticFlags.bIsCarrierActor) {
		return;
	}
	for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
		if (actor->standOn == actorIdx) {
			actor->standOn = -1;
		}
	}
}

void Actor::processActorExtraBonus(int32 actorIdx) { // GiveExtraBonus
	ActorStruct *actor = &_engine->_scene->sceneActors[actorIdx];

	int32 numBonus = 0;

	int8 bonusTable[8];
	for (int32 a = 0; a < 5; a++) {
		if (actor->bonusParameter & (1 << (a + 4))) {
			bonusTable[numBonus++] = a;
		}
	}

	if (numBonus == 0) {
		return;
	}

	int8 currentBonus = bonusTable[_engine->getRandomNumber(numBonus)];
	// if bonus is magic an no magic level yet, then give life points
	if (!_engine->_gameState->magicLevelIdx && currentBonus == 2) {
		currentBonus = 1;
	}
	currentBonus += 3;

	if (actor->dynamicFlags.bIsDead) {
		_engine->_extra->addExtraBonus(actor->x, actor->y, actor->z, 0x100, 0, currentBonus, actor->bonusAmount);
		// FIXME add constant for sample index
		_engine->_sound->playSample(11, 0x1000, 1, actor->x, actor->y, actor->z, actorIdx);
	} else {
		int32 angle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->z);
		_engine->_extra->addExtraBonus(actor->x, actor->y + actor->boudingBox.y.topRight, actor->z, 200, angle, currentBonus, actor->bonusAmount);
		// FIXME add constant for sample index
		_engine->_sound->playSample(11, 0x1000, 1, actor->x, actor->y + actor->boudingBox.y.topRight, actor->z, actorIdx);
	}
}

} // namespace TwinE
