/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_FMODEXSOUND_H
#define SWORD25_FMODEXSOUND_H

#include "sword25/sfx/soundengine.h"

namespace Sword25 {

class FMODExSound : public SoundEngine {
public:
	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// -----------------------------------------------------------------------------

	FMODExSound(Kernel *pKernel);
	virtual ~FMODExSound();

	bool            Init(uint SampleRate, uint Channels = 32);
	void            Update();
	void            SetVolume(float Volume, SOUND_TYPES Type);
	float           GetVolume(SOUND_TYPES Type);
	void            PauseAll();
	void            ResumeAll();
	void            PauseLayer(uint Layer);
	void            ResumeLayer(uint Layer);
	bool            PlaySound(const Common::String &FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, uint Layer);
	uint    PlaySoundEx(const Common::String &FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, uint Layer);
	uint    PlayDynamicSoundEx(DynamicSoundReadCallback ReadCallback, void *UserData, SOUND_TYPES Type, uint SampleRate, uint BitsPerSample, uint Channels, float Volume = 1.0f, float Pan = 0.0f, uint Layer = 0);

	void    SetSoundVolume(uint Handle, float Volume);
	void    SetSoundPanning(uint Handle, float Pan);
	void    PauseSound(uint Handle);
	void    ResumeSound(uint Handle);
	void    StopSound(uint Handle);
	bool    IsSoundPaused(uint Handle);
	bool    IsSoundPlaying(uint Handle);
	float   GetSoundVolume(uint Handle);
	float   GetSoundPanning(uint Handle);
	float   GetSoundTime(uint Handle);

	Resource    *LoadResource(const Common::String &FileName);
	bool            CanLoadResource(const Common::String &FileName);

	// -----------------------------------------------------------------------------
	// Persistenz
	// -----------------------------------------------------------------------------

	bool persist(OutputPersistenceBlock &writer);
	bool unpersist(InputPersistenceBlock &reader);

#if 0
private:
	struct PlayingSoundData {
		PlayingSoundData() {};
		PlayingSoundData(BS_Resource *ResourcePtr_, BS_FMODExChannel *ChannelPtr_, SOUND_TYPES Type_, uint Layer_, float Volume_, DynamicSoundReadCallback ReadCallback_ = 0, void *UserData_ = 0) :
			ResourcePtr(ResourcePtr_),
			ChannelPtr(ChannelPtr_),
			Type(Type_),
			Layer(Layer_),
			Volume(Volume_),
			ReadCallback(ReadCallback_),
			UserData(UserData_),
			Paused(false),
			PausedLayer(false),
			PausedGlobal(false)
		{}

		BS_Resource                *ResourcePtr;
		BS_FMODExChannel           *ChannelPtr;
		SOUND_TYPES                 Type;
		uint                Layer;
		DynamicSoundReadCallback    ReadCallback;
		void                       *UserData;

		float   Volume;
		bool    Paused;
		bool    PausedLayer;
		bool    PausedGlobal;
	};

	typedef BS_Hashmap<uint, PlayingSoundData>                  PSM;
	typedef BS_Hashmap<uint, PlayingSoundData>::iterator        PSM_ITER;
	typedef BS_Hashmap<uint, PlayingSoundData>::const_iterator  PSM_CONST_ITER;
	PSM m_PlayingSoundsMap;

	FMOD_SYSTEM    *m_FMOD;
	float           m_Volumes[3];
	uint    m_NextHandle;

	void                RemoveInactiveSounds();
	PlayingSoundData   *GetPlayingSoundDataByHandle(uint Handle);
	uint        PlaySoundInternal(const Common::String &FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, uint Layer, uint Handle, uint Position);
	uint        CountPlayingDynamicSounds();

	static FMOD_RESULT F_CALLBACK FMODExDynamicSoundSetPosCallback(FMOD_SOUND *sound, int subsound, uint position, FMOD_TIMEUNIT postype);
	static FMOD_RESULT F_CALLBACK FMODExDynamicSoundReadCallback(FMOD_SOUND *sound, void *data, uint datalen);
	static FMOD_RESULT F_CALLBACK DSPReadCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, uint length, int inchannels, int outchannels);
#endif
};

} // End of namespace Sword25

#endif
