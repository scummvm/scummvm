// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef SWORD25_FMODEXSOUND_H
#define SWORD25_FMODEXSOUND_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include <map>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/hashmap.h"
#include "sword25/sfx/soundengine.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_FMODExChannel;
struct FMOD_SYSTEM;
struct FMOD_CHANNEL;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_FMODExSound : public BS_SoundEngine
{
public:
	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// -----------------------------------------------------------------------------
	
	BS_FMODExSound(BS_Kernel* pKernel);
	virtual ~BS_FMODExSound();

	bool			Init(unsigned int SampleRate, unsigned int Channels = 32);
	void			Update();
	void			SetVolume(float Volume, SOUND_TYPES Type);
	float			GetVolume(SOUND_TYPES Type);
	void			PauseAll();
	void			ResumeAll();
	void			PauseLayer(unsigned int Layer);
	void			ResumeLayer(unsigned int Layer);
	bool			PlaySound(const std::string& FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, unsigned int Layer);
	unsigned int	PlaySoundEx(const std::string& FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, unsigned int Layer);
	unsigned int	PlayDynamicSoundEx(DynamicSoundReadCallback ReadCallback, void * UserData, SOUND_TYPES Type, unsigned int SampleRate, unsigned int BitsPerSample, unsigned int Channels, float Volume = 1.0f, float Pan = 0.0f, unsigned int Layer = 0);

	void	SetSoundVolume(unsigned int Handle, float Volume);
	void	SetSoundPanning(unsigned int Handle, float Pan);
	void	PauseSound(unsigned int Handle);
	void	ResumeSound(unsigned int Handle);
	void	StopSound(unsigned int Handle);
	bool	IsSoundPaused(unsigned int Handle);
	bool	IsSoundPlaying(unsigned int Handle);
	float	GetSoundVolume(unsigned int Handle);
	float	GetSoundPanning(unsigned int Handle);
	float	GetSoundTime(unsigned int Handle);

	BS_Resource *	LoadResource(const std::string& FileName);
	bool			CanLoadResource(const std::string& FileName);

	// -----------------------------------------------------------------------------
	// Persistenz
	// -----------------------------------------------------------------------------
	
	bool Persist(BS_OutputPersistenceBlock & Writer);
	bool Unpersist(BS_InputPersistenceBlock & Reader);
	
private:
	struct PlayingSoundData
	{
		PlayingSoundData() {};
		PlayingSoundData(BS_Resource * ResourcePtr_, BS_FMODExChannel * ChannelPtr_, SOUND_TYPES Type_, unsigned int Layer_, float Volume_, DynamicSoundReadCallback ReadCallback_ = 0, void * UserData_ = 0) :
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

		BS_Resource *				ResourcePtr;
		BS_FMODExChannel *			ChannelPtr;
		SOUND_TYPES					Type;
		unsigned int				Layer;
		DynamicSoundReadCallback	ReadCallback;
		void *						UserData;

		float	Volume;
		bool	Paused;
		bool	PausedLayer;
		bool	PausedGlobal;
	};

	typedef BS_Hashmap<unsigned int, PlayingSoundData>					PSM;
	typedef BS_Hashmap<unsigned int, PlayingSoundData>::iterator		PSM_ITER;
	typedef BS_Hashmap<unsigned int, PlayingSoundData>::const_iterator	PSM_CONST_ITER;
	PSM m_PlayingSoundsMap;

	FMOD_SYSTEM *	m_FMOD;
	float			m_Volumes[3];
	unsigned int	m_NextHandle;

	void				RemoveInactiveSounds();
	PlayingSoundData *	GetPlayingSoundDataByHandle(unsigned int Handle);
	unsigned int		PlaySoundInternal(const std::string& FileName, SOUND_TYPES Type, float Volume, float Pan, bool Loop, int LoopStart, int LoopEnd, unsigned int Layer, unsigned int Handle, unsigned int Position);
	unsigned int		CountPlayingDynamicSounds();

	static FMOD_RESULT F_CALLBACK FMODExDynamicSoundSetPosCallback(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
	static FMOD_RESULT F_CALLBACK FMODExDynamicSoundReadCallback(FMOD_SOUND *sound, void *data, unsigned int datalen);
	static FMOD_RESULT F_CALLBACK DSPReadCallback(FMOD_DSP_STATE * dsp_state, float * inbuffer,	float * outbuffer, unsigned int length, int inchannels, int outchannels);
};

#endif
