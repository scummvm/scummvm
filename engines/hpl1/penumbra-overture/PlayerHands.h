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

#ifndef GAME_PLAYER_HANDS_H
#define GAME_PLAYER_HANDS_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameTypes.h"

using namespace hpl;

class cInit;

//------------------------------------------------

class cHudModelPose {
public:
	cHudModelPose() {}
	cHudModelPose(const cVector3f &avPos, const cVector3f &avRot) {
		mvPos = avPos;
		mvRot = avRot;
	}

	cMatrixf ToMatrix() {
		cMatrixf mtxPose = cMath::MatrixRotate(mvRot, eEulerRotationOrder_XYZ);
		mtxPose.SetTranslation(mvPos);
		return mtxPose;
	}

	cVector3f mvPos;
	cVector3f mvRot;
};

//------------------------------------------------

enum eHudModelState {
	eHudModelState_Idle,
	eHudModelState_Equip,
	eHudModelState_Unequip
};

//------------------------------------------------

cHudModelPose GetPoseFromElem(const tString &asName, TiXmlElement *apElem);

cMatrixf InterpolatePosesToMatrix(float afT, const cHudModelPose &aPoseA, const cHudModelPose &aPoseB);

//------------------------------------------------

class iHudModel {
	friend class cPlayerHands;

public:
	iHudModel(ePlayerHandType aType);
	virtual ~iHudModel() = default;

	cMeshEntity *GetEntity() { return mpEntity; }

	void LoadEntities();
	void DestroyEntities();

	void EquipEffect(bool abJustCreated);
	void UnequipEffect();

	void Reset();

	tString msName;
	tString msModelFile;

	cHudModelPose mEquipPose;
	cHudModelPose mUnequipPose;

	tString msEquipSound;
	tString msUnequipSound;

	float mfEquipTime;
	float mfUnequipTime;

	eHudModelState GetState() { return mState; }

	Common::Array<cParticleSystem3D *> mvParticleSystems;
	Common::Array<cBillboard *> mvBillboards;
	Common::Array<iLight3D *> mvLights;
	Common::Array<cColor> mvLightColors;
	Common::Array<float> mvLightRadii;
	Common::Array<cSoundEntity *> mvSoundEntities;

	virtual void LoadData(TiXmlElement *apRootElem) = 0;

protected:
	virtual void ResetExtraData() = 0;

	virtual bool UpdatePoseMatrix(cMatrixf &aPoseMtx, float afTimeStep) { return false; }

	virtual void LoadExtraEntites() {}
	virtual void DestroyExtraEntities() {}

	virtual void PostSceneDraw() {}

	ePlayerHandType mType;

	cInit *mpInit;

	cMesh *mpMesh;
	cMeshEntity *mpEntity;

	eHudModelState mState;

	float mfTime;

	tString msNextModel;
};

//------------------------------------------------

class cHudModel_Normal : iHudModel {
	friend class cPlayerHands;

public:
	cHudModel_Normal() : iHudModel(ePlayerHandType_Normal) {}
	virtual ~cHudModel_Normal() = default;

	void LoadData(TiXmlElement *apRootElem) {}

	void ResetExtraData() {}
};

//------------------------------------------------

typedef Common::StableMap<tString, iHudModel *> tHudModelMap;
typedef tHudModelMap::iterator tHudModelMapIt;

//------------------------------------------------

class cPlayerHands : public iUpdateable {
public:
	cPlayerHands(cInit *apInit);
	~cPlayerHands();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();
	void OnDraw();

	void OnWorldExit();
	void OnWorldLoad();

	void AddHudModel(iHudModel *apHudModel);
	bool AddModelFromFile(const tString &asFile);

	iHudModel *GetModel(const tString &asName);

	void SetCurrentModel(int alNum, const tString &asName);
	iHudModel *GetCurrentModel(int alNum) { return mvCurrentHudModels[alNum]; }

private:
	void UpdatePrevPostions();

	cInit *mpInit;
	cMeshManager *mpMeshManager;

	tVector3fList mlstPositions;
	tVector3fList mlstRotations;
	int mlMaxPositions;
	int mlMaxRotations;

	cVector3f mvSmoothCameraPos;
	cVector3f mvSmoothCameraRot;

	tHudModelMap m_mapHudModels;
	iHudModel *mvCurrentHudModels[2];
	int mlCurrentModelNum;
};

#endif // GAME_PLAYER_HANDS_H
