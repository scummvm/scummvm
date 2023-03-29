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
 */

#include "hpl1/engine/scene/SoundEntity.h"

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/SoundEntityManager.h"
#include "hpl1/engine/sound/LowLevelSound.h"
#include "hpl1/engine/sound/Sound.h"
#include "hpl1/engine/sound/SoundChannel.h"
#include "hpl1/engine/sound/SoundEntityData.h"
#include "hpl1/engine/sound/SoundHandler.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

namespace hpl {

tSoundEntityGlobalCallbackList *cSoundEntity::mlstGlobalCallbacks = nullptr;

void cSoundEntity::initGlobalCallbackList() {
	cSoundEntity::mlstGlobalCallbacks = new tSoundEntityGlobalCallbackList;
}
void cSoundEntity::finalizeGlobalCallbackList() {
	delete cSoundEntity::mlstGlobalCallbacks;
}

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSoundEntity::cSoundEntity(const tString &asName, cSoundEntityData *apData,
						   cSoundEntityManager *apSoundEntityManager,
						   cWorld3D *apWorld,
						   cSoundHandler *apSoundHandler, bool abRemoveWhenOver)
	: iEntity3D(asName) {
	mbRemoveWhenOver = abRemoveWhenOver;

	// Log("Created sound entity '%s'\n",msName.c_str());

	mpSoundHandler = apSoundHandler;
	mpSoundEntityManager = apSoundEntityManager;
	mpWorld = apWorld;

	mpData = apData;

	mBoundingVolume.SetSize(mpData->GetMaxDistance() * 2);

	for (int i = 0; i < 3; i++) {
		mvSounds[i] = NULL;
		// mvSoundEntries[i] = NULL;
		mvSoundId[i] = -1;
	}

	mfIntervalCount = mpData->GetInterval();

	mfVolume = mpData->GetVolume();

	mbStopped = false; // If the sound should be stopped
	mbStarted = false; // If the sound started playing-

	mbSkipStartEnd = false; // If the end and start sounds should be skipped.

	mbPrioRemove = false; // If the sounds was removed because too many where playing

	mbFadingOut = false; // If the sound is fading out.

	mbOutOfRange = false; // If the sound is out of range.

	mbLog = false;

	mfSleepCount = 0;

	mpSoundCallback = hplNew(cSoundEntityChannelCallback, ());
	mpSoundCallback->mpEntity = this;

	if (mpSoundHandler->GetSilent()) {
		mbStarted = true;
		mbStopped = true;
		mfVolume = 0;
		mbSkipStartEnd = true;
		mbRemoveWhenOver = true;
	}
}

cSoundEntity::~cSoundEntity() {
	if (mbLog)
		Log("Delete entity start...");
	for (int i = 0; i < 3; i++) {
		if (mpSoundHandler->IsValid(mvSounds[i])) {
			mvSounds[i]->Stop();
		}
	}

	hplDelete(mpSoundCallback);

	if (mbLog)
		Log("end\n");

	// Not sure if this should be here. SOund entities take little memory
	// might be better of releasing all of em at exit.
	// mpSoundEntityManager->Destroy(mpData);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cSoundEntityChannelCallback::OnPriorityRelease() {
	if (mpEntity->mbStopped || mpEntity->mbFadingOut)
		return;

	mpEntity->mbPrioRemove = true;
}

//-----------------------------------------------------------------------

cSoundEntry *cSoundEntity::GetSoundEntry(eSoundEntityType aType) {
	if (mvSounds[aType] == NULL)
		return NULL;

	// if(mvSoundEntries[aType]==NULL)
	//{
	//	mvSoundEntries[aType] = mpSoundHandler->GetEntryFromSound(mvSounds[aType]);
	// }
	// return mvSoundEntries[aType];

	// if(mpSoundHandler->IsValid(mvSounds[aType])==false) return NULL;

	return mpSoundHandler->GetEntryFromSound(mvSounds[aType]);
}

//-----------------------------------------------------------------------
void cSoundEntity::Play(bool abPlayStart) {
	if (mpSoundHandler->GetSilent())
		return;

	if (mbLog)
		Log("Play entity start...");
	mbStopped = false;
	mbOutOfRange = false;
	mbFadingOut = false;

	if (abPlayStart && mbSkipStartEnd == false) {
		if (mpData->GetLoop() && mpData->GetStartSoundName() != "") {
			PlaySound(mpData->GetStartSoundName(), false, eSoundEntityType_Start);
			mbStarted = false;
		}
	}

	if (mvSounds[eSoundEntityType_Main] == NULL && mvSounds[eSoundEntityType_Start] == NULL && (mpData->GetLoop() == false || mpData->GetInterval() == 0)) {
		PlaySound(mpData->GetMainSoundName(), mpData->GetLoop(), eSoundEntityType_Main);
		mbStarted = true;

		// Log("-- Start playing sound %s\n",msName.c_str());
	} else {
		/*Log("-- Couldn't play sound %s\n",msName.c_str());
		Log("Main: %d Start: %d Loop: %d Interval: %f\n",
						(int)mvSounds[eSoundEntityType_Main],
						(int)mvSounds[eSoundEntityType_Start],
						mpData->GetLoop() ? 1 : 0,
						mpData->GetInterval());*/
	}

	if (mbLog)
		Log("end\n");
}

//-----------------------------------------------------------------------

void cSoundEntity::Stop(bool abPlayEnd) {
	if (mbStopped)
		return;

	mbStopped = true;
	mbOutOfRange = false;
	// mvSoundEntries[eSoundEntityType_Main] = NULL;

	if (mvSounds[eSoundEntityType_Main] == NULL)
		return;

	// Log("entity Stopping %s\n",mpData->GetName().c_str());

	if (mbLog)
		Log("Stop entity start...");

	if (mpData->GetLoop()) {
		if (abPlayEnd && mbSkipStartEnd == false) {
			PlaySound(mpData->GetStopSoundName(), false, eSoundEntityType_Stop);
		}

		if (mpSoundHandler->IsValid(mvSounds[eSoundEntityType_Main])) {
			mvSounds[eSoundEntityType_Main]->Stop();
		}
	} else {
		if (mpSoundHandler->IsValid(mvSounds[eSoundEntityType_Main])) {
			mvSounds[eSoundEntityType_Main]->Stop();
		}
	}

	mvSounds[eSoundEntityType_Main] = NULL;

	if (mbLog)
		Log("End\n");
}

//-----------------------------------------------------------------------

void cSoundEntity::FadeIn(float afSpeed) {
	if (mpSoundHandler->GetSilent())
		return;

	if (mbLog)
		Log("Fade in entity start...");

	Play(false);
	cSoundEntry *pEntry = GetSoundEntry(eSoundEntityType_Main);
	if (pEntry) {
		pEntry->mfNormalVolumeMul = 0;
		pEntry->mfNormalVolumeFadeDest = 1;
		pEntry->mfNormalVolumeFadeSpeed = afSpeed;
		if (pEntry->mfNormalVolumeFadeSpeed < 0)
			pEntry->mfNormalVolumeFadeSpeed = -pEntry->mfNormalVolumeFadeSpeed;
		// Log("Setting speed: %f\n",pEntry->mfNormalVolumeFadeSpeed);
	}

	if (mbLog)
		Log("end\n");
}

void cSoundEntity::FadeOut(float afSpeed) {
	mbFadingOut = true;
	_fadeSpeed = afSpeed;
	if (mpSoundHandler->GetSilent())
		return;

	if (mbLog)
		Log("Fade out entity start...");

	if (mvSounds[eSoundEntityType_Main] == NULL) {
		mbStopped = true;
		return;
	}

	cSoundEntry *pEntry = GetSoundEntry(eSoundEntityType_Main);
	if (pEntry) {
		pEntry->mfNormalVolumeFadeDest = 0;
		pEntry->mfNormalVolumeFadeSpeed = afSpeed;
		if (pEntry->mfNormalVolumeFadeSpeed > 0)
			pEntry->mfNormalVolumeFadeSpeed = -pEntry->mfNormalVolumeFadeSpeed;
	} else {
		mbStopped = true;
	}

	if (mbLog)
		Log("end\n");
}

//-----------------------------------------------------------------------

bool cSoundEntity::IsStopped() {
	if (mbStopped && mvSounds[eSoundEntityType_Stop] == NULL) {
		return true;
	}

	return false;
}

bool cSoundEntity::IsFadingOut() {
	return mbFadingOut;
}

//-----------------------------------------------------------------------

bool cSoundEntity::GetRemoveWhenOver() {
	return mbRemoveWhenOver;
}

//-----------------------------------------------------------------------

void cSoundEntity::UpdateLogic(float afTimeStep) {
	if (mpSoundHandler->GetSilent()) {
		return;
	}

	if (mfSleepCount > 0) {
		mfSleepCount -= afTimeStep;
		return;
	}

	//////////////////////////////////////////////
	// If out of range check if it is inside range.
	if (mbOutOfRange && mbStopped == false) {
		float fRange = GetListenerSqrLength();
		float fMaxRange = mpData->GetMaxDistance();
		fMaxRange = fMaxRange * fMaxRange;

		if (fRange < fMaxRange) {
			Play(false);
			mbOutOfRange = false;
		} else {
			return;
		}
	}

	if (mbLog)
		Log("Update entity start...");

	/////////////////////////////////////////////////
	// Go through all sounds and check if they are playing,
	// if so update their positions else remove.
	for (int i = 0; i < 3; i++) {
		if (mvSounds[i]) {
			if (mpSoundHandler->IsValidId(mvSounds[i], mvSoundId[i]) == false) {
				mvSounds[i] = NULL;
				// mvSoundEntries[i] = NULL;

				// if the sound has stopped due to priority try starting again
				if (mbPrioRemove && i == (int)eSoundEntityType_Main) {
					if (mbStopped == false) {
						mbStarted = false;
						// mbPrioRemove = false;
						mfSleepCount = 0.3f;
					} else {
						// if(mbLog)Log("  -Sound already stopped..\n");
					}
				}
				// If the sound has already started, stop it.
				else if ((mpData->GetInterval() == 0 || mpData->GetLoop() == false) &&
						 mbStarted) {
					// Log("Stopping %s!\n",GetName().c_str());
					mbStopped = true;
				}
			} else {
				if (mbPrioRemove) {
					// if(mbLog)Log("  -Removed + valid? This is baaaad!\n");
				}

				if (mvSounds[i] && !mvSounds[i]->GetPositionRelative()) {
					mvSounds[i]->SetPosition(GetWorldPosition());
				}
			}
		}
	}

	//////////////////////////////////////////////
	// Check if sound is playing, and if so update it.
	if (mbStopped == false) {
		/////////////////////////////////////////////////////////////////////////
		// Sound is not playing, start it and since it has not been invoked by Play
		// no need to play start sound. Only do this if interval is 0 else it might
		// be a deliberate pause.
		if (mvSounds[eSoundEntityType_Main] == NULL && mvSounds[eSoundEntityType_Start] == NULL && (mpData->GetLoop() == false || mpData->GetInterval() == 0)) {
			if (PlaySound(mpData->GetMainSoundName(), mpData->GetLoop(), eSoundEntityType_Main)) {
				mbStarted = true;
				mbPrioRemove = false;

				// Call the callbacks that the sound has started.
				tSoundEntityGlobalCallbackListIt it = mlstGlobalCallbacks->begin();
				for (; it != mlstGlobalCallbacks->end(); ++it) {
					iSoundEntityGlobalCallback *pCallback = *it;
					pCallback->OnStart(this);
				}
			} else if (mbPrioRemove) {
				mfSleepCount = 0.3f;
			}
		}

		///////////////////////////////////////////////////////////
		// Check if looping and interval is not 0.
		// then there needs to be some updating.
		if (mpData->GetLoop() && mpData->GetInterval() > 0 && mvSounds[eSoundEntityType_Start] == NULL && mvSounds[eSoundEntityType_Main] == NULL) {
			mfIntervalCount += afTimeStep;

			// if the interval time has elapsed the sound might be played again.
			if (mfIntervalCount >= mpData->GetInterval()) {
				// Check random and if rand is right play the sound.
				if (cMath::RandRectf(0, 1) <= mpData->GetRandom() ||
					mpData->GetRandom() == 0) {
					PlaySound(mpData->GetMainSoundName(), false, eSoundEntityType_Main);
				}
				mfIntervalCount = 0;
			}
		}

		//////////////////////////////////////////////
		// If the sound is looped and out of range, stop it
		if (mvSounds[eSoundEntityType_Start] == NULL && mpData->GetLoop() && mpData->GetUse3D()) {
			float fRange = GetListenerSqrLength();
			float fMaxRange = mpData->GetMaxDistance() + 1.0f; // Add one meter to avoid oscillation
			fMaxRange = fMaxRange * fMaxRange;

			if (fRange > fMaxRange) {
				if (mvSounds[eSoundEntityType_Main]) {
					if (mpSoundHandler->IsValid(mvSounds[eSoundEntityType_Main])) {
						mvSounds[eSoundEntityType_Main]->Stop();
					}

					mvSounds[eSoundEntityType_Main] = NULL;
					if (mbFadingOut)
						mbStopped = true;
				}

				mbOutOfRange = true;
			}
		}
	}

	if (mbLog)
		Log("end\n");
}

//-----------------------------------------------------------------------

void cSoundEntity::AddGlobalCallback(iSoundEntityGlobalCallback *apCallback) {
	mlstGlobalCallbacks->push_back(apCallback);
}

//-----------------------------------------------------------------------

void cSoundEntity::RemoveGlobalCallback(iSoundEntityGlobalCallback *apCallback) {
	tSoundEntityGlobalCallbackListIt it = mlstGlobalCallbacks->begin();
	for (; it != mlstGlobalCallbacks->end(); ++it) {
		if (apCallback == *it) {
			mlstGlobalCallbacks->erase(it);
			return;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSoundEntity::PlaySound(const tString &asName, bool abLoop, eSoundEntityType aType) {
	if (asName == "")
		return false;

	if (mpData->GetUse3D()) {
		mvSounds[aType] = mpSoundHandler->Play3D(
			asName, abLoop,
			mfVolume, GetWorldPosition(),
			mpData->GetMinDistance(), mpData->GetMaxDistance(),
			eSoundDest_World, false, mpData->GetPriority());
	} else {
		mvSounds[aType] = mpSoundHandler->Play3D(
			asName, abLoop,
			mfVolume, cVector3f(0, 0, 1),
			mpData->GetMinDistance(), mpData->GetMaxDistance(),
			eSoundDest_World, true, mpData->GetPriority());
	}

	if (mvSounds[aType] == NULL) {
		if (!mbPrioRemove)
			Error("Couldn't play sound '%s' for sound entity %s\n", asName.c_str(), msName.c_str());

		if (aType == eSoundEntityType_Main && !mbPrioRemove) {
			mbStopped = true;
			// mbRemoveWhenOver = true;
		}
		return false;
	} else {
		mvSounds[aType]->SetBlockable(mpData->GetBlockable());
		mvSounds[aType]->SetBlockVolumeMul(mpData->GetBlockVolumeMul());
		mvSounds[aType]->SetPriorityModifier(mpData->GetPriority());

		mvSoundId[aType] = mvSounds[aType]->GetId();

		if (aType == eSoundEntityType_Main) {
			mvSounds[aType]->SetCallBack(mpSoundCallback);
		}
	}

	return true;

	// for(int i=0; i<3; ++i) mvSoundEntries[i] = NULL;
}

//-----------------------------------------------------------------------

float cSoundEntity::GetListenerSqrLength() {
	cVector3f vListenerPos = mpWorld->GetSound()->GetLowLevel()->GetListenerPosition();
	return cMath::Vector3DistSqr(vListenerPos, GetWorldPosition());
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_cSoundEntity, cSaveData_iEntity3D)
	kSerializeVar(msData, eSerializeType_String)
		kSerializeVar(mbStopped, eSerializeType_Bool)
			kSerializeVar(mbRemoveWhenOver, eSerializeType_Bool)
				kSerializeVar(mbStarted, eSerializeType_Bool)
					kSerializeVar(mbFadingOut, eSerializeType_Bool)
						kSerializeVar(mfVolume, eSerializeType_Float32)
							kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cSoundEntity::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	cWorld3D *pWorld = apGame->GetScene()->GetWorld3D();

	if (mbStopped && mbRemoveWhenOver)
		return NULL;
	// if(mbStopped && mbFadingOut) return NULL;

	cSoundEntity *pEntity = pWorld->CreateSoundEntity(msName, msData, mbRemoveWhenOver);

	if (mbFadingOut)
		pEntity->Stop(false);

	return pEntity;
}

//-----------------------------------------------------------------------

int cSaveData_cSoundEntity::GetSaveCreatePrio() {
	return 3;
}

//-----------------------------------------------------------------------

iSaveData *cSoundEntity::CreateSaveData() {
	return hplNew(cSaveData_cSoundEntity, ());
}

//-----------------------------------------------------------------------

void cSoundEntity::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(cSoundEntity);

	pData->msData = mpData == NULL ? "" : mpData->GetName();

	kSaveData_SaveTo(mbStopped);
	kSaveData_SaveTo(mbRemoveWhenOver);
	kSaveData_SaveTo(mbStarted);
	kSaveData_SaveTo(mbFadingOut);
	kSaveData_SaveTo(mfVolume);
}

//-----------------------------------------------------------------------

void cSoundEntity::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(cSoundEntity);

	kSaveData_LoadFrom(mfVolume);
	kSaveData_LoadFrom(mbStopped);
	kSaveData_LoadFrom(mbRemoveWhenOver);
	// kSaveData_LoadFrom(mbStarted);
}

//-----------------------------------------------------------------------

void cSoundEntity::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(cSoundEntity);

	/*Log("-- %s After setup: Main: %d Start: %d Loop: %d Interval: %f Stopped: %d Started: %d\n",
		msName.c_str(),
		(int)mvSounds[eSoundEntityType_Main],
		(int)mvSounds[eSoundEntityType_Start],
		mpData->GetLoop() ? 1 : 0,
		mpData->GetInterval(),
		mbStopped ? 1 : 0,
		mbStarted ? 1 : 0);*/
}

//-----------------------------------------------------------------------

} // namespace hpl
