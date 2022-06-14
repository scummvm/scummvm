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

#include "hpl1/engine/scene/Light3DPoint.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Camera3D.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/scene/SectorVisibility.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLight3DPoint::cLight3DPoint(tString asName, cResources *apResources) : iLight3D(asName, apResources) {
	mLightType = eLight3DType_Point;

	UpdateBoundingVolume();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSectorVisibilityContainer *cLight3DPoint::CreateSectorVisibility() {
	return mpWorld3D->GetPortalContainer()->CreateVisibiltyFromBV(GetBoundingVolume());
}

//-----------------------------------------------------------------------

void cLight3DPoint::UpdateBoundingVolume() {
	mBoundingVolume.SetSize(mfFarAttenuation * 2);
	mBoundingVolume.SetPosition(GetWorldPosition());
}
//-----------------------------------------------------------------------

bool cLight3DPoint::CreateClipRect(cRect2l &aClipRect, cRenderSettings *apRenderSettings,
								   iLowLevelGraphics *apLowLevelGraphics) {
	cVector2f vScreenSize = apLowLevelGraphics->GetScreenSize();
	bool bVisible = cMath::GetClipRectFromBV(aClipRect, *GetBoundingVolume(),
											 apRenderSettings->mpCamera->GetViewMatrix(),
											 apRenderSettings->mpCamera->GetProjectionMatrix(),
											 apRenderSettings->mpCamera->GetNearClipPlane(),
											 cVector2l((int)vScreenSize.x, (int)vScreenSize.y));
	return bVisible;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_cLight3DPoint, cSaveData_iLight3D)
	kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cLight3DPoint::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	cWorld3D *pWorld = apGame->GetScene()->GetWorld3D();

	return pWorld->CreateLightPoint(msName);
}

//-----------------------------------------------------------------------

int cSaveData_cLight3DPoint::GetSaveCreatePrio() {
	return 3;
}

//-----------------------------------------------------------------------

iSaveData *cLight3DPoint::CreateSaveData() {
	return hplNew(cSaveData_cLight3DPoint, ());
}

//-----------------------------------------------------------------------

void cLight3DPoint::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(cLight3DPoint);
}

//-----------------------------------------------------------------------

void cLight3DPoint::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(cLight3DPoint);
}

//-----------------------------------------------------------------------

void cLight3DPoint::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(cLight3DPoint);
}

//-----------------------------------------------------------------------

} // namespace hpl
