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

#include "twine/extra.h"
#include "common/memstream.h"
#include "common/util.h"
#include "twine/actor.h"
#include "twine/collision.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/input.h"
#include "twine/interface.h"
#include "twine/movements.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/sound.h"
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

int32 Extra::addExtra(int32 actorIdx, int32 x, int32 y, int32 z, int32 info0, int32 targetActor, int32 maxSpeed, int32 strengthOfHit) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = info0;
		extra->type = 0x80;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->actorIdx = actorIdx;
		extra->lifeTime = targetActor;
		extra->destZ = maxSpeed;
		extra->strengthOfHit = strengthOfHit;

		_engine->_movements->setActorAngle(0, maxSpeed, 50, &extra->trackActorMove);
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
		extra->info0 = 0x61;
		extra->type = 0x1001;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->actorIdx = 0x28;
		extra->lifeTime = _engine->lbaTime;
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

void Extra::throwExtra(ExtraListStruct *extra, int32 var1, int32 var2, int32 var3, int32 var4) { // InitFly
	extra->type |= 2;

	extra->lastX = extra->x;
	extra->lastY = extra->y;
	extra->lastZ = extra->z;

	_engine->_movements->rotateActor(var3, 0, var1);

	extra->destY = -_engine->_renderer->destZ;

	_engine->_movements->rotateActor(0, _engine->_renderer->destX, var2);

	extra->destX = _engine->_renderer->destX;
	extra->destZ = _engine->_renderer->destZ;

	extra->angle = var4;
	extra->lifeTime = _engine->lbaTime;
}

void Extra::addExtraSpecial(int32 x, int32 y, int32 z, ExtraSpecialType type) { // InitSpecial
	const int16 flag = 0x8000 + type;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = flag;
		extra->info1 = 0;

		if (type == kHitStars) {
			extra->type = 9;

			extra->x = x;
			extra->y = y;
			extra->z = z;

			// same as InitFly
			throwExtra(extra, _engine->getRandomNumber(256) + 128, _engine->getRandomNumber(1024), 50, 20);

			extra->strengthOfHit = 0;
			extra->lifeTime = _engine->lbaTime;
			extra->actorIdx = 100;
		}
		if (type == kExplodeCloud) {
			extra->type = 1;

			extra->x = x;
			extra->y = y;
			extra->z = z;

			extra->strengthOfHit = 0;
			extra->lifeTime = _engine->lbaTime;
			extra->actorIdx = 5;
		}
		break;
	}
}

int32 Extra::addExtraBonus(int32 x, int32 y, int32 z, int32 param, int32 angle, int32 type, int32 bonusAmount) { // ExtraBonus
	int32 i;

	for (i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = type;
		extra->type = 0x4071;

		/*if(type == SPRITEHQR_KEY) {
			extra->type = 0x4030;
		}*/

		extra->x = x;
		extra->y = y;
		extra->z = z;

		// same as InitFly
		throwExtra(extra, param, angle, 40, 15);

		extra->strengthOfHit = 0;
		extra->lifeTime = _engine->lbaTime;
		extra->actorIdx = 1000;
		extra->info1 = bonusAmount;
		return i;
	}

	return -1;
}

int32 Extra::addExtraThrow(int32 actorIdx, int32 x, int32 y, int32 z, int32 sprite, int32 var2, int32 var3, int32 var4, int32 var5, int32 strengthOfHit) { // ThrowExtra
	int32 i;

	for (i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = sprite;
		extra->type = 0x210C;
		extra->x = x;
		extra->y = y;
		extra->z = z;

		// same as InitFly
		throwExtra(extra, var2, var3, var4, var5);

		extra->strengthOfHit = strengthOfHit;
		extra->lifeTime = _engine->lbaTime;
		extra->actorIdx = actorIdx;
		extra->info1 = 0;

		return i;
	}

	return -1;
}

int32 Extra::addExtraAiming(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActorIdx, int32 maxSpeed, int32 strengthOfHit) { // ExtraSearch
	int32 i;

	for (i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = spriteIdx;
		extra->type = 0x80;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->actorIdx = actorIdx;
		extra->lifeTime = targetActorIdx;
		extra->destZ = maxSpeed;
		extra->strengthOfHit = strengthOfHit;
		_engine->_movements->setActorAngle(0, maxSpeed, 50, &extra->trackActorMove);
		const ActorStruct *actor = _engine->_scene->getActor(targetActorIdx);
		extra->angle = _engine->_movements->getAngleAndSetTargetActorDistance(x, z, actor->x, actor->z);

		return i;
	}

	return -1;
}

// cseg01:00018168
int32 Extra::findExtraKey() {
	int32 i;

	for (i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 == SPRITEHQR_KEY) {
			return i;
		}
	}

	return -1;
}

// cseg01:00018250
int32 Extra::addExtraAimingAtKey(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 extraIdx) { // addMagicBallAimingAtKey
	int32 i;

	for (i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &extraList[i];
		if (extra->info0 != -1) {
			continue;
		}
		extra->info0 = spriteIdx;
		extra->type = 0x200;
		extra->info1 = 0;
		extra->x = x;
		extra->y = y;
		extra->z = z;
		extra->actorIdx = extraIdx;
		extra->destZ = 0x0FA0;
		extra->strengthOfHit = 0;
		_engine->_movements->setActorAngle(0, 0x0FA0, 50, &extra->trackActorMove);
		extra->angle = _engine->_movements->getAngleAndSetTargetActorDistance(x, z, extraList[extraIdx].x, extraList[extraIdx].z);

		return i;
	}

	return -1;
}

void Extra::addExtraThrowMagicball(int32 x, int32 y, int32 z, int32 param1, int32 angle, int32 param2, int32 param3) { // ThrowMagicBall
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
		_engine->_gameState->magicBallIdx = addExtraThrow(0, x, y, z, ballSprite, param1, angle, param2, param3, ballStrength);
		break;
	case 1:
		_engine->_gameState->magicBallAuxBounce = 4;
		_engine->_gameState->magicBallIdx = addExtraThrow(0, x, y, z, ballSprite, param1, angle, param2, param3, ballStrength);
		break;
	case 2:
	case 3:
	case 4:
		_engine->_gameState->magicBallNumBounce = 1;
		_engine->_gameState->magicBallAuxBounce = 4;
		_engine->_gameState->magicBallIdx = addExtraThrow(0, x, y, z, ballSprite, param1, angle, param2, param3, ballStrength);
		break;
	case 5:
		_engine->_gameState->magicBallIdx = addExtraAimingAtKey(0, x, y, z, ballSprite, extraIdx);
		break;
	}

	if (_engine->_gameState->inventoryMagicPoints > 0) {
		_engine->_gameState->inventoryMagicPoints--;
	}
}

void Extra::drawSpecialShape(const int16 *shapeTable, int32 x, int32 y, int32 color, int32 angle, int32 size) {
	int16 currentShapeTable = *(shapeTable++);

	int16 var_8 = ((*(shapeTable++)) * size) >> 4;
	int16 temp1 = ((*(shapeTable++)) * size) >> 4;

	_engine->_redraw->renderLeft = 0x7D00;
	_engine->_redraw->renderRight = -0x7D00;
	_engine->_redraw->renderTop = 0x7D00;
	_engine->_redraw->renderBottom = -0x7D00;

	_engine->_movements->rotateActor(var_8, temp1, angle);

	int32 computedX = _engine->_renderer->destX + x;
	int32 computedY = _engine->_renderer->destZ + y;

	if (computedX < _engine->_redraw->renderLeft) {
		_engine->_redraw->renderLeft = computedX;
	}

	if (computedX > _engine->_redraw->renderRight) {
		_engine->_redraw->renderRight = computedX;
	}

	if (computedY < _engine->_redraw->renderTop) {
		_engine->_redraw->renderTop = computedY;
	}

	if (computedY > _engine->_redraw->renderBottom) {
		_engine->_redraw->renderBottom = computedY;
	}

	int32 numEntries = 1;

	int32 currentX = computedX;
	int32 currentY = computedY;

	while (numEntries < currentShapeTable) {
		var_8 = ((*(shapeTable++)) * size) >> 4;
		temp1 = ((*(shapeTable++)) * size) >> 4;

		int32 oldComputedX = currentX;
		int32 oldComputedY = currentY;

		_engine->_renderer->projPosX = currentX;
		_engine->_renderer->projPosY = currentY;

		_engine->_movements->rotateActor(var_8, temp1, angle);

		currentX = _engine->_renderer->destX + x;
		currentY = _engine->_renderer->destZ + y;

		if (currentX < _engine->_redraw->renderLeft) {
			_engine->_redraw->renderLeft = currentX;
		}

		if (currentX > _engine->_redraw->renderRight) {
			_engine->_redraw->renderRight = currentX;
		}

		if (currentY < _engine->_redraw->renderTop) {
			_engine->_redraw->renderTop = currentY;
		}

		if (currentY > _engine->_redraw->renderBottom) {
			_engine->_redraw->renderBottom = currentY;
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
	int32 specialType;
	ExtraListStruct *extra = &extraList[extraIdx];

	specialType = extra->info0 & 0x7FFF;

	switch (specialType) {
	case kHitStars:
		drawSpecialShape(hitStarsShapeTable, x, y, 15, (_engine->lbaTime << 5) & 0x300, 4);
		break;
	case kExplodeCloud: {
		int32 cloudTime = 1 + _engine->lbaTime - extra->lifeTime;

		if (cloudTime > 32) {
			cloudTime = 32;
		}

		drawSpecialShape(explodeCloudShapeTable, x, y, 15, 0, cloudTime);
		break;
	}
	}
}

void Extra::processMagicballBounce(ExtraListStruct *extra, int32 x, int32 y, int32 z) {
	if (_engine->_grid->getBrickShape(x, extra->y, z)) {
		extra->destY = -extra->destY;
	}
	if (_engine->_grid->getBrickShape(extra->x, y, z)) {
		extra->destX = -extra->destX;
	}
	if (_engine->_grid->getBrickShape(x, y, extra->z)) {
		extra->destZ = -extra->destZ;
	}

	extra->x = x;
	extra->lastX = x;
	extra->y = y;
	extra->lastY = y;
	extra->z = z;
	extra->lastZ = z;

	extra->lifeTime = _engine->lbaTime;
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
		if (extra->type & 0x1) {
			if (extra->actorIdx + extra->lifeTime <= _engine->lbaTime) {
				extra->info0 = -1;
				continue;
			}
		}
		// reset extra
		if (extra->type & 0x800) {
			extra->info0 = -1;
			continue;
		}
		//
		if (extra->type & 0x1000) {
			extra->info0 = _engine->_collision->getAverageValue(97, 100, 30, _engine->lbaTime - extra->lifeTime);
			continue;
		}
		// process extra moving
		if (extra->type & 0x2) {
			currentExtraX = extra->x;
			currentExtraY = extra->y;
			currentExtraZ = extra->z;

			int32 currentExtraSpeedX = extra->destX * (_engine->lbaTime - extra->lifeTime);
			extra->x = currentExtraSpeedX + extra->lastX;

			int32 currentExtraSpeedY = extra->destY * (_engine->lbaTime - extra->lifeTime);
			currentExtraSpeedY += extra->lastY;
			extra->y = currentExtraSpeedY - ABS(((extra->angle * (_engine->lbaTime - extra->lifeTime)) * (_engine->lbaTime - extra->lifeTime)) >> 4);

			extra->z = extra->destZ * (_engine->lbaTime - extra->lifeTime) + extra->lastZ;

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
				if (extra->type & 0x20) {
					extra->type &= 0xFFED;
				} else {
					extra->info0 = -1;
				}

				continue;
			}
		}
		//
		if (extra->type & 0x4000) {
			if (_engine->lbaTime - extra->lifeTime > 40) {
				extra->type &= 0xBFFF;
			}
			continue;
		}
		// process actor target hit
		if (extra->type & 0x80) {
			int32 actorIdxAttacked = extra->lifeTime;
			int32 actorIdx = extra->actorIdx;

			const ActorStruct *actor = _engine->_scene->getActor(actorIdxAttacked);
			currentExtraX = actor->x;
			currentExtraY = actor->y + 1000;
			currentExtraZ = actor->z;

			int32 tmpAngle = _engine->_movements->getAngleAndSetTargetActorDistance(extra->x, extra->z, currentExtraX, currentExtraZ);
			int32 angle = (tmpAngle - extra->angle) & 0x3FF;

			if (angle > 400 && angle < 600) {
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
			int32 pos = _engine->_movements->getRealAngle(&extra->trackActorMove);
			if (!pos) {
				pos = 1;
			}

			_engine->_movements->rotateActor(pos, 0, angle2);
			extra->y -= _engine->_renderer->destZ;

			_engine->_movements->rotateActor(0, _engine->_renderer->destX, tmpAngle);
			extra->x += _engine->_renderer->destX;
			extra->z += _engine->_renderer->destZ;

			_engine->_movements->setActorAngle(0, extra->destZ, 50, &extra->trackActorMove);

			if (actorIdxAttacked == _engine->_collision->checkExtraCollisionWithActors(extra, actorIdx)) {
				if (i == _engine->_gameState->magicBallIdx) {
					_engine->_gameState->magicBallIdx = -1;
				}

				extra->info0 = -1;
				continue;
			}
		}
		// process magic ball extra aiming for key
		if (extra->type & 0x200) {
			//				int32 actorIdxAttacked = extra->lifeTime;
			ExtraListStruct *extraKey = &extraList[extra->actorIdx];
			int32 actorIdx = extra->actorIdx;

			int32 tmpAngle = _engine->_movements->getAngleAndSetTargetActorDistance(extra->x, extra->z, extraKey->x, extraKey->z);
			int32 angle = (tmpAngle - extra->angle) & 0x3FF;

			if (angle > 400 && angle < 600) {
				_engine->_sound->playSample(Samples::ItemFound, 4096, 1, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->y, _engine->_scene->sceneHero->z, 0);

				if (extraKey->info1 > 1) {
					_engine->_renderer->projectPositionOnScreen(extraKey->x - _engine->_grid->cameraX, extraKey->y - _engine->_grid->cameraY, extraKey->z - _engine->_grid->cameraZ);
					_engine->_redraw->addOverlay(koNumber, extraKey->info1, _engine->_renderer->projPosX, _engine->_renderer->projPosY, koNormal, 0, 2);
				}

				_engine->_redraw->addOverlay(koSprite, SPRITEHQR_KEY, 10, 30, koNormal, 0, 2);

				_engine->_gameState->inventoryNumKeys += extraKey->info1;
				extraKey->info0 = -1;

				extra->info0 = -1;
				_engine->_gameState->magicBallIdx = addExtra(-1, extra->x, extra->y, extra->z, SPRITEHQR_KEY, 0, 8000, 0);
				continue;
			}
			int32 angle2 = _engine->_movements->getAngleAndSetTargetActorDistance(extra->y, 0, extraKey->y, _engine->_movements->targetActorDistance);
			int32 pos = _engine->_movements->getRealAngle(&extra->trackActorMove);

			if (!pos) {
				pos = 1;
			}

			_engine->_movements->rotateActor(pos, 0, angle2);
			extra->y -= _engine->_renderer->destZ;

			_engine->_movements->rotateActor(0, _engine->_renderer->destX, tmpAngle);
			extra->x += _engine->_renderer->destX;
			extra->z += _engine->_renderer->destZ;

			_engine->_movements->setActorAngle(0, extra->destZ, 50, &extra->trackActorMove);

			if (actorIdx == _engine->_collision->checkExtraCollisionWithExtra(extra, _engine->_gameState->magicBallIdx)) {
				_engine->_sound->playSample(Samples::ItemFound, 4096, 1, _engine->_scene->sceneHero->x, _engine->_scene->sceneHero->y, _engine->_scene->sceneHero->z, 0);

				if (extraKey->info1 > 1) {
					_engine->_renderer->projectPositionOnScreen(extraKey->x - _engine->_grid->cameraX, extraKey->y - _engine->_grid->cameraY, extraKey->z - _engine->_grid->cameraZ);
					_engine->_redraw->addOverlay(koNumber, extraKey->info1, _engine->_renderer->projPosX, _engine->_renderer->projPosY, koNormal, 0, 2);
				}

				_engine->_redraw->addOverlay(koSprite, SPRITEHQR_KEY, 10, 30, koNormal, 0, 2);

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
		if (extra->type & 0x4) {
			if (_engine->_collision->checkExtraCollisionWithActors(extra, extra->actorIdx) != -1) {
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
		if (extra->type & 0x8) {
			int32 process = 0;

			if (_engine->_collision->checkExtraCollisionWithBricks(currentExtraX, currentExtraY, currentExtraZ, extra->x, extra->y, extra->z)) {
				// if not touch the ground
				if (!(extra->type & 0x2000)) {
					process = 1;
				}
			} else {
				// if touch the ground
				if (extra->type & 0x2000) {
					extra->type &= 0xDFFF; // set flag out of ground
				}
			}

			if (process) {
				// show explode cloud
				if (extra->type & 0x100) {
					addExtraSpecial(currentExtraX, currentExtraY, currentExtraZ, kExplodeCloud);
				}
				// if extra is magic ball
				if (i == _engine->_gameState->magicBallIdx) {
					_engine->_sound->playSample(Samples::Hit, _engine->getRandomNumber(300) + 3946, 1, extra->x, extra->y, extra->z);

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
		if (extra->type & 0x10) {
			int32 process = 0;

			if (_engine->_collision->checkExtraCollisionWithBricks(currentExtraX, currentExtraY, currentExtraZ, extra->x, extra->y, extra->z)) {
				// if not touch the ground
				if (!(extra->type & 0x2000)) {
					process = 1;
				}
			} else {
				// if touch the ground
				if (extra->type & 0x2000) {
					extra->type &= 0xDFFF; // set flag out of ground
				}
			}

			if (process) {
				Common::MemoryReadStream stream(_engine->_resources->spriteBoundingBoxPtr, _engine->_resources->spriteBoundingBoxSize);
				stream.seek(extra->info0 * 16);
				stream.skip(8);
				extra->y = (_engine->_collision->collisionY << 8) + 0x100 - stream.readSint16LE();
				extra->type &= 0xFFED;
				continue;
			}
		}
		// get extras on ground
		if ((extra->type & 0x20) && !(extra->type & 0x2)) {
			// if hero touch extra
			if (_engine->_collision->checkExtraCollisionWithActors(extra, -1) == 0) {
				_engine->_sound->playSample(Samples::ItemFound, 4096, 1, extra->x, extra->y, extra->z);

				if (extra->info1 > 1 && !_engine->_input->isActionActive(TwinEActionType::MoveBackward)) {
					_engine->_renderer->projectPositionOnScreen(extra->x - _engine->_grid->cameraX, extra->y - _engine->_grid->cameraY, extra->z - _engine->_grid->cameraZ);
					_engine->_redraw->addOverlay(koNumber, extra->info1, _engine->_renderer->projPosX, _engine->_renderer->projPosY, 158, koNormal, 2);
				}

				_engine->_redraw->addOverlay(koSprite, extra->info0, 10, 30, 0, koNormal, 2);

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
