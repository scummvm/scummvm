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
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/scene/Node3D.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

bool cPhysicsBodyNewton::mbUseCallback = true;

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPhysicsBodyNewton::cPhysicsBodyNewton(const tString &asName, iPhysicsWorld *apWorld, iCollideShape *apShape)
	: iPhysicsBody(asName, apWorld, apShape) {
#if 0
  		cPhysicsWorldNewton *pWorldNewton = static_cast<cPhysicsWorldNewton*>(apWorld);
		cCollideShapeNewton *pShapeNewton = static_cast<cCollideShapeNewton*>(apShape);

		mpNewtonWorld = pWorldNewton->GetNewtonWorld();
		mpNewtonBody = NewtonCreateBody(pWorldNewton->GetNewtonWorld(),
										pShapeNewton->GetNewtonCollision());

		mpCallback = hplNew( cPhysicsBodyNewtonCallback, () );

		AddCallback(mpCallback);

		// Setup the callbacks and set this body as user data
		// This is so that the transform gets updated and
		// to add gravity, forces and user sink.
		NewtonBodySetForceAndTorqueCallback(mpNewtonBody,OnUpdateCallback);
		NewtonBodySetTransformCallback(mpNewtonBody, OnTransformCallback);
		NewtonBodySetUserData(mpNewtonBody, this);

		//Set default property settings
		mbGravity = true;

		mfMaxLinearSpeed =0;
		mfMaxAngularSpeed =0;
		mfMass =0;

		mfAutoDisableLinearThreshold = 0.01f;
		mfAutoDisableAngularThreshold = 0.01f;
		mlAutoDisableNumSteps = 10;

		//Log("Creating newton body '%s' %d\n",msName.c_str(),(size_t)this);
#endif
}

//-----------------------------------------------------------------------

cPhysicsBodyNewton::~cPhysicsBodyNewton() {
	// Log(" Destroying newton body '%s' %d\n",msName.c_str(),(size_t)this);
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::DeleteLowLevel() {
#if 0
  		//Log(" Newton body %d\n", (size_t)mpNewtonBody);
		NewtonDestroyBody(mpNewtonWorld,mpNewtonBody);
		//Log(" Callback\n");
		hplDelete(mpCallback);
#endif
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
	auto mTemp = apEntity->GetLocalMatrix().GetTranspose();
	NewtonBodySetMatrix(pRigidBody->mpNewtonBody, &mTemp.m[0][0]);

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

	if (apMaterial == NULL)
		return;

	cPhysicsMaterialNewton *pNewtonMat = static_cast<cPhysicsMaterialNewton *>(mpMaterial);

	NewtonBodySetMaterialGroupID(mpNewtonBody, pNewtonMat->GetId());
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetLinearVelocity(const cVector3f &avVel) {
	const float arrayVec[] = {avVel.x, avVel.y, avVel.y};
	NewtonBodySetVelocity(mpNewtonBody, arrayVec);
}
cVector3f cPhysicsBodyNewton::GetLinearVelocity() const {
	float arrayVec[3];
	NewtonBodyGetVelocity(mpNewtonBody, arrayVec);
	return {arrayVec[0], arrayVec[1], arrayVec[2]};
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAngularVelocity(const cVector3f &avVel) {
	const float faVel[] = {avVel.x, avVel.y, avVel.y};
	NewtonBodySetOmega(mpNewtonBody, faVel);
}
cVector3f cPhysicsBodyNewton::GetAngularVelocity() const {
	float faVel[3];
	NewtonBodyGetOmega(mpNewtonBody, faVel);
	return {faVel[0], faVel[1], faVel[2]};
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
	float fDamp[3] = {afDamping, afDamping, afDamping};
	NewtonBodySetAngularDamping(mpNewtonBody, fDamp);
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

	float faInertia[3];
	float faOffset[3];
	NewtonConvexCollisionCalculateInertialMatrix(pShapeNewton->GetNewtonCollision(),
												 faInertia, faOffset);

	cVector3f vInertia = {faInertia[0], faInertia[1], faInertia[2]}; // = pShapeNewton->GetInertia(afMass);
	vInertia = vInertia * afMass;

	NewtonBodySetCentreOfMass(mpNewtonBody, faOffset);

	NewtonBodySetMassMatrix(mpNewtonBody, afMass, vInertia.x, vInertia.y, vInertia.z);
	mfMass = afMass;
}
float cPhysicsBodyNewton::GetMass() const {
	return mfMass;
}

void cPhysicsBodyNewton::SetMassCentre(const cVector3f &avCentre) {
	const float faCenter[3] = {avCentre.x, avCentre.y, avCentre.z};
	NewtonBodySetCentreOfMass(mpNewtonBody, faCenter);
}

cVector3f cPhysicsBodyNewton::GetMassCentre() const {
	float faCentre[3];
	NewtonBodyGetCentreOfMass(mpNewtonBody, faCentre);
	return {faCentre[0], faCentre[1], faCentre[3]};
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
#if 0
  		cVector3f vMassCentre = GetMassCentre();
		if(vMassCentre != cVector3f(0,0,0))
		{
			cVector3f vCentreOffset = cMath::MatrixMul( GetWorldMatrix().GetRotation(),
														vMassCentre);

			cVector3f vWorldPosition = GetWorldPosition() + vCentreOffset;
			NewtonAddBodyImpulse(mpNewtonBody, avImpulse.v, vWorldPosition.v);
		}
		else
		{
			NewtonAddBodyImpulse(mpNewtonBody, avImpulse.v, GetWorldPosition().v);
		}
#endif
}
void cPhysicsBodyNewton::AddImpulseAtPosition(const cVector3f &avImpulse, const cVector3f &avPos) {
#if 0
  		NewtonAddBodyImpulse(mpNewtonBody, avImpulse.v, avPos.v);
#endif
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetEnabled(bool abEnabled) {
#if 0
  		if (abEnabled)
			NewtonWorldUnfreezeBody(mpNewtonWorld, mpNewtonBody);
		else
			NewtonWorldFreezeBody(mpNewtonWorld, mpNewtonBody);
#endif
}
bool cPhysicsBodyNewton::GetEnabled() const {
#if 0
  		return NewtonBodyGetSleepingState(mpNewtonBody) ==0?false: true;
#endif
	return false;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAutoDisable(bool abEnabled) {
#if 0
  		NewtonBodySetAutoFreeze(mpNewtonBody, abEnabled ? 1 : 0);
#endif
}
bool cPhysicsBodyNewton::GetAutoDisable() const {
#if 0
  		return NewtonBodyGetAutoFreeze(mpNewtonBody) == 0 ? false : true;
#endif
	return false;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAutoDisableLinearThreshold(float afThresold) {
#if 0
  		mfAutoDisableLinearThreshold = afThresold;
		NewtonBodySetFreezeTreshold(mpNewtonBody, mfAutoDisableLinearThreshold,
			mfAutoDisableAngularThreshold, mlAutoDisableNumSteps);
#endif
}
float cPhysicsBodyNewton::GetAutoDisableLinearThreshold() const {
	return mfAutoDisableLinearThreshold;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAutoDisableAngularThreshold(float afThresold) {
#if 0
  		mfAutoDisableAngularThreshold = afThresold;
		NewtonBodySetFreezeTreshold(mpNewtonBody, mfAutoDisableLinearThreshold,
			mfAutoDisableAngularThreshold, mlAutoDisableNumSteps);
#endif
}
float cPhysicsBodyNewton::GetAutoDisableAngularThreshold() const {
	return mfAutoDisableAngularThreshold;
}

//-----------------------------------------------------------------------

void cPhysicsBodyNewton::SetAutoDisableNumSteps(int anNum) {
#if 0
  		mlAutoDisableNumSteps = anNum;
		NewtonBodySetFreezeTreshold(mpNewtonBody, mfAutoDisableLinearThreshold,
			mfAutoDisableAngularThreshold, mlAutoDisableNumSteps);
#endif
}

int cPhysicsBodyNewton::GetAutoDisableNumSteps() const {
	return mlAutoDisableNumSteps;
}

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

static iLowLevelGraphics *gpLowLevelGraphics;
static cColor gDebugColor;

////////////////////////////////////////////

static void RenderDebugPolygon(const NewtonBody *apNewtonBody,
							   int alVertexCount, const dFloat *apFaceVertex, int alId) {
	int i;

	i = alVertexCount - 1;
	cVector3f vP0(apFaceVertex[i * 3 + 0], apFaceVertex[i * 3 + 1], apFaceVertex[i * 3 + 2]);
	for (i = 0; i < alVertexCount; ++i) {
		cVector3f vP1(apFaceVertex[i * 3 + 0], apFaceVertex[i * 3 + 1], apFaceVertex[i * 3 + 2]);

		gpLowLevelGraphics->DrawLine(vP0, vP1, gDebugColor);

		vP0 = vP1;
	}
}

////////////////////////////////////////////

void cPhysicsBodyNewton::RenderDebugGeometry(iLowLevelGraphics *apLowLevel, const cColor &aColor) {
#if 0
  		gpLowLevelGraphics = apLowLevel;
		gDebugColor = aColor;
		NewtonBodyForEachPolygonDo (mpNewtonBody, RenderDebugPolygon);
#endif
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

void cPhysicsBodyNewton::OnTransformCallback(const NewtonBody *apBody, const dFloat *apMatrix) {
	cPhysicsBodyNewton *pRigidBody = (cPhysicsBodyNewton *)NewtonBodyGetUserData(apBody);

	pRigidBody->m_mtxLocalTransform.FromTranspose(apMatrix);

	mbUseCallback = false;
	pRigidBody->SetTransformUpdated(true);
	mbUseCallback = true;

	if (pRigidBody->mpNode)
		pRigidBody->mpNode->SetMatrix(pRigidBody->m_mtxLocalTransform);
}

//-----------------------------------------------------------------------

// callback for buoyancy
static cPlanef gSurfacePlane;
static int BuoyancyPlaneCallback(const int alCollisionID, void *apContext,
								 const float *afGlobalSpaceMatrix, float *afGlobalSpacePlane) {
	afGlobalSpacePlane[0] = gSurfacePlane.a;
	afGlobalSpacePlane[1] = gSurfacePlane.b;
	afGlobalSpacePlane[2] = gSurfacePlane.c;
	afGlobalSpacePlane[3] = gSurfacePlane.d;
	return 1;
}

void cPhysicsBodyNewton::OnUpdateCallback(const NewtonBody *apBody) {
#if 0
  float fMass;
		float fX,fY,fZ;

		cPhysicsBodyNewton* pRigidBody = (cPhysicsBodyNewton*) NewtonBodyGetUserData(apBody);

		if(pRigidBody->IsActive()==false) return;

		cVector3f vGravity = pRigidBody->mpWorld->GetGravity();

		//Create some gravity
		if (pRigidBody->mbGravity)
		{
			NewtonBodyGetMassMatrix(apBody, &fMass, &fX, &fY, &fZ);

			float fForce[3] = { fMass * vGravity.x, fMass * vGravity.y, fMass * vGravity.z};

			NewtonBodyAddForce(apBody, &fForce[0]);
		}

		// Create Buoyancy
		if (pRigidBody->mBuoyancy.mbActive)
		{
			gSurfacePlane = pRigidBody->mBuoyancy.mSurface;
			NewtonBodyAddBuoyancyForce( apBody,
										pRigidBody->mBuoyancy.mfDensity,
										pRigidBody->mBuoyancy.mfLinearViscosity,
										pRigidBody->mBuoyancy.mfAngularViscosity,
										vGravity.v, BuoyancyPlaneCallback,
										pRigidBody);
		}

		// Add forces from calls to Addforce(..), etc
		NewtonBodyAddForce(apBody, pRigidBody->mvTotalForce.v);
		NewtonBodyAddTorque(apBody, pRigidBody->mvTotalTorque.v);

		// Check so that all speeds are within thresholds
		// Linear
		if (pRigidBody->mfMaxLinearSpeed > 0)
		{
			cVector3f vVel = pRigidBody->GetLinearVelocity();
			float fSpeed = vVel.Length();
			if (fSpeed > pRigidBody->mfMaxLinearSpeed)
			{
				vVel = cMath::Vector3Normalize(vVel) * pRigidBody->mfMaxLinearSpeed;
				pRigidBody->SetLinearVelocity(vVel);
			}
		}
		// Angular
		if (pRigidBody->mfMaxAngularSpeed > 0)
		{
			cVector3f vVel = pRigidBody->GetAngularVelocity();
			float fSpeed = vVel.Length();
			if (fSpeed > pRigidBody->mfMaxAngularSpeed)
			{
				vVel = cMath::Vector3Normalize(vVel) * pRigidBody->mfMaxAngularSpeed;
				pRigidBody->SetAngularVelocity(vVel);
			}
		}

		//cVector3f vForce;
		//NewtonBodyGetForce(apBody,vForce.v);
		//Log("Engine force %s\n",pRigidBody->mvTotalForce.ToString().c_str());
		//Log("Engine force %s, body force: %s \n",pRigidBody->mvTotalForce.ToString().c_str(),
		//										vForce.ToString().c_str());
#endif
}

//-----------------------------------------------------------------------

} // namespace hpl
