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

#include "hpl1/penumbra-overture/GameLamp.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEntityLoader_GameLamp::cEntityLoader_GameLamp(const tString &asName, cInit *apInit)
	: cEntityLoader_Object(asName) {
	mpInit = apInit;
}

cEntityLoader_GameLamp::~cEntityLoader_GameLamp() {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameLamp::BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
										cWorld3D *apWorld) {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameLamp::AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
									   cWorld3D *apWorld) {
	cGameLamp *pObject = hplNew(cGameLamp, (mpInit, mpEntity->GetName()));

	pObject->msFileName = msFileName;
	pObject->m_mtxOnLoadTransform = a_mtxTransform;

	// Set the engine objects to the object
	pObject->SetBodies(mvBodies);
	pObject->SetMeshEntity(mpEntity);
	pObject->SetLights(mvLights);
	pObject->SetParticleSystems(mvParticleSystems);
	pObject->SetBillboards(mvBillboards);
	pObject->SetSoundEntities(mvSoundEntities);

	///////////////////////////////////
	// Load game properties
	TiXmlElement *pGameElem = apRootElem->FirstChildElement("GAME");
	if (pGameElem) {
		pObject->mfTurnOnTime = cString::ToFloat(pGameElem->Attribute("TurnOnTime"), 0);
		pObject->mfTurnOffTime = cString::ToFloat(pGameElem->Attribute("TurnOffTime"), 0);

		pObject->mfMaxInteractDist = cString::ToFloat(pGameElem->Attribute("InteractDist"), 1.8f);

		pObject->mbInteractOff = cString::ToBool(pGameElem->Attribute("InteractOff"), true);
		pObject->mbInteractOn = cString::ToBool(pGameElem->Attribute("InteractOn"), true);

		pObject->msTurnOnSound = cString::ToString(pGameElem->Attribute("TurnOnSound"), "");
		pObject->msTurnOffSound = cString::ToString(pGameElem->Attribute("TurnOffSound"), "");

		pObject->msOnItem = cString::ToString(pGameElem->Attribute("OnItem"), "");
		pObject->msOffItem = cString::ToString(pGameElem->Attribute("OffItem"), "");

		pObject->msOffMaterialName = cString::ToString(pGameElem->Attribute("OffMaterial"), "");
		pObject->msOffSubMesh = cString::ToString(pGameElem->Attribute("OffSubMesh"), "");

		////////////////////////////////////
		// Flickering
		pObject->mbFlickering = cString::ToBool(pGameElem->Attribute("Flickering"), false);
		pObject->msFlickerOffSound = cString::ToString(pGameElem->Attribute("FlickerOffSound"), "");
		pObject->msFlickerOnSound = cString::ToString(pGameElem->Attribute("FlickerOnSound"), "");
		pObject->msFlickerOffPS = cString::ToString(pGameElem->Attribute("FlickerOffPS"), "");
		pObject->msFlickerOnPS = cString::ToString(pGameElem->Attribute("FlickerOnPS"), "");
		pObject->mfFlickerOnMinLength = cString::ToFloat(pGameElem->Attribute("FlickerOnMinLength"), 0);
		pObject->mfFlickerOffMinLength = cString::ToFloat(pGameElem->Attribute("FlickerOffMinLength"), 0);
		pObject->mfFlickerOnMaxLength = cString::ToFloat(pGameElem->Attribute("FlickerOnMaxLength"), 0);
		pObject->mfFlickerOffMaxLength = cString::ToFloat(pGameElem->Attribute("FlickerOffMaxLength"), 0);
		pObject->mFlickerOffColor = cString::ToColor(pGameElem->Attribute("FlickerOffColor"), cColor(0, 0));
		pObject->mfFlickerOffRadius = cString::ToFloat(pGameElem->Attribute("FlickerOffRadius"), 0);
		pObject->mbFlickerFade = cString::ToBool(pGameElem->Attribute("FlickerFade"), 0);
		pObject->mfFlickerOnFadeLength = cString::ToFloat(pGameElem->Attribute("FlickerOnFadeLength"), 0);
		pObject->mfFlickerOffFadeLength = cString::ToFloat(pGameElem->Attribute("FlickerOffFadeLength"), 0);
	} else {
		Error("Couldn't find game element for entity '%s'\n", mpEntity->GetName().c_str());
	}

	///////////////////////////////////
	// Add a the object as user data to the body, to get the obejct later on.
	for (size_t i = 0; i < mvBodies.size(); ++i) {
		if (mvBodies[i]) {
			mvBodies[i]->SetUserData((void *)pObject);
		}
	}

	pObject->Init();

	/////////////////////////////////
	// Add to map handler
	mpInit->mpMapHandler->AddGameEntity(pObject);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameLamp::cGameLamp(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_Lamp;
	mbHasInteraction = true;

	mbLit = true;
	mfMaxInteractDist = 2.1f;

	mfAlpha = 1;
	mfTurnOnTime = 2;
	mfTurnOffTime = 1;

	mbInteractOff = true;
	mbInteractOn = true;

	msTurnOnSound = "";
	msTurnOffSound = "";

	msOnItem = "";
	msOffItem = "";

	mpOffMaterial = NULL;
	mpOnMaterial = NULL;

	mpSubMesh = NULL;

	mbSaveLights = false;
}

//-----------------------------------------------------------------------

cGameLamp::~cGameLamp(void) {
	mpSubMesh->SetCustomMaterial(NULL, false);
	if (mpOffMaterial)
		mpInit->mpGame->GetResources()->GetMaterialManager()->Destroy(mpOffMaterial);
	if (mpOnMaterial)
		mpInit->mpGame->GetResources()->GetMaterialManager()->Destroy(mpOnMaterial);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameLamp::OnPlayerPick() {
	float fPickedDist = mpInit->mpPlayer->GetPickedDist();
	if (fPickedDist < mfMaxInteractDist) {
		if ((mbLit && mbInteractOff && msOffItem == "") || (!mbLit && mbInteractOn && msOnItem == "")) {
			mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Active);
		} else if (msDescription == _W("")) {
			if (mpInit->mpPlayer->GetState() == ePlayerState_InteractMode)
				mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Inactive);
			else
				mpInit->mpPlayer->SetCrossHairState(eCrossHairState_None);
		}
	} else if (fPickedDist > mfMaxExamineDist || msDescription == _W("")) {
		if (mpInit->mpPlayer->GetState() == ePlayerState_InteractMode)
			mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Inactive);
		else
			mpInit->mpPlayer->SetCrossHairState(eCrossHairState_None);
	}
}

//-----------------------------------------------------------------------

void cGameLamp::OnPlayerInteract() {
	if (mpInit->mpPlayer->GetPickedDist() < mfMaxInteractDist) {
		bool bInteracted = false;
		if (mbLit && mbInteractOff && msOffItem == "") {
			SetLit(false, true);
			bInteracted = true;
		} else if (!mbLit && mbInteractOn && msOnItem == "") {
			SetLit(true, true);
			bInteracted = true;
		}

		// If no interaction, use grab mode
		if (mpInit->mpPlayer->GetPickedBody()->GetMass() != 0 &&
			bInteracted == false && ((mbLit && mbInteractOff) || (!mbLit && mbInteractOn))) {
			mpInit->mpPlayer->mbPickAtPoint = false;
			mpInit->mpPlayer->mbRotateWithPlayer = true;
			mpInit->mpPlayer->mbUseNormalMass = false;
			mpInit->mpPlayer->mfGrabMassMul = (float)mvBodies.size();
			mpInit->mpPlayer->mfCurrentMaxInteractDist = mfMaxInteractDist;

			mpInit->mpPlayer->SetPushBody(mpInit->mpPlayer->GetPickedBody());
			mpInit->mpPlayer->ChangeState(ePlayerState_Grab);
		}
	}
}

//-----------------------------------------------------------------------

bool cGameLamp::OnUseItem(cInventoryItem *apItem) {
	if (mbLit && mbInteractOff && msOffItem == apItem->GetName()) {
		SetLit(false, true);
		return true;
	} else if (!mbLit && mbInteractOn && msOnItem == apItem->GetName()) {
		SetLit(true, true);
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

void cGameLamp::Update(float afTimeStep) {
	////////////////////////////////
	// Update alpha
	if (mbFlickering && mbLit) {
		for (size_t i = 0; i < mvLights.size(); ++i) {
			if (mvLights[i]->GetFlickerActive() == false &&
				mvLights[i]->IsFading() == false) {
				mvLights[i]->SetFlickerActive(true);
				SetUpFlicker((int)i);
			}
		}
	}

	////////////////////////////////
	// Check if material should be off because of flickering
	if (mbLit && mbFlickering && mpSubMesh && mpOffMaterial) {
		bool bHasLight = true;
		for (size_t i = 0; i < mvLights.size(); ++i) {
			iLight3D *pLight = mvLights[i];
			if (pLight->GetDiffuseColor() == cColor(0, 0, 0, 0) ||
				pLight->GetFarAttenuation() <= 0) {
				bHasLight = false;
				break;
			}
		}

		if (bHasLight) {
			if (mpOnMaterial)
				mpSubMesh->SetCustomMaterial(mpOnMaterial, false);
			else
				mpSubMesh->SetCustomMaterial(NULL, false);
		} else {
			mpSubMesh->SetCustomMaterial(mpOffMaterial, false);
		}
	}

	////////////////////////////////
	// Update alpha
	bool bChanged = false;
	if (mbLit && mfAlpha < 1) {
		mfAlpha += (1 / mfTurnOnTime) * afTimeStep;
		if (mfAlpha > 1.0f)
			mfAlpha = 1.0f;
		bChanged = true;
	} else if (mbLit == false && mfAlpha > 0) {
		mfAlpha -= (1 / mfTurnOffTime) * afTimeStep;
		if (mfAlpha < 0)
			mfAlpha = 0;
		bChanged = true;
	}

	////////////////////////////////
	// Update billboards
	if (bChanged) {
		for (size_t i = 0; i < mvBillboards.size(); ++i) {
			mvBillboards[i]->SetColor(mvBBColors[i] * mfAlpha);

			if (mfAlpha == 0)
				mvBillboards[i]->SetVisible(false);
			else if (mvBillboards[i]->IsVisible() == false)
				mvBillboards[i]->SetVisible(true);
		}
	}
}

//-----------------------------------------------------------------------

void cGameLamp::SetLit(bool abX, bool abFade) {
	if (mbLit == abX)
		return;

	mbLit = abX;

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	//////////////////////////////////////////////
	// Turn On
	if (mbLit) {
		for (size_t i = 0; i < mvLights.size(); ++i) {
			mvLights[i]->SetVisible(true);
			if (abFade) {
				mvLights[i]->SetFlickerActive(false);
				mvLights[i]->FadeTo(mvLightColors[i], mvLights[i]->GetFarAttenuation(), mfTurnOnTime);
			} else {
				mvLights[i]->SetDiffuseColor(mvLightColors[i]);
				mvLights[i]->SetFlickerActive(mbFlickering);
			}
		}
		for (size_t i = 0; i < mvParticleSystems.size(); ++i) {
			mvParticleSystems[i] = pWorld->CreateParticleSystem(
				mvParticleSystemNames[i].msName,
				mvParticleSystemNames[i].msDataName,
				1, mvParticleSystemNames[i].m_mtxTransform);
			mpMeshEntity->AddChild(mvParticleSystems[i]);

			/*Log("Creating ps %s at pos (%s) meshpos: (%s)\n",
				mvParticleSystems[i]->GetName().c_str(),
				mvParticleSystems[i]->GetWorldPosition().ToString().c_str(),
				mpMeshEntity->GetWorldPosition().ToString().c_str());*/

			mvParticleSystems[i]->SetTransformUpdated(true);

			if (!abFade) {
				for (int j = 0; j < 3 * 60; ++j)
					mvParticleSystems[i]->UpdateLogic(1.0f / 60.0f);
			}
		}
		for (size_t i = 0; i < mvBillboards.size(); ++i) {
			if (!abFade)
				mvBillboards[i]->SetVisible(true);
		}
		for (size_t i = 0; i < mvSoundEntities.size(); ++i) {
			if (abFade)
				mvSoundEntities[i]->Play(true);
			else
				mvSoundEntities[i]->Play(false);
		}

		if (mpSubMesh && mpOffMaterial) {
			if (mpOnMaterial)
				mpSubMesh->SetCustomMaterial(mpOnMaterial, false);
			else
				mpSubMesh->SetCustomMaterial(NULL, false);
		}
		if (!abFade)
			mfAlpha = 1;

		if (msTurnOnSound != "" && abFade) {
			cSoundEntity *pSound = pWorld->CreateSoundEntity("LampOn", msTurnOnSound, true);
			if (pSound)
				pSound->SetPosition(mpMeshEntity->GetBoundingVolume()->GetWorldCenter());
		}
	}
	//////////////////////////////////////////////
	// Turn Off
	else {
		for (size_t i = 0; i < mvLights.size(); ++i) {
			mvLights[i]->SetFlickerActive(false);
			if (abFade)
				mvLights[i]->FadeTo(cColor(0, 0), mvLights[i]->GetFarAttenuation(), mfTurnOffTime);
			else {
				mvLights[i]->SetVisible(false);
				mvLights[i]->SetDiffuseColor(cColor(0, 0));
			}
		}
		for (size_t i = 0; i < mvParticleSystems.size(); ++i) {
			if (abFade)
				mvParticleSystems[i]->Kill();
			else
				mvParticleSystems[i]->KillInstantly();

			mvParticleSystems[i] = NULL;
		}
		for (size_t i = 0; i < mvBillboards.size(); ++i) {
			if (!abFade)
				mvBillboards[i]->SetVisible(false);
		}
		for (size_t i = 0; i < mvSoundEntities.size(); ++i) {
			if (abFade)
				mvSoundEntities[i]->Stop(true);
			else
				mvSoundEntities[i]->Stop(false);
		}
		if (mpSubMesh && mpOffMaterial)
			mpSubMesh->SetCustomMaterial(mpOffMaterial, false);

		if (!abFade)
			mfAlpha = 0;

		if (msTurnOffSound != "" && abFade) {
			cSoundEntity *pSound = pWorld->CreateSoundEntity("LampOff", msTurnOffSound, true);
			if (pSound)
				pSound->SetPosition(mpMeshEntity->GetBoundingVolume()->GetWorldCenter());
		}
	}

	if (msLitChangeCallback != "") {
		tString sBool = mbLit ? "true" : "false";
		tString sCommand = msLitChangeCallback + "(" + sBool + ")";
		mpInit->RunScriptCommand(sCommand);
	}
}

//-----------------------------------------------------------------------

void cGameLamp::SetFlicker(bool abX) {
	mbFlickering = abX;
	for (size_t i = 0; i < mvLights.size(); ++i) {
		mvLights[i]->SetFlickerActive(mbFlickering);
		SetUpFlicker((int)i);
	}
}

//-----------------------------------------------------------------------

void cGameLamp::Init() {
	//////////////////////
	// Set up lights
	for (size_t i = 0; i < mvLights.size(); ++i) {
		mvLightColors.push_back(mvLights[i]->GetDiffuseColor());

		// Log("Setting lamp %s color to: %s\n",msName.c_str(),mvLights[i]->GetDiffuseColor().ToString().c_str());

		mvLights[i]->SetFlickerActive(mbFlickering);
		if (mbFlickering) {
			SetUpFlicker((int)i);
		}
	}

	//////////////////////
	// Billboards
	for (size_t i = 0; i < mvBillboards.size(); ++i) {
		mvBBColors.push_back(mvBillboards[i]->GetColor());
	}

	//////////////////////
	// Set up Particle Systems
	mvParticleSystemNames.resize(mvParticleSystems.size());
	for (size_t i = 0; i < mvParticleSystems.size(); ++i) {
		mvParticleSystemNames[i].msName = mvParticleSystems[i]->GetName();
		mvParticleSystemNames[i].msDataName = mvParticleSystems[i]->GetDataName();
		mvParticleSystemNames[i].m_mtxTransform = mvParticleSystems[i]->GetLocalMatrix();
	}

	//////////////////////
	// Set up Materials
	if (msOffSubMesh == "" || mpMeshEntity->GetSubMeshEntityNum() == 1) {
		mpSubMesh = mpMeshEntity->GetSubMeshEntity(0);
	} else {
		mpSubMesh = mpMeshEntity->GetSubMeshEntityName(msOffSubMesh);
	}
	if (mpSubMesh) {
		mpOffMaterial = mpInit->mpGame->GetResources()->GetMaterialManager()->CreateMaterial(msOffMaterialName);
		if (mpOffMaterial == NULL) {
			Warning("Could not load material '%s'\n", msOffMaterialName.c_str());
			return;
		}

		mpOnMaterial = mpSubMesh->GetCustomMaterial();
	} else {
		Warning("Couldn't find sub mesh '%s' for lamp\n", msOffSubMesh.c_str());
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameLamp::SetUpFlicker(int alIdx) {
	mvLights[alIdx]->SetFlicker(mFlickerOffColor, mfFlickerOffRadius,
								mfFlickerOnMinLength, mfFlickerOnMaxLength,
								msFlickerOnSound, msFlickerOnPS,
								mfFlickerOffMinLength, mfFlickerOffMaxLength,
								msFlickerOffSound, msFlickerOffPS,
								mbFlickerFade, mfFlickerOnFadeLength, mfFlickerOffFadeLength);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cGameLamp_SaveData, iGameEntity_SaveData)
	kSerializeVar(mbLit, eSerializeType_Bool)
		kSerializeVar(mbFlickering, eSerializeType_Bool)
			kSerializeVar(msLitChangeCallback, eSerializeType_String)
				kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameLamp_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameLamp::CreateSaveData() {
	return hplNew(cGameLamp_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameLamp::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameLamp_SaveData *pData = static_cast<cGameLamp_SaveData *>(apSaveData);

	kCopyToVar(pData, mbLit);
	kCopyToVar(pData, msLitChangeCallback);
	kCopyToVar(pData, mbFlickering);
}

//-----------------------------------------------------------------------

void cGameLamp::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameLamp_SaveData *pData = static_cast<cGameLamp_SaveData *>(apSaveData);

	kCopyFromVar(pData, msLitChangeCallback);

	SetLit(pData->mbLit, false);
	SetFlicker(pData->mbFlickering);
}
//-----------------------------------------------------------------------
