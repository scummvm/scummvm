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

#include "twine/scene/actor.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "twine/audio/sound.h"
#include "twine/debugger/debug_scene.h"
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
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

Actor::Actor(TwinEEngine *engine) : _engine(engine) {
}

void Actor::restartHeroScene() {
	ActorStruct *sceneHero = _engine->_scene->_sceneHero;
	sceneHero->_controlMode = ControlMode::kManual;
	memset(&sceneHero->_dynamicFlags, 0, sizeof(sceneHero->_dynamicFlags));
	memset(&sceneHero->_staticFlags, 0, sizeof(sceneHero->_staticFlags));

	sceneHero->_staticFlags.bComputeCollisionWithObj = 1;
	sceneHero->_staticFlags.bComputeCollisionWithBricks = 1;
	sceneHero->_staticFlags.bIsZonable = 1;
	sceneHero->_staticFlags.bCanDrown = 1;
	sceneHero->_staticFlags.bCanFall = 1;

	sceneHero->_armor = 1;
	sceneHero->_positionInMoveScript = -1;
	sceneHero->_labelIdx = -1;
	sceneHero->_positionInLifeScript = 0;
	sceneHero->_zone = -1;
	sceneHero->_angle = _previousHeroAngle;

	_engine->_movements->setActorAngleSafe(sceneHero->_angle, sceneHero->_angle, ANGLE_0, &sceneHero->_move);
	setBehaviour(_previousHeroBehaviour);

	_cropBottomScreen = 0;
}

void Actor::loadBehaviourEntity(ActorStruct *actor, EntityData &entityData, int16 &bodyAnimIndex, int32 index) {
	if (!entityData.loadFromHQR(Resources::HQR_FILE3D_FILE, index, _engine->isLBA1())) {
		error("Failed to load actor 3d data for index: %i", index);
	}

	actor->_entityDataPtr = &entityData;
	bodyAnimIndex = entityData.getAnimIndex(AnimationTypes::kStanding);
	if (bodyAnimIndex == -1) {
		error("Could not find animation data for 3d data with index %i", index);
	}
}

void Actor::loadHeroEntities() {
	ActorStruct *sceneHero = _engine->_scene->_sceneHero;
	loadBehaviourEntity(sceneHero, _heroEntityATHLETIC, _heroAnimIdxATHLETIC, FILE3DHQR_HEROATHLETIC);
	loadBehaviourEntity(sceneHero, _heroEntityAGGRESSIVE, _heroAnimIdxAGGRESSIVE, FILE3DHQR_HEROAGGRESSIVE);
	loadBehaviourEntity(sceneHero, _heroEntityDISCRETE, _heroAnimIdxDISCRETE, FILE3DHQR_HERODISCRETE);
	loadBehaviourEntity(sceneHero, _heroEntityPROTOPACK, _heroAnimIdxPROTOPACK, FILE3DHQR_HEROPROTOPACK);
	loadBehaviourEntity(sceneHero, _heroEntityNORMAL, _heroAnimIdxNORMAL, FILE3DHQR_HERONORMAL);

	_engine->_animations->_currentActorAnimExtraPtr = AnimationTypes::kStanding;
	sceneHero->_animExtraPtr = _engine->_animations->_currentActorAnimExtraPtr;
}

void Actor::setBehaviour(HeroBehaviourType behaviour) {
	ActorStruct *sceneHero = _engine->_scene->_sceneHero;
	switch (behaviour) {
	case HeroBehaviourType::kNormal:
		_heroBehaviour = behaviour;
		sceneHero->_entityDataPtr = &_heroEntityNORMAL;
		break;
	case HeroBehaviourType::kAthletic:
		_heroBehaviour = behaviour;
		sceneHero->_entityDataPtr = &_heroEntityATHLETIC;
		break;
	case HeroBehaviourType::kAggressive:
		_heroBehaviour = behaviour;
		sceneHero->_entityDataPtr = &_heroEntityAGGRESSIVE;
		break;
	case HeroBehaviourType::kDiscrete:
		_heroBehaviour = behaviour;
		sceneHero->_entityDataPtr = &_heroEntityDISCRETE;
		break;
	case HeroBehaviourType::kProtoPack:
		_heroBehaviour = behaviour;
		sceneHero->_entityDataPtr = &_heroEntityPROTOPACK;
		break;
	};

	const BodyType bodyIdx = sceneHero->_body;

	sceneHero->_entity = -1;
	sceneHero->_body = BodyType::btNone;

	initModelActor(bodyIdx, OWN_ACTOR_SCENE_INDEX);

	sceneHero->_anim = AnimationTypes::kAnimNone;
	sceneHero->_animType = AnimType::kAnimationTypeLoop;

	_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, OWN_ACTOR_SCENE_INDEX);
}

void Actor::initSpriteActor(int32 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);

	if (localActor->_staticFlags.bIsSpriteActor && localActor->_sprite != -1 && localActor->_entity != localActor->_sprite) {
		const BoundingBox *spritebbox = _engine->_resources->_spriteBoundingBox.bbox(localActor->_sprite);
		localActor->_entity = localActor->_sprite;
		localActor->_boundingBox = *spritebbox;
	}
}

TextId Actor::getTextIdForBehaviour() const {
	if (_heroBehaviour == HeroBehaviourType::kAggressive && _autoAggressive) {
		return TextId::kBehaviourAggressiveAuto;
	}
	// the other values are matching the text ids
	return (TextId)(int32)_heroBehaviour;
}

int32 Actor::initBody(BodyType bodyIdx, int32 actorIdx, ActorBoundingBox &actorBoundingBox) {
	if (bodyIdx == BodyType::btNone) {
		return -1;
	}
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const EntityBody *body = actor->_entityDataPtr->getBody((int)bodyIdx);
	if (body == nullptr) {
		warning("Failed to get entity body for body idx %i", (int)bodyIdx);
		return -1;
	}
	actorBoundingBox = body->actorBoundingBox;
	return body->hqrBodyIndex;
}

void Actor::initModelActor(BodyType bodyIdx, int16 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);
	if (localActor->_staticFlags.bIsSpriteActor) {
		return;
	}

	debug(1, "Load body %i for actor %i", (int)bodyIdx, actorIdx);

	if (IS_HERO(actorIdx) && _heroBehaviour == HeroBehaviourType::kProtoPack && localActor->_body != BodyType::btTunic && localActor->_body != BodyType::btNormal) {
		setBehaviour(HeroBehaviourType::kNormal);
	}

	ActorBoundingBox actorBoundingBox;
	const int32 newBody = initBody(bodyIdx, actorIdx, actorBoundingBox);
	if (newBody == -1) {
		localActor->_body = BodyType::btNone;
		localActor->_entity = -1;
		localActor->_boundingBox = BoundingBox();
		debug("Failed to initialize body %i for actor %i", (int)bodyIdx, actorIdx);
		return;
	}

	if (localActor->_entity == newBody) {
		return;
	}

	localActor->_entity = newBody;
	localActor->_body = bodyIdx;

	if (actorBoundingBox.hasBoundingBox) {
		localActor->_boundingBox = actorBoundingBox.bbox;
	} else {
		const BodyData &bd = _engine->_resources->_bodyData[localActor->_entity];
		localActor->_boundingBox = bd.bbox;

		int32 size = 0;
		const int32 distX = localActor->_boundingBox.maxs.x - localActor->_boundingBox.mins.x;
		const int32 distZ = localActor->_boundingBox.maxs.z - localActor->_boundingBox.mins.z;
		if (localActor->_staticFlags.bUseMiniZv) {
			// take smaller for bound
			if (distX < distZ)
				size = distX / 2;
			else
				size = distZ / 2;
		} else {
			// take average for bound
			size = (distZ + distX) / 4;
		}

		localActor->_boundingBox.mins.x = -size;
		localActor->_boundingBox.maxs.x = size;
		localActor->_boundingBox.mins.z = -size;
		localActor->_boundingBox.maxs.z = size;
	}
}

void Actor::initActor(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	if (actor->_staticFlags.bIsSpriteActor) {
		if (actor->_strengthOfHit != 0) {
			actor->_dynamicFlags.bIsHitting = 1;
		}

		actor->_entity = -1;

		initSpriteActor(actorIdx);

		_engine->_movements->setActorAngleSafe(ANGLE_0, ANGLE_0, ANGLE_0, &actor->_move);

		if (actor->_staticFlags.bUsesClipping) {
			actor->_animStep = actor->pos();
		}
	} else {
		actor->_entity = -1;

		debug(1, "Init actor %i with model %i", actorIdx, (int)actor->_body);
		initModelActor(actor->_body, actorIdx);

		actor->_previousAnimIdx = -1;
		actor->_animType = AnimType::kAnimationTypeLoop;

		if (actor->_entity != -1) {
			_engine->_animations->initAnim(actor->_anim, AnimType::kAnimationTypeLoop, AnimationTypes::kAnimInvalid, actorIdx);
		}

		_engine->_movements->setActorAngleSafe(actor->_angle, actor->_angle, ANGLE_0, &actor->_move);
	}

	actor->_positionInMoveScript = -1;
	actor->_labelIdx = -1;
	actor->_positionInLifeScript = 0;
}

void Actor::resetActor(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	actor->_actorIdx = actorIdx;
	actor->_body = BodyType::btNormal;
	actor->_anim = AnimationTypes::kStanding;
	actor->_pos = IVec3(0, -1, 0);
	actor->_spriteActorRotation = 0;

	actor->_boundingBox = BoundingBox();

	actor->_angle = 0;
	actor->_speed = 40;
	actor->_controlMode = ControlMode::kNoMove;

	actor->_cropLeft = 0;
	actor->_cropTop = 0;
	actor->_cropRight = 0;
	actor->_cropBottom = 0;

	actor->setBrickShape(ShapeType::kNone);
	actor->_collision = -1;
	actor->_carryBy = -1;
	actor->_zone = -1;

	memset(&actor->_staticFlags, 0, sizeof(StaticFlagsStruct));
	memset(&actor->_dynamicFlags, 0, sizeof(DynamicFlagsStruct));
	memset(&actor->_bonusParameter, 0, sizeof(BonusParameter));

	actor->setLife(kActorMaxLife);
	actor->_armor = 1;
	actor->_hitBy = -1;
	actor->_lastRotationAngle = ANGLE_0;
	actor->_animStep = IVec3();
	actor->_entity = -1;
	actor->_previousAnimIdx = -1;
	actor->_animType = AnimType::kAnimationTypeLoop;
	actor->_animPosition = 0;

	_engine->_movements->setActorAngleSafe(ANGLE_0, ANGLE_0, ANGLE_0, &actor->_move);

	actor->_positionInMoveScript = -1;
	actor->_positionInLifeScript = 0;
}

void Actor::hitActor(int32 actorIdx, int32 actorIdxAttacked, int32 strengthOfHit, int32 angle) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdxAttacked);
	if (actor->_life <= 0) {
		return;
	}

	if (IS_HERO(actorIdxAttacked) && _engine->_debugScene->_godMode) {
		return;
	}

	actor->_hitBy = actorIdx;

	if (actor->_armor <= strengthOfHit) {
		if (actor->_anim == AnimationTypes::kBigHit || actor->_anim == AnimationTypes::kHit2) {
			const int32 tmpAnimPos = actor->_animPosition;
			if (actor->_animExtra != AnimationTypes::kStanding) {
				_engine->_animations->processAnimActions(actorIdxAttacked);
			}

			actor->_animPosition = tmpAnimPos;
		} else {
			if (angle != -1) {
				_engine->_movements->setActorAngleSafe(angle, angle, ANGLE_0, &actor->_move);
			}

			if (_engine->getRandomNumber() & 1) {
				_engine->_animations->initAnim(AnimationTypes::kHit2, AnimType::kAnimationInsert, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			} else {
				_engine->_animations->initAnim(AnimationTypes::kBigHit, AnimType::kAnimationInsert, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			}
		}

		_engine->_extra->addExtraSpecial(actor->_pos.x, actor->_pos.y + 1000, actor->_pos.z, ExtraSpecialType::kHitStars);

		if (!actorIdxAttacked) {
			_engine->_movements->_lastJoyFlag = true;
		}

		actor->_life -= strengthOfHit;
		if (actor->_life < 0) {
			actor->_life = 0;
		}
	} else {
		_engine->_animations->initAnim(AnimationTypes::kHit, AnimType::kAnimationInsert, AnimationTypes::kAnimInvalid, actorIdxAttacked);
	}
}

void Actor::processActorCarrier(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (!actor->_staticFlags.bIsCarrierActor) {
		return;
	}
	for (int32 a = 0; a < _engine->_scene->_sceneNumActors; a++) {
		if (actor->_carryBy == actorIdx) {
			actor->_carryBy = -1;
		}
	}
}

void Actor::processActorExtraBonus(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	const int bonusSprite = _engine->_extra->getBonusSprite(actor->_bonusParameter);
	if (bonusSprite == -1) {
		return;
	}
	if (actor->_dynamicFlags.bIsDead) {
		_engine->_extra->addExtraBonus(actor->pos(), ANGLE_90, ANGLE_0, bonusSprite, actor->_bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, actor->pos(), actorIdx);
	} else {
		const ActorStruct *sceneHero = _engine->_scene->_sceneHero;
		const int32 angle = _engine->_movements->getAngleAndSetTargetActorDistance(actor->pos(), sceneHero->pos());
		const IVec3 pos(actor->_pos.x, actor->_pos.y + actor->_boundingBox.maxs.y, actor->_pos.z);
		_engine->_extra->addExtraBonus(pos, ANGLE_70, angle, bonusSprite, actor->_bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, pos, actorIdx);
	}
}

void ActorStruct::loadModel(int32 modelIndex, bool lba1) {
	_entity = modelIndex;
	if (!_staticFlags.bIsSpriteActor) {
		debug(1, "Init actor with model %i", modelIndex);
		if (!_entityData.loadFromHQR(Resources::HQR_FILE3D_FILE, modelIndex, lba1)) {
			error("Failed to load entity data for index %i", modelIndex);
		}
		_entityDataPtr = &_entityData;
	} else {
		_entityDataPtr = nullptr;
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
	return _anim == AnimationTypes::kRightPunch || _anim == AnimationTypes::kLeftPunch || _anim == AnimationTypes::kKick;
}

bool ActorStruct::isAttackWeaponAnimationActive() const {
	return _anim == AnimationTypes::kSabreAttack || _anim == AnimationTypes::kThrowBall || _anim == AnimationTypes::kSabreUnknown;
}

bool ActorStruct::isJumpAnimationActive() const {
	return _anim == AnimationTypes::kJump;
}

} // namespace TwinE
