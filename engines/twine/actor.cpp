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
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "twine/animations.h"
#include "twine/extra.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/hqr.h"
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

Actor::~Actor() {
	_engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX)->entityDataPtr = nullptr;
	free(heroEntityNORMAL);
	free(heroEntityATHLETIC);
	free(heroEntityAGGRESSIVE);
	free(heroEntityDISCRETE);
	free(heroEntityPROTOPACK);

	for (size_t i = 0; i < ARRAYSIZE(bodyTable); ++i) {
		free(bodyTable[i]);
	}
}

void Actor::restartHeroScene() {
	_engine->_scene->sceneHero->controlMode = ControlMode::kManual;
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
	heroEntityATHLETICSize = HQR::getAllocEntry(&heroEntityATHLETIC, Resources::HQR_FILE3D_FILE, FILE3DHQR_HEROATHLETIC);
	if (heroEntityATHLETICSize == 0) {
		error("Failed to load actor athletic 3d data");
	}
	_engine->_scene->sceneHero->entityDataPtr = heroEntityATHLETIC;
	heroAnimIdxATHLETIC = _engine->_animations->getBodyAnimIndex(AnimationTypes::kStanding);

	heroEntityAGGRESSIVESize = HQR::getAllocEntry(&heroEntityAGGRESSIVE, Resources::HQR_FILE3D_FILE, FILE3DHQR_HEROAGGRESSIVE);
	if (heroEntityAGGRESSIVESize == 0) {
		error("Failed to load actor aggressive 3d data");
	}
	_engine->_scene->sceneHero->entityDataPtr = heroEntityAGGRESSIVE;
	heroAnimIdxAGGRESSIVE = _engine->_animations->getBodyAnimIndex(AnimationTypes::kStanding);

	heroEntityDISCRETESize = HQR::getAllocEntry(&heroEntityDISCRETE, Resources::HQR_FILE3D_FILE, FILE3DHQR_HERODISCRETE);
	if (heroEntityDISCRETESize == 0) {
		error("Failed to load actor discrete 3d data");
	}
	_engine->_scene->sceneHero->entityDataPtr = heroEntityDISCRETE;
	heroAnimIdxDISCRETE = _engine->_animations->getBodyAnimIndex(AnimationTypes::kStanding);

	heroEntityPROTOPACKSize = HQR::getAllocEntry(&heroEntityPROTOPACK, Resources::HQR_FILE3D_FILE, FILE3DHQR_HEROPROTOPACK);
	if (heroEntityPROTOPACKSize == 0) {
		error("Failed to load actor protopack 3d data");
	}
	_engine->_scene->sceneHero->entityDataPtr = heroEntityPROTOPACK;
	heroAnimIdxPROTOPACK = _engine->_animations->getBodyAnimIndex(AnimationTypes::kStanding);

	heroEntityNORMALSize = HQR::getAllocEntry(&heroEntityNORMAL, Resources::HQR_FILE3D_FILE, FILE3DHQR_HERONORMAL);
	if (heroEntityNORMALSize == 0) {
		error("Failed to load actor normal 3d data");
	}
	_engine->_scene->sceneHero->entityDataPtr = heroEntityNORMAL;
	_engine->_scene->sceneHero->entityDataSize = heroEntityNORMALSize;
	heroAnimIdxNORMAL = _engine->_animations->getBodyAnimIndex(AnimationTypes::kStanding);

	_engine->_scene->sceneHero->animExtraPtr = _engine->_animations->currentActorAnimExtraPtr;
}

void Actor::setBehaviour(HeroBehaviourType behaviour) {
	switch (behaviour) {
	case HeroBehaviourType::kNormal:
		heroBehaviour = behaviour;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityNORMAL;
		_engine->_scene->sceneHero->entityDataSize = heroEntityNORMALSize;
		break;
	case HeroBehaviourType::kAthletic:
		heroBehaviour = behaviour;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityATHLETIC;
		_engine->_scene->sceneHero->entityDataSize = heroEntityATHLETICSize;
		break;
	case HeroBehaviourType::kAggressive:
		heroBehaviour = behaviour;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityAGGRESSIVE;
		_engine->_scene->sceneHero->entityDataSize = heroEntityAGGRESSIVESize;
		break;
	case HeroBehaviourType::kDiscrete:
		heroBehaviour = behaviour;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityDISCRETE;
		_engine->_scene->sceneHero->entityDataSize = heroEntityDISCRETESize;
		break;
	case HeroBehaviourType::kProtoPack:
		heroBehaviour = behaviour;
		_engine->_scene->sceneHero->entityDataPtr = heroEntityPROTOPACK;
		_engine->_scene->sceneHero->entityDataSize = heroEntityPROTOPACKSize;
		break;
	};

	const int32 bodyIdx = _engine->_scene->sceneHero->body;

	_engine->_scene->sceneHero->entity = -1;
	_engine->_scene->sceneHero->body = -1;

	initModelActor(bodyIdx, 0);

	_engine->_scene->sceneHero->anim = AnimationTypes::kAnimNone;
	_engine->_scene->sceneHero->animType = 0;

	_engine->_animations->initAnim(AnimationTypes::kStanding, 0, AnimationTypes::kAnimInvalid, 0);
}

void Actor::initSpriteActor(int32 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);

	if (localActor->staticFlags.bIsSpriteActor && localActor->sprite != -1 && localActor->entity != localActor->sprite) {
		Common::MemoryReadStream stream(_engine->_resources->spriteBoundingBoxPtr, _engine->_resources->spriteBoundingBoxSize);
		stream.seek(localActor->sprite * 16);
		stream.skip(4);

		localActor->entity = localActor->sprite;
		localActor->boudingBox.x.bottomLeft = stream.readSint16LE();
		localActor->boudingBox.x.topRight = stream.readSint16LE();
		localActor->boudingBox.y.bottomLeft = stream.readSint16LE();
		localActor->boudingBox.y.topRight = stream.readSint16LE();
		localActor->boudingBox.z.bottomLeft = stream.readSint16LE();
		localActor->boudingBox.z.topRight = stream.readSint16LE();
	}
}

int32 Actor::getTextIdForBehaviour() const {
	if (_engine->_actor->heroBehaviour == HeroBehaviourType::kAggressive && _engine->_actor->autoAgressive) {
		return TextId::kBehaviourAgressiveAuto;
	}
	// the other values are matching the text ids
	return (int32)_engine->_actor->heroBehaviour;
}

int32 Actor::initBody(int32 bodyIdx, int32 actorIdx, ActorBoundingBox &actorBoundingBox) {
	if (bodyIdx == -1) {
		return -1;
	}
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	Common::MemorySeekableReadWriteStream stream(actor->entityDataPtr, actor->entityDataSize);
	do {
		const uint8 type = stream.readByte();
		if (type == 0xFF) {
			return -1;
		}

		uint8 idx = stream.readByte();
		const int32 pos = stream.pos();
		const uint8 size = stream.readByte();
		if (type == 1) { // 1 = body data - 3 is animdata
			if (idx == bodyIdx) {
				const int16 bodyIndex = stream.readUint16LE();

				// TODO: move into resources class
				int32 index;
				if (!(bodyIndex & 0x8000)) {
					index = currentPositionInBodyPtrTab;
					currentPositionInBodyPtrTab++;
					bodyTableSize[index] = HQR::getAllocEntry(&bodyTable[index], Resources::HQR_BODY_FILE, bodyIndex & 0xFFFF);
					if (bodyTableSize[index] == 0) {
						error("HQR ERROR: Loading body entities");
					}
					_engine->_renderer->prepareIsoModel(bodyTable[index]);
					stream.seek(stream.pos() - sizeof(uint16));
					stream.writeUint16LE(index + 0x8000);
				} else {
					index = bodyIndex & 0x7FFF;
				}

				actorBoundingBox.hasBoundingBox = stream.readByte();
				if (!actorBoundingBox.hasBoundingBox) {
					return index;
				}

				if (stream.readByte() != 14) {
					return index;
				}

				actorBoundingBox.bottomLeftX = stream.readUint16LE();
				actorBoundingBox.bottomLeftY = stream.readUint16LE();
				actorBoundingBox.bottomLeftZ = stream.readUint16LE();

				actorBoundingBox.topRightX = stream.readUint16LE();
				actorBoundingBox.topRightY = stream.readUint16LE();
				actorBoundingBox.topRightZ = stream.readUint16LE();

				return index;
			}
		}
		stream.seek(pos + size);
	} while (1);
}

void Actor::initModelActor(int32 bodyIdx, int16 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);
	if (localActor->staticFlags.bIsSpriteActor) {
		return;
	}

	if (IS_HERO(actorIdx) && heroBehaviour == HeroBehaviourType::kProtoPack && localActor->armor != 0 && localActor->armor != 1) {
		setBehaviour(HeroBehaviourType::kNormal);
	}

	ActorBoundingBox actorBoundingBox;
	const int32 entityIdx = initBody(bodyIdx, actorIdx, actorBoundingBox);
	if (entityIdx == -1) {
		localActor->body = -1;
		localActor->entity = -1;

		localActor->boudingBox.x.bottomLeft = 0;
		localActor->boudingBox.x.topRight = 0;
		localActor->boudingBox.y.bottomLeft = 0;
		localActor->boudingBox.y.topRight = 0;
		localActor->boudingBox.z.bottomLeft = 0;
		localActor->boudingBox.z.topRight = 0;
		return;
	}

	if (localActor->entity == entityIdx) {
		return;
	}

	localActor->entity = entityIdx;
	localActor->body = bodyIdx;
	int currentIndex = localActor->entity;

	if (actorBoundingBox.hasBoundingBox) {
		localActor->boudingBox.x.bottomLeft = actorBoundingBox.bottomLeftX;
		localActor->boudingBox.x.topRight = actorBoundingBox.topRightX;
		localActor->boudingBox.y.bottomLeft = actorBoundingBox.bottomLeftY;
		localActor->boudingBox.y.topRight = actorBoundingBox.topRightY;
		localActor->boudingBox.z.bottomLeft = actorBoundingBox.bottomLeftZ;
		localActor->boudingBox.z.topRight = actorBoundingBox.topRightZ;
	} else {
		Common::MemoryReadStream stream(bodyTable[localActor->entity], bodyTableSize[localActor->entity]);
		stream.skip(2);
		const int16 var1 = stream.readSint16LE();
		const int16 var2 = stream.readSint16LE();
		localActor->boudingBox.y.bottomLeft = stream.readSint16LE();
		localActor->boudingBox.y.topRight = stream.readSint16LE();
		const int16 var3 = stream.readSint16LE();
		const int16 var4 = stream.readSint16LE();

		int32 result = 0;
		const int32 result1 = var2 - var1;
		const int32 result2 = var4 - var3;
		if (localActor->staticFlags.bUseMiniZv) {
			// take smaller for bound
			result = MIN(result1, result2);

			result = ABS(result);
			result >>= 1;
		} else {
			// take average for bound
			result = result2 + result1;
			result = ABS(result);
			result >>= 2;
		}

		localActor->boudingBox.x.bottomLeft = -result;
		localActor->boudingBox.x.topRight = result;
		localActor->boudingBox.z.bottomLeft = -result;
		localActor->boudingBox.z.topRight = result;
	}

	if (currentIndex == -1) {
		return;
	}

	if (localActor->previousAnimIdx == -1) {
		return;
	}

	_engine->_renderer->copyActorInternAnim(bodyTable[currentIndex], bodyTable[localActor->entity]);
}

void Actor::initActor(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	if (actor->staticFlags.bIsSpriteActor) {
		if (actor->strengthOfHit != 0) {
			actor->dynamicFlags.bIsHitting = 1;
		}

		actor->entity = -1;

		initSpriteActor(actorIdx);

		_engine->_movements->setActorAngleSafe(ANGLE_0, ANGLE_0, 0, &actor->move);

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
			_engine->_animations->initAnim(actor->anim, 0, AnimationTypes::kAnimInvalid, actorIdx);
		}

		_engine->_movements->setActorAngleSafe(actor->angle, actor->angle, 0, &actor->move);
	}

	actor->positionInMoveScript = -1;
	actor->labelIdx = -1;
	actor->positionInLifeScript = 0;
}

void Actor::resetActor(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	actor->body = 0;
	actor->anim = AnimationTypes::kStanding;
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
	actor->controlMode = ControlMode::kNoMove;

	actor->cropLeft = 0;
	actor->cropTop = 0;
	actor->cropRight = 0;
	actor->cropBottom = 0;

	actor->setBrickShape(ShapeType::kNone);
	actor->collision = -1;
	actor->standOn = -1;
	actor->zone = -1;

	memset(&actor->staticFlags, 0, sizeof(StaticFlagsStruct));
	memset(&actor->dynamicFlags, 0, sizeof(DynamicFlagsStruct));
	memset(&actor->bonusParameter, 0, sizeof(BonusParameter));

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

	_engine->_movements->setActorAngleSafe(ANGLE_0, ANGLE_0, 0, &actor->move);

	actor->positionInMoveScript = -1;
	actor->positionInLifeScript = 0;
}

void Actor::hitActor(int32 actorIdx, int32 actorIdxAttacked, int32 strengthOfHit, int32 angle) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdxAttacked);
	if (actor->life <= 0) {
		return;
	}

	actor->hitBy = actorIdx;

	if (actor->armor <= strengthOfHit) {
		if (actor->anim == AnimationTypes::kBigHit || actor->anim == AnimationTypes::kHit2) {
			const int32 tmpAnimPos = actor->animPosition;
			if (actor->animExtra != AnimationTypes::kStanding) {
				_engine->_animations->processAnimActions(actorIdxAttacked);
			}

			actor->animPosition = tmpAnimPos;
		} else {
			if (angle != -1) {
				_engine->_movements->setActorAngleSafe(angle, angle, 0, &actor->move);
			}

			if (_engine->getRandomNumber() & 1) {
				_engine->_animations->initAnim(AnimationTypes::kHit2, 3, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			} else {
				_engine->_animations->initAnim(AnimationTypes::kBigHit, 3, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			}
		}

		_engine->_extra->addExtraSpecial(actor->x, actor->y + 1000, actor->z, ExtraSpecialType::kHitStars);

		if (!actorIdxAttacked) {
			_engine->_movements->heroMoved = true;
		}

		actor->life -= strengthOfHit;

		if (actor->life < 0) {
			actor->life = 0;
		}
	} else {
		_engine->_animations->initAnim(AnimationTypes::kHit, 3, AnimationTypes::kAnimInvalid, actorIdxAttacked);
	}
}

void Actor::processActorCarrier(int32 actorIdx) { // CheckCarrier
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
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
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	const int bonusSprite = _engine->_extra->getBonusSprite(actor->bonusParameter);
	if (bonusSprite == -1) {
		return;
	}
	if (actor->dynamicFlags.bIsDead) {
		_engine->_extra->addExtraBonus(actor->x, actor->y, actor->z, ANGLE_90, 0, bonusSprite, actor->bonusAmount);
		// FIXME add constant for sample index
		_engine->_sound->playSample(Samples::ItemPopup, 4096, 1, actor->x, actor->y, actor->z, actorIdx);
	} else {
		const int32 angle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->z);
		_engine->_extra->addExtraBonus(actor->x, actor->y + actor->boudingBox.y.topRight, actor->z, 200, angle, bonusSprite, actor->bonusAmount);
		// FIXME add constant for sample index
		_engine->_sound->playSample(Samples::ItemPopup, 4096, 1, actor->x, actor->y + actor->boudingBox.y.topRight, actor->z, actorIdx);
	}
}

ActorStruct::~ActorStruct() {
	free(entityDataPtr);
}

void ActorStruct::loadModel(int32 modelIndex) {
	entity = modelIndex;
	if (!staticFlags.bIsSpriteActor) {
		entityDataSize = HQR::getAllocEntry(&entityDataPtr, Resources::HQR_FILE3D_FILE, modelIndex);
	} else {
		entityDataSize = 0;
		free(entityDataPtr);
		entityDataPtr = nullptr;
	}
}

int32 ActorMoveStruct::getRealAngle(int32 time) {
	if (numOfStep) {
		const int32 timePassed = time - timeOfChange;

		if (timePassed >= numOfStep) { // rotation is finished
			numOfStep = 0;
			return to;
		}

		int32 remainingAngle = NormalizeAngle(to - from);
		remainingAngle *= timePassed;
		remainingAngle /= numOfStep;
		remainingAngle += from;

		return remainingAngle;
	}

	return to;
}

int32 ActorMoveStruct::getRealValue(int32 time) {
	if (!numOfStep) {
		return to;
	}

	if (time - timeOfChange >= numOfStep) {
		numOfStep = 0;
		return to;
	}

	int32 tempStep = to - from;
	tempStep *= time - timeOfChange;
	tempStep /= numOfStep;

	return tempStep + from;
}


} // namespace TwinE
