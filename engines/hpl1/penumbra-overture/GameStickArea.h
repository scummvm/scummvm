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

#ifndef GAME_GAME_STICK_AREA_H
#define GAME_GAME_STICK_AREA_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//------------------------------------------

class cGameStickArea_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameStickArea_SaveData);

public:
	cVector3f mvSize;

	tString msAttachFunction;
	tString msDetachFunction;

	tString msAttachSound;
	tString msDetachSound;

	tString msAttachPS;
	tString msDetachPS;

	bool mbCanDeatch;

	tString msAttachedBody;

	bool mbCheckCenterInArea;

	float mfPoseTime;

	float mfSetMtxTime;

	iGameEntity *CreateEntity();
};

//------------------------------------------

class cGameStickArea : public iGameEntity {
	typedef iGameEntity super;
	friend class cAreaLoader_GameStickArea;

public:
	cGameStickArea(cInit *apInit, const tString &asName);
	~cGameStickArea(void);

	void OnPlayerPick();

	void Update(float afTimeStep);

	void OnPostSceneDraw();

	iPhysicsBody *GetAttachedBody() { return mpAttachedBody; }
	void DetachBody();

	void SetAttachFunction(const tString &asX) { msAttachFunction = asX; }
	void SetDetachFunction(const tString &asX) { msDetachFunction = asX; }

	void SetAttachSound(const tString &asX) { msAttachSound = asX; }
	void SetDetachSound(const tString &asX) { msDetachSound = asX; }

	void SetAttachPS(const tString &asX) { msAttachPS = asX; }
	void SetDetachPS(const tString &asX) { msDetachPS = asX; }

	void SetCanDeatch(bool abX) { mbCanDeatch = abX; }
	bool GetCanDeatch();

	void SetCheckCenterInArea(bool abX) { mbCheckCenterInArea = abX; }

	void SetPoseTime(float afX) { mfPoseTime = afX; }

	void SetMoveBody(bool abX) { mbMoveBody = abX; }
	bool GetMoveBody() { return mbMoveBody; }
	void SetRotateBody(bool abX) { mbRotateBody = abX; }
	bool GetRotateBody() { return mbRotateBody; }

	tString GetCallbackFunc(const tString &asFunc, iPhysicsBody *apBody);

	static bool mbAllowAttachment;

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);
	void SetupSaveData(iGameEntity_SaveData *apSaveData);

private:
	tString msAttachFunction;
	tString msDetachFunction;

	tString msAttachSound;
	tString msDetachSound;

	tString msAttachPS;
	tString msDetachPS;

	bool mbMoveBody;
	bool mbRotateBody;

	bool mbCheckCenterInArea;

	float mfPoseTime;

	bool mbCanDeatch;

	bool mbBodyGravity;
	float mfBodyMass;
	iPhysicsBody *mpAttachedBody;
	iPhysicsBody *mpLastAttachedBody;

	float mfSetMtxTime;
	cMatrixf mtxAttachedStart;
};

//------------------------------------------

class cAreaLoader_GameStickArea : public iArea3DLoader {

public:
	cAreaLoader_GameStickArea(const tString &asName, cInit *apInit);
	~cAreaLoader_GameStickArea();

	iEntity3D *Load(const tString &asName, const cVector3f &avSize, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

private:
	cInit *mpInit;
};

#endif // GAME_GAME_STICK_AREA_H
