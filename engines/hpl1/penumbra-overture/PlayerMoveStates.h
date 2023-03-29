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

#ifndef GAME_PLAYER_MOVE_STATES_H
#define GAME_PLAYER_MOVE_STATES_H

class cPlayer;

//////////////////////////////////////////////////////////////////////////
// BASE STATE
//////////////////////////////////////////////////////////////////////////

iPlayerMoveState::iPlayerMoveState(cPlayer *apPlayer, cInit *apInit) {
	mpPlayer = apPlayer;
	mpInit = apInit;

	mpGameConfig = mpInit->mpGameConfig;
	mpHeadMove = mpPlayer->GetHeadMove();

	// Speed setup
	mfForwardSpeed = 3.0f;
	mfBackwardSpeed = 1.5f;
	mfSidewaySpeed = 2.0f;

	mfForwardAcc = 6.0f;
	mfForwardDeacc = 12.0f;
	mfSidewayAcc = 6.0f;
	mfSidewayDeacc = 12.0f;

	mbActive = false;

	// Head move setup
	mfMaxHeadMove = 0.05f;
	mfMinHeadMove = -0.06f;
	mfHeadMoveSpeed = 0.38f;
	mfHeadMoveBackSpeed = 0.23f;

	mfHeightAdd = 0;
	mfHeightAddSpeed = 1.5f;

	mfSpeedMul = 1;

	// FootStep multiplier
	mfFootStepMul = 1.0f;

	msStepType = "walk";
}

void iPlayerMoveState::SetupBody() {
	iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
	float fMul = mpPlayer->GetSpeedMul() * mpPlayer->GetHealthSpeedMul();

	pCharBody->SetMaxPositiveMoveSpeed(eCharDir_Forward, mfForwardSpeed * fMul);
	pCharBody->SetMaxNegativeMoveSpeed(eCharDir_Forward, -mfBackwardSpeed * fMul);

	pCharBody->SetMaxPositiveMoveSpeed(eCharDir_Right, mfSidewaySpeed * fMul);
	pCharBody->SetMaxNegativeMoveSpeed(eCharDir_Right, -mfSidewaySpeed * fMul);

	pCharBody->SetMoveAcc(eCharDir_Forward, mfForwardAcc);
	pCharBody->SetMoveDeacc(eCharDir_Forward, mfForwardDeacc);
	pCharBody->SetMoveAcc(eCharDir_Right, mfSidewayAcc);
	pCharBody->SetMoveDeacc(eCharDir_Right, mfSidewayDeacc);
}

void iPlayerMoveState::InitState(iPlayerMoveState *apPrevState) {
	if (apPrevState) {
		apPrevState->LeaveState(apPrevState);
		apPrevState->mbActive = false;
	}

	EnterState(apPrevState);

	// Set up body
	SetupBody();

	mpHeadMove->mfMaxHeadMove = mfMaxHeadMove;
	mpHeadMove->mfMinHeadMove = mfMinHeadMove;
	mpHeadMove->mfHeadMoveSpeed = mfHeadMoveSpeed;
	mpHeadMove->mfHeadMoveBackSpeed = mfHeadMoveBackSpeed;
	mpHeadMove->mfFootStepMul = mfFootStepMul;
}

void iPlayerMoveState::Start() {
	if (mbActive == false) {
		mbActive = true;

		// Set up body
		iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
		float fMul = mpPlayer->GetSpeedMul();

		pCharBody->SetMaxPositiveMoveSpeed(eCharDir_Forward, mfForwardSpeed * fMul);
		pCharBody->SetMaxNegativeMoveSpeed(eCharDir_Forward, -mfBackwardSpeed * fMul);

		pCharBody->SetMaxPositiveMoveSpeed(eCharDir_Right, mfSidewaySpeed * fMul);
		pCharBody->SetMaxNegativeMoveSpeed(eCharDir_Right, -mfSidewaySpeed * fMul);

		pCharBody->SetMoveAcc(eCharDir_Forward, mfForwardAcc);
		pCharBody->SetMoveDeacc(eCharDir_Forward, mfForwardDeacc);
		pCharBody->SetMoveAcc(eCharDir_Right, mfSidewayAcc);
		pCharBody->SetMoveDeacc(eCharDir_Right, mfSidewayDeacc);

		mpHeadMove->mfMaxHeadMove = mfMaxHeadMove;
		mpHeadMove->mfMinHeadMove = mfMinHeadMove;
		mpHeadMove->mfHeadMoveSpeed = mfHeadMoveSpeed;
		mpHeadMove->mfHeadMoveBackSpeed = mfHeadMoveBackSpeed;
		mpHeadMove->mfFootStepMul = mfFootStepMul;

		mpHeadMove->Start();
	}
}

void iPlayerMoveState::Stop() {
	mpHeadMove->Stop();

	mbActive = false;
}

void iPlayerMoveState::Update(float afTimeStep) {
	// Update height add
	float fPlayerHeightAdd = mpPlayer->GetHeightAdd();
	if (fPlayerHeightAdd < mfHeightAdd) {
		fPlayerHeightAdd += mfHeightAddSpeed * afTimeStep;
		if (fPlayerHeightAdd > mfHeightAdd)
			fPlayerHeightAdd = mfHeightAdd;
	}

	if (fPlayerHeightAdd > mfHeightAdd) {
		fPlayerHeightAdd -= mfHeightAddSpeed * afTimeStep;
		if (fPlayerHeightAdd < mfHeightAdd)
			fPlayerHeightAdd = mfHeightAdd;
	}
	mpPlayer->SetHeightAdd(fPlayerHeightAdd);

	OnUpdate(afTimeStep);
}

//////////////////////////////////////////////////////////////////////////
// WALK STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerMoveState_Walk : public iPlayerMoveState {
public:
	cPlayerMoveState_Walk(cPlayer *apPlayer, cInit *apInit) : iPlayerMoveState(apPlayer, apInit) {
		mfForwardSpeed = mpGameConfig->GetFloat("Movement_Walk", "ForwardSpeed", 0);
		mfBackwardSpeed = mpGameConfig->GetFloat("Movement_Walk", "BackwardSpeed", 0);
		mfSidewaySpeed = mpGameConfig->GetFloat("Movement_Walk", "SidewaySpeed", 0);

		mfForwardAcc = mpGameConfig->GetFloat("Movement_Walk", "ForwardAcc", 0);
		mfForwardDeacc = mpGameConfig->GetFloat("Movement_Walk", "ForwardDeacc", 0);
		mfSidewayAcc = mpGameConfig->GetFloat("Movement_Walk", "SidewayAcc", 0);
		mfSidewayDeacc = mpGameConfig->GetFloat("Movement_Walk", "SidewayDeacc", 0);

		// Head move setup
		mfMaxHeadMove = mpGameConfig->GetFloat("Movement_Walk", "MaxHeadMove", 0);
		mfMinHeadMove = mpGameConfig->GetFloat("Movement_Walk", "MinHeadMove", 0);
		mfHeadMoveSpeed = mpGameConfig->GetFloat("Movement_Walk", "HeadMoveSpeed", 0);
		mfHeadMoveBackSpeed = 0.23f;

		// FootStep multiplier
		mfFootStepMul = 1.0f;

		msStepType = "walk";

		mType = ePlayerMoveState_Walk;
	}
};

//////////////////////////////////////////////////////////////////////////
// RUN STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerMoveState_Run : public iPlayerMoveState {
public:
	cPlayerMoveState_Run(cPlayer *apPlayer, cInit *apInit) : iPlayerMoveState(apPlayer, apInit) {
		mfForwardSpeed = mpGameConfig->GetFloat("Movement_Run", "ForwardSpeed", 0);
		mfBackwardSpeed = mpGameConfig->GetFloat("Movement_Run", "BackwardSpeed", 0);
		mfSidewaySpeed = mpGameConfig->GetFloat("Movement_Run", "SidewaySpeed", 0);

		mfForwardAcc = mpGameConfig->GetFloat("Movement_Run", "ForwardAcc", 0);
		mfForwardDeacc = mpGameConfig->GetFloat("Movement_Run", "ForwardDeacc", 0);
		mfSidewayAcc = mpGameConfig->GetFloat("Movement_Run", "SidewayAcc", 0);
		mfSidewayDeacc = mpGameConfig->GetFloat("Movement_Run", "SidewayDeacc", 0);

		// Head move setup
		mfMaxHeadMove = mpGameConfig->GetFloat("Movement_Run", "MaxHeadMove", 0);
		mfMinHeadMove = mpGameConfig->GetFloat("Movement_Run", "MinHeadMove", 0);
		mfHeadMoveSpeed = mpGameConfig->GetFloat("Movement_Run", "HeadMoveSpeed", 0);
		mfHeadMoveBackSpeed = 0.23f;

		// FootStep multiplier
		mfFootStepMul = 1.0f;

		msStepType = "run";

		mType = ePlayerMoveState_Run;
	}
};

//////////////////////////////////////////////////////////////////////////
// STILL STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerMoveState_Still : public iPlayerMoveState {
public:
	cPlayerMoveState_Still(cPlayer *apPlayer, cInit *apInit) : iPlayerMoveState(apPlayer, apInit) {
		mfForwardSpeed = 0.0f;
		mfBackwardSpeed = 0.0f;
		mfSidewaySpeed = 0.0f;

		mfForwardAcc = 0.0f;
		mfForwardDeacc = 6.0f;
		mfSidewayAcc = 0.0f;
		mfSidewayDeacc = 6.0f;

		// Head move setup
		mfMaxHeadMove = 0.03f;
		mfMinHeadMove = -0.03f;
		mfHeadMoveSpeed = 0.12f;
		mfHeadMoveBackSpeed = 0.23f;

		// FootStep multiplier
		mfFootStepMul = 0.6f;

		msStepType = "walk";

		mType = ePlayerMoveState_Still;
	}

	void EnterState(iPlayerMoveState *apPrevState) {
		if (apPrevState) {
			mfHeightAdd = apPrevState->mfHeightAdd;
			mfHeightAddSpeed = apPrevState->mfHeightAddSpeed;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// JUMP STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerMoveState_Jump : public iPlayerMoveState {
public:
	bool mbFirstUpdate;
	float mfStartForce;
	float mfMaxForward;
	float mfMaxSide;

	float mfDefaultFowardSpeed;
	float mfDefaultSidewaySpeed;

	cPlayerMoveState_Jump(cPlayer *apPlayer, cInit *apInit) : iPlayerMoveState(apPlayer, apInit) {
		mfForwardSpeed = mpGameConfig->GetFloat("Movement_Jump", "ForwardSpeed", 0);
		mfDefaultFowardSpeed = mfForwardSpeed;
		mfBackwardSpeed = mpGameConfig->GetFloat("Movement_Jump", "BackwardSpeed", 0);
		mfSidewaySpeed = mpGameConfig->GetFloat("Movement_Jump", "SidewaySpeed", 0);
		mfDefaultSidewaySpeed = mfSidewaySpeed;

		mfForwardAcc = mpGameConfig->GetFloat("Movement_Jump", "ForwardAcc", 0);
		mfForwardDeacc = mpGameConfig->GetFloat("Movement_Jump", "ForwardDeacc", 0);
		mfSidewayAcc = mpGameConfig->GetFloat("Movement_Jump", "SidewayAcc", 0);
		mfSidewayDeacc = mpGameConfig->GetFloat("Movement_Jump", "SidewayDeacc", 0);

		// Head move setup
		mfMaxHeadMove = 0.00f;
		mfMinHeadMove = 0.00f;
		mfHeadMoveSpeed = 0.0f;
		mfHeadMoveBackSpeed = 0.33f;

		// mfHeightAdd =-0.3f;
		// mfHeightAddSpeed = 1.2f;

		// FootStep multiplier
		mfFootStepMul = 0.0f;

		msStepType = "run";

		mfStartForce = mpInit->mpGameConfig->GetFloat("Player", "JumpStartForce", 1);

		mType = ePlayerMoveState_Jump;
	}

	void EnterState(iPlayerMoveState *apPrevState) {
		iCharacterBody *pBody = mpPlayer->GetCharacterBody();

		if (apPrevState)
			mPrevMoveState = apPrevState->mType;
		else
			mPrevMoveState = ePlayerMoveState_Walk;

		float fForce = mfStartForce;
		if (mPrevMoveState == ePlayerMoveState_Crouch) {
			fForce *= 0.5f;
			mfForwardSpeed = mfDefaultFowardSpeed * 0.6f;
			mfSidewaySpeed = mfDefaultSidewaySpeed * 0.6f;
		} else {
			mfForwardSpeed = mfDefaultFowardSpeed;
			mfSidewaySpeed = mfDefaultSidewaySpeed;
		}

		// Set the current move speed as force speed.
		cVector3f vVel = 0;
		cVector3f vForward = pBody->GetMoveMatrix().GetForward() * -1.0f;
		cVector3f vRight = pBody->GetMoveMatrix().GetRight();

		vVel += vForward * pBody->GetMoveSpeed(eCharDir_Forward);
		vVel += vRight * pBody->GetMoveSpeed(eCharDir_Right);

		pBody->AddForceVelocity(vVel);

		pBody->SetMoveSpeed(eCharDir_Forward, 0);
		pBody->SetMoveSpeed(eCharDir_Right, 0);

		// Add jump force
		pBody->AddForce(cVector3f(0, fForce * mpPlayer->GetDefaultMass(), 0));

		mbFirstUpdate = true;

		// Get the maximum speed allowed.
		if (mPrevMoveState == ePlayerMoveState_Still || mPrevMoveState == ePlayerMoveState_Walk) {
			mfMaxForward = mpPlayer->GetMoveStateData(ePlayerMoveState_Walk)->mfForwardSpeed;
			mfMaxSide = mpPlayer->GetMoveStateData(ePlayerMoveState_Walk)->mfSidewaySpeed;
		} else {
			mfMaxForward = apPrevState->mfForwardSpeed;
			mfMaxSide = apPrevState->mfSidewaySpeed;
		}
	}

	void LeaveState(iPlayerMoveState *apNextState) {
		iCharacterBody *pBody = mpPlayer->GetCharacterBody();

		pBody->AddForceVelocity(pBody->GetForceVelocity() * -0.7f);
	}

	void OnUpdate(float afTimeStep) {
		iCharacterBody *pBody = mpPlayer->GetCharacterBody();

		if (mbFirstUpdate) {
			mbFirstUpdate = false;
			return;
		}

		// Check so that speed is correct
		cVector3f vForceVel = pBody->GetForceVelocity();
		float fSpeed = vForceVel.Length();

		if (fSpeed > mfMaxForward) {
			// float fNeg = fSpeed - mfMaxForward;

			float fForwardSpeed = pBody->GetMoveSpeed(eCharDir_Forward);
			/*if(fForwardSpeed>0){
				fForwardSpeed -= fNeg;
				if(fForwardSpeed<0)fForwardSpeed =0;
			}
			else {
				fForwardSpeed += fNeg;
				if(fForwardSpeed>0)fForwardSpeed =0;
			}*/
			pBody->SetMoveSpeed(eCharDir_Forward, fForwardSpeed);
		}

		// Skip sideways here, it is sucha strange way to jump anyway.

		// Check if the jumpbutton is down.
		if (mpPlayer->GetJumpButtonDown() &&
			mpPlayer->GetJumpCount() < mpPlayer->GetMaxJumpCount()) {
			float fMul = 0.4f + 0.5f * (1 - mpPlayer->GetJumpCount() / mpPlayer->GetMaxJumpCount());

			pBody->AddForce(cVector3f(0, -pBody->GetCustomGravity().y * pBody->GetMass() * fMul, 0));
		} else if (pBody->GetForceVelocity().y > 0) {
			// Add some extra gravity
			pBody->AddForce(cVector3f(0, -20.0f * pBody->GetMass(), 0));
		}

		// check if the body is on ground, and if so end jump.
		if (pBody->IsOnGround() && pBody->GetForceVelocity().y == 0) {
			mpPlayer->ChangeMoveState(mPrevMoveState);
		} else if (mpPlayer->GetLandedFromJump()) {
			mpPlayer->SetLandedFromJump(false);
			mpPlayer->ChangeMoveState(mPrevMoveState);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// CROUCH STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerMoveState_Crouch : public iPlayerMoveState {
public:
	bool mbFirstUpdate;

	cPlayerMoveState_Crouch(cPlayer *apPlayer, cInit *apInit) : iPlayerMoveState(apPlayer, apInit) {
		mfForwardSpeed = mpGameConfig->GetFloat("Movement_Crouch", "ForwardSpeed", 0);
		mfBackwardSpeed = mpGameConfig->GetFloat("Movement_Crouch", "BackwardSpeed", 0);
		mfSidewaySpeed = mpGameConfig->GetFloat("Movement_Crouch", "SidewaySpeed", 0);

		mfForwardAcc = mpGameConfig->GetFloat("Movement_Crouch", "ForwardAcc", 0);
		mfForwardDeacc = mpGameConfig->GetFloat("Movement_Crouch", "ForwardDeacc", 0);
		mfSidewayAcc = mpGameConfig->GetFloat("Movement_Crouch", "SidewayAcc", 0);
		mfSidewayDeacc = mpGameConfig->GetFloat("Movement_Crouch", "SidewayDeacc", 0);

		// Head move setup
		mfMaxHeadMove = mpGameConfig->GetFloat("Movement_Crouch", "MaxHeadMove", 0);
		mfMinHeadMove = mpGameConfig->GetFloat("Movement_Crouch", "MinHeadMove", 0);
		mfHeadMoveSpeed = mpGameConfig->GetFloat("Movement_Crouch", "HeadMoveSpeed", 0);
		mfHeadMoveBackSpeed = 0.23f;

		mfHeightAdd = -(mpPlayer->GetSize().y - mpPlayer->GetCrouchHeight());
		mfHeightAddSpeed = 1.8f;

		// FootStep multiplier
		mfFootStepMul = 1.0f;

		msStepType = "sneak";

		mType = ePlayerMoveState_Crouch;
	}

	void EnterState(iPlayerMoveState *apPrevState) {
		iCharacterBody *pBody = mpPlayer->GetCharacterBody();

		cVector3f vFeetPos = pBody->GetPosition() - cVector3f(0, pBody->GetShape()->GetSize().y / 2, 0);
		pBody->SetActiveSize(1);
		pBody->SetPosition(vFeetPos + cVector3f(0, pBody->GetShape()->GetSize().y / 2, 0));
		// pBody->SetGravityActive(false);
	}

	void LeaveState(iPlayerMoveState *apNextState) {
		iCharacterBody *pBody = mpPlayer->GetCharacterBody();

		cVector3f vFeetPos = pBody->GetPosition() - cVector3f(0, pBody->GetShape()->GetSize().y / 2, 0);
		pBody->SetActiveSize(0);
		pBody->SetPosition(vFeetPos + cVector3f(0, pBody->GetShape()->GetSize().y / 2, 0));

		/////////////////////////////////////////////////
		// Check if the player will fit with the newer size
		cInit *pInit = mpPlayer->GetInit();
		iPhysicsWorld *pWorld = pInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

		pBody->SetPosition(pBody->GetPosition() + cVector3f(0, 0.005f, 0));

		// Check with both bodies. This removes some bugs.
		for (int i = 0; i < 2; ++i) {
			iCollideShape *pShape = pBody->GetExtraBody(i)->GetShape();

			cVector3f vNewPos = pBody->GetPosition();
			/*bool bCollide = */ pWorld->CheckShapeWorldCollision(&vNewPos, pShape,
																  cMath::MatrixTranslate(pBody->GetPosition()),
																  pBody->GetBody(), false, true);

			/*Log("Collide when leaving crouch: %d. NewPos: %s OldPos: %s\n",bCollide,
														vNewPos.ToString().c_str(),
														pBody->GetPosition().ToString().c_str());*/

			// If the body is pushed down, then something is colliding from above.
			// if so, set crouch mode again.
			if (vNewPos != pBody->GetPosition()) {
				pBody->SetPosition(pBody->GetPosition() - cVector3f(0, 0.005f, 0));
				mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
				return;
			}
		}

		pBody->SetPosition(pBody->GetPosition() - cVector3f(0, 0.005f, 0));
	}

	void OnUpdate(float afTimeStep) {
	}
};

#endif // GAME_PLAYER_MOVE_STATES_H
