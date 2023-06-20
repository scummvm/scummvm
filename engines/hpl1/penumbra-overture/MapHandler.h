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

#ifndef GAME_MAP_HANDLER_H
#define GAME_MAP_HANDLER_H

#include "hpl1/engine/engine.h"

// Linux X11 Defines This
#undef DestroyAll

using namespace hpl;

class cInit;
class iGameEntity;
class iGameEnemy;
class cMapHandler_GlobalSave;
class cGameStickArea;
class cSavedWorld;
class cGameTimer;
class cGameItem;

typedef Common::MultiMap<tString, iGameEntity *> tGameEntityMap;
typedef tGameEntityMap::iterator tGameEntityMapIt;

typedef Common::Array<iGameEntity *> tGameEntityVec;
typedef tGameEntityVec::iterator tGameEntityVecIt;

typedef Common::List<iGameEnemy *> tGameEnemyList;
typedef tGameEnemyList::iterator tGameEnemyListIt;

typedef Common::List<cGameItem *> tGameItemList;
typedef tGameItemList::iterator tGameItemListIt;

typedef Common::List<cGameStickArea *> tGameStickAreaList;
typedef tGameStickAreaList::iterator tGameStickAreaListIt;

typedef cSTLIterator<iGameEnemy *, tGameEnemyList, tGameEnemyListIt> tGameEnemyIterator;
typedef cSTLMapIterator<iGameEntity *, tGameEntityMap, tGameEntityMapIt> tGameEntityIterator;

//-----------------------------------------

class cMapChanger {
public:
	bool mbActive;
	tString msNewMap;
	tString msPosName;
	tString msDoneSound;
	float mfFadeInTime;
	tString msLoadTextCat;
	tString msLoadTextEntry;
};

//-----------------------------------------

class cWorldCache {
public:
	cWorldCache(cInit *apInit);
	~cWorldCache();

	void AddResources();
	void DecResources();

	int GetCount() { return mlCount; }

private:
	cInit *mpInit;
	cResources *mpResources;

	int mlCount;

	tResourceBaseList mlstMaterials;
	tResourceBaseList mlstMeshes;
	tResourceBaseList mlstAnimations;
};

//-----------------------------------------

class cLoadedMap {
public:
	tString msName;
	double mfTime;
};

//-----------------------------------------

typedef Common::List<cGameTimer *> tGameTimerList;
typedef tGameTimerList::iterator tGameTimerListIt;

//-----------------------------------------

class cEffectLightFlash {
public:
	cEffectLightFlash(cInit *apInit, const cVector3f &avPos, float afRadius, const cColor &aColor, float afAddTime, float afNegTime);
	~cEffectLightFlash();

	void Update(float afTimeStep);

	bool IsDead() { return mbDead; }

private:
	cInit *mpInit;

	iLight3D *mpLight;

	float mfRadius;
	float mfNegTime;

	bool mbIsDying;

	bool mbDead;
};

typedef Common::List<cEffectLightFlash *> tEffectLightFlashList;
typedef tEffectLightFlashList::iterator tEffectLightFlashListIt;

//-----------------------------------------

class cMapHandlerSoundCallback : public iSoundEntityGlobalCallback {
public:
	cMapHandlerSoundCallback(cInit *apInit);

	void OnStart(cSoundEntity *apSoundEntity);

private:
	cInit *mpInit;
	tStringVec mvEnemyHearableSounds;
};
//-----------------------------------------

class cMapHandler : public iUpdateable {
	friend class cMapHandlerSoundCallback;

public:
	cMapHandler(cInit *apInit);
	~cMapHandler();

	bool Load(const tString &asFile, const tString &asStartPos);
	bool LoadSimple(const tString &asFile, bool abLoadEntities);

	void ChangeMap(const tString &asMap, const tString &asPos, const tString &asStartSound,
				   const tString &asStopSound, float afFadeOutTime, float afFadeInTime,
				   tString asLoadTextCat, tString asLoadTextEntry);

	void AddSaveData(cSavedWorld *apSavedWorld);
	void LoadSaveData(cSavedWorld *apSavedWorld);

	void DestroyAll();

	void OnWorldLoad();
	void OnStart();
	void OnPostSceneDraw();
	void Update(float afTimeStep);
	void Reset();
	void OnDraw();

	cWorldCache *GetWorldCache() { return mpWorldCache; }

	cGameTimer *CreateTimer(const tString &asName, float afTime, const tString &asCallback, bool abGlobal);
	cGameTimer *GetTimer(const tString &asName);

	void AddLightFlash(const cVector3f &avPos, float afRadius, const cColor &aColor, float afAddTime, float afNegTime);

	void AddGameEnemy(iGameEnemy *apEnemy);
	tGameEnemyIterator GetGameEnemyIterator();

	void AddGameItem(cGameItem *apItem);
	void RemoveGameItem(cGameItem *apItem);

	void AddStickArea(cGameStickArea *apArea);
	void RemoveStickArea(cGameStickArea *apArea);
	cGameStickArea *GetBodyStickArea(iPhysicsBody *apBody);

	void AddGameEntity(iGameEntity *apEntity);
	void RemoveGameEntity(iGameEntity *apEntity);
	iGameEntity *GetGameEntity(const tString &asName, bool abErrorMessage = true);
	tGameEntityIterator GetGameEntityIterator();

	iGameEntity *GetLatestEntity();
	const tString &GetCurrentMapName() { return msCurrentMap; }
	void SetCurrentMapName(const tString &asName) { msCurrentMap = asName; }
	double GetGameTime() { return mfGameTime; }
	bool IsPreUpdating() { return mbPreUpdating; }

	void SetMapGameName(const tWString &asName) { msMapGameName = asName; }
	tWString GetMapGameName() { return msMapGameName; }

	bool IsDestroyingAll() { return mbDestroyingAll; }

	bool IsChangingMap() { return mMapChanger.mbActive; }

	/////////////////////////////////////////
	// Saving
	void SaveToGlobal(cMapHandler_GlobalSave *apSave);
	void LoadFromGlobal(cMapHandler_GlobalSave *apSave);

private:
	void RenderItemEffect();

	void PrintSoundsPlaying();

	void PreUpdate(double afTimeSinceVisit);

	double AddLoadedMap(cWorld3D *apWorld);

	void UpdateTimers(float afTimeStep);
	void RemoveLocalTimers();

	double mfGameTime;

	cInit *mpInit;

	cScene *mpScene;
	cResources *mpResources;

	iTexture *mpMapChangeTexture;

	cMapChanger mMapChanger;
	cWorldCache *mpWorldCache;

	iGameEntity *mpLatestEntity;
	tString msCurrentMap;
	tWString msMapGameName;

	Common::Array<cLoadedMap> mvLoadedMaps;

	tGameTimerList mlstTimers;

	cMapHandlerSoundCallback *mpSoundCallback;

	tGameEntityMap m_mapGameEntities;
	tGameEnemyList mlstGameEnemies;
	tGameItemList mlstGameItems;
	tEffectLightFlashList mlstLightFlashes;
	tGameStickAreaList mlstGameStickAreas;

	bool mbPreUpdating;

	bool mbDestroyingAll;
};

#endif // GAME_MAP_HANDLER_H
