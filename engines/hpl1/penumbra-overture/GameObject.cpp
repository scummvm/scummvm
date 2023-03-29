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

#include "hpl1/penumbra-overture/GameObject.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/AttackHandler.h"
#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameStickArea.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameObjectBodyCallback::cGameObjectBodyCallback(cInit *apInit, cGameObject *apObject) {
	mpInit = apInit;

	mpObject = apObject;
}

//-----------------------------------------------------------------------

bool cGameObjectBodyCallback::OnBeginCollision(iPhysicsBody *apBody, iPhysicsBody *apCollideBody) {
	return true;
}

//-----------------------------------------------------------------------

void cGameObjectBodyCallback::OnCollide(iPhysicsBody *apBody, iPhysicsBody *apCollideBody,
										cPhysicsContactData *apContactData) {
	// Log("OnCollide %s vs %s\n",apBody->GetName().c_str(),apCollideBody->GetName().c_str());

	/////////////////////////////////////////
	// Damage on enemies
	if (apCollideBody->IsCharacter()) {
		// Check if there is an enemy.
		iGameEnemy *pEnemy = (iGameEnemy *)apCollideBody->GetUserData();

		if (pEnemy && apBody->GetMass() > 4) {
			float fSpeed = apBody->GetLinearVelocity().Length();
			float fImpulseSize = fSpeed * apBody->GetMass();

			if (fSpeed > 4.5f && fImpulseSize > 25 && ABS(apContactData->mfMaxContactNormalSpeed) >= 1.0f) {
				pEnemy->Damage(fImpulseSize * 0.1f, 1);
			}
		}
	}

	/////////////////////////////////////////
	// Check if the object breaks
	if (mpObject->mBreakProps.mbActive) {
		float fImpulseSize = apBody->GetLinearVelocity().Length() * apBody->GetMass() +
							 apCollideBody->GetLinearVelocity().Length() * apCollideBody->GetMass();

		if (fImpulseSize >= mpObject->mBreakProps.mfMinImpulse &&
			ABS(apContactData->mfMaxContactNormalSpeed) >= mpObject->mBreakProps.mfMinNormalSpeed) {
			if (mpInit->mbDebugInteraction) {
				Log("------ Breakage ----------\n");
				Log(" Body '%s' by Body '%s'\n", apBody->GetName().c_str(), apCollideBody->GetName().c_str());
				Log(" Impulse: %f (%fm/s * %fkg) + (%fm/s * %fkg)\n", fImpulseSize,
					apBody->GetLinearVelocity().Length(), apBody->GetMass(),
					apCollideBody->GetLinearVelocity().Length(), apCollideBody->GetMass());
				Log("-------------------------\n");
			}

			mpObject->Break();
		}
	}

	////////////////////////////////////////////
	// Do Some Damage
	if (mpObject->mDamageProps.mbActive) {
		float fDamage = 0;

		// Damage by linear velocity.
		float fSpeed = apBody->GetLinearVelocity().Length();
		if (fSpeed > mpObject->mDamageProps.mfMinLinearDamageSpeed) {
			if (fSpeed > mpObject->mDamageProps.mfMaxLinearDamageSpeed)
				fSpeed = mpObject->mDamageProps.mfMaxLinearDamageSpeed;

			fDamage = mpObject->mDamageProps.mfMinDamage +
					  (mpObject->mDamageProps.mfMaxDamage - mpObject->mDamageProps.mfMinDamage) *
						  ((fSpeed - mpObject->mDamageProps.mfMinLinearDamageSpeed) /
						   (mpObject->mDamageProps.mfMaxLinearDamageSpeed - mpObject->mDamageProps.mfMinLinearDamageSpeed));
		}

		// Damage by angular velocity.
		fSpeed = apBody->GetAngularVelocity().Length();
		if (fSpeed > mpObject->mDamageProps.mfMinAngularDamageSpeed) {
			if (fSpeed > mpObject->mDamageProps.mfMaxAngularDamageSpeed)
				fSpeed = mpObject->mDamageProps.mfMaxAngularDamageSpeed;

			float fTempDamage = mpObject->mDamageProps.mfMinDamage +
								(mpObject->mDamageProps.mfMaxDamage - mpObject->mDamageProps.mfMinDamage) *
									((fSpeed - mpObject->mDamageProps.mfMinAngularDamageSpeed) /
									 (mpObject->mDamageProps.mfMaxAngularDamageSpeed - mpObject->mDamageProps.mfMinAngularDamageSpeed));

			if (fTempDamage > fDamage)
				fDamage = fTempDamage;
		}

		//////////////////////////
		// Do the damage
		if (fDamage > 0) {
			// Player
			if (apCollideBody == mpInit->mpPlayer->GetCharacterBody()->GetBody()) {
				mpInit->mpPlayer->Damage(fDamage, ePlayerDamageType_BloodSplash);
			}
			// Entity
			else {
				iGameEntity *pEntity = (iGameEntity *)apCollideBody->GetUserData();
				if (pEntity) {
					pEntity->Damage(fDamage, mpObject->mDamageProps.mlDamageStrength);
				}
			}
		}
	}

	/*if(apCollideBody->IsCharacter()==false) return;

	//Check if there is an enemy.
	iGameEnemy *pEnemy = NULL;
	if(apCollideBody->GetUserData())
	{
	pEnemy = (iGameEnemy*)apCollideBody->GetUserData();
	}

	////////////////////////////////////////////
	// Stun
	if(pEnemy && mpObject->GetInteractMode() != eObjectInteractMode_Push)
	{
	float fSpeed = apBody->GetLinearVelocity().Length();
	float fMass = apBody->GetMass();
	float fImpulse = fSpeed * fMass;

	if(pEnemy->mfMinStunSpeed <= fSpeed && pEnemy->mfMinStunImpulse <= fImpulse)
	{
	//Log("Stun by %s speed: %f mass: %f\n",apBody->GetName().c_str(),fSpeed,fMass);
	pEnemy->Stun(pEnemy->mfObjectCollideStun, true);
	}
	}*/
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cEntityLoader_GameObject::cEntityLoader_GameObject(const tString &asName, cInit *apInit)
	: cEntityLoader_Object(asName) {
	mpInit = apInit;
}

cEntityLoader_GameObject::~cEntityLoader_GameObject() {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameObject::BeforeLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
										  cWorld3D *apWorld) {
}

//-----------------------------------------------------------------------

void cEntityLoader_GameObject::AfterLoad(TiXmlElement *apRootElem, const cMatrixf &a_mtxTransform,
										 cWorld3D *apWorld) {
	cGameObject *pObject = hplNew(cGameObject, (mpInit, mpEntity->GetName()));

	pObject->msFileName = msFileName;
	pObject->m_mtxOnLoadTransform = a_mtxTransform;

	// Set the engine objects to the object
	pObject->SetBodies(mvBodies);
	pObject->SetBeams(mvBeams);
	pObject->SetMeshEntity(mpEntity);
	pObject->SetParticleSystems(mvParticleSystems);
	pObject->SetSoundEntities(mvSoundEntities);
	pObject->SetLights(mvLights);

	///////////////////////////////////
	// Load game properties
	TiXmlElement *pGameElem = apRootElem->FirstChildElement("GAME");
	if (pGameElem) {
		////////////////////////////////////////////
		// General
		pObject->mInteractMode = ToInteractMode(pGameElem->Attribute("InteractMode"));

		pObject->mfHealth = cString::ToFloat(pGameElem->Attribute("Health"), 0);
		pObject->mlToughness = cString::ToInt(pGameElem->Attribute("Toughness"), 0);

		pObject->mfForwardUpMul = cString::ToFloat(pGameElem->Attribute("ForwardUpMul"), 1);
		pObject->mfForwardRightMul = cString::ToFloat(pGameElem->Attribute("ForwardRightMul"), 1);

		pObject->mfUpMul = cString::ToFloat(pGameElem->Attribute("UpMul"), 1);
		pObject->mfRightMul = cString::ToFloat(pGameElem->Attribute("RightMul"), 1);

		pObject->mbPickAtPoint = cString::ToBool(pGameElem->Attribute("PickAtPoint"), false);
		pObject->mbRotateWithPlayer = cString::ToBool(pGameElem->Attribute("RotateWithPlayer"), true);
		pObject->mbUseNormalMass = cString::ToBool(pGameElem->Attribute("UseNormalMass"), false);
		pObject->mfGrabMassMul = cString::ToFloat(pGameElem->Attribute("GrabMassMul"), 1.0f);

		pObject->mbCanBeThrown = cString::ToBool(pGameElem->Attribute("CanBeThrown"), true);
		pObject->mbCanBePulled = cString::ToBool(pGameElem->Attribute("CanBePulled"), true);

		pObject->mbDestroyable = cString::ToBool(pGameElem->Attribute("Destroyable"), false);
		pObject->mfDestroyStrength = cString::ToFloat(pGameElem->Attribute("DestroyStrength"), 0.0f);
		pObject->msDestoySound = cString::ToString(pGameElem->Attribute("DestoySound"), "");

		pObject->mbPauseControllers = cString::ToBool(pGameElem->Attribute("PauseControllers"), true);
		pObject->mbPauseGravity = cString::ToBool(pGameElem->Attribute("PauseGravity"), true);

		pObject->mbForceLightOffset = cString::ToBool(pGameElem->Attribute("ForceLightOffset"), false);
		pObject->mvLightOffset = cString::ToVector3f(pGameElem->Attribute("LightOffset"), 0);

		pObject->mfHapticTorqueMul = cString::ToFloat(pGameElem->Attribute("HapticTorqueMul"), 1.0f);

		////////////////////////////////////////////
		// Disappear
		pObject->mDisappearProps.mbActive = cString::ToBool(pGameElem->Attribute("Disappear"), false);
		if (pObject->mDisappearProps.mbActive) {
			pObject->mDisappearProps.mfMinTime = cString::ToFloat(pGameElem->Attribute("DisappearMinTime"), 0);
			pObject->mDisappearProps.mfMaxTime = cString::ToFloat(pGameElem->Attribute("DisappearMaxTime"), 0);
			pObject->mDisappearProps.mfMinDistance = cString::ToFloat(pGameElem->Attribute("DisappearMinDist"), 0);

			pObject->mDisappearProps.mfMinCloseDistance = cString::ToFloat(pGameElem->Attribute("DisappearMinCloseDist"), 0);

			pObject->mDisappearProps.mfTime = cMath::RandRectf(pObject->mDisappearProps.mfMinTime,
															   pObject->mDisappearProps.mfMaxTime);
		}

		////////////////////////////////////////////
		// Breakable
		pObject->mBreakProps.mbActive = cString::ToBool(pGameElem->Attribute("Breakable"), false);
		if (pObject->mBreakProps.mbActive || pObject->mDisappearProps.mbActive) {
			pObject->mBreakProps.msSound = cString::ToString(pGameElem->Attribute("BreakSound"), "");
			pObject->mBreakProps.msEntity = cString::ToString(pGameElem->Attribute("BreakEntity"), "");
			pObject->mBreakProps.msPS = cString::ToString(pGameElem->Attribute("BreakPS"), "");

			pObject->mBreakProps.mfMinImpulse = cString::ToFloat(pGameElem->Attribute("BreakImpulse"), 99999);
			pObject->mBreakProps.mfMinNormalSpeed = cString::ToFloat(pGameElem->Attribute("BreakNormalSpeed"), 99999);
			pObject->mBreakProps.mfMinPlayerImpulse = cString::ToFloat(pGameElem->Attribute("BreakPlayerImpulse"), 99999);
			pObject->mBreakProps.mfCenterForce = cString::ToFloat(pGameElem->Attribute("BreakCenterForce"), 0);

			pObject->mBreakProps.mbExplosion = cString::ToBool(pGameElem->Attribute("BreakExplosion"), false);
			pObject->mBreakProps.mfExpl_Radius = cString::ToFloat(pGameElem->Attribute("BreakExpl_Radius"), 0);
			pObject->mBreakProps.mfExpl_MinDamage = cString::ToFloat(pGameElem->Attribute("BreakExpl_MinDamage"), 0);
			pObject->mBreakProps.mfExpl_MaxDamage = cString::ToFloat(pGameElem->Attribute("BreakExpl_MaxDamage"), 0);
			pObject->mBreakProps.mfExpl_MinForce = cString::ToFloat(pGameElem->Attribute("BreakExpl_MinForce"), 0);
			pObject->mBreakProps.mfExpl_MaxForce = cString::ToFloat(pGameElem->Attribute("BreakExpl_MaxForce"), 0);
			pObject->mBreakProps.mfExpl_MaxImpulse = cString::ToFloat(pGameElem->Attribute("BreakExpl_MaxImpulse"), 0);
			pObject->mBreakProps.mfExpl_MinMass = cString::ToFloat(pGameElem->Attribute("BreakExpl_MinMass"), 0);
			pObject->mBreakProps.mlExpl_Strength = cString::ToInt(pGameElem->Attribute("BreakExpl_Strength"), 0);

			pObject->mBreakProps.mbLightFlash = cString::ToBool(pGameElem->Attribute("LightFlash"), false);
			pObject->mBreakProps.mLight_Color = cString::ToColor(pGameElem->Attribute("LightFlash_Color"), cColor(0, 0));
			pObject->mBreakProps.mfLight_Radius = cString::ToFloat(pGameElem->Attribute("LightFlash_Radius"), 0);
			pObject->mBreakProps.mfLight_AddTime = cString::ToFloat(pGameElem->Attribute("LightFlash_AddTime"), 0);
			pObject->mBreakProps.mfLight_NegTime = cString::ToFloat(pGameElem->Attribute("LightFlash_NegTime"), 0);
			pObject->mBreakProps.mvLight_Offset = cString::ToVector3f(pGameElem->Attribute("LightFlash_Offset"), 0);

			pObject->mBreakProps.mbEarRing = cString::ToBool(pGameElem->Attribute("EarRing"), false);
			pObject->mBreakProps.mfEarRing_MaxDist = cString::ToFloat(pGameElem->Attribute("EarRing_MaxDist"), 0);
			pObject->mBreakProps.mfEarRing_Time = cString::ToFloat(pGameElem->Attribute("EarRing_Time"), 0);

			// Set all bodies as volatile
			for (size_t i = 0; i < mvBodies.size(); ++i) {
				mvBodies[i]->SetVolatile(true);
			}
		}
		////////////////////////////////////////////
		// Attracts Enemies
		pObject->mAttractProps.mbActive = cString::ToBool(pGameElem->Attribute("AttractEnemies"), false);
		if (pObject->mAttractProps.mbActive) {
			pObject->mAttractProps.mfDistance = cString::ToFloat(pGameElem->Attribute("AttractDistance"), 0);
			tString sSubTypes = cString::ToString(pGameElem->Attribute("AttractSubtypes"), "");
			cString::GetStringVec(sSubTypes, pObject->mAttractProps.mvSubtypes, NULL);

			pObject->mAttractProps.mbIsEaten = cString::ToBool(pGameElem->Attribute("AttractIsEaten"), false);
			pObject->mAttractProps.mfEatLength = cString::ToFloat(pGameElem->Attribute("AttractEatLength"), 0);
		}

		////////////////////////////////////////////
		// Mode specific

		// Push mode
		if (pObject->mInteractMode == eObjectInteractMode_Push) {
			pObject->mfMaxInteractDist = cString::ToFloat(pGameElem->Attribute("MaxInteractDist"), mpInit->mpPlayer->GetMaxPushDist());
			pObject->mbHasInteraction = true;
		}
		// Move Mode
		else if (pObject->mInteractMode == eObjectInteractMode_Move) {
			pObject->mfMaxInteractDist = cString::ToFloat(pGameElem->Attribute("MaxInteractDist"), mpInit->mpPlayer->GetMaxMoveDist());
			pObject->mbHasInteraction = true;
		}
		// Grab Mode
		else if (pObject->mInteractMode == eObjectInteractMode_Grab) {
			pObject->mfMaxInteractDist = cString::ToFloat(pGameElem->Attribute("MaxInteractDist"), mpInit->mpPlayer->GetMaxGrabDist());
			pObject->mbHasInteraction = true;
		}

		/////////////////////////////////////
		// Damage
		pObject->mDamageProps.mbActive = cString::ToBool(pGameElem->Attribute("DamageCharacter"), false);
		if (pObject->mDamageProps.mbActive) {
			pObject->mDamageProps.mfMinLinearDamageSpeed = cString::ToFloat(pGameElem->Attribute("MinLinearDamageSpeed"), 0);
			pObject->mDamageProps.mfMinAngularDamageSpeed = cString::ToFloat(pGameElem->Attribute("MinAngularDamageSpeed"), 0);
			pObject->mDamageProps.mfMaxLinearDamageSpeed = cString::ToFloat(pGameElem->Attribute("MaxLinearDamageSpeed"), 0);
			pObject->mDamageProps.mfMaxAngularDamageSpeed = cString::ToFloat(pGameElem->Attribute("MaxAngularDamageSpeed"), 0);
			pObject->mDamageProps.mfMinDamage = cString::ToFloat(pGameElem->Attribute("MinDamage"), 0);
			pObject->mDamageProps.mfMaxDamage = cString::ToFloat(pGameElem->Attribute("MaxDamage"), 0);
			pObject->mDamageProps.mlDamageStrength = cString::ToInt(pGameElem->Attribute("DamageStrength"), 0);
		}

		// Add callback for all bodies
		pObject->mpBodyCallback = hplNew(cGameObjectBodyCallback, (mpInit, pObject));
		for (size_t i = 0; i < mvBodies.size(); ++i)
			mvBodies[i]->AddBodyCallback(pObject->mpBodyCallback);
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

	/////////////////////////////////
	// Add to map handler
	mpInit->mpMapHandler->AddGameEntity(pObject);

	pObject->SetUpTransMaterials();
	pObject->SetupBreakObject();
	pObject->SetupForceOffset();

	/////////////////////////////////
	// Add to map handler
}

//-----------------------------------------------------------------------

eObjectInteractMode cEntityLoader_GameObject::ToInteractMode(const char *apString) {
	if (apString == NULL)
		return eObjectInteractMode_Static;

	tString sName = cString::ToLowerCase(apString);

	if (sName == "grab")
		return eObjectInteractMode_Grab;
	if (sName == "move")
		return eObjectInteractMode_Move;
	if (sName == "push")
		return eObjectInteractMode_Push;
	if (sName == "static")
		return eObjectInteractMode_Static;

	Warning("Invalid object interact mode '%s'\n", apString);

	return eObjectInteractMode_Static;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameObject::cGameObject(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_Object;
	mInteractMode = eObjectInteractMode_Static;

	mpBodyCallback = NULL;

	mpCurrentAttraction = NULL;
	mfAttractCount = 4.0f;

	mbIsMover = false;

	mfCloseToSameCount = cMath::RandRectf(0, 5.5f);
}

//-----------------------------------------------------------------------

cGameObject::~cGameObject(void) {
	if (mpBodyCallback)
		hplDelete(mpBodyCallback);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameObject::OnPlayerPick() {
	if (mvCallbackScripts[eGameEntityScriptType_PlayerInteract] &&
		mpInit->mpPlayer->GetPickedDist() < mfMaxInteractDist &&
		mpInit->mpPlayer->mbProxyTouching) {
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Active);
	} else if (mInteractMode == eObjectInteractMode_Static && msDescription == _W("")) {
		if (mpInit->mpPlayer->GetState() == ePlayerState_InteractMode)
			mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Inactive);
		else
			mpInit->mpPlayer->SetCrossHairState(eCrossHairState_None);
	} else {
		// mpInit->mpPlayer->SetCrossHairState(eCrossHairState_None);
	}
}

//-----------------------------------------------------------------------

void cGameObject::OnPlayerInteract() {
	iPhysicsBody *pBody = mpInit->mpPlayer->GetPickedBody();

	cGameStickArea *pStickArea = mpInit->mpMapHandler->GetBodyStickArea(pBody);
	if ((pBody->GetMass() == 0 && pStickArea == NULL) ||
		(pStickArea && pStickArea->GetCanDeatch() == false)) {
		return;
	}

	switch (mInteractMode) {
	case eObjectInteractMode_Push:
		PushObject();
		break;
	case eObjectInteractMode_Move:
		MoveObject();
		break;
	case eObjectInteractMode_Grab:
		GrabObject();
		break;
	case eObjectInteractMode_Static:
		break;
	case eObjectInteractMode_LastEnum:
		break;
	}
}

//-----------------------------------------------------------------------

void cGameObject::Update(float afTimeStep) {
	UpdateAttraction(afTimeStep);

	////////////////////////////////
	// Disappear
	if (mDisappearProps.mbActive) {
		// TODO: Check per body!
		float fMinDist = cMath::Vector3Dist(mpInit->mpPlayer->GetCamera()->GetPosition(),
											mvBodies[0]->GetWorldPosition());

		for (size_t i = 1; i < mvBodies.size(); ++i) {
			if (mvBodies[i] == NULL)
				continue;

			float fDist = cMath::Vector3Dist(mpInit->mpPlayer->GetCamera()->GetPosition(),
											 mvBodies[i]->GetWorldPosition());
			if (fDist < fMinDist)
				fMinDist = fDist;
		}

		if (fMinDist >= mDisappearProps.mfMinDistance) {
			mDisappearProps.mfTime -= afTimeStep;
			if (mDisappearProps.mfTime <= 0) {
				Break();
			}
		}

		if (mDisappearProps.mfMinCloseDistance > 0) {
			cVector3f vPos = mpMeshEntity->GetBoundingVolume()->GetWorldCenter();
			float fMinSqrDist = mDisappearProps.mfMinCloseDistance *
								mDisappearProps.mfMinCloseDistance;
			if (mfCloseToSameCount <= 0) {
				mfCloseToSameCount = 5.5f;

				tGameEntityIterator it = mpInit->mpMapHandler->GetGameEntityIterator();
				while (it.HasNext()) {
					iGameEntity *pEntity = it.Next();
					if (pEntity == this)
						continue;
					if (pEntity->GetType() != eGameEntityType_Object)
						continue;
					if (pEntity->GetFileName() != GetFileName())
						continue;

					cGameObject *pObject = static_cast<cGameObject *>(pEntity);

					float fSqrDist = cMath::Vector3DistSqr(
						pObject->GetMeshEntity()->GetBoundingVolume()->GetWorldCenter(),
						vPos);
					if (fSqrDist < fMinSqrDist) {
						mDisappearProps.mfTime = 0;
						Break();
					}
				}
			} else {
				mfCloseToSameCount -= afTimeStep;
			}
		}
	}

	///////////////////////////////
	// Force offset
	if (mbForceLightOffset) {
		for (size_t i = 0; i < mvLights.size(); ++i) {
			iLight3D *pLight = mvLights[i];

			pLight->SetMatrix(cMath::MatrixMul(mpMeshEntity->GetWorldMatrix(),
											   mvLightLocalOffsets[i]));

			pLight->SetPosition(pLight->GetWorldPosition() + mvLightOffset);
		}
	}
}

//-----------------------------------------------------------------------

void cGameObject::OnPlayerGravityCollide(iCharacterBody *apCharBody, cCollideData *apCollideData) {
	if (mBreakProps.mbActive) {
		///////////////////////////////////////////////
		// Check so that the player is really on top of the
		// object and not just sliding
		bool bPushDown = false;
		for (int i = 0; i < apCollideData->mlNumOfPoints; i++) {
			cCollidePoint &point = apCollideData->mvContactPoints[i];
			if (point.mvNormal.y > 0.001f &&
				point.mvNormal.y > ABS(point.mvNormal.x) &&
				point.mvNormal.y > ABS(point.mvNormal.z)) {
				bPushDown = true;
			}
		}

		if (bPushDown) {
			////////////////////////////////////
			// Check the impulse created by the player
			float fImpulse = apCharBody->GetMass() * ABS(apCharBody->GetForceVelocity().y);

			if (mBreakProps.mfMinPlayerImpulse <= fImpulse) {
				if (mpInit->mbDebugInteraction) {
					Log("------ Breakage ----------\n");
					Log(" Body '%s' by Player\n", mvBodies[0]->GetName().c_str());
					Log(" Impulse: %f : %fm/s * %fkg (from Player)\n", fImpulse,
						ABS(apCharBody->GetForceVelocity().y), apCharBody->GetMass());
					Log("-------------------------\n");
				}

				Break();
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameObject::SetInteractMode(eObjectInteractMode aInteractMode) {
	mInteractMode = aInteractMode;

	if (mInteractMode == eObjectInteractMode_Static) {
		mbHasInteraction = false;
	} else {
		mbHasInteraction = true;
	}
}

//-----------------------------------------------------------------------

void cGameObject::BreakAction() {
	if (mvBodies.empty())
		return;

	iPhysicsBody *pParentBody = mvBodies[0];

	for (size_t i = 1; i < mvBodies.size(); ++i) {
		if (pParentBody->GetMass() != 0) {
			if (mvBodies[i]->GetMass() > pParentBody->GetMass() || mvBodies[i]->GetMass() == 0)
				pParentBody = mvBodies[i];
		}
	}

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
		mpInit->mpPlayer->GetPushBody() == pParentBody) {
		mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
	}

	//////////////////
	// Sound
	if (mBreakProps.msSound != "") {
		cSoundEntity *pSound = pWorld->CreateSoundEntity("Break", mBreakProps.msSound, true);
		if (pSound)
			pSound->SetPosition(mpMeshEntity->GetWorldPosition());
	}

	//////////////////
	// Particle System
	if (mBreakProps.msPS != "") {
		/*cParticleSystem3D *pPS = */ pWorld->CreateParticleSystem("Break", mBreakProps.msPS, cVector3f(1, 1, 1),
																   pParentBody->GetWorldMatrix());
	}

	//////////////////
	// Entity
	if (mBreakProps.msEntity != "") {
		iEntity3D *pEntity = pWorld->CreateEntity(mpMeshEntity->GetName() + "_broken",
												  pParentBody->GetWorldMatrix(),
												  mBreakProps.msEntity, true);
		if (pEntity) {
			iGameEntity *pGameEntity = mpInit->mpMapHandler->GetLatestEntity();

			for (int i = 0; i < pGameEntity->GetBodyNum(); ++i) {
				// Add the object velocity
				iPhysicsBody *pBody = pGameEntity->GetBody(i);
				pBody->SetLinearVelocity(pParentBody->GetLinearVelocity());
				// TODO: Add torque

				// Force from center
				cVector3f vBodyCentre = cMath::MatrixMul(pBody->GetWorldMatrix(), pBody->GetMassCentre());

				cVector3f vForceDir = vBodyCentre - pParentBody->GetLocalPosition();
				vForceDir.Normalise();

				pBody->AddForce(vForceDir * mBreakProps.mfCenterForce);
			}
		}
	}

	//////////////////
	// Explosion
	if (mBreakProps.mbExplosion) {
		mpInit->mpAttackHandler->CreateSplashDamage(
			pParentBody->GetWorldPosition(),
			mBreakProps.mfExpl_Radius,
			mBreakProps.mfExpl_MinDamage, mBreakProps.mfExpl_MaxDamage,
			mBreakProps.mfExpl_MinForce, mBreakProps.mfExpl_MaxForce,
			mBreakProps.mfExpl_MaxImpulse,
			eAttackTargetFlag_Bodies | eAttackTargetFlag_Player | eAttackTargetFlag_Enemy,
			mBreakProps.mfExpl_MinMass,
			mBreakProps.mlExpl_Strength);
	}

	//////////////////
	// Light flash
	if (mBreakProps.mbLightFlash) {
		mpInit->mpMapHandler->AddLightFlash(pParentBody->GetWorldPosition() + mBreakProps.mvLight_Offset,
											mBreakProps.mfLight_Radius,
											mBreakProps.mLight_Color,
											mBreakProps.mfLight_AddTime,
											mBreakProps.mfLight_NegTime);
	}

	//////////////////
	// Ear ring
	if (mBreakProps.mbEarRing) {
		cPlayer *pPlayer = mpInit->mpPlayer;
		float fDist = cMath::Vector3Dist(pParentBody->GetWorldPosition(),
										 pPlayer->GetCamera()->GetPosition());

		// Log("Ear ring dist: %f max: %f\n",fDist, mBreakProps.mfEarRing_MaxDist);

		if (fDist <= mBreakProps.mfEarRing_MaxDist) {
			pPlayer->GetEarRing()->Start(mBreakProps.mfEarRing_Time);
		}
	}

	mpInit->mpGame->ResetLogicTimer();
}

//-----------------------------------------------------------------------

void cGameObject::OnDeath(float afDamage) {
	if (mBreakProps.mbActive) {
		Break();
	}
}

//-----------------------------------------------------------------------

void cGameObject::SetupBreakObject() {
	if (mBreakProps.mbActive == false)
		return;

	if (mBreakProps.msEntity != "")
		PreloadModel(mBreakProps.msEntity);
	if (mBreakProps.msPS != "") {
		cParticleSystem3D *pPS = mpInit->mpGame->GetResources()->GetParticleManager()->CreatePS3D(
			"", mBreakProps.msPS, 1, cMatrixf::Identity);
		hplDelete(pPS);
	}
	if (mBreakProps.msSound != "") {
		mpInit->PreloadSoundEntityData(mBreakProps.msSound);
	}
}

//-----------------------------------------------------------------------

void cGameObject::SetupForceOffset() {
	if (mbForceLightOffset == false)
		return;

	mvLightLocalOffsets.resize(mvLights.size());
	for (size_t i = 0; i < mvLights.size(); ++i) {
		mvLightLocalOffsets[i] = mvLights[i]->GetLocalMatrix();

		if (mvLights[i]->GetParent())
			mvLights[i]->GetParent()->RemoveEntity(mvLights[i]);
		if (mvLights[i]->GetEntityParent())
			mvLights[i]->GetEntityParent()->RemoveChild(mvLights[i]);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameObject::GrabObject() {
	if (mpInit->mpPlayer->GetPickedDist() > mfMaxInteractDist) {
		if (mpInit->mpPlayer->GetState() == ePlayerState_InteractMode)
			mpInit->mpEffectHandler->GetSubTitle()->Add(kTranslate("Player", "ObjectTooFar"), 2.0f, true);

		return;
	}

	// Set some properties
	mpInit->mpPlayer->mbGrabbingMoveBody = mbIsMover;
	mpInit->mpPlayer->mfHapticTorqueMul = mfHapticTorqueMul;

	mpInit->mpPlayer->mbPickAtPoint = mbPickAtPoint;
	mpInit->mpPlayer->mbRotateWithPlayer = mbRotateWithPlayer;
	mpInit->mpPlayer->mbUseNormalMass = mbUseNormalMass;
	mpInit->mpPlayer->mfGrabMassMul = mfGrabMassMul;

	mpInit->mpPlayer->mbCanBeThrown = mbCanBeThrown;

	mpInit->mpPlayer->mfCurrentMaxInteractDist = mfMaxInteractDist;

	mpInit->mpPlayer->SetPushBody(mpInit->mpPlayer->GetPickedBody());
	mpInit->mpPlayer->ChangeState(ePlayerState_Grab);
}

//-----------------------------------------------------------------------

void cGameObject::MoveObject() {
	float fDist = GetMoveDist();

	if (fDist > mfMaxInteractDist) {
		if (mpInit->mpPlayer->GetState() == ePlayerState_InteractMode)
			mpInit->mpEffectHandler->GetSubTitle()->Add(kTranslate("Player", "ObjectTooFar"), 2.0f, true);

		return;
	}

	// Set some properties
	mpInit->mpPlayer->mfForwardUpMul = mfForwardUpMul;
	mpInit->mpPlayer->mfForwardRightMul = mfForwardRightMul;

	mpInit->mpPlayer->mfUpMul = mfUpMul;
	mpInit->mpPlayer->mfRightMul = mfRightMul;

	mpInit->mpPlayer->mbCanBeThrown = mbCanBeThrown;

	mpInit->mpPlayer->mfCurrentMaxInteractDist = mfMaxInteractDist;

	mpInit->mpPlayer->SetPushBody(mpInit->mpPlayer->GetPickedBody());
	mpInit->mpPlayer->ChangeState(ePlayerState_Move);
}

float cGameObject::GetMoveDist() {
	/*cVector3f vBodyPos = mpInit->mpPlayer->GetPickedPos();
	cVector3f vPlayerPos = mpInit->mpPlayer->GetCamera()->GetPosition();

	float fDist = cMath::Vector3Dist(vBodyPos, vPlayerPos);

	return fDist;*/
	return mpInit->mpPlayer->GetPickedDist();
}

//-----------------------------------------------------------------------

void cGameObject::PushObject() {
	float fDist = GetPushDist();

	if (fDist > mfMaxInteractDist) {
		if (mpInit->mpPlayer->GetState() == ePlayerState_InteractMode)
			mpInit->mpEffectHandler->GetSubTitle()->Add(kTranslate("Player", "ObjectTooFar"), 2.0f, true);

		return;
	}

	mpInit->mpPlayer->mbPickAtPoint = mbPickAtPoint;

	mpInit->mpPlayer->mbCanBeThrown = mbCanBeThrown;
	mpInit->mpPlayer->mbCanBePulled = mbCanBePulled;

	mpInit->mpPlayer->mfCurrentMaxInteractDist = mfMaxInteractDist;

	mpInit->mpPlayer->SetPushBody(mpInit->mpPlayer->GetPickedBody());
	mpInit->mpPlayer->ChangeState(ePlayerState_Push);
}

float cGameObject::GetPushDist() {
	/*cVector3f vBodyPos = mpInit->mpPlayer->GetPickedPos();
	cVector3f vPlayerPos = mpInit->mpPlayer->GetCamera()->GetPosition();

	float fDist = cMath::Dist2D(cVector2f(vBodyPos.x,vBodyPos.z),cVector2f(vPlayerPos.x,vPlayerPos.z));
	*/
	return mpInit->mpPlayer->GetPickedDist();
}

//-----------------------------------------------------------------------

void cGameObject::UpdateAttraction(float afTimeStep) {
	if (mAttractProps.mbActive == false)
		return;

	///////////////////////////////////////////////
	// Check if the current attraction is near enough
	if (mpCurrentAttraction) {
		////////////////////////////////////////
		// Check for the attracted enemy
		iCharacterBody *pBody = mpCurrentAttraction->GetMover()->GetCharBody();
		float fDist = cMath::Vector3Dist(pBody->GetFeetPosition(),
										 mvBodies[0]->GetWorldPosition());

		// Check if the enemy have become busy.
		if ((mpCurrentAttraction->GetCurrentStateId() != STATE_MOVETO && fDist < pBody->GetSize().x * 0.75f) ||
			(mAttractProps.mbIsEaten && fDist < pBody->GetSize().x * 0.75f)) {
			// Use half the width here.
			if (mAttractProps.mbIsEaten) {
				mpCurrentAttraction->SetTempFloat(mAttractProps.mfEatLength);
				mpCurrentAttraction->ChangeState(STATE_EAT);
			}

			mpCurrentAttraction->SetAttracted(false);
			mpCurrentAttraction = NULL;

			mbDestroyMe = true;
			return;
		} else if (mpCurrentAttraction->GetCurrentStateId() != STATE_MOVETO) {
			mpCurrentAttraction->SetAttracted(false);
			mpCurrentAttraction = NULL;
			return;
		}
	}
	///////////////////////////////////////////////
	// Check if any enemy is near enough to be attracted
	else {
		if (mfAttractCount <= 0) {
			float fClosestDist = 10000.0f;
			iGameEnemy *pChosenEnemy = NULL;

			tGameEnemyIterator enemyIt = mpInit->mpMapHandler->GetGameEnemyIterator();
			while (enemyIt.HasNext()) {
				iGameEnemy *pEnemy = enemyIt.Next();

				if (pEnemy->GetHealth() <= 0 || pEnemy->IsActive() == false ||
					pEnemy->IsAttracted()) {
					continue;
				}

				bool bCorrectSub = false;
				for (size_t i = 0; i < mAttractProps.mvSubtypes.size(); ++i) {
					if (mAttractProps.mvSubtypes[i] == pEnemy->GetSubType())
						bCorrectSub = true;
				}

				if (bCorrectSub == false)
					continue;

				// Check if the enemy has already been attracted by this object
				if (m_setAttractedEnemies.find(pEnemy) != m_setAttractedEnemies.end())
					continue;

				float fDist = cMath::Vector3Dist(pEnemy->GetMover()->GetCharBody()->GetFeetPosition(),
												 mvBodies[0]->GetWorldPosition());

				if (fDist < mAttractProps.mfDistance && fDist < fClosestDist) {
					fClosestDist = fDist;
					pChosenEnemy = pEnemy;
				}
			}

			if (pChosenEnemy) {
				if (pChosenEnemy->MoveToPos(mvBodies[0]->GetWorldPosition() + cVector3f(0, 0.2f, 0))) {
					mpCurrentAttraction = pChosenEnemy;
					pChosenEnemy->SetAttracted(true);
					m_setAttractedEnemies.insert(pChosenEnemy);
				}
			}

			mfAttractCount = 0.2f;
		} else {
			mfAttractCount -= afTimeStep;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cGameObject_SaveData, iGameEntity_SaveData)
	kSerializeVar(mInteractMode, eSerializeType_Int32)
		kSerializeVar(mfMaxInteractDist, eSerializeType_Float32)
			kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameObject_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameObject::CreateSaveData() {
	return hplNew(cGameObject_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameObject::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameObject_SaveData *pData = static_cast<cGameObject_SaveData *>(apSaveData);

	kCopyToVar(pData, mInteractMode);
}

//-----------------------------------------------------------------------

void cGameObject::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameObject_SaveData *pData = static_cast<cGameObject_SaveData *>(apSaveData);

	kCopyFromVar(pData, mInteractMode);
}
//-----------------------------------------------------------------------
