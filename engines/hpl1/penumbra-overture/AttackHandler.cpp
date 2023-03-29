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

#include "hpl1/penumbra-overture/AttackHandler.h"

#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameObject.h"
#include "hpl1/penumbra-overture/GameSwingDoor.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

//////////////////////////////////////////////////////////////////////////
// RAY CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAttackRayCallback::cAttackRayCallback() {
	mbSkipCharacter = false;
}

//-----------------------------------------------------------------------

void cAttackRayCallback::Reset() {
	mpClosestBody = NULL;
	mbSkipCharacter = false;
}

//-----------------------------------------------------------------------

bool cAttackRayCallback::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	if (pBody->GetCollide() == false)
		return true;
	if (pBody == mpSkipBody)
		return true;
	if (mbSkipCharacter && pBody->IsCharacter())
		return true;

	if (apParams->mfDist < mfShortestDist || mpClosestBody == NULL) {
		mpClosestBody = pBody;
		mfShortestDist = apParams->mfDist;
		mvPosition = apParams->mvPoint;
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SPLASH DAMAGE BLOCK CHECK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSplashDamageBlockCheck::cSplashDamageBlockCheck(cInit *apInit) {
	mpInit = apInit;
	mbIntersected = false;
}

//-----------------------------------------------------------------------

bool cSplashDamageBlockCheck::CheckBlock(const cVector3f &avStart, const cVector3f &avEnd) {
	mbIntersected = false;
	iPhysicsWorld *pWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	pWorld->CastRay(this, avStart, avEnd, false, false, false, true);

	return mbIntersected;
}

//-----------------------------------------------------------------------

bool cSplashDamageBlockCheck::BeforeIntersect(iPhysicsBody *pBody) {
	if (pBody->IsCharacter() || pBody->GetMass() != 0 || pBody->GetBlocksSound() == false) {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

bool cSplashDamageBlockCheck::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	mbIntersected = true;
	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAttackHandler::cAttackHandler(cInit *apInit) : iUpdateable("AttackHandler") {
	mpInit = apInit;

	mpSplashBlockCheck = hplNew(cSplashDamageBlockCheck, (apInit));
}

//-----------------------------------------------------------------------

cAttackHandler::~cAttackHandler(void) {
	hplDelete(mpSplashBlockCheck);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cAttackHandler::CreateLineAttack(const cVector3f &avStart, const cVector3f &avEnd, float afDamage,
									  eAttackTargetFlag aTarget, iPhysicsBody *apSkipBody,
									  iPhysicsBody **apPickedBody) {
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	mRayCallback.mTarget = aTarget;
	mRayCallback.mpSkipBody = apSkipBody;

	if (apPickedBody)
		*apPickedBody = NULL;

	mRayCallback.Reset();
	pPhysicsWorld->CastRay(&mRayCallback, avStart, avEnd, true, false, true);

	if (mRayCallback.mpClosestBody == NULL)
		return false;

	if (apPickedBody)
		*apPickedBody = mRayCallback.mpClosestBody;

	if (aTarget & eAttackTargetFlag_Player) {
		if (mpInit->mpPlayer->GetCharacterBody()->GetBody() == mRayCallback.mpClosestBody) {
			if (afDamage > 0)
				mpInit->mpPlayer->Damage(afDamage, ePlayerDamageType_BloodSplash);
			return true;
		}
	}

	if (aTarget & eAttackTargetFlag_Enemy) {
	}

	return false;
}

//-----------------------------------------------------------------------

bool cAttackHandler::CreateShapeAttack(iCollideShape *apShape, const cMatrixf &a_mtxOffset,
									   const cVector3f &avOrigin, float afDamage,
									   float afMinMass, float afMaxMass, float afMinImpulse, float afMaxImpulse,
									   int alStrength,
									   eAttackTargetFlag aTarget, iPhysicsBody *apSkipBody) {
	bool bHit = false;

	tPhysicsBodyList lstBodies;

	mpLastSwingDoor = NULL;

	///////////////////////////////
	// Set up boudning box
	cBoundingVolume tempBV = apShape->GetBoundingVolume();
	tempBV.SetTransform(a_mtxOffset);

	///////////////////////////////
	// Iterate bodies
	cCollideData collideData;
	collideData.SetMaxSize(1);

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();

	// Get bodies and add to list, this incase the portal contaniner gets changed.
	Common::List<iPhysicsBody *> lstTempBodies;
	cPhysicsBodyIterator bodyIt = pPhysicsWorld->GetBodyIterator();
	while (bodyIt.HasNext()) {
		iPhysicsBody *pBody = static_cast<iPhysicsBody *>(bodyIt.Next());
		lstTempBodies.push_back(pBody);
	}

	Common::List<iPhysicsBody *>::iterator it = lstTempBodies.begin();
	for (; it != lstTempBodies.end(); ++it) {
		iPhysicsBody *pBody = *it;
		/*float fMass = */ pBody->GetMass();

		if (pBody->IsActive() == false)
			continue;
		if (pBody->GetCollide() == false)
			continue;

		if (cMath::CheckCollisionBV(tempBV, *pBody->GetBV())) {
			iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();

			///////////////////////////////
			// Check for collision
			if (pPhysicsWorld->CheckShapeCollision(pBody->GetShape(), pBody->GetLocalMatrix(),
												   apShape, a_mtxOffset, collideData, 1) == false) {
				continue;
			}

			///////////////////////////
			// Player
			if (aTarget & eAttackTargetFlag_Player) {
				if (mpInit->mpPlayer->GetCharacterBody()->GetBody() == pBody) {
					// Check with line if there is a free path, if not skip damage.
					cVector3f vEnd = pBody->GetWorldPosition();

					mRayCallback.Reset();
					mRayCallback.mbSkipCharacter = true;
					pPhysicsWorld->CastRay(&mRayCallback, avOrigin, vEnd, true, false, false);
					mRayCallback.mbSkipCharacter = false;

					// Damage
					if (mRayCallback.mpClosestBody == NULL) {
						if (afDamage > 0) {
							mpInit->mpPlayer->Damage(afDamage, ePlayerDamageType_BloodSplash);
						}
					}

					// Impulse
					float fMass2 = mpInit->mpPlayer->GetCharacterBody()->GetMass();
					float fForceSize = 0;
					if (fMass2 > afMaxMass * 10)
						fForceSize = 0;
					else if (fMass2 <= afMinMass * 10)
						fForceSize = afMaxImpulse * 10;
					else {
						float fT = (fMass2 - afMinMass * 10) / (afMaxMass * 10 - afMinMass * 10);
						fForceSize = afMinImpulse * 10 * fT + afMaxImpulse * 10 * (1 - fT);
					}

					cVector3f vForceDir = mpInit->mpPlayer->GetCharacterBody()->GetPosition() - avOrigin;
					vForceDir.Normalise();
					vForceDir += cVector3f(0, 0.1f, 0);

					mpInit->mpPlayer->GetCharacterBody()->AddForce(vForceDir * fForceSize * 300);

					bHit = true;
					continue;
				}
			}

			///////////////////////////
			// Enemy
			if (aTarget & eAttackTargetFlag_Enemy) {
			}

			///////////////////////////
			// Bodies
			if (aTarget & eAttackTargetFlag_Bodies) {
				if (pBody->IsCharacter() == false) {
					lstBodies.push_back(pBody);

					if (pEntity) {
						pEntity->Damage(afDamage, alStrength);

						if (pEntity->GetType() == eGameEntityType_SwingDoor) {
							mpLastSwingDoor = static_cast<cGameSwingDoor *>(pEntity);
						}
					}

					bHit = true;
				}
			}
		}
	}

	////////////////////////////////
	// Iterate bodies hit, this to
	// spread out the impulse.
	float fAmount = (float)lstBodies.size();
	for (tPhysicsBodyListIt it2 = lstBodies.begin(); it2 != lstBodies.end(); ++it2) {
		iPhysicsBody *pBody = *it2;

		// Calculate force
		float fMass = pBody->GetMass();
		float fForceSize = 0;
		if (fMass > afMaxMass)
			fForceSize = 0;
		else if (fMass <= afMinMass)
			fForceSize = afMaxImpulse;
		else {
			float fT = (fMass - afMinMass) / (afMaxMass - afMinMass);
			fForceSize = afMinImpulse * fT + afMaxImpulse * (1 - fT);
		}

		fForceSize /= fAmount;

		if (fMass > 0 && fForceSize > 0) {
			cVector3f vDir = pBody->GetWorldPosition() - avOrigin;
			vDir.Normalise();

			pBody->AddImpulse(vDir * fForceSize);
		}
	}

	return bHit;
}

//-----------------------------------------------------------------------

bool cAttackHandler::CreateLineDestroyBody(const cVector3f &avStart, const cVector3f &avEnd,
										   float afStrength, float afForce, iPhysicsBody *apSkipBody,
										   iPhysicsBody **apPickedBody) {
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	mRayCallback.mpSkipBody = apSkipBody;

	if (apPickedBody)
		*apPickedBody = NULL;

	mRayCallback.Reset();
	pPhysicsWorld->CastRay(&mRayCallback, avStart, avEnd, true, false, true);

	if (mRayCallback.mpClosestBody == NULL)
		return false;

	if (apPickedBody)
		*apPickedBody = mRayCallback.mpClosestBody;
	iPhysicsBody *pBody = mRayCallback.mpClosestBody;

	if (pBody->IsCharacter() == false && pBody->GetMass() > 0 && pBody->GetUserData()) {
		iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();
		if (pEntity->GetType() != eGameEntityType_Object)
			return false;

		cGameObject *pObject = static_cast<cGameObject *>(pEntity);

		cVector3f vForward = avEnd - avStart;
		vForward.Normalise();

		pBody->AddForce(vForward * afForce);

		// Destroy object if possible.
		if (pObject->IsDestroyable() && pObject->GetDestroyStrength() <= afStrength) {
			for (int i = 0; i < pBody->GetJointNum(); ++i) {
				iPhysicsJoint *pJoint = pBody->GetJoint(i);
				pJoint->Break();
			}

			if (pObject->GetInteractMode() == eObjectInteractMode_Move)
				pObject->SetInteractMode(eObjectInteractMode_Grab);

			if (pObject->GetDestroySound() != "") {
				cSoundEntity *pSound = mpInit->mpGame->GetScene()->GetWorld3D()->CreateSoundEntity(
					"Destroy", pObject->GetDestroySound(),
					true);
				if (pSound)
					pSound->SetPosition(pBody->GetWorldPosition());
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

static float CalcSize(float afDist, float afMaxDist, float afMinVal, float afMaxVal) {
	float fMul = 1 - (afDist / afMaxDist);
	if (fMul < 0)
		return 0;

	return afMinVal + (afMaxVal - afMinVal) * fMul;
}

//----------------------------------------------------------------------

void cAttackHandler::CreateSplashDamage(const cVector3f &avCenter, float afRadius, float afMinDamage,
										float afMaxDamge, float afMinForce, float afMaxForce,
										float afMaxImpulse,
										unsigned int aTarget, float afMinMass,
										int alStrength) {
	cBoundingVolume tempBV;
	tempBV.SetSize(afRadius * 2);
	tempBV.SetPosition(avCenter);

	if (aTarget & eAttackTargetFlag_Player) {
		cBoundingVolume *pCharBV = mpInit->mpPlayer->GetCharacterBody()->GetBody()->GetBV();
		if (cMath::CheckCollisionBV(tempBV, *pCharBV) &&
			mpSplashBlockCheck->CheckBlock(pCharBV->GetWorldCenter(), avCenter) == false) {
			float fDist = cMath::Vector3Dist(pCharBV->GetPosition(), avCenter);
			mpInit->mpPlayer->Damage(CalcSize(fDist, afRadius, afMinDamage, afMaxDamge), ePlayerDamageType_BloodSplash);

			float fForceSize = CalcSize(fDist, afRadius, afMinForce, afMaxForce);
			cVector3f vForceDir = mpInit->mpPlayer->GetCharacterBody()->GetPosition() - avCenter;
			vForceDir.Normalise();

			mpInit->mpPlayer->GetCharacterBody()->AddForce(vForceDir * fForceSize * 10);
		}
	}
	// Enemies now have the user data added to the body, this is not needed.
	/*if(aTarget & eAttackTargetFlag_Enemy)
	{
		tGameEnemyIterator it = mpMapHandler->GetGameEnemyIterator();
		while(it.HasNext())
		{
			iGameEnemy *pEnemy = it.Next();

			cBoundingVolume* pCharBV = pEnemy->GetMover()->GetCharBody()->GetBody()->GetBV();
			if(cMath::CheckCollisionBV(tempBV, *pCharBV))
			{
				float fDist = cMath::Vector3Dist(pCharBV->GetPosition(),avCenter);

				pEnemy->Damage(CalcSize(fDist,afRadius,afMinDamage,afMaxDamge),-1);
			}
		}
	}*/
	if (aTarget & eAttackTargetFlag_Bodies) {
		iPhysicsWorld *pWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

		Common::List<iPhysicsBody *> lstBodies;
		cPhysicsBodyIterator bodyIt = pWorld->GetBodyIterator();
		while (bodyIt.HasNext()) {
			lstBodies.push_back(bodyIt.Next());
		}

		//////////////////////////
		// Damage Iteration
		Common::List<iPhysicsBody *>::iterator it = lstBodies.begin();
		for (; it != lstBodies.end(); ++it) {
			iPhysicsBody *pBody = *it;

			// if(pBody->IsCharacter() || pBody->GetMass()==0) continue;
			// if(pBody->IsCharacter()) continue;
			if (pBody->IsActive() == false)
				continue;
			// if(pBody->GetMass() <= afMinMass) continue;

			iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();

			if (pEntity && pEntity->IsActive() && cMath::CheckCollisionBV(tempBV, *pBody->GetBV()) && mpSplashBlockCheck->CheckBlock(pBody->GetWorldPosition(), avCenter) == false) {
				// If enemies are not to be target, skip.
				if (!(aTarget & eAttackTargetFlag_Enemy) && pEntity->GetType() == eGameEntityType_Enemy) {
					continue;
				}

				float fDist = cMath::Vector3Dist(avCenter, pBody->GetLocalPosition());
				/*float fForceSize = CalcSize(fDist,afRadius,afMinForce,afMaxForce);
				cVector3f vForceDir = pBody->GetLocalPosition() - avCenter;
				vForceDir.Normalise();

				if(fForceSize / pBody->GetMass() > afMaxImpulse)
				{
					fForceSize = afMaxImpulse * pBody->GetMass();
				}

				pBody->AddForce(vForceDir * fForceSize);*/

				if (pEntity) {
					float fDamage = CalcSize(fDist, afRadius, afMinDamage, afMaxDamge);
					pEntity->Damage(fDamage, alStrength);
				}
			}
		}

		// Update map handler so all stuff that should be broken is.
		// mpInit->mpMapHandler->Update(1.0f/60.0f);

		//////////////////////////
		// Force Iteration
		it = lstBodies.begin();
		for (; it != lstBodies.end(); ++it) {
			iPhysicsBody *pBody = *it;

			if (pBody->IsCharacter() || pBody->GetMass() == 0)
				continue;
			if (pBody->IsActive() == false)
				continue;
			if (pBody->GetMass() <= afMinMass)
				continue;

			iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();

			if (cMath::CheckCollisionBV(tempBV, *pBody->GetBV())) {
				cVector3f vBodyPos = pBody->GetLocalPosition() +
									 cMath::MatrixMul(pBody->GetLocalMatrix().GetRotation(),
													  pBody->GetMassCentre());

				float fDist = cMath::Vector3Dist(avCenter, vBodyPos);
				float fForceSize = CalcSize(fDist, afRadius, afMinForce, afMaxForce);
				cVector3f vForceDir = vBodyPos - avCenter;
				vForceDir.Normalise();

				if (fForceSize / pBody->GetMass() > afMaxImpulse) {
					fForceSize = afMaxImpulse * pBody->GetMass();
				}
				pBody->AddForce(vForceDir * fForceSize);

				if (pEntity) {
					float fImpulse = (fForceSize / pBody->GetMass()) * 1.0f / 60.0f;
					if (fImpulse > 15)
						fImpulse = 15;

					pEntity->SetLastImpulse(vForceDir * fImpulse);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void cAttackHandler::OnStart() {
	mpMapHandler = mpInit->mpMapHandler;
}

//-----------------------------------------------------------------------

void cAttackHandler::Update(float afTimeStep) {
}

//-----------------------------------------------------------------------

void cAttackHandler::Reset() {
}
