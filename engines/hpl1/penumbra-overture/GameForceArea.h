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

#ifndef GAME_GAME_FORCE_AREA_H
#define GAME_GAME_FORCE_AREA_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//------------------------------------------

class cGameForceArea_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameForceArea_SaveData);

public:
	cVector3f mvSize;

	float mfMaxForce;
	float mfConstant;
	float mfDestSpeed;
	float mfMaxMass;

	bool mbMulWithMass;
	bool mbAffectCharacters;
	bool mbAffectBodies;
	bool mbForceAtPoint;

	iGameEntity *CreateEntity();
};

//------------------------------------------

class cGameForceArea : public iGameEntity {
	typedef iGameEntity super;
	friend class cAreaLoader_GameForceArea;

public:
	cGameForceArea(cInit *apInit, const tString &asName);
	~cGameForceArea(void);

	void Setup();

	void Update(float afTimeStep);
	void OnPostSceneDraw();

	void SetMaxForce(float afX) { mfMaxForce = afX; }
	void SetConstant(float afX) { mfConstant = afX; }
	void SetDestSpeed(float afX) { mfDestSpeed = afX; }
	void SetMaxMass(float afX) { mfMaxMass = afX; }

	void SetMulWithMass(bool abX) { mbMulWithMass = abX; }
	void SetAffectCharacters(bool abX) { mbAffectCharacters = abX; }
	void SetAffectBodies(bool abX) { mbAffectBodies = abX; }
	void SetForceAtPoint(bool abX) { mbForceAtPoint = abX; }

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);
	void SetupSaveData(iGameEntity_SaveData *apSaveData);

private:
	float mfMaxForce;
	float mfConstant;
	float mfDestSpeed;
	float mfMaxMass;

	bool mbMulWithMass;
	bool mbAffectCharacters;
	bool mbAffectBodies;
	bool mbForceAtPoint;

	cVector3f mvUp;
};

//------------------------------------------

class cAreaLoader_GameForceArea : public iArea3DLoader {

public:
	cAreaLoader_GameForceArea(const tString &asName, cInit *apInit);
	~cAreaLoader_GameForceArea();

	iEntity3D *Load(const tString &asName, const cVector3f &avSize, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

private:
	cInit *mpInit;
};

#endif // GAME_GAME_DAMAGE_AREA_H
