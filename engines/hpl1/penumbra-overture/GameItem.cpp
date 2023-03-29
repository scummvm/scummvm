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

#include "hpl1/penumbra-overture/GameItem.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameItemType.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// RAY CAST
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameItem_InViewRay::SetUp(iPhysicsBody *apSkipBody) {
	mpSkipBody = apSkipBody;
	mbIntersected = false;
}

bool cGameItem_InViewRay::OnIntersect(iPhysicsBody *apBody, cPhysicsRayParams *apParams) {
	// float &fDist = apParams->mfDist;

	// Must be positive
	if (apParams->mfT < 0 || apParams->mfT > 1)
		return true;

	// No characters or skip body allowed
	if (apBody->IsCharacter() || apBody == mpSkipBody)
		return true;

	if (apBody->GetCollide() == false || apBody->IsActive() == false)
		return true;

	mbIntersected = true;
	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEntityLoader_GameItem::cEntityLoader_GameItem(const tString &asName, cInit *apInit)
	: cEntityLoader_Object(asName) {
	mpInit = apInit;
}

cEntityLoader_GameItem::~cEntityLoader_GameItem() {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameItem::BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
										cWorld3D *apWorld) {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameItem::AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
									   cWorld3D *apWorld) {
	cGameItem *pObject = hplNew(cGameItem, (mpInit, mpEntity->GetName()));

	pObject->msSubType = msSubType;
	pObject->msFileName = msFileName;
	pObject->m_mtxOnLoadTransform = a_mtxTransform;

	// Set the engine objects to the object
	pObject->SetBodies(mvBodies);
	pObject->SetMeshEntity(mpEntity);

	///////////////////////////////////
	// Add a the object as user data to the body, to get the obejct later on.
	for (size_t i = 0; i < mvBodies.size(); ++i) {
		mvBodies[i]->SetUserData((void *)pObject);
	}
	///////////////////////////////////
	// Load game properties
	TiXmlElement *pGameElem = apRootElem->FirstChildElement("GAME");
	if (pGameElem) {
		pObject->mfEnterFlashDist = cString::ToFloat(pGameElem->Attribute("EnterFlashDist"), 3);
		pObject->mfExitFlashDist = cString::ToFloat(pGameElem->Attribute("ExitFlashDistt"), 6);

		pObject->mbSkipRayCheck = cString::ToBool(pGameElem->Attribute("SkipRayCheck"), false);

		pObject->msImageFile = cString::ToString(pGameElem->Attribute("ImageFile"), "");
		pObject->mbCanBeDropped = cString::ToBool(pGameElem->Attribute("CanBeDropped"), true);
		pObject->mbHasCount = cString::ToBool(pGameElem->Attribute("HasCount"), false);
		pObject->mlCount = cString::ToInt(pGameElem->Attribute("Count"), 1);

		pObject->msPickUpSound = cString::ToString(pGameElem->Attribute("PickUpSound"), "player_pickup_generic");

		tString sNameCat = cString::ToString(pGameElem->Attribute("NameCat"), "");
		tString sNameEntry = cString::ToString(pGameElem->Attribute("NameEntry"), "");
		tString sDescCat = cString::ToString(pGameElem->Attribute("DescCat"), "");
		tString sDescEntry = cString::ToString(pGameElem->Attribute("DescEntry"), "");

		pObject->mItemType = ToItemType(pGameElem->Attribute("ItemType"));

		pObject->msHudModelFile = cString::ToString(pGameElem->Attribute("HudModelFile"), "");
		pObject->msHudModelName = cString::ToString(pGameElem->Attribute("HudModelName"), "");

		pObject->SetGameName(kTranslate(sNameCat, sNameEntry));
		pObject->SetDescription(kTranslate(sDescCat, sDescEntry));
	} else {
		Error("Couldn't find game element for entity '%s'\n", mpEntity->GetName().c_str());
	}

	/////////////////////////////////
	// Add to map handler
	mpInit->mpMapHandler->AddGameEntity(pObject);
	mpInit->mpMapHandler->AddGameItem(pObject);
}

//-----------------------------------------------------------------------

eGameItemType cEntityLoader_GameItem::ToItemType(const char *apString) {
	if (apString == NULL)
		return eGameItemType_Normal;

	tString asType = cString::ToLowerCase(apString);

	if (asType == "normal")
		return eGameItemType_Normal;
	else if (asType == "food")
		return eGameItemType_Food;
	else if (asType == "note")
		return eGameItemType_Note;
	else if (asType == "map")
		return eGameItemType_Map;
	else if (asType == "flashlight")
		return eGameItemType_Flashlight;
	else if (asType == "notebook")
		return eGameItemType_Notebook;
	else if (asType == "battery")
		return eGameItemType_Battery;
	else if (asType == "glowstick")
		return eGameItemType_GlowStick;
	else if (asType == "flare")
		return eGameItemType_Flare;
	else if (asType == "painkillers")
		return eGameItemType_Painkillers;
	else if (asType == "weaponmelee")
		return eGameItemType_WeaponMelee;
	else if (asType == "throw")
		return eGameItemType_Throw;

	return eGameItemType_Normal;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameItem::cGameItem(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_Item;

	mbHasInteraction = true;
	mfMaxInteractDist = 2.0f;
	mfMaxExamineDist = 4.0f;

	mfCheckFlashCount = 0;
	mfCheckFlashMax = 1.0f / 20.0f;
	mfStartFlashCount = 0;
	mfFlashAlpha = 0;
	mfFlashAlphaAdd = -1;
	mbHasBeenFlashed = false;
	mfEnterFlashDist = 2;
	mfExitFlashDist = 5;
}

//-----------------------------------------------------------------------

cGameItem::~cGameItem(void) {
	mpInit->mpMapHandler->RemoveGameItem(this);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameItem::OnWorldLoad() {
}

//-----------------------------------------------------------------------

void cGameItem::OnPlayerPick() {
	if (mpInit->mpPlayer->GetPickedDist() <= mfMaxInteractDist) {
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_PickUp);
	}
}

//-----------------------------------------------------------------------

void cGameItem::OnPlayerInteract() {
	if (mpInit->mpPlayer->GetPickedDist() <= mfMaxInteractDist) {
		mpInit->mpInventory->AddItem(this);

		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui(msPickUpSound, false, 1);

		mbDestroyMe = true;
	}
}

//-----------------------------------------------------------------------

bool cGameItem::IsInView(float afMinDist) {
	cCamera3D *pCam = mpInit->mpPlayer->GetCamera();

	// Distance
	float fDistSqr = cMath::Vector3DistSqr(pCam->GetPosition(),
										   mvBodies[0]->GetWorldPosition());
	if (fDistSqr > afMinDist * afMinDist) {
		return false;
	}

	// Angle
	cVector3f vToMesh = mvBodies[0]->GetWorldPosition() - pCam->GetPosition();
	vToMesh.Normalise();
	float fAngle = cMath::Vector3Angle(pCam->GetForward(), vToMesh);
	if (fAngle > cMath::ToRad(43)) {
		return false;
	}

	if (mbSkipRayCheck)
		return true;

	// Raycast
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	mRayCallback.SetUp(mvBodies[0]);
	pPhysicsWorld->CastRay(&mRayCallback, pCam->GetPosition(),
						   mpMeshEntity->GetBoundingVolume()->GetWorldCenter(), // mvBodies[0]->GetWorldPosition(),
						   false, false, false);
	if (mRayCallback.mbIntersected) {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

void cGameItem::Update(float afTimeStep) {
	if (mbActive == false)
		return;
	if (mpInit->mbFlashItems == false)
		return;

	//////////////////////////////////////
	// Update flash
	if (mfFlashAlphaAdd > 0 || mfFlashAlpha > 0) {
		if (mfFlashAlphaAdd < 0) {
			mfFlashAlpha += mfFlashAlphaAdd * afTimeStep * 0.8f;
			if (mfFlashAlpha <= 0) {
				mfFlashAlpha = 0;
				// mfFlashAlphaAdd = -mfFlashAlphaAdd;
			}
		} else {
			mfFlashAlpha += mfFlashAlphaAdd * afTimeStep * 1.5f;
			if (mfFlashAlpha >= 1) {
				mfFlashAlpha = 1;
				mfFlashAlphaAdd = -mfFlashAlphaAdd;
			}
		}
	}
	//////////////////////////////////////
	// Check if player is near
	else if (mbHasBeenFlashed == false) {
		mfCheckFlashCount += afTimeStep;

		if (mfCheckFlashCount >= mfCheckFlashMax) {
			mfCheckFlashCount = 0;
			if (IsInView(mfEnterFlashDist)) {
				mfStartFlashCount += mfCheckFlashMax;
			} else {
				mfStartFlashCount -= mfCheckFlashMax;
				if (mfStartFlashCount < 0)
					mfStartFlashCount = 0;
			}
		}

		if (mfStartFlashCount >= 0.1f) {
			mfStartFlashCount = 0;
			mbHasBeenFlashed = true;
			mfFlashAlpha = 0;
			mfFlashAlphaAdd = 1;
		}
	}
	//////////////////////////////////////
	// Check if player is out of flash area.
	else {
		mfCheckFlashCount += afTimeStep;
		if (mfCheckFlashCount >= mfCheckFlashMax) {
			mfCheckFlashCount = 0;
			if (IsInView(mfExitFlashDist) == false) {
				mfStartFlashCount += mfCheckFlashMax;
			} else {
				mfStartFlashCount -= mfCheckFlashMax;
				if (mfStartFlashCount < 0)
					mfStartFlashCount = 0;
			}
		}

		if (mfStartFlashCount >= 0.75f) {
			mbHasBeenFlashed = false;
			mfStartFlashCount = 0;
		}
	}
}

//-----------------------------------------------------------------------

void cGameItem::OnPostSceneDraw() {
	/*iLowLevelGraphics *pLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();
	cCamera3D *pCam = mpInit->mpPlayer->GetCamera();


	pLowGfx->SetDepthTestActive(true);
	pLowGfx->SetDepthWriteActive(false);

	pLowGfx->SetBlendActive(true);
	pLowGfx->SetBlendFunc(eBlendFunc_One,eBlendFunc_One);

	for(int i=0; i<  mpMeshEntity->GetMesh()->GetSubMeshNum(); i++)
	{
		cSubMeshEntity *pSubEntity = mpMeshEntity->GetSubMeshEntity(i);
		cSubMesh *pSubMesh = mpMeshEntity->GetMesh()->GetSubMesh(i);
		iVertexBuffer *pVtxBuffer = pSubEntity->GetVertexBuffer();

		pLowGfx->SetTexture(0,NULL);
		pVtxBuffer->Bind();
		pVtxBuffer->Draw();
		pVtxBuffer->UnBind();
	}

	pLowGfx->SetBlendActive(false);
	pLowGfx->SetDepthTestActive(true);
	pLowGfx->SetDepthWriteActive(true);*/
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

kBeginSerialize(cGameItem_SaveData, iGameEntity_SaveData)
	kSerializeVar(mbHasBeenFlashed, eSerializeType_Bool)
		kSerializeVar(mfEnterFlashDist, eSerializeType_Float32)
			kSerializeVar(mfExitFlashDist, eSerializeType_Float32)
				kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameItem_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameItem::CreateSaveData() {
	return hplNew(cGameItem_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameItem::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameItem_SaveData *pData = static_cast<cGameItem_SaveData *>(apSaveData);

	kCopyToVar(pData, mbHasBeenFlashed);
	kCopyToVar(pData, mfEnterFlashDist);
	kCopyToVar(pData, mfExitFlashDist);
}

//-----------------------------------------------------------------------

void cGameItem::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameItem_SaveData *pData = static_cast<cGameItem_SaveData *>(apSaveData);

	kCopyFromVar(pData, mbHasBeenFlashed);
	kCopyFromVar(pData, mfEnterFlashDist);
	kCopyFromVar(pData, mfExitFlashDist);
}
//-----------------------------------------------------------------------
