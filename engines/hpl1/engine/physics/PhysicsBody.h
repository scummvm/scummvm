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

#ifndef HPL_PHYSICS_BODY_H
#define HPL_PHYSICS_BODY_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/scene/Entity3D.h"

namespace hpl {

class iPhysicsWorld;
class iCollideShape;
class iPhysicsMaterial;
class iLowLevelGraphics;
class cNode3D;
class cSoundEntity;
class iPhysicsJoint;
class cPhysicsContactData;
class iCharacterBody;

//------------------------------------------

class iPhysicsBody;
class iPhysicsBodyCallback {
public:
	virtual ~iPhysicsBodyCallback() {}
	virtual bool OnBeginCollision(iPhysicsBody *apBody, iPhysicsBody *apCollideBody) = 0;
	virtual void OnCollide(iPhysicsBody *apBody, iPhysicsBody *apCollideBody,
						   cPhysicsContactData *apContactData) = 0;
};

typedef Common::List<iPhysicsBodyCallback *> tPhysicsBodyCallbackList;
typedef tPhysicsBodyCallbackList::iterator tPhysicsBodyCallbackListIt;

//------------------------------------------

class cSaveData_iCollideShape : public iSerializable {
	kSerializableClassInit(cSaveData_iCollideShape) public : int mType;
	cMatrixf m_mtxOffset;
	cVector3f mvSize;
};

//------------------------------------------

kSaveData_ChildClass(iEntity3D, iPhysicsBody) {
	kSaveData_ClassInit(iPhysicsBody) public : cContainerList<cSaveData_iCollideShape> mlstShapes;

	tString msMaterial;

	bool mbBlocksSound;
	bool mbIsCharacter;
	bool mbCollideCharacter;

	cVector3f mvLinearVelocity;
	cVector3f mvAngularVelocity;
	float mfLinearDamping;
	float mfAngularDamping;
	float mfMaxLinearSpeed;
	float mfMaxAngularSpeed;

	float mfMass;

	bool mbEnabled;
	bool mbAutoDisable;
	bool mbContinuousCollision;

	bool mbGravity;

	bool mbCollide;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//------------------------------------------

struct cPhysicsBody_Buoyancy {
	cPhysicsBody_Buoyancy() : mbActive(false), mfDensity(1),
							  mfLinearViscosity(1), mfAngularViscosity(1) {}

	bool mbActive;

	float mfDensity;
	float mfLinearViscosity;
	float mfAngularViscosity;

	cPlanef mSurface;
};

//------------------------------------------

class iPhysicsBody : public iEntity3D {
	typedef iEntity3D super;

public:
	iPhysicsBody(const tString &asName, iPhysicsWorld *apWorld, iCollideShape *apShape);
	virtual ~iPhysicsBody();

	void Destroy();

	virtual void SetMaterial(iPhysicsMaterial *apMaterial) = 0;
	iPhysicsMaterial *GetMaterial();

	cNode3D *GetNode();
	cNode3D *CreateNode();

	iCollideShape *GetShape();

	void AddJoint(iPhysicsJoint *apJoint);
	iPhysicsJoint *GetJoint(int alIndex);
	int GetJointNum();
	void RemoveJoint(iPhysicsJoint *apJoint);

	virtual void SetLinearVelocity(const cVector3f &avVel) = 0;
	virtual cVector3f GetLinearVelocity() const = 0;
	virtual void SetAngularVelocity(const cVector3f &avVel) = 0;
	virtual cVector3f GetAngularVelocity() const = 0;
	virtual void SetLinearDamping(float afDamping) = 0;
	virtual float GetLinearDamping() const = 0;
	virtual void SetAngularDamping(float afDamping) = 0;
	virtual float GetAngularDamping() const = 0;
	virtual void SetMaxLinearSpeed(float afSpeed) = 0;
	virtual float GetMaxLinearSpeed() const = 0;
	virtual void SetMaxAngularSpeed(float afDamping) = 0;
	virtual float GetMaxAngularSpeed() const = 0;
	virtual cMatrixf GetInertiaMatrix() = 0;

	cVector3f GetVelocityAtPosition(cVector3f avPos);

	virtual void SetMass(float afMass) = 0;
	virtual float GetMass() const = 0;
	virtual void SetMassCentre(const cVector3f &avCentre) = 0;
	virtual cVector3f GetMassCentre() const = 0;

	virtual void AddForce(const cVector3f &avForce) = 0;
	virtual void AddForceAtPosition(const cVector3f &avForce, const cVector3f &avPos) = 0;
	virtual void AddTorque(const cVector3f &avTorque) = 0;
	virtual void AddImpulse(const cVector3f &avImpulse) = 0;
	virtual void AddImpulseAtPosition(const cVector3f &avImpulse, const cVector3f &avPos) = 0;

	virtual void SetEnabled(bool abEnabled) = 0;
	virtual bool GetEnabled() const = 0;
	virtual void SetAutoDisable(bool abEnabled) = 0;
	virtual bool GetAutoDisable() const = 0;
#if 0
	virtual void SetAutoDisableLinearThreshold(float afThresold) = 0;
	virtual float GetAutoDisableLinearThreshold() const = 0;
	virtual void SetAutoDisableAngularThreshold(float afThresold) = 0;
	virtual float GetAutoDisableAngularThreshold() const = 0;
	virtual void SetAutoDisableNumSteps(int alNum) = 0;
	virtual int GetAutoDisableNumSteps() const = 0;
#endif
	virtual void SetContinuousCollision(bool abOn) = 0;
	virtual bool GetContinuousCollision() = 0;

	virtual void SetGravity(bool abEnabled) = 0;
	virtual bool GetGravity() const = 0;

	virtual void RenderDebugGeometry(iLowLevelGraphics *apLowLevel, const cColor &aColor) = 0;

	void UpdateBeforeSimulate(float afTimeStep);
	void UpdateAfterSimulate(float afTimeStep);

	cBoundingVolume *GetBV() { return &mBoundingVolume; }

	void SetBlocksSound(bool abX) { mbBlocksSound = abX; }
	bool GetBlocksSound() { return mbBlocksSound; }

	void SetBlocksLight(bool abX) { mbBlocksLight = abX; }
	bool GetBlocksLight() { return mbBlocksLight; }

	void SetScrapeSoundEntity(cSoundEntity *apEntity) { mpScrapeSoundEntity = apEntity; }
	cSoundEntity *GetScrapeSoundEntity() { return mpScrapeSoundEntity; }
	void SetScrapeBody(iPhysicsBody *apBody) { mpScrapeBody = apBody; }
	iPhysicsBody *GetScrapeBody() { return mpScrapeBody; }
	const cMatrixf &GetPreveScrapeMatrix() { return m_mtxPrevScrapeMatrix; }
	void SetPreveScrapeMatrix(const cMatrixf &a_mtxMtx) { m_mtxPrevScrapeMatrix = a_mtxMtx; }

	void SetRollSoundEntity(cSoundEntity *apEntity) { mpRollSoundEntity = apEntity; }
	cSoundEntity *GetRollSoundEntity() { return mpRollSoundEntity; }

	void SetHasImpact(bool abX) { mbHasImpact = abX; }
	bool HasImpact() { return mbHasImpact; }
	void SetHasSlide(bool abX) { mbHasSlide = abX; }
	bool HasSlide() { return mbHasSlide; }

	bool HasCollision() { return mbHasCollision; }

	void SetUserData(void *apUserData) { mpUserData = apUserData; }
	void *GetUserData() { return mpUserData; }

	void AddBodyCallback(iPhysicsBodyCallback *apCallback);
	void RemoveBodyCallback(iPhysicsBodyCallback *apCallback);

	bool OnBeginCollision(iPhysicsBody *apBody);
	void OnCollide(iPhysicsBody *apBody, cPhysicsContactData *apContactData);

	void SetCollide(bool abX) { mbCollide = abX; }
	bool GetCollide() { return mbCollide; }

	void SetIsCharacter(bool abX) { mbIsCharacter = abX; }
	bool IsCharacter() { return mbIsCharacter; }

	void SetCollideCharacter(bool abX) { mbCollideCharacter = abX; }
	bool GetCollideCharacter() { return mbCollideCharacter; }

	void SetCharacterBody(iCharacterBody *apCharBody) { mpCharacterBody = apCharBody; }
	iCharacterBody *GetCharacterBody() { return mpCharacterBody; }

	void SetIsRagDoll(bool abX) { mbIsRagDoll = abX; }
	bool IsRagDoll() { return mbIsRagDoll; }

	void SetCollideRagDoll(bool abX) { mbCollideRagDoll = abX; }
	bool GetCollideRagDoll() { return mbCollideRagDoll; }

	void SetVolatile(bool abX) { mbVolatile = abX; }
	bool IsVolatile() { return mbVolatile; }

	void SetPushedByCharacterGravity(bool abX) { mbPushedByCharacterGravity = abX; }
	bool GetPushedByCharacterGravity() { return mbPushedByCharacterGravity; }

	void SetBuoyancyId(int alX) { mlBuoyancyId = alX; }
	void SetBuoyancyActive(bool abX) { mBuoyancy.mbActive = abX; }
	void SetBuoyancyDensity(float afX) { mBuoyancy.mfDensity = afX; }
	void SetBuoyancyLinearViscosity(float afX) { mBuoyancy.mfLinearViscosity = afX; }
	void SetBuoyancyAngularViscosity(float afX) { mBuoyancy.mfAngularViscosity = afX; }
	void SetBuoyancySurface(const cPlanef &aP) { mBuoyancy.mSurface = aP; }

	int GetBuoyancyId() { return mlBuoyancyId; }
	bool GetBuoyancyActive() { return mBuoyancy.mbActive; }
	float GetBuoyancyDensity() { return mBuoyancy.mfDensity; }
	float GetBuoyancyLinearViscosity() { return mBuoyancy.mfLinearViscosity; }
	float GetBuoyancyAngularViscosity() { return mBuoyancy.mfAngularViscosity; }
	cPlanef SetBuoyancySurface() { return mBuoyancy.mSurface; }

	void SetCanAttachCharacter(bool abX) { mbCanAttachCharacter = abX; }
	bool GetCanAttachCharacter() { return mbCanAttachCharacter; }
	void AddAttachedCharacter(iCharacterBody *apChar);
	void RemoveAttachedCharacter(iCharacterBody *apChar);

	iPhysicsWorld *GetWorld() { return mpWorld; }

	void DisableAfterSimulation() { mbDisableAfterSimulation = true; }

	// Entity implementation
	tString GetEntityType() { return "Body"; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

	virtual void DeleteLowLevel() = 0;

protected:
	void CreateSaveCollideShapes(cContainerList<cSaveData_iCollideShape> *apShapeList);

	iPhysicsWorld *mpWorld;
	iCollideShape *mpShape;
	iPhysicsMaterial *mpMaterial;
	cNode3D *mpNode;

	iCharacterBody *mpCharacterBody;

	Common::Array<iPhysicsJoint *> mvJoints;

	Common::List<iCharacterBody *> mlstAttachedCharacters;

	iPhysicsBody *mpScrapeBody;
	cSoundEntity *mpScrapeSoundEntity;
	cSoundEntity *mpRollSoundEntity;
	cMatrixf m_mtxPrevScrapeMatrix;
	bool mbHasImpact;
	bool mbHasSlide;
	int mlSlideCount;
	int mlImpactCount;

	bool mbPushedByCharacterGravity;

	bool mbBlocksSound;
	bool mbBlocksLight;
	bool mbIsCharacter;
	bool mbCollideCharacter;
	bool mbIsRagDoll;
	bool mbCollideRagDoll;
	bool mbVolatile;

	bool mbCanAttachCharacter;

	cPhysicsBody_Buoyancy mBuoyancy;
	int mlBuoyancyId;

	bool mbDisableAfterSimulation;

	bool mbHasCollision;

	tPhysicsBodyCallbackList mlstBodyCallbacks;

	void *mpUserData;

	bool mbCollide;
};

} // namespace hpl

#endif // HPL_PHYSICS_BODY_H
