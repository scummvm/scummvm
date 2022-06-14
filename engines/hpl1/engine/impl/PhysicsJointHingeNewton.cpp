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
 * This file is part of HPL1 Engine.
 */

#include "hpl1/engine/impl/PhysicsJointHingeNewton.h"

#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/impl/PhysicsBodyNewton.h"
#include "hpl1/engine/impl/PhysicsWorldNewton.h"

#include "hpl1/engine/math/Math.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPhysicsJointHingeNewton::cPhysicsJointHingeNewton(const tString &asName,
												   iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
												   iPhysicsWorld *apWorld, const cVector3f &avPivotPoint, const cVector3f avPinDir)
	: iPhysicsJointNewton<iPhysicsJointHinge>(asName, apParentBody, apChildBody, apWorld, avPivotPoint) {
	const float fPivotPoint[3] = {avPivotPoint.x, avPivotPoint.y, avPivotPoint.z};
	const float fPinDir[3] = {avPinDir.x, avPinDir.y, avPinDir.z};
	mpNewtonJoint = NewtonConstraintCreateHinge(mpNewtonWorld, fPivotPoint, fPinDir,
												mpNewtonChildBody, mpNewtonParentBody);

	// Add callback and user data.
	NewtonJointSetUserData(mpNewtonJoint, (void *)this);
	NewtonHingeSetUserCallback(mpNewtonJoint, LimitCallback);

	mvPinDir = avPinDir;
	mvPivotPoint = avPivotPoint;

	mfMaxAngle = 0;
	mfMinAngle = 0;

	mfPreviousAngle = 0;
}

//-----------------------------------------------------------------------

cPhysicsJointHingeNewton::~cPhysicsJointHingeNewton() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPhysicsJointHingeNewton::SetMaxAngle(float afAngle) {
	mfMaxAngle = afAngle;
}
void cPhysicsJointHingeNewton::SetMinAngle(float afAngle) {
	mfMinAngle = afAngle;
}
float cPhysicsJointHingeNewton::GetMaxAngle() {
	return mfMaxAngle;
}
float cPhysicsJointHingeNewton::GetMinAngle() {
	return mfMinAngle;
}

//-----------------------------------------------------------------------

cVector3f cPhysicsJointHingeNewton::GetVelocity() {
	return cVector3f(0, 0, 0);
}
cVector3f cPhysicsJointHingeNewton::GetAngularVelocity() {
	float fSpeed = NewtonHingeGetJointOmega(mpNewtonJoint);
	return mvPinDir * fSpeed;
}
cVector3f cPhysicsJointHingeNewton::GetForce() {
	cVector3f vForce;
	NewtonHingeGetJointForce(mpNewtonJoint, &vForce.x);
	return vForce;
}

//-----------------------------------------------------------------------

float cPhysicsJointHingeNewton::GetDistance() {
	return 0;
}
float cPhysicsJointHingeNewton::GetAngle() {
	return NewtonHingeGetJointAngle(mpNewtonJoint);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// STATIC CALLBACKS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

unsigned cPhysicsJointHingeNewton::LimitCallback(const NewtonJoint *pHinge, NewtonHingeSliderUpdateDesc *pDesc) {
	cPhysicsJointHingeNewton *pHingeJoint = (cPhysicsJointHingeNewton *)NewtonJointGetUserData(pHinge);

	// pHingeJoint->OnPhysicsUpdate();

	if (pHingeJoint->mfMaxAngle == 0 && pHingeJoint->mfMinAngle == 0)
		return 0;

	float fAngle = NewtonHingeGetJointAngle(pHinge);

	// Avoid oscillation
	CheckLimitAutoSleep(pHingeJoint, pHingeJoint->mfMinAngle, pHingeJoint->mfMaxAngle, fAngle);

	bool bSkipLimitCheck = false;
	if (fabs(pHingeJoint->mfPreviousAngle - fAngle) > cMath::ToRad(300))
		bSkipLimitCheck = true;

	// Max limit
	if (fAngle > pHingeJoint->mfMaxAngle && bSkipLimitCheck == false) {
		pHingeJoint->OnMaxLimit();

		pDesc->m_accel = NewtonHingeCalculateStopAlpha(pHinge, pDesc, pHingeJoint->mfMaxAngle);
		pDesc->m_maxFriction = 0;

		pHingeJoint->mfPreviousAngle = fAngle;
		return 1;
	}
	// Min limit
	else if (fAngle < pHingeJoint->mfMinAngle && bSkipLimitCheck == false) {
		pHingeJoint->OnMinLimit();

		pDesc->m_accel = NewtonHingeCalculateStopAlpha(pHinge, pDesc, pHingeJoint->mfMinAngle);
		pDesc->m_minFriction = 0;

		pHingeJoint->mfPreviousAngle = fAngle;
		return 1;
	} else {
		if (pHingeJoint->mpParentBody == NULL || pHingeJoint->mpParentBody->GetMass() == 0) {
			if ((pHingeJoint->mfStickyMaxDistance != 0 &&
				 fabs(fAngle - pHingeJoint->mfMaxAngle) < pHingeJoint->mfStickyMaxDistance) ||
				(pHingeJoint->mfStickyMinDistance != 0 &&
				 fabs(fAngle - pHingeJoint->mfMinAngle) < pHingeJoint->mfStickyMinDistance)) {
				pHingeJoint->mpChildBody->SetAngularVelocity(0);
				pHingeJoint->mpChildBody->SetLinearVelocity(0);
			}
		}

		pHingeJoint->OnNoLimit();
	}

	pHingeJoint->mfPreviousAngle = fAngle;
	return 0;
}

//-----------------------------------------------------------------------

} // namespace hpl
