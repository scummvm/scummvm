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

#ifndef GAME_GAME_OBJECT_H
#define GAME_GAME_OBJECT_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/GameEntity.h"

using namespace hpl;

//-----------------------------------------

class iGameEnemy;

//-----------------------------------------

class cGameObject_SaveData : public iGameEntity_SaveData {
	kSerializableClassInit(cGameObject_SaveData);

public:
	eObjectInteractMode mInteractMode;

	iGameEntity *CreateEntity();
};

//------------------------------------------
class cGameObject;

class cGameObjectBodyCallback : public iPhysicsBodyCallback {
public:
	cGameObjectBodyCallback(cInit *apInit, cGameObject *apObject);

	bool OnBeginCollision(iPhysicsBody *apBody, iPhysicsBody *apCollideBody);
	void OnCollide(iPhysicsBody *apBody, iPhysicsBody *apCollideBody, cPhysicsContactData *apContactData);

	cInit *mpInit;

	cGameObject *mpObject;
};

//--------------------------------------

class cObjectDisappearProperties {
public:
	cObjectDisappearProperties() {
		mbActive = false;
	}

	bool mbActive;

	float mfMinTime;
	float mfMaxTime;

	float mfTime;
	float mfMinDistance;

	float mfMinCloseDistance;
};

//--------------------------------------

class cObjectBreakProperties {
public:
	cObjectBreakProperties() {
		mbActive = false;
	}

	bool mbActive;

	tString msEntity;
	tString msSound;
	tString msPS;
	float mfMinImpulse;
	float mfMinNormalSpeed;
	float mfMinPlayerImpulse;
	float mfCenterForce;

	bool mbExplosion;
	float mfExpl_Radius;
	float mfExpl_MinDamage;
	float mfExpl_MaxDamage;
	float mfExpl_MinForce;
	float mfExpl_MaxForce;
	float mfExpl_MaxImpulse;
	float mfExpl_MinMass;
	int mlExpl_Strength;

	bool mbLightFlash;
	cColor mLight_Color;
	float mfLight_Radius;
	float mfLight_AddTime;
	float mfLight_NegTime;
	cVector3f mvLight_Offset;

	bool mbEarRing;
	float mfEarRing_MaxDist;
	float mfEarRing_Time;
};

//--------------------------------------

class cObjectAttractProperties {
public:
	cObjectAttractProperties() {
		mbActive = false;
	}

	bool mbActive;
	float mfDistance;
	tStringVec mvSubtypes;

	bool mbIsEaten;
	float mfEatLength;
};

//--------------------------------------

class cObjectDamageProperties {
public:
	cObjectDamageProperties() {
		mbActive = false;
	}

	bool mbActive;
	float mfMinLinearDamageSpeed;
	float mfMinAngularDamageSpeed;
	float mfMaxLinearDamageSpeed;
	float mfMaxAngularDamageSpeed;
	float mfMinDamage;
	float mfMaxDamage;
	int mlDamageStrength;
};

//--------------------------------------

class cGameObject : public iGameEntity {
	typedef iGameEntity super;
	friend class cEntityLoader_GameObject;
	friend class cGameObjectBodyCallback;

public:
	cGameObject(cInit *apInit, const tString &asName);
	~cGameObject(void);

	void OnPlayerInteract();
	void OnPlayerPick();

	bool IsSaved() {
		if (mDisappearProps.mbActive)
			return false;
		return mbIsSaved;
	}

	void Update(float afTimeStep);

	void BreakAction();

	void OnDeath(float afDamage);

	bool IsBreakable() { return mBreakProps.mbActive; }

	void OnPlayerGravityCollide(iCharacterBody *apCharBody, cCollideData *apCollideData);

	void SetInteractMode(eObjectInteractMode aInteractMode);
	eObjectInteractMode GetInteractMode() { return mInteractMode; }

	void SetupBreakObject();

	bool IsDestroyable() { return mbDestroyable; }
	float GetDestroyStrength() { return mfDestroyStrength; }
	const tString &GetDestroySound() { return msDestoySound; }

	void SetupForceOffset();

	// SaveObject implementation
	iGameEntity_SaveData *CreateSaveData();
	void SaveToSaveData(iGameEntity_SaveData *apSaveData);
	void LoadFromSaveData(iGameEntity_SaveData *apSaveData);

private:
	void GrabObject();

	void MoveObject();
	float GetMoveDist();

	void PushObject();
	float GetPushDist();

	void UpdateAttraction(float afTimeStep);
	std::set<iGameEnemy *> m_setAttractedEnemies;
	iGameEnemy *mpCurrentAttraction;
	float mfAttractCount;

	eObjectInteractMode mInteractMode;

	float mfForwardUpMul;
	float mfForwardRightMul;

	float mfUpMul;
	float mfRightMul;

	bool mbPickAtPoint;
	bool mbRotateWithPlayer;

	bool mbUseNormalMass;
	float mfGrabMassMul;

	bool mbCanBeThrown;
	bool mbCanBePulled;

	bool mbIsMover;

	bool mbDestroyable;
	float mfDestroyStrength;
	tString msDestoySound;

	bool mbForceLightOffset;
	cVector3f mvLightOffset;
	tMatrixfVec mvLightLocalOffsets;

	float mfHapticTorqueMul;

	float mfCloseToSameCount;

	cObjectBreakProperties mBreakProps;

	cObjectDisappearProperties mDisappearProps;

	cObjectAttractProperties mAttractProps;

	cObjectDamageProperties mDamageProps;

	cGameObjectBodyCallback *mpBodyCallback;
};

//--------------------------------------

class cEntityLoader_GameObject : public cEntityLoader_Object {
public:
	cEntityLoader_GameObject(const tString &asName, cInit *apInit);
	~cEntityLoader_GameObject();

	static eObjectInteractMode ToInteractMode(const char *apString);

private:
	void BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);
	void AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform, cWorld3D *apWorld);

	cInit *mpInit;
};

#endif // GAME_GAME_OBJECT_H
