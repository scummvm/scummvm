/** @file actor.cpp
	@brief
	This file contains scene actor routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "actor.h"
#include "lbaengine.h"
#include "scene.h"
#include "hqrdepack.h"
#include "resources.h"
#include "renderer.h"
#include "grid.h"
#include "animations.h"
#include "renderer.h"
#include "movements.h"
#include "gamestate.h"
#include "sound.h"
#include "extra.h"

/** Actors 3D body table - size of NUM_BODIES */
uint8 *bodyTable[NUM_BODIES];

/** Restart hero variables while opening new scenes */
void restartHeroScene() {
	sceneHero->controlMode = 1;
	memset(&sceneHero->dynamicFlags, 0, 2);
	memset(&sceneHero->staticFlags, 0, 2);

	sceneHero->staticFlags.bComputeCollisionWithObj = 1;
	sceneHero->staticFlags.bComputeCollisionWithBricks = 1;
	sceneHero->staticFlags.bIsZonable = 1;
	sceneHero->staticFlags.bCanDrown = 1;
	sceneHero->staticFlags.bCanFall = 1;

	sceneHero->armor = 1;
	sceneHero->positionInMoveScript = -1;
	sceneHero->labelIdx = -1;
	sceneHero->positionInLifeScript = 0;
	sceneHero->zone = -1;
	sceneHero->angle = previousHeroAngle;

	setActorAngleSafe(sceneHero->angle, sceneHero->angle, 0, &sceneHero->move);
	setBehaviour(previousHeroBehaviour);

	cropBottomScreen = 0;
}

/** Load hero 3D body and animations */
void loadHeroEntities() {
	hqrGetallocEntry(&heroEntityATHLETIC, HQR_FILE3D_FILE, FILE3DHQR_HEROATHLETIC);
	sceneHero->entityDataPtr = heroEntityATHLETIC;
	heroAnimIdxATHLETIC = getBodyAnimIndex(0, 0);

	hqrGetallocEntry(&heroEntityAGGRESSIVE, HQR_FILE3D_FILE, FILE3DHQR_HEROAGGRESSIVE);
	sceneHero->entityDataPtr = heroEntityAGGRESSIVE;
	heroAnimIdxAGGRESSIVE = getBodyAnimIndex(0, 0);

	hqrGetallocEntry(&heroEntityDISCRETE, HQR_FILE3D_FILE, FILE3DHQR_HERODISCRETE);
	sceneHero->entityDataPtr = heroEntityDISCRETE;
	heroAnimIdxDISCRETE = getBodyAnimIndex(0, 0);

	hqrGetallocEntry(&heroEntityPROTOPACK, HQR_FILE3D_FILE, FILE3DHQR_HEROPROTOPACK);
	sceneHero->entityDataPtr = heroEntityPROTOPACK;
	heroAnimIdxPROTOPACK = getBodyAnimIndex(0, 0);

	hqrGetallocEntry(&heroEntityNORMAL, HQR_FILE3D_FILE, FILE3DHQR_HERONORMAL);
	sceneHero->entityDataPtr = heroEntityNORMAL;
	heroAnimIdxNORMAL = getBodyAnimIndex(0, 0);

	sceneHero->animExtraPtr = currentActorAnimExtraPtr;
}

/** Set hero behaviour
	@param behaviour behaviour value to set */
void setBehaviour(int32 behaviour) {
	int32 bodyIdx;

	switch (behaviour) {
	case kNormal:
		heroBehaviour = kNormal;
		sceneHero->entityDataPtr = heroEntityNORMAL;
		break;
	case kAthletic:
		heroBehaviour = kAthletic;
		sceneHero->entityDataPtr = heroEntityATHLETIC;
		break;
	case kAggressive:
		heroBehaviour = kAggressive;
		sceneHero->entityDataPtr = heroEntityAGGRESSIVE;
		break;
	case kDiscrete:
		heroBehaviour = kDiscrete;
		sceneHero->entityDataPtr = heroEntityDISCRETE;
		break;
	case kProtoPack:
		heroBehaviour = kProtoPack;
		sceneHero->entityDataPtr = heroEntityPROTOPACK;
		break;
	};

	bodyIdx = sceneHero->body;

	sceneHero->entity = -1;
	sceneHero->body = -1;

	initModelActor(bodyIdx, 0);

	sceneHero->anim = -1;
	sceneHero->animType = 0;

	initAnim(kStanding, 0, 255, 0);
}

/** Initialize sprite actor
	@param actorIdx sprite actor index */
void initSpriteActor(int32 actorIdx) {
	ActorStruct *localActor = &sceneActors[actorIdx];

	if (localActor->staticFlags.bIsSpriteActor && localActor->sprite != -1 && localActor->entity != localActor->sprite) {
		int16 *ptr = (int16 *)(spriteBoundingBoxPtr + localActor->sprite * 16 + 4);

		localActor->entity = localActor->sprite;
		localActor->boudingBox.X.bottomLeft = *(ptr++);
		localActor->boudingBox.X.topRight = *(ptr++);
		localActor->boudingBox.Y.bottomLeft = *(ptr++);
		localActor->boudingBox.Y.topRight = *(ptr++);
		localActor->boudingBox.Z.bottomLeft = *(ptr++);
		localActor->boudingBox.Z.topRight = *(ptr++);
	}
}

/** Initialize 3D actor body
	@param bodyIdx 3D actor body index
	@param actorIdx 3D actor index */
int32 initBody(int32 bodyIdx, int32 actorIdx) {
	ActorStruct *localActor;
	uint8 *bodyPtr;
	uint8 var1;
	uint8 var2;
	uint8 *bodyPtr2;
	uint8 *bodyPtr3;
	uint8 *bodyPtr4;
//	int16 *bodyPtr5;
	int16 flag;
	int32 index;

	localActor = &sceneActors[actorIdx];
	bodyPtr = localActor->entityDataPtr;

	do {
		var1 = *(bodyPtr++);

		if (var1 == 0xFF)
			return (-1);

		bodyPtr2 = bodyPtr + 1;

		if (var1 == 1) {
			var2 = *(bodyPtr);

			if (var2 == bodyIdx) {
				bodyPtr3 = bodyPtr2 + 1;
				flag = *((uint16*)bodyPtr3);

				if (!(flag & 0x8000)) {
					hqrGetallocEntry(&bodyTable[currentPositionInBodyPtrTab], HQR_BODY_FILE, flag & 0xFFFF);

					if (!bodyTable[currentPositionInBodyPtrTab]) {
						printf("HQR ERROR: Loading body entities\n");
						exit(1);
					}
					prepareIsoModel(bodyTable[currentPositionInBodyPtrTab]);
					*((uint16*)bodyPtr3) = currentPositionInBodyPtrTab + 0x8000;
					index = currentPositionInBodyPtrTab;
					currentPositionInBodyPtrTab++;
				} else {
					flag &= 0x7FFF;
					index = flag;
				}

				bodyPtr3 += 2;
				bottomLeftX = -32000;

				bodyPtr4 = bodyPtr3;
				bodyPtr3++;

				if (!*bodyPtr4)
					return (index);

				bodyPtr4 = bodyPtr3;
				bodyPtr3++;

				if (*bodyPtr4 != 14)
					return (index);

//				bodyPtr5 = (int16 *) bodyPtr3;

				bottomLeftX = *((uint16*)bodyPtr3);
				bodyPtr3 += 2;
				bottomLeftY = *((uint16*)bodyPtr3);
				bodyPtr3 += 2;
				bottomLeftZ = *((uint16*)bodyPtr3);
				bodyPtr3 += 2;

				topRightX = *((uint16*)bodyPtr3);
				bodyPtr3 += 2;
				topRightY = *((uint16*)bodyPtr3);
				bodyPtr3 += 2;
				topRightZ = *((uint16*)bodyPtr3);
				bodyPtr3 += 2;

				return (index);
			}
		}

		bodyPtr = *bodyPtr2 + bodyPtr2;
	} while (1);
}

/** Initialize 3D actor
	@param bodyIdx 3D actor body index
	@param actorIdx 3D actor index */
void initModelActor(int32 bodyIdx, int16 actorIdx) {
	ActorStruct *localActor;
	int32  entityIdx;
	int    currentIndex;
	uint16 *ptr;
	int16  var1, var2, var3, var4;

	int32 result, result1, result2;

	result = 0;

	localActor = &sceneActors[actorIdx];

	if (localActor->staticFlags.bIsSpriteActor)
		return;

	if (actorIdx == 0 && heroBehaviour == kProtoPack && localActor->armor != 0 && localActor->armor != 1) { // if hero
		setBehaviour(kNormal);
	}

	if (bodyIdx != -1) {
		entityIdx = initBody(bodyIdx, actorIdx);
	} else {
		entityIdx = -1;
	}

	if (entityIdx != -1) {
		if (localActor->entity == entityIdx)
			return;

		localActor->entity = entityIdx;
		localActor->body = bodyIdx;
		currentIndex = localActor->entity;

		if (bottomLeftX == -32000) {
			ptr = (uint16 *) bodyTable[localActor->entity];
			ptr++;

			var1 = *((int16 *)ptr++);
			var2 = *((int16 *)ptr++);
			localActor->boudingBox.Y.bottomLeft = *((int16 *)ptr++);
			localActor->boudingBox.Y.topRight = *((int16 *)ptr++);
			var3 = *((int16 *)ptr++);
			var4 = *((int16 *)ptr++);

			if (localActor->staticFlags.bUseMiniZv) {
				result1 = var2 - var1; // take smaller for bound
				result2 = var4 - var3;

				if (result1 < result2)
					result = result1;
				else
					result = result2;

				result = abs(result);
				result >>= 1;
			} else {
				result1 = var2 - var1; // take average for bound
				result2 = var4 - var3;

				result = result2 + result1;
				result = abs(result);
				result >>= 2;
			}

			localActor->boudingBox.X.bottomLeft = -result;
			localActor->boudingBox.X.topRight = result;
			localActor->boudingBox.Z.bottomLeft = -result;
			localActor->boudingBox.Z.topRight = result;
		} else {
			localActor->boudingBox.X.bottomLeft = bottomLeftX;
			localActor->boudingBox.X.topRight = topRightX;
			localActor->boudingBox.Y.bottomLeft = bottomLeftY;
			localActor->boudingBox.Y.topRight = topRightY;
			localActor->boudingBox.Z.bottomLeft = bottomLeftZ;
			localActor->boudingBox.Z.topRight = topRightZ;
		}

		if (currentIndex == -1)
			return;

		if (localActor->previousAnimIdx == -1)
			return;

		copyActorInternAnim(bodyTable[currentIndex], bodyTable[localActor->entity]);

		return;
	}

	localActor->body = -1;
	localActor->entity = -1;

	localActor->boudingBox.X.bottomLeft = 0;
	localActor->boudingBox.X.topRight = 0;
	localActor->boudingBox.Y.bottomLeft = 0;
	localActor->boudingBox.Y.topRight = 0;
	localActor->boudingBox.Z.bottomLeft = 0;
	localActor->boudingBox.Z.topRight = 0;
}

/** Initialize actors
	@param actorIdx actor index to init */
void initActor(int16 actorIdx) {
	ActorStruct *actor = &sceneActors[actorIdx];

	if (actor->staticFlags.bIsSpriteActor) { // if sprite actor
		if (actor->strengthOfHit != 0) {
			actor->dynamicFlags.bIsHitting = 1;
		}

		actor->entity = -1;

		initSpriteActor(actorIdx);

		setActorAngleSafe(0, 0, 0, &actor->move);

		if (actor->staticFlags.bUsesClipping) {
			actor->lastX = actor->X;
			actor->lastY = actor->Y;
			actor->lastZ = actor->Z;
		}

	} else {
		actor->entity = -1;

		initModelActor(actor->body, actorIdx);

		actor->previousAnimIdx = -1;
		actor->animType = 0;

		if (actor->entity != -1) {
			initAnim(actor->anim, 0, 255, actorIdx);
		}

		setActorAngleSafe(actor->angle, actor->angle, 0, &actor->move);
	}

	actor->positionInMoveScript = -1;
	actor->labelIdx = -1;
	actor->positionInLifeScript = 0;
}

/** Reset actor
	@param actorIdx actor index to init */
void resetActor(int16 actorIdx) {
	ActorStruct *actor = &sceneActors[actorIdx];

	actor->body = 0;
	actor->anim = 0;
	actor->X = 0;
	actor->Y = -1;
	actor->Z = 0;

	actor->boudingBox.X.bottomLeft = 0;
	actor->boudingBox.X.topRight = 0;
	actor->boudingBox.Y.bottomLeft = 0;
	actor->boudingBox.Y.topRight = 0;
	actor->boudingBox.Z.bottomLeft = 0;
	actor->boudingBox.Z.topRight = 0;

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

	memset(&actor->staticFlags,0,2);
	memset(&actor->dynamicFlags,0,2);

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

	setActorAngleSafe(0, 0, 0, &actor->move);

	actor->positionInMoveScript = -1;
	actor->positionInLifeScript = 0;
}

/** Process hit actor
	@param actorIdx actor hitting index
	@param actorIdxAttacked actor attacked index
	@param strengthOfHit actor hitting strength of hit
	@param angle angle of actor hitting */
void hitActor(int32 actorIdx, int32 actorIdxAttacked, int32 strengthOfHit, int32 angle) {
	ActorStruct *actor = &sceneActors[actorIdxAttacked];

	if (actor->life <= 0) {
		return;
	}

	actor->hitBy = actorIdx;

	if (actor->armor <= strengthOfHit) {
		if (actor->anim == kBigHit || actor->anim == kHit2) {
			int32 tmpAnimPos;
			tmpAnimPos = actor->animPosition;
			if (actor->animExtra) {
				processAnimActions(actorIdxAttacked);
			}

			actor->animPosition = tmpAnimPos;
		} else {
			if (angle != -1) {
				setActorAngleSafe(angle, angle, 0, &actor->move);
			}

			if (rand() & 1) {
				initAnim(kHit2, 3, 255, actorIdxAttacked);
			} else {
				initAnim(kBigHit, 3, 255, actorIdxAttacked);
			}
		}

		addExtraSpecial(actor->X, actor->Y + 1000, actor->Z, kHitStars);

		if (!actorIdxAttacked) {
			heroMoved = 1;
		}

		actor->life -= strengthOfHit;

		if (actor->life < 0) {
			actor->life = 0;
		}
	} else {
		initAnim(kHit, 3, 255, actorIdxAttacked);
	}
}

/** Process actor carrier */
void processActorCarrier(int32 actorIdx) { // CheckCarrier
	int32 a;
	ActorStruct *actor = &sceneActors[actorIdx];

	if (actor->staticFlags.bIsCarrierActor) {
		for (a = 0; a < sceneNumActors; a++) {
			if (actor->standOn == actorIdx) {
				actor->standOn = -1;
			}
		}
	}
}

/** Process actor extra bonus */
void processActorExtraBonus(int32 actorIdx) { // GiveExtraBonus
	int32 a, numBonus;
	int8 bonusTable[8], currentBonus;
	ActorStruct *actor = &sceneActors[actorIdx];

	numBonus = 0;

	for (a = 0; a < 5; a++) {
		if (actor->bonusParameter & (1 << (a + 4))) {
			bonusTable[numBonus++] = a;
		}
	}

	if (numBonus) {
		currentBonus = bonusTable[Rnd(numBonus)];
		// if bonus is magic an no magic level yet, then give life points
		if (!magicLevelIdx && currentBonus == 2) {
			currentBonus = 1;
		}
		currentBonus += 3;

		if (actor->dynamicFlags.bIsDead) {
			addExtraBonus(actor->X, actor->Y, actor->Z, 0x100, 0, currentBonus, actor->bonusAmount);
			// FIXME add constant for sample index
			playSample(11, 0x1000, 1, actor->X, actor->Y, actor->Z, actorIdx);
		} else {
			int32 angle = getAngleAndSetTargetActorDistance(actor->X, actor->Z, sceneHero->X, sceneHero->Z);
			addExtraBonus(actor->X, actor->Y + actor->boudingBox.Y.topRight, actor->Z, 200, angle, currentBonus, actor->bonusAmount);
			// FIXME add constant for sample index
			playSample(11, 0x1000, 1, actor->X, actor->Y + actor->boudingBox.Y.topRight, actor->Z, actorIdx);
		}
	}
}
