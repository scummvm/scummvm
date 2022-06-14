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

#include "hpl1/engine/physics/PhysicsJointBall.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_iPhysicsJointBall, cSaveData_iPhysicsJoint)
	kSerializeVar(mfMaxConeAngle, eSerializeType_Float32)
		kSerializeVar(mfMaxTwistAngle, eSerializeType_Float32)
			kSerializeVar(mvConePin, eSerializeType_Vector3f)
				kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_iPhysicsJointBall::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	iPhysicsWorld *apWorld = apGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	cMatrixf mtxChildTemp, mtxParentTemp;

	iPhysicsBody *pChildBody = static_cast<iPhysicsBody *>(apSaveObjectHandler->Get(mlChildBodyId));
	if (pChildBody == NULL)
		return NULL;

	iPhysicsBody *pParentBody = NULL;
	if (mlParentBodyId > 0)
		pParentBody = static_cast<iPhysicsBody *>(apSaveObjectHandler->Get(mlParentBodyId));

	mtxChildTemp = pChildBody->GetLocalMatrix();
	if (pParentBody)
		mtxParentTemp = pParentBody->GetLocalMatrix();

	pChildBody->SetMatrix(m_mtxChildBodySetup);
	if (pParentBody)
		pParentBody->SetMatrix(m_mtxParentBodySetup);

	iPhysicsJointBall *pJoint = apWorld->CreateJointBall(msName, mvStartPivotPoint, pParentBody, pChildBody);
	pJoint->SetConeLimits(mvConePin, mfMaxConeAngle, mfMaxTwistAngle);

	pChildBody->SetMatrix(mtxChildTemp);
	if (pParentBody)
		pParentBody->SetMatrix(mtxParentTemp);

	return pJoint;
}

//-----------------------------------------------------------------------

int cSaveData_iPhysicsJointBall::GetSaveCreatePrio() {
	return 1;
}

//-----------------------------------------------------------------------

iSaveData *iPhysicsJointBall::CreateSaveData() {
	return hplNew(cSaveData_iPhysicsJointBall, ());
}

//-----------------------------------------------------------------------

void iPhysicsJointBall::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(iPhysicsJointBall);

	kSaveData_SaveTo(mfMaxConeAngle);
	kSaveData_SaveTo(mfMaxTwistAngle);
	kSaveData_SaveTo(mvConePin);
}

//-----------------------------------------------------------------------

void iPhysicsJointBall::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(iPhysicsJointBall);

	kSaveData_LoadFrom(mfMaxConeAngle);
	kSaveData_LoadFrom(mfMaxTwistAngle);
	kSaveData_LoadFrom(mvConePin);
}

//-----------------------------------------------------------------------

void iPhysicsJointBall::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(iPhysicsJointBall);
}

//-----------------------------------------------------------------------
} // namespace hpl
