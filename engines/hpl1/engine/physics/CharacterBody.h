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

#ifndef HPL_CHARACTER_BODY_H
#define HPL_CHARACTER_BODY_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

namespace hpl {

class iPhysicsWorld;
class iCollideShape;
class iPhysicsBody;
class cCamera3D;
class iCharacterBody;
class iEntity3D;

enum eCharDir {
	eCharDir_Forward = 0,
	eCharDir_Right = 1,
	eCharDir_LastEnum = 2
};

class iCharacterBodyCallback {
public:
	virtual ~iCharacterBodyCallback() = default;
	virtual void OnHitGround(iCharacterBody *apCharBody, const cVector3f &avVel) = 0;
	virtual void OnGravityCollide(iCharacterBody *apCharBody, iPhysicsBody *apCollideBody,
								  cCollideData *apCollideData) = 0;
};

//------------------------------------------------

class cCharacterBodyRay : public iPhysicsRayCallback {
public:
	cCharacterBodyRay();

	void Clear();
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	float mfMinDist;
	bool mbCollide;
};

//------------------------------------------------

class cCharacterBodyCollideGravity : public iPhysicsWorldCollisionCallback {
public:
	cCharacterBodyCollideGravity();

	void OnCollision(iPhysicsBody *apBody, cCollideData *apCollideData);

	iCharacterBody *mpCharBody;
};

//------------------------------------------------

class cCharacterBodyCollidePush : public iPhysicsWorldCollisionCallback {
public:
	cCharacterBodyCollidePush();

	void OnCollision(iPhysicsBody *apBody, cCollideData *apCollideData);

	iCharacterBody *mpCharBody;
};

//-----------------------------------

kSaveData_BaseClass(iCharacterBody) {
	kSaveData_ClassInit(iCharacterBody) public : tString msName;

	float mfMass;
	bool mbGravityActive;
	float mfMaxGravitySpeed;

	bool mbActive;

	bool mbCollideCharacter;

	cVector3f mvPosition;
	cVector3f mvLastPosition;
	float mfMaxPosMoveSpeed[2];
	float mfMaxNegMoveSpeed[2];
	float mfMoveSpeed[2];
	float mfMoveAcc[2];
	float mfMoveDeacc[2];
	bool mbMoving[2];

	float mfPitch;
	float mfYaw;

	bool mbOnGround;

	float mfMaxPushMass;
	float mfPushForce;
	bool mbPushIn2D;

	cVector3f mvForce;
	cVector3f mvVelolcity;

	cVector3f mvSize;

	cMatrixf m_mtxMove;

	int mlEntityId;
	cMatrixf m_mtxEntityOffset;
	int mlEntitySmoothPosNum;

	float mfMaxStepHeight;
	float mfStepClimbSpeed;
	float mfClimbForwardMul;
	float mfClimbHeightAdd;
	bool mbClimbing;

	float mfGroundFriction;
	float mfAirFriction;

	int mlBodyId;

	cContainerList<int> mvExtraBodyIds;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//------------------------------------------------

class iCharacterBody : public iSaveObject {
	typedef iSaveObject super;
	friend class cSaveData_iCharacterBody;
	friend class cCharacterBodyCollideGravity;

public:
	iCharacterBody(const tString &asName, iPhysicsWorld *apWorld, const cVector3f avSize);
	virtual ~iCharacterBody();

	const tString &GetName() { return msName; }

	///////////////////////////////////////
	// Properties

	float GetMass();
	void SetMass(float afMass);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	cVector3f GetSize();

	void SetCollideCharacter(bool abX);
	bool GetCollideCharacter() { return mbCollideCharacter; }

	void SetTestCollision(bool abX) { mbTestCollision = abX; }
	bool GetTestCollision() { return mbTestCollision; }

	void SetMaxPositiveMoveSpeed(eCharDir aDir, float afX);
	float GetMaxPositiveMoveSpeed(eCharDir aDir);
	void SetMaxNegativeMoveSpeed(eCharDir aDir, float afX);
	float GetMaxNegativeMoveSpeed(eCharDir aDir);

	void SetMoveSpeed(eCharDir aDir, float afX);
	float GetMoveSpeed(eCharDir aDir);
	void SetMoveAcc(eCharDir aDir, float afX);
	float GetMoveAcc(eCharDir aDir);
	void SetMoveDeacc(eCharDir aDir, float afX);
	float GetMoveDeacc(eCharDir aDir);

	cVector3f GetVelocity(float afFrameTime);

	void SetPosition(const cVector3f &avPos, bool abSmooth = false);
	const cVector3f &GetPosition();
	const cVector3f &GetLastPosition();
	void SetFeetPosition(const cVector3f &avPos, bool abSmooth = false);
	cVector3f GetFeetPosition();

	void SetYaw(float afX);
	void AddYaw(float afX);
	float GetYaw();
	void SetPitch(float afX);
	void AddPitch(float afX);
	float GetPitch();

	cVector3f GetForward();
	cVector3f GetRight();
	cVector3f GetUp();

	cMatrixf &GetMoveMatrix();

	void SetGravityActive(bool abX);
	bool GravityIsActive();
	void SetMaxGravitySpeed(float afX);
	float GetMaxGravitySpeed();

	bool GetCustomGravityActive();
	void SetCustomGravityActive(bool abX);
	void SetCustomGravity(const cVector3f &avCustomGravity);
	cVector3f GetCustomGravity();

	void SetMaxPushMass(float afX) { mfMaxPushMass = afX; }
	void SetPushForce(float afX) { mfPushForce = afX; }
	float GetMaxPushMass() { return mfMaxPushMass; }
	float GetPushForce() { return mfPushForce; }

	bool GetPushIn2D() { return mbPushIn2D; }
	void SetPushIn2D(bool abX) { mbPushIn2D = abX; }

	void AddForceVelocity(const cVector3f &avVel) { mvVelolcity += avVel; }
	void SetForceVelocity(const cVector3f &avVel) { mvVelolcity = avVel; }
	cVector3f GetForceVelocity() { return mvVelolcity; }

	int AddExtraSize(const cVector3f &avSize);
	void SetActiveSize(int alNum);

	///////////////////////////////////////
	// Actions
	void SetForce(const cVector3f &avForce);
	void AddForce(const cVector3f &avForce);
	cVector3f GetForce();

	void Move(eCharDir aDir, float afMul, float afTimeStep);

	void Update(float afTimeStep);

	///////////////////////////////////////
	// Other
	void SetCamera(cCamera3D *apCam);
	cCamera3D *GetCamera();
	void SetCameraPosAdd(const cVector3f &avAdd);
	cVector3f GetCameraPosAdd();
	void SetCameraSmoothPosNum(int alNum) { mlCameraSmoothPosNum = alNum; }
	int GetCameraSmoothPosNum() { return mlCameraSmoothPosNum; }

	void SetEntity(iEntity3D *apEntity);
	iEntity3D *GetEntity();

	void SetEntityOffset(const cMatrixf &a_mtxOffset);
	const cMatrixf &GetEntityOffset();

	void SetEntityPostOffset(const cMatrixf &a_mtxOffset);
	const cMatrixf &GetEntityPostOffset();

	void SetEntitySmoothPosNum(int alNum) { mlEntitySmoothPosNum = alNum; }
	int GetEntitySmoothPosNum() { return mlEntitySmoothPosNum; }

	void SetUserData(void *apUserData) { mpUserData = apUserData; }
	void *GetUserData() { return mpUserData; }

	void SetCallback(iCharacterBodyCallback *apCallback) { mpCallback = apCallback; }

	void SetEnableNearbyBodies(bool abX) { mbEnableNearbyBodies = abX; }
	bool GetEnableNearbyBodies() { return mbEnableNearbyBodies; }

	iPhysicsBody *GetBody() { return mpBody; }
	iCollideShape *GetShape();

	iPhysicsBody *GetExtraBody(size_t alIdx) { return mvExtraBodies[alIdx]; }

	bool IsOnGround();

	float GetMaxStepSize() { return mfMaxStepHeight; }
	void SetMaxStepSize(float afSize) { mfMaxStepHeight = afSize; }

	void SetStepClimbSpeed(float afX) { mfStepClimbSpeed = afX; }
	float GetStepClimbSpeed() { return mfStepClimbSpeed; }

	void SetAccurateClimbing(bool abX) { mbAccurateClimbing = abX; }
	bool GetAccurateClimbing() { return mbAccurateClimbing; }

	void SetClimbForwardMul(float afX) { mfClimbForwardMul = afX; }
	float GetClimbForwardMul() { return mfClimbForwardMul; }

	void SetClimbHeightAdd(float afX) { mfClimbHeightAdd = afX; }
	float GetClimbHeightAdd() { return mfClimbHeightAdd; }

	void SetGroundFriction(float afX) { mfGroundFriction = afX; }
	float GetGroundFriction() { return mfGroundFriction; }

	void SetAirFriction(float afX) { mfAirFriction = afX; }
	float GetAirFriction() { return mfAirFriction; }

	bool IsClimbing() { return mbClimbing; }

	void SetAttachedBody(iPhysicsBody *apBody);
	iPhysicsBody *GetAttachedBody() { return mpAttachedBody; }

	// O=nly sue when you know what you are doing, Update calls these
	void UpdateMoveMarix();

	void UpdateCamera();
	void UpdateEntity();

	void UpdateAttachment();

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	tString msName;

	float mfMass;

	bool mbActive;

	bool mbCollideCharacter;

	bool mbTestCollision;

	bool mbGravityActive;
	float mfMaxGravitySpeed;
	bool mbCustomGravity;
	cVector3f mvCustomGravity;

	cVector3f mvPosition;
	cVector3f mvLastPosition;

	float mfMaxPosMoveSpeed[2];
	float mfMaxNegMoveSpeed[2];

	float mfMoveSpeed[2];
	float mfMoveAcc[2];
	float mfMoveDeacc[2];
	bool mbMoving[2];

	float mfPitch;
	float mfYaw;

	bool mbOnGround;

	float mfMaxPushMass;
	float mfPushForce;
	bool mbPushIn2D;

	float mfCheckStepClimbCount;
	float mfCheckStepClimbInterval;

	cVector3f mvForce;
	cVector3f mvVelolcity;

	cVector3f mvSize;

	cMatrixf m_mtxMove;

	cCamera3D *mpCamera;
	cVector3f mvCameraPosAdd;
	int mlCameraSmoothPosNum;
	tVector3fList mlstCameraPos;

	iEntity3D *mpEntity;
	cMatrixf m_mtxEntityOffset;
	cMatrixf m_mtxEntityPostOffset;
	int mlEntitySmoothPosNum;
	tVector3fList mlstEntityPos;

	float mfGroundFriction;
	float mfAirFriction;

	void *mpUserData;

	iPhysicsBody *mpAttachedBody;
	cMatrixf m_mtxAttachedPrevMatrix;
	bool mbAttachmentJustAdded;

	iCharacterBodyCallback *mpCallback;

	cCharacterBodyRay *mpRayCallback;

	cCharacterBodyCollideGravity *mpCollideCallbackGravity;
	cCharacterBodyCollidePush *mpCollideCallbackPush;

	float mfMaxStepHeight;
	float mfStepClimbSpeed;
	float mfClimbForwardMul;
	float mfClimbHeightAdd;
	bool mbClimbing;
	bool mbAccurateClimbing;

	bool mbEnableNearbyBodies;

	iPhysicsBody *mpBody;
	iPhysicsWorld *mpWorld;

	Common::Array<iPhysicsBody *> mvExtraBodies;
};

} // namespace hpl

#endif // HPL_CHARACTER_BODY_H
