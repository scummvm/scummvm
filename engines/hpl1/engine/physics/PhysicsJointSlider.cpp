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

#include "hpl1/engine/physics/PhysicsJointSlider.h"

#include "hpl1/engine/scene/World3D.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/game/Game.h"

#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// SAVE OBJECT STUFF
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	kBeginSerialize(cSaveData_iPhysicsJointSlider, cSaveData_iPhysicsJoint)
		kSerializeVar(mfMaxDistance, eSerializeType_Float32)
		kSerializeVar(mfMinDistance, eSerializeType_Float32)
		kSerializeVar(mvPin, eSerializeType_Vector3f)
		kEndSerialize()

		//-----------------------------------------------------------------------

		iSaveObject* cSaveData_iPhysicsJointSlider::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler,cGame *apGame)
	{
		iPhysicsWorld *apWorld = apGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

		cMatrixf mtxChildTemp, mtxParentTemp;

		iPhysicsBody *pChildBody = static_cast<iPhysicsBody*>(apSaveObjectHandler->Get(mlChildBodyId));
		if(pChildBody==NULL) return NULL;

		iPhysicsBody *pParentBody = NULL;
		if(mlParentBodyId>0) pParentBody = static_cast<iPhysicsBody*>(apSaveObjectHandler->Get(mlParentBodyId));

		mtxChildTemp = pChildBody->GetLocalMatrix();
		if(pParentBody) mtxParentTemp = pParentBody->GetLocalMatrix();

		pChildBody->SetMatrix(m_mtxChildBodySetup);
		if(pParentBody) pParentBody->SetMatrix(m_mtxParentBodySetup);

		iPhysicsJointSlider *pJoint = apWorld->CreateJointSlider(msName,mvStartPivotPoint,mvPinDir,pParentBody,pChildBody);

		pChildBody->SetMatrix(mtxChildTemp);
		if(pParentBody) pParentBody->SetMatrix(mtxParentTemp);

		return pJoint;
	}

	//-----------------------------------------------------------------------

	int cSaveData_iPhysicsJointSlider::GetSaveCreatePrio()
	{
		return 1;
	}

	//-----------------------------------------------------------------------

	iSaveData* iPhysicsJointSlider::CreateSaveData()
	{
		return hplNew( cSaveData_iPhysicsJointSlider, () );
	}

	//-----------------------------------------------------------------------

	void iPhysicsJointSlider::SaveToSaveData(iSaveData *apSaveData)
	{
		kSaveData_SaveToBegin(iPhysicsJointSlider);

		kSaveData_SaveTo(mfMaxDistance);
		kSaveData_SaveTo(mfMinDistance);
		kSaveData_SaveTo(mvPin);
	}

	//-----------------------------------------------------------------------

	void iPhysicsJointSlider::LoadFromSaveData(iSaveData *apSaveData)
	{
		kSaveData_LoadFromBegin(iPhysicsJointSlider);

		kSaveData_LoadFrom(mfMaxDistance);
		kSaveData_LoadFrom(mfMinDistance);
		kSaveData_LoadFrom(mvPin);
	}

	//-----------------------------------------------------------------------

	void iPhysicsJointSlider::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame)
	{
		kSaveData_SetupBegin(iPhysicsJointSlider);
	}

	//-----------------------------------------------------------------------
}
