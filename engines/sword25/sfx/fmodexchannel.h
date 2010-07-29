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

#ifndef SWORD25_FMODEXCHANNEL_H
#define SWORD25_FMODEXCHANNEL_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

struct FMOD_CHANNEL;
struct FMOD_SOUND;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_FMODExChannel
{
public:
	BS_FMODExChannel(FMOD_CHANNEL * ChannelPtr, FMOD_SOUND * SoundPtr);
	virtual ~BS_FMODExChannel();

	bool SetPaused(bool Paused);
	bool SetVolume(float Volume);
	bool SetPanning(float Panning);
	bool SetLoop(bool Loop);
	bool SetLoopPoints(unsigned int LoopStart, unsigned int LoopEnd);
	bool SetPosition(unsigned int Position);
	bool Stop();

	float			GetVolume();
	float			GetPanning();
	unsigned int	GetPosition();
	unsigned int	GetTime();
	unsigned int	GetLoopStart();
	unsigned int	GetLoopEnd();
	bool			IsLooping();
	bool			IsPaused();
	bool			IsPlaying();

private:
	FMOD_CHANNEL *	m_ChannelPtr;
	FMOD_SOUND *	m_SoundPtr;
};

#endif
