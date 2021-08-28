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

	void handlePushing(const IVec3 &minsTest, const IVec3 &maxsTest, const ActorStruct *actor, ActorStruct *actorTest);
public:
	Collision(TwinEEngine *engine);
	/** Actor collision coordinate */
	IVec3 _collision;

	/** Actor collision coordinate */
	IVec3 _processCollision;

	/** Cause damage in current processed actor */
	int32 _causeActorDamage = 0; //fieldCauseDamage

	/**
	 * Check if actor 1 is standing in actor 2
	 * @param actorIdx1 Actor 1 index
	 * @param actorIdx2 Actor 2 index
	 */
	bool standingOnActor(int32 actorIdx1, int32 actorIdx2) const;

	int32 getAverageValue(int32 start, int32 end, int32 maxDelay, int32 delay) const;

	/**
	 * Reajust actor position in scene according with brick shape bellow actor
	 * @param brickShape Shape of brick bellow the actor
	 */
	void reajustActorPosition(ShapeType brickShape);

	/**
	 * Check collision with actors
	 * @param actorIx Current process actor index
	 */
	int32 checkCollisionWithActors(int32 actorIdx);

	/**
	 * Check Hero collision with bricks
	 * @param x Hero X coordinate
	 * @param y Hero Y coordinate
	 * @param z Hero Z coordinate
	 * @param damageMask Cause damage mask
	 */
	void checkHeroCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask);

	/**
	 * Check other actor collision with bricks
	 * @param x Actor X coordinate
	 * @param y Actor Y coordinate
	 * @param z Actor Z coordinate
	 * @param damageMask Cause damage mask
	 */
	void checkActorCollisionWithBricks(int32 x, int32 y, int32 z, int32 damageMask);

	/** Make actor to stop falling */
	void stopFalling();

	/**
	 * Check extra collision with actors
	 * @param extra to process
	 * @param actorIdx actor to check collision
	 */
	int32 checkExtraCollisionWithActors(ExtraListStruct *extra, int32 actorIdx);

	/** Check extra collision with bricks */
	bool checkExtraCollisionWithBricks(int32 x, int32 y, int32 z, const IVec3 &oldPos);

	/**
	 * Check extra collision with another extra
	 * @param extra to process
	 * @param extraIdx extra index to check collision
	 */
	int32 checkExtraCollisionWithExtra(ExtraListStruct *extra, int32 extraIdx) const;
};

} // namespace TwinE
#endif
