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

#ifndef HPL_LIGHT3D_SPOT_H
#define HPL_LIGHT3D_SPOT_H

#include "hpl1/engine/scene/Light3D.h"

namespace hpl {

class cResources;
class iTexture;
class cFrustum;

//------------------------------------------

kSaveData_ChildClass(iLight3D, cLight3DSpot) {
	kSaveData_ClassInit(cLight3DSpot) public : tString msTexture;

	float mfFOV;
	float mfAspect;
	float mfNearClipPlane;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//------------------------------------------

class cLight3DSpot : public iLight3D {
	typedef iLight3D super;

public:
	cLight3DSpot(tString asName, cResources *apResources);
	~cLight3DSpot();

	const cMatrixf &GetViewMatrix();
	const cMatrixf &GetProjectionMatrix();
	const cMatrixf &GetViewProjMatrix();

	void SetTexture(iTexture *apTexture);
	iTexture *GetTexture();

	void SetFOV(float afAngle) {
		mfFOV = afAngle;
		mbProjectionUpdated = true;
	}
	float GetFOV() { return mfFOV; }

	void SetAspect(float afAngle) {
		mfAspect = afAngle;
		mbProjectionUpdated = true;
	}
	float GetAspect() { return mfAspect; }

	void SetNearClipPlane(float afX) {
		mfNearClipPlane = afX;
		mbProjectionUpdated = true;
	}
	float GetNearClipPlane() { return mfNearClipPlane; }

	void SetFarAttenuation(float afX);
	void SetNearAttenuation(float afX);

	cFrustum *GetFrustum();

	bool CollidesWithBV(cBoundingVolume *apBV);

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

private:
	cSectorVisibilityContainer *CreateSectorVisibility();
	void ExtraXMLProperties(TiXmlElement *apMainElem);
	void UpdateBoundingVolume();
	bool CreateClipRect(cRect2l &aCliprect, cRenderSettings *apRenderSettings, iLowLevelGraphics *apLowLevelGraphics);

	cMatrixf m_mtxProjection;
	cMatrixf m_mtxViewProj;
	cMatrixf m_mtxView;

	cFrustum *mpFrustum;

	iTexture *mpTexture;

	float mfFOV;
	float mfAspect;
	float mfNearClipPlane;

	bool mbProjectionUpdated;
	bool mbViewProjUpdated;
	bool mbFrustumUpdated;

	int mlViewProjMatrixCount;
	int mlViewMatrixCount;
	int mlFrustumMatrixCount;
};

} // namespace hpl

#endif // HPL_LIGHT3D_SPOT_H
