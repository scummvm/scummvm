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

#include "hpl1/engine/impl/PhysicsBodyNewton.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/impl/CollideShapeNewton.h"
#include "hpl1/engine/impl/PhysicsMaterialNewton.h"
#include "hpl1/engine/impl/PhysicsWorldNewton.h"
#include "hpl1/engine/libraries/angelscript/angelscript.h"
#include "hpl1/engine/libraries/newton/Newton.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/math/Vector3.h"
#include "hpl1/engine/scene/Node3D.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

bool cPhysicsBodyNewton::mbUseCallback = true;

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPhysicsBodyNewton::cPhysicsBodyNewton(const tString &asName, iPhysicsWorld *apWorld, iCollideShape *apShape)
	: iPhysicsBody(asName, apWorld, apShape) {
	cPhysicsWorldNewton *pWorldNewton = static_cast<cPhysicsWorldNewton *>(apWorld);
	cCollideShapeNewton *pShapeNewton = static_cast<cCollideShapeNewton *>(apShape);

	mpNewtonWorld = pWorldNewton->GetNewtonWorld();
	mpNewtonBody = NewtonCreateBody(pWorldNewton->GetNewtonWorld(),
									pShapeNewton->GetNewtonCollision(), cMatrixf::Identity.v);

	mpCallback = hplNew(cPhysicsBodyNewtonCallback, ());

	AddCallback(mpCallback);

	// Setup the callbacks and set this body as user data
	// This is so that the transform gets updated and
	// to add gravity, forces and user sink.
	NewtonBodySetForceAndTorqueCallback(mpNewtonBody, OnUpdateCallback);
	NewtonBodySetTransformCallback(mpNewtonBody, OnTransformCallback);
	NewtonBodySetUserData(mpNewtonBody, this);

	// Set default property settings
	mbGravity = true;

	mfMaxLinearSpeed = 0;
	mfMaxAngularSpeed = 0;
	mfMass = 0;

	mfAutoDisableLinearThreshold = 0.01f;
	mfAutoDisableAngularThreshold = 0.01f;
	mlAutoDisableNumSteps = 10;

	// Log("Creating newton body '%s' %d\n",msName.c_str(),(size_t)this);
}

//-----------------------------------------------------------------------

cPhysicsBodyNewton::~cPhysicsBodyNewton() {
	// Log(" Destroying newton body '%s' %d\n",msName.c_str(),(size_t)this);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::DeleteLowLevel() {
	// Log(" Newton body %d\n", (size_t)mpNewtonBody);
	NewtonDestroyBody(mpNewtonWorld, mpNewtonBody);
	// Log(" Callback\n");
	hplDelete(mpCallback);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CALLBACK METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPhysicsBodyNewtonCallback::OnTransformUpdate(iEntity3D *apEntity) {
	if (cPhysicsBodyNewton::mbUseCallback == false)
		return;

	cPhysicsBodyNewton *pRigidBody = static_cast<cPhysicsBodyNewton *>(apEntity);
	cMatrixf mTemp = apEntity->GetLocalMatrix().GetTranspose();
	NewtonBodySetMatrix(pRigidBody->mpNewtonBody, mTemp.v);

	if (pRigidBody->mpNode)
		pRigidBody->mpNode->SetMatrix(apEntity->GetLocalMatrix());
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetMaterial(iPhysicsMaterial *apMaterial) {
	mpMaterial = apMaterial;

	if (apMaterial == nullptr)
		return;

	cPhysicsMaterialNewton *pNewtonMat = static_cast<cPhysicsMaterialNewton *>(mpMaterial);

	NewtonBodySetMaterialGroupID(mpNewtonBody, pNewtonMat->GetId());
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetLinearVelocity(const cVector3f &avVel) {
	VEC3_CONST_ARRAY(vel, avVel);
	NewtonBodySetVelocity(mpNewtonBody, vel);
}
cVector3f cPhysicsBodyNewton::GetLinearVelocity() const {
	float vel[3];
	NewtonBodyGetVelocity(mpNewtonBody, vel);
	return cVector3f::fromArray(vel);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAngularVelocity(const cVector3f &avVel) {
	VEC3_CONST_ARRAY(vel, avVel);
	NewtonBodySetOmega(mpNewtonBody, vel);
}
cVector3f cPhysicsBodyNewton::GetAngularVelocity() const {
	float vel[3];
	NewtonBodyGetOmega(mpNewtonBody, vel);
	return cVector3f::fromArray(vel);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetLinearDamping(float afDamping) {
	NewtonBodySetLinearDamping(mpNewtonBody, afDamping);
}
float cPhysicsBodyNewton::GetLinearDamping() const {
	return NewtonBodyGetLinearDamping(mpNewtonBody);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAngularDamping(float afDamping) {
	float damp[3] = {afDamping, afDamping, afDamping};
	NewtonBodySetAngularDamping(mpNewtonBody, damp);
}
float cPhysicsBodyNewton::GetAngularDamping() const {
	float fDamp[3];
	NewtonBodyGetAngularDamping(mpNewtonBody, fDamp);
	return fDamp[0];
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetMaxLinearSpeed(float afSpeed) {
	mfMaxLinearSpeed = afSpeed;
}
float cPhysicsBodyNewton::GetMaxLinearSpeed() const {
	return mfMaxLinearSpeed;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetMaxAngularSpeed(float afDamping) {
	mfMaxAngularSpeed = afDamping;
}
float cPhysicsBodyNewton::GetMaxAngularSpeed() const {
	return mfMaxAngularSpeed;
}

//-----------------------------------------------------------------------

cMatrixf cPhysicsBodyNewton::GetInertiaMatrix() {
	float fIxx, fIyy, fIzz, fMass;

	NewtonBodyGetMassMatrix(mpNewtonBody, &fMass, &fIxx, &fIyy, &fIzz);

	cMatrixf mtxRot = GetLocalMatrix().GetRotation();
	cMatrixf mtxTransRot = mtxRot.GetTranspose();
	cMatrixf mtxI(fIxx, 0, 0, 0,
				  0, fIyy, 0, 0,
				  0, 0, fIzz, 0,
				  0, 0, 0, 1);

	return cMath::MatrixMul(cMath::MatrixMul(mtxRot, mtxI), mtxTransRot);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetMass(float afMass) {
	cCollideShapeNewton *pShapeNewton = static_cast<cCollideShapeNewton *>(mpShape);

	float inertia[3];
	float offset[3];
	NewtonConvexCollisionCalculateInertialMatrix(pShapeNewton->GetNewtonCollision(),
												 inertia, offset);

	cVector3f vInertia = cVector3f::fromArray(inertia) * afMass; // = pShapeNewton->GetInertia(afMass);

	NewtonBodySetCentreOfMass(mpNewtonBody, offset);

	NewtonBodySetMassMatrix(mpNewtonBody, afMass, vInertia.x, vInertia.y, vInertia.z);
	mfMass = afMass;
}
float cPhysicsBodyNewton::GetMass() const {
	return mfMass;
}

void cPhysicsBodyNewton::SetMassCentre(const cVector3f &avCentre) {
	VEC3_CONST_ARRAY(ctr, avCentre);
	NewtonBodySetCentreOfMass(mpNewtonBody, ctr);
}

cVector3f cPhysicsBodyNewton::GetMassCentre() const {
	float center[3];
	NewtonBodyGetCentreOfMass(mpNewtonBody, center);
	return cVector3f::fromArray(center);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::AddForce(const cVector3f &avForce) {
	mvTotalForce += avForce;
	SetEnabled(true);

	// Log("Added force %s\n",avForce.ToString().c_str());
}

void cPhysicsBodyNewton::AddForceAtPosition(const cVector3f &avForce, const cVector3f &avPos) {
	mvTotalForce += avForce;

	cVector3f vLocalPos = avPos - GetLocalPosition();
	cVector3f vMassCentre = GetMassCentre();
	if (vMassCentre != cVector3f(0, 0, 0)) {
		vMassCentre = cMath::MatrixMul(GetLocalMatrix().GetRotation(), vMassCentre);
		vLocalPos -= vMassCentre;
	}

	cVector3f vTorque = cMath::Vector3Cross(vLocalPos, avForce);

	mvTotalTorque += vTorque;
	SetEnabled(true);

	// Log("Added force %s\n",avForce.ToString().c_str());
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::AddTorque(const cVector3f &avTorque) {
	mvTotalTorque += avTorque;
	SetEnabled(true);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::AddImpulse(const cVector3f &avImpulse) {
	cVector3f vMassCentre = GetMassCentre();
	VEC3_CONST_ARRAY(impulse, avImpulse);
	if (vMassCentre != cVector3f(0, 0, 0)) {
		cVector3f vCentreOffset = cMath::MatrixMul(GetWorldMatrix().GetRotation(),
												   vMassCentre);
		VEC3_CONST_ARRAY(worldPosition, (GetWorldPosition() + vCentreOffset));
		NewtonBodyAddImpulse(mpNewtonBody, impulse, worldPosition);
	} else {
		VEC3_CONST_ARRAY(worldPosition, GetWorldPosition());
		NewtonBodyAddImpulse(mpNewtonBody, impulse, worldPosition);
	}
}
void cPhysicsBodyNewton::AddImpulseAtPosition(const cVector3f &avImpulse, const cVector3f &avPos) {
	VEC3_CONST_ARRAY(impulse, avImpulse);
	VEC3_CONST_ARRAY(pos, avPos);
	NewtonBodyAddImpulse(mpNewtonBody, impulse, pos);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetEnabled(bool abEnabled) {
	NewtonBodySetFreezeState(mpNewtonBody, !abEnabled);
}
bool cPhysicsBodyNewton::GetEnabled() const {
	return NewtonBodyGetSleepState(mpNewtonBody) == 0 ? false : true;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAutoDisable(bool abEnabled) {
	NewtonBodySetAutoSleep(mpNewtonBody, abEnabled);
}
bool cPhysicsBodyNewton::GetAutoDisable() const {
	return NewtonBodyGetAutoSleep(mpNewtonBody) == 0 ? false : true;
}

//-----------------------------------------------------------------------
#if 0
void cPhysicsBodyNewton::SetAutoDisableLinearThreshold(float afThresold) {
	mfAutoDisableLinearThreshold = afThresold;
	NewtonBodySetFreezeTreshold(mpNewtonBody, mfAutoDisableLinearThreshold,
								mfAutoDisableAngularThreshold, mlAutoDisableNumSteps);
}
float cPhysicsBodyNewton::GetAutoDisableLinearThreshold() const {
	return mfAutoDisableLinearThreshold;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAutoDisableAngularThreshold(float afThresold) {
	mfAutoDisableAngularThreshold = afThresold;
	NewtonBodySetFreezeTreshold(mpNewtonBody, mfAutoDisableLinearThreshold,
								mfAutoDisableAngularThreshold, mlAutoDisableNumSteps);
}
float cPhysicsBodyNewton::GetAutoDisableAngularThreshold() const {
	return mfAutoDisableAngularThreshold;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAutoDisableNumSteps(int anNum) {
	mlAutoDisableNumSteps = anNum;
	NewtonBodySetFreezeTreshold(mpNewtonBody, mfAutoDisableLinearThreshold,
								mfAutoDisableAngularThreshold, mlAutoDisableNumSteps);
}
int cPhysicsBodyNewton::GetAutoDisableNumSteps() const {
	return mlAutoDisableNumSteps;
}
#endif
//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetContinuousCollision(bool abOn) {
	NewtonBodySetContinuousCollisionMode(mpNewtonBody, abOn ? 1 : 0);
}

bool cPhysicsBodyNewton::GetContinuousCollision() {
	return NewtonBodyGetContinuousCollisionMode(mpNewtonBody) == 1 ? true : false;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetGravity(bool abEnabled) {
	mbGravity = abEnabled;
}
bool cPhysicsBodyNewton::GetGravity() const {
	return mbGravity;
}

//-----------------------------------------------------------------------

struct DrawParameters {
	iLowLevelGraphics *lowLevelGraphics;
	cColor drawColor;
};

///////////////////////////////////////////
static void RenderDebugPolygon(void *params, int alVertexCount, const dFloat *apFaceVertex, int alId) {
	int i = alVertexCount - 1;
	const DrawParameters *drawParams = static_cast<DrawParameters *>(params);
	cVector3f vP0(apFaceVertex[i * 3 + 0], apFaceVertex[i * 3 + 1], apFaceVertex[i * 3 + 2]);
	for (i = 0; i < alVertexCount; ++i) {
		cVector3f vP1(apFaceVertex[i * 3 + 0], apFaceVertex[i * 3 + 1], apFaceVertex[i * 3 + 2]);
		drawParams->lowLevelGraphics->DrawLine(vP0, vP1, drawParams->drawColor);
		vP0 = vP1;
	}
}

////////////////////////////////////////////

void cPhysicsBodyNewton::RenderDebugGeometry(iLowLevelGraphics *apLowLevel, const cColor &aColor) {
	DrawParameters params{apLowLevel, aColor};
	NewtonCollisionForEachPolygonDo(NewtonBodyGetCollision(mpNewtonBody), GetLocalMatrix().GetTranspose().v, RenderDebugPolygon, static_cast<void *>(&params));
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::ClearForces() {
	mvTotalForce = cVector3f(0, 0, 0);
	mvTotalTorque = cVector3f(0, 0, 0);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// STATIC NEWTON CALLBACKS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::OnTransformCallback(const NewtonBody *apBody, const dFloat *apMatrix, int) {
	cPhysicsBodyNewton *pRigidBody = (cPhysicsBodyNewton *)NewtonBodyGetUserData(apBody);

	pRigidBody->m_mtxLocalTransform.FromTranspose(apMatrix);

	mbUseCallback = false;
	pRigidBody->SetTransformUpdated(true);
	mbUseCallback = true;

	if (pRigidBody->mpNode)
		pRigidBody->mpNode->SetMatrix(pRigidBody->m_mtxLocalTransform);
}

//-----------------------------------------------------------------------

int cPhysicsBodyNewton::BuoyancyPlaneCallback(const int alCollisionID, void *apContext,
											  const float *afGlobalSpaceMatrix, float *afGlobalSpacePlane) {
	cPlanef surfacePlane = static_cast<cPhysicsBodyNewton *>(apContext)->mBuoyancy.mSurface;
	afGlobalSpacePlane[0] = surfacePlane.a;
	afGlobalSpacePlane[1] = surfacePlane.b;
	afGlobalSpacePlane[2] = surfacePlane.c;
	afGlobalSpacePlane[3] = surfacePlane.d;
	return 1;
}

void cPhysicsBodyNewton::OnUpdateCallback(NewtonBody *apBody, float, int) {
	float fMass;
	float fX, fY, fZ;

	cPhysicsBodyNewton *pRigidBody = (cPhysicsBodyNewton *)NewtonBodyGetUserData(apBody);

	if (pRigidBody->IsActive() == false)
		return;

	cVector3f vGravity = pRigidBody->mpWorld->GetGravity();

	// Create some gravity
	if (pRigidBody->mbGravity) {
		NewtonBodyGetMassMatrix(apBody, &fMass, &fX, &fY, &fZ);
		VEC3_CONST_ARRAY(force, (vGravity * fMass));
		NewtonBodyAddForce(apBody, force);
	}

	// Create Buoyancy
	if (pRigidBody->mBuoyancy.mbActive) {
		VEC3_CONST_ARRAY(gravity, vGravity);
		NewtonBodyAddBuoyancyForce(apBody,
								   pRigidBody->mBuoyancy.mfDensity,
								   pRigidBody->mBuoyancy.mfLinearViscosity,
								   pRigidBody->mBuoyancy.mfAngularViscosity,
								   gravity, BuoyancyPlaneCallback,
								   pRigidBody);
	}

	// Add forces from calls to Addforce(..), etc
	VEC3_CONST_ARRAY(totForce, pRigidBody->mvTotalForce);
	NewtonBodyAddForce(apBody, totForce);
	VEC3_CONST_ARRAY(totTorque, pRigidBody->mvTotalTorque);
	NewtonBodyAddTorque(apBody, totTorque);

	// Check so that all speeds are within thresholds
	// Linear
	if (pRigidBody->mfMaxLinearSpeed > 0) {
		cVector3f vVel = pRigidBody->GetLinearVelocity();
		float fSpeed = vVel.Length();
		if (fSpeed > pRigidBody->mfMaxLinearSpeed) {
			vVel = cMath::Vector3Normalize(vVel) * pRigidBody->mfMaxLinearSpeed;
			pRigidBody->SetLinearVelocity(vVel);
		}
	}
	// Angular
	if (pRigidBody->mfMaxAngularSpeed > 0) {
		cVector3f vVel = pRigidBody->GetAngularVelocity();
		float fSpeed = vVel.Length();
		if (fSpeed > pRigidBody->mfMaxAngularSpeed) {
			vVel = cMath::Vector3Normalize(vVel) * pRigidBody->mfMaxAngularSpeed;
			pRigidBody->SetAngularVelocity(vVel);
		}
	}

	// cVector3f vForce;
	// NewtonBodyGetForce(apBody,vForce.v);
	// Log("Engine force %s\n",pRigidBody->mvTotalForce.ToString().c_str());
	// Log("Engine force %s, body force: %s \n",pRigidBody->mvTotalForce.ToString().c_str(),
	//										vForce.ToString().c_str());
}

//-----------------------------------------------------------------------

} // namespace hpl
