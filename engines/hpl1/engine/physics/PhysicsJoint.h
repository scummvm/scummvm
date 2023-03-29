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
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_PHYSICS_JOINT_H
#define HPL_PHYSICS_JOINT_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"

#include "hpl1/engine/game/SaveGame.h"

#include "hpl1/engine/physics/PhysicsController.h"

namespace hpl {

class iPhysicsBody;
class iPhysicsWorld;
class cSoundEntity;
class iPhysicsJoint;
class iPhysicsController;

typedef Hpl1::Std::map<tString, iPhysicsController *> tPhysicsControllerMap;
typedef tPhysicsControllerMap::iterator tPhysicsControllerMapIt;

typedef cSTLMapIterator<iPhysicsController *, tPhysicsControllerMap, tPhysicsControllerMapIt> cPhysicsControllerIterator;

//-----------------------------------

enum ePhysicsJointType {
	ePhysicsJointType_Ball,
	ePhysicsJointType_Hinge,
	ePhysicsJointType_Slider,
	ePhysicsJointType_Screw,
	ePhysicsJointType_LastEnum
};

//-----------------------------------

enum ePhysicsJointSpeed {
	ePhysicsJointSpeed_Linear,
	ePhysicsJointSpeed_Angular,
	ePhysicsJointSpeed_LastEnum
};

//-----------------------------------

class cJointLimitEffect : public iSerializable {
	kSerializableClassInit(cJointLimitEffect) public : tString msSound;
	float mfMinSpeed;
	float mfMaxSpeed;
};

//-----------------------------------

class iPhysicsJointCallback {
public:
	virtual ~iPhysicsJointCallback() {}

	virtual void OnMinLimit(iPhysicsJoint *apJoint) = 0;
	virtual void OnMaxLimit(iPhysicsJoint *apJoint) = 0;

	// Ugly trick to support joint script callback.
	virtual bool IsScript() { return false; }
};

//-----------------------------------

kSaveData_BaseClass(iPhysicsJoint) {
	kSaveData_ClassInit(iPhysicsJoint) public : tString msName;

	int mlParentBodyId;
	int mlChildBodyId;

	cMatrixf m_mtxParentBodySetup;
	cMatrixf m_mtxChildBodySetup;

	cVector3f mvPinDir;
	cVector3f mvStartPivotPoint;

	cContainerList<cSaveData_iPhysicsController> mlstControllers;

	cJointLimitEffect mMaxLimit;
	cJointLimitEffect mMinLimit;

	tString msMoveSound;

	float mfMinMoveSpeed;
	float mfMinMoveFreq;
	float mfMinMoveFreqSpeed;
	float mfMinMoveVolume;
	float mfMaxMoveFreq;
	float mfMaxMoveFreqSpeed;
	float mfMaxMoveVolume;
	float mfMiddleMoveSpeed;
	float mfMiddleMoveVolume;
	int mMoveSpeedType;

	bool mbBreakable;
	float mfBreakForce;
	tString msBreakSound;
	bool mbBroken;

	tString msCallbackMaxFunc;
	tString msCallbackMinFunc;
	bool mbAutoDeleteCallback;
};

//-----------------------------------

class iPhysicsJoint : public iSaveObject {
	typedef iSaveObject super;

public:
	iPhysicsJoint(const tString &asName, iPhysicsBody *apParentBody, iPhysicsBody *apChildBody,
				  iPhysicsWorld *apWorld, const cVector3f &avPivotPoint);
	virtual ~iPhysicsJoint();

	const tString &GetName() { return msName; }

	iPhysicsBody *GetParentBody() { return mpParentBody; }
	iPhysicsBody *GetChildBody() { return mpChildBody; }

	void RemoveBody(iPhysicsBody *apBody);

	cVector3f GetPivotPoint() { return mvPivotPoint; }
	cVector3f GetPinDir() { return mvPinDir; }

	virtual ePhysicsJointType GetType() = 0;

	virtual void SetCollideBodies(bool abX) = 0;
	virtual bool GetCollideBodies() = 0;

	virtual void SetStiffness(float afX) = 0;
	virtual float GetStiffness() = 0;

	virtual cVector3f GetVelocity() = 0;
	virtual cVector3f GetAngularVelocity() = 0;
	virtual cVector3f GetForce() = 0;

	virtual float GetDistance() = 0;
	virtual float GetAngle() = 0;

	cJointLimitEffect *GetMaxLimit() { return &mMaxLimit; }
	cJointLimitEffect *GetMinLimit() { return &mMinLimit; }

	void SetMoveSound(tString &asName) { msMoveSound = asName; }

	void SetMoveSpeedType(ePhysicsJointSpeed aType) { mMoveSpeedType = aType; }
	void SetMinMoveSpeed(float afX) { mfMinMoveSpeed = afX; }
	void SetMinMoveFreq(float afX) { mfMinMoveFreq = afX; }
	void SetMinMoveFreqSpeed(float afX) { mfMinMoveFreqSpeed = afX; }
	void SetMinMoveVolume(float afX) { mfMinMoveVolume = afX; }
	void SetMaxMoveFreq(float afX) { mfMaxMoveFreq = afX; }
	void SetMaxMoveVolume(float afX) { mfMaxMoveVolume = afX; }
	void SetMaxMoveFreqSpeed(float afX) { mfMaxMoveFreqSpeed = afX; }
	void SetMiddleMoveSpeed(float afX) { mfMiddleMoveSpeed = afX; }
	void SetMiddleMoveVolume(float afX) { mfMiddleMoveVolume = afX; }

	void SetCallback(iPhysicsJointCallback *apCallback, bool abAutoDelete) {
		mpCallback = apCallback;
		mbAutoDeleteCallback = abAutoDelete;
	}

	iPhysicsJointCallback *GetCallback() { return mpCallback; }

	bool CheckBreakage();

	void SetBreakable(bool abX) { mbBreakable = abX; }
	bool IsBreakable() { return mbBreakable; }
	void SetBreakForce(float afForce) { mfBreakForce = afForce; }
	float GetBreakForce() { return mfBreakForce; }
	void SetBreakSound(const tString &asSound) { msBreakSound = asSound; }

	void SetLimitAutoSleep(bool abX) { mbLimitAutoSleep = abX; }
	void SetLimitAutoSleepDist(float afX) { mfLimitAutoSleepDist = afX; }
	void SetLimitAutoSleepNumSteps(int alX) { mlLimitAutoSleepNumSteps = alX; }

	bool GetLimitAutoSleep() { return mbLimitAutoSleep; }
	float GetLimitAutoSleepDist() { return mfLimitAutoSleepDist; }
	int GetLimitAutoSleepNumSteps() { return mlLimitAutoSleepNumSteps; }

	void SetStickyMinDistance(float afX) { mfStickyMinDistance = afX; }
	void SetStickyMaxDistance(float afX) { mfStickyMaxDistance = afX; }
	float GetStickyMinDistance() { return mfStickyMinDistance; }
	float GetStickyMaxDistance() { return mfStickyMaxDistance; }

	void Break();

	bool IsBroken() { return mbBroken; }

	void SetUserData(void *apUserData) { mpUserData = apUserData; }

	void AddController(iPhysicsController *apController);
	iPhysicsController *GetController(const tString &asName);
	bool ChangeController(const tString &asName);
	cPhysicsControllerIterator GetControllerIterator();

	void SetAllControllersPaused(bool abX);

	void OnPhysicsUpdate();

	void SetSound(cSoundEntity *apSound) { mpSound = apSound; }
	cSoundEntity *GetSound() { return mpSound; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	tString msName;

	iPhysicsBody *mpParentBody;
	iPhysicsBody *mpChildBody;
	iPhysicsWorld *mpWorld;

	cMatrixf m_mtxParentBodySetup;
	cMatrixf m_mtxChildBodySetup;

	cVector3f mvPinDir;
	cVector3f mvPivotPoint;
	cVector3f mvStartPivotPoint;

	cVector3f mvLocalPivot;

	float mfStickyMinDistance;
	float mfStickyMaxDistance;

	tPhysicsControllerMap m_mapControllers;

	cJointLimitEffect mMaxLimit;
	cJointLimitEffect mMinLimit;

	int mlSpeedCount;

	cMatrixf m_mtxPrevChild;
	cMatrixf m_mtxPrevParent;

	tString msMoveSound;

	float mfMinMoveSpeed;
	float mfMinMoveFreq;
	float mfMinMoveFreqSpeed;
	float mfMinMoveVolume;
	float mfMaxMoveFreq;
	float mfMaxMoveFreqSpeed;
	float mfMaxMoveVolume;
	float mfMiddleMoveSpeed;
	float mfMiddleMoveVolume;
	ePhysicsJointSpeed mMoveSpeedType;

	bool mbBreakable;
	float mfBreakForce;
	tString msBreakSound;
	bool mbBroken;

	bool mbLimitAutoSleep;
	float mfLimitAutoSleepDist;
	int mlLimitAutoSleepNumSteps;

	cSoundEntity *mpSound;
	bool mbHasCollided;

	iPhysicsJointCallback *mpCallback;
	bool mbAutoDeleteCallback;

	int mlLimitStepCount;

	void *mpUserData;

	static void CheckLimitAutoSleep(iPhysicsJoint *apJoint, const float afMin, const float afMax,
									const float afDist);

	void OnMaxLimit();
	void OnMinLimit();
	void OnNoLimit();

	void CalcSoundFreq(float afSpeed, float *apFreq, float *apVol);

	void LimitEffect(cJointLimitEffect *pEffect);
};

} // namespace hpl

#endif // HPL_PHYSICS_JOINT_H
