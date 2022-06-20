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

#ifndef GAME_GAME_SAVE_AREA_H
#define GAME_GAME_SAVE_AREA_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//-----------------------------------------

class cGameSaveArea_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameSaveArea_SaveData);

public:
	cVector3f mvSize;

	tString msMessageCat;
	tString msMessageEntry;

	tString msSound;

	bool mbHasBeenUsed;

	iGameEntity *CreateEntity();
};

//------------------------------------------

class cGameSaveArea : public iGameEntity {
	typedef iGameEntity super;
	friend class cAreaLoader_GameSaveArea;

public:
	cGameSaveArea(cInit *apInit, const tString &asName);
	~cGameSaveArea(void);

	void OnPlayerPick();
	void OnPlayerInteract();

	void SetMessageCat(const tString &asCat) { msMessageCat = asCat; }
	void SetMessageEntry(const tString &asEntry) { msMessageEntry = asEntry; }

	void SetSound(const tString &asSound) { msSound = asSound; }

	void SetHasBeenUsed(bool abX) { mbHasBeenUsed = abX; }

	tString &GetMessageCat() { return msMessageCat; }
	tString &GetMessageEntry() { return msMessageEntry; }

	tString &GetSound() { return msSound; }

	bool GetHasBeenUsed() { return mbHasBeenUsed; }

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);

private:
	tString msMessageCat;
	tString msMessageEntry;

	tString msSound;

	bool mbHasBeenUsed;
};

//--------------------------------------

class cAreaLoader_GameSaveArea : public iArea3DLoader {

public:
	cAreaLoader_GameSaveArea(const tString &asName, cInit *apInit);
	~cAreaLoader_GameSaveArea();

	iEntity3D *Load(const tString &asName, const cVector3f &avSize, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

private:
	cInit *mpInit;
};

#endif // GAME_GAME_SAVE_AREA_H
