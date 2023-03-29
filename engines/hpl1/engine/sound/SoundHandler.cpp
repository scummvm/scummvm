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

#include "hpl1/engine/sound/SoundHandler.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/SoundManager.h"
#include "hpl1/engine/sound/LowLevelSound.h"
#include "hpl1/engine/sound/SoundChannel.h"
#include "hpl1/engine/sound/SoundData.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"
#include "hpl1/engine/scene/World3D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSoundHandler::cSoundHandler(iLowLevelSound *apLowLevelSound, cResources *apResources) {
	mpLowLevelSound = apLowLevelSound;
	mpResources = apResources;

	mfSpeed = 1;
	mfNewSpeed = 1;
	mfSpeedRate = 0;
	mfVolume = 1;
	mfNewVolume = 1;
	mfVolumeRate = 0;

	mpWorld3D = NULL;

	mlCount = 0;
	mlIdCount = 0;

	mbSilent = false;

	mAffectedBySpeed = eSoundDest_World;
	mAffectedByVolume = eSoundDest_World;
}

//-----------------------------------------------------------------------

cSoundHandler::~cSoundHandler() {
	tSoundEntryListIt it;
	it = mlstGuiSounds.begin();
	while (it != mlstGuiSounds.end()) {
		it->mpSound->Stop();
		hplDelete(it->mpSound);
		it = mlstGuiSounds.erase(it);
	}

	it = mlstWorldSounds.begin();
	while (it != mlstWorldSounds.end()) {
		it->mpSound->Stop();
		hplDelete(it->mpSound);
		it = mlstWorldSounds.erase(it);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

void cSoundRayCallback::Reset() {
	mbHasCollided = false;
}

bool cSoundRayCallback::BeforeIntersect(iPhysicsBody *pBody) {
	if (pBody->GetBlocksSound())
		return true;

	return false;
}
bool cSoundRayCallback::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	mbHasCollided = true;
	return false;
}

//-----------------------------------------------------------------------

void cSoundEntry::Update(float afTimeStep) {
	if (mfNormalVolumeMul != mfNormalVolumeFadeDest) {
		// Log("speed %s: %f\n", msName.c_str(),mfNormalVolumeFadeSpeed);

		mfNormalVolumeMul += mfNormalVolumeFadeSpeed * afTimeStep;

		if (mfNormalVolumeMul < 0)
			mfNormalVolumeMul = 0;
		if (mfNormalVolumeMul > 1)
			mfNormalVolumeMul = 1;

		if (mfNormalVolumeFadeSpeed < 0) {
			if (mfNormalVolumeMul <= mfNormalVolumeFadeDest) {
				mfNormalVolumeMul = mfNormalVolumeFadeDest;
			}
		} else {
			if (mfNormalVolumeMul >= mfNormalVolumeFadeDest) {
				mfNormalVolumeMul = mfNormalVolumeFadeDest;
			}
		}
	}

	if (ABS(mfNormalVolumeFadeDest) < 0.001f && ABS(mfNormalVolumeMul) < 0.001f && mfNormalVolumeFadeSpeed <= 0) {
		mpSound->Stop();
	}
}

//-----------------------------------------------------------------------

iSoundChannel *cSoundHandler::Play(const tString &asName, bool abLoop, float afVolume, const cVector3f &avPos,
								   float afMinDist, float afMaxDist, eSoundDest mType, bool abRelative, bool ab3D, int alPriorityModifier,
								   eSoundDest aEffectType) {
	if (asName == "")
		return NULL;

	// Calculate priority
	int lPrio = 255;
	if (mType == eSoundDest_World && !abRelative) {
		lPrio = alPriorityModifier;

		float fDist = cMath::Vector3Dist(avPos, mpLowLevelSound->GetListenerPosition());
		if (fDist >= afMaxDist)
			lPrio += 0;
		else if (fDist >= afMinDist)
			lPrio += 10;
		else
			lPrio += 100;
	}

	// Create sound channel
	iSoundChannel *pSound = CreateChannel(asName, lPrio);
	if (pSound == NULL) {
		Warning("Can't find sound '%s' (may also be due too many sounds playing).\n", asName.c_str());
		return NULL;
	}

	// Set up channel
	pSound->SetLooping(abLoop);
	pSound->SetMinDistance(afMinDist);
	pSound->SetMaxDistance(afMaxDist);
	pSound->Set3D(ab3D);
	pSound->SetPriority(lPrio);

	/////// NEW -- Set sound to use Environment if its a world sound

	if (aEffectType == eSoundDest_World)
		pSound->SetAffectedByEnv(true);

	//////////////////////////////////////////////////////////////////

	if (mType == eSoundDest_Gui) {
		pSound->SetPositionRelative(true);
		//			pSound->SetPosition(avPos);
		pSound->SetRelPosition(avPos);

		cVector3f vPos = cMath::MatrixMul(mpLowLevelSound->GetListenerMatrix(), avPos);
		pSound->SetPosition(vPos);

		// Needed?
		//			pSound->SetPosition(mpLowLevelSound->GetListenerPosition() +
		//								pSound->GetRelPosition());
	} else {

		pSound->SetPositionRelative(abRelative);
		if (abRelative) {
			pSound->SetRelPosition(avPos);
			cVector3f vPos = cMath::MatrixMul(mpLowLevelSound->GetListenerMatrix(), avPos);
			pSound->SetPosition(vPos);
		} else {
			pSound->SetPosition(avPos);
		}
	}

	pSound->SetId(mlIdCount);

	cSoundEntry Entry;
	Entry.mpSound = pSound;
	Entry.mfNormalVolume = afVolume;
	Entry.msName = asName;
	Entry.mfNormalSpeed = 1.0f;

	Entry.mfBlockFadeDest = 1;
	Entry.mfBlockFadeSpeed = 1;
	Entry.mfBlockMul = 1;
	Entry.mbFirstTime = true;

	Entry.mEffectType = aEffectType;

	////////////////////////
	// Set start volume
	// GUI
	if (mType == eSoundDest_Gui) {
		pSound->SetVolume(afVolume);
	}
	// World
	else {
		pSound->SetVolume(0);
		// UpdateDistanceVolume3D(&Entry,1.0f/60.0f,false,mType);
	}

	// If it is silent do everything as normal except stop the sound at start.
	if (mbSilent) {
		pSound->SetLooping(false);
		pSound->Stop();
	} else {
		pSound->Play();
	}

	if (mType == eSoundDest_Gui) {
		mlstGuiSounds.push_back(Entry);
	} else {
		mlstWorldSounds.push_back(Entry);
	}

	// Log("Starting sound '%s' prio: %d\n",	pSound->GetData()->GetName().c_str(),
	//										pSound->GetPriority());

	mlIdCount++;

	return pSound;
}

//-----------------------------------------------------------------------

iSoundChannel *cSoundHandler::PlayGui(const tString &asName, bool abLoop, float afVolume, const cVector3f &avPos,
									  eSoundDest aEffectType) {
	return Play(asName, abLoop, afVolume, avPos, 1.0f, 1000.0f, eSoundDest_Gui, true, false, 0, aEffectType);
}

//-----------------------------------------------------------------------

iSoundChannel *cSoundHandler::PlayStream(const tString &asFileName, bool abLoop, float afVolume, bool ab3D, eSoundDest aEffectType) {
	if (asFileName == "")
		return NULL;

	iSoundData *pData = mpResources->GetSoundManager()->CreateSoundData(asFileName, true, abLoop);
	if (pData == NULL) {
		Error("Couldn't load stream '%s'\n", asFileName.c_str());
		return nullptr;
	}

	iSoundChannel *pSound = pData->CreateChannel(256);
	if (!pSound) {
		Error("Can't create sound channel for '%s'\n", asFileName.c_str());
		return NULL;
	}

	// If it is silent do everything as normal except stop the sound at start.
	if (mbSilent) {
		// pSound->SetLooping(false);
		pSound->Stop();
	} else {
		pSound->Play();
	}

	pSound->SetId(mlIdCount);

	pSound->Set3D(ab3D);

	cSoundEntry Entry;
	Entry.mpSound = pSound;
	Entry.mfNormalVolume = afVolume;
	Entry.msName = asFileName;
	Entry.mfNormalSpeed = 1.0f;

	Entry.mfBlockFadeDest = 1;
	Entry.mfBlockFadeSpeed = 1;
	Entry.mfBlockMul = 1;
	Entry.mbFirstTime = true;

	Entry.mbStream = true;

	Entry.mEffectType = aEffectType;

	/////////////////////////
	// Setup position
	pSound->SetPositionRelative(true);
	pSound->SetRelPosition(cVector3f(0, 0, 1));
	cVector3f vPos = cMath::MatrixMul(mpLowLevelSound->GetListenerMatrix(),
									  pSound->GetRelPosition());
	pSound->SetPosition(vPos);

	mlstGuiSounds.push_back(Entry);

	mlIdCount++;

	return pSound;
}

//-----------------------------------------------------------------------

bool cSoundHandler::Stop(const tString &asName) {
	cSoundEntry *pEntry = GetEntry(asName);
	if (pEntry) {
		pEntry->mpSound->Stop();
	} else {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

bool cSoundHandler::StopAllExcept(const tString &asName) {
	return false;
}

//-----------------------------------------------------------------------

void cSoundHandler::StopAll(tFlag mTypes) {
	tSoundEntryListIt it;
	if (mTypes & eSoundDest_Gui) {
		it = mlstGuiSounds.begin();
		while (it != mlstGuiSounds.end()) {
			it->mpSound->SetPaused(false);
			it->mpSound->Stop();
			it++;
		}
	}

	if (mTypes & eSoundDest_World) {
		it = mlstWorldSounds.begin();
		while (it != mlstWorldSounds.end()) {
			// Log("Stopping: %s\n",it->mpSound->GetData()->GetName().c_str());
			it->mpSound->SetPaused(false);
			it->mpSound->Stop();
			it++;
		}
	}
}
//-----------------------------------------------------------------------

void cSoundHandler::PauseAll(tFlag mTypes) {
	tSoundEntryListIt it;
	if (mTypes & eSoundDest_Gui) {
		it = mlstGuiSounds.begin();
		while (it != mlstGuiSounds.end()) {
			it->mpSound->SetPaused(true);
			it++;
		}
	}

	if (mTypes & eSoundDest_World) {
		it = mlstWorldSounds.begin();
		while (it != mlstWorldSounds.end()) {
			it->mpSound->SetPaused(true);
			it++;
		}
	}
}
//-----------------------------------------------------------------------

void cSoundHandler::ResumeAll(tFlag mTypes) {
	tSoundEntryListIt it;
	if (mTypes & eSoundDest_Gui) {
		it = mlstGuiSounds.begin();
		while (it != mlstGuiSounds.end()) {
			it->mpSound->SetPaused(false);
			it++;
		}
	}

	if (mTypes & eSoundDest_World) {
		it = mlstWorldSounds.begin();
		while (it != mlstWorldSounds.end()) {
			it->mpSound->SetPaused(false);
			it++;
		}
	}
}

//-----------------------------------------------------------------------

bool cSoundHandler::IsPlaying(const tString &asName) {
	cSoundEntry *pEntry = GetEntry(asName);

	if (pEntry)
		return pEntry->mpSound->IsPlaying();

	return false;
}
//-----------------------------------------------------------------------

bool cSoundHandler::IsValid(iSoundChannel *apChannel) {
	tSoundEntryListIt it;
	it = mlstWorldSounds.begin();
	while (it != mlstWorldSounds.end()) {
		if (it->mpSound == apChannel)
			return true;
		it++;
	}

	it = mlstGuiSounds.begin();
	while (it != mlstGuiSounds.end()) {
		if (it->mpSound == apChannel)
			return true;
		it++;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cSoundHandler::IsValidId(iSoundChannel *apChannel, int alId) {
	if (apChannel == NULL)
		return false;

	tSoundEntryListIt it = mlstWorldSounds.begin();
	while (it != mlstWorldSounds.end()) {
		if (it->mpSound == apChannel && it->mpSound->GetId() == alId)
			return true;
		it++;
	}

	it = mlstGuiSounds.begin();
	while (it != mlstGuiSounds.end()) {
		if (it->mpSound == apChannel && it->mpSound->GetId() == alId)
			return true;
		it++;
	}

	return false;
}

//-----------------------------------------------------------------------

void cSoundHandler::Update(float afTimeStep) {
	if (mfNewSpeed != mfSpeed) {
		mfSpeed += mfSpeedRate;
		if (mfSpeedRate < 0 && mfSpeed < mfNewSpeed)
			mfSpeed = mfNewSpeed;
		if (mfSpeedRate > 0 && mfSpeed > mfNewSpeed)
			mfSpeed = mfNewSpeed;
	}

	if (mfNewVolume != mfVolume) {
		mfVolume += mfVolumeRate * afTimeStep;
		if (mfVolumeRate < 0 && mfVolume < mfNewVolume)
			mfVolume = mfNewVolume;
		if (mfVolumeRate > 0 && mfVolume > mfNewVolume)
			mfVolume = mfNewVolume;
	}

	tSoundEntryListIt it;
	it = mlstGuiSounds.begin();
	while (it != mlstGuiSounds.end()) {
		if (UpdateEntry(&(*it), afTimeStep, eSoundDest_Gui) == false) {
			it = mlstGuiSounds.erase(it);
		} else {
			++it;
		}
	}

	it = mlstWorldSounds.begin();
	while (it != mlstWorldSounds.end()) {
		if (UpdateEntry(&(*it), afTimeStep, eSoundDest_World) == false) {
			it = mlstWorldSounds.erase(it);
		} else {
			++it;
		}
	}

	mlCount++;
}
//-----------------------------------------------------------------------

/**
 *
 * \todo support types.
 * \param afSpeed New speed to run sounds at
 * \param afRate Rate by which the current speed transform to the new
 * \param mTypes Types affected, not working yet :S
 */
void cSoundHandler::SetSpeed(float afSpeed, float afRate, tFlag aTypes) {
	mfNewSpeed = afSpeed;

	if (mfNewSpeed > mfSpeed && afRate < 0)
		afRate = -afRate;
	if (mfNewSpeed < mfSpeed && afRate > 0)
		afRate = -afRate;

	mfSpeedRate = afRate;

	mAffectedBySpeed = aTypes;

	if (afRate == 0)
		mfSpeed = mfNewSpeed;
}

//-----------------------------------------------------------------------

void cSoundHandler::SetVolume(float afVolume, float afRate, tFlag aTypes) {
	mfNewVolume = afVolume;

	if (mfNewVolume > mfVolume && afRate < 0)
		afRate = -afRate;
	if (mfNewVolume < mfVolume && afRate > 0)
		afRate = -afRate;

	mfVolumeRate = afRate;

	mAffectedByVolume = aTypes;

	if (afRate == 0)
		mfVolume = mfNewVolume;
}

//-----------------------------------------------------------------------

void cSoundHandler::SetWorld3D(cWorld3D *apWorld3D) {
	mpWorld3D = apWorld3D;
}

//-----------------------------------------------------------------------

cSoundEntry *cSoundHandler::GetEntryFromSound(iSoundChannel *apSound) {
	tSoundEntryListIt it = mlstGuiSounds.begin();
	while (it != mlstGuiSounds.end()) {
		if (it->mpSound == apSound) {
			// Log("returning from GUI %d\n",&(*it));
			return &(*it);
		}
		++it;
	}

	it = mlstWorldSounds.begin();
	while (it != mlstWorldSounds.end()) {
		if (it->mpSound == apSound) {
			// Log("returning from World %d\n",&(*it));
			return &(*it);
		}
		++it;
	}

	return NULL;
}

//-----------------------------------------------------------------------

tSoundEntryList *cSoundHandler::GetWorldEntryList() {
	return &mlstWorldSounds;
}

tSoundEntryList *cSoundHandler::GetGuiEntryList() {
	return &mlstGuiSounds;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSoundHandler::UpdateEntry(cSoundEntry *apEntry, float afTimeStep, tFlag aTypes) {
	// if(apEntry->mbStream) return;

	apEntry->Update(afTimeStep);

	/*Log("Updating entry: '%s' vol: %f playing: %d\n",apEntry->msName.c_str(),
													apEntry->mpSound->GetVolume(),
													apEntry->mpSound->IsPlaying()?1:0);*/
	// Log("Pos: %s\n",apEntry->mpSound->GetPosition().ToString().c_str());

	if (!apEntry->mpSound->IsPlaying() && !apEntry->mpSound->GetPaused()) {
		iSoundChannel *pSound = apEntry->mpSound;

		if (pSound->GetStopUsed() == false && pSound->GetCallBack() && pSound->GetLooping() && apEntry->mfNormalVolumeFadeDest != 0) {
			pSound->GetCallBack()->OnPriorityRelease();
			// Log("On prio release!\n");
		}

		// Log("Stopping: %s Time: %f / %f\n",pSound->GetData()->GetName().c_str(),
		//									pSound->GetElapsedTime(),
		//									pSound->GetTotalTime());

		pSound->Stop();
		hplDelete(pSound);

		return false;
	} else {
		if (mAffectedBySpeed & apEntry->mEffectType) {
			float fSpeed = mfSpeed * apEntry->mfNormalSpeed;
			if (apEntry->mpSound->GetSpeed() != fSpeed) {
				apEntry->mpSound->SetSpeed(fSpeed);
			}
		} else {
			if (apEntry->mpSound->GetSpeed() != apEntry->mfNormalSpeed) {
				apEntry->mpSound->SetSpeed(apEntry->mfNormalSpeed);
			}
		}

		//////////////////////////////
		// Update block fade:
		if (apEntry->mfBlockMul != apEntry->mfBlockFadeDest) {
			apEntry->mfBlockMul += apEntry->mfBlockFadeSpeed * afTimeStep;
			if (apEntry->mfBlockFadeSpeed < 0) {
				if (apEntry->mfBlockMul < apEntry->mfBlockFadeDest)
					apEntry->mfBlockMul = apEntry->mfBlockFadeDest;
			} else {
				if (apEntry->mfBlockMul > apEntry->mfBlockFadeDest)
					apEntry->mfBlockMul = apEntry->mfBlockFadeDest;
			}
		}

		// Update the sound position
		// 3D Position!
		if (apEntry->mbStream) {
			apEntry->mpSound->SetVolume(apEntry->mfNormalVolume * apEntry->mfNormalVolumeMul * mfVolume);
		}
		// else if(apEntry->mpSound->Get3D())
		if (apEntry->mpSound->Get3D()) {
			UpdateDistanceVolume3D(apEntry, afTimeStep, apEntry->mbFirstTime ? false : true, aTypes);
		}
		// 2D Position
		else {
			if (apEntry->mpSound->GetPositionRelative()) {
				iSoundChannel *pSound = apEntry->mpSound;

				cVector3f vPos = cMath::MatrixMul(mpLowLevelSound->GetListenerMatrix(), pSound->GetRelPosition());
				pSound->SetPosition(vPos);

				if (apEntry->mEffectType & mAffectedByVolume) {
					pSound->SetVolume(apEntry->mfNormalVolume * apEntry->mfNormalVolumeMul * mfVolume);
				} else {
					pSound->SetVolume(apEntry->mfNormalVolume * apEntry->mfNormalVolumeMul);
				}
			} else {
				iSoundChannel *pSound = apEntry->mpSound;
				float fDX = pSound->GetPosition().x - mpLowLevelSound->GetListenerPosition().x;
				float fDY = pSound->GetPosition().y - mpLowLevelSound->GetListenerPosition().y;

				float fDist = sqrt(fDX * fDX + fDY * fDY);

				if (fDist >= pSound->GetMaxDistance()) {
					pSound->SetVolume(0);
				} else {
					if (fDist < pSound->GetMinDistance()) {
						pSound->SetVolume(apEntry->mfNormalVolume); // some other stuff to support volume effects
					} else {
						float fVolume = 1 - ((fDist - pSound->GetMinDistance()) /
											 (pSound->GetMaxDistance() - pSound->GetMinDistance()));
						fVolume *= apEntry->mfNormalVolume;

						pSound->SetVolume(fVolume);
					}
					float fPan = 1 - (0.5f - 0.4f * (fDX / pSound->GetMaxDistance()));
					pSound->SetPan(fPan);
				}
			}
		}
	}

	apEntry->mbFirstTime = false;

	return true;
}

//-----------------------------------------------------------------------

void cSoundHandler::UpdateDistanceVolume3D(cSoundEntry *apEntry, float afTimeStep, bool abFade,
										   tFlag aTypes) {
	if (mpWorld3D == NULL) {
		return;
	}

	if (apEntry->mpSound->GetPositionRelative()) {
		iSoundChannel *pSound = apEntry->mpSound;
		cVector3f vPos = cMath::MatrixMul(mpLowLevelSound->GetListenerMatrix(),
										  pSound->GetRelPosition());

		pSound->SetPosition(vPos);
		//			cVector3f vPos = cMath::MatrixMul(mpLowLevelSound->GetListenerMatrix(),
		//												pSound->GetRelPosition());

		//			pSound->SetPosition(pSound->GetRelPosition());

		if (aTypes & mAffectedByVolume) {
			pSound->SetVolume(apEntry->mfNormalVolume * apEntry->mfNormalVolumeMul * mfVolume);
		} else {
			pSound->SetVolume(apEntry->mfNormalVolume * apEntry->mfNormalVolumeMul);
		}
	} else {
		// Log("%s ",apEntry->msName.c_str());

		iSoundChannel *pSound = apEntry->mpSound;
		float fDist = cMath::Vector3Dist(pSound->GetPosition(),
										 mpLowLevelSound->GetListenerPosition());

		if (fDist >= pSound->GetMaxDistance()) {
			pSound->SetVolume(0);

			// Set very low priority
			pSound->SetPriority(0);

			// Log(" max distance ");
		} else {
			float fVolume = 0;
			// bool bBlocked = false;

			////////////////////////////////////////
			// Check if sound is blocked.
			if (pSound->GetBlockable() && mpWorld3D && mpWorld3D->GetPhysicsWorld() && (apEntry->mlCount % 30) == 0) {
				iPhysicsWorld *pPhysicsWorld = mpWorld3D->GetPhysicsWorld();

				mSoundRayCallback.Reset();

				pPhysicsWorld->CastRay(&mSoundRayCallback, pSound->GetPosition(),
									   mpLowLevelSound->GetListenerPosition(),
									   false, false, false, true);

				// Log(" from (%s) to (%s) ",pSound->GetPosition().ToString().c_str(),
				//						mpLowLevelSound->GetListenerPosition().ToString().c_str());

				// Log(" callback: %d ",mSoundRayCallback.HasCollided()?1:0);

				if (mSoundRayCallback.HasCollided()) {
					apEntry->mfBlockFadeDest = 0.0f;
					apEntry->mfBlockFadeSpeed = -1.0f / 0.55f;

					if (abFade == false) {
						apEntry->mfBlockMul = 0.0f;
					}

					pSound->SetFiltering(true, 0xF);
					// bBlocked = true;
				} else {
					// pSound->SetFiltering(false, 0xF);

					apEntry->mfBlockFadeDest = 1;
					apEntry->mfBlockFadeSpeed = 1.0f / 0.2f;

					if (abFade == false) {
						apEntry->mfBlockMul = 1.0f;
					}
				}

				// Log("Blocked: %d ",bBlocked ? 1 : 0);
			}
			++apEntry->mlCount;

			/////////////////////////////////////
			// Lower volume according to distance
			if (fDist < pSound->GetMinDistance()) {
				// Set high priority
				pSound->SetPriority(100);

				fVolume = apEntry->mfNormalVolume; // some other stuff to support volume effects
			} else {
				// Set medium priority
				pSound->SetPriority(10);

				float fDelta = fDist - pSound->GetMinDistance();
				float fMaxDelta = pSound->GetMaxDistance() - pSound->GetMinDistance();

				fVolume = 1 - (fDelta / fMaxDelta);
				float fSqr = fVolume * fVolume;

				// Log("Lin: %f Sqr: %f ",fVolume,fSqr);

				// fade between normal and square
				fVolume = fVolume * apEntry->mfBlockMul + (1.0f - apEntry->mfBlockMul) * fSqr;

				// Log("Mix: %f ",fVolume);

				fVolume *= apEntry->mfNormalVolume;
			}

			float fBlock = pSound->GetBlockVolumeMul() +
						   apEntry->mfBlockMul * (1 - pSound->GetBlockVolumeMul());

			if (aTypes & apEntry->mEffectType) {
				pSound->SetVolume(fBlock * fVolume * apEntry->mfNormalVolumeMul * mfVolume);
				// pSound->SetFilterGainHF(fBlock * fVolume * apEntry->mfNormalVolumeMul * mfVolume);
			} else {
				pSound->SetVolume(fBlock * fVolume * apEntry->mfNormalVolumeMul);
			}

			// pSound->SetFilterGainHF(0.1f);

			// Log("Vol: %f\n",fBlock * fVolume * apEntry->mfNormalVolumeMul);
			// Log("%s Block: %f\n",apEntry->msName.c_str(),apEntry->mfBlockMul);
		}
	}
}

//-----------------------------------------------------------------------

cSoundEntry *cSoundHandler::GetEntry(const tString &asName) {
	tSoundEntryListIt it = mlstGuiSounds.begin();
	while (it != mlstGuiSounds.end()) {
		if (cString::ToLowerCase(it->msName) == cString::ToLowerCase(asName)) {
			return &(*it);
		}
		it++;
	}

	it = mlstWorldSounds.begin();
	while (it != mlstWorldSounds.end()) {
		// Log("'%s' vs '%s'\n", it->msName.c_str(), asName.c_str());
		if (cString::ToLowerCase(it->msName) == cString::ToLowerCase(asName)) {
			return &(*it);
		}
		it++;
	}

	return NULL;
}

//-----------------------------------------------------------------------

iSoundChannel *cSoundHandler::CreateChannel(const tString &asName, int alPriority) {
	int lNum = cString::ToInt(cString::GetLastChar(asName).c_str(), 0);
	iSoundChannel *pSound = NULL;
	iSoundData *pData = NULL;
	tString sName;
	tString sBaseName = asName;

	// Try loading it from the buffer
	if (lNum >= 1 && lNum <= 9) {
		pData = mpResources->GetSoundManager()->CreateSoundData(asName, false);
	} else {
		int lCount = 0;
		int lLastNum = -1;

		// Check what the last sound played was.
		tPlayedSoundNumMapIt SoundIt = m_mapPlayedSound.find(sBaseName);
		if (SoundIt == m_mapPlayedSound.end()) {
			m_mapPlayedSound.insert(tPlayedSoundNumMap::value_type(sBaseName, 0));
			SoundIt = m_mapPlayedSound.find(sBaseName);
		} else {
			lLastNum = SoundIt->second;
		}

		sName = sBaseName + cString::ToString(lCount + 1);
		pData = mpResources->GetSoundManager()->CreateSoundData(sName, false);
		while (pData) {
			lCount++;
			sName = sBaseName + cString::ToString(lCount + 1);
			pData = mpResources->GetSoundManager()->CreateSoundData(sName, false);
		}

		if (lCount > 0) {
			int lNum2 = cMath::RandRectl(1, lCount);

			if (lCount > 2) {
				while (lLastNum == lNum2)
					lNum2 = cMath::RandRectl(1, lCount);
			}
			SoundIt->second = lNum2;

			sName = sBaseName + cString::ToString(lNum2);

			pData = mpResources->GetSoundManager()->CreateSoundData(sName, false);

		} else {
			pData = NULL;
		}
	}

	// Try to stream it
	if (pData == NULL) {
		sName = "stream_" + sBaseName;

		pData = mpResources->GetSoundManager()->CreateSoundData(sName, true);
		if (pData == NULL) {
			Error("Couldn't stream sound '%s'\n", asName.c_str());
			return NULL;
		}
	}

	// Create sound channel
	pSound = pData->CreateChannel(alPriority);
	if (pSound == NULL) {
		// Warning("Couldn't play sound '%s'\n",asName.c_str());
	}

	return pSound;
}

//-----------------------------------------------------------------------

} // namespace hpl
