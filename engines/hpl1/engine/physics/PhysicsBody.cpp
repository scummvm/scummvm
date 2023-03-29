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

#include "hpl1/engine/physics/PhysicsBody.h"

#include "hpl1/engine/physics/CollideShape.h"
#include "hpl1/engine/physics/PhysicsJoint.h"
#include "hpl1/engine/physics/PhysicsMaterial.h"
#include "hpl1/engine/physics/PhysicsWorld.h"
#include "hpl1/engine/physics/SurfaceData.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/scene/SoundEntity.h"
#include "hpl1/engine/sound/SoundChannel.h"
#include "hpl1/engine/sound/SoundHandler.h"

#include "hpl1/engine/scene/Node3D.h"

#include "common/algorithm.h"
#include "hpl1/engine/math/Math.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iPhysicsBody::iPhysicsBody(const tString &asName, iPhysicsWorld *apWorld, iCollideShape *apShape)
	: iEntity3D(asName) {
	mpWorld = apWorld;
	mpShape = apShape;
	mpNode = NULL;

	// Increment user count for the shape
	apShape->IncUserCount();

	mBoundingVolume.SetLocalMinMax(apShape->GetBoundingVolume().GetMin(),
								   apShape->GetBoundingVolume().GetMax());

	// Set the default material so that body always has a material.
	mpMaterial = mpWorld->GetMaterialFromName("Default");

	mpCharacterBody = NULL;

	mbBlocksSound = false;
	mbBlocksLight = true;

	mpScrapeBody = NULL;
	mpScrapeSoundEntity = NULL;
	mpRollSoundEntity = NULL;
	mbHasImpact = false;
	mbHasSlide = false;

	mlSlideCount = 0;
	mlImpactCount = 0;

	mlBuoyancyId = -1;

	mbCanAttachCharacter = false;

	mpUserData = NULL;

	mbPushedByCharacterGravity = false;

	mbIsCharacter = false;
	mbCollideCharacter = true;

	mbIsRagDoll = false;
	mbCollideRagDoll = true;

	mbCollide = true;

	mbVolatile = false;

	mbDisableAfterSimulation = false;

	mbHasCollision = false;

	m_mtxPrevScrapeMatrix = cMatrixf::Identity;

	// Log("Creating body %s\n",msName.c_str());
}

//-----------------------------------------------------------------------

iPhysicsBody::~iPhysicsBody() {
}

//-----------------------------------------------------------------------

void iPhysicsBody::Destroy() {
	// Log("Start Destroying newton body '%s' %d\n",msName.c_str(),(size_t)this);

	if (mpNode)
		hplDelete(mpNode);
	mpWorld->DestroyShape(mpShape);

	// Log(" Joints\n");
	for (int i = 0; i < (int)mvJoints.size(); i++) {
		iPhysicsJoint *pJoint = mvJoints[i];

		pJoint->RemoveBody(this);

		if (pJoint->GetParentBody() == NULL && pJoint->GetChildBody() == NULL) {
			mpWorld->DestroyJoint(pJoint);
		}

		// Skip removing for now, just makes things messy...
		/*if(	pJoint->GetParentBody() == this ||
			(pJoint->GetParentBody() == NULL && pJoint->GetChildBody()== this) )
		{
			//Log("  Destroy joint %d\n",(size_t)pJoint);
			mpWorld->DestroyJoint(pJoint);
		}
		else if(pJoint->GetParentBody() == this)
		{
			//Remove this body from the joint
			pJoint->RemoveBody(this);
		}*/
	}

	// Log("Deleted body '%s'\n",msName.c_str());

	if (mpScrapeSoundEntity && mpWorld->GetWorld3D()->SoundEntityExists(mpScrapeSoundEntity))
		mpWorld->GetWorld3D()->DestroySoundEntity(mpScrapeSoundEntity);
	if (mpRollSoundEntity && mpWorld->GetWorld3D()->SoundEntityExists(mpRollSoundEntity))
		mpWorld->GetWorld3D()->DestroySoundEntity(mpRollSoundEntity);

	DeleteLowLevel();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cVector3f iPhysicsBody::GetVelocityAtPosition(cVector3f avPos) {
	return GetLinearVelocity() + cMath::Vector3Cross(GetAngularVelocity(), avPos - GetLocalPosition());
}

//-----------------------------------------------------------------------

void iPhysicsBody::AddJoint(iPhysicsJoint *apJoint) {
	mvJoints.push_back(apJoint);
}

iPhysicsJoint *iPhysicsBody::GetJoint(int alIndex) {
	return mvJoints[alIndex];
}

int iPhysicsBody::GetJointNum() {
	return (int)mvJoints.size();
}

void iPhysicsBody::RemoveJoint(iPhysicsJoint *apJoint) {
	Common::Array<iPhysicsJoint *>::iterator it = mvJoints.begin();
	for (; it != mvJoints.end(); ++it) {
		if (*it == apJoint) {
			mvJoints.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------

void iPhysicsBody::UpdateBeforeSimulate(float afTimeStep) {
	// if(msName == "headalive01_throat2") Log("headalive01_throat2 Active: %d\n", IsActive());
	// if(msName == "headalive01_throat") Log("headalive01_throat Active: %d\n", IsActive());

	// Reset that the body has had contact
	SetHasSlide(false);
	SetHasImpact(false);
	mbHasCollision = false;
}

void iPhysicsBody::UpdateAfterSimulate(float afTimeStep) {
	//////////////////////////////////
	// Check disabling from callback
	if (mbDisableAfterSimulation) {
		mbDisableAfterSimulation = false;
		SetEnabled(false);
	}

	//////////////////////////////////
	// Check slide sound
	if (HasSlide() == false) {
		if (GetScrapeSoundEntity()) {
			if (mlSlideCount <= 0) {
				// Log("Stopped scrape %d on body '%s' IN BODY!\n", (size_t)GetScrapeSoundEntity(),
				//												 GetName().c_str());

				if (mpWorld->GetWorld3D())
					if (mpWorld->GetWorld3D()->SoundEntityExists(GetScrapeSoundEntity())) {
						GetScrapeSoundEntity()->FadeOut(5.2f);
					}

				SetScrapeSoundEntity(NULL);
				SetScrapeBody(NULL);
			} else if (mlSlideCount > 0) {
				mlSlideCount--;
			}
		}
	} else {
		mlSlideCount = 8;
	}

	//////////////////////////////////
	// Update rolling sound
	if (mpMaterial)
		mpMaterial->GetSurfaceData()->UpdateRollEffect(this);
}

//-----------------------------------------------------------------------

cNode3D *iPhysicsBody::GetNode() {
	return mpNode;
}
cNode3D *iPhysicsBody::CreateNode() {
	if (mpNode)
		return mpNode;

	mpNode = hplNew(cNode3D, ());
	return mpNode;
}

//-----------------------------------------------------------------------

void iPhysicsBody::AddBodyCallback(iPhysicsBodyCallback *apCallback) {
	mlstBodyCallbacks.push_back(apCallback);
}

void iPhysicsBody::RemoveBodyCallback(iPhysicsBodyCallback *apCallback) {
	tPhysicsBodyCallbackListIt it = mlstBodyCallbacks.begin();
	for (; it != mlstBodyCallbacks.end(); ++it) {
		if (apCallback == *it) {
			mlstBodyCallbacks.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------

bool iPhysicsBody::OnBeginCollision(iPhysicsBody *apBody) {
	if (mlstBodyCallbacks.empty())
		return true;

	bool bReturn = true;

	// Log("Checking before collide callbacks for '%s' ",apBody->GetName().c_str());

	tPhysicsBodyCallbackListIt it = mlstBodyCallbacks.begin();
	for (; it != mlstBodyCallbacks.end(); ++it) {
		iPhysicsBodyCallback *pCallback = *it;

		// Log("Callback %d ,",(size_t)pCallback);

		if (pCallback->OnBeginCollision(this, apBody) == false)
			bReturn = false;
	}

	// Log(" END\n");

	return bReturn;
}

//-----------------------------------------------------------------------

void iPhysicsBody::OnCollide(iPhysicsBody *apBody, cPhysicsContactData *apContactData) {
	mbHasCollision = true;

	if (mlstBodyCallbacks.empty())
		return;

	// Log("Checking on collide callbacks for '%s' ",apBody->GetName().c_str());

	tPhysicsBodyCallbackListIt it = mlstBodyCallbacks.begin();
	for (; it != mlstBodyCallbacks.end(); ++it) {
		iPhysicsBodyCallback *pCallback = *it;

		// Log("Callback %d ,",(size_t)pCallback);

		pCallback->OnCollide(this, apBody, apContactData);
	}

	// Log(" END\n");
}

//-----------------------------------------------------------------------

iPhysicsMaterial *iPhysicsBody::GetMaterial() {
	return mpMaterial;
}

//-----------------------------------------------------------------------

iCollideShape *iPhysicsBody::GetShape() {
	return mpShape;
}

//-----------------------------------------------------------------------

void iPhysicsBody::AddAttachedCharacter(iCharacterBody *apChar) {
	RemoveAttachedCharacter(apChar);

	mlstAttachedCharacters.push_back(apChar);
}

void iPhysicsBody::RemoveAttachedCharacter(iCharacterBody *apChar) {
	Common::List<iCharacterBody *>::iterator it = mlstAttachedCharacters.begin();
	for (; it != mlstAttachedCharacters.end(); ++it) {
		if (apChar == *it) {
			mlstAttachedCharacters.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerializeBase(cSaveData_iCollideShape)
	kSerializeVar(mType, eSerializeType_Int32)
		kSerializeVar(m_mtxOffset, eSerializeType_Matrixf)
			kSerializeVar(mvSize, eSerializeType_Vector3f)
				kEndSerialize()

	//-----------------------------------------------------------------------

	kBeginSerialize(cSaveData_iPhysicsBody, cSaveData_iEntity3D)
		kSerializeClassContainer(mlstShapes, cSaveData_iCollideShape, eSerializeType_Class)

			kSerializeVar(msMaterial, eSerializeType_String)

				kSerializeVar(mbBlocksSound, eSerializeType_Bool)
					kSerializeVar(mbIsCharacter, eSerializeType_Bool)
						kSerializeVar(mbCollideCharacter, eSerializeType_Bool)

							kSerializeVar(mvLinearVelocity, eSerializeType_Vector3f)
								kSerializeVar(mvAngularVelocity, eSerializeType_Vector3f)
									kSerializeVar(mfLinearDamping, eSerializeType_Float32)
										kSerializeVar(mfAngularDamping, eSerializeType_Float32)
											kSerializeVar(mfMaxLinearSpeed, eSerializeType_Float32)
												kSerializeVar(mfMaxAngularSpeed, eSerializeType_Float32)

													kSerializeVar(mfMass, eSerializeType_Float32)

														kSerializeVar(mbEnabled, eSerializeType_Bool)
															kSerializeVar(mbAutoDisable, eSerializeType_Bool)
																kSerializeVar(mbContinuousCollision, eSerializeType_Bool)

																	kSerializeVar(mbGravity, eSerializeType_Bool)

																		kSerializeVar(mbCollide, eSerializeType_Bool)
																			kEndSerialize()

	//-----------------------------------------------------------------------

	static iCollideShape *_CreateShape(cSaveData_iCollideShape *apData, iPhysicsWorld *apWorld) {
	switch ((eCollideShapeType)apData->mType) {
	case eCollideShapeType_Box:
		return apWorld->CreateBoxShape(apData->mvSize, &apData->m_mtxOffset);
	case eCollideShapeType_Sphere:
		return apWorld->CreateSphereShape(apData->mvSize, &apData->m_mtxOffset);
	case eCollideShapeType_Cylinder:
		return apWorld->CreateCylinderShape(apData->mvSize.x, apData->mvSize.y, &apData->m_mtxOffset);
	case eCollideShapeType_Capsule:
		return apWorld->CreateCapsuleShape(apData->mvSize.x, apData->mvSize.y, &apData->m_mtxOffset);
	case eCollideShapeType_Null:
	case eCollideShapeType_ConvexHull:
	case eCollideShapeType_Mesh:
	case eCollideShapeType_Compound:
	case eCollideShapeType_LastEnum:
		break;
	}

	Warning("Invalid shape type %d!\n", apData->mType);

	return NULL;
}

static iCollideShape *CreateCollideShapeFromSave(cContainerList<cSaveData_iCollideShape> *apShapeList,
												 iPhysicsWorld *apWorld) {
	cContainerListIterator<cSaveData_iCollideShape> it = apShapeList->GetIterator();
	if (apShapeList->Size() == 1) {
		return _CreateShape(&it.Next(), apWorld);
	} else {
		tCollideShapeVec vShapes;
		while (it.HasNext()) {
			vShapes.push_back(_CreateShape(&it.Next(), apWorld));
		}

		return apWorld->CreateCompundShape(vShapes);
	}
}

iSaveObject *cSaveData_iPhysicsBody::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	iPhysicsWorld *pWorld = apGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	// Get the collider
	iCollideShape *pShape = CreateCollideShapeFromSave(&mlstShapes, pWorld);
	if (pShape == NULL)
		return NULL;

	iPhysicsBody *pBody = pWorld->CreateBody(msName, pShape);

	return pBody;
}

//-----------------------------------------------------------------------

int cSaveData_iPhysicsBody::GetSaveCreatePrio() {
	return 0;
}

//-----------------------------------------------------------------------

iSaveData *iPhysicsBody::CreateSaveData() {
	return hplNew(cSaveData_iPhysicsBody, ());
}

//-----------------------------------------------------------------------

void iPhysicsBody::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(iPhysicsBody);

	// Collider
	CreateSaveCollideShapes(&pData->mlstShapes);

	// Material
	pData->msMaterial = mpMaterial == NULL ? "" : mpMaterial->GetName();

	// Save vars
	kSaveData_SaveTo(mbBlocksSound);
	kSaveData_SaveTo(mbIsCharacter);
	kSaveData_SaveTo(mbCollideCharacter);

	// Save interface properties
	pData->mvLinearVelocity = GetLinearVelocity();
	pData->mvAngularVelocity = GetAngularVelocity();
	pData->mfLinearDamping = GetLinearDamping();
	pData->mfAngularDamping = GetAngularDamping();
	pData->mfMaxLinearSpeed = GetMaxLinearSpeed();
	pData->mfMaxAngularSpeed = GetMaxAngularSpeed();

	pData->mfMass = GetMass();

	pData->mbEnabled = GetEnabled();
	pData->mbAutoDisable = GetAutoDisable();
	pData->mbContinuousCollision = GetContinuousCollision();

	pData->mbGravity = GetGravity();

	pData->mbCollide = GetCollide();
}

//-----------------------------------------------------------------------

void iPhysicsBody::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(iPhysicsBody);

	// Material
	if (pData->msMaterial != "") {
		iPhysicsMaterial *pMat = mpWorld->GetMaterialFromName(pData->msMaterial);
		if (pMat)
			SetMaterial(pMat);
	}

	// Save vars
	kSaveData_LoadFrom(mbBlocksSound);
	kSaveData_LoadFrom(mbIsCharacter);
	kSaveData_LoadFrom(mbCollideCharacter);

	// Save interface properties
	SetLinearVelocity(pData->mvLinearVelocity);
	SetAngularVelocity(pData->mvAngularVelocity);
	SetLinearDamping(pData->mfLinearDamping);
	SetAngularDamping(pData->mfAngularDamping);
	SetMaxLinearSpeed(pData->mfMaxLinearSpeed);
	SetMaxAngularSpeed(pData->mfMaxAngularSpeed);

	SetMass(pData->mfMass);

	SetEnabled(pData->mbEnabled);
	SetAutoDisable(pData->mbAutoDisable);
	SetContinuousCollision(pData->mbContinuousCollision);

	SetGravity(pData->mbGravity);

	SetCollide(pData->mbCollide);
}

//-----------------------------------------------------------------------

void iPhysicsBody::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(iPhysicsBody);
}

//-----------------------------------------------------------------------

void iPhysicsBody::CreateSaveCollideShapes(cContainerList<cSaveData_iCollideShape> *apShapeList) {
	if (mpShape->GetType() == eCollideShapeType_Compound) {
		for (int i = 0; i < mpShape->GetSubShapeNum(); ++i) {
			iCollideShape *pShape = mpShape->GetSubShape(i);

			cSaveData_iCollideShape Shape;
			Shape.mType = (int)pShape->GetType();
			Shape.m_mtxOffset = pShape->GetOffset();
			Shape.mvSize = pShape->GetSize();

			apShapeList->Add(Shape);
		}
	} else {
		cSaveData_iCollideShape Shape;
		Shape.mType = (int)mpShape->GetType();
		Shape.m_mtxOffset = mpShape->GetOffset();
		Shape.mvSize = mpShape->GetSize();

		apShapeList->Add(Shape);
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
