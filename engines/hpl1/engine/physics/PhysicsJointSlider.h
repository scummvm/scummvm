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

#ifndef HPL_PHYSICS_JOINT_SLIDER_H
#define HPL_PHYSICS_JOINT_SLIDER_H

#include "hpl1/engine/physics/PhysicsJoint.h"

namespace hpl {

//-----------------------------------

kSaveData_ChildClass(iPhysicsJoint, iPhysicsJointSlider) {
	kSaveData_ClassInit(iPhysicsJointSlider) public : float mfMaxDistance;
	float mfMinDistance;
	cVector3f mvPin;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//-----------------------------------

class iPhysicsJointSlider : public iPhysicsJoint {
	typedef iPhysicsJoint super;

public:
	iPhysicsJointSlider(const tString &asName, iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
						iPhysicsWorld *apWorld, const cVector3f &avPivotPoint)
		: iPhysicsJoint(asName, apParentBody, apChildBody, apWorld, avPivotPoint) {}
	virtual ~iPhysicsJointSlider() {}
	/**
	 * Set the maximum distance the bodies can be from each other, relative to the start dist between them
	 * This is true if pin points towards the child.
	 * In other words, distance increases as the distance between start pivot and current pivot
	 * increases in the opposite direction of the pin.
	 */
	virtual void SetMaxDistance(float afX) = 0;
	/**
	 * Set the minimum distance the bodies can be from each other, relative to the start dist between them
	 * This is true if pin points towards the child.
	 */
	virtual void SetMinDistance(float afX) = 0;
	virtual float GetMaxDistance() = 0;
	virtual float GetMinDistance() = 0;

	ePhysicsJointType GetType() { return ePhysicsJointType_Slider; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	float mfMaxDistance;
	float mfMinDistance;
	cVector3f mvPin;
};

} // namespace hpl

#endif // HPL_PHYSICS_JOINT_SLIDER_H
