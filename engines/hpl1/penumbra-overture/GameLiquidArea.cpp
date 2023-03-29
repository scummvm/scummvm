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

#include "hpl1/penumbra-overture/GameLiquidArea.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAreaLoader_GameLiquidArea::cAreaLoader_GameLiquidArea(const tString &asName, cInit *apInit)
	: iArea3DLoader(asName) {
	mpInit = apInit;
}

cAreaLoader_GameLiquidArea::~cAreaLoader_GameLiquidArea() {
}

//-----------------------------------------------------------------------

iEntity3D *cAreaLoader_GameLiquidArea::Load(const tString &asName, const cVector3f &avSize,
											const cMatrixf &a_mtxTransform, cWorld3D *apWorld) {
	cGameLiquidArea *pArea = hplNew(cGameLiquidArea, (mpInit, asName));

	pArea->m_mtxOnLoadTransform = a_mtxTransform;

	// Create physics data
	iPhysicsWorld *pPhysicsWorld = apWorld->GetPhysicsWorld();
	iCollideShape *pShape = pPhysicsWorld->CreateBoxShape(avSize, NULL);
	Common::Array<iPhysicsBody *> vBodies;
	vBodies.push_back(pPhysicsWorld->CreateBody(asName, pShape));

	vBodies[0]->SetCollide(false);
	vBodies[0]->SetCollideCharacter(false);
	vBodies[0]->SetMatrix(a_mtxTransform);

	vBodies[0]->SetUserData(pArea);
	pArea->SetBodies(vBodies);

	mpInit->mpMapHandler->AddGameEntity(pArea);

	pArea->Setup();

	// Return something else later perhaps.
	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

cGameLiquidArea::cGameLiquidArea(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_LiquidArea;

	mfDensity = 100;
	mfLinearViscosity = 1;
	mfAngularViscosity = 1;

	mbHasInteraction = false;

	mpPhysicsMaterial = NULL;

	mbHasWaves = true;
	mfWaveAmp = 0.04f;
	mfWaveFreq = 3;

	mfTimeCount = 0;
}

//-----------------------------------------------------------------------

cGameLiquidArea::~cGameLiquidArea(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameLiquidArea::SetPhysicsMaterial(const tString asName) {
	if (asName == "")
		return;

	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	mpPhysicsMaterial = pPhysicsWorld->GetMaterialFromName(asName);
	if (mpPhysicsMaterial == NULL) {
		Error("Liquid '%s' could not find material '%s'\n", GetName().c_str(),
			  mpPhysicsMaterial->GetName().c_str());
	}
}

//-----------------------------------------------------------------------

void cGameLiquidArea::OnPlayerPick() {
}

//-----------------------------------------------------------------------

void cGameLiquidArea::Update(float afTimeStep) {
	if (IsActive() == false)
		return;

	iPhysicsBody *pAreaBody = mvBodies[0];
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();
	cCamera3D *pCam = mpInit->mpPlayer->GetCamera();
	float fSurfaceY = mvBodies[0]->GetWorldPosition().y +
					  mvBodies[0]->GetShape()->GetSize().y / 2;

	cCollideData collideData;
	collideData.SetMaxSize(1);

	mfTimeCount += afTimeStep;

	////////////////////////////////////////////////////////
	// Update waves

	////////////////////////////////////////////////////////
	// Check if player camera is in water.
	if (cMath::PointBVCollision(pCam->GetPosition(), *pAreaBody->GetBV())) {
		if (mpInit->mpEffectHandler->GetUnderwater()->IsActive() == false) {
			mpInit->mpEffectHandler->GetUnderwater()->SetActive(true);
			mpInit->mpEffectHandler->GetUnderwater()->SetColor(mColor);
		}
	} else {
		mpInit->mpEffectHandler->GetUnderwater()->SetActive(false);
	}

	////////////////////////////////////////////////////////
	// Iterate all bodies in world and check for intersection
	cPortalContainerEntityIterator bodyIt = pWorld->GetPortalContainer()->GetEntityIterator(
		pAreaBody->GetBoundingVolume());
	while (bodyIt.HasNext()) {
		iPhysicsBody *pBody = static_cast<iPhysicsBody *>(bodyIt.Next());

		/*iGameEntity *pEntity = (iGameEntity *)*/ pBody->GetUserData();

		if (pBody->GetCollide() && pBody->IsActive()) {
			if (pBody->GetMass() == 0 && pBody->IsCharacter() == false)
				continue;

			/////////////////////////
			// Bounding volume check
			if (cMath::CheckCollisionBV(*pBody->GetBV(), *pAreaBody->GetBV()) == false) {
				pBody->SetBuoyancyActive(false);
				continue;
			}

			///////////////////////////////
			// Check for collision
			if (pPhysicsWorld->CheckShapeCollision(pBody->GetShape(), pBody->GetLocalMatrix(),
												   pAreaBody->GetShape(), pAreaBody->GetLocalMatrix(), collideData, 1) == false) {
				pBody->SetBuoyancyActive(false);
				continue;
			}

			if (pBody->IsCharacter()) {
				iCharacterBody *pCharBody = pBody->GetCharacterBody();

				float fToSurface = cMath::Abs(fSurfaceY - pCharBody->GetFeetPosition().y);
				float fCharHeight = pCharBody->GetSize().y;
				if (fToSurface > fCharHeight)
					fToSurface = fCharHeight;

				float fRadius = pCharBody->GetSize().x / 2;
				float fVolume = fToSurface * fRadius * fRadius * kPif;
				float fWaterWeight = fVolume * mfDensity;
				cVector3f vForce = pPhysicsWorld->GetGravity() * -fWaterWeight;

				// Log("Tosurface: %f Vol: %f\n",fToSurface,fVolume);

				pCharBody->AddForce(vForce);

				if (pBody->GetBuoyancyActive() == false) {
					SplashEffect(pBody);
					pBody->SetBuoyancyActive(true);
				}
			} else {
				if (pBody->GetBuoyancyActive() == false) {
					pBody->SetBuoyancySurface(mSurfacePlane);
					pBody->SetBuoyancyDensity(mfDensity);
					pBody->SetBuoyancyLinearViscosity(mfLinearViscosity);
					pBody->SetBuoyancyAngularViscosity(mfAngularViscosity);

					SplashEffect(pBody);
					// Log("Splash body: %s\n",pBody->GetName().c_str());

					pBody->SetBuoyancyActive(true);
				}

				if (mbHasWaves) {
					cVector3f vPos = cMath::MatrixMul(pBody->GetLocalMatrix(), pBody->GetMassCentre());

					float fAddX = sin(mfTimeCount * mfWaveFreq + vPos.x * 15) * mfWaveAmp;
					float fAddZ = sin(mfTimeCount * mfWaveFreq + vPos.z * 15) * mfWaveAmp;

					// pBody->AddForce(cVector3f(0, 9.8f * (fAddZ+fAddX)*pBody->GetMass()*2, 0));
					// pBody->AddForce(cVector3f(0, 9.8f * cMath::RandRectf(-0.1, 0.1f)*pBody->GetMass()*2, 0));
					// Log("F:%f Amp %f\n",fAddZ+fAddX,mfWaveAmp);
					// pBody->AddTorque(cVector3f((fAddZ+fAddX)*pBody->GetMass(), (fAddZ+fAddX)*pBody->GetMass(),
					//							(fAddZ+fAddX)*pBody->GetMass()));

					cPlanef tempPlane;
					tempPlane.FromNormalPoint(cVector3f(0, 1, 0),
											  cVector3f(0, fSurfaceY + fAddX + fAddZ, 0));

					pBody->SetBuoyancySurface(tempPlane);
					pBody->SetEnabled(true);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameLiquidArea::Setup() {
	// Log("SETUP!\n");
	float fHeight = mvBodies[0]->GetShape()->GetSize().y;
	cVector3f vPos = mvBodies[0]->GetWorldPosition();
	mSurfacePlane.FromNormalPoint(cVector3f(0, 1, 0),
								  cVector3f(0, vPos.y, 0) + cVector3f(0, fHeight / 2, 0));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameLiquidArea::SplashEffect(iPhysicsBody *apBody) {
	if (mpPhysicsMaterial == NULL)
		return;

	cSurfaceData *pSurface = mpPhysicsMaterial->GetSurfaceData();

	float fSpeed;
	if (apBody->IsCharacter())
		fSpeed = apBody->GetCharacterBody()->GetForceVelocity().Length();
	else
		fSpeed = apBody->GetLinearVelocity().Length();

	cSurfaceImpactData *pImpact = pSurface->GetImpactDataFromSpeed(fSpeed);

	if (pImpact == NULL)
		return;

	cVector3f vPos = cMath::MatrixMul(apBody->GetLocalMatrix(), apBody->GetMassCentre());

	vPos.y = mvBodies[0]->GetWorldPosition().y +
			 mvBodies[0]->GetShape()->GetSize().y / 2;

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	if (pImpact->GetPSName() != "") {
		pWorld->CreateParticleSystem("Splash", pImpact->GetPSName(), 1, cMath::MatrixTranslate(vPos));
	}

	if (pImpact->GetSoundName() != "") {
		cSoundEntity *pSound = pWorld->CreateSoundEntity("Splash", pImpact->GetSoundName(), true);
		if (pSound) {
			pSound->SetPosition(vPos);
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cGameLiquidArea_SaveData, iGameEntity_SaveData)
	kSerializeVar(mvSize, eSerializeType_Vector3f)

		kSerializeVar(mfDensity, eSerializeType_Float32)
			kSerializeVar(mfLinearViscosity, eSerializeType_Float32)
				kSerializeVar(mfAngularViscosity, eSerializeType_Float32)

					kSerializeVar(msPhysicsMaterial, eSerializeType_String)

						kSerializeVar(mColor, eSerializeType_Color)

							kSerializeVar(mbHasWaves, eSerializeType_Bool)

								kSerializeVar(mSurfacePlane, eSerializeType_Planef)
									kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameLiquidArea_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameLiquidArea::CreateSaveData() {
	return hplNew(cGameLiquidArea_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameLiquidArea::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameLiquidArea_SaveData *pData = static_cast<cGameLiquidArea_SaveData *>(apSaveData);

	kCopyToVar(pData, mfDensity);
	kCopyToVar(pData, mfLinearViscosity);
	kCopyToVar(pData, mfAngularViscosity);
	kCopyToVar(pData, mSurfacePlane);
	kCopyToVar(pData, mColor);
	kCopyToVar(pData, mbHasWaves);

	if (mpPhysicsMaterial)
		pData->msPhysicsMaterial = mpPhysicsMaterial->GetName();
	else
		pData->msPhysicsMaterial = "";

	pData->mvSize = mvBodies[0]->GetShape()->GetSize();
}

//-----------------------------------------------------------------------

void cGameLiquidArea::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameLiquidArea_SaveData *pData = static_cast<cGameLiquidArea_SaveData *>(apSaveData);

	kCopyFromVar(pData, mfDensity);
	kCopyFromVar(pData, mfLinearViscosity);
	kCopyFromVar(pData, mfAngularViscosity);
	kCopyFromVar(pData, mSurfacePlane);
	kCopyFromVar(pData, mColor);
	kCopyFromVar(pData, mbHasWaves);
	SetPhysicsMaterial(pData->msPhysicsMaterial);
}

//-----------------------------------------------------------------------

void cGameLiquidArea::SetupSaveData(iGameEntity_SaveData *apSaveData) {
	super::SetupSaveData(apSaveData);
}
//-----------------------------------------------------------------------
