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

#include "hpl1/engine/impl/OpenALSoundChannel.h"
#include "hpl1/engine/impl/LowLevelSoundOpenAL.h"
#include "hpl1/engine/impl/OpenALSoundData.h"
#include "hpl1/engine/resources/SoundManager.h"

#include "audio/mixer.h"
#include "common/system.h"
#include "hpl1/debug.h"
#include "hpl1/engine/math/Math.h"

#define mixer g_system->getMixer()

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cOpenALSoundChannel::cOpenALSoundChannel(cOpenALSoundData *soundData, Audio::SeekableAudioStream *audioStream, cSoundManager *apSoundManger, cLowLevelSoundOpenAL *lowLevelSound, int priority)
	: iSoundChannel(soundData, apSoundManger), _audioStream(audioStream), _lowLevelSound(lowLevelSound), _priority(priority) {
	Hpl1::logInfo(Hpl1::kDebugAudio, "creating sound channel form file %s\n", mpData->GetName().c_str());
	if (!_audioStream)
		Hpl1::logError(Hpl1::kDebugAudio, "sound channel created with null audio stream%s", ".");
#if 0
  		mlChannel = alChannel;


		for(int i=0;i<3;i++)
		{
			mfPosition[i] = 0;
			mfVelocity[i] = 0;
		}

		OAL_Source_SetAttributes ( mlChannel, mfPosition, mfVelocity );
		OAL_Source_SetFilterType(mlChannel, eOALFilterType_LowPass);
#endif

	//		SetAffectedByEnv(true);
	//		SetFilterGainHF(0.01f);

	//		SetFiltering(true,0x3);
	//		OAL_Source_SetMinMaxDistance ( mlChannel, 100000.0f, 200000.f );
}

//-----------------------------------------------------------------------

cOpenALSoundChannel::~cOpenALSoundChannel() {
	_lowLevelSound->closeChannel(this);
	if (_audioStream)
		delete _audioStream;
	if (mpSoundManger)
		mpSoundManger->Destroy(mpData);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cOpenALSoundChannel::Play() {
	if (!_audioStream) {
		Hpl1::logWarning(Hpl1::kDebugAudio, "trying to play an empty audio stream%c", '\n');
		return;
	}
	Hpl1::logInfo(Hpl1::kDebugAudio, "playing sound channel from data %s\n", mpData->GetName().c_str());
	if (!_lowLevelSound->playChannel(this)) {
		Hpl1::logWarning(Hpl1::kDebugAudio, "sound channel from data %s could not be played\n",
						 mpData->GetName().c_str());
	}
	SetVolume(mfVolume);
	if (mbLooping)
		mixer->loopChannel(_handle);
	mbStopUsed = false;
	mbPaused = false;
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::Stop() {
	Hpl1::logInfo(Hpl1::kDebugAudio, "stopping audio channel from data %s\n", mpData->GetName().c_str());
	mixer->stopHandle(_handle);
	mbStopUsed = true;
	mbLooping = false;
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetPaused(bool pause) {
	Hpl1::logInfo(Hpl1::kDebugAudio, "%spausing sound channel from data %s\n", pause ? "" : "un",
				  mpData->GetName().c_str());
	mixer->pauseHandle(_handle, pause);
	mbPaused = pause;
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetSpeed(float afSpeed) {
	mfSpeed = afSpeed;
#if 0
		OAL_Source_SetPitch ( mlChannel, afSpeed );
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetVolume(float volume) {
	mfVolume = cMath::Clamp(volume, 0.0, 1.0);
	mixer->setChannelVolume(_handle, static_cast<byte>(mfVolume * 255.f));
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetLooping(bool loop) {
	Hpl1::logInfo(Hpl1::kDebugAudio, "%slooping audio from source %s\n", loop ? "" : "un", mpData->GetName().c_str());
	const bool previousState = mbLooping;
	mbLooping = loop;
	if (IsPlaying() && loop) // it has already started
		mixer->loopChannel(_handle);
	else if (previousState && !loop && IsPlaying()) { // unlooped while playing
		_lowLevelSound->closeChannel(this);
		Play();
	}
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetPan(float afPan) {
	// Log("Pan: %d\n", lPan);
	//		cVector3f vPosition = mvPosition;
	//		vPosition.x =
	//		OAL_Source_SetAttributes ( mlChannel, mvPosition.
	//		FSOUND_SetPan(mlChannel, lPan);
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::Set3D(bool ab3D) {
	mb3D = ab3D;
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetPositionRelative(bool abRelative) {
	mbPositionRelative = abRelative;
	//		OAL_Source_SetPositionRelative ( mlChannel, abRelative );
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetPosition(const cVector3f &avPos) {
	mvPosition = avPos;
#if 0
	OAL_Source_SetAttributes ( mlChannel, mvPosition.v, mvVelocity.v );
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetVelocity(const cVector3f &avVel) {
	mvVelocity = avVel;
#if 0
	OAL_Source_SetAttributes ( mlChannel, mvPosition.v, mvVelocity.v );
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetMinDistance(float afMin) {
	mfMinDistance = afMin;
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetMaxDistance(float afMax) {
	mfMaxDistance = afMax;
}
//-----------------------------------------------------------------------

bool cOpenALSoundChannel::IsPlaying() {
	return mixer->isSoundHandleActive(_handle);
}
//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetPriority(int alX) {
	_priority = MAX(alX, 255);
}

//-----------------------------------------------------------------------

int cOpenALSoundChannel::GetPriority() {
	return _priority;
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetAffectedByEnv(bool abAffected) {
	//		if (!(gpGame->GetSound()->GetLowLevel()->IsEnvAudioAvailable()))
	//			return;

#if 0
  		iSoundChannel::SetAffectedByEnv(abAffected);

		if (mbAffectedByEnv)
			OAL_Source_SetAuxSendSlot(mlChannel,0,0);
		else
			OAL_Source_SetAuxSendSlot(mlChannel,0,-1);
#endif
}

void cOpenALSoundChannel::SetFiltering(bool abEnabled, int alFlags) {
#if 0
  //		if (!(gpGame->GetSound()->GetLowLevel()->IsEnvAudioAvailable()))
//			return;

		OAL_Source_SetFiltering(mlChannel,abEnabled, alFlags);
#endif
}

void cOpenALSoundChannel::SetFilterGain(float afGain) {
	HPL1_UNIMPLEMENTED(cOpenALSoundChannel::SetFilterGain);
}

void cOpenALSoundChannel::SetFilterGainHF(float afGainHF) {
	HPL1_UNIMPLEMENTED(cOpenALSoundChannel::SetFilterGainHF);
}

} // namespace hpl
