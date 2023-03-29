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

#ifndef HPL_PHYSICS_JOINT_NEWTON_H
#define HPL_PHYSICS_JOINT_NEWTON_H

#include "hpl1/engine/impl/PhysicsBodyNewton.h"
#include "hpl1/engine/impl/PhysicsWorldNewton.h"
#include "hpl1/engine/libraries/newton/Newton.h"

#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

template<typename T>
class iPhysicsJointNewton : public T {
public:
	iPhysicsJointNewton(const tString &asName, iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
						iPhysicsWorld *apWorld, const cVector3f &avPivotPoint)
		: T(asName, apParentBody, apChildBody, apWorld, avPivotPoint) {
		cPhysicsWorldNewton *pNWorld = static_cast<cPhysicsWorldNewton *>(apWorld);

		mpNewtonWorld = pNWorld->GetNewtonWorld();

		cPhysicsBodyNewton *pNParent = static_cast<cPhysicsBodyNewton *>(apParentBody);
		cPhysicsBodyNewton *pNChild = static_cast<cPhysicsBodyNewton *>(apChildBody);

		if (apParentBody == NULL)
			mpNewtonParentBody = NULL;
		else
			mpNewtonParentBody = pNParent->GetNewtonBody();

		mpNewtonChildBody = pNChild->GetNewtonBody();
	}

	virtual ~iPhysicsJointNewton() {
		// Skip this for now and let newton handle it..
		// Log("Destroying newton joint!\n");
		if (this->mpChildBody || this->mpParentBody)
			NewtonDestroyJoint(mpNewtonWorld, mpNewtonJoint);
	}

	///////////////////////

	void SetCollideBodies(bool abX) {
		NewtonJointSetCollisionState(mpNewtonJoint, abX ? 1 : 0);
	}

	bool GetCollideBodies() {
		return NewtonJointGetCollisionState(mpNewtonJoint) == 0 ? false : true;
	}

	///////////////////////

	void SetStiffness(float afX) {
		NewtonJointSetStiffness(mpNewtonJoint, afX);
	}
	float GetStiffness() {
		return NewtonJointGetStiffness(mpNewtonJoint);
	}

	///////////////////////

protected:
	NewtonJoint *mpNewtonJoint;
	NewtonWorld *mpNewtonWorld;
	NewtonBody *mpNewtonParentBody;
	NewtonBody *mpNewtonChildBody;
};

} // namespace hpl

#endif // HPL_PHYSICS_JOINT_NEWTON_H
