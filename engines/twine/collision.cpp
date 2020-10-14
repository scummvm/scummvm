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
#include "twine/animations.h"
#include "twine/collision.h"
#include "common/debug.h"
#include "common/util.h"
#include "twine/extra.h"
#include "twine/grid.h"
#include "twine/movements.h"
#include "twine/renderer.h"
#include "twine/scene.h"
#include "twine/twine.h"

namespace TwinE {

Collision::Collision(TwinEEngine *engine) : _engine(engine) {
}

int32 Collision::standingOnActor(int32 actorIdx1, int32 actorIdx2) { // CheckZvOnZv
	int32 x1Left, y1Left, z1Left, x1Right, y1Right, z1Right;
	int32 x2Left, y2Left, z2Left, x2Right, y2Right, z2Right;
	ActorStruct *actor1;
	ActorStruct *actor2;

	actor1 = &_engine->_scene->sceneActors[actorIdx1];
	actor2 = &_engine->_scene->sceneActors[actorIdx2];

	// Current actor (actor 1)
	x1Left = _engine->_movements->processActorX + actor1->boudingBox.x.bottomLeft;
	x1Right = _engine->_movements->processActorX + actor1->boudingBox.x.topRight;

	y1Left = _engine->_movements->processActorY + actor1->boudingBox.y.bottomLeft;
	y1Right = _engine->_movements->processActorY + actor1->boudingBox.y.topRight;

	z1Left = _engine->_movements->processActorZ + actor1->boudingBox.z.bottomLeft;
	z1Right = _engine->_movements->processActorZ + actor1->boudingBox.z.topRight;

	// Actor 2
	x2Left = actor2->x + actor2->boudingBox.x.bottomLeft;
	x2Right = actor2->x + actor2->boudingBox.x.topRight;

	y2Left = actor2->y + actor2->boudingBox.y.bottomLeft;
	y2Right = actor2->y + actor2->boudingBox.y.topRight;

	z2Left = actor2->z + actor2->boudingBox.z.bottomLeft;
	z2Right = actor2->z + actor2->boudingBox.z.topRight;

	if (x1Left >= x2Right)
		return 0; // not standing

	if (x1Right <= x2Left)
		return 0;

	if (y1Left > (y2Right + 1))
		return 0;

	if (y1Left <= (y2Right - 0x100))
		return 0;

	if (y1Right <= y2Left)
		return 0;

	if (z1Left >= z2Right)
		return 0;

	if (z1Right <= z2Left)
		return 0;

	return 1; // standing
}

int32 Collision::getAverageValue(int32 var0, int32 var1, int32 var2, int32 var3) {
	if (var3 <= 0) {
		return var0;
	}

	if (var3 >= var2) {
		return var1;
	}

	return ((((var1 - var0) * var3) / var2) + var0);
}

void Collision::reajustActorPosition(int32 brickShape) {
	int32 brkX, brkY, brkZ;

	if (!brickShape) {
		return;
	}

	brkX = (collisionX << 9) - 0x100;
	brkY = collisionY << 8;
	brkZ = (collisionZ << 9) - 0x100;

	// double-side stairs
	if (brickShape >= kDoubleSideStairsTop1 && brickShape <= kDoubleSideStairsRight2) {
		switch (brickShape) {
		case kDoubleSideStairsTop1:
			if (_engine->_movements->processActorZ - collisionZ <= _engine->_movements->processActorX - collisionX) {
				brickShape = kStairsTopLeft;
			} else {
				brickShape = kStairsTopRight;
			}
			break;
		case kDoubleSideStairsBottom1:
			if (_engine->_movements->processActorZ - collisionZ <= _engine->_movements->processActorX - collisionX) {
				brickShape = kStairsBottomLeft;
			} else {
				brickShape = kStairsBottomRight;
			}
			break;
		case kDoubleSideStairsLeft1:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = kStairsTopLeft;
			} else {
				brickShape = kStairsBottomLeft;
			}
			break;
		case kDoubleSideStairsRight1:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = kStairsTopRight;
			} else {
				brickShape = kStairsBottomRight;
			}
			break;
		case kDoubleSideStairsTop2:
			if (_engine->_movements->processActorX - collisionX >= _engine->_movements->processActorZ - collisionZ) {
				brickShape = kStairsTopRight;
			} else {
				brickShape = kStairsTopLeft;
			}
			break;
		case kDoubleSideStairsBottom2:
			if (_engine->_movements->processActorZ - collisionZ <= _engine->_movements->processActorX - collisionX) {
				brickShape = kStairsBottomRight;
			} else {
				brickShape = kStairsBottomLeft;
			}
			break;
		case kDoubleSideStairsLeft2:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = kStairsBottomLeft;
			} else {
				brickShape = kStairsTopLeft;
			}
			break;
		case kDoubleSideStairsRight2:
			if (512 - _engine->_movements->processActorX - collisionX <= _engine->_movements->processActorZ - collisionZ) {
				brickShape = kStairsBottomRight;
			} else {
				brickShape = kStairsTopRight;
			}
			break;
		default:
			if (_engine->cfgfile.Debug == 1) {
				debug("Brick Shape %d unsupported\n", brickShape);
			}
			break;
		}
	}

	if (brickShape >= kStairsTopLeft && brickShape <= kStairsBottomRight) {
		switch (brickShape) {
		case kStairsTopLeft:
			_engine->_movements->processActorY = brkY + getAverageValue(0, 0x100, 0x200, _engine->_movements->processActorX - brkX);
			break;
		case kStairsTopRight:
			_engine->_movements->processActorY = brkY + getAverageValue(0, 0x100, 0x200, _engine->_movements->processActorZ - brkZ);
			break;
		case kStairsBottomLeft:
			_engine->_movements->processActorY = brkY + getAverageValue(0x100, 0, 0x200, _engine->_movements->processActorZ - brkZ);
			break;
		case kStairsBottomRight:
			_engine->_movements->processActorY = brkY + getAverageValue(0x100, 0, 0x200, _engine->_movements->processActorX - brkX);
			break;
		default:
			break;
		}
	}
}

int32 Collision::checkCollisionWithActors(int32 actorIdx) {
	int32 a, xLeft, xRight, yLeft, yRight, zLeft, zRight;
	ActorStruct *actor, *actorTest;

	actor = &_engine->_scene->sceneActors[actorIdx];

	xLeft = _engine->_movements->processActorX + actor->boudingBox.x.bottomLeft;
	xRight = _engine->_movements->processActorX + actor->boudingBox.x.topRight;

	yLeft = _engine->_movements->processActorY + actor->boudingBox.y.bottomLeft;
	yRight = _engine->_movements->processActorY + actor->boudingBox.y.topRight;

	zLeft = _engine->_movements->processActorZ + actor->boudingBox.z.bottomLeft;
	zRight = _engine->_movements->processActorZ + actor->boudingBox.z.topRight;

	actor->collision = -1;

	for (a = 0; a < _engine->_scene->sceneNumActors; a++) {
		actorTest = &_engine->_scene->sceneActors[a];

		// aviod current processed actor
		if (a != actorIdx && actorTest->entity != -1 && !actor->staticFlags.bComputeLowCollision && actorTest->standOn != actorIdx) {
			int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;

			xLeftTest = actorTest->x + actorTest->boudingBox.x.bottomLeft;
			xRightTest = actorTest->x + actorTest->boudingBox.x.topRight;

			yLeftTest = actorTest->y + actorTest->boudingBox.y.bottomLeft;
			yRightTest = actorTest->y + actorTest->boudingBox.y.topRight;

			zLeftTest = actorTest->z + actorTest->boudingBox.z.bottomLeft;
			zRightTest = actorTest->z + actorTest->boudingBox.z.topRight;

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
							int32 newAngle;

							newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(_engine->_movements->processActorX, _engine->_movements->processActorZ, actorTest->x, actorTest->z);

							if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
								actorTest->lastY = 0;

								if (actorTest->staticFlags.bUseMiniZv) {
									if (newAngle >= 0x80 && newAngle < 0x180 && actor->angle > 0x80 && actor->angle < 0x180) {
										actorTest->lastX = 192;
									}
									if (newAngle >= 0x180 && newAngle < 0x280 && actor->angle > 0x180 && actor->angle < 0x280) {
										actorTest->lastZ = -64;
									}
									if (newAngle >= 0x280 && newAngle < 0x380 && actor->angle > 0x280 && actor->angle < 0x380) {
										actorTest->lastX = -64;
									}
									if ((newAngle >= 0x380 || newAngle < 0x80) && (actor->angle > 0x380 || actor->angle < 0x80)) {
										actorTest->lastX = 192;
									}
								} else {
									actorTest->lastX = _engine->_movements->processActorX - actor->collisionX;
									actorTest->lastZ = _engine->_movements->processActorZ - actor->collisionZ;
								}
							}

							if ((actorTest->boudingBox.x.topRight - actorTest->boudingBox.x.bottomLeft == actorTest->boudingBox.z.topRight - actorTest->boudingBox.z.bottomLeft) &&
							    (actor->boudingBox.x.topRight - actor->boudingBox.x.bottomLeft == actor->boudingBox.z.topRight - actor->boudingBox.z.bottomLeft)) {
								if (newAngle < 0x180) {
									_engine->_movements->processActorX = xLeftTest - actor->boudingBox.x.topRight;
								}
								if (newAngle >= 0x180 && newAngle < 0x280) {
									_engine->_movements->processActorZ = zRightTest - actor->boudingBox.z.bottomLeft;
								}
								if (newAngle >= 0x280 && newAngle < 0x380) {
									_engine->_movements->processActorX = xRightTest - actor->boudingBox.x.bottomLeft;
								}
								if (newAngle >= 0x380 || (newAngle < 0x380 && newAngle < 0x80)) {
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
					int32 newAngle;

					if (standingOnActor(actorIdx, a)) {
						_engine->_actor->hitActor(actorIdx, a, 1, -1);
					}

					newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(_engine->_movements->processActorX, _engine->_movements->processActorZ, actorTest->x, actorTest->z);

					if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
						actorTest->lastY = 0;

						if (actorTest->staticFlags.bUseMiniZv) {
							if (newAngle >= 0x80 && newAngle < 0x180 && actor->angle > 0x80 && actor->angle < 0x180) {
								actorTest->lastX = 192;
							}
							if (newAngle >= 0x180 && newAngle < 0x280 && actor->angle > 0x180 && actor->angle < 0x280) {
								actorTest->lastZ = -64;
							}
							if (newAngle >= 0x280 && newAngle < 0x380 && actor->angle > 0x280 && actor->angle < 0x380) {
								actorTest->lastX = -64;
							}
							if ((newAngle >= 0x380 || newAngle < 0x80) && (actor->angle > 0x380 || actor->angle < 0x80)) {
								actorTest->lastX = 192;
							}
						} else {
							actorTest->lastX = _engine->_movements->processActorX - actor->collisionX;
							actorTest->lastZ = _engine->_movements->processActorZ - actor->collisionZ;
						}
					}

					if ((actorTest->boudingBox.x.topRight - actorTest->boudingBox.x.bottomLeft == actorTest->boudingBox.z.topRight - actorTest->boudingBox.z.bottomLeft) &&
					    (actor->boudingBox.x.topRight - actor->boudingBox.x.bottomLeft == actor->boudingBox.z.topRight - actor->boudingBox.z.bottomLeft)) {
						if (newAngle < 0x180) {
							_engine->_movements->processActorX = xLeftTest - actor->boudingBox.x.topRight;
						}
						if (newAngle >= 0x180 && newAngle < 0x280) {
							_engine->_movements->processActorZ = zRightTest - actor->boudingBox.z.bottomLeft;
						}
						if (newAngle >= 0x280 && newAngle < 0x380) {
							_engine->_movements->processActorX = xRightTest - actor->boudingBox.x.bottomLeft;
						}
						if (newAngle >= 0x380 || (newAngle < 0x380 && newAngle < 0x80)) {
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

		for (a = 0; a < _engine->_scene->sceneNumActors; a++) {
			actorTest = &_engine->_scene->sceneActors[a];

			// aviod current processed actor
			if (a != actorIdx && actorTest->entity != -1 && !actorTest->staticFlags.bIsHidden && actorTest->standOn != actorIdx) {
				int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;

				xLeftTest = actorTest->x + actorTest->boudingBox.x.bottomLeft;
				xRightTest = actorTest->x + actorTest->boudingBox.x.topRight;

				yLeftTest = actorTest->y + actorTest->boudingBox.y.bottomLeft;
				yRightTest = actorTest->y + actorTest->boudingBox.y.topRight;

				zLeftTest = actorTest->z + actorTest->boudingBox.z.bottomLeft;
				zRightTest = actorTest->z + actorTest->boudingBox.z.topRight;

				if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
					_engine->_actor->hitActor(actorIdx, a, actor->strengthOfHit, actor->angle + 0x200);
					actor->dynamicFlags.bIsHitting = 0;
				}
			}
		}
	}

	return actor->collision;
}

void Collision::checkHeroCollisionWithBricks(int32 X, int32 Y, int32 Z, int32 damageMask) {
	int32 brickShape;

	brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

	_engine->_movements->processActorX += X;
	_engine->_movements->processActorY += Y;
	_engine->_movements->processActorZ += Z;

	if (_engine->_movements->processActorX >= 0 && _engine->_movements->processActorZ >= 0 && _engine->_movements->processActorX <= 0x7E00 && _engine->_movements->processActorZ <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShapeFull(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ, _engine->_movements->processActorPtr->boudingBox.y.topRight);

		if (brickShape == kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShapeFull(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->previousActorZ + Z, _engine->_movements->processActorPtr->boudingBox.y.topRight);

			if (brickShape == kSolid) {
				brickShape = _engine->_grid->getBrickShapeFull(X + _engine->_movements->previousActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ, _engine->_movements->processActorPtr->boudingBox.y.topRight);

				if (brickShape != kSolid) {
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

void Collision::checkActorCollisionWithBricks(int32 X, int32 Y, int32 Z, int32 damageMask) {
	int32 brickShape;

	brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

	_engine->_movements->processActorX += X;
	_engine->_movements->processActorY += Y;
	_engine->_movements->processActorZ += Z;

	if (_engine->_movements->processActorX >= 0 && _engine->_movements->processActorZ >= 0 && _engine->_movements->processActorX <= 0x7E00 && _engine->_movements->processActorZ <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

		if (brickShape == kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActorX, _engine->_movements->processActorY, _engine->_movements->previousActorZ + Z);

			if (brickShape == kSolid) {
				brickShape = _engine->_grid->getBrickShape(X + _engine->_movements->previousActorX, _engine->_movements->processActorY, _engine->_movements->processActorZ);

				if (brickShape != kSolid) {
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
	int32 fall;

	if (_engine->_animations->currentlyProcessedActorIdx == 0) {
		fall = _engine->_scene->heroYBeforeFall - _engine->_movements->processActorY;

		if (fall >= 0x1000) {
			_engine->_extra->addExtraSpecial(_engine->_movements->processActorPtr->x, _engine->_movements->processActorPtr->y + 1000, _engine->_movements->processActorPtr->z, kHitStars);
			_engine->_movements->processActorPtr->life--;
			_engine->_animations->initAnim(kLandingHit, 2, 0, _engine->_animations->currentlyProcessedActorIdx);
		} else if (fall >= 0x800) {
			_engine->_extra->addExtraSpecial(_engine->_movements->processActorPtr->x, _engine->_movements->processActorPtr->y + 1000, _engine->_movements->processActorPtr->z, kHitStars);
			_engine->_movements->processActorPtr->life--;
			_engine->_animations->initAnim(kLandingHit, 2, 0, _engine->_animations->currentlyProcessedActorIdx);
		} else if (fall > 10) {
			_engine->_animations->initAnim(kLanding, 2, 0, _engine->_animations->currentlyProcessedActorIdx);
		} else {
			_engine->_animations->initAnim(kStanding, 0, 0, _engine->_animations->currentlyProcessedActorIdx);
		}

		_engine->_scene->heroYBeforeFall = 0;
	} else {
		_engine->_animations->initAnim(kLanding, 2, _engine->_movements->processActorPtr->animExtra, _engine->_animations->currentlyProcessedActorIdx);
	}

	_engine->_movements->processActorPtr->dynamicFlags.bIsFalling = 0;
}

int32 Collision::checkExtraCollisionWithActors(ExtraListStruct *extra, int32 actorIdx) {
	int32 a;
	int32 xLeft, xRight, yLeft, yRight, zLeft, zRight;
	int16 *spriteBounding;
	ActorStruct *actorTest;

	spriteBounding = (int16 *)(_engine->_scene->spriteBoundingBoxPtr + extra->info0 * 16 + 4);

	xLeft = *(spriteBounding++) + extra->x;
	xRight = *(spriteBounding++) + extra->x;

	yLeft = *(spriteBounding++) + extra->y;
	yRight = *(spriteBounding++) + extra->y;

	zLeft = *(spriteBounding++) + extra->z;
	zRight = *(spriteBounding++) + extra->z;

	for (a = 0; a < _engine->_scene->sceneNumActors; a++) {
		actorTest = &_engine->_scene->sceneActors[a];

		if (a != actorIdx && actorTest->entity != -1) {
			int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;

			xLeftTest = actorTest->x + actorTest->boudingBox.x.bottomLeft;
			xRightTest = actorTest->x + actorTest->boudingBox.x.topRight;

			yLeftTest = actorTest->y + actorTest->boudingBox.y.bottomLeft;
			yRightTest = actorTest->y + actorTest->boudingBox.y.topRight;

			zLeftTest = actorTest->z + actorTest->boudingBox.z.bottomLeft;
			zRightTest = actorTest->z + actorTest->boudingBox.z.topRight;

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

int32 Collision::checkExtraCollisionWithBricks(int32 X, int32 Y, int32 Z, int32 oldX, int32 oldY, int32 oldZ) {
	int32 averageX, averageY, averageZ;

	if (_engine->_grid->getBrickShape(oldX, oldY, oldZ)) {
		return 1;
	}

	averageX = ABS(X + oldX) / 2;
	averageY = ABS(Y + oldY) / 2;
	averageZ = ABS(Z + oldZ) / 2;

	if (_engine->_grid->getBrickShape(averageX, averageY, averageZ)) {
		return 1;
	}

	if (_engine->_grid->getBrickShape(ABS(oldX + averageX) / 2, ABS(oldY + averageY) / 2, ABS(oldZ + averageZ) / 2)) {
		return 1;
	}

	if (_engine->_grid->getBrickShape(ABS(X + averageX) / 2, ABS(Y + averageY) / 2, ABS(Z + averageZ) / 2)) {
		return 1;
	}

	return 0;
}

int32 Collision::checkExtraCollisionWithExtra(ExtraListStruct *extra, int32 extraIdx) {
	int32 i;
	int32 xLeft, xRight, yLeft, yRight, zLeft, zRight;
	int16 *spriteBounding;

	spriteBounding = (int16 *)(_engine->_scene->spriteBoundingBoxPtr + extra->info0 * 16 + 4);

	xLeft = *(spriteBounding++) + extra->x;
	xRight = *(spriteBounding++) + extra->x;

	yLeft = *(spriteBounding++) + extra->y;
	yRight = *(spriteBounding++) + extra->y;

	zLeft = *(spriteBounding++) + extra->z;
	zRight = *(spriteBounding++) + extra->z;

	for (i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extraTest = &_engine->_extra->extraList[i];
		if (i != extraIdx && extraTest->info0 != -1) {
			int32 xLeftTest, xRightTest, yLeftTest, yRightTest, zLeftTest, zRightTest;
			//            int16 * spriteBoundingTest;
			//	        spriteBoundingTest = (int16*)(_engine->_scene->spriteBoundingBoxPtr + extraTest->info0 * 16 + 4);

			xLeftTest = *(spriteBounding++) + extraTest->x;
			xRightTest = *(spriteBounding++) + extraTest->x;

			yLeftTest = *(spriteBounding++) + extraTest->y;
			yRightTest = *(spriteBounding++) + extraTest->y;

			zLeftTest = *(spriteBounding++) + extraTest->z;
			zRightTest = *(spriteBounding++) + extraTest->z;

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
