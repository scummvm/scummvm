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

#ifndef TWINE_COLLISION_H
#define TWINE_COLLISION_H

#include "common/scummsys.h"
#include "twine/scene/extra.h"

namespace TwinE {

class TwinEEngine;

class Collision {
private:
	TwinEEngine *_engine;

public:
	Collision(TwinEEngine *engine);
	/** Actor collition X coordinate */
	int32 collisionX = 0; // getPosVar1
	/** Actor collition Y coordinate */
	int32 collisionY = 0; // getPosVar2
	/** Actor collition Z coordinate */
	int32 collisionZ = 0; // getPosVar3

	/** Actor collition X coordinate */
	int32 processCollisionX = 0; // processActorVar11
	/** Actor collition Y coordinate */
	int32 processCollisionY = 0; // processActorVar12
	/** Actor collition Z coordinate */
	int32 processCollisionZ = 0; // processActorVar13

	/** Cause damage in current processed actor */
	int32 causeActorDamage = 0; //fieldCauseDamage

	/**
	 * Check if actor 1 is standing in actor 2
	 * @param actorIdx1 Actor 1 index
	 * @param actorIdx2 Actor 2 index
	 */
	bool standingOnActor(int32 actorIdx1, int32 actorIdx2);

	int32 getAverageValue(int32 var0, int32 var1, int32 var2, int32 var3);

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
	bool checkExtraCollisionWithBricks(int32 x, int32 y, int32 z, int32 oldX, int32 oldY, int32 oldZ);

	/**
	 * Check extra collision with another extra
	 * @param extra to process
	 * @param extraIdx extra index to check collision
	 */
	int32 checkExtraCollisionWithExtra(ExtraListStruct *extra, int32 extraIdx) const;
};

} // namespace TwinE
#endif
