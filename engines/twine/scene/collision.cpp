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

	const IVec3 &processActor = _engine->_movements->_processActor;
	const IVec3 &mins1 = processActor + actor1->_boudingBox.mins;
	const IVec3 &maxs1 = processActor + actor1->_boudingBox.maxs;

	const IVec3 &mins2 = actor2->pos() + actor2->_boudingBox.mins;
	const IVec3 &maxs2 = actor2->pos() + actor2->_boudingBox.maxs;

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

	const int32 brkX = (_collision.x * BRICK_SIZE) - BRICK_HEIGHT;
	const int32 brkY = _collision.y * BRICK_HEIGHT;
	const int32 brkZ = (_collision.z * BRICK_SIZE) - BRICK_HEIGHT;

	IVec3 &processActor = _engine->_movements->_processActor;

	// double-side stairs
	if (brickShape >= ShapeType::kDoubleSideStairsTop1 && brickShape <= ShapeType::kDoubleSideStairsRight2) {
		switch (brickShape) {
		case ShapeType::kDoubleSideStairsTop1:
			if (processActor.z - _collision.z <= processActor.x - _collision.x) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsTopRight;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom1:
			if (processActor.z - _collision.z <= processActor.x - _collision.x) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft1:
			if (BRICK_SIZE - processActor.x - _collision.x <= processActor.z - _collision.z) {
				brickShape = ShapeType::kStairsTopLeft;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight1:
			if (BRICK_SIZE - processActor.x - _collision.x <= processActor.z - _collision.z) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsBottomRight;
			}
			break;
		case ShapeType::kDoubleSideStairsTop2:
			if (processActor.x - _collision.x >= processActor.z - _collision.z) {
				brickShape = ShapeType::kStairsTopRight;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsBottom2:
			if (processActor.z - _collision.z <= processActor.x - _collision.x) {
				brickShape = ShapeType::kStairsBottomRight;
			} else {
				brickShape = ShapeType::kStairsBottomLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsLeft2:
			if (BRICK_SIZE - processActor.x - _collision.x <= processActor.z - _collision.z) {
				brickShape = ShapeType::kStairsBottomLeft;
			} else {
				brickShape = ShapeType::kStairsTopLeft;
			}
			break;
		case ShapeType::kDoubleSideStairsRight2:
			if (BRICK_SIZE - processActor.x - _collision.x <= processActor.z - _collision.z) {
				brickShape = ShapeType::kStairsBottomRight;
			} else {
				brickShape = ShapeType::kStairsTopRight;
			}
			break;
		default:
			if (_engine->_cfgfile.Debug) {
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

	IVec3 &processActor = _engine->_movements->_processActor;
	IVec3 &previousActor = _engine->_movements->_previousActor;
	IVec3 mins = processActor + actor->_boudingBox.mins;
	IVec3 maxs = processActor + actor->_boudingBox.maxs;

	actor->_collision = -1;

	for (int32 a = 0; a < _engine->_scene->_sceneNumActors; a++) {
		ActorStruct *actorTest = _engine->_scene->getActor(a);

		// aviod current processed actor
		if (a != actorIdx && actorTest->_entity != -1 && !actor->_staticFlags.bComputeLowCollision && actorTest->_standOn != actorIdx) {
			const IVec3 &minsTest = actorTest->pos() + actorTest->_boudingBox.mins;
			const IVec3 &maxsTest = actorTest->pos() + actorTest->_boudingBox.maxs;

			if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
				actor->_collision = a; // mark as collision with actor a

				if (actorTest->_staticFlags.bIsCarrierActor) {
					if (actor->_dynamicFlags.bIsFalling) {
						processActor.y = maxsTest.y - actor->_boudingBox.mins.y + 1;
						actor->_standOn = a;
					} else {
						if (standingOnActor(actorIdx, a)) {
							processActor.y = maxsTest.y - actor->_boudingBox.mins.y + 1;
							actor->_standOn = a;
						} else {
							const int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(processActor, actorTest->pos());

							if (actorTest->_staticFlags.bCanBePushed && !actor->_staticFlags.bCanBePushed) {
								actorTest->_lastPos.y = 0;

								if (actorTest->_staticFlags.bUseMiniZv) {
									if (newAngle >= ANGLE_45 && newAngle < ANGLE_135 && actor->_angle > ANGLE_45 && actor->_angle < ANGLE_135) {
										actorTest->_lastPos.x = 192;
									}
									if (newAngle >= ANGLE_135 && newAngle < ANGLE_225 && actor->_angle > ANGLE_135 && actor->_angle < ANGLE_225) {
										actorTest->_lastPos.z = -64;
									}
									if (newAngle >= ANGLE_225 && newAngle < ANGLE_315 && actor->_angle > ANGLE_225 && actor->_angle < ANGLE_315) {
										actorTest->_lastPos.x = -64;
									}
									if ((newAngle >= ANGLE_315 || newAngle < ANGLE_45) && (actor->_angle > ANGLE_315 || actor->_angle < ANGLE_45)) {
										actorTest->_lastPos.x = 192;
									}
								} else {
									actorTest->_lastPos.x = processActor.x - actor->_collisionPos.x;
									actorTest->_lastPos.z = processActor.z - actor->_collisionPos.z;
								}
							}

							if ((actorTest->_boudingBox.maxs.x - actorTest->_boudingBox.mins.x == actorTest->_boudingBox.maxs.z - actorTest->_boudingBox.mins.z) &&
								(actor->_boudingBox.maxs.x - actor->_boudingBox.mins.x == actor->_boudingBox.maxs.z - actor->_boudingBox.mins.z)) {
								if (newAngle < ANGLE_135) {
									processActor.x = minsTest.x - actor->_boudingBox.maxs.x;
								}
								if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
									processActor.z = maxsTest.z - actor->_boudingBox.mins.z;
								}
								if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
									processActor.x = maxsTest.x - actor->_boudingBox.mins.x;
								}
								if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
									processActor.z = minsTest.z - actor->_boudingBox.maxs.z;
								}
							} else {
								if (!actor->_dynamicFlags.bIsFalling) {
									processActor = previousActor;
								}
							}
						}
					}
				} else {
					if (standingOnActor(actorIdx, a)) {
						_engine->_actor->hitActor(actorIdx, a, 1, -1);
					}

					int32 newAngle = _engine->_movements->getAngleAndSetTargetActorDistance(processActor, actorTest->pos());

					if (actorTest->_staticFlags.bCanBePushed && !actor->_staticFlags.bCanBePushed) {
						actorTest->_lastPos.y = 0;

						if (actorTest->_staticFlags.bUseMiniZv) {
							if (newAngle >= ANGLE_45 && newAngle < ANGLE_135 && actor->_angle > ANGLE_45 && actor->_angle < ANGLE_135) {
								actorTest->_lastPos.x = 192;
							}
							if (newAngle >= ANGLE_135 && newAngle < ANGLE_225 && actor->_angle > ANGLE_135 && actor->_angle < ANGLE_225) {
								actorTest->_lastPos.z = -64;
							}
							if (newAngle >= ANGLE_225 && newAngle < ANGLE_315 && actor->_angle > ANGLE_225 && actor->_angle < ANGLE_315) {
								actorTest->_lastPos.x = -64;
							}
							if ((newAngle >= ANGLE_315 || newAngle < ANGLE_45) && (actor->_angle > ANGLE_315 || actor->_angle < ANGLE_45)) {
								actorTest->_lastPos.x = 192;
							}
						} else {
							actorTest->_lastPos.x = processActor.x - actor->_collisionPos.x;
							actorTest->_lastPos.z = processActor.z - actor->_collisionPos.z;
						}
					}

					if ((actorTest->_boudingBox.maxs.x - actorTest->_boudingBox.mins.x == actorTest->_boudingBox.maxs.z - actorTest->_boudingBox.mins.z) &&
						(actor->_boudingBox.maxs.x - actor->_boudingBox.mins.x == actor->_boudingBox.maxs.z - actor->_boudingBox.mins.z)) {
						if (newAngle < ANGLE_135) {
							processActor.x = minsTest.x - actor->_boudingBox.maxs.x;
						}
						if (newAngle >= ANGLE_135 && newAngle < ANGLE_225) {
							processActor.z = maxsTest.z - actor->_boudingBox.mins.z;
						}
						if (newAngle >= ANGLE_225 && newAngle < ANGLE_315) {
							processActor.x = maxsTest.x - actor->_boudingBox.mins.x;
						}
						if (newAngle >= ANGLE_315 || (newAngle < ANGLE_315 && newAngle < ANGLE_45)) {
							processActor.z = minsTest.z - actor->_boudingBox.maxs.z;
						}
					} else {
						if (!actor->_dynamicFlags.bIsFalling) {
							processActor = previousActor;
						}
					}
				}
			}
		}
	}

	if (actor->_dynamicFlags.bIsHitting) {
		_engine->_movements->rotateActor(0, 200, actor->_angle);

		mins.x = _engine->_renderer->_destPos.x + processActor.x + actor->_boudingBox.mins.x;
		mins.y = processActor.y + actor->_boudingBox.mins.y;
		mins.z = _engine->_renderer->_destPos.z + processActor.z + actor->_boudingBox.mins.z;

		maxs.x = _engine->_renderer->_destPos.x + processActor.x + actor->_boudingBox.maxs.x;
		maxs.y = processActor.y + actor->_boudingBox.maxs.y;
		maxs.z = _engine->_renderer->_destPos.z + processActor.z + actor->_boudingBox.maxs.z;

		for (int32 a = 0; a < _engine->_scene->_sceneNumActors; a++) {
			const ActorStruct *actorTest = _engine->_scene->getActor(a);

			// aviod current processed actor
			if (a != actorIdx && actorTest->_entity != -1 && !actorTest->_staticFlags.bIsHidden && actorTest->_standOn != actorIdx) {
				const IVec3 minsTest = actorTest->pos() + actorTest->_boudingBox.mins;
				const IVec3 maxsTest = actorTest->pos() + actorTest->_boudingBox.maxs;
				if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
					_engine->_actor->hitActor(actorIdx, a, actor->_strengthOfHit, actor->_angle + ANGLE_180);
					actor->_dynamicFlags.bIsHitting = 0;
				}
			}
		}
	}

	return actor->_collision;
}

void Collision::checkHeroCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	IVec3 &processActor = _engine->_movements->_processActor;
	IVec3 &previousActor = _engine->_movements->_previousActor;
	ShapeType brickShape = _engine->_grid->getBrickShape(processActor);

	processActor.x += x;
	processActor.y += y;
	processActor.z += z;

	if (processActor.x >= 0 && processActor.z >= 0 && processActor.x <= 0x7E00 && processActor.z <= 0x7E00) {
		const BoundingBox &bbox = _engine->_actor->_processActorPtr->_boudingBox;
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShapeFull(processActor, bbox.maxs.y);

		if (brickShape == ShapeType::kSolid) {
			_causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShapeFull(processActor.x, processActor.y, previousActor.z + z, bbox.maxs.y);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShapeFull(x + previousActor.x, processActor.y, processActor.z, bbox.maxs.y);

				if (brickShape != ShapeType::kSolid) {
					_processCollision.x = previousActor.x;
				}
			} else {
				_processCollision.z = previousActor.z;
			}
		}
	}

	processActor = _processCollision;
}

void Collision::checkActorCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask) {
	IVec3 &processActor = _engine->_movements->_processActor;
	IVec3 &previousActor = _engine->_movements->_previousActor;
	ShapeType brickShape = _engine->_grid->getBrickShape(processActor);

	processActor.x += x;
	processActor.y += y;
	processActor.z += z;

	if (processActor.x >= 0 && processActor.z >= 0 && processActor.x <= 0x7E00 && processActor.z <= 0x7E00) {
		reajustActorPosition(brickShape);
		brickShape = _engine->_grid->getBrickShape(processActor);

		if (brickShape == ShapeType::kSolid) {
			_causeActorDamage |= damageMask;
			brickShape = _engine->_grid->getBrickShape(processActor.x, processActor.y, previousActor.z + z);

			if (brickShape == ShapeType::kSolid) {
				brickShape = _engine->_grid->getBrickShape(x + previousActor.x, processActor.y, processActor.z);

				if (brickShape != ShapeType::kSolid) {
					_processCollision.x = previousActor.x;
				}
			} else {
				_processCollision.z = previousActor.z;
			}
		}
	}

	processActor = _processCollision;
}

void Collision::stopFalling() { // ReceptionObj()
	if (IS_HERO(_engine->_animations->_currentlyProcessedActorIdx)) {
		const IVec3 &processActor = _engine->_movements->_processActor;
		const int32 fall = _engine->_scene->_heroYBeforeFall - processActor.y;

		if (fall >= BRICK_HEIGHT * 8) {
			const IVec3 &actorPos = _engine->_actor->_processActorPtr->pos();
			_engine->_extra->addExtraSpecial(actorPos.x, actorPos.y + 1000, actorPos.z, ExtraSpecialType::kHitStars);
			if (fall >= BRICK_HEIGHT * 16) {
				_engine->_actor->_processActorPtr->setLife(0);
			} else {
				_engine->_actor->_processActorPtr->addLife(-1);
			}
			_engine->_animations->initAnim(AnimationTypes::kLandingHit, AnimType::kAnimationType_2, AnimationTypes::kStanding, _engine->_animations->_currentlyProcessedActorIdx);
		} else if (fall > 10) {
			_engine->_animations->initAnim(AnimationTypes::kLanding, AnimType::kAnimationType_2, AnimationTypes::kStanding, _engine->_animations->_currentlyProcessedActorIdx);
		} else {
			_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeLoop, AnimationTypes::kStanding, _engine->_animations->_currentlyProcessedActorIdx);
		}

		_engine->_scene->_heroYBeforeFall = 0;
	} else {
		_engine->_animations->initAnim(AnimationTypes::kLanding, AnimType::kAnimationType_2, _engine->_actor->_processActorPtr->_animExtra, _engine->_animations->_currentlyProcessedActorIdx);
	}

	_engine->_actor->_processActorPtr->_dynamicFlags.bIsFalling = 0;
}

int32 Collision::checkExtraCollisionWithActors(ExtraListStruct *extra, int32 actorIdx) {
	const BoundingBox *bbox = _engine->_resources->_spriteBoundingBox.bbox(extra->info0);
	const IVec3 mins = bbox->mins + extra->pos;
	const IVec3 maxs = bbox->maxs + extra->pos;

	for (int32 a = 0; a < _engine->_scene->_sceneNumActors; a++) {
		const ActorStruct *actorTest = _engine->_scene->getActor(a);

		if (a != actorIdx && actorTest->_entity != -1) {
			const IVec3 minsTest = actorTest->pos() + actorTest->_boudingBox.mins;
			const IVec3 maxsTest = actorTest->pos() + actorTest->_boudingBox.maxs;

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
	const BoundingBox *bbox = _engine->_resources->_spriteBoundingBox.bbox(index);
	const IVec3 mins = bbox->mins + extra->pos;
	const IVec3 maxs = bbox->maxs + extra->pos;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		const ExtraListStruct *extraTest = &_engine->_extra->_extraList[i];
		if (i != extraIdx && extraTest->info0 != -1) {
			// TODO: shouldn't this be extraTest->info0 as index?
			const BoundingBox *testbbox = _engine->_resources->_spriteBoundingBox.bbox(++index);
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
