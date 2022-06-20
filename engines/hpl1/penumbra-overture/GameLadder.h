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

#ifndef GAME_GAME_LADDER_H
#define GAME_GAME_LADDER_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//-----------------------------------------

class cGameLadder_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameLadder_SaveData);

public:
	cVector3f mvSize;

	tString msAttachSound;
	tString msClimbUpSound;
	tString msClimbDownSound;

	iGameEntity *CreateEntity();
};

//------------------------------------------

class cGameLadder : public iGameEntity {
	typedef iGameEntity super;
	friend class cAreaLoader_GameLadder;

public:
	cGameLadder(cInit *apInit, const tString &asName);
	~cGameLadder(void);

	void OnPlayerPick();
	void OnPlayerInteract();

	void OnPostSceneDraw();

	void Setup();

	float GetDist2D();
	float GetHeight();

	const cVector3f &GetForward() { return mvForward; }
	float GetMaxY() { return mfMaxY; }
	float GetMinY() { return mfMinY; }

	const tString &GetAttachSound() { return msAttachSound; }
	const tString &GetClimbUpSound() { return msClimbUpSound; }
	const tString &GetClimbDownSound() { return msClimbDownSound; }

	void SetAttachSound(const tString &asX) { msAttachSound = asX; }
	void SetClimbUpSound(const tString &asX) { msClimbUpSound = asX; }
	void SetClimbDownSound(const tString &asX) { msClimbDownSound = asX; }

	cVector3f GetStartRotation();
	cVector3f GetStartPosition();

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);

private:
	cVector3f mvForward;
	float mfMaxY;
	float mfMinY;

	tString msAttachSound;
	tString msClimbUpSound;
	tString msClimbDownSound;
};

//--------------------------------------

class cAreaLoader_GameLadder : public iArea3DLoader {

public:
	cAreaLoader_GameLadder(const tString &asName, cInit *apInit);
	~cAreaLoader_GameLadder();

	iEntity3D *Load(const tString &asName, const cVector3f &avSize, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

private:
	cInit *mpInit;
};

#endif // GAME_GAME_LADDER_H
