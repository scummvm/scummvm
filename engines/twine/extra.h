/** @file extra.h
	@brief
	This file contains extra (bonus, projectils, keys, etc.) routines

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

#include "sys.h"
#include "actor.h"

#ifndef EXTRA_H
#define EXTRA_H

#define EXTRA_MAX_ENTRIES		50

typedef struct ExtraListStruct
{
	int16 info0; // field_0
	int16 X;
	int16 Y;
	int16 Z;

	int16 lastX; // field_8
	int16 lastY; // field_A
	int16 lastZ; // field_C

	ActorMoveStruct trackActorMove;

	int16 destX; // field_E
	int16 destY; // field_10
	int16 destZ; // field_12
	int16 type;  // field_14
	int16 angle; // field_16
	int32 lifeTime;
	int16 actorIdx; // field_ 1C
	int16 strengthOfHit; // field_1E
	int16 info1; // field_20
} ExtraListStruct;

ExtraListStruct extraList[EXTRA_MAX_ENTRIES];

enum ExtraSpecialType {
	kHitStars = 0,
	kExplodeCloud = 1
};

int32 addExtra(int32 actorIdx, int32 X, int32 Y, int32 Z, int32 info0, int32 targetActor, int32 maxSpeed, int32 strengthOfHit);

/** Add extra explosion
	@param X Explostion X coordinate
	@param Y Explostion Y coordinate
	@param Z Explostion Z coordinate */
int32 addExtraExplode(int32 X, int32 Y, int32 Z);

/** Reset all used extras */
void resetExtras();

void addExtraSpecial(int32 X, int32 Y, int32 Z, int32 type);
int32 addExtraBonus(int32 X, int32 Y, int32 Z, int32 param, int32 angle, int32 type, int32 bonusAmount);
int32 addExtraThrow(int32 actorIdx, int32 X, int32 Y, int32 Z, int32 sprite, int32 var2, int32 var3, int32 var4, int32 var5, int32 strengthOfHit);
int32 addExtraAiming(int32 actorIdx, int32 X, int32 Y, int32 Z, int32 spriteIdx, int32 targetActorIdx, int32 maxSpeed, int32 strengthOfHit);
void addExtraThrowMagicball(int32 X, int32 Y, int32 Z, int32 param1, int32 angle, int32 param2, int32 param3);

void drawExtraSpecial(int32 extraIdx, int32 X, int32 Y);

/** Process extras */
void processExtras();


#endif

