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
#include "hpl1/engine/scene/AnimationState.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/graphics/Animation.h"
#include "hpl1/engine/resources/AnimationManager.h"

#include "hpl1/engine/system/LowLevelSystem.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cAnimationState::cAnimationState(cAnimation* apAnimation, const tString &asName,
									cAnimationManager *apAnimationManager)
	{
		mpAnimation = apAnimation;

		mpAnimationManager = apAnimationManager;

		mfLength = mpAnimation->GetLength();
		msName = asName;

		mbActive = false;

		mfTimePos = 0;
		mfWeight = 1;
		mfSpeed = 1.0f;
		mfBaseSpeed = 1.0f;
		mfTimePos = 0;
		mfPrevTimePos=0;

		mbLoop =false;
		mbPaused = false;

		mfSpecialEventTime =0;

		mfFadeStep=0;
	}

	//-----------------------------------------------------------------------

	cAnimationState::~cAnimationState()
	{
		STLDeleteAll(mvEvents);

		if(mpAnimationManager)
			mpAnimationManager->Destroy(mpAnimation);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cAnimationState::Update(float afTimeStep)
	{
		//Update animation
		AddTimePosition(afTimeStep);

		//Fading
		if(mfFadeStep!=0)
		{
			mfWeight += mfFadeStep*afTimeStep;

			if(mfFadeStep<0 && mfWeight<=0)
			{
				mfWeight =0;
				mbActive = false;
				mfFadeStep =0;
			}
			else if(mfFadeStep>0 && mfWeight>=1)
			{
				mfWeight =1;
				mfFadeStep =0;
			}
		}
	}

	//-----------------------------------------------------------------------

	bool cAnimationState::IsFading()
	{
		return mfFadeStep!=0;
	}


	//-----------------------------------------------------------------------

	bool cAnimationState::IsOver()
	{
		if(mbLoop) return false;

		return mfTimePos >= mfLength;
	}

	//-----------------------------------------------------------------------

	void cAnimationState::FadeIn(float afTime)
	{
		mfFadeStep = 1.0f / std::abs(afTime);
	}

	void cAnimationState::FadeOut(float afTime)
	{
		mfFadeStep = -1.0f / std::abs(afTime);
	}

	//-----------------------------------------------------------------------

	void cAnimationState::SetLength(float afLength)
	{
		mfLength = afLength;
	}

	float cAnimationState::GetLength()
	{
		return mfLength;
	}

	//-----------------------------------------------------------------------


	void cAnimationState::SetWeight(float afWeight)
	{
		mfWeight = afWeight;
	}
	float cAnimationState::GetWeight()
	{
		return mfWeight;
	}

	//-----------------------------------------------------------------------

	void cAnimationState::SetSpeed(float afSpeed)
	{
		mfSpeed = afSpeed;
	}
	float cAnimationState::GetSpeed()
	{
		return mfSpeed;
	}

	//-----------------------------------------------------------------------

	void cAnimationState::SetBaseSpeed(float afSpeed)
	{
		mfBaseSpeed = afSpeed;
	}
	float cAnimationState::GetBaseSpeed()
	{
		return mfBaseSpeed;
	}

	//-----------------------------------------------------------------------

	void cAnimationState::SetTimePosition(float afPosition)
	{
		if(mbLoop)
		{
			mfTimePos = cMath::Wrap(afPosition,0,mfLength);
		}
		else
		{
			mfTimePos = cMath::Clamp(afPosition, 0, mfLength);
		}
	}

	float cAnimationState::GetTimePosition()
	{
		return mfTimePos;
	}

	float cAnimationState::GetPreviousTimePosition()
	{
		return mfPrevTimePos;
	}

	//-----------------------------------------------------------------------

	void cAnimationState::SetRelativeTimePosition(float afPosition)
	{
		SetTimePosition(afPosition * mfLength);
	}

	float cAnimationState::GetRelativeTimePosition()
	{
		return mfTimePos / mfLength;
	}

	//-----------------------------------------------------------------------

	bool cAnimationState::IsActive()
	{
		return mbActive;
	}
	void cAnimationState::SetActive(bool abActive)
	{
		if(mbActive == abActive) return;

		mbActive = abActive;

		//Should this really be here?
		mbPaused = false;
		mfFadeStep =0;
	}

	//-----------------------------------------------------------------------

	bool cAnimationState::IsLooping()
	{
		return mbLoop;
	}
	void cAnimationState::SetLoop(bool abLoop)
	{
		mbLoop = abLoop;
	}

	//-----------------------------------------------------------------------

	bool cAnimationState::IsPaused()
	{
		return mbPaused;
	}

	void cAnimationState::SetPaused(bool abPaused)
	{
		mbPaused = abPaused;
	}

	//-----------------------------------------------------------------------

	bool cAnimationState::IsAfterSpecialEvent()
	{
		return mfTimePos > mfSpecialEventTime;
	}

	bool cAnimationState::IsBeforeSpecialEvent()
	{
		return mfTimePos < mfSpecialEventTime;
	}

	//-----------------------------------------------------------------------

	void cAnimationState::AddTimePosition(float afAdd)
	{
		if(mbPaused) return;

		mfPrevTimePos = mfTimePos;

		mfTimePos += afAdd*mfSpeed*mfBaseSpeed;

		SetTimePosition(mfTimePos);
	}

	//-----------------------------------------------------------------------

	cAnimation* cAnimationState::GetAnimation()
	{
		return mpAnimation;
	}

	//-----------------------------------------------------------------------

	cAnimationEvent *cAnimationState::CreateEvent()
	{
		cAnimationEvent *pEvent = hplNew( cAnimationEvent, () );
		pEvent->mfTime =0;
		pEvent->mType = eAnimationEventType_LastEnum;
		pEvent->msValue = "";
		mvEvents.push_back(pEvent);

		return pEvent;
	}

	cAnimationEvent *cAnimationState::GetEvent(int alIdx)
	{
		return mvEvents[alIdx];
	}

	int cAnimationState::GetEventNum()
	{
		return (int)mvEvents.size();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// SAVE OBJECT STUFF
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cSaveData_cAnimationState)
	kSerializeVar(msName, eSerializeType_String)

	kSerializeVar(msAnimationName, eSerializeType_String)
	kSerializeVar(mfDefaultSpeed, eSerializeType_Float32)

	kSerializeVar(mfLength, eSerializeType_Float32)
	kSerializeVar(mfWeight, eSerializeType_Float32)
	kSerializeVar(mfSpeed, eSerializeType_Float32)
	kSerializeVar(mfTimePos, eSerializeType_Float32)

	kSerializeVar(mbActive, eSerializeType_Bool)
	kSerializeVar(mbLoop, eSerializeType_Bool)
	kSerializeVar(mbPaused, eSerializeType_Bool)

	kSerializeVar(mfFadeStep, eSerializeType_Float32)
	kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject* cSaveData_cAnimationState::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler,cGame *apGame)
	{
		return NULL;
	}

	//-----------------------------------------------------------------------

	int cSaveData_cAnimationState::GetSaveCreatePrio()
	{
		return 2;
	}

	//-----------------------------------------------------------------------

	iSaveData* cAnimationState::CreateSaveData()
	{
		return hplNew( cSaveData_cAnimationState, () );
	}

	//-----------------------------------------------------------------------

	void cAnimationState::SaveToSaveData(iSaveData *apSaveData)
	{
		kSaveData_SaveToBegin(cAnimationState);

		kSaveData_SaveTo(msName);

		pData->msAnimationName = mpAnimation->GetFileName();
		kSaveData_SaveTo(mfBaseSpeed);

		kSaveData_SaveTo(mfLength);
		kSaveData_SaveTo(mfWeight);
		kSaveData_SaveTo(mfSpeed);
		kSaveData_SaveTo(mfTimePos);

		kSaveData_SaveTo(mbActive);
		kSaveData_SaveTo(mbLoop);
		kSaveData_SaveTo(mbPaused);

		kSaveData_SaveTo(mfFadeStep);
	}

	//-----------------------------------------------------------------------

	void cAnimationState::LoadFromSaveData(iSaveData *apSaveData)
	{
		kSaveData_LoadFromBegin(cAnimationState);

		kSaveData_LoadFrom(msName);

		kSaveData_LoadFrom(mfLength);
		kSaveData_LoadFrom(mfWeight);
		kSaveData_LoadFrom(mfSpeed);
		kSaveData_LoadFrom(mfTimePos);

		kSaveData_LoadFrom(mfBaseSpeed);

		kSaveData_LoadFrom(mbActive);
		kSaveData_LoadFrom(mbLoop);
		kSaveData_LoadFrom(mbPaused);

		kSaveData_LoadFrom(mfFadeStep);
	}

	//-----------------------------------------------------------------------

	void cAnimationState::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame)
	{
		kSaveData_SetupBegin(cAnimationState);
	}

	//-----------------------------------------------------------------------

}
