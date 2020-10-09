/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ICB_FXMANAGER_H__INCLUDED_
#define ICB_FXMANAGER_H__INCLUDED_

#include "engines/icb/sound/direct_sound.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace ICB {

extern bool8 noSoundEngine;

#define SAMPLE_NAME_LENGTH 64

typedef struct Effect {
	enum FxFlags { EMPTY, DELAYED, QUEUED, PLAYING, READY };

	char name[SAMPLE_NAME_LENGTH]; // Sample name
	int delay;                     // The delay until playing
	int looped;                    // Loop this effect

	int pitch;  // Sampling rate (Hz)
	int pan;    // Pan (DirectSound scale)
	int volume; // Volume (DirectSound scale)

	int rate;      // Original buffer sample rate
	FxFlags flags; // Status of sample
	int length;    // Length of sample in millisecs at base rate...
	Audio::RewindableAudioStream *_stream;
	Audio::SoundHandle _handle;
} Effect;

class FxManager {
private:
	Effect m_effects[MAX_FX];

public:
	FxManager();
	~FxManager();

public:
	// Register wavs and unregister (high level load and unload)
	int Register(const int32 id, const char *name, const int delay = 0, uint32 byteOffsetInCluster = 0);
	void Unregister(int id);
	void UnregisterAll();

	// Called on a timer 10 times a second
	bool8 Poll();

	// Pretty important really
	void Play(int id);
	void Stop(int id);

	void StopAll(void);

	// All realtime tweakers
	void SetVolume(int id, int vol);
	void SetPitch(int id, int pitch);

	// A DirectSound buffer can support panning OR 3D position but not both
	// A parameter for Register() lets you decide which to support
	void SetPan(int id, int pan);

	// Helpers and debug shat
	void SetLooping(int id, int loop = 1) { m_effects[id].looped = loop; }
	bool8 IsPlaying(int id) { return (m_effects[id].flags == Effect::PLAYING) ? TRUE8 : FALSE8; }

	int GetDefaultRate(const char *name, uint32 byteOffsetInCluster = 0);

	int GetDefaultLength(int id) { return m_effects[id].length; }

private:
	bool8 Load(int id, const char *name, uint32 byteOffsetInCluster = 0);

	// Can get original sampling rate by channel or examine a wav file
	int GetDefaultRateByID(int id) { return m_effects[id].rate; }
	int GetDefaultRateByName(const char *name, uint32 byteOffsetInCluster = 0);
};

} // End of namespace ICB

#endif
