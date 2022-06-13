/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HPL_PHYSICS_JOINT_HINGE_H
#define HPL_PHYSICS_JOINT_HINGE_H

#include "hpl1/engine/physics/PhysicsJoint.h"

namespace hpl {

	//-----------------------------------

	kSaveData_ChildClass(iPhysicsJoint, iPhysicsJointHinge)
	{
		kSaveData_ClassInit(iPhysicsJointHinge)
	public:
		float mfMaxAngle;
		float mfMinAngle;

		virtual iSaveObject* CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler,cGame *apGame);
		virtual int GetSaveCreatePrio();
	};

	//-----------------------------------

	class iPhysicsJointHinge : public iPhysicsJoint
	{
	typedef iPhysicsJoint super;
	public:
		iPhysicsJointHinge(const tString &asName, iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
			iPhysicsWorld *apWorld,const cVector3f &avPivotPoint)
			: iPhysicsJoint(asName,apParentBody,apChildBody, apWorld,avPivotPoint){}
		virtual ~iPhysicsJointHinge(){}

		virtual void SetMaxAngle(float afAngle)=0;
		virtual void SetMinAngle(float afAngle)=0;
		virtual float GetMaxAngle()=0;
		virtual float GetMinAngle()=0;

		ePhysicsJointType GetType(){ return ePhysicsJointType_Hinge;}

		//SaveObject implementation
		virtual iSaveData* CreateSaveData();
		virtual void SaveToSaveData(iSaveData *apSaveData);
		virtual void LoadFromSaveData(iSaveData *apSaveData);
		virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

	protected:
		float mfMaxAngle;
		float mfMinAngle;
	};
};
#endif // HPL_PHYSICS_JOINT_HINGE_H
