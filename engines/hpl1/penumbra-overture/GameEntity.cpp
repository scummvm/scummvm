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

#include "hpl1/penumbra-overture/GameEntity.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/GameStickArea.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"
#include "hpl1/penumbra-overture/PlayerState.h"

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGameEntity::iGameEntity(cInit *apInit, const tString &asName) {
	mpInit = apInit;

	msName = asName;
	msFileName = "";

	mbActive = true;

	mbIsSaved = true;

	mfHealth = 0;
	mlToughness = 0;

	mbDestroyMe = false;
	mbBreakMe = false;

	mpMeshEntity = NULL;

	mpCharBody = NULL;

	mType = eGameEntityType_Unkown;

	msDescription = _W("");
	msGameName = _W("");

	mbShowDescritionOnce = false;

	mfMaxExamineDist = 6.0f;
	mfMaxInteractDist = 1.5f;

	mbHasInteraction = false;

	mbHasBeenExamined = false;

	mbPauseControllers = false;
	mbPauseGravity = false;

	for (int i = 0; i < eGameEntityScriptType_LastEnum; ++i) {
		mvCallbackScripts[i] = NULL;
	}

	mbSaveLights = true;

	mbUpdatingCollisionCallbacks = false;

	mbTransActive = false;

	mvLastImpulse = 0;
}

//-----------------------------------------------------------------------

iGameEntity::~iGameEntity() {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	//////////////////////////////////////////////
	// Remove all references to from all otter
	// Log("Deleting '%s'\n",msName.c_str());

	// Player
	mpInit->mpPlayer->RemoveCollideScriptWithChildEntity(this);

	// Other entities (check so not all entities are being destroyed,
	//                  in that case it is not needed and might be bad).
	if (mpInit->mpMapHandler->IsDestroyingAll() == false) {
		tGameEntityIterator it = mpInit->mpMapHandler->GetGameEntityIterator();
		while (it.HasNext()) {
			iGameEntity *pEntity = it.Next();
			pEntity->RemoveCollideScriptWithChildEntity(this);
		}
	}

	//////////////////////////////////////////////
	// Destroy all graphics in the entity!
	if (pWorld && mpInit->mbDestroyGraphics) {
		for (size_t i = 0; i < mvBodies.size(); ++i) {
			// Make sure that this body is not picked!
			if (mpInit->mpPlayer->GetPickedBody() == mvBodies[i])
				mpInit->mpPlayer->GetPickRay()->mpPickedBody = NULL;

			if (mpInit->mpPlayer->GetPushBody() == mvBodies[i]) {
				ePlayerState state = mpInit->mpPlayer->GetState();
				if (state == ePlayerState_Move ||
					state == ePlayerState_Grab ||
					state == ePlayerState_Push) {
					ePlayerState prevState = mpInit->mpPlayer->GetStateData(state)->mPreviuosState;
					if (prevState == ePlayerState_InteractMode)
						mpInit->mpPlayer->ChangeState(ePlayerState_InteractMode);
					else
						mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
				}

				mpInit->mpPlayer->SetPushBody(NULL);
			}

			pWorld->GetPhysicsWorld()->DestroyBody(mvBodies[i]);
		}
		if (mpMeshEntity)
			pWorld->DestroyMeshEntity(mpMeshEntity);

		for (size_t i = 0; i < mvLights.size(); ++i)
			pWorld->DestroyLight(mvLights[i]);
		for (size_t i = 0; i < mvParticleSystems.size(); ++i)
			if (mvParticleSystems[i])
				mvParticleSystems[i]->Kill();
		for (size_t i = 0; i < mvBillboards.size(); ++i)
			pWorld->DestroyBillboard(mvBillboards[i]);
		for (size_t i = 0; i < mvBeams.size(); ++i)
			pWorld->DestroyBeam(mvBeams[i]);
		for (size_t i = 0; i < mvSoundEntities.size(); ++i) {
			pWorld->DestroySoundEntity(mvSoundEntities[i]);
		}

		if (mpCharBody)
			pWorld->GetPhysicsWorld()->DestroyCharacterBody(mpCharBody);
	}

	// Delete callbacks
	for (int i = 0; i < eGameEntityScriptType_LastEnum; ++i) {
		if (mvCallbackScripts[i])
			hplDelete(mvCallbackScripts[i]);
	}
	STLMapDeleteAll(m_mapCollideCallbacks);

	STLDeleteAll(mvTransMaterials);

	for (size_t i = 0; i < mvPreloadedBreakMeshes.size(); ++i) {
		mpInit->mpGame->GetResources()->GetMeshManager()->Destroy(mvPreloadedBreakMeshes[i]);
	}
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void iGameEntity::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	for (size_t i = 0; i < mvBodies.size(); ++i) {
		mvBodies[i]->SetActive(mbActive);
		mvBodies[i]->SetTransformUpdated(true);
	}

	if (mpMeshEntity) {
		mpMeshEntity->SetVisible(mbActive);
		mpMeshEntity->SetActive(mbActive);
		if (mbActive)
			mpMeshEntity->UpdateLogic(0.01f);
	}

	if (mpCharBody) {
		mpCharBody->SetActive(mbActive);
		if (mbActive) {
			// Pre update the character body to make sure it is on ground.
			// Seems to mess up stuff
			/*for(int i=0; i<120; ++i)
			{
				//mpCharBody->Update(1.0f / 60.0f);
			}*/
		}
	}

	for (size_t i = 0; i < mvParticleSystems.size(); ++i) {
		if (mvParticleSystems[i])
			mvParticleSystems[i]->SetVisible(mbActive);
		if (mvParticleSystems[i])
			mvParticleSystems[i]->SetActive(mbActive);
	}

	for (size_t i = 0; i < mvLights.size(); ++i)
		if (mvLights[i])
			mvLights[i]->SetVisible(mbActive);

	for (size_t i = 0; i < mvBillboards.size(); ++i)
		if (mvBillboards[i])
			mvBillboards[i]->SetVisible(mbActive);

	OnSetActive(mbActive);
}

//-----------------------------------------------------------------------

float iGameEntity::GetPickedDistance() {
	return mpInit->mpPlayer->GetPickedDist();
}

//-----------------------------------------------------------------------

eCrossHairState iGameEntity::GetPickCrossHairState(iPhysicsBody *apBody) {
	float fDistance = GetPickedDistance();

	//////////////////////////////////////////
	// Interaction available
	cGameStickArea *pStickArea = mpInit->mpMapHandler->GetBodyStickArea(apBody);
	if (apBody->GetMass() != 0 || mType == eGameEntityType_Area || (pStickArea && pStickArea->GetCanDeatch())) {
		if (mvCallbackScripts[eGameEntityScriptType_PlayerInteract] || mbHasInteraction) {
			if (fDistance <= mfMaxInteractDist)
				return eCrossHairState_Active;
		}
	}

	//////////////////////////////////////////
	// Examine available
	if (mvCallbackScripts[eGameEntityScriptType_PlayerExamine] || msDescription != _W("")) {
		if (fDistance <= mfMaxExamineDist)
			return eCrossHairState_Examine;
	}

	//////////////////////////////////////////
	// Too far
	if (mvCallbackScripts[eGameEntityScriptType_PlayerInteract] || mbHasInteraction) {
		// if(fDistance <= mfMaxExamineDist) return eCrossHairState_Invalid;
		if (apBody->GetMass() == 0 && mvCallbackScripts[eGameEntityScriptType_PlayerInteract] == NULL) {
			return eCrossHairState_None;
		} else {
			return eCrossHairState_Invalid;
		}
	}

	return eCrossHairState_None;
}

//-----------------------------------------------------------------------

void iGameEntity::DestroyLight(iLight3D *apLight) {
	STLFindAndRemove(mvLights, apLight);
	mpInit->mpGame->GetScene()->GetWorld3D()->DestroyLight(apLight);
}
void iGameEntity::DestroyParticleSystem(cParticleSystem3D *apPS) {
	STLFindAndRemove(mvParticleSystems, apPS);
	mpInit->mpGame->GetScene()->GetWorld3D()->DestroyParticleSystem(apPS);
}
void iGameEntity::DestroyBillboard(cBillboard *apBillboard) {
	STLFindAndRemove(mvBillboards, apBillboard);
	mpInit->mpGame->GetScene()->GetWorld3D()->DestroyBillboard(apBillboard);
}
void iGameEntity::SetSoundEntity(cSoundEntity *apSound) {
	STLFindAndRemove(mvSoundEntities, apSound);
	mpInit->mpGame->GetScene()->GetWorld3D()->DestroySoundEntity(apSound);
}

//-----------------------------------------------------------------------

void iGameEntity::PlayerPick() {
	//////////////////////
	// Script stuff
	/*cWorld3D *pWorld = */ mpInit->mpGame->GetScene()->GetWorld3D();
	if (mvCallbackScripts[eGameEntityScriptType_PlayerPick]) {
		tString sCommand = GetScriptCommand(eGameEntityScriptType_PlayerPick);
		mpInit->RunScriptCommand(sCommand);
	}

	OnPlayerPick();
}

void iGameEntity::PlayerInteract() {
	//////////////////////
	// Script stuff
	if (GetPickedDistance() <= mfMaxInteractDist) {
		/*cWorld3D *pWorld = */ mpInit->mpGame->GetScene()->GetWorld3D();
		if (mvCallbackScripts[eGameEntityScriptType_PlayerInteract]) {
			tString sCommand = GetScriptCommand(eGameEntityScriptType_PlayerInteract);
			mpInit->RunScriptCommand(sCommand);
		}
	}

	OnPlayerInteract();
}

void iGameEntity::PlayerExamine() {
	//////////////////////
	// Script stuff
	if (GetPickedDistance() <= mfMaxExamineDist) {
		/*cWorld3D *pWorld = */ mpInit->mpGame->GetScene()->GetWorld3D();
		if (mvCallbackScripts[eGameEntityScriptType_PlayerExamine]) {
			tString sCommand = GetScriptCommand(eGameEntityScriptType_PlayerExamine);
			mpInit->RunScriptCommand(sCommand);
		}
	}

	OnPlayerExamine();
}

//-----------------------------------------------------------------------

void iGameEntity::OnPlayerExamine() {
	if (mfMaxExamineDist >= mpInit->mpPlayer->GetPickedDist() && msDescription != _W("")) {
		mpInit->mpGameMessageHandler->Add(msDescription);
		// if(mbShowDescritionOnce) msDescription = _W("");
		mbHasBeenExamined = true;

		//////////////////////////////
		// Set focus on the object
		mpInit->mpEffectHandler->GetDepthOfField()->FocusOnBody(mpInit->mpPlayer->GetPickedBody());
		mpInit->mpEffectHandler->GetDepthOfField()->SetFocusBody(NULL);
		mpInit->mpEffectHandler->GetDepthOfField()->SetActive(true, 1.2f);

		mpInit->mpGameMessageHandler->SetFocusIsedUsed(true);
	}
}

//-----------------------------------------------------------------------

void iGameEntity::Damage(float afDamage, int alStrength) {
	if (mfHealth > 0) {
		if (mType == eGameEntityType_Enemy) {
			// if(mpInit->mDifficulty== eGameDifficulty_Easy) afDamage *= 2.0f;
			if (mpInit->mDifficulty == eGameDifficulty_Hard)
				afDamage /= 2.0f;
		}

		int lDiff = mlToughness - alStrength;

		if (alStrength >= 0) {
			float fDamageMul = 1 - (0.25f * (float)lDiff);
			if (fDamageMul < 0)
				fDamageMul = 0;

			// Could be 2 here, depends on what you wanna do. This way the damage is never increased.
			if (fDamageMul > 1)
				fDamageMul = 1;

			afDamage *= fDamageMul;
		}

		mfHealth -= ABS(afDamage);

		if (mfHealth <= 0) {
			OnDeath(afDamage);
		} else {
			OnDamage(afDamage);
		}
	}
}

void iGameEntity::SetHealth(float afHealth) {
	if (afHealth <= 0 && mfHealth > 0) {
		mfHealth = afHealth;
		OnDeath(0);
	} else {
		mfHealth = afHealth;
	}
}

//-----------------------------------------------------------------------

void iGameEntity::SetUpTransMaterials() {
	mvNormalMaterials.resize(mpMeshEntity->GetSubMeshEntityNum());
	mvTransMaterials.resize(mpMeshEntity->GetSubMeshEntityNum());

	mbTransShadow = mpMeshEntity->IsShadowCaster();

	for (int i = 0; i < mpMeshEntity->GetSubMeshEntityNum(); ++i) {
		cSubMeshEntity *pSubEntity = mpMeshEntity->GetSubMeshEntity(i);
		cSubMesh *pSubMesh = mpMeshEntity->GetMesh()->GetSubMesh(i);

		iMaterial *pNormalMaterial = pSubEntity->GetMaterial();

		mvNormalMaterials[i] = pSubEntity->GetCustomMaterial();

		// create material for the transperancy
		iMaterial *pTransMaterial = mpInit->mpGame->GetGraphics()->GetMaterialHandler()->Create(
			"Trans", "Modulative", eMaterialPicture_Texture);

		// Set texture for the trans material
		iTexture *pDiffTex = pNormalMaterial->GetTexture(eMaterialTexture_Diffuse);
		if (pDiffTex) {
			pDiffTex->IncUserCount();
			pTransMaterial->SetTexture(pDiffTex, eMaterialTexture_Diffuse);

			mvTransMaterials[i] = pTransMaterial;
		} else {
			Log("Sub mesh '%s' material '%s' does not have diffuse!\n", pSubMesh->GetName().c_str(),
				pNormalMaterial->GetName().c_str());
		}
	}
}

void iGameEntity::SetTransActive(bool abX) {
	if (mbTransActive == abX)
		return;

	mbTransActive = abX;

	if (mbTransShadow) {
		// mpMeshEntity->SetForceShadow(mbTransActive);
	}

	for (int i = 0; i < mpMeshEntity->GetSubMeshEntityNum(); ++i) {
		cSubMeshEntity *pSubEntity = mpMeshEntity->GetSubMeshEntity(i);

		if (mbTransActive) {
			pSubEntity->SetCustomMaterial(mvTransMaterials[i], false);
		} else {
			pSubEntity->SetCustomMaterial(mvNormalMaterials[i], false);
		}
	}
}

//-----------------------------------------------------------------------

static inline tString GetCollideCommand(const tString &asFuncName, const tString &asParent,
										const tString &asChild) {
	return asFuncName + "(\"" + asParent + "\", \"" + asChild + "\")";
}

////////////////////////////

void iGameEntity::OnUpdate(float afTimeStep) {
	if (mbActive == false)
		return;

	////////////////////////////////////////////
	/// Script Collide test stuff
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	/*cWorld3D *pWorld = */ mpInit->mpGame->GetScene()->GetWorld3D();

	////////////////
	// If entity has character body add it to the array and then remove.
	Common::Array<iPhysicsBody *> vTempBodies;
	if (mpCharBody) {
		for (size_t i = 0; i < mvBodies.size(); ++i) {
			vTempBodies.push_back(mvBodies[i]);
		}
		mvBodies.clear();
		mvBodies.push_back(mpCharBody->GetBody());
	}

	cCollideData collideData;
	collideData.SetMaxSize(1);

	// if(msName == "liftclose") Log("--- Start collision test\n");
	mbUpdatingCollisionCallbacks = true;
	tGameCollideScriptMapIt CollideIt = m_mapCollideCallbacks.begin();
	for (; CollideIt != m_mapCollideCallbacks.end(); ++CollideIt) {
		cGameCollideScript *pCallback = CollideIt->second;
		iGameEntity *pEntity = pCallback->mpEntity;

		if (pEntity->IsActive() == false)
			continue;

		bool bCollide = false;

		for (size_t i = 0; i < mvBodies.size(); ++i)
			for (size_t j = 0; j < pEntity->mvBodies.size(); ++j) {
				iPhysicsBody *pParentBody = mvBodies[i];
				iPhysicsBody *pChildBody = pEntity->mvBodies[j];

				// if(msName == "liftclose") Log("Start shape collision....");
				if (cMath::CheckCollisionBV(*pParentBody->GetBV(), *pChildBody->GetBV())) {
					bCollide = pPhysicsWorld->CheckShapeCollision(pParentBody->GetShape(),
																  pParentBody->GetLocalMatrix(),
																  pChildBody->GetShape(),
																  pChildBody->GetLocalMatrix(),
																  collideData, 1);
				}
				// if(msName == "liftclose") Log("end it\n");
				if (bCollide)
					break;
			}

		// Run Collide scripts
		if (bCollide) {
			// if(msName == "liftclose") Log("entity %s collided!\n",msName.c_str());

			if (pCallback->mbCollides) {
				if (pCallback->msFuncName[eGameCollideScriptType_During] != "") {
					tString sCommand = GetCollideCommand(
						pCallback->msFuncName[eGameCollideScriptType_During],
						msName, CollideIt->first);
					mpInit->RunScriptCommand(sCommand);
				}
			} else {
				if (pCallback->msFuncName[eGameCollideScriptType_Enter] != "") {
					tString sCommand = GetCollideCommand(
						pCallback->msFuncName[eGameCollideScriptType_Enter],
						msName, CollideIt->first);
					mpInit->RunScriptCommand(sCommand);
				}

				pCallback->mbCollides = true;
			}
		} else {
			if (pCallback->mbCollides) {
				if (pCallback->msFuncName[eGameCollideScriptType_Leave] != "") {
					tString sCommand = GetCollideCommand(
						pCallback->msFuncName[eGameCollideScriptType_Leave],
						msName, CollideIt->first);
					mpInit->RunScriptCommand(sCommand);
				}

				pCallback->mbCollides = false;
			}
		}
	}
	mbUpdatingCollisionCallbacks = false;

	////////////////
	// If entity has character body remove the previuously added.
	if (mpCharBody) {
		mvBodies.clear();
		for (size_t i = 0; i < vTempBodies.size(); ++i)
			mvBodies.push_back(vTempBodies[i]);
	}

	// if(msName == "liftclose") Log("--- End collision test\n");

	//////////////////////////////////////////////////
	// Check if any callback should be deleted
	CollideIt = m_mapCollideCallbacks.begin();
	for (; CollideIt != m_mapCollideCallbacks.end();) {
		cGameCollideScript *pCallback = CollideIt->second;
		tGameCollideScriptMapIt currentIt = CollideIt;
		++CollideIt;

		if (pCallback->mbDeleteMe) {
			hplDelete(pCallback);
			m_mapCollideCallbacks.erase(currentIt);
		}
	}

	////////////////////////////////////////////
	/// Script Update
	if (mvCallbackScripts[eGameEntityScriptType_OnUpdate]) {
		tString sCommand = GetScriptCommand(eGameEntityScriptType_OnUpdate);
		mpInit->RunScriptCommand(sCommand);
	}

	///////////////////////////////////////////
	// update entity specific stuff.
	Update(afTimeStep);
}

//-----------------------------------------------------------------------

void iGameEntity::AddCollideScript(eGameCollideScriptType aType, const tString &asFunc, const tString &asEntity) {
	cGameCollideScript *pCallback;

	// Check if the function already exist
	tGameCollideScriptMapIt it = m_mapCollideCallbacks.find(asEntity);
	if (it != m_mapCollideCallbacks.end()) {
		pCallback = it->second;
	} else {
		pCallback = hplNew(cGameCollideScript, ());

		// Get the entity
		iGameEntity *pEntity = mpInit->mpMapHandler->GetGameEntity(asEntity);
		if (pEntity == NULL) {
			Warning("Couldn't find entity '%s'\n", asEntity.c_str());
			hplDelete(pCallback);
			return;
		}

		// Set the entity
		pCallback->mpEntity = pEntity;

		// Add to container
		m_mapCollideCallbacks.insert(tGameCollideScriptMap::value_type(asEntity, pCallback));
	}

	pCallback->msFuncName[aType] = asFunc;
}

//-----------------------------------------------------------------------

void iGameEntity::RemoveCollideScriptWithChildEntity(iGameEntity *apEntity) {
	tGameCollideScriptMapIt it = m_mapCollideCallbacks.begin();
	for (; it != m_mapCollideCallbacks.end();) {
		cGameCollideScript *pCallback = it->second;
		tGameCollideScriptMapIt currentIt = it;
		++it;

		if (pCallback && pCallback->mpEntity == apEntity) {
			if (mbUpdatingCollisionCallbacks) {
				pCallback->mbDeleteMe = true;
			} else {
				hplDelete(pCallback);
				m_mapCollideCallbacks.erase(currentIt);
			}
		}
	}
}

//-----------------------------------------------------------------------

void iGameEntity::RemoveCollideScript(eGameCollideScriptType aType, const tString &asEntity) {
	tGameCollideScriptMapIt it = m_mapCollideCallbacks.find(asEntity);
	if (it != m_mapCollideCallbacks.end()) {
		cGameCollideScript *pCallback = it->second;

		pCallback->msFuncName[aType] = "";
		// if there are no functions left, erase
		if (pCallback->msFuncName[0] == "" && pCallback->msFuncName[1] == "" && pCallback->msFuncName[2] == "") {
			if (mbUpdatingCollisionCallbacks) {
				pCallback->mbDeleteMe = true;
			} else {
				hplDelete(pCallback);
				m_mapCollideCallbacks.erase(it);
			}
		}
	} else {
		Warning("Entity '%s' callback doesn't exist in '%s'\n", asEntity.c_str(), msName.c_str());
	}
}

//-----------------------------------------------------------------------

void iGameEntity::AddScript(eGameEntityScriptType aType, const tString &asFunc) {
	cGameEntityScript *pScript = mvCallbackScripts[aType];

	if (pScript == NULL) {
		pScript = hplNew(cGameEntityScript, ());
		mvCallbackScripts[aType] = pScript;
	}

	pScript->msScriptFunc = asFunc;
}

//-----------------------------------------------------------------------

void iGameEntity::RemoveScript(eGameEntityScriptType aType) {
	if (mvCallbackScripts[aType]) {
		hplDelete(mvCallbackScripts[aType]);
		mvCallbackScripts[aType] = NULL;
	}
}

//-----------------------------------------------------------------------

void iGameEntity::CreateVar(const tString &asName, int alVal) {
	tGameEntityVarMapIt it = m_mapVars.find(asName);
	if (it == m_mapVars.end()) {
		m_mapVars.insert(tGameEntityVarMap::value_type(asName, alVal));
	}
}

void iGameEntity::SetVar(const tString &asName, int alVal) {
	tGameEntityVarMapIt it = m_mapVars.find(asName);
	if (it == m_mapVars.end()) {
		Warning("Entity '%s' var '%s' not found!\n", msName.c_str(), asName.c_str());
		return;
	}

	it->second = alVal;
}

void iGameEntity::AddVar(const tString &asName, int alVal) {
	tGameEntityVarMapIt it = m_mapVars.find(asName);
	if (it == m_mapVars.end()) {
		Warning("Entity '%s' var '%s' not found!\n", msName.c_str(), asName.c_str());
		return;
	}

	it->second += alVal;
}

int iGameEntity::GetVar(const tString &asName) {
	tGameEntityVarMapIt it = m_mapVars.find(asName);
	if (it == m_mapVars.end()) {
		Warning("Entity '%s' var '%s' not found!\n", msName.c_str(), asName.c_str());
		return 0;
	}

	return it->second;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

tString iGameEntity::GetScriptCommand(eGameEntityScriptType aType) {
	return mvCallbackScripts[aType]->msScriptFunc + "(\"" + msName + "\")";
}

//-----------------------------------------------------------------------

void iGameEntity::PreloadModel(const tString &asFile) {
	tString sFileName = cString::SetFileExt(asFile, "ent");
	tString sPath = mpInit->mpGame->GetResources()->GetFileSearcher()->GetFilePath(sFileName);

	if (sPath != "") {
		TiXmlDocument *pEntityDoc = hplNew(TiXmlDocument, ());
		if (pEntityDoc->LoadFile(sPath.c_str()) == false) {
			Error("Couldn't load '%s'!\n", sPath.c_str());
		} else {
			TiXmlElement *pRootElem = pEntityDoc->FirstChildElement();
			TiXmlElement *pGraphicsElem = pRootElem->FirstChildElement("GRAPHICS");

			tString sModelFile = cString::ToString(pGraphicsElem->Attribute("ModelFile"), "");

			cMesh *pMesh = mpInit->mpGame->GetResources()->GetMeshManager()->CreateMesh(sModelFile);
			mvPreloadedBreakMeshes.push_back(pMesh);

			for (int i = 0; i < pMesh->GetReferenceNum(); ++i) {
				cMeshReference *pRef = pMesh->GetReference(i);

				PreloadModel(pRef->msFile);
			}
		}
		hplDelete(pEntityDoc);
	} else {
		Error("Entity file '%s' was not found!\n", sFileName.c_str());
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEnginePS_SaveData *iGameEntity_SaveData::GetParticleSystem(cParticleSystem3D *apPS) {
	for (size_t i = 0; i < mvPS.Size(); ++i) {
		if (mvPS[i].msName == apPS->GetName())
			return &mvPS[i];
	}

	return NULL;
}

cEngineSound_SaveData *iGameEntity_SaveData::GetSoundEntity(cSoundEntity *apSound) {
	for (size_t i = 0; i < mvSounds.Size(); ++i) {
		if (mvSounds[i].msName == apSound->GetName())
			return &mvSounds[i];
	}

	return NULL;
}

//-----------------------------------------------------------------------

kBeginSerializeBase(cGameEntityScript)
	kSerializeVar(mlNum, eSerializeType_Int32)
		kSerializeVar(msScriptFunc, eSerializeType_String)
			kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cGameEntityAnimation_SaveData)
		kSerializeVar(mbActive, eSerializeType_Bool)
			kSerializeVar(mbLoop, eSerializeType_Bool)

				kSerializeVar(mfWeight, eSerializeType_Float32)
					kSerializeVar(mfFadeStep, eSerializeType_Float32)
						kSerializeVar(mfTimePos, eSerializeType_Float32)
							kSerializeVar(mfSpeed, eSerializeType_Float32)
								kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBaseVirtual(iGameEntity_SaveData)
		kSerializeVar(mbActive, eSerializeType_Bool)
			kSerializeVar(msFileName, eSerializeType_String)
				kSerializeVar(msName, eSerializeType_String)
					kSerializeVar(mfHealth, eSerializeType_Float32)
						kSerializeVar(mfMaxExamineDist, eSerializeType_Float32)
							kSerializeVar(mfMaxInteractDist, eSerializeType_Float32)
								kSerializeVar(msGameName, eSerializeType_WString)
									kSerializeVar(msDescription, eSerializeType_WString)
										kSerializeVar(mbHasBeenExamined, eSerializeType_Bool)
											kSerializeVar(mbShowDescritionOnce, eSerializeType_Bool)
												kSerializeVar(mType, eSerializeType_Int32)
													kSerializeVar(m_mtxTransform, eSerializeType_Matrixf)

														kSerializeClassContainer(mlstCollideCallbacks, cSaveGame_cGameCollideScript, eSerializeType_Class)
															kSerializeClassContainer(mlstCallbackScripts, cGameEntityScript, eSerializeType_Class)
																kSerializeClassContainer(mlstVars, cScriptVar, eSerializeType_Class)

																	kSerializeClassContainer(mvBodies, cEngineBody_SaveData, eSerializeType_Class)
																		kSerializeClassContainer(mvPS, cEnginePS_SaveData, eSerializeType_Class)
																			kSerializeClassContainer(mvLights, cEngineLight_SaveData, eSerializeType_Class)
																				kSerializeClassContainer(mvSounds, cEngineSound_SaveData, eSerializeType_Class)

																					kSerializeClassContainer(mvAnimations, cGameEntityAnimation_SaveData, eSerializeType_Class)

																						kEndSerialize()
	//-----------------------------------------------------------------------

	void iGameEntity::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	// Properties
	kCopyToVar(apSaveData, mbActive);
	kCopyToVar(apSaveData, msName);
	kCopyToVar(apSaveData, msFileName);
	kCopyToVar(apSaveData, mfHealth);
	kCopyToVar(apSaveData, mfMaxInteractDist);
	kCopyToVar(apSaveData, mfMaxExamineDist);
	kCopyToVar(apSaveData, msGameName);
	kCopyToVar(apSaveData, msDescription);
	kCopyToVar(apSaveData, mbShowDescritionOnce);
	kCopyToVar(apSaveData, mbHasBeenExamined);
	kCopyToVar(apSaveData, mType);

	apSaveData->m_mtxTransform = m_mtxOnLoadTransform;

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	// Collide scripts
	tGameCollideScriptMapIt colIt = m_mapCollideCallbacks.begin();
	for (; colIt != m_mapCollideCallbacks.end(); ++colIt) {
		cGameCollideScript *pScript = colIt->second;
		cSaveGame_cGameCollideScript savedScript;
		savedScript.LoadFrom(pScript);

		apSaveData->mlstCollideCallbacks.Add(savedScript);
	}

	// Script functions
	for (int i = 0; i < eGameEntityScriptType_LastEnum; ++i) {
		if (mvCallbackScripts[i]) {
			cGameEntityScript script;
			script.mlNum = i;
			script.msScriptFunc = mvCallbackScripts[i]->msScriptFunc;
			apSaveData->mlstCallbackScripts.Add(script);
		}
	}

	// Script variables
	tGameEntityVarMapIt varIt = m_mapVars.begin();
	for (; varIt != m_mapVars.end(); ++varIt) {
		cScriptVar scriptVar;
		scriptVar.mlVal = varIt->second;
		scriptVar.msName = varIt->first;
		apSaveData->mlstVars.Add(scriptVar);
	}

	// Bodies
	apSaveData->mvBodies.Resize(mvBodies.size());
	for (size_t i = 0; i < mvBodies.size(); ++i) {
		apSaveData->mvBodies[i].FromBody(mvBodies[i]);
	}

	// Log("Saving particles for %s\n",GetName().c_str());
	// Particle Systems
	apSaveData->mvPS.Resize(mvParticleSystems.size());
	for (size_t i = 0; i < mvParticleSystems.size(); ++i) {
		// Log("%d, ",i);

		if (pWorld->ParticleSystemExists(mvParticleSystems[i]) == false) {
			mvParticleSystems[i] = NULL;
			Warning("particle system %d in %s does not exist anymore!\n", i, GetName().c_str());
		}

		apSaveData->mvPS[i].FromPS(mvParticleSystems[i]);
	}
	// Log("Done\n");

	// Lights
	if (mbSaveLights) {
		apSaveData->mvLights.Resize(mvLights.size());
		for (size_t i = 0; i < mvLights.size(); ++i) {
			apSaveData->mvLights[i].FromLight(mvLights[i]);
		}
	}

	// Sounds
	apSaveData->mvSounds.Resize(mvSoundEntities.size());
	for (size_t i = 0; i < mvSoundEntities.size(); ++i) {
		apSaveData->mvSounds[i].FromSound(mvSoundEntities[i]);
	}

	// Animations
	if (mpMeshEntity) {
		apSaveData->mvAnimations.Resize(mpMeshEntity->GetAnimationStateNum());
		for (int i = 0; i < mpMeshEntity->GetAnimationStateNum(); ++i) {
			cAnimationState *pAnim = mpMeshEntity->GetAnimationState(i);
			cGameEntityAnimation_SaveData &saveAnim = apSaveData->mvAnimations[i];
			saveAnim.mbActive = pAnim->IsActive();
			saveAnim.mbLoop = pAnim->IsLooping();

			saveAnim.mfWeight = pAnim->GetWeight();
			saveAnim.mfFadeStep = pAnim->GetFadeStep();
			saveAnim.mfTimePos = pAnim->GetTimePosition();
			saveAnim.mfSpeed = pAnim->GetSpeed();
		}
	}
}

//-----------------------------------------------------------------------

void iGameEntity::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	// Properties
	kCopyFromVar(apSaveData, msName);
	kCopyFromVar(apSaveData, msFileName);
	kCopyFromVar(apSaveData, mfHealth);
	kCopyFromVar(apSaveData, mfMaxInteractDist);
	kCopyFromVar(apSaveData, mfMaxExamineDist);
	kCopyFromVar(apSaveData, msGameName);
	kCopyFromVar(apSaveData, msDescription);
	kCopyFromVar(apSaveData, mbShowDescritionOnce);
	kCopyFromVar(apSaveData, mbHasBeenExamined);
	kCopyFromVar(apSaveData, mType);

	SetActive(apSaveData->mbActive);

	// Script functions
	cContainerListIterator<cGameEntityScript> scriptIt = apSaveData->mlstCallbackScripts.GetIterator();
	while (scriptIt.HasNext()) {
		cGameEntityScript &script = scriptIt.Next();

		mvCallbackScripts[script.mlNum] = hplNew(cGameEntityScript, ());
		mvCallbackScripts[script.mlNum]->msScriptFunc = script.msScriptFunc;
	}

	// Script variables
	cContainerListIterator<cScriptVar> scriptVar = apSaveData->mlstVars.GetIterator();
	while (scriptVar.HasNext()) {
		cScriptVar &var = scriptVar.Next();
		CreateVar(var.msName, var.mlVal);
	}

	// Bodies
	for (size_t i = 0; i < mvBodies.size(); ++i) {
		apSaveData->mvBodies[i].ToBody(mvBodies[i]);
	}

	// Lights
	if (mbSaveLights) {
		for (size_t i = 0; i < mvLights.size(); ++i) {
			apSaveData->mvLights[i].ToLight(mvLights[i]);
		}
	}

	// Particle Systems
	int lCount = 0;
	for (Common::Array<cParticleSystem3D *>::iterator it = mvParticleSystems.begin();
		 it != mvParticleSystems.end();) {
		cParticleSystem3D *pPS = *it;

		/*if(pPS)
			Log("Loading particle system %d, %s\n",i,pPS->GetName().c_str());
		else
			Log("Loading particle system %d, NULL\n",i);*/

		cEnginePS_SaveData *pSavePS = apSaveData->GetParticleSystem(pPS);
		if (pSavePS) {
			pSavePS->ToPS(pPS);
			++it;
		} else {
			// check if a null was previously saved
			if (apSaveData->mvPS.Size() == mvParticleSystems.size() &&
				apSaveData->mvPS[lCount].msType == "") {
				++it;
			}
			// a particle system has been removed.
			else {
				mpInit->mpGame->GetScene()->GetWorld3D()->DestroyParticleSystem(pPS);
				it = mvParticleSystems.erase(it);
			}
		}

		++lCount;
	}

	// Sounds
	for (Common::Array<cSoundEntity *>::iterator it = mvSoundEntities.begin();
		 it != mvSoundEntities.end();) {
		cSoundEntity *pSound = *it;
		cEngineSound_SaveData *pSaveSound = apSaveData->GetSoundEntity(pSound);
		if (pSaveSound) {
			pSaveSound->ToSound(pSound);
			++it;
		} else {
			mpInit->mpGame->GetScene()->GetWorld3D()->DestroySoundEntity(pSound);
			it = mvSoundEntities.erase(it);
		}
	}

	// Animations
	if (mpMeshEntity) {
		if (mpMeshEntity->GetAnimationStateNum() == (int)apSaveData->mvAnimations.Size()) {
			for (int i = 0; i < mpMeshEntity->GetAnimationStateNum(); ++i) {
				cAnimationState *pAnim = mpMeshEntity->GetAnimationState(i);
				cGameEntityAnimation_SaveData &saveAnim = apSaveData->mvAnimations[i];
				pAnim->SetActive(saveAnim.mbActive);
				pAnim->SetLoop(saveAnim.mbLoop);

				pAnim->SetWeight(saveAnim.mfWeight);
				pAnim->SetFadeStep(saveAnim.mfFadeStep);
				pAnim->SetTimePosition(saveAnim.mfTimePos);
				pAnim->SetSpeed(saveAnim.mfSpeed);
			}
		} else {
			Error("Number of animations in saved entity '%s' of type '%s' does not match!\n",
				  GetName().c_str(), mpMeshEntity->GetName().c_str());
		}
	}
}
//-----------------------------------------------------------------------

void iGameEntity::SetupSaveData(iGameEntity_SaveData *apSaveData) {
	// Collide scripts
	cContainerListIterator<cSaveGame_cGameCollideScript> colIt = apSaveData->mlstCollideCallbacks.GetIterator();
	while (colIt.HasNext()) {
		cSaveGame_cGameCollideScript &savedScript = colIt.Next();
		cGameCollideScript *pCallback = hplNew(cGameCollideScript, ());

		pCallback->mpEntity = mpInit->mpMapHandler->GetGameEntity(savedScript.msEntity);
		if (pCallback->mpEntity == NULL) {
			Warning("Couldn't find entity '%s'\n", savedScript.msEntity.c_str());
			hplDelete(pCallback);
			continue;
		}
		savedScript.SaveTo(pCallback);

		m_mapCollideCallbacks.insert(tGameCollideScriptMap::value_type(savedScript.msEntity, pCallback));
	}

	// Log("Setup save data!\n");
}

//-----------------------------------------------------------------------
