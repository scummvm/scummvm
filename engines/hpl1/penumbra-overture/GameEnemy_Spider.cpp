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

#include "hpl1/penumbra-overture/GameEnemy_Spider.h"

#include "hpl1/penumbra-overture/AttackHandler.h"
#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GameSwingDoor.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

//////////////////////////////////////////////////////////////////////////
// BASE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGameEnemyState_Spider_Base::iGameEnemyState_Spider_Base(int alId, cInit *apInit, iGameEnemy *apEnemy)
	: iGameEnemyState(alId, apInit, apEnemy) {
	mpEnemySpider = static_cast<cGameEnemy_Spider *>(mpEnemy);
}

//-----------------------------------------------------------------------

void iGameEnemyState_Spider_Base::OnSeePlayer(const cVector3f &avPosition, float afChance) {
	if (mpPlayer->GetHealth() <= 0)
		return;

	if (afChance >= mpEnemySpider->mfIdleMinSeeChance) {
		mpEnemy->ChangeState(STATE_HUNT);
		mpEnemySpider->PlaySound(mpEnemySpider->msIdleFoundPlayerSound);
	}
}

bool iGameEnemyState_Spider_Base::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	float afDistance = (mpMover->GetCharBody()->GetPosition() - avPosition).Length();

	if (afVolume >= mpEnemySpider->mfIdleMinHearVolume && afDistance > 0.4f) {
		// mpEnemy->SetTempPosition(avPosition);
		// mpEnemy->ChangeState(STATE_INVESTIGATE);
		return true;
	}

	return false;
}

void iGameEnemyState_Spider_Base::OnTakeHit(float afDamage) {
	if (afDamage >= 5) {
		mpEnemy->ChangeState(STATE_KNOCKDOWN);
	}
}

void iGameEnemyState_Spider_Base::OnFlashlight(const cVector3f &avPosition) {
	// mpInit->mpEffectHandler->GetSubTitle()->Add("Flashlight!",0.5f);
	// OnSeePlayer(mpPlayer->GetCharacterBody()->GetFeetPosition(),1.0f);
	if (mlId == STATE_KNOCKDOWN)
		return;

	if (mpEnemySpider->mbFleeFromFlashlight)
		mpEnemy->ChangeState(STATE_FLEE);
}

void iGameEnemyState_Spider_Base::OnDeath(float afDamage) {
	mpEnemy->ChangeState(STATE_KNOCKDOWN);
	// mpEnemy->ChangeState(STATE_DEAD);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// IDLE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Idle::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Idle", false, 0.2f);

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemySpider->mfIdleFOV);

	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);

	mbStopped = true;

	mfNextWalkTime = cMath::RandRectf(mpEnemySpider->mfIdleMinWaitLength,
									  mpEnemySpider->mfIdleMaxWaitLength);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Idle::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Idle::OnUpdate(float afTimeStep) {
	if (mpMover->IsMoving() == false || mpMover->GetStuckCounter() > 2.0f) {
		mpMover->ResetStuckCounter();

		if (mbStopped == false) {
			mbStopped = true;

			mpEnemy->PlayAnim("Idle", false, 0.9f);

			mfNextWalkTime = cMath::RandRectf(mpEnemySpider->mfIdleMinWaitLength,
											  mpEnemySpider->mfIdleMaxWaitLength);
		} else if (mfNextWalkTime <= 0) {
			mbStopped = false;

			// Animation
			mpEnemy->UseMoveStateAnimations();

			// Setup body
			mpEnemy->SetupBody();
			mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemySpider->mfHuntSpeed);

			cAINode *pNode = NULL;

			if (mpEnemy->GetPatrolNodeNum() == 0) {
				pNode = mpMover->GetAINodeInRange(1, 5);
			} else {
				int lNodeNum = cMath::RandRectl(0, mpEnemy->GetPatrolNodeNum() - 1);
				tString sName = mpEnemy->GetPatrolNode(lNodeNum)->msNodeName;
				pNode = mpMover->GetNodeContainer()->GetNodeFromName(sName);
			}

			if (pNode) {
				mpMover->MoveToPos(pNode->GetPosition());
			} else {
				mpEnemy->ChangeState(STATE_IDLE);
			}
		} else {
			mfNextWalkTime -= afTimeStep;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// HUNT STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Hunt::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	float fMul = 1.0f;

	mpEnemy->SetupBody();
	if (mpInit->mDifficulty == eGameDifficulty_Easy)
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemySpider->mfHuntSpeed * 0.7f * fMul);
	else if (mpInit->mDifficulty == eGameDifficulty_Normal)
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemySpider->mfHuntSpeed * fMul);
	else
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemySpider->mfHuntSpeed * 1.2f * fMul);

	// Setup enemy
	mpEnemy->SetFOV(mpEnemySpider->mfHuntFOV);

	mfUpdatePathCount = 0;
	mfUpdateFreq = 1.0f;
	mbFreePlayerPath = false;

	mbLostPlayer = false;
	mfLostPlayerCount = 0;
	mfMaxLostPlayerCount = mpEnemySpider->mfHuntForLostPlayerTime;

	mpInit->mpMusicHandler->AddAttacker(mpEnemy);

	if (mpEnemySpider->mbPathFind == false)
		mpMover->Stop();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Hunt::OnLeaveState(iGameEnemyState *apNextState) {
	mpMover->SetMaxDoorToughness(-1);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Hunt::OnUpdate(float afTimeStep) {
	if (mpPlayer->GetHealth() <= 0) {
		mpEnemy->ChangeState(STATE_IDLE);
		return;
	}

	if (mpMover->GetStuckCounter() > 2.1f) {
		mpEnemy->ChangeState(STATE_FLEE);
		mpMover->ResetStuckCounter();
		return;
	}

	// Check if he should attack.
	if (mpMover->DistanceToChar2D(mpPlayer->GetCharacterBody()) < mpEnemySpider->mfAttackDistance) {
		mpEnemy->ChangeState(STATE_ATTACK);
	}

	if (mpEnemySpider->mbPathFind) {
		if (mfUpdatePathCount <= 0) {
			mfUpdatePathCount = mfUpdateFreq;

			/*cAINodeContainer *pNodeCont = */ mpEnemy->GetMover()->GetNodeContainer();

			// Check if there is a free path to the player
			if (mbLostPlayer == false && mpMover->FreeDirectPathToChar(mpPlayer->GetCharacterBody())) {
				mbFreePlayerPath = true;
				mpMover->Stop();
			} else {
				mbFreePlayerPath = false;
			}

			// Get path to player
			if (mbFreePlayerPath == false && mbLostPlayer == false) {
				if (mpMover->MoveToPos(mpEnemy->GetLastPlayerPos()) == false) {
					mfUpdatePathCount *= 5.0f;
				}
			}
		} else {
			mfUpdatePathCount -= afTimeStep;
		}
	} else {
		mbFreePlayerPath = true;

		if (mpEnemy->CanSeePlayer() == false) {
			mfLostPlayerCount -= afTimeStep;

			if (mfLostPlayerCount <= 0 || mpMover->GetStuckCounter() > 0.5f) {
				mpEnemy->ChangeState(STATE_IDLE);
			}
		} else {
			mfLostPlayerCount = mfMaxLostPlayerCount * 2;
		}
	}

	////////////////////////////////
	// Go directly towards the player
	if (mbFreePlayerPath) {
		// Go towards player
		mpMover->MoveDirectToPos(mpPlayer->GetCharacterBody()->GetFeetPosition(), afTimeStep);
	}
	////////////////////////////////
	// Update path search
	else {
		if (mbLostPlayer == false && mpMover->IsMoving() == false) {
			mbLostPlayer = true;
			mfLostPlayerCount = mfMaxLostPlayerCount;
		}

		if (mbLostPlayer) {
			mpMover->GetCharBody()->Move(eCharDir_Forward, 1.0f, afTimeStep);

			mfLostPlayerCount -= afTimeStep;
			if (mfLostPlayerCount <= 0 || mpMover->GetStuckCounter() > 0.5f) {
				mpEnemy->ChangeState(STATE_IDLE);
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Hunt::OnSeePlayer(const cVector3f &avPosition, float afChance) {
	if (mbLostPlayer && afChance >= mpEnemySpider->mfHuntMinSeeChance) {
		mbLostPlayer = false;
		mfUpdatePathCount = 0;
	}
}

//-----------------------------------------------------------------------

bool cGameEnemyState_Spider_Hunt::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	//////////////////////////////////
	// If player is lost the sound might be of help
	if (mbLostPlayer) {
		// Check if sound can be heard
		if (afVolume >= mpEnemySpider->mfHuntMinHearVolume) {
			// Check if a node is found near the sound.
			cAINode *pNode = mpMover->GetAINodeAtPosInRange(avPosition, 0.0f, 5.0f, true, 0.1f);
			if (pNode) {
				// Update last player postion.
				mbLostPlayer = false;
				mfUpdatePathCount = 0;
				mpEnemy->SetLastPlayerPos(pNode->GetPosition());

				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Hunt::OnDraw() {
	// mpInit->mpDefaultFont->Draw(cVector3f(230,10,100),14,cColor(1,1,1,1),eFontAlign_Left,
	//							"Freepath: %d",mbFreePlayerPath);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// ATTACK STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Attack::OnEnterState(iGameEnemyState *apPrevState) {
	// Setup body
	mpEnemy->SetupBody();
	mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemySpider->mfHuntSpeed);

	// Animation
	mpEnemy->PlayAnim("Attack", false, 0.2f);

	mpEnemySpider->PlaySound(mpEnemySpider->msAttackStartSound);

	mfJumpTimer = mpEnemySpider->mfAttackJumpTime;

	mfDamageTimer = mpEnemySpider->mfAttackDamageTime;
	mbAttacked = false;
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Attack::OnLeaveState(iGameEnemyState *apNextState) {
	// mpMover->GetCharBody()->SetForce(0);
	mpEnemySpider->SetSkipSoundTriggerCount(2.0f);
	mpMover->ResetStuckCounter();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Attack::OnUpdate(float afTimeStep) {
	if (mfJumpTimer > 0) {
		mfJumpTimer -= afTimeStep;

		if (mfJumpTimer <= 0) {
			cVector3f vDirection = mpInit->mpPlayer->GetCamera()->GetPosition() -
								   mpMover->GetCharBody()->GetPosition();
			float fHeight = cMath::Abs(vDirection.y);
			vDirection.Normalise();

			cVector3f vForce = vDirection * mpEnemySpider->mfAttackForce;
			vForce.y = fHeight * 0.5f * mpEnemySpider->mfAttackForce;
			mpMover->GetCharBody()->AddForce(vForce);
		}
	}

	cVector3f vStart = mpPlayer->GetCharacterBody()->GetPosition();
	vStart.y = 0;
	cVector3f vEnd = mpMover->GetCharBody()->GetPosition();
	vEnd.y = 0;
	float fDist2D = cMath::Vector3DistSqr(vStart, vEnd);
	float fMinRange = mpEnemySpider->mfAttackDamageRange + 0.4f;

	if (fDist2D <= fMinRange * fMinRange && mfDamageTimer <= 0) {
		if (mbAttacked == false) {
			cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
							 mpMover->GetCharBody()->GetForward() *
								 mpEnemySpider->mfAttackDamageRange;

			cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
			cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
			mtxOffset.SetTranslation(vPos);

			eAttackTargetFlag target = eAttackTargetFlag_Player | eAttackTargetFlag_Bodies;

			mpInit->mpPlayer->mbDamageFromPos = true;
			mpInit->mpPlayer->mvDamagePos = mpMover->GetCharBody()->GetPosition();
			if (mpInit->mpAttackHandler->CreateShapeAttack(mpEnemySpider->GetAttackShape(),
														   mtxOffset,
														   mpMover->GetCharBody()->GetPosition(),
														   cMath::RandRectf(mpEnemySpider->mfAttackMinDamage,
																			mpEnemySpider->mfAttackMaxDamage),

														   mpEnemySpider->mfAttackMinMass, mpEnemySpider->mfAttackMaxMass,
														   mpEnemySpider->mfAttackMinImpulse, mpEnemySpider->mfAttackMaxImpulse,

														   mpEnemySpider->mlAttackStrength,

														   target, NULL)) {
				mpEnemySpider->PlaySound(mpEnemySpider->msAttackHitSound);
			}
			mpInit->mpPlayer->mbDamageFromPos = false;
			mbAttacked = true;
		}
	} else if (mfDamageTimer > 0) {
		mfDamageTimer -= afTimeStep;
	}
}

void cGameEnemyState_Spider_Attack::OnFlashlight(const cVector3f &avPosition) {
	if (mfJumpTimer > 0 && mpEnemySpider->mbFleeFromFlashlight)
		mpEnemy->ChangeState(STATE_FLEE);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Attack::OnAnimationOver(const tString &asName) {
	mpEnemy->ChangeState(STATE_HUNT);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Attack::OnPostSceneDraw() {
	cCamera3D *pCamera = static_cast<cCamera3D *>(mpInit->mpGame->GetScene()->GetCamera());

	cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
					 mpMover->GetCharBody()->GetForward() *
						 mpEnemySpider->mfAttackDamageRange;

	cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
	cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
	mtxOffset.SetTranslation(vPos);

	cMatrixf mtxCollider = cMath::MatrixMul(pCamera->GetViewMatrix(), mtxOffset);

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetMatrix(eMatrix_ModelView, mtxCollider);

	cVector3f vSize = mpEnemySpider->GetAttackShape()->GetSize();
	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(vSize * 0.5f, vSize * -0.5f,
																cColor(1, 0, 1, 1));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FLEE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Flee::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	mpEnemy->SetupBody();
	mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemySpider->mfHuntSpeed);

	cVector3f vDir = mpMover->GetCharBody()->GetPosition() - mpPlayer->GetCharacterBody()->GetPosition();
	vDir.Normalise();

	cVector3f vStart = mpMover->GetCharBody()->GetPosition() + vDir * mpEnemySpider->mfFleeMaxDistance;

	cAINode *pNode = mpMover->GetAINodeAtPosInRange(vStart,
													mpEnemySpider->mfFleeMinDistance,
													mpEnemySpider->mfFleeMaxDistance,
													false, 0);
	if (pNode) {
		mpMover->MoveToPos(pNode->GetPosition());
	} else {
		mpEnemy->ChangeState(STATE_HUNT);
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Flee::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Flee::OnUpdate(float afTimeStep) {
	// Move forward
	if (mpMover->IsMoving() == false || mpMover->GetStuckCounter() > 2.3f) {
		mpMover->ResetStuckCounter();

		if (mlPreviousState != STATE_FLEE && mlPreviousState != STATE_ATTACK)
			mpEnemy->ChangeState(mlPreviousState);
		else
			mpEnemy->ChangeState(STATE_HUNT);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// KNOCK DOWN STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_KnockDown::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Idle", true, 0.7f);

	// Sound
	mpEnemy->PlaySound(mpEnemySpider->msKnockDownSound);

	// Setup body
	mpEnemy->SetupBody();

	// Go to rag doll
	mpEnemy->GetMeshEntity()->AlignBodiesToSkeleton(false);
	mpEnemy->GetMeshEntity()->SetSkeletonPhysicsActive(true);
	mpEnemy->GetMeshEntity()->Stop();

	mpEnemy->GetMover()->GetCharBody()->SetEntity(NULL);
	mpEnemy->GetMover()->GetCharBody()->SetActive(false);

	mpEnemy->GetMover()->Stop();

	mfTimer = 2.0f;
	mbCheckAnim = false;
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_KnockDown::OnLeaveState(iGameEnemyState *apNextState) {
	mpEnemy->GetMover()->GetCharBody()->SetEntity(mpEnemy->GetMeshEntity());
	mpEnemy->GetMover()->GetCharBody()->SetActive(true);
	mpMover->ResetStuckCounter();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_KnockDown::OnUpdate(float afTimeStep) {
	if (mbCheckAnim) {
	} else {
		mfTimer -= afTimeStep;

		if (mfTimer <= 0) {
			// Get the forward vector from root bone (the right vector)
			cNodeIterator StateIt = mpEnemy->GetMeshEntity()->GetRootNode()->GetChildIterator();
			// cBoneState *pBoneState = static_cast<cBoneState *>(StateIt.Next());

			// Play animation and fade physics
			float fFadeTime = 0.7f;
			mbCheckAnim = true;

			mpEnemy->GetMeshEntity()->Stop();
			if (mpEnemy->GetHealth() > 0)
				mpEnemy->PlayAnim("Walk", false, fFadeTime);
			else
				mpEnemy->PlayAnim("Death", false, fFadeTime);

			mpEnemy->GetMeshEntity()->FadeSkeletonPhysicsWeight(fFadeTime);

			// Calculate values
			cVector3f vPosition;
			cVector3f vAngles;
			/*cMatrixf mtxTransform = */ mpEnemy->GetMeshEntity()->CalculateTransformFromSkeleton(&vPosition, &vAngles);

			cVector3f vGroundPos = vPosition;
			/*bool bFoundGround = */ mpEnemy->GetGroundFinder()->GetGround(vPosition, cVector3f(0, -1, 0), &vGroundPos, NULL);

			// Set body
			iCharacterBody *pCharBody = mpEnemy->GetMover()->GetCharBody();

			vGroundPos.z -= pCharBody->GetEntityOffset().GetTranslation().z;
			pCharBody->SetFeetPosition(vGroundPos);
			pCharBody->SetYaw(vAngles.y - mpEnemy->GetModelOffsetAngles().y);
			pCharBody->SetEntity(mpEnemy->GetMeshEntity());
			pCharBody->SetActive(true);

			for (int i = 0; i < 3; ++i) {
				pCharBody->Update(1.0f / 60.0f);

				mpEnemy->GetMeshEntity()->UpdateLogic(1.0f / 60.0f);
				mpEnemy->GetMeshEntity()->UpdateGraphics(NULL, 1.0f / 60.0f, NULL);
			}

			if (mpEnemy->GetHealth() > 0)
				mpEnemy->ChangeState(STATE_HUNT);
			else
				mpEnemy->ChangeState(STATE_DEAD);
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_KnockDown::OnAnimationOver(const tString &asName) {
	if (mpEnemy->GetHealth() > 0)
		mpEnemy->ChangeState(STATE_HUNT);
	else
		mpEnemy->ChangeState(STATE_DEAD);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// DEAD STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Dead::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Death", false, 0.7f);

	// Sound
	mpEnemy->PlaySound(mpEnemySpider->msDeathSound);

	// Setup body
	mpEnemy->SetupBody();

	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);

	// Temp:
	// mpMover->GetCharBody()->GetBody()->SetCollideCharacter(false);
	mpMover->GetCharBody()->GetBody()->SetActive(false);

	mpEnemy->GetMover()->Stop();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Dead::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Spider_Dead::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameEnemy_Spider::cGameEnemy_Spider(cInit *apInit, const tString &asName, TiXmlElement *apGameElem) : iGameEnemy(apInit, asName, apGameElem) {
	LoadBaseProperties(apGameElem);

	//////////////////////////////
	// State properties
	mbPathFind = cString::ToBool(apGameElem->Attribute("PathFind"), true);

	mfIdleFOV = cMath::ToRad(cString::ToFloat(apGameElem->Attribute("IdleFOV"), 0));
	msIdleFoundPlayerSound = cString::ToString(apGameElem->Attribute("IdleFoundPlayerSound"), "");
	mfIdleMinSeeChance = cString::ToFloat(apGameElem->Attribute("IdleMinSeeChance"), 0);
	mfIdleMinHearVolume = cString::ToFloat(apGameElem->Attribute("IdleMinHearVolume"), 0);
	mfIdleMinWaitLength = cString::ToFloat(apGameElem->Attribute("IdleMinWaitLength"), 0);
	mfIdleMaxWaitLength = cString::ToFloat(apGameElem->Attribute("IdleMaxWaitLength"), 0);

	mfHuntFOV = cMath::ToRad(cString::ToFloat(apGameElem->Attribute("HuntFOV"), 0));
	mfHuntSpeed = cString::ToFloat(apGameElem->Attribute("HuntSpeed"), 0);
	mfHuntForLostPlayerTime = cString::ToFloat(apGameElem->Attribute("HuntForLostPlayerTime"), 0);
	mfHuntMinSeeChance = cString::ToFloat(apGameElem->Attribute("IdleMinSeeChance"), 0);
	mfHuntMinHearVolume = cString::ToFloat(apGameElem->Attribute("IdleMinHearVolume"), 0);

	mfAttackDistance = cString::ToFloat(apGameElem->Attribute("AttackDistance"), 0);
	mfAttackForce = cString::ToFloat(apGameElem->Attribute("AttackForce"), 0);
	mfAttackJumpTime = cString::ToFloat(apGameElem->Attribute("AttackJumpTime"), 0);
	mfAttackDamageTime = cString::ToFloat(apGameElem->Attribute("AttackDamageTime"), 0);
	mvAttackDamageSize = cString::ToVector3f(apGameElem->Attribute("AttackDamageSize"), 0);
	mfAttackDamageRange = cString::ToFloat(apGameElem->Attribute("AttackDamageRange"), 0);
	mfAttackMinDamage = cString::ToFloat(apGameElem->Attribute("AttackMinDamage"), 0);
	mfAttackMaxDamage = cString::ToFloat(apGameElem->Attribute("AttackMaxDamage"), 0);
	msAttackStartSound = cString::ToString(apGameElem->Attribute("AttackStartSound"), "");
	msAttackHitSound = cString::ToString(apGameElem->Attribute("AttackHitSound"), "");
	mfAttackMinMass = cString::ToFloat(apGameElem->Attribute("AttackMinMass"), 0);
	mfAttackMaxMass = cString::ToFloat(apGameElem->Attribute("AttackMaxMass"), 0);
	mfAttackMinImpulse = cString::ToFloat(apGameElem->Attribute("AttackMinImpulse"), 0);
	mfAttackMaxImpulse = cString::ToFloat(apGameElem->Attribute("AttackMaxImpulse"), 0);
	mlAttackStrength = cString::ToInt(apGameElem->Attribute("AttackStrength"), 0);
	mvPreloadSounds.push_back(msAttackStartSound);
	mvPreloadSounds.push_back(msAttackHitSound);

	mfFleeMinDistance = cString::ToFloat(apGameElem->Attribute("FleeMinDistance"), 0);
	mfFleeMaxDistance = cString::ToFloat(apGameElem->Attribute("FleeMaxDistance"), 0);
	mbFleeFromFlashlight = cString::ToBool(apGameElem->Attribute("FleeFromFlashlight"), true);

	msKnockDownSound = cString::ToString(apGameElem->Attribute("KnockDownSound"), "");
	mvPreloadSounds.push_back(msKnockDownSound);

	msDeathSound = cString::ToString(apGameElem->Attribute("DeathSound"), "");
	mvPreloadSounds.push_back(msDeathSound);

	//////////////////////////////
	// Set up states
	AddState(hplNew(cGameEnemyState_Spider_Idle, (STATE_IDLE, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Spider_Hunt, (STATE_HUNT, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Spider_Attack, (STATE_ATTACK, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Spider_Flee, (STATE_FLEE, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Spider_KnockDown, (STATE_KNOCKDOWN, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Spider_Dead, (STATE_DEAD, mpInit, this)));
}

//-----------------------------------------------------------------------

cGameEnemy_Spider::~cGameEnemy_Spider() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemy_Spider::OnLoad() {
	// Create attack shape
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	mpAttackShape = pPhysicsWorld->CreateBoxShape(mvAttackDamageSize, NULL);

	// Set up shape
	ChangeState(STATE_IDLE);
}

//-----------------------------------------------------------------------

void cGameEnemy_Spider::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

void cGameEnemy_Spider::ShowPlayer(const cVector3f &avPlayerFeetPos) {
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL ||
		mlCurrentState == STATE_INVESTIGATE) {
		mvLastPlayerPos = avPlayerFeetPos;
		ChangeState(STATE_HUNT);
	}
}

//-----------------------------------------------------------------------

bool cGameEnemy_Spider::MoveToPos(const cVector3f &avFeetPos) {
// FIXME: Code identical in each branch. Development leftover?
#if 0
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL) {
		// SetTempPosition(avFeetPos);
		// ChangeState(STATE_INVESTIGATE);
		// return true;
		return false;
	} else {
		return false;
	}
#else
	return false;
#endif
}

//-----------------------------------------------------------------------

bool cGameEnemy_Spider::IsFighting() {
	if (mfHealth <= 0 || IsActive() == false)
		return false;
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL ||
		mlCurrentState == STATE_INVESTIGATE)
		return false;

	return true;
}

//-----------------------------------------------------------------------
