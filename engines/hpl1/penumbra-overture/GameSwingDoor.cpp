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

#include "hpl1/penumbra-overture/GameSwingDoor.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEntityLoader_GameSwingDoor::cEntityLoader_GameSwingDoor(const tString &asName, cInit *apInit)
	: cEntityLoader_Object(asName) {
	mpInit = apInit;
}

cEntityLoader_GameSwingDoor::~cEntityLoader_GameSwingDoor() {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameSwingDoor::BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
											 cWorld3D *apWorld) {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameSwingDoor::AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
											cWorld3D *apWorld) {
	cGameSwingDoor *pObject = hplNew(cGameSwingDoor, (mpInit, mpEntity->GetName()));

	pObject->msFileName = msFileName;
	pObject->m_mtxOnLoadTransform = a_mtxTransform;

	// Set the engine objects to the object
	pObject->SetBodies(mvBodies);
	pObject->SetJoints(mvJoints);
	pObject->SetMeshEntity(mpEntity);

	///////////////////////////////////
	// Load game properties
	TiXmlElement *pGameElem = apRootElem->FirstChildElement("GAME");
	if (pGameElem) {
		pObject->mfHealth = cString::ToFloat(pGameElem->Attribute("Health"), 0);
		pObject->mlToughness = cString::ToInt(pGameElem->Attribute("Toughness"), 0);

		pObject->msBreakSound = cString::ToString(pGameElem->Attribute("BreakSound"), "");

		pObject->msBreakEntity = cString::ToString(pGameElem->Attribute("BreakEntity"), "");
		pObject->msBreakPS = cString::ToString(pGameElem->Attribute("BreakPS"), "");
	} else {
		Error("Couldn't find game element for entity '%s'\n", mpEntity->GetName().c_str());
	}

	///////////////////////////////////
	// Add a the object as user data to the body, to get the obejct later on.
	for (size_t i = 0; i < mvBodies.size(); ++i) {
		if (mvBodies[i]) {
			mvBodies[i]->SetUserData((void *)pObject);
		}
	}

	pObject->SetupPhysics(apWorld);
	pObject->SetupBreakObject();

	/////////////////////////////////
	// Add to map handler
	mpInit->mpMapHandler->AddGameEntity(pObject);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameSwingDoor::cGameSwingDoor(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_SwingDoor;
	mbHasInteraction = true;

	mbPauseControllers = true;
	mbPauseGravity = true;

	mbLocked = false;

	mfHealth = 100.0f;
}

//-----------------------------------------------------------------------

cGameSwingDoor::~cGameSwingDoor(void) {
	/*iPhysicsWorld *pPhysicsWorld = */ mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	// for(size_t i=0; i<mvStopControllers.size(); ++i)
	//{
	//	pPhysicsWorld->DestroyController(mvStopControllers[i]);
	// }
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameSwingDoor::SetupPhysics(cWorld3D *apWorld) {
	mvJointDefaults.resize(mvJoints.size());

	iPhysicsWorld *pPhysicsWorld = apWorld->GetPhysicsWorld();

	for (size_t i = 0; i < mvJoints.size(); ++i) {
		iPhysicsJoint *pJoint = mvJoints[i];

		///////////////////////////////////
		// Create Stop controller
		iPhysicsController *pController = pPhysicsWorld->CreateController("Stop");

		// pController->SetBody(pJoint->GetChildBody());
		// pController->SetJoint(pJoint);

		pController->SetActive(true);
		pController->SetType(ePhysicsControllerType_Pid);
		pController->SetA(1.5f);
		pController->SetB(1.0f);
		pController->SetC(0.0f);
		pController->SetPidIntegralSize(20);

		pController->SetInputType(ePhysicsControllerInput_AngularSpeed, ePhysicsControllerAxis_Y);
		pController->SetDestValue(.0f);

		pController->SetOutputType(ePhysicsControllerOutput_Torque, ePhysicsControllerAxis_Y);
		pController->SetMaxOutput(0.0f);
		pController->SetMulMassWithOutput(true);

		pJoint->AddController(pController);

		mvStopControllers.push_back(pController);

		///////////////////////////////////
		// Set defaults
		iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge *>(pJoint);

		mvJointDefaults[i].mfMin = pHingeJoint->GetMinAngle();
		mvJointDefaults[i].mfMax = pHingeJoint->GetMaxAngle();
	}
}

//-----------------------------------------------------------------------

void cGameSwingDoor::OnPlayerPick() {
}

//-----------------------------------------------------------------------

void cGameSwingDoor::OnPlayerInteract() {
	float fDist = mpInit->mpPlayer->GetPickedDist();

	if (fDist > mfMaxInteractDist)
		return;

	// Set some properties
	mpInit->mpPlayer->mfForwardUpMul = 1.0f;
	mpInit->mpPlayer->mfForwardRightMul = 1.0f;

	mpInit->mpPlayer->mfUpMul = 1.0f;
	mpInit->mpPlayer->mfRightMul = 1.0f;

	mpInit->mpPlayer->mfCurrentMaxInteractDist = mfMaxInteractDist;

	mpInit->mpPlayer->SetPushBody(mpInit->mpPlayer->GetPickedBody());
	mpInit->mpPlayer->ChangeState(ePlayerState_Move);
}

//-----------------------------------------------------------------------

void cGameSwingDoor::Update(float afTimeStep) {
}

//-----------------------------------------------------------------------

void cGameSwingDoor::SetLocked(bool abX) {
	if (mbLocked == abX)
		return;

	mbLocked = abX;

	for (size_t i = 0; i < mvJoints.size(); ++i) {
		iPhysicsJoint *pJoint = mvJoints[i];
		iPhysicsJointHinge *pHingeJoint = static_cast<iPhysicsJointHinge *>(pJoint);

		if (mbLocked) {
			if (ABS(pHingeJoint->GetMinAngle()) > ABS(pHingeJoint->GetMaxAngle()))
				pHingeJoint->SetMinAngle(cMath::ToRad(-1));
			else
				pHingeJoint->SetMaxAngle(cMath::ToRad(1));
		} else {
			pHingeJoint->SetMinAngle(mvJointDefaults[i].mfMin);
			pHingeJoint->SetMaxAngle(mvJointDefaults[i].mfMax);
		}
	}

	for (size_t i = 0; i < mvBodies.size(); ++i) {
		iPhysicsBody *pBody = mvBodies[i];
		pBody->SetEnabled(true);
	}
}

//-----------------------------------------------------------------------

void cGameSwingDoor::OnDamage(float afDamage) {
}

//-----------------------------------------------------------------------

void cGameSwingDoor::OnDeath(float afDamage) {
	if (msBreakSound != "") {
		cSoundEntity *pSound = mpInit->mpGame->GetScene()->GetWorld3D()->CreateSoundEntity("Break", msBreakSound, true);
		if (pSound)
			pSound->SetPosition(mvBodies[0]->GetWorldPosition());
	}

	if (msBreakEntity == "") {
		///////////////////////////
		// Break all joints.
		for (size_t i = 0; i < mvJoints.size(); ++i) {
			mvJoints[i]->Break();
		}
		mvJoints.clear();
	} else {
		Break();
	}
}

//-----------------------------------------------------------------------

void cGameSwingDoor::BreakAction() {
	if (mvBodies.empty())
		return;

	iPhysicsBody *pDynBody = NULL;
	iPhysicsBody *pStaticBody = NULL;

	for (size_t i = 0; i < mvBodies.size(); ++i) {
		if (mvBodies[i]->GetMass() != 0)
			pDynBody = mvBodies[i];
		if (mvBodies[i]->GetMass() == 0)
			pStaticBody = mvBodies[i];
	}

	if (pDynBody == NULL && pStaticBody == NULL)
		return;

	//////////////////
	// Script
	if (mvCallbackScripts[eGameEntityScriptType_OnBreak]) {
		tString sCommand = GetScriptCommand(eGameEntityScriptType_OnBreak);
		mpInit->RunScriptCommand(sCommand);
	}

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	//////////////////
	// Check if player os holding object
	if ((mpInit->mpPlayer->GetState() == ePlayerState_Grab ||
		 mpInit->mpPlayer->GetState() == ePlayerState_Move ||
		 mpInit->mpPlayer->GetState() == ePlayerState_Push) &&
		(mpInit->mpPlayer->GetPushBody() == pDynBody ||
		 mpInit->mpPlayer->GetPushBody() == pStaticBody)) {
		mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
	}

	/*cBoundingVolume *pBV = */ mpMeshEntity->GetBoundingVolume();

	//////////////////
	// Particle System
	if (msBreakPS != "") {
		/*cParticleSystem3D *pPS = */ pWorld->CreateParticleSystem("Break", msBreakPS, cVector3f(1, 1, 1),
																   pDynBody->GetWorldMatrix());
	}

	//////////////////
	// Entity
	if (msBreakEntity != "") {
		iEntity3D *pEntity = pWorld->CreateEntity(mpMeshEntity->GetName() + "_broken",
												  pDynBody->GetWorldMatrix(),
												  msBreakEntity, true);
		if (pEntity) {
			iGameEntity *pGameEntity = mpInit->mpMapHandler->GetLatestEntity();

			cVector3f vImpulse = mvLastImpulse * 2; // / (float)pGameEntity->GetBodyNum();

			for (int i = 0; i < pGameEntity->GetBodyNum(); ++i) {
				// Add the object velocity
				iPhysicsBody *pNewBody = pGameEntity->GetBody(i);

				if (pNewBody->GetMass() == 0 && pStaticBody) {
					pNewBody->SetMatrix(pStaticBody->GetWorldMatrix());
				}

				// pNewBody->SetLinearVelocity(pBody->GetLinearVelocity());
				pNewBody->AddImpulse(vImpulse);
			}
		}
	}

	mpInit->mpGame->ResetLogicTimer();
}

//-----------------------------------------------------------------------

void cGameSwingDoor::SetupBreakObject() {
	if (msBreakEntity != "")
		PreloadModel(msBreakEntity);
	if (msBreakPS != "") {
		cParticleSystem3D *pPS = mpInit->mpGame->GetResources()->GetParticleManager()->CreatePS3D(
			"", msBreakPS, 1, cMatrixf::Identity);
		hplDelete(pPS);
	}
	if (msBreakSound != "") {
		mpInit->PreloadSoundEntityData(msBreakSound);
	}
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cGameSwingDoor_SaveData, iGameEntity_SaveData)
	kSerializeVar(mbLocked, eSerializeType_Bool)

		kSerializeVar(msBreakSound, eSerializeType_String)
			kSerializeVar(msBreakEntity, eSerializeType_String)
				kSerializeVar(msBreakPS, eSerializeType_String)
					kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameSwingDoor_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameSwingDoor::CreateSaveData() {
	return hplNew(cGameSwingDoor_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameSwingDoor::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameSwingDoor_SaveData *pData = static_cast<cGameSwingDoor_SaveData *>(apSaveData);

	kCopyToVar(pData, mbLocked);

	kCopyToVar(pData, msBreakSound);
	kCopyToVar(pData, msBreakEntity);
	kCopyToVar(pData, msBreakPS);
}

//-----------------------------------------------------------------------

void cGameSwingDoor::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameSwingDoor_SaveData *pData = static_cast<cGameSwingDoor_SaveData *>(apSaveData);

	kCopyFromVar(pData, msBreakSound);
	kCopyFromVar(pData, msBreakEntity);
	kCopyFromVar(pData, msBreakPS);

	SetLocked(pData->mbLocked);
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
