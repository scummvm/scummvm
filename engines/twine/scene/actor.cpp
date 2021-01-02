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

#include "twine/scene/actor.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "twine/audio/sound.h"
#include "twine/parser/entity.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/animations.h"
#include "twine/scene/extra.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
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
	ActorStruct *sceneHero = _engine->_scene->sceneHero;
	sceneHero->controlMode = ControlMode::kManual;
	memset(&sceneHero->dynamicFlags, 0, sizeof(sceneHero->dynamicFlags));
	memset(&sceneHero->staticFlags, 0, sizeof(sceneHero->staticFlags));

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

	_engine->_movements->setActorAngleSafe(sceneHero->angle, sceneHero->angle, ANGLE_0, &sceneHero->move);
	setBehaviour(previousHeroBehaviour);

	cropBottomScreen = 0;
}

int32 Actor::loadBehaviourEntity(ActorStruct *sceneHero, uint8 **ptr, int16 &bodyAnimIndex, int32 index) {
	const int32 size = HQR::getAllocEntry(ptr, Resources::HQR_FILE3D_FILE, index);
	if (size == 0) {
		error("Failed to load actor 3d data for index: %i", index);
	}
	sceneHero->entityDataPtr = *ptr;
	sceneHero->entityDataSize = size;
	bodyAnimIndex = _engine->_animations->getBodyAnimIndex(AnimationTypes::kStanding);
	if (bodyAnimIndex == -1) {
		error("Could not find animation data for 3d data with index %i", index);
	}
	return size;
}

void Actor::loadHeroEntities() {
	ActorStruct *sceneHero = _engine->_scene->sceneHero;
	heroEntityATHLETICSize = loadBehaviourEntity(sceneHero, &heroEntityATHLETIC, heroAnimIdxATHLETIC, FILE3DHQR_HEROATHLETIC);
	heroEntityAGGRESSIVESize = loadBehaviourEntity(sceneHero, &heroEntityAGGRESSIVE, heroAnimIdxAGGRESSIVE, FILE3DHQR_HEROAGGRESSIVE);
	heroEntityDISCRETESize = loadBehaviourEntity(sceneHero, &heroEntityDISCRETE, heroAnimIdxDISCRETE, FILE3DHQR_HERODISCRETE);
	heroEntityPROTOPACKSize = loadBehaviourEntity(sceneHero, &heroEntityPROTOPACK, heroAnimIdxPROTOPACK, FILE3DHQR_HEROPROTOPACK);
	heroEntityNORMALSize = loadBehaviourEntity(sceneHero, &heroEntityNORMAL, heroAnimIdxNORMAL, FILE3DHQR_HERONORMAL);

	sceneHero->animExtraPtr = _engine->_animations->currentActorAnimExtraPtr;
}

void Actor::setBehaviour(HeroBehaviourType behaviour) {
	ActorStruct *sceneHero = _engine->_scene->sceneHero;
	switch (behaviour) {
	case HeroBehaviourType::kNormal:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = heroEntityNORMAL;
		sceneHero->entityDataSize = heroEntityNORMALSize;
		break;
	case HeroBehaviourType::kAthletic:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = heroEntityATHLETIC;
		sceneHero->entityDataSize = heroEntityATHLETICSize;
		break;
	case HeroBehaviourType::kAggressive:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = heroEntityAGGRESSIVE;
		sceneHero->entityDataSize = heroEntityAGGRESSIVESize;
		break;
	case HeroBehaviourType::kDiscrete:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = heroEntityDISCRETE;
		sceneHero->entityDataSize = heroEntityDISCRETESize;
		break;
	case HeroBehaviourType::kProtoPack:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = heroEntityPROTOPACK;
		sceneHero->entityDataSize = heroEntityPROTOPACKSize;
		break;
	};

	const int32 bodyIdx = sceneHero->body;

	sceneHero->entity = -1;
	sceneHero->body = -1;

	initModelActor(bodyIdx, 0);

	sceneHero->anim = AnimationTypes::kAnimNone;
	sceneHero->animType = 0;

	_engine->_animations->initAnim(AnimationTypes::kStanding, 0, AnimationTypes::kAnimInvalid, 0);
}

void Actor::initSpriteActor(int32 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);

	if (localActor->staticFlags.bIsSpriteActor && localActor->sprite != -1 && localActor->entity != localActor->sprite) {
		const BoundingBox *spritebbox = _engine->_resources->spriteBoundingBox.bbox(localActor->sprite);
		localActor->entity = localActor->sprite;
		ZVBox &bbox = localActor->boudingBox;
		bbox.x.bottomLeft = spritebbox->mins.x;
		bbox.x.topRight = spritebbox->maxs.x;
		bbox.y.bottomLeft = spritebbox->mins.y;
		bbox.y.topRight = spritebbox->maxs.y;
		bbox.z.bottomLeft = spritebbox->mins.z;
		bbox.z.topRight = spritebbox->maxs.z;
	}
}

int32 Actor::getTextIdForBehaviour() const {
	if (_engine->_actor->heroBehaviour == HeroBehaviourType::kAggressive && _engine->_actor->autoAgressive) {
		return TextId::kBehaviourAgressiveAuto;
	}
	// the other values are matching the text ids
	return (int32)_engine->_actor->heroBehaviour;
}

// see Animations::getBodyAnimIndex
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
					bodyData[index].loadFromBuffer(bodyTable[index], bodyTableSize[index]);
					Renderer::prepareIsoModel(bodyTable[index]);
					stream.seek(stream.pos() - sizeof(uint16));
					stream.writeUint16LE(index + 0x8000);
				} else {
					index = bodyIndex & 0x7FFF;
				}

				actorBoundingBox.hasBoundingBox = stream.readByte();
				if (!actorBoundingBox.hasBoundingBox) {
					return index;
				}

				if (stream.readByte() != ActionType::ACTION_ZV) {
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

	debug("Load body %i for actor %i", bodyIdx, actorIdx);

	if (IS_HERO(actorIdx) && heroBehaviour == HeroBehaviourType::kProtoPack && localActor->armor != 0 && localActor->armor != 1) {
		setBehaviour(HeroBehaviourType::kNormal);
	}

	ActorBoundingBox actorBoundingBox;
	const int32 entityIdx = initBody(bodyIdx, actorIdx, actorBoundingBox);
	if (entityIdx == -1) {
		localActor->body = -1;
		localActor->entity = -1;

		ZVBox &bbox = localActor->boudingBox;
		bbox.x.bottomLeft = 0;
		bbox.x.topRight = 0;
		bbox.y.bottomLeft = 0;
		bbox.y.topRight = 0;
		bbox.z.bottomLeft = 0;
		bbox.z.topRight = 0;
		debug("Failed to initialize body %i for actor %i", bodyIdx, actorIdx);
		return;
	}

	if (localActor->entity == entityIdx) {
		return;
	}

	localActor->entity = entityIdx;
	localActor->body = bodyIdx;

	if (actorBoundingBox.hasBoundingBox) {
		ZVBox &bbox = localActor->boudingBox;
		bbox.x.bottomLeft = actorBoundingBox.bottomLeftX;
		bbox.x.topRight = actorBoundingBox.topRightX;
		bbox.y.bottomLeft = actorBoundingBox.bottomLeftY;
		bbox.y.topRight = actorBoundingBox.topRightY;
		bbox.z.bottomLeft = actorBoundingBox.bottomLeftZ;
		bbox.z.topRight = actorBoundingBox.topRightZ;
	} else {
		ZVBox &bbox = localActor->boudingBox;
		const BodyData &bd = bodyData[localActor->entity];
		bbox.y.bottomLeft = bd.minsy;
		bbox.y.topRight = bd.maxsy;

		int32 result = 0;
		const int32 distX = bd.maxsx - bd.minsx;
		const int32 distZ = bd.maxsz - bd.minsz;
		if (localActor->staticFlags.bUseMiniZv) {
			// take smaller for bound
			result = MIN(distX, distZ);

			result = ABS(result);
			result >>= 1;
		} else {
			// take average for bound
			result = distZ + distX;
			result = ABS(result);
			result >>= 2;
		}

		bbox.x.bottomLeft = -result;
		bbox.x.topRight = result;
		bbox.z.bottomLeft = -result;
		bbox.z.topRight = result;
	}
}

void Actor::initActor(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	if (actor->staticFlags.bIsSpriteActor) {
		if (actor->strengthOfHit != 0) {
			actor->dynamicFlags.bIsHitting = 1;
		}

		actor->entity = -1;

		initSpriteActor(actorIdx);

		_engine->_movements->setActorAngleSafe(ANGLE_0, ANGLE_0, ANGLE_0, &actor->move);

		if (actor->staticFlags.bUsesClipping) {
			actor->lastX = actor->x;
			actor->lastY = actor->y;
			actor->lastZ = actor->z;
		}
	} else {
		actor->entity = -1;

		debug("Init actor %i with model %i", actorIdx, actor->body);
		initModelActor(actor->body, actorIdx);

		actor->previousAnimIdx = -1;
		actor->animType = 0;

		if (actor->entity != -1) {
			_engine->_animations->initAnim(actor->anim, 0, AnimationTypes::kAnimInvalid, actorIdx);
		}

		_engine->_movements->setActorAngleSafe(actor->angle, actor->angle, ANGLE_0, &actor->move);
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

	ZVBox &bbox = actor->boudingBox;
	bbox.x.bottomLeft = 0;
	bbox.x.topRight = 0;
	bbox.y.bottomLeft = 0;
	bbox.y.topRight = 0;
	bbox.z.bottomLeft = 0;
	bbox.z.topRight = 0;

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

	_engine->_movements->setActorAngleSafe(ANGLE_0, ANGLE_0, ANGLE_0, &actor->move);

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
				_engine->_movements->setActorAngleSafe(angle, angle, ANGLE_0, &actor->move);
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
		_engine->_extra->addExtraBonus(actor->x, actor->y, actor->z, ANGLE_90, ANGLE_0, bonusSprite, actor->bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, actor->x, actor->y, actor->z, actorIdx);
	} else {
		ActorStruct *sceneHero = _engine->_scene->sceneHero;
		const int32 angle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, sceneHero->x, sceneHero->z);
		_engine->_extra->addExtraBonus(actor->x, actor->y + actor->boudingBox.y.topRight, actor->z, ANGLE_70, angle, bonusSprite, actor->bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, actor->x, actor->y + actor->boudingBox.y.topRight, actor->z, actorIdx);
	}
}

void Actor::clearBodyTable() {
	currentPositionInBodyPtrTab = 0;
}

ActorStruct::~ActorStruct() {
	free(entityDataPtr);
}

void ActorStruct::loadModel(int32 modelIndex) {
	entity = modelIndex;
	if (!staticFlags.bIsSpriteActor) {
		debug("Init actor with model %i", modelIndex);
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

bool ActorStruct::isAttackAnimationActive() const {
	return anim == AnimationTypes::kRightPunch || anim == AnimationTypes::kLeftPunch || anim == AnimationTypes::kKick;
}

bool ActorStruct::isJumpAnimationActive() const {
	return anim == AnimationTypes::kJump;
}

} // namespace TwinE
