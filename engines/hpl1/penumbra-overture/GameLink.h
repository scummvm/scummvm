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

#ifndef GAME_GAME_LINK_H
#define GAME_GAME_LINK_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//-----------------------------------------

class cGameLink_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameLink_SaveData);

public:
	cVector3f mvSize;

	tString msMapFile;
	tString msMapPos;
	tString msStartSound;
	tString msStopSound;
	float mfFadeOutTime;
	float mfFadeInTime;
	tString msLoadTextCat;
	tString msLoadTextEntry;

	iGameEntity *CreateEntity();
};

//------------------------------------------

class cGameLink : public iGameEntity {
	typedef iGameEntity super;
	friend class cAreaLoader_GameLink;

public:
	cGameLink(cInit *apInit, const tString &asName);
	~cGameLink(void);

	void OnPlayerPick();
	void OnPlayerInteract();

	tString msMapFile;
	tString msMapPos;
	tString msStartSound;
	tString msStopSound;
	float mfFadeOutTime;
	float mfFadeInTime;
	tString msLoadTextCat;
	tString msLoadTextEntry;

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);
};

//--------------------------------------

class cAreaLoader_GameLink : public iArea3DLoader {

public:
	cAreaLoader_GameLink(const tString &asName, cInit *apInit);
	~cAreaLoader_GameLink();

	iEntity3D *Load(const tString &asName, const cVector3f &avSize, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

private:
	cInit *mpInit;
};

#endif // GAME_GAME_LINK_H
