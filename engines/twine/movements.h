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

#ifndef TWINE_MOVEMENTS_H
#define TWINE_MOVEMENTS_H

#include "twine/actor.h"
#include "common/scummsys.h"

namespace TwinE {

/** Control mode types */
enum ControlMode {
	kNoMove = 0,
	kManual = 1,
	kFollow = 2,
	kTrack = 3,
	kFollow2 = 4,
	kTrackAttack = 5,
	kSameXZ = 6,
	kRandom = 7
};

class TwinEEngine;

class Movements {
private:
	TwinEEngine *_engine;
public:
	Movements(TwinEEngine *engine);
	/** Hero moved */
	int16 heroMoved = 0; // twinsenMove
	/** Hero Action */
	int16 heroAction = 0; // action

	/** Process actor.x coordinate */
	int16 processActorX = 0;
	/** Process actor.y coordinate */
	int16 processActorY = 0;
	/** Process actor.z coordinate */
	int16 processActorZ = 0;

	ActorStruct *processActorPtr = nullptr; // processActorVar1

	/** Previous process actor.x coordinate */
	int16 previousActorX = 0; // processActorVar2
	/** Previous process actor.y coordinate */
	int16 previousActorY = 0; // processActorVar3
	/** Previous process actor.z coordinate */
	int16 previousActorZ = 0; // processActorVar4

	int32 targetActorDistance = 0; // DoTrackVar1

	/**
	 * Get shadow position
	 * @param X Shadow X coordinate
	 * @param Y Shadow Y coordinate
	 * @param Z Shadow Z coordinate
	 */
	void getShadowPosition(int32 X, int32 Y, int32 Z);

	/**
	 * Set actor safe angle
	 * @param startAngle start angle
	 * @param endAngle end angle
	 * @param stepAngle number of steps
	 * @param movePtr time pointer to update
	 */
	void setActorAngleSafe(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr);

	/**
	 * Clear actors safe angle
	 * @param actorPtr actor pointer
	 */
	void clearRealAngle(ActorStruct *actorPtr);

	/**
	 * Set actor safe angle
	 * @param startAngle start angle
	 * @param endAngle end angle
	 * @param stepAngle number of steps
	 * @param movePtr time pointer to update
	 */
	void setActorAngle(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr);

	/**
	 * Get actor angle
	 * @param x1 Actor 1 X
	 * @param z1 Actor 1 Z
	 * @param x2 Actor 2 X
	 * @param z2 Actor 2 Z
	 */
	int32 getAngleAndSetTargetActorDistance(int32 x1, int32 z1, int32 x2, int32 z2);

	/**
	 * Get actor real angle
	 * @param movePtr time pointer to process
	 */
	int32 getRealAngle(ActorMoveStruct *movePtr);

	/**
	 * Get actor step
	 * @param movePtr time pointer to process
	 */
	int32 getRealValue(ActorMoveStruct *movePtr);

	/**
	 * Rotate actor with a given angle
	 * @param X Actor current X coordinate
	 * @param Z Actor current Z coordinate
	 * @param angle Actor angle to rotate
	 */
	void rotateActor(int32 X, int32 Z, int32 angle);

	/**
	 * Get distance value in 2D
	 * @param x1 Actor 1 X coordinate
	 * @param z1 Actor 1 Z coordinate
	 * @param x2 Actor 2 X coordinate
	 * @param z2 Actor 2 Z coordinate
	 */
	int32 getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2);

	/**
	 * Get distance value in 3D
	 * @param x1 Actor 1 X coordinate
	 * @param y1 Actor 1 Y coordinate
	 * @param z1 Actor 1 Z coordinate
	 * @param x2 Actor 2 X coordinate
	 * @param y2 Actor 2 Y coordinate
	 * @param z2 Actor 2 Z coordinate
	 */
	int32 getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2);

	/**
	 * Move actor around the scene
	 * @param angleFrom Current actor angle
	 * @param angleTo Angle to rotate
	 * @param speed Rotate speed
	 * @param movePtr Pointer to process movements
	 */
	void moveActor(int32 angleFrom, int32 angleTo, int32 speed, ActorMoveStruct *movePtr);

	void processActorMovements(int32 actorIdx);
};

} // namespace TwinE

#endif
