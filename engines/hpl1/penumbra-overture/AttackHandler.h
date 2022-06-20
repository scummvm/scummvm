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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of Penumbra Overture.
 */

#ifndef GAME_ATTACK_HANDLER_H
#define GAME_ATTACK_HANDLER_H

#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;
class cMapHandler;
class cGameSwingDoor;

//---------------------------

#define eAttackTargetFlag_None 0
#define eAttackTargetFlag_Player (1 << 0)
#define eAttackTargetFlag_Enemy (1 << 1)
#define eAttackTargetFlag_Bodies (1 << 2)

typedef tFlag eAttackTargetFlag;

//---------------------------

class cAttackRayCallback : public iPhysicsRayCallback {
public:
	cAttackRayCallback();

	void Reset();
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	eAttackTargetFlag mTarget;
	iPhysicsBody *mpSkipBody;
	iPhysicsBody *mpClosestBody;
	float mfShortestDist;
	cVector3f mvPosition;
	bool mbSkipCharacter;
};

//---------------------------

class cSplashDamageBlockCheck : public iPhysicsRayCallback {
public:
	cSplashDamageBlockCheck(cInit *apInit);

	bool CheckBlock(const cVector3f &avStart, const cVector3f &avEnd);

	bool BeforeIntersect(iPhysicsBody *pBody);
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

private:
	cInit *mpInit;
	bool mbIntersected;
};

//---------------------------

class cAttackHandler : public iUpdateable {
public:
	cAttackHandler(cInit *apInit);
	~cAttackHandler();

	bool CreateLineAttack(const cVector3f &avStart, const cVector3f &avEnd, float afDamage,
						  eAttackTargetFlag aTarget, iPhysicsBody *apSkipBody,
						  iPhysicsBody **apPickedBody = NULL);

	bool CreateShapeAttack(iCollideShape *apShape, const cMatrixf &a_mtxOffset,
						   const cVector3f &avOrigin, float afDamage,
						   float afMinMass, float afMaxMass, float afMinImpulse, float afMaxImpulse,
						   int alStrength,
						   eAttackTargetFlag aTarget, iPhysicsBody *apSkipBody);

	bool CreateLineDestroyBody(const cVector3f &avStart, const cVector3f &avEnd, float afStrength,
							   float afForce, iPhysicsBody *apSkipBody, iPhysicsBody **apPickedBody = NULL);

	void CreateSplashDamage(const cVector3f &avCenter, float afRadius, float afMinDamage,
							float afMaxDamge, float afMinForce, float afMaxForce,
							float afMaxImpulse,
							unsigned int aTarget, float afMinMass,
							int alStrength);

	cGameSwingDoor *GetLastSwingDoor() { return mpLastSwingDoor; }

	void OnStart();
	void Update(float afTimeStep);
	void Reset();

private:
	cInit *mpInit;
	cMapHandler *mpMapHandler;

	cGameSwingDoor *mpLastSwingDoor;

	cAttackRayCallback mRayCallback;
	cSplashDamageBlockCheck *mpSplashBlockCheck;
};

#endif // GAME_ATTACK_HANDLER_H
