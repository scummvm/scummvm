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

#ifndef HPL_LOWLEVELSOUND_OPENAL_H
#define HPL_LOWLEVELSOUND_OPENAL_H

#include "common/array.h"
#include "hpl1/engine/sound/LowLevelSound.h"

namespace Audio {

class Mixer;

}
namespace hpl {

class cOpenALSoundChannel;

class cLowLevelSoundOpenAL : public iLowLevelSound {
public:
	cLowLevelSoundOpenAL();
	~cLowLevelSoundOpenAL();

	void GetSupportedFormats(tStringList &alstFormats);

	iSoundData *LoadSoundData(const tString &asName, const tString &asFilePath,
							  const tString &asType, bool abStream, bool abLoopStream);

	void UpdateSound(float afTimeStep);

	void SetListenerAttributes(const cVector3f &avPos, const cVector3f &avVel,
							   const cVector3f &avForward, const cVector3f &avUp);
	void SetListenerPosition(const cVector3f &avPos);

	void SetSetRolloffFactor(float afFactor);

	void SetListenerAttenuation(bool abEnabled);

	void Init(bool abUseHardware, bool abForceGeneric, bool abUseEnvAudio, int alMaxChannels,
			  int alStreamUpdateFreq, bool abUseThreading, bool abUseVoiceManagement,
			  int alMaxMonoSourceHint, int alMaxStereoSourceHint,
			  int alStreamingBufferSize, int alStreamingBufferCount, bool abEnableLowLevelLog,
			  tString asDeviceName);

	void SetVolume(float afVolume);

	void SetEnvVolume(float afEnvVolume);

	iSoundEnvironment *LoadSoundEnvironment(const tString &asFilePath);
	void SetSoundEnvironment(iSoundEnvironment *apSoundEnv);
	void FadeSoundEnvironment(iSoundEnvironment *apSourceSoundEnv, iSoundEnvironment *apDestSoundEnv, float afT);

	bool playChannel(cOpenALSoundChannel *channel);
	void closeChannel(cOpenALSoundChannel *channel);

private:
	static const int MAX_ACTIVE_CHANNELS = 32;
	Common::Array<cOpenALSoundChannel *> _activeChannels;
	Audio::Mixer *_mixer;
	tString mvFormats[30];
	bool mbInitialized;
	bool mbNullEffectAttached;
};

} // namespace hpl

#endif // HPL_LOWLEVELSOUND_OPENAL_H
