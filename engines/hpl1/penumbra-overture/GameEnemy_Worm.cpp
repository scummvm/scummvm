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

#include "hpl1/penumbra-overture/GameEnemy_Worm.h"

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

iGameEnemyState_Worm_Base::iGameEnemyState_Worm_Base(int alId, cInit *apInit, iGameEnemy *apEnemy)
	: iGameEnemyState(alId, apInit, apEnemy) {
	mpEnemyWorm = static_cast<cGameEnemy_Worm *>(mpEnemy);
}

//-----------------------------------------------------------------------

void iGameEnemyState_Worm_Base::OnSeePlayer(const cVector3f &avPosition, float afChance) {
	if (mpPlayer->GetHealth() <= 0)
		return;

	if (afChance >= mpEnemyWorm->mfIdleMinSeeChance) {
		mpEnemy->ChangeState(STATE_HUNT);
		// mpEnemyWorm->PlaySound(mpEnemyWorm->msIdleFoundPlayerSound);
	}
}

bool iGameEnemyState_Worm_Base::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	/*float afDistance = (mpMover->GetCharBody()->GetPosition() - avPosition).Length();

	if(afVolume >= mpEnemyWorm->mfIdleMinHearVolume && afDistance > 0.4f)
	{
		mpEnemy->SetTempPosition(avPosition);
		mpEnemy->ChangeState(STATE_INVESTIGATE);
		return true;
	}*/

	return false;
}

void iGameEnemyState_Worm_Base::OnTakeHit(float afDamage) {
}

void iGameEnemyState_Worm_Base::OnFlashlight(const cVector3f &avPosition) {
	// mpInit->mpEffectHandler->GetSubTitle()->Add("Flashlight!",0.5f);
	OnSeePlayer(mpPlayer->GetCharacterBody()->GetFeetPosition(), 1.0f);
}

void iGameEnemyState_Worm_Base::OnDeath(float afDamage) {
	// mpEnemy->ChangeState(STATE_KNOCKDOWN);
	mpEnemy->ChangeState(STATE_DEAD);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// IDLE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Idle::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Idle", true, 0.2f);

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyWorm->mfIdleFOV);

	// mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Idle::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Idle::OnUpdate(float afTimeStep) {
	mpEnemy->ChangeState(STATE_HUNT);
	return;
	if (mpMover->IsMoving() == false || mpMover->GetStuckCounter() > 2.0f) {
		mpMover->ResetStuckCounter();

		if (mbStopped == false) {
			mbStopped = true;

			mpEnemy->PlayAnim("Idle", false, 0.9f);

			mfNextWalkTime = cMath::RandRectf(mpEnemyWorm->mfIdleMinWaitLength,
											  mpEnemyWorm->mfIdleMaxWaitLength);
		} else if (mfNextWalkTime <= 0) {
			mbStopped = false;

			// Animation
			mpEnemy->UseMoveStateAnimations();

			// Setup body
			mpEnemy->SetupBody();
			mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyWorm->mfHuntSpeed);

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

void cGameEnemyState_Worm_Hunt::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Idle", true, 0.2f);

	float fMul = 1.0f;

	// Setup body
	mpEnemy->SetupBody();
	if (mpInit->mDifficulty == eGameDifficulty_Easy)
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyWorm->mfHuntSpeed * 0.7f * fMul);
	else if (mpInit->mDifficulty == eGameDifficulty_Normal)
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyWorm->mfHuntSpeed * fMul);
	else
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyWorm->mfHuntSpeed * 1.2f * fMul);

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyWorm->mfHuntFOV);

	mfUpdatePathCount = 0;
	mfUpdateFreq = 1.0f;
	mbFreePlayerPath = false;

	mbLostPlayer = false;
	mfLostPlayerCount = 0;
	mfMaxLostPlayerCount = mpEnemyWorm->mfHuntForLostPlayerTime;

	mpInit->mpMusicHandler->AddAttacker(mpEnemy);

	mfAttackCount = mpEnemyWorm->mfAttackInterval;

	mfAttackSoundCount = 0;

	mfSoundCount = cMath::RandRectf(mpEnemyWorm->mfHuntSoundMinInteraval,
									mpEnemyWorm->mfHuntSoundMaxInteraval);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Hunt::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Hunt::OnUpdate(float afTimeStep) {
	// cAnimationState *pState = mpEnemy->GetMeshEntity()->GetAnimationState(0);
	// Log("Anim: %s %f %d\n",pState->GetName(),pState->GetTimePosition(),pState->IsActive());

	mpEnemy->SetLastPlayerPos(mpInit->mpPlayer->GetCharacterBody()->GetFeetPosition());

	if (mpPlayer->GetHealth() <= 0) {
		mpEnemy->ChangeState(STATE_IDLE);
		return;
	}

	////////////////////////////////
	// Check if stuck
	if (mpMover->GetStuckCounter() > 2.1f) {
		// mpEnemy->ChangeState(STATE_FLEE);
		mpMover->ResetStuckCounter();
		return;
	}

	//////////////////////////////
	// Hunt sound count
	if (mfSoundCount <= 0) {
		mfSoundCount = cMath::RandRectf(mpEnemyWorm->mfHuntSoundMinInteraval,
										mpEnemyWorm->mfHuntSoundMaxInteraval);

		mpEnemy->PlaySound(mpEnemyWorm->msHuntSound);
	} else {
		mfSoundCount -= afTimeStep;
	}

	//////////////////////////////
	// Attack sound count
	if (mfAttackSoundCount > 0) {
		mfAttackSoundCount -= afTimeStep;
	}

	////////////////////////////////
	// Check if attack should begin
	if (mfAttackCount <= 0) {
		mfAttackCount = mpEnemyWorm->mfAttackInterval;

		cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
						 mpMover->GetCharBody()->GetForward() *
							 (0 +
							  mpEnemyWorm->mvAttackDamageSize.z / 2.0f);

		cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
		cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
		mtxOffset.SetTranslation(vPos);

		eAttackTargetFlag target = eAttackTargetFlag_Player | eAttackTargetFlag_Bodies;

		if (mpInit->mpAttackHandler->CreateShapeAttack(mpEnemyWorm->GetAttackShape(),
													   mtxOffset,
													   mpMover->GetCharBody()->GetPosition(),
													   mpEnemyWorm->mfAttackDamage,

													   mpEnemyWorm->mfAttackMinMass, mpEnemyWorm->mfAttackMaxMass,
													   mpEnemyWorm->mfAttackMinImpulse, mpEnemyWorm->mfAttackMaxImpulse,

													   mpEnemyWorm->mlAttackStrength,

													   target, NULL)) {
			if (mfAttackSoundCount <= 0) {
				mpEnemy->PlaySound(mpEnemyWorm->msAttackHitSound);

				mfAttackSoundCount = mpEnemyWorm->mfAttackHitSoundInterval;
			}
		}
	} else {
		mfAttackCount -= afTimeStep;
	}

	////////////////////////////////
	// Update the path
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
				mfUpdatePathCount *= 2.0f;
			}
		}
	} else {
		mfUpdatePathCount -= afTimeStep;
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
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Hunt::OnSeePlayer(const cVector3f &avPosition, float afChance) {
	if (mbLostPlayer && afChance >= mpEnemyWorm->mfHuntMinSeeChance) {
		mbLostPlayer = false;
		mfUpdatePathCount = 0;
	}
}

//-----------------------------------------------------------------------

bool cGameEnemyState_Worm_Hunt::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	//////////////////////////////////
	// If player is lost the sound might be of help
	if (mbLostPlayer) {
		// Check if sound can be heard
		if (afVolume >= mpEnemyWorm->mfHuntMinHearVolume) {
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

void cGameEnemyState_Worm_Hunt::OnDraw() {
	// mpInit->mpDefaultFont->Draw(cVector3f(230,10,100),14,cColor(1,1,1,1),eFontAlign_Left,
	//							"Freepath: %d",mbFreePlayerPath);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Hunt::OnPostSceneDraw() {
	cCamera3D *pCamera = static_cast<cCamera3D *>(mpInit->mpGame->GetScene()->GetCamera());

	cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
					 mpMover->GetCharBody()->GetForward() *
						 (0 + mpEnemyWorm->mvAttackDamageSize.z / 2.0f);

	cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
	cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
	mtxOffset.SetTranslation(vPos);

	cMatrixf mtxCollider = cMath::MatrixMul(pCamera->GetViewMatrix(), mtxOffset);

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetMatrix(eMatrix_ModelView, mtxCollider);

	cVector3f vSize = mpEnemyWorm->GetAttackShape()->GetSize();
	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(vSize * 0.5f, vSize * -0.5f,
																cColor(1, 0, 1, 1));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// DEAD STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Dead::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Idle", true, 0.2f);

	// Setup body
	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);

	if (mpEnemyWorm->mpMoveSound)
		mpEnemyWorm->mpMoveSound->Stop(false);
	mpEnemyWorm->mpMoveSound = NULL;
}

//-----------------------------------------------------------------------

void cGameEnemyState_Worm_Dead::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameEnemy_Worm::cGameEnemy_Worm(cInit *apInit, const tString &asName, TiXmlElement *apGameElem) : iGameEnemy(apInit, asName, apGameElem) {
	mpMoveSound = NULL;

	LoadBaseProperties(apGameElem);

	mbSetFeetAtGroundOnStart = false;
	mbAttachMeshToBody = false;
	mbRemoveAttackerOnDisable = false;

	//////////////////////////////
	// State properties
	msMoveSound = cString::ToString(apGameElem->Attribute("MoveSound"), "");

	mfIdleFOV = cMath::ToRad(cString::ToFloat(apGameElem->Attribute("IdleFOV"), 0));
	msIdleFoundPlayerSound = cString::ToString(apGameElem->Attribute("IdleFoundPlayerSound"), "");
	mfIdleMinSeeChance = cString::ToFloat(apGameElem->Attribute("IdleMinSeeChance"), 0);
	mfIdleMinHearVolume = cString::ToFloat(apGameElem->Attribute("IdleMinHearVolume"), 0);

	mfHuntFOV = cMath::ToRad(cString::ToFloat(apGameElem->Attribute("HuntFOV"), 0));
	mfHuntSpeed = cString::ToFloat(apGameElem->Attribute("HuntSpeed"), 0);
	mfHuntMinSeeChance = cString::ToFloat(apGameElem->Attribute("IdleMinSeeChance"), 0);
	mfHuntMinHearVolume = cString::ToFloat(apGameElem->Attribute("IdleMinHearVolume"), 0);

	msHuntSound = cString::ToString(apGameElem->Attribute("HuntSound"), "");
	mfHuntSoundMinInteraval = cString::ToFloat(apGameElem->Attribute("HuntSoundMinInteraval"), 0);
	mfHuntSoundMaxInteraval = cString::ToFloat(apGameElem->Attribute("HuntSoundMaxInteraval"), 0);

	mfAttackInterval = cString::ToFloat(apGameElem->Attribute("AttackInterval"), 0);
	mfAttackDamage = cString::ToFloat(apGameElem->Attribute("AttackDamage"), 0);
	msAttackHitSound = cString::ToString(apGameElem->Attribute("AttackHitSound"), "");
	mfAttackHitSoundInterval = cString::ToFloat(apGameElem->Attribute("AttackHitSoundInterval"), 0);
	mfAttackMinMass = cString::ToFloat(apGameElem->Attribute("AttackMinMass"), 0);
	mfAttackMaxMass = cString::ToFloat(apGameElem->Attribute("AttackMaxMass"), 0);
	mfAttackMinImpulse = cString::ToFloat(apGameElem->Attribute("AttackMinImpulse"), 0);
	mfAttackMaxImpulse = cString::ToFloat(apGameElem->Attribute("AttackMaxImpulse"), 0);
	mlAttackStrength = cString::ToInt(apGameElem->Attribute("AttackStrength"), 0);

	mvAttackDamageSize = cString::ToVector3f(apGameElem->Attribute("AttackDamageSize"), 0);

	//////////////////////////////
	// Set up states
	AddState(hplNew(cGameEnemyState_Worm_Idle, (STATE_IDLE, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Worm_Hunt, (STATE_HUNT, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Worm_Dead, (STATE_DEAD, mpInit, this)));

	/////////////////////////////
	// Internal variables
	mvLastForward = cVector3f(0, 0, 1);

	mlMaxSegmentPostions = 20;
	mfTurnSpeed = cMath::ToRad(160.0f);
}

//-----------------------------------------------------------------------

cGameEnemy_Worm::~cGameEnemy_Worm() {
	if (mpMoveSound)
		mpInit->mpGame->GetScene()->GetWorld3D()->DestroySoundEntity(mpMoveSound);

	STLDeleteAll(mvTailSegments);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// MESH ENTITY CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameEnemy_Worm_MeshCallback::cGameEnemy_Worm_MeshCallback(cGameEnemy_Worm *apWorm) {
	mpWorm = apWorm;
}
//-----------------------------------------------------------------------

/*OLD CALC MATRIX VERSION:
cMatrixf mtxRot = cMatrixf::Identity;
mtxRot.SetRight(mpWorm->mvRootRight*-1); // *-1 = straange
mtxRot.SetUp(mpWorm->mvRootUp);
mtxRot.SetForward(mpWorm->mvRootForward);

mtxRot = cMath::MatrixInverse(mtxRot);

iCharacterBody *pCharBody = mpWorm->mpMover->GetCharBody();
cMeshEntity *pEntity = mpWorm->mpMeshEntity;
cMatrixf mtxEntity = mtxRot;
//mtxEntity = cMath::MatrixMul(pCharBody->GetEntityPostOffset(),mtxEntity);
mtxEntity.SetTranslation(mtxEntity.GetTranslation() + mpWorm->mvRootPosition +
pCharBody->GetEntityOffset().GetTranslation());

//mtxEntity = cMath::MatrixMul(mtxEntity,pCharBody->GetEntityOffset());

//pEntity->GetRootNode()->SetMatrix(mtxEntity);
pEntity->SetMatrix(mtxEntity);
//mpWorm->mpRootBone->SetWorldMatrix(mtxEntity);
*/

void cGameEnemy_Worm_MeshCallback::AfterAnimationUpdate(cMeshEntity *apMeshEntity, float afTimeStep) {
	// Set bone matrix of root
	// if(mpWorm->mbAttachMeshToBody==false)
	if (mpWorm->mpMover->GetCharBody()->GetEntity() == NULL) {
		iCharacterBody *pCharBody = mpWorm->mpMover->GetCharBody();
		cMeshEntity *pEntity = mpWorm->mpMeshEntity;

		cMatrixf mtxEntity = cMatrixf::Identity;

		cVector3f vAngles = cMath::GetAngleFromPoints3D(0, mpWorm->mvRootForward);
		mtxEntity = cMath::MatrixRotate(cVector3f(-vAngles.x, kPif + vAngles.y, 0), eEulerRotationOrder_XYZ);

		mtxEntity.SetTranslation(mtxEntity.GetTranslation() + mpWorm->mvRootPosition +
								 pCharBody->GetEntityOffset().GetTranslation());

		pEntity->SetMatrix(mtxEntity);
	}

	// return;

	// Set bone matrix of segments
	for (size_t i = 0; i < mpWorm->mvTailSegments.size(); ++i) {
		cWormTailSegment *pSegment = mpWorm->mvTailSegments[i];

		//////////////////////////////////////////////////
		// Change orientation according to forward
		cMatrixf mtxTrans = cMatrixf::Identity;
		{
			/*mtxTrans.SetRight(pSegment->mvRight*-1); // *-1 = straaange
			mtxTrans.SetUp(pSegment->mvUp);
			mtxTrans.SetForward(pSegment->mvForward);

			mtxTrans = cMath::MatrixInverse(mtxTrans);*/

			cVector3f vAngles = cMath::GetAngleFromPoints3D(0, pSegment->mvForward);
			mtxTrans = cMath::MatrixRotate(cVector3f(-vAngles.x, kPif + vAngles.y, 0), eEulerRotationOrder_XYZ);

			mtxTrans = cMath::MatrixMul(mtxTrans, pSegment->m_mtxBaseRot);
		}

		////////////////////////////////////////
		// Set world matrix of bone
		mtxTrans.SetTranslation(pSegment->mvPostion);
		pSegment->mpBone->SetWorldMatrix(mtxTrans);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cVector3Smoother::cVector3Smoother() {
	mlMaxVecs = 20;
}

void cVector3Smoother::Add(const cVector3f &avVec) {
	mlstVecs.push_back(avVec);
	if ((int)mlstVecs.size() > mlMaxVecs)
		mlstVecs.pop_front();
}

cVector3f cVector3Smoother::GetAverage() {
	cVector3f vAverage = 0;
	Common::List<cVector3f>::iterator it = mlstVecs.begin();
	for (; it != mlstVecs.end(); ++it) {
		vAverage += *it;
	}

	return vAverage / (float)mlstVecs.size();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemy_Worm::OnLoad() {
	mbFirstUpdate = true;
	mvFirstUpdatePos = mpMover->GetCharBody()->GetPosition();
	mvRootPosition = mvFirstUpdatePos;

	mvLastForward = mpMover->GetCharBody()->GetForward();

	// Create attack shape
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	mpAttackShape = pPhysicsWorld->CreateBoxShape(mvAttackDamageSize, NULL);

	mpMeshCallback = hplNew(cGameEnemy_Worm_MeshCallback, (this));
	mpMeshEntity->SetCallback(mpMeshCallback);

	// Set up enemy
	ChangeState(STATE_IDLE);

	SetupTail();
}

//-----------------------------------------------------------------------

cVector3f gvBackPos, gvPrevBackPos;

void cGameEnemy_Worm::OnUpdate(float afTimeStep) {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	iCharacterBody *pCharBody = mpMover->GetCharBody();
	cVector3f vMovement;

	////////////////////////////////////////
	// Update movesound

	if (mlCurrentState == STATE_HUNT) {
		if (mpMoveSound == NULL) {
			mpMoveSound = pWorld->CreateSoundEntity("WormMove", msMoveSound, true);
			// Log("created move sound...\n");
		} else {
			if (pWorld->SoundEntityExists(mpMoveSound)) {
				// Log("Updating move sound...");
				mpMoveSound->SetPosition(pCharBody->GetPosition());
				// Log("done\n");
			} else {
				mpMoveSound = NULL;
			}
		}
	} else {
		if (mpMoveSound) {
			pWorld->DestroySoundEntity(mpMoveSound);
			mpMoveSound = NULL;
		}
	}

	////////////////////////////////////////
	// Check so the body is moving
	if (pCharBody->GetForceVelocity().Length() < 0.0001f &&
		pCharBody->GetMoveSpeed(eCharDir_Forward) < 0.001f) {
		return;
	}

	/////////////////////////////////////////
	// Get the position of the worm

	cVector3f vPrevRootPos = mvRootPosition;

	// Add newer position to list
	if (mbFirstUpdate) {
		mlstRootPositions.push_back(mvFirstUpdatePos);
		mbFirstUpdate = false;
	} else
		mlstRootPositions.push_back(pCharBody->GetPosition());

	if ((int)mlstRootPositions.size() > mlMaxSegmentPostions)
		mlstRootPositions.pop_front();

	// Get smooth position
	mvRootPosition = 0;
	Common::List<cVector3f>::iterator posIt = mlstRootPositions.begin();
	for (; posIt != mlstRootPositions.end(); ++posIt) {
		mvRootPosition += *posIt;
	}
	mvRootPosition = mvRootPosition / (float)mlstRootPositions.size();

	// Get movement this update
	vMovement = mvRootPosition - vPrevRootPos;

	/////////////////////////////////////////
	// Calculate direction vectors
	if (vMovement.SqrLength() > 0.00001f)
		mvRootGoalForward = cMath::Vector3Normalize(vMovement);
	// else
	//	mvRootGoalForward = mvRootGoalForward;

	// Rotate vectors to get closer to goal
	{
		float fAngleDist = cMath::Vector3Angle(mvRootForward, mvRootGoalForward);
		if (fAngleDist > 0.001f) {
			// Iterate to split the turning into 10 smaller parts
			cVector3f vTurnVec = cMath::Vector3Cross(mvRootForward, mvRootGoalForward);

			cQuaternion qRotation;
			if (fAngleDist <= afTimeStep * mfTurnSpeed)
				qRotation = cQuaternion(fAngleDist, vTurnVec);
			else
				qRotation = cQuaternion(afTimeStep * mfTurnSpeed, vTurnVec);

			cMatrixf mtxRot = cMath::MatrixQuaternion(qRotation);
			mvRootForward = cMath::MatrixMul(mtxRot, mvRootForward);
			// mvRootForward.y=0;
			mvRootForward.Normalise();

			/*cVector3f vLastRight = mvRootRight;
			mvRootRight = cMath::MatrixMul(	mtxRot,	mvRootRight);
			mvRootRight.y =0;
			mvRootRight.Normalise(); //Make sure x-z plane

			mvRootUp = cMath::Vector3Cross(mvRootRight,mvRootForward);
			mvRootUp.Normalise();*/
		}
	}

	////////////////////////////////////////
	// Get the ass position and direction
	cVector3f vSegForward = mvRootForward;
	cVector3f vSegBackPos = mpRootBone->GetWorldPosition() + vSegForward * -mvTailSegments[0]->mfDistToFront;
	gvBackPos = vSegBackPos;

	/////////////////////////////////////////
	// Iterate the segments.
	for (size_t i = 0; i < mvTailSegments.size(); ++i) {
		cWormTailSegment *pSegment = mvTailSegments[i];

		//////////////////////////////////
		// Change postion of segment

		// Get add newer pos and smooth all the previuos
		cVector3f vPrevPos = pSegment->mvPostion;
		pSegment->mlstPositions.push_back(vSegBackPos);
		if ((int)pSegment->mlstPositions.size() > mlMaxSegmentPostions) {
			pSegment->mlstPositions.pop_front();
		}
		pSegment->mvPostion = 0;
		Common::List<cVector3f>::iterator posIt2 = pSegment->mlstPositions.begin();
		for (; posIt2 != pSegment->mlstPositions.end(); ++posIt2) {
			pSegment->mvPostion += *posIt2;
		}
		pSegment->mvPostion = pSegment->mvPostion / (float)pSegment->mlstPositions.size();

		/////////////////////////////////////////////
		// Get the movement vector
		cVector3f vSegMovement = pSegment->mvPostion - vPrevPos;

		/////////////////////////////////////////////
		// Update body position
		pSegment->mpBody->SetPosition(pSegment->mvPostion);

		//////////////////////////////////////////////
		// Get the direction vector of the segment
		{
			if (vSegMovement.SqrLength() > 0.00001f)
				pSegment->mvGoalForward = cMath::Vector3Normalize(vSegMovement);
			// Used for more stiff worms
			// pSegment->mvGoalForward = vSegForward;

			float fAngleDist = cMath::Vector3Angle(pSegment->mvForward, pSegment->mvGoalForward);
			if (fAngleDist > 0.001f) {
				cVector3f vTurnVec = cMath::Vector3Cross(pSegment->mvForward, pSegment->mvGoalForward);

				cQuaternion qRotation;
				if (fAngleDist <= afTimeStep * mfTurnSpeed)
					qRotation = cQuaternion(fAngleDist, vTurnVec);
				else
					qRotation = cQuaternion(afTimeStep * mfTurnSpeed, vTurnVec);

				pSegment->mvForward = cMath::MatrixMul(cMath::MatrixQuaternion(qRotation),
													   pSegment->mvForward);
				pSegment->mvForward.Normalise();

				/*pSegment->mvRight = cMath::MatrixMul(	cMath::MatrixQuaternion(qRotation),
														pSegment->mvRight);

				pSegment->mvRight.y =0; pSegment->mvRight.Normalise();


				pSegment->mvUp = cMath::Vector3Cross(pSegment->mvRight,pSegment->mvForward);
				pSegment->mvUp.Normalise();*/
			}
		}
		vSegForward = pSegment->mvForward;

		////////////////////////////////////////
		// Set the New back pos
		if (i < mvTailSegments.size() - 1) {
			vSegBackPos = pSegment->mvPostion +
						  pSegment->mvForward * -mvTailSegments[i + 1]->mfDistToFront;
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemy_Worm::ExtraPostSceneDraw() {
	iLowLevelGraphics *pLowLevelGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();

	pLowLevelGfx->SetDepthTestActive(false);

	// pLowLevelGfx->DrawLine(mpRootBone->GetWorldPosition(),gvBackPos,cColor(1,1));
	pLowLevelGfx->DrawLine(mpRootBone->GetWorldPosition(),
						   mpRootBone->GetWorldPosition() + mvRootForward, cColor(0, 0, 1, 1));
	pLowLevelGfx->DrawLine(mpRootBone->GetWorldPosition(),
						   mpRootBone->GetWorldPosition() + mvRootUp, cColor(0, 1, 0, 1));
	pLowLevelGfx->DrawLine(mpRootBone->GetWorldPosition(),
						   mpRootBone->GetWorldPosition() + mvRootRight, cColor(1, 0, 0, 1));

	pLowLevelGfx->DrawSphere(mpRootBone->GetWorldPosition(), 0.3f, cColor(1, 0, 1, 1));

	for (size_t i = 0; i < mvTailSegments.size(); ++i) {
		cWormTailSegment *pSegment = mvTailSegments[i];

		pLowLevelGfx->DrawSphere(pSegment->mvPostion, 0.3f, cColor(1, 1));

		pLowLevelGfx->DrawLine(pSegment->mvPostion,
							   pSegment->mvPostion + pSegment->mvForward * 0.5f,
							   cColor(1, 0, 1, 1));

		cVector3f vForward = cMath::MatrixMul(cMatrixf::Identity, pSegment->mvForward);
		cVector3f vRight = cMath::MatrixMul(cMatrixf::Identity, pSegment->mvRight);
		cVector3f vUp = cMath::MatrixMul(cMatrixf::Identity, pSegment->mvUp);
		cVector3f vPos = pSegment->mpBone->GetWorldPosition();

		pLowLevelGfx->DrawLine(vPos, vPos + vRight * 0.6f, cColor(1, 0, 0, 1));
		pLowLevelGfx->DrawLine(vPos, vPos + vUp * 0.6f, cColor(0, 1, 0, 1));
		pLowLevelGfx->DrawLine(vPos, vPos + vForward * 0.6f, cColor(0, 0, 1, 1));

		// pLowLevelGfx->DrawLine( pSegment->mvPostion,pSegment->mvBackPos,cColor(0,0,1,1));
	}

	pLowLevelGfx->SetDepthTestActive(true);
}

//-----------------------------------------------------------------------

void cGameEnemy_Worm::ShowPlayer(const cVector3f &avPlayerFeetPos) {
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL ||
		mlCurrentState == STATE_INVESTIGATE) {
		mvLastPlayerPos = avPlayerFeetPos;
		ChangeState(STATE_HUNT);
	}
}

//-----------------------------------------------------------------------

bool cGameEnemy_Worm::MoveToPos(const cVector3f &avFeetPos) {
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL) {
		SetTempPosition(avFeetPos);
		ChangeState(STATE_INVESTIGATE);
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------

bool cGameEnemy_Worm::IsFighting() {
	if (mfHealth <= 0 || IsActive() == false)
		return false;
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL ||
		mlCurrentState == STATE_INVESTIGATE)
		return false;

	return true;
}
//-----------------------------------------------------------------------

void cGameEnemy_Worm::SetupTail() {
	// Log("Setting up tail!\n");

	iCharacterBody *pCharBody = mpMover->GetCharBody();
	cMeshEntity *pEntity = mpMeshEntity;

	// Set up character body
	pCharBody->SetCollideCharacter(false);

	// Set up mesh entity matrix.
	// Do not set any rotation here since then the base
	// bone matrices will not be correct.
	if (mbAttachMeshToBody == false) {
		cMatrixf mtxEntity = cMatrixf::Identity;
		mtxEntity.SetTranslation(pCharBody->GetPosition() +
								 pCharBody->GetEntityOffset().GetTranslation());
		pEntity->SetMatrix(mtxEntity);
	}

	// Get root bone and directions
	mpRootBone = mpMeshEntity->GetBoneStateFromName("Root");
	mvRootForward = mpMover->GetCharBody()->GetForward();
	mvRootUp = mpMover->GetCharBody()->GetUp();
	mvRootRight = mpMover->GetCharBody()->GetRight();

	mvTailSegments.resize(7);
	for (int i = 0; i < 7; ++i) {
		// Create and set to right data
		mvTailSegments[i] = hplNew(cWormTailSegment, ());
		cWormTailSegment *pSegment = mvTailSegments[i];
		if (i == 0) {
			mpRootSegment = pSegment;
		} else {
			mvTailSegments[i - 1]->mpChildSegment = pSegment;
		}
		/////////////////////////////////////////
		// Set up

		// Get bones
		tString sBoneName = "Tail0" + cString::ToString(i + 1);
		pSegment->mpBone = mpMeshEntity->GetBoneStateFromName(sBoneName);
		pSegment->mpBone->SetActive(false);

		// Start Position,forward and rotation
		pSegment->mvPostion = pSegment->mpBone->GetWorldPosition();

		pSegment->mvForward = mpMover->GetCharBody()->GetForward();
		pSegment->mvUp = mpMover->GetCharBody()->GetUp();
		pSegment->mvRight = mpMover->GetCharBody()->GetRight();

		pSegment->mvGoalForward = pSegment->mvForward;

		// Get the rotation it needs to be pointed forward.
		pSegment->m_mtxBaseRot = mvTailSegments[i]->mpBone->GetWorldMatrix().GetRotation();

		// Create body
		iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
		iCollideShape *pShape = pPhysicsWorld->CreateSphereShape(pCharBody->GetSize().x / 2.0f, NULL);
		pSegment->mpBody = pPhysicsWorld->CreateBody("Tail0" + cString::ToString(i + 1), pShape);
		pSegment->mpBody->SetMass(0);
		pSegment->mpBody->SetPosition(pSegment->mvPostion);
		pSegment->mpBody->SetIsCharacter(true);
		pSegment->mpBody->SetActive(IsActive());

		mvBodies.push_back(pSegment->mpBody);

		if (i == 0) {
			pSegment->mfDistToFront = cMath::Vector3Dist(mpRootBone->GetWorldPosition(),
														 pSegment->mpBone->GetWorldPosition());
		} else {
			pSegment->mfDistToFront = cMath::Vector3Dist(
				mvTailSegments[i - 1]->mpBone->GetWorldPosition(),
				pSegment->mpBone->GetWorldPosition());
		}

		// Log("%d Dist: %f Fwd: %s\n",i,pSegment->mfDistToFront,pSegment->mvForward.ToString().c_str());
	}
}

//-----------------------------------------------------------------------
