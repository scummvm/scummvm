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

	const IVec3 &processActor = _engine->_movements->processActor;
	const IVec3 &mins1 = processActor + actor1->boudingBox.mins;
	const IVec3 &maxs1 = processActor + actor1->boudingBox.maxs;

	const IVec3 &mins2 = actor2->pos + actor2->boudingBox.mins;
	const IVec3 &maxs2 = actor2->pos + actor2->boudingBox.maxs;

	if (mins1.x >= maxs2.x) {
		return false;
	}

	if (maxs1.x <= mins2.x) {
		return false;
	}

	if (mins1.y > (maxs2.y + 1)) {
		return false;
	}

	if (mins1.y <= (maxs2.y - BRICK_HEIGHT)) {
		return false;
	}

	if (maxs1.y <= mins2.y) {
		return false;
	}

	if (mins1.z >= maxs2.z) {
		return false;
	}

	if (maxs1.z <= mins2.z) {
		return false;
	}

	return true;
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

	IVec3 &processActor = _engine->_movements->processActor;

	// double-side stairs
	if (brickShape >= ShapeType::kDoubleSideStairsTop1 && brickShape <= ShapeType::kDoubleSideStairsRight2) {
		switch (brickShape) {
		case ShapeType::kDoubleSideStairsTop1:
			if (processActor.z - collision.z <= processActor.x - collision.x) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsTopRight;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom1:
			if (processActor.z - collision.z <= processActor.x - collision.x) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft1:
			if (512 - processActor.x - collision.x <= processActor.z - collision.z) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight1:
			if (512 - processActor.x - collision.x <= processActor.z - collision.z) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsTop2:
			if (processActor.x - collision.x >= processActor.z - collision.z) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom2:
			if (processActor.z - collision.z <= processActor.x - collision.x) {
				brickShape = ShapeType::kStairsBottomRight;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft2:
			if (512 - processActor.x - collision.x <= processActor.z - collision.z) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight2:
			if (512 - processActor.x - collision.x <= processActor.z - collision.z) {
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
			processActor.y = brkY + getAverageValue(0, BRICK_HEIGHT, BRICK_SIZE, processActor.x - brkX);
			break;
		case ShapeType::kStairsTopRight:
			processActor.y = brkY + getAverageValue(0, BRICK_HEIGHT, BRICK_SIZE, processActor.z - brkZ);
			break;
		case ShapeType::kStairsBottomLeft:
			processActor.y = brkY + getAverageValue(BRICK_HEIGHT, 0, BRICK_SIZE, processActor.z - brkZ);
			break;
		case ShapeType::kStairsBottomRight:
			processActor.y = brkY + getAverageValue(BRICK_HEIGHT, 0, BRICK_SIZE, processActor.x - brkX);
			break;
		default:
			break;
		}
	}
}

int32 Collision::checkCollisionWithActors(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	IVec3 &processActor = _engine->_movements->processActor;
	IVec3 mins = processActor + actor->boudingBox.mins;
	IVec3 maxs = processActor + actor->boudingBox.maxs;

	actor->collision = -1;

	for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
		ActorStruct *actorTest = _engine->_scene->getActor(a);

		// aviod current processed actor
		if (a != actorIdx && actorTest->entity != -1 && !actor->staticFlags.bComputeLowCollision && actorTest->standOn != actorIdx) {
			const IVec3 &minsTest = actorTest->pos + actorTest->boudingBox.mins;
			const IVec3 &maxsTest = actorTest->pos + actorTest->boudingBox.maxs;

			if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
				actor->collision = a; // mark as collision with actor a

				if (actorTest->staticFlags.bIsCarrierActor) {
					if (actor->dynamicFlags.bIsFalling) {
						processActor.y = maxsTest.y - actor->boudingBox.mins.y + 1;
						actor->standOn = a;
					} else {
						if (standingOnActor(actorIdx, a)) {
							processActor.y = maxsTest.y - actor->boudingBox.mins.y + 1;
							actor->standOn = a;
						} else {
							const int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(processActor, actorTest->pos);

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
									actorTest->lastPos.x = processActor.x - actor->collisionPos.x;
									actorTest->lastPos.z = processActor.z - actor->collisionPos.z;
								}
							}

							if ((actorTest->boudingBox.maxs.x - actorTest->boudingBox.mins.x == actorTest->boudingBox.maxs.z - actorTest->boudingBox.mins.z) &&
								(actor->boudingBox.maxs.x - actor->boudingBox.mins.x == actor->boudingBox.maxs.z - actor->boudingBox.mins.z)) {
								if (newAngle < ANGLE_135) {
									processActor.x = minsTest.x - actor->boudingBox.maxs.x;
								}
								if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
									processActor.z = maxsTest.z - actor->boudingBox.mins.z;
								}
								if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
									processActor.x = maxsTest.x - actor->boudingBox.mins.x;
								}
								if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
									processActor.z = minsTest.z - actor->boudingBox.maxs.z;
								}
							} else {
								if (!actor->dynamicFlags.bIsFalling) {
									processActor = _engine->_movements->previousActor;
								}
							}
						}
					}
				} else {
					if (standingOnActor(actorIdx, a)) {
						_engine->_actor->hitActor(actorIdx, a, 1, -1);
					}

					int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(processActor, actorTest->pos);

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
							actorTest->lastPos.x = processActor.x - actor->collisionPos.x;
							actorTest->lastPos.z = processActor.z - actor->collisionPos.z;
						}
					}

					if ((actorTest->boudingBox.maxs.x - actorTest->boudingBox.mins.x == actorTest->boudingBox.maxs.z - actorTest->boudingBox.mins.z) &&
						(actor->boudingBox.maxs.x - actor->boudingBox.mins.x == actor->boudingBox.maxs.z - actor->boudingBox.mins.z)) {
						if (newAngle < ANGLE_135) {
							processActor.x = minsTest.x - actor->boudingBox.maxs.x;
						}
						if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
							processActor.z = maxsTest.z - actor->boudingBox.mins.z;
						}
						if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
							processActor.x = maxsTest.x - actor->boudingBox.mins.x;
						}
						if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
							processActor.z = minsTest.z - actor->boudingBox.maxs.z;
						}
					} else {
						if (!actor->dynamicFlags.bIsFalling) {
							processActor = _engine->_movements->previousActor;
						}
					}
				}
			}
		}
	}

	if (actor->dynamicFlags.bIsHitting) {
		_engine->_movements->rotateActor(0, 200, actor->angle);

		mins.x = _engine->_renderer->destPos.x + processActor.x + actor->boudingBox.mins.x;
		mins.y = processActor.y + actor->boudingBox.mins.y;
		mins.z = _engine->_renderer->destPos.z + processActor.z + actor->boudingBox.mins.z;

		maxs.x = _engine->_renderer->destPos.x + processActor.x + actor->boudingBox.maxs.x;
		maxs.y = processActor.y + actor->boudingBox.maxs.y;
		maxs.z = _engine->_renderer->destPos.z + processActor.z + actor->boudingBox.maxs.z;

		for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
			const ActorStruct *actorTest = _engine->_scene->getActor(a);

			// aviod current processed actor
			if (a != actorIdx && actorTest->entity != -1 && !actorTest->staticFlags.bIsHidden && actorTest->standOn != actorIdx) {
				const IVec3 minsTest = actorTest->pos + actorTest->boudingBox.mins;
				const IVec3 maxsTest = actorTest->pos + actorTest->boudingBox.maxs;
				if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
					_engine->_actor->hitActor(actorIdx, a, actor->strengthOfHit, actor->angle + ANGLE_180);
					actor->dynamicFlags.bIsHitting = 0;
				}
			}
		}
	}

	return actor->collision;
}

void Collision::checkHeroCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	IVec3 &processActor = _engine->_movements->processActor;
	IVec3 &previousActor = _engine->_movements->previousActor;
	ShapeType brickShape = _engine->_grid->getBrickShape(processActor);

	processActor.x += x;
	processActor.y += y;
	processActor.z += z;

	if (processActor.x >= 0 && processActor.z >= 0 && processActor.x <= 0x7E00 && processActor.z <= 0x7E00) {
		const BoundingBox &bbox = _engine->_actor->processActorPtr->boudingBox;
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShapeFull(processActor, bbox.maxs.y);

		if (brickShape == ShapeType::kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShapeFull(processActor.x, processActor.y, previousActor.z + z, bbox.maxs.y);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShapeFull(x + previousActor.x, processActor.y, processActor.z, bbox.maxs.y);

				if (brickShape != ShapeType::kSolid) {
					processCollision.x = previousActor.x;
				}
			} else {
				processCollision.z = previousActor.z;
			}
		}
	}

	processActor = processCollision;
}

void Collision::checkActorCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	IVec3 &processActor = _engine->_movements->processActor;
	IVec3 &previousActor = _engine->_movements->previousActor;
	ShapeType brickShape = _engine->_grid->getBrickShape(processActor);

	processActor.x += x;
	processActor.y += y;
	processActor.z += z;

	if (processActor.x >= 0 && processActor.z >= 0 && processActor.x <= 0x7E00 && processActor.z <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShape(processActor);

		if (brickShape == ShapeType::kSolid) {
			causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShape(processActor.x, processActor.y, previousActor.z + z);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShape(x + previousActor.x, processActor.y, processActor.z);

				if (brickShape != ShapeType::kSolid) {
					processCollision.x = previousActor.x;
				}
			} else {
				processCollision.z = previousActor.z;
			}
		}
	}

	processActor = processCollision;
}

void Collision::stopFalling() { // ReceptionObj()
	if (IS_HERO(_engine->_animations->currentlyProcessedActorIdx)) {
		const IVec3 &processActor = _engine->_movements->processActor;
		const int32 fall = _engine->_scene->heroYBeforeFall - processActor.y;

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
	const IVec3 mins = bbox->mins + extra->pos;
	const IVec3 maxs = bbox->maxs + extra->pos;

	for (int32 a = 0; a < _engine->_scene->sceneNumActors; a++) {
		const ActorStruct *actorTest = _engine->_scene->getActor(a);

		if (a != actorIdx && actorTest->entity != -1) {
			const IVec3 minsTest = actorTest->pos + actorTest->boudingBox.mins;
			const IVec3 maxsTest = actorTest->pos + actorTest->boudingBox.maxs;

			if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
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
	const IVec3 mins = bbox->mins + extra->pos;
	const IVec3 maxs = bbox->maxs + extra->pos;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		const ExtraListStruct *extraTest = &_engine->_extra->extraList[i];
		if (i != extraIdx && extraTest->info0 != -1) {
			// TODO: shouldn't this be extraTest->info0 as index?
			const BoundingBox *testbbox = _engine->_resources->spriteBoundingBox.bbox(++index);
			const IVec3 minsTest = testbbox->mins + extraTest->pos;
			const IVec3 maxsTest = testbbox->maxs + extraTest->pos;

			if (mins.x >= minsTest.x) {
				continue;
			}
			if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
				return i;
			}
		}
	}

	return -1;
}

} // namespace TwinE
