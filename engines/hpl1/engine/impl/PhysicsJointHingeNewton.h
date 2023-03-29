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

#ifndef HPL_PHYSICS_JOINT_HINGE_NEWTON_H
#define HPL_PHYSICS_JOINT_HINGE_NEWTON_H

#include "hpl1/engine/impl/PhysicsJointNewton.h"
#include "hpl1/engine/physics/PhysicsJointHinge.h"

namespace hpl {

class cPhysicsJointHingeNewton : public iPhysicsJointNewton<iPhysicsJointHinge> {
public:
	cPhysicsJointHingeNewton(const tString &asName, iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
							 iPhysicsWorld *apWorld, const cVector3f &avPivotPoint, const cVector3f avPinDir);
	~cPhysicsJointHingeNewton();

	void SetMaxAngle(float afAngle);
	void SetMinAngle(float afAngle);
	float GetMaxAngle();
	float GetMinAngle();

	cVector3f GetVelocity();
	cVector3f GetAngularVelocity();
	cVector3f GetForce();

	float GetDistance();
	float GetAngle();

private:
	float mfPreviousAngle;
	static unsigned LimitCallback(const NewtonJoint *pHinge, NewtonHingeSliderUpdateDesc *pDesc);
};

} // namespace hpl

#endif // HPL_PHYSICS_JOINT_HINGE_NEWTON_H
