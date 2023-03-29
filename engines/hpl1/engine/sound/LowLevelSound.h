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

#ifndef HPL_LOWLEVELSOUND_H
#define HPL_LOWLEVELSOUND_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

class iSoundData;
class iSoundEnvironment;
// class iSoundFilter;

typedef Common::List<iSoundEnvironment *> tSoundEnvList;
typedef tSoundEnvList::iterator tSoundEnvListIt;

class iLowLevelSound {
public:
	iLowLevelSound();
	virtual ~iLowLevelSound();

	/**
	 * Get the formats supported
	 * \param &alstFormats
	 */
	virtual void GetSupportedFormats(tStringList &alstFormats) = 0;

	virtual iSoundData *LoadSoundData(const tString &asName, const tString &asFilePath,
									  const tString &asType, bool abStream, bool abLoopStream) = 0;

	virtual void UpdateSound(float afTimeStep) = 0;

	virtual void SetListenerAttributes(const cVector3f &avPos, const cVector3f &avVel,
									   const cVector3f &avForward, const cVector3f &avUp) = 0;
	virtual void SetListenerPosition(const cVector3f &avPos) = 0;
	const cMatrixf &GetListenerMatrix() { return m_mtxListener; }

	virtual void SetListenerAttenuation(bool abEnabled) = 0;

	virtual void SetSetRolloffFactor(float afFactor) = 0;

	virtual void SetVolume(float afVolume) = 0;

	cVector3f &GetListenerPosition() { return mvListenerPosition; }
	cVector3f &GetListenerVelocity() { return mvListenerVelocity; }
	cVector3f &GetListenerForward() { return mvListenerForward; }
	cVector3f &GetListenerUp() { return mvListenerUp; }
	bool GetListenerAttenuation() { return mbListenerAttenuation; }

	float GetVolume() { return mfVolume; }

	// virtual void LogSoundStatus() {}
	virtual void Init(bool abUseHardware, bool abForceGeneric, bool abUseEnvAudio, int alMaxChannels,
					  int alStreamUpdateFreq, bool abUseThreading, bool abUseVoiceManagement,
					  int alMaxMonoSourceHint, int alMaxStereoSourceHint,
					  int alStreamingBufferSize, int alStreamingBufferCount, bool abEnableLowLevelLog, tString asDeviceName) = 0;

	bool IsHardwareAccelerated() { return mbHardwareAcc; }
	bool IsEnvAudioAvailable() { return mbEnvAudioEnabled; }

	virtual void SetEnvVolume(float afVolume) = 0;
	float GetEnvVolume() { return mfEnvVolume; }

	virtual iSoundEnvironment *LoadSoundEnvironment(const tString &asFilePath) = 0;

	virtual void SetSoundEnvironment(iSoundEnvironment *apSoundEnv) = 0;
	virtual void FadeSoundEnvironment(iSoundEnvironment *apSourceSoundEnv, iSoundEnvironment *apDestSoundEnv, float afT) = 0;

	iSoundEnvironment *GetSoundEnvironmentFromFileName(const tString &asName);
	// void DestroySoundEnvironment( iSoundEnvironment* apSoundEnv);

protected:
	float mfVolume;
	float mfEnvVolume;
	bool mbListenerAttenuation;

	bool mbHardwareAcc;
	bool mbEnvAudioEnabled;

	cVector3f mvListenerUp;
	cVector3f mvListenerForward;
	cVector3f mvListenerRight;
	cVector3f mvListenerPosition;
	cVector3f mvListenerVelocity;

	cMatrixf m_mtxListener;

	tSoundEnvList mlstSoundEnv;
};

} // namespace hpl

#endif // HPL_LOWLEVELSOUND_H
