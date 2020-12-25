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

#include "twine/scene/collision.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/util.h"
#include "twine/renderer/renderer.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/extra.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/twine.h"

namespace TwinE {

Collision::Collision(TwinEEngine *engine) : _engine(engine) {
}

bool Collision::standingOnActor(int32 actorIdx1, int32 actorIdx2) {
	const ActorStruct *actor1 = _engine->_scene->getActor(actorIdx1);
	const ActorStruct *actor2 = _engine->_scene->getActor(actorIdx2);

	// Current actor (actor 1)
	const int32 x1Left = _engine->_movements->processActorX + actor1->boudingBox.x.bottomLeft;
	const int32 x1Right = _engine->_movements->processActorX + actor1->boudingBox.x.topRight;

	const int32 y1Left = _engine->_movements->processActorY + actor1->boudingBox.y.bottomLeft;
	const int32 y1Right = _engine->_movements->processActorY + actor1->boudingBox.y.topRight;

	const int32 z1Left = _engine->_movements->processActorZ + actor1->boudingBox.z.bottomLeft;
	const int32 z1Right = _engine->_movements->processActorZ + actor1->boudingBox.z.topRight;

	// Actor 2
	const int32 x2Left = actor2->x + actor2->boudingBox.x.bottomLeft;
	const int32 x2Right = actor2->x + actor2->boudingBox.x.topRight;

	const int32 y2Left = actor2->y + actor2->boudingBox.y.bottomLeft;
	const int32 y2Right = actor2->y + actor2->boudingBox.y.topRight;

	const int32 z2Left = actor2->z + actor2->boudingBox.z.bottomLeft;
	const int32 z2Right = actor2->z + actor2->boudingBox.z.topRight;

	if (x1Left >= x2Right) {
		return false; // not standing
	}

	if (x1Right <= x2Left) {
		return false;
	}

	if (y1Left > (y2Right + 1)) {
		return false;
	}

	if (y1Left <= (y2Right - 0x100)) {
		return false;
	}

	if (y1Right <= y2Left) {
		return false;
	}

	if (z1Left >= z2Right) {
		return false;
	}

	if (z1Right <= z2Left) {
		return false;
	}

	return true; // standing
}

int32 Collision::getAverageValue(int32 var0, int32 var1, int32 var2, int32 var3) {
	if (var3 <= 0) {
		return var0;
	}

	if (var3 >= var2) {
		return var1;
	}

	return (((var1 - var0) * var3) / var2) + var0;
}

void Collision::reajustActorPosition(ShapeType brickShape) {
	if (brickShape == ShapeType::kNone) {
		return;
	}

	const int32 brkX = (collisionX * BRICK_SIZE) - BRICK_HEIGHT;
	const int32 brkY = collisionY * BRICK_HEIGHT;
	const int32 brkZ = (collisionZ * BRICK_SIZE) - BRICK_HEIGHT;

	// double-side stairs
	if (brickShape >= ShapeType::kDoubleSideStairsTop1 && brickShape <= ShapeType::kDoubleSideStairsRight2) {
		switch (brickShape) {
		case ShapeType::kDoubleSideStairsTop1:
			if (_engine->_movements->processActorZ - collisionZ <= _engine->_movements->processActorX - collisionX) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsTopRight;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom1:
			if (_engine->_movements->processActorZ - collisionZ <= _engine->_movements->processActorX - collisionX) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft1:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight1:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsTop2:
			if (_engine->_movements->processActorX - collisionX >= _engine->_movements->processActorZ - collisionZ) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom2:
			if (_engine->_movements->processActorZ - collisionZ <= _engine->_movements->processActorX - collisionX) {
				brickShape = ShapeType::kStairsBottomRight;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft2:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight2:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = ShapeType::kStairsBottomRight;
			} else {
				brickShape = ShapeType::kStairsTopRight;
			}
			break;
		default:
			if (_engine->cfgfile.Debug) {
				debug("Brick Shape %d unsupported", (int)brickShape);
			}
			break;
		}
	}

	if (brickShape >= ShapeType::kStairsTopLeft && brickShape <= ShapeType::kStairsBottomRight) {
		switch (brickShape) {
		case ShapeType::kStairsTopLeft:
			_engine->_movements->processActorY = brkY + getAverageValue(0, BRICK_HEIGHT, BRICK_SIZE, _engine->_movements->processActorX - brkX);
			break;
		case ShapeType::kStairsTopRight:
			_engine->_movements->processActorY = brkY + getAverageValue(0, BRICK_HEIGHT, BRICK_SIZE, _engine->_movements->processActorZ - brkZ);
			break;
		case ShapeType::kStairsBottomLeft:
			_engine->_movements->processActorY = brkY + getAverageValue(BRICK_HEIGHT, 0, BRICK_SIZE, _engine->_movements->processActorZ - brkZ);
			break;
		case ShapeType::kStairsBottomRight:
			_engine->_movements->processActorY = brkY + getAverageValue(BRICK_HEIGHT, 0, BRICK_SIZE, _engine->_movements->processActorX - brkX);
			break;
		default:
			break;
		}
	}
}

int32 Collision::checkCollisionWithActors(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	int32 xLeft = _engine->_movements->processActorX + actor->boudingBox.x.bottomLeft;
	int32 xRight = _engine->_movements->processActorX + actor->boudingBox.x.topRight;

	int32 yLeft = _engine->_movements->processActorY + actor->boudingBox.y.bottomLeft;
	int32 yRight = _engine->_movements->processActorY + actor->boudingBox.y.topRight;

	int32 zLeft = _engine->_movements->processActorZ + actor->boudingBox.z.bottomLeft;
	int32 zRight = _engine->_movements->processActorZ + actor->boudingBox.z.topRight;

	actor->collision = -1;

	for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
		ActorStruct *actorTest = _engine->_scene->getActor(a);

		// aviod current processed actor
		if (a != actorIdx && actorTest->entity != -1 && !actor->staticFlags.bComputeLowCollision && actorTest->standOn != actorIdx) {
			const int32 xLeftTest = actorTest->x + actorTest->boudingBox.x.bottomLeft;
			const int32 xRightTest = actorTest->x + actorTest->boudingBox.x.topRight;

			const int32 yLeftTest = actorTest->y + actorTest->boudingBox.y.bottomLeft;
			const int32 yRightTest = actorTest->y + actorTest->boudingBox.y.topRight;

			const int32 zLeftTest = actorTest->z + actorTest->boudingBox.z.bottomLeft;
			const int32 zRightTest = actorTest->z + actorTest->boudingBox.z.topRight;

			if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
				actor->collision = a; // mark as collision with actor a

				if (actorTest->staticFlags.bIsCarrierActor) {
					if (actor->dynamicFlags.bIsFalling) {
						_engine->_movements->processActorY = yRightTest - actor->boudingBox.y.bottomLeft + 1;
						actor->standOn = a;
					} else {
						if (standingOnActor(actorIdx, a)) {
							_engine->_movements->processActorY = yRightTest - actor->boudingBox.y.bottomLeft + 1;
							actor->standOn = a;
						} else {
							int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(_engine->_movements->processActorX, _engine->_movements->processActorZ, actorTest->x, actorTest->z);

							if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
								actorTest->lastY = 0;

								if (actorTest->staticFlags.bUseMiniZv) {
									if (newAngle >= ANGLE_45 && newAngle < ANGLE_135 && actor->angle > ANGLE_45 && actor->angle < ANGLE_135) {
										actorTest->lastX = 192;
									}
									if (newAngle >= ANGLE_135 && newAngle < ANGLE_225 && actor->angle > ANGLE_135 && actor->angle < ANGLE_225) {
										actorTest->lastZ = -64;
									}
									if (newAngle >= ANGLE_225 && newAngle < ANGLE_315 && actor->angle > ANGLE_225 && actor->angle < ANGLE_315) {
										actorTest->lastX = -64;
									}
									if ((newAngle >= ANGLE_315 || newAngle < ANGLE_45) && (actor->angle > ANGLE_315 || actor->angle < ANGLE_45)) {
										actorTest->lastX = 192;
									}
								} else {
									actorTest->lastX = _engine->_movements->processActorX - actor->collisionX;
									actorTest->lastZ = _engine->_movements->processActorZ - actor->collisionZ;
								}
							}

							if ((actorTest->boudingBox.x.topRight - actorTest->boudingBox.x.bottomLeft == actorTest->boudingBox.z.topRight - actorTest->boudingBox.z.bottomLeft) &&
							    (actor->boudingBox.x.topRight - actor->boudingBox.x.bottomLeft == actor->boudingBox.z.topRight - actor->boudingBox.z.bottomLeft)) {
								if (newAngle < ANGLE_135) {
									_engine->_movements->processActorX = xLeftTest - actor->boudingBox.x.topRight;
								}
								if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
									_engine->_movements->processActorZ = zRightTest - actor->boudingBox.z.bottomLeft;
								}
								if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
									_engine->_movements->processActorX = xRightTest - actor->boudingBox.x.bottomLeft;
								}
								if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
									_engine->_movements->processActorZ = zLeftTest - actor->boudingBox.z.topRight;
								}
							} else {
								if (!actor->dynamicFlags.bIsFalling) {
									_engine->_movements->processActorX = _engine->_movements->previousActorX;
									_engine->_movements->processActorY = _engine->_movements->previousActorY;
									_engine->_movements->processActorZ = _engine->_movements->previousActorZ;
								}
							}
						}
					}
				} else {
					if (standingOnActor(actorIdx, a)) {
						_engine->_actor->hitActor(actorIdx, a, 1, -1);
					}

					int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(_engine->_movements->processActorX, _engine->_movements->processActorZ, actorTest->x, actorTest->z);

					if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
						actorTest->lastY = 0;

						if (actorTest->staticFlags.bUseMiniZv) {
							if (newAngle >= ANGLE_45 && newAngle < ANGLE_135 && actor->angle > ANGLE_45 && actor->angle < ANGLE_135) {
								actorTest->lastX = 192;
							}
							if (newAngle >= ANGLE_135 && newAngle < ANGLE_225 && actor->angle > ANGLE_135 && actor->angle < ANGLE_225) {
								actorTest->lastZ = -64;
							}
							if (newAngle >= ANGLE_225 && newAngle < ANGLE_315 && actor->angle > ANGLE_225 && actor->angle < ANGLE_315) {
								actorTest->lastX = -64;
							}
							if ((newAngle >= ANGLE_315 || newAngle < ANGLE_45) && (actor->angle > ANGLE_315 || actor->angle < ANGLE_45)) {
								actorTest->lastX = 192;
							}
						} else {
							actorTest->lastX = _engine->_movements->processActorX - actor->collisionX;
							actorTest->lastZ = _engine->_movements->processActorZ - actor->collisionZ;
						}
					}

					if ((actorTest->boudingBox.x.topRight - actorTest->boudingBox.x.bottomLeft == actorTest->boudingBox.z.topRight - actorTest->boudingBox.z.bottomLeft) &&
					    (actor->boudingBox.x.topRight - actor->boudingBox.x.bottomLeft == actor->boudingBox.z.topRight - actor->boudingBox.z.bottomLeft)) {
						if (newAngle < ANGLE_135) {
							_engine->_movements->processActorX = xLeftTest - actor->boudingBox.x.topRight;
						}
						if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
							_engine->_movements->processActorZ = zRightTest - actor->boudingBox.z.bottomLeft;
						}
						if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
							_engine->_movements->processActorX = xRightTest - actor->boudingBox.x.bottomLeft;
						}
						if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
							_engine->_movements->processActorZ = zLeftTest - actor->boudingBox.z.topRight;
						}
					} else {
						if (!actor->dynamicFlags.bIsFalling) {
							_engine->_movements->processActorX = _engine->_movements->previousActorX;
							_engine->_movements->processActorY = _engine->_movements->previousActorY;
							_engine->_movements->processActorZ = _engine->_movements->previousActorZ;
						}
					}
				}
			}
		}
	}

	if (actor->dynamicFlags.bIsHitting) {
		_engine->_movements->rotateActor(0, 200, actor->angle);

		xLeft = _engine->_renderer->destX + _engine->_movements->processActorX + actor->boudingBox.x.bottomLeft;
		xRight = _engine->_renderer->destX + _engine->_movements->processActorX + actor->boudingBox.x.topRight;

		yLeft = _engine->_movements->processActorY + actor->boudingBox.y.bottomLeft;
		yRight = _engine->_movements->processActorY + actor->boudingBox.y.topRight;

		zLeft = _engine->_renderer->destZ + _engine->_movements->processActorZ + actor->boudingBox.z.bottomLeft;
		zRight = _engine->_renderer->destZ + _engine->_movements->processActorZ + actor->boudingBox.z.topRight;

		for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
			const ActorStruct *actorTest = _engine->_scene->getActor(a);

			// aviod current processed actor
			if (a != actorIdx && actorTest->entity != -1 && !actorTest->staticFlags.bIsHidden && actorTest->standOn != actorIdx) {
				const int32 xLeftTest = actorTest->x + actorTest->boudingBox.x.bottomLeft;
				const int32 xRightTest = actorTest->x + actorTest->boudingBox.x.topRight;

				const int32 yLeftTest = actorTest->y + actorTest->boudingBox.y.bottomLeft;
				const int32 yRightTest = actorTest->y + actorTest->boudingBox.y.topRight;

				const int32 zLeftTest = actorTest->z + actorTest->boudingBox.z.bottomLeft;
				const int32 zRightTest = actorTest->z + actorTest->boudingBox.z.topRight;

				if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
					_engine->_actor->hitActor(actorIdx, a, actor->strengthOfHit, actor->angle + ANGLE_90);
					actor->dynamicFlags.bIsHitting = 0;
				}
			}
		}
	}

	return actor->collision;
}

void Collision::checkHeroCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	ShapeType brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

	_engine->_movements->processActorX += x;
	_engine->_movements->processActorY += y;
	_engine->_movements->processActorZ += z;

	if (_engine->_movements->processActorX >= 0 && _engine->_movements->processActorZ >= 0 && _engine->_movements->processActorX <= 0x7E00 && _engine->_movements->processActorZ <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShapeFull(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ, _engine->_actor->processActorPtr->boudingBox.y.topRight);

		if (brickShape == ShapeType::kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShapeFull(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->previousActorZ + z, _engine->_actor->processActorPtr->boudingBox.y.topRight);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShapeFull(x + _engine->_movements->previousActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ, _engine->_actor->processActorPtr->boudingBox.y.topRight);

				if (brickShape != ShapeType::kSolid) {
					processCollisionX = _engine->_movements->previousActorX;
				}
			} else {
				processCollisionZ = _engine->_movements->previousActorZ;
			}
		}
	}

	_engine->_movements->processActorX = processCollisionX;
	_engine->_movements->processActorY = processCollisionY;
	_engine->_movements->processActorZ = processCollisionZ;
}

void Collision::checkActorCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	ShapeType brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

	_engine->_movements->processActorX += x;
	_engine->_movements->processActorY += y;
	_engine->_movements->processActorZ += z;

	if (_engine->_movements->processActorX >= 0 && _engine->_movements->processActorZ >= 0 && _engine->_movements->processActorX <= 0x7E00 && _engine->_movements->processActorZ <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

		if (brickShape == ShapeType::kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->previousActorZ + z);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShape(x + _engine->_movements->previousActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

				if (brickShape != ShapeType::kSolid) {
					processCollisionX = _engine->_movements->previousActorX;
				}
			} else {
				processCollisionZ = _engine->_movements->previousActorZ;
			}
		}
	}

	_engine->_movements->processActorX = processCollisionX;
	_engine->_movements->processActorY = processCollisionY;
	_engine->_movements->processActorZ = processCollisionZ;
}

void Collision::stopFalling() { // ReceptionObj()
	if (IS_HERO(_engine->_animations->currentlyProcessedActorIdx)) {
		const int32 fall = _engine->_scene->heroYBeforeFall - _engine->_movements->processActorY;

		if (fall >= BRICK_HEIGHT * 8) {
			_engine->_extra->addExtraSpecial(_engine->_actor->processActorPtr->x, _engine->_actor->processActorPtr->y + 1000, _engine->_actor->processActorPtr->z, ExtraSpecialType::kHitStars);
			_engine->_actor->processActorPtr->life--;
			_engine->_animations->initAnim(AnimationTypes::kLandingHit, 2, AnimationTypes::kStanding, _engine->_animations->currentlyProcessedActorIdx);
		} else if (fall > 10) {
			_engine->_animations->initAnim(AnimationTypes::kLanding, 2, AnimationTypes::kStanding, _engine->_animations->currentlyProcessedActorIdx);
		} else {
			_engine->_animations->initAnim(AnimationTypes::kStanding, 0, AnimationTypes::kStanding, _engine->_animations->currentlyProcessedActorIdx);
		}

		_engine->_scene->heroYBeforeFall = 0;
	} else {
		_engine->_animations->initAnim(AnimationTypes::kLanding, 2, _engine->_actor->processActorPtr->animExtra, _engine->_animations->currentlyProcessedActorIdx);
	}

	_engine->_actor->processActorPtr->dynamicFlags.bIsFalling = 0;
}

int32 Collision::checkExtraCollisionWithActors(ExtraListStruct *extra, int32 actorIdx) {
	const BoundingBox *bbox = _engine->_resources->spriteBoundingBox.bbox(extra->info0);
	const int32 xLeft = bbox->mins.x + extra->x;
	const int32 xRight = bbox->maxs.x + extra->x;
	const int32 yLeft = bbox->mins.y + extra->y;
	const int32 yRight = bbox->maxs.y + extra->y;
	const int32 zLeft = bbox->mins.z + extra->z;
	const int32 zRight = bbox->maxs.z + extra->z;

	for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
		const ActorStruct *actorTest = _engine->_scene->getActor(a);

		if (a != actorIdx && actorTest->entity != -1) {
			const int32 xLeftTest = actorTest->x + actorTest->boudingBox.x.bottomLeft;
			const int32 xRightTest = actorTest->x + actorTest->boudingBox.x.topRight;
			const int32 yLeftTest = actorTest->y + actorTest->boudingBox.y.bottomLeft;
			const int32 yRightTest = actorTest->y + actorTest->boudingBox.y.topRight;
			const int32 zLeftTest = actorTest->z + actorTest->boudingBox.z.bottomLeft;
			const int32 zRightTest = actorTest->z + actorTest->boudingBox.z.topRight;

			if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
				if (extra->strengthOfHit != 0) {
					_engine->_actor->hitActor(actorIdx, a, extra->strengthOfHit, -1);
				}

				return a;
			}
		}
	}

	return -1;
}

bool Collision::checkExtraCollisionWithBricks(int32 x, int32 y, int32 z, int32 oldX, int32 oldY, int32 oldZ) {
	if (_engine->_grid->getBrickShape(oldX, oldY, oldZ) != ShapeType::kNone) {
		return true;
	}

	const int32 averageX = ABS(x + oldX) / 2;
	const int32 averageY = ABS(y + oldY) / 2;
	const int32 averageZ = ABS(z + oldZ) / 2;

	if (_engine->_grid->getBrickShape(averageX, averageY, averageZ) != ShapeType::kNone) {
		return true;
	}

	if (_engine->_grid->getBrickShape(ABS(oldX + averageX) / 2, ABS(oldY + averageY) / 2, ABS(oldZ + averageZ) / 2) != ShapeType::kNone) {
		return true;
	}

	if (_engine->_grid->getBrickShape(ABS(x + averageX) / 2, ABS(y + averageY) / 2, ABS(z + averageZ) / 2) != ShapeType::kNone) {
		return true;
	}

	return false;
}

int32 Collision::checkExtraCollisionWithExtra(ExtraListStruct *extra, int32 extraIdx) const {
	int32 index = extra->info0;
	const BoundingBox *bbox = _engine->_resources->spriteBoundingBox.bbox(index);
	const int32 xLeft = bbox->mins.x + extra->x;
	const int32 xRight = bbox->maxs.x + extra->x;
	const int32 yLeft = bbox->mins.y + extra->y;
	const int32 yRight = bbox->maxs.y + extra->y;
	const int32 zLeft = bbox->mins.z + extra->z;
	const int32 zRight = bbox->maxs.z + extra->z;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		const ExtraListStruct *extraTest = &_engine->_extra->extraList[i];
		if (i != extraIdx && extraTest->info0 != -1) {
			// TODO: shouldn't this be extraTest->info0 as index?
			const BoundingBox *testbbox = _engine->_resources->spriteBoundingBox.bbox(++index);
			const int32 xLeftTest = testbbox->mins.x + extraTest->x;
			const int32 xRightTest = testbbox->maxs.x + extraTest->x;
			const int32 yLeftTest = testbbox->mins.y + extraTest->y;
			const int32 yRightTest = testbbox->maxs.y + extraTest->y;
			const int32 zLeftTest = testbbox->mins.z + extraTest->z;
			const int32 zRightTest = testbbox->maxs.z + extraTest->z;

			if (xLeft < xLeftTest) {
				if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
					return i;
				}
			}
		}
	}

	return -1;
}

} // namespace TwinE
