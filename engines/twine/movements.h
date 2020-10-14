/** @file movements.h
	@brief
	This file contains movies routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MOVEMENTS_H
#define MOVEMENTS_H

#include "sys.h"
#include "actor.h"

/** Control mode types */
enum ControlMode {
	kNoMove			= 0,
	kManual			= 1,
	kFollow			= 2,
	kTrack			= 3,
	kFollow2		= 4,
	kTrackAttack	= 5,
	kSameXZ			= 6,
	kRandom			= 7
};

/** Hero moved */
int16 heroMoved; // twinsenMove
/** Hero Action */
int16 heroAction; // action

/** Process actor X coordinate */
int16 processActorX;
/** Process actor Y coordinate */
int16 processActorY;
/** Process actor Z coordinate */
int16 processActorZ;

ActorStruct *processActorPtr; // processActorVar1

/** Previous process actor X coordinate */
int16 previousActorX; // processActorVar2
/** Previous process actor Y coordinate */
int16 previousActorY; // processActorVar3
/** Previous process actor Z coordinate */
int16 previousActorZ; // processActorVar4

int32 targetActorDistance; // DoTrackVar1


/** Get shadow position
	@param X Shadow X coordinate
	@param Y Shadow Y coordinate
	@param Z Shadow Z coordinate */
void getShadowPosition(int32 X, int32 Y, int32 Z);

/** Set actor safe angle
	@param startAngle start angle
	@param endAngle end angle
	@param stepAngle number of steps
	@param movePtr time pointer to update */
void setActorAngleSafe(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct * movePtr);

/** Clear actors safe angle
	@param actorPtr actor pointer */
void clearRealAngle(ActorStruct * actorPtr);

/** Set actor safe angle
	@param startAngle start angle
	@param endAngle end angle
	@param stepAngle number of steps
	@param movePtr time pointer to update */
void setActorAngle(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct * movePtr);

/** Get actor angle
	@param x1 Actor 1 X
	@param z1 Actor 1 Z
	@param x2 Actor 2 X
	@param z2 Actor 2 Z */
int32 getAngleAndSetTargetActorDistance(int32 x1, int32 z1, int32 x2, int32 z2);

/** Get actor real angle
	@param movePtr time pointer to process */
int32 getRealAngle(ActorMoveStruct * movePtr);

/** Get actor step
	@param movePtr time pointer to process */
int32 getRealValue(ActorMoveStruct * movePtr);

/** Rotate actor with a given angle
	@param X Actor current X coordinate
	@param Z Actor current Z coordinate
	@param angle Actor angle to rotate */
void rotateActor(int32 X, int32 Z, int32 angle);

/** Get distance value in 2D
	@param x1 Actor 1 X coordinate
	@param z1 Actor 1 Z coordinate
	@param x2 Actor 2 X coordinate
	@param z2 Actor 2 Z coordinate */
int32 getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2);

/** Get distance value in 3D
	@param x1 Actor 1 X coordinate
	@param y1 Actor 1 Y coordinate
	@param z1 Actor 1 Z coordinate
	@param x2 Actor 2 X coordinate
	@param y2 Actor 2 Y coordinate
	@param z2 Actor 2 Z coordinate */
int32 getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2);

/** Move actor around the scene
	@param angleFrom Current actor angle
	@param angleTo Angle to rotate
	@param speed Rotate speed
	@param movePtr Pointer to process movements */
void moveActor(int32 angleFrom, int32 angleTo, int32 speed, ActorMoveStruct *movePtr);

void processActorMovements(int32 actorIdx);

#endif
