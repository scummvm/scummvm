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

#include "hpl1/engine/physics/CharacterBody.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/physics/CollideShape.h"
#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"
#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iCharacterBody::iCharacterBody(const tString &asName, iPhysicsWorld *apWorld, const cVector3f avSize) {
	msName = asName;

	mpWorld = apWorld;

	mvSize = avSize;

	mbActive = true;

	mbCollideCharacter = true;

	mbTestCollision = true;

	mbOnGround = false;

	float fRadius = cMath::Max(avSize.x, avSize.z) * 0.5f;
	cMatrixf mtxOffset = cMath::MatrixRotateZ(kPi2f);
	iCollideShape *pCollider = NULL;

	if (fabs(fRadius * 2.0f - avSize.y) < 0.01)
		pCollider = mpWorld->CreateSphereShape(fRadius, NULL);
	else
		pCollider = mpWorld->CreateCylinderShape(fRadius, avSize.y, &mtxOffset);
	// pCollider = mpWorld->CreateCapsuleShape(fRadius, avSize.y,&mtxOffset);

	mpBody = mpWorld->CreateBody(asName, pCollider);
	mpBody->SetMass(0);
	mpBody->SetGravity(false);
	mpBody->SetIsCharacter(true);
	mpBody->SetCharacterBody(this);

	mvExtraBodies.push_back(mpBody);

	mfYaw = 0;
	mfPitch = 0;

	// Set move properties
	for (int i = 0; i < eCharDir_LastEnum; i++) {
		mfMaxPosMoveSpeed[i] = 10;
		mfMaxNegMoveSpeed[i] = -10;
		mfMoveSpeed[i] = 0;
		mfMoveAcc[i] = 20;
		mfMoveDeacc[i] = 20;
		mbMoving[i] = false;
	}

	mvForce = cVector3f(0, 0, 0);
	mvVelolcity = cVector3f(0, 0, 0);

	mbGravityActive = true;

	mfMaxGravitySpeed = 30.0f;

	mbEnableNearbyBodies = false;

	mpCamera = NULL;
	mvCameraPosAdd = cVector3f(0, 0, 0);

	mpEntity = NULL;
	m_mtxEntityOffset = cMatrixf::Identity;
	m_mtxEntityPostOffset = cMatrixf::Identity;

	mpUserData = NULL;

	mlCameraSmoothPosNum = 0;
	mlEntitySmoothPosNum = 0;

	mfMaxStepHeight = mvSize.y * 0.2f;
	mfStepClimbSpeed = 1.0f;
	mfClimbForwardMul = 1.0f;
	mfClimbHeightAdd = 0.01f;
	mbClimbing = false;
	mbAccurateClimbing = false;

	mfCheckStepClimbCount = 0;
	mfCheckStepClimbInterval = 1 / 20.0f;

	mfGroundFriction = 0.1f;
	mfAirFriction = 0.01f;

	mpRayCallback = hplNew(cCharacterBodyRay, ());
	mpCollideCallbackGravity = hplNew(cCharacterBodyCollideGravity, ());
	mpCollideCallbackPush = hplNew(cCharacterBodyCollidePush, ());

	mpCollideCallbackGravity->mpCharBody = this;
	mpCollideCallbackPush->mpCharBody = this;

	mfMass = 1;

	mfMaxPushMass = 0;
	mfPushForce = 0;
	mbPushIn2D = true;

	mpCallback = NULL;

	mpAttachedBody = NULL;
	mbAttachmentJustAdded = true;

	mbCustomGravity = false;
	mvCustomGravity = cVector3f(0, 9.8f, 0);
}

//-----------------------------------------------------------------------

iCharacterBody::~iCharacterBody() {
	for (size_t i = 0; i < mvExtraBodies.size(); i++)
		mpWorld->DestroyBody(mvExtraBodies[i]);

	hplDelete(mpRayCallback);
	hplDelete(mpCollideCallbackGravity);
	hplDelete(mpCollideCallbackPush);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// RAY CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCharacterBodyRay::cCharacterBodyRay() {
}

//-----------------------------------------------------------------------

void cCharacterBodyRay::Clear() {
	mfMinDist = 10000.0f;
	mbCollide = false;
}

//-----------------------------------------------------------------------

bool cCharacterBodyRay::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	if (pBody->IsCharacter() == false && pBody->GetCollideCharacter() &&
		apParams->mfDist < mfMinDist) {
		mfMinDist = apParams->mfDist;
		mbCollide = true;
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// COLLIDE GRAVITY CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCharacterBodyCollideGravity::cCharacterBodyCollideGravity() {
	mpCharBody = NULL;
}

//-----------------------------------------------------------------------

void cCharacterBodyCollideGravity::OnCollision(iPhysicsBody *apBody, cCollideData *apCollideData) {
	if (apBody->GetCanAttachCharacter()) {
		if (mpCharBody->GetAttachedBody() == NULL) {
			mpCharBody->SetAttachedBody(apBody);
		}
	}

	if (apBody->GetMass() == 0 || apBody->GetPushedByCharacterGravity() == false)
		return;

	bool bPushDown = false;
	cVector3f vPoint(0, 0, 0);
	float fNumPoints = 0;

	// Go through all of the contact points and check if any is a movement up.
	// This means the body is below the character and should be pushed down.
	for (int i = 0; i < apCollideData->mlNumOfPoints; i++) {
		// TODO: Get the point at which to apply the force.
		cCollidePoint &point = apCollideData->mvContactPoints[i];
		if (point.mvNormal.y > 0.001f) {
			bPushDown = true;
			fNumPoints += 1;
			vPoint += point.mvPoint;
		}
	}

	if (bPushDown) {
		float fForceAdd = 0;

		// Totally unrealistic force add
		// Skip this for now and add better with more object specific stuff
		/*if(mpCharBody->GetForceVelocity().y < 0)
		{
			fForceAdd = mpCharBody->GetForceVelocity().y * mpCharBody->GetMass() * 10;
		}*/

		vPoint = vPoint / fNumPoints;
		apBody->AddForceAtPosition(cVector3f(0, mpCharBody->GetMass() * -9.8f + fForceAdd, 0), vPoint);
	}

	if (mpCharBody->mpCallback)
		mpCharBody->mpCallback->OnGravityCollide(mpCharBody, apBody, apCollideData);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// COLLIDE PUSH CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCharacterBodyCollidePush::cCharacterBodyCollidePush() {
	mpCharBody = NULL;
}

//-----------------------------------------------------------------------

void cCharacterBodyCollidePush::OnCollision(iPhysicsBody *apBody, cCollideData *apCollideData) {
	// No pushing if the player is not moving
	if (mpCharBody->GetMoveSpeed(eCharDir_Forward) == 0 &&
		mpCharBody->GetMoveSpeed(eCharDir_Right) == 0)
		return;

	// Check what bodies not to push.
	if (apBody->GetMass() == 0 || apBody->GetMass() > mpCharBody->GetMaxPushMass())
		return;

	bool bPush = false;
	cVector3f vPoint(0, 0, 0);
	float fNumPoints = 0;

	// Go through all of the contact points
	for (int i = 0; i < apCollideData->mlNumOfPoints; i++) {
		cCollidePoint &point = apCollideData->mvContactPoints[i];

		bPush = true;
		fNumPoints += 1;
		vPoint += point.mvPoint;
	}

	if (bPush) {
		vPoint = vPoint / fNumPoints;

		if (mpCharBody->GetPushIn2D()) {
			cVector3f vDir = apBody->GetWorldPosition() - mpCharBody->GetPosition();
			vDir.y = 0;
			vDir.Normalise();

			apBody->AddForceAtPosition(vDir * mpCharBody->GetPushForce(), vPoint);
		} else {
			cVector3f vDir = cMath::Vector3Normalize(apBody->GetWorldPosition() - mpCharBody->GetPosition());

			apBody->AddForceAtPosition(vDir * mpCharBody->GetPushForce(), vPoint);
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

float iCharacterBody::GetMass() {
	return mfMass;
}
void iCharacterBody::SetMass(float afMass) {
	mfMass = afMass;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;

	for (size_t i = 0; i < mvExtraBodies.size(); ++i) {
		if (mvExtraBodies[i]) {
			mvExtraBodies[i]->SetActive(mbActive);
		}
	}
}

//-----------------------------------------------------------------------

void iCharacterBody::SetCollideCharacter(bool abX) {
	if (abX == mbCollideCharacter)
		return;

	mbCollideCharacter = abX;

	for (size_t i = 0; i < mvExtraBodies.size(); ++i) {
		if (mvExtraBodies[i]) {
			mvExtraBodies[i]->SetCollideCharacter(mbCollideCharacter);
		}
	}
}

//-----------------------------------------------------------------------

cVector3f iCharacterBody::GetSize() {
	return mvSize;
}

//-----------------------------------------------------------------------

int iCharacterBody::AddExtraSize(const cVector3f &avSize) {
	float fRadius = cMath::Max(avSize.x, avSize.z) * 0.5f;
	cMatrixf mtxOffset = cMath::MatrixRotateZ(kPi2f);
	iCollideShape *pCollider = mpWorld->CreateCylinderShape(fRadius, avSize.y, &mtxOffset);
	// iCollideShape *pCollider = mpWorld->CreateBoxShape(avSize,NULL);

	iPhysicsBody *pBody = mpWorld->CreateBody(msName, pCollider);
	pBody->SetMass(0);
	pBody->SetGravity(false);
	pBody->SetIsCharacter(true);
	pBody->SetActive(false);
	pBody->SetCharacterBody(this);

	mvExtraBodies.push_back(pBody);

	return (int)mvExtraBodies.size() - 1;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetActiveSize(int alNum) {
	mpBody->SetActive(false);
	mpBody = mvExtraBodies[alNum];
	mpBody->SetActive(true);

	// Set size of the new body.
	mvSize.y = mpBody->GetShape()->GetHeight();
	mvSize.x = mpBody->GetShape()->GetRadius() * 2;
	mvSize.z = mpBody->GetShape()->GetRadius() * 2;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetMaxPositiveMoveSpeed(eCharDir aDir, float afX) {
	mfMaxPosMoveSpeed[aDir] = afX;
}
float iCharacterBody::GetMaxPositiveMoveSpeed(eCharDir aDir) {
	return mfMaxPosMoveSpeed[aDir];
}
void iCharacterBody::SetMaxNegativeMoveSpeed(eCharDir aDir, float afX) {
	mfMaxNegMoveSpeed[aDir] = afX;
}
float iCharacterBody::GetMaxNegativeMoveSpeed(eCharDir aDir) {
	return mfMaxNegMoveSpeed[aDir];
}

//-----------------------------------------------------------------------

void iCharacterBody::SetMoveSpeed(eCharDir aDir, float afX) {
	mfMoveSpeed[aDir] = afX;
}
float iCharacterBody::GetMoveSpeed(eCharDir aDir) {
	return mfMoveSpeed[aDir];
}

void iCharacterBody::SetMoveAcc(eCharDir aDir, float afX) {
	mfMoveAcc[aDir] = afX;
}
float iCharacterBody::GetMoveAcc(eCharDir aDir) {
	return mfMoveAcc[aDir];
}

void iCharacterBody::SetMoveDeacc(eCharDir aDir, float afX) {
	mfMoveDeacc[aDir] = afX;
}
float iCharacterBody::GetMoveDeacc(eCharDir aDir) {
	return mfMoveDeacc[aDir];
}

//-----------------------------------------------------------------------

cVector3f iCharacterBody::GetVelocity(float afFrameTime) {
	if (afFrameTime <= 0)
		return 0;

	return (mvPosition - mvLastPosition) / afFrameTime;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetPosition(const cVector3f &avPos, bool abSmooth) {
	mvForce = 0;
	mvVelolcity = 0;
	mvLastPosition = avPos;
	mvPosition = avPos;
	mpBody->SetPosition(avPos);

	if (!abSmooth)
		mlstCameraPos.clear();
}
const cVector3f &iCharacterBody::GetPosition() {
	return mvPosition;
}
const cVector3f &iCharacterBody::GetLastPosition() {
	return mvLastPosition;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetFeetPosition(const cVector3f &avPos, bool abSmooth) {
	SetPosition(avPos + cVector3f(0, mpBody->GetShape()->GetSize().y / 2, 0), abSmooth);
}

cVector3f iCharacterBody::GetFeetPosition() {
	return mvPosition - cVector3f(0, mpBody->GetShape()->GetSize().y / 2, 0);
}

//-----------------------------------------------------------------------

void iCharacterBody::SetYaw(float afX) {
	mfYaw = afX;
}

float iCharacterBody::GetYaw() {
	return mfYaw;
}

void iCharacterBody::AddYaw(float afX) {
	mfYaw += afX;
}

void iCharacterBody::SetPitch(float afX) {
	mfPitch = afX;
}

void iCharacterBody::AddPitch(float afX) {
	mfPitch += afX;
}

float iCharacterBody::GetPitch() {
	return mfPitch;
}

//-----------------------------------------------------------------------

cVector3f iCharacterBody::GetForward() {
	return m_mtxMove.GetForward() * -1.0f;
}
cVector3f iCharacterBody::GetRight() {
	return m_mtxMove.GetRight();
}
cVector3f iCharacterBody::GetUp() {
	return m_mtxMove.GetUp();
}

//-----------------------------------------------------------------------

cMatrixf &iCharacterBody::GetMoveMatrix() {
	return m_mtxMove;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetGravityActive(bool abX) {
	mbGravityActive = abX;
}

bool iCharacterBody::GravityIsActive() {
	return mbGravityActive;
}

void iCharacterBody::SetMaxGravitySpeed(float afX) {
	mfMaxGravitySpeed = afX;
}
float iCharacterBody::GetMaxGravitySpeed() {
	return mfMaxGravitySpeed;
}

//-----------------------------------------------------------------------

bool iCharacterBody::GetCustomGravityActive() {
	return mbCustomGravity;
}

void iCharacterBody::SetCustomGravityActive(bool abX) {
	mbCustomGravity = abX;
}

void iCharacterBody::SetCustomGravity(const cVector3f &avCustomGravity) {
	mvCustomGravity = avCustomGravity;
}

cVector3f iCharacterBody::GetCustomGravity() {
	return mvCustomGravity;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetForce(const cVector3f &avForce) {
	mvForce = avForce;
}

void iCharacterBody::AddForce(const cVector3f &avForce) {
	mvForce += avForce;
}

cVector3f iCharacterBody::GetForce() {
	return mvForce;
}

//-----------------------------------------------------------------------

void iCharacterBody::Move(eCharDir aDir, float afMul, float afTimeStep) {
	mfMoveSpeed[aDir] += mfMoveAcc[aDir] * afMul * afTimeStep;

	mbMoving[aDir] = true;

	// Clamp the speed.
	mfMoveSpeed[aDir] = cMath::Clamp(mfMoveSpeed[aDir], mfMaxNegMoveSpeed[aDir], mfMaxPosMoveSpeed[aDir]);
}

//-----------------------------------------------------------------------

void iCharacterBody::Update(float afTimeStep) {
	if (mbActive == false)
		return;

	// Clear attached body
	iPhysicsBody *pLastAttached = mpAttachedBody;
	SetAttachedBody(NULL);

	// Update the move matrix.
	UpdateMoveMarix();

	mvLastPosition = mvPosition;

	/////////////////////////////
	// Ground and air friction:
	float fFriction = mbOnGround ? mfGroundFriction : mfAirFriction;

	// This is not working for some reason... Probably because it thinks its on the ground when it is really in the air.
	/*if(mbOnGround)
	{
		float fSpeed = mvVelolcity.Length();
		cVector3f vDir = cMath::Vector3Normalize(mvVelolcity);

		fSpeed -= fFriction * afTimeStep;
		if(fSpeed<0) fSpeed=0;

		mvVelolcity = vDir * fSpeed;
	}
	else*/
	{
		cVector3f vVelXZ(mvVelolcity.x, 0, mvVelolcity.z);
		float fSpeed = vVelXZ.Length();
		vVelXZ.Normalise();

		fSpeed -= fFriction * afTimeStep;
		if (fSpeed < 0)
			fSpeed = 0;

		mvVelolcity.x = vVelXZ.x * fSpeed;
		mvVelolcity.z = vVelXZ.z * fSpeed;
	}

	////////////////////////////////
	// Set the postion to the body
	mpBody->SetPosition(mvPosition);

	//////////////////////////
	// Enable objects around the character
	if (mvLastPosition != mvPosition) {
		cBoundingVolume largeBV = *mpBody->GetBV();
		largeBV.SetSize(largeBV.GetSize() * 1.02f);

		mpWorld->EnableBodiesInBV(&largeBV, true);
	}

	/////////////////////////////////////////////////////
	// If the character is not moving in a direction, apply deacceleration.
	for (int i = 0; i < eCharDir_LastEnum; i++) {
		if (mbMoving[i] == false) {
			if (mfMoveSpeed[i] > 0) {
				mfMoveSpeed[i] -= mfMoveDeacc[i] * afTimeStep;
				if (mfMoveSpeed[i] < 0)
					mfMoveSpeed[i] = 0;
			} else {
				mfMoveSpeed[i] += mfMoveDeacc[i] * afTimeStep;
				if (mfMoveSpeed[i] > 0)
					mfMoveSpeed[i] = 0;
			}
		} else {
			mbMoving[i] = false;
		}
	}

	/////////////////////////////////////////
	// Update postion
	if (mbClimbing)
		mfCheckStepClimbCount = 0;
	else
		mfCheckStepClimbCount -= afTimeStep;

	mbClimbing = false;

	cVector3f vPosAdd(0, 0, 0);

	// Movement velocity
	cVector3f vForward = GetMoveMatrix().GetForward() * -1.0f;
	cVector3f vRight = GetMoveMatrix().GetRight();

	vPosAdd += vForward * mfMoveSpeed[eCharDir_Forward] * afTimeStep;
	vPosAdd += vRight * mfMoveSpeed[eCharDir_Right] * afTimeStep;

	// Make sure speed is not greate than max forward.
	float fMaxStep = mfMoveSpeed[eCharDir_Forward] >= 0 ? mfMaxPosMoveSpeed[eCharDir_Forward] : mfMaxNegMoveSpeed[eCharDir_Forward];
	fMaxStep *= afTimeStep;
	fMaxStep = cMath::Abs(fMaxStep);

	float fStepLength = vPosAdd.Length();
	if (fStepLength > fMaxStep) {
		vPosAdd = vPosAdd / fStepLength;
		vPosAdd = vPosAdd * fMaxStep;
	}

	// Check if collision should be tested
	if (mbTestCollision == false) {
		mpBody->SetPosition(mvPosition);

		UpdateCamera();
		UpdateEntity();

		mvVelolcity = 0; // This is a fix so the velocity is not screwed up later on.

		return;
	}

	/////////////////////////////////////
	// Check for collision.
	// Might wanna test this for x, y and z independently.
	mvPosition += vPosAdd;

	cVector3f vNewPos;
	// vNewPos = mvPosition;
	if (mvLastPosition.x != mvPosition.x || mvLastPosition.z != mvLastPosition.z) {
		mpWorld->CheckShapeWorldCollision(&vNewPos, mpBody->GetShape(), cMath::MatrixTranslate(mvPosition), mpBody,
										  false, true, mpCollideCallbackPush, mbCollideCharacter);
	} else {
		vNewPos = mvPosition;
	}

	// If the player is halted, check if there might be a step infront of him.
	if ((mvPosition.x != vNewPos.x || mvPosition.z != vNewPos.z ||
		 (mbGravityActive == false && mvPosition.y != vNewPos.y))) {
		mvPosition = vNewPos;

		// Log("--- Colliding ----!\n");

		if (mfCheckStepClimbCount <= 0) {
			// Send a ray in front of the player.
			cVector3f vShapeSize = mpBody->GetShape()->GetSize();
			float fRadius = mpBody->GetShape()->GetRadius();
			float fForwadAdd = vPosAdd.Length();

			// The direction of the movement.
			cVector3f vMoveDir = cMath::Vector3Normalize(vPosAdd);

			// Log("MoveDir: %s\n",vMoveDir.ToString().c_str());

			cVector3f vStepAdd[3];
			cVector3f vStart[3];
			cVector3f vEnd[3];
			bool bCollided[3];
			float fMinDist[3];
			int lNumRays = 1;
			if (mbAccurateClimbing)
				lNumRays = 3;

			/////////////////////////////////
			// Calculate the different movements
			vStepAdd[0] = vMoveDir * (fRadius + fForwadAdd);

			if (mbAccurateClimbing) {
				cVector3f vRightDir = cMath::MatrixMul(cMath::MatrixRotateY(kPi4f), vMoveDir);
				vStepAdd[1] = (vRightDir * fRadius) + (vMoveDir * fForwadAdd);

				cVector3f vLeftDir = cMath::MatrixMul(cMath::MatrixRotateY(-kPi4f), vMoveDir);
				vStepAdd[2] = (vLeftDir * fRadius) + (vMoveDir * fForwadAdd);
			}

			/////////////////////////////////
			// Shot the rays
			for (int i = 0; i < lNumRays; ++i) {
				vStart[i] = mvPosition + cVector3f(0, vShapeSize.y / 2, 0) + vStepAdd[i];
				vEnd[i] = vStart[i] - cVector3f(0, vShapeSize.y, 0);

				mpRayCallback->Clear();
				mpWorld->CastRay(mpRayCallback, vStart[i], vEnd[i], true, false, false);
				bCollided[i] = mpRayCallback->mbCollide;
				fMinDist[i] = mpRayCallback->mfMinDist;
			}

			/////////////////////////////////7
			// Check if the step can be climbed.
			for (int i = 0; i < lNumRays; ++i) {
				if (bCollided[i] == false)
					continue;
				// Log("Ray collided!\n");

				float fHeight = vShapeSize.y - fMinDist[i];

				// Log("Height: %f MinDist: %f\n",fHeight,mpRayCallback->mfMinDist);

				if (fHeight <= mfMaxStepHeight) {
					// Log("Trying to step up!\n");
					// Check if there is any collision on the new pos
					cVector3f vStepPos = mvPosition + cVector3f(0, fHeight + mfClimbHeightAdd, 0) +
										 (vMoveDir * fForwadAdd * mfClimbForwardMul);

					mpWorld->CheckShapeWorldCollision(&vNewPos, mpBody->GetShape(),
													  cMath::MatrixTranslate(vStepPos), mpBody,
													  false, true, NULL, mbCollideCharacter);
					// Log("Old pos: (%s) StepPos: (%s) NewPos: (%s)\n",mvPosition.ToString().c_str(),
					//							vStepPos.ToString().c_str(),vNewPos.ToString().c_str());
					if (vNewPos == vStepPos) {
						// mvPosition = vStepPos;
						// Climb the stair.
						mvPosition.y += mfStepClimbSpeed * afTimeStep;
						mbClimbing = true;
						break;
					}
				}
			}

			mfCheckStepClimbCount = mfCheckStepClimbInterval;
		}
	}

	if (mbGravityActive == false) {
		// Set the postion to the body
		mpBody->SetPosition(mvPosition);

		UpdateCamera();
		UpdateEntity();

		mvVelolcity = 0; // This is a fix so the velocity is not screwed up later on. (still is though...)

		return;
	}

	/////////////////////////////////////
	// Update External forces
	// Log(" vel1: %s\n",mvVelolcity.ToString().c_str());
	cVector3f vBeforeForcePos = mvPosition;

	mvVelolcity += mvForce * (afTimeStep * (1.0f / mfMass));
	if (mbGravityActive && mbClimbing == false) {
		if (mbCustomGravity)
			mvVelolcity += mvCustomGravity * afTimeStep;
		else
			mvVelolcity += mpWorld->GetGravity() * afTimeStep;

		float fLength = mvVelolcity.Length();
		if (fLength > mfMaxGravitySpeed) {
			mvVelolcity = (mvVelolcity / fLength) * mfMaxGravitySpeed;
		}
	}

	// Log(" vel2: %s\n",mvVelolcity.ToString().c_str());

	mvForce = cVector3f(0, 0, 0);

	cVector3f vLastVelocity(0, 0, 0);

	////////////////////////////
	// Check x and z collision
	if (mvVelolcity.x != 0 || mvVelolcity.z != 0) {
		mvPosition += cVector3f(mvVelolcity.x, 0, mvVelolcity.z) * afTimeStep;

		vNewPos = mvPosition;
		/*bool bCollide = */ mpWorld->CheckShapeWorldCollision(&vNewPos, mpBody->GetShape(), cMath::MatrixTranslate(mvPosition),
															   mpBody, false, true, NULL, mbCollideCharacter);

		// Set new velocity depending on collisions.
		vLastVelocity.x = mvVelolcity.x;
		vLastVelocity.z = mvVelolcity.z;

		mvVelolcity.x = (vNewPos.x - vBeforeForcePos.x) * (1 / afTimeStep);
		mvVelolcity.z = (vNewPos.z - vBeforeForcePos.z) * (1 / afTimeStep);

		// Make sure the speed doesn't change direction.
		// if((vLastVelocity.x<0 && mvVelolcity.x>0) || (vLastVelocity.x>0 && mvVelolcity.x<0)) mvVelolcity.x =0;
		// if((vLastVelocity.z<0 && mvVelolcity.z>0) || (vLastVelocity.z>0 && mvVelolcity.z<0)) mvVelolcity.z =0;

		mvPosition = vNewPos;
		// Log("Some strange force!!\n");
	}

	// if(mbGravityActive==false)return;
	// Log("Before yforce: %s\n",mvPosition.ToString().c_str());

	////////////////////////////////
	// Check y collision, this is to be sure if the character touches the ground.
	bool bCollide = false;

	// Log("VelY 2: %f\n", mvVelolcity.y);

	cVector3f vPosbeforeGrav = mvPosition;
	mvPosition.y += mvVelolcity.y * afTimeStep;

	vNewPos = mvPosition;
	bCollide = mpWorld->CheckShapeWorldCollision(&vNewPos, mpBody->GetShape(),
												 cMath::MatrixTranslate(mvPosition), mpBody, false, true,
												 mpCollideCallbackGravity, mbCollideCharacter);

	// Set new velocity depending on collisions.
	vLastVelocity.y = mvVelolcity.y;

	// Log(" vel4: %s\n",mvVelolcity.ToString().c_str());

	// If climbing we don't wanna leave behind a great up speed.
	if (mbClimbing) {
		if (mvVelolcity.y < 0)
			mvVelolcity.y = 0;
	} else {
		// mvVelolcity.y = (vNewPos.y - vBeforeForcePos.y) * (1/afTimeStep);
		mvVelolcity.y = (vNewPos.y - vPosbeforeGrav.y) * (1 / afTimeStep);

		// This s a fix that I am not 100% sure is good.
		if (vLastVelocity.y < 0 && mvVelolcity.y > 0)
			mvVelolcity.y = 0;
		if (vLastVelocity.y > 0 && mvVelolcity.y < 0)
			mvVelolcity.y = 0;
	}

	// Log(" vel5: %s\n",mvVelolcity.ToString().c_str());

	// Log("After Velocity: %s\n--------\n",mvVelolcity.ToString().c_str());
	// Make sure the speed doesn't change direction.Should not be needed.
	// if((vLastVelocity.y<0 && mvVelolcity.y>0) || (vLastVelocity.y>0 && mvVelolcity.y<0)) mvVelolcity.y =0;

	mvPosition = vNewPos;

	///////////////////////////////////////////////
	// Determine if character is on the ground.
	if (mbClimbing) {
		// if climbing there is no gravity working but we still want foot steps.
		mbOnGround = true;
	} else {
		if (bCollide && vLastVelocity.y <= 0 && ABS(mvVelolcity.y) < (ABS(vLastVelocity.y) - 0.001f)) {
			if (mbOnGround == false) {
				if (mpCallback)
					mpCallback->OnHitGround(this, vLastVelocity);
			}

			mbOnGround = true;
		} else {
			mbOnGround = false;
		}
	}

	// This is so that the character does not slide down on
	// small slopes.
	if (mbOnGround && mvVelolcity.y < 0 && mvVelolcity.y > -0.15) {
		mvPosition = vPosbeforeGrav;
	}

	// debug:
	// mbOnGround = true;

	//////////////////////////////
	// Update attached objects
	UpdateCamera();
	UpdateEntity();

	////////////////////////////
	// Update body if it is attached
	if (pLastAttached != mpAttachedBody)
		mbAttachmentJustAdded = true;
	UpdateAttachment();

	//////////////////////////////
	// Enable close by objects

	if (mbEnableNearbyBodies) {
		cWorld3D *pWorld = mpWorld->GetWorld3D();
		cPortalContainerEntityIterator bodyIt = pWorld->GetPortalContainer()->GetEntityIterator(
			mpBody->GetBoundingVolume());

		cBoundingVolume bv = *mpBody->GetBV();
		bv.SetLocalMinMax(bv.GetLocalMin() - cVector3f(0.03f, 0.03f, 0.03f), bv.GetLocalMax() + cVector3f(0.03f, 0.03f, 0.03f));
		float fHeadY = mpBody->GetWorldPosition().y + mpBody->GetShape()->GetSize().y / 2 - 0.01f;
		// float fBVHeight = 0.1;
		// bv.SetPosition(mpBody->GetWorldPosition().y + fBVHeight/2);
		// bv.SetSize( cVector3f(mpBody->GetShape()->GetSize().x*0.9f, fBVHeight, mpBody->GetShape()->GetSize().z*0.9f) );

		while (bodyIt.HasNext()) {
			iPhysicsBody *pBody = static_cast<iPhysicsBody *>(bodyIt.Next());

			if (pBody->IsActive() &&
				fHeadY <= pBody->GetLocalPosition().y &&
				pBody->GetEnabled() == false &&
				cMath::CheckCollisionBV(*pBody->GetBV(), bv)) {
				pBody->SetEnabled(true);
			}
		}
	}
}

//-----------------------------------------------------------------------

void iCharacterBody::SetCamera(cCamera3D *apCam) {
	mpCamera = apCam;
}

cCamera3D *iCharacterBody::GetCamera() {
	return mpCamera;
}

void iCharacterBody::SetCameraPosAdd(const cVector3f &avAdd) {
	mvCameraPosAdd = avAdd;
}
cVector3f iCharacterBody::GetCameraPosAdd() {
	return mvCameraPosAdd;
}

//-----------------------------------------------------------------------

iCollideShape *iCharacterBody::GetShape() {
	return mpBody->GetShape();
}

//-----------------------------------------------------------------------

bool iCharacterBody::IsOnGround() {
	return mbOnGround;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetEntity(iEntity3D *apEntity) {
	mpEntity = apEntity;
}
iEntity3D *iCharacterBody::GetEntity() {
	return mpEntity;
}
void iCharacterBody::SetEntityOffset(const cMatrixf &a_mtxOffset) {
	m_mtxEntityOffset = a_mtxOffset;
}
const cMatrixf &iCharacterBody::GetEntityOffset() {
	return m_mtxEntityOffset;
}

void iCharacterBody::SetEntityPostOffset(const cMatrixf &a_mtxOffset) {
	m_mtxEntityPostOffset = a_mtxOffset;
}
const cMatrixf &iCharacterBody::GetEntityPostOffset() {
	return m_mtxEntityPostOffset;
}

//-----------------------------------------------------------------------

void iCharacterBody::SetAttachedBody(iPhysicsBody *apBody) {
	if (apBody == mpAttachedBody)
		return;

	if (mpAttachedBody)
		mpAttachedBody->RemoveAttachedCharacter(this);

	mpAttachedBody = apBody;
	if (mpAttachedBody)
		mpAttachedBody->AddAttachedCharacter(this);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
void iCharacterBody::UpdateMoveMarix() {
	m_mtxMove = cMath::MatrixRotateY(-mfYaw);
	m_mtxMove = cMath::MatrixMul(cMath::MatrixRotateX(-mfPitch), m_mtxMove);
	m_mtxMove.SetTranslation(mvPosition * -1);
}

//-----------------------------------------------------------------------

void iCharacterBody::UpdateCamera() {
	if (mpCamera == NULL)
		return;

	if (mlCameraSmoothPosNum <= 0) {
		cVector3f vPos = (mvPosition - cVector3f(0, mpBody->GetShape()->GetSize().y / 2.0f, 0)) +
						 cVector3f(0, mvSize.y, 0);
		mpCamera->SetPosition(vPos + mvCameraPosAdd);
	}
	// Smooth the camera position
	else {
		// Add the newest position.
		mlstCameraPos.push_back(mvPosition);
		// If to too large remove the oldest.
		if ((int)mlstCameraPos.size() > mlCameraSmoothPosNum) {
			mlstCameraPos.erase(mlstCameraPos.begin());
		}

		float fNum = (float)mlstCameraPos.size();

		// Add all positions and devide by the number of em.
		// that way we get the average
		cVector3f vTotalPos(0, 0, 0);
		tVector3fListIt it = mlstCameraPos.begin();
		for (; it != mlstCameraPos.end(); ++it) {
			vTotalPos += *it;
		}

		cVector3f vPos = vTotalPos / fNum;
		cVector3f vFirstSize = mvExtraBodies[0]->GetShape()->GetSize();

		cVector3f vHeadPos = (vPos - cVector3f(0, mpBody->GetShape()->GetSize().y / 2.0f, 0)) +
							 cVector3f(0, vFirstSize.y, 0);

		mpCamera->SetPosition(vHeadPos + mvCameraPosAdd);
	}
}

//-----------------------------------------------------------------------

void iCharacterBody::UpdateEntity() {
	if (mpEntity == NULL)
		return;

	if (mlEntitySmoothPosNum <= 0) {
		cMatrixf mtxEntity = cMath::MatrixRotateY(mfYaw);
		mtxEntity.SetTranslation(mvPosition);

		mpEntity->SetMatrix(cMath::MatrixMul(mtxEntity, m_mtxEntityOffset));
	}
	// Smooth the camera position
	else {
		// Add the newest position.
		mlstEntityPos.push_back(mvPosition);
		// If to too large remove the oldest.
		if ((int)mlstEntityPos.size() > mlEntitySmoothPosNum) {
			mlstEntityPos.erase(mlstEntityPos.begin());
		}

		float fNum = (float)mlstEntityPos.size();

		// Add all positions and devide by the number of em.
		// that way we get the average
		cVector3f vTotalPos(0, 0, 0);
		tVector3fListIt it = mlstEntityPos.begin();
		for (; it != mlstEntityPos.end(); ++it) {
			vTotalPos += *it;
		}

		cVector3f vPos = vTotalPos / fNum;

		cMatrixf mtxEntity = cMath::MatrixInverse(m_mtxMove);
		mtxEntity.SetTranslation(0);

		mtxEntity = cMath::MatrixMul(m_mtxEntityPostOffset, mtxEntity);
		mtxEntity.SetTranslation(mtxEntity.GetTranslation() + vPos);

		mtxEntity = cMath::MatrixMul(mtxEntity, m_mtxEntityOffset);

		mpEntity->SetMatrix(mtxEntity);
	}
}

//-----------------------------------------------------------------------

void iCharacterBody::UpdateAttachment() {
	if (mpAttachedBody == NULL) {
		mbAttachmentJustAdded = true;
		return;
	}

	// Log("Updateattachment %d\n",mbAttachmentJustAdded);

	if (mbAttachmentJustAdded) {
		mbAttachmentJustAdded = false;
	} else {
		cVector3f vPosAdd = mpAttachedBody->GetWorldPosition() -
							m_mtxAttachedPrevMatrix.GetTranslation();

		mvPosition += vPosAdd;
	}

	m_mtxAttachedPrevMatrix = mpAttachedBody->GetWorldMatrix();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_iCharacterBody, iSaveData)
	kSerializeVar(msName, eSerializeType_String)

		kSerializeVar(mfMass, eSerializeType_Float32)
			kSerializeVar(mbGravityActive, eSerializeType_Bool)
				kSerializeVar(mfMaxGravitySpeed, eSerializeType_Float32)

					kSerializeVar(mbActive, eSerializeType_Bool)

						kSerializeVar(mbCollideCharacter, eSerializeType_Bool)

							kSerializeVar(mvPosition, eSerializeType_Vector3f)
								kSerializeVar(mvLastPosition, eSerializeType_Vector3f)
									kSerializeVarArray(mfMaxPosMoveSpeed, eSerializeType_Float32, 2)
										kSerializeVarArray(mfMaxNegMoveSpeed, eSerializeType_Float32, 2)
											kSerializeVarArray(mfMoveSpeed, eSerializeType_Float32, 2)
												kSerializeVarArray(mfMoveAcc, eSerializeType_Float32, 2)
													kSerializeVarArray(mfMoveDeacc, eSerializeType_Float32, 2)
														kSerializeVarArray(mbMoving, eSerializeType_Bool, 2)

															kSerializeVar(mfPitch, eSerializeType_Float32)
																kSerializeVar(mfYaw, eSerializeType_Float32)

																	kSerializeVar(mbOnGround, eSerializeType_Bool)

																		kSerializeVar(mfMaxPushMass, eSerializeType_Float32)
																			kSerializeVar(mfPushForce, eSerializeType_Float32)

																				kSerializeVar(mvForce, eSerializeType_Vector3f)
																					kSerializeVar(mvVelolcity, eSerializeType_Vector3f)

																						kSerializeVar(mvSize, eSerializeType_Vector3f)

																							kSerializeVar(m_mtxMove, eSerializeType_Matrixf)

																								kSerializeVar(mlEntityId, eSerializeType_Int32)
																									kSerializeVar(m_mtxEntityOffset, eSerializeType_Matrixf)
																										kSerializeVar(mlEntitySmoothPosNum, eSerializeType_Int32)

																											kSerializeVar(mfMaxStepHeight, eSerializeType_Float32)
																												kSerializeVar(mfStepClimbSpeed, eSerializeType_Float32)
																													kSerializeVar(mfClimbForwardMul, eSerializeType_Float32)
																														kSerializeVar(mfClimbHeightAdd, eSerializeType_Float32)
																															kSerializeVar(mbClimbing, eSerializeType_Bool)

																																kSerializeVar(mfGroundFriction, eSerializeType_Float32)

																																	kSerializeVar(mlBodyId, eSerializeType_Int32)

																																		kSerializeVarContainer(mvExtraBodyIds, eSerializeType_Int32)
																																			kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_iCharacterBody::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	iPhysicsWorld *pWorld = apGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	iCharacterBody *pBody = pWorld->CreateCharacterBody(msName, mvSize);

	// Destroy the recently created body.
	pWorld->DestroyBody(pBody->mpBody);
	pBody->mvExtraBodies.clear();

	return pBody;
}

//-----------------------------------------------------------------------

int cSaveData_iCharacterBody::GetSaveCreatePrio() {
	return 1;
}

//-----------------------------------------------------------------------

iSaveData *iCharacterBody::CreateSaveData() {
	return hplNew(cSaveData_iCharacterBody, ());
}

//-----------------------------------------------------------------------

void iCharacterBody::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(iCharacterBody);

	//////////////////////////
	// Variables
	kSaveData_SaveTo(msName);

	kSaveData_SaveTo(mfMass);
	kSaveData_SaveTo(mbGravityActive);
	kSaveData_SaveTo(mfMaxGravitySpeed);

	kSaveData_SaveTo(mbActive);

	kSaveData_SaveTo(mbCollideCharacter);

	kSaveData_SaveTo(mvPosition);
	kSaveData_SaveTo(mvLastPosition);

	kSaveData_SaveTo(mfMaxPosMoveSpeed[0]);
	kSaveData_SaveTo(mfMaxNegMoveSpeed[0]);
	kSaveData_SaveTo(mfMoveSpeed[0]);
	kSaveData_SaveTo(mfMoveAcc[0]);
	kSaveData_SaveTo(mfMoveDeacc[0]);
	kSaveData_SaveTo(mbMoving[0]);

	kSaveData_SaveTo(mfMaxPosMoveSpeed[1]);
	kSaveData_SaveTo(mfMaxNegMoveSpeed[1]);
	kSaveData_SaveTo(mfMoveSpeed[1]);
	kSaveData_SaveTo(mfMoveAcc[1]);
	kSaveData_SaveTo(mfMoveDeacc[1]);
	kSaveData_SaveTo(mbMoving[1]);

	kSaveData_SaveTo(mfPitch);
	kSaveData_SaveTo(mfYaw);

	kSaveData_SaveTo(mbOnGround);

	kSaveData_SaveTo(mfMaxPushMass);
	kSaveData_SaveTo(mfPushForce);

	kSaveData_SaveTo(mvForce);
	kSaveData_SaveTo(mvVelolcity);

	kSaveData_SaveTo(mvSize);

	kSaveData_SaveTo(m_mtxMove);

	kSaveData_SaveTo(m_mtxEntityOffset);
	kSaveData_SaveTo(mlEntitySmoothPosNum);

	kSaveData_SaveTo(mfMaxStepHeight);
	kSaveData_SaveTo(mfStepClimbSpeed);
	kSaveData_SaveTo(mfClimbForwardMul);
	kSaveData_SaveTo(mfClimbHeightAdd);
	kSaveData_SaveTo(mbClimbing);

	kSaveData_SaveTo(mfGroundFriction);

	//////////////////////////
	// Containers

	//////////////////////////
	// Pointers
	kSaveData_SaveObject(mpEntity, mlEntityId);
	kSaveData_SaveObject(mpBody, mlBodyId);
	kSaveData_SaveIdList(mvExtraBodies, Common::Array<iPhysicsBody *>::iterator, mvExtraBodyIds);
}

//-----------------------------------------------------------------------

void iCharacterBody::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(iCharacterBody);

	//////////////////////////
	// Variables
	kSaveData_LoadFrom(msName);

	kSaveData_LoadFrom(mfMass);
	kSaveData_LoadFrom(mbGravityActive);
	kSaveData_LoadFrom(mfMaxGravitySpeed);

	kSaveData_LoadFrom(mbActive);

	kSaveData_LoadFrom(mbCollideCharacter);

	kSaveData_LoadFrom(mvPosition);
	kSaveData_LoadFrom(mvLastPosition);

	kSaveData_LoadFrom(mfMaxPosMoveSpeed[0]);
	kSaveData_LoadFrom(mfMaxNegMoveSpeed[0]);
	kSaveData_LoadFrom(mfMoveSpeed[0]);
	kSaveData_LoadFrom(mfMoveAcc[0]);
	kSaveData_LoadFrom(mfMoveDeacc[0]);
	kSaveData_LoadFrom(mbMoving[0]);

	kSaveData_LoadFrom(mfMaxPosMoveSpeed[1]);
	kSaveData_LoadFrom(mfMaxNegMoveSpeed[1]);
	kSaveData_LoadFrom(mfMoveSpeed[1]);
	kSaveData_LoadFrom(mfMoveAcc[1]);
	kSaveData_LoadFrom(mfMoveDeacc[1]);
	kSaveData_LoadFrom(mbMoving[1]);

	kSaveData_LoadFrom(mfPitch);
	kSaveData_LoadFrom(mfYaw);

	kSaveData_LoadFrom(mbOnGround);

	kSaveData_LoadFrom(mfMaxPushMass);
	kSaveData_LoadFrom(mfPushForce);

	kSaveData_LoadFrom(mvForce);
	kSaveData_LoadFrom(mvVelolcity);

	kSaveData_LoadFrom(mvSize);

	kSaveData_LoadFrom(m_mtxMove);

	kSaveData_LoadFrom(m_mtxEntityOffset);
	kSaveData_LoadFrom(mlEntitySmoothPosNum);

	kSaveData_LoadFrom(mfMaxStepHeight);
	kSaveData_LoadFrom(mfStepClimbSpeed);
	kSaveData_LoadFrom(mfClimbForwardMul);
	kSaveData_LoadFrom(mfClimbHeightAdd);
	kSaveData_LoadFrom(mbClimbing);

	kSaveData_LoadFrom(mfGroundFriction);
}

//-----------------------------------------------------------------------

void iCharacterBody::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(iCharacterBody);

	//////////////////////////
	// Pointers
	kSaveData_LoadObject(mpEntity, mlEntityId, iEntity3D *);
	kSaveData_LoadObject(mpBody, mlBodyId, iPhysicsBody *);
	// kSaveData_LoadIdList(mvExtraBodies,mvExtraBodyIds,iPhysicsBody*);
	mvExtraBodies.clear();
	cContainerListIterator<int> it = pData->mvExtraBodyIds.GetIterator();
	while (it.HasNext()) {
		int lId = it.Next();
		iPhysicsBody *pBody = static_cast<iPhysicsBody *>(apSaveObjectHandler->Get(lId));
		mvExtraBodies.push_back(pBody);
		if (pBody == NULL) {
			Warning("Couldn't find save object with id %d\n", lId);
		}
	}

	// Make sure all bodies are setup
	SetCollideCharacter(mbCollideCharacter);
}

//-----------------------------------------------------------------------

} // namespace hpl
