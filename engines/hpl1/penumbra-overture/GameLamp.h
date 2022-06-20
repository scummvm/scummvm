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

#ifndef GAME_GAME_LAMP_H
#define GAME_GAME_LAMP_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//----------------------------------------------

class cGameLamp_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameLamp_SaveData);

public:
	bool mbLit;
	tString msLitChangeCallback;

	bool mbFlickering;

	iGameEntity *CreateEntity();
};

//----------------------------------------------

class cGameLamp_PS {
public:
	tString msName;
	tString msDataName;
	cMatrixf m_mtxTransform;
};

//----------------------------------------------

class cGameLamp : public iGameEntity {
	typedef iGameEntity super;
	friend class cEntityLoader_GameLamp;

public:
	cGameLamp(cInit *apInit, const tString &asName);
	~cGameLamp(void);

	void OnPlayerInteract();
	void OnPlayerPick();
	bool OnUseItem(cInventoryItem *apItem);

	void Update(float afTimeStep);

	void SetLitChangeCallback(const tString &asCallback) { msLitChangeCallback = asCallback; }

	void SetLit(bool abX, bool abFade);

	void SetFlicker(bool abX);

	void Init();

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);

private:
	void SetUpFlicker(int alIdx);

	bool mbLit;

	float mfAlpha;
	float mfTurnOnTime;
	float mfTurnOffTime;

	bool mbInteractOff;
	bool mbInteractOn;

	tString msTurnOnSound;
	tString msTurnOffSound;

	tString msOnItem;
	tString msOffItem;

	tString msLitChangeCallback;

	tString msOffMaterialName;
	tString msOffSubMesh;
	iMaterial *mpOffMaterial;
	iMaterial *mpOnMaterial;
	cSubMeshEntity *mpSubMesh;

	// Flickering
	bool mbFlickering;
	tString msFlickerOffSound;
	tString msFlickerOnSound;
	tString msFlickerOffPS;
	tString msFlickerOnPS;
	float mfFlickerOnMinLength;
	float mfFlickerOffMinLength;
	float mfFlickerOnMaxLength;
	float mfFlickerOffMaxLength;
	cColor mFlickerOffColor;
	float mfFlickerOffRadius;
	bool mbFlickerFade;
	float mfFlickerOnFadeLength;
	float mfFlickerOffFadeLength;

	// Init data
	std::vector<cColor> mvLightColors;
	std::vector<cColor> mvBBColors;
	std::vector<cGameLamp_PS> mvParticleSystemNames;
};

//--------------------------------------

class cEntityLoader_GameLamp : public cEntityLoader_Object {
public:
	cEntityLoader_GameLamp(const tString &asName, cInit *apInit);
	~cEntityLoader_GameLamp();

private:
	void BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);
	void AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

	cInit *mpInit;
};

#endif // GAME_GAME_LAMP_H
