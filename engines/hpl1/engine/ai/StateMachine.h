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
#ifndef HPL_STATE_MACHINE_H
#define HPL_STATE_MACHINE_H

#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/engine/game/GameTypes.h"

#include <map>

namespace hpl {

	//-----------------------------------------
	class cStateMachine;

	class iAIState
	{
	friend class cStateMachine;
	public:
		iAIState();
		virtual ~iAIState(){}

		virtual void OnUpdate(float afTime)=0;

		virtual void OnEnterState(int alLastState)=0;
		virtual void OnLeaveState(int alNextState)=0;

		int GetId(){ return mlId;}
		const tString& GetName(){ return msName;}
		float GetUpdateStep(){ return mfUpdateStep;}

		void Sleep(float afTime);

	protected:
		int mlId;
		tString msName;
		float mfUpdateStep;
		cStateMachine *mpStateMachine;

	private:
		void Update(float afTime);
		void SetStateMachine(cStateMachine *apStateMachine){ mpStateMachine = apStateMachine;}

		float mfTimeCount;
	};

	typedef std::map<int, iAIState*> tAIStateMap;
	typedef tAIStateMap::iterator tAIStateMapIt;

	//-----------------------------------------

	class cStateMachine
	{
	public:
		cStateMachine();
		virtual ~cStateMachine();

		void Update(float afTime);

		/**
		 * Adds a new state to the state machine. The state machine will destroy them when deleted.
		 */
		void AddState(iAIState *apState, const tString& asName, int alId, float afUpdateStep);

		void ChangeState(int alId);

		void SetActive(bool abX){ mbActive = abX;}
		bool IsActive(){ return mbActive;}

		iAIState* GetState(int alId);

		iAIState* CurrentState();

	private:
		bool mbActive;

		tAIStateMap m_mapStates;

		iAIState *mpCurrentState;
	};

};
#endif // HPL_STATE_MACHINE_H
