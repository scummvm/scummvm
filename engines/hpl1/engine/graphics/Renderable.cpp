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

#include "hpl1/engine/graphics/Renderable.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/math/Frustum.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/graphics/RenderList.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iRenderable::iRenderable(const tString &asName) : iEntity3D(asName)
	{
		mbRendered = true;
		mlLastMatrixCount = -1;

		mbStatic = false;

		mlRenderCount = -1;
		mlPrevRenderCount = -1;

		mlCalcScaleMatrixCount = -1;
		mvCalcScale = cVector3f(1,1,1);

		mbForceShadow = false;

		mbIsOneSided = false;
		mvOneSidedNormal =0;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cMatrixf* iRenderable::GetInvModelMatrix()
	{
		cMatrixf *pModelMatrix = GetModelMatrix(NULL);
		if(pModelMatrix==NULL) return NULL;

		if(mlLastMatrixCount != GetMatrixUpdateCount())
		{
			mlLastMatrixCount = GetMatrixUpdateCount();

			m_mtxInvModel = cMath::MatrixInverse(*pModelMatrix);
		}

		return &m_mtxInvModel;
	}

	//-----------------------------------------------------------------------

	const cVector3f& iRenderable::GetCalcScale()
	{
		cMatrixf *pModelMatrix = GetModelMatrix(NULL);

		if(pModelMatrix != NULL && mlCalcScaleMatrixCount != GetMatrixUpdateCount())
		{
			mlCalcScaleMatrixCount = GetMatrixUpdateCount();
			mvCalcScale.x = pModelMatrix->GetRight().Length();
			mvCalcScale.y = pModelMatrix->GetUp().Length();
			mvCalcScale.z = pModelMatrix->GetForward().Length();
		}

		return mvCalcScale;
	}

	//-----------------------------------------------------------------------

	bool iRenderable::CollidesWithBV(cBoundingVolume *apBV)
	{
		return cMath::CheckCollisionBV(*GetBoundingVolume(), *apBV);
	}

	//-----------------------------------------------------------------------

	bool iRenderable::CollidesWithFrustum(cFrustum *apFrustum)
	{
		return apFrustum->CollideBoundingVolume(GetBoundingVolume()) != eFrustumCollision_Outside;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// SAVE OBJECT STUFF
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	kBeginSerializeVirtual(cSaveData_iRenderable,cSaveData_iEntity3D)
	kSerializeVar(mbStatic, eSerializeType_Bool)
	kSerializeVar(mbRendered, eSerializeType_Bool)
	kSerializeVar(mfZ, eSerializeType_Float32)
	kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveData* iRenderable::CreateSaveData()
	{
		return NULL;
	}

	//-----------------------------------------------------------------------

	void iRenderable::SaveToSaveData(iSaveData *apSaveData)
	{
		kSaveData_SaveToBegin(iRenderable);

		kSaveData_SaveTo(mbStatic);
		kSaveData_SaveTo(mbRendered);
		kSaveData_SaveTo(mfZ);
	}

	//-----------------------------------------------------------------------

	void iRenderable::LoadFromSaveData(iSaveData *apSaveData)
	{
		kSaveData_LoadFromBegin(iRenderable);

		kSaveData_LoadFrom(mbStatic);
		kSaveData_LoadFrom(mbRendered);
		kSaveData_LoadFrom(mfZ);
	}

	//-----------------------------------------------------------------------

	void iRenderable::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame)
	{
		kSaveData_SetupBegin(iRenderable);
	}

	//-----------------------------------------------------------------------
}
