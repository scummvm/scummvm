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

#include "hpl1/penumbra-overture/MapHandler.h"

#include "hpl1/penumbra-overture/FadeHandler.h"
#include "hpl1/penumbra-overture/GameArea.h"
#include "hpl1/penumbra-overture/GameDamageArea.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/GameForceArea.h"
#include "hpl1/penumbra-overture/GameItem.h"
#include "hpl1/penumbra-overture/GameLadder.h"
#include "hpl1/penumbra-overture/GameLink.h"
#include "hpl1/penumbra-overture/GameLiquidArea.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GameStickArea.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapLoadText.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHands.h"
#include "hpl1/penumbra-overture/SaveHandler.h"
#include "hpl1/penumbra-overture/SaveTypes.h"
#include "hpl1/penumbra-overture/TriggerHandler.h"
#include "hpl1/penumbra-overture/Triggers.h"

//////////////////////////////////////////////////////////////////////////
// WORLD CACHE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cWorldCache::cWorldCache(cInit *apInit) {
	mpInit = apInit;
	mpResources = mpInit->mpGame->GetResources();

	mlCount = 0;
}

cWorldCache::~cWorldCache() {
	DecResources();
}

//-----------------------------------------------------------------------

void cWorldCache::AddResources() {
	////////////////////////////
	// Materials
	{
		cResourceBaseIterator it = mpResources->GetMaterialManager()->GetResourceBaseIterator();
		while (it.HasNext()) {
			iResourceBase *pResource = it.Next();
			pResource->IncUserCount();
			mlstMaterials.push_back(pResource);
		}
	}
	////////////////////////////
	// Meshes
	{
		cResourceBaseIterator it = mpResources->GetMeshManager()->GetResourceBaseIterator();
		while (it.HasNext()) {
			iResourceBase *pResource = it.Next();
			pResource->IncUserCount();
			mlstMeshes.push_back(pResource);
		}
	}
	////////////////////////////
	// Animations
	{
		cResourceBaseIterator it = mpResources->GetAnimationManager()->GetResourceBaseIterator();
		while (it.HasNext()) {
			iResourceBase *pResource = it.Next();
			pResource->IncUserCount();
			mlstAnimations.push_back(pResource);
		}
	}

	// mlCount++;
}

//-----------------------------------------------------------------------

void cWorldCache::DecResources() {
	////////////////////////////
	// Materials
	{
		tResourceBaseListIt it = mlstMaterials.begin();
		for (; it != mlstMaterials.end(); ++it) {
			iResourceBase *pResource = *it;
			mpResources->GetMaterialManager()->Destroy(pResource);
		}
		mlstMaterials.clear();
	}
	////////////////////////////
	// Meshes
	{
		tResourceBaseListIt it = mlstMeshes.begin();
		for (; it != mlstMeshes.end(); ++it) {
			iResourceBase *pResource = *it;
			mpResources->GetMeshManager()->Destroy(pResource);
		}
		mlstMeshes.clear();
	}
	////////////////////////////
	// Animations
	{
		tResourceBaseListIt it = mlstAnimations.begin();
		for (; it != mlstAnimations.end(); ++it) {
			iResourceBase *pResource = *it;
			mpResources->GetAnimationManager()->Destroy(pResource);
		}
		mlstAnimations.clear();
	}

	// mlCount--;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SOUND CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMapHandlerSoundCallback::cMapHandlerSoundCallback(cInit *apInit) {
	mpInit = apInit;

	///////////////////////////////////////////
	// Load all sounds that can heard by enemies
	tString sFile = "sounds/EnemySounds.dat";
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (sFile.c_str()));
	if (pXmlDoc->LoadFile() == false) {
		Error("Couldn't load XML file '%s'!\n", sFile.c_str());
		hplDelete(pXmlDoc);
		return;
	}

	// Get the root.
	TiXmlElement *pRootElem = pXmlDoc->RootElement();

	TiXmlElement *pChildElem = pRootElem->FirstChildElement();
	for (; pChildElem != NULL; pChildElem = pChildElem->NextSiblingElement()) {
		tString sName = cString::ToString(pChildElem->Attribute("name"), "");
		mvEnemyHearableSounds.push_back(sName);
	}

	hplDelete(pXmlDoc);
}

//-----------------------------------------------------------------------

void cMapHandlerSoundCallback::OnStart(cSoundEntity *apSoundEntity) {
	///////////////////////////
	// Check if the sound is something to worry bout
	tString sTypeName = apSoundEntity->GetData()->GetName();

	bool bUsed = false;
	for (size_t i = 0; i < mvEnemyHearableSounds.size(); ++i) {
		tString &sName = mvEnemyHearableSounds[i];
		if (sTypeName.size() >= sName.size() && sName == sTypeName.substr(0, sName.size())) {
			bUsed = true;
		}
	}
	if (bUsed == false)
		return;

	// Add a sound trigger
	cGameTrigger_Sound *pSound = hplNew(cGameTrigger_Sound, ());
	pSound->mpSound = apSoundEntity->GetData();
	mpInit->mpTriggerHandler->Add(pSound, eGameTriggerType_Sound,
								  apSoundEntity->GetWorldPosition(),
								  10, 1.0f / 60.0f, apSoundEntity->GetData()->GetMaxDistance());
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LIGHT FLASH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEffectLightFlash::cEffectLightFlash(cInit *apInit, const cVector3f &avPos,
									 float afRadius, const cColor &aColor,
									 float afAddTime, float afNegTime) {
	mpInit = apInit;

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	mpLight = pWorld->CreateLightPoint("Flash");
	mpLight->SetFarAttenuation(afRadius);
	mpLight->SetDiffuseColor(cColor(0, 0));
	mpLight->SetCastShadows(true);
	mpLight->SetIsSaved(false);
	mpLight->SetPosition(avPos);

	mfRadius = afRadius;
	mfNegTime = afNegTime;

	// Log("Fade to %s : %f in time %f\n",aColor.ToString().c_str(),mfRadius,afAddTime);
	mpLight->FadeTo(aColor, mfRadius, afAddTime);

	mbIsDying = false;
	mbDead = false;
}

cEffectLightFlash::~cEffectLightFlash() {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	pWorld->DestroyLight(mpLight);
}

//-----------------------------------------------------------------------

void cEffectLightFlash::Update(float afTimeStep) {
	/*cColor Col = */ mpLight->GetDiffuseColor();
	// Log("Update...\n");
	if (mbIsDying == false) {
		if (mpLight->IsFading() == false) {
			// Log("Fade to black\n");
			mpLight->FadeTo(cColor(0, 0), mfRadius, mfNegTime);
			mbIsDying = true;
		}
	} else {
		if (mpLight->IsFading() == false) {
			// Log("Kill\n");
			mbDead = true;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMapHandler::cMapHandler(cInit *apInit) : iUpdateable("MapHandler") {
	mpInit = apInit;

	mbPreUpdating = false;

	mpScene = apInit->mpGame->GetScene();
	mpResources = apInit->mpGame->GetResources();

	mpWorldCache = hplNew(cWorldCache, (apInit));

	mfGameTime = 0;

	mbDestroyingAll = false;

	Reset();

	mpSoundCallback = hplNew(cMapHandlerSoundCallback, (apInit));
	cSoundEntity::AddGlobalCallback(mpSoundCallback);

	mpMapChangeTexture = mpInit->mpGame->GetResources()->GetTextureManager()->Create2D("other_mapchange.jpg", false);
}

//-----------------------------------------------------------------------

cMapHandler::~cMapHandler(void) {
	if (mpMapChangeTexture)
		mpInit->mpGame->GetResources()->GetTextureManager()->Destroy(mpMapChangeTexture);

	hplDelete(mpSoundCallback);
	hplDelete(mpWorldCache);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

#ifdef DEMO_VERSION
static int glNumOfLoads = 0;
#endif

bool cMapHandler::Load(const tString &asFile, const tString &asStartPos) {
	tString sMapName = cString::ToLowerCase(cString::SetFileExt(asFile, ""));
	cWorld3D *pWorld = NULL;
	cWorld3D *pLastMap = NULL;
	bool bFirstTime = false;
	double fTimeSinceVisit = 0;

#ifdef DEMO_VERSION
	glNumOfLoads++;
	if (glNumOfLoads > 5) {
		CreateMessageBoxW(_W("Demo not playable any more!\n"),
						  _W("The limits of the demo have been exceeded!\n"));
		exit(0);
	}
#endif

	unsigned long lStartTime = mpInit->mpGame->GetSystem()->GetLowLevel()->getTime();

	if (sMapName != msCurrentMap) {
		////////////////////////////////////////
		// DESTRUCTION OF STUFF NOT SAVED //////

		// remove all local timer
		// RemoveLocalTimers();

		// Exit script
		if (mpScene->GetWorld3D()) {
			// OnMapLoad for all entities
			tGameEntityMapIt GIt = m_mapGameEntities.begin();
			for (; GIt != m_mapGameEntities.end(); ++GIt) {
				iGameEntity *pEntity = GIt->second;

				pEntity->OnWorldExit();
			}

			mpScene->GetWorld3D()->GetScript()->Run("OnExit()");

			pLastMap = mpScene->GetWorld3D();
		}

		mpInit->mpMusicHandler->OnWorldExit();
		// Destroy the player objects so they are no saved.
		mpInit->mpPlayer->OnWorldExit();
		mpInit->mpPlayerHands->OnWorldExit();
		mpInit->mpGameMessageHandler->OnWorldExit();

		////////////////////////////////////////
		// SAVING /////////////////////////////

		// Save the map
		if (msCurrentMap != "" && mpScene->GetWorld3D() != NULL) {
			mpInit->mpSaveHandler->SaveData(msCurrentMap);
		}

		msCurrentMap = sMapName;

		////////////////////////////////////////
		// DESTRUCTION OF SAVED STUFF //////////

		mpInit->mpInventory->ClearCallbacks();

		// Reset the rendering.
		mpInit->mpGame->GetGraphics()->GetRenderer3D()->SetAmbientColor(cColor(0, 1));
		mpInit->mpGame->GetGraphics()->GetRenderer3D()->SetSkyBoxActive(false);
		mpInit->mpGame->GetGraphics()->GetRenderer3D()->SetFogActive(false);

		// Remove all current objects
		mpInit->mbDestroyGraphics = false;
		DestroyAll();
		mpInit->mbDestroyGraphics = true;

		// Destroy all sound entities on previous map.
		if (mpScene->GetWorld3D()) {
			mpScene->GetWorld3D()->DestroyAllSoundEntities();
		}

		mpInit->mpPlayer->ClearCollideScripts();

		// Stop all sound
		mpInit->mpGame->GetSound()->GetSoundHandler()->StopAll(eSoundDest_World);
		mpInit->mpGame->GetSound()->Update(1.0f / 60.0f);

		////////////////////////////////////////
		// LOAD THE MAP ////////////////////////

		// Load
		if (mpScene->HasLoadedWorld(asFile)) {
			// Log("-------- Loaded NOT first time! ----------------\n");
			pWorld = mpScene->LoadWorld3D(asFile, true, eWorldLoadFlag_NoGameEntities);
			// eWorldLoadFlag_NoLights |
			// eWorldLoadFlag_NoEntities |

			// eWorldLoadFlag_NoGameEntities);
			if (pWorld)
				mpScene->SetWorld3D(pWorld);
			else {
				Error("Couldn't load map '%s'\n", asFile.c_str());
				return false;
			}
			mpInit->mpSaveHandler->LoadData(msCurrentMap);
			mpInit->mpGame->GetResources()->GetSoundManager()->DestroyUnused(mpInit->mlMaxSoundDataNum);
			mpInit->mpGame->GetResources()->GetParticleManager()->DestroyUnused(mpInit->mlMaxPSDataNum);
		} else {
			// Log("-------- FIRST TIME first time! ----------------\n");
			pWorld = mpScene->LoadWorld3D(asFile, true, 0);
			mpInit->mpGame->GetResources()->GetSoundManager()->DestroyUnused(mpInit->mlMaxSoundDataNum);
			mpInit->mpGame->GetResources()->GetParticleManager()->DestroyUnused(mpInit->mlMaxPSDataNum);

			if (pWorld)
				mpScene->SetWorld3D(pWorld);
			else {
				Error("Couldn't load map '%s'\n", asFile.c_str());
				return false;
			}
			bFirstTime = true;
		}

		///////////////////////////
		// Add to cache
		mpWorldCache->DecResources();
		mpWorldCache->AddResources();

		///////////////////////////
		// Destroy old map
		if (pLastMap) {
			mpScene->DestroyWorld3D(pLastMap);
		}

		fTimeSinceVisit = AddLoadedMap(pWorld);

		pWorld->GetPhysicsWorld()->SetMaxTimeStep(mpInit->mfMaxPhysicsTimeStep);

		// OnMapLoad for all entities
		tGameEntityMapIt GIt = m_mapGameEntities.begin();
		for (; GIt != m_mapGameEntities.end(); ++GIt) {
			iGameEntity *pEntity = GIt->second;

			pEntity->OnWorldLoad();
		}

		// OnMapLoad for player
		mpInit->mpPlayer->OnWorldLoad();

		mpInit->mpPlayerHands->OnWorldLoad();

		mpInit->mpMusicHandler->OnWorldLoad();

		if (bFirstTime) {
			// Set a default name
			msMapGameName = cString::To16Char(cString::SetFileExt(asFile, ""));

			// Init script
			if (pWorld->GetScript())
				pWorld->GetScript()->Run("OnStart()");
		}
	} else {
		if (mpScene->GetWorld3D() != NULL) {
			pWorld = mpScene->GetWorld3D();
		} else {
			Error("No world has been loaded!\n");
			return false;
		}
	}

	mpInit->mpPlayer->SetStartPos(asStartPos);

	// Run global script
	if (mpInit->mpGlobalScript) {
		if (bFirstTime)
			mpInit->mpGlobalScript->Run("OnMapStart()");
		mpInit->mpGlobalScript->Run("OnMapLoad()");
	}
	if (pWorld->GetScript()) {
		pWorld->GetScript()->Run("OnLoad()");
	}

	// After script has been run callback
	tGameEntityMapIt GIt = m_mapGameEntities.begin();
	for (; GIt != m_mapGameEntities.end(); ++GIt) {
		iGameEntity *pEntity = GIt->second;

		pEntity->OnPostLoadScripts();
	}

	// Log("After load and before preupdate:\n");

	unsigned long lTime = mpInit->mpGame->GetSystem()->GetLowLevel()->getTime() - lStartTime;
	Log("Loading map '%s' took: %d ms\n", pWorld->GetFileName().c_str(), lTime);

	PreUpdate(fTimeSinceVisit);

	mpInit->mpGame->ResetLogicTimer();

	// mpInit->mpGame->GetSound()->GetSoundHandler()->ResumeAll(eSoundDest_World | eSoundDest_Gui);

	// Log("After load and preupdate:\n");

	// Set physics accuracy
	pWorld->GetPhysicsWorld()->SetAccuracyLevel(mpInit->mPhysicsAccuracy);

	return true;
}

//-----------------------------------------------------------------------

bool cMapHandler::LoadSimple(const tString &asFile, bool abLoadEntities) {
	tString sMapName = cString::ToLowerCase(cString::SetFileExt(asFile, ""));
	cWorld3D *pWorld = NULL;

	DestroyAll();
	mpInit->mpGame->GetSound()->GetSoundHandler()->StopAll(eSoundDest_World);
	mpInit->mpGame->GetSound()->Update(1.0f / 60.0f);

	mpInit->mpGame->GetGraphics()->GetRenderer3D()->SetAmbientColor(cColor(0, 1));

	cWorld3D *pOldWorld = mpScene->GetWorld3D();

	// Delete all sound entities
	if (pOldWorld) {
		pOldWorld->DestroyAllSoundEntities();
	}

	// Set the current map.
	msCurrentMap = sMapName;

	if (abLoadEntities == false) {
		pWorld = mpScene->LoadWorld3D(asFile, true, eWorldLoadFlag_NoGameEntities);
		// eWorldLoadFlag_NoLights |
		// eWorldLoadFlag_NoEntities |
		// eWorldLoadFlag_NoGameEntities);
		if (pWorld == NULL) {
			Error("Couldn't load map '%s'\n", asFile.c_str());
			return false;
		}
		mpInit->mpSaveHandler->LoadData(msCurrentMap);
		mpInit->mpGame->GetResources()->GetSoundManager()->DestroyUnused(mpInit->mlMaxSoundDataNum);
		mpInit->mpGame->GetResources()->GetParticleManager()->DestroyUnused(mpInit->mlMaxPSDataNum);
	} else {
		pWorld = mpScene->LoadWorld3D(asFile, true, 0);
		mpInit->mpGame->GetResources()->GetSoundManager()->DestroyUnused(mpInit->mlMaxSoundDataNum);
		mpInit->mpGame->GetResources()->GetParticleManager()->DestroyUnused(mpInit->mlMaxPSDataNum);

		if (pWorld == NULL) {
			Error("Couldn't load map '%s'\n", asFile.c_str());
			return false;
		}
	}

	// Destroy old world, if there is any.
	if (pOldWorld) {
		mpScene->DestroyWorld3D(pOldWorld);
	}

	// Set physics update
	if (pWorld->GetPhysicsWorld())
		pWorld->GetPhysicsWorld()->SetMaxTimeStep(mpInit->mfMaxPhysicsTimeStep);

	// OnMapLoad for all entities
	tGameEntityMapIt GIt = m_mapGameEntities.begin();
	for (; GIt != m_mapGameEntities.end(); ++GIt) {
		iGameEntity *pEntity = GIt->second;

		pEntity->OnWorldLoad();
	}

	// OnMapLoad for player
	mpInit->mpPlayer->OnWorldLoad();

	mpInit->mpGame->ResetLogicTimer();

	// Physics set up
	pWorld->GetPhysicsWorld()->SetAccuracyLevel(mpInit->mPhysicsAccuracy);

	return true;
}

//-----------------------------------------------------------------------

void cMapHandler::ChangeMap(const tString &asMap, const tString &asPos, const tString &asStartSound,
							const tString &asStopSound, float afFadeOutTime, float afFadeInTime,
							tString asLoadTextCat, tString asLoadTextEntry) {
	mMapChanger.msNewMap = asMap;
	mMapChanger.msPosName = asPos;
	mMapChanger.msDoneSound = asStopSound;
	mMapChanger.mfFadeInTime = afFadeInTime;
	mMapChanger.mbActive = true;
	mMapChanger.msLoadTextCat = asLoadTextCat;
	mMapChanger.msLoadTextEntry = asLoadTextEntry;

	mpInit->mpFadeHandler->FadeOut(afFadeOutTime);

	if (asStartSound != "")
		mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui(asStartSound, false, 1);

	mpInit->mpPlayer->SetActive(false);
}

//-----------------------------------------------------------------------

void cMapHandler::AddSaveData(cSavedWorld *apSavedWorld) {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	/////////////////////////
	// Properties
	apSavedWorld->msGameName = msMapGameName;

	/////////////////////////
	// Local timers
	for (tGameTimerListIt it = mlstTimers.begin(); it != mlstTimers.end(); ++it) {
		cGameTimer *pTimer = *it;

		if (pTimer->mbGlobal == false) {
			apSavedWorld->mlstTimers.Add(*pTimer);
		}
	}

	/////////////////////////
	// Map script vars
	tScriptVarMap *pVarMap = mpScene->GetLocalVarMap();
	tScriptVarMapIt varIt = pVarMap->begin();
	for (; varIt != pVarMap->end(); ++varIt) {
		cScriptVar &scriptVar = varIt->second;
		scriptVar.mlVal = scriptVar.mlVal;
		scriptVar.msName = scriptVar.msName;
		apSavedWorld->mlstVars.Add(scriptVar);
	}

	//////////////////
	// Engine data

	// Lights
	{
		cLight3DListIterator it = pWorld->GetLightIterator();
		while (it.HasNext()) {
			iLight3D *pLight = it.Next();
			cEngineLight_SaveData saveLight;

			if (pLight->IsSaved() && pLight->GetParent() == NULL && pLight->GetEntityParent() == NULL) {
				saveLight.FromLight(pLight);
				apSavedWorld->mlstLights.Add(saveLight);
			}
		}
	}

	// Particle systems
	{
		cParticleSystem3DIterator it = pWorld->GetParticleSystemIterator();
		while (it.HasNext()) {
			cParticleSystem3D *pPS = it.Next();
			cEnginePS_SaveData savePS;

			if (pPS->IsSaved() && pPS->GetParent() == NULL && pPS->GetEntityParent() == NULL && pPS->IsDying() == false) {
				savePS.FromPS(pPS);
				apSavedWorld->mlstPS.Add(savePS);
			}
		}
	}

	// Beams
	{
		cBeamIterator it = pWorld->GetBeamIterator();
		while (it.HasNext()) {
			cBeam *pBeam = it.Next();
			cEngineBeam_SaveData saveBeam;

			if (pBeam->IsSaved() && pBeam->GetParent() == NULL && pBeam->GetEntityParent() == NULL) {
				saveBeam.FromBeam(pBeam);
				apSavedWorld->mlstBeams.Add(saveBeam);
			}
		}
	}

	// Sounds
	{
		cSoundEntityIterator it = pWorld->GetSoundEntityIterator();
		while (it.HasNext()) {
			cSoundEntity *pSound = it.Next();
			cEngineSound_SaveData saveSound;

			if (pSound->IsSaved() && pSound->GetParent() == NULL && pSound->GetEntityParent() == NULL) {
				// Check if the sound should be saved.
				if (pSound->GetRemoveWhenOver() && pSound->GetData()->GetLoop() == false)
					continue;

				saveSound.FromSound(pSound);
				apSavedWorld->mlstSounds.Add(saveSound);
			}
		}
	}

	// Joints
	{
		cPhysicsJointIterator it = pWorld->GetPhysicsWorld()->GetJointIterator();
		while (it.HasNext()) {
			iPhysicsJoint *pJoint = it.Next();
			cEngineJoint_SaveData saveJoint;

			if (pJoint->IsSaved()) {
				// Check if the joint should be saved.
				if (pJoint->IsBroken())
					continue;

				saveJoint.FromJoint(pJoint);
				apSavedWorld->mlstJoints.Add(saveJoint);
			}
		}
	}

	//////////////////
	// Map handler
	tGameEntityMapIt GIt = m_mapGameEntities.begin();
	for (; GIt != m_mapGameEntities.end(); ++GIt) {
		iGameEntity *pEntity = GIt->second;

		if (pEntity->IsSaved()) {
			// Log("Saving entity: '%s'\n",pEntity->GetName().c_str());

			iGameEntity_SaveData *pSaveData = pEntity->CreateSaveData();
			pEntity->SaveToSaveData(pSaveData);
			apSavedWorld->mlstEntities.Add(pSaveData);
			// Log("Adding save data: %d\n", pSaveData);
		} else {
			// Log("Skipping entity: '%s'\n",pEntity->GetName().c_str());
		}
	}
}

//-----------------------------------------------------------------------

void cMapHandler::LoadSaveData(cSavedWorld *apSavedWorld) {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();
	Common::List<iGameEntity *> lstEntities;

	/////////////////////////
	// Properties
	msMapGameName = apSavedWorld->msGameName;

	////////////////////////////////
	// Script variables
	cContainerListIterator<cScriptVar> scriptVar = apSavedWorld->mlstVars.GetIterator();
	while (scriptVar.HasNext()) {
		cScriptVar &var = scriptVar.Next();
		cScriptVar *pVar = mpScene->CreateLocalVar(var.msName);
		pVar->mlVal = var.mlVal;
	}

	/////////////////////////
	// Local timers
	cContainerListIterator<cGameTimer> timerIt = apSavedWorld->mlstTimers.GetIterator();
	while (timerIt.HasNext()) {
		cGameTimer &savedTimer = timerIt.Next();

		cGameTimer *pTimer = hplNew(cGameTimer, ());
		*pTimer = savedTimer;

		mlstTimers.push_back(pTimer);
	}

	////////////////////////////////
	// Engine data

	{
		///////////////////////
		// Lights
		cContainerListIterator<cEngineLight_SaveData> it = apSavedWorld->mlstLights.GetIterator();
		while (it.HasNext()) {
			cEngineLight_SaveData &saveLight = it.Next();

			iLight3D *pLight = pWorld->GetLight(saveLight.msName);
			if (pLight) {
				saveLight.ToLight(pLight);
			} else {
				// Create light
				// This is most probably a temp light so skip it.
			}
		}
	}

	{
		//////////////////////////
		// Particle systems

		// Go through particle systems and throw away those not present.
		cParticleSystem3DIterator psIt = pWorld->GetParticleSystemIterator();
		while (psIt.HasNext()) {
			cParticleSystem3D *pPS = psIt.Next();
			if (apSavedWorld->PSExists(pPS) == false) {
				pPS->KillInstantly();
			}
		}
		// Set data to map particle systems or create if no system exist.
		cContainerListIterator<cEnginePS_SaveData> it = apSavedWorld->mlstPS.GetIterator();
		while (it.HasNext()) {
			cEnginePS_SaveData &savePS = it.Next();

			cParticleSystem3D *pPS = pWorld->GetParticleSystem(savePS.msName);
			if (pPS) {
				savePS.ToPS(pPS);
			} else {
				pPS = pWorld->CreateParticleSystem(savePS.msName, savePS.msType, savePS.mvSize,
												   savePS.m_mtxTransform);
				savePS.ToPS(pPS);
			}
		}
	}

	//////////////////////////
	// Beams
	{
		// Go through beams and throw away those not present.
		cBeamIterator beamIt = pWorld->GetBeamIterator();
		while (beamIt.HasNext()) {
			cBeam *pBeam = beamIt.Next();
			if (apSavedWorld->BeamExists(pBeam) == false) {
				mpScene->GetWorld3D()->DestroyBeam(pBeam);
			}
		}
		// Set data to map particle systems or create if no system exist.
		cContainerListIterator<cEngineBeam_SaveData> it = apSavedWorld->mlstBeams.GetIterator();
		while (it.HasNext()) {
			cEngineBeam_SaveData &saveBeam = it.Next();

			cBeam *pBeam = pWorld->GetBeam(saveBeam.msName);
			if (pBeam) {
				saveBeam.ToBeam(pBeam);
			} else {
				pBeam = pWorld->CreateBeam(saveBeam.msName);
				saveBeam.ToBeam(pBeam);
			}
		}
	}

	{
		//////////////////////////
		// Sounds

		// Go through sound entities and throw away those not present.
		cSoundEntityIterator soundIt = pWorld->GetSoundEntityIterator();
		while (soundIt.HasNext()) {
			cSoundEntity *pSound = soundIt.Next();
			if (apSavedWorld->SoundExists(pSound) == false) {
				mpScene->GetWorld3D()->DestroySoundEntity(pSound);
			}
		}
		// Set data to map sound entities
		cContainerListIterator<cEngineSound_SaveData> it = apSavedWorld->mlstSounds.GetIterator();
		while (it.HasNext()) {
			cEngineSound_SaveData &saveSound = it.Next();

			cSoundEntity *pSound = pWorld->GetSoundEntity(saveSound.msName);
			if (pSound) {
				saveSound.ToSound(pSound);
			}
		}
	}

	////////////////////////////////
	// Create Entities
	cContainerListIterator<iGameEntity_SaveData *> it = apSavedWorld->mlstEntities.GetIterator();
	while (it.HasNext()) {
		iGameEntity_SaveData *pSaveEntity = it.Next();

		//////////////////////////////////
		// The entity is loaded from file
		if (pSaveEntity->msFileName != "") {
			if (pWorld->CreateEntity(pSaveEntity->msName, pSaveEntity->m_mtxTransform, pSaveEntity->msFileName, true)) {
				iGameEntity *pGameEntity = mpInit->mpMapHandler->GetLatestEntity();
				pGameEntity->LoadFromSaveData(pSaveEntity);

				pGameEntity->SetSaveData(pSaveEntity);
				lstEntities.push_back(pGameEntity);
			}
		}
		//////////////////////////////////
		// The entity is an area.
		else {
			cVector3f vSize = 0;
			tString sType = "";

			// Get type and size
			switch (pSaveEntity->mType) {
			case eGameEntityType_Area:
				vSize = static_cast<cGameArea_SaveData *>(pSaveEntity)->mvSize;
				sType = "script";
				break;
			case eGameEntityType_Link:
				vSize = static_cast<cGameLink_SaveData *>(pSaveEntity)->mvSize;
				sType = "link";
				break;
			case eGameEntityType_StickArea:
				vSize = static_cast<cGameStickArea_SaveData *>(pSaveEntity)->mvSize;
				sType = "stick";
				break;
			case eGameEntityType_SaveArea:
				vSize = static_cast<cGameStickArea_SaveData *>(pSaveEntity)->mvSize;
				sType = "save";
				break;
			case eGameEntityType_Ladder:
				vSize = static_cast<cGameLadder_SaveData *>(pSaveEntity)->mvSize;
				sType = "ladder";
				break;
			case eGameEntityType_DamageArea:
				vSize = static_cast<cGameDamageArea_SaveData *>(pSaveEntity)->mvSize;
				sType = "damage";
				break;
			case eGameEntityType_ForceArea:
				vSize = static_cast<cGameDamageArea_SaveData *>(pSaveEntity)->mvSize;
				sType = "force";
				break;
			case eGameEntityType_LiquidArea:
				vSize = static_cast<cGameLiquidArea_SaveData *>(pSaveEntity)->mvSize;
				sType = "liquid";
				break;
			default:
				break;
			}

			// Load entity
			if (sType != "") {
				iArea3DLoader *pLoader = pWorld->GetResources()->GetArea3DLoader(sType);
				if (pLoader) {
					pLoader->Load(pSaveEntity->msName, vSize, pSaveEntity->m_mtxTransform, pWorld);

					iGameEntity *pGameEntity = mpInit->mpMapHandler->GetLatestEntity();
					pGameEntity->LoadFromSaveData(pSaveEntity);

					pGameEntity->SetSaveData(pSaveEntity);
					lstEntities.push_back(pGameEntity);
				}
			}
		}
	}

	////////////////////////////////
	// Setup Entities
	Common::List<iGameEntity *>::iterator entIt = lstEntities.begin();
	for (; entIt != lstEntities.end(); ++entIt) {
		iGameEntity *pEntity = *entIt;

		pEntity->SetupSaveData(pEntity->GetSaveData());
	}

	{
		//////////////////////////
		// Joints
		//  this need to be done after entity creation so all joints are created.

		// Go through joints and throw away those not present.
		cPhysicsJointIterator jointIt = pPhysicsWorld->GetJointIterator();
		while (jointIt.HasNext()) {
			iPhysicsJoint *pJoint = jointIt.Next();
			if (apSavedWorld->JointExists(pJoint) == false) {
				pPhysicsWorld->DestroyJoint(pJoint);
			}
		}
		// Set data to map Joint entities
		cContainerListIterator<cEngineJoint_SaveData> it2 = apSavedWorld->mlstJoints.GetIterator();
		while (it2.HasNext()) {
			cEngineJoint_SaveData &saveJoint = it2.Next();

			iPhysicsJoint *pJoint = pPhysicsWorld->GetJoint(saveJoint.msName);
			if (pJoint) {
				saveJoint.ToJoint(pJoint);
			}
		}
	}
}
//-----------------------------------------------------------------------

void cMapHandler::OnStart() {
}

//-----------------------------------------------------------------------

void cMapHandler::OnWorldLoad() {
}

//-----------------------------------------------------------------------

void cMapHandler::OnDraw() {
	tGameEntityMapIt GIt = m_mapGameEntities.begin();
	for (; GIt != m_mapGameEntities.end(); ++GIt) {
		iGameEntity *pEntity = GIt->second;

		pEntity->OnDraw();
	}
}

void cMapHandler::RenderItemEffect() {
	if (mpInit->mbFlashItems == false)
		return;

	// Check if any item needs the effect.
	bool bFound = false;
	tGameItemListIt it = mlstGameItems.begin();
	for (; it != mlstGameItems.end(); ++it) {
		cGameItem *pItem = *it;
		if (pItem->IsActive() && pItem->GetFlashAlpha() > 0) {
			bFound = true;
			break;
		}
	}
	if (bFound == false)
		return;

	cCamera3D *pCam = static_cast<cCamera3D *>(mpScene->GetCamera());
	iLowLevelGraphics *pLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();

	pLowGfx->SetDepthTestActive(true);
	pLowGfx->SetDepthWriteActive(false);

	pLowGfx->SetBlendActive(true);
	pLowGfx->SetBlendFunc(eBlendFunc_One, eBlendFunc_One);

	pLowGfx->SetActiveTextureUnit(0);
	pLowGfx->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Constant);
	pLowGfx->SetTextureEnv(eTextureParam_ColorSource0, eTextureSource_Texture);
	pLowGfx->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);

	it = mlstGameItems.begin();
	for (; it != mlstGameItems.end(); ++it) {
		cGameItem *pItem = *it;

		if (pItem->IsActive() == false)
			continue;
		if (pItem->GetFlashAlpha() <= 0)
			continue;

		cMeshEntity *pMeshEntity = pItem->GetMeshEntity();

		pLowGfx->SetTextureConstantColor(cColor(pItem->GetFlashAlpha(), 0));

		pLowGfx->SetMatrix(eMatrix_ModelView, cMath::MatrixMul(pCam->GetViewMatrix(),
															   pMeshEntity->GetWorldMatrix()));
		for (int i = 0; i < pMeshEntity->GetMesh()->GetSubMeshNum(); i++) {
			cSubMeshEntity *pSubEntity = pMeshEntity->GetSubMeshEntity(i);
			iVertexBuffer *pVtxBuffer = pSubEntity->GetVertexBuffer();
			iMaterial *pMaterial = pSubEntity->GetMaterial();

			iGpuProgram *gpuProgram = pMaterial->getGpuProgram(eMaterialRenderType_Z, 0, NULL);
			if (gpuProgram) {
				gpuProgram->Bind();
				gpuProgram->SetMatrixf("worldViewProj",
									   eGpuProgramMatrix_ViewProjection,
									   eGpuProgramMatrixOp_Identity);
				gpuProgram->SetColor3f("ambientColor", pItem->GetFlashAlpha());
			}

			pLowGfx->SetTexture(0, pMaterial->GetTexture(eMaterialTexture_Diffuse));

			pVtxBuffer->Bind();
			pVtxBuffer->Draw();
			pVtxBuffer->UnBind();

			if (gpuProgram)
				gpuProgram->UnBind();
		}
	}

	pLowGfx->SetTexture(0, NULL);

	pLowGfx->SetActiveTextureUnit(0);
	pLowGfx->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Previous);
	pLowGfx->SetTextureEnv(eTextureParam_ColorSource0, eTextureSource_Texture);

	pLowGfx->SetBlendActive(false);
	pLowGfx->SetDepthTestActive(true);
	pLowGfx->SetDepthWriteActive(true);
}
//-----------------------------------------------------------------------

void cMapHandler::OnPostSceneDraw() {
	////////////////////////////////////
	// Draw effect on items
	RenderItemEffect();

	cCamera3D *pCam = static_cast<cCamera3D *>(mpScene->GetCamera());
	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetMatrix(eMatrix_ModelView, pCam->GetViewMatrix());

	// mpScene->GetWorld3D()->GetPhysicsWorld()->RenderDebugGeometry(
	//	mpInit->mpGame->GetGraphics()->GetLowLevel(),cColor(1,0.5f,1));

	/*mpInit->mpGame->GetGraphics()->GetLowLevel()->SetDepthTestActive(false);
	cParticleSystem3DIterator PIt = mpScene->GetWorld3D()->GetParticleSystemIterator();
	while(PIt.HasNext())
	{
		cParticleSystem3D*pPS = PIt.Next();

		//if(pPS->GetName() == "tripod01_tripodflame")
		//{
		//	static bool bStatic = true;
		//	if(bStatic){//pPS->SetTransformUpdated(true);
		//		Log("Once!!!\n");
		//	}
		//	bStatic =false;

			//pPS->SetPosition(pPS->GetLocalPosition());
			mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawSphere(pPS->GetWorldPosition(),
																0.3f,cColor(1,0,1));

			mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(
												pPS->GetEmitter(0)->GetBoundingVolume()->GetMax(),
												pPS->GetEmitter(0)->GetBoundingVolume()->GetMin(),
												cColor(0,1,1));

	}
	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetDepthTestActive(true);*/

	////////////////////////////////////
	// Let Entities post draw
	tGameEntityMapIt GIt = m_mapGameEntities.begin();
	for (; GIt != m_mapGameEntities.end(); ++GIt) {
		iGameEntity *pEntity = GIt->second;

		pEntity->OnPostSceneDraw();
	}

	return;
	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetTexture(0, NULL);
	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetBlendActive(false);

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetDepthTestActive(false);

	/*cVector3f vMin = */ mpScene->GetWorld3D()->GetPhysicsWorld()->GetWorldSizeMin();
	/*cVector3f vMax = */ mpScene->GetWorld3D()->GetPhysicsWorld()->GetWorldSizeMax();

	// mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(vMax,vMin,cColor(1,0,1));

	// mpScene->GetWorld3D()->DrawMeshBoundingBoxes(cColor(1,0.5f,1),false);
	// mpScene->GetWorld3D()->GetPhysicsWorld()->RenderDebugGeometry(
	//						mpInit->mpGame->GetGraphics()->GetLowLevel(),cColor(1,0.5f,1));
	// mpScene->GetWorld3D()->GetPhysicsWorld()->SetSaveContactPoints(true);
	// mpScene->GetWorld3D()->GetPhysicsWorld()->RenderContactPoints(mpInit->mpGame->GetGraphics()->GetLowLevel(),cColor(1,0.5f,1),
	//											cColor(1,1,0.5f));

	cLight3DListIterator lightIt = mpScene->GetWorld3D()->GetLightIterator();
	while (lightIt.HasNext()) {
		iLight3D *pLight = lightIt.Next();

		if (pLight->IsVisible() && pLight->IsActive() && pLight->GetLightType() == eLight3DType_Spot) {
			cBoundingVolume *pBV = pLight->GetBoundingVolume();
			mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(
				pBV->GetMax(), pBV->GetMin(), cColor(1, 0, 1));
		}
	}

	/*mpInit->mpGame->GetGraphics()->GetLowLevel()->SetDepthTestActive(false);
	cBillboardIterator billIt = mpScene->GetWorld3D()->GetBillboardIterator();
	while(billIt.HasNext())
	{
		cBillboard *pBillboard = billIt.Next();

		mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(
								pBillboard->GetBoundingVolume()->GetMax(),
								pBillboard->GetBoundingVolume()->GetMin(),cColor(1,0,1));
	}*/

	GIt = m_mapGameEntities.begin();
	for (; GIt != m_mapGameEntities.end(); ++GIt) {
		// iGameEntity *pEntity = GIt->second;

		// if(pEntity->GetBodyNum()<=0) continue;

		// mpInit->mpGame->GetGraphics()->GetLowLevel()->SetDepthTestActive(false);

		// mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(
		//						pEntity->GetMeshEntity()->GetBoundingVolume()->GetMax(),
		//						pEntity->GetMeshEntity()->GetBoundingVolume()->GetMin(),cColor(1,0,1));

		/*for(int i=0; i < pEntity->GetBodyNum(); ++i)
		{
			iPhysicsBody* pBody = pEntity->GetBody(i);
			if(pBody){
				cBoundingVolume *pBV = pBody->GetBV();
				mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(pBV->GetMax(),pBV->GetMin(),
																cColor(1,0.5f,1));
				//pBody->RenderDebugGeometry(mpInit->mpGame->GetGraphics()->GetLowLevel(),cColor(1,0.5f,1));
			}
		}*/
	}

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetDepthTestActive(true);
}

//-----------------------------------------------------------------------

void cMapHandler::Update(float afTimeStep) {
	mfGameTime += (double)afTimeStep;

	// iLowLevelSystem *pLowLevelSystem =mpInit->mpGame->GetSystem()->GetLowLevel();

	// LogUpdate(" Flashes!\n");

	////////////////////////////
	// Update light flashes
	tEffectLightFlashListIt FlashIt = mlstLightFlashes.begin();
	for (; FlashIt != mlstLightFlashes.end();) {
		cEffectLightFlash *pFlash = *FlashIt;

		pFlash->Update(afTimeStep);
		if (pFlash->IsDead()) {
			hplDelete(pFlash);
			FlashIt = mlstLightFlashes.erase(FlashIt);
		} else {
			++FlashIt;
		}
	}

	// LogUpdate(" Timers\n");
	////////////////////////////
	// Update timers
	UpdateTimers(afTimeStep);

	// LogUpdate(" Entities!\n");
	////////////////////////////
	// Update entities.
	tGameEntityMapIt GIt = m_mapGameEntities.begin();
	for (; GIt != m_mapGameEntities.end();) {
		iGameEntity *pEntity = GIt->second;

		// unsigned int lTime = pLowLevelSystem->GetTime();
		// LogUpdate("  %s\n",pEntity->GetName().c_str());

		if (pEntity->IsActive()) {
			pEntity->OnUpdate(afTimeStep);
		}

		if (pEntity->GetDestroyMe() || pEntity->GetBreakMe()) {
			// LogUpdate("    destroying\n");
			if (pEntity->GetBreakMe())
				pEntity->BreakAction();

			m_mapGameEntities.erase(GIt++);
			hplDelete(pEntity);
			// LogUpdate("    done destroying\n");
		} else {
			////LogUpdate("  updating %s took %d ms\n",	pEntity->GetName().c_str(),
			//										pLowLevelSystem->GetTime() - lTime);
			++GIt;
		}
	}

	////////////////////////////
	// Check for map change
	// LogUpdate("  Map change\n");
	if (mMapChanger.mbActive && mpInit->mpFadeHandler->IsActive() == false) {
		mMapChanger.mbActive = false;

		// Log("---------- CHANGING TO MAP: '%s' ------------------\n",mMapChanger.msNewMap.c_str());

		// Make sure that the graphics are not destroyed at the end of the map so
		// That the next map doesn't have to reload em.
		mpInit->mbDestroyGraphics = false;

		// Draw Loading image
		if (mMapChanger.msLoadTextCat != "") {
			mpInit->mpMapLoadText->SetText(mMapChanger.msLoadTextCat, mMapChanger.msLoadTextEntry);
			mpInit->mpMapLoadText->SetActive(true);
		} else {
			mpInit->mpGraphicsHelper->DrawLoadingScreen("other_loading.jpg");
		}

		// Fade in here so that script will over ride it.
		mpInit->mpFadeHandler->FadeIn(mMapChanger.mfFadeInTime);

		Load(mMapChanger.msNewMap, mMapChanger.msPosName);

		mpInit->mbDestroyGraphics = true;

		if (mMapChanger.msDoneSound != "")
			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui(mMapChanger.msDoneSound, false, 1);

		mpInit->mpPlayer->SetActive(true);

		// Log("Map change over...\n");
	}
}

//-----------------------------------------------------------------------

void cMapHandler::Reset() {
#ifdef DEMO_VERSION
	glNumOfLoads = 0;
#endif

	mMapChanger.mbActive = false;
	msCurrentMap = "";

	if (mpInit->mbResetCache)
		mpWorldCache->DecResources();

	mvLoadedMaps.clear();

	// Timers
	STLDeleteAll(mlstTimers);

	DestroyAll();

	// World3D
	if (mpScene->GetWorld3D())
		mpScene->DestroyWorld3D(mpScene->GetWorld3D());
	mpScene->SetWorld3D(NULL);

	// Make sure no occulssion queries are left.
	mpInit->mpGame->GetGraphics()->GetRenderer3D()->GetRenderList()->Clear();
}

//-----------------------------------------------------------------------

void cMapHandler::DestroyAll() {
	mbDestroyingAll = true;
	RemoveLocalTimers();

	// Game entities
	STLMapDeleteAll(m_mapGameEntities);
	m_mapGameEntities.clear();
	mlstGameEnemies.clear();
	mlstGameItems.clear();

	// Light flashes
	STLDeleteAll(mlstLightFlashes);
	mlstLightFlashes.clear();
	mbDestroyingAll = false;
}

//-----------------------------------------------------------------------

cGameTimer *cMapHandler::CreateTimer(const tString &asName, float afTime, const tString &asCallback, bool abGlobal) {
	cGameTimer *pTimer = hplNew(cGameTimer, ());

	pTimer->msName = asName;
	pTimer->msCallback = asCallback;
	pTimer->mfTime = afTime;
	pTimer->mbGlobal = abGlobal;

	mlstTimers.push_back(pTimer);

	return pTimer;
}

cGameTimer *cMapHandler::GetTimer(const tString &asName) {
	return (cGameTimer *)STLFindByName(mlstTimers, asName);
}

//-----------------------------------------------------------------------

void cMapHandler::AddLightFlash(const cVector3f &avPos, float afRadius, const cColor &aColor, float afAddTime, float afNegTime) {
	cEffectLightFlash *pFlash = hplNew(cEffectLightFlash, (mpInit, avPos, afRadius, aColor, afAddTime, afNegTime));

	mlstLightFlashes.push_back(pFlash);
}

//-----------------------------------------------------------------------

void cMapHandler::AddStickArea(cGameStickArea *apArea) {
	mlstGameStickAreas.push_back(apArea);
}

//-----------------------------------------------------------------------

void cMapHandler::RemoveStickArea(cGameStickArea *apArea) {
	tGameStickAreaListIt it = mlstGameStickAreas.begin();
	for (; it != mlstGameStickAreas.end(); ++it) {
		if (*it == apArea) {
			mlstGameStickAreas.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------

cGameStickArea *cMapHandler::GetBodyStickArea(iPhysicsBody *apBody) {
	tGameStickAreaListIt it = mlstGameStickAreas.begin();
	for (; it != mlstGameStickAreas.end(); ++it) {
		cGameStickArea *pArea = *it;

		if (pArea->GetAttachedBody() == apBody) {
			return pArea;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------

void cMapHandler::AddGameEnemy(iGameEnemy *apEnemy) {
	mlstGameEnemies.push_back(apEnemy);
}

tGameEnemyIterator cMapHandler::GetGameEnemyIterator() {
	return tGameEnemyIterator(&mlstGameEnemies);
}

//-----------------------------------------------------------------------

void cMapHandler::AddGameItem(cGameItem *apItem) {
	mlstGameItems.push_back(apItem);
}

void cMapHandler::RemoveGameItem(cGameItem *apItem) {
	tGameItemListIt it = mlstGameItems.begin();
	for (; it != mlstGameItems.end(); ++it) {
		cGameItem *pItem = *it;
		if (pItem == apItem) {
			mlstGameItems.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------

void cMapHandler::AddGameEntity(iGameEntity *apEntity) {
	iGameEntity *pSameNameEntity = GetGameEntity(apEntity->GetName(), false);
	if (pSameNameEntity) {
		Warning("Entity '%s' with file '%s' has name already taken by '%s'!\n",
				apEntity->GetName().c_str(),
				apEntity->GetFileName().c_str(),
				pSameNameEntity->GetFileName().c_str());
	}

	mpLatestEntity = apEntity;
	m_mapGameEntities.insert(tGameEntityMap::value_type(apEntity->GetName(), apEntity));
}

void cMapHandler::RemoveGameEntity(iGameEntity *apEntity) {
	m_mapGameEntities.erase(apEntity->GetName());

	// If the entity is an enemy remove that aswel
	{
		tGameEnemyListIt it = mlstGameEnemies.begin();
		for (; it != mlstGameEnemies.end(); ++it) {
			iGameEntity *pEnemy = *it;
			if (pEnemy == apEntity) {
				mlstGameEnemies.erase(it);
				break;
			}
		}
	}

	hplDelete(apEntity);
}

iGameEntity *cMapHandler::GetGameEntity(const tString &asName, bool abErrorMessage) {
	tGameEntityMapIt it = m_mapGameEntities.find(asName);
	if (it == m_mapGameEntities.end()) {
		if (abErrorMessage)
			Error("Couldn't find game entity '%s'\n", asName.c_str());
		return NULL;
	}

	return it->second;
}

//-----------------------------------------------------------------------

tGameEntityIterator cMapHandler::GetGameEntityIterator() {
	return tGameEntityIterator(&m_mapGameEntities);
}

//-----------------------------------------------------------------------

iGameEntity *cMapHandler::GetLatestEntity() {
	return mpLatestEntity;
}

//-----------------------------------------------------------------------

void cMapHandler::SaveToGlobal(cMapHandler_GlobalSave *apSave) {
	cMapHandler_GlobalSave *pData = apSave;

	/////////////////
	// Variables
	kSaveData_SaveTo(mfGameTime);
	kSaveData_SaveTo(msCurrentMap);

	/////////////////
	// Containers

	// Loaded maps
	pData->mvLoadedMaps.Resize(mvLoadedMaps.size());
	for (size_t i = 0; i < mvLoadedMaps.size(); ++i) {
		pData->mvLoadedMaps[i].mfTime = mvLoadedMaps[i].mfTime;
		pData->mvLoadedMaps[i].msName = mvLoadedMaps[i].msName;
	}

	// Timers
	tGameTimerListIt it = mlstTimers.begin();
	for (; it != mlstTimers.end(); ++it) {
		cGameTimer *pTimer = *it;
		cMapHandlerTimer_GlobalSave timerSave;

		if (pTimer->mbGlobal) {
			timerSave.mfTime = pTimer->mfTime;
			timerSave.msName = pTimer->msName;
			timerSave.msCallback = pTimer->msCallback;

			timerSave.mbGlobal = pTimer->mbGlobal;

			timerSave.mbDeleteMe = pTimer->mbDeleteMe;
			timerSave.mbPaused = pTimer->mbPaused;

			pData->mlstTimers.Add(timerSave);
		}
	}
}

void cMapHandler::LoadFromGlobal(cMapHandler_GlobalSave *apSave) {
	cMapHandler_GlobalSave *pData = apSave;

	/////////////////
	// Variables
	kSaveData_LoadFrom(mfGameTime);
	// Should not be loadead so that current map is ""
	// kSaveData_LoadFrom(msCurrentMap);

	/////////////////
	// Containers

	// Loaded maps
	mvLoadedMaps.resize(pData->mvLoadedMaps.Size());
	for (size_t i = 0; i < mvLoadedMaps.size(); ++i) {
		mvLoadedMaps[i].mfTime = pData->mvLoadedMaps[i].mfTime;
		mvLoadedMaps[i].msName = pData->mvLoadedMaps[i].msName;
	}

	// Timers
	cContainerListIterator<cMapHandlerTimer_GlobalSave> it = pData->mlstTimers.GetIterator();
	while (it.HasNext()) {
		cGameTimer *pTimer = hplNew(cGameTimer, ());
		cMapHandlerTimer_GlobalSave &timerSave = it.Next();

		pTimer->mfTime = timerSave.mfTime;
		pTimer->msName = timerSave.msName;
		pTimer->msCallback = timerSave.msCallback;

		pTimer->mbGlobal = timerSave.mbGlobal;

		pTimer->mbDeleteMe = timerSave.mbDeleteMe;
		pTimer->mbPaused = timerSave.mbPaused;

		mlstTimers.push_back(pTimer);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

void cMapHandler::PrintSoundsPlaying() {
	Log("Sounds: ");
	tSoundEntryList *pEntryList = mpInit->mpGame->GetSound()->GetSoundHandler()->GetWorldEntryList();
	for (tSoundEntryListIt it = pEntryList->begin(); it != pEntryList->end(); ++it) {
		iSoundChannel *pSound = it->mpSound;

		Log("'%s', ", pSound->GetData()->GetName().c_str());
	}

	Log("\n");
}
//-----------------------------------------------------------------------

void cMapHandler::UpdateTimers(float afTimeStep) {
	for (tGameTimerListIt it = mlstTimers.begin(); it != mlstTimers.end();) {
		cGameTimer *pTimer = *it;

		if (pTimer->mbDeleteMe) {
			it = mlstTimers.erase(it);
			hplDelete(pTimer);
		} else {
			if (pTimer->mbPaused == false)
				pTimer->mfTime -= afTimeStep;

			if (pTimer->mfTime <= 0) {
				tString sCommand = pTimer->msCallback + "(\"" + pTimer->msName + "\")";
				mpInit->RunScriptCommand(sCommand);

				it = mlstTimers.erase(it);
				hplDelete(pTimer);
			} else {
				++it;
			}
		}
	}
}

void cMapHandler::RemoveLocalTimers() {
	for (tGameTimerListIt it = mlstTimers.begin(); it != mlstTimers.end();) {
		cGameTimer *pTimer = *it;

		if (pTimer->mbGlobal == false) {
			it = mlstTimers.erase(it);
			hplDelete(pTimer);
		} else {
			++it;
		}
	}
}

//-----------------------------------------------------------------------

void cMapHandler::PreUpdate(double afTimeSinceVisit) {
	cWorld3D *pWorld = mpScene->GetWorld3D();
	iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();

	mbPreUpdating = true;

	// unsigned long lStart = mpInit->mpGame->GetSystem()->GetLowLevel()->getTime();

	// Enable all physic bodies
	cPhysicsBodyIterator bodyIt = pPhysicsWorld->GetBodyIterator();
	while (bodyIt.HasNext()) {
		iPhysicsBody *pBody = bodyIt.Next();
		pBody->SetEnabled(true);
	}

	//////////////////////////////////////
	// Pre update game stuff
	if (afTimeSinceVisit == 0) {
		pWorld->PreUpdate(2.0f, mpInit->mpGame->GetStepSize());
	} else {
		float fTime = (float)afTimeSinceVisit;
		if (fTime > 3)
			fTime = 3;
		float fStepSize = mpInit->mpGame->GetStepSize();

		mpInit->mpGame->GetSound()->GetSoundHandler()->SetSilent(true);

		while (fTime > 0) {
			// if(pWorld->GetPhysicsWorld()) pWorld->GetPhysicsWorld()->Update(fStepSize);
			// pWorld->UpdateParticles(fStepSize);
			pWorld->Update(fStepSize);

			tGameEnemyListIt it = mlstGameEnemies.begin();
			for (; it != mlstGameEnemies.end(); ++it) {
				iGameEnemy *pEnemy = *it;
				pEnemy->Update(fStepSize);
			}

			fTime -= fStepSize;
		}

		mpInit->mpGame->GetSound()->GetSoundHandler()->SetSilent(false);
	}

	// unsigned long lTime = mpInit->mpGame->GetSystem()->GetLowLevel()->getTime() - lStart;

	// Log("PREUPDATE time: %d\n",lTime);

	mbPreUpdating = false;
}

//-----------------------------------------------------------------------

double cMapHandler::AddLoadedMap(cWorld3D *apWorld) {
	// Check if map is loaded
	for (size_t i = 0; i < mvLoadedMaps.size(); ++i) {
		if (mvLoadedMaps[i].msName == apWorld->GetName()) {
			double fTime = mvLoadedMaps[i].mfTime;
			mvLoadedMaps[i].mfTime = GetGameTime();
			return GetGameTime() - fTime;
		}
	}

	cLoadedMap LoadedMap;
	LoadedMap.msName = apWorld->GetName();
	LoadedMap.mfTime = GetGameTime();

	mvLoadedMaps.push_back(LoadedMap);
	return 0;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
