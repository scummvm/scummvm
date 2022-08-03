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

#include "hpl1/engine/impl/LowLevelSoundOpenAL.h"
#include "hpl1/engine/impl/OpenALSoundData.h"
#include "hpl1/engine/impl/OpenALSoundEnvironment.h"
#include "hpl1/engine/system/String.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/system/low_level_system.h"
#include "common/system.h"
#include "audio/mixer.h"
#include "hpl1/debug.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLowLevelSoundOpenAL::cLowLevelSoundOpenAL() {
	mvFormats[0] = "OGG";
	mvFormats[1] = "WAV";
	mvFormats[2] = "";
	mbInitialized = false;
	mbEnvAudioEnabled = false;
	mbNullEffectAttached = false;
}

//-----------------------------------------------------------------------

cLowLevelSoundOpenAL::~cLowLevelSoundOpenAL() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHOD
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iSoundData *cLowLevelSoundOpenAL::LoadSoundData(const tString &asName, const tString &asFilePath,
												const tString &asType, bool abStream, bool abLoopStream) {
	cOpenALSoundData *pSoundData = hplNew(cOpenALSoundData, (asName, abStream));

	pSoundData->SetLoopStream(abLoopStream);

	if (pSoundData->CreateFromFile(asFilePath) == false) {
		hplDelete(pSoundData);
		return NULL;
	}

	return pSoundData;
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::GetSupportedFormats(tStringList &alstFormats) {
	int lPos = 0;

	while (mvFormats[lPos] != "") {
		alstFormats.push_back(mvFormats[lPos]);
		lPos++;
	}
}
//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::UpdateSound(float afTimeStep) {
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::SetListenerAttributes(const cVector3f &avPos, const cVector3f &avVel,
												 const cVector3f &avForward, const cVector3f &avUp) {
	mvListenerPosition = avPos;
	mvListenerVelocity = avVel;
	mvListenerForward = avForward;
	mvListenerUp = avUp;

	mvListenerRight = cMath::Vector3Cross(mvListenerForward, mvListenerUp);

	//		m_mtxListener = cMatrixf(
	//				-mvListenerRight.x, -mvListenerRight.y,-mvListenerRight.z, avPos.x,
	//				-mvListenerUp.x, -mvListenerUp.y,-mvListenerUp.z, avPos.y,
	//				-mvListenerForward.x, -mvListenerForward.y,-mvListenerForward.z, avPos.z,
	//				0, 0,0, 1
	//			);
	m_mtxListener = cMatrixf::Identity;
	m_mtxListener.SetRight(mvListenerRight);
	m_mtxListener.SetUp(mvListenerUp);
	m_mtxListener.SetForward(mvListenerForward * -1);
	m_mtxListener = cMath::MatrixInverse(m_mtxListener);
	m_mtxListener.SetTranslation(mvListenerPosition);
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::SetListenerPosition(const cVector3f &avPos) {
  	mvListenerPosition = avPos;
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::SetListenerAttenuation(bool abEnabled) {
	mbListenerAttenuation = abEnabled;
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::SetSetRolloffFactor(float afFactor) {
	HPL1_UNIMPLEMENTED(cLowLevelSoundOpenAL::SetSetRolloffFactor);
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::SetVolume(float volume) {
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, static_cast<byte>(volume * 255.f));
	mfVolume = volume;
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::Init(bool abUseHardware, bool abForceGeneric, bool abUseEnvAudio, int alMaxChannels,
								int alStreamUpdateFreq, bool abUseThreading, bool abUseVoiceManagement,
								int alMaxMonoSourceHint, int alMaxStereoSourceHint,
								int alStreamingBufferSize, int alStreamingBufferCount, bool abEnableLowLevelLog, tString asDeviceName) {

	//Default listener settings.
	mvListenerForward = cVector3f(0,0,1);
	mvListenerUp = cVector3f(0,1,0);

	SetVolume(1.0f);
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::SetEnvVolume(float afEnvVolume) {
	HPL1_UNIMPLEMENTED(cLowLevelSoundOpenAL::SetEnvVolume);
}

//-----------------------------------------------------------------------

iSoundEnvironment *cLowLevelSoundOpenAL::LoadSoundEnvironment(const tString &asFilePath) {
	HPL1_UNIMPLEMENTED(cLowLevelSoundOpenAL::LoadSoundEnvironment);
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::SetSoundEnvironment(iSoundEnvironment *apSoundEnv) {
	HPL1_UNIMPLEMENTED(cLowLevelSoundOpenAL::SetSoundEnvironment);
}

//-----------------------------------------------------------------------

void cLowLevelSoundOpenAL::FadeSoundEnvironment(iSoundEnvironment *apSourceSoundEnv, iSoundEnvironment *apDestSoundEnv, float afT) {
	HPL1_UNIMPLEMENTED(cLowLevelSoundOpenAL::FadeSoundEnvironment);
}

} // namespace hpl
