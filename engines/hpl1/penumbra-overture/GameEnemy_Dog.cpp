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

#include "hpl1/penumbra-overture/GameEnemy_Dog.h"

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

iGameEnemyState_Dog_Base::iGameEnemyState_Dog_Base(int alId, cInit *apInit, iGameEnemy *apEnemy)
	: iGameEnemyState(alId, apInit, apEnemy) {
	mpEnemyDog = static_cast<cGameEnemy_Dog *>(mpEnemy);
}

//-----------------------------------------------------------------------

void iGameEnemyState_Dog_Base::OnSeePlayer(const cVector3f &avPosition, float afChance) {
	// return;
	if (mpPlayer->GetHealth() <= 0)
		return;

	if (afChance >= mpEnemyDog->mfIdleMinSeeChance) {
		/*if( (mlId == STATE_IDLE || mlId == STATE_INVESTIGATE || mlId == STATE_PATROL) &&
			cMath::RandRectf(0,1) < mpEnemyDog->mfIdleCallBackupChance &&
			mpEnemy->CheckForTeamMate(8,true)==false)
		{
			mpEnemy->ChangeState(STATE_CALLBACKUP);
		}
		else
		{
			//mpEnemy->ChangeState(STATE_HUNT);
			//mpEnemyDog->PlaySound(mpEnemyDog->msIdleFoundPlayerSound);
		}*/

		float fDist = cMath::Vector3Dist(mpMover->GetCharBody()->GetFeetPosition(),
										 mpPlayer->GetCharacterBody()->GetFeetPosition());
		if (fDist >= mpEnemyDog->mfAttentionMinDist) {
			mpEnemy->ChangeState(STATE_ATTENTION);
		} else {
			mpEnemy->ChangeState(STATE_HUNT);
			mpEnemyDog->PlaySound(mpEnemyDog->msIdleFoundPlayerSound);
		}
	}
}

bool iGameEnemyState_Dog_Base::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	// return false;
	float afDistance = (mpMover->GetCharBody()->GetPosition() - avPosition).Length();

	if (afVolume >= mpEnemyDog->mfIdleMinHearVolume && afDistance > 0.4f) {
		mpEnemy->SetTempPosition(avPosition);
		mpEnemy->ChangeState(STATE_INVESTIGATE);
		return true;
	}

	return false;
}

void iGameEnemyState_Dog_Base::OnTakeHit(float afDamage) {
	if (afDamage >= mpEnemyDog->mfMinKnockDamage) {
		if (mpInit->mbWeaponAttacking) {
			float fChance = afDamage / mpEnemyDog->mfCertainKnockDamage; //(mpEnemyDog->mfCertainKnockDamage*4);
			if (fChance > cMath::RandRectf(0, 1)) {
				mpEnemy->ChangeState(STATE_KNOCKDOWN);
			}
		} else {
			if (afDamage >= mpEnemyDog->mfCertainKnockDamage) {
				mpEnemy->ChangeState(STATE_KNOCKDOWN);
			} else {
				float fChance = afDamage / mpEnemyDog->mfCertainKnockDamage;
				if (fChance > cMath::RandRectf(0, 1)) {
					mpEnemy->ChangeState(STATE_KNOCKDOWN);
				}
			}
		}
	}
}

void iGameEnemyState_Dog_Base::OnFlashlight(const cVector3f &avPosition) {
	// mpInit->mpEffectHandler->GetSubTitle()->Add("Flashlight!",0.5f,true);
	// OnSeePlayer(mpPlayer->GetCharacterBody()->GetFeetPosition(),1.0f);

	// mpEnemy->SetLastPlayerPos(mpPlayer->GetCharacterBody()->GetFeetPosition());
	// mpEnemy->ChangeState(STATE_HUNT);

	mpEnemy->SetTempPosition(avPosition);
	mpEnemy->ChangeState(STATE_INVESTIGATE);
}

void iGameEnemyState_Dog_Base::OnDeath(float afDamage) {
	mpEnemy->ChangeState(STATE_DEAD);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// IDLE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Idle::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyDog->mfIdleFOV);

	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Idle::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Idle::OnUpdate(float afTimeStep) {
	if (mpEnemy->GetPatrolNodeNum() > 0) {
		mpEnemy->ChangeState(STATE_PATROL);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PATROL STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Patrol::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyDog->mfIdleFOV);

	// Setup patrol
	cEnemyPatrolNode *pPatrolNode = mpEnemy->CurrentPatrolNode();
	cAINode *pNode = mpMover->GetNodeContainer()->GetNodeFromName(pPatrolNode->msNodeName);

	if (mpEnemy->GetDoorBreakCount() > 3.0f) {
		mpEnemy->SetDoorBreakCount(0);
		mpMover->SetMaxDoorToughness(0);
	}

	mbWaiting = false;
	mbAnimation = false;
	mlStuckAtMaxCount = 0;

	mfIdleSoundTime = cMath::RandRectf(mpEnemyDog->mfIdleSoundMinInteraval,
									   mpEnemyDog->mfIdleSoundMaxInteraval);

	mpMover->SetMaxDoorToughness(-1);

	if (mpMover->MoveToPos(pNode->GetPosition()) == false) {
		// tString sStr = "Could not get to path node "+pPatrolNode->msNodeName;
		// mpInit->mpEffectHandler->GetSubTitle()->Add(sStr,3,true);

		mpEnemy->IncCurrentPatrolNode();
		mbWaiting = true;
		mpEnemy->SetWaitTime(1.0f);
	} else {
		// tString sStr = "Moving to path node "+pPatrolNode->msNodeName;
		// mpInit->mpEffectHandler->GetSubTitle()->Add(sStr,3,true);
	}

	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Patrol::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Patrol::OnUpdate(float afTimeStep) {
	/////////////////////////////////
	// Waiting for timer or animation to end
	if (mbWaiting) {
		//////////////////////////////
		// Play idle sound
		if (mfIdleSoundTime <= 0) {
			mfIdleSoundTime = cMath::RandRectf(mpEnemyDog->mfIdleSoundMinInteraval,
											   mpEnemyDog->mfIdleSoundMaxInteraval);

			mpEnemy->PlaySound(mpEnemyDog->msIdleSound);
		} else {
			mfIdleSoundTime -= afTimeStep;
		}

		//////////////////////////////
		// Timer is up
		if (mpEnemy->GetWaitTimeCount() >= mpEnemy->GetWaitTime()) {
			if (mbAnimation == false) {
				mpEnemy->SetWaitTimeCount(0);
				cEnemyPatrolNode *pPatrolNode = mpEnemy->CurrentPatrolNode();
				cAINode *pNode = mpMover->GetNodeContainer()->GetNodeFromName(pPatrolNode->msNodeName);

				mpEnemy->UseMoveStateAnimations();
				mbWaiting = false;

				if (mpMover->MoveToPos(pNode->GetPosition()) == false) {
					// tString sStr = "Could not get to path node "+pPatrolNode->msNodeName;
					// mpInit->mpEffectHandler->GetSubTitle()->Add(sStr,3,false);
					mpEnemy->IncCurrentPatrolNode();
					mbWaiting = true;
					mpEnemy->SetWaitTime(1.0f);
				} else {
					// tString sStr = "Moving to path node "+pPatrolNode->msNodeName;
					// mpInit->mpEffectHandler->GetSubTitle()->Add(sStr,3,false);
				}

			} else {
				mpEnemy->GetCurrentAnimation()->SetLoop(false);
			}
		} else {
			mpEnemy->AddWaitTimeCount(afTimeStep);
		}
	}
	/////////////////////////////////
	// Check if path is over
	else {
		//////////////////////////////
		// Play idle sound
		if (mfIdleSoundTime <= 0) {
			mfIdleSoundTime = cMath::RandRectf(mpEnemyDog->mfIdleSoundMinInteraval,
											   mpEnemyDog->mfIdleSoundMaxInteraval);

			mpEnemy->PlaySound(mpEnemyDog->msIdleSound);
		} else {
			mfIdleSoundTime -= afTimeStep;
		}

		//////////////////////////////
		// Stuck counter
		if (mpMover->GetStuckCounter() > 1.7f) {
			if (mpEnemy->CheckForDoor()) {
				mpEnemy->ChangeState(STATE_BREAKDOOR);
			} else {
				mlStuckAtMaxCount++;
				if (mlStuckAtMaxCount >= 6) {
					mpEnemy->ChangeState(STATE_IDLE);
					mpEnemy->SetWaitTime(1.0f);
					mpEnemy->IncCurrentPatrolNode();
				}
			}
			mpMover->ResetStuckCounter();
		}

		//////////////////////////////
		// Got to ned of path
		if (mpMover->IsMoving() == false) {
			cEnemyPatrolNode *pPatrolNode = mpEnemy->CurrentPatrolNode();

			mpEnemy->SetWaitTime(pPatrolNode->mfWaitTime);
			mpEnemy->IncCurrentPatrolNode();

			if (pPatrolNode->msAnimation != "") {
				mpEnemy->PlayAnim(pPatrolNode->msAnimation, true, 0.2f);
				mbAnimation = true;
			}

			mbWaiting = true;
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Patrol::OnAnimationOver(const tString &asName) {
	mbAnimation = false;
}

//////////////////////////////////////////////////////////////////////////
// ATTENTION STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attention::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Angry", true, 0.2f);

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyDog->mfIdleFOV);

	mpMover->Stop();
	mpMover->TurnToPos(mpPlayer->GetCharacterBody()->GetFeetPosition());

	mpEnemy->PlaySound(mpEnemyDog->msAttentionSound);
	mfTime = mpEnemyDog->mfAttentionTime;

#ifndef DEMO_VERSION
	if (mpInit->mDifficulty == eGameDifficulty_Easy)
		mfTime *= 1.7f;

#endif
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attention::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attention::OnUpdate(float afTimeStep) {
	mpMover->TurnToPos(mpPlayer->GetCharacterBody()->GetFeetPosition());
	mfTime -= afTimeStep;

	if (mfTime <= 0) {
		if (mpEnemy->CanSeePlayer()) {
			mpEnemy->ChangeState(STATE_HUNT);
			mpEnemy->PlaySound(mpEnemyDog->msIdleFoundPlayerSound);
		} else {
			if (mlPreviousState == STATE_ATTENTION)
				mpEnemy->ChangeState(STATE_IDLE);
			else
				mpEnemy->ChangeState(mlPreviousState);
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attention::OnSeePlayer(const cVector3f &avPosition, float afChance) {
}
bool cGameEnemyState_Dog_Attention::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	return false;
}
void cGameEnemyState_Dog_Attention::OnFlashlight(const cVector3f &avPosition) {
	mpEnemy->ChangeState(STATE_HUNT);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attention::OnDraw() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// EAT STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Eat::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Eating", true, 0.2f);

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyDog->mfEatFOV);

	mfTime = mpEnemy->GetTempFloat();

	mpMover->GetCharBody()->SetMoveSpeed(eCharDir_Forward, 0);
	mpMover->Stop();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Eat::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Eat::OnUpdate(float afTimeStep) {
	mfTime -= afTimeStep;
	if (mfTime <= 0) {
		mpEnemy->ChangeState(mlPreviousState); // STATE_IDLE);
	}
}

//-----------------------------------------------------------------------

bool cGameEnemyState_Dog_Eat::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	// return false;
	if (afVolume >= mpEnemyDog->mfEatMinHearVolume) {
		mpEnemy->SetTempPosition(avPosition);
		mpEnemy->ChangeState(STATE_INVESTIGATE);
		return true;
	}

	return false;
}

void cGameEnemyState_Dog_Eat::OnSeePlayer(const cVector3f &avPosition, float afChance) {
	// return;
	if (mpPlayer->GetHealth() <= 0)
		return;

	if (afChance >= mpEnemyDog->mfEatMinSeeChance) {
		mpEnemy->ChangeState(STATE_HUNT);
		mpEnemyDog->PlaySound(mpEnemyDog->msIdleFoundPlayerSound);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// INVESTIGATE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Investigate::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyDog->mfIdleFOV);

	// Play sound
	mpEnemy->PlaySound(mpEnemyDog->msInvestigateSound);

	cAINode *pNode = mpMover->GetAINodeAtPosInRange(mpEnemy->GetTempPosition(), 0.0f, 5.0f, true, 0.1f);

	if (mpEnemy->GetDoorBreakCount() > 6.0f) {
		mpEnemy->SetDoorBreakCount(0);
		mpMover->SetMaxDoorToughness(0);
	}

	if (pNode) {
		if (mpMover->MoveToPos(pNode->GetPosition()) == false) {
			mpEnemy->ChangeState(STATE_IDLE);
		}
	} else {
		mpEnemy->ChangeState(STATE_IDLE);
	}

	mpMover->SetMaxDoorToughness(-1);

	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);

	mfIdleSoundTime = cMath::RandRectf(mpEnemyDog->mfIdleSoundMinInteraval,
									   mpEnemyDog->mfIdleSoundMaxInteraval);

	if (apPrevState->GetId() != STATE_INVESTIGATE) {
		mfHighestVolume = 0.0f;
	}

	mfHearSoundCount = 5.0f;
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Investigate::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Investigate::OnUpdate(float afTimeStep) {
	if (mfHearSoundCount > 0) {
		mfHearSoundCount -= afTimeStep;
		if (mfHearSoundCount <= 0)
			mfHearSoundCount = 0;
	}

	////////////////////////////////
	// Play idle sound
	if (mfIdleSoundTime <= 0) {
		mfIdleSoundTime = cMath::RandRectf(mpEnemyDog->mfIdleSoundMinInteraval,
										   mpEnemyDog->mfIdleSoundMaxInteraval);

		mpEnemy->PlaySound(mpEnemyDog->msIdleSound);
	} else {
		mfIdleSoundTime -= afTimeStep;
	}

	////////////////////////////////
	// Stuck counter
	if (mpMover->GetStuckCounter() > 1.5f) {
		if (mlKnockCount == 1) {
			mpEnemy->ChangeState(STATE_IDLE);
			mlKnockCount = 0;
		} else {
			if (mpEnemy->CheckForDoor()) {
				mpEnemy->ChangeState(STATE_BREAKDOOR);
			}
			mpMover->ResetStuckCounter();
			mlKnockCount++;
		}
	}

	if (mpMover->IsMoving() == false) {
		mlKnockCount = 0;
		mpEnemy->ChangeState(STATE_IDLE);
	}
}

//-----------------------------------------------------------------------

bool cGameEnemyState_Dog_Investigate::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	if (mfHearSoundCount <= 0 && mfHighestVolume < afVolume &&
		afVolume >= mpEnemyDog->mfIdleMinHearVolume) {
		mfHighestVolume = afVolume;
		mpEnemy->SetTempPosition(avPosition);
		OnEnterState(this);

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// MOVE TO STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_MoveTo::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	mpEnemy->SetupBody();

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyDog->mfIdleFOV);

	// Play sound
	mpEnemy->PlaySound(mpEnemyDog->msInvestigateSound);

	if (mpMover->MoveToPos(mpEnemy->GetTempPosition()) == false) {
		// mpInit->mpEffectHandler->GetSubTitle()->Add("Could not move to pos!\n",3,true);
		mpEnemy->ChangeState(apPrevState->GetId());
		return;
	} else {
		// mpInit->mpEffectHandler->GetSubTitle()->Add("Moving to pos!\n",3,true);
	}

	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);

	mfIdleSoundTime = cMath::RandRectf(mpEnemyDog->mfIdleSoundMinInteraval,
									   mpEnemyDog->mfIdleSoundMaxInteraval);

	mlBreakCount = 0;
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_MoveTo::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_MoveTo::OnUpdate(float afTimeStep) {
	////////////////////////////////
	// Play idle sound
	if (mfIdleSoundTime <= 0) {
		mfIdleSoundTime = cMath::RandRectf(mpEnemyDog->mfIdleSoundMinInteraval,
										   mpEnemyDog->mfIdleSoundMaxInteraval);

		mpEnemy->PlaySound(mpEnemyDog->msIdleSound);
	} else {
		mfIdleSoundTime -= afTimeStep;
	}

	////////////////////////////////
	// Stuck counter
	if (mpMover->GetStuckCounter() > 1.5f) {
		if (mlBreakCount == 1) {
			mpEnemy->ChangeState(STATE_IDLE);
		} else {
			if (mpEnemy->CheckForDoor()) {
				mpEnemy->ChangeState(STATE_BREAKDOOR);
			}
			mpMover->ResetStuckCounter();
			mlBreakCount++;
		}
	}

	if (mpMover->IsMoving() == false) {
		mpEnemy->ChangeState(STATE_IDLE);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// HUNT STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Hunt::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	mpEnemy->SetupBody();

#ifndef DEMO_VERSION
	float fMul = 1.0f;

	if (mpInit->mDifficulty == eGameDifficulty_Easy)
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyDog->mfHuntSpeed * 0.7f * fMul);
	else if (mpInit->mDifficulty == eGameDifficulty_Normal)
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyDog->mfHuntSpeed * fMul);
	else
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyDog->mfHuntSpeed * 1.25f * fMul);
#else
	mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyDog->mfHuntSpeed);
#endif

	// Setup enemy
	mpEnemy->SetFOV(mpEnemyDog->mfHuntFOV);

	mfUpdatePathCount = 0;
	mfUpdateFreq = 1.0f;
	mbFreePlayerPath = false;

	if (mbBreakingDoor && mpEnemy->CanSeePlayer() == false) {
		mlBreakDoorCount++;
		if (mlBreakDoorCount >= 3) {
			mpEnemy->ChangeState(STATE_IDLE);
			return;
		}
	} else {
		mlBreakDoorCount = 0;
	}

	mbBreakingDoor = false;
	mbFoundNoPath = false;
	mbLostPlayer = false;
	mfLostPlayerCount = 0;
	mfMaxLostPlayerCount = mpEnemyDog->mfHuntForLostPlayerTime;

	mlStuckAtMaxCount = 0;

	mpInit->mpMusicHandler->AddAttacker(mpEnemy);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Hunt::OnLeaveState(iGameEnemyState *apNextState) {
	mpMover->SetMaxDoorToughness(-1);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Hunt::OnUpdate(float afTimeStep) {
	if (mpPlayer->GetHealth() <= 0) {
		mpEnemy->ChangeState(STATE_IDLE);
		return;
	}

	if (mpMover->GetStuckCounter() > 1.1f) {
		if (mpEnemy->CheckForDoor()) {
			mbBreakingDoor = true;
			mpEnemy->ChangeState(STATE_BREAKDOOR);
		} else {
			mlStuckAtMaxCount++;
			if (mlStuckAtMaxCount >= 6) {
				mpEnemy->ChangeState(STATE_IDLE);
			}
		}
		mpMover->ResetStuckCounter();
	}

	if (mfUpdatePathCount <= 0) {
		mbFoundNoPath = false;
		// mpInit->mpEffectHandler->GetSubTitle()->Add("Update Path!",1.0f,true);
		mfUpdatePathCount = mfUpdateFreq;

		/*cAINodeContainer *pNodeCont = */ mpEnemy->GetMover()->GetNodeContainer();

		// Log("%s: Checking free path\n",mpEnemy->GetName().c_str());

		// Check if there is a free path to the player
		if (mbLostPlayer == false && mpMover->FreeDirectPathToChar(mpPlayer->GetCharacterBody())) {
			mbFreePlayerPath = true;
			mpMover->Stop();
			mpMover->SetMaxDoorToughness(-1);
		} else {
			mbFreePlayerPath = false;
		}

		// Get path to player
		if (mbFreePlayerPath == false && mbLostPlayer == false) {
			if (mpEnemy->GetDoorBreakCount() > 6.0f) {
				mpMover->SetMaxDoorToughness(0);
			}

			// Log("%s: Move to pos\n",mpEnemy->GetName().c_str());

			if (mpMover->MoveToPos(mpEnemy->GetLastPlayerPos()) == false) {
				bool bFoundAnotherWay = false;
				/*float fHeight = mpMover->GetCharBody()->GetPosition().y -
								mpPlayer->GetCharacterBody()->GetPosition().y;
				if(cMath::Abs(fHeight) > mpMover->GetNodeContainer()->GetMaxHeight())
				{
					cVector3f vPos = mpEnemy->GetLastPlayerPos();
					vPos.y = mpMover->GetCharBody()->GetFeetPosition().y+0.1f;

					if(mpMover->MoveToPos(vPos))
					{
						bFoundAnotherWay = true;
					}
				}*/

				if (bFoundAnotherWay == false) {
					mfUpdatePathCount = mfUpdateFreq * 5.0f;
					mpMover->Stop();
					// Set this so the enemey at least runs toward the player.
					mbFoundNoPath = true;
				}
			}

			// Log("%s: Done with that.\n",mpEnemy->GetName().c_str());
		}
	} else {
		mfUpdatePathCount -= afTimeStep;
	}

	////////////////////////////////
	// Go directly towards the player
	if (mbFreePlayerPath || (mbFoundNoPath && mpMover->IsMoving() == false)) {
		// Go towards player
		mpMover->MoveDirectToPos(mpPlayer->GetCharacterBody()->GetFeetPosition(), afTimeStep);

		// Check if he should attack.
		if (mpMover->DistanceToChar2D(mpPlayer->GetCharacterBody()) < mpEnemyDog->mfAttackDistance) {
			float fHeight = mpMover->GetCharBody()->GetPosition().y -
							mpPlayer->GetCharacterBody()->GetPosition().y;

			// Player is above
			if (fHeight < 0) {
				fHeight += mpMover->GetCharBody()->GetSize().y / 2.0f;
				float fMax = mpEnemyDog->mvAttackDamageSize.y; /// 2.0f;
				if (fHeight > -fMax) {
					mpEnemy->ChangeState(STATE_ATTACK);
				} else {
					// random attack if player is not too far up.
					if (cMath::RandRectf(0, 1) < 0.2f) // fHeight*2 > -fMax &&
						mpEnemy->ChangeState(STATE_ATTACK);
					else
						mpEnemy->ChangeState(STATE_FLEE);
				}
			} else {
				mpEnemy->ChangeState(STATE_ATTACK);
			}
		}
	}
	////////////////////////////////
	// Update path search
	else if (mbFreePlayerPath == false) {
		if (mbLostPlayer == false && mpMover->IsMoving() == false && mpEnemy->CanSeePlayer() == false) {
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

void cGameEnemyState_Dog_Hunt::OnSeePlayer(const cVector3f &avPosition, float afChance) {
	if (mbLostPlayer && afChance >= mpEnemyDog->mfHuntMinSeeChance) {
		mbLostPlayer = false;
		mfUpdatePathCount = 0;
	}
}

//-----------------------------------------------------------------------

bool cGameEnemyState_Dog_Hunt::OnHearNoise(const cVector3f &avPosition, float afVolume) {
	//////////////////////////////////
	// If player is lost the sound might be of help
	if (mbLostPlayer) {
		// Check if sound can be heard
		if (afVolume >= mpEnemyDog->mfHuntMinHearVolume) {
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

void cGameEnemyState_Dog_Hunt::OnDraw() {
	/*float fWantedSpeed = */ mpMover->GetCharBody()->GetMoveSpeed(eCharDir_Forward);
	float fRealSpeed = cMath::Vector3Dist(mpMover->GetCharBody()->GetPosition(),
										  mpMover->GetCharBody()->GetLastPosition());
	fRealSpeed = fRealSpeed / (1.0f / 60.0f);

	float fDist = mpMover->DistanceToChar2D(mpPlayer->GetCharacterBody());

	mpInit->mpDefaultFont->draw(cVector3f(0, 110, 100), 14, cColor(1, 1, 1, 1), eFontAlign_Left,
								Common::U32String::format("LostPlayerCount: %f FreePath: %d NoPath: %d MaxStuck: %d Dist: %f / %f",
														  mfLostPlayerCount, mbFreePlayerPath,
														  mbFoundNoPath,
														  mlStuckAtMaxCount,
														  fDist,
														  mpEnemyDog->mfAttackDistance));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// ATTACK STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attack::OnEnterState(iGameEnemyState *apPrevState) {
	///////////////
	// Setup body
	mpEnemy->SetupBody();
	if (mpEnemyDog->mfAttackSpeed > 0) {
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyDog->mfAttackSpeed);
		mpMover->SetMaxTurnSpeed(10000.0f);

		// mpMover->SetMinBreakAngle(cMath::ToRad(140));
	}

	///////////////
	// Animation
	float fHeight = mpPlayer->GetCharacterBody()->GetPosition().y -
					mpMover->GetCharBody()->GetPosition().y;

	// Player is above
	if (fHeight > 0.1f)
		mpEnemy->PlayAnim("Attack", false, 0.2f);
	else
		mpEnemy->PlayAnim("AttackLow", false, 0.2f);

	///////////////
	// Other
	mpEnemyDog->PlaySound(mpEnemyDog->msAttackStartSound);

	mfDamageTimer = mpEnemyDog->mfAttackDamageTime;
	mfJumpTimer = mpEnemyDog->mfAttackJumpTime;
	mbAttacked = false;

	if (mpEnemy->GetOnAttackCallback() != "") {
		tString sCommand = mpEnemy->GetOnAttackCallback() + "(\"" + mpEnemy->GetName() + "\")";
		mpInit->RunScriptCommand(sCommand);
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attack::OnLeaveState(iGameEnemyState *apNextState) {
	mpEnemyDog->SetSkipSoundTriggerCount(2.0f);
	mpMover->ResetStuckCounter();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attack::OnUpdate(float afTimeStep) {
	// Move forward
	if (mpEnemyDog->mfAttackSpeed > 0) {
		if (mfJumpTimer <= 0) {
			mpMover->MoveDirectToPos(mpPlayer->GetCharacterBody()->GetFeetPosition(), afTimeStep);
		} else {
			mpMover->TurnToPos(mpPlayer->GetCharacterBody()->GetFeetPosition());
			mfJumpTimer -= afTimeStep;
		}
	}

	if (mbAttacked)
		return;

	//////////////////////////////////////
	// Get the 2D distance to the player
	cVector3f vStart = mpPlayer->GetCharacterBody()->GetPosition();
	vStart.y = 0;
	cVector3f vEnd = mpMover->GetCharBody()->GetPosition();
	vEnd.y = 0;
	float fDist2D = cMath::Vector3DistSqr(vStart, vEnd);
	float fMinRange = mpEnemyDog->mfAttackDamageRange;

	////////////////////////////////////////
	// Check if dog is in range of player
	if (fDist2D <= fMinRange * fMinRange && mfDamageTimer <= 0) {
		if (mbAttacked == false) {
			cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
							 mpMover->GetCharBody()->GetForward() *
								 mpEnemyDog->mfAttackDamageRange;

			cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
			cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
			mtxOffset.SetTranslation(vPos);

			eAttackTargetFlag target = eAttackTargetFlag_Player | eAttackTargetFlag_Bodies;

			mpInit->mpPlayer->mbDamageFromPos = true;
			mpInit->mpPlayer->mvDamagePos = mpMover->GetCharBody()->GetPosition();
			if (mpInit->mpAttackHandler->CreateShapeAttack(mpEnemyDog->GetAttackShape(),
														   mtxOffset,
														   mpMover->GetCharBody()->GetPosition(),
														   cMath::RandRectf(mpEnemyDog->mfAttackMinDamage,
																			mpEnemyDog->mfAttackMaxDamage),

														   mpEnemyDog->mfAttackMinMass, mpEnemyDog->mfAttackMaxMass,
														   mpEnemyDog->mfAttackMinImpulse, mpEnemyDog->mfAttackMaxImpulse,

														   mpEnemyDog->mlAttackStrength,

														   target, NULL)) {
				mpEnemyDog->PlaySound(mpEnemyDog->msAttackHitSound);
			}
			mpInit->mpPlayer->mbDamageFromPos = false;
			mbAttacked = true;
		}
	} else {
		mfDamageTimer -= afTimeStep;
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attack::OnAnimationOver(const tString &asName) {
	if (mpPlayer->GetHealth() <= 0) {
		float fDist = mpMover->DistanceToChar2D(mpInit->mpPlayer->GetCharacterBody());
		if (fDist < 2.3f) {
			mpEnemy->SetTempFloat(60.0f);
			mpEnemy->ChangeState(STATE_EAT);
		} else {
			mpEnemy->ChangeState(STATE_FLEE);
		}
	} else {
		mpEnemy->ChangeState(STATE_FLEE);
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Attack::OnPostSceneDraw() {
	cCamera3D *pCamera = static_cast<cCamera3D *>(mpInit->mpGame->GetScene()->GetCamera());

	cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
					 mpMover->GetCharBody()->GetForward() *
						 mpEnemyDog->mfAttackDamageRange;

	cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
	cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
	mtxOffset.SetTranslation(vPos);

	cMatrixf mtxCollider = cMath::MatrixMul(pCamera->GetViewMatrix(), mtxOffset);

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetMatrix(eMatrix_ModelView, mtxCollider);

	cVector3f vSize = mpEnemyDog->GetAttackShape()->GetSize();
	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(vSize * 0.5f, vSize * -0.5f,
																cColor(1, 0, 1, 1));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FLEE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Flee::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->UseMoveStateAnimations();

	// Setup body
	mpEnemy->SetupBody();
	mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyDog->mfHuntSpeed);
	mpMover->GetCharBody()->SetMaxNegativeMoveSpeed(eCharDir_Forward, -mpEnemyDog->mfFleeBackSpeed);

	float fPosMul = 1;
	if (apPrevState->GetId() == STATE_KNOCKDOWN)
		fPosMul = 4.0f;

	mbBackingFromBreakDoor = false;
	if (apPrevState->GetId() == STATE_BREAKDOOR)
		mbBackingFromBreakDoor = true;

	///////////////////////////////////////
	// The dog has just broken a door
	if (mbBackingFromBreakDoor) {
		mfBackAngle = mpMover->GetCharBody()->GetYaw();
		mbBackwards = true;

		mfTimer = mpEnemyDog->mfFleeBackTime;
		mfCheckBehindTime = 1.0f / 10.0f;
	}
	///////////////////////////////////////
	// Normal flee
	else {
		if ((apPrevState->GetId() == STATE_KNOCKDOWN || apPrevState->GetId() == STATE_HUNT ||
			 cMath::RandRectf(0, 1) < 0) // mpEnemyDog->mfFleePositionChance)
		) {
			cAINode *pNode = mpMover->GetAINodeInRange(mpEnemyDog->mfFleePositionMinDistance * fPosMul,
													   mpEnemyDog->mfFleePositionMaxDistance * fPosMul);
			if (pNode) {
				mpMover->MoveToPos(pNode->GetPosition());
			} else {
				mpEnemy->ChangeState(STATE_HUNT);
			}
			mfTimer = mpEnemyDog->mfFleePositionMaxTime;
			mbBackwards = false;
		} else if (cMath::RandRectf(0, 1) < mpEnemyDog->mfFleeBackChance) {
			mfBackAngle = mpMover->GetCharBody()->GetYaw();
			mbBackwards = true;

			mfTimer = mpEnemyDog->mfFleeBackTime;
			mfCheckBehindTime = 1.0f / 10.0f;
		} else {
			mpEnemy->ChangeState(STATE_HUNT);
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Flee::OnLeaveState(iGameEnemyState *apNextState) {
	mpMover->ResetStuckCounter();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Flee::OnUpdate(float afTimeStep) {
	mfTimer -= afTimeStep;

	if (mbBackwards) {
		if (mfCheckBehindTime <= 0) {
			mfCheckBehindTime = 1.0f / 20.0f;

			bool bHit = mpEnemy->GetGroundFinder()->GetGround(mpMover->GetCharBody()->GetPosition(),
															  mpMover->GetCharBody()->GetForward() * -1,
															  NULL, NULL, 1.9f);
			if (bHit) {
				if (mbBackingFromBreakDoor) {
					if (mlPreviousState == STATE_FLEE)
						mpEnemy->ChangeState(STATE_HUNT);
					else
						mpEnemy->ChangeState(mlPreviousState);
				} else {
					mpEnemy->ChangeState(STATE_HUNT);
				}
			}
		} else {
			mfCheckBehindTime -= afTimeStep;
		}

		if (mfTimer <= 0) {
			if (mbBackingFromBreakDoor)
				mpEnemy->ChangeState(mlPreviousState);
			else
				mpEnemy->ChangeState(STATE_HUNT);
		}
		mpMover->GetCharBody()->Move(eCharDir_Forward, -1.0f, afTimeStep);
		mpMover->TurnToPos(mpInit->mpPlayer->GetCharacterBody()->GetFeetPosition());
	} else {
		// Move forward
		if (mpMover->IsMoving() == false || mpMover->GetStuckCounter() > 0.3f || mfTimer <= 0) {
			// Check if there is any enemies nearaby and if anyone is allready fighting
			if (mpEnemy->CheckForTeamMate(mpEnemyDog->mfCallBackupRange * 1.5f, false) &&
				mpEnemy->CheckForTeamMate(8, true) == false) {
				float fPlayerDist = mpMover->DistanceToChar(mpInit->mpPlayer->GetCharacterBody());
				// Log("Dist: %f\n",fPlayerDist);
				if (fPlayerDist > 8) {
					mpEnemy->ChangeState(STATE_CALLBACKUP);
				} else {
					mpEnemy->ChangeState(STATE_HUNT);
				}
				// Log("Back from flee!!\n");
			} else {
				// if(mpInit->mpMusicHandler->AttackerExist(mpEnemy))
				{
					mpEnemy->ChangeState(STATE_HUNT);
				}
				// else
				//{
				//	mpEnemy->ChangeState(STATE_IDLE);
				// }
			}
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CALL BACKUP STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_CallBackup::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim(mpEnemyDog->msCallBackupAnimation, false, 0.2f);

	// Sound
	mpEnemyDog->PlaySound(mpEnemyDog->msCallBackupSound);

	// Iterate enemies and show them the player
	cVector3f vPostion = mpMover->GetCharBody()->GetFeetPosition();

	tGameEnemyIterator it = mpInit->mpMapHandler->GetGameEnemyIterator();
	while (it.HasNext()) {
		iGameEnemy *pEnemy = it.Next();
		if (pEnemy->GetEnemyType() != mpEnemy->GetEnemyType())
			continue;
		if (pEnemy == mpEnemy || pEnemy->IsActive() == false || pEnemy->GetHealth() <= 0)
			continue;

		cGameEnemy_Dog *pDog = static_cast<cGameEnemy_Dog *>(pEnemy);

		float fDist = cMath::Vector3Dist(pDog->GetMover()->GetCharBody()->GetPosition(),
										 vPostion);

		if (fDist <= mpEnemyDog->mfCallBackupRange) {
			pDog->ShowPlayer(mpEnemyDog->GetLastPlayerPos());
			break; // Call only for one dog backup!
		}
	}

	mpMover->Stop();
	mpMover->GetCharBody()->SetMoveSpeed(eCharDir_Forward, 0);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_CallBackup::OnLeaveState(iGameEnemyState *apNextState) {
	mpMover->ResetStuckCounter();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_CallBackup::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_CallBackup::OnAnimationOver(const tString &asName) {
	mpEnemy->ChangeState(STATE_HUNT);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// BREAK DOOR STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_BreakDoor::OnEnterState(iGameEnemyState *apPrevState) {
	// Setup body
	mpEnemy->SetupBody();
	if (mpEnemyDog->mfBreakDoorSpeed > 0)
		mpMover->GetCharBody()->SetMaxPositiveMoveSpeed(eCharDir_Forward, mpEnemyDog->mfBreakDoorSpeed);

	// Animation
	mpEnemy->PlayAnim(mpEnemyDog->msBreakDoorAnimation, false, 0.2f);

	mpEnemyDog->PlaySound(mpEnemyDog->msBreakDoorStartSound);

	mfDamageTimer = mpEnemyDog->mfBreakDoorDamageTime;
	mfStopMoveTimer = mpEnemyDog->mfBreakDoorDamageTime + 1.1f;
	mbAttacked = false;
	mbStopped = false;

	mlCount = 0;
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_BreakDoor::OnLeaveState(iGameEnemyState *apNextState) {
	mpEnemyDog->SetSkipSoundTriggerCount(2.0f);
	mpMover->ResetStuckCounter();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_BreakDoor::OnUpdate(float afTimeStep) {
	// Move forward
	if (mpEnemyDog->mfBreakDoorSpeed > 0 && mlCount == 0 && mbAttacked == false) {
		// Skip this for now
		// mpMover->GetCharBody()->Move(eCharDir_Forward,1.0f,afTimeStep);
	}

	if (mfDamageTimer <= 0) {
		if (!mbStopped) {
			mpMover->Stop();
			mbStopped = true;
		}
	} else {
		mfDamageTimer -= afTimeStep;
	}

	if (mfDamageTimer <= 0) {
		if (mbAttacked == false) {
			cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
							 mpMover->GetCharBody()->GetForward() *
								 mpEnemyDog->mfBreakDoorDamageRange;

			cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
			cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
			mtxOffset.SetTranslation(vPos);

			eAttackTargetFlag target = eAttackTargetFlag_Player | eAttackTargetFlag_Bodies;

			if (mpInit->mpAttackHandler->CreateShapeAttack(mpEnemyDog->GetBreakDoorShape(),
														   mtxOffset,
														   mpMover->GetCharBody()->GetPosition(),
														   cMath::RandRectf(mpEnemyDog->mfBreakDoorMinDamage,
																			mpEnemyDog->mfBreakDoorMaxDamage),

														   mpEnemyDog->mfBreakDoorMinMass, mpEnemyDog->mfBreakDoorMaxMass,
														   mpEnemyDog->mfBreakDoorMinImpulse, mpEnemyDog->mfBreakDoorMaxImpulse,

														   mpEnemyDog->mlBreakDoorStrength,

														   target, NULL)) {
				mpEnemyDog->PlaySound(mpEnemyDog->msBreakDoorHitSound);

				cGameSwingDoor *pDoor = mpInit->mpAttackHandler->GetLastSwingDoor();
				if (pDoor) {
// FIXME: Code identical in each branch. Development leftover?
#if 0
					/////////////////////////////
					// The door is unbreakable
					if (pDoor->GetToughness() - mpEnemyDog->mlBreakDoorStrength >= 4) {
						cMatrixf mtxDoor = pDoor->GetBody(0)->GetWorldMatrix();
						cMatrixf mtxInvDoor = cMath::MatrixInverse(mtxDoor);

						cVector3f vDoorForward = mtxInvDoor.GetForward();
						cVector3f vEnemyForward = mpMover->GetCharBody()->GetForward();

						if (cMath::Vector3Dot(vDoorForward, vEnemyForward) < 0) {
							mpEnemy->AddDoorBreakCount(2); // 8);
							// mpInit->mpEffectHandler->GetSubTitle()->Add("Cannot break this door! On BAD side",4);
						} else {
							mpEnemy->AddDoorBreakCount(2);
							// Just continue hitting it since it is just barricaded.
							// mpInit->mpEffectHandler->GetSubTitle()->Add("Cannot break this door! On GOOD side",4);
						}
					}
					/////////////////////////////
					// The door is breakable
					else {
						mpEnemy->AddDoorBreakCount(2);
					}
#else
					mpEnemy->AddDoorBreakCount(2);
#endif
				}
			}
			mbAttacked = true;
		}
	} else {
		mfDamageTimer -= afTimeStep;
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_BreakDoor::OnAnimationOver(const tString &asName) {
	if (mlCount == 0) {
		if (mpEnemyDog->mbBreakDoorRiseAtEnd) {
			mpEnemy->PlayAnim("RiseRight", false, 0.2f);
			mlCount++;
		} else {
			// mpEnemy->ChangeState(mlPreviousState);
			mpEnemy->ChangeState(STATE_FLEE);
			mpEnemy->GetState(STATE_FLEE)->SetPreviousState(mlPreviousState);
		}
	} else {
		// mpEnemy->ChangeState(mlPreviousState);
		mpEnemy->ChangeState(STATE_FLEE);
		mpEnemy->GetState(STATE_FLEE)->SetPreviousState(mlPreviousState);
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_BreakDoor::OnPostSceneDraw() {
	cCamera3D *pCamera = static_cast<cCamera3D *>(mpInit->mpGame->GetScene()->GetCamera());

	cVector3f vPos = mpMover->GetCharBody()->GetPosition() +
					 mpMover->GetCharBody()->GetForward() *
						 mpEnemyDog->mfBreakDoorDamageRange;

	cVector3f vRot = cVector3f(0, mpMover->GetCharBody()->GetYaw(), 0);
	cMatrixf mtxOffset = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
	mtxOffset.SetTranslation(vPos);

	cMatrixf mtxCollider = cMath::MatrixMul(pCamera->GetViewMatrix(), mtxOffset);

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetMatrix(eMatrix_ModelView, mtxCollider);

	cVector3f vSize = mpEnemyDog->GetBreakDoorShape()->GetSize();
	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(vSize * 0.5f, vSize * -0.5f,
																cColor(1, 0, 1, 1));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// KNOCK DOWN STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_KnockDown::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Idle", true, 0.7f);

	// Sound
	mpEnemy->PlaySound(mpEnemyDog->msKnockDownSound);

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

	mlPrevToughness = mpEnemy->GetToughness();
	mpEnemy->SetToughness(12);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_KnockDown::OnLeaveState(iGameEnemyState *apNextState) {
	mpEnemy->SetToughness(mlPrevToughness);

	mpEnemy->GetMover()->GetCharBody()->SetEntity(mpEnemy->GetMeshEntity());
	mpEnemy->GetMover()->GetCharBody()->SetActive(true);
	mpMover->ResetStuckCounter();
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_KnockDown::OnUpdate(float afTimeStep) {
	if (mbCheckAnim) {
		iCharacterBody *pCharBody = mpEnemy->GetMover()->GetCharBody();
		cBoundingVolume *pBV = pCharBody->GetBody()->GetBoundingVolume();

		////////////////////////////////////////////////
		// Add a force to all objects around dog.
		iPhysicsWorld *pWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

		Common::List<iPhysicsBody *> lstBodies;
		cPhysicsBodyIterator bodyIt = pWorld->GetBodyIterator();
		while (bodyIt.HasNext()) {
			lstBodies.push_back(bodyIt.Next());
		}

		//////////////////////////
		// Force Iteration
		Common::List<iPhysicsBody *>::iterator it = lstBodies.begin();
		for (; it != lstBodies.end(); ++it) {
			iPhysicsBody *pBody = *it;

			if (pBody->GetCollideCharacter() == false)
				continue;
			if (pBody->IsActive() == false)
				continue;
			if (pBody == pCharBody->GetBody())
				continue;

			if (cMath::CheckCollisionBV(*pBody->GetBoundingVolume(), *pBV)) {
				cVector3f vDir = pBody->GetWorldPosition() - pCharBody->GetPosition();
				float fLength = vDir.Length();
				// vDir.y *= 0.1f;
				// if(vDir.x ==0 && vDir.z ==0) vDir.x = 0.3f;
				vDir.Normalise();

				if (fLength == 0)
					fLength = 0.001f;
				float fForce = (1 / fLength) * 2;
				if (fForce > 300)
					fForce = 300;
				if (mpInit->mpPlayer->GetState() == ePlayerState_Grab &&
					mpInit->mpPlayer->GetPushBody() == pBody) {
					fForce *= 40;
				}

				if (pBody->IsCharacter()) {
					pBody->GetCharacterBody()->AddForce(vDir * fForce * 10 *
														pBody->GetCharacterBody()->GetMass());
				} else {
					pBody->AddForce(vDir * fForce * pBody->GetMass());
				}
			}
		}
	} else {
		mfTimer -= afTimeStep;

		if (mfTimer <= 0) {
			// Get the forward vector from root bone (the right vector)
			cNodeIterator StateIt = mpEnemy->GetMeshEntity()->GetRootNode()->GetChildIterator();
			cBoneState *pBoneState = static_cast<cBoneState *>(StateIt.Next());

			cVector3f vRight = cMath::MatrixInverse(pBoneState->GetWorldMatrix()).GetForward();

			// Play animation and fade physics
			float fFadeTime = 1.0f;
			mbCheckAnim = true;

			mpEnemy->GetMeshEntity()->Stop();
			if (cMath::Vector3Dot(vRight, cVector3f(0, 1, 0)) < 0)
				mpEnemy->PlayAnim("RiseRight", false, fFadeTime);
			else
				mpEnemy->PlayAnim("RiseLeft", false, fFadeTime);

			mpEnemy->GetMeshEntity()->FadeSkeletonPhysicsWeight(fFadeTime);

			// Calculate values
			cVector3f vPosition;
			cVector3f vAngles;
			/*cMatrixf mtxTransform = */ mpEnemy->GetMeshEntity()->CalculateTransformFromSkeleton(&vPosition, &vAngles);

			// Seems to work better...
			vPosition = mpEnemy->GetMeshEntity()->GetBoundingVolume()->GetWorldCenter();
			cVector3f vGroundPos = vPosition;

			/*bool bFoundGround = */ mpEnemy->GetGroundFinder()->GetGround(vPosition, cVector3f(0, -1, 0), &vGroundPos, NULL);

			// Log("Found ground: %d | %s -> %s\n",bFoundGround,vPosition.ToString().c_str(),
			//									vGroundPos.ToString().c_str());

			// Set body
			iCharacterBody *pCharBody = mpEnemy->GetMover()->GetCharBody();

			// vGroundPos -= pCharBody->GetEntityOffset().GetTranslation();
			float fYAngle = vAngles.y - mpEnemy->GetModelOffsetAngles().y;
			// cVector3f vAdd = cVector3f(0,0,pCharBody->GetEntityOffset().GetTranslation().z);
			// vAdd = cMath::MatrixMul(cMath::MatrixRotateY(fYAngle),vAdd);
			// vGroundPos += vAdd;
			pCharBody->SetFeetPosition(vGroundPos);
			pCharBody->SetYaw(fYAngle);
			pCharBody->SetEntity(mpEnemy->GetMeshEntity());
			pCharBody->SetActive(true);
			// pCharBody->Update(1.0f / 60.0f);
			// pCharBody->SetActive(false);

			for (int i = 0; i < 3; ++i) {
				pCharBody->Update(1.0f / 60.0f);

				mpEnemy->GetMeshEntity()->UpdateLogic(1.0f / 60.0f);
				mpEnemy->GetMeshEntity()->UpdateGraphics(NULL, 1.0f / 60.0f, NULL);
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_KnockDown::OnAnimationOver(const tString &asName) {
	iCharacterBody *pCharBody = mpEnemy->GetMover()->GetCharBody();

// FIXME: Code identical in each branch. Development leftover?
#if 0
	if (mpEnemy->CheckForTeamMate(mpEnemyDog->mfCallBackupRange * 1.5f, false) &&
		mpEnemy->CheckForTeamMate(14, true) == false) {
		pCharBody->SetActive(true);
		mpEnemy->ChangeState(STATE_FLEE);
	} else {
		pCharBody->SetActive(true);
		// mpEnemy->ChangeState(STATE_HUNT);
		mpEnemy->ChangeState(STATE_FLEE);
	}
#else
	pCharBody->SetActive(true);
	mpEnemy->ChangeState(STATE_FLEE);
#endif
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// DEAD STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Dead::OnEnterState(iGameEnemyState *apPrevState) {
	// Animation
	mpEnemy->PlayAnim("Idle", true, 0.7f);

	// Sound
	if (mpEnemy->IsLoading() == false)
		mpEnemy->PlaySound(mpEnemyDog->msDeathSound);

	// Setup body
	mpEnemy->SetupBody();

	// Go to ragdoll
	if (mpEnemy->IsLoading() == false)
		mpEnemy->GetMeshEntity()->AlignBodiesToSkeleton(false);

	mpEnemy->GetMeshEntity()->SetSkeletonPhysicsActive(true);
	mpEnemy->GetMeshEntity()->Stop();

	mpEnemy->GetMover()->GetCharBody()->SetEntity(NULL);
	mpEnemy->GetMover()->GetCharBody()->SetActive(false);

	mpEnemy->GetMover()->Stop();

	mpInit->mpMusicHandler->RemoveAttacker(mpEnemy);
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Dead::OnLeaveState(iGameEnemyState *apNextState) {
}

//-----------------------------------------------------------------------

void cGameEnemyState_Dog_Dead::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameEnemy_Dog::cGameEnemy_Dog(cInit *apInit, const tString &asName, TiXmlElement *apGameElem) : iGameEnemy(apInit, asName, apGameElem) {
	LoadBaseProperties(apGameElem);

	//////////////////////////////
	// Special properties

	mfLengthBodyToAss = cString::ToFloat(apGameElem->Attribute("LengthBodyToAss"), 1.5f);

	mfMinKnockDamage = cString::ToFloat(apGameElem->Attribute("MinKnockDamage"), 0);
	mfCertainKnockDamage = cString::ToFloat(apGameElem->Attribute("CertainKnockDamage"), 0);

	//////////////////////////////
	// State properties
	mfIdleFOV = cMath::ToRad(cString::ToFloat(apGameElem->Attribute("IdleFOV"), 0));
	msIdleFoundPlayerSound = cString::ToString(apGameElem->Attribute("IdleFoundPlayerSound"), "");
	mfIdleMinSeeChance = cString::ToFloat(apGameElem->Attribute("IdleMinSeeChance"), 0);
	mfIdleMinHearVolume = cString::ToFloat(apGameElem->Attribute("IdleMinHearVolume"), 0);
	msIdleSound = cString::ToString(apGameElem->Attribute("IdleSound"), "");
	mfIdleSoundMinInteraval = cString::ToFloat(apGameElem->Attribute("IdleSoundMinInteraval"), 0);
	mfIdleSoundMaxInteraval = cString::ToFloat(apGameElem->Attribute("IdleSoundMaxInteraval"), 0);
	mfIdleCallBackupChance = cString::ToFloat(apGameElem->Attribute("IdleCallBackupChance"), 0);
	mvPreloadSounds.push_back(msIdleSound);

	msInvestigateSound = cString::ToString(apGameElem->Attribute("InvestigateSound"), "");
	mvPreloadSounds.push_back(msInvestigateSound);

	msAttentionSound = cString::ToString(apGameElem->Attribute("AttentionSound"), "");
	mfAttentionTime = cString::ToFloat(apGameElem->Attribute("AttentionTime"), 0);
	mfAttentionMinDist = cString::ToFloat(apGameElem->Attribute("AttentionMinDist"), 0);
	mvPreloadSounds.push_back(msAttentionSound);

	mfHuntFOV = cMath::ToRad(cString::ToFloat(apGameElem->Attribute("HuntFOV"), 0));
	mfHuntSpeed = cString::ToFloat(apGameElem->Attribute("HuntSpeed"), 0);
	mfHuntForLostPlayerTime = cString::ToFloat(apGameElem->Attribute("HuntForLostPlayerTime"), 0);
	mfHuntMinSeeChance = cString::ToFloat(apGameElem->Attribute("IdleMinSeeChance"), 0);
	mfHuntMinHearVolume = cString::ToFloat(apGameElem->Attribute("IdleMinHearVolume"), 0);

	mfAttackDistance = cString::ToFloat(apGameElem->Attribute("AttackDistance"), 0);
	mfAttackSpeed = cString::ToFloat(apGameElem->Attribute("AttackSpeed"), 0);
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

	msBreakDoorAnimation = cString::ToString(apGameElem->Attribute("BreakDoorAnimation"), "");
	mfBreakDoorSpeed = cString::ToFloat(apGameElem->Attribute("BreakDoorSpeed"), 0);
	mfBreakDoorDamageTime = cString::ToFloat(apGameElem->Attribute("BreakDoorDamageTime"), 0);
	mvBreakDoorDamageSize = cString::ToVector3f(apGameElem->Attribute("BreakDoorDamageSize"), 0);
	mfBreakDoorDamageRange = cString::ToFloat(apGameElem->Attribute("BreakDoorDamageRange"), 0);
	mfBreakDoorMinDamage = cString::ToFloat(apGameElem->Attribute("BreakDoorMinDamage"), 0);
	mfBreakDoorMaxDamage = cString::ToFloat(apGameElem->Attribute("BreakDoorMaxDamage"), 0);
	msBreakDoorStartSound = cString::ToString(apGameElem->Attribute("BreakDoorStartSound"), "");
	msBreakDoorHitSound = cString::ToString(apGameElem->Attribute("BreakDoorHitSound"), "");
	mfBreakDoorMinMass = cString::ToFloat(apGameElem->Attribute("BreakDoorMinMass"), 0);
	mfBreakDoorMaxMass = cString::ToFloat(apGameElem->Attribute("BreakDoorMaxMass"), 0);
	mfBreakDoorMinImpulse = cString::ToFloat(apGameElem->Attribute("BreakDoorMinImpulse"), 0);
	mfBreakDoorMaxImpulse = cString::ToFloat(apGameElem->Attribute("BreakDoorMaxImpulse"), 0);
	mlBreakDoorStrength = cString::ToInt(apGameElem->Attribute("BreakDoorStrength"), 0);
	mbBreakDoorRiseAtEnd = cString::ToBool(apGameElem->Attribute("BreakDoorRiseAtEnd"), false);
	mvPreloadSounds.push_back(msBreakDoorStartSound);
	mvPreloadSounds.push_back(msBreakDoorHitSound);

	msKnockDownSound = cString::ToString(apGameElem->Attribute("KnockDownSound"), "");
	mvPreloadSounds.push_back(msKnockDownSound);

	msDeathSound = cString::ToString(apGameElem->Attribute("DeathSound"), "");
	mvPreloadSounds.push_back(msDeathSound);

	mfFleePositionChance = cString::ToFloat(apGameElem->Attribute("FleePositionChance"), 0);
	mfFleePositionMaxTime = cString::ToFloat(apGameElem->Attribute("FleePositionMaxTime"), 0);
	mfFleePositionMinDistance = cString::ToFloat(apGameElem->Attribute("FleePositionMinDistance"), 0);
	mfFleePositionMaxDistance = cString::ToFloat(apGameElem->Attribute("FleePositionMaxDistance"), 0);
	mfFleeBackChance = cString::ToFloat(apGameElem->Attribute("FleeBackChance"), 0);
	mfFleeBackTime = cString::ToFloat(apGameElem->Attribute("FleeBackTime"), 0);
	mfFleeBackSpeed = cString::ToFloat(apGameElem->Attribute("FleeBackSpeed"), 0);

	msCallBackupAnimation = cString::ToString(apGameElem->Attribute("CallBackupAnimation"), "");
	msCallBackupSound = cString::ToString(apGameElem->Attribute("CallBackupSound"), "");
	mfCallBackupRange = cString::ToFloat(apGameElem->Attribute("CallBackupRange"), 0);
	mvPreloadSounds.push_back(msCallBackupSound);

	mfEatFOV = cMath::ToRad(cString::ToFloat(apGameElem->Attribute("EatFOV"), 0));
	mfEatMinSeeChance = cString::ToFloat(apGameElem->Attribute("EatMinSeeChance"), 0);
	mfEatMinHearVolume = cString::ToFloat(apGameElem->Attribute("EatMinHearVolume"), 0);

	//////////////////////////////
	// Set up states
	AddState(hplNew(cGameEnemyState_Dog_Idle, (STATE_IDLE, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Hunt, (STATE_HUNT, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Attack, (STATE_ATTACK, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Flee, (STATE_FLEE, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_KnockDown, (STATE_KNOCKDOWN, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Dead, (STATE_DEAD, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Patrol, (STATE_PATROL, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Investigate, (STATE_INVESTIGATE, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_BreakDoor, (STATE_BREAKDOOR, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_CallBackup, (STATE_CALLBACKUP, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_MoveTo, (STATE_MOVETO, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Eat, (STATE_EAT, mpInit, this)));
	AddState(hplNew(cGameEnemyState_Dog_Attention, (STATE_ATTENTION, mpInit, this)));
}

//-----------------------------------------------------------------------

cGameEnemy_Dog::~cGameEnemy_Dog() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameEnemy_Dog::OnLoad() {
	// Create attack shape
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	mpAttackShape = pPhysicsWorld->CreateBoxShape(mvAttackDamageSize, NULL);
	mpBreakDoorShape = pPhysicsWorld->CreateBoxShape(mvBreakDoorDamageSize, NULL);

	// Set up shape
	ChangeState(STATE_IDLE);
}

//-----------------------------------------------------------------------

void cGameEnemy_Dog::OnUpdate(float afTimeStep) {
	if (IsActive() == false)
		return;

	///////////////////////////////////
	// Regenerate health:
	if (mfHealth > 0) {
		if (mpInit->mDifficulty != eGameDifficulty_Easy &&
			mfHealth <= mfMaxHealth * 0.5f) {
			mfHealth += afTimeStep * (10.0f / 60.0f); // 10 heal units / min
		}
	}

	///////////////////////////////////
	// Check for ass in wall
	if (mfHealth > 0 && mpMover->GetCharBody()->IsActive()) {
		float fMaxMove = afTimeStep * 2;
		static int lAssCount = 0;
		lAssCount++;
		if (lAssCount % 2 == 0) {
			iCharacterBody *pCharBody = mpMover->GetCharBody();

			cVector3f vPos, vNormal;
			mFindGround.GetGround(pCharBody->GetPosition(), pCharBody->GetForward() * -1,
								  &vPos, &vNormal, mfLengthBodyToAss);
			float fDist = cMath::Vector3Dist(pCharBody->GetPosition(), vPos);
			if (fDist < mfLengthBodyToAss) {
				float fAdd = mfLengthBodyToAss - fDist;
				if (fAdd > fMaxMove)
					fAdd = fMaxMove;

				cVector3f vAdd = pCharBody->GetForward() * fAdd;
				pCharBody->SetPosition(pCharBody->GetPosition() + vAdd);
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameEnemy_Dog::ShowPlayer(const cVector3f &avPlayerFeetPos) {
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL ||
		mlCurrentState == STATE_INVESTIGATE) {
		mvLastPlayerPos = avPlayerFeetPos;
		ChangeState(STATE_HUNT);
	}
}

//-----------------------------------------------------------------------

bool cGameEnemy_Dog::MoveToPos(const cVector3f &avFeetPos) {
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL) {
		SetTempPosition(avFeetPos);
		ChangeState(STATE_MOVETO);
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------

bool cGameEnemy_Dog::IsFighting() {
	if (mfHealth <= 0 || IsActive() == false)
		return false;
	if (mlCurrentState == STATE_IDLE || mlCurrentState == STATE_PATROL ||
		mlCurrentState == STATE_INVESTIGATE)
		return false;

	return true;
}

//-----------------------------------------------------------------------
