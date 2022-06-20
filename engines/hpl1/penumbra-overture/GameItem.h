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

#ifndef GAME_GAME_ITEM_H
#define GAME_GAME_ITEM_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//-----------------------------------------

class cGameItem_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameItem_SaveData);

public:
	bool mbHasBeenFlashed;
	float mfEnterFlashDist;
	float mfExitFlashDist;

	iGameEntity *CreateEntity();
};

//------------------------------------------

class cGameItem_InViewRay : public iPhysicsRayCallback {
public:
	void SetUp(iPhysicsBody *apSkipBody);
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	bool mbIntersected;
	iPhysicsBody *mpSkipBody;
};

//------------------------------------------

class cGameItem : public iGameEntity {
	typedef iGameEntity super;
	friend class cEntityLoader_GameItem;

public:
	cGameItem(cInit *apInit, const tString &asName);
	~cGameItem(void);

	void OnWorldLoad();

	void OnPlayerPick();

	void OnPlayerInteract();

	void Update(float afTimeStep);

	void OnPostSceneDraw();

	float GetFlashAlpha() { return mfFlashAlpha; }

	const tString &GetImageFile() { return msImageFile; }
	eGameItemType GetItemType() { return mItemType; }

	bool CanBeDropped() { return mbCanBeDropped; }

	bool HasCount() { return mbHasCount; }
	int GetCount() { return mlCount; }

	tString GetHudModelFile() { return msHudModelFile; }
	tString GetHudModelName() { return msHudModelName; }

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);

private:
	bool IsInView(float afMinDist);

	tString msImageFile;
	eGameItemType mItemType;
	bool mbCanBeDropped;
	bool mbHasCount;
	int mlCount;

	float mfCheckFlashCount;
	float mfCheckFlashMax;
	float mfStartFlashCount;
	float mfFlashAlpha;
	float mfFlashAlphaAdd;
	bool mbHasBeenFlashed;
	float mfEnterFlashDist;
	float mfExitFlashDist;

	bool mbSkipRayCheck;

	cGameItem_InViewRay mRayCallback;

	tString msHudModelFile;
	tString msHudModelName;

	tString msPickUpSound;
};

//--------------------------------------

class cEntityLoader_GameItem : public cEntityLoader_Object {

public:
	cEntityLoader_GameItem(const tString &asName, cInit *apInit);
	~cEntityLoader_GameItem();

	static eGameItemType ToItemType(const char *apString);

private:
	void BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);
	void AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

	cInit *mpInit;
};

#endif // GAME_GAME_ITEM_H
