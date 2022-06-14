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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hpl1/engine/ai/StateMachine.h"

#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iAIState::iAIState() {
	mfTimeCount = 0;
}

//-----------------------------------------------------------------------

void iAIState::Sleep(float afTime) {
}

//-----------------------------------------------------------------------

void iAIState::Update(float afTime) {
	// Update according to the time step
	mfTimeCount += afTime;
	while (mfTimeCount >= mfUpdateStep) {
		OnUpdate(mfUpdateStep);
		mfTimeCount -= mfUpdateStep;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cStateMachine::cStateMachine() {
	mbActive = true;

	mpCurrentState = NULL;
}

//-----------------------------------------------------------------------

cStateMachine::~cStateMachine() {
	STLMapDeleteAll(m_mapStates);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cStateMachine::Update(float afTime) {
	if (mbActive == false || mpCurrentState == NULL)
		return;

	mpCurrentState->Update(afTime);
}

//-----------------------------------------------------------------------

void cStateMachine::AddState(iAIState *apState, const tString &asName, int alId, float afUpdateStep) {
	apState->SetStateMachine(this);
	apState->mlId = alId;
	apState->msName = asName;
	apState->mfUpdateStep = afUpdateStep;

	if (m_mapStates.empty())
		mpCurrentState = apState;

	m_mapStates.insert(tAIStateMap::value_type(alId, apState));
}

//-----------------------------------------------------------------------

void cStateMachine::ChangeState(int alId) {
	if (alId == mpCurrentState->GetId())
		return;

	iAIState *pState = GetState(alId);
	if (pState == NULL) {
		Warning("State %d does not exist!\n", alId);
		return;
	}

	if (mpCurrentState)
		mpCurrentState->OnLeaveState(pState->GetId());
	pState->OnEnterState(mpCurrentState == NULL ? -1 : mpCurrentState->GetId());

	mpCurrentState = pState;
}

//-----------------------------------------------------------------------

iAIState *cStateMachine::GetState(int alId) {
	tAIStateMapIt it = m_mapStates.find(alId);
	if (it == m_mapStates.end())
		return NULL;

	return it->second;
}
//-----------------------------------------------------------------------

iAIState *cStateMachine::CurrentState() {
	return mpCurrentState;
}

//-----------------------------------------------------------------------
} // namespace hpl
