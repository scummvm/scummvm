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

bool Collision::standingOnActor(int32 actorIdx1, int32 actorIdx2) const {
	const ActorStruct *actor1 = _engine->_scene->getActor(actorIdx1);
	const ActorStruct *actor2 = _engine->_scene->getActor(actorIdx2);

	// Current actor (actor 1)
	const int32 x1Left = _engine->_movements->processActor.x + actor1->boudingBox.mins.x;
	const int32 x1Right = _engine->_movements->processActor.x + actor1->boudingBox.maxs.x;
	const int32 y1Left = _engine->_movements->processActor.y + actor1->boudingBox.mins.y;
	const int32 y1Right = _engine->_movements->processActor.y + actor1->boudingBox.maxs.y;
	const int32 z1Left = _engine->_movements->processActor.z + actor1->boudingBox.mins.z;
	const int32 z1Right = _engine->_movements->processActor.z + actor1->boudingBox.maxs.z;

	// Actor 2
	const int32 x2Left = actor2->pos.x + actor2->boudingBox.mins.x;
	const int32 x2Right = actor2->pos.x + actor2->boudingBox.maxs.x;
	const int32 y2Left = actor2->pos.y + actor2->boudingBox.mins.y;
	const int32 y2Right = actor2->pos.y + actor2->boudingBox.maxs.y;
	const int32 z2Left = actor2->pos.z + actor2->boudingBox.mins.z;
	const int32 z2Right = actor2->pos.z + actor2->boudingBox.maxs.z;

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

int32 Collision::getAverageValue(int32 start, int32 end, int32 maxDelay, int32 delay) const {
	if (delay <= 0) {
		return start;
	}

	if (delay >= maxDelay) {
		return end;
	}

	return (((end - start) * delay) / maxDelay) + start;
}

void Collision::reajustActorPosition(ShapeType brickShape) {
	if (brickShape == ShapeType::kNone) {
		return;
	}

	const int32 brkX = (collision.x * BRICK_SIZE) - BRICK_HEIGHT;
	const int32 brkY = collision.y * BRICK_HEIGHT;
	const int32 brkZ = (collision.z * BRICK_SIZE) - BRICK_HEIGHT;

	// double-side stairs
	if (brickShape >= ShapeType::kDoubleSideStairsTop1 && brickShape <= ShapeType::kDoubleSideStairsRight2) {
		switch (brickShape) {
		case ShapeType::kDoubleSideStairsTop1:
			if (_engine->_movements->processActor.z - collision.z <= _engine->_movements->processActor.x - collision.x) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsTopRight;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom1:
			if (_engine->_movements->processActor.z - collision.z <= _engine->_movements->processActor.x - collision.x) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft1:
			if (512 - _engine->_movements->processActor.x - collision.x <= _engine->_movements->processActor.z - collision.z) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight1:
			if (512 - _engine->_movements->processActor.x - collision.x <= _engine->_movements->processActor.z - collision.z) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsTop2:
			if (_engine->_movements->processActor.x - collision.x >= _engine->_movements->processActor.z - collision.z) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom2:
			if (_engine->_movements->processActor.z - collision.z <= _engine->_movements->processActor.x - collision.x) {
				brickShape = ShapeType::kStairsBottomRight;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft2:
			if (512 - _engine->_movements->processActor.x - collision.x <= _engine->_movements->processActor.z - collision.z) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight2:
			if (512 - _engine->_movements->processActor.x - collision.x <= _engine->_movements->processActor.z - collision.z) {
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
			_engine->_movements->processActor.y = brkY + getAverageValue(0, BRICK_HEIGHT, BRICK_SIZE, _engine->_movements->processActor.x - brkX);
			break;
		case ShapeType::kStairsTopRight:
			_engine->_movements->processActor.y = brkY + getAverageValue(0, BRICK_HEIGHT, BRICK_SIZE, _engine->_movements->processActor.z - brkZ);
			break;
		case ShapeType::kStairsBottomLeft:
			_engine->_movements->processActor.y = brkY + getAverageValue(BRICK_HEIGHT, 0, BRICK_SIZE, _engine->_movements->processActor.z - brkZ);
			break;
		case ShapeType::kStairsBottomRight:
			_engine->_movements->processActor.y = brkY + getAverageValue(BRICK_HEIGHT, 0, BRICK_SIZE, _engine->_movements->processActor.x - brkX);
			break;
		default:
			break;
		}
	}
}

int32 Collision::checkCollisionWithActors(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	int32 xLeft = _engine->_movements->processActor.x + actor->boudingBox.mins.x;
	int32 xRight = _engine->_movements->processActor.x + actor->boudingBox.maxs.x;
	int32 yLeft = _engine->_movements->processActor.y + actor->boudingBox.mins.y;
	int32 yRight = _engine->_movements->processActor.y + actor->boudingBox.maxs.y;
	int32 zLeft = _engine->_movements->processActor.z + actor->boudingBox.mins.z;
	int32 zRight = _engine->_movements->processActor.z + actor->boudingBox.maxs.z;

	actor->collision = -1;

	for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
		ActorStruct *actorTest = _engine->_scene->getActor(a);

		// aviod current processed actor
		if (a != actorIdx && actorTest->entity != -1 && !actor->staticFlags.bComputeLowCollision && actorTest->standOn != actorIdx) {
			const int32 xLeftTest = actorTest->pos.x + actorTest->boudingBox.mins.x;
			const int32 xRightTest = actorTest->pos.x + actorTest->boudingBox.maxs.x;
			const int32 yLeftTest = actorTest->pos.y + actorTest->boudingBox.mins.y;
			const int32 yRightTest = actorTest->pos.y + actorTest->boudingBox.maxs.y;
			const int32 zLeftTest = actorTest->pos.z + actorTest->boudingBox.mins.z;
			const int32 zRightTest = actorTest->pos.z + actorTest->boudingBox.maxs.z;

			if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
				actor->collision = a; // mark as collision with actor a

				if (actorTest->staticFlags.bIsCarrierActor) {
					if (actor->dynamicFlags.bIsFalling) {
						_engine->_movements->processActor.y = yRightTest - actor->boudingBox.mins.y + 1;
						actor->standOn = a;
					} else {
						if (standingOnActor(actorIdx, a)) {
							_engine->_movements->processActor.y = yRightTest - actor->boudingBox.mins.y + 1;
							actor->standOn = a;
						} else {
							int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(_engine->_movements->processActor, actorTest->pos);

							if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
								actorTest->lastPos.y = 0;

								if (actorTest->staticFlags.bUseMiniZv) {
									if (newAngle >= ANGLE_45 && newAngle < ANGLE_135 && actor->angle > ANGLE_45 && actor->angle < ANGLE_135) {
										actorTest->lastPos.x = 192;
									}
									if (newAngle >= ANGLE_135 && newAngle < ANGLE_225 && actor->angle > ANGLE_135 && actor->angle < ANGLE_225) {
										actorTest->lastPos.z = -64;
									}
									if (newAngle >= ANGLE_225 && newAngle < ANGLE_315 && actor->angle > ANGLE_225 && actor->angle < ANGLE_315) {
										actorTest->lastPos.x = -64;
									}
									if ((newAngle >= ANGLE_315 || newAngle < ANGLE_45) && (actor->angle > ANGLE_315 || actor->angle < ANGLE_45)) {
										actorTest->lastPos.x = 192;
									}
								} else {
									actorTest->lastPos.x = _engine->_movements->processActor.x - actor->collisionPos.x;
									actorTest->lastPos.z = _engine->_movements->processActor.z - actor->collisionPos.z;
								}
							}

							if ((actorTest->boudingBox.maxs.x - actorTest->boudingBox.mins.x == actorTest->boudingBox.maxs.z - actorTest->boudingBox.mins.z) &&
							    (actor->boudingBox.maxs.x - actor->boudingBox.mins.x == actor->boudingBox.maxs.z - actor->boudingBox.mins.z)) {
								if (newAngle < ANGLE_135) {
									_engine->_movements->processActor.x = xLeftTest - actor->boudingBox.maxs.x;
								}
								if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
									_engine->_movements->processActor.z = zRightTest - actor->boudingBox.mins.z;
								}
								if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
									_engine->_movements->processActor.x = xRightTest - actor->boudingBox.mins.x;
								}
								if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
									_engine->_movements->processActor.z = zLeftTest - actor->boudingBox.maxs.z;
								}
							} else {
								if (!actor->dynamicFlags.bIsFalling) {
									_engine->_movements->processActor = _engine->_movements->previousActor;
								}
							}
						}
					}
				} else {
					if (standingOnActor(actorIdx, a)) {
						_engine->_actor->hitActor(actorIdx, a, 1, -1);
					}

					int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(_engine->_movements->processActor, actorTest->pos);

					if (actorTest->staticFlags.bCanBePushed && !actor->staticFlags.bCanBePushed) {
						actorTest->lastPos.y = 0;

						if (actorTest->staticFlags.bUseMiniZv) {
							if (newAngle >= ANGLE_45 && newAngle < ANGLE_135 && actor->angle > ANGLE_45 && actor->angle < ANGLE_135) {
								actorTest->lastPos.x = 192;
							}
							if (newAngle >= ANGLE_135 && newAngle < ANGLE_225 && actor->angle > ANGLE_135 && actor->angle < ANGLE_225) {
								actorTest->lastPos.z = -64;
							}
							if (newAngle >= ANGLE_225 && newAngle < ANGLE_315 && actor->angle > ANGLE_225 && actor->angle < ANGLE_315) {
								actorTest->lastPos.x = -64;
							}
							if ((newAngle >= ANGLE_315 || newAngle < ANGLE_45) && (actor->angle > ANGLE_315 || actor->angle < ANGLE_45)) {
								actorTest->lastPos.x = 192;
							}
						} else {
							actorTest->lastPos.x = _engine->_movements->processActor.x - actor->collisionPos.x;
							actorTest->lastPos.z = _engine->_movements->processActor.z - actor->collisionPos.z;
						}
					}

					if ((actorTest->boudingBox.maxs.x - actorTest->boudingBox.mins.x == actorTest->boudingBox.maxs.z - actorTest->boudingBox.mins.z) &&
					    (actor->boudingBox.maxs.x - actor->boudingBox.mins.x == actor->boudingBox.maxs.z - actor->boudingBox.mins.z)) {
						if (newAngle < ANGLE_135) {
							_engine->_movements->processActor.x = xLeftTest - actor->boudingBox.maxs.x;
						}
						if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
							_engine->_movements->processActor.z = zRightTest - actor->boudingBox.mins.z;
						}
						if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
							_engine->_movements->processActor.x = xRightTest - actor->boudingBox.mins.x;
						}
						if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
							_engine->_movements->processActor.z = zLeftTest - actor->boudingBox.maxs.z;
						}
					} else {
						if (!actor->dynamicFlags.bIsFalling) {
							_engine->_movements->processActor = _engine->_movements->previousActor;
						}
					}
				}
			}
		}
	}

	if (actor->dynamicFlags.bIsHitting) {
		_engine->_movements->rotateActor(0, 200, actor->angle);

		xLeft = _engine->_renderer->destPos.x + _engine->_movements->processActor.x + actor->boudingBox.mins.x;
		xRight = _engine->_renderer->destPos.x + _engine->_movements->processActor.x + actor->boudingBox.maxs.x;

		yLeft = _engine->_movements->processActor.y + actor->boudingBox.mins.y;
		yRight = _engine->_movements->processActor.y + actor->boudingBox.maxs.y;

		zLeft = _engine->_renderer->destPos.z + _engine->_movements->processActor.z + actor->boudingBox.mins.z;
		zRight = _engine->_renderer->destPos.z + _engine->_movements->processActor.z + actor->boudingBox.maxs.z;

		for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
			const ActorStruct *actorTest = _engine->_scene->getActor(a);

			// aviod current processed actor
			if (a != actorIdx && actorTest->entity != -1 && !actorTest->staticFlags.bIsHidden && actorTest->standOn != actorIdx) {
				const int32 xLeftTest =  actorTest->pos.x + actorTest->boudingBox.mins.x;
				const int32 xRightTest = actorTest->pos.x + actorTest->boudingBox.maxs.x;
				const int32 yLeftTest =  actorTest->pos.y + actorTest->boudingBox.mins.y;
				const int32 yRightTest = actorTest->pos.y + actorTest->boudingBox.maxs.y;
				const int32 zLeftTest =  actorTest->pos.z + actorTest->boudingBox.mins.z;
				const int32 zRightTest = actorTest->pos.z + actorTest->boudingBox.maxs.z;

				if (xLeft < xRightTest && xRight > xLeftTest && yLeft < yRightTest && yRight > yLeftTest && zLeft < zRightTest && zRight > zLeftTest) {
					_engine->_actor->hitActor(actorIdx, a, actor->strengthOfHit, actor->angle + ANGLE_180);
					actor->dynamicFlags.bIsHitting = 0;
				}
			}
		}
	}

	return actor->collision;
}

void Collision::checkHeroCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	ShapeType brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z);

	_engine->_movements->processActor.x += x;
	_engine->_movements->processActor.y += y;
	_engine->_movements->processActor.z += z;

	if (_engine->_movements->processActor.x >= 0 && _engine->_movements->processActor.z >= 0 && _engine->_movements->processActor.x <= 0x7E00 && _engine->_movements->processActor.z <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShapeFull(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z, _engine->_actor->processActorPtr->boudingBox.maxs.y);

		if (brickShape == ShapeType::kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShapeFull(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->previousActor.z + z, _engine->_actor->processActorPtr->boudingBox.maxs.y);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShapeFull(x + _engine->_movements->previousActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z, _engine->_actor->processActorPtr->boudingBox.maxs.y);

				if (brickShape != ShapeType::kSolid) {
					processCollision.x = _engine->_movements->previousActor.x;
				}
			} else {
				processCollision.z = _engine->_movements->previousActor.z;
			}
		}
	}

	_engine->_movements->processActor = processCollision;
}

void Collision::checkActorCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	ShapeType brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z);

	_engine->_movements->processActor.x += x;
	_engine->_movements->processActor.y += y;
	_engine->_movements->processActor.z += z;

	if (_engine->_movements->processActor.x >= 0 && _engine->_movements->processActor.z >= 0 && _engine->_movements->processActor.x <= 0x7E00 && _engine->_movements->processActor.z <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z);

		if (brickShape == ShapeType::kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShape(_engine->_movements->processActor.x, _engine->_movements->processActor.y, _engine->_movements->previousActor.z + z);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShape(x + _engine->_movements->previousActor.x, _engine->_movements->processActor.y, _engine->_movements->processActor.z);

				if (brickShape != ShapeType::kSolid) {
					processCollision.x = _engine->_movements->previousActor.x;
				}
			} else {
				processCollision.z = _engine->_movements->previousActor.z;
			}
		}
	}

	_engine->_movements->processActor = processCollision;
}

void Collision::stopFalling() { // ReceptionObj()
	if (IS_HERO(_engine->_animations->currentlyProcessedActorIdx)) {
		const int32 fall = _engine->_scene->heroYBeforeFall - _engine->_movements->processActor.y;

		if (fall >= BRICK_HEIGHT * 8) {
			_engine->_extra->addExtraSpecial(_engine->_actor->processActorPtr->pos.x, _engine->_actor->processActorPtr->pos.y + 1000, _engine->_actor->processActorPtr->pos.z, ExtraSpecialType::kHitStars);
			if (fall >= BRICK_HEIGHT * 16) {
				_engine->_actor->processActorPtr->setLife(0);
			} else {
				_engine->_actor->processActorPtr->addLife(-1);
			}
			_engine->_animations->initAnim(AnimationTypes::kLandingHit, AnimType::kAnimationType_2, AnimationTypes::kStanding, _engine->_animations->currentlyProcessedActorIdx);
		} else if (fall > 10) {
			_engine->_animations->initAnim(AnimationTypes::kLanding, AnimType::kAnimationType_2, AnimationTypes::kStanding, _engine->_animations->currentlyProcessedActorIdx);
		} else {
			_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kStanding, _engine->_animations->currentlyProcessedActorIdx);
		}

		_engine->_scene->heroYBeforeFall = 0;
	} else {
		_engine->_animations->initAnim(AnimationTypes::kLanding, AnimType::kAnimationType_2, _engine->_actor->processActorPtr->animExtra, _engine->_animations->currentlyProcessedActorIdx);
	}

	_engine->_actor->processActorPtr->dynamicFlags.bIsFalling = 0;
}

int32 Collision::checkExtraCollisionWithActors(ExtraListStruct *extra, int32 actorIdx) {
	const BoundingBox *bbox = _engine->_resources->spriteBoundingBox.bbox(extra->info0);
	const int32 xLeft = bbox->mins.x + extra->pos.x;
	const int32 xRight = bbox->maxs.x + extra->pos.x;
	const int32 yLeft = bbox->mins.y + extra->pos.y;
	const int32 yRight = bbox->maxs.y + extra->pos.y;
	const int32 zLeft = bbox->mins.z + extra->pos.z;
	const int32 zRight = bbox->maxs.z + extra->pos.z;

	for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
		const ActorStruct *actorTest = _engine->_scene->getActor(a);

		if (a != actorIdx && actorTest->entity != -1) {
			const int32 xLeftTest = actorTest->pos.x + actorTest->boudingBox.mins.x;
			const int32 xRightTest = actorTest->pos.x + actorTest->boudingBox.maxs.x;
			const int32 yLeftTest = actorTest->pos.y + actorTest->boudingBox.mins.y;
			const int32 yRightTest = actorTest->pos.y + actorTest->boudingBox.maxs.y;
			const int32 zLeftTest = actorTest->pos.z + actorTest->boudingBox.mins.z;
			const int32 zRightTest = actorTest->pos.z + actorTest->boudingBox.maxs.z;

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
	const int32 xLeft = bbox->mins.x + extra->pos.x;
	const int32 xRight = bbox->maxs.x + extra->pos.x;
	const int32 yLeft = bbox->mins.y + extra->pos.y;
	const int32 yRight = bbox->maxs.y + extra->pos.y;
	const int32 zLeft = bbox->mins.z + extra->pos.z;
	const int32 zRight = bbox->maxs.z + extra->pos.z;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		const ExtraListStruct *extraTest = &_engine->_extra->extraList[i];
		if (i != extraIdx && extraTest->info0 != -1) {
			// TODO: shouldn't this be extraTest->info0 as index?
			const BoundingBox *testbbox = _engine->_resources->spriteBoundingBox.bbox(++index);
			const int32 xLeftTest = testbbox->mins.x + extraTest->pos.x;
			const int32 xRightTest = testbbox->maxs.x + extraTest->pos.x;
			const int32 yLeftTest = testbbox->mins.y + extraTest->pos.y;
			const int32 yRightTest = testbbox->maxs.y + extraTest->pos.y;
			const int32 zLeftTest = testbbox->mins.z + extraTest->pos.z;
			const int32 zRightTest = testbbox->maxs.z + extraTest->pos.z;

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
