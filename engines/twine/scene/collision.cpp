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

#include "twine/scene/collision.h"
#include "common/util.h"
#include "twine/debugger/debug_state.h"
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

bool Collision::checkZvOnZv(int32 actorIdx1, int32 actorIdx2) const {
	const ActorStruct *actor1 = _engine->_scene->getActor(actorIdx1);
	const ActorStruct *actor2 = _engine->_scene->getActor(actorIdx2);

	const IVec3 &processActor = actor1->_processActor;
	const IVec3 &mins1 = processActor + actor1->_boundingBox.mins;
	const IVec3 &maxs1 = processActor + actor1->_boundingBox.maxs;

	const IVec3 &mins2 = actor2->posObj() + actor2->_boundingBox.mins;
	const IVec3 &maxs2 = actor2->posObj() + actor2->_boundingBox.maxs;

	if (mins1.x >= maxs2.x) {
		return false;
	}

	if (maxs1.x <= mins2.x) {
		return false;
	}

	if (mins1.y > (maxs2.y + 1)) {
		return false;
	}

	if (mins1.y <= (maxs2.y - SIZE_BRICK_Y)) {
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

void Collision::reajustPos(IVec3 &processActor, ShapeType brickShape) const {
	if (brickShape <= ShapeType::kSolid) {
		return;
	}

	const int32 xw = (_collision.x * SIZE_BRICK_XZ) - DEMI_BRICK_XZ; // upper left corner of the brick
	const int32 yw = _collision.y * SIZE_BRICK_Y;
	const int32 zw = (_collision.z * SIZE_BRICK_XZ) - DEMI_BRICK_XZ;

	// double-side stairs
	switch (brickShape) {
	case ShapeType::kDoubleSideStairsTop1:
		if (processActor.x - xw < processActor.z - zw) {
			brickShape = ShapeType::kStairsTopRight;
		} else {
			brickShape = ShapeType::kStairsTopLeft;
		}
		break;
	case ShapeType::kDoubleSideStairsBottom1:
		if (processActor.x - xw < processActor.z - zw) {
			brickShape = ShapeType::kStairsBottomRight;
		} else {
			brickShape = ShapeType::kStairsBottomLeft;
		}
		break;
	case ShapeType::kDoubleSideStairsTop2:
		if (processActor.x - xw < processActor.z - zw) {
			brickShape = ShapeType::kStairsTopLeft;
		} else {
			brickShape = ShapeType::kStairsTopRight;
		}
		break;
	case ShapeType::kDoubleSideStairsBottom2:
		if (processActor.x - xw < processActor.z - zw) {
			brickShape = ShapeType::kStairsBottomLeft;
		} else {
			brickShape = ShapeType::kStairsBottomRight;
		}
		break;
	case ShapeType::kDoubleSideStairsLeft1:
		if (SIZE_BRICK_XZ - (processActor.x - xw) > processActor.z - zw) {
			brickShape = ShapeType::kStairsBottomLeft;
		} else {
			brickShape = ShapeType::kStairsTopLeft;
		}
		break;
	case ShapeType::kDoubleSideStairsRight1:
		if (SIZE_BRICK_XZ - (processActor.x - xw) > processActor.z - zw) {
			brickShape = ShapeType::kStairsBottomRight;
		} else {
			brickShape = ShapeType::kStairsTopRight;
		}
		break;
	case ShapeType::kDoubleSideStairsLeft2:
		if (SIZE_BRICK_XZ - (processActor.x - xw) > processActor.z - zw) {
			brickShape = ShapeType::kStairsTopLeft;
		} else {
			brickShape = ShapeType::kStairsBottomLeft;
		}
		break;
	case ShapeType::kDoubleSideStairsRight2:
		if (SIZE_BRICK_XZ - (processActor.x - xw) > processActor.z - zw) {
			brickShape = ShapeType::kStairsTopRight;
		} else {
			brickShape = ShapeType::kStairsBottomRight;
		}
		break;
	default:
		break;
	}

	switch (brickShape) {
	case ShapeType::kStairsTopLeft:
		processActor.y = yw + boundRuleThree(0, SIZE_BRICK_Y, SIZE_BRICK_XZ, processActor.x - xw);
		break;
	case ShapeType::kStairsTopRight:
		processActor.y = yw + boundRuleThree(0, SIZE_BRICK_Y, SIZE_BRICK_XZ, processActor.z - zw);
		break;
	case ShapeType::kStairsBottomLeft:
		processActor.y = yw + boundRuleThree(SIZE_BRICK_Y, 0, SIZE_BRICK_XZ, processActor.z - zw);
		break;
	case ShapeType::kStairsBottomRight:
		processActor.y = yw + boundRuleThree(SIZE_BRICK_Y, 0, SIZE_BRICK_XZ, processActor.x - xw);
		break;
	default:
		break;
	}
}

void Collision::handlePushing(IVec3 &processActor, const IVec3 &minsTest, const IVec3 &maxsTest, ActorStruct *ptrobj, ActorStruct *ptrobjt) {
	const int32 newAngle = _engine->_movements->getAngle(processActor, ptrobjt->posObj());

	// protect against chain reactions
	if (ptrobjt->_flags.bCanBePushed && !ptrobj->_flags.bCanBePushed) {
		ptrobjt->_animStep.y = 0;

		if (ptrobjt->_flags.bUseMiniZv) {
			if (newAngle >= LBAAngles::ANGLE_45 && newAngle < LBAAngles::ANGLE_135 && ptrobj->_beta >= LBAAngles::ANGLE_45 && ptrobj->_beta < LBAAngles::ANGLE_135) {
				ptrobjt->_animStep.x = SIZE_BRICK_XZ / 4 + SIZE_BRICK_XZ / 8;
			}
			if (newAngle >= LBAAngles::ANGLE_135 && newAngle < LBAAngles::ANGLE_225 && ptrobj->_beta >= LBAAngles::ANGLE_135 && ptrobj->_beta < LBAAngles::ANGLE_225) {
				ptrobjt->_animStep.z = -SIZE_BRICK_XZ / 4 + SIZE_BRICK_XZ / 8;
			}
			if (newAngle >= LBAAngles::ANGLE_225 && newAngle < LBAAngles::ANGLE_315 && ptrobj->_beta >= LBAAngles::ANGLE_225 && ptrobj->_beta < LBAAngles::ANGLE_315) {
				ptrobjt->_animStep.x = -SIZE_BRICK_XZ / 4 + SIZE_BRICK_XZ / 8;
			}
			if ((newAngle >= LBAAngles::ANGLE_315 || newAngle < LBAAngles::ANGLE_45) && (ptrobj->_beta >= LBAAngles::ANGLE_315 || ptrobj->_beta < LBAAngles::ANGLE_45)) {
				ptrobjt->_animStep.z = SIZE_BRICK_XZ / 4 + SIZE_BRICK_XZ / 8;
			}
		} else {
			// induced displacement before readjustment?
			ptrobjt->_animStep.x = processActor.x - ptrobj->_oldPos.x;
			ptrobjt->_animStep.z = processActor.z - ptrobj->_oldPos.z;
		}
	}

	// so patch tempo
	if ((ptrobjt->_boundingBox.maxs.x - ptrobjt->_boundingBox.mins.x == ptrobjt->_boundingBox.maxs.z - ptrobjt->_boundingBox.mins.z) &&
		(ptrobj->_boundingBox.maxs.x - ptrobj->_boundingBox.mins.x == ptrobj->_boundingBox.maxs.z - ptrobj->_boundingBox.mins.z)) {
		if (newAngle >= LBAAngles::ANGLE_45 && newAngle < LBAAngles::ANGLE_135) {
			processActor.x = minsTest.x - ptrobj->_boundingBox.maxs.x;
		}
		if (newAngle >= LBAAngles::ANGLE_135 && newAngle < LBAAngles::ANGLE_225) {
			processActor.z = maxsTest.z - ptrobj->_boundingBox.mins.z;
		}
		if (newAngle >= LBAAngles::ANGLE_225 && newAngle < LBAAngles::ANGLE_315) {
			processActor.x = maxsTest.x - ptrobj->_boundingBox.mins.x;
		}
		if (newAngle >= LBAAngles::ANGLE_315 || newAngle < LBAAngles::ANGLE_45) {
			processActor.z = minsTest.z - ptrobj->_boundingBox.maxs.z;
		}
	} else if (!ptrobj->_workFlags.bIsFalling) {
		// refuse pos
		processActor = ptrobj->_oldPos;
	}
}

bool Collision::checkValidObjPos(int32 actorIdx) {
	const ActorStruct *ptrobj = _engine->_scene->getActor(actorIdx);

	const IVec3 m0 = ptrobj->posObj() + ptrobj->_boundingBox.mins;
	const IVec3 m1 = ptrobj->posObj() + ptrobj->_boundingBox.maxs;

	if (m0.x < 0 || m0.x > SCENE_SIZE_MAX) {
		return false;
	}
	if (m1.x < 0 || m1.x > SCENE_SIZE_MAX) {
		return false;
	}
	if (m0.z < 0 || m0.z > SCENE_SIZE_MAX) {
		return false;
	}
	if (m1.z < 0 || m1.z > SCENE_SIZE_MAX) {
		return false;
	}

	Grid *grid = _engine->_grid;
	if (grid->worldColBrickFull(m0.x, m0.y, m0.z, ptrobj->_boundingBox.maxs.y, actorIdx) != ShapeType::kNone) {
		return false;
	}
	if (grid->worldColBrickFull(m1.x, m0.y, m0.z, ptrobj->_boundingBox.maxs.y, actorIdx) != ShapeType::kNone) {
		return false;
	}
	if (grid->worldColBrickFull(m1.x, m0.y, m1.z, ptrobj->_boundingBox.maxs.y, actorIdx) != ShapeType::kNone) {
		return false;
	}
	if (grid->worldColBrickFull(m0.x, m0.y, m1.z, ptrobj->_boundingBox.maxs.y, actorIdx) != ShapeType::kNone) {
		return false;
	}

	for (int32 n = 0; n < _engine->_scene->_nbObjets; ++n) {
		const ActorStruct *ptrobjt = _engine->_scene->getActor(n);
		if (n != actorIdx && ptrobjt->_body != -1 && !ptrobj->_flags.bIsInvisible && ptrobjt->_carryBy != actorIdx) {
			const IVec3 &t0 = ptrobjt->posObj() + ptrobjt->_boundingBox.mins;
			const IVec3 &t1 = ptrobjt->posObj() + ptrobjt->_boundingBox.maxs;
			if (m0.x < t1.x && m1.x > t0.x && m0.y < t1.y && m1.y > t0.y && m0.z < t1.z && m1.z > t0.z) {
				return false;
			}
		}
	}
	return true;
}

int32 Collision::checkObjCol(int32 actorIdx) {
	ActorStruct *ptrobj = _engine->_scene->getActor(actorIdx);

	IVec3 &processActor = ptrobj->_processActor;
	IVec3 mins = processActor + ptrobj->_boundingBox.mins;
	IVec3 maxs = processActor + ptrobj->_boundingBox.maxs;

	ptrobj->_objCol = -1;

	for (int32 a = 0; a < _engine->_scene->_nbObjets; a++) {
		ActorStruct *ptrobjt = _engine->_scene->getActor(a);

		// avoid current processed actor
		if (a != actorIdx && ptrobjt->_body != -1 && !ptrobj->_flags.bIsInvisible && ptrobjt->_carryBy != actorIdx) {
			const IVec3 &minsTest = ptrobjt->posObj() + ptrobjt->_boundingBox.mins;
			const IVec3 &maxsTest = ptrobjt->posObj() + ptrobjt->_boundingBox.maxs;

			if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
				ptrobj->_objCol = a; // mark as collision with actor a

				if (ptrobjt->_flags.bIsCarrierActor) {
					if (ptrobj->_workFlags.bIsFalling) {
						// I touch a carrier
						processActor.y = maxsTest.y - ptrobj->_boundingBox.mins.y + 1;
						ptrobj->_carryBy = a;
						continue;
					} else if (checkZvOnZv(actorIdx, a)) {
						// I walk on a carrier
						processActor.y = maxsTest.y - ptrobj->_boundingBox.mins.y + 1;
						ptrobj->_carryBy = a;
						continue;
					}
				} else {
					// I step on someone
					if (checkZvOnZv(actorIdx, a)) {
						_engine->_actor->hitObj(actorIdx, a, 1, -1);
					}
				}
				handlePushing(processActor, minsTest, maxsTest, ptrobj, ptrobjt);
			}
		}
	}

	// test moves ZV further if hit
	if (ptrobj->_workFlags.bIsHitting) {
		const IVec2 &destPos = _engine->_renderer->rotate(0, 200, ptrobj->_beta);
		mins = processActor + ptrobj->_boundingBox.mins;
		mins.x += destPos.x;
		mins.z += destPos.y;

		maxs = processActor + ptrobj->_boundingBox.maxs;
		maxs.x += destPos.x;
		maxs.z += destPos.y;

		for (int32 a = 0; a < _engine->_scene->_nbObjets; a++) {
			const ActorStruct *actorTest = _engine->_scene->getActor(a);

			// avoid current processed actor
			if (a != actorIdx && actorTest->_body != -1 && !actorTest->_flags.bIsInvisible && actorTest->_carryBy != actorIdx) {
				const IVec3 minsTest = actorTest->posObj() + actorTest->_boundingBox.mins;
				const IVec3 maxsTest = actorTest->posObj() + actorTest->_boundingBox.maxs;
				if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
					_engine->_actor->hitObj(actorIdx, a, ptrobj->_strengthOfHit, ptrobj->_beta + LBAAngles::ANGLE_180);
					ptrobj->_workFlags.bIsHitting = 0;
				}
			}
		}
	}

	return ptrobj->_objCol;
}

void Collision::setCollisionPos(const IVec3 &pos) {
	_processCollision = pos;
}

uint32 Collision::doCornerReajustTwinkel(ActorStruct *actor, int32 x, int32 y, int32 z, int32 damageMask) {
	IVec3 &processActor = actor->_processActor;
	const IVec3 &oldPos = actor->_oldPos;
	ShapeType orgcol = _engine->_grid->worldColBrick(processActor);
	uint32 _col1 = 0;

	processActor.x += x;
	processActor.y += y;
	processActor.z += z;

	if (processActor.x >= 0 && processActor.z >= 0 && processActor.x <= SCENE_SIZE_MAX && processActor.z <= SCENE_SIZE_MAX) {
		const BoundingBox &bbox = actor->_boundingBox;
		reajustPos(processActor, orgcol);
		ShapeType col = _engine->_grid->worldColBrickFull(processActor, bbox.maxs.y, OWN_ACTOR_SCENE_INDEX);

		if (col == ShapeType::kSolid) {
			_col1 |= damageMask;
			if (_engine->_grid->worldColBrickFull(processActor.x, processActor.y, oldPos.z + z, bbox.maxs.y, OWN_ACTOR_SCENE_INDEX) == ShapeType::kSolid) {
				if (_engine->_grid->worldColBrickFull(x + oldPos.x, processActor.y, processActor.z, bbox.maxs.y, OWN_ACTOR_SCENE_INDEX) != ShapeType::kSolid) {
					_processCollision.x = oldPos.x;
				}
			} else {
				_processCollision.z = oldPos.z;
			}
		}
	}

	processActor = _processCollision;
	return _col1;
}

uint32 Collision::doCornerReajust(ActorStruct *actor, int32 x, int32 y, int32 z, int32 damageMask) {
	IVec3 &processActor = actor->_processActor;
	const IVec3 &previousActor = actor->_oldPos;
	ShapeType orgcol = _engine->_grid->worldColBrick(processActor);
	uint32 _col1 = 0;

	processActor.x += x;
	processActor.y += y;
	processActor.z += z;

	if (processActor.x >= 0 && processActor.z >= 0 && processActor.x <= SCENE_SIZE_MAX && processActor.z <= SCENE_SIZE_MAX) {
		reajustPos(processActor, orgcol);
		ShapeType col = _engine->_grid->worldColBrick(processActor);

		if (col == ShapeType::kSolid) {
			_col1 |= damageMask;
			if (_engine->_grid->worldColBrick(processActor.x, processActor.y, previousActor.z + z) == ShapeType::kSolid) {
				if (_engine->_grid->worldColBrick(x + previousActor.x, processActor.y, processActor.z) != ShapeType::kSolid) {
					_processCollision.x = previousActor.x;
				}
			} else {
				_processCollision.z = previousActor.z;
			}
		}
	}

	processActor = _processCollision;
	return _col1;
}

void Collision::receptionObj(int actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (IS_HERO(actorIdx)) {
		const IVec3 &processActor = actor->_processActor;
		const int32 fall = _engine->_scene->_startYFalling - processActor.y;

		if (fall >= SIZE_BRICK_Y * 8) {
			const IVec3 &actorPos = actor->posObj();
			_engine->_extra->initSpecial(actorPos.x, actorPos.y + 1000, actorPos.z, ExtraSpecialType::kHitStars);
			if (fall >= SIZE_BRICK_Y * 16) {
				actor->setLife(0);
			} else {
				actor->addLife(-1);
			}
			_engine->_animations->initAnim(AnimationTypes::kLandingHit, AnimType::kAnimationAllThen, AnimationTypes::kStanding, actorIdx);
		} else if (fall > 2 * SIZE_BRICK_Y) {
			_engine->_animations->initAnim(AnimationTypes::kLanding, AnimType::kAnimationAllThen, AnimationTypes::kStanding, actorIdx);
		} else {
			if (actor->_workFlags.bWasWalkingBeforeFalling) {
				// try to not interrupt walk animation if Twinsen falls down from small height
				_engine->_animations->initAnim(AnimationTypes::kForward, AnimType::kAnimationTypeRepeat, AnimationTypes::kStanding, actorIdx);
			} else {
				_engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, AnimationTypes::kStanding, actorIdx);
			}
		}

		_engine->_scene->_startYFalling = 0;
	} else {
		_engine->_animations->initAnim(AnimationTypes::kLanding, AnimType::kAnimationAllThen, actor->_nextGenAnim, actorIdx);
	}

	if (actor->_workFlags.bIsFalling) {
		debugC(1, TwinE::kDebugCollision, "Actor %d reset falling", actorIdx);
	}

	actor->_workFlags.bIsFalling = 0;
	actor->_workFlags.bWasWalkingBeforeFalling = 0;
}

int32 Collision::extraCheckObjCol(ExtraListStruct *extra, int32 actorIdx) {
	const BoundingBox *bbox = _engine->_resources->_spriteBoundingBox.bbox(extra->sprite);
	const IVec3 mins = bbox->mins + extra->pos;
	const IVec3 maxs = bbox->maxs + extra->pos;

	for (int32 a = 0; a < _engine->_scene->_nbObjets; a++) {
		const ActorStruct *actorTest = _engine->_scene->getActor(a);

		if (a != actorIdx && actorTest->_body != -1) {
			const IVec3 minsTest = actorTest->posObj() + actorTest->_boundingBox.mins;
			const IVec3 maxsTest = actorTest->posObj() + actorTest->_boundingBox.maxs;

			if (mins.x < maxsTest.x && maxs.x > minsTest.x && mins.y < maxsTest.y && maxs.y > minsTest.y && mins.z < maxsTest.z && maxs.z > minsTest.z) {
				if (extra->strengthOfHit != 0) {
					_engine->_actor->hitObj(actorIdx, a, extra->strengthOfHit, -1);
				}

				return a;
			}
		}
	}

	return -1;
}

bool Collision::fullWorldColBrick(int32 x, int32 y, int32 z, const IVec3 &oldPos) {
	if (_engine->_grid->worldColBrick(oldPos) != ShapeType::kNone) {
		return true;
	}

	const int32 averageX = ABS(x + oldPos.x) / 2;
	const int32 averageY = ABS(y + oldPos.y) / 2;
	const int32 averageZ = ABS(z + oldPos.z) / 2;

	if (_engine->_grid->worldColBrick(averageX, averageY, averageZ) != ShapeType::kNone) {
		return true;
	}

	if (_engine->_grid->worldColBrick(ABS(oldPos.x + averageX) / 2, ABS(oldPos.y + averageY) / 2, ABS(oldPos.z + averageZ) / 2) != ShapeType::kNone) {
		return true;
	}

	if (_engine->_grid->worldColBrick(ABS(x + averageX) / 2, ABS(y + averageY) / 2, ABS(z + averageZ) / 2) != ShapeType::kNone) {
		return true;
	}

	return false;
}

int32 Collision::extraCheckExtraCol(ExtraListStruct *extra, int32 extraIdx) const {
	int32 index = extra->sprite;
	const BoundingBox *bbox = _engine->_resources->_spriteBoundingBox.bbox(index);
	const IVec3 mins = bbox->mins + extra->pos;
	const IVec3 maxs = bbox->maxs + extra->pos;

	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		const ExtraListStruct *extraTest = &_engine->_extra->_extraList[i];
		if (i != extraIdx && extraTest->sprite != -1) {
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
