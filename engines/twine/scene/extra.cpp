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

#include "twine/scene/extra.h"
#include "common/memstream.h"
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
#include "twine/twine.h"

namespace TwinE {

/** Hit Stars shape info */
static const int16 hitStarsShapeTable[] = {
    10,
    0,
    -20,
    4,
    -6,
    19,
    -6,
    7,
    2,
    12,
    16,
    0,
    7,
    -12,
    16,
    -7,
    2,
    -19,
    -6,
    -4,
    -6};

/** Explode Cloud shape info */
static const int16 explodeCloudShapeTable[] = {
    18,
    0,
    -20,
    6,
    -16,
    8,
    -10,
    14,
    -12,
    20,
    -4,
    18,
    4,
    12,
    4,
    16,
    8,
    8,
    16,
    2,
    12,
    -4,
    18,
    -10,
    16,
    -12,
    8,
    -16,
    10,
    -20,
    4,
    -12,
    -8,
    -6,
    -6,
    -10,
    -12};

Extra::Extra(TwinEEngine *engine) : _engine(engine) {}

int32 Extra::addExtra(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActor, int32 maxSpeed, int32 strengthOfHit) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = spriteIdx;
		extra->type = ExtraType::UNK7;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->payload.actorIdx = actorIdx;
		extra->spawnTime = targetActor;
		extra->destZ = maxSpeed;
		extra->strengthOfHit = strengthOfHit;

		_engine->_movements->setActorAngle(ANGLE_0, maxSpeed, ANGLE_17, &extra->trackActorMove);
		const ActorStruct *actor = _engine->_scene->getActor(targetActor);
		extra->angle = _engine->_movements->getAngleAndSetTargetActorDistance(x, z, actor->x, actor->z);
		return i;
	}
	return -1;
}

int32 Extra::addExtraExplode(int32 x, int32 y, int32 z) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = SPRITEHQR_EXPLOSION_FIRST_FRAME;
		extra->type = ExtraType::TIME_OUT | ExtraType::UNK12;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->payload.lifeTime = 40;
		extra->spawnTime = _engine->lbaTime;
		extra->strengthOfHit = 0;
		return i;
	}
	return -1;
}

void Extra::resetExtras() {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		extra->info0 = -1;
		extra->info1 = 1;
	}
}

void Extra::throwExtra(ExtraListStruct *extra, int32 xAngle, int32 yAngle, int32 x, int32 extraAngle) { // InitFly
	extra->type |= ExtraType::FLY;

	extra->lastX = extra->x;
	extra->lastY = extra->y;
	extra->lastZ = extra->z;

	_engine->_movements->rotateActor(x, 0, xAngle);

	extra->destY = -_engine->_renderer->destZ;

	_engine->_movements->rotateActor(0, _engine->_renderer->destX, yAngle);

	extra->destX = _engine->_renderer->destX;
	extra->destZ = _engine->_renderer->destZ;

	extra->angle = extraAngle;
	extra->spawnTime = _engine->lbaTime;
}

int32 Extra::addExtraSpecial(int32 x, int32 y, int32 z, ExtraSpecialType type) { // InitSpecial
	const int16 flag = 0x8000 + (int16)type;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = flag;
		extra->info1 = 0;

		if (type == ExtraSpecialType::kHitStars) {
			extra->type = ExtraType::TIME_OUT | ExtraType::UNK3;

			extra->x = x;
			extra->y = y;
			extra->z = z;

			// same as InitFly
			throwExtra(extra, _engine->getRandomNumber(ANGLE_90) + ANGLE_45, _engine->getRandomNumber(ANGLE_360), 50, 20);

			extra->strengthOfHit = 0;
			extra->payload.lifeTime = 100;
		} else if (type == ExtraSpecialType::kExplodeCloud) {
			extra->type = ExtraType::TIME_OUT;

			extra->x = x;
			extra->y = y;
			extra->z = z;

			extra->strengthOfHit = 0;
			extra->spawnTime = _engine->lbaTime;
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
	// if bonus is magic an no magic level yet, then give life points
	if (!_engine->_gameState->magicLevelIdx && bonusSprite == SPRITEHQR_MAGICPOINTS) {
		bonusSprite = SPRITEHQR_KASHES;
	}

	return bonusSprite;
}

int32 Extra::addExtraBonus(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 type, int32 bonusAmount) { // ExtraBonus
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = type;
		extra->type = ExtraType::TIME_OUT | ExtraType::TAKABLE | ExtraType::FLASH | ExtraType::STOP_COL | ExtraType::BONUS;

		/*if(type == SPRITEHQR_KEY) {
			extra->type = ExtraFlag::STOP_COL | ExtraFlag::TAKABLE | ExtraFlag::BONUS;
		}*/

		extra->x = x;
		extra->y = y;
		extra->z = z;

		// same as InitFly
		throwExtra(extra, xAngle, yAngle, 40, ToAngle(15));

		extra->strengthOfHit = 0;
		extra->payload.lifeTime = 1000;
		extra->info1 = bonusAmount;
		return i;
	}

	return -1;
}

int32 Extra::addExtraThrow(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle, int32 strengthOfHit) { // ThrowExtra
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = spriteIdx;
		extra->type = ExtraType::UNK2 | ExtraType::UNK3 | ExtraType::UNK8 | ExtraType::WAIT_NO_COL;
		extra->x = x;
		extra->y = y;
		extra->z = z;

		// same as InitFly
		throwExtra(extra, xAngle, yAngle, xRotPoint, extraAngle);

		extra->strengthOfHit = strengthOfHit;
		extra->spawnTime = _engine->lbaTime;
		extra->payload.actorIdx = actorIdx;
		extra->info1 = 0;

		return i;
	}

	return -1;
}

int32 Extra::addExtraAiming(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActorIdx, int32 finalAngle, int32 strengthOfHit) { // ExtraSearch
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = spriteIdx;
		extra->type = ExtraType::UNK7;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->payload.actorIdx = actorIdx;
		extra->spawnTime = targetActorIdx;
		extra->destZ = finalAngle;
		extra->strengthOfHit = strengthOfHit;
		_engine->_movements->setActorAngle(ANGLE_0, finalAngle, ANGLE_17, &extra->trackActorMove);
		const ActorStruct *actor = _engine->_scene->getActor(targetActorIdx);
		extra->angle = _engine->_movements->getAngleAndSetTargetActorDistance(x, z, actor->x, actor->z);

		return i;
	}

	return -1;
}

// cseg01:00018168
int32 Extra::findExtraKey() {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 == SPRITEHQR_KEY) {
			return i;
		}
	}

	return -1;
}

// cseg01:00018250
int32 Extra::addExtraAimingAtKey(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 extraIdx) { // addMagicBallAimingAtKey
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = spriteIdx;
		extra->type = ExtraType::UNK9;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->payload.extraIdx = extraIdx;
		extra->destZ = 4000;
		extra->strengthOfHit = 0;
		_engine->_movements->setActorAngle(ANGLE_0, 4000, ANGLE_17, &extra->trackActorMove);
		extra->angle = _engine->_movements->getAngleAndSetTargetActorDistance(x, z, extraList[extraIdx].x, extraList[extraIdx].z);

		return i;
	}

	return -1;
}

void Extra::addExtraThrowMagicball(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle) { // ThrowMagicBall
	int32 ballSprite = -1;
	int32 ballStrength = 0;
	int32 extraIdx = -1;

	// TODO: check against MagicballStrengthType
	switch (_engine->_gameState->magicLevelIdx) {
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

	_engine->_gameState->magicBallNumBounce = ((_engine->_gameState->inventoryMagicPoints - 1) / 20) + 1;
	if (_engine->_gameState->inventoryMagicPoints == 0) {
		_engine->_gameState->magicBallNumBounce = 0;
	}

	extraIdx = findExtraKey();
	if (extraIdx != -1) { // there is a key to aim
		_engine->_gameState->magicBallNumBounce = 5;
	}

	switch (_engine->_gameState->magicBallNumBounce) {
	case 0:
		_engine->_gameState->magicBallIdx = addExtraThrow(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, xAngle, yAngle, xRotPoint, extraAngle, ballStrength);
		break;
	case 1:
		_engine->_gameState->magicBallAuxBounce = 4;
		_engine->_gameState->magicBallIdx = addExtraThrow(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, xAngle, yAngle, xRotPoint, extraAngle, ballStrength);
		break;
	case 2:
	case 3:
	case 4:
		_engine->_gameState->magicBallNumBounce = 1;
		_engine->_gameState->magicBallAuxBounce = 4;
		_engine->_gameState->magicBallIdx = addExtraThrow(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, xAngle, yAngle, xRotPoint, extraAngle, ballStrength);
		break;
	case 5:
		_engine->_gameState->magicBallIdx = addExtraAimingAtKey(OWN_ACTOR_SCENE_INDEX, x, y, z, ballSprite, extraIdx);
		break;
	}

	if (_engine->_gameState->inventoryMagicPoints > 0) {
		_engine->_gameState->inventoryMagicPoints--;
	}
}

void Extra::drawSpecialShape(const int16 *shapeTable, int32 x, int32 y, int32 color, int32 angle, int32 size) {
	int16 currentShapeTable = *(shapeTable++);

	int16 var_x = ((*(shapeTable++)) * size) >> 4;
	int16 var_z = ((*(shapeTable++)) * size) >> 4;

	_engine->_redraw->renderRect.left = 0x7D00;
	_engine->_redraw->renderRect.right = -0x7D00;
	_engine->_redraw->renderRect.top = 0x7D00;
	_engine->_redraw->renderRect.bottom = -0x7D00;

	_engine->_movements->rotateActor(var_x, var_z, angle);

	int32 computedX = _engine->_renderer->destX + x;
	int32 computedY = _engine->_renderer->destZ + y;

	if (computedX < _engine->_redraw->renderRect.left) {
		_engine->_redraw->renderRect.left = computedX;
	}

	if (computedX > _engine->_redraw->renderRect.right) {
		_engine->_redraw->renderRect.right = computedX;
	}

	if (computedY < _engine->_redraw->renderRect.top) {
		_engine->_redraw->renderRect.top = computedY;
	}

	if (computedY > _engine->_redraw->renderRect.bottom) {
		_engine->_redraw->renderRect.bottom = computedY;
	}

	int32 numEntries = 1;

	int32 currentX = computedX;
	int32 currentY = computedY;

	while (numEntries < currentShapeTable) {
		var_x = ((*(shapeTable++)) * size) >> 4;
		var_z = ((*(shapeTable++)) * size) >> 4;

		int32 oldComputedX = currentX;
		int32 oldComputedY = currentY;

		_engine->_renderer->projPosX = currentX;
		_engine->_renderer->projPosY = currentY;

		_engine->_movements->rotateActor(var_x, var_z, angle);

		currentX = _engine->_renderer->destX + x;
		currentY = _engine->_renderer->destZ + y;

		if (currentX < _engine->_redraw->renderRect.left) {
			_engine->_redraw->renderRect.left = currentX;
		}

		if (currentX > _engine->_redraw->renderRect.right) {
			_engine->_redraw->renderRect.right = currentX;
		}

		if (currentY < _engine->_redraw->renderRect.top) {
			_engine->_redraw->renderRect.top = currentY;
		}

		if (currentY > _engine->_redraw->renderRect.bottom) {
			_engine->_redraw->renderRect.bottom = currentY;
		}

		_engine->_renderer->projPosX = currentX;
		_engine->_renderer->projPosY = currentY;

		_engine->_interface->drawLine(oldComputedX, oldComputedY, currentX, currentY, color);

		numEntries++;

		currentX = _engine->_renderer->projPosX;
		currentY = _engine->_renderer->projPosY;
	}

	_engine->_renderer->projPosX = currentX;
	_engine->_renderer->projPosY = currentY;
	_engine->_interface->drawLine(currentX, currentY, computedX, computedY, color);
}

void Extra::drawExtraSpecial(int32 extraIdx, int32 x, int32 y) {
	ExtraListStruct *extra = &extraList[extraIdx];
	ExtraSpecialType specialType = (ExtraSpecialType)(extra->info0 & 0x7FFF);

	switch (specialType) {
	case ExtraSpecialType::kHitStars:
		drawSpecialShape(hitStarsShapeTable, x, y, 15, (_engine->lbaTime << 5) & ANGLE_270, 4);
		break;
	case ExtraSpecialType::kExplodeCloud: {
		int32 cloudTime = 1 + _engine->lbaTime - extra->spawnTime;

		if (cloudTime > 32) {
			cloudTime = 32;
		}

		drawSpecialShape(explodeCloudShapeTable, x, y, 15, 0, cloudTime);
		break;
	}
	}
}

void Extra::processMagicballBounce(ExtraListStruct *extra, int32 x, int32 y, int32 z) {
	if (_engine->_grid->getBrickShape(x, extra->y, z) != ShapeType::kNone) {
		extra->destY = -extra->destY;
	}
	if (_engine->_grid->getBrickShape(extra->x, y, z) != ShapeType::kNone) {
		extra->destX = -extra->destX;
	}
	if (_engine->_grid->getBrickShape(x, y, extra->z) != ShapeType::kNone) {
		extra->destZ = -extra->destZ;
	}

	extra->x = x;
	extra->lastX = x;
	extra->y = y;
	extra->lastY = y;
	extra->z = z;
	extra->lastZ = z;

	extra->spawnTime = _engine->lbaTime;
}

void Extra::processExtras() {
	int32 currentExtraX = 0;
	int32 currentExtraY = 0;
	int32 currentExtraZ = 0;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 == -1) {
			continue;
		}
		// process extra life time
		if (extra->type & ExtraType::TIME_OUT) {
			if (extra->payload.lifeTime + extra->spawnTime <= _engine->lbaTime) {
				extra->info0 = -1;
				continue;
			}
		}
		// reset extra
		if (extra->type & ExtraType::UNK11) {
			extra->info0 = -1;
			continue;
		}
		//
		if (extra->type & ExtraType::UNK12) {
			extra->info0 = _engine->_collision->getAverageValue(97, 100, 30, _engine->lbaTime - extra->spawnTime);
			continue;
		}
		// process extra moving
		if (extra->type & ExtraType::FLY) {
			currentExtraX = extra->x;
			currentExtraY = extra->y;
			currentExtraZ = extra->z;

			int32 currentExtraSpeedX = extra->destX * (_engine->lbaTime - extra->spawnTime);
			extra->x = currentExtraSpeedX + extra->lastX;

			int32 currentExtraSpeedY = extra->destY * (_engine->lbaTime - extra->spawnTime);
			currentExtraSpeedY += extra->lastY;
			extra->y = currentExtraSpeedY - ABS(((extra->angle * (_engine->lbaTime - extra->spawnTime)) * (_engine->lbaTime - extra->spawnTime)) >> 4);

			extra->z = extra->destZ * (_engine->lbaTime - extra->spawnTime) + extra->lastZ;

			// check if extra is out of scene
			if (extra->y < 0 || extra->x < 0 || extra->x > 0x7E00 || extra->z < 0 || extra->z > 0x7E00) {
				// if extra is Magic Ball
				if (i == _engine->_gameState->magicBallIdx) {
					int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

					if (extra->info0 == SPRITEHQR_MAGICBALL_GREEN) {
						spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
					}
					if (extra->info0 == SPRITEHQR_MAGICBALL_RED) {
						spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
					}

					_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, spriteIdx, 0, 10000, 0);
				}

				// if can take extra on ground
				if (extra->type & ExtraType::TAKABLE) {
					extra->type &= ~(ExtraType::FLY | ExtraType::STOP_COL);
				} else {
					extra->info0 = -1;
				}

				continue;
			}
		}
		//
		if (extra->type & ExtraType::BONUS) {
			if (_engine->lbaTime - extra->spawnTime > 40) {
				extra->type &= ~ExtraType::BONUS;
			}
			continue;
		}
		// process actor target hit
		if (extra->type & ExtraType::UNK7) {
			int32 actorIdxAttacked = extra->spawnTime;
			int32 actorIdx = extra->payload.actorIdx;

			const ActorStruct *actor = _engine->_scene->getActor(actorIdxAttacked);
			currentExtraX = actor->x;
			currentExtraY = actor->y + 1000;
			currentExtraZ = actor->z;

			int32 tmpAngle = _engine->_movements->getAngleAndSetTargetActorDistance(extra->x, extra->z, currentExtraX, currentExtraZ);
			int32 angle = ClampAngle(tmpAngle - extra->angle);

			if (angle > ANGLE_140 && angle < ANGLE_210) {
				if (extra->strengthOfHit) {
					_engine->_actor->hitActor(actorIdx, actorIdxAttacked, extra->strengthOfHit, -1);
				}

				if (i == _engine->_gameState->magicBallIdx) {
					_engine->_gameState->magicBallIdx = -1;
				}

				extra->info0 = -1;
				continue;
			}

			const int32 angle2 = _engine->_movements->getAngleAndSetTargetActorDistance(extra->y, 0, currentExtraY, _engine->_movements->targetActorDistance);
			int32 pos = extra->trackActorMove.getRealAngle(_engine->lbaTime);
			if (!pos) {
				pos = 1;
			}

			_engine->_movements->rotateActor(pos, 0, angle2);
			extra->y -= _engine->_renderer->destZ;

			_engine->_movements->rotateActor(0, _engine->_renderer->destX, tmpAngle);
			extra->x += _engine->_renderer->destX;
			extra->z += _engine->_renderer->destZ;

			_engine->_movements->setActorAngle(ANGLE_0, extra->destZ, ANGLE_17, &extra->trackActorMove);

			if (actorIdxAttacked == _engine->_collision->checkExtraCollisionWithActors(extra, actorIdx)) {
				if (i == _engine->_gameState->magicBallIdx) {
					_engine->_gameState->magicBallIdx = -1;
				}

				extra->info0 = -1;
				continue;
			}
		}
		// process magic ball extra aiming for key
		if (extra->type & ExtraType::UNK9) {
			//				int32 actorIdxAttacked = extra->lifeTime;
			ExtraListStruct *extraKey = &extraList[extra->payload.extraIdx];
			int32 extraIdx = extra->payload.extraIdx;

			int32 tmpAngle = _engine->_movements->getAngleAndSetTargetActorDistance(extra->x, extra->z, extraKey->x, extraKey->z);
			int32 angle = ClampAngle(tmpAngle - extra->angle);

			if (angle > ToAngle(400) && angle < ToAngle(600)) {
				_engine->_sound->playSample(Samples::ItemFound, 1, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->y, _engine->_scene->sceneHero->z, 0);

				if (extraKey->info1 > 1) {
					_engine->_renderer->projectPositionOnScreen(extraKey->x - _engine->_grid->cameraX, extraKey->y - _engine->_grid->cameraY, extraKey->z - _engine->_grid->cameraZ);
					_engine->_redraw->addOverlay(OverlayType::koNumber, extraKey->info1, _engine->_renderer->projPosX, _engine->_renderer->projPosY, 0, OverlayPosType::koNormal, 2);
				}

				_engine->_redraw->addOverlay(OverlayType::koSprite, SPRITEHQR_KEY, 10, 30, 0, OverlayPosType::koNormal, 2);

				_engine->_gameState->inventoryNumKeys += extraKey->info1;
				extraKey->info0 = -1;

				extra->info0 = -1;
				_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, SPRITEHQR_KEY, 0, 8000, 0);
				continue;
			}
			int32 angle2 = _engine->_movements->getAngleAndSetTargetActorDistance(extra->y, 0, extraKey->y, _engine->_movements->targetActorDistance);
			int32 pos = extra->trackActorMove.getRealAngle(_engine->lbaTime);

			if (!pos) {
				pos = 1;
			}

			_engine->_movements->rotateActor(pos, 0, angle2);
			extra->y -= _engine->_renderer->destZ;

			_engine->_movements->rotateActor(0, _engine->_renderer->destX, tmpAngle);
			extra->x += _engine->_renderer->destX;
			extra->z += _engine->_renderer->destZ;

			_engine->_movements->setActorAngle(ANGLE_0, extra->destZ, ANGLE_17, &extra->trackActorMove);

			if (extraIdx == _engine->_collision->checkExtraCollisionWithExtra(extra, _engine->_gameState->magicBallIdx)) {
				_engine->_sound->playSample(Samples::ItemFound, 1, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->y, _engine->_scene->sceneHero->z, 0);

				if (extraKey->info1 > 1) {
					_engine->_renderer->projectPositionOnScreen(extraKey->x - _engine->_grid->cameraX, extraKey->y - _engine->_grid->cameraY, extraKey->z - _engine->_grid->cameraZ);
					_engine->_redraw->addOverlay(OverlayType::koNumber, extraKey->info1, _engine->_renderer->projPosX, _engine->_renderer->projPosY, 0, OverlayPosType::koNormal, 2);
				}

				_engine->_redraw->addOverlay(OverlayType::koSprite, SPRITEHQR_KEY, 10, 30, 0, OverlayPosType::koNormal, 2);

				_engine->_gameState->inventoryNumKeys += extraKey->info1;
				extraKey->info0 = -1;

				extra->info0 = -1;
				_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, SPRITEHQR_KEY, 0, 8000, 0);
				continue;
			}
			if (extraKey->info0 == -1) {
				int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

				if (extra->info0 == SPRITEHQR_MAGICBALL_GREEN) {
					spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
				}
				if (extra->info0 == SPRITEHQR_MAGICBALL_RED) {
					spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
				}

				extra->info0 = -1;
				_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, spriteIdx, 0, 8000, 0);
				continue;
			}
		}
		// process extra collision with actors
		if (extra->type & ExtraType::UNK2) {
			if (_engine->_collision->checkExtraCollisionWithActors(extra, extra->payload.actorIdx) != -1) {
				// if extra is Magic Ball
				if (i == _engine->_gameState->magicBallIdx) {
					int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

					if (extra->info0 == SPRITEHQR_MAGICBALL_GREEN) {
						spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
					}
					if (extra->info0 == SPRITEHQR_MAGICBALL_RED) {
						spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
					}

					_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, spriteIdx, 0, 10000, 0);
				}

				extra->info0 = -1;
				continue;
			}
		}
		// process extra collision with scene ground
		if (extra->type & ExtraType::UNK3) {
			int32 process = 0;

			if (_engine->_collision->checkExtraCollisionWithBricks(currentExtraX, currentExtraY, currentExtraZ, extra->x, extra->y, extra->z)) {
				// if not touch the ground
				if (!(extra->type & ExtraType::WAIT_NO_COL)) {
					process = 1;
				}
			} else {
				// if touch the ground
				if (extra->type & ExtraType::WAIT_NO_COL) {
					extra->type &= ~ExtraType::WAIT_NO_COL; // set flag out of ground
				}
			}

			if (process) {
				// show explode cloud
				if (extra->type & ExtraType::UNK8) {
					addExtraSpecial(currentExtraX, currentExtraY, currentExtraZ, ExtraSpecialType::kExplodeCloud);
				}
				// if extra is magic ball
				if (i == _engine->_gameState->magicBallIdx) {
					_engine->_sound->playSample(Samples::Hit, 1, extra->x, extra->y, extra->z);

					// cant bounce with not magic points
					if (_engine->_gameState->magicBallNumBounce <= 0) {
						int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

						if (extra->info0 == SPRITEHQR_MAGICBALL_GREEN) {
							spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
						}
						if (extra->info0 == SPRITEHQR_MAGICBALL_RED) {
							spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
						}

						_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, spriteIdx, 0, 10000, 0);

						extra->info0 = -1;
						continue;
					}

					// if has magic points
					if (_engine->_gameState->magicBallNumBounce == 1) {
						if (!_engine->_gameState->magicBallAuxBounce--) {
							int32 spriteIdx = SPRITEHQR_MAGICBALL_YELLOW_TRANS;

							if (extra->info0 == SPRITEHQR_MAGICBALL_GREEN) {
								spriteIdx = SPRITEHQR_MAGICBALL_GREEN_TRANS;
							}
							if (extra->info0 == SPRITEHQR_MAGICBALL_RED) {
								spriteIdx = SPRITEHQR_MAGICBALL_RED_TRANS;
							}

							_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, spriteIdx, 0, 10000, 0);

							extra->info0 = -1;
							continue;
						}
						processMagicballBounce(extra, currentExtraX, currentExtraY, currentExtraZ);
					}
				} else {
					extra->info0 = -1;
					continue;
				}
			}
		}
		// extra stop moving while collision with bricks
		if (extra->type & ExtraType::STOP_COL) {
			int32 process = 0;

			if (_engine->_collision->checkExtraCollisionWithBricks(currentExtraX, currentExtraY, currentExtraZ, extra->x, extra->y, extra->z)) {
				// if not touch the ground
				if (!(extra->type & ExtraType::WAIT_NO_COL)) {
					process = 1;
				}
			} else {
				// if touch the ground
				if (extra->type & ExtraType::WAIT_NO_COL) {
					extra->type &= ~ExtraType::WAIT_NO_COL; // set flag out of ground
				}
			}

			if (process) {
				const BoundingBox *bbox = _engine->_resources->spriteBoundingBox.bbox(extra->info0);
				extra->y = (_engine->_collision->collisionY * BRICK_HEIGHT) + BRICK_HEIGHT - bbox->mins.y;
				extra->type &= ~(ExtraType::STOP_COL | ExtraType::FLY);
				continue;
			}
		}
		// get extras on ground
		if ((extra->type & ExtraType::TAKABLE) && !(extra->type & ExtraType::FLY)) {
			// if hero touch extra
			if (_engine->_collision->checkExtraCollisionWithActors(extra, -1) == 0) {
				_engine->_sound->playSample(Samples::ItemFound, 1, extra->x, extra->y, extra->z);

				if (extra->info1 > 1 && !_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
					_engine->_renderer->projectPositionOnScreen(extra->x - _engine->_grid->cameraX, extra->y - _engine->_grid->cameraY, extra->z - _engine->_grid->cameraZ);
					_engine->_redraw->addOverlay(OverlayType::koNumber, extra->info1, _engine->_renderer->projPosX, _engine->_renderer->projPosY, 158, OverlayPosType::koNormal, 2);
				}

				_engine->_redraw->addOverlay(OverlayType::koSprite, extra->info0, 10, 30, 0, OverlayPosType::koNormal, 2);

				if (extra->info0 == SPRITEHQR_KASHES) {
					_engine->_gameState->inventoryNumKashes += extra->info1;
					if (_engine->_gameState->inventoryNumKashes > 999) {
						_engine->_gameState->inventoryNumKashes = 999;
					}
				} else if (extra->info0 == SPRITEHQR_LIFEPOINTS) {
					_engine->_scene->sceneHero->life += extra->info1;
					if (_engine->_scene->sceneHero->life > 50) {
						_engine->_scene->sceneHero->life = 50;
					}
				} else if (extra->info0 == SPRITEHQR_MAGICPOINTS && _engine->_gameState->magicLevelIdx) {
					_engine->_gameState->inventoryMagicPoints += extra->info1 * 2;
					if (_engine->_gameState->inventoryMagicPoints > _engine->_gameState->magicLevelIdx * 20) {
						_engine->_gameState->inventoryMagicPoints = _engine->_gameState->magicLevelIdx * 20;
					}
				} else if (extra->info0 == SPRITEHQR_KEY) {
					_engine->_gameState->inventoryNumKeys += extra->info1;
				} else if (extra->info0 == SPRITEHQR_CLOVERLEAF) {
					_engine->_gameState->inventoryNumLeafs += extra->info1;
					if (_engine->_gameState->inventoryNumLeafs > _engine->_gameState->inventoryNumLeafsBox) {
						_engine->_gameState->inventoryNumLeafs = _engine->_gameState->inventoryNumLeafsBox;
					}
				}

				extra->info0 = -1;
			}
		}
	}
}

} // namespace TwinE
