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

#include "hpl1/penumbra-overture/GameSaveArea.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAreaLoader_GameSaveArea::cAreaLoader_GameSaveArea(const tString &asName, cInit *apInit)
	: iArea3DLoader(asName) {
	mpInit = apInit;
}

cAreaLoader_GameSaveArea::~cAreaLoader_GameSaveArea() {
}

//-----------------------------------------------------------------------

iEntity3D *cAreaLoader_GameSaveArea::Load(const tString &asName, const cVector3f &avSize,
										  const cMatrixf &a_mtxTransform, cWorld3D *apWorld) {
	cGameSaveArea *pLink = hplNew(cGameSaveArea, (mpInit, asName));

	pLink->m_mtxOnLoadTransform = a_mtxTransform;

	// Create physics data
	iPhysicsWorld *pPhysicsWorld = apWorld->GetPhysicsWorld();
	iCollideShape *pShape = pPhysicsWorld->CreateBoxShape(avSize, NULL);
	Common::Array<iPhysicsBody *> vBodies;
	vBodies.push_back(pPhysicsWorld->CreateBody(asName, pShape));

	vBodies[0]->SetCollide(false);
	vBodies[0]->SetCollideCharacter(false);
	vBodies[0]->SetMatrix(a_mtxTransform);

	vBodies[0]->SetUserData(pLink);
	pLink->SetBodies(vBodies);

	mpInit->mpMapHandler->AddGameEntity(pLink);

	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameSaveArea::cGameSaveArea(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_SaveArea;

	msDescription = kTranslate("Save", "DefaultDesc");

	msMessageCat = "";
	msMessageEntry = "";

	msSound = "";

	mbHasBeenUsed = false;
}

//-----------------------------------------------------------------------

cGameSaveArea::~cGameSaveArea(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameSaveArea::OnPlayerPick() {
	if (mpInit->mpPlayer->GetPickedDist() <= 2.1) {
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Active);
	}
}

void cGameSaveArea::OnPlayerInteract() {
	if (mpInit->mpPlayer->GetPickedDist() <= 2.1) {
		mpInit->mpEffectHandler->GetSaveEffect()->NormalSave(mvBodies[0]->GetWorldPosition(), this);
	}
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

kBeginSerialize(cGameSaveArea_SaveData, iGameEntity_SaveData)
	kSerializeVar(mvSize, eSerializeType_Vector3f)
		kSerializeVar(msMessageCat, eSerializeType_String)
			kSerializeVar(msMessageEntry, eSerializeType_String)

				kSerializeVar(msSound, eSerializeType_String)

					kSerializeVar(mbHasBeenUsed, eSerializeType_Bool)
						kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameSaveArea_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameSaveArea::CreateSaveData() {
	return hplNew(cGameSaveArea_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameSaveArea::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameSaveArea_SaveData *pData = static_cast<cGameSaveArea_SaveData *>(apSaveData);

	pData->mvSize = mvBodies[0]->GetShape()->GetSize();

	kCopyToVar(pData, msMessageCat);
	kCopyToVar(pData, msMessageEntry);

	kCopyToVar(pData, msSound);

	kCopyToVar(pData, mbHasBeenUsed);
}

//-----------------------------------------------------------------------

void cGameSaveArea::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameSaveArea_SaveData *pData = static_cast<cGameSaveArea_SaveData *>(apSaveData);

	kCopyFromVar(pData, msMessageCat);
	kCopyFromVar(pData, msMessageEntry);

	kCopyFromVar(pData, msSound);

	kCopyFromVar(pData, mbHasBeenUsed);
}
//-----------------------------------------------------------------------
