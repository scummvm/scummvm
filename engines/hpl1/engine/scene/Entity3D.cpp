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

#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Node3D.h"

#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/scene/PortalContainer.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
iEntity3D::iEntity3D(tString asName) : iEntity(asName) {
	m_mtxLocalTransform = cMatrixf::Identity;
	m_mtxWorldTransform = cMatrixf::Identity;

	mbTransformUpdated = true;

	mlCount = 0;

	mlGlobalRenderCount = cRenderList::GetGlobalRenderCount();

	msSourceFile = "";

	mbApplyTransformToBV = true;
	mbUpdateBoundingVolume = true;

	mpParent = NULL;

	mlIteratorCount = -1;

	mpCurrentSector = NULL;
}

iEntity3D::~iEntity3D() {
	if (mpParent)
		mpParent->RemoveChild(this);

	for (tEntity3DListIt it = mlstChildren.begin(); it != mlstChildren.end(); ++it) {
		iEntity3D *pChild = *it;
		pChild->mpParent = NULL;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cVector3f iEntity3D::GetLocalPosition() {
	return m_mtxLocalTransform.GetTranslation();
}

//-----------------------------------------------------------------------

cMatrixf &iEntity3D::GetLocalMatrix() {
	return m_mtxLocalTransform;
}

//-----------------------------------------------------------------------

cVector3f iEntity3D::GetWorldPosition() {
	UpdateWorldTransform();

	return m_mtxWorldTransform.GetTranslation();
}

//-----------------------------------------------------------------------

cMatrixf &iEntity3D::GetWorldMatrix() {
	UpdateWorldTransform();

	return m_mtxWorldTransform;
}

//-----------------------------------------------------------------------

void iEntity3D::SetPosition(const cVector3f &avPos) {
	m_mtxLocalTransform.m[0][3] = avPos.x;
	m_mtxLocalTransform.m[1][3] = avPos.y;
	m_mtxLocalTransform.m[2][3] = avPos.z;

	SetTransformUpdated();
}

//-----------------------------------------------------------------------

void iEntity3D::SetMatrix(const cMatrixf &a_mtxTransform) {
	m_mtxLocalTransform = a_mtxTransform;

	SetTransformUpdated();
}

//-----------------------------------------------------------------------

void iEntity3D::SetWorldPosition(const cVector3f &avWorldPos) {
	if (mpParent) {
		SetPosition(avWorldPos - mpParent->GetWorldPosition());
	} else {
		SetPosition(avWorldPos);
	}
}

//-----------------------------------------------------------------------

void iEntity3D::SetWorldMatrix(const cMatrixf &a_mtxWorldTransform) {
	if (mpParent) {
		SetMatrix(cMath::MatrixMul(cMath::MatrixInverse(mpParent->GetWorldMatrix()),
								   a_mtxWorldTransform));
	} else {
		SetMatrix(a_mtxWorldTransform);
	}
}

//-----------------------------------------------------------------------

void iEntity3D::SetTransformUpdated(bool abUpdateCallbacks) {
	mbTransformUpdated = true;
	mlCount++;

	// Perhaps not update this yet? This is baaaad!
	if (mbApplyTransformToBV)
		mBoundingVolume.SetTransform(GetWorldMatrix());

	mbUpdateBoundingVolume = true;

	// Update children
	for (tEntity3DListIt EntIt = mlstChildren.begin(); EntIt != mlstChildren.end(); ++EntIt) {
		iEntity3D *pChild = *EntIt;
		pChild->SetTransformUpdated(true);
	}

	// Update callbacks
	if (mlstCallbacks.empty() || abUpdateCallbacks == false)
		return;

	tEntityCallbackListIt it = mlstCallbacks.begin();
	for (; it != mlstCallbacks.end(); ++it) {
		iEntityCallback *pCallback = *it;
		pCallback->OnTransformUpdate(this);
	}
}

//-----------------------------------------------------------------------

bool iEntity3D::GetTransformUpdated() {
	return mbTransformUpdated;
}

//-----------------------------------------------------------------------

int iEntity3D::GetTransformUpdateCount() {
	return mlCount;
}

//-----------------------------------------------------------------------

void iEntity3D::AddCallback(iEntityCallback *apCallback) {
	mlstCallbacks.push_back(apCallback);
}

//-----------------------------------------------------------------------

void iEntity3D::RemoveCallback(iEntityCallback *apCallback) {
	STLFindAndDelete(mlstCallbacks, apCallback);
}

//-----------------------------------------------------------------------

void iEntity3D::AddChild(iEntity3D *apEntity) {
	if (apEntity == NULL)
		return;
	if (apEntity->mpParent != NULL)
		return;

	mlstChildren.push_back(apEntity);
	apEntity->mpParent = this;

	apEntity->SetTransformUpdated(true);
}

void iEntity3D::RemoveChild(iEntity3D *apEntity) {
	for (tEntity3DListIt it = mlstChildren.begin(); it != mlstChildren.end();) {
		iEntity3D *pChild = *it;
		if (pChild == apEntity) {
			pChild->mpParent = NULL;
			it = mlstChildren.erase(it);
		} else {
			++it;
		}
	}
}

bool iEntity3D::IsChild(iEntity3D *apEntity) {
	for (tEntity3DListIt it = mlstChildren.begin(); it != mlstChildren.end(); ++it) {
		iEntity3D *pChild = *it;
		if (pChild == apEntity)
			return true;
	}
	return false;
}

iEntity3D *iEntity3D::GetEntityParent() {
	return mpParent;
}

//-----------------------------------------------------------------------
bool iEntity3D::IsInSector(cSector *apSector) {
	// Log("-- %s --\n",msName.c_str());
	// bool bShouldReturnTrue = false;

	if (apSector == GetCurrentSector()) {
		// Log("Should return true\n");
		// bShouldReturnTrue = true;
		return true;
	}

	tRenderContainerDataList *pDataList = GetRenderContainerDataList();
	tRenderContainerDataListIt it = pDataList->begin();
	for (; it != pDataList->end(); ++it) {
		iRenderContainerData *pRenderContainerData = *it;
		cSector *pSector = static_cast<cSector *>(pRenderContainerData);

		// Log("%s (%d) vs %s (%d)\n",pSector->GetId().c_str(),pSector, apSector->GetId().c_str(),apSector);
		if (pSector == apSector) {
			// Log("return true!\n");
			return true;
		}
	}

	// if(bShouldReturnTrue)Log(" %s should have returned true. Sectors: %d\n",msName.c_str(),		mlstRenderContainerData.size());
	// Log("return false!\n");
	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
void iEntity3D::UpdateWorldTransform() {
	if (mbTransformUpdated) {
		mbTransformUpdated = false;

		// first check if there is a node parent
		if (mpParentNode) {
			cNode3D *pNode3D = static_cast<cNode3D *>(mpParentNode);

			m_mtxWorldTransform = cMath::MatrixMul(pNode3D->GetWorldMatrix(), m_mtxLocalTransform);
		}
		// If there is no node parent check for entity parent
		else if (mpParent) {
			m_mtxWorldTransform = cMath::MatrixMul(mpParent->GetWorldMatrix(), m_mtxLocalTransform);
		} else {
			m_mtxWorldTransform = m_mtxLocalTransform;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerializeVirtual(cSaveData_iEntity3D, cSaveData_iEntity)
	kSerializeVar(m_mtxLocalTransform, eSerializeType_Matrixf)
		kSerializeVar(mBoundingVolume, eSerializeType_Class)
			kSerializeVar(msSourceFile, eSerializeType_String)

				kSerializeVar(mlParentId, eSerializeType_Int32)
					kSerializeVarContainer(mlstChildren, eSerializeType_Int32)
						kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveData *iEntity3D::CreateSaveData() {
	return NULL;
}

//-----------------------------------------------------------------------

void iEntity3D::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(iEntity3D);

	// Log("-------- Saving %s --------------\n",msName.c_str());

	kSaveData_SaveTo(m_mtxLocalTransform);
	kSaveData_SaveTo(mBoundingVolume);
	kSaveData_SaveTo(msSourceFile);

	kSaveData_SaveObject(mpParent, mlParentId);
	kSaveData_SaveIdList(mlstChildren, tEntity3DListIt, mlstChildren);

	/*if(mlstChildren.empty()==false)
	{
		Log("Children in '%s'/'%s': ",msName.c_str(),GetEntityType().c_str());
		for(tEntity3DListIt it=mlstChildren.begin(); it != mlstChildren.end(); ++it)
		{
			iEntity3D *pEntity = *it;
			Log("('%d/%s'/'%s'), ",pEntity->GetSaveObjectId(),pEntity->GetName().c_str(),pEntity->GetEntityType().c_str());
		}
		Log("\n");
	}*/
}

//-----------------------------------------------------------------------

void iEntity3D::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(iEntity3D);

	// Log("-------- Loading %s --------------\n",msName.c_str());

	SetMatrix(pData->m_mtxLocalTransform);

	// Not sure of this is needed:
	kSaveData_LoadFrom(mBoundingVolume);

	kSaveData_LoadFrom(msSourceFile);
}

//-----------------------------------------------------------------------

void iEntity3D::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(iEntity3D);

	// Log("-------- Setup %s --------------\n",msName.c_str());

	// kSaveData_LoadObject(mpParent,mlParentId,iEntity3D*);
	// kSaveData_LoadIdList(mlstChildren,mlstChildren,iEntity3D*);
	if (pData->mlParentId != -1 && mpParent == NULL) {
		iEntity3D *pParentEntity = static_cast<iEntity3D *>(apSaveObjectHandler->Get(pData->mlParentId));
		if (pParentEntity)
			pParentEntity->AddChild(this);
		else
			Error("Couldn't find parent entity id %d for '%s'\n", pData->mlParentId, GetName().c_str());
	}

	cContainerListIterator<int> it = pData->mlstChildren.GetIterator();
	while (it.HasNext()) {
		int mlId = it.Next();
		if (mlId != -1) {
			iEntity3D *pChildEntity = static_cast<iEntity3D *>(apSaveObjectHandler->Get(mlId));
			if (pChildEntity)
				AddChild(pChildEntity);
			else
				Error("Couldn't find child entity id %d for '%s'\n", mlId, GetName().c_str());
		}
	}

	SetTransformUpdated(true);
}

//-----------------------------------------------------------------------
} // namespace hpl
