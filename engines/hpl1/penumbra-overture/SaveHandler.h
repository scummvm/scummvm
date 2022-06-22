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

#ifndef GAME_SAVE_HANDLER_H
#define GAME_SAVE_HANDLER_H

#include "hpl1/engine/engine.h"

using namespace hpl;

class cInit;

#include "hpl1/penumbra-overture/GameTypes.h"
#include "hpl1/penumbra-overture/SaveTypes.h"

class iGameEntity_SaveData;

//---------------------------------------------

class cMusic_GlobalSave : public iSerializable {
	kSerializableClassInit(cMusic_GlobalSave) public : tString msName;
	float mfVolume;
	bool mbLoop;
};

//---------------------------------------------

class cGameMusic_GlobalSave : public iSerializable {
	kSerializableClassInit(cGameMusic_GlobalSave) public : tString msFile;
	bool mbLoop;
	float mfVolume;
};

class cGameMusicHandler_GlobalSave : public iSerializable {
	kSerializableClassInit(cGameMusicHandler_GlobalSave) public : int mlCurrentMaxPrio;
	bool mbAttackPlaying;
	bool mbEnemyClosePlaying;

	cContainerVec<cGameMusic_GlobalSave> mvGameMusic;
};

//---------------------------------------------

class cNotebookTask_GlobalSave : public iSerializable {
	kSerializableClassInit(cNotebookTask_GlobalSave) public : tString msName;
	tWString msText;
};

class cNotebookNote_GlobalSave : public iSerializable {
	kSerializableClassInit(cNotebookNote_GlobalSave) public : tWString msName;
	bool mbRead;
	tString msTextCat;
	tString msTextEntry;
};

class cNotebook_GlobalSave : public iSerializable {
	kSerializableClassInit(cNotebook_GlobalSave) public :

		cContainerList<cNotebookNote_GlobalSave> mlstNotes;
	cContainerList<cNotebookTask_GlobalSave> mlstTasks;
};

//---------------------------------------------

class cInventorySlot_GlobalSave : public iSerializable {
	kSerializableClassInit(cInventorySlot_GlobalSave) public : tString msItemName;
};

class cInventoryItem_GlobalSave : public iSerializable {
	kSerializableClassInit(cInventoryItem_GlobalSave) public : tString msName;
	tWString msGameName;
	tWString msDescription;

	tString msSubType;

	eGameItemType mItemType;

	tString msEntityFile;
	tString msGfxObjectFile;

	tString msHudModelFile;
	tString msHudModelName;

	bool mbCanBeDropped;
	bool mbHasCount;
	int mlCount;
};

class cInventory_GlobalSave : public iSerializable {
	kSerializableClassInit(cInventory_GlobalSave) public : bool mbNoteBookActive;

	cContainerList<cInventorySlot_GlobalSave> mlstSlots;
	cContainerVec<cInventorySlot_GlobalSave> mvEquipSlots;

	cContainerList<cInventoryItem_GlobalSave> mlstItems;
};

//---------------------------------------------

class cSceneLoadedMap_GlobalSave : public iSerializable {
	kSerializableClassInit(cSceneLoadedMap_GlobalSave) public : tString msName;
};

//---------------------------------------------

class cMapHandlerLoadedMap_GlobalSave : public iSerializable {
	kSerializableClassInit(cMapHandlerLoadedMap_GlobalSave) public : tString msName;
	double mfTime;
};

/////////////

class cMapHandlerTimer_GlobalSave : public iSerializable {
	kSerializableClassInit(cMapHandlerTimer_GlobalSave) public : tString msName;
	tString msCallback;

	bool mbGlobal;

	float mfTime;

	bool mbDeleteMe;
	bool mbPaused;
};

/////////////

class cMapHandler_GlobalSave : public iSerializable {
	kSerializableClassInit(cMapHandler_GlobalSave);

public:
	double mfGameTime;
	tString msCurrentMap;

	cContainerVec<cMapHandlerLoadedMap_GlobalSave> mvLoadedMaps;
	cContainerList<cMapHandlerTimer_GlobalSave> mlstTimers;
};

//---------------------------------------------

class cPlayer_GlobalSave_CameraPS : public iSerializable {
	kSerializableClassInit(cPlayer_GlobalSave_CameraPS);

public:
	tString msName;
	tString msFile;
};

class cPlayer_GlobalSave : public iSerializable {
	kSerializableClassInit(cPlayer_GlobalSave);

public:
	//////////////////////////////
	// Stats
	int mlStat_NumOfSaves;

	//////////////////////////////
	// Global vars
	float mfForwardUpMul;
	float mfForwardRightMul;

	float mfUpMul;
	float mfRightMul;

	bool mbPickAtPoint;
	bool mbRotateWithPlayer;
	bool mbUseNormalMass;

	float mfGrabMassMul;

	//////////////////////////////
	// Private
	bool mbActive;

	float mfHeightAdd;

	float mfSpeedMul;
	float mfHealthSpeedMul;

	float mfHeadMoveSizeMul;
	float mfHeadMoveSpeedMul;

	ePlayerState mState;

	ePlayerMoveState mMoveState;

	eCrossHairState mCrossHairState;

	bool mbItemFlash;

	float mfHealth;

	float mfPower;

	float mfMass;

	cContainerVec<cPlayer_GlobalSave_CameraPS> mvOnCameraPS;

	//////////////////////////////
	// Lights
	bool mbFlashlightActive;
	bool mbFlashlightDisabled;
	bool mbGlowstickActive;
	bool mbFlareActive;
	float mfFlareTime;

	//////////////////////////////
	// Body and Camera Specific
	cVector3f mvPosition;
	float mfYaw;
	float mfPitch;
};

//---------------------------------------------

class cSavedWorld : public iSerializable {
	kSerializableClassInit_nodestructor(cSavedWorld) public : ~cSavedWorld();

	void Reset();

	tString msName;
	tWString msGameName;

	// Ambient color
	cColor mAmbientColor;

	// Fog
	bool mbFogActive;
	float mfFogStartDist;
	float mfFogEndDist;
	cColor mFogColor;
	bool mbFogCulling;

	// Skybox
	bool mbSkyboxActive;
	cColor mSkyboxColor;
	tString msSkyboxFile;

	// Script variables
	cContainerList<cScriptVar> mlstVars;

	// Inventory variables
	cContainerList<cInventoryUseCallback> mlstUseCallbacks;
	cContainerList<cInventoryPickupCallback> mlstPickupCallbacks;
	cContainerList<cInventoryCombineCallback> mlstCombineCallbacks;

	// Player collider callbacks
	cContainerList<cSaveGame_cGameCollideScript> mlstCollideCallbacks;

	// Local Timers
	cContainerList<cGameTimer> mlstTimers;

	// Entities
	cContainerList<iGameEntity_SaveData *> mlstEntities;

	// Engine types
	cContainerList<cEngineLight_SaveData> mlstLights;
	cContainerList<cEnginePS_SaveData> mlstPS;
	cContainerList<cEngineBeam_SaveData> mlstBeams;
	cContainerList<cEngineSound_SaveData> mlstSounds;

	cContainerList<cEngineJoint_SaveData> mlstJoints;

	bool PSExists(cParticleSystem3D *apPS);
	bool BeamExists(cBeam *apPS);
	bool SoundExists(cSoundEntity *apSound);
	bool JointExists(iPhysicsJoint *apJoint);
};

//----------------------------

class cSavedGame : public iSerializable {
	kSerializableClassInit_nodestructor(cSavedGame) public : ~cSavedGame();

	void Reset();
	void ResetWorlds();
	void ResetGlobalData();
	cSavedWorld *GetSavedWorld(const tString &asName);

	// Variables:
	eGameDifficulty mDifficulty;

	tString msOnRadioEndCallback;

	cPlayer_GlobalSave mPlayer;
	cMapHandler_GlobalSave mMapHandler;
	cInventory_GlobalSave mInventory;
	cNotebook_GlobalSave mNotebook;
	cGameMusicHandler_GlobalSave mGameMusicHandler;
	cMusic_GlobalSave mMusic;

	cContainerList<cSceneLoadedMap_GlobalSave> mvSceneLoadedMap;
	cContainerList<cScriptVar> mlstScriptVars;

	cContainerList<cSavedWorld *> mlstWorlds;
};

//----------------------------

class cSaveHandler : public iUpdateable {
public:
	cSaveHandler(cInit *apInit);
	~cSaveHandler();

	void SaveData(const tString &asName);
	void LoadData(const tString &asName);

	void SaveGameToFile(const tWString &asFile);
	void LoadGameFromFile(const tWString &asFile);

	void AutoSave(const tWString &asDir, int alMaxSaves);
	void AutoLoad(const tWString &asDir);

	void ClearSavedMaps();

	void OnWorldLoad();
	void OnStart();
	void Reset();

	void OnExit();

	const tWString &GetSaveDir() { return msSaveDir; }

	void DeleteOldestIfMax(const tWString &asDir, const tWString &asMask, int alMaxFiles);
	tWString GetLatest(const tWString &asDir, const tWString &asMask);

private:
	cInit *mpInit;

	cSavedGame *mpSavedGame;
	tWString msSaveDir;
};

#endif // GAME_SAVE_HANDLER_H
