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

#ifndef HPL_OPENAL_SOUND_CHANNEL_H
#define HPL_OPENAL_SOUND_CHANNEL_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/file.h"
#include "common/str.h"
#include "hpl1/engine/impl/OpenALSoundData.h"
#include "hpl1/engine/sound/SoundChannel.h"
#include "hpl1/engine/sound/SoundData.h"

//#include "OALWrapper/OAL_Funcs.h"

namespace hpl {

class cLowLevelSoundOpenAL;

class cOpenALSoundChannel : public iSoundChannel {
	friend class cLowLevelSoundOpenAL;

public:
	cOpenALSoundChannel(cOpenALSoundData *soundData, Audio::SeekableAudioStream *audioStream, cSoundManager *apSoundManger, cLowLevelSoundOpenAL *lowLevelSound, int priority);
	~cOpenALSoundChannel();

	void Play();
	void Stop();

	void SetPaused(bool abX);
	void SetSpeed(float afSpeed);
	void SetVolume(float afVolume);
	void SetLooping(bool abLoop);
	void SetPan(float afPan);
	void Set3D(bool ab3D);

	void SetPriority(int alX);
	int GetPriority();

	void SetPositionRelative(bool abRelative);
	void SetPosition(const cVector3f &avPos);
	void SetVelocity(const cVector3f &avVel);

	void SetMinDistance(float fMin);
	void SetMaxDistance(float fMax);

	bool IsPlaying();

	bool IsBufferUnderrun() { return false; }
	double GetElapsedTime() { return g_system->getMixer()->getElapsedTime(_handle).secs(); }
	double GetTotalTime() { return _audioStream->getLength().secs(); }

	void SetAffectedByEnv(bool abAffected);
	void SetFiltering(bool abEnabled, int alFlags);
	void SetFilterGain(float afGain);
	void SetFilterGainHF(float afGainHF);

private:
	void restart();

	Audio::SoundHandle _handle;
	Audio::SeekableAudioStream *_audioStream;
	cLowLevelSoundOpenAL *_lowLevelSound;
	int _priority;
	// int mlDefaultFreq;

	// float mfPosition[3];
	// float mfVelocity[3];
};

} // namespace hpl

#endif // HPL_OPENAL_CHANNEL_H
