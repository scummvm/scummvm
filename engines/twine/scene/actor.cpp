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
#include "twine/debugger/debug_state.h"
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
	memset(&sceneHero->_workFlags, 0, sizeof(sceneHero->_workFlags));
	memset(&sceneHero->_staticFlags, 0, sizeof(sceneHero->_staticFlags));

	sceneHero->_staticFlags.bComputeCollisionWithObj = 1;
	sceneHero->_staticFlags.bComputeCollisionWithBricks = 1;
	sceneHero->_staticFlags.bCheckZone = 1;
	sceneHero->_staticFlags.bCanDrown = 1;
	sceneHero->_staticFlags.bObjFallable = 1;

	sceneHero->_armor = 1;
	sceneHero->_offsetTrack = -1;
	sceneHero->_labelTrack = -1;
	sceneHero->_offsetLife = 0;
	sceneHero->_zoneSce = -1;
	sceneHero->_beta = _previousHeroAngle;

	_engine->_movements->initRealAngle(sceneHero->_beta, sceneHero->_beta, LBAAngles::ANGLE_0, &sceneHero->realAngle);
	setBehaviour(_previousHeroBehaviour);

	_cropBottomScreen = 0;
}

void Actor::loadBehaviourEntity(ActorStruct *actor, EntityData &entityData, int16 &bodyAnimIndex, int32 index) {
	_engine->_resources->loadEntityData(entityData, index);
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
	sceneHero->_ptrAnimAction = _engine->_animations->_currentActorAnimExtraPtr;
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
	case HeroBehaviourType::kMax:
		break;
	}

	const BodyType bodyIdx = sceneHero->_genBody;

	sceneHero->_body = -1;
	sceneHero->_genBody = BodyType::btNone;

	initBody(bodyIdx, OWN_ACTOR_SCENE_INDEX);

	sceneHero->_genAnim = AnimationTypes::kAnimNone;
	sceneHero->_flagAnim = AnimType::kAnimationTypeRepeat;

	_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, OWN_ACTOR_SCENE_INDEX);
}

void Actor::setFrame(int32 actorIdx, uint32 frame) {
#if 0
	// TODO: converted from asm - not yet adapted
	ActorStruct *obj = _engine->_scene->getActor(actorIdx);
	T_PTR_NUM tempNextBody = obj->NextBody;
	void *tempNextTexture = obj->NextTexture;

	if (frame >= obj->NbFrames) {
		return;
	}

	obj->_body = tempNextBody;
	obj->Texture = tempNextTexture;

	T_PTR_NUM tempAnim = obj->_anim;
	void (*TransFctAnim)() = nullptr; // Couldn't find this yet

	if (TransFctAnim != nullptr) {
		uint32 ebp = frame;
		TransFctAnim();
		tempAnim = (T_PTR_NUM)(void *)tempAnim.Ptr;
		frame = ebp;
	}

	obj->Interpolator = 0;
	obj->LastAnimStepX = 0;
	obj->LastAnimStepY = 0;
	obj->LastAnimStepZ = 0;

	uint16 nbGroups = *((uint16 *)(tempAnim.Ptr + 2));

	obj->LastAnimStepAlpha = 0;
	obj->LastAnimStepBeta = 0;
	obj->LastAnimStepGamma = 0;
	obj->LastOfsIsPtr = 0;

	uint32 lastOfsFrame = nbGroups * 8 + 8; // infos frame + 4 WORDs per group

	obj->LastNbGroups = nbGroups;
	obj->NextNbGroups = nbGroups;
	obj->NbGroups = nbGroups;

	lastOfsFrame *= frame;
	uint32 timerRefHR = 0; // Replace with actual TimerRefHR

	lastOfsFrame += 8; // Skip header

	obj->LastTimer = timerRefHR;
	obj->Time = timerRefHR;
	obj->Status = 1; // STATUS_FRAME
	obj->LastOfsFrame = lastOfsFrame;
	obj->LastFrame = frame;

	uint32 ecx = nbGroups * 2 - 2; // 2 DWORDs per group, no group 0
	T_PTR_NUM ebpPtr = tempAnim;
	tempAnim.Ptr = tempAnim.Ptr + lastOfsFrame + 16;

	memcpy(obj->CurrentFrame, tempAnim.Ptr, ecx);

	if (++frame == obj->NbFrames) {
		uint16 time = *((uint16 *)(ebpPtr.Ptr + 8));
		frame = 0;
		tempAnim.Ptr = (void *)(8);
	} else {
		uint16 time = *((uint16 *)tempAnim.Ptr);
		tempAnim.Ptr -= ebpPtr.Ptr;
		tempAnim.Num += obj->LastTimer;
		obj->NextFrame = frame;
		obj->NextOfsFrame = (uint32)(tempAnim.Ptr);
		obj->NextTimer = time;
		obj->Master = *((uint16 *)(tempAnim.Ptr + 8));
		obj->Status = 1; // STATUS_FRAME
	}
#endif
}

void Actor::initSprite(int32 spriteNum, int32 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);

	localActor->_sprite = spriteNum;
	if (!localActor->_staticFlags.bSprite3D) {
		return;
	}
	if (spriteNum != -1 && localActor->_body != spriteNum) {
		const BoundingBox *spritebbox = _engine->_resources->_spriteBoundingBox.bbox(spriteNum);
		localActor->_body = spriteNum;
		localActor->_boundingBox = *spritebbox;
	}
}

TextId Actor::getTextIdForBehaviour() const {
	if (_heroBehaviour == HeroBehaviourType::kAggressive && _combatAuto) {
		return TextId::kBehaviourAggressiveAuto;
	}
	// the other values are matching the text ids
	return (TextId)(int32)_heroBehaviour;
}

int32 Actor::searchBody(BodyType bodyIdx, int32 actorIdx, ActorBoundingBox &actorBoundingBox) {
	if (bodyIdx == BodyType::btNone) {
		return -1;
	}
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const EntityBody *body = actor->_entityDataPtr->getEntityBody((int)bodyIdx);
	if (body == nullptr) {
		warning("Failed to get entity body for body idx %i", (int)bodyIdx);
		return -1;
	}
	actorBoundingBox = body->actorBoundingBox;
	return (int)bodyIdx;
}

void Actor::initBody(BodyType bodyIdx, int16 actorIdx) {
	ActorStruct *localActor = _engine->_scene->getActor(actorIdx);
	if (localActor->_staticFlags.bSprite3D) {
		return;
	}

	debug(1, "Load body %i for actor %i", (int)bodyIdx, actorIdx);

	if (IS_HERO(actorIdx) && _heroBehaviour == HeroBehaviourType::kProtoPack && bodyIdx != BodyType::btTunic && bodyIdx != BodyType::btNormal) {
		setBehaviour(HeroBehaviourType::kNormal);
	}

	ActorBoundingBox actorBoundingBox;
	const int32 newBody = searchBody(bodyIdx, actorIdx, actorBoundingBox);
	if (newBody == -1) {
		localActor->_genBody = BodyType::btNone;
		localActor->_body = -1;
		localActor->_boundingBox = BoundingBox();
		debug("Failed to initialize body %i for actor %i", (int)bodyIdx, actorIdx);
		return;
	}

	if (localActor->_body == newBody) {
		return;
	}

	const int32 oldBody = localActor->_body;
	localActor->_body = newBody;
	localActor->_genBody = bodyIdx;

	if (actorBoundingBox.hasBoundingBox) {
		localActor->_boundingBox = actorBoundingBox.bbox;
	} else {
		const BodyData &bd = localActor->_entityDataPtr->getBody(localActor->_body);
		localActor->_boundingBox = bd.bbox;

		int32 size = 0;
		const int32 distX = bd.bbox.maxs.x - bd.bbox.mins.x;
		const int32 distZ = bd.bbox.maxs.z - bd.bbox.mins.z;
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
	if (oldBody != -1 && localActor->_anim != -1) {
		copyInterAnim(localActor->_entityDataPtr->getBody(oldBody), localActor->_entityDataPtr->getBody(localActor->_body));
	}
}

void Actor::copyInterAnim(const BodyData &src, BodyData &dest) {
	if (!src.isAnimated() || !dest.isAnimated()) {
		return;
	}

	const int16 numBones = MIN<int16>((int16)src.getNumBones(), (int16)dest.getNumBones());
	for (int16 i = 0; i < numBones; ++i) {
		const BoneFrame *srcBoneFrame = src.getBoneState(i);
		BoneFrame *destBoneFrame = dest.getBoneState(i);
		*destBoneFrame = *srcBoneFrame;
	}
}

void Actor::startInitObj(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	if (actor->_staticFlags.bSprite3D) {
		if (actor->_strengthOfHit != 0) {
			actor->_workFlags.bIsHitting = 1;
		}

		actor->_body = -1;

		initSprite(actor->_sprite, actorIdx);

		_engine->_movements->initRealAngle(LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, &actor->realAngle);

		if (actor->_staticFlags.bSpriteClip) {
			actor->_animStep = actor->posObj();
		}
	} else {
		actor->_body = -1;

		debug(1, "Init actor %i with model %i", actorIdx, (int)actor->_genBody);
		initBody(actor->_genBody, actorIdx);

		actor->_anim = -1;
		actor->_flagAnim = AnimType::kAnimationTypeRepeat;

		if (actor->_body != -1) {
			_engine->_animations->initAnim(actor->_genAnim, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, actorIdx);
		}

		_engine->_movements->initRealAngle(actor->_beta, actor->_beta, LBAAngles::ANGLE_0, &actor->realAngle);
	}

	actor->_offsetTrack = -1;
	actor->_labelTrack = -1;
	actor->_offsetLife = 0;
}

void Actor::initObject(int16 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	*actor = ActorStruct(_engine->getMaxLife());

	actor->_actorIdx = actorIdx;
	actor->_posObj = IVec3(0, SIZE_BRICK_Y, 0);

	memset(&actor->_staticFlags, 0, sizeof(StaticFlagsStruct));
	memset(&actor->_workFlags, 0, sizeof(DynamicFlagsStruct));
	memset(&actor->_bonusParameter, 0, sizeof(BonusParameter));

	_engine->_movements->initRealAngle(LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, &actor->realAngle);
}

void Actor::hitObj(int32 actorIdx, int32 actorIdxAttacked, int32 hitforce, int32 angle) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdxAttacked);
	if (actor->_lifePoint <= 0) {
		return;
	}

	if (IS_HERO(actorIdxAttacked) && _engine->_debugState->_godMode) {
		return;
	}

	actor->_hitBy = actorIdx;

	if (actor->_armor <= hitforce) {
		if (actor->_genAnim == AnimationTypes::kBigHit || actor->_genAnim == AnimationTypes::kHit2) {
			if (actor->_nextGenAnim != AnimationTypes::kStanding) {
				const int32 tmpAnimPos = actor->_frame;
				actor->_frame = 1;
				_engine->_animations->processAnimActions(actorIdxAttacked);
				actor->_frame = tmpAnimPos;
			}

		} else {
			if (angle != -1) {
				_engine->_movements->initRealAngle(angle, angle, LBAAngles::ANGLE_0, &actor->realAngle);
			}

			if (_engine->getRandomNumber() & 1) {
				_engine->_animations->initAnim(AnimationTypes::kHit2, AnimType::kAnimationInsert, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			} else {
				_engine->_animations->initAnim(AnimationTypes::kBigHit, AnimType::kAnimationInsert, AnimationTypes::kAnimInvalid, actorIdxAttacked);
			}
		}

		_engine->_extra->initSpecial(actor->_posObj.x, actor->_posObj.y + 1000, actor->_posObj.z, ExtraSpecialType::kHitStars);

		if (!actorIdxAttacked) {
			_engine->_movements->_lastJoyFlag = true;
		}

		actor->addLife(-hitforce);
	} else {
		_engine->_animations->initAnim(AnimationTypes::kHit, AnimType::kAnimationInsert, AnimationTypes::kAnimInvalid, actorIdxAttacked);
	}
}

void Actor::checkCarrier(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (!actor->_staticFlags.bIsCarrierActor) {
		return;
	}
	for (int32 a = 0; a < _engine->_scene->_nbObjets; a++) {
		if (actor->_carryBy == actorIdx) {
			actor->_carryBy = -1;
		}
	}
}

void Actor::giveExtraBonus(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	const int bonusSprite = _engine->_extra->getBonusSprite(actor->_bonusParameter);
	if (bonusSprite == -1) {
		return;
	}
	if (actor->_workFlags.bIsDead) {
		_engine->_extra->addExtraBonus(actor->posObj(), LBAAngles::ANGLE_90, LBAAngles::ANGLE_0, bonusSprite, actor->_bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, actor->posObj(), actorIdx);
	} else {
		const ActorStruct *sceneHero = _engine->_scene->_sceneHero;
		const int32 angle = _engine->_movements->getAngle(actor->posObj(), sceneHero->posObj());
		const IVec3 pos(actor->_posObj.x, actor->_posObj.y + actor->_boundingBox.maxs.y, actor->_posObj.z);
		_engine->_extra->addExtraBonus(pos, LBAAngles::ANGLE_70, angle, bonusSprite, actor->_bonusAmount);
		_engine->_sound->playSample(Samples::ItemPopup, 1, pos, actorIdx);
	}
}

// Lba2
#define	START_AROUND_BETA	1024
#define	END_AROUND_BETA		3072
#define	STEP_AROUND_BETA	128	// 16 pos testees
#define GetAngle2D(x0, z0, x1, z1) GetAngleVector2D((x1) - (x0), (z1) - (z0))

void Actor::posObjectAroundAnother(uint8 numsrc, uint8 numtopos) {
#if 0
	ActorStruct *objsrc;
	ActorStruct *objtopos;
	int32 beta, dist, dist2;
	int32 step;

	objsrc = _engine->_scene->getActor(numsrc);
	objtopos = _engine->_scene->getActor(numtopos);

	int32 xb = objsrc->Obj.X;
	int32 zb = objsrc->Obj.Z;

	objtopos->Obj.Y = objsrc->Obj.Y;

	dist = MAX(objsrc->XMin, objsrc->XMax);
	dist = MAX(dist, objsrc->ZMin);
	dist = MAX(dist, objsrc->ZMax);

	dist2 = MAX(objtopos->XMin, objtopos->XMax);
	dist2 = MAX(dist2, objtopos->ZMin);
	dist2 = MAX(dist2, objtopos->ZMax);

	dist += dist / 2 + dist2 + dist2 / 2;

	beta = ClampAngle(objsrc->Obj.Beta + START_AROUND_BETA);

	for (step = 0; step < (4096 / STEP_AROUND_BETA); step++, beta += STEP_AROUND_BETA) {
		beta &= 4095;
		_engine->_renderer->rotate(0, dist, beta);

		objtopos->Obj.X = xb + X0;
		objtopos->Obj.Z = zb + Z0;

		if (_engine->_collision->checkValidObjPos(numtopos, numsrc)) {
			// accepte position
			break;
		}
	}

	objtopos->Obj.Beta = ClampAngle(GetAngle2D(xb, zb, objtopos->Obj.X, objtopos->Obj.Z));
#endif
}

int16 ActorMoveStruct::getRealValueFromTime(int32 time) {
	if (timeValue) {
		const int32 delta = time - memoTicks;

		if (delta >= timeValue) { // rotation is finished
			timeValue = 0;
			return endValue;
		}

		int32 t = ((endValue - startValue) * delta) / timeValue;
		t += startValue;

		return (int16)t;
	}

	return endValue;
}

int16 ActorMoveStruct::getRealAngle(int32 time) {
	if (timeValue) {
		int32 delta = time - memoTicks;
		if (delta < timeValue) {
			int32 t = NormalizeAngle(endValue - startValue);
			t = (t * delta) / timeValue;
			t += startValue;
			return (int16)t;
		}

		timeValue = 0;
	}

	return endValue;
}

bool ActorStruct::isAttackAnimationActive() const {
	return _genAnim == AnimationTypes::kRightPunch || _genAnim == AnimationTypes::kLeftPunch || _genAnim == AnimationTypes::kKick;
}

bool ActorStruct::isAttackWeaponAnimationActive() const {
	return _genAnim == AnimationTypes::kSabreAttack || _genAnim == AnimationTypes::kThrowBall || _genAnim == AnimationTypes::kSabreUnknown;
}

bool ActorStruct::isJumpAnimationActive() const {
	return _genAnim == AnimationTypes::kJump;
}

} // namespace TwinE
