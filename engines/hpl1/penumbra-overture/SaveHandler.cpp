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

#include "hpl1/penumbra-overture/SaveHandler.h"

#include "common/savefile.h"
#include "hpl1/debug.h"
#include "hpl1/hpl1.h"
#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/RadioHandler.h"
#include "hpl1/string.h"

//////////////////////////////////////////////////////////////////////////
// SAVED WORLD
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSavedWorld::~cSavedWorld() {
	Reset();
}

void cSavedWorld::Reset() {
	////////////////////////////////
	// Clear and deleter entities
	cContainerListIterator<iGameEntity_SaveData *> it = mlstEntities.GetIterator();
	while (it.HasNext()) {
		iGameEntity_SaveData *pSaveEntity = it.Next();
		// Log(" delete %d\n", pSaveEntity);
		hplDelete(pSaveEntity);
	}
	mlstEntities.Clear();

	mlstVars.Clear();

	// Inventory variables
	mlstUseCallbacks.Clear();
	mlstPickupCallbacks.Clear();
	mlstCombineCallbacks.Clear();

	// Player collider callbacks
	mlstCollideCallbacks.Clear();

	mlstLights.Clear();
	mlstPS.Clear();
	mlstSounds.Clear();
	mlstJoints.Clear();
	mlstBeams.Clear();

	mlstTimers.Clear();
}

//-----------------------------------------------------------------------

bool cSavedWorld::PSExists(cParticleSystem3D *apPS) {
	cContainerListIterator<cEnginePS_SaveData> it = mlstPS.GetIterator();
	while (it.HasNext()) {
		cEnginePS_SaveData &savePS = it.Next();

		if (savePS.msName == apPS->GetName())
			return true;
	}

	return false;
}
//-----------------------------------------------------------------------

bool cSavedWorld::BeamExists(cBeam *apBeam) {
	cContainerListIterator<cEngineBeam_SaveData> it = mlstBeams.GetIterator();
	while (it.HasNext()) {
		cEngineBeam_SaveData &saveBeam = it.Next();

		if (saveBeam.msName == apBeam->GetName())
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cSavedWorld::SoundExists(cSoundEntity *apSound) {
	cContainerListIterator<cEngineSound_SaveData> it = mlstSounds.GetIterator();
	while (it.HasNext()) {
		cEngineSound_SaveData &saveSound = it.Next();

		if (saveSound.msName == apSound->GetName())
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cSavedWorld::JointExists(iPhysicsJoint *apJoint) {
	cContainerListIterator<cEngineJoint_SaveData> it = mlstJoints.GetIterator();
	while (it.HasNext()) {
		cEngineJoint_SaveData &saveJoint = it.Next();

		if (saveJoint.msName == apJoint->GetName())
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVED GAME
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSavedGame::~cSavedGame() {
	Reset();
}

//-----------------------------------------------------------------------

void cSavedGame::Reset() {
	ResetWorlds();
	ResetGlobalData();
}

void cSavedGame::ResetWorlds() {
	cContainerListIterator<cSavedWorld *> it = mlstWorlds.GetIterator();
	while (it.HasNext()) {
		cSavedWorld *pWorld = it.Next();

		// Log("delete world %d, '%s'\n", pWorld, pWorld->msName.c_str());
		hplDelete(pWorld);
	}
	mlstWorlds.Clear();
}

void cSavedGame::ResetGlobalData() {
	mMapHandler.mvLoadedMaps.Clear();
	mMapHandler.mlstTimers.Clear();

	mInventory.mlstItems.Clear();
	mInventory.mlstSlots.Clear();
	mInventory.mvEquipSlots.Clear();

	mNotebook.mlstNotes.Clear();
	mNotebook.mlstTasks.Clear();

	mlstScriptVars.Clear();

	mvSceneLoadedMap.Clear();
}

cSavedWorld *cSavedGame::GetSavedWorld(const tString &asName) {
	tString sLowName = cString::ToLowerCase(asName);

	// See if world allready exists
	cContainerListIterator<cSavedWorld *> it = mlstWorlds.GetIterator();
	while (it.HasNext()) {
		cSavedWorld *pWorld = it.Next();
		if (cString::ToLowerCase(pWorld->msName) == sLowName) {
			return pWorld;
		}
	}

	// Create newer world
	cSavedWorld *pWorld = hplNew(cSavedWorld, ());
	pWorld->msName = asName;
	mlstWorlds.Add(pWorld);

	// Log("Creating saved world %d, %s\n",pWorld,asName.c_str());

	return pWorld;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSaveHandler::cSaveHandler(cInit *apInit) : iUpdateable("SaveHandler") {
	mpInit = apInit;

	mpSavedGame = hplNew(cSavedGame, ());

	Reset();
}

//-----------------------------------------------------------------------

cSaveHandler::~cSaveHandler(void) {
	hplDelete(mpSavedGame);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cSaveHandler::SaveData(const tString &asName) {
	cSavedWorld *pSavedWorld = mpSavedGame->GetSavedWorld(asName);

	pSavedWorld->Reset();

	///////////////////////
	// Map properties
	cRenderer3D *pRenderer = mpInit->mpGame->GetGraphics()->GetRenderer3D();

	pSavedWorld->mAmbientColor = pRenderer->GetAmbientColor();

	pSavedWorld->mbFogActive = pRenderer->GetFogActive();
	pSavedWorld->mfFogStartDist = pRenderer->GetFogStart();
	pSavedWorld->mfFogEndDist = pRenderer->GetFogEnd();
	pSavedWorld->mFogColor = pRenderer->GetFogColor();
	pSavedWorld->mbFogCulling = pRenderer->GetFogCulling();

	pSavedWorld->mbSkyboxActive = pRenderer->GetSkyBoxActive();
	pSavedWorld->mSkyboxColor = pRenderer->GetSkyBoxColor();

	if (pRenderer->GetSkyBox())
		pSavedWorld->msSkyboxFile = pRenderer->GetSkyBox()->GetName();
	else
		pSavedWorld->msSkyboxFile = "";

	///////////////////////
	// Inventory callbacks
	cInventory *pInventory = mpInit->mpInventory;

	//////////////////////
	// Use callbacks
	{
		pSavedWorld->mlstUseCallbacks.Clear();
		tInventoryUseCallbackMapIt it = pInventory->m_mapUseCallbacks.begin();
		for (; it != pInventory->m_mapUseCallbacks.end(); ++it) {
			pSavedWorld->mlstUseCallbacks.Add(*(it->second));
		}
	}

	//////////////////////
	// Pickup callbacks
	{
		pSavedWorld->mlstPickupCallbacks.Clear();
		tInventoryPickupCallbackMapIt it = pInventory->m_mapPickupCallbacks.begin();
		for (; it != pInventory->m_mapPickupCallbacks.end(); ++it) {
			pSavedWorld->mlstPickupCallbacks.Add(*(it->second));
		}
	}

	//////////////////////
	// Combine callbacks
	{
		pSavedWorld->mlstCombineCallbacks.Clear();
		tInventoryCombineCallbackListIt it = pInventory->mlstCombineCallbacks.begin();
		for (; it != pInventory->mlstCombineCallbacks.end(); ++it) {
			pSavedWorld->mlstCombineCallbacks.Add(*(*it));
		}
	}

	//////////////////////
	// Player world data
	mpInit->mpPlayer->AddSaveData(pSavedWorld);

	/////////////////////////
	// Entities on map
	mpInit->mpMapHandler->AddSaveData(pSavedWorld);

	// cSerializeClass::SetLog(true);
	// cSerializeClass::SaveToFile(pSavedWorld,"TestSave.txt","SaveGame");
}

//-----------------------------------------------------------------------

void cSaveHandler::LoadData(const tString &asName) {
	/*cWorld3D *pWorld = */ mpInit->mpGame->GetScene()->GetWorld3D();
	cSavedWorld *pSavedWorld = mpSavedGame->GetSavedWorld(asName);

	///////////////////////
	// Map properties
	cRenderer3D *pRenderer = mpInit->mpGame->GetGraphics()->GetRenderer3D();
	cResources *pResources = mpInit->mpGame->GetResources();

	pRenderer->SetAmbientColor(pSavedWorld->mAmbientColor);

	pRenderer->SetFogActive(pSavedWorld->mbFogActive);
	pRenderer->SetFogStart(pSavedWorld->mfFogStartDist);
	pRenderer->SetFogEnd(pSavedWorld->mfFogEndDist);
	pRenderer->SetFogColor(pSavedWorld->mFogColor);
	pRenderer->SetFogCulling(pSavedWorld->mbFogCulling);

	pRenderer->SetSkyBoxActive(pSavedWorld->mbSkyboxActive);
	pRenderer->SetSkyBoxColor(pSavedWorld->mSkyboxColor);

	if (pSavedWorld->msSkyboxFile != "") {
		iTexture *pTex = pResources->GetTextureManager()->CreateCubeMap(pSavedWorld->msSkyboxFile, false);
		pRenderer->SetSkyBox(pTex, true);
	} else {
		pRenderer->SetSkyBox(NULL, false);
	}

	///////////////////////
	// Inventory callbacks
	// cInventory *pInventory = mpInit->mpInventory;

	///////////////////////
	// Use callbacks
	{
		cContainerListIterator<cInventoryUseCallback> it = pSavedWorld->mlstUseCallbacks.GetIterator();
		while (it.HasNext()) {
			cInventoryUseCallback &temp = it.Next();
			mpInit->mpInventory->AddUseCallback(temp.msItem, temp.msObject, temp.msFunction);
		}
	}

	///////////////////////
	// Pickup callbacks
	{
		cContainerListIterator<cInventoryPickupCallback> it = pSavedWorld->mlstPickupCallbacks.GetIterator();
		while (it.HasNext()) {
			cInventoryPickupCallback &temp = it.Next();
			mpInit->mpInventory->AddPickupCallback(temp.msItem, temp.msFunction);
		}
	}
	///////////////////////
	// Combine callbacks
	{
		cContainerListIterator<cInventoryCombineCallback> it = pSavedWorld->mlstCombineCallbacks.GetIterator();
		while (it.HasNext()) {
			cInventoryCombineCallback &temp = it.Next();
			mpInit->mpInventory->AddCombineCallback(temp.msItem1, temp.msItem2, temp.msFunction);
		}
	}

	/////////////////////////
	// Entities on map
	mpInit->mpMapHandler->LoadSaveData(pSavedWorld);

	//////////////////////
	// Player world data (do after entities are created)
	mpInit->mpPlayer->LoadSaveData(pSavedWorld);
}

//-----------------------------------------------------------------------

void cSaveHandler::SaveGameToFile(const tWString &asFile) {
	// Reset all global data
	mpSavedGame->ResetGlobalData();

	//////////////////////////////
	// Save the current map:
	SaveData(mpInit->mpMapHandler->GetCurrentMapName());

	//////////////////////////////
	// Save Global variables
	mpSavedGame->mlstScriptVars.Clear();
	tScriptVarMap *pGlobalVarMap = mpInit->mpGame->GetScene()->GetGlobalVarMap();
	tScriptVarMapIt VarIt = pGlobalVarMap->begin();
	for (; VarIt != pGlobalVarMap->end(); ++VarIt) {
		mpSavedGame->mlstScriptVars.Add(VarIt->second);
	}

	//////////////////////////////
	// Save global properties
	mpSavedGame->mDifficulty = mpInit->mDifficulty;
	mpSavedGame->msOnRadioEndCallback = mpInit->mpRadioHandler->GetOnEndCallback();

	//////////////////////////////
	// Save player
	mpInit->mpPlayer->SaveToGlobal(&mpSavedGame->mPlayer);

	//////////////////////////////
	// Save map handler
	mpInit->mpMapHandler->SaveToGlobal(&mpSavedGame->mMapHandler);

	//////////////////////////////
	// Save loaded maps in Scene
	mpSavedGame->mvSceneLoadedMap.Clear();
	tStringSet *pStringSet = mpInit->mpGame->GetScene()->GetLoadedMapsSet();
	tStringSetIt it = pStringSet->begin();
	for (; it != pStringSet->end(); ++it) {
		cSceneLoadedMap_GlobalSave loadedMap;
		loadedMap.msName = *it;
		mpSavedGame->mvSceneLoadedMap.Add(loadedMap);
	}

	//////////////////////////////
	// Save inventory
	mpInit->mpInventory->SaveToGlobal(&mpSavedGame->mInventory);

	/////////////////////////////////
	// Save notebook
	mpInit->mpNotebook->SaveToGlobal(&mpSavedGame->mNotebook);

	//////////////////////////////
	// Load game music
	mpInit->mpMusicHandler->SaveToGlobal(&mpSavedGame->mGameMusicHandler);

	//////////////////////////////
	// Save current music
	cMusicEntry *pMusic = mpInit->mpGame->GetSound()->GetMusicHandler()->GetCurrentSong();
	if (pMusic) {
		mpSavedGame->mMusic.msName = pMusic->msFileName;
		mpSavedGame->mMusic.mfVolume = pMusic->mfMaxVolume;
		mpSavedGame->mMusic.mbLoop = pMusic->mbLoop;
	} else {
		mpSavedGame->mMusic.msName = "";
	}

	//////////////////////////////
	// Write to file:

	// tWString sSavePath = mpInit->mpGame->GetSystem()->GetLowLevel()->GetSystemSpecialPath(eSystemPath_Personal);

	// if(cString::GetLastCharW(sSavePath) != _W("/") && cString::GetLastCharW(sSavePath) != _W("\\"))
	// sSavePath += _W("/");

	tWString sSavePath = msSaveDir + asFile;
	// tString sSaveFile = cString::To8Char(sSavePath);
	// Log("Saving to %s\n",sSaveFile.c_str());
	// cSerializeClass::SetLog(true);
	cSerializeClass::SaveToFile(mpSavedGame, sSavePath, "SaveGame");
}

//-----------------------------------------------------------------------

void cSaveHandler::LoadGameFromFile(const tWString &asFile, bool drawLoadingScreen) {
	///////////////////////////////
	// Draw loading screen.
	if (drawLoadingScreen) // if the loading screen has already been drawn, drawing it twice causes bugs
		mpInit->mpGraphicsHelper->DrawLoadingScreen("");

	// 1. Reset everything
	// 2. Load all data from file
	// 3. Load data to player, map handler, inventory,etc
	// 4. Load current map in mpSaveGame. Do NOT call any scripts.
	// 5. Load save data to map
	// 6. Play some music

	mpInit->mpMapHandler->GetWorldCache()->DecResources();
	mpInit->mpMapHandler->GetWorldCache()->AddResources();
	mpInit->mbResetCache = false;
	mpInit->ResetGame(false);
	mpInit->mbResetCache = true;

	mpInit->mpGame->GetSound()->GetMusicHandler()->Stop(0);
	mpInit->mpGame->GetSound()->Update(1 / 60.0f);

	cSerializeClass::SetLog(false);
	tWString sSavePath = msSaveDir + asFile;
	// tString sSaveFile = cString::To8Char(sSavePath);
	cSerializeClass::LoadFromFile(mpSavedGame, sSavePath);

	//////////////////////////////
	// Load global properties
	mpInit->mDifficulty = mpSavedGame->mDifficulty;
	mpInit->mpRadioHandler->SetOnEndCallback(mpSavedGame->msOnRadioEndCallback);

	//////////////////////////////////////////
	// Load the save map handler data
	mpInit->mpMapHandler->LoadFromGlobal(&mpSavedGame->mMapHandler);

	//////////////////////////////
	// Load Global variables
	cContainerListIterator<cScriptVar> VarIt = mpSavedGame->mlstScriptVars.GetIterator();
	while (VarIt.HasNext()) {
		cScriptVar &tempVar = VarIt.Next();
		cScriptVar *pVar = mpInit->mpGame->GetScene()->CreateGlobalVar(tempVar.msName);
		pVar->mlVal = tempVar.mlVal;

		// Log("Created global var: '%s' %d\n",pVar->msName.c_str(),pVar->mlVal);
	}

	////////////////////////////////
	// Load loaded maps in Scene
	tStringSet *pStringSet = mpInit->mpGame->GetScene()->GetLoadedMapsSet();
	cContainerListIterator<cSceneLoadedMap_GlobalSave> it = mpSavedGame->mvSceneLoadedMap.GetIterator();
	while (it.HasNext()) {
		pStringSet->insert(it.Next().msName);
	}

	/////////////////////////////////
	// Load inventory
	mpInit->mpInventory->LoadFromGlobal(&mpSavedGame->mInventory);

	/////////////////////////////////
	// Load notebook
	mpInit->mpNotebook->LoadFromGlobal(&mpSavedGame->mNotebook);

	///////////////////////////////
	// Load the map
	// TOOD: Use the correct file!
	mpInit->mpMapHandler->LoadSimple(mpSavedGame->mMapHandler.msCurrentMap + ".dae", false);
	// LoadData(mpSavedGame->mMapHandler.msCurrentMap);
	// mpInit->mpMapHandler->Load(mpSavedGame->mMapHandler.msCurrentMap+".dae","link01");

	//////////////////////////////
	// Load player properties.
	mpInit->mpPlayer->LoadFromGlobal(&mpSavedGame->mPlayer);

	//////////////////////////////
	// Load game music
	mpInit->mpMusicHandler->LoadFromGlobal(&mpSavedGame->mGameMusicHandler);

	/////////////////////////////
	// Start Music
	if (mpSavedGame->mMusic.msName != "" && mpSavedGame->mMusic.mbLoop) {
		mpInit->mpGame->GetSound()->GetMusicHandler()->Play(mpSavedGame->mMusic.msName,
															mpSavedGame->mMusic.mfVolume, 0.5f, true);
	}

	// Reset logic timer.
	mpInit->mpGame->ResetLogicTimer();

	// Quick fix, make sure player is always active:
	mpInit->mpPlayer->SetActive(true);
}

//-----------------------------------------------------------------------

void cSaveHandler::AutoSave(const tWString &asDir, int alMaxSaves) {
	//////////////////////
	// Check the available autosaves
	DeleteOldestIfMax(asDir, _W(":*"), alMaxSaves);

	//////////////////////
	// Save the autosave
	tWString sMapName = mpInit->mpMapHandler->GetMapGameName();
	sMapName = cString::ReplaceCharToW(sMapName, _W("\n"), _W(" "));
	sMapName = cString::ReplaceCharToW(sMapName, _W(":"), _W(" "));
	cDate date = mpInit->mpGame->GetSystem()->GetLowLevel()->getDate();
	tWString sFile = Common::U32String::format("%S: %S %d-%d-%d %d:%d:%d",
											   asDir.c_str(),
											   sMapName.c_str(),
											   date.year,
											   date.month + 1,
											   date.month_day,
											   date.hours,
											   date.minutes,
											   date.seconds);
	SaveGameToFile(sFile);

	mpInit->mpGame->ResetLogicTimer();
}

//-----------------------------------------------------------------------

void cSaveHandler::AutoLoad(const tWString &asDir) {
	tWString latestSave = GetLatest(asDir + _W(":*"));
	LoadGameFromFile(latestSave);
	mpInit->mpGame->ResetLogicTimer();
}

//-----------------------------------------------------------------------

void cSaveHandler::ClearSavedMaps() {
	Log("Clearing all previously saved maps...\n");
	mpSavedGame->ResetWorlds();
}

//-----------------------------------------------------------------------

void cSaveHandler::OnWorldLoad() {
}

//-----------------------------------------------------------------------

void cSaveHandler::OnStart() {
}

//-----------------------------------------------------------------------

void cSaveHandler::Reset() {
	mpSavedGame->Reset();
}

void cSaveHandler::OnExit() {
	// Log(" doing nothing...\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cDate cSaveHandler::parseDate(const Common::String &saveFile) {
	cDate date;
	auto firstDigit = Common::find_if(saveFile.begin(), saveFile.end(), Common::isDigit);
	Common::String strDate = saveFile.substr(Common::distance(saveFile.begin(), firstDigit));
	sscanf(strDate.c_str(), "%d-%d-%d %d:%d:%d", &date.year, &date.month, &date.month_day, &date.hours, &date.minutes, &date.seconds);
	return date;
}

template<typename DateCmp>
Common::String firstSave(const Common::StringArray &saves, DateCmp cmp) {
	if (saves.empty())
		return "";
	cDate latestDate = cSaveHandler::parseDate(saves.front());
	const Common::String *latestSave = &saves.front();
	for (auto it = saves.begin() + 1; it != saves.end(); ++it) {
		cDate d = cSaveHandler::parseDate(*it);
		if (cmp(d, latestDate)) {
			latestDate = d;
			latestSave = it;
		}
	}
	return *latestSave;
}

void cSaveHandler::DeleteOldestIfMax(const tWString &asDir, const tWString &asMask, int alMaxFiles) {
	const Common::StringArray saves = Hpl1::g_engine->listInternalSaves(asDir + asMask);
	if (static_cast<int>(saves.size()) > alMaxFiles) {
		const Common::String oldest = firstSave(saves, [](const cDate &a, const cDate &b) { return a < b; });
		Hpl1::logInfo(Hpl1::kDebugSaves, "removing save %s\n", oldest.c_str());
		Hpl1::g_engine->removeSaveFile(oldest);
	}
}

//-----------------------------------------------------------------------

tWString cSaveHandler::GetLatest(const tWString &asMask) {
	Common::StringArray saves = Hpl1::g_engine->listInternalSaves(asMask);
	return firstSave(saves, [](const cDate &a, const cDate &b) { return a > b; });
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SERIALIZABLE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
kBeginSerializeBase(cMusic_GlobalSave)
	kSerializeVar(msName, eSerializeType_String)
		kSerializeVar(mfVolume, eSerializeType_Float32)
			kSerializeVar(mbLoop, eSerializeType_Bool)
				kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cGameMusic_GlobalSave)
		kSerializeVar(msFile, eSerializeType_String)
			kSerializeVar(mbLoop, eSerializeType_Bool)
				kSerializeVar(mfVolume, eSerializeType_Float32)
					kEndSerialize()

						kBeginSerializeBase(cGameMusicHandler_GlobalSave)
							kSerializeVar(mlCurrentMaxPrio, eSerializeType_Int32)
								kSerializeVar(mbAttackPlaying, eSerializeType_Bool)
									kSerializeVar(mbEnemyClosePlaying, eSerializeType_Bool)
										kSerializeClassContainer(mvGameMusic, cGameMusic_GlobalSave, eSerializeType_Class)
											kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cNotebookTask_GlobalSave)
		kSerializeVar(msName, eSerializeType_String)
			kSerializeVar(msText, eSerializeType_WString)
				kEndSerialize()

					kBeginSerializeBase(cNotebookNote_GlobalSave)
						kSerializeVar(msName, eSerializeType_WString)
							kSerializeVar(mbRead, eSerializeType_Bool)
								kSerializeVar(msTextCat, eSerializeType_String)
									kSerializeVar(msTextEntry, eSerializeType_String)
										kEndSerialize()

											kBeginSerializeBase(cNotebook_GlobalSave)
												kSerializeClassContainer(mlstNotes, cNotebookNote_GlobalSave, eSerializeType_Class)
													kSerializeClassContainer(mlstTasks, cNotebookTask_GlobalSave, eSerializeType_Class)
														kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cInventorySlot_GlobalSave)
		kSerializeVar(msItemName, eSerializeType_String)
			kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cInventoryItem_GlobalSave)
		kSerializeVar(msName, eSerializeType_String)
			kSerializeVar(msGameName, eSerializeType_WString)
				kSerializeVar(msDescription, eSerializeType_WString)

					kSerializeVar(mItemType, eSerializeType_Int32)

						kSerializeVar(msSubType, eSerializeType_String)

							kSerializeVar(msEntityFile, eSerializeType_String)
								kSerializeVar(msGfxObjectFile, eSerializeType_String)

									kSerializeVar(msHudModelFile, eSerializeType_String)
										kSerializeVar(msHudModelName, eSerializeType_String)

											kSerializeVar(mbCanBeDropped, eSerializeType_Bool)
												kSerializeVar(mbHasCount, eSerializeType_Bool)
													kSerializeVar(mlCount, eSerializeType_Int32)
														kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cInventory_GlobalSave)
		kSerializeVar(mbNoteBookActive, eSerializeType_Bool)
			kSerializeClassContainer(mlstSlots, cInventorySlot_GlobalSave, eSerializeType_Class)
				kSerializeClassContainer(mvEquipSlots, cInventorySlot_GlobalSave, eSerializeType_Class)
					kSerializeClassContainer(mlstItems, cInventoryItem_GlobalSave, eSerializeType_Class)
						kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cSceneLoadedMap_GlobalSave)
		kSerializeVar(msName, eSerializeType_String)
			kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cMapHandlerLoadedMap_GlobalSave)
		kSerializeVar(msName, eSerializeType_String)
			kSerializeVar(mfTime, eSerializeType_Float32)
				kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cMapHandlerTimer_GlobalSave)
		kSerializeVar(msName, eSerializeType_String)
			kSerializeVar(msCallback, eSerializeType_String)
				kSerializeVar(mbGlobal, eSerializeType_Bool)
					kSerializeVar(mfTime, eSerializeType_Float32)
						kSerializeVar(mbDeleteMe, eSerializeType_Bool)
							kSerializeVar(mbPaused, eSerializeType_Bool)
								kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cMapHandler_GlobalSave)
		kSerializeVar(mfGameTime, eSerializeType_Float32)
			kSerializeVar(msCurrentMap, eSerializeType_String)

				kSerializeClassContainer(mvLoadedMaps, cMapHandlerLoadedMap_GlobalSave, eSerializeType_Class)
					kSerializeClassContainer(mlstTimers, cMapHandlerTimer_GlobalSave, eSerializeType_Class)
						kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cPlayer_GlobalSave_CameraPS)
		kSerializeVar(msName, eSerializeType_String)
			kSerializeVar(msFile, eSerializeType_String)
				kEndSerialize()
	//-----------------------------------------------------------------------

	kBeginSerializeBase(cPlayer_GlobalSave)
	/////////////////
	// Stats
	kSerializeVar(mlStat_NumOfSaves, eSerializeType_Int32)

	/////////////////
	// Global
	kSerializeVar(mfForwardUpMul, eSerializeType_Float32)
		kSerializeVar(mfForwardRightMul, eSerializeType_Float32)
			kSerializeVar(mfUpMul, eSerializeType_Float32)
				kSerializeVar(mfRightMul, eSerializeType_Float32)

					kSerializeVar(mbPickAtPoint, eSerializeType_Bool)
						kSerializeVar(mbRotateWithPlayer, eSerializeType_Bool)
							kSerializeVar(mbUseNormalMass, eSerializeType_Bool)

								kSerializeVar(mfGrabMassMul, eSerializeType_Float32)

	//////////////////////////////
	// Private
	kSerializeVar(mbActive, eSerializeType_Bool)

		kSerializeVar(mfHeightAdd, eSerializeType_Float32)

			kSerializeVar(mfSpeedMul, eSerializeType_Float32)
				kSerializeVar(mfHealthSpeedMul, eSerializeType_Float32)

					kSerializeVar(mfHeadMoveSizeMul, eSerializeType_Float32)
						kSerializeVar(mfHeadMoveSpeedMul, eSerializeType_Float32)

							kSerializeVar(mState, eSerializeType_Int32)
								kSerializeVar(mMoveState, eSerializeType_Int32)
									kSerializeVar(mCrossHairState, eSerializeType_Int32)
										kSerializeVar(mbItemFlash, eSerializeType_Bool)

											kSerializeVar(mfHealth, eSerializeType_Float32)
												kSerializeVar(mfPower, eSerializeType_Float32)
													kSerializeVar(mfMass, eSerializeType_Float32)

														kSerializeClassContainer(mvOnCameraPS, cPlayer_GlobalSave_CameraPS, eSerializeType_Class)

	//////////////////////////////
	// Lights
	kSerializeVar(mbFlashlightActive, eSerializeType_Bool)
		kSerializeVar(mbFlashlightDisabled, eSerializeType_Bool)
			kSerializeVar(mbGlowstickActive, eSerializeType_Bool)
				kSerializeVar(mbFlareActive, eSerializeType_Bool)
					kSerializeVar(mfFlareTime, eSerializeType_Float32)

	//////////////////////////////
	// Body and Camera Specific
	kSerializeVar(mvPosition, eSerializeType_Vector3f)
		kSerializeVar(mfYaw, eSerializeType_Float32)
			kSerializeVar(mfPitch, eSerializeType_Float32)
				kEndSerialize()

	//-----------------------------------------------------------------------
	kBeginSerializeBase(cSavedWorld)
		kSerializeVar(msName, eSerializeType_String)
			kSerializeVar(msGameName, eSerializeType_WString)

				kSerializeVar(mAmbientColor, eSerializeType_Color)

					kSerializeVar(mbFogActive, eSerializeType_Bool)
						kSerializeVar(mfFogStartDist, eSerializeType_Float32)
							kSerializeVar(mfFogEndDist, eSerializeType_Float32)
								kSerializeVar(mFogColor, eSerializeType_Color)
									kSerializeVar(mbFogCulling, eSerializeType_Bool)

										kSerializeVar(mbSkyboxActive, eSerializeType_Bool)
											kSerializeVar(mSkyboxColor, eSerializeType_Color)
												kSerializeVar(msSkyboxFile, eSerializeType_String)

													kSerializeClassContainer(mlstVars, cScriptVar, eSerializeType_Class)

														kSerializeClassContainer(mlstUseCallbacks, cInventoryUseCallback, eSerializeType_Class)
															kSerializeClassContainer(mlstPickupCallbacks, cInventoryPickupCallback, eSerializeType_Class)
																kSerializeClassContainer(mlstCombineCallbacks, cInventoryCombineCallback, eSerializeType_Class)

																	kSerializeClassContainer(mlstCollideCallbacks, cSaveGame_cGameCollideScript, eSerializeType_Class)

																		kSerializeClassContainer(mlstTimers, cGameTimer, eSerializeType_Class)

																			kSerializeClassContainer(mlstEntities, iGameEntity_SaveData, eSerializeType_ClassPointer)

																				kSerializeClassContainer(mlstLights, cEngineLight_SaveData, eSerializeType_Class)
																					kSerializeClassContainer(mlstPS, cEnginePS_SaveData, eSerializeType_Class)
																						kSerializeClassContainer(mlstBeams, cEngineBeam_SaveData, eSerializeType_Class)
																							kSerializeClassContainer(mlstSounds, cEngineSound_SaveData, eSerializeType_Class)
																								kSerializeClassContainer(mlstJoints, cEngineJoint_SaveData, eSerializeType_Class)
																									kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cSavedGame)
		kSerializeVar(mDifficulty, eSerializeType_Int32)

			kSerializeVar(msOnRadioEndCallback, eSerializeType_String)

				kSerializeVar(mPlayer, eSerializeType_Class)
					kSerializeVar(mMapHandler, eSerializeType_Class)
						kSerializeVar(mInventory, eSerializeType_Class)
							kSerializeVar(mNotebook, eSerializeType_Class)
								kSerializeVar(mMusic, eSerializeType_Class)
									kSerializeVar(mGameMusicHandler, eSerializeType_Class)

										kSerializeClassContainer(mvSceneLoadedMap, cSceneLoadedMap_GlobalSave, eSerializeType_Class)
											kSerializeClassContainer(mlstScriptVars, cScriptVar, eSerializeType_Class)

												kSerializeClassContainer(mlstWorlds, cSavedWorld, eSerializeType_ClassPointer)
													kEndSerialize()

	//-----------------------------------------------------------------------
