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
 * This file is part of Penumbra Overture.
 */

#include "hpl1/penumbra-overture/GameArea.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAreaLoader_GameArea::cAreaLoader_GameArea(const tString &asName, cInit *apInit)
	: iArea3DLoader(asName) {
	mpInit = apInit;
}

cAreaLoader_GameArea::~cAreaLoader_GameArea() {
}

//-----------------------------------------------------------------------

iEntity3D *cAreaLoader_GameArea::Load(const tString &asName, const cVector3f &avSize,
									  const cMatrixf &a_mtxTransform, cWorld3D *apWorld) {
	cGameArea *pArea = hplNew(cGameArea, (mpInit, asName));

	pArea->m_mtxOnLoadTransform = a_mtxTransform;

	// Create physics data
	iPhysicsWorld *pPhysicsWorld = apWorld->GetPhysicsWorld();
	iCollideShape *pShape = pPhysicsWorld->CreateBoxShape(avSize, NULL);
	std::vector<iPhysicsBody *> vBodies;
	vBodies.push_back(pPhysicsWorld->CreateBody(asName, pShape));

	vBodies[0]->SetCollide(false);
	vBodies[0]->SetCollideCharacter(false);
	vBodies[0]->SetMatrix(a_mtxTransform);

	vBodies[0]->SetUserData(pArea);
	pArea->SetBodies(vBodies);

	mpInit->mpMapHandler->AddGameEntity(pArea);

	// Log("Loaded area %s\n",asName.c_str());

	// Return something else later perhaps.
	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameArea::cGameArea(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_Area;

	mbUseCustomIcon = false;
}

//-----------------------------------------------------------------------

cGameArea::~cGameArea(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameArea::OnPlayerPick() {
	if (mbUseCustomIcon && mpInit->mpPlayer->GetPickedDist() <= mfMaxInteractDist) {
		mpInit->mpPlayer->SetCrossHairState(mCustomIcon);
	}
}

//-----------------------------------------------------------------------

void cGameArea::SetCustomIcon(eCrossHairState aIcon) {
	mbUseCustomIcon = true;
	mCustomIcon = aIcon;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cGameArea_SaveData, iGameEntity_SaveData)
	kSerializeVar(mbUseCustomIcon, eSerializeType_Bool)
		kSerializeVar(mCustomIcon, eSerializeType_Int32)
			kSerializeVar(mvSize, eSerializeType_Vector3f)
				kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameArea_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameArea::CreateSaveData() {
	return hplNew(cGameArea_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameArea::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameArea_SaveData *pData = static_cast<cGameArea_SaveData *>(apSaveData);

	kCopyToVar(pData, mbUseCustomIcon);
	kCopyToVar(pData, mCustomIcon);

	pData->mvSize = mvBodies[0]->GetShape()->GetSize();
}

//-----------------------------------------------------------------------

void cGameArea::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameArea_SaveData *pData = static_cast<cGameArea_SaveData *>(apSaveData);

	kCopyFromVar(pData, mbUseCustomIcon);
	kCopyFromVar(pData, mCustomIcon);
}
//-----------------------------------------------------------------------
