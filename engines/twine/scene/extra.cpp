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

#include "twine/scene/extra.h"
#include "common/util.h"
#include "twine/audio/sound.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/renderer/redraw.h"
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

/** Hit Stars shape info */
static const ShapeData hitStarsData[]{
	{4, -6},
	{19, -6},
	{7, 2},
	{12, 16},
	{0, 7},
	{-12, 16},
	{-7, 2},
	{-19, -6},
	{-4, -6}};

/** Explode Cloud shape info */
static const ShapeData explodeCloudData[]{
	{0, -20},
	{6, -16},
	{8, -10},
	{14, -12},
	{20, -4},
	{18, 4},
	{12, 4},
	{16, 8},
	{8, 16},
	{2, 12},
	{-4, 18},
	{-10, 16},
	{-12, 8},
	{-16, 10},
	{-20, 4},
	{-12, -8},
	{-6, -6},
	{-10, -12}};

const ExtraShape hitStarsShape { ARRAYSIZE(hitStarsData), hitStarsData };
const ExtraShape explodeCloudShape { ARRAYSIZE(explodeCloudData), explodeCloudData };

Extra::Extra(TwinEEngine *engine) : _engine(engine) {}

int32 Extra::extraSearch(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActor, int32 maxSpeed, int32 strengthOfHit) { // ExtraSearch
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite != -1) {
			continue;
		}
		extra->sprite = spriteIdx;
		extra->type = ExtraType::SEARCH_OBJ;
		extra->info1 = 0;
		extra->pos.x = x;
		extra->pos.y = y;
		extra->pos.z = z;
		extra->payload.actorIdx = actorIdx;
		extra->spawnTime = targetActor;
		extra->destPos.z = maxSpeed;
		extra->strengthOfHit = strengthOfHit;

		_engine->_movements->initRealValue(LBAAngles::ANGLE_0, maxSpeed, LBAAngles::ANGLE_17, &extra->trackActorMove);
		const ActorStruct *actor = _engine->_scene->getActor(targetActor);
		extra->angle = _engine->_movements->getAngle(extra->pos, actor->posObj());
		return i;
	}
	return -1;
}

int32 Extra::addExtraExplode(int32 x, int32 y, int32 z) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite != -1) {
			continue;
		}
		extra->sprite = SPRITEHQR_EXPLOSION_FIRST_FRAME;
		extra->type = ExtraType::TIME_OUT | ExtraType::EXPLOSION;
		extra->info1 = 0;
		extra->pos.x = x;
		extra->pos.y = y;
		extra->pos.z = z;
		extra->payload.lifeTime = 40;
		extra->spawnTime = _engine->timerRef;
		extra->strengthOfHit = 0;
		return i;
	}
	return -1;
}

void Extra::clearExtra() {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		extra->sprite = -1;
		extra->info1 = 1;
	}
}

void Extra::initFly(ExtraListStruct *extra, int32 xAngle, int32 yAngle, int32 x, int32 extraAngle) {
	extra->type |= ExtraType::FLY;

	extra->lastPos = extra->pos;

	IVec2 destPos = _engine->_renderer->rotate(x, 0, xAngle);

	extra->destPos.y = -destPos.y;

	destPos = _engine->_renderer->rotate(0, destPos.x, yAngle);

	extra->destPos.x = destPos.x;
	extra->destPos.z = destPos.y;

	extra->angle = extraAngle;
	extra->spawnTime = _engine->timerRef;
}

int32 Extra::initSpecial(int32 x, int32 y, int32 z, ExtraSpecialType type) {
	const int16 flag = EXTRA_SPECIAL_MASK + (int16)type;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite != -1) {
			continue;
		}
		extra->sprite = flag;
		extra->info1 = 0;

		if (type == ExtraSpecialType::kHitStars) {
			extra->type = ExtraType::TIME_OUT | ExtraType::END_COL;

			extra->pos.x = x;
			extra->pos.y = y;
			extra->pos.z = z;

			initFly(extra, _engine->getRandomNumber(LBAAngles::ANGLE_90) + LBAAngles::ANGLE_45, _engine->getRandomNumber(LBAAngles::ANGLE_360), 50, 20);

			extra->strengthOfHit = 0;
			extra->payload.lifeTime = 100;
		} else if (type == ExtraSpecialType::kExplodeCloud) {
			extra->type = ExtraType::TIME_OUT;

			extra->pos.x = x;
			extra->pos.y = y;
			extra->pos.z = z;

			extra->strengthOfHit = 0;
			extra->spawnTime = _engine->timerRef;
			extra->payload.lifeTime = 5;
		}
		return i;
	}
	return -1;
}

int Extra::getBonusSprite(BonusParameter bonusParameter) const {
	int numBonus = 0;
	int8 bonusSprites[5];
	if (bonusParameter.kashes) {
		bonusSprites[numBonus++] = SPRITEHQR_KASHES;
	}
	if (bonusParameter.lifepoints) {
		bonusSprites[numBonus++] = SPRITEHQR_LIFEPOINTS;
	}
	if (bonusParameter.magicpoints) {
		bonusSprites[numBonus++] = SPRITEHQR_MAGICPOINTS;
	}
	if (bonusParameter.key) {
		bonusSprites[numBonus++] = SPRITEHQR_KEY;
	}
	if (bonusParameter.cloverleaf) {
		bonusSprites[numBonus++] = SPRITEHQR_CLOVERLEAF;
	}

	if (numBonus == 0) {
		return -1;
	}

	const int bonusIndex = _engine->getRandomNumber(numBonus);
	assert(bonusIndex >= 0);
	assert(bonusIndex < numBonus);
	int8 bonusSprite = bonusSprites[bonusIndex];
	// if bonus is magic and no magic level yet, then give life points
	if (!_engine->_gameState->_magicLevelIdx && bonusSprite == SPRITEHQR_MAGICPOINTS) {
		bonusSprite = SPRITEHQR_LIFEPOINTS;
	}

	return bonusSprite;
}

int32 Extra::addExtraBonus(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 type, int32 bonusAmount) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite != -1) {
			continue;
		}
		extra->sprite = type;
		extra->type = ExtraType::STOP_COL | ExtraType::TAKABLE | ExtraType::WAIT_SOME_TIME;

		if (type != SPRITEHQR_KEY) {
			extra->type |= ExtraType::TIME_OUT | ExtraType::FLASH;
		}

		extra->pos.x = x;
		extra->pos.y = y;
		extra->pos.z = z;

		initFly(extra, xAngle, yAngle, 40, ToAngle(15));

		extra->strengthOfHit = 0;
		extra->payload.lifeTime = _engine->toSeconds(20);
		extra->info1 = bonusAmount;
		return i;
	}

	return -1;
}

int32 Extra::throwExtra(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle, int32 strengthOfHit) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite != -1) {
			continue;
		}
		extra->sprite = spriteIdx;
		extra->type = ExtraType::END_OBJ | ExtraType::END_COL | ExtraType::IMPACT | ExtraType::WAIT_NO_COL;
		extra->pos.x = x;
		extra->pos.y = y;
		extra->pos.z = z;

		initFly(extra, xAngle, yAngle, xRotPoint, extraAngle);

		extra->strengthOfHit = strengthOfHit;
		extra->spawnTime = _engine->timerRef;
		extra->payload.actorIdx = actorIdx;
		extra->info1 = 0;

		return i;
	}

	return -1;
}

int32 Extra::addExtraAiming(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActorIdx, int32 finalAngle, int32 strengthOfHit) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite != -1) {
			continue;
		}
		extra->sprite = spriteIdx;
		extra->type = ExtraType::SEARCH_OBJ;
		extra->info1 = 0;
		extra->pos.x = x;
		extra->pos.y = y;
		extra->pos.z = z;
		extra->payload.actorIdx = actorIdx;
		extra->spawnTime = targetActorIdx;
		extra->destPos.z = finalAngle;
		extra->strengthOfHit = strengthOfHit;
		_engine->_movements->initRealValue(LBAAngles::ANGLE_0, finalAngle, LBAAngles::ANGLE_17, &extra->trackActorMove);
		const ActorStruct *actor = _engine->_scene->getActor(targetActorIdx);
		extra->angle = _engine->_movements->getAngle(extra->pos, actor->posObj());

		return i;
	}

	return -1;
}

int32 Extra::searchBonusKey() const {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		const ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite == SPRITEHQR_KEY) {
			return i;
		}
	}

	return -1;
}

int32 Extra::extraSearchKey(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 extraIdx) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite != -1) {
			continue;
		}
		extra->sprite = spriteIdx;
		extra->type = ExtraType::MAGIC_BALL_KEY;
		extra->info1 = 0;
		extra->pos.x = x;
		extra->pos.y = y;
		extra->pos.z = z;
		extra->payload.extraIdx = extraIdx;
		extra->destPos.z = 4000;
		extra->strengthOfHit = 0;
		_engine->_movements->initRealValue(LBAAngles::ANGLE_0, 4000, LBAAngles::ANGLE_17, &extra->trackActorMove);
		extra->angle = _engine->_movements->getAngle(extra->pos, _extraList[extraIdx].pos);

		return i;
	}

	return -1;
}

void Extra::addExtraThrowMagicball(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle) {
	int32 ballSprite = -1;
	int32 ballStrength = 0;

	switch (_engine->_gameState->_magicLevelIdx) {
	case 0:
	case 1:
		ballSprite = SPRITEHQR_MAGICBALL_YELLOW;
		ballStrength = 4;
		break;
	case 2:
		ballSprite = SPRITEHQR_MAGICBALL_GREEN;
		ballStrength = 6;
		break;
	case 3:
		ballSprite = SPRITEHQR_MAGICBALL_RED;
		ballStrength = 8;
		break;
	case 4:
		ballSprite = SPRITEHQR_MAGICBALL_FIRE;
		ballStrength = 10;
		break;
	}

	_engine->_gameState->_magicBallType = ((_engine->_gameState->_magicPoint - 1) / 20) + 1;
	if (_engine->_gameState->_magicPoint == 0) {
		_engine->_gameState->_magicBallType = 0;
	}

	const int32 extraIdx = searchBonusKey();
	if (extraIdx != -1) { // there is a key to aim
		_engine->_gameState->_magicBallType = 5;
	}

	switch (_engine->_gameState->_magicBallType) {
	case 0:
		_engine->_gameState->_magicBall = throwExtra(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, xAngle, yAngle, xRotPoint, extraAngle, ballStrength);
		break;
	case 1:
		_engine->_gameState->_magicBallCount = 4;
		_engine->_gameState->_magicBall = throwExtra(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, xAngle, yAngle, xRotPoint, extraAngle, ballStrength);
		break;
	case 2:
	case 3:
	case 4:
		_engine->_gameState->_magicBallType = 1;
		_engine->_gameState->_magicBallCount = 4;
		_engine->_gameState->_magicBall = throwExtra(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, xAngle, yAngle, xRotPoint, extraAngle, ballStrength);
		break;
	case 5:
		_engine->_gameState->_magicBall = extraSearchKey(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, extraIdx);
		break;
	}

	if (_engine->_gameState->_magicPoint > 0) {
		_engine->_gameState->_magicPoint--;
	}
}

void Extra::aff2DShape(const ExtraShape &shapeTable, int32 x, int32 y, int32 color, int32 angle, int32 zoom, Common::Rect &renderRect) {
	int shapeDataIndex = 0;
	int16 shapeX = shapeTable.data[shapeDataIndex].x * zoom / 16;
	int16 shapeZ = shapeTable.data[shapeDataIndex].z * zoom / 16;

	++shapeDataIndex;

	_engine->clearScreenMinMax(renderRect);

	IVec2 destPos = _engine->_renderer->rotate(shapeX, shapeZ, angle);

	const int32 computedX = destPos.x + x;
	const int32 computedY = destPos.y + y;

	_engine->adjustScreenMax(renderRect, computedX, computedY);

	int32 currentX = computedX;
	int32 currentY = computedY;

	for (int32 numEntries = 1; numEntries < shapeTable.n; ++numEntries) {
		shapeX = shapeTable.data[shapeDataIndex].x * zoom / 16;
		shapeZ = shapeTable.data[shapeDataIndex].z * zoom / 16;
		++shapeDataIndex;

		const int32 oldComputedX = currentX;
		const int32 oldComputedY = currentY;

		destPos = _engine->_renderer->rotate(shapeX, shapeZ, angle);

		currentX = destPos.x + x;
		currentY = destPos.y + y;

		_engine->adjustScreenMax(renderRect, currentX, currentY);
		_engine->_interface->drawLine(oldComputedX, oldComputedY, currentX, currentY, color);
	}
	_engine->_interface->drawLine(currentX, currentY, computedX, computedY, color);
}

void Extra::affSpecial(int32 extraIdx, int32 x, int32 y, Common::Rect &renderRect) {
	ExtraListStruct *extra = &_extraList[extraIdx];
	ExtraSpecialType specialType = (ExtraSpecialType)(extra->sprite & (EXTRA_SPECIAL_MASK - 1));

	switch (specialType) {
	case ExtraSpecialType::kHitStars:
		aff2DShape(hitStarsShape, x, y, COLOR_WHITE, (_engine->timerRef * 32) & LBAAngles::ANGLE_270, 4, renderRect);
		break;
	case ExtraSpecialType::kExplodeCloud: {
		int32 zoom = 1 + _engine->timerRef - extra->spawnTime;

		if (zoom > 32) {
			zoom = 32;
		}

		aff2DShape(explodeCloudShape, x, y, COLOR_WHITE, LBAAngles::ANGLE_0, zoom, renderRect);
		break;
	}
	case ExtraSpecialType::kFountain:
		break;
	}
}

void Extra::bounceExtra(ExtraListStruct *extra, int32 x, int32 y, int32 z) {
	if (_engine->_grid->worldColBrick(x, extra->pos.y, z) != ShapeType::kNone) {
		extra->destPos.y = -extra->destPos.y;
	}
	if (_engine->_grid->worldColBrick(extra->pos.x, y, z) != ShapeType::kNone) {
		extra->destPos.x = -extra->destPos.x;
	}
	if (_engine->_grid->worldColBrick(x, y, extra->pos.z) != ShapeType::kNone) {
		extra->destPos.z = -extra->destPos.z;
	}

	extra->pos.x = x;
	extra->lastPos.x = x;
	extra->pos.y = y;
	extra->lastPos.y = y;
	extra->pos.z = z;
	extra->lastPos.z = z;

	extra->spawnTime = _engine->timerRef;
}

void Extra::gereExtras() {
	int32 currentExtraX = 0;
	int32 currentExtraY = 0;
	int32 currentExtraZ = 0;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_extraList[i];
		if (extra->sprite == -1) {
			continue;
		}
		// process extra life time
		if (extra->type & ExtraType::TIME_OUT) {
			if (extra->payload.lifeTime + extra->spawnTime <= _engine->timerRef) {
				extra->sprite = -1;
				continue;
			}
		}
		// reset extra
		if (extra->type & ExtraType::ONE_FRAME) {
			extra->sprite = -1;
			continue;
		}
		const int32 deltaT = _engine->timerRef - extra->spawnTime;

		if (extra->type & ExtraType::EXPLOSION) {
			extra->sprite = boundRuleThree(SPRITEHQR_EXPLOSION_FIRST_FRAME, 100, 30, deltaT);
			continue;
		}
		// process extra moving
		if (extra->type & ExtraType::FLY) {
			currentExtraX = extra->pos.x;
			currentExtraY = extra->pos.y;
			currentExtraZ = extra->pos.z;

			const int32 currentExtraSpeedX = extra->destPos.x * deltaT;
			extra->pos.x = currentExtraSpeedX + extra->lastPos.x;

			const int32 currentExtraSpeedY = extra->destPos.y * deltaT;
			extra->pos.y = currentExtraSpeedY + extra->lastPos.y - ABS(extra->angle * deltaT * deltaT / 16);

			extra->pos.z = extra->destPos.z * deltaT + extra->lastPos.z;

			// check if extra is out of scene
			if (extra->pos.y < 0 || extra->pos.x < 0 || extra->pos.x > SCENE_SIZE_MAX || extra->pos.z < 0 || extra->pos.z > SCENE_SIZE_MAX) {
				// if extra is Magic Ball
				if (i == _engine->_gameState->_magicBall) {
					int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

					if (extra->sprite == SPRITEHQR_MAGICBALL_GREEN) {
						spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
					}
					if (extra->sprite == SPRITEHQR_MAGICBALL_RED) {
						spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
					}

					_engine->_gameState->_magicBall = extraSearch(-1, extra->pos.x, extra->pos.y, extra->pos.z,
							spriteIdx, OWN_ACTOR_SCENE_INDEX, 10000, 0);
				}

				// if can take extra on ground
				if (extra->type & ExtraType::TAKABLE) {
					extra->type &= ~(ExtraType::FLY | ExtraType::STOP_COL);
				} else {
					extra->sprite = -1;
				}

				continue;
			}
		}

		if (extra->type & ExtraType::WAIT_SOME_TIME) {
			if (_engine->timerRef - extra->spawnTime > 40) {
				extra->type &= ~ExtraType::WAIT_SOME_TIME;
			}
			continue;
		}
		// process actor target hit
		if (extra->type & ExtraType::SEARCH_OBJ) {
			int32 actorIdxAttacked = extra->spawnTime;
			int32 actorIdx = extra->payload.actorIdx;

			const ActorStruct *actor = _engine->_scene->getActor(actorIdxAttacked);
			currentExtraX = actor->_posObj.x;
			currentExtraY = actor->_posObj.y + 1000;
			currentExtraZ = actor->_posObj.z;

			const int32 tmpAngle = _engine->_movements->getAngle(extra->pos, actor->posObj());
			const int32 angle = ClampAngle(tmpAngle - extra->angle);

			if (angle > LBAAngles::ANGLE_140 && angle < LBAAngles::ANGLE_210) {
				if (extra->strengthOfHit) {
					_engine->_actor->hitObj(actorIdx, actorIdxAttacked, extra->strengthOfHit, -1);
				}

				if (i == _engine->_gameState->_magicBall) {
					_engine->_gameState->_magicBall = -1;
				}

				extra->sprite = -1;
				continue;
			}

			const int32 angle2 = _engine->_movements->getAngle(extra->pos.y, 0, currentExtraY, _engine->_movements->_targetActorDistance);
			int32 pos = extra->trackActorMove.getRealValueFromTime(_engine->timerRef);
			if (!pos) {
				pos = 1;
			}

			IVec2 destPos = _engine->_renderer->rotate(pos, 0, angle2);
			extra->pos.y -= destPos.y;

			destPos = _engine->_renderer->rotate(0, destPos.x, tmpAngle);
			extra->pos.x += destPos.x;
			extra->pos.z += destPos.y;

			_engine->_movements->initRealValue(LBAAngles::ANGLE_0, extra->destPos.z, LBAAngles::ANGLE_17, &extra->trackActorMove);

			if (actorIdxAttacked == _engine->_collision->extraCheckObjCol(extra, actorIdx)) {
				if (i == _engine->_gameState->_magicBall) {
					_engine->_gameState->_magicBall = -1;
				}

				extra->sprite = -1;
				continue;
			}
		}
		// process magic ball extra aiming for key
		if (extra->type & ExtraType::MAGIC_BALL_KEY) {
			ExtraListStruct *extraKey = &_extraList[extra->payload.extraIdx];
			const int32 extraIdx = extra->payload.extraIdx;

			const int32 tmpAngle = _engine->_movements->getAngle(extra->pos, extraKey->pos);
			const int32 angle = ClampAngle(tmpAngle - extra->angle);

			if (angle > LBAAngles::ANGLE_140 && angle < LBAAngles::ANGLE_210) {
				_engine->_sound->playSample(Samples::ItemFound, 1, _engine->_scene->_sceneHero->posObj(), OWN_ACTOR_SCENE_INDEX);

				if (extraKey->info1 > 1) {
					const IVec3 &projPos = _engine->_renderer->projectPoint(extraKey->pos - _engine->_grid->_worldCube);
					_engine->_redraw->addOverlay(OverlayType::koNumber, extraKey->info1, projPos.x, projPos.y, COLOR_BLACK, OverlayPosType::koNormal, 2);
				}

				_engine->_redraw->addOverlay(OverlayType::koSprite, SPRITEHQR_KEY, 10, 30, 0, OverlayPosType::koNormal, 2);

				_engine->_gameState->addKeys(extraKey->info1);
				extraKey->sprite = -1;

				extra->sprite = -1;
				_engine->_gameState->_magicBall = extraSearch(-1, extra->pos.x, extra->pos.y, extra->pos.z, SPRITEHQR_KEY, 0, 8000, 0);
				continue;
			}
			const int32 angle2 = _engine->_movements->getAngle(extra->pos.y, 0, extraKey->pos.y, _engine->_movements->_targetActorDistance);
			int32 pos = extra->trackActorMove.getRealValueFromTime(_engine->timerRef);

			if (!pos) {
				pos = 1;
			}

			IVec2 destPos = _engine->_renderer->rotate(pos, 0, angle2);
			extra->pos.y -= destPos.y;

			destPos = _engine->_renderer->rotate(0, destPos.x, tmpAngle);
			extra->pos.x += destPos.x;
			extra->pos.z += destPos.y;

			_engine->_movements->initRealValue(LBAAngles::ANGLE_0, extra->destPos.z, LBAAngles::ANGLE_17, &extra->trackActorMove);

			if (extraIdx == _engine->_collision->extraCheckExtraCol(extra, _engine->_gameState->_magicBall)) {
				_engine->_sound->playSample(Samples::ItemFound, 1, _engine->_scene->_sceneHero->posObj(), OWN_ACTOR_SCENE_INDEX);

				if (extraKey->info1 > 1) {
					const IVec3 &projPos = _engine->_renderer->projectPoint(extraKey->pos - _engine->_grid->_worldCube);
					_engine->_redraw->addOverlay(OverlayType::koNumber, extraKey->info1, projPos.x, projPos.y, COLOR_BLACK, OverlayPosType::koNormal, 2);
				}

				_engine->_redraw->addOverlay(OverlayType::koSprite, SPRITEHQR_KEY, 10, 30, 0, OverlayPosType::koNormal, 2);

				_engine->_gameState->addKeys(extraKey->info1);
				extraKey->sprite = -1;

				extra->sprite = -1;
				_engine->_gameState->_magicBall = extraSearch(-1, extra->pos.x, extra->pos.y, extra->pos.z, SPRITEHQR_KEY, 0, 8000, 0);
				continue;
			}
			if (extraKey->sprite == -1) {
				int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

				if (extra->sprite == SPRITEHQR_MAGICBALL_GREEN) {
					spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
				}
				if (extra->sprite == SPRITEHQR_MAGICBALL_RED) {
					spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
				}

				extra->sprite = -1;
				_engine->_gameState->_magicBall = extraSearch(-1, extra->pos.x, extra->pos.y, extra->pos.z,
						spriteIdx, 0, 8000, 0);
				continue;
			}
		}
		// process extra collision with actors
		if (extra->type & ExtraType::END_OBJ) {
			if (_engine->_collision->extraCheckObjCol(extra, extra->payload.actorIdx) != -1) {
				// if extra is Magic Ball
				if (i == _engine->_gameState->_magicBall) {
					int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

					if (extra->sprite == SPRITEHQR_MAGICBALL_GREEN) {
						spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
					}
					if (extra->sprite == SPRITEHQR_MAGICBALL_RED) {
						spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
					}

					_engine->_gameState->_magicBall = extraSearch(-1, extra->pos.x, extra->pos.y, extra->pos.z,
							spriteIdx, 0, 10000, 0);
				}

				extra->sprite = -1;
				continue;
			}
		}
		// process extra collision with scene ground
		if (extra->type & ExtraType::END_COL) {
			bool flagcol = false;

			if (_engine->_collision->fullWorldColBrick(currentExtraX, currentExtraY, currentExtraZ, extra->pos)) {
				// if not touch the ground
				if (!(extra->type & ExtraType::WAIT_NO_COL)) {
					flagcol = true;
				}
			} else {
				// if touch the ground
				if (extra->type & ExtraType::WAIT_NO_COL) {
					extra->type &= ~ExtraType::WAIT_NO_COL; // set flag out of ground
				}
			}

			if (flagcol) {
				// show explode cloud
				if (extra->type & ExtraType::IMPACT) {
					initSpecial(currentExtraX, currentExtraY, currentExtraZ, ExtraSpecialType::kExplodeCloud);
				}
				// if extra is magic ball
				if (i == _engine->_gameState->_magicBall) {
					_engine->_sound->playSample(Samples::Hit, 1, extra->pos);

					// can't bounce with not magic points
					if (_engine->_gameState->_magicBallType <= 0) {
						int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

						if (extra->sprite == SPRITEHQR_MAGICBALL_GREEN) {
							spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
						}
						if (extra->sprite == SPRITEHQR_MAGICBALL_RED) {
							spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
						}

						_engine->_gameState->_magicBall = extraSearch(-1, extra->pos.x, extra->pos.y, extra->pos.z, spriteIdx, 0, 10000, 0);

						extra->sprite = -1;
						continue;
					}

					// if has magic points
					if (_engine->_gameState->_magicBallType == 1) {
						if (!_engine->_gameState->_magicBallCount--) {
							int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

							if (extra->sprite == SPRITEHQR_MAGICBALL_GREEN) {
								spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
							}
							if (extra->sprite == SPRITEHQR_MAGICBALL_RED) {
								spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
							}

							_engine->_gameState->_magicBall = extraSearch(-1, extra->pos.x, extra->pos.y, extra->pos.z, spriteIdx, 0, 10000, 0);

							extra->sprite = -1;
							continue;
						}
						bounceExtra(extra, currentExtraX, currentExtraY, currentExtraZ);
					}
				} else {
					extra->sprite = -1;
					continue;
				}
			}
		}
		// extra stop moving while collision with bricks
		if (extra->type & ExtraType::STOP_COL) {
			bool process = false;

			if (_engine->_collision->fullWorldColBrick(currentExtraX, currentExtraY, currentExtraZ, extra->pos)) {
				// if not touch the ground
				if (!(extra->type & ExtraType::WAIT_NO_COL)) {
					process = true;
				}
			} else {
				// if touch the ground
				if (extra->type & ExtraType::WAIT_NO_COL) {
					extra->type &= ~ExtraType::WAIT_NO_COL; // set flag out of ground
				}
			}

			if (process) {
				const BoundingBox *bbox = _engine->_resources->_spriteBoundingBox.bbox(extra->sprite);
				extra->pos.y = (_engine->_collision->_collision.y * SIZE_BRICK_Y) + SIZE_BRICK_Y - bbox->mins.y;
				extra->type &= ~(ExtraType::STOP_COL | ExtraType::FLY);
				continue;
			}
		}
		// get extras on ground
		if ((extra->type & ExtraType::TAKABLE) && !(extra->type & ExtraType::FLY)) {
			// if hero touch extra
			if (_engine->_collision->extraCheckObjCol(extra, -1) == 0) {
				_engine->_sound->playSample(Samples::ItemFound, 1, extra->pos);

				if (extra->info1 > 1) {
					const IVec3 &projPos = _engine->_renderer->projectPoint(extra->pos - _engine->_grid->_worldCube);
					const int16 fontColor = COLOR_158;
					_engine->_redraw->addOverlay(OverlayType::koNumber, extra->info1, projPos.x, projPos.y, fontColor, OverlayPosType::koNormal, 2);
				}

				_engine->_redraw->addOverlay(OverlayType::koSprite, extra->sprite, 10, 30, 0, OverlayPosType::koNormal, 2);

				if (extra->sprite == SPRITEHQR_KASHES) {
					_engine->_gameState->addKashes(extra->info1);
				} else if (extra->sprite == SPRITEHQR_LIFEPOINTS) {
					_engine->_scene->_sceneHero->addLife(extra->info1);
				} else if (extra->sprite == SPRITEHQR_MAGICPOINTS && _engine->_gameState->_magicLevelIdx) {
					_engine->_gameState->addMagicPoints(extra->info1 * 2);
				} else if (extra->sprite == SPRITEHQR_KEY) {
					_engine->_gameState->addKeys(extra->info1);
				} else if (extra->sprite == SPRITEHQR_CLOVERLEAF) {
					_engine->_gameState->addLeafs(extra->info1);
				}

				extra->sprite = -1;
			}
		}
	}
}

} // namespace TwinE
