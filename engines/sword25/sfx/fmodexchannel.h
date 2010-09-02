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

class BS_FMODExChannel {
public:
	BS_FMODExChannel(FMOD_CHANNEL *ChannelPtr, FMOD_SOUND *SoundPtr);
	virtual ~BS_FMODExChannel();

	bool SetPaused(bool Paused);
	bool SetVolume(float Volume);
	bool SetPanning(float Panning);
	bool SetLoop(bool Loop);
	bool SetLoopPoints(uint LoopStart, uint LoopEnd);
	bool SetPosition(uint Position);
	bool Stop();

	float           GetVolume();
	float           GetPanning();
	uint    GetPosition();
	uint    GetTime();
	uint    GetLoopStart();
	uint    GetLoopEnd();
	bool            IsLooping();
	bool            IsPaused();
	bool            IsPlaying();

private:
	FMOD_CHANNEL   *m_ChannelPtr;
	FMOD_SOUND     *m_SoundPtr;
};

#endif
