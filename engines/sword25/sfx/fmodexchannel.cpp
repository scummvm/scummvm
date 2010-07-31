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

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "FMODEXCHANNEL"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/sfx/fmodexexception.h"
#include "sword25/sfx/fmodexchannel.h"

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_FMODExChannel::BS_FMODExChannel(FMOD_CHANNEL * ChannelPtr, FMOD_SOUND * SoundPtr) :
	m_ChannelPtr(ChannelPtr),
	m_SoundPtr(SoundPtr)
{
}

// -----------------------------------------------------------------------------

BS_FMODExChannel::~BS_FMODExChannel()
{
	if (m_ChannelPtr) FMOD_Channel_Stop(m_ChannelPtr);
	if (m_SoundPtr) FMOD_Sound_Release(m_SoundPtr);
}

// -----------------------------------------------------------------------------
// FMOD Ex macht alle Kanäle ungültig, sobald sie nicht mehr abgespielt werden,
// oder wenn der Kanal in der zwischenzeit neu vergeben wurde.
// Dann führen alle Aufrufe von Funktionen dieser Kanäle zu dem Fehlern
// FMOD_ERR_INVALID_HANDLE oder FMOD_ERR_CHANNEL_STOLEN
// Dieses Soundsystem entfernt aber nur jeden Frame alle toten Kanäle. Daher
// kann es vorkommen, dass an einem bereits toten Kanal Aufrufe getätigt werden.
// Diese Fehler werden daher von den folgenden Methoden ignoriert.
// -----------------------------------------------------------------------------

namespace
{
	bool IsImportantError(FMOD_RESULT Result)
	{
		return Result != FMOD_OK && Result != FMOD_ERR_INVALID_HANDLE && Result != FMOD_ERR_CHANNEL_STOLEN;
	}
}

// -----------------------------------------------------------------------------
// Setter
// -----------------------------------------------------------------------------

bool BS_FMODExChannel::SetPaused(bool Paused)
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_RESULT Result = FMOD_Channel_SetPaused(m_ChannelPtr, Paused ? 1 : 0);
	if (IsImportantError(Result))
	{
		BS_FMODExException("FMOD_Channel_SetPaused()", Result).Log();
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::SetVolume(float Volume)
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_RESULT Result = FMOD_Channel_SetVolume(m_ChannelPtr, Volume);
	if (IsImportantError(Result))
	{
		BS_FMODExException("FMOD_Channel_SetVolume()", Result).Log();
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::SetPanning(float Panning)
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_RESULT Result = FMOD_Channel_SetPan(m_ChannelPtr, Panning);
	if (IsImportantError(Result))
	{
		BS_FMODExException("FMOD_Channel_SetPan()", Result).Log();
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::SetLoop(bool Loop)
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_RESULT Result = FMOD_Channel_SetLoopCount(m_ChannelPtr, Loop ? -1 : 0);
	if (IsImportantError(Result))
	{
		BS_FMODExException("FMOD_Channel_SetLoopCount()", Result).Log();
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::SetLoopPoints(unsigned int LoopStart, unsigned int LoopEnd)
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_RESULT Result = FMOD_Channel_SetLoopPoints(m_ChannelPtr, LoopStart, FMOD_TIMEUNIT_PCM, LoopEnd, FMOD_TIMEUNIT_PCM);
	if (IsImportantError(Result))
	{
		BS_FMODExException("FMOD_Channel_SetLoopPoints()", Result).Log();
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::SetPosition(unsigned int Position)
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_RESULT Result = FMOD_Channel_SetPosition(m_ChannelPtr, Position, FMOD_TIMEUNIT_PCM);
	if (IsImportantError(Result))
	{
		BS_FMODExException("FMOD_Channel_SetPosition()", Result).Log();
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::Stop()
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_RESULT Result = FMOD_Channel_Stop(m_ChannelPtr);
	if (IsImportantError(Result))
	{
		BS_FMODExException("FMOD_Channel_Stop()", Result).Log();
		return false;
	}
	else
		return true;
}

// -----------------------------------------------------------------------------
// Getter
// -----------------------------------------------------------------------------

float BS_FMODExChannel::GetVolume()
{
	BS_ASSERT(m_ChannelPtr);

	float Volume = 0;
	FMOD_RESULT Result = FMOD_Channel_GetVolume(m_ChannelPtr, &Volume);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetVolume()", Result).Log();
	
	return Volume;
}

// -----------------------------------------------------------------------------

float BS_FMODExChannel::GetPanning()
{
	BS_ASSERT(m_ChannelPtr);

	float Panning = 0;
	FMOD_RESULT Result = FMOD_Channel_GetPan(m_ChannelPtr, &Panning);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetPan()", Result).Log();
	
	return Panning;
}

// -----------------------------------------------------------------------------

unsigned int BS_FMODExChannel::GetPosition()
{
	BS_ASSERT(m_ChannelPtr);

	unsigned int Position = 0;
	FMOD_RESULT Result = FMOD_Channel_GetPosition(m_ChannelPtr, &Position, FMOD_TIMEUNIT_PCM);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetPosition()", Result).Log();

	return Position;
}

// -----------------------------------------------------------------------------

unsigned int BS_FMODExChannel::GetTime()
{
	BS_ASSERT(m_ChannelPtr);

	unsigned int Time = 0;
	FMOD_RESULT Result = FMOD_Channel_GetPosition(m_ChannelPtr, &Time, FMOD_TIMEUNIT_MS);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetPosition()", Result).Log();

	return Time;
}

// -----------------------------------------------------------------------------

unsigned int BS_FMODExChannel::GetLoopStart()
{
	BS_ASSERT(m_ChannelPtr);
	unsigned int LoopStart = 0;
	FMOD_RESULT Result = FMOD_Channel_GetLoopPoints(m_ChannelPtr, &LoopStart, FMOD_TIMEUNIT_PCM, 0, FMOD_TIMEUNIT_PCM);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetLoopPoints()", Result).Log();

	return LoopStart;
}

// -----------------------------------------------------------------------------

unsigned int BS_FMODExChannel::GetLoopEnd()
{
	BS_ASSERT(m_ChannelPtr);
	unsigned int LoopEnd = 0;
	FMOD_RESULT Result = FMOD_Channel_GetLoopPoints(m_ChannelPtr, 0, FMOD_TIMEUNIT_PCM, &LoopEnd, FMOD_TIMEUNIT_PCM);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetLoopPoints()", Result).Log();

	return LoopEnd;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::IsLooping()
{
	BS_ASSERT(m_ChannelPtr);

	int LoopCount = 0;
	FMOD_RESULT Result = FMOD_Channel_GetLoopCount(m_ChannelPtr, &LoopCount);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetLoopCount()", Result).Log();
	
	return LoopCount == -1;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::IsPaused()
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_BOOL Paused = 0;
	FMOD_RESULT Result = FMOD_Channel_GetPaused(m_ChannelPtr, &Paused);
	if (IsImportantError(Result)) BS_FMODExException("FMOD_Channel_GetPaused()", Result).Log();
	
	return Paused != 0;
}

// -----------------------------------------------------------------------------

bool BS_FMODExChannel::IsPlaying()
{
	BS_ASSERT(m_ChannelPtr);

	FMOD_BOOL Playing = 0;
	FMOD_RESULT Result = FMOD_Channel_IsPlaying(m_ChannelPtr, &Playing);
	if (IsImportantError(Result))	BS_FMODExException("FMOD_Channel_IsPlaying()", Result).Log();

	return Playing != 0;
}
