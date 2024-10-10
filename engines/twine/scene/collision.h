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

#ifndef TWINE_SCENE_COLLISION_H
#define TWINE_SCENE_COLLISION_H

#include "common/scummsys.h"
#include "twine/shared.h"

namespace TwinE {

class ActorStruct;
struct ExtraListStruct;
class TwinEEngine;

class Collision {
private:
	TwinEEngine *_engine;

	void handlePushing(IVec3 &processActor, const IVec3 &minsTest, const IVec3 &maxsTest, ActorStruct *actor, ActorStruct *actorTest);

	/** Actor collision coordinate */
	IVec3 _processCollision; // SaveNxw, SaveNyw, SaveNzw
public:
	Collision(TwinEEngine *engine);
	/** Actor collision coordinate */
	IVec3 _collision; // YMap

	/**
	 * Check if actor 1 is standing in actor 2
	 * @param actorIdx1 Actor 1 index
	 * @param actorIdx2 Actor 2 index
	 */
	bool checkZvOnZv(int32 actorIdx1, int32 actorIdx2) const;

	/**
	 * Reajust actor position in scene according with brick shape bellow actor
	 * @param brickShape Shape of brick bellow the actor
	 */
	void reajustPos(IVec3 &processActor, ShapeType brickShape) const;

	/**
	 * Check collision with actors
	 * @param actorIx Current process actor index
	 */
	int32 checkObjCol(int32 actorIdx);
	bool checkValidObjPos(int32 actorIdx);

	void setCollisionPos(const IVec3 &pos);
	/**
	 * Check Hero collision with bricks
	 * @param x Hero X coordinate
	 * @param y Hero Y coordinate
	 * @param z Hero Z coordinate
	 * @param damageMask Cause damage mask
	 */
	uint32 doCornerReajustTwinkel(ActorStruct *actor, int32 x, int32 y, int32 z, int32 damageMask);

	/**
	 * Check other actor collision with bricks
	 * @param x Actor X coordinate
	 * @param y Actor Y coordinate
	 * @param z Actor Z coordinate
	 * @param damageMask Cause damage mask
	 */
	uint32 doCornerReajust(ActorStruct *actor, int32 x, int32 y, int32 z, int32 damageMask);

	/** Make actor to stop falling */
	void receptionObj(int actorIdx);

	/**
	 * Check extra collision with actors
	 * @param extra to process
	 * @param actorIdx actor to check collision
	 */
	int32 extraCheckObjCol(ExtraListStruct *extra, int32 actorIdx);

	/** Check extra collision with bricks */
	bool fullWorldColBrick(int32 x, int32 y, int32 z, const IVec3 &oldPos);

	/**
	 * Check extra collision with another extra
	 * @param extra to process
	 * @param extraIdx extra index to check collision
	 */
	int32 extraCheckExtraCol(ExtraListStruct *extra, int32 extraIdx) const;
};

} // namespace TwinE
#endif
