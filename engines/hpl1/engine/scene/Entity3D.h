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

#ifndef HPL_ENTITY3D_H
#define HPL_ENTITY3D_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Entity.h"
#include "hpl1/engine/system/Container.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "common/list.h"
#include "hpl1/engine/math/BoundingVolume.h"

namespace hpl {

class iEntity3D;
class cSector;

class iEntityCallback {
public:
	virtual ~iEntityCallback() = default;
	virtual void OnTransformUpdate(iEntity3D *apEntity) = 0;
};

typedef Common::List<iEntityCallback *> tEntityCallbackList;
typedef tEntityCallbackList::iterator tEntityCallbackListIt;

//------------------------------------

// Used for the render container to add specific data to
// the object.
class iRenderContainerData {
};

typedef Common::List<iRenderContainerData *> tRenderContainerDataList;
typedef tRenderContainerDataList::iterator tRenderContainerDataListIt;

//------------------------------------

kSaveData_ChildClass(iEntity, iEntity3D) {
	kSaveData_ClassInit(iEntity3D) public : cMatrixf m_mtxLocalTransform;
	cBoundingVolume mBoundingVolume;
	tString msSourceFile;

	int mlParentId;
	cContainerList<int> mlstChildren;
};

//-----------------------------------------

class iEntity3D;

typedef Common::List<iEntity3D *> tEntity3DList;
typedef tEntity3DList::iterator tEntity3DListIt;

class iEntity3D : public iEntity {
	typedef iEntity super;

public:
	iEntity3D(tString asName);
	virtual ~iEntity3D();

	cVector3f GetLocalPosition();
	cMatrixf &GetLocalMatrix();

	cVector3f GetWorldPosition();
	cMatrixf &GetWorldMatrix();

	void SetPosition(const cVector3f &avPos);
	void SetMatrix(const cMatrixf &a_mtxTransform);

	void SetWorldPosition(const cVector3f &avWorldPos);
	void SetWorldMatrix(const cMatrixf &a_mtxWorldTransform);

	void SetTransformUpdated(bool abUpdateCallbacks = true);
	bool GetTransformUpdated();

	int GetTransformUpdateCount();

	void AddCallback(iEntityCallback *apCallback);
	void RemoveCallback(iEntityCallback *apCallback);

	void SetSourceFile(const tString &asFile) { msSourceFile = asFile; }
	const tString &GetSourceFile() { return msSourceFile; }

	virtual cBoundingVolume *GetBoundingVolume() { return &mBoundingVolume; }

	/**
	 * The child hierarchy will only work if the child has no node parent.
	 **/
	void AddChild(iEntity3D *apEntity);
	void RemoveChild(iEntity3D *apEntity);
	bool IsChild(iEntity3D *apEntity);
	iEntity3D *GetEntityParent();

	virtual tRenderContainerDataList *GetRenderContainerDataList() { return &mlstRenderContainerData; }

	inline int GetIteratorCount() { return mlIteratorCount; }
	inline void SetIteratorCount(const int alX) { mlIteratorCount = alX; }

	inline void SetCurrentSector(cSector *apSetor) { mpCurrentSector = apSetor; }
	virtual cSector *GetCurrentSector() const { return mpCurrentSector; }

	bool IsInSector(cSector *apSector);

	inline int GetGlobalRenderCount() { return mlGlobalRenderCount; }
	inline void SetGlobalRenderCount(int alX) { mlGlobalRenderCount = alX; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	cMatrixf m_mtxLocalTransform;
	cMatrixf m_mtxWorldTransform;

	cBoundingVolume mBoundingVolume;
	bool mbUpdateBoundingVolume;
	bool mbApplyTransformToBV; // Only temp!!

	bool mbTransformUpdated;

	int mlGlobalRenderCount;

	int mlCount;

	tString msSourceFile;

	tEntityCallbackList mlstCallbacks;

	tRenderContainerDataList mlstRenderContainerData;

	tEntity3DList mlstChildren;
	iEntity3D *mpParent;

	cSector *mpCurrentSector;

	int mlIteratorCount;

private:
	void UpdateWorldTransform();
};

} // namespace hpl

#endif // HPL_ENTITY3D_H
