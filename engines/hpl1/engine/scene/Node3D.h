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

#ifndef HPL_NODE3D_H
#define HPL_NODE3D_H

#include "common/list.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Node.h"

namespace hpl {

//------------------------------------

kSaveData_ChildClass(iNode, cNode3D) {
	kSaveData_ClassInit(cNode3D) public : tString msName;
	tString msSource;
	bool mbAutoDeleteChildren;
	cMatrixf m_mtxLocalTransform;
	int mlParentId;

	iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	int GetSaveCreatePrio();
};

//------------------------------------

class cNode3D : public iNode {
	typedef iNode super;

public:
	cNode3D(const tString &asName = "", bool abAutoDeleteChildren = true);
	virtual ~cNode3D();

	iNode *CreateChild();
	cNode3D *CreateChild3D(const tString &asName = "", bool abAutoDeleteChildren = true);

	cVector3f GetLocalPosition();
	cMatrixf &GetLocalMatrix();

	cVector3f GetWorldPosition();
	cMatrixf &GetWorldMatrix();

	void SetPosition(const cVector3f &avPos);
	void SetMatrix(const cMatrixf &a_mtxTransform, bool abSetChildrenUpdated = true);

	void SetWorldPosition(const cVector3f &avWorldPos);
	void SetWorldMatrix(const cMatrixf &a_mtxWorldTransform);

	const char *GetName();

	cNode3D *GetParent();

	void AddRotation(const cVector3f &avRot, eEulerRotationOrder aOrder);
	void AddRotation(const cQuaternion &aqRotation);

	void AddScale(const cVector3f &avScale);

	void AddTranslation(const cVector3f &avTrans);

	void SetSource(const tString &asSource);
	const char *GetSource();

	/**
	 * Updates the matrix with the added scales, translations and rotation. It also resets these values.
	 */
	void UpdateMatrix(bool abSetChildrenUpdated);

	// Extra stuff that shouldn't be used externally
	void SetParent(cNode3D *apNode);
	void AddChild(cNode3D *apChild);

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

	void UpdateWorldTransform();
	void SetWorldTransformUpdated();

private:
	tString msName;
	tString msSource;

	bool mbAutoDeleteChildren;

	cMatrixf m_mtxLocalTransform;
	cMatrixf m_mtxWorldTransform;

	cVector3f mvWorldPosition;

	cMatrixf m_mtxRotation;
	cVector3f mvScale;
	cVector3f mvTranslation;

	bool mbTransformUpdated;

	cNode3D *mpParent;
};

} // namespace hpl

#endif // HPL_NODE3D_H
