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
#include "hpl1/engine/impl/OpenALSoundData.h"
#include "hpl1/engine/resources/SoundManager.h"

#include "hpl1/engine/math/Math.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cOpenALSoundChannel::cOpenALSoundChannel(iSoundData *apData, int alChannel, cSoundManager *apSoundManger)
	: iSoundChannel(apData, apSoundManger) {
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
#if 0
  		if(mlChannel>=0)
			OAL_Source_Stop ( mlChannel );

		if(mpSoundManger)mpSoundManger->Destroy(mpData);
#endif
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cOpenALSoundChannel::Play() {
	// Log("Playing %s - Source %d Ref %d\n",mpData->GetName().c_str(), mlChannel & 0xFFF, (mlChannel & 0x7ffff000) >> 12);

	SetPaused(false);

	mbStopUsed = false;
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::Stop() {
#if 0
  		//Log("Stopping %s - Source %d Ref %d\n",mpData->GetName().c_str(), mlChannel & 0xFFF, (mlChannel & 0x7ffff000) >> 12);
		OAL_Source_Stop ( mlChannel );
		mlChannel = -1;

		mbStopUsed = true;
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetPaused(bool abX) {
#if 0
  //		Log("Setting %s %spaused! - Source %d\n",mpData->GetName().c_str(),abX?"":"un", mlChannel);

		mbPaused = abX;
		OAL_Source_SetPaused ( mlChannel, abX );

//		if(mbPaused)
//		{
			//Log("Setting %s paused!\n",mpData->GetName().c_str());
//		}
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetSpeed(float afSpeed) {
#if 0
  		mfSpeed = afSpeed;

		OAL_Source_SetPitch ( mlChannel, afSpeed );
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetVolume(float afVolume) {
#if 0
  		mfVolume = afVolume;

		OAL_Source_SetVolume(mlChannel, mfVolume);
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetLooping(bool abLoop) {
#if 0
  		mbLooping = abLoop;

		OAL_Source_SetLoop ( mlChannel, abLoop );
#endif
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
#if 0
  		mvPosition = avPos;

		OAL_Source_SetAttributes ( mlChannel, mvPosition.v, mvVelocity.v );
#endif
}

//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetVelocity(const cVector3f &avVel) {
#if 0
  		mvVelocity = avVel;

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
#if 0
  		return OAL_Source_IsPlaying( mlChannel );
#endif
	return false;
}
//-----------------------------------------------------------------------

void cOpenALSoundChannel::SetPriority(int alX) {
#if 0
  		int lPrio = alX+mlPriorityModifier;
		if(lPrio>255)lPrio = 255;

		OAL_Source_SetPriority ( mlChannel, lPrio );
#endif
}

//-----------------------------------------------------------------------

int cOpenALSoundChannel::GetPriority() {
#if 0
  		return OAL_Source_GetPriority ( mlChannel );
#endif
	return 0;
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
#if 0
  //		if (!(gpGame->GetSound()->GetLowLevel()->IsEnvAudioAvailable()))
//			return;

		OAL_Source_SetFilterGain(mlChannel, afGain);
#endif
}

void cOpenALSoundChannel::SetFilterGainHF(float afGainHF) {
#if 0
  //		if (!(gpGame->GetSound()->GetLowLevel()->IsEnvAudioAvailable()))
//			return;

		OAL_Source_SetFilterGainHF(mlChannel, afGainHF);
#endif
}

} // namespace hpl
