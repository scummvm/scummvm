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
	free(_heroEntityNORMAL);
	free(_heroEntityATHLETIC);
	free(_heroEntityAGGRESSIVE);
	free(_heroEntityDISCRETE);
	free(_heroEntityPROTOPACK);
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
	_heroEntityATHLETICSize = loadBehaviourEntity(sceneHero, &_heroEntityATHLETIC, heroAnimIdxATHLETIC, FILE3DHQR_HEROATHLETIC);
	_heroEntityAGGRESSIVESize = loadBehaviourEntity(sceneHero, &_heroEntityAGGRESSIVE, heroAnimIdxAGGRESSIVE, FILE3DHQR_HEROAGGRESSIVE);
	_heroEntityDISCRETESize = loadBehaviourEntity(sceneHero, &_heroEntityDISCRETE, heroAnimIdxDISCRETE, FILE3DHQR_HERODISCRETE);
	_heroEntityPROTOPACKSize = loadBehaviourEntity(sceneHero, &_heroEntityPROTOPACK, heroAnimIdxPROTOPACK, FILE3DHQR_HEROPROTOPACK);
	_heroEntityNORMALSize = loadBehaviourEntity(sceneHero, &_heroEntityNORMAL, heroAnimIdxNORMAL, FILE3DHQR_HERONORMAL);

	sceneHero->animExtraPtr = _engine->_animations->currentActorAnimExtraPtr;
}

void Actor::setBehaviour(HeroBehaviourType behaviour) {
	ActorStruct *sceneHero = _engine->_scene->sceneHero;
	switch (behaviour) {
	case HeroBehaviourType::kNormal:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = _heroEntityNORMAL;
		sceneHero->entityDataSize = _heroEntityNORMALSize;
		break;
	case HeroBehaviourType::kAthletic:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = _heroEntityATHLETIC;
		sceneHero->entityDataSize = _heroEntityATHLETICSize;
		break;
	case HeroBehaviourType::kAggressive:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = _heroEntityAGGRESSIVE;
		sceneHero->entityDataSize = _heroEntityAGGRESSIVESize;
		break;
	case HeroBehaviourType::kDiscrete:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = _heroEntityDISCRETE;
		sceneHero->entityDataSize = _heroEntityDISCRETESize;
		break;
	case HeroBehaviourType::kProtoPack:
		heroBehaviour = behaviour;
		sceneHero->entityDataPtr = _heroEntityPROTOPACK;
		sceneHero->entityDataSize = _heroEntityPROTOPACKSize;
		break;
	};

	const BodyType bodyIdx = sceneHero->body;

	sceneHero->entity = -1;
	sceneHero->body = BodyType::btNone;

	initModelActor(bodyIdx, OWN_ACTOR_SCENE_INDEX);

	sceneHero->anim = AnimationTypes::kAnimNone;
	sceneHero->animType = AnimType::kAnimationTypeLoop;

	_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, 0);
}

void Actor::initSpriteActor(int32 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);

	if (localActor->staticFlags.bIsSpriteActor && localActor->sprite != -1 && localActor->entity != localActor->sprite) {
		const BoundingBox *spritebbox = _engine->_resources->spriteBoundingBox.bbox(localActor->sprite);
		localActor->entity = localActor->sprite;
		localActor->boudingBox = *spritebbox;
	}
}

int32 Actor::getTextIdForBehaviour() const {
	if (heroBehaviour == HeroBehaviourType::kAggressive && autoAggressive) {
		return TextId::kBehaviourAggressiveAuto;
	}
	// the other values are matching the text ids
	return (int32)heroBehaviour;
}

// see Animations::getBodyAnimIndex
int32 Actor::initBody(BodyType bodyIdx, int32 actorIdx, ActorBoundingBox &actorBoundingBox) {
	if (bodyIdx == BodyType::btNone) {
		return -1;
	}
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	Common::MemorySeekableReadWriteStream stream(actor->entityDataPtr, actor->entityDataSize);
	do {
		const uint8 type = stream.readByte();
		if (type == 0xFF) {
			return -1;
		}

		BodyType idx = (BodyType)stream.readByte();
		const int32 pos = stream.pos();
		const uint8 size = stream.readByte();
		if (type == 1) { // 1 = body data - 3 is animdata
			if (idx == bodyIdx) {
				const int16 bodyIndex = stream.readUint16LE();

				// TODO: move into resources class
				int32 index;
				if (!(bodyIndex & 0x8000)) {
					index = _currentPositionInBodyPtrTab;
					_currentPositionInBodyPtrTab++;
					_engine->_resources->bodyTableSize[index] = HQR::getAllocEntry(&_engine->_resources->bodyTable[index], Resources::HQR_BODY_FILE, bodyIndex & 0xFFFF);
					if (_engine->_resources->bodyTableSize[index] == 0) {
						error("HQR ERROR: Loading body entity for actor %i: %i", actorIdx, (int)bodyIdx);
					}
					if (!_engine->_resources->bodyData[index].loadFromBuffer(_engine->_resources->bodyTable[index], _engine->_resources->bodyTableSize[index])) {
						error("HQR ERROR: Parsing body entity for actor %i: %i", actorIdx, (int)bodyIdx);
					}
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

				actorBoundingBox.bbox.mins.x = stream.readSint16LE();
				actorBoundingBox.bbox.mins.y = stream.readSint16LE();
				actorBoundingBox.bbox.mins.z = stream.readSint16LE();
				actorBoundingBox.bbox.maxs.x = stream.readSint16LE();
				actorBoundingBox.bbox.maxs.y = stream.readSint16LE();
				actorBoundingBox.bbox.maxs.z = stream.readSint16LE();

				return index;
			}
		}
		stream.seek(pos + size);
	} while (1);
}

void Actor::initModelActor(BodyType bodyIdx, int16 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);
	if (localActor->staticFlags.bIsSpriteActor) {
		return;
	}

	debug(1, "Load body %i for actor %i", (int)bodyIdx, actorIdx);

	if (IS_HERO(actorIdx) && heroBehaviour == HeroBehaviourType::kProtoPack && localActor->armor != 0 && localActor->armor != 1) {
		setBehaviour(HeroBehaviourType::kNormal);
	}

	ActorBoundingBox actorBoundingBox;
	const int32 entityIdx = initBody(bodyIdx, actorIdx, actorBoundingBox);
	if (entityIdx == -1) {
		localActor->body = BodyType::btNone;
		localActor->entity = -1;

		BoundingBox &bbox = localActor->boudingBox;
		bbox.mins = IVec3();
		bbox.maxs = IVec3();
		debug("Failed to initialize body %i for actor %i", (int)bodyIdx, actorIdx);
		return;
	}

	if (localActor->entity == entityIdx) {
		return;
	}

	localActor->entity = entityIdx;
	localActor->body = bodyIdx;

	if (actorBoundingBox.hasBoundingBox) {
		BoundingBox &bbox = localActor->boudingBox;
		bbox.mins.x = actorBoundingBox.bbox.mins.x;
		bbox.maxs.x = actorBoundingBox.bbox.maxs.x;
		bbox.mins.y = actorBoundingBox.bbox.mins.y;
		bbox.maxs.y = actorBoundingBox.bbox.maxs.y;
		bbox.mins.z = actorBoundingBox.bbox.mins.z;
		bbox.maxs.z = actorBoundingBox.bbox.maxs.z;
	} else {
		BoundingBox &bbox = localActor->boudingBox;
		const BodyData &bd = _engine->_resources->bodyData[localActor->entity];
		bbox.mins.y = bd.bbox.mins.y;
		bbox.maxs.y = bd.bbox.maxs.y;

		int32 result = 0;
		const int32 distX = bd.bbox.maxs.x - bd.bbox.mins.x;
		const int32 distZ = bd.bbox.maxs.z - bd.bbox.mins.z;
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

		bbox.mins.x = -result;
		bbox.maxs.x = result;
		bbox.mins.z = -result;
		bbox.maxs.z = result;
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
			actor->lastPos = actor->pos;
		}
	} else {
		actor->entity = -1;

		debug(1, "Init actor %i with model %i", actorIdx, (int)actor->body);
		initModelActor(actor->body, actorIdx);

		actor->previousAnimIdx = -1;
		actor->animType = AnimType::kAnimationTypeLoop;

		if (actor->entity != -1) {
			_engine->_animations->initAnim(actor->anim, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
		}

		_engine->_movements->setActorAngleSafe(actor->angle, actor->angle, ANGLE_0, &actor->move);
	}

	actor->positionInMoveScript = -1;
	actor->labelIdx = -1;
	actor->positionInLifeScript = 0;
}

void Actor::resetActor(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	actor->actorIdx = actorIdx;
	actor->body = BodyType::btNormal;
	actor->anim = AnimationTypes::kStanding;
	actor->pos.x = 0;
	actor->pos.y = -1;
	actor->pos.z = 0;

	BoundingBox &bbox = actor->boudingBox;
	bbox.mins = IVec3();
	bbox.maxs = IVec3();

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

	actor->setLife(kActorMaxLife);
	actor->armor = 1;
	actor->hitBy = -1;
	actor->lastRotationAngle = ANGLE_0;
	actor->lastPos.x = 0;
	actor->lastPos.y = 0;
	actor->lastPos.z = 0;
	actor->entity = -1;
	actor->previousAnimIdx = -1;
	actor->animType = AnimType::kAnimationTypeLoop;
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
				_engine->_animations->initAnim(AnimationTypes::kHit2, AnimType::kAnimationType_3, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			} else {
				_engine->_animations->initAnim(AnimationTypes::kBigHit, AnimType::kAnimationType_3, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			}
		}

		_engine->_extra->addExtraSpecial(actor->pos.x, actor->pos.y + 1000, actor->pos.z, ExtraSpecialType::kHitStars);

		if (!actorIdxAttacked) {
			_engine->_movements->heroMoved = true;
		}

		actor->life -= strengthOfHit;
		if (actor->life < 0) {
			actor->life = 0;
		}
	} else {
		_engine->_animations->initAnim(AnimationTypes::kHit, AnimType::kAnimationType_3, AnimationTypes::kAnimInvalid, actorIdxAttacked);
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
		_engine->_extra->addExtraBonus(actor->pos.x, actor->pos.y, actor->pos.z, ANGLE_90, ANGLE_0, bonusSprite, actor->bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, actor->pos, actorIdx);
	} else {
		ActorStruct *sceneHero = _engine->_scene->sceneHero;
		const int32 angle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->pos, sceneHero->pos);
		_engine->_extra->addExtraBonus(actor->pos.x, actor->pos.y + actor->boudingBox.maxs.y, actor->pos.z, ANGLE_70, angle, bonusSprite, actor->bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, actor->pos.x, actor->pos.y + actor->boudingBox.maxs.y, actor->pos.z, actorIdx);
	}
}

void Actor::clearBodyTable() {
	_currentPositionInBodyPtrTab = 0;
}

ActorStruct::~ActorStruct() {
	free(entityDataPtr);
}

void ActorStruct::loadModel(int32 modelIndex) {
	entity = modelIndex;
	if (!staticFlags.bIsSpriteActor) {
		debug(1, "Init actor with model %i", modelIndex);
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

bool ActorStruct::isAttackWeaponAnimationActive() const {
	return anim == AnimationTypes::kSabreAttack || anim == AnimationTypes::kThrowBall || anim == AnimationTypes::kSabreUnknown;
}

bool ActorStruct::isJumpAnimationActive() const {
	return anim == AnimationTypes::kJump;
}

} // namespace TwinE
